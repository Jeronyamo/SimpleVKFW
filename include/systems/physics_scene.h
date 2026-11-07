#ifndef SVKFW_PHYSICS_SCENE_H
#define SVKFW_PHYSICS_SCENE_H

#include <cstdint>
#include <vector>

#include "scene/scene_graph.h"
#include "systems/physics.h"
#include "core/system.h"
#include "core/event_handler.h"


namespace Simple {
    namespace Physics {
        namespace SG = SceneGraph;

//  == === ==== ================================== ==== === ==  \\
                    Scene bodies
//  == === ==== ================================== ==== === ==  \\

        // Note: links the bodies of 'Physics::World' to the scene graph: a body follows (kinematic) or drives (dynamic) a bone
        //       of a skin or a node. Everything is in the world's component storage (the engine's 'ecs'):
        //         - a body entity has 'Body', 'Collider' and a 'SceneLink';
        //         - a model entity has a 'PhysicsModel' with the bodies and joints that are removed with the model.
        //       Sources of bodies:
        //         - skeletons with rigid bodies and joints (PMX: 'Skeleton::rigid_bodies', 'joints'), on EV_TYPE_SCENE_MODEL_ADDED
        //           or 'addModel';
        //         - colliders fitted to the mesh bounds of the nodes of a model ('addColliders'; 'auto_colliders' does it for
        //           added models without rigid bodies, e.g. assimp models);
        //         - single bodies on nodes ('addBody').
        //       Per frame:
        //         PHASE_ANIMATION  the animated pose (forward kinematics without physics); bodies that follow bones or nodes
        //                          get their kinematic targets (after a reset: all bodies are put there);
        //         PHASE_PHYSICS    'World' steps;
        //         PHASE_SCENE      dynamic bodies override their bones ('Pose::physics_model'; SCENE_RIGID_PHYSICS_BONE_POSITION
        //                          keeps the animated bone position), forward kinematics again: descendants, IK and grants
        //                          follow the physics. Dynamic node bodies set the local transform of their node.
        //       Models with a uniform scale are supported (the shapes are scaled); gravity is the world's setting
        //       (MMD: 98 units/s^2 for scale 1, the first model sets it by its scale).
        //
        //       Physics::World       world(engine.ecs);
        //       Physics::SceneBodies bodies(scene, world);
        //       engine.addSystem(&world);
        //       engine.addSystem(&bodies);        // listens to the scene events of 'core/event_types.h'
        //       ...
        //       engine.runPhase(Core::PHASE_ANIMATION, time); // after the animations are applied
        //       engine.runPhase(Core::PHASE_PHYSICS, time);
        //       engine.runPhase(Core::PHASE_SCENE, time);     // instead of 'scene.updateSkins()' for these skins
        //
        //       Without the engine: 'addModel', 'reset', then 'simulate(dt)' each frame.

        enum LinkTarget : uint32_t {
            LINK_BONE = 0,  // 'SceneLink::skin', 'bone'
            LINK_NODE       // 'SceneLink::node'
        };

        // Note: the link of a body entity to the scene
        struct SceneLink {
            LinkTarget    target = LINK_BONE;
            uint32_t      skin = SG::SCENE_NONE;
            uint32_t      bone = SG::SCENE_NONE;  // SCENE_NONE: the skeleton space itself
            uint32_t      node = SG::SCENE_NONE;
            // Bones: SCENE_RIGID_FOLLOW_BONE (kinematic) or a dynamic mode; nodes: the motion tag of the body decides
            SG::RigidMode mode = SG::SCENE_RIGID_FOLLOW_BONE;
            // Bones: body = offset & bone model (skeleton space, unscaled); nodes: the body center in the node space
            mat4          offset = Mat::Eye4;
            mat4          inverse_offset = Mat::Eye4;
            float         scale = 1.f;            // nodes: the uniform world scale, kept when a dynamic body moves the node
        }; // SceneLink END

        // Note: a model entity: what is removed with the model
        struct PhysicsModel {
            uint32_t              root = SG::SCENE_NONE;
            uint32_t              skin = SG::SCENE_NONE;
            float                 scale = 1.f;
            std::vector<EntityID> bodies, joints;
        }; // PhysicsModel END

        enum ColliderFit : uint32_t {
            FIT_BOX = 0,
            FIT_SPHERE,   // the sphere around the box
            FIT_CAPSULE   // along Y
        };

        class SceneBodies : public Core::System {
        public:
            SG::Scene        &scene;
            World            &world;
            ComponentStorage &ecs;
            bool              enabled = true;     // false: plain animation (bones and nodes are not overridden)
            float             mmd_gravity = 98.f; // units/s^2 at scale 1
            // Colliders for added models without rigid bodies (e.g. assimp models), one per node with meshes
            bool              auto_colliders = false;
            ColliderFit       auto_fit = FIT_BOX;
            TagID             auto_motion = TagID::TAG_PHYSICS_STATIC;

            SceneBodies(SG::Scene &_scene, World &_world);
            ~SceneBodies() override;

        // System
            const char *name() const override { return "scene_bodies"; }
            uint32_t phases() const override { return Core::phaseBit(Core::PHASE_ANIMATION) | Core::phaseBit(Core::PHASE_SCENE); }
            // Subscribes to EV_TYPE_SCENE_MODEL_ADDED, CLEARED, RESET
            void attach(Core::Engine &_engine) override;
            void detach(Core::Engine &_engine) override;
            void update(Core::Phase _phase, const Core::FrameTime &_time) override;

        // Models

            // The bodies and joints of the skin's skeleton (or fitted colliders with 'auto_colliders'); returns the model
            // entity (none if nothing was created). The bodies are placed at the next update ('requestReset')
            EntityID addModel(uint32_t _root, uint32_t _skin);
            // Colliders fitted to the mesh bounds of '_root' and its descendants (one body per node with meshes); added to
            // '_model' if it is a model entity (removed with it). Returns the number of bodies
            uint32_t addColliders(uint32_t _root, ColliderFit _fit, TagID _motion, EntityID _model = EntityID::None);
            // One body on a node, '_center' in the node space; added to '_model' if it is a model entity, otherwise it stays
            // until the game destroys it (keep the node index valid)
            EntityID addBody(uint32_t _node, const Collider &_collider, TagID _motion, const vec3 &_center = {0.f, 0.f, 0.f},
                             EntityID _model = EntityID::None);
            // A collider of the box (node space) scaled by '_scale'
            static Collider fitCollider(const SG::AABB &_box, ColliderFit _fit, float _scale = 1.f);

            // Removes all models (bodies added without a model stay)
            void clear();
            // Puts all linked bodies where their bones and nodes are in the current (animated) pose, with zero velocity
            void reset();
            // 'reset' at the next update
            void requestReset() { reset_pending_ = true; }
            // Without the engine: the three phases ('_dt' seconds of simulation, 0: no step, bones still follow the bodies)
            void simulate(float _dt);

            uint32_t modelCount() const { return (uint32_t)models_.size(); }

        private:
            std::vector<EntityID> models_;
            std::vector<EntityID> node_bodies_; // all bodies linked to nodes (checked with 'isValid')
            bool                  reset_pending_ = false;
            Event::EventHandler  *events_ = nullptr;
            std::vector<Event::SubscriptionID> subscriptions_;

            void animate_();
            void writeBack_();
            // World pose of a body from the current pose of its bone or node
            bool bodyPose_(const SceneLink &_link, vec3 &_position, Quat &_rotation) const;
            void animatedPose_(uint32_t _skin);
            void skeletonBodies_(PhysicsModel &_model);
        }; // SceneBodies END
    }; // Physics END
}; // Simple END

#endif // SVKFW_PHYSICS_SCENE_H
