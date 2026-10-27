#ifndef SVKFW_INPUT_HANDLER_H
#define SVKFW_INPUT_HANDLER_H

#include "main/camera.h"
#include "main/timer.h"
#include "interface/glfwrap.h"
#include "interface/imgui_wrap.h"

#include <unordered_map>


namespace Simple {
    namespace WindowInput {
    // Frame time constants

        const double FPS30_FrameTime_Sec = 1. / 30;
        const double FPS60_FrameTime_Sec = 1. / 60;


    // InputType

        enum InputTypeGLFW {
            INPUT_TYPE_GLFW_KEY          = 1 << 0,
            INPUT_TYPE_GLFW_CURSOR       = 1 << 1,
            INPUT_TYPE_GLFW_MOUSE_BUTTON = 1 << 2,
            INPUT_TYPE_GLFW_SCROLL       = 1 << 3,
            INPUT_TYPE_GLFW_FBUFFER_SIZE = 1 << 4,
            INPUT_TYPE_GLFW_CURSOR_ENTER = 1 << 5,
            INPUT_TYPE_GLFW_CHAR         = 1 << 6,
            INPUT_TYPE_GLFW_MONITOR      = 1 << 7,
        }; // InputTypeGLFW END

    // InputHandlerItf

        struct InputHandlerItf {
            std::vector<uint16_t> handled_keys; // can be used by setKeysHandler() when no keys are specified as argument, tells what keys are handled by InputHandler

            virtual ~InputHandlerItf();

            virtual void processEventKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods, double _frame_time);
            virtual void processEventCursor(GLFWwindow* _window, double _pos_x, double _pos_y, double _frame_time);
            virtual void processEventMButton(GLFWwindow* _window, int _button, int _action, int _mods, double _frame_time);
            virtual void processEventScroll(GLFWwindow* _window, double _offset_x, double _offset_y, double _frame_time);
            virtual void processEventFBufferSize(GLFWwindow* _window, int _width, int _height, double _frame_time);
            virtual void processEventCursorEnter(GLFWwindow* _window, int _entered);
            virtual void processEventChar(GLFWwindow* _window, uint32_t _character);
            virtual void processEventMonitor(GLFWmonitor* _monitor, int _event);
        }; // InputHandlerBase END


    // Callbacks

