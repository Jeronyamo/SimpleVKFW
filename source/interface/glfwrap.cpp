// Note: volk implementation (Vulkan function pointers) is compiled once, here
#define VOLK_IMPLEMENTATION
#define VK_NO_PROTOTYPES
#include "volk.h"

#include "interface/glfwrap.h"

namespace Simple {

    Monitor::Monitor(GLFWmonitor* _m) : monitor{_m} {}

    Monitor::operator GLFWmonitor*() const { return monitor; }

    const char* Monitor::getName() const { return glfwGetMonitorName(monitor); }

    vec2i Monitor::getPos() const {
        vec2i _tmp{0};
        glfwGetMonitorPos(monitor, &_tmp.x, &_tmp.y);
        return _tmp;
    }

    vec4i Monitor::getWorkarea() const {
        vec4i _tmp{0};
        glfwGetMonitorWorkarea(monitor, &_tmp.x, &_tmp.y, &_tmp.z, &_tmp.w);
        return _tmp;
    }

    vec2i Monitor::getPhysicalSize() const {
        vec2i _tmp{0};
        glfwGetMonitorPhysicalSize(monitor, &_tmp.x, &_tmp.y);
        return _tmp;
    }

    vec2f Monitor::getContentScale() const {
        vec2f _tmp{0.f};
        glfwGetMonitorContentScale(monitor, &_tmp.x, &_tmp.y);
        return _tmp;
    }

    void  Monitor::setUserPointer(void *_ptr) const {
        glfwSetMonitorUserPointer(monitor, _ptr);
    }

    void* Monitor::getUserPointer() const {
        return glfwGetMonitorUserPointer(monitor);
    }

    const GLFWvidmode* Monitor::getVideoMode() const {
        return glfwGetVideoMode(monitor);
    }

    const GLFWvidmode* Monitor::getVideoModes(int &count) const {
        return glfwGetVideoModes(monitor, &count);
    }

    void Monitor::setGamma(float _gamma) const {
        glfwSetGamma(monitor, _gamma);
    }

    const GLFWgammaramp* Monitor::getGammaRamp() const {
        return glfwGetGammaRamp(monitor);
    }

    void Monitor::setGammaRamp(const GLFWgammaramp* _ramp) const {
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

    Window::Window(Window &&_window) : window{_window.window}, windowed_pos_reso{_window.windowed_pos_reso} {
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

    void Window::setIcon(const std::vector<GLFWImageView> &_images) {
        std::vector<GLFWimage> __images(_images.size());
        for (size_t i = 0u; i < _images.size(); ++i) __images[i] = _images[i].image;
        glfwSetWindowIcon(window, (int)__images.size(), __images.empty() ? nullptr : __images.data());
    }

    void Window::setIcon(const GLFWImageView &_image) {
        glfwSetWindowIcon(window, 1, &_image.image);
    }

    void Window::resetIcon() {
        glfwSetWindowIcon(window, 0, nullptr);
    }

    bool Window::isFullScreen() const { return glfwGetWindowMonitor(window) != nullptr; }

    GLFWmonitor *Window::getMonitor() const { return glfwGetWindowMonitor(window); }

    vec4i Window::toFullScreen(Monitor _m, const vec2i &_reso, int _refresh_rate) {
        if (_m == nullptr)
            _m = glfwGetPrimaryMonitor();
        // Note: the size must be a video mode (the work area excludes the task bar, so it usually isn't one)
        const GLFWvidmode* __mode = _m.getVideoMode();
        const vec2i __reso{ _reso.x > 0 ? _reso.x : __mode->width, _reso.y > 0 ? _reso.y : __mode->height };
        vec4i __res = getPosReso();
        if (!isFullScreen()) windowed_pos_reso = __res;
        glfwSetWindowMonitor(window, _m, 0, 0, __reso.x, __reso.y, _refresh_rate);
        return __res;
    }

    vec4i Window::toWindowedFullScreen(Monitor _m) {
        if (_m == nullptr)
            _m = glfwGetPrimaryMonitor();
        // Note: with the current video mode (size, refresh rate) GLFW doesn't change the mode, which is GLFW's windowed full screen
        const GLFWvidmode* __mode = _m.getVideoMode();
        vec4i __res = getPosReso();
        if (!isFullScreen()) windowed_pos_reso = __res;
        glfwSetWindowMonitor(window, _m, 0, 0, __mode->width, __mode->height, __mode->refreshRate);
        return __res;
    }

    void Window::toWindowed() {
        vec4i __pos_reso = windowed_pos_reso;
        if (__pos_reso.x == INT32_MIN) {
            // Unknown: 3/4 of the work area of the window's monitor (or the primary one), centered
            Monitor __m = isFullScreen() ? getMonitor() : glfwGetPrimaryMonitor();
            const vec4i __area = __m.getWorkarea();
            __pos_reso.z = __area.z * 3 / 4;
            __pos_reso.w = __area.w * 3 / 4;
            __pos_reso.x = __area.x + (__area.z - __pos_reso.z) / 2;
            __pos_reso.y = __area.y + (__area.w - __pos_reso.w) / 2;
        }
        glfwSetWindowMonitor(window, nullptr, __pos_reso.x, __pos_reso.y, __pos_reso.z, __pos_reso.w, GLFW_DONT_CARE);
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
        if (_m == nullptr)
            _m = glfwGetPrimaryMonitor();
        const GLFWvidmode* __mode = _m.getVideoMode();
        return { _w ? _w : (uint32_t)__mode->width, _h ? _h : (uint32_t)__mode->height, _name, _m.monitor, _shared_win };
    }

    Window Window::WindowedFullScreen(const char *_name, Monitor _m, GLFWwindow *_shared_win) {
        if (_m == nullptr)
            _m = glfwGetPrimaryMonitor();
        // Note: hints matching the current video mode keep the mode (GLFW's windowed full screen)
        const GLFWvidmode* __mode = _m.getVideoMode();
        glfwWindowHint(GLFW_RED_BITS,     __mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS,   __mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS,    __mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, __mode->refreshRate);
        return { (uint32_t)__mode->width, (uint32_t)__mode->height, _name, _m.monitor, _shared_win };
    }

    GLFWImageView::GLFWImageView() {
        image.width  = 0;
        image.height = 0;
        image.pixels = nullptr;
    }

    GLFWImageView::GLFWImageView(const Img::Image4u &_img) {
        static_assert(sizeof(Img::Image4u::ColorType) == 4u, "Img::Image4u texels must be tightly packed RGBA8");
        image.width  = _img.width;
        image.height = _img.height;
        image.pixels = _img.img.empty() ? nullptr : const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(_img.img.data())); // GLFW only reads
    }

    GLFWImageView::GLFWImageView(const GLFWImageView &_view) : image(_view.image), storage(_view.storage) {
        if (!storage.empty()) image.pixels = storage.data();
    }

    GLFWImageView& GLFWImageView::operator=(const GLFWImageView &_view) {
        if (this == &_view) return *this;
        image   = _view.image;
        storage = _view.storage;
        if (!storage.empty()) image.pixels = storage.data();
        return *this;
    }

    GLFWImageView::operator const GLFWimage&() const { return image; }

    unsigned char GLFWImageView::toByte(unsigned char _value) { return _value; }

    unsigned char GLFWImageView::toByte(float _value) {
        return (unsigned char)(255.f * (_value < 0.f ? 0.f : (_value > 1.f ? 1.f : _value)) + 0.5f);
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
}; // Simple END
