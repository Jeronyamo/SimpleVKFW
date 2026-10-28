#ifndef SVKFW_GLFWRAP_H
#define SVKFW_GLFWRAP_H

// Note: volk implementation is compiled in glfwrap.cpp
#define VK_NO_PROTOTYPES

#include "volk.h"
#include "GLFW/glfw3.h"
#include "common/basic_math.h"
#include "main/event_handler.h"
#include "math/vectors.h"
#include "main/image.h"

#include <unordered_set>
#include <vector>


namespace Simple {

//  ============  GLFW Monitor wrapper  ============  \\

    struct Monitor {
        GLFWmonitor* monitor;

        Monitor(GLFWmonitor* _m);

        operator GLFWmonitor*() const;


        const char* getName() const;
        vec2i getPos() const;
        vec4i getWorkarea() const;
        vec2i getPhysicalSize() const;
        vec2f getContentScale() const;
        // Note: setters are const too: they change the GLFW monitor, not this handle
        void  setUserPointer(void *_ptr) const;
        void* getUserPointer() const;
        const GLFWvidmode* getVideoMode() const;
        const GLFWvidmode* getVideoModes(int &count) const;
        void setGamma(float _gamma) const;
        const GLFWgammaramp* getGammaRamp() const;
        void setGammaRamp(const GLFWgammaramp* _ramp) const;
    }; // Monitor END


    namespace AvailableMonitors {
        extern Monitor primary; // Note: set in 'all_init()', GLFW must be initialized before the call
        extern std::unordered_set<GLFWmonitor*> all;
        extern GLFWmonitorfun previous_callback;


        // Note: if a monitor callback is replaced by another one, the new callback should call this function too
        void monitorCallbackAllUpd(GLFWmonitor* monitor, int event);

        void all_refresh();

        void all_init();
    }; // AvailableMonitors END


//  ============  GLFW image (icons, cursors)  ============  \\

    // GLFW images are always 8-bit RGBA (not premultiplied), rows from the top. An Img::Image4u is used without a copy (the image
    // must outlive the view and not be resized); other images are converted into 'storage': 1 channel -> gray, 2 -> gray + alpha,
    // 3 -> RGB + opaque alpha; float channels are clamped to [0, 1] and scaled to [0, 255].
    // Note: GLFW copies the pixels in the calls that take images, so a temporary view is enough there
    struct GLFWImageView {
        GLFWimage image;
        std::vector<unsigned char> storage; // converted images only

        GLFWImageView();
        GLFWImageView(const Img::Image4u &_img);
        template <typename T, int Ch>
        GLFWImageView(const Img::Image<T, Ch> &_img) {
            typedef typename Img::Image<T, Ch>::ColorType   ColorType;
            typedef typename Img::Image<T, Ch>::ChannelType ChannelType;
            static_assert(sizeof(ColorType) == Ch * sizeof(ChannelType), "Img::Image texels must be tightly packed");
            image.width  = _img.width;
            image.height = _img.height;
            storage.resize((size_t)_img.width * _img.height * 4u);
            for (size_t i = 0u; i < _img.img.size(); ++i) {
                const ChannelType *__src = reinterpret_cast<const ChannelType*>(&_img.img[i]);
                unsigned char     *__dst = &storage[4u * i];
                if (Ch <= 2) __dst[0] = __dst[1] = __dst[2] = toByte(__src[0]);
                else       { __dst[0] = toByte(__src[0]); __dst[1] = toByte(__src[1]); __dst[2] = toByte(__src[2]); }
                __dst[3] = Ch == 2 ? toByte(__src[1]) : (Ch == 4 ? toByte(__src[Ch - 1]) : (unsigned char)255u);
            }
            image.pixels = storage.data();
        }
        GLFWImageView(const GLFWImageView &_view);
        GLFWImageView& operator=(const GLFWImageView &_view);

        operator const GLFWimage&() const;

        static unsigned char toByte(unsigned char _value);
        static unsigned char toByte(float _value);
    }; // GLFWImageView END


//  ============  GLFW Window wrapper  ============  \\

