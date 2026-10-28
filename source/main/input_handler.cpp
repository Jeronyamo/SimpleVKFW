#include "main/input_handler.h"
#include "data/filetypes.h"

#include <cctype>
#include <chrono>
#include <ctime>
#include <fstream>
#include <sstream>

namespace Simple {
    namespace WindowInput {

    // InputBinding

        namespace {
            struct InputName {
                const char *name;
                int device;
                int code;
            };

            // Note: names are compared in lower case, without '_' and spaces
            const InputName input_names[] = {
                {"Space", INPUT_DEVICE_KEY, GLFW_KEY_SPACE}, {"Apostrophe", INPUT_DEVICE_KEY, GLFW_KEY_APOSTROPHE},
                {"Comma", INPUT_DEVICE_KEY, GLFW_KEY_COMMA}, {"Minus", INPUT_DEVICE_KEY, GLFW_KEY_MINUS},
                {"Period", INPUT_DEVICE_KEY, GLFW_KEY_PERIOD}, {"Slash", INPUT_DEVICE_KEY, GLFW_KEY_SLASH},
                {"Semicolon", INPUT_DEVICE_KEY, GLFW_KEY_SEMICOLON}, {"Equal", INPUT_DEVICE_KEY, GLFW_KEY_EQUAL},
                {"LeftBracket", INPUT_DEVICE_KEY, GLFW_KEY_LEFT_BRACKET}, {"Backslash", INPUT_DEVICE_KEY, GLFW_KEY_BACKSLASH},
                {"RightBracket", INPUT_DEVICE_KEY, GLFW_KEY_RIGHT_BRACKET}, {"GraveAccent", INPUT_DEVICE_KEY, GLFW_KEY_GRAVE_ACCENT},
                {"World1", INPUT_DEVICE_KEY, GLFW_KEY_WORLD_1}, {"World2", INPUT_DEVICE_KEY, GLFW_KEY_WORLD_2},
                {"Escape", INPUT_DEVICE_KEY, GLFW_KEY_ESCAPE}, {"Enter", INPUT_DEVICE_KEY, GLFW_KEY_ENTER},
                {"Tab", INPUT_DEVICE_KEY, GLFW_KEY_TAB}, {"Backspace", INPUT_DEVICE_KEY, GLFW_KEY_BACKSPACE},
                {"Insert", INPUT_DEVICE_KEY, GLFW_KEY_INSERT}, {"Delete", INPUT_DEVICE_KEY, GLFW_KEY_DELETE},
                {"Right", INPUT_DEVICE_KEY, GLFW_KEY_RIGHT}, {"Left", INPUT_DEVICE_KEY, GLFW_KEY_LEFT},
                {"Down", INPUT_DEVICE_KEY, GLFW_KEY_DOWN}, {"Up", INPUT_DEVICE_KEY, GLFW_KEY_UP},
                {"PageUp", INPUT_DEVICE_KEY, GLFW_KEY_PAGE_UP}, {"PageDown", INPUT_DEVICE_KEY, GLFW_KEY_PAGE_DOWN},
                {"Home", INPUT_DEVICE_KEY, GLFW_KEY_HOME}, {"End", INPUT_DEVICE_KEY, GLFW_KEY_END},
                {"CapsLock", INPUT_DEVICE_KEY, GLFW_KEY_CAPS_LOCK}, {"ScrollLock", INPUT_DEVICE_KEY, GLFW_KEY_SCROLL_LOCK},
                {"NumLock", INPUT_DEVICE_KEY, GLFW_KEY_NUM_LOCK}, {"PrintScreen", INPUT_DEVICE_KEY, GLFW_KEY_PRINT_SCREEN},
                {"Pause", INPUT_DEVICE_KEY, GLFW_KEY_PAUSE},
                {"KPDecimal", INPUT_DEVICE_KEY, GLFW_KEY_KP_DECIMAL}, {"KPDivide", INPUT_DEVICE_KEY, GLFW_KEY_KP_DIVIDE},
                {"KPMultiply", INPUT_DEVICE_KEY, GLFW_KEY_KP_MULTIPLY}, {"KPSubtract", INPUT_DEVICE_KEY, GLFW_KEY_KP_SUBTRACT},
                {"KPAdd", INPUT_DEVICE_KEY, GLFW_KEY_KP_ADD}, {"KPEnter", INPUT_DEVICE_KEY, GLFW_KEY_KP_ENTER},
                {"KPEqual", INPUT_DEVICE_KEY, GLFW_KEY_KP_EQUAL},
                {"LeftShift", INPUT_DEVICE_KEY, GLFW_KEY_LEFT_SHIFT}, {"LeftControl", INPUT_DEVICE_KEY, GLFW_KEY_LEFT_CONTROL},
                {"LeftAlt", INPUT_DEVICE_KEY, GLFW_KEY_LEFT_ALT}, {"LeftSuper", INPUT_DEVICE_KEY, GLFW_KEY_LEFT_SUPER},
                {"RightShift", INPUT_DEVICE_KEY, GLFW_KEY_RIGHT_SHIFT}, {"RightControl", INPUT_DEVICE_KEY, GLFW_KEY_RIGHT_CONTROL},
                {"RightAlt", INPUT_DEVICE_KEY, GLFW_KEY_RIGHT_ALT}, {"RightSuper", INPUT_DEVICE_KEY, GLFW_KEY_RIGHT_SUPER},
                {"Menu", INPUT_DEVICE_KEY, GLFW_KEY_MENU},
                {"MouseLeft", INPUT_DEVICE_MOUSE_BUTTON, GLFW_MOUSE_BUTTON_LEFT},
                {"MouseRight", INPUT_DEVICE_MOUSE_BUTTON, GLFW_MOUSE_BUTTON_RIGHT},
                {"MouseMiddle", INPUT_DEVICE_MOUSE_BUTTON, GLFW_MOUSE_BUTTON_MIDDLE},
            };

            const struct { const char *name; int mod; } mod_names[] = {
                {"shift", GLFW_MOD_SHIFT}, {"ctrl", GLFW_MOD_CONTROL}, {"control", GLFW_MOD_CONTROL},
                {"alt", GLFW_MOD_ALT}, {"super", GLFW_MOD_SUPER}, {"win", GLFW_MOD_SUPER}, {"cmd", GLFW_MOD_SUPER},
            };

            std::string normalizeName(const std::string &_text) {
                std::string __res;
                for (size_t i = 0u; i < _text.size(); ++i) {
                    const unsigned char c = (unsigned char)_text[i];
                    if (c == '_' || std::isspace(c)) continue;
                    __res += (char)std::tolower(c);
                }
                return __res;
            }

            // A part without modifiers: "w", "1", "f5", "kp7", "mouse4", or a name of the table
            bool parseInputName(const std::string &_name, int &_device, int &_code) {
                if (_name.size() == 1u && _name[0] >= 'a' && _name[0] <= 'z') { _device = INPUT_DEVICE_KEY; _code = GLFW_KEY_A + (_name[0] - 'a'); return true; }
                if (_name.size() == 1u && _name[0] >= '0' && _name[0] <= '9') { _device = INPUT_DEVICE_KEY; _code = GLFW_KEY_0 + (_name[0] - '0'); return true; }

                // "f1".."f25", "kp0".."kp9", "mouse1".."mouse8"
                const struct { const char *prefix; int device; int first; int min_num; int max_num; } numbered[] = {
                    {"f",     INPUT_DEVICE_KEY,          GLFW_KEY_F1,          1, 25},
                    {"kp",    INPUT_DEVICE_KEY,          GLFW_KEY_KP_0,        0,  9},
                    {"mouse", INPUT_DEVICE_MOUSE_BUTTON, GLFW_MOUSE_BUTTON_1,  1,  8},
                };
                for (size_t i = 0u; i < sizeof(numbered) / sizeof(numbered[0]); ++i) {
                    const std::string __prefix = numbered[i].prefix;
                    if (_name.size() <= __prefix.size() || _name.compare(0u, __prefix.size(), __prefix) != 0) continue;
                    const std::string __num = _name.substr(__prefix.size());
                    if (__num.size() > 2u || !std::isdigit((unsigned char)__num[0]) || (__num.size() == 2u && !std::isdigit((unsigned char)__num[1])))
                        continue;
                    const int __n = std::atoi(__num.c_str());
                    if (__n < numbered[i].min_num || __n > numbered[i].max_num) continue;
                    _device = numbered[i].device;
                    _code   = numbered[i].first + (__n - numbered[i].min_num);
                    return true;
                }

                for (size_t i = 0u; i < sizeof(input_names) / sizeof(input_names[0]); ++i)
                    if (_name == normalizeName(input_names[i].name)) { _device = input_names[i].device; _code = input_names[i].code; return true; }

                // Aliases
                if (_name == "esc")    { _device = INPUT_DEVICE_KEY;          _code = GLFW_KEY_ESCAPE;              return true; }
                if (_name == "return") { _device = INPUT_DEVICE_KEY;          _code = GLFW_KEY_ENTER;               return true; }
                if (_name == "lmb")    { _device = INPUT_DEVICE_MOUSE_BUTTON; _code = GLFW_MOUSE_BUTTON_LEFT;       return true; }
                if (_name == "rmb")    { _device = INPUT_DEVICE_MOUSE_BUTTON; _code = GLFW_MOUSE_BUTTON_RIGHT;      return true; }
                if (_name == "mmb")    { _device = INPUT_DEVICE_MOUSE_BUTTON; _code = GLFW_MOUSE_BUTTON_MIDDLE;     return true; }
                return false;
            }

            // Names for 'toString'
            std::string inputName(int _device, int _code) {
                if (_device == INPUT_DEVICE_MOUSE_BUTTON) {
                    if (_code == GLFW_MOUSE_BUTTON_LEFT)   return "MouseLeft";
                    if (_code == GLFW_MOUSE_BUTTON_RIGHT)  return "MouseRight";
                    if (_code == GLFW_MOUSE_BUTTON_MIDDLE) return "MouseMiddle";
                    return "Mouse" + std::to_string(_code - GLFW_MOUSE_BUTTON_1 + 1);
                }
                if (_code >= GLFW_KEY_A  && _code <= GLFW_KEY_Z)   return std::string(1, char('A' + (_code - GLFW_KEY_A)));
                if (_code >= GLFW_KEY_0  && _code <= GLFW_KEY_9)   return std::string(1, char('0' + (_code - GLFW_KEY_0)));
                if (_code >= GLFW_KEY_F1 && _code <= GLFW_KEY_F25) return "F" + std::to_string(_code - GLFW_KEY_F1 + 1);
                if (_code >= GLFW_KEY_KP_0 && _code <= GLFW_KEY_KP_9) return "KP" + std::to_string(_code - GLFW_KEY_KP_0);
                for (size_t i = 0u; i < sizeof(input_names) / sizeof(input_names[0]); ++i)
                    if (input_names[i].device == _device && input_names[i].code == _code)
                        return input_names[i].name;
                return "Unknown";
            }
        }; // anonymous namespace END

        InputBinding::InputBinding() {}

        InputBinding::InputBinding(int _device, int _code, int _mods) : device{_device}, code{_code}, mods{_mods & MODS_MASK} {}

        InputBinding InputBinding::Key(int _key, int _mods) { return InputBinding(INPUT_DEVICE_KEY, _key, _mods); }

        InputBinding InputBinding::MouseButton(int _button, int _mods) { return InputBinding(INPUT_DEVICE_MOUSE_BUTTON, _button, _mods); }

        bool InputBinding::parse(const std::string &_text, InputBinding &_binding) {
            InputBinding __res;
            bool __has_input = false;
            size_t __start = 0u;
            while (__start <= _text.size()) {
                size_t __end = _text.find('+', __start);
                if (__end == std::string::npos) __end = _text.size();
                const std::string __part = normalizeName(_text.substr(__start, __end - __start));
                __start = __end + 1u;
                if (__part.empty()) return false;

                bool __is_mod = false;
                for (size_t i = 0u; i < sizeof(mod_names) / sizeof(mod_names[0]); ++i)
                    if (__part == mod_names[i].name) { __res.mods |= mod_names[i].mod; __is_mod = true; }
                if (__is_mod) continue;

                // Note: exactly one key or button, the last part
                if (__has_input || !parseInputName(__part, __res.device, __res.code)) return false;
                __has_input = true;
                if (__end != _text.size()) return false;
            }
            if (!__has_input) return false;
            _binding = __res;
            return true;
        }

        std::string InputBinding::toString() const {
            std::string __res;
            if (mods & GLFW_MOD_CONTROL) __res += "Ctrl+";
            if (mods & GLFW_MOD_ALT)     __res += "Alt+";
            if (mods & GLFW_MOD_SHIFT)   __res += "Shift+";
            if (mods & GLFW_MOD_SUPER)   __res += "Super+";
            return __res + inputName(device, code);
        }

        bool InputBinding::isNone() const { return code == GLFW_KEY_UNKNOWN; }

        bool InputBinding::operator==(const InputBinding &_binding) const {
            return device == _binding.device && code == _binding.code && mods == _binding.mods;
        }

        int64_t InputBinding::code64() const { return (input64() << 8) | int64_t(mods & MODS_MASK); }

        int64_t InputBinding::input64() const { return (int64_t(device) << 32) | int64_t(uint32_t(code)); }


    // InputEvent

        bool InputEvent::isBroadcast() const { return type >= INPUT_EVENT_FBUFFER_SIZE; }

        InputBinding InputEvent::binding() const {
            return InputBinding(type == INPUT_EVENT_MOUSE_BUTTON ? INPUT_DEVICE_MOUSE_BUTTON : INPUT_DEVICE_KEY, code, mods);
        }


    // Actions

        namespace Actions {
            const char *const window_names[WINDOW_COUNT] = { "quit", "toggle_fullscreen", "toggle_true_fullscreen" };
            const char *const camera_names[CAMERA_COUNT] = { "move_forward", "move_back", "move_left", "move_right", "rotate" };
            const char *const screenshot_names[SCREENSHOT_COUNT] = { "screenshot_scene", "screenshot_frame", "screenshot_depth", "screenshot_stencil" };
        }; // Actions END


    // InputHandlerItf

        InputHandlerItf::~InputHandlerItf() {}

        void InputHandlerItf::declareActions(const char *const *_names, uint32_t _count) {
            actions.clear();
            for (uint32_t i = 0u; i < _count; ++i) addAction(_names[i]);
        }

        ActionId InputHandlerItf::addAction(const std::string &_name) {
            SVKFW_ASSERT(findAction(_name) == ACTION_NONE, std::invalid_argument, "WindowInput::InputHandlerItf :: addAction",
                            "the action '" + _name + "' is already declared");
            ActionState __action;
            __action.name = _name;
            actions.push_back(__action);
            return ActionId(actions.size() - 1u);
        }

        ActionId InputHandlerItf::findAction(const std::string &_name) const {
            for (size_t i = 0u; i < actions.size(); ++i)
                if (actions[i].name == _name) return ActionId(i);
            return ACTION_NONE;
        }

        void InputHandlerItf::bind(const InputBinding &_binding, ActionId _action) {
            SVKFW_ASSERT(_action < actions.size(), std::out_of_range, "WindowInput::InputHandlerItf :: bind", "unknown action");
            bindings[_binding.code64()] = _action;
        }

        void InputHandlerItf::unbind(const InputBinding &_binding) { bindings.erase(_binding.code64()); }

        void InputHandlerItf::clearBindings() {
            releaseAll();
            bindings.clear();
        }

        bool InputHandlerItf::loadInputMap(const std::string &_fpath) {
            std::ifstream __file(_fpath.c_str(), std::ios::binary);
            if (!__file) {
                fprintf(svkfwwarn, SVKFW_WRAPWARN("WindowInput::InputHandlerItf :: loadInputMap", "could not open '%s'\n"), _fpath.c_str());
                return false;
            }
            std::stringstream __text;
            __text << __file.rdbuf();
            const std::string __str = __text.str();

            File::ContentJSON __json;
            __json.readFromString(__str.c_str());

            // Note: objects keep their keys and values in turn
            const File::ContentJSON::JSONNode &__root = __json.root_node;
            const File::ContentJSON::JSONNode *__inputs = nullptr;
            if (__root.node_type == File::JSON_TYPE_OBJ)
                for (size_t i = 0u; i + 1u < __root.value_arr_obj.size(); i += 2u)
                    if (__root.value_arr_obj[i].node_type == File::JSON_TYPE_STR && __root.value_arr_obj[i].value_str == "inputs")
                        __inputs = &__root.value_arr_obj[i + 1u];
            if (__inputs == nullptr || __inputs->node_type != File::JSON_TYPE_ARR) {
                fprintf(svkfwwarn, SVKFW_WRAPWARN("WindowInput::InputHandlerItf :: loadInputMap", "'%s' has no \"inputs\" array\n"), _fpath.c_str());
                return false;
            }

            for (size_t n = 0u; n < __inputs->value_arr_obj.size(); ++n) {
                const File::ContentJSON::JSONNode &__input = __inputs->value_arr_obj[n];
                const std::string *__key = nullptr, *__action = nullptr;
                if (__input.node_type == File::JSON_TYPE_OBJ)
                    for (size_t i = 0u; i + 1u < __input.value_arr_obj.size(); i += 2u) {
                        const File::ContentJSON::JSONNode &__name  = __input.value_arr_obj[i];
                        const File::ContentJSON::JSONNode &__value = __input.value_arr_obj[i + 1u];
                        if (__name.node_type != File::JSON_TYPE_STR || __value.node_type != File::JSON_TYPE_STR) continue;
                        if (__name.value_str == "key")    __key    = &__value.value_str;
                        if (__name.value_str == "action") __action = &__value.value_str;
                    }
                if (__key == nullptr || __action == nullptr) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("WindowInput::InputHandlerItf :: loadInputMap", "'%s': input %u needs string \"key\" and \"action\"\n"), _fpath.c_str(), (unsigned) n);
                    continue;
                }
                InputBinding __binding;
                if (!InputBinding::parse(*__key, __binding)) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("WindowInput::InputHandlerItf :: loadInputMap", "'%s': unknown key '%s'\n"), _fpath.c_str(), __key->c_str());
                    continue;
                }
                const ActionId __id = findAction(*__action);
                if (__id == ACTION_NONE) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("WindowInput::InputHandlerItf :: loadInputMap", "'%s': unknown action '%s'\n"), _fpath.c_str(), __action->c_str());
                    continue;
                }
                bind(__binding, __id);
            }
            return true;
        }

        bool InputHandlerItf::isDown(ActionId _action) const { return _action < actions.size() && actions[_action].held > 0u; }

        bool InputHandlerItf::wasPressed(ActionId _action) const { return _action < actions.size() && actions[_action].presses > 0u; }

        bool InputHandlerItf::wasReleased(ActionId _action) const { return _action < actions.size() && actions[_action].releases > 0u; }

        float InputHandlerItf::axis(ActionId _positive, ActionId _negative) const {
            const bool __pos = isDown(_positive), __neg = isDown(_negative);
            if (__pos && __neg)
                return actions[_positive].press_order > actions[_negative].press_order ? 1.f : -1.f;
            return __pos ? 1.f : (__neg ? -1.f : 0.f);
        }

        void InputHandlerItf::releaseAll() {
            std::unordered_map<int64_t, ActionId> __held;
            __held.swap(held_input);
            for (std::unordered_map<int64_t, ActionId>::iterator it = __held.begin(); it != __held.end(); ++it) {
                ActionState &__action = actions[it->second];
                if (__action.held) --__action.held;
                ++__action.releases;
                onAction(it->second, GLFW_RELEASE);
            }
        }

        void InputHandlerItf::setEnabled(bool _enabled) {
            if (!_enabled) releaseAll();
            enabled = _enabled;
        }

        void InputHandlerItf::newFrame() {
            for (size_t i = 0u; i < actions.size(); ++i)
                actions[i].presses = actions[i].releases = 0u;
        }

        bool InputHandlerItf::onEvent(const InputEvent &_event) { return processBinding(_event); }

        void InputHandlerItf::onAction(ActionId _action, int _glfw_action) {}

        bool InputHandlerItf::processBinding(const InputEvent &_event) {
            if (_event.type != INPUT_EVENT_KEY && _event.type != INPUT_EVENT_MOUSE_BUTTON) return false;
            const InputBinding __binding = _event.binding();
            const int64_t __input = __binding.input64();

            // Repeats and releases go to the action of the press
            if (_event.action != GLFW_PRESS) {
                std::unordered_map<int64_t, ActionId>::iterator __held = held_input.find(__input);
                if (__held == held_input.end()) return false;
                const ActionId __id = __held->second;
                if (_event.action == GLFW_RELEASE) {
                    held_input.erase(__held);
                    if (actions[__id].held) --actions[__id].held;
                    ++actions[__id].releases;
                }
                onAction(__id, _event.action);
                return true;
            }

            std::unordered_map<int64_t, ActionId>::const_iterator __bound = bindings.find(__binding.code64());
            if (__bound == bindings.end() && __binding.mods != 0)
                __bound = bindings.find(InputBinding(__binding.device, __binding.code).code64());
            if (__bound == bindings.end()) return false;

            const ActionId __id = __bound->second;
            if (held_input.count(__input)) return true; // a press without a release (e.g. lost focus), already held
            held_input[__input] = __id;
            ActionState &__action = actions[__id];
            ++__action.held;
            ++__action.presses;
            __action.press_order = ++press_counter;
            onAction(__id, GLFW_PRESS);
            return true;
        }


    // ActionCallbacks

        ActionId ActionCallbacks::addAction(const std::string &_name, const std::function<void()> &_on_press) {
            const ActionId __id = InputHandlerItf::addAction(_name);
            callbacks.resize(actions.size());
            callbacks[__id] = _on_press;
            return __id;
        }

        void ActionCallbacks::onAction(ActionId _action, int _glfw_action) {
            if (_glfw_action == GLFW_PRESS && _action < callbacks.size() && callbacks[_action])
                callbacks[_action]();
        }


    // ImGuiLayer

        ImGuiLayer::ImGuiLayer(ImGUI::ImGuiHandler &_imgui) : imgui{_imgui} {}

        bool ImGuiLayer::onEvent(const InputEvent &_event) {
            GLFWwindow *__w = _event.window;
            switch (_event.type) {
                case INPUT_EVENT_KEY:
                    ImGui_ImplGlfw_KeyCallback(__w, _event.code, _event.scancode, _event.action, _event.mods);
                    return _event.action != GLFW_RELEASE && imgui.isKeyboardInUse();
                case INPUT_EVENT_MOUSE_BUTTON:
                    ImGui_ImplGlfw_MouseButtonCallback(__w, _event.code, _event.action, _event.mods);
                    return _event.action == GLFW_PRESS && imgui.isGUIHovered();
                case INPUT_EVENT_CURSOR:
                    ImGui_ImplGlfw_CursorPosCallback(__w, _event.x, _event.y);
                    return imgui.isGUIHovered();
                case INPUT_EVENT_SCROLL:
                    ImGui_ImplGlfw_ScrollCallback(__w, _event.dx, _event.dy);
                    return imgui.isGUIHovered();
                case INPUT_EVENT_CHAR:
                    ImGui_ImplGlfw_CharCallback(__w, _event.character);
                    return imgui.isKeyboardInUse();
                case INPUT_EVENT_FOCUS:
                    // Note: ImGui releases its keys and buttons on focus loss (GLFW itself sends releases of the pressed keys)
                    ImGui_ImplGlfw_WindowFocusCallback(__w, _event.flag ? GLFW_TRUE : GLFW_FALSE);
                    return false;
                case INPUT_EVENT_CURSOR_ENTER:
                    ImGui_ImplGlfw_CursorEnterCallback(__w, _event.flag ? GLFW_TRUE : GLFW_FALSE);
                    return false;
                default:
                    return false;
            }
        }


    // FrameInputHandlerGLFW

        FrameInputHandlerGLFW::FrameInputHandlerGLFW(GLFWwindow* _window, uint32_t _cback_types) : imgui_layer{imgui_handler} {
            layers.push_back(&imgui_layer);
            if (_window) setGLFWCallbacks(_window, _cback_types);
        }

        void FrameInputHandlerGLFW::setGLFWCallbacks(GLFWwindow* _window, uint32_t _cback_types) {
            if (_cback_types & InputTypeGLFW::INPUT_TYPE_GLFW_KEY)
                glfwSetKeyCallback(_window, callbackKey);
            if (_cback_types & InputTypeGLFW::INPUT_TYPE_GLFW_SCROLL)
                glfwSetScrollCallback(_window, callbackScroll);
            if (_cback_types & InputTypeGLFW::INPUT_TYPE_GLFW_CURSOR)
                glfwSetCursorPosCallback(_window, callbackCursor);
            if (_cback_types & InputTypeGLFW::INPUT_TYPE_GLFW_MOUSE_BUTTON)
                glfwSetMouseButtonCallback(_window, callbackMButton);
            if (_cback_types & InputTypeGLFW::INPUT_TYPE_GLFW_FBUFFER_SIZE)
                glfwSetFramebufferSizeCallback(_window, callbackFBufferSize);
            if (_cback_types & InputTypeGLFW::INPUT_TYPE_GLFW_CURSOR_ENTER)
                glfwSetCursorEnterCallback(_window, callbackCursorEnter);
            if (_cback_types & InputTypeGLFW::INPUT_TYPE_GLFW_CHAR)
                glfwSetCharCallback(_window, callbackChar);
            if (_cback_types & InputTypeGLFW::INPUT_TYPE_GLFW_MONITOR)
                glfwSetMonitorCallback(callbackMonitor);
            if (_cback_types & InputTypeGLFW::INPUT_TYPE_GLFW_FOCUS)
                glfwSetWindowFocusCallback(_window, callbackFocus);

            glfwSetWindowUserPointer(_window, this);
        }

        void FrameInputHandlerGLFW::pushLayer(InputHandlerItf *_layer) { layers.push_back(_layer); }

        void FrameInputHandlerGLFW::insertLayer(uint32_t _index, InputHandlerItf *_layer) {
            layers.insert(layers.begin() + (_index < layers.size() ? _index : layers.size()), _layer);
        }

        void FrameInputHandlerGLFW::removeLayer(InputHandlerItf *_layer) {
            for (size_t i = 0u; i < layers.size(); ++i)
                if (layers[i] == _layer) { layers.erase(layers.begin() + i); return; }
        }

        bool FrameInputHandlerGLFW::dispatch(const InputEvent &_event) {
            const bool __broadcast = _event.isBroadcast();
            for (size_t i = 0u; i < layers.size(); ++i) {
                if (!layers[i]->enabled) continue;
                if (layers[i]->onEvent(_event) && !__broadcast) return true;
            }
            return false;
        }

        double FrameInputHandlerGLFW::getFrameTime() const { return frame_time; }

        void FrameInputHandlerGLFW::resetFrameTime() {
            frame_time = 0.;
            frame_clock.tick();
        }

        void FrameInputHandlerGLFW::frameTick() {
            frame_time = frame_clock.tick();
            for (size_t i = 0u; i < layers.size(); ++i)
                layers[i]->newFrame();
        }


    // Callbacks

        namespace {
            FrameInputHandlerGLFW *dispatcherOf(GLFWwindow* _window) {
                return static_cast<FrameInputHandlerGLFW*>(glfwGetWindowUserPointer(_window));
            }

            InputEvent makeEvent(InputEventType _type, GLFWwindow* _window) {
                InputEvent __in_event;
                __in_event.type   = _type;
                __in_event.window = _window;
                return __in_event;
            }
        }; // anonymous namespace END

        void callbackKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods) {
            FrameInputHandlerGLFW *__dispatcher = dispatcherOf(_window);
            if (__dispatcher == nullptr) return;
            InputEvent __in_event = makeEvent(INPUT_EVENT_KEY, _window);
            __in_event.code     = _key;
            __in_event.scancode = _scancode;
            __in_event.action   = _action;
            __in_event.mods     = _mods;
            __dispatcher->dispatch(__in_event);
        }

        void callbackCursor(GLFWwindow* _window, double _pos_x, double _pos_y) {
            FrameInputHandlerGLFW *__dispatcher = dispatcherOf(_window);
            if (__dispatcher == nullptr) return;
            InputEvent __in_event = makeEvent(INPUT_EVENT_CURSOR, _window);
            __in_event.x  = _pos_x;
            __in_event.y  = _pos_y;
            __in_event.dx = __dispatcher->cursor_known ? _pos_x - __dispatcher->cursor_pos.x : 0.;
            __in_event.dy = __dispatcher->cursor_known ? _pos_y - __dispatcher->cursor_pos.y : 0.;
            __dispatcher->cursor_pos   = vec2d{_pos_x, _pos_y};
            __dispatcher->cursor_known = true;
            __dispatcher->dispatch(__in_event);
        }

        void callbackMButton(GLFWwindow* _window, int _button, int _action, int _mods) {
            FrameInputHandlerGLFW *__dispatcher = dispatcherOf(_window);
            if (__dispatcher == nullptr) return;
            InputEvent __in_event = makeEvent(INPUT_EVENT_MOUSE_BUTTON, _window);
            __in_event.code   = _button;
            __in_event.action = _action;
            __in_event.mods   = _mods;
            __dispatcher->dispatch(__in_event);
        }

        void callbackScroll(GLFWwindow* _window, double _offset_x, double _offset_y) {
            FrameInputHandlerGLFW *__dispatcher = dispatcherOf(_window);
            if (__dispatcher == nullptr) return;
            InputEvent __in_event = makeEvent(INPUT_EVENT_SCROLL, _window);
            __in_event.dx = _offset_x;
            __in_event.dy = _offset_y;
            __dispatcher->dispatch(__in_event);
        }

        void callbackFBufferSize(GLFWwindow* _window, int _width, int _height) {
            FrameInputHandlerGLFW *__dispatcher = dispatcherOf(_window);
            if (__dispatcher == nullptr) return;
            InputEvent __in_event = makeEvent(INPUT_EVENT_FBUFFER_SIZE, _window);
            __in_event.width  = _width;
            __in_event.height = _height;
            __dispatcher->dispatch(__in_event);
        }

        void callbackCursorEnter(GLFWwindow* _window, int _entered) {
            FrameInputHandlerGLFW *__dispatcher = dispatcherOf(_window);
            if (__dispatcher == nullptr) return;
            InputEvent __in_event = makeEvent(INPUT_EVENT_CURSOR_ENTER, _window);
            __in_event.flag = _entered == GLFW_TRUE;
            if (!__in_event.flag) __dispatcher->cursor_known = false; // the next position may be far away
            __dispatcher->dispatch(__in_event);
        }

        void callbackChar(GLFWwindow* _window, uint32_t _character) {
            FrameInputHandlerGLFW *__dispatcher = dispatcherOf(_window);
            if (__dispatcher == nullptr) return;
            InputEvent __in_event = makeEvent(INPUT_EVENT_CHAR, _window);
            __in_event.character = _character;
            __dispatcher->dispatch(__in_event);
        }

        void callbackMonitor(GLFWmonitor* _monitor, int _event) {
            ImGui_ImplGlfw_MonitorCallback(_monitor, _event);
            AvailableMonitors::monitorCallbackAllUpd(_monitor, _event); // this callback replaces the one set by AvailableMonitors
        }

        void callbackFocus(GLFWwindow* _window, int _focused) {
            FrameInputHandlerGLFW *__dispatcher = dispatcherOf(_window);
            if (__dispatcher == nullptr) return;
            InputEvent __in_event = makeEvent(INPUT_EVENT_FOCUS, _window);
            __in_event.flag = _focused == GLFW_TRUE;
            __dispatcher->dispatch(__in_event);
        }


    // DefaultHandler

        DefaultHandler::DefaultHandler(GLFWwindow *_window) : managed_window{_window} {
            declareActions(Actions::window_names, Actions::WINDOW_COUNT);
        }

        DefaultHandler::~DefaultHandler() {
            managed_window.window = nullptr; // Not handling this window here
        }

        void DefaultHandler::onAction(ActionId _action, int _glfw_action) {
            if (_glfw_action != GLFW_PRESS) return;
            switch (_action) {
                case Actions::WINDOW_QUIT:
                    glfwSetWindowShouldClose(managed_window.window, GLFW_TRUE);
                    break;
                case Actions::WINDOW_TOGGLE_FULLSCREEN:
                    if (managed_window.isFullScreen())
                        managed_window.toWindowed();
                    else if (true_fullscreen)
                        managed_window.toFullScreen();
                    else
                        managed_window.toWindowedFullScreen();
                    break;
                case Actions::WINDOW_TOGGLE_TRUE_FULLSCREEN:
                    true_fullscreen = !true_fullscreen;
                    break;
                default: break;
            }
        }


    // ScreenshotHandler

        ScreenshotHandler::ScreenshotHandler() {
            declareActions(Actions::screenshot_names, Actions::SCREENSHOT_COUNT);
        }

        void ScreenshotHandler::setTarget(ActionId _action, VKFW::Graph::FrameCapture::Kind _kind, VKFW::Graph::PassId _after,
                                          VKFW::Graph::ResourceId _res, const std::string &_name) {
            SVKFW_ASSERT(_action < Actions::SCREENSHOT_COUNT, std::out_of_range, "WindowInput::ScreenshotHandler :: setTarget", "unknown action");
            Target &__target = targets[_action];
            __target.kind  = _kind;
            __target.after = _after;
            __target.res   = _res;
            __target.set   = true;
            __target.name  = _name;
            if (__target.name.empty()) {
                const std::string __prefix = "screenshot_";
                __target.name = actions[_action].name;
                if (__target.name.compare(0u, __prefix.size(), __prefix) == 0) __target.name.erase(0u, __prefix.size());
            }
        }

        bool ScreenshotHandler::request(ActionId _action) {
            if (_action >= Actions::SCREENSHOT_COUNT || !targets[_action].set) return false;
            const Target &__target = targets[_action];

            const std::chrono::system_clock::time_point __now = std::chrono::system_clock::now();
            const time_t __now_sec = std::chrono::system_clock::to_time_t(__now);
            const long   __now_ms  = long(std::chrono::duration_cast<std::chrono::milliseconds>(__now.time_since_epoch()).count() % 1000);
            char __time_str[48];
            const size_t __time_len = strftime(__time_str, sizeof(__time_str), "%Y-%m-%d_%H-%M-%S", localtime(&__now_sec));
            snprintf(__time_str + __time_len, sizeof(__time_str) - __time_len, "-%03ld", __now_ms);

            if (!capture.request(__target.kind, __target.after, __target.res, directory + "/" + __time_str + "_" + __target.name + ".png")) {
                printf(SVKFW_WRAPINFO("WindowInput::ScreenshotHandler", "A screenshot is already being taken\n"));
                return false;
            }
            return true;
        }

        void ScreenshotHandler::record(VKFW::Graph::RenderGraph &_graph, uint32_t _frame) { capture.record(_graph, _frame); }

        bool ScreenshotHandler::save(uint32_t _frame) {
            if (!capture.save(_frame)) return false;
            printf(SVKFW_WRAPINFO("WindowInput::ScreenshotHandler", "Screenshot saved: %s\n"), capture.file_name.c_str());
            return true;
        }

        void ScreenshotHandler::destroy() {
            save(capture.recorded_frame);
            capture.destroy();
        }

        void ScreenshotHandler::onAction(ActionId _action, int _glfw_action) {
            if (_glfw_action == GLFW_PRESS) request(_action);
        }


    // Cameras

        CameraControlsBase::CameraControlsBase() {
            declareActions(Actions::camera_names, Actions::CAMERA_COUNT);
        }

        float CameraControlsBase::moveForward() const { return axis(Actions::CAMERA_MOVE_FORWARD, Actions::CAMERA_MOVE_BACK); }
        float CameraControlsBase::moveRight() const { return axis(Actions::CAMERA_MOVE_RIGHT, Actions::CAMERA_MOVE_LEFT); }

        CameraControls<ProjPerspective, ViewGeneral>::CameraControls(const ProjPerspective &_proj, const ViewGeneral &_view) : camera{_proj, _view} {}

        mat4 CameraControls<ProjPerspective, ViewGeneral>::getView() const { return camera.getView(); }
        mat4 CameraControls<ProjPerspective, ViewGeneral>::getProj() const { return camera.getProj(); }
        ProjPerspective& CameraControls<ProjPerspective, ViewGeneral>::getClassProj() { return camera.proj_class; }
        ViewGeneral& CameraControls<ProjPerspective, ViewGeneral>::getClassView() { return camera.view_class; }

        void CameraControls<ProjPerspective, ViewGeneral>::manualFrameUpdate(double _frame_time) {
            vec3 __front_vec = (camera.view_class.look_at - camera.view_class.cam_pos).normalize();
            vec3 __right_vec = Math::cross(__front_vec, camera.view_class.up_vect);

            const vec3 __offset = (moveForward() * __front_vec * _frame_time + moveRight() * __right_vec * _frame_time).cast<float>();
            camera.view_class.cam_pos += __offset;
            camera.view_class.look_at += __offset;
        }

        bool CameraControls<ProjPerspective, ViewGeneral>::onEvent(const InputEvent &_event) {
            switch (_event.type) {
                case INPUT_EVENT_SCROLL:
                    camera.proj_class.fovy = Math::clampCL(camera.proj_class.fovy - (float) _event.dy, 20.f, 45.f);
                    return true;
                case INPUT_EVENT_FBUFFER_SIZE:
                    if (_event.width > 0 && _event.height > 0) // 0x0 when minimized, the aspect ratio would be NaN
                        camera.proj_class.updateReso({ (float) _event.width, (float) _event.height });
                    return false;
                default:
                    return processBinding(_event);
            }
        }


        CameraControls<ProjPerspective, ViewCentered>::CameraControls(const ProjPerspective &_proj, const ViewCentered &_view) : camera{_proj, _view} {}

        mat4 CameraControls<ProjPerspective, ViewCentered>::getView() const { return camera.getView(); }
        mat4 CameraControls<ProjPerspective, ViewCentered>::getProj() const { return camera.getProj(); }
        ProjPerspective& CameraControls<ProjPerspective, ViewCentered>::getClassProj() { return camera.proj_class; }
        ViewCentered&    CameraControls<ProjPerspective, ViewCentered>::getClassView() { return camera.view_class; }

        void CameraControls<ProjPerspective, ViewCentered>::manualFrameUpdate(double _frame_time) {
            vec3f __front_vec = (camera.view_class.look_at - camera.view_class.calcCamPos()).normalize();
            vec3f __right_vec = Math::cross(camera.view_class.up_vect, __front_vec);

            camera.view_class.look_at += moveForward() * __front_vec * _frame_time;
            camera.view_class.look_at += moveRight()   * __right_vec * _frame_time;
        }

        bool CameraControls<ProjPerspective, ViewCentered>::onEvent(const InputEvent &_event) {
            switch (_event.type) {
                case INPUT_EVENT_CURSOR:
                    if (!isDown(Actions::CAMERA_ROTATE)) return false;
                    camera.view_class.yaw   += (float) _event.dx * rotate_speed;
                    camera.view_class.pitch += (float) _event.dy * rotate_speed;
                    return true;
                case INPUT_EVENT_SCROLL:
                    camera.proj_class.fovy = Math::clampCL(camera.proj_class.fovy - (float) _event.dy, 20.f, 45.f);
                    return true;
                case INPUT_EVENT_FBUFFER_SIZE:
                    if (_event.width > 0 && _event.height > 0) // 0x0 when minimized, the aspect ratio would be NaN
                        camera.proj_class.updateReso({ (float) _event.width, (float) _event.height });
                    return false;
                default:
                    return processBinding(_event);
            }
        }


        CameraControls<ProjPerspective, ViewPOV>::CameraControls(const ProjPerspective &_proj, const ViewPOV &_view) : camera{_proj, _view} {}

        mat4 CameraControls<ProjPerspective, ViewPOV>::getView() const { return camera.getView(); }
        mat4 CameraControls<ProjPerspective, ViewPOV>::getProj() const { return camera.getProj(); }
        ProjPerspective& CameraControls<ProjPerspective, ViewPOV>::getClassProj() { return camera.proj_class; }
        ViewPOV& CameraControls<ProjPerspective, ViewPOV>::getClassView() { return camera.view_class; }

        void CameraControls<ProjPerspective, ViewPOV>::manualFrameUpdate(double _frame_time) {
            vec3 __front_vec = (camera.view_class.calcLookAt() - camera.view_class.cam_pos).normalize();
            vec3 __right_vec = Math::cross(camera.view_class.up_vect, __front_vec);

            camera.view_class.cam_pos += moveForward() * __front_vec * _frame_time;
            camera.view_class.cam_pos += moveRight()   * __right_vec * _frame_time;
        }

        bool CameraControls<ProjPerspective, ViewPOV>::onEvent(const InputEvent &_event) {
            switch (_event.type) {
                case INPUT_EVENT_CURSOR:
                    if (!isDown(Actions::CAMERA_ROTATE)) return false;
                    camera.view_class.yaw   += (float) _event.dx * rotate_speed;
                    camera.view_class.pitch -= (float) _event.dy * rotate_speed;
                    return true;
                case INPUT_EVENT_SCROLL:
                    camera.proj_class.fovy = Math::clampCL(camera.proj_class.fovy - (float) _event.dy, 20.f, 45.f);
                    return true;
                case INPUT_EVENT_FBUFFER_SIZE:
                    if (_event.width > 0 && _event.height > 0) // 0x0 when minimized, the aspect ratio would be NaN
                        camera.proj_class.updateReso({ (float) _event.width, (float) _event.height });
                    return false;
                default:
                    return processBinding(_event);
            }
        }
    }; // WindowInput END
}; // Simple END
