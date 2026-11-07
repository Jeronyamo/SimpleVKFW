#ifndef SVKFW_SYSTEM_H
#define SVKFW_SYSTEM_H

#include <cstdint>


namespace Simple {
    namespace Core {
        class Engine;

//  == === ==== ================================== ==== === ==  \\
                    Systems
//  == === ==== ================================== ==== === ==  \\

        // Note: the parts of a frame, in order. The app runs them ('Engine::runPhase') at its points of the frame, so the
        //       order is fixed while the app keeps its own render code:
        //         INPUT      input layers, camera
        //         ANIMATION  animations applied; simulations read the animated pose (kinematic targets)
        //         PHYSICS    the physics world steps
        //         SCENE      simulations write the scene back (bones, nodes), transforms
        //         PREPARE    culling, renderer data of the frame
        //         RENDER     recording and submitting
        enum Phase : uint32_t {
            PHASE_INPUT = 0,
            PHASE_ANIMATION,
            PHASE_PHYSICS,
            PHASE_SCENE,
            PHASE_PREPARE,
            PHASE_RENDER,
            PHASE_COUNT
        }; // Phase END

        inline uint32_t phaseBit(Phase _phase) { return 1u << (uint32_t)_phase; }

        struct FrameTime {
            float    dt = 0.f;      // real seconds since the last frame
            float    sim_dt = 0.f;  // seconds of simulated time (0: paused, e.g. by the editor's playback)
            uint64_t frame = 0u;    // frames since the start of the engine loop
        }; // FrameTime END

        // Note: a system works on the engine's components ('Engine::ecs') and talks to the rest through events
        //       ('Engine::events'). It subscribes in 'attach' and unsubscribes in 'detach'; 'update' is called for each phase
        //       of 'phases()'. The engine measures 'update_ms' (CPU time of the last frame, all phases)
        struct System {
            float update_ms = 0.f;

            virtual ~System() {}
            virtual const char *name() const = 0;
            // 'phaseBit' of the phases 'update' is called for
            virtual uint32_t phases() const = 0;
            virtual void attach(Engine &_engine) { (void)_engine; }
            virtual void detach(Engine &_engine) { (void)_engine; }
            virtual void update(Phase _phase, const FrameTime &_time) = 0;
        }; // System END
    }; // Core END
}; // Simple END

#endif // SVKFW_SYSTEM_H
