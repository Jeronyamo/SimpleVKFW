#ifndef SVKFW_IMGUIWRAP_H
#define SVKFW_IMGUIWRAP_H

#include "interface/glfwrap.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <atomic>


// IMGUIGUI.H - This file contains various pieces of ImGUI interface. ImGUI-backend-independent.
// OGLGUI.H - Everything that binds ImGUI and GLFW/OpenGL together
// ^ changed to vulkan, then moved from vkgui.h back here

namespace Simple {
    //TODO: Rethink about ImGUI context and initialisation
    struct ImGUIContext {
        ImGuiContext *context;

         ImGUIContext() : context{ImGui::CreateContext()} {}
        ~ImGUIContext() { ImGui::DestroyContext(context); }
    };

    struct ImGUIInitClass {
        static std::atomic_flag initialized; 
        ImGUIInitClass() {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
        }

        void enableForWindow(const Window &_w, bool install_callbacks) {
            if (!initialized.test_and_set()) {
                ImGui_ImplGlfw_InitForOpenGL(_w.window, install_callbacks);
                ImGui_ImplOpenGL3_Init();
            }
        }

        void enableForGLFWwindow(GLFWwindow *_w, bool install_callbacks) {
            if (!initialized.test_and_set()) {
                ImGui_ImplGlfw_InitForOpenGL(_w, install_callbacks);
                ImGui_ImplOpenGL3_Init();
            }
        }

        ~ImGUIInitClass() {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    } imgui_init;

    std::atomic_flag ImGUIInitClass::initialized = ATOMIC_FLAG_INIT; 

};

#endif