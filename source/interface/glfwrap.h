#ifndef SVKFW_GLFWRAP_H
#define SVKFW_GLFWRAP_H

#define VOLK_IMPLEMENTATION
#define VK_NO_PROTOTYPES

#include "volk.h"
#include "GLFW/glfw3.h"
#include "common/basic_math.h"
#include "main/event_handler.h"
#include "math/vectors.h"

#include <memory>
#include <unordered_set>


namespace Simple {

//  ============  GLFW Monitor wrapper  ============  \\

    struct Monitor {
        GLFWmonitor* monitor;

        Monitor(GLFWmonitor* _m) : monitor{_m} {}

        operator GLFWmonitor*() { return monitor; }


        const char* getName() { return glfwGetMonitorName(monitor); }
        vec2i getPos() {
            vec2i _tmp{0};
            glfwGetMonitorPos(monitor, &_tmp.x, &_tmp.y);
            return _tmp;
        }
        vec4i getWorkarea() {
            vec4i _tmp{0};
            glfwGetMonitorWorkarea(monitor, &_tmp.x, &_tmp.y, &_tmp.z, &_tmp.w);
            return _tmp;
        }
        vec2i getPhysicalSize() {
            vec2i _tmp{0};
            glfwGetMonitorPhysicalSize(monitor, &_tmp.x, &_tmp.y);
            return _tmp;
        }
        vec2f getContentScale() {
            vec2f _tmp{0.f};
            glfwGetMonitorContentScale(monitor, &_tmp.x, &_tmp.y);
            return _tmp;
        }
        void  setUserPointer(void *_ptr) {
            glfwSetMonitorUserPointer(monitor, _ptr);
        }
        void* getUserPointer() {
            return glfwGetMonitorUserPointer(monitor);
        }
        const GLFWvidmode* getVideoMode() {
            return glfwGetVideoMode(monitor);
        }
        const GLFWvidmode* getVideoModes(int &count) {
            return glfwGetVideoModes(monitor, &count);
        }
        void setGamma(float _gamma) {
            glfwSetGamma(monitor, _gamma);
        }
        const GLFWgammaramp* getGammaRamp() {
            return glfwGetGammaRamp(monitor);
        }
        void setGammaRamp(const GLFWgammaramp* _ramp) {
            glfwSetGammaRamp(monitor, _ramp);
        }
    }; // Monitor END


    namespace AvailableMonitors {
        Monitor primary = glfwGetPrimaryMonitor();
        std::unordered_set<GLFWmonitor*> all;
        GLFWmonitorfun previous_callback = nullptr;


        void monitorCallbackAllUpd(GLFWmonitor* monitor, int event) {
            primary = glfwGetPrimaryMonitor();

            switch (event) {
                case GLFW_CONNECTED: {
                    all.insert(monitor);
                    break;
                }
                case GLFW_DISCONNECTED: {
                    all.erase(monitor);
                    break;
                }
            }
        }

        void all_refresh() {
            int count = 0;
            GLFWmonitor **_m = glfwGetMonitors(&count);

            all.clear();
            if (_m != nullptr && count > 0)
                all.insert(_m, _m + count);
        }

        void all_init() {
            all_refresh();
            previous_callback = glfwSetMonitorCallback(monitorCallbackAllUpd);
        }
    }; // AvailableMonitors END


//  ============  GLFW Window wrapper  ============  \\

    // TODO: all sorts of setters/getters, framebuffer, other necessary stuff
    struct Window {
        GLFWwindow *window;

        Window(GLFWwindow *_window = nullptr) : window{_window} {}
        Window(uint32_t _w, uint32_t _h, const char *_name, GLFWmonitor *_m = nullptr,
               GLFWwindow *_window_to_share = nullptr) {
            window = glfwCreateWindow(_w, _h, _name, _m, _window_to_share);
            if (window == nullptr)
                throw std::runtime_error(SVKFW_WRAPERR("Window constructor","could not create GLFW window"));
        }
        Window(Window &&_window) : window{_window.window} {
            _window.window = nullptr;
        }
        ~Window() {
            if (window != nullptr)
                glfwDestroyWindow(window);
        }


        bool shouldClose() { return glfwWindowShouldClose(window); }
        void setClose(bool _close = true) { glfwSetWindowShouldClose(window, _close ? GLFW_TRUE : GLFW_FALSE); }

        vec2i getPos() const {
            int x, y;
            glfwGetWindowPos(window, &x, &y);
            return { x, y };
        }
        void setPos(const vec2i &_ul) {
            glfwSetWindowPos(window, _ul.x, _ul.y);
        }

        vec2i getResolution() const {
            int x, y;
            glfwGetWindowSize(window, &x, &y);
            return { x, y };
        }
        void setResolution(const vec2u &_reso) {
            vec2i __r{ getResolution() };
            if (_reso.x) __r.x = (int) _reso.x;
            if (_reso.y) __r.y = (int) _reso.y;
            glfwSetWindowSize(window, __r.x, __r.y);
        }

