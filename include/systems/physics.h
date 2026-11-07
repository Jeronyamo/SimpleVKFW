#ifndef SVKFW_PHYSICS_WORLD_H
#define SVKFW_PHYSICS_WORLD_H

#include <cstdint>
#include <vector>
#include <set>
#include <utility>

#include "math/vectors.h"
#include "math/linalg.h"
#include "core/etcs_base.h"
#include "core/event_handler.h"
#include "core/system.h"


namespace Simple {
    namespace Physics {

//  == === ==== ================================== ==== === ==  \\
                    Rigid body physics (XPBD)
//  == === ==== ================================== ==== === ==  \\

        // Note: a small rigid body solver on top of the ETCS: a body is an entity with a 'Body' and a 'Collider' component
        //       and one motion tag (TAG_PHYSICS_STATIC / KINEMATIC / DYNAMIC); a joint is an entity with a 'Joint' component.
        //       The solver is XPBD (extended position based dynamics, Mueller et al. 2020, "Detailed rigid body simulation
        //       with extended position based dynamics"): every fixed step is split into substeps; a substep integrates
        //       gravity and velocities, corrects the positions (joints, contacts with static friction), derives the velocities
        //       from the moved positions and applies restitution and dynamic friction. Substeps make long joint chains
        //       (hair, skirts) stiff without many iterations, and joints with springs are just soft constraints.
        //
        //       Units are free (MMD: ~8 cm per unit, gravity 98 units/s^2); the world is left-handed with Y up like the engine,
        //       the solver doesn't depend on the handedness.
        //
        //       ComponentStorage ecs;
        //       Physics::World world(ecs);
        //       world.createBody(Physics::Body::at({0.f, 5.f, 0.f}), Physics::Collider::sphere(0.5f), TagID::TAG_PHYSICS_DYNAMIC);
        //       world.createBody(Physics::Body(), Physics::Collider::plane(), TagID::TAG_PHYSICS_STATIC);  // the ground
        //       ...
        //       world.update(frame_dt); // fixed steps (settings.fixed_step) with an accumulator
        //
        //       As a system of the engine ('core/system.h') it uses the engine's components and events and steps in
        //       PHASE_PHYSICS by 'FrameTime::sim_dt':
        //
        //       Physics::World world(engine.ecs);
        //       engine.addSystem(&world);

        typedef Affine::Rotation::Quaternion Quat;

        enum ShapeType : uint32_t {
            SHAPE_SPHERE = 0, // size.x: radius
            SHAPE_BOX,        // size: half extents
            SHAPE_CAPSULE,    // along the local Y: size.x radius, size.y length of the cylinder part (Bullet style)
            SHAPE_PLANE       // the local XZ plane (normal +Y), infinite; static bodies only
        };

        struct Collider {
            ShapeType shape = SHAPE_SPHERE;
            vec3      size = {0.5f, 0.5f, 0.5f};
            float     friction = 0.5f;    // static and dynamic friction coefficient
            float     restitution = 0.f;
            uint32_t  group = 1u;         // bits: the groups of the body
            uint32_t  mask  = 0xFFFFFFFFu;// bits: the groups it collides with (both bodies must accept each other)

            static Collider sphere (float _radius);
            static Collider box    (const vec3 &_half_extents);
            static Collider capsule(float _radius, float _length);
            static Collider plane  ();
        }; // Collider END

        struct Body {
            vec3  position = {0.f, 0.f, 0.f};  // the center of mass, world space
            Quat  rotation;
            vec3  velocity = {0.f, 0.f, 0.f};
            vec3  angular_velocity = {0.f, 0.f, 0.f}; // world space, radians/s
            float mass = 1.f;                 // dynamic bodies; 'World::createBody' computes the inertia from the collider
            vec3  inertia = {1.f, 1.f, 1.f};  // local principal moments (diagonal); 0 on an axis: no rotation around it
            float linear_damping = 0.f, angular_damping = 0.f; // fraction of the velocity lost per second (Bullet style)
            float gravity_scale = 1.f;
            vec3  force  = {0.f, 0.f, 0.f};   // world space, applied during the next step, then cleared
            vec3  torque = {0.f, 0.f, 0.f};

