#ifndef SVKFW_SCENE_PHYSICS_H
#define SVKFW_SCENE_PHYSICS_H

#include <cstdint>
#include <vector>

#include "scene/scene_graph.h"
#include "physics/physics_world.h"


namespace Simple {
    namespace SceneGraph {

//  == === ==== ================================== ==== === ==  \\
                    MMD physics
//  == === ==== ================================== ==== === ==  \\

        // Note: simulates the rigid bodies and joints of skeletons ('Skeleton::rigid_bodies', 'joints', read from PMX) with
        //       'Physics::World' in the world space, so models collide with each other and with the bodies the game adds to
        //       'world'. Per frame ('update'):
        //         1. the animated pose (forward kinematics without physics);
        //         2. bodies that follow bones (SCENE_RIGID_FOLLOW_BONE) get their bone pose as kinematic targets;
        //         3. the world steps (fixed 60 Hz steps, see 'Physics::WorldSettings');
        //         4. dynamic bodies override their bones ('Pose::physics_model'; SCENE_RIGID_PHYSICS_BONE_POSITION keeps the
        //            animated bone position), forward kinematics again: descendants, IK and grants follow the physics.
        //       Models with a uniform scale are supported (the shapes are scaled); gravity is the world's setting
        //       (MMD: 98 units/s^2 for scale 1, 'addModel' scales it by the first model's scale).
        //
        //       MMDPhysics physics;
        //       uint32_t root = loadPMX(scene, "chara.pmx");
        //       physics.addModel(scene, scene.nodes[root].skin);
        //       physics.reset(scene);                 // after loading, seeking or moving a model
        //       ...
        //       applyAnimation(clip, binding, skeleton, frame, scene.pose(skin));
        //       physics.update(scene, dt);            // instead of 'scene.updateSkins()' for these skins

        // Note: the link of a body entity to its bone (a component in 'MMDPhysics::ecs')
        struct BoneLink {
            uint32_t  skin = SCENE_NONE;
            uint32_t  bone = SCENE_NONE;      // SCENE_NONE: the skeleton space itself
            RigidMode mode = SCENE_RIGID_FOLLOW_BONE;
            mat4      offset = Mat::Eye4;     // body = offset & bone model (skeleton space, unscaled)
            mat4      inverse_offset = Mat::Eye4;
        }; // BoneLink END

        class MMDPhysics {
        public:
            ComponentStorage ecs;
            Physics::World   world;
            bool             enabled = true;   // false: plain animation (bones are not overridden)
            float            mmd_gravity = 98.f; // units/s^2 at scale 1

            MMDPhysics();

            // Creates the bodies and joints of the skin's skeleton (nothing if it has none). Call 'reset' afterwards
            void addModel(Scene &_scene, uint32_t _skin);
            // Removes the bodies of all models (bodies the game added directly to 'world' stay)
            void clear();
            // Puts all bodies of the models where their bones are in the current (animated) pose, with zero velocity
            void reset(Scene &_scene);
            // Steps the physics by '_dt' seconds (0: no step, the bones still follow the bodies) and poses the skins
            void update(Scene &_scene, float _dt);

            uint32_t modelCount() const { return (uint32_t)models_.size(); }

        private:
            struct Model {
                uint32_t              skin = SCENE_NONE;
                float                 scale = 1.f;
                std::vector<EntityID> bodies, joints;
            };
            std::vector<Model> models_;

            // World pose of a body from the current pose of its bone
            void bodyPose_(const Scene &_scene, const BoneLink &_link, vec3 &_position, Quat &_rotation) const;
            void animatedPose_(Scene &_scene, uint32_t _skin);
        }; // MMDPhysics END
    }; // SceneGraph END
}; // Simple END

#endif // SVKFW_SCENE_PHYSICS_H