        vec4i getPosReso() const {
            vec4i  __res;
            __res.subvec<'x','y'>() = getPos();
            __res.subvec<'z','w'>() = getResolution();
            return __res;
        }

        vec2i getFramebufferSize() const {
            int x, y;
            glfwGetFramebufferSize(window, &x, &y);
            return { x, y };
        }

        void limitResize(const vec2i &_min = GLFW_DONT_CARE, const vec2i &_max = GLFW_DONT_CARE) {
            glfwSetWindowSizeLimits(window, _min.x, _min.y, _max.x, _max.y);
        }

        void fixateCurrentAspectRatio() {
            vec2i __r{ getResolution() };
            glfwSetWindowAspectRatio(window, __r.x, __r.y);
        }
        void setAspectRatio(const vec2i &_asp_ratio = GLFW_DONT_CARE) {
            glfwSetWindowAspectRatio(window, _asp_ratio.x, _asp_ratio.y);
        }

        // TODO:
        void setIcon() {
            GLFWimage img;
        }


// FullScreen|Windowed mode

        // Returns window resolution before changing to full screen
        vec4i toFullScreen(Monitor _m = nullptr) {
            if (_m == nullptr)
                _m = glfwGetPrimaryMonitor();
            vec4i __ul_reso = _m.getWorkarea();
            vec4i __res = getPosReso();
            glfwSetWindowMonitor(window, _m, __ul_reso.x, __ul_reso.y, __ul_reso.z, __ul_reso.w, GLFW_DONT_CARE);
            return __res;
        }

        // Returns window resolution before changing to full screen
        vec4i toWindowedFullScreen(Monitor _m = nullptr) {
            if (_m == nullptr)
                _m = glfwGetPrimaryMonitor();
            const GLFWvidmode* __mode = _m.getVideoMode();
            vec4i __res = getPosReso();
            glfwSetWindowMonitor(window, _m, 0, 0, __mode->width, __mode->height, __mode->refreshRate);
            return __res;
        }

        void toWindowed() {
            vec2i __ul{}, __reso{};
            glfwGetWindowSize(window, &__reso.x, &__reso.y);
            glfwSetWindowMonitor(window, NULL, __ul.x, __ul.y, __reso.x, __reso.y, GLFW_DONT_CARE);
        }

        void toWindowedWithParams(const vec2i &_pos, const vec2i &_reso, int _refresh_rate = GLFW_DONT_CARE) {
            glfwSetWindowMonitor(window, NULL, _pos.x, _pos.y, _reso.x, _reso.y, _refresh_rate);
        }

        void toWindowedWithParams(const vec4i &_pos_reso, int _refresh_rate = GLFW_DONT_CARE) {
            glfwSetWindowMonitor(window, NULL, _pos_reso.x, _pos_reso.y, _pos_reso.z, _pos_reso.w, _refresh_rate);
        }


// Static member functions

        static void Hint(int _hint, int _val) {
            glfwWindowHint(_hint, _val);
        }
        static void Hints(const std::initializer_list<vec2i> &_hint_val_pairs) {
            for (vec2i _pair : _hint_val_pairs)
                glfwWindowHint(_pair.x, _pair.y);
        }


        static Window Windowed(const char *_name, uint32_t _w, uint32_t _h,
                               GLFWwindow *_shared_win = nullptr) {
            return { _w, _h, _name, nullptr, _shared_win };
        }
        // TODO:
        static Window FullScreen(const char *_name, uint32_t _w, uint32_t _h, Monitor _m = nullptr,
                                 GLFWwindow *_shared_win = nullptr) {
            return { _w, _h, _name, _m == nullptr ? glfwGetPrimaryMonitor() : _m.monitor, _shared_win };
        }
        // TODO:
        static Window WindowedFullScreen(const char *_name, uint32_t _w, uint32_t _h, Monitor _m = nullptr,
                                         GLFWwindow *_shared_win = nullptr) {
            return { _w, _h, _name, _m == nullptr ? glfwGetPrimaryMonitor() : _m.monitor, _shared_win };
        }
    }; // Window END


//  ============  GLFW Init/Terminate wrapper  ============  \\

    // a global variable of this class "automatically" turns GLFW on/off (heresy)
    struct GLFWInitClass {
         GLFWInitClass() {
            glfwGetError(0);
            if (!glfwInit()) {
                throw std::runtime_error(SVKFW_WRAPERR("GLFWInitClass", "could not initialize GLFW"));
            }
            AvailableMonitors::all_init();
        }
        ~GLFWInitClass() {
            if (glfwGetError(0) != GLFW_NOT_INITIALIZED)
                glfwTerminate();
        }
    } glfw_init; // GLFWInitClass END
}; // Simple END


#endif