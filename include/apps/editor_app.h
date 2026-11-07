#ifndef SVKFW_EDITOR_APP_H
#define SVKFW_EDITOR_APP_H

#include "core/engine.h"


namespace Simple {
    namespace Apps {
        // Note: the editor: a window with the scene, the MMD studio (models, motions, timeline, physics), the music studio and
        //       the camera controls. It adds the physics systems ('Physics::World', 'Physics::SceneBodies') to the engine and
        //       re-uploads the scene renderer on EV_TYPE_SCENE_CHANGED.
        //       Started by EV_TYPE_RUN_EDITOR (see 'main.cpp'); quits when its window is closed
        class EditorApp : public Core::AppItf {
        public:
            EditorApp();
            ~EditorApp() override;

            const char *name() const override { return "editor"; }
            bool start(Core::Engine &_engine) override;
            bool frame() override;
            void stop() override;

            // The objects of the running editor (window, Vulkan, scene, widgets), see 'source/apps/editor_app.cpp'
            struct State;

        private:
            State *state_ = nullptr;
        }; // EditorApp END
    }; // Apps END
}; // Simple END

#endif // SVKFW_EDITOR_APP_H
