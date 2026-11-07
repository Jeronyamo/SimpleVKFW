#include "apps/game_app.h"

#include <cstdio>

#include "common/terminal.h"

namespace Simple {
    namespace Apps {

        bool GameApp::start(Core::Engine &_engine) {
            engine_ = &_engine;
            printf(SVKFW_WRAPINFO("GameApp", "there is no game yet, run the editor (--editor)\n"));
            return true;
        }

        bool GameApp::frame() { return false; }

        void GameApp::stop() { engine_ = nullptr; }
    }; // Apps END
}; // Simple END
