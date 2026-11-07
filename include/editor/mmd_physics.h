#ifndef SVKFW_MMD_PHYSICS_H
#define SVKFW_MMD_PHYSICS_H

#include "interface/imgui_wrap.h"
#include "systems/physics.h"
#include "systems/physics_scene.h"


namespace Simple {
    namespace Editor {
        // Note: the settings window of the physics systems (menu "MMD" -> "Physics"): on/off, gravity, substeps, reset,
        //       colliders for models without rigid bodies, statistics. The systems belong to the app and get the models
        //       through the scene events of the MMD studio.
        //
        //       MMDPhysicsWidget mmd_physics(&world, &scene_bodies);
        //       main_window_handler.imgui_handler.widgets.push_back(&mmd_physics);
        struct MMDPhysicsWidget : ImGUI::WidgetItf {
            Physics::World       *world = nullptr;
            Physics::SceneBodies *bodies = nullptr;
            bool                  show_window = false;

            MMDPhysicsWidget(Physics::World *_world, Physics::SceneBodies *_bodies);

        // WidgetItf
            bool begin() override;
            void end() override;
            void render() override;
            void renderFull() override;
        }; // MMDPhysicsWidget END
    }; // Editor END
}; // Simple END

#endif // SVKFW_MMD_PHYSICS_H