    // TODO: all sorts of setters/getters, framebuffer, other necessary stuff
    struct Window {
        GLFWwindow *window;
        // The position and size before the last switch to full screen (see 'toWindowed'); x == INT32_MIN if unknown
        vec4i windowed_pos_reso{INT32_MIN, 0, 0, 0};

        Window(GLFWwindow *_window = nullptr);
        Window(uint32_t _w, uint32_t _h, const char *_name, GLFWmonitor *_m = nullptr,
               GLFWwindow *_window_to_share = nullptr);
        Window(Window &&_window);
       ~Window();


        bool shouldClose();
        void setClose(bool _close = true);
        void destroy();

        vec2i getPos() const;
        void setPos(const vec2i &_ul);

        vec2i getResolution() const;
        void setResolution(const vec2u &_reso);

        vec4i getPosReso() const;

        vec2i getFramebufferSize() const;

        void limitResize(const vec2i &_min = GLFW_DONT_CARE, const vec2i &_max = GLFW_DONT_CARE);

        void fixateCurrentAspectRatio();
        void setAspectRatio(const vec2i &_asp_ratio = GLFW_DONT_CARE);

        // Several sizes let the system pick the best one (e.g. 16x16, 32x32, 48x48)
        // Note: no effect on macOS (no window icons); on Wayland GLFW reports GLFW_FEATURE_UNAVAILABLE
        void setIcon(const std::vector<GLFWImageView> &_images);
        void setIcon(const GLFWImageView &_image);
        // The default icon of the system
        void resetIcon();


// FullScreen|Windowed mode

        // The window has a monitor (full screen or windowed full screen)
        bool isFullScreen() const;
        GLFWmonitor *getMonitor() const;

        // Exclusive full screen with a video mode of the monitor ('_m' == nullptr: primary). Returns the window position and size
        // before the switch (also kept in 'windowed_pos_reso' if the window was windowed).
        // '_reso' == 0: the current video mode of the monitor (otherwise GLFW picks the closest video mode)
        vec4i toFullScreen(Monitor _m = nullptr, const vec2i &_reso = 0, int _refresh_rate = GLFW_DONT_CARE);

        // Windowed full screen ("borderless"): the current video mode of the monitor, so the mode doesn't change.
        // Returns the window position and size before the switch (also kept in 'windowed_pos_reso')
        vec4i toWindowedFullScreen(Monitor _m = nullptr);

        // Back to the position and size before the last switch to full screen (if unknown: 3/4 of the monitor work area, centered).
        // Note: the position and size are kept in this object, so switch through the same 'Window'
        void toWindowed();

        void toWindowedWithParams(const vec2i &_pos, const vec2i &_reso, int _refresh_rate = GLFW_DONT_CARE);

        void toWindowedWithParams(const vec4i &_pos_reso, int _refresh_rate = GLFW_DONT_CARE);


// Static member functions

        static void Hint(int _hint, int _val);
        static void Hints(const std::initializer_list<vec2i> &_hint_val_pairs);


        static Window Windowed(const char *_name, uint32_t _w, uint32_t _h,
                               GLFWwindow *_shared_win = nullptr);
        // Exclusive full screen with the closest video mode to '_w' x '_h' (0: the current mode); '_m' == nullptr: primary monitor
        static Window FullScreen(const char *_name, uint32_t _w = 0u, uint32_t _h = 0u, Monitor _m = nullptr,
                                 GLFWwindow *_shared_win = nullptr);
        // Windowed full screen: the current video mode of the monitor (sets the color bits and refresh rate hints)
        static Window WindowedFullScreen(const char *_name, Monitor _m = nullptr, GLFWwindow *_shared_win = nullptr);
    }; // Window END


//  ============  GLFW Init/Terminate wrapper  ============  \\

    struct GLFWInitClass {
        bool is_initialized = false;

         GLFWInitClass();
        ~GLFWInitClass();

        // Can be called manually to terminate GLFW before the program exits
        void terminate();
    }; // GLFWInitClass END
}; // Simple END


#endif