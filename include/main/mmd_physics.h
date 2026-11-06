#ifndef SVKFW_MMD_PHYSICS_H
#define SVKFW_MMD_PHYSICS_H

#include "main/mmd_editor.h"
#include "scene/scene_physics.h"


namespace Simple {
    namespace Editor {
        // Note: the MMD physics ('scene/scene_physics.h') as the simulation of the MMD studio, with a settings window
        //       (menu "MMD" -> "Physics"): on/off, gravity, substeps, reset, statistics.
        //
        //       MMDPhysicsWidget mmd_physics;
        //       mmd_studio.simulation = &mmd_physics;
        //       main_window_handler.imgui_handler.widgets.push_back(&mmd_physics);
        struct MMDPhysicsWidget : ImGUI::WidgetItf, MMDSimulationItf {
            SG::MMDPhysics physics;
            bool           show_window = false;
            bool           reset_requested = false;
            float          last_update_ms = 0.f; // CPU time of the last 'update'

            MMDPhysicsWidget();

        // MMDSimulationItf
            void addModel(SG::Scene &_scene, uint32_t _skin) override;
            void reset(SG::Scene &_scene) override;
            void clear() override;
            void update(SG::Scene &_scene, float _dt) override;

        // WidgetItf
            bool begin() override;
            void end() override;
            void render() override;
            void renderFull() override;
        }; // MMDPhysicsWidget END
    }; // Editor END
}; // Simple END

#endif // SVKFW_MMD_PHYSICS_H