        void callbackKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);
        void callbackCursor(GLFWwindow* _window, double _pos_x, double _pos_y);
        void callbackMButton(GLFWwindow* _window, int _button, int _action, int _mods);
        void callbackScroll(GLFWwindow* _window, double _offset_x, double _offset_y);
        void callbackFBufferSize(GLFWwindow* _window, int _width, int _height);
        void callbackCursorEnter(GLFWwindow* _window, int _entered);
        void callbackChar(GLFWwindow* _window, uint32_t _character);
        void callbackMonitor(GLFWmonitor* _monitor, int _event);


    // FrameInputHandlerGLFW

        // Frame GLFW event handler
        struct FrameInputHandlerGLFW {
            InputHandlerItf* handler_bind_keys = nullptr;
            std::unordered_map<int, InputHandlerItf*> handler_bind_key;
            bool use_specific_key_handlers = true;

            InputHandlerItf* handler_bind_mbuttons = nullptr;
            std::unordered_map<int, InputHandlerItf*> handler_bind_mbutton;
            bool use_specific_mbutton_handlers = true;

            InputHandlerItf* handler_bind_cursor  = nullptr;
            InputHandlerItf* handler_bind_scroll  = nullptr;
            InputHandlerItf* handler_fbuffer_size = nullptr;

            ClockTick<std::chrono::high_resolution_clock, double> frame_clock;
            double frame_time = 0.; // updated every frame

            ImGUI::ImGuiHandler imgui_handler;

            FrameInputHandlerGLFW(GLFWwindow* _window = nullptr, uint32_t _cback_types = UINT32_MAX);


        // Callbacks and handlers
            // Boolean

            bool hasKeysHandler() const;
            bool hasCursorHandler() const;
            bool hasScrollHandler() const;
            bool hasMButtonsHandler() const;
            bool hasFBufferSizeHandler() const;
            bool hasKeySpecificHandler(uint16_t _key) const;
            bool hasKeyScancodeSpecificHandler(int _scancode) const;
            bool hasMButtonSpecificHandler(int _button) const;

            // Setters
            // Sets this object as user pointer for the window and sets callbacks
            void setGLFWCallbacks(GLFWwindow* _window, uint32_t _cback_types = UINT32_MAX);

            // 'use_for_all' - ignore any key-specific handlers
            InputHandlerItf* setAllKeysHandler(InputHandlerItf *_handler, bool _use_for_all);
            InputHandlerItf* setAllKeysHandler(InputHandlerItf *_handler);
            // If 'keys' vector is empty, uses 'handled_keys' member
            void setKeysHandler(InputHandlerItf *_handler, const std::vector<uint16_t> &_keys = {});

            void setCursorHandler(InputHandlerItf *_handler);

            void setScrollHandler(InputHandlerItf *_handler);

            // 'use_for_all' - ignore any button-specific handlers
            InputHandlerItf* setAllMButtonsHandler(InputHandlerItf *_handler, bool _use_for_all);
            InputHandlerItf* setAllMButtonsHandler(InputHandlerItf *_handler);
            void setMButtonsHandler(InputHandlerItf *_handler, const std::vector<int> &_buttons);

            InputHandlerItf* setFBufferSizeHandler(InputHandlerItf *_handler);


            // Getters

            // Returns key-specific handler if exists, else handler_bind_keys
            InputHandlerItf* getKeyHandler(uint16_t _key);
            // Returns key-specific handler if exists, else handler_bind_keys
            InputHandlerItf* getKeyScancodeHandler(int _scancode);

        // Called in GLFW callbacks

            void eventKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);

            void eventCursor(GLFWwindow* _window, double _pos_x, double _pos_y);

            void eventScroll(GLFWwindow* _window, double _offset_x, double _offset_y);

            void eventMButton(GLFWwindow* _window, int _button, int _action, int _mods);

            void eventFBufferSize(GLFWwindow* _window, int _width, int _height);


        // Timer handling

            double getFrameTime() const;
            void resetFrameTime();


        // Call every frame

            void frameTick();
        }; // FrameInputHandlerGLFW END

    // Callbacks which call FrameInputHandlerGLFW

        void callbackKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);

        void callbackCursor(GLFWwindow* _window, double _pos_x, double _pos_y);

        void callbackMButton(GLFWwindow* _window, int _button, int _action, int _mods);

        void callbackScroll(GLFWwindow* _window, double _offset_x, double _offset_y);

        void callbackFBufferSize(GLFWwindow* _window, int _width, int _height);

        void callbackCursorEnter(GLFWwindow* _window, int _entered);

        void callbackChar(GLFWwindow* _window, uint32_t _character);

        void callbackMonitor(GLFWmonitor* _monitor, int _event);


    // Predefined input handlers

        struct DefaultHandler : InputHandlerItf {
            Window managed_window;
            vec4i windowed_pos_size;
            bool fullscreen_mode = false;
            bool true_fullscreen = false;

            DefaultHandler(GLFWwindow *_window);
            ~DefaultHandler();

            void processEventKey(GLFWwindow *_window, int _key, int _scancode, int _action, int _mods, double _frame_time) override;
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
            P& getClassProj() { return camera.proj_class; }
            V& getClassView() { return camera.view_class; }

            void processEventKey(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods, double _frame_time) override {}
            void processEventCursor(GLFWwindow* _window, double _pos_x, double _pos_y, double _frame_time) override {}
            void processEventMButton(GLFWwindow* _window, int _button, int _action, int _mods, double _frame_time) override {}
            void processEventScroll(GLFWwindow* _window, double _offset_x, double _offset_y, double _frame_time) override {}
        }; // CameraControls END

        template<>
        struct CameraControls<ProjPerspective, ViewGeneral> : InputHandlerItf {
            Camera<ProjPerspective, ViewGeneral> camera;
            vec4i8 cam_move_state;

            CameraControls(const ProjPerspective &_proj, const ViewGeneral &_view);

            mat4 getView() const;
            mat4 getProj() const;
            ProjPerspective& getClassProj();
            ViewGeneral& getClassView();

            void processEventKey(GLFWwindow *_window, int _key, int _scancode, int _action, int _mods, double _frame_time) override;

            void manualFrameUpdate(double _frame_time);

            void processEventScroll(GLFWwindow *_window, double _offset_x, double _offset_y, double _frame_time) override;

            void processEventFBufferSize(GLFWwindow *_window, int _width, int _height, double _frame_time) override;
        }; // CameraControls<ProjPerspective, ViewGeneral> END

        template<>
        struct CameraControls<ProjPerspective, ViewCentered> : InputHandlerItf {
            Camera<ProjPerspective, ViewCentered> camera;
            vec4i8 cam_move_state;
            vec2 last_cursor_pos{};
            bool LMB_active = false;
            const float speed_coef = 0.4f;

            CameraControls(const ProjPerspective &_proj, const ViewCentered &_view);

            mat4 getView() const;
            mat4 getProj() const;
            ProjPerspective& getClassProj();
            ViewCentered&    getClassView();

            void processEventKey(GLFWwindow *_window, int _key, int _scancode, int _action, int _mods, double _frame_time) override;

            void manualFrameUpdate(double _frame_time);

            // Note: ImGui already receives mouse button events in 'callbackMButton'
            void processEventMButton(GLFWwindow* _window, int _button, int _action, int _mods, double _frame_time) override;


            void processEventScroll(GLFWwindow *_window, double _offset_x, double _offset_y, double _frame_time) override;

            void processEventCursor(GLFWwindow *_window, double _pos_x, double _pos_y, double _frame_time) override;

            void processEventFBufferSize(GLFWwindow *_window, int _width, int _height, double _frame_time) override;
        }; // CameraControls<ProjPerspective, ViewCentered> END

        template<>
        struct CameraControls<ProjPerspective, ViewPOV> : InputHandlerItf {
            Camera<ProjPerspective, ViewPOV> camera;
            vec4i8 cam_move_state;
            vec2 last_cursor_pos{};
            bool LMB_active = false;
            const float speed_coef = 0.4f;

            CameraControls(const ProjPerspective &_proj, const ViewPOV &_view);

            mat4 getView() const;
            mat4 getProj() const;
            ProjPerspective& getClassProj();
            ViewPOV& getClassView();

            void processEventKey(GLFWwindow *_window, int _key, int _scancode, int _action, int _mods, double _frame_time) override;

            void processEventMButton(GLFWwindow* _window, int _button, int _action, int _mods, double _frame_time) override;

            void processEventScroll(GLFWwindow *_window, double _offset_x, double _offset_y, double _frame_time) override;

            void processEventCursor(GLFWwindow *_window, double _pos_x, double _pos_y, double _frame_time) override;

            void processEventFBufferSize(GLFWwindow *_window, int _width, int _height, double _frame_time) override;

            void manualFrameUpdate(double _frame_time);
        }; // CameraControls<ProjPerspective, ViewPOV> END

        using CameraControlsPOV = CameraControls<ProjPerspective, ViewPOV>;
        using CameraControlsOrthoGeneral = CameraControls<ProjOrtho, ViewGeneral>;
        using CameraControlsGeneral = CameraControls<ProjPerspective, ViewGeneral>;
        using CameraControlsOrthoCentered = CameraControls<ProjOrtho, ViewCentered>;
        using CameraControlsCentered = CameraControls<ProjPerspective, ViewCentered>;
    }; // WindowInput END
}; // Simple END

#endif