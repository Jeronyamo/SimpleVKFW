#ifndef SVKFW_GLFWRAP_H
#define SVKFW_GLFWRAP_H

// Note: volk implementation is compiled in glfwrap.cpp
#define VK_NO_PROTOTYPES

#include "volk.h"
#include "GLFW/glfw3.h"
#include "common/basic_math.h"
#include "main/event_handler.h"
#include "math/vectors.h"

#include <unordered_set>


namespace Simple {

//  ============  GLFW Monitor wrapper  ============  \\

    struct Monitor {
        GLFWmonitor* monitor;

        Monitor(GLFWmonitor* _m);

        operator GLFWmonitor*();


        const char* getName();
        vec2i getPos();
        vec4i getWorkarea();
        vec2i getPhysicalSize();
        vec2f getContentScale();
        void  setUserPointer(void *_ptr);
        void* getUserPointer();
        const GLFWvidmode* getVideoMode();
        const GLFWvidmode* getVideoModes(int &count);
        void setGamma(float _gamma);
        const GLFWgammaramp* getGammaRamp();
        void setGammaRamp(const GLFWgammaramp* _ramp);
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


//  ============  GLFW Window wrapper  ============  \\

    // TODO: all sorts of setters/getters, framebuffer, other necessary stuff
    struct Window {
        GLFWwindow *window;

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

        // TODO:
        void setIcon();


// FullScreen|Windowed mode

        // Returns window resolution before changing to full screen
        vec4i toFullScreen(Monitor _m = nullptr);

        // Returns window resolution before changing to full screen
        vec4i toWindowedFullScreen(Monitor _m = nullptr);

        void toWindowed();

        void toWindowedWithParams(const vec2i &_pos, const vec2i &_reso, int _refresh_rate = GLFW_DONT_CARE);

        void toWindowedWithParams(const vec4i &_pos_reso, int _refresh_rate = GLFW_DONT_CARE);


// Static member functions

        static void Hint(int _hint, int _val);
        static void Hints(const std::initializer_list<vec2i> &_hint_val_pairs);


        static Window Windowed(const char *_name, uint32_t _w, uint32_t _h,
                               GLFWwindow *_shared_win = nullptr);
        // TODO:
        static Window FullScreen(const char *_name, uint32_t _w, uint32_t _h, Monitor _m = nullptr,
                                 GLFWwindow *_shared_win = nullptr);
        // TODO:
        static Window WindowedFullScreen(const char *_name, uint32_t _w, uint32_t _h, Monitor _m = nullptr,
                                         GLFWwindow *_shared_win = nullptr);
    }; // Window END


//  ============  GLFW Init/Terminate wrapper  ============  \\

    // a global variable of this class "automatically" turns GLFW on/off (heresy)
    struct GLFWInitClass {
        bool is_initialized = false;

         GLFWInitClass();
        ~GLFWInitClass();

        // Can be called manually to terminate GLFW before the program exit
        void terminate();
    }; // GLFWInitClass END
    extern GLFWInitClass glfw_init;

}; // Simple END


#endif