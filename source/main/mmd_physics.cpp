#include "main/mmd_physics.h"

#include <chrono>

#include "imgui.h"

namespace Simple {
    namespace Editor {

        MMDPhysicsWidget::MMDPhysicsWidget() {}

        void MMDPhysicsWidget::addModel(SG::Scene &_scene, uint32_t _skin) { physics.addModel(_scene, _skin); }

        void MMDPhysicsWidget::reset(SG::Scene &_scene) { physics.reset(_scene); }

        void MMDPhysicsWidget::clear() { physics.clear(); }

        void MMDPhysicsWidget::update(SG::Scene &_scene, float _dt) {
            const auto __start = std::chrono::steady_clock::now();
            if (reset_requested) { physics.reset(_scene); reset_requested = false; }
            physics.update(_scene, _dt);
            last_update_ms = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - __start).count();
        }

        bool MMDPhysicsWidget::begin() { return true; }

        void MMDPhysicsWidget::end() {}

        void MMDPhysicsWidget::renderFull() {
            if (begin()) render();
            end();
        }

        void MMDPhysicsWidget::render() {
            if (ImGui::BeginMainMenuBar()) {
                // Appended to the "MMD" menu of the MMD studio
                if (ImGui::BeginMenu("MMD")) {
                    ImGui::Separator();
                    ImGui::MenuItem("Physics", nullptr, &show_window);
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
            if (!show_window) return;
            ImGui::SetNextWindowSize(ImVec2(320.f, 0.f), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Physics##mmd", &show_window)) {
                Physics::WorldSettings &s = physics.world.settings;
                ImGui::Checkbox("Enabled", &physics.enabled);
                ImGui::SameLine();
                if (ImGui::Button("Reset")) reset_requested = true;
                float __gravity = -s.gravity.y;
                if (ImGui::DragFloat("Gravity", &__gravity, 0.5f, 0.f, 1000.f, "%.1f units/s^2")) s.gravity.y = -__gravity;
                int __substeps = (int)s.substeps;
                if (ImGui::SliderInt("Substeps", &__substeps, 1, 32)) s.substeps = (uint32_t)__substeps;
                float __hz = 1.f / s.fixed_step;
                if (ImGui::SliderFloat("Step rate", &__hz, 30.f, 240.f, "%.0f Hz")) s.fixed_step = 1.f / __hz;
                ImGui::Separator();
                const Physics::WorldStats &st = physics.world.stats;
                ImGui::Text("Models: %u", physics.modelCount());
                ImGui::Text("Bodies: %u (%u dynamic), joints: %u", st.bodies, st.dynamic_bodies, st.joints);
                ImGui::Text("Pairs: %u, contacts: %u", st.pairs, st.contacts);
                ImGui::Text("Update: %.2f ms (%u steps)", last_update_ms, st.steps);
            }
            ImGui::End();
        }
    }; // Editor END
}; // Simple END
