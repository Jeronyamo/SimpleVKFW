#ifndef SVKFW_IMGUI_EVENTS_H
#define SVKFW_IMGUI_EVENTS_H

#include "interface/imgui_wrap.h"


namespace Simple {
    namespace Event {
        struct ImGuiTestWindow : ImGUI::WidgetItf {
            ImGUI::WidgetWindow test_window{"Test window"};
            ImGUI::WidgetDragFloat test_drag_float1{"Test float drag 1", 1, {}, -1.f, 1.f, 0.1f};
            ImGUI::WidgetDragFloat test_drag_float2{"Test float drag 2", 2, {}, -1.f, 1.f, 0.1f};
            ImGUI::WidgetSliderFloat test_slider_float{"Test float slider", 2, {}, -1.f, 1.f};
            ImGUI::WidgetRadioButton test_radio_button{"Test radio button"};

            ImGuiTestWindow();
            virtual ~ImGuiTestWindow() override;

            virtual bool begin() override;
            virtual void end() override;
            virtual void render() override;
            virtual void renderFull() override;
        }; // ImGuiTestWindow END
    }; // Event END
}; // Simple END

#endif