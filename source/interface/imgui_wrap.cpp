#include "interface/imgui_wrap.h"

namespace Simple {
    namespace ImGUI {

        ActionItf::ActionItf() {}

        ActionItf::~ActionItf() {}

        WidgetItf::WidgetItf() {}

        WidgetItf::~WidgetItf() {}

        ImGuiWindowFlags WidgetItf::reqWindowFlags() { return 0; }

        WidgetText::WidgetText(const std::string &_text) : text{new char[_text.size()+1]} {
            memcpy(text, _text.data(), (_text.size()+1)*sizeof(std::string::value_type));
        }

        WidgetText::WidgetText(const WidgetText &_widget) {
            if (_widget.text != nullptr) {
                size_t __text_len = std::strlen(_widget.text)+1;
                text = new char[__text_len];
                memcpy(text, _widget.text, __text_len*sizeof(char));
            }
        }

        WidgetText::~WidgetText() { safeDeleteArr(text); }

        bool      WidgetText::begin() { return true; }

        void        WidgetText::end() {}

        void     WidgetText::render() { ImGui::TextUnformatted(text); }

        void WidgetText::renderFull() { render(); }

        WidgetButton::WidgetButton(const std::string &_name, ActionItf *_action, vec2f _size) : name{new char[_name.size()+1]}, action{_action}, size{_size} {
            memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
        }

        WidgetButton::WidgetButton(const WidgetButton &_widget) : action{_widget.action}, size{_widget.size} {
            safeDeleteArr(name);
            uint32_t __name_len = std::strlen(_widget.name)+1;
            name = new char[__name_len]{};
            memcpy(name, _widget.name, __name_len*sizeof(char));
        }

        WidgetButton::~WidgetButton() { safeDeleteArr(name); }

        bool      WidgetButton::begin() { return ImGui::Button(name, {size.x, size.y}); }

        void        WidgetButton::end() {}

        void     WidgetButton::render() { if (action) action->action(); }

        void WidgetButton::renderFull() { if (begin()) render(); }

        WidgetSmallButton::WidgetSmallButton(const std::string &_name, ActionItf *_action) : name{new char[_name.size()+1]}, action{_action} {
            memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
        }

        WidgetSmallButton::WidgetSmallButton(const WidgetSmallButton &_widget) : action{_widget.action} {
            safeDeleteArr(name);
            uint32_t __name_len = std::strlen(_widget.name)+1;
            name = new char[__name_len]{};
            memcpy(name, _widget.name, __name_len*sizeof(char));
        }

        WidgetSmallButton::~WidgetSmallButton() { safeDeleteArr(name); }

        bool      WidgetSmallButton::begin() { return ImGui::SmallButton(name); }

        void        WidgetSmallButton::end() {}

        void     WidgetSmallButton::render() { if (action) action->action(); }

        void WidgetSmallButton::renderFull() { if (begin()) render(); }

        WidgetRadioButton::WidgetRadioButton(const std::string &_name, ActionItf *_action, bool _active) : name{new char[_name.size()+1]}, action{_action}, active{_active} {
            memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
        }

        WidgetRadioButton::WidgetRadioButton(const WidgetRadioButton &_widget) : action{_widget.action}, active{_widget.active} {
            safeDeleteArr(name);
            uint32_t __name_len = std::strlen(_widget.name)+1;
            name = new char[__name_len]{};
            memcpy(name, _widget.name, __name_len*sizeof(char));
        }

        WidgetRadioButton::~WidgetRadioButton() { safeDeleteArr(name); }

        bool      WidgetRadioButton::begin() { return ImGui::RadioButton(name, active); }

        void        WidgetRadioButton::end() {}

        void     WidgetRadioButton::render() { active = !active; printf("Radio button is %d\n", active);   if (action) action->action(); }

        void WidgetRadioButton::renderFull() { if (begin()) render(); }

        WidgetCheckbox::WidgetCheckbox(const std::string &_name, bool _default_value, ActionItf *_action)
                        : name{new char[_name.size()+1]}, value{_default_value}, action{_action} {
            memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
        }

        WidgetCheckbox::WidgetCheckbox(const WidgetCheckbox &_widget) : value{_widget.value}, action{_widget.action} {
            safeDeleteArr(name);
            uint32_t __name_len = std::strlen(_widget.name)+1;
            name = new char[__name_len]{};
            memcpy(name, _widget.name, __name_len*sizeof(char));
        }

