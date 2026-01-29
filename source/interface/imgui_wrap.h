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


// Widget wrappers

    // "Static"/"Utility" widgets

        // Text output

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

        // TODO: ImGui::TextColored(), ImGui::TextWrapped(), ImGui::LabelText(), ImGui::BulletText()

        // TODO: ImGui::Spacing(), ImGui::NewLine(), ImGui::SameLine(), ImGui::Separator(), ImGui::Dummy()

        // TODO: ImGui::BeginTooltip(), ImGui::EndTooltip(), ImGui::BeginPopup(), ImGui::EndPopup(), ImGui::BeginPopupModal(), ImGui::EndPopupModal()


    // "Interactive"/"Main" widgets

        // Buttons

        struct WidgetButton : WidgetItf {
            char *name = nullptr;
            ActionItf *action = nullptr;
            vec2f size{0.f};

            WidgetButton(const std::string &_name, ActionItf *_action = nullptr, vec2f _size = {}) : name{new char[_name.size()+1]}, action{_action}, size{_size} {
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

        struct WidgetSmallButton : WidgetItf {
            char *name = nullptr;
            ActionItf *action = nullptr;

            WidgetSmallButton(const std::string &_name, ActionItf *_action = nullptr) : name{new char[_name.size()+1]}, action{_action} {
                memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
            }
            WidgetSmallButton(const WidgetSmallButton &_widget) : action{_widget.action} {
                safeDeleteArr(name);
                uint32_t __name_len = std::strlen(_widget.name)+1;
                name = new char[__name_len]{};
                memcpy(name, _widget.name, __name_len*sizeof(char));
            }
            virtual ~WidgetSmallButton() override { safeDeleteArr(name); }

            virtual bool      begin() override { return ImGui::SmallButton(name); }
            virtual void        end() override {}
            virtual void     render() override { if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
        }; // WidgetSmallButton END

        // TODO: ImGui::InvisibleButton(), ImGui::ImageButton()

        struct WidgetRadioButton : WidgetItf {
            char *name = nullptr;
            ActionItf *action = nullptr;
            bool active = false;

            WidgetRadioButton(const std::string &_name, ActionItf *_action = nullptr, bool _active = true) : name{new char[_name.size()+1]}, action{_action}, active{_active} {
                memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
            }
            WidgetRadioButton(const WidgetRadioButton &_widget) : action{_widget.action}, active{_widget.active} {
                safeDeleteArr(name);
                uint32_t __name_len = std::strlen(_widget.name)+1;
                name = new char[__name_len]{};
                memcpy(name, _widget.name, __name_len*sizeof(char));
            }
            virtual ~WidgetRadioButton() override { safeDeleteArr(name); }

            virtual bool      begin() override { return ImGui::RadioButton(name, active); }
            virtual void        end() override {}
            virtual void     render() override { active = !active; printf("Radio button is %d\n", active);   if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
        }; // WidgetRadioButton END

        // TODO: ImGui::ColorButton()

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


        // Input text/multiline

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
                             ActionItf *_action = nullptr, ImGuiInputTextFlags _flags = 0, float _step = 0.f, float _step_fast = 0.f)
                            : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, action{_action},
                              flags{_flags}, step{_step}, step_fast{_step_fast} {
                SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetInputFloat Constructor", "expected 'val_count' values are [1,4]\n");
                memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
            }
            WidgetInputFloat(const WidgetInputFloat &_widget) : value{_widget.value}, action{_widget.action}, step{_widget.step},
                                                                step_fast{_widget.step_fast}, val_count{_widget.val_count}, flags{_widget.flags} {
                safeDeleteArr(name);
                uint32_t __name_len = std::strlen(_widget.name)+1;
                name = new char[__name_len]{};
                memcpy(name, _widget.name, __name_len*sizeof(char));
            }
            virtual ~WidgetInputFloat() override { safeDeleteArr(name); }

            virtual bool begin() override {
                switch (val_count) {
                    case 1: return ImGui::InputFloat (name, &value.x, step, step_fast, format, flags);
                    case 2: return ImGui::InputFloat2(name, &value.x, format, flags);
                    case 3: return ImGui::InputFloat3(name, &value.x, format, flags);
                    case 4: return ImGui::InputFloat4(name, &value.x, format, flags);
                    default: break;
                }
                return false;
            }
            virtual void        end() override {}
            virtual void     render() override { if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
        }; // WidgetInputFloat END

        struct WidgetInputInt32 : WidgetItf {
            char *name = nullptr;
            ActionItf *action = nullptr;
            vec4i32 value{};
            int32_t step = 1, step_fast = 100;
            uint32_t val_count = 0u; // 1 to 4
            ImGuiInputTextFlags flags = 0;

            WidgetInputInt32(const std::string &_name, uint32_t _val_count, vec4i32 _default_value = {0}, ActionItf *_action = nullptr,
                             ImGuiInputTextFlags _flags = 0, int32_t _step = 1, int32_t _step_fast = 100)
                            : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, action{_action},
                              flags{_flags}, step{_step}, step_fast{_step_fast} {
                SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetInputInt32 Constructor", "expected 'val_count' values are [1,4]\n");
                memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
            }
            WidgetInputInt32(const WidgetInputInt32 &_widget) : value{_widget.value}, action{_widget.action}, step{_widget.step},
                                                                step_fast{_widget.step_fast}, val_count{_widget.val_count}, flags{_widget.flags} {
                safeDeleteArr(name);
                uint32_t __name_len = std::strlen(_widget.name)+1;
                name = new char[__name_len]{};
                memcpy(name, _widget.name, __name_len*sizeof(char));
            }
            virtual ~WidgetInputInt32() override { safeDeleteArr(name); }

            virtual bool begin() override {
                switch (val_count) {
                    case 1: return ImGui::InputInt (name, &value.x, step, step_fast, flags);
                    case 2: return ImGui::InputInt2(name, &value.x, flags);
                    case 3: return ImGui::InputInt3(name, &value.x, flags);
                    case 4: return ImGui::InputInt4(name, &value.x, flags);
                    default: break;
                }
                return false;
            }
            virtual void        end() override {}
            virtual void     render() override { if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
        }; // WidgetInputInt32 END

        struct WidgetInputUint32 : WidgetItf {
            char *name = nullptr;
            const char *format = "%d";
            ActionItf *action = nullptr;
            vec4u32 value{};
            uint32_t step = 1u, step_fast = 100u;
            uint32_t val_count = 0u; // 1 to 4
            ImGuiInputTextFlags flags = 0; // Does not support 'ImGuiInputTextFlags_CharsHexadecimal' flag. If value is expected to be hexadecimal, use 'format' = "%08X" instead

            WidgetInputUint32(const std::string &_name, uint32_t _val_count, vec4u32 _default_value = {0u}, const char *_format = "%d",
                              ActionItf *_action = nullptr, ImGuiInputTextFlags _flags = 0, uint32_t _step = 1u, uint32_t _step_fast = 100u)
                            : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, format{_format},
                              action{_action}, flags{_flags}, step{_step}, step_fast{_step_fast} {
                SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetInputUint32 Constructor", "expected 'val_count' values are [1,4]\n");
                memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
            }
            WidgetInputUint32(const WidgetInputUint32 &_widget) : value{_widget.value}, action{_widget.action}, step{_widget.step},
                                                                  step_fast{_widget.step_fast}, val_count{_widget.val_count}, flags{_widget.flags} {
                safeDeleteArr(name);
                uint32_t __name_len = std::strlen(_widget.name)+1;
                name = new char[__name_len]{};
                memcpy(name, _widget.name, __name_len*sizeof(char));
            }
            virtual ~WidgetInputUint32() override { safeDeleteArr(name); }

            virtual bool begin() override {
                switch (val_count) {
                    case 1: return ImGui::InputScalar (name, ImGuiDataType_U32, &value.x,            &step, &step_fast, format, flags);
                    case 2:
                    case 3:
                    case 4: return ImGui::InputScalarN(name, ImGuiDataType_U32, &value.x, val_count, &step, &step_fast, format, flags);
                    default: break;
                }
                return false;
            }
            virtual void        end() override {}
            virtual void     render() override { if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
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
                              float _lim_max = 1.f, const char *_format = "%.3f", ActionItf *_action = nullptr, ImGuiSliderFlags _flags = 0)
                            : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, format{_format},
                              action{_action}, flags{_flags}, limits{_lim_min, _lim_max} {
                SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetSliderFloat Constructor", "expected 'val_count' values are [1,4]\n");
                memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
            }
            WidgetSliderFloat(const WidgetSliderFloat &_widget) : value{_widget.value}, action{_widget.action}, limits{_widget.limits},
                                                                val_count{_widget.val_count}, flags{_widget.flags} {
                safeDeleteArr(name);
                uint32_t __name_len = std::strlen(_widget.name)+1;
                name = new char[__name_len]{};
                memcpy(name, _widget.name, __name_len*sizeof(char));
            }
            virtual ~WidgetSliderFloat() override { safeDeleteArr(name); }

            virtual bool begin() override {
                switch (val_count) {
                    case 1: return ImGui::SliderFloat (name, &value.x, limits.x, limits.y, format, flags);
                    case 2: return ImGui::SliderFloat2(name, &value.x, limits.x, limits.y, format, flags);
                    case 3: return ImGui::SliderFloat3(name, &value.x, limits.x, limits.y, format, flags);
                    case 4: return ImGui::SliderFloat4(name, &value.x, limits.x, limits.y, format, flags);
                    default: break;
                }
                return false;
            }
            virtual void        end() override {}
            virtual void     render() override { if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
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
                              int32_t _lim_max = 100, const char *_format = "%d", ActionItf *_action = nullptr, ImGuiSliderFlags _flags = 0)
                            : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, format{_format},
                              action{_action}, flags{_flags}, limits{_lim_min, _lim_max} {
                SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetSliderInt32 Constructor", "expected 'val_count' values are [1,4]\n");
                memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
            }
            WidgetSliderInt32(const WidgetSliderInt32 &_widget) : value{_widget.value}, action{_widget.action}, limits{_widget.limits},
                                                                val_count{_widget.val_count}, flags{_widget.flags} {
                safeDeleteArr(name);
                uint32_t __name_len = std::strlen(_widget.name)+1;
                name = new char[__name_len]{};
                memcpy(name, _widget.name, __name_len*sizeof(char));
            }
            virtual ~WidgetSliderInt32() override { safeDeleteArr(name); }

            virtual bool begin() override {
                switch (val_count) {
                    case 1: return ImGui::SliderInt (name, &value.x, limits.x, limits.y, format, flags);
                    case 2: return ImGui::SliderInt2(name, &value.x, limits.x, limits.y, format, flags);
                    case 3: return ImGui::SliderInt3(name, &value.x, limits.x, limits.y, format, flags);
                    case 4: return ImGui::SliderInt4(name, &value.x, limits.x, limits.y, format, flags);
                    default: break;
                }
                return false;
            }
            virtual void        end() override {}
            virtual void     render() override { if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
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
                               uint32_t _lim_max = 100u, const char *_format = "%d", ActionItf *_action = nullptr, ImGuiSliderFlags _flags = 0)
                            : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, action{_action}, flags{_flags}, limits{_lim_min, _lim_max} {
                SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetSliderUint32 Constructor", "expected 'val_count' values are [1,4]\n");
                memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
            }
            WidgetSliderUint32(const WidgetSliderUint32 &_widget) : value{_widget.value}, action{_widget.action}, limits{_widget.limits},
                                                                val_count{_widget.val_count}, flags{_widget.flags} {
                safeDeleteArr(name);
                uint32_t __name_len = std::strlen(_widget.name)+1;
                name = new char[__name_len]{};
                memcpy(name, _widget.name, __name_len*sizeof(char));
            }
            virtual ~WidgetSliderUint32() override { safeDeleteArr(name); }

            virtual bool begin() override {
                switch (val_count) {
                    case 1: return ImGui::SliderScalar (name, ImGuiDataType_U32, &value.x,            &limits.x, &limits.y, format, flags);
                    case 2:
                    case 3:
                    case 4: return ImGui::SliderScalarN(name, ImGuiDataType_U32, &value.x, val_count, &limits.x, &limits.y, format, flags);
                    default: break;
                }
                return false;
            }
            virtual void        end() override {}
            virtual void     render() override { if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
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
                            float _speed = 1.f, const char *_format = "%.3f", ActionItf *_action = nullptr, ImGuiSliderFlags _flags = 0)
                            : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, format{_format}, action{_action}, flags{_flags}, limits{_lim_min, _lim_max}, speed{_speed} {
                SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetDragFloat Constructor", "expected 'val_count' values are [1,4]\n");
                memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
            }
            WidgetDragFloat(const WidgetDragFloat &_widget) : value{_widget.value}, action{_widget.action}, limits{_widget.limits},
                                                              val_count{_widget.val_count}, flags{_widget.flags} {
                safeDeleteArr(name);
                uint32_t __name_len = std::strlen(_widget.name)+1;
                name = new char[__name_len]{};
                memcpy(name, _widget.name, __name_len*sizeof(char));
            }
            virtual ~WidgetDragFloat() override { safeDeleteArr(name); }

            virtual bool begin() override {
                switch (val_count) {
                    case 1: return ImGui::DragFloat (name, &value.x, speed, limits.x, limits.y, format, flags);
                    case 2: return ImGui::DragFloat2(name, &value.x, speed, limits.x, limits.y, format, flags);
                    case 3: return ImGui::DragFloat3(name, &value.x, speed, limits.x, limits.y, format, flags);
                    case 4: return ImGui::DragFloat4(name, &value.x, speed, limits.x, limits.y, format, flags);
                    default: break;
                }
                return false;
            }
            virtual void        end() override {}
            virtual void     render() override { if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
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
                            int32_t _lim_max = 100, float _speed = 1.f, ActionItf *_action = nullptr, ImGuiSliderFlags _flags = 0)
                            : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, format{_format},
                              action{_action}, flags{_flags}, limits{_lim_min, _lim_max}, speed{_speed} {
                SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetDragInt32 Constructor", "expected 'val_count' values are [1,4]\n");
                memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
            }
            WidgetDragInt32(const WidgetDragInt32 &_widget) : value{_widget.value}, action{_widget.action}, limits{_widget.limits},
                                                                val_count{_widget.val_count}, flags{_widget.flags} {
                safeDeleteArr(name);
                uint32_t __name_len = std::strlen(_widget.name)+1;
                name = new char[__name_len]{};
                memcpy(name, _widget.name, __name_len*sizeof(char));
            }
            virtual ~WidgetDragInt32() override { safeDeleteArr(name); }

            virtual bool begin() override {
                switch (val_count) {
                    case 1: return ImGui::DragInt (name, &value.x, speed, limits.x, limits.y, format, flags);
                    case 2: return ImGui::DragInt2(name, &value.x, speed, limits.x, limits.y, format, flags);
                    case 3: return ImGui::DragInt3(name, &value.x, speed, limits.x, limits.y, format, flags);
                    case 4: return ImGui::DragInt4(name, &value.x, speed, limits.x, limits.y, format, flags);
                    default: break;
                }
                return false;
            }
            virtual void        end() override {}
            virtual void     render() override { if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
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

            WidgetDragUint32(const std::string &_name, uint32_t _val_count, vec4u32 _default_value = {0u}, const char *_format = "%d",
                             uint32_t _lim_min = 0u, uint32_t _lim_max = 100u, float _speed = 1.f, ActionItf *_action = nullptr, ImGuiSliderFlags _flags = 0)
                            : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, format{_format},
                              action{_action}, flags{_flags}, limits{_lim_min, _lim_max}, speed{_speed} {
                SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetDragUint32 Constructor", "expected 'val_count' values are [1,4]\n");
                memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
            }
            WidgetDragUint32(const WidgetDragUint32 &_widget) : value{_widget.value}, action{_widget.action}, limits{_widget.limits},
                                                                val_count{_widget.val_count}, flags{_widget.flags} {
                safeDeleteArr(name);
                uint32_t __name_len = std::strlen(_widget.name)+1;
                name = new char[__name_len]{};
                memcpy(name, _widget.name, __name_len*sizeof(char));
            }
            virtual ~WidgetDragUint32() override { safeDeleteArr(name); }

            virtual bool begin() override {
                const char *__format = "%d";
                switch (val_count) {
                    case 1: return ImGui::DragScalar (name, ImGuiDataType_U32, &value.x,            speed, &limits.x, &limits.y, __format, flags);
                    case 2:
                    case 3:
                    case 4: return ImGui::DragScalarN(name, ImGuiDataType_U32, &value.x, val_count, speed, &limits.x, &limits.y, __format, flags);
                    default: break;
                }
                return false;
            }
            virtual void        end() override {}
            virtual void     render() override { if (action) action->action(); }
            virtual void renderFull() override { if (begin()) render(); }
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


        // Menu widgets

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


        // Window widgets

        struct WidgetWindowChild : WidgetItf {
            std::string name;
            vec2f       size;
            ImGuiChildFlags   child_flags;
            ImGuiWindowFlags window_flags;
            uint32_t is_began = false;
            std::vector<WidgetItf*> widgets;

            WidgetWindowChild(const std::string &_name, vec2f _size = {}, ImGuiChildFlags _child_flags = 0,
                              ImGuiWindowFlags _window_flags = 0) : name{_name}, size{_size}, child_flags{_child_flags},
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


// ImGUI Handler

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
                return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
            }
            bool isKeyboardInUse() {
                return ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
            }
        }; // ImGuiHandler END
    }; // ImGUI END
}; // Simple END

#endif