#include "systems/physics_scene.h"
#include "core/engine.h"

#include <cmath>
#include <algorithm>

namespace Simple {
    namespace Physics {

        SceneBodies::SceneBodies(SG::Scene &_scene, World &_world) : scene(_scene), world(_world), ecs(_world.ecs) {}

        SceneBodies::~SceneBodies() {}


//  == === ==== >  System

        void SceneBodies::attach(Core::Engine &_engine) {
            events_ = &_engine.events;
            subscriptions_.push_back(events_->subscribe(Event::EV_TYPE_SCENE_MODEL_ADDED, [this](Event::EventHandler &, const Event::Event &_e) {
                const Event::SceneModelEvent __model = _e.get<Event::SceneModelEvent>();
                addModel(__model.root, __model.skin);
            }));
            subscriptions_.push_back(events_->subscribe(Event::EV_TYPE_SCENE_CLEARED, [this](Event::EventHandler &, const Event::Event &) { clear(); }));
            subscriptions_.push_back(events_->subscribe(Event::EV_TYPE_SCENE_RESET,   [this](Event::EventHandler &, const Event::Event &) { requestReset(); }));
        }

        void SceneBodies::detach(Core::Engine &_engine) {
            if (events_ == &_engine.events)
                for (size_t i = 0u; i < subscriptions_.size(); ++i) events_->unsubscribe(subscriptions_[i]);
            subscriptions_.clear();
            events_ = nullptr;
        }

        void SceneBodies::update(Core::Phase _phase, const Core::FrameTime &) {
            if (_phase == Core::PHASE_ANIMATION) animate_();
            else if (_phase == Core::PHASE_SCENE) writeBack_();
        }

        void SceneBodies::simulate(float _dt) {
            animate_();
            world.update(_dt);
            writeBack_();
        }


//  == === ==== >  Poses

        void SceneBodies::animatedPose_(uint32_t _skin) {
            SG::Skin &__skin = scene.skins[_skin];
            __skin.pose.use_physics = false;
            __skin.pose.computeFK(scene.skeletons[__skin.skeleton]);
        }

        bool SceneBodies::bodyPose_(const SceneLink &_link, vec3 &_position, Quat &_rotation) const {
            if (_link.target == LINK_NODE) {
                if (_link.node >= scene.world.size()) return false;
                const mat4 &W = scene.world[_link.node];
                _position = SG::transformPoint(W, SG::translationOf(_link.offset));
                _rotation = SG::rotationOf(W).normalize();
                return true;
            }
            if (_link.skin >= scene.skins.size()) return false;
            const SG::Skin &__skin = scene.skins[_link.skin];
            const mat4 __bone = _link.bone == SG::SCENE_NONE || _link.bone >= __skin.pose.model.size() ? Mat::Eye4 : __skin.pose.model[_link.bone];
            const mat4 __skel = _link.offset & __bone;
            const mat4 &W = scene.world[__skin.node];
            _position = SG::transformPoint(W, SG::translationOf(__skel));
            _rotation = (SG::rotationOf(W) * SG::rotationOf(__skel)).normalize();
            return true;
        }


//  == === ==== >  Models and bodies

