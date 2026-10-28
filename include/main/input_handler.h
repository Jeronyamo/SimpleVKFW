#ifndef SVKFW_INPUT_HANDLER_H
#define SVKFW_INPUT_HANDLER_H

#include "main/camera.h"
#include "main/timer.h"
#include "interface/glfwrap.h"
#include "interface/imgui_wrap.h"
#include "interface/vkfw_graph.h"

#include <functional>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>


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
            INPUT_TYPE_GLFW_FOCUS        = 1 << 8,
        }; // InputTypeGLFW END


    // InputBinding

        enum InputDevice {
            INPUT_DEVICE_KEY          = 0,
            INPUT_DEVICE_MOUSE_BUTTON = 1,
        }; // InputDevice END

        // A key or a mouse button with exact modifiers (Shift/Ctrl/Alt/Super; Caps/Num Lock are ignored).
        // Text form (case-insensitive, '+' between parts): "W", "Ctrl+1", "Ctrl+Shift+F5", "Alt+Enter", "MouseLeft", "Ctrl+Mouse4".
        // Key names follow GLFW_KEY_* without the prefix and underscores ("PageUp", "KP5", "LeftShift", "GraveAccent")
        struct InputBinding {
            static const int MODS_MASK = GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT | GLFW_MOD_SUPER;

            int device = INPUT_DEVICE_KEY;
            int code   = GLFW_KEY_UNKNOWN; // GLFW_KEY_* or GLFW_MOUSE_BUTTON_*
            int mods   = 0;

            InputBinding();
            InputBinding(int _device, int _code, int _mods = 0);

            static InputBinding Key(int _key, int _mods = 0);
            static InputBinding MouseButton(int _button, int _mods = 0);

            // Returns false (and leaves '_binding' unchanged) if the text is not a binding
            static bool parse(const std::string &_text, InputBinding &_binding);
            std::string toString() const;

            bool isNone() const;
            bool operator==(const InputBinding &_binding) const;

            // Keys for hash maps: the whole binding / the key or button without modifiers
            int64_t code64() const;
            int64_t input64() const;
        }; // InputBinding END


    // InputEvent

        enum InputEventType {
            INPUT_EVENT_KEY = 0,
            INPUT_EVENT_MOUSE_BUTTON,
            INPUT_EVENT_CURSOR,
            INPUT_EVENT_SCROLL,
            INPUT_EVENT_CHAR,
            // Broadcast: every layer gets these, the result of 'onEvent' is ignored
            INPUT_EVENT_FBUFFER_SIZE,
            INPUT_EVENT_FOCUS,
            INPUT_EVENT_CURSOR_ENTER,
        }; // InputEventType END

        // One GLFW event; the fields of other event types are zero
        struct InputEvent {
            InputEventType type   = INPUT_EVENT_KEY;
            GLFWwindow    *window = nullptr;

            // Key/mouse button: GLFW_KEY_*/GLFW_MOUSE_BUTTON_*, GLFW_PRESS/RELEASE/REPEAT, GLFW_MOD_*
            int code = 0, scancode = 0, action = 0, mods = 0;
            // Cursor: position and its change since the previous cursor event (also when that event was consumed);
            // scroll: offsets in 'dx', 'dy'
            double x = 0., y = 0., dx = 0., dy = 0.;
            // Framebuffer size
            int width = 0, height = 0;
            // Focus / cursor enter
            bool flag = false;
            uint32_t character = 0u;

            bool isBroadcast() const;
            InputBinding binding() const; // key and mouse button events
        }; // InputEvent END


    // Actions

        typedef uint32_t ActionId;
        const ActionId ACTION_NONE = UINT32_MAX;

        struct ActionState {
            std::string name;
            uint32_t held     = 0u; // the number of bound inputs held now
            uint32_t presses  = 0u; // since the start of the frame
            uint32_t releases = 0u; // since the start of the frame
            uint64_t press_order = 0u; // for 'axis': the later press wins
        }; // ActionState END

        // Action ids of the predefined handlers; the input maps refer to them by these names
        namespace Actions {
            // 'DefaultHandler' (resources/input_maps/window.json)
            enum Window : ActionId {
                WINDOW_QUIT = 0,
                WINDOW_TOGGLE_FULLSCREEN,
                WINDOW_TOGGLE_TRUE_FULLSCREEN, // makes the next fullscreen exclusive (a video mode) or windowed
                WINDOW_COUNT
            };
            extern const char *const window_names[WINDOW_COUNT];

            // 'CameraControls' (resources/input_maps/camera.json)
            enum Camera : ActionId {
                CAMERA_MOVE_FORWARD = 0,
                CAMERA_MOVE_BACK,
                CAMERA_MOVE_LEFT,
                CAMERA_MOVE_RIGHT,
                CAMERA_ROTATE, // held: the cursor rotates the camera
                CAMERA_COUNT
            };
            extern const char *const camera_names[CAMERA_COUNT];

            // 'ScreenshotHandler' (resources/input_maps/screenshots.json)
            enum Screenshot : ActionId {
                SCREENSHOT_SCENE = 0, // the swapchain image without the UI
                SCREENSHOT_FRAME,     // the whole frame
                SCREENSHOT_DEPTH,
                SCREENSHOT_STENCIL,
                SCREENSHOT_COUNT
            };
            extern const char *const screenshot_names[SCREENSHOT_COUNT];
        }; // Actions END


    // InputHandlerItf

        // One layer of the input stack (see 'FrameInputHandlerGLFW'). The handler declares its actions (ids are indices,
        // names are used by the input maps), the input map binds keys and mouse buttons to them.
        // A bound press is consumed by the layer, so lower layers don't get it; its repeats and its release go to the same
        // layer. A press with modifiers that has no exact binding uses the binding without modifiers (Shift+W -> W)
        struct InputHandlerItf {
            std::vector<ActionState> actions;
            std::unordered_map<int64_t, ActionId> bindings;   // InputBinding::code64() -> action
            std::unordered_map<int64_t, ActionId> held_input; // InputBinding::input64() -> action of the press
            uint64_t press_counter = 0u;
            bool enabled = true;

            virtual ~InputHandlerItf();

        // Actions and bindings

            // Ids are the indices of the names
            void declareActions(const char *const *_names, uint32_t _count);
            ActionId addAction(const std::string &_name);
            ActionId findAction(const std::string &_name) const;

            void bind(const InputBinding &_binding, ActionId _action);
            void unbind(const InputBinding &_binding);
            void clearBindings();

            // Reads a JSON input map and adds its bindings:
            //   { "inputs": [ { "key": "Ctrl+1", "action": "screenshot_scene" }, { "key": "W", "action": "move_forward" } ] }
            // Unknown keys or actions are reported and skipped. Returns false if the file can't be read or has no "inputs"
            bool loadInputMap(const std::string &_fpath);

        // State

            bool isDown(ActionId _action) const;
            bool wasPressed(ActionId _action) const;  // since the start of the frame
            bool wasReleased(ActionId _action) const; // since the start of the frame
            // +1 if '_positive' is held, -1 if '_negative' is; both: the one pressed later
            float axis(ActionId _positive, ActionId _negative) const;

            // Releases all held actions (calls 'onAction' with GLFW_RELEASE)
            void releaseAll();
            void setEnabled(bool _enabled);

        // Called by FrameInputHandlerGLFW

            virtual void newFrame();
            // Returns true if the event is consumed. The default handles key and mouse button bindings
            virtual bool onEvent(const InputEvent &_event);
            // A bound input of the action was pressed, repeated or released (GLFW_PRESS/REPEAT/RELEASE)
            virtual void onAction(ActionId _action, int _glfw_action);

        protected:
            bool processBinding(const InputEvent &_event);
        }; // InputHandlerItf END


    // ActionCallbacks

        // Actions with functions called on press, e.g. hotkeys (actions are added in code, keys come from an input map)
        struct ActionCallbacks : InputHandlerItf {
            std::vector<std::function<void()>> callbacks; // per action

            ActionId addAction(const std::string &_name, const std::function<void()> &_on_press);

            void onAction(ActionId _action, int _glfw_action) override;
        }; // ActionCallbacks END


    // ImGuiLayer

        // Passes all events to ImGui and consumes those it wants (key presses while it has the keyboard focus,
        // mouse presses, cursor and scroll over its windows). Releases are never consumed
        struct ImGuiLayer : InputHandlerItf {
            ImGUI::ImGuiHandler &imgui;

            explicit ImGuiLayer(ImGUI::ImGuiHandler &_imgui);

            bool onEvent(const InputEvent &_event) override;
        }; // ImGuiLayer END


    // FrameInputHandlerGLFW

        // Frame GLFW event dispatcher: events go through 'layers' from the first one until a layer consumes them.
        // The first layer is ImGui ('imgui_layer'); 'pushLayer' adds below the others, 'insertLayer' at a position.
        // Note: it is the user pointer of its window, so it is not copyable and must outlive the window callbacks
        struct FrameInputHandlerGLFW {
            ImGUI::ImGuiHandler imgui_handler;
            ImGuiLayer          imgui_layer;
            std::vector<InputHandlerItf*> layers;

            ClockTick<std::chrono::high_resolution_clock, double> frame_clock;
            double frame_time = 0.; // updated every frame

            vec2d cursor_pos{};      // the last cursor position
            bool  cursor_known = false;

            FrameInputHandlerGLFW(GLFWwindow* _window = nullptr, uint32_t _cback_types = UINT32_MAX);

            // Sets this object as user pointer for the window and sets callbacks
            void setGLFWCallbacks(GLFWwindow* _window, uint32_t _cback_types = UINT32_MAX);

        // Layers

            void pushLayer(InputHandlerItf *_layer);
            void insertLayer(uint32_t _index, InputHandlerItf *_layer);
            void removeLayer(InputHandlerItf *_layer);

            // Returns true if a layer consumed the event
            bool dispatch(const InputEvent &_event);

        // Timer handling

            double getFrameTime() const;
            void resetFrameTime();


        // Call every frame (before 'glfwPollEvents')

            void frameTick();

        private:
            FrameInputHandlerGLFW(const FrameInputHandlerGLFW &);
            FrameInputHandlerGLFW& operator=(const FrameInputHandlerGLFW &);
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

        void callbackFocus(GLFWwindow* _window, int _focused);


    // Predefined input handlers

        // Window controls: Actions::Window
        struct DefaultHandler : InputHandlerItf {
            Window managed_window; // keeps the windowed position and size between switches
            bool true_fullscreen = false;

            DefaultHandler(GLFWwindow *_window);
            ~DefaultHandler();

            void onAction(ActionId _action, int _glfw_action) override;
        }; // DefaultHandler END


    // Screenshots

        // Actions::Screenshot: each action requests one capture (see 'VKFW::Graph::FrameCapture'). What it copies is set by
        // 'setTarget' (the render graph is usually made after the input handlers); actions without a target do nothing.
        // Files: '<directory>/<date_time-ms>_<name>.png'. In the main loop: 'save' after the fence of the frame is waited,
        // 'record' before 'RenderGraph::execute', 'destroy' before the device is destroyed
        struct ScreenshotHandler : InputHandlerItf {
            struct Target {
                VKFW::Graph::FrameCapture::Kind kind = VKFW::Graph::FrameCapture::VKFW_CAPTURE_COLOR;
                VKFW::Graph::PassId     after;
                VKFW::Graph::ResourceId res;
                std::string             name;
                bool                    set = false;
            };

            VKFW::Graph::FrameCapture capture;
            Target      targets[Actions::SCREENSHOT_COUNT];
            std::string directory = "screenshots";

            ScreenshotHandler();

            // '_name' is the end of the file name ("" - the action name without "screenshot_")
            void setTarget(ActionId _action, VKFW::Graph::FrameCapture::Kind _kind, VKFW::Graph::PassId _after,
                           VKFW::Graph::ResourceId _res, const std::string &_name = "");
            // The screenshot of the action, also without its key; returns false if one is pending or there is no target
            bool request(ActionId _action);

            void record(VKFW::Graph::RenderGraph &_graph, uint32_t _frame);
            // Returns true if a file is written (and prints its name)
            bool save(uint32_t _frame);
            // Writes the last capture if it is recorded (the device must be idle) and frees the buffer
            void destroy();

            void onAction(ActionId _action, int _glfw_action) override;
        }; // ScreenshotHandler END


    // Camera-related input handlers

        // Movement and rotation actions of the cameras: Actions::Camera
        struct CameraControlsBase : InputHandlerItf {
            const float rotate_speed = 0.4f; // degrees per pixel

            CameraControlsBase();

            // Forward/back and right/left in [-1, 1]
            float moveForward() const;
            float moveRight() const;
        }; // CameraControlsBase END

        template <class P, class V>
        struct CameraControls : CameraControlsBase {
            Camera<P, V> camera;

            CameraControls(const P &_proj, const V &_view) : camera{_proj, _view} {}

            mat4 getView() const { return camera.getView(); }
            mat4 getProj() const { return camera.getProj(); }
            P& getClassProj() { return camera.proj_class; }
            V& getClassView() { return camera.view_class; }

            void manualFrameUpdate(double _frame_time) {}
        }; // CameraControls END

        template<>
        struct CameraControls<ProjPerspective, ViewGeneral> : CameraControlsBase {
            Camera<ProjPerspective, ViewGeneral> camera;

            CameraControls(const ProjPerspective &_proj, const ViewGeneral &_view);

            mat4 getView() const;
            mat4 getProj() const;
            ProjPerspective& getClassProj();
            ViewGeneral& getClassView();

            void manualFrameUpdate(double _frame_time);

            bool onEvent(const InputEvent &_event) override;
        }; // CameraControls<ProjPerspective, ViewGeneral> END

        template<>
        struct CameraControls<ProjPerspective, ViewCentered> : CameraControlsBase {
            Camera<ProjPerspective, ViewCentered> camera;

            CameraControls(const ProjPerspective &_proj, const ViewCentered &_view);

            mat4 getView() const;
            mat4 getProj() const;
            ProjPerspective& getClassProj();
            ViewCentered&    getClassView();

            void manualFrameUpdate(double _frame_time);

            bool onEvent(const InputEvent &_event) override;
        }; // CameraControls<ProjPerspective, ViewCentered> END

        template<>
        struct CameraControls<ProjPerspective, ViewPOV> : CameraControlsBase {
            Camera<ProjPerspective, ViewPOV> camera;

            CameraControls(const ProjPerspective &_proj, const ViewPOV &_view);

            mat4 getView() const;
            mat4 getProj() const;
            ProjPerspective& getClassProj();
            ViewPOV& getClassView();

            void manualFrameUpdate(double _frame_time);

            bool onEvent(const InputEvent &_event) override;
        }; // CameraControls<ProjPerspective, ViewPOV> END

        using CameraControlsPOV = CameraControls<ProjPerspective, ViewPOV>;
        using CameraControlsOrthoGeneral = CameraControls<ProjOrtho, ViewGeneral>;
        using CameraControlsGeneral = CameraControls<ProjPerspective, ViewGeneral>;
        using CameraControlsOrthoCentered = CameraControls<ProjOrtho, ViewCentered>;
        using CameraControlsCentered = CameraControls<ProjPerspective, ViewCentered>;
    }; // WindowInput END
}; // Simple END

#endif
