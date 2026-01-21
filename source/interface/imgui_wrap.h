#ifndef SVKFW_IMGUI_WRAP_H
#define SVKFW_IMGUI_WRAP_H

#define IMGUI_IMPL_VULKAN_USE_VOLK

#include "interface/vkfw.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <atomic>


namespace Simple {
    struct ImGuiHandler {
        ImGuiContext *context = nullptr;
        ImGui_ImplVulkan_InitInfo imgui_info{};

         ImGuiHandler() {
            SVKFW_WASSERT(IMGUI_CHECKVERSION(), "ImGuiHandler Constructor", "ImGUI version mismatch\n");
            context = ImGui::CreateContext();
        }
        ~ImGuiHandler() {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext(context);
        }

        void enableForSVKFWWindow(const Window &_w, bool _install_callbacks) {
            ImGui_ImplGlfw_InitForVulkan(_w.window, _install_callbacks);
            ImGui_ImplVulkan_Init(&imgui_info);
        }
        void enableForGLFWwindow(GLFWwindow *_w, bool _install_callbacks) {
            ImGui_ImplGlfw_InitForVulkan(_w, _install_callbacks);
            ImGui_ImplVulkan_Init(&imgui_info);
        }

        void initializeContext(VKFW::VulkanContext* _vk_context, VKFW::ContextIndex _ci_qfamily,
                               VKFW::ContextIndex _ci_graphics_queue, uint32_t _image_count) {
            imgui_info.Instance = _vk_context->instance;
            imgui_info.PhysicalDevice = _vk_context->phys_device;
            imgui_info.Device = _vk_context->device;
            imgui_info.QueueFamily = _vk_context->queue_families[_ci_qfamily];
            imgui_info.Queue = _vk_context->queues[_ci_graphics_queue];
            imgui_info.RenderPass = _vk_context->render_pass;
            // imgui_info.DescriptorPool = _vk_context->descriptor_context.pools[_ci_descriptor_pool];
            imgui_info.MinImageCount = getImguiMinImageCount();
            imgui_info.ImageCount = std::max(_image_count, getImguiMinImageCount());
            imgui_info.DescriptorPoolSize = getImguiMinPoolSize();
        }

        uint32_t getImguiMinImageCount() {
            return 2; // read ImGui_ImplVulkan_InitInfo::MinImageCount description
        }
        uint32_t getImguiMinPoolSize() {
            return IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
        }
    };
};

#endif