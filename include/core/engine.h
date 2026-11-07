#ifndef SVKFW_ENGINE_H
#define SVKFW_ENGINE_H

#include <cstdint>
#include <vector>
#include <functional>

#include "core/etcs_base.h"
#include "core/event_handler.h"
#include "core/system.h"


namespace Simple {
    namespace Core {
//  == === ==== ================================== ==== === ==  \\
                    Engine
//  == === ==== ================================== ==== === ==  \\

        // Note: an app is what the engine runs: the editor, the game ('apps/'). It owns its window and renderer and adds its
        //       systems to the engine in 'start'; 'frame' is one frame of it (input, phases, rendering)
        struct AppItf {
            virtual ~AppItf() {}
            virtual const char *name() const = 0;
            // False: the app couldn't start ('stop' isn't called)
            virtual bool start(Engine &_engine) = 0;
            // False: the app wants to quit (e.g. its window was closed)
            virtual bool frame() = 0;
            virtual void stop() = 0;
        }; // AppItf END

        typedef std::function<AppItf*()> AppFactory;

        // Note: the top level: the event handler, the global component storage and the systems. One loop for events and
        //       frames, on one thread: each iteration polls the events (app start/quit requests, queued system events),
        //       then runs one frame of the app. An app is started by its event (EV_TYPE_RUN_EDITOR, EV_TYPE_RUN_GAME), which
        //       is ignored while an app is running; EV_TYPE_QUIT (or 'quit') ends the loop.
        //
        //       Core::Engine engine;
        //       engine.registerApp(Event::EV_TYPE_RUN_EDITOR, []() -> Core::AppItf* { return new Apps::EditorApp(); });
        //       engine.events.write(Event::EV_TYPE_RUN_EDITOR);
        //       return engine.run();
        //
        //       The factories keep the engine independent of the apps (the apps depend on the engine, not the other way)
        class Engine {
        public:
            Event::EventHandler events;
            ComponentStorage    ecs;
            uint64_t            max_frames = 0u; // > 0: quits after that many frames (tests, benchmarks)

            Engine();
            ~Engine();
            Engine(const Engine&) = delete;
            Engine &operator=(const Engine&) = delete;

        // Systems (not owned)

            // Calls 'attach'; systems run in the order they were added
            void addSystem(System *_system);
            // Calls 'detach'; returns false if it wasn't added
            bool removeSystem(System *_system);
            const std::vector<System*> &systems() const { return systems_; }
            // Calls 'update' of the systems with the phase (and measures their time)
            void runPhase(Phase _phase, const FrameTime &_time);

        // Apps

            void registerApp(uint32_t _event_type, const AppFactory &_factory);
            AppItf *app() const { return app_; }
            uint64_t frame() const { return frame_; }

            // Runs until there is no app (none was requested, or it quit); returns the exit code (0: the app ran and
            // stopped normally)
            int  run();
            void quit() { quit_ = true; }

        private:
            struct Registered {
                uint32_t   type;
                AppFactory factory;
            };
            std::vector<System*>    systems_;
            std::vector<double>     frame_ms_; // per system: the time of the current frame, moved to 'update_ms' after it
            std::vector<Registered> apps_;
            AppItf  *app_ = nullptr;
            AppItf  *pending_ = nullptr;
            uint64_t frame_ = 0u;
            bool     quit_ = false;
            int      exit_code_ = 0;

            void stopApp_();
        }; // Engine END
    }; // Core END
}; // Simple END

#endif // SVKFW_ENGINE_H
