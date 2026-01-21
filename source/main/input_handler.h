#ifndef SVKFW_INPUT_HANDLER_H
#define SVKFW_INPUT_HANDLER_H

#include "main/camera.h"
#include "main/timer.h"
#include "interface/glfwrap.h"

#include <unordered_map>


namespace Simple {
    namespace WindowInput {
    // Frame time constants

        const double FPS30_FrameTime_Sec = 1. / 30;
        const double FPS60_FrameTime_Sec = 1. / 60;


    // InputType

        enum InputTypeGLFW {
            INPUT_TYPE_GLFW_KEY = 1,
            INPUT_TYPE_GLFW_CURSOR = 2,
            INPUT_TYPE_GLFW_MOUSE_BUTTON = 4,
            INPUT_TYPE_GLFW_SCROLL = 8,
            INPUT_TYPE_GLFW_FBUFFER_SIZE = 16,
        }; // InputTypeGLFW END


    // InputHandlerItf

        struct InputHandlerItf {
            std::vector<uint16_t> handled_keys; // can be used by setKeysHandler() when no keys are specified as argument, tells what keys are handled by InputHandler

            virtual ~InputHandlerItf() {}

            virtual void processEventKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods, double _frame_time) {}
            virtual void processEventCursor(GLFWwindow* _window, double _pos_x, double _pos_y, double _frame_time) {}
            virtual void processEventMButton(GLFWwindow* _window, int _button, int _action, int _mods, double _frame_time) {}
            virtual void processEventScroll(GLFWwindow* _window, double _offset_x, double _offset_y, double _frame_time) {}
            virtual void processEventFBufferSize(GLFWwindow* _window, int _width, int _height, double _frame_time) {}
        }; // InputHandlerBase END


    // Callbacks

