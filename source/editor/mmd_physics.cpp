#include "editor/mmd_physics.h"

#include "imgui.h"

namespace Simple {
    namespace Editor {

        MMDPhysicsWidget::MMDPhysicsWidget(Physics::World *_world, Physics::SceneBodies *_bodies) : world(_world), bodies(_bodies) {}

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
            if (!show_window || !world || !bodies) return;
            ImGui::SetNextWindowSize(ImVec2(340.f, 0.f), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Physics##mmd", &show_window)) {
                Physics::WorldSettings &s = world->settings;
                ImGui::Checkbox("Enabled", &bodies->enabled);
                ImGui::SameLine();
                if (ImGui::Button("Reset")) bodies->requestReset();
                float __gravity = -s.gravity.y;
                if (ImGui::DragFloat("Gravity", &__gravity, 0.5f, 0.f, 1000.f, "%.1f units/s^2")) s.gravity.y = -__gravity;
                int __substeps = (int)s.substeps;
                if (ImGui::SliderInt("Substeps", &__substeps, 1, 32)) s.substeps = (uint32_t)__substeps;
                float __hz = 1.f / s.fixed_step;
                if (ImGui::SliderFloat("Step rate", &__hz, 30.f, 240.f, "%.0f Hz")) s.fixed_step = 1.f / __hz;

                // Note: applied to the models loaded afterwards (models without rigid bodies, e.g. OBJ, glTF, FBX)
                ImGui::SeparatorText("Models without rigid bodies");
                ImGui::Checkbox("Colliders from mesh bounds", &bodies->auto_colliders);
                if (bodies->auto_colliders) {
                    const char *__fits[] = { "Box", "Sphere", "Capsule" };
                    int __fit = (int)bodies->auto_fit;
                    if (ImGui::Combo("Shape", &__fit, __fits, 3)) bodies->auto_fit = (Physics::ColliderFit)__fit;
                    const char *__motions[] = { "Static", "Kinematic (follows the node)", "Dynamic (moves the node)" };
                    int __motion = bodies->auto_motion == TagID::TAG_PHYSICS_DYNAMIC ? 2 : bodies->auto_motion == TagID::TAG_PHYSICS_KINEMATIC ? 1 : 0;
                    if (ImGui::Combo("Motion", &__motion, __motions, 3))
                        bodies->auto_motion = __motion == 2 ? TagID::TAG_PHYSICS_DYNAMIC : __motion == 1 ? TagID::TAG_PHYSICS_KINEMATIC
                                                                                                        : TagID::TAG_PHYSICS_STATIC;
                    ImGui::TextDisabled("For the models loaded afterwards");
                }

                ImGui::Separator();
                const Physics::WorldStats &st = world->stats;
                ImGui::Text("Models: %u", bodies->modelCount());
                ImGui::Text("Bodies: %u (%u dynamic), joints: %u", st.bodies, st.dynamic_bodies, st.joints);
                ImGui::Text("Pairs: %u, contacts: %u", st.pairs, st.contacts);
                ImGui::Text("Update: %.2f ms (%u steps), bones: %.2f ms", world->update_ms, st.steps, bodies->update_ms);
            }
            ImGui::End();
        }
    }; // Editor END
}; // Simple END