        WidgetCheckbox::~WidgetCheckbox() { safeDeleteArr(name); }

        bool      WidgetCheckbox::begin() { return ImGui::Checkbox(name, &value); }

        void        WidgetCheckbox::end() {}

        void     WidgetCheckbox::render() { if (action) action->action(); }

        void WidgetCheckbox::renderFull() { if (begin()) render(); }

        WidgetInputText::WidgetInputText(const std::string &_name, uint32_t _buf_size, ActionItf *_action)
                        : name_size{(uint32_t)_name.size()+1}, buf_size{_buf_size+1}, action{_action} {
            name    = new char[name_size]{};
            buf_str = new char[ buf_size]{};
            memcpy(name, _name.data(), name_size * sizeof(std::string::value_type));
        }

        WidgetInputText::WidgetInputText(const WidgetInputText &_widget) : name_size{_widget.name_size}, buf_size{_widget.buf_size},
                                                          action{_widget.action} {
            safeDeleteArr(name);
            safeDeleteArr(buf_str);
            name    = new char[name_size]{};
            buf_str = new char[ buf_size]{};
            memcpy(name, _widget.name, name_size * sizeof(char));
            memcpy(buf_str, _widget.buf_str, buf_size * sizeof(char));
        }

        WidgetInputText::~WidgetInputText() {
            safeDeleteArr(name);
            safeDeleteArr(buf_str);
            name_size = 0u;
            buf_size  = 0u;
        }

        bool      WidgetInputText::begin() { return ImGui::InputText(name, buf_str, buf_size); }

        void        WidgetInputText::end() {}

        void     WidgetInputText::render() { if (action) action->action(); }

        void WidgetInputText::renderFull() { if (begin()) render(); }

        WidgetInputFloat::WidgetInputFloat(const std::string &_name, uint32_t _val_count, vec4f _default_value, const char *_format,
                         ActionItf *_action, ImGuiInputTextFlags _flags, float _step, float _step_fast)
                        : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, action{_action},
                          flags{_flags}, step{_step}, step_fast{_step_fast} {
            SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetInputFloat Constructor", "expected 'val_count' values are [1,4]\n");
            memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
        }

        WidgetInputFloat::WidgetInputFloat(const WidgetInputFloat &_widget) : format{_widget.format}, action{_widget.action}, value{_widget.value}, step{_widget.step},
                                                            step_fast{_widget.step_fast}, val_count{_widget.val_count}, flags{_widget.flags} {
            safeDeleteArr(name);
            uint32_t __name_len = std::strlen(_widget.name)+1;
            name = new char[__name_len]{};
            memcpy(name, _widget.name, __name_len*sizeof(char));
        }

        WidgetInputFloat::~WidgetInputFloat() { safeDeleteArr(name); }

        bool WidgetInputFloat::begin() {
            switch (val_count) {
                case 1: return ImGui::InputFloat (name, &value.x, step, step_fast, format, flags);
                case 2: return ImGui::InputFloat2(name, &value.x, format, flags);
                case 3: return ImGui::InputFloat3(name, &value.x, format, flags);
                case 4: return ImGui::InputFloat4(name, &value.x, format, flags);
                default: break;
            }
            return false;
        }

        void        WidgetInputFloat::end() {}

        void     WidgetInputFloat::render() { if (action) action->action(); }

        void WidgetInputFloat::renderFull() { if (begin()) render(); }

        WidgetInputInt32::WidgetInputInt32(const std::string &_name, uint32_t _val_count, vec4i32 _default_value, ActionItf *_action,
                         ImGuiInputTextFlags _flags, int32_t _step, int32_t _step_fast)
                        : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, action{_action},
                          flags{_flags}, step{_step}, step_fast{_step_fast} {
            SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetInputInt32 Constructor", "expected 'val_count' values are [1,4]\n");
            memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
        }

        WidgetInputInt32::WidgetInputInt32(const WidgetInputInt32 &_widget) : value{_widget.value}, action{_widget.action}, step{_widget.step},
                                                            step_fast{_widget.step_fast}, val_count{_widget.val_count}, flags{_widget.flags} {
            safeDeleteArr(name);
            uint32_t __name_len = std::strlen(_widget.name)+1;
            name = new char[__name_len]{};
            memcpy(name, _widget.name, __name_len*sizeof(char));
        }

        WidgetInputInt32::~WidgetInputInt32() { safeDeleteArr(name); }

        bool WidgetInputInt32::begin() {
            switch (val_count) {
                case 1: return ImGui::InputInt (name, &value.x, step, step_fast, flags);
                case 2: return ImGui::InputInt2(name, &value.x, flags);
                case 3: return ImGui::InputInt3(name, &value.x, flags);
                case 4: return ImGui::InputInt4(name, &value.x, flags);
                default: break;
            }
            return false;
        }

        void        WidgetInputInt32::end() {}

        void     WidgetInputInt32::render() { if (action) action->action(); }

        void WidgetInputInt32::renderFull() { if (begin()) render(); }

        WidgetInputUint32::WidgetInputUint32(const std::string &_name, uint32_t _val_count, vec4u32 _default_value, const char *_format,
                          ActionItf *_action, ImGuiInputTextFlags _flags, uint32_t _step, uint32_t _step_fast)
                        : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, format{_format},
                          action{_action}, flags{_flags}, step{_step}, step_fast{_step_fast} {
            SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetInputUint32 Constructor", "expected 'val_count' values are [1,4]\n");
            memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
        }

        WidgetInputUint32::WidgetInputUint32(const WidgetInputUint32 &_widget) : format{_widget.format}, action{_widget.action}, value{_widget.value}, step{_widget.step},
                                                              step_fast{_widget.step_fast}, val_count{_widget.val_count}, flags{_widget.flags} {
            safeDeleteArr(name);
            uint32_t __name_len = std::strlen(_widget.name)+1;
            name = new char[__name_len]{};
            memcpy(name, _widget.name, __name_len*sizeof(char));
        }

        WidgetInputUint32::~WidgetInputUint32() { safeDeleteArr(name); }

        bool WidgetInputUint32::begin() {
            switch (val_count) {
                case 1: return ImGui::InputScalar (name, ImGuiDataType_U32, &value.x,            &step, &step_fast, format, flags);
                case 2:
                case 3:
                case 4: return ImGui::InputScalarN(name, ImGuiDataType_U32, &value.x, val_count, &step, &step_fast, format, flags);
                default: break;
            }
            return false;
        }

        void        WidgetInputUint32::end() {}

        void     WidgetInputUint32::render() { if (action) action->action(); }

        void WidgetInputUint32::renderFull() { if (begin()) render(); }

        WidgetSliderFloat::WidgetSliderFloat(const std::string &_name, uint32_t _val_count, vec4f _default_value, float _lim_min,
                          float _lim_max, const char *_format, ActionItf *_action, ImGuiSliderFlags _flags)
                        : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, format{_format},
                          action{_action}, flags{_flags}, limits{_lim_min, _lim_max} {
            SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetSliderFloat Constructor", "expected 'val_count' values are [1,4]\n");
            memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
        }

        WidgetSliderFloat::WidgetSliderFloat(const WidgetSliderFloat &_widget) : format{_widget.format}, action{_widget.action}, value{_widget.value}, limits{_widget.limits},
                                                            val_count{_widget.val_count}, flags{_widget.flags} {
            safeDeleteArr(name);
            uint32_t __name_len = std::strlen(_widget.name)+1;
            name = new char[__name_len]{};
            memcpy(name, _widget.name, __name_len*sizeof(char));
        }

        WidgetSliderFloat::~WidgetSliderFloat() { safeDeleteArr(name); }

        bool WidgetSliderFloat::begin() {
            switch (val_count) {
                case 1: return ImGui::SliderFloat (name, &value.x, limits.x, limits.y, format, flags);
                case 2: return ImGui::SliderFloat2(name, &value.x, limits.x, limits.y, format, flags);
                case 3: return ImGui::SliderFloat3(name, &value.x, limits.x, limits.y, format, flags);
                case 4: return ImGui::SliderFloat4(name, &value.x, limits.x, limits.y, format, flags);
                default: break;
            }
            return false;
        }

        void        WidgetSliderFloat::end() {}

        void     WidgetSliderFloat::render() { if (action) action->action(); }

        void WidgetSliderFloat::renderFull() { if (begin()) render(); }

        WidgetSliderInt32::WidgetSliderInt32(const std::string &_name, uint32_t _val_count, vec4i32 _default_value, int32_t _lim_min,
                          int32_t _lim_max, const char *_format, ActionItf *_action, ImGuiSliderFlags _flags)
                        : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, format{_format},
                          action{_action}, flags{_flags}, limits{_lim_min, _lim_max} {
            SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetSliderInt32 Constructor", "expected 'val_count' values are [1,4]\n");
            memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
        }

        WidgetSliderInt32::WidgetSliderInt32(const WidgetSliderInt32 &_widget) : format{_widget.format}, action{_widget.action}, value{_widget.value}, limits{_widget.limits},
                                                            val_count{_widget.val_count}, flags{_widget.flags} {
            safeDeleteArr(name);
            uint32_t __name_len = std::strlen(_widget.name)+1;
            name = new char[__name_len]{};
            memcpy(name, _widget.name, __name_len*sizeof(char));
        }

        WidgetSliderInt32::~WidgetSliderInt32() { safeDeleteArr(name); }

        bool WidgetSliderInt32::begin() {
            switch (val_count) {
                case 1: return ImGui::SliderInt (name, &value.x, limits.x, limits.y, format, flags);
                case 2: return ImGui::SliderInt2(name, &value.x, limits.x, limits.y, format, flags);
                case 3: return ImGui::SliderInt3(name, &value.x, limits.x, limits.y, format, flags);
                case 4: return ImGui::SliderInt4(name, &value.x, limits.x, limits.y, format, flags);
                default: break;
            }
            return false;
        }

        void        WidgetSliderInt32::end() {}

        void     WidgetSliderInt32::render() { if (action) action->action(); }

        void WidgetSliderInt32::renderFull() { if (begin()) render(); }

        WidgetSliderUint32::WidgetSliderUint32(const std::string &_name, uint32_t _val_count, vec4u32 _default_value, uint32_t _lim_min,
                           uint32_t _lim_max, const char *_format, ActionItf *_action, ImGuiSliderFlags _flags)
                        : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, format{_format}, action{_action}, flags{_flags}, limits{_lim_min, _lim_max} {
            SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetSliderUint32 Constructor", "expected 'val_count' values are [1,4]\n");
            memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
        }

        WidgetSliderUint32::WidgetSliderUint32(const WidgetSliderUint32 &_widget) : format{_widget.format}, action{_widget.action}, value{_widget.value}, limits{_widget.limits},
                                                            val_count{_widget.val_count}, flags{_widget.flags} {
            safeDeleteArr(name);
            uint32_t __name_len = std::strlen(_widget.name)+1;
            name = new char[__name_len]{};
            memcpy(name, _widget.name, __name_len*sizeof(char));
        }

        WidgetSliderUint32::~WidgetSliderUint32() { safeDeleteArr(name); }

        bool WidgetSliderUint32::begin() {
            switch (val_count) {
                case 1: return ImGui::SliderScalar (name, ImGuiDataType_U32, &value.x,            &limits.x, &limits.y, format, flags);
                case 2:
                case 3:
                case 4: return ImGui::SliderScalarN(name, ImGuiDataType_U32, &value.x, val_count, &limits.x, &limits.y, format, flags);
                default: break;
            }
            return false;
        }

        void        WidgetSliderUint32::end() {}

        void     WidgetSliderUint32::render() { if (action) action->action(); }

        void WidgetSliderUint32::renderFull() { if (begin()) render(); }

        WidgetDragFloat::WidgetDragFloat(const std::string &_name, uint32_t _val_count, vec4f _default_value, float _lim_min, float _lim_max,
                        float _speed, const char *_format, ActionItf *_action, ImGuiSliderFlags _flags)
                        : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, format{_format}, action{_action}, flags{_flags}, limits{_lim_min, _lim_max}, speed{_speed} {
            SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetDragFloat Constructor", "expected 'val_count' values are [1,4]\n");
            memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
        }

        WidgetDragFloat::WidgetDragFloat(const WidgetDragFloat &_widget) : format{_widget.format}, action{_widget.action}, value{_widget.value}, speed{_widget.speed}, limits{_widget.limits},
                                                          val_count{_widget.val_count}, flags{_widget.flags} {
            safeDeleteArr(name);
            uint32_t __name_len = std::strlen(_widget.name)+1;
            name = new char[__name_len]{};
            memcpy(name, _widget.name, __name_len*sizeof(char));
        }

        WidgetDragFloat::~WidgetDragFloat() { safeDeleteArr(name); }

        bool WidgetDragFloat::begin() {
            switch (val_count) {
                case 1: return ImGui::DragFloat (name, &value.x, speed, limits.x, limits.y, format, flags);
                case 2: return ImGui::DragFloat2(name, &value.x, speed, limits.x, limits.y, format, flags);
                case 3: return ImGui::DragFloat3(name, &value.x, speed, limits.x, limits.y, format, flags);
                case 4: return ImGui::DragFloat4(name, &value.x, speed, limits.x, limits.y, format, flags);
                default: break;
            }
            return false;
        }

        void        WidgetDragFloat::end() {}

        void     WidgetDragFloat::render() { if (action) action->action(); }

        void WidgetDragFloat::renderFull() { if (begin()) render(); }

        WidgetDragInt32::WidgetDragInt32(const std::string &_name, uint32_t _val_count, vec4i32 _default_value, const char *_format, int32_t _lim_min,
                        int32_t _lim_max, float _speed, ActionItf *_action, ImGuiSliderFlags _flags)
                        : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, format{_format},
                          action{_action}, flags{_flags}, limits{_lim_min, _lim_max}, speed{_speed} {
            SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetDragInt32 Constructor", "expected 'val_count' values are [1,4]\n");
            memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
        }

        WidgetDragInt32::WidgetDragInt32(const WidgetDragInt32 &_widget) : format{_widget.format}, action{_widget.action}, value{_widget.value}, speed{_widget.speed}, limits{_widget.limits},
                                                            val_count{_widget.val_count}, flags{_widget.flags} {
            safeDeleteArr(name);
            uint32_t __name_len = std::strlen(_widget.name)+1;
            name = new char[__name_len]{};
            memcpy(name, _widget.name, __name_len*sizeof(char));
        }

        WidgetDragInt32::~WidgetDragInt32() { safeDeleteArr(name); }

        bool WidgetDragInt32::begin() {
            switch (val_count) {
                case 1: return ImGui::DragInt (name, &value.x, speed, limits.x, limits.y, format, flags);
                case 2: return ImGui::DragInt2(name, &value.x, speed, limits.x, limits.y, format, flags);
                case 3: return ImGui::DragInt3(name, &value.x, speed, limits.x, limits.y, format, flags);
                case 4: return ImGui::DragInt4(name, &value.x, speed, limits.x, limits.y, format, flags);
                default: break;
            }
            return false;
        }

        void        WidgetDragInt32::end() {}

        void     WidgetDragInt32::render() { if (action) action->action(); }

        void WidgetDragInt32::renderFull() { if (begin()) render(); }

        WidgetDragUint32::WidgetDragUint32(const std::string &_name, uint32_t _val_count, vec4u32 _default_value, const char *_format,
                         uint32_t _lim_min, uint32_t _lim_max, float _speed, ActionItf *_action, ImGuiSliderFlags _flags)
                        : name{new char[_name.size()+1]}, val_count{_val_count}, value{_default_value}, format{_format},
                          action{_action}, flags{_flags}, limits{_lim_min, _lim_max}, speed{_speed} {
            SVKFW_WASSERT(val_count > 0 && val_count < 5, "ImGUI:: WidgetDragUint32 Constructor", "expected 'val_count' values are [1,4]\n");
            memcpy(name, _name.data(), (_name.size()+1)*sizeof(std::string::value_type));
        }

        WidgetDragUint32::WidgetDragUint32(const WidgetDragUint32 &_widget) : format{_widget.format}, action{_widget.action}, value{_widget.value}, speed{_widget.speed}, limits{_widget.limits},
                                                            val_count{_widget.val_count}, flags{_widget.flags} {
            safeDeleteArr(name);
            uint32_t __name_len = std::strlen(_widget.name)+1;
            name = new char[__name_len]{};
            memcpy(name, _widget.name, __name_len*sizeof(char));
        }

        WidgetDragUint32::~WidgetDragUint32() { safeDeleteArr(name); }

        bool WidgetDragUint32::begin() {
            switch (val_count) {
                case 1: return ImGui::DragScalar (name, ImGuiDataType_U32, &value.x,            speed, &limits.x, &limits.y, format, flags);
                case 2:
                case 3:
                case 4: return ImGui::DragScalarN(name, ImGuiDataType_U32, &value.x, val_count, speed, &limits.x, &limits.y, format, flags);
                default: break;
            }
            return false;
        }

        void        WidgetDragUint32::end() {}

        void     WidgetDragUint32::render() { if (action) action->action(); }

        void WidgetDragUint32::renderFull() { if (begin()) render(); }

        WidgetMenuItem::WidgetMenuItem(const std::string &_name, const std::string &_shortcut) {
            SVKFW_ASSERT(    _name.size() < 256, std::invalid_argument, "WidgetMenuItem Constructor",     "'name' length >= 256");
            SVKFW_ASSERT(_shortcut.size() < 256, std::invalid_argument, "WidgetMenuItem Constructor", "'shortcut' length >= 256");
            name_len     = (uint8_t)     _name.size();
            shortcut_len = (uint8_t) _shortcut.size();

            name = new char[name_len + shortcut_len + 2];
            shortcut = name + name_len + 1;
            memcpy(    name,     _name.data(), (    name_len+1)*sizeof(std::string::value_type));
            memcpy(shortcut, _shortcut.data(), (shortcut_len+1)*sizeof(std::string::value_type));
        }

        WidgetMenuItem::WidgetMenuItem(const WidgetMenuItem &_menu_item) : is_selected{_menu_item.is_selected}, is_enabled{_menu_item.is_enabled},
                                                            name_len{_menu_item.name_len}, shortcut_len{_menu_item.shortcut_len} {
            name = new char[name_len + shortcut_len + 2];
            shortcut = name + name_len + 1;
            memcpy(    name, _menu_item.    name, (    name_len+1)*sizeof(char));
            memcpy(shortcut, _menu_item.shortcut, (shortcut_len+1)*sizeof(char));
        }

        WidgetMenuItem& WidgetMenuItem::operator=(const WidgetMenuItem &_menu_item) {
            if (this == &_menu_item) return *this;
            safeDeleteArr(name);
            name_len     = _menu_item.name_len;
            shortcut_len = _menu_item.shortcut_len;
            is_selected  = _menu_item.is_selected;
            is_enabled   = _menu_item.is_enabled;
            name = new char[name_len + shortcut_len + 2];
            shortcut = name + name_len + 1;
            memcpy(    name, _menu_item.    name, (    name_len+1)*sizeof(char));
            memcpy(shortcut, _menu_item.shortcut, (shortcut_len+1)*sizeof(char));
            return *this;
        }

        WidgetMenuItem::~WidgetMenuItem() { safeDeleteArr(name); }

        bool WidgetMenuItem::begin() {
            return ImGui::MenuItem(name, shortcut, &is_selected, is_enabled);
        }

        void WidgetMenuItem::end() {}

        void WidgetMenuItem::render() {}

        void WidgetMenuItem::renderFull() {}

        WidgetMenu::WidgetMenu(const std::string &_name, const std::vector<std::pair<WidgetMenuItem, ActionItf*>> &_item_action)
                        : name{_name}, item_action{_item_action} {}

        WidgetMenu::WidgetMenu(const WidgetMenu &_menu) : name{_menu.name}, item_action{_menu.item_action} {}

        WidgetMenu::~WidgetMenu() { end(); }

        bool  WidgetMenu::begin() {
            is_began = ImGui::BeginMenu(name.c_str());
            return is_began;
        }

        void    WidgetMenu::end() {
            if (is_began) ImGui::EndMenu();
            is_began = false;
        }

        void WidgetMenu::render() {
            for (auto &ia : item_action) {
                if (ia.first.begin() && ia.second != nullptr)
                    ia.second->action();
            }
        }

        void WidgetMenu::renderFull() {
            if (begin()) {
                render();
                end();
            }
        }

        WidgetMenuBar::WidgetMenuBar(const std::vector<WidgetMenu> &_menus, bool _is_main) : menus{_menus}, is_main_menu_bar{_is_main} {}

        WidgetMenuBar::WidgetMenuBar(const WidgetMenuBar &_menu_bar) : menus{_menu_bar.menus}, is_main_menu_bar{_menu_bar.is_main_menu_bar} {}

        bool  WidgetMenuBar::begin() {
            is_began = is_main_menu_bar ? ImGui::BeginMainMenuBar() : ImGui::BeginMenuBar();
            return is_began;
        }

        void    WidgetMenuBar::end() {
            if (is_began) is_main_menu_bar ? ImGui::EndMainMenuBar() : ImGui::EndMenuBar();
            is_began = false;
        }

        void WidgetMenuBar::render() {
            for (auto &menu : menus) menu.renderFull();
        }

        void WidgetMenuBar::renderFull() {
            if (begin()) {
                render();
                end();
            }
        }

        ImGuiWindowFlags WidgetMenuBar::reqWindowFlags() { return is_main_menu_bar ? 0 : ImGuiWindowFlags_MenuBar; }

        WidgetWindowChild::WidgetWindowChild(const std::string &_name, vec2f _size, ImGuiChildFlags _child_flags,
                          ImGuiWindowFlags _window_flags) : name{_name}, size{_size}, child_flags{_child_flags},
                                                                window_flags{_window_flags} {}

        bool  WidgetWindowChild::begin() {
            ImGui::BeginChild(name.c_str(), {size.x, size.y}, child_flags, window_flags);
            is_began = true;
            return is_began;
        }

        void    WidgetWindowChild::end() {
            if (is_began) ImGui::EndChild();
            is_began = false;
        }

        void WidgetWindowChild::render() {
            for (auto& w : widgets) w->renderFull();
        }

        void WidgetWindowChild::renderFull() {
            if (begin()) {
                render();
            }
            end();
        }

        ImGuiWindowFlags WidgetWindowChild::reqWindowFlags() {
            ImGuiWindowFlags __res = 0;
            for (auto& w : widgets) __res |= w->reqWindowFlags();
            return __res;
        }

        WidgetWindow::WidgetWindow(const std::string &_name, ImGuiWindowFlags _window_flags) : name{_name}, window_flags{_window_flags} {}

        bool  WidgetWindow::begin() {
            ImGui::Begin(name.c_str(), &is_window_open, window_flags);
            is_began = true;
            return is_began;
        }

        void    WidgetWindow::end() {
            if (is_began) ImGui::End();
            is_began = false;
        }

        void WidgetWindow::render() {
            for (auto& w : widgets) w->renderFull();
        }

        void WidgetWindow::renderFull() {
            if (begin()) {
                render();
            }
            end();
        }

        ImGuiWindowFlags WidgetWindow::reqWindowFlags() {
            ImGuiWindowFlags __res = 0;
            for (auto& w : widgets) __res |= w->reqWindowFlags();
            return __res;
        }

        ImGuiHandler::ImGuiHandler() {  createContext(); }

        ImGuiHandler::~ImGuiHandler() { destroyContext(); }

        void ImGuiHandler::createContext() {
            SVKFW_WASSERT(IMGUI_CHECKVERSION(), "ImGUI::ImGuiHandler Constructor", "ImGUI version mismatch\n");
            context = ImGui::CreateContext();
            imgui_io    = &ImGui::GetIO();
            imgui_style = &ImGui::GetStyle();

            setDefaultStyleSVKFW();
        }

        void ImGuiHandler::destroyContext() {
            if (context != nullptr) {
                imgui_io    = nullptr;
                imgui_style = nullptr;
                // Note: backends can't be shut down if they weren't initialized
                if (backends_initialized) {
                    ImGui_ImplVulkan_Shutdown();
                    ImGui_ImplGlfw_Shutdown();
                    backends_initialized = false;
                }
                ImGui::DestroyContext(context);
                context = nullptr;
            }
        }

        void ImGuiHandler::enableForSVKFWWindow(const Window &_w, bool _install_callbacks) {
            enableForGLFWwindow(_w.window, _install_callbacks);
        }

        void ImGuiHandler::enableForGLFWwindow(GLFWwindow *_w, bool _install_callbacks) {
            ImGui_ImplGlfw_InitForVulkan(_w, _install_callbacks);
            ImGui_ImplVulkan_Init(&imgui_info);
            backends_initialized = true;
        }

        void ImGuiHandler::setDefaultStyleSVKFW() {
            const Color::col3f __color_theme = Color::convert<Color::RGB, Color::HSV>(Color::col3f{166.f, 78.f, 20.f} / 255);

            if (imgui_style) {
                for (uint32_t col_id = 0u; col_id < ImGuiCol_COUNT; ++col_id) {
                    Color::col3f __curr_col{imgui_style->Colors[col_id].x, imgui_style->Colors[col_id].y, imgui_style->Colors[col_id].z};
                    __curr_col = Color::convert<Color::RGB, Color::HSV>(__curr_col);
                    __curr_col.x = __color_theme.x;
                    __curr_col = Color::convert<Color::HSV, Color::RGB>(__curr_col);
                    imgui_style->Colors[col_id] = ImVec4(__curr_col.x, __curr_col.y, __curr_col.z, imgui_style->Colors[col_id].w);
                }
            }
        }

        void ImGuiHandler::setFont(const std::string &_font_path, float _size_pixels) {
            if (imgui_io)
                imgui_io->Fonts->AddFontFromFileTTF(_font_path.c_str(), _size_pixels);
        }

        void ImGuiHandler::newFrame() {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        void ImGuiHandler::render() {
            for (auto& w : widgets) w->renderFull();
            ImGui::Render();
        }

        void ImGuiHandler::fillCommandBuffer(VkCommandBuffer _cmd_buffer) {
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _cmd_buffer);
        }

        void ImGuiHandler::initializeContext(const VKFW::VulkanContext &_vkfw, VKFW::QueueFamilyId _qfamily_id,
                               VKFW::QueueId _graphics_queue_id, uint32_t _image_count, VkRenderPass _render_pass) {
            imgui_info.Instance                    = _vkfw.cxt_core.instance.handle;
            imgui_info.PhysicalDevice              = _vkfw.cxt_core.phys_device.handle;
            imgui_info.Device                      = _vkfw.cxt_core.device.handle;
            imgui_info.QueueFamily                 = _vkfw.get(_qfamily_id);
            imgui_info.Queue                       = _vkfw.get(_graphics_queue_id);
            imgui_info.ApiVersion                  = _vkfw.cxt_core.apiVersion();
            // Note: ImGui draws inside the rendering of the swapchain image ('CommandRecorder::beginSwapchainRendering' or a graph pass);
            //       without dynamic rendering it uses '_render_pass' (e.g. 'Graph::RenderGraph::renderPass'), or the render pass of the core context
            if (_vkfw.cxt_core.profile.dynamic_rendering) {
                color_format = _vkfw.cxt_swapchain.format();
                imgui_info.UseDynamicRendering = true;
                imgui_info.PipelineInfoMain.PipelineRenderingCreateInfo = VkPipelineRenderingCreateInfoKHR{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR};
                imgui_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount    = 1u;
                imgui_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &color_format;
            }
            else
                imgui_info.PipelineInfoMain.RenderPass = _render_pass != VK_NULL_HANDLE ? _render_pass : _vkfw.cxt_core.render_pass.handle;
            // imgui_info.DescriptorPool = _vkfw.get(_ci_descr_pool);
            imgui_info.MinImageCount = getMinImageCount();
            imgui_info.ImageCount = std::max(_image_count, getMinImageCount());
            imgui_info.DescriptorPoolSize = getMinPoolSize();
        }

        void ImGuiHandler::setMinImageCount(uint32_t _new_count) {
            if (_new_count < 2) return;
            min_image_count = _new_count;
            ImGui_ImplVulkan_SetMinImageCount(_new_count);
        }

        uint32_t ImGuiHandler::getMinImageCount() {
            return min_image_count; // read ImGui_ImplVulkan_InitInfo::MinImageCount description
        }

        uint32_t ImGuiHandler::getMinPoolSize() {
            return descr_pool_size;
        }

        bool ImGuiHandler::isGUIHovered() {
            return imgui_io != nullptr && imgui_io->WantCaptureMouse;
        }

        bool ImGuiHandler::isKeyboardInUse() {
            return imgui_io != nullptr && imgui_io->WantCaptureKeyboard;
        }
    }; // ImGUI END
}; // Simple END