        void callbackKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);
        void callbackCursor(GLFWwindow* _window, double _pos_x, double _pos_y);
        void callbackMButton(GLFWwindow* _window, int _button, int _action, int _mods);
        void callbackScroll(GLFWwindow* _window, double _offset_x, double _offset_y);
        void callbackFBufferSize(GLFWwindow* _window, int _width, int _height);


    // FrameInputHandlerGLFW

        // Frame GLFW event handler
        struct FrameInputHandlerGLFW {
            InputHandlerItf* handler_bind_keys;
            std::unordered_map<int, InputHandlerItf*> handler_bind_key;
            bool use_specific_key_handlers = true;

            InputHandlerItf* handler_bind_mbuttons;
            std::unordered_map<int, InputHandlerItf*> handler_bind_mbutton;
            bool use_specific_mbutton_handlers = true;

            InputHandlerItf* handler_bind_cursor;
            InputHandlerItf* handler_bind_scroll;
            InputHandlerItf* handler_fbuffer_size;

            ClockTick<std::chrono::high_resolution_clock, double> frame_clock;
            double frame_time = 0.; // updated every frame


            FrameInputHandlerGLFW(GLFWwindow* _window = nullptr, uint32_t _cback_types = UINT32_MAX) {
                if (_window) setGLFWCallbacks(_window, _cback_types);
            }


        // Callbacks and handlers
            // Boolean

            bool hasKeysHandler() const { return handler_bind_keys; }
            bool hasCursorHandler() const { return handler_bind_cursor; }
            bool hasScrollHandler() const { return handler_bind_scroll; }
            bool hasMButtonsHandler() const { return handler_bind_mbuttons; }
            bool hasFBufferSizeHandler() const { return handler_fbuffer_size; }
            bool hasKeySpecificHandler(uint16_t _key) const {
                return hasKeyScancodeSpecificHandler(glfwGetKeyScancode(_key));
            }
            bool hasKeyScancodeSpecificHandler(int _scancode) const {
                return handler_bind_key.find(_scancode) != handler_bind_key.end();
            }
            bool hasMButtonSpecificHandler(int _button) const {
                return handler_bind_mbutton.find(_button) != handler_bind_key.end();
            }

            // Setters
            // Sets this object as user pointer for the window and sets callbacks
            void setGLFWCallbacks(GLFWwindow* _window, uint32_t _cback_types = UINT32_MAX) {
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

                glfwSetWindowUserPointer(_window, this);
            }

            // 'use_for_all' - ignore any key-specific handlers
            InputHandlerItf* setAllKeysHandler(InputHandlerItf *_handler, bool _use_for_all) {
                use_specific_key_handlers = !_use_for_all;
                return setAllKeysHandler(_handler);
            }
            InputHandlerItf* setAllKeysHandler(InputHandlerItf *_handler) {
                InputHandlerItf* __prev_handler = handler_bind_keys;
                handler_bind_keys = _handler;
                return __prev_handler;
            }
            // If 'keys' vector is empty, uses 'handled_keys' member
            void setKeysHandler(InputHandlerItf *_handler, const std::vector<uint16_t> &_keys = {}) {
                for (uint16_t key : (_keys.empty() ? _handler->handled_keys : _keys))
                    handler_bind_key[glfwGetKeyScancode(key)] = _handler;
            }

            void setCursorHandler(InputHandlerItf *_handler) {
                handler_bind_cursor = _handler;
            }

            void setScrollHandler(InputHandlerItf *_handler) {
                handler_bind_scroll = _handler;
            }

            // 'use_for_all' - ignore any button-specific handlers
            InputHandlerItf* setAllMButtonsHandler(InputHandlerItf *_handler, bool _use_for_all) {
                use_specific_mbutton_handlers = !_use_for_all;
                return setAllMButtonsHandler(_handler);
            }
            InputHandlerItf* setAllMButtonsHandler(InputHandlerItf *_handler) {
                InputHandlerItf* __prev_handler = handler_bind_mbuttons;
                handler_bind_mbuttons = _handler;
                return __prev_handler;
            }
            void setMButtonsHandler(InputHandlerItf *_handler, const std::vector<int> &_buttons) {
                for (uint16_t button : _buttons)
                    handler_bind_mbutton[button] = _handler;
            }

            InputHandlerItf* setFBufferSizeHandler(InputHandlerItf *_handler) {
                InputHandlerItf* __prev_handler = handler_fbuffer_size;
                handler_fbuffer_size = _handler;
                return __prev_handler;
            }


            // Getters

            // Returns key-specific handler if exists, else handler_bind_keys
            InputHandlerItf* getKeyHandler(uint16_t _key) {
                return getKeyScancodeHandler(glfwGetKeyScancode(_key));
            }
            // Returns key-specific handler if exists, else handler_bind_keys
            InputHandlerItf* getKeyScancodeHandler(int _scancode) {
                return hasKeyScancodeSpecificHandler(_scancode) ? handler_bind_key[_scancode] : handler_bind_keys;
            }

        // Called in GLFW callbacks

            void eventKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods) {
                InputHandlerItf *__input_handler = handler_bind_keys;
                if (use_specific_key_handlers && hasKeyScancodeSpecificHandler(_scancode))
                    __input_handler = handler_bind_key[_scancode];

                if (__input_handler != nullptr)
                    __input_handler->processEventKey(_window, _key, _scancode, _action, _mods, frame_time);
                // else
                //     fprintf(svkfwwarn, "%s %s", SVKFW_WRAPWARN("WindowInput :: FrameInputHandlerGLFW :: eventKey", " no handler set for key:"), glfwGetKeyName(_key, _scancode));
            }

            void eventCursor(GLFWwindow* _window, double _pos_x, double _pos_y) {
                if (handler_bind_cursor != nullptr)
                    handler_bind_cursor->processEventCursor(_window, _pos_x, _pos_y, frame_time);
                // else
                //     fprintf(svkfwwarn, SVKFW_WRAPWARN("WindowInput :: FrameInputHandlerGLFW :: eventCursor", " no handler set for cursor position"));
            }

            void eventScroll(GLFWwindow* _window, double _offset_x, double _offset_y) {
                if (handler_bind_scroll != nullptr)
                    handler_bind_scroll->processEventScroll(_window, _offset_x, _offset_y, frame_time);
                // else
                //     fprintf(svkfwwarn, SVKFW_WRAPWARN("WindowInput :: FrameInputHandlerGLFW :: eventScroll", " no handler set for mouse scroll"));
            }

            void eventMButton(GLFWwindow* _window, int _button, int _action, int _mods) {
                InputHandlerItf *__input_handler = handler_bind_mbuttons;
                if (use_specific_mbutton_handlers && hasMButtonSpecificHandler(_button))
                    __input_handler = handler_bind_mbutton[_button];

                if (__input_handler != nullptr)
                    __input_handler->processEventMButton(_window, _button, _action, _mods, frame_time);
                // else
                //     fprintf(svkfwwarn, "%s %d", SVKFW_WRAPWARN("WindowInput :: FrameInputHandlerGLFW :: eventMButton", " no handler set for mouse button:"), _button);
            }

            void eventFBufferSize(GLFWwindow* _window, int _width, int _height) {
                if (handler_fbuffer_size != nullptr)
                    handler_fbuffer_size->processEventFBufferSize(_window, _width, _height, frame_time);
            }


        // Timer handling

            double getFrameTime() const { return frame_time; }
            void resetFrameTime() {
                frame_time = 0.;
                frame_clock.tick();
            }


        // Call every frame

            void frameTick() {
                frame_time = frame_clock.tick();
            }
        }; // FrameInputHandlerGLFW END

    // Callbacks which call FrameInputHandlerGLFW

        void callbackKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods) {
            FrameInputHandlerGLFW *__input_handler = static_cast<FrameInputHandlerGLFW*>(glfwGetWindowUserPointer(_window));
            __input_handler->eventKey(_window, _key, _scancode, _action, _mods);
        }

        void callbackCursor(GLFWwindow* _window, double _pos_x, double _pos_y) {
            FrameInputHandlerGLFW *__input_handler = static_cast<FrameInputHandlerGLFW*>(glfwGetWindowUserPointer(_window));
            __input_handler->eventCursor(_window, _pos_x, _pos_y);
        }

        void callbackMButton(GLFWwindow* _window, int _button, int _action, int _mods) {
            FrameInputHandlerGLFW *__input_handler = static_cast<FrameInputHandlerGLFW*>(glfwGetWindowUserPointer(_window));
            __input_handler->eventMButton(_window, _button, _action, _mods);
        }

        void callbackScroll(GLFWwindow* _window, double _offset_x, double _offset_y) {
            FrameInputHandlerGLFW *__input_handler = static_cast<FrameInputHandlerGLFW*>(glfwGetWindowUserPointer(_window));
            __input_handler->eventScroll(_window, _offset_x, _offset_y);
        }

        void callbackFBufferSize(GLFWwindow* _window, int _width, int _height) {
            FrameInputHandlerGLFW *__input_handler = static_cast<FrameInputHandlerGLFW*>(glfwGetWindowUserPointer(_window));
            __input_handler->eventFBufferSize(_window, _width, _height);
        }


    // Predefined input handlers

        struct DefaultHandler : InputHandlerItf {
            Window managed_window;
            vec4i windowed_pos_size;
            bool fullscreen_mode = false;
            bool true_fullscreen = false;

            DefaultHandler(GLFWwindow *_window) : managed_window{_window} {
                handled_keys = { GLFW_KEY_ESCAPE, GLFW_KEY_F1, GLFW_KEY_F5 };
            }
            ~DefaultHandler() {
                managed_window.window = nullptr; // Not handling this window here
            }

            void processEventKey(GLFWwindow *_window, int _key, int _scancode, int _action, int _mods, double _frame_time) override {
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
        }; // DefaultHandler END


    // Camera-related input handlers

        template <class P, class V>
        struct CameraControls : InputHandlerItf {
            Camera<P, V> camera;

            CameraControls(const P &_proj, const V &_view) : camera{_proj, _view} {
                handled_keys = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D };
            }

            mat4 getView() const { return camera.getView(); }
            mat4 getProj() const { return camera.getProj(); }
            P& getClassView() { return camera.proj_class; }
            V& getClassProj() { return camera.view_class; }

            void processEventKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods, double _frame_time) override {}
            void processEventCursor(GLFWwindow* _window, double _pos_x, double _pos_y, double _frame_time) override {}
            void processEventMButton(GLFWwindow* _window, int _button, int _action, int _mods, double _frame_time) override {}
            void processEventScroll(GLFWwindow* _window, double _offset_x, double _offset_y, double _frame_time) override {}
        }; // CameraControls END

        template<>
        struct CameraControls<ProjPerspective, ViewGeneral> : InputHandlerItf {
            Camera<ProjPerspective, ViewGeneral> camera;
            vec4i8 cam_move_state;

            CameraControls(const ProjPerspective &_proj, const ViewGeneral &_view) : camera{_proj, _view} {}

            mat4 getView() const { return camera.getView(); }
            mat4 getProj() const { return camera.getProj(); }
            ProjPerspective& getClassView() { return camera.proj_class; }
            ViewGeneral& getClassProj() { return camera.view_class; }

            void processEventKey(GLFWwindow *_window, int _key, int _scancode, int _action, int _mods, double _frame_time) override {
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

            void manualFrameUpdate(double _frame_time) {
                vec3 __front_vec = (camera.view_class.look_at - camera.view_class.cam_pos).normalize();
                vec3 __right_vec = Math::cross(__front_vec, camera.view_class.up_vect);

                camera.view_class.cam_pos += (cam_move_state.y - cam_move_state.x) * __front_vec * _frame_time;
                camera.view_class.look_at += (cam_move_state.y - cam_move_state.x) * __front_vec * _frame_time;
                camera.view_class.cam_pos += (cam_move_state.w - cam_move_state.z) * __right_vec * _frame_time;
                camera.view_class.look_at += (cam_move_state.w - cam_move_state.z) * __right_vec * _frame_time;
            }

            void processEventScroll(GLFWwindow *_window, double _offset_x, double _offset_y, double _frame_time) override {
                camera.proj_class.fovy = Math::clampCL(camera.proj_class.fovy - (float) _offset_y, 20.f, 45.f);
            }

            void processEventFBufferSize(GLFWwindow *_window, int _width, int _height, double _frame_time) override {
                camera.proj_class.updateReso({ (float) _width, (float) _height });
            }
        }; // CameraControls<ProjPerspective, ViewGeneral> END

        template<>
        struct CameraControls<ProjPerspective, ViewCentered> : InputHandlerItf {
            Camera<ProjPerspective, ViewCentered> camera;
            vec4i8 cam_move_state;

            CameraControls(const ProjPerspective &_proj, const ViewCentered &_view) : camera{_proj, _view} {
                handled_keys = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D };
            }

            mat4 getView() const { return camera.getView(); }
            mat4 getProj() const { return camera.getProj(); }
            ProjPerspective& getClassView() { return camera.proj_class; }
            ViewCentered&    getClassProj() { return camera.view_class; }

            void processEventKey(GLFWwindow *_window, int _key, int _scancode, int _action, int _mods, double _frame_time) override {
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

            void manualFrameUpdate(double _frame_time) {
                vec3f __front_vec = (camera.view_class.look_at - camera.view_class.calcCamPos()).normalize();
                vec3f __right_vec = Math::cross(camera.view_class.up_vect, __front_vec);

                camera.view_class.look_at += (cam_move_state.y - cam_move_state.x) * __front_vec * _frame_time;
                camera.view_class.look_at += (cam_move_state.w - cam_move_state.z) * __right_vec * _frame_time;
            }

            void processEventScroll(GLFWwindow *_window, double _offset_x, double _offset_y, double _frame_time) override {
                camera.proj_class.fovy = Math::clampCL(camera.proj_class.fovy - (float) _offset_y, 20.f, 45.f);
            }

            void processEventCursor(GLFWwindow *_window, double _pos_x, double _pos_y, double _frame_time) override {
                static vec2 last_cursor_pos{};
                static bool LMB_active = false;
                const float speed_coef = 0.4f;

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

            void processEventFBufferSize(GLFWwindow *_window, int _width, int _height, double _frame_time) override {
                camera.proj_class.updateReso({ (float) _width, (float) _height });
            }
        }; // CameraControls<ProjPerspective, ViewCentered> END

        template<>
        struct CameraControls<ProjPerspective, ViewPOV> : InputHandlerItf {
            Camera<ProjPerspective, ViewPOV> camera;
            vec4i8 cam_move_state;

            CameraControls(const ProjPerspective &_proj, const ViewPOV &_view) : camera{_proj, _view} {
                handled_keys = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D };
            }

            mat4 getView() const { return camera.getView(); }
            mat4 getProj() const { return camera.getProj(); }
            ProjPerspective& getClassView() { return camera.proj_class; }
            ViewPOV& getClassProj() { return camera.view_class; }

            void processEventKey(GLFWwindow *_window, int _key, int _scancode, int _action, int _mods, double _frame_time) override {
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

            void processEventScroll(GLFWwindow *_window, double _offset_x, double _offset_y, double _frame_time) override {
                camera.proj_class.fovy = Math::clampCL(camera.proj_class.fovy - (float) _offset_y, 20.f, 45.f);
            }

            void processEventCursor(GLFWwindow *_window, double _pos_x, double _pos_y, double _frame_time) override {
                static vec2 last_cursor_pos{};
                static bool LMB_active = false;
                const float speed_coef = 0.4f;

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

            void processEventFBufferSize(GLFWwindow *_window, int _width, int _height, double _frame_time) override {
                camera.proj_class.updateReso({ (float) _width, (float) _height });
            }

            void manualFrameUpdate(double _frame_time) {
                vec3 __front_vec = (camera.view_class.calcLookAt() - camera.view_class.cam_pos).normalize();
                vec3 __right_vec = Math::cross(camera.view_class.up_vect, __front_vec);

                camera.view_class.cam_pos += (cam_move_state.y - cam_move_state.x) * __front_vec * _frame_time;
                camera.view_class.cam_pos += (cam_move_state.w - cam_move_state.z) * __right_vec * _frame_time;
            }
        }; // CameraControls<ProjPerspective, ViewPOV> END

        using CameraControlsPOV = CameraControls<ProjPerspective, ViewPOV>;
        using CameraControlsOrthoGeneral = CameraControls<ProjOrtho, ViewGeneral>;
        using CameraControlsGeneral = CameraControls<ProjPerspective, ViewGeneral>;
        using CameraControlsOrthoCentered = CameraControls<ProjOrtho, ViewCentered>;
        using CameraControlsCentered = CameraControls<ProjPerspective, ViewCentered>;
    }; // WindowInput END
}; // Simple END

#endif