        void SceneBodies::skeletonBodies_(PhysicsModel &_model) {
            const SG::Skin &__skin = scene.skins[_model.skin];
            const SG::Skeleton &__sk = scene.skeletons[__skin.skeleton];
            const float s = _model.scale;

            std::vector<mat4> __bind(__sk.rigid_bodies.size());
            for (size_t i = 0u; i < __sk.rigid_bodies.size(); ++i) {
                const SG::RigidBodyDesc &d = __sk.rigid_bodies[i];
                __bind[i] = SG::rigidTransform(d.rotation, d.position);

                SceneLink __link;
                __link.target = LINK_BONE;
                __link.skin = _model.skin;
                __link.bone = d.bone < __sk.bones.size() ? d.bone : SG::SCENE_NONE;
                __link.mode = d.mode;
                __link.offset = __link.bone == SG::SCENE_NONE ? __bind[i] : (__bind[i] & __sk.bones[__link.bone].inverse_bind);
                __link.inverse_offset = __link.offset.inverse<float>();

                Body __body;
                bodyPose_(__link, __body.position, __body.rotation);
                __body.mass            = d.mass > 0.f ? d.mass : 1.f;
                __body.linear_damping  = d.linear_damping;
                __body.angular_damping = d.angular_damping;

                Collider __col;
                switch (d.shape) {
                    case SG::SCENE_RIGID_BOX:     __col = Collider::box(d.size * s); break;
                    case SG::SCENE_RIGID_CAPSULE: __col = Collider::capsule(d.size.x * s, d.size.y * s); break;
                    default:                      __col = Collider::sphere(d.size.x * s);
                }
                __col.friction    = d.friction;
                __col.restitution = d.restitution;
                __col.group = 1u << (d.group & 15u);
                __col.mask  = d.collision_mask & 0xFFFFu;

                const EntityID e = world.createBody(__body, __col, d.mode == SG::SCENE_RIGID_FOLLOW_BONE ? TagID::TAG_PHYSICS_KINEMATIC
                                                                                                         : TagID::TAG_PHYSICS_DYNAMIC);
                ecs.add<SceneLink>(e, __link);
                _model.bodies.push_back(e);
            }

            for (size_t i = 0u; i < __sk.joints.size(); ++i) {
                const SG::JointDesc &d = __sk.joints[i];
                if (d.body_a >= _model.bodies.size() || d.body_b >= _model.bodies.size()) continue;
                const mat4 __joint = SG::rigidTransform(d.rotation, d.position);
                // The joint frame in the body spaces (bind pose); positions scaled like the world
                const mat4 __in_a = __joint & __bind[d.body_a].inverse<float>();
                const mat4 __in_b = __joint & __bind[d.body_b].inverse<float>();
                Joint j;
                j.body_a   = _model.bodies[d.body_a];
                j.body_b   = _model.bodies[d.body_b];
                j.anchor_a = SG::translationOf(__in_a) * s;
                j.anchor_b = SG::translationOf(__in_b) * s;
                j.frame_a  = SG::rotationOf(__in_a).normalize();
                j.frame_b  = SG::rotationOf(__in_b).normalize();
                j.linear_min  = d.linear_min * s;  j.linear_max  = d.linear_max * s;
                j.angular_min = d.angular_min;     j.angular_max = d.angular_max;
                j.linear_spring  = d.linear_spring;
                j.angular_spring = d.angular_spring;
                _model.joints.push_back(world.createJoint(j));
            }
        }

        EntityID SceneBodies::addModel(uint32_t _root, uint32_t _skin) {
            const bool __skinned = _skin < scene.skins.size();
            const bool __rigid   = __skinned && !scene.skeletons[scene.skins[_skin].skeleton].rigid_bodies.empty();
            if (!__rigid && (!auto_colliders || _root >= scene.nodes.size())) return EntityID::None;
            scene.updateTransforms();

            PhysicsModel __model;
            __model.root = _root;
            __model.skin = __skinned ? _skin : SG::SCENE_NONE;
            const uint32_t __node = __skinned ? scene.skins[_skin].node : _root;
            __model.scale = __node < scene.world.size() ? SG::uniformScaleOf(scene.world[__node]) : 1.f;
            if (__model.scale <= 0.f) __model.scale = 1.f;
            const EntityID __entity = ecs.create();

            if (__rigid) {
                animatedPose_(_skin);
                if (models_.empty()) world.settings.gravity = vec3{0.f, -mmd_gravity * __model.scale, 0.f};
                skeletonBodies_(__model);
                ecs.add<PhysicsModel>(__entity, __model);
            }
            else {
                ecs.add<PhysicsModel>(__entity, __model);
                if (addColliders(_root, auto_fit, auto_motion, __entity) == 0u) { ecs.destroy(__entity); return EntityID::None; }
            }
            models_.push_back(__entity);
            requestReset();
            return __entity;
        }

        Collider SceneBodies::fitCollider(const SG::AABB &_box, ColliderFit _fit, float _scale) {
            const vec3 __size = _box.size() * _scale;
            switch (_fit) {
                case FIT_SPHERE:
                    return Collider::sphere(std::max(0.5f * __size.norm(), 1e-3f));
                case FIT_CAPSULE: {
                    const float __r = std::max(0.5f * std::max(__size.x, __size.z), 1e-3f);
                    return Collider::capsule(__r, std::max(__size.y - 2.f * __r, 0.f));
                }
                default:
                    return Collider::box({std::max(0.5f * __size.x, 1e-3f), std::max(0.5f * __size.y, 1e-3f), std::max(0.5f * __size.z, 1e-3f)});
            }
        }

        EntityID SceneBodies::addBody(uint32_t _node, const Collider &_collider, TagID _motion, const vec3 &_center, EntityID _model) {
            if (_node >= scene.nodes.size()) return EntityID::None;
            scene.updateTransforms();
            SceneLink __link;
            __link.target = LINK_NODE;
            __link.node   = _node;
            __link.mode   = _motion == TagID::TAG_PHYSICS_DYNAMIC ? SG::SCENE_RIGID_PHYSICS : SG::SCENE_RIGID_FOLLOW_BONE;
            __link.offset = SG::translation(_center);
            __link.inverse_offset = SG::translation(-_center);
            __link.scale  = SG::uniformScaleOf(scene.world[_node]);
            if (__link.scale <= 0.f) __link.scale = 1.f;

            Body __body;
            bodyPose_(__link, __body.position, __body.rotation);
            const EntityID e = world.createBody(__body, _collider, _motion);
            ecs.add<SceneLink>(e, __link);
            node_bodies_.push_back(e);
            if (PhysicsModel *__model = ecs.get<PhysicsModel>(_model)) __model->bodies.push_back(e);
            return e;
        }

        uint32_t SceneBodies::addColliders(uint32_t _root, ColliderFit _fit, TagID _motion, EntityID _model) {
            if (_root >= scene.nodes.size()) return 0u;
            scene.updateTransforms();
            uint32_t __count = 0u;
            std::vector<uint32_t> __stack(1u, _root);
            while (!__stack.empty()) {
                const uint32_t n = __stack.back();
                __stack.pop_back();
                const SG::Node &__node = scene.nodes[n];
                for (size_t c = 0u; c < __node.children.size(); ++c) __stack.push_back(__node.children[c]);
                SG::AABB __box;
                for (size_t m = 0u; m < __node.meshes.size(); ++m)
                    if (__node.meshes[m] < scene.meshes.size()) __box.add(scene.meshes[__node.meshes[m]].bounds);
                if (!__box.valid()) continue;
                const float __scale = std::max(SG::uniformScaleOf(scene.world[n]), 1e-6f);
                if (!addBody(n, fitCollider(__box, _fit, __scale), _motion, __box.center(), _model).isNone()) ++__count;
            }
            return __count;
        }

        void SceneBodies::clear() {
            for (size_t i = 0u; i < models_.size(); ++i) {
                if (const PhysicsModel *m = ecs.get<PhysicsModel>(models_[i])) {
                    for (const EntityID &e : m->joints) ecs.destroy(e);
                    for (const EntityID &e : m->bodies) ecs.destroy(e);
                }
                ecs.destroy(models_[i]);
            }
            models_.clear();
            node_bodies_.erase(std::remove_if(node_bodies_.begin(), node_bodies_.end(),
                                              [this](const EntityID &_e) { return !ecs.isValid(_e); }), node_bodies_.end());
            world.removeBrokenJoints();
            world.resetTime();
            reset_pending_ = false;
        }

        void SceneBodies::reset() {
            scene.updateTransforms();
            for (size_t i = 0u; i < models_.size(); ++i) {
                const PhysicsModel *m = ecs.get<PhysicsModel>(models_[i]);
                if (m && m->skin < scene.skins.size()) animatedPose_(m->skin);
            }
            ecs.each<SceneLink>([this](EntityID _e, SceneLink &_link) {
                vec3 p;
                Quat q;
                if (bodyPose_(_link, p, q)) world.teleport(_e, p, q, true);
            });
            world.resetTime();
            reset_pending_ = false;
        }


//  == === ==== >  Per frame

