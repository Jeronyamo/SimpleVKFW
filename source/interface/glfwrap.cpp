// Note: volk implementation (Vulkan function pointers) is compiled once, here
#define VOLK_IMPLEMENTATION
#define VK_NO_PROTOTYPES
#include "volk.h"

#include "interface/glfwrap.h"

namespace Simple {

    Monitor::Monitor(GLFWmonitor* _m) : monitor{_m} {}

    Monitor::operator GLFWmonitor*() { return monitor; }

    const char* Monitor::getName() { return glfwGetMonitorName(monitor); }

    vec2i Monitor::getPos() {
        vec2i _tmp{0};
        glfwGetMonitorPos(monitor, &_tmp.x, &_tmp.y);
        return _tmp;
    }

    vec4i Monitor::getWorkarea() {
        vec4i _tmp{0};
        glfwGetMonitorWorkarea(monitor, &_tmp.x, &_tmp.y, &_tmp.z, &_tmp.w);
        return _tmp;
    }

    vec2i Monitor::getPhysicalSize() {
        vec2i _tmp{0};
        glfwGetMonitorPhysicalSize(monitor, &_tmp.x, &_tmp.y);
        return _tmp;
    }

    vec2f Monitor::getContentScale() {
        vec2f _tmp{0.f};
        glfwGetMonitorContentScale(monitor, &_tmp.x, &_tmp.y);
        return _tmp;
    }

    void  Monitor::setUserPointer(void *_ptr) {
        glfwSetMonitorUserPointer(monitor, _ptr);
    }

    void* Monitor::getUserPointer() {
        return glfwGetMonitorUserPointer(monitor);
    }

    const GLFWvidmode* Monitor::getVideoMode() {
        return glfwGetVideoMode(monitor);
    }

    const GLFWvidmode* Monitor::getVideoModes(int &count) {
        return glfwGetVideoModes(monitor, &count);
    }

    void Monitor::setGamma(float _gamma) {
        glfwSetGamma(monitor, _gamma);
    }

    const GLFWgammaramp* Monitor::getGammaRamp() {
        return glfwGetGammaRamp(monitor);
    }

    void Monitor::setGammaRamp(const GLFWgammaramp* _ramp) {
        glfwSetGammaRamp(monitor, _ramp);
    }

    namespace AvailableMonitors {

        Monitor primary = nullptr;

        std::unordered_set<GLFWmonitor*> all;

        GLFWmonitorfun previous_callback = nullptr;

