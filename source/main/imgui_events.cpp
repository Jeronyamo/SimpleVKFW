#include "main/imgui_events.h"

namespace Simple {
    namespace Event {

        ImGuiWidgetLoadFile::ImGuiWidgetLoadFile() {}

        bool ImGuiWidgetLoadFile::begin() {
            editor_window.begin();
            motion_path_input.begin();
            camera_path_input.begin();
            fix_blink.begin();
            reflect.begin();
            reinterp.begin();
            return edit_button.begin();
        }

        void ImGuiWidgetLoadFile::end() {
            editor_window.end();
        }

        void ImGuiWidgetLoadFile::render() {
            if (std::strlen(motion_path_input.buf_str) > 0) {
                File::ContentVMD::ModifyData(Util::stringStrip(motion_path_input.buf_str), fix_blink.value, reflect.value, reinterp.value);
            }
            if (std::strlen(camera_path_input.buf_str) > 0) {
                File::ContentVMD::ModifyCamera(Util::stringStrip(camera_path_input.buf_str), reinterp.value);
            }
        }

        void ImGuiWidgetLoadFile::renderFull() {
            if (begin()) render();
            end();
        }

        ImGuiWindowFlags ImGuiWidgetLoadFile::reqWindowFlags() { return 0; }

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