        void SceneBodies::animate_() {
            scene.updateTransforms();
            if (!enabled) {
                for (size_t i = 0u; i < models_.size(); ++i) {
                    const PhysicsModel *m = ecs.get<PhysicsModel>(models_[i]);
                    if (m && m->skin < scene.skins.size() && scene.skins[m->skin].pose.use_physics) animatedPose_(m->skin);
                }
                return;
            }
            if (reset_pending_) reset();
            // The animated pose; bodies that follow bones and nodes get their targets
            for (size_t i = 0u; i < models_.size(); ++i) {
                const PhysicsModel *m = ecs.get<PhysicsModel>(models_[i]);
                if (m && m->skin < scene.skins.size()) animatedPose_(m->skin);
            }
            ecs.each<SceneLink>([this](EntityID _e, SceneLink &_link) {
                vec3 p;
                Quat q;
                if (bodyPose_(_link, p, q)) world.setKinematicTarget(_e, p, q);
            }, TagFilter().all(TagID::TAG_PHYSICS_KINEMATIC));
        }

        void SceneBodies::writeBack_() {
            if (!enabled) return;
            // Dynamic bodies drive their bones
            for (size_t i = 0u; i < models_.size(); ++i) {
                const PhysicsModel *m = ecs.get<PhysicsModel>(models_[i]);
                if (!m || m->skin >= scene.skins.size()) continue;
                SG::Skin &__skin = scene.skins[m->skin];
                SG::Pose &__pose = __skin.pose;
                const SG::Skeleton &__sk = scene.skeletons[__skin.skeleton];
                const mat4 &W = scene.world[__skin.node];
                const mat4 __inv_w = W.inverse<float>();
                const Quat __inv_qw = SG::rotationOf(W).normalize().conjugate();
                __pose.physics_override.assign(__sk.bones.size(), 0u);
                __pose.physics_model.resize(__sk.bones.size());
                for (const EntityID &e : m->bodies) {
                    const SceneLink *__link = ecs.get<SceneLink>(e);
                    const Body *__body = ecs.get<Body>(e);
                    if (!__link || !__body || __link->target != LINK_BONE || __link->bone == SG::SCENE_NONE ||
                        !ecs.hasTag(e, TagID::TAG_PHYSICS_DYNAMIC) || ecs.hasTag(e, TagID::TAG_PHYSICS_DISABLED)) continue;
                    const mat4 __skel = SG::rigidTransform((__inv_qw * __body->rotation).normalize(), SG::transformPoint(__inv_w, __body->position));
                    mat4 __bone = __link->inverse_offset & __skel;
                    if (__link->mode == SG::SCENE_RIGID_PHYSICS_BONE_POSITION) {
                        const vec3 __animated = SG::translationOf(__pose.model[__link->bone]);
                        __bone.M[3][0] = __animated.x; __bone.M[3][1] = __animated.y; __bone.M[3][2] = __animated.z;
                    }
                    __pose.physics_model[__link->bone] = __bone;
                    __pose.physics_override[__link->bone] = 1u;
                }
                __pose.use_physics = true;
                __pose.computeFK(__sk);
            }

            // Dynamic node bodies move their nodes (world = scale & rotation & translation, the center stays at the body).
            // Note: a dynamic node under another dynamic node uses the parent's previous world transform (one frame behind)
            bool __moved = false;
            for (size_t i = 0u; i < node_bodies_.size(); ++i) {
                const EntityID e = node_bodies_[i];
                const SceneLink *__link = ecs.get<SceneLink>(e);
                const Body *__body = ecs.get<Body>(e);
                if (!__link || !__body || __link->node >= scene.nodes.size() || !ecs.hasTag(e, TagID::TAG_PHYSICS_DYNAMIC) ||
                    ecs.hasTag(e, TagID::TAG_PHYSICS_DISABLED)) continue;
                const float s = __link->scale;
                const vec3 __center = SG::transformDir(SG::rotation(__body->rotation), SG::translationOf(__link->offset) * s);
                const mat4 __world = SG::scaling({s, s, s}) & SG::rigidTransform(__body->rotation, __body->position - __center);
                const uint32_t __parent = scene.nodes[__link->node].parent;
                scene.setLocal(__link->node, __parent < scene.world.size() ? (__world & scene.world[__parent].inverse<float>()) : __world);
                __moved = true;
            }
            if (__moved) scene.updateTransforms();
        }
    }; // Physics END
}; // Simple END
