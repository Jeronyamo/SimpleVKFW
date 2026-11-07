#ifndef SVKFW_GAME_APP_H
#define SVKFW_GAME_APP_H

#include "core/engine.h"


namespace Simple {
    namespace Apps {
        // Note: the game (started by EV_TYPE_RUN_GAME); a placeholder for now: it says so and quits
        class GameApp : public Core::AppItf {
        public:
            const char *name() const override { return "game"; }
            bool start(Core::Engine &_engine) override;
            bool frame() override;
            void stop() override;

        private:
            Core::Engine *engine_ = nullptr;
        }; // GameApp END
    }; // Apps END
}; // Simple END

#endif // SVKFW_GAME_APP_H