        void monitorCallbackAllUpd(GLFWmonitor* monitor, int event) {
            primary = glfwGetPrimaryMonitor();
            if (previous_callback != nullptr)
                previous_callback(monitor, event);

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
            primary = glfwGetPrimaryMonitor();
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


    Window::Window(GLFWwindow *_window) : window{_window} {}

    Window::Window(uint32_t _w, uint32_t _h, const char *_name, GLFWmonitor *_m,
           GLFWwindow *_window_to_share) {
        window = glfwCreateWindow(_w, _h, _name, _m, _window_to_share);
        if (window == nullptr)
            throw std::runtime_error(SVKFW_WRAPERR("Window constructor","could not create GLFW window"));
    }

    Window::Window(Window &&_window) : window{_window.window} {
        _window.window = nullptr;
    }

    Window::~Window() {}

    bool Window::shouldClose() { return glfwWindowShouldClose(window); }

    void Window::setClose(bool _close) { glfwSetWindowShouldClose(window, _close ? GLFW_TRUE : GLFW_FALSE); }

    void Window::destroy() { glfwDestroyWindow(window); }

    vec2i Window::getPos() const {
        int x, y;
        glfwGetWindowPos(window, &x, &y);
        return { x, y };
    }

    void Window::setPos(const vec2i &_ul) {
        glfwSetWindowPos(window, _ul.x, _ul.y);
    }

    vec2i Window::getResolution() const {
        int x, y;
        glfwGetWindowSize(window, &x, &y);
        return { x, y };
    }

    void Window::setResolution(const vec2u &_reso) {
        vec2i __r{ getResolution() };
        if (_reso.x) __r.x = (int) _reso.x;
        if (_reso.y) __r.y = (int) _reso.y;
        glfwSetWindowSize(window, __r.x, __r.y);
    }

    vec4i Window::getPosReso() const {
        vec4i  __res;
        __res.subvec<'x','y'>() = getPos();
        __res.subvec<'z','w'>() = getResolution();
        return __res;
    }

    vec2i Window::getFramebufferSize() const {
        int x, y;
        glfwGetFramebufferSize(window, &x, &y);
        return { x, y };
    }

    void Window::limitResize(const vec2i &_min, const vec2i &_max) {
        glfwSetWindowSizeLimits(window, _min.x, _min.y, _max.x, _max.y);
    }

    void Window::fixateCurrentAspectRatio() {
        vec2i __r{ getResolution() };
        glfwSetWindowAspectRatio(window, __r.x, __r.y);
    }

    void Window::setAspectRatio(const vec2i &_asp_ratio) {
        glfwSetWindowAspectRatio(window, _asp_ratio.x, _asp_ratio.y);
    }

    void Window::setIcon() {
        GLFWimage img;
    }

    vec4i Window::toFullScreen(Monitor _m) {
        if (_m == nullptr)
            _m = glfwGetPrimaryMonitor();
        vec4i __ul_reso = _m.getWorkarea();
        vec4i __res = getPosReso();
        glfwSetWindowMonitor(window, _m, __ul_reso.x, __ul_reso.y, __ul_reso.z, __ul_reso.w, GLFW_DONT_CARE);
        return __res;
    }

    vec4i Window::toWindowedFullScreen(Monitor _m) {
        if (_m == nullptr)
            _m = glfwGetPrimaryMonitor();
        const GLFWvidmode* __mode = _m.getVideoMode();
        vec4i __res = getPosReso();
        glfwSetWindowMonitor(window, _m, 0, 0, __mode->width, __mode->height, __mode->refreshRate);
        return __res;
    }

    void Window::toWindowed() {
        vec2i __ul{}, __reso{};
        glfwGetWindowSize(window, &__reso.x, &__reso.y);
        glfwSetWindowMonitor(window, NULL, __ul.x, __ul.y, __reso.x, __reso.y, GLFW_DONT_CARE);
    }

    void Window::toWindowedWithParams(const vec2i &_pos, const vec2i &_reso, int _refresh_rate) {
        glfwSetWindowMonitor(window, NULL, _pos.x, _pos.y, _reso.x, _reso.y, _refresh_rate);
    }

    void Window::toWindowedWithParams(const vec4i &_pos_reso, int _refresh_rate) {
        glfwSetWindowMonitor(window, NULL, _pos_reso.x, _pos_reso.y, _pos_reso.z, _pos_reso.w, _refresh_rate);
    }

    void Window::Hint(int _hint, int _val) {
        glfwWindowHint(_hint, _val);
    }

    void Window::Hints(const std::initializer_list<vec2i> &_hint_val_pairs) {
        for (vec2i _pair : _hint_val_pairs)
            glfwWindowHint(_pair.x, _pair.y);
    }

    Window Window::Windowed(const char *_name, uint32_t _w, uint32_t _h,
                           GLFWwindow *_shared_win) {
        return { _w, _h, _name, nullptr, _shared_win };
    }

    Window Window::FullScreen(const char *_name, uint32_t _w, uint32_t _h, Monitor _m,
                             GLFWwindow *_shared_win) {
        return { _w, _h, _name, _m == nullptr ? glfwGetPrimaryMonitor() : _m.monitor, _shared_win };
    }

    Window Window::WindowedFullScreen(const char *_name, uint32_t _w, uint32_t _h, Monitor _m,
                                     GLFWwindow *_shared_win) {
        return { _w, _h, _name, _m == nullptr ? glfwGetPrimaryMonitor() : _m.monitor, _shared_win };
    }

    GLFWInitClass::GLFWInitClass() {
       glfwGetError(0);
       if (!glfwInit()) {
           throw std::runtime_error(SVKFW_WRAPERR("GLFWInitClass", "could not initialize GLFW"));
       }
       is_initialized = true;
       AvailableMonitors::all_init();
    }

    GLFWInitClass::~GLFWInitClass() { terminate(); }

    void GLFWInitClass::terminate() {
        if (is_initialized)
            glfwTerminate();
        is_initialized = false;
    }

    GLFWInitClass glfw_init;
}; // Simple END
