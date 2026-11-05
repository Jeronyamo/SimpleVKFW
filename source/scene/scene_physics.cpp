#include "scene/scene_physics.h"

#include <cmath>

namespace Simple {
    namespace SceneGraph {

        namespace {
            vec3 rowTranslation(const mat4 &_m) { return { _m.M[3][0], _m.M[3][1], _m.M[3][2] }; }

            float uniformScale(const mat4 &_m) {
                return std::sqrt(_m.M[0][0] * _m.M[0][0] + _m.M[0][1] * _m.M[0][1] + _m.M[0][2] * _m.M[0][2]);
            }

            // Rotation then translation (row-vector style)
            mat4 rigid(const Quat &_q, const vec3 &_p) {
                mat4 __m = rotation(_q);
                __m.M[3][0] = _p.x; __m.M[3][1] = _p.y; __m.M[3][2] = _p.z;
                return __m;
            }
        } // anonymous namespace END

        MMDPhysics::MMDPhysics() : world(ecs) {}

        void MMDPhysics::animatedPose_(Scene &_scene, uint32_t _skin) {
            Skin &__skin = _scene.skins[_skin];
            __skin.pose.use_physics = false;
            __skin.pose.computeFK(_scene.skeletons[__skin.skeleton]);
        }

        void MMDPhysics::bodyPose_(const Scene &_scene, const BoneLink &_link, vec3 &_position, Quat &_rotation) const {
            const Skin &__skin = _scene.skins[_link.skin];
            const mat4 __bone = _link.bone == SCENE_NONE || _link.bone >= __skin.pose.model.size() ? Mat::Eye4 : __skin.pose.model[_link.bone];
            const mat4 __skel = _link.offset & __bone;
            const mat4 &W = _scene.world[__skin.node];
            _position = transformPoint(W, rowTranslation(__skel));
            _rotation = (rotationOf(W) * rotationOf(__skel)).normalize();
        }

        void MMDPhysics::addModel(Scene &_scene, uint32_t _skin) {
            if (_skin >= _scene.skins.size()) return;
            const Skin &__skin = _scene.skins[_skin];
            const Skeleton &__sk = _scene.skeletons[__skin.skeleton];
            if (__sk.rigid_bodies.empty()) return;
            _scene.updateTransforms();
            animatedPose_(_scene, _skin);

            Model __model;
            __model.skin  = _skin;
            __model.scale = uniformScale(_scene.world[__skin.node]);
            if (__model.scale <= 0.f) __model.scale = 1.f;
            const float s = __model.scale;
            if (models_.empty()) world.settings.gravity = vec3{0.f, -mmd_gravity * s, 0.f};

            std::vector<mat4> __bind(__sk.rigid_bodies.size());
            for (size_t i = 0u; i < __sk.rigid_bodies.size(); ++i) {
                const RigidBodyDesc &d = __sk.rigid_bodies[i];
                __bind[i] = rigid(d.rotation, d.position);

                BoneLink __link;
                __link.skin = _skin;
                __link.bone = d.bone < __sk.bones.size() ? d.bone : SCENE_NONE;
                __link.mode = d.mode;
                __link.offset = __link.bone == SCENE_NONE ? __bind[i] : (__bind[i] & __sk.bones[__link.bone].inverse_bind);
                __link.inverse_offset = __link.offset.inverse<float>();

                Physics::Body __body;
                bodyPose_(_scene, __link, __body.position, __body.rotation);
                __body.mass            = d.mass > 0.f ? d.mass : 1.f;
                __body.linear_damping  = d.linear_damping;
                __body.angular_damping = d.angular_damping;

                Physics::Collider __col;
                switch (d.shape) {
                    case SCENE_RIGID_BOX:     __col = Physics::Collider::box(d.size * s); break;
                    case SCENE_RIGID_CAPSULE: __col = Physics::Collider::capsule(d.size.x * s, d.size.y * s); break;
                    default:                  __col = Physics::Collider::sphere(d.size.x * s);
                }
                __col.friction    = d.friction;
                __col.restitution = d.restitution;
                __col.group = 1u << (d.group & 15u);
                __col.mask  = d.collision_mask & 0xFFFFu;

                const EntityID e = world.createBody(__body, __col, d.mode == SCENE_RIGID_FOLLOW_BONE ? TagID::TAG_PHYSICS_KINEMATIC
                                                                                                       : TagID::TAG_PHYSICS_DYNAMIC);
                ecs.add<BoneLink>(e, __link);
                __model.bodies.push_back(e);
            }

            for (size_t i = 0u; i < __sk.joints.size(); ++i) {
                const JointDesc &d = __sk.joints[i];
                if (d.body_a >= __model.bodies.size() || d.body_b >= __model.bodies.size()) continue;
                const mat4 __joint = rigid(d.rotation, d.position);
                // The joint frame in the body spaces (bind pose); positions scaled like the world
                const mat4 __in_a = __joint & __bind[d.body_a].inverse<float>();
                const mat4 __in_b = __joint & __bind[d.body_b].inverse<float>();
                Physics::Joint j;
                j.body_a   = __model.bodies[d.body_a];
                j.body_b   = __model.bodies[d.body_b];
                j.anchor_a = rowTranslation(__in_a) * s;
                j.anchor_b = rowTranslation(__in_b) * s;
                j.frame_a  = rotationOf(__in_a).normalize();
                j.frame_b  = rotationOf(__in_b).normalize();
                j.linear_min  = d.linear_min * s;  j.linear_max  = d.linear_max * s;
                j.angular_min = d.angular_min;     j.angular_max = d.angular_max;
                j.linear_spring  = d.linear_spring;
                j.angular_spring = d.angular_spring;
                __model.joints.push_back(world.createJoint(j));
            }
            models_.push_back(__model);
        }

        void MMDPhysics::clear() {
            for (const Model &m : models_) {
                for (const EntityID &e : m.joints) ecs.destroy(e);
                for (const EntityID &e : m.bodies) ecs.destroy(e);
            }
            models_.clear();
            world.removeBrokenJoints();
            world.resetTime();
        }

        void MMDPhysics::reset(Scene &_scene) {
            _scene.updateTransforms();
            for (const Model &m : models_) {
                if (m.skin >= _scene.skins.size()) continue;
                animatedPose_(_scene, m.skin);
                for (const EntityID &e : m.bodies) {
                    const BoneLink *__link = ecs.get<BoneLink>(e);
                    if (!__link) continue;
                    vec3 p;
                    Quat q;
                    bodyPose_(_scene, *__link, p, q);
                    world.teleport(e, p, q, true);
                }
            }
            world.resetTime();
        }

        void MMDPhysics::update(Scene &_scene, float _dt) {
            _scene.updateTransforms();
            if (!enabled) {
                for (const Model &m : models_)
                    if (m.skin < _scene.skins.size() && _scene.skins[m.skin].pose.use_physics) animatedPose_(_scene, m.skin);
                return;
            }
            // 1-2. The animated pose; bodies that follow bones get their targets
            for (const Model &m : models_) {
                if (m.skin >= _scene.skins.size()) continue;
                animatedPose_(_scene, m.skin);
                for (const EntityID &e : m.bodies) {
                    const BoneLink *__link = ecs.get<BoneLink>(e);
                    if (!__link || !ecs.hasTag(e, TagID::TAG_PHYSICS_KINEMATIC)) continue;
                    vec3 p;
                    Quat q;
                    bodyPose_(_scene, *__link, p, q);
                    world.setKinematicTarget(e, p, q);
                }
            }
            // 3. Simulation
            world.update(_dt);
            // 4. Dynamic bodies drive their bones
            for (const Model &m : models_) {
                if (m.skin >= _scene.skins.size()) continue;
                Skin &__skin = _scene.skins[m.skin];
                Pose &__pose = __skin.pose;
                const Skeleton &__sk = _scene.skeletons[__skin.skeleton];
                const mat4 &W = _scene.world[__skin.node];
                const mat4 __inv_w = W.inverse<float>();
                const Quat __inv_qw = rotationOf(W).normalize().conjugate();
                __pose.physics_override.assign(__sk.bones.size(), 0u);
                __pose.physics_model.resize(__sk.bones.size());
                for (const EntityID &e : m.bodies) {
                    const BoneLink *__link = ecs.get<BoneLink>(e);
                    const Physics::Body *__body = ecs.get<Physics::Body>(e);
                    if (!__link || !__body || __link->bone == SCENE_NONE || !ecs.hasTag(e, TagID::TAG_PHYSICS_DYNAMIC) ||
                        ecs.hasTag(e, TagID::TAG_PHYSICS_DISABLED)) continue;
                    const mat4 __skel = rigid((__inv_qw * __body->rotation).normalize(), transformPoint(__inv_w, __body->position));
                    mat4 __bone = __link->inverse_offset & __skel;
                    if (__link->mode == SCENE_RIGID_PHYSICS_BONE_POSITION) {
                        const vec3 __animated = rowTranslation(__pose.model[__link->bone]);
                        __bone.M[3][0] = __animated.x; __bone.M[3][1] = __animated.y; __bone.M[3][2] = __animated.z;
                    }
                    __pose.physics_model[__link->bone] = __bone;
                    __pose.physics_override[__link->bone] = 1u;
                }
                __pose.use_physics = true;
                __pose.computeFK(__sk);
            }
        }
    }; // SceneGraph END
}; // Simple END
