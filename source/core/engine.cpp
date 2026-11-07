#include "core/engine.h"

#include <algorithm>
#include <chrono>
#include <cstdio>

#include "common/terminal.h"

namespace Simple {
    namespace Core {

        Engine::Engine() {}

        Engine::~Engine() {
            stopApp_();
            delete pending_;
            for (size_t i = systems_.size(); i-- > 0u;) systems_[i]->detach(*this);
        }

        void Engine::addSystem(System *_system) {
            if (!_system || std::find(systems_.begin(), systems_.end(), _system) != systems_.end()) return;
            systems_.push_back(_system);
            frame_ms_.push_back(0.);
            _system->attach(*this);
        }

        bool Engine::removeSystem(System *_system) {
            const std::vector<System*>::iterator __it = std::find(systems_.begin(), systems_.end(), _system);
            if (__it == systems_.end()) return false;
            frame_ms_.erase(frame_ms_.begin() + (__it - systems_.begin()));
            systems_.erase(__it);
            _system->detach(*this);
            return true;
        }

        void Engine::runPhase(Phase _phase, const FrameTime &_time) {
            const uint32_t __bit = phaseBit(_phase);
            for (size_t i = 0u; i < systems_.size(); ++i) {
                if ((systems_[i]->phases() & __bit) == 0u) continue;
                const std::chrono::steady_clock::time_point __start = std::chrono::steady_clock::now();
                systems_[i]->update(_phase, _time);
                frame_ms_[i] += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - __start).count();
            }
        }

        void Engine::registerApp(uint32_t _event_type, const AppFactory &_factory) {
            apps_.push_back(Registered{_event_type, _factory});
            const AppFactory __factory = _factory;
            events.subscribe(_event_type, [this, __factory](Event::EventHandler &, const Event::Event &_event) {
                if (app_ || pending_) {
                    printf(SVKFW_WRAPINFO("Engine", "an app is running, event %u ignored\n"), _event.type);
                    return;
                }
                pending_ = __factory();
            });
        }

        void Engine::stopApp_() {
            if (!app_) return;
            app_->stop();
            delete app_;
            app_ = nullptr;
        }

        int Engine::run() {
            const Event::SubscriptionID __quit = events.subscribe(Event::EV_TYPE_QUIT, [this](Event::EventHandler &, const Event::Event &) {
                quit_ = true;
            });
            quit_ = false;
            exit_code_ = 0;
            frame_ = 0u;
            bool __ran = false;
            while (true) {
                events.poll();
                if (pending_) {
                    AppItf *__app = pending_;
                    pending_ = nullptr;
                    printf(SVKFW_WRAPINFO("Engine", "starting %s\n"), __app->name());
                    if (__app->start(*this)) { app_ = __app; __ran = true; }
                    else {
                        printf(SVKFW_WRAPERR("Engine", "%s couldn't start\n"), __app->name());
                        delete __app;
                        exit_code_ = 1;
                        quit_ = true;
                    }
                }
                if (quit_ || !app_) break;

                if (!app_->frame()) quit_ = true;
                ++frame_;
                for (size_t i = 0u; i < systems_.size(); ++i) {
                    systems_[i]->update_ms = (float)frame_ms_[i];
                    frame_ms_[i] = 0.;
                }
                if (max_frames > 0u && frame_ >= max_frames) quit_ = true;
            }
            stopApp_();
            events.unsubscribe(__quit);
            if (!__ran && exit_code_ == 0) {
                printf(SVKFW_WRAPINFO("Engine", "no app was started\n"));
                exit_code_ = 1;
            }
            return exit_code_;
        }
    }; // Core END
}; // Simple END
