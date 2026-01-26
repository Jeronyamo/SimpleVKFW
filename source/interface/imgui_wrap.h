#ifndef SVKFW_IMGUI_WRAP_H
#define SVKFW_IMGUI_WRAP_H

#define IMGUI_IMPL_VULKAN_USE_VOLK

#include "interface/vkfw.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include <atomic>


namespace Simple {
    namespace ImGUI {
        // Called when button is pressed etc.
        struct ActionItf {
            ActionItf() {}
            virtual ~ActionItf() {}
            virtual void action() = 0;
        };

        struct WidgetItf {
            WidgetItf() {}
            virtual ~WidgetItf() {}

            virtual bool      begin() = 0;
            virtual void        end() = 0;
            virtual void     render() = 0;
            virtual void renderFull() = 0; // calls 'begin()', 'render()', and 'end()'
            virtual ImGuiWindowFlags reqWindowFlags() { return 0; }
        }; // WidgetItf END


        struct WidgetText : WidgetItf {
            char *text = nullptr;

            WidgetText(const std::string &_text) : text{new char[_text.size()+1]} {
                memcpy(text, _text.data(), (_text.size()+1)*sizeof(std::string::value_type));
            }
            WidgetText(const WidgetText &_widget) {
                safeDeleteArr(text);
                if (_widget.text != nullptr) {
                    size_t __text_len = std::strlen(_widget.text)+1;
                    text = new char[__text_len];
                    memcpy(text, _widget.text, __text_len*sizeof(char));
                }
            }
            virtual ~WidgetText() override { safeDeleteArr(text); }

            virtual bool      begin() override { return true; }
            virtual void        end() override {}
            virtual void     render() override { ImGui::Text(text); }
            virtual void renderFull() override { render(); }
        }; // WidgetText END

        struct WidgetButton : WidgetItf {
            char *name = nullptr;
            ActionItf *action = nullptr;
            vec2f size{0.f};

            WidgetButton(const std::string &_name, ActionItf *_action = nullptr) : name{new char[_name.size()+1]}, action{_action} {
                memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
            }
            WidgetButton(const WidgetButton &_widget) : action{_widget.action}, size{_widget.size} {
                safeDeleteArr(name);
                uint32_t __name_len = std::strlen(_widget.name)+1;
                name = new char[__name_len]{};
                memcpy(name, _widget.name, __name_len*sizeof(char));
            }
            virtual ~WidgetButton() override { safeDeleteArr(name); }

            virtual bool      begin() override { return ImGui::Button(name, {size.x, size.y}); }
            virtual void        end() override {}
            virtual void     render() override { if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
        }; // WidgetButton END

        struct WidgetCheckbox : WidgetItf {
            char *name = nullptr;
            ActionItf *action = nullptr;
            bool value = false;

            WidgetCheckbox(const std::string &_name, bool _default_value = false, ActionItf *_action = nullptr)
                            : name{new char[_name.size()+1]}, value{_default_value}, action{_action} {
                memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
            }
            WidgetCheckbox(const WidgetCheckbox &_widget) : value{_widget.value}, action{_widget.action} {
                safeDeleteArr(name);
                uint32_t __name_len = std::strlen(_widget.name)+1;
                name = new char[__name_len]{};
                memcpy(name, _widget.name, __name_len*sizeof(char));
            }
            virtual ~WidgetCheckbox() override { safeDeleteArr(name); }

            virtual bool      begin() override { return ImGui::Checkbox(name, &value); }
            virtual void        end() override {}
            virtual void     render() override { if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
        }; // WidgetCheckbox END

        struct WidgetInputText : WidgetItf {
            char *name = nullptr, *buf_str = nullptr;
            uint32_t name_size, buf_size;
            ActionItf *action = nullptr;

            WidgetInputText(const std::string &_name, uint32_t _buf_size = 255u, ActionItf *_action = nullptr)
                            : name_size{(uint32_t)_name.size()+1}, buf_size{_buf_size+1}, action{_action} {
                name    = new char[name_size]{};
                buf_str = new char[ buf_size]{};
                memcpy(name, _name.data(), name_size * sizeof(std::string::value_type));
            }
            WidgetInputText(const WidgetInputText &_widget) : name_size{_widget.name_size}, buf_size{_widget.buf_size},
                                                              action{_widget.action} {
                safeDeleteArr(name);
                safeDeleteArr(buf_str);
                name    = new char[name_size]{};
                buf_str = new char[ buf_size]{};
                memcpy(name, _widget.name, name_size * sizeof(char));
                memcpy(buf_str, _widget.buf_str, buf_size * sizeof(char));
            }
            ~WidgetInputText() {
                safeDeleteArr(name);
                safeDeleteArr(buf_str);
                name_size = 0u;
                buf_size  = 0u;
            }

            virtual bool      begin() override { return ImGui::InputText(name, buf_str, buf_size); }
            virtual void        end() override {}
            virtual void     render() override { if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
        }; // WidgetInputText END

        struct WidgetMenuItem : WidgetItf {
            char *name = nullptr, *shortcut = nullptr;
            bool is_selected = false, is_enabled = true;
            uint8_t name_len = 0, shortcut_len = 0;
            uint32_t pad_;

            WidgetMenuItem(const std::string &_name, const std::string &_shortcut = "") {
                SVKFW_ASSERT(    _name.size() < 256, std::invalid_argument, "WidgetMenuItem Constructor",     "'name' length >= 256");
                SVKFW_ASSERT(_shortcut.size() < 256, std::invalid_argument, "WidgetMenuItem Constructor", "'shortcut' length >= 256");
                name_len     = (uint8_t)     _name.size();
                shortcut_len = (uint8_t) _shortcut.size();

                name = new char[name_len + shortcut_len + 2];
                shortcut = name + name_len + 1;
                memcpy(    name,     _name.data(), (    name_len+1)*sizeof(std::string::value_type));
                memcpy(shortcut, _shortcut.data(), (shortcut_len+1)*sizeof(std::string::value_type));
            }
            WidgetMenuItem(const WidgetMenuItem &_menu_item) : name_len{_menu_item.name_len}, shortcut_len{_menu_item.shortcut_len} {
                safeDeleteArr(name);
                name = new char[name_len + shortcut_len + 2];
                shortcut = name + name_len + 1;
                memcpy(    name, _menu_item.    name, (    name_len+1)*sizeof(char));
                memcpy(shortcut, _menu_item.shortcut, (shortcut_len+1)*sizeof(char));
            }
            virtual ~WidgetMenuItem() override { safeDeleteArr(name); }

            virtual bool begin() override {
                return ImGui::MenuItem(name, shortcut, &is_selected, is_enabled);
            }
            virtual void end()        override {}
            virtual void render()     override {}
            virtual void renderFull() override {}
        }; // WidgetMenuItem END

        struct WidgetMenu : WidgetItf {
            std::string name;
            std::vector<std::pair<WidgetMenuItem, ActionItf*>> item_action;
            uint32_t is_began = false;

            WidgetMenu(const std::string &_name, const std::vector<std::pair<WidgetMenuItem, ActionItf*>> &_item_action)
                            : name{_name}, item_action{_item_action} {}
            WidgetMenu(const WidgetMenu &_menu) : name{_menu.name}, item_action{_menu.item_action} {}
            virtual ~WidgetMenu() override { end(); }

            virtual bool  begin() override {
                is_began = ImGui::BeginMenu(name.c_str());
                return is_began;
            }
            virtual void    end() override {
                if (is_began) ImGui::EndMenu();
                is_began = false;
            }
            virtual void render() override {
                for (auto &ia : item_action) {
                    if (ia.second != nullptr && ia.first.begin())
                        ia.second->action();
                }
            }
            virtual void renderFull() override {
                if (begin()) {
                    render();
                    end();
                }
            }
        }; // WidgetMenu END

        struct WidgetMenuBar : WidgetItf { // ImGuiWindowFlags_MenuBar flag for window
            std::vector<WidgetMenu> menus;
            uint32_t is_main_menu_bar = false, is_began = false;

            WidgetMenuBar(const std::vector<WidgetMenu> &_menus, bool _is_main = false) : menus{_menus}, is_main_menu_bar{_is_main} {}
            WidgetMenuBar(const WidgetMenuBar &_menu_bar) : menus{_menu_bar.menus}, is_main_menu_bar{_menu_bar.is_main_menu_bar} {}

            virtual bool  begin() override {
                is_began = is_main_menu_bar ? ImGui::BeginMainMenuBar() : ImGui::BeginMenuBar();
                return is_began;
            }
            virtual void    end() override {
                if (is_began) is_main_menu_bar ? ImGui::EndMainMenuBar() : ImGui::EndMenuBar();
                is_began = false;
            }
            virtual void render() override {
                for (auto &menu : menus) menu.renderFull();
            }
            virtual void renderFull() override {
                if (begin()) {
                    render();
                    end();
                }
            }

            virtual ImGuiWindowFlags reqWindowFlags() override { return is_main_menu_bar ? 0 : ImGuiWindowFlags_MenuBar; }
        }; // WidgetMenuBar END


        struct WidgetWindowChild : WidgetItf {
            std::string name;
            vec2f       size;
            ImGuiChildFlags   child_flags;
            ImGuiWindowFlags window_flags;
            uint32_t is_began = false;
            std::vector<WidgetItf*> widgets;

            WidgetWindowChild(const std::string &_name, vec2f _size, ImGuiChildFlags _child_flags = 0,
                              ImGuiWindowFlags _window_flags = 0) : name{_name}, child_flags{_child_flags},
                                                                    window_flags{_window_flags} {}

            virtual bool  begin() override {
                ImGui::BeginChild(name.c_str(), {size.x, size.y}, child_flags, window_flags);
                is_began = true;
                return is_began;
            }
            virtual void    end() override {
                if (is_began) ImGui::EndChild();
                is_began = false;
            }
            virtual void render() override {
                for (auto& w : widgets) w->renderFull();
            }
            virtual void renderFull() override {
                if (begin()) {
                    render();
                }
                end();
            }

            virtual ImGuiWindowFlags reqWindowFlags() override {
                ImGuiWindowFlags __res = 0;
                for (auto& w : widgets) __res |= w->reqWindowFlags();
                return __res;
            }
        }; // WidgetWindowChild END

        struct WidgetWindow : WidgetItf {
            std::string name;
            std::vector<WidgetItf*> widgets;
            ImGuiWindowFlags window_flags;
            uint16_t is_began = false;
            bool is_window_open = true, pad_;

            WidgetWindow(const std::string &_name, ImGuiWindowFlags _window_flags = 0) : name{_name}, window_flags{_window_flags} {}

            virtual bool  begin() override {
                ImGui::Begin(name.c_str(), &is_window_open, window_flags);
                is_began = true;
                return is_began;
            }
            virtual void    end() override {
                if (is_began) ImGui::End();
                is_began = false;
            }
            virtual void render() override {
                for (auto& w : widgets) w->renderFull();
            }
            virtual void renderFull() override {
                if (begin()) {
                    render();
                }
                end();
            }

            virtual ImGuiWindowFlags reqWindowFlags() override {
                ImGuiWindowFlags __res = 0;
                for (auto& w : widgets) __res |= w->reqWindowFlags();
                return __res;
            }
        }; // WidgetWindow END


        struct ImGuiHandler {
            ImGuiContext *context = nullptr;
            ImGui_ImplVulkan_InitInfo imgui_info{};
            uint32_t min_image_count = 2u;
            uint32_t descr_pool_size = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
            std::vector<WidgetItf*> widgets; // Main Menu Bar or Window widgets

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

            void newFrame() {
                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
            }
            void render() {
                for (auto& w : widgets) w->renderFull();
                ImGui::Render();
            }
            void fillCommandBuffer(VkCommandBuffer _cmd_buffer) {
                ImDrawData* __draw_data = ImGui::GetDrawData();
                ImGui_ImplVulkan_RenderDrawData(__draw_data, _cmd_buffer);
            }

            void initializeContext(VKFW::VulkanContext* _vk_context, VKFW::ContextIndex _ci_qfamily,
                                VKFW::ContextIndex _ci_graphics_queue, uint32_t _image_count) {
                imgui_info.Instance = _vk_context->instance;
                imgui_info.PhysicalDevice = _vk_context->phys_device;
                imgui_info.Device = _vk_context->device;
                imgui_info.QueueFamily = _vk_context->queue_families[_ci_qfamily];
                imgui_info.Queue = _vk_context->queues[_ci_graphics_queue];
                imgui_info.PipelineInfoMain.RenderPass = _vk_context->render_pass;
                // imgui_info.DescriptorPool = _vk_context->descriptor_context.pools[_ci_descriptor_pool];
                imgui_info.MinImageCount = getMinImageCount();
                imgui_info.ImageCount = std::max(_image_count, getMinImageCount());
                imgui_info.DescriptorPoolSize = getMinPoolSize();
            }

            void setMinImageCount(uint32_t _new_count) {
                if (_new_count < 2) return;
                min_image_count = _new_count;
                ImGui_ImplVulkan_SetMinImageCount(_new_count);
            }
            uint32_t getMinImageCount() {
                return min_image_count; // read ImGui_ImplVulkan_InitInfo::MinImageCount description
            }
            uint32_t getMinPoolSize() {
                return descr_pool_size;
            }

            bool isGUIHovered() {
                return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
            }
            bool isKeyboardInUse() {
                return ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
            }
        }; // ImGuiHandler END
    }; // ImGUI END
}; // Simple END

#endif