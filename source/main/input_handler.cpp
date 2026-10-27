#include "main/input_handler.h"

namespace Simple {
    namespace WindowInput {

        InputHandlerItf::~InputHandlerItf() {}

        void InputHandlerItf::processEventKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods, double _frame_time) {}

        void InputHandlerItf::processEventCursor(GLFWwindow* _window, double _pos_x, double _pos_y, double _frame_time) {}

        void InputHandlerItf::processEventMButton(GLFWwindow* _window, int _button, int _action, int _mods, double _frame_time) {}

        void InputHandlerItf::processEventScroll(GLFWwindow* _window, double _offset_x, double _offset_y, double _frame_time) {}

        void InputHandlerItf::processEventFBufferSize(GLFWwindow* _window, int _width, int _height, double _frame_time) {}

        void InputHandlerItf::processEventCursorEnter(GLFWwindow* _window, int _entered) {}

        void InputHandlerItf::processEventChar(GLFWwindow* _window, uint32_t _character) {}

        void InputHandlerItf::processEventMonitor(GLFWmonitor* _monitor, int _event) {}

        FrameInputHandlerGLFW::FrameInputHandlerGLFW(GLFWwindow* _window, uint32_t _cback_types) {
            if (_window) setGLFWCallbacks(_window, _cback_types);
        }

        bool FrameInputHandlerGLFW::hasKeysHandler() const { return handler_bind_keys; }

        bool FrameInputHandlerGLFW::hasCursorHandler() const { return handler_bind_cursor; }

        bool FrameInputHandlerGLFW::hasScrollHandler() const { return handler_bind_scroll; }

        bool FrameInputHandlerGLFW::hasMButtonsHandler() const { return handler_bind_mbuttons; }

        bool FrameInputHandlerGLFW::hasFBufferSizeHandler() const { return handler_fbuffer_size; }

        bool FrameInputHandlerGLFW::hasKeySpecificHandler(uint16_t _key) const {
            return hasKeyScancodeSpecificHandler(glfwGetKeyScancode(_key));
        }

        bool FrameInputHandlerGLFW::hasKeyScancodeSpecificHandler(int _scancode) const {
            return handler_bind_key.find(_scancode) != handler_bind_key.end();
        }

        bool FrameInputHandlerGLFW::hasMButtonSpecificHandler(int _button) const {
            return handler_bind_mbutton.find(_button) != handler_bind_mbutton.end();
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

            glfwSetWindowUserPointer(_window, this);
        }

        InputHandlerItf* FrameInputHandlerGLFW::setAllKeysHandler(InputHandlerItf *_handler, bool _use_for_all) {
            use_specific_key_handlers = !_use_for_all;
            return setAllKeysHandler(_handler);
        }

        InputHandlerItf* FrameInputHandlerGLFW::setAllKeysHandler(InputHandlerItf *_handler) {
            InputHandlerItf* __prev_handler = handler_bind_keys;
            handler_bind_keys = _handler;
            return __prev_handler;
        }

        void FrameInputHandlerGLFW::setKeysHandler(InputHandlerItf *_handler, const std::vector<uint16_t> &_keys) {
            for (uint16_t key : (_keys.empty() ? _handler->handled_keys : _keys))
                handler_bind_key[glfwGetKeyScancode(key)] = _handler;
        }

        void FrameInputHandlerGLFW::setCursorHandler(InputHandlerItf *_handler) {
            handler_bind_cursor = _handler;
        }

        void FrameInputHandlerGLFW::setScrollHandler(InputHandlerItf *_handler) {
            handler_bind_scroll = _handler;
        }

        InputHandlerItf* FrameInputHandlerGLFW::setAllMButtonsHandler(InputHandlerItf *_handler, bool _use_for_all) {
            use_specific_mbutton_handlers = !_use_for_all;
            return setAllMButtonsHandler(_handler);
        }

        InputHandlerItf* FrameInputHandlerGLFW::setAllMButtonsHandler(InputHandlerItf *_handler) {
            InputHandlerItf* __prev_handler = handler_bind_mbuttons;
            handler_bind_mbuttons = _handler;
            return __prev_handler;
        }

        void FrameInputHandlerGLFW::setMButtonsHandler(InputHandlerItf *_handler, const std::vector<int> &_buttons) {
            for (int button : _buttons)
                handler_bind_mbutton[button] = _handler;
        }

        InputHandlerItf* FrameInputHandlerGLFW::setFBufferSizeHandler(InputHandlerItf *_handler) {
            InputHandlerItf* __prev_handler = handler_fbuffer_size;
            handler_fbuffer_size = _handler;
            return __prev_handler;
        }

        InputHandlerItf* FrameInputHandlerGLFW::getKeyHandler(uint16_t _key) {
            return getKeyScancodeHandler(glfwGetKeyScancode(_key));
        }

        InputHandlerItf* FrameInputHandlerGLFW::getKeyScancodeHandler(int _scancode) {
            return hasKeyScancodeSpecificHandler(_scancode) ? handler_bind_key[_scancode] : handler_bind_keys;
        }

        void FrameInputHandlerGLFW::eventKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods) {
            InputHandlerItf *__input_handler = handler_bind_keys;
            if (use_specific_key_handlers && hasKeyScancodeSpecificHandler(_scancode))
                __input_handler = handler_bind_key[_scancode];

            if (__input_handler != nullptr)
                __input_handler->processEventKey(_window, _key, _scancode, _action, _mods, frame_time);
            // else
            //     fprintf(svkfwwarn, "%s %s", SVKFW_WRAPWARN("WindowInput :: FrameInputHandlerGLFW :: eventKey", " no handler set for key:"), glfwGetKeyName(_key, _scancode));
        }

        void FrameInputHandlerGLFW::eventCursor(GLFWwindow* _window, double _pos_x, double _pos_y) {
            if (handler_bind_cursor != nullptr)
                handler_bind_cursor->processEventCursor(_window, _pos_x, _pos_y, frame_time);
            // else
            //     fprintf(svkfwwarn, SVKFW_WRAPWARN("WindowInput :: FrameInputHandlerGLFW :: eventCursor", " no handler set for cursor position"));
        }

        void FrameInputHandlerGLFW::eventScroll(GLFWwindow* _window, double _offset_x, double _offset_y) {
            if (handler_bind_scroll != nullptr)
                handler_bind_scroll->processEventScroll(_window, _offset_x, _offset_y, frame_time);
            // else
            //     fprintf(svkfwwarn, SVKFW_WRAPWARN("WindowInput :: FrameInputHandlerGLFW :: eventScroll", " no handler set for mouse scroll"));
        }

        void FrameInputHandlerGLFW::eventMButton(GLFWwindow* _window, int _button, int _action, int _mods) {
            InputHandlerItf *__input_handler = handler_bind_mbuttons;
            if (use_specific_mbutton_handlers && hasMButtonSpecificHandler(_button))
                __input_handler = handler_bind_mbutton[_button];

            if (__input_handler != nullptr)
                __input_handler->processEventMButton(_window, _button, _action, _mods, frame_time);
            // else
            //     fprintf(svkfwwarn, "%s %d", SVKFW_WRAPWARN("WindowInput :: FrameInputHandlerGLFW :: eventMButton", " no handler set for mouse button:"), _button);
        }

        void FrameInputHandlerGLFW::eventFBufferSize(GLFWwindow* _window, int _width, int _height) {
            if (handler_fbuffer_size != nullptr)
                handler_fbuffer_size->processEventFBufferSize(_window, _width, _height, frame_time);
        }

        double FrameInputHandlerGLFW::getFrameTime() const { return frame_time; }

        void FrameInputHandlerGLFW::resetFrameTime() {
            frame_time = 0.;
            frame_clock.tick();
        }

        void FrameInputHandlerGLFW::frameTick() {
            frame_time = frame_clock.tick();
        }

        void callbackKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods) {
            FrameInputHandlerGLFW *__input_handler = static_cast<FrameInputHandlerGLFW*>(glfwGetWindowUserPointer(_window));
            ImGui_ImplGlfw_KeyCallback(_window, _key, _scancode, _action, _mods);
            if (__input_handler->imgui_handler.isKeyboardInUse()) {
                if (_action == GLFW_PRESS) return;
            }
            __input_handler->eventKey(_window, _key, _scancode, _action, _mods);
        }

        void callbackCursor(GLFWwindow* _window, double _pos_x, double _pos_y) {
            FrameInputHandlerGLFW *__input_handler = static_cast<FrameInputHandlerGLFW*>(glfwGetWindowUserPointer(_window));
            ImGui_ImplGlfw_CursorPosCallback(_window, _pos_x, _pos_y);
            if (__input_handler->imgui_handler.isGUIHovered()) return;
            __input_handler->eventCursor(_window, _pos_x, _pos_y);
        }

        void callbackMButton(GLFWwindow* _window, int _button, int _action, int _mods) {
            FrameInputHandlerGLFW *__input_handler = static_cast<FrameInputHandlerGLFW*>(glfwGetWindowUserPointer(_window));
            ImGui_ImplGlfw_MouseButtonCallback(_window, _button, _action, _mods);
            if (__input_handler->imgui_handler.isGUIHovered()) {
                if (_action == GLFW_PRESS) return;
            }
            __input_handler->eventMButton(_window, _button, _action, _mods);
        }

        void callbackScroll(GLFWwindow* _window, double _offset_x, double _offset_y) {
            FrameInputHandlerGLFW *__input_handler = static_cast<FrameInputHandlerGLFW*>(glfwGetWindowUserPointer(_window));
            ImGui_ImplGlfw_ScrollCallback(_window, _offset_x, _offset_y);
            if (__input_handler->imgui_handler.isGUIHovered()) return;
            __input_handler->eventScroll(_window, _offset_x, _offset_y);
        }

        void callbackFBufferSize(GLFWwindow* _window, int _width, int _height) {
            FrameInputHandlerGLFW *__input_handler = static_cast<FrameInputHandlerGLFW*>(glfwGetWindowUserPointer(_window));
            __input_handler->eventFBufferSize(_window, _width, _height);
        }

        void callbackCursorEnter(GLFWwindow* _window, int _entered) {
            ImGui_ImplGlfw_CursorEnterCallback(_window, _entered);
        }

        void callbackChar(GLFWwindow* _window, uint32_t _character) {
            ImGui_ImplGlfw_CharCallback(_window, _character);
        }

        void callbackMonitor(GLFWmonitor* _monitor, int _event) {
            ImGui_ImplGlfw_MonitorCallback(_monitor, _event);
            AvailableMonitors::monitorCallbackAllUpd(_monitor, _event); // this callback replaces the one set by AvailableMonitors
        }

        DefaultHandler::DefaultHandler(GLFWwindow *_window) : managed_window{_window} {
            handled_keys = { GLFW_KEY_ESCAPE, GLFW_KEY_F1, GLFW_KEY_F5 };
        }

        DefaultHandler::~DefaultHandler() {
            managed_window.window = nullptr; // Not handling this window here
        }

        void DefaultHandler::processEventKey(GLFWwindow *_window, int _key, int _scancode, int _action, int _mods, double _frame_time) {
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_ESCAPE) && _action == GLFW_RELEASE) {
                glfwSetWindowShouldClose(managed_window.window, GLFW_TRUE);
            }
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_F5) && _action == GLFW_RELEASE) {
                if (fullscreen_mode)
                    managed_window.toWindowedWithParams(windowed_pos_size);
                else if (true_fullscreen)
                    windowed_pos_size = managed_window.toFullScreen();
                else
                    windowed_pos_size = managed_window.toWindowedFullScreen();

                fullscreen_mode = !fullscreen_mode;
            }
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_F1) && _action == GLFW_RELEASE) {
                true_fullscreen = !true_fullscreen;
            }
        }

        CameraControls<ProjPerspective, ViewGeneral>::CameraControls(const ProjPerspective &_proj, const ViewGeneral &_view) : camera{_proj, _view} {}

        mat4 CameraControls<ProjPerspective, ViewGeneral>::getView() const { return camera.getView(); }

        mat4 CameraControls<ProjPerspective, ViewGeneral>::getProj() const { return camera.getProj(); }

        ProjPerspective& CameraControls<ProjPerspective, ViewGeneral>::getClassProj() { return camera.proj_class; }

        ViewGeneral& CameraControls<ProjPerspective, ViewGeneral>::getClassView() { return camera.view_class; }

        void CameraControls<ProjPerspective, ViewGeneral>::processEventKey(GLFWwindow *_window, int _key, int _scancode, int _action, int _mods, double _frame_time) {
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_S)) {
                if (_action == GLFW_PRESS) {
                    cam_move_state.x = bool(cam_move_state.y) + 1;
                }
                if (_action == GLFW_RELEASE) {
                    cam_move_state.x = 0;
                    if (cam_move_state.y > 1) cam_move_state.y = 1;
                }
            }
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_W)) {
                if (_action == GLFW_PRESS) {
                    cam_move_state.y = bool(cam_move_state.x) + 1;
                }
                if (_action == GLFW_RELEASE) {
                    cam_move_state.y = 0;
                    if (cam_move_state.x > 1) cam_move_state.x = 1;
                }
            }
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_A)) {
                if (_action == GLFW_PRESS) {
                    cam_move_state.z = bool(cam_move_state.w) + 1;
                }
                if (_action == GLFW_RELEASE) {
                    cam_move_state.z = 0;
                    if (cam_move_state.w > 1) cam_move_state.w = 1;
                }
            }
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_D)) {
                if (_action == GLFW_PRESS) {
                    cam_move_state.w = bool(cam_move_state.z) + 1;
                }
                if (_action == GLFW_RELEASE) {
                    cam_move_state.w = 0;
                    if (cam_move_state.z > 1) cam_move_state.z = 1;
                }
            }
        }

        void CameraControls<ProjPerspective, ViewGeneral>::manualFrameUpdate(double _frame_time) {
            vec3 __front_vec = (camera.view_class.look_at - camera.view_class.cam_pos).normalize();
            vec3 __right_vec = Math::cross(__front_vec, camera.view_class.up_vect);

            camera.view_class.cam_pos += (cam_move_state.y - cam_move_state.x) * __front_vec * _frame_time;
            camera.view_class.look_at += (cam_move_state.y - cam_move_state.x) * __front_vec * _frame_time;
            camera.view_class.cam_pos += (cam_move_state.w - cam_move_state.z) * __right_vec * _frame_time;
            camera.view_class.look_at += (cam_move_state.w - cam_move_state.z) * __right_vec * _frame_time;
        }

        void CameraControls<ProjPerspective, ViewGeneral>::processEventScroll(GLFWwindow *_window, double _offset_x, double _offset_y, double _frame_time) {
            camera.proj_class.fovy = Math::clampCL(camera.proj_class.fovy - (float) _offset_y, 20.f, 45.f);
        }

        void CameraControls<ProjPerspective, ViewGeneral>::processEventFBufferSize(GLFWwindow *_window, int _width, int _height, double _frame_time) {
            camera.proj_class.updateReso({ (float) _width, (float) _height });
        }

        CameraControls<ProjPerspective, ViewCentered>::CameraControls(const ProjPerspective &_proj, const ViewCentered &_view) : camera{_proj, _view} {
            handled_keys = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D };
        }

        mat4 CameraControls<ProjPerspective, ViewCentered>::getView() const { return camera.getView(); }

        mat4 CameraControls<ProjPerspective, ViewCentered>::getProj() const { return camera.getProj(); }

        ProjPerspective& CameraControls<ProjPerspective, ViewCentered>::getClassProj() { return camera.proj_class; }

        ViewCentered&    CameraControls<ProjPerspective, ViewCentered>::getClassView() { return camera.view_class; }

        void CameraControls<ProjPerspective, ViewCentered>::processEventKey(GLFWwindow *_window, int _key, int _scancode, int _action, int _mods, double _frame_time) {
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_S)) {
                if (_action == GLFW_PRESS) {
                    cam_move_state.x = bool(cam_move_state.y) + 1;
                }
                if (_action == GLFW_RELEASE) {
                    cam_move_state.x = 0;
                    if (cam_move_state.y > 1) cam_move_state.y = 1;
                }
            }
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_W)) {
                if (_action == GLFW_PRESS) {
                    cam_move_state.y = bool(cam_move_state.x) + 1;
                }
                if (_action == GLFW_RELEASE) {
                    cam_move_state.y = 0;
                    if (cam_move_state.x > 1) cam_move_state.x = 1;
                }
            }
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_A)) {
                if (_action == GLFW_PRESS) {
                    cam_move_state.z = bool(cam_move_state.w) + 1;
                }
                if (_action == GLFW_RELEASE) {
                    cam_move_state.z = 0;
                    if (cam_move_state.w > 1) cam_move_state.w = 1;
                }
            }
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_D)) {
                if (_action == GLFW_PRESS) {
                    cam_move_state.w = bool(cam_move_state.z) + 1;
                }
                if (_action == GLFW_RELEASE) {
                    cam_move_state.w = 0;
                    if (cam_move_state.z > 1) cam_move_state.z = 1;
                }
            }
        }

        void CameraControls<ProjPerspective, ViewCentered>::manualFrameUpdate(double _frame_time) {
            vec3f __front_vec = (camera.view_class.look_at - camera.view_class.calcCamPos()).normalize();
            vec3f __right_vec = Math::cross(camera.view_class.up_vect, __front_vec);

            camera.view_class.look_at += (cam_move_state.y - cam_move_state.x) * __front_vec * _frame_time;
            camera.view_class.look_at += (cam_move_state.w - cam_move_state.z) * __right_vec * _frame_time;
        }

        void CameraControls<ProjPerspective, ViewCentered>::processEventMButton(GLFWwindow* _window, int _button, int _action, int _mods, double _frame_time) {}

        void CameraControls<ProjPerspective, ViewCentered>::processEventScroll(GLFWwindow *_window, double _offset_x, double _offset_y, double _frame_time) {
            camera.proj_class.fovy = Math::clampCL(camera.proj_class.fovy - (float) _offset_y, 20.f, 45.f);
        }

        void CameraControls<ProjPerspective, ViewCentered>::processEventCursor(GLFWwindow *_window, double _pos_x, double _pos_y, double _frame_time) {
            if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_RELEASE) {
                vec2 __cur_pos{(float)_pos_x, (float)_pos_y};
                if (LMB_active) {
                    vec2 __pos_offset{__cur_pos - last_cursor_pos};

                    camera.view_class.yaw   += __pos_offset.x * speed_coef;
                    camera.view_class.pitch += __pos_offset.y * speed_coef;
                }
                LMB_active = true;
                last_cursor_pos = __cur_pos;
            }
            else
                LMB_active = false;
        }

        void CameraControls<ProjPerspective, ViewCentered>::processEventFBufferSize(GLFWwindow *_window, int _width, int _height, double _frame_time) {
            camera.proj_class.updateReso({ (float) _width, (float) _height });
        }

        CameraControls<ProjPerspective, ViewPOV>::CameraControls(const ProjPerspective &_proj, const ViewPOV &_view) : camera{_proj, _view} {
            handled_keys = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D };
        }

        mat4 CameraControls<ProjPerspective, ViewPOV>::getView() const { return camera.getView(); }

        mat4 CameraControls<ProjPerspective, ViewPOV>::getProj() const { return camera.getProj(); }

        ProjPerspective& CameraControls<ProjPerspective, ViewPOV>::getClassProj() { return camera.proj_class; }

        ViewPOV& CameraControls<ProjPerspective, ViewPOV>::getClassView() { return camera.view_class; }

        void CameraControls<ProjPerspective, ViewPOV>::processEventKey(GLFWwindow *_window, int _key, int _scancode, int _action, int _mods, double _frame_time) {
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_S)) {
                if (_action == GLFW_PRESS) {
                    cam_move_state.x = bool(cam_move_state.y) + 1;
                }
                if (_action == GLFW_RELEASE) {
                    cam_move_state.x = 0;
                    if (cam_move_state.y > 1) cam_move_state.y = 1;
                }
            }
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_W)) {
                if (_action == GLFW_PRESS) {
                    cam_move_state.y = bool(cam_move_state.x) + 1;
                }
                if (_action == GLFW_RELEASE) {
                    cam_move_state.y = 0;
                    if (cam_move_state.x > 1) cam_move_state.x = 1;
                }
            }
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_A)) {
                if (_action == GLFW_PRESS) {
                    cam_move_state.z = bool(cam_move_state.w) + 1;
                }
                if (_action == GLFW_RELEASE) {
                    cam_move_state.z = 0;
                    if (cam_move_state.w > 1) cam_move_state.w = 1;
                }
            }
            if (_scancode == glfwGetKeyScancode(GLFW_KEY_D)) {
                if (_action == GLFW_PRESS) {
                    cam_move_state.w = bool(cam_move_state.z) + 1;
                }
                if (_action == GLFW_RELEASE) {
                    cam_move_state.w = 0;
                    if (cam_move_state.z > 1) cam_move_state.z = 1;
                }
            }
        }

        void CameraControls<ProjPerspective, ViewPOV>::processEventMButton(GLFWwindow* _window, int _button, int _action, int _mods, double _frame_time) {}

        void CameraControls<ProjPerspective, ViewPOV>::processEventScroll(GLFWwindow *_window, double _offset_x, double _offset_y, double _frame_time) {
            camera.proj_class.fovy = Math::clampCL(camera.proj_class.fovy - (float) _offset_y, 20.f, 45.f);
        }

        void CameraControls<ProjPerspective, ViewPOV>::processEventCursor(GLFWwindow *_window, double _pos_x, double _pos_y, double _frame_time) {
            if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_RELEASE) {
                vec2 __cur_pos{(float)_pos_x, (float)_pos_y};
                if (LMB_active) {
                    vec2 __pos_offset{__cur_pos - last_cursor_pos};

                    camera.view_class.yaw   += __pos_offset.x * speed_coef;
                    camera.view_class.pitch -= __pos_offset.y * speed_coef;
                }
                LMB_active = true;
                last_cursor_pos = __cur_pos;
            }
            else
                LMB_active = false;
        }

        void CameraControls<ProjPerspective, ViewPOV>::processEventFBufferSize(GLFWwindow *_window, int _width, int _height, double _frame_time) {
            camera.proj_class.updateReso({ (float) _width, (float) _height });
        }

        void CameraControls<ProjPerspective, ViewPOV>::manualFrameUpdate(double _frame_time) {
            vec3 __front_vec = (camera.view_class.calcLookAt() - camera.view_class.cam_pos).normalize();
            vec3 __right_vec = Math::cross(camera.view_class.up_vect, __front_vec);

            camera.view_class.cam_pos += (cam_move_state.y - cam_move_state.x) * __front_vec * _frame_time;
            camera.view_class.cam_pos += (cam_move_state.w - cam_move_state.z) * __right_vec * _frame_time;
        }
    }; // WindowInput END
}; // Simple END
