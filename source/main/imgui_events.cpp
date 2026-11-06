#include "main/imgui_events.h"

namespace Simple {
    namespace Event {

        ImGuiTestWindow::ImGuiTestWindow() {}

        ImGuiTestWindow::~ImGuiTestWindow() {}

        bool ImGuiTestWindow::begin() {
            test_window.begin();
            test_drag_float1.begin();
            test_drag_float2.begin();
            test_slider_float.begin();
            test_radio_button.renderFull();
            return true;
        }

        void ImGuiTestWindow::end() {
            test_window.end();
        }

        void ImGuiTestWindow::render() {

        }

        void ImGuiTestWindow::renderFull() {
            begin();
            end();
        }
    }; // Event END
}; // Simple END