            // Kinematic bodies: the pose to reach by the end of the next 'World::update' (moved smoothly over its steps)
            vec3  target_position = {0.f, 0.f, 0.f};
            Quat  target_rotation;
            bool  has_target = false;

            // Solver state
            float inv_mass = 0.f;
            vec3  inv_inertia = {0.f, 0.f, 0.f};
            vec3  prev_position = {0.f, 0.f, 0.f}, frame_position = {0.f, 0.f, 0.f};
            Quat  prev_rotation, frame_rotation;

            static Body at(const vec3 &_position, const Quat &_rotation = Quat(), float _mass = 1.f);
        }; // Body END

        // Note: a 6DOF joint (like Bullet's 'btGeneric6DofSpringConstraint', which MMD uses). The joint frame is fixed in
        //       both bodies ('anchor_*', 'frame_*' in the body spaces); the limits are for body B's frame in body A's frame:
        //       translation per axis and rotation as Euler XYZ angles (radians, q = qX * qY * qZ,
        //       'Affine::Rotation::eulerXYZ'). Per axis: min == max locks it,
        //       min < max limits it, min > max frees it. Springs (stiffness per axis, 0: none) pull towards zero offset / angle
        struct Joint {
            EntityID body_a, body_b;
            vec3  anchor_a = {0.f, 0.f, 0.f}, anchor_b = {0.f, 0.f, 0.f};
            Quat  frame_a, frame_b;
            vec3  linear_min  = {0.f, 0.f, 0.f}, linear_max  = {0.f, 0.f, 0.f};
            vec3  angular_min = {0.f, 0.f, 0.f}, angular_max = {0.f, 0.f, 0.f};
            vec3  linear_spring  = {0.f, 0.f, 0.f};
            vec3  angular_spring = {0.f, 0.f, 0.f};
            bool  collide_connected = false; // contacts between the two bodies
        }; // Joint END

        // Payload of EV_TYPE_PHYSICS_CONTACT_BEGIN / END ('core/event_handler.h'); 'point' and 'normal' (from B to A) are
        // zero for END events
        struct ContactEvent {
            EntityID body_a, body_b;
            float    point[3], normal[3]; // plain arrays: the payload must be trivially copyable
            float    depth;
        }; // ContactEvent END

        struct WorldSettings {
            vec3     gravity = {0.f, -98.f, 0.f}; // MMD: 9.8 m/s^2 with ~10 units per meter
            float    fixed_step = 1.f / 60.f;
            uint32_t substeps = 8u;
            uint32_t max_steps = 4u;        // per 'update' (the rest of the time is dropped: no spiral of death)
            float    contact_margin = 0.02f;// bodies closer than this are tested every substep
            float    max_velocity = 1000.f; // clamp (units/s) against explosions
            float    max_angular_velocity = 100.f; // radians/s
        }; // WorldSettings END

        struct WorldStats {
            uint32_t steps = 0u;         // by the last 'update'
            uint32_t bodies = 0u, dynamic_bodies = 0u, joints = 0u;
            uint32_t pairs = 0u;         // broad phase pairs of the last step
            uint32_t contacts = 0u;      // contacts of the last substep
        }; // WorldStats END

        // A contact point of two shapes (see 'collideShapes'); the normal points from B to A
        struct ShapeContact {
            vec3  normal;
            vec3  point_a, point_b; // world points on the surfaces
            float depth;
        }; // ShapeContact END

        class World : public Core::System {
        public:
            ComponentStorage    &ecs;
            Event::EventHandler *events = nullptr; // optional: contact events go here
            WorldSettings        settings;
            WorldStats           stats;

            explicit World(ComponentStorage &_ecs);

        // System: steps by 'sim_dt' in PHASE_PHYSICS; contact events go to the engine's handler
            const char *name() const override { return "physics"; }
            uint32_t phases() const override { return Core::phaseBit(Core::PHASE_PHYSICS); }
            void attach(Core::Engine &_engine) override;
            void detach(Core::Engine &_engine) override;
            void update(Core::Phase _phase, const Core::FrameTime &_time) override;

            // Creates a body entity; dynamic bodies get 'inv_mass' and the inertia from the collider (mass <= 0: 1)
            EntityID createBody(const Body &_body, const Collider &_collider, TagID _motion);
            // The joint entity; both bodies must exist
            EntityID createJoint(const Joint &_joint);
            // Recomputes the inertia of a dynamic body from its collider and mass (after changing them)
            void     updateMass(EntityID _body);
            void     setMotion(EntityID _body, TagID _motion);

            // Moves a body without velocity (kinematic targets are dropped)
            void teleport(EntityID _body, const vec3 &_position, const Quat &_rotation, bool _reset_velocity = true);
            void setKinematicTarget(EntityID _body, const vec3 &_position, const Quat &_rotation);

            // Advances the time by '_dt': runs the due fixed steps; returns their number
            uint32_t update(float _dt);
            // One fixed step of '_h' seconds (kinematic bodies reach their targets)
            void     step(float _h);
            // Drops the time accumulated by 'update'
            void     resetTime() { accumulator_ = 0.f; }

            // Joints and bodies of these entities are removed with the entity ('ecs.destroy'); call this after destroying
            // bodies to drop the joints that referred to them
            void     removeBrokenJoints();

            // The solver's dense copy of a body state (internal)
            struct SolverState {
                vec3  x, px, v, w;   // position, previous position, velocities
                Quat  q, pq;         // rotation, previous rotation
                float im;            // inverse mass (0: not dynamic)
                vec3  ii;            // local inverse inertia
                float iw[9];         // world inverse inertia (row-major), updated once per substep
                vec3  fx, tx;        // kinematic: the pose at the start of the update and the target
                Quat  fq, tq;
                bool  has_target;
            };

        private:
            struct SolverBody {
                EntityID  entity;
                Body     *body;
                Collider *collider;
                bool      dynamic;
                bool      kinematic;
                bool      report;
                float     radius;  // bounding sphere (planes: 0, they are tested against everything)
            };
            struct SolverJoint {
                Joint   *joint;
                uint32_t a, b;
            };
            struct Contact {
                uint32_t a, b;      // solver bodies; the normal points from b to a
                vec3     normal;
                vec3     ra, rb;    // contact points relative to the centers (world)
                float    depth;
                float    lambda_n;  // normal impulse of the position step
                float    vn;        // relative normal velocity before the velocity update
                float    friction, restitution;
            };

            float accumulator_ = 0.f;
            std::pair<uint32_t, uint32_t> stepFraction_ = std::make_pair(0u, 1u); // step index, steps of the current 'update'
            std::vector<SolverBody>  bodies_;
            std::vector<SolverState> states_;
            std::vector<ShapeContact> found_;
            std::vector<float>       damping_;
            std::vector<SolverJoint> joints_;
            std::vector<uint32_t>    slot_;          // entity index -> solver body
            std::vector<std::pair<uint32_t, uint32_t>> pairs_;
            std::set<std::pair<uint64_t, uint64_t>>    jointed_;
            std::vector<Contact>     contacts_;
            std::set<std::pair<uint64_t, uint64_t>>    touching_, touching_prev_;

            void gather_();
            void broadPhase_(float _h);
            void collide_();
            void solveJoint_(const SolverJoint &_joint, float _h);
            void solveContacts_(float _h);
            void solveVelocities_(float _h);
            void reportContacts_();
        }; // World END

    // Shape helpers (used by the world, exposed for tests)

        // Fills '_out' with the contacts of two colliders at the given poses (the normal points from B to A)
        void collideShapes(const Collider &_ca, const vec3 &_pa, const Quat &_qa,
                           const Collider &_cb, const vec3 &_pb, const Quat &_qb, std::vector<ShapeContact> &_out);
    }; // Physics END
}; // Simple END

#endif // SVKFW_PHYSICS_WORLD_H
