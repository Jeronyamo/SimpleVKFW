#ifndef SVKFW_IMGUI_WRAP_H
#define SVKFW_IMGUI_WRAP_H

#define IMGUI_IMPL_VULKAN_USE_VOLK

#include "interface/vkfw.h"
#include "main/colormodel.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include <atomic>


namespace Simple {
    namespace ImGUI {
        // Called when button is pressed etc.
        struct ActionItf {
            ActionItf();
            virtual ~ActionItf();
            virtual void action() = 0;
        };

        struct WidgetItf {
            WidgetItf();
            virtual ~WidgetItf();

            virtual bool      begin() = 0;
            virtual void        end() = 0;
            virtual void     render() = 0;
            virtual void renderFull() = 0; // calls 'begin()', 'render()', and 'end()'
            virtual ImGuiWindowFlags reqWindowFlags();
        }; // WidgetItf END


// Widget wrappers

    // "Static"/"Utility" widgets

        // Text output

        struct WidgetText : WidgetItf {
            char *text = nullptr;

            WidgetText(const std::string &_text);
            WidgetText(const WidgetText &_widget);
            virtual ~WidgetText() override;

            virtual bool      begin() override;
            virtual void        end() override;
            virtual void     render() override; // Note: 'ImGui::Text(text)' treats text as a format string
            virtual void renderFull() override;
        }; // WidgetText END

        // TODO: ImGui::TextColored(), ImGui::TextWrapped(), ImGui::LabelText(), ImGui::BulletText()

        // TODO: ImGui::Spacing(), ImGui::NewLine(), ImGui::SameLine(), ImGui::Separator(), ImGui::Dummy()

        // TODO: ImGui::BeginTooltip(), ImGui::EndTooltip(), ImGui::BeginPopup(), ImGui::EndPopup(), ImGui::BeginPopupModal(), ImGui::EndPopupModal()


    // "Interactive"/"Main" widgets

        // Buttons

        struct WidgetButton : WidgetItf {
            char *name = nullptr;
            ActionItf *action = nullptr;
            vec2f size{0.f};

            WidgetButton(const std::string &_name, ActionItf *_action = nullptr, vec2f _size = {});
            WidgetButton(const WidgetButton &_widget);
            virtual ~WidgetButton() override;

            virtual bool      begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetButton END

        struct WidgetSmallButton : WidgetItf {
            char *name = nullptr;
            ActionItf *action = nullptr;

            WidgetSmallButton(const std::string &_name, ActionItf *_action = nullptr);
            WidgetSmallButton(const WidgetSmallButton &_widget);
            virtual ~WidgetSmallButton() override;

            virtual bool      begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetSmallButton END

        // TODO: ImGui::InvisibleButton(), ImGui::ImageButton()

        struct WidgetRadioButton : WidgetItf {
            char *name = nullptr;
            ActionItf *action = nullptr;
            bool active = false;

            WidgetRadioButton(const std::string &_name, ActionItf *_action = nullptr, bool _active = true);
            WidgetRadioButton(const WidgetRadioButton &_widget);
            virtual ~WidgetRadioButton() override;

            virtual bool      begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetRadioButton END

        // TODO: ImGui::ColorButton()

        struct WidgetCheckbox : WidgetItf {
            char *name = nullptr;
            ActionItf *action = nullptr;
            bool value = false;

            WidgetCheckbox(const std::string &_name, bool _default_value = false, ActionItf *_action = nullptr);
            WidgetCheckbox(const WidgetCheckbox &_widget);
            virtual ~WidgetCheckbox() override;

            virtual bool      begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetCheckbox END


        // Input text/multiline

        struct WidgetInputText : WidgetItf {
            char *name = nullptr, *buf_str = nullptr;
            uint32_t name_size, buf_size;
            ActionItf *action = nullptr;

            WidgetInputText(const std::string &_name, uint32_t _buf_size = 255u, ActionItf *_action = nullptr);
            WidgetInputText(const WidgetInputText &_widget);
            ~WidgetInputText();

            virtual bool      begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetInputText END

        // TODO: ImGui::InputTextMultiline()


        // Input numbers

        struct WidgetInputFloat : WidgetItf {
            char *name = nullptr;
            const char *format = "%.3f";
            ActionItf *action = nullptr;
            vec4f value{};
            float step = 0.f, step_fast = 0.f;
            uint32_t val_count = 0u; // 1 to 4
            ImGuiInputTextFlags flags = 0;

            WidgetInputFloat(const std::string &_name, uint32_t _val_count, vec4f _default_value = {0.f}, const char *_format = "%.3f",
                             ActionItf *_action = nullptr, ImGuiInputTextFlags _flags = 0, float _step = 0.f, float _step_fast = 0.f);
            WidgetInputFloat(const WidgetInputFloat &_widget);
            virtual ~WidgetInputFloat() override;

            virtual bool begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetInputFloat END

        struct WidgetInputInt32 : WidgetItf {
            char *name = nullptr;
            ActionItf *action = nullptr;
            vec4i32 value{};
            int32_t step = 1, step_fast = 100;
            uint32_t val_count = 0u; // 1 to 4
            ImGuiInputTextFlags flags = 0;

            WidgetInputInt32(const std::string &_name, uint32_t _val_count, vec4i32 _default_value = {0}, ActionItf *_action = nullptr,
                             ImGuiInputTextFlags _flags = 0, int32_t _step = 1, int32_t _step_fast = 100);
            WidgetInputInt32(const WidgetInputInt32 &_widget);
            virtual ~WidgetInputInt32() override;

            virtual bool begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetInputInt32 END

        struct WidgetInputUint32 : WidgetItf {
            char *name = nullptr;
            const char *format = "%d";
            ActionItf *action = nullptr;
            vec4u32 value{};
            uint32_t step = 1u, step_fast = 100u;
            uint32_t val_count = 0u; // 1 to 4
            ImGuiInputTextFlags flags = 0; // Does not support 'ImGuiInputTextFlags_CharsHexadecimal' flag. If value is expected to be hexadecimal, use 'format' = "%08X" instead

            WidgetInputUint32(const std::string &_name, uint32_t _val_count, vec4u32 _default_value = {0u}, const char *_format = "%u",
                              ActionItf *_action = nullptr, ImGuiInputTextFlags _flags = 0, uint32_t _step = 1u, uint32_t _step_fast = 100u);
            WidgetInputUint32(const WidgetInputUint32 &_widget);
            virtual ~WidgetInputUint32() override;

            virtual bool begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetInputUint32 END

        // TODO: ImGui::InputScalar() - general case, only if needed


        // Sliders

        struct WidgetSliderFloat : WidgetItf {
            char *name = nullptr;
            const char *format = "%.3f";
            ActionItf *action = nullptr;
            vec4f value{};
            vec2f limits = {0.f, 1.f}; // '.x' - min, '.y' - max
            uint32_t val_count = 0u; // 1 to 4
            ImGuiSliderFlags flags = 0;

            WidgetSliderFloat(const std::string &_name, uint32_t _val_count, vec4f _default_value = {0.f}, float _lim_min = 0.f,
                              float _lim_max = 1.f, const char *_format = "%.3f", ActionItf *_action = nullptr, ImGuiSliderFlags _flags = 0);
            WidgetSliderFloat(const WidgetSliderFloat &_widget);
            virtual ~WidgetSliderFloat() override;

            virtual bool begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetSliderFloat END

        struct WidgetSliderInt32 : WidgetItf {
            char *name = nullptr;
            const char *format = "%d";
            ActionItf *action = nullptr;
            vec4i32 value{};
            vec2i32 limits = {0, 100}; // '.x' - min, '.y' - max
            uint32_t val_count = 0u; // 1 to 4
            ImGuiSliderFlags flags = 0;

            WidgetSliderInt32(const std::string &_name, uint32_t _val_count, vec4i32 _default_value = {0}, int32_t _lim_min = 0,
                              int32_t _lim_max = 100, const char *_format = "%d", ActionItf *_action = nullptr, ImGuiSliderFlags _flags = 0);
            WidgetSliderInt32(const WidgetSliderInt32 &_widget);
            virtual ~WidgetSliderInt32() override;

            virtual bool begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetSliderInt32 END

        struct WidgetSliderUint32 : WidgetItf {
            char *name = nullptr;
            const char *format = "%d";
            ActionItf *action = nullptr;
            vec4u32 value{};
            vec2u32 limits = {0u, 100u}; // '.x' - min, '.y' - max
            uint32_t val_count = 0u; // 1 to 4
            ImGuiSliderFlags flags = 0;

            WidgetSliderUint32(const std::string &_name, uint32_t _val_count, vec4u32 _default_value = {0u}, uint32_t _lim_min = 0u,
                               uint32_t _lim_max = 100u, const char *_format = "%u", ActionItf *_action = nullptr, ImGuiSliderFlags _flags = 0);
            WidgetSliderUint32(const WidgetSliderUint32 &_widget);
            virtual ~WidgetSliderUint32() override;

            virtual bool begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetSliderUint32 END

        // TODO: ImGui::SliderScalar() - general case, only if needed


        // Drag widgets

        struct WidgetDragFloat : WidgetItf {
            char *name = nullptr;
            const char *format = "%.3f";
            ActionItf *action = nullptr;
            vec4f value{};
            vec2f limits = {0.f, 1.f}; // '.x' - min, '.y' - max
            float speed = 1.f;
            uint32_t val_count = 0u; // 1 to 4
            ImGuiSliderFlags flags = 0;

            WidgetDragFloat(const std::string &_name, uint32_t _val_count, vec4f _default_value = {0.f}, float _lim_min = 0.f, float _lim_max = 1.f,
                            float _speed = 1.f, const char *_format = "%.3f", ActionItf *_action = nullptr, ImGuiSliderFlags _flags = 0);
            WidgetDragFloat(const WidgetDragFloat &_widget);
            virtual ~WidgetDragFloat() override;

            virtual bool begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetDragFloat END

        struct WidgetDragInt32 : WidgetItf {
            char *name = nullptr;
            const char *format = "%d";
            ActionItf *action = nullptr;
            vec4i32 value{};
            vec2i32 limits = {0, 100}; // '.x' - min, '.y' - max
            float speed = 1.f;
            uint32_t val_count = 0u; // 1 to 4
            ImGuiSliderFlags flags = 0;

            WidgetDragInt32(const std::string &_name, uint32_t _val_count, vec4i32 _default_value = {0}, const char *_format = "%d", int32_t _lim_min = 0,
                            int32_t _lim_max = 100, float _speed = 1.f, ActionItf *_action = nullptr, ImGuiSliderFlags _flags = 0);
            WidgetDragInt32(const WidgetDragInt32 &_widget);
            virtual ~WidgetDragInt32() override;

            virtual bool begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetDragInt32 END

        struct WidgetDragUint32 : WidgetItf {
            char *name = nullptr;
            const char *format = "%d";
            ActionItf *action = nullptr;
            vec4u32 value{};
            vec2u32 limits = {0u, 100u}; // '.x' - min, '.y' - max
            float speed = 1.f;
            uint32_t val_count = 0u; // 1 to 4
            ImGuiSliderFlags flags = 0;

            WidgetDragUint32(const std::string &_name, uint32_t _val_count, vec4u32 _default_value = {0u}, const char *_format = "%u",
                             uint32_t _lim_min = 0u, uint32_t _lim_max = 100u, float _speed = 1.f, ActionItf *_action = nullptr, ImGuiSliderFlags _flags = 0);
            WidgetDragUint32(const WidgetDragUint32 &_widget);
            virtual ~WidgetDragUint32() override;

            virtual bool begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;
        }; // WidgetDragUint32 END

        // TODO: ImGui::DragScalar() - general case, only if needed


        // Color

        // TODO: ImGui::ColorEdit3(), ImGui::ColorEdit4(), ImGui::ColorPicker3(), ImGui::ColorPicker4()


        // Plots

        // TODO: ImGui::PlotLines(), ImGui::PlotHistogram()

        // TODO: ImGui::Image(), ImGui::ProgressBar(), ImGui::Bullet()


        // Tab

        // TODO: ImGui::BeginTabBar(), ImGui::BeginTabItem(), ImGui::EndTabItem()

        // TODO: ImGui::Columns(), ImGui::BeginTable()


        // Tree structures

        // TODO: ImGui::TreeNode(), ImGui::TreePush(), ImGui::TreePop(), ImGui::CollapsingHeader()

        // TODO: ImGui::Combo(), ImGui::ListBox(), ImGui::Selectable()


        // Custom widgets

        struct WidgetAudioPlayer : WidgetItf {
            
        }; // WidgetAudioPlayer END


        // Menu widgets

        struct WidgetMenuItem : WidgetItf {
            char *name = nullptr, *shortcut = nullptr;
            bool is_selected = false, is_enabled = true;
            uint8_t name_len = 0, shortcut_len = 0;
            uint32_t pad_;

            WidgetMenuItem(const std::string &_name, const std::string &_shortcut = "");
            WidgetMenuItem(const WidgetMenuItem &_menu_item);
            WidgetMenuItem& operator=(const WidgetMenuItem &_menu_item);
            virtual ~WidgetMenuItem() override;

            virtual bool begin() override;
            virtual void end()        override;
            virtual void render()     override;
            virtual void renderFull() override;
        }; // WidgetMenuItem END

        struct WidgetMenu : WidgetItf {
            std::string name;
            std::vector<std::pair<WidgetMenuItem, ActionItf*>> item_action;
            uint32_t is_began = false;

            WidgetMenu(const std::string &_name, const std::vector<std::pair<WidgetMenuItem, ActionItf*>> &_item_action);
            WidgetMenu(const WidgetMenu &_menu);
            virtual ~WidgetMenu() override;

            virtual bool  begin() override;
            virtual void    end() override;
            virtual void render() override;
            virtual void renderFull() override;
        }; // WidgetMenu END

        struct WidgetMenuBar : WidgetItf { // ImGuiWindowFlags_MenuBar flag for window
            std::vector<WidgetMenu> menus;
            uint32_t is_main_menu_bar = false, is_began = false;

            WidgetMenuBar(const std::vector<WidgetMenu> &_menus, bool _is_main = false);
            WidgetMenuBar(const WidgetMenuBar &_menu_bar);

            virtual bool  begin() override;
            virtual void    end() override;
            virtual void render() override;
            virtual void renderFull() override;

            virtual ImGuiWindowFlags reqWindowFlags() override;
        }; // WidgetMenuBar END


        // Window widgets

        struct WidgetWindowChild : WidgetItf {
            std::string name;
            vec2f       size;
            ImGuiChildFlags   child_flags;
            ImGuiWindowFlags window_flags;
            uint32_t is_began = false;
            std::vector<WidgetItf*> widgets;

            WidgetWindowChild(const std::string &_name, vec2f _size = {}, ImGuiChildFlags _child_flags = 0,
                              ImGuiWindowFlags _window_flags = 0);

            virtual bool  begin() override;
            virtual void    end() override;
            virtual void render() override;
            virtual void renderFull() override;

            virtual ImGuiWindowFlags reqWindowFlags() override;
        }; // WidgetWindowChild END

        struct WidgetWindow : WidgetItf {
            std::string name;
            std::vector<WidgetItf*> widgets;
            ImGuiWindowFlags window_flags;
            uint16_t is_began = false;
            bool is_window_open = true, pad_;

            WidgetWindow(const std::string &_name, ImGuiWindowFlags _window_flags = 0);

            virtual bool  begin() override;
            virtual void    end() override;
            virtual void render() override;
            virtual void renderFull() override;

            virtual ImGuiWindowFlags reqWindowFlags() override;
        }; // WidgetWindow END


// ImGUI Handler

        struct ImGuiHandler {
            ImGuiContext *context = nullptr;
            ImGui_ImplVulkan_InitInfo imgui_info{};
            ImGuiIO    *imgui_io;
            ImGuiStyle *imgui_style;
            uint32_t min_image_count = 2u;
            uint32_t descr_pool_size = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
            std::vector<WidgetItf*> widgets; // Main Menu Bar or Window widgets
            bool backends_initialized = false;
            VkFormat color_format = VK_FORMAT_UNDEFINED; // Note: the attachment format for dynamic rendering, must outlive the backend


            ImGuiHandler();
           ~ImGuiHandler();

            void createContext();

            void destroyContext();

            void enableForSVKFWWindow(const Window &_w, bool _install_callbacks);
            void enableForGLFWwindow(GLFWwindow *_w, bool _install_callbacks);

            void setDefaultStyleSVKFW();
            void setFont(const std::string &_font_path, float _size_pixels = 0.f);

            void newFrame();
            void render();
            void fillCommandBuffer(VkCommandBuffer _cmd_buffer);

            void initializeContext(const VKFW::VulkanContext &_vkfw, VKFW::QueueFamilyId _qfamily_id,
                                   VKFW::QueueId _graphics_queue_id, uint32_t _image_count, VkRenderPass _render_pass = VK_NULL_HANDLE);

            void setMinImageCount(uint32_t _new_count);
            uint32_t getMinImageCount();
            uint32_t getMinPoolSize();

            // Note: 'WantCapture*' flags are set by ImGui when the input must not be passed to the application
            bool isGUIHovered();
            bool isKeyboardInUse();
        }; // ImGuiHandler END
    }; // ImGUI END
}; // Simple END

#endif