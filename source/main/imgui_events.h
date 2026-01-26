#ifndef SVKFW_IMGUI_EVENTS_H
#define SVKFW_IMGUI_EVENTS_H

#include "interface/imgui_wrap.h"
#include "data/filetypes.h"


namespace Simple {
    namespace Event {
        struct ImGuiWidgetLoadFile : ImGUI::WidgetItf {
            File::ReaderWriterVMD motion_file, camera_file;

            ImGUI::WidgetWindow editor_window{"VMD Editor"};
            ImGUI::WidgetInputText motion_path_input{"Motion path:"}, camera_path_input{"Camera path:"};
            ImGUI::WidgetButton edit_button{"Edit file"};
            ImGUI::WidgetCheckbox fix_blink{"Fix blink", true}, reflect{"Reflect", true};

            ImGuiWidgetLoadFile() {}

            virtual bool begin() override {
                editor_window.begin();
                motion_path_input.begin();
                camera_path_input.begin();
                fix_blink.begin();
                reflect.begin();
                return edit_button.begin();
            }
            virtual void end() override {
                editor_window.end();
            }
            virtual void render() override {
                if (std::strlen(motion_path_input.buf_str) > 0) {
                    File::ContentVMD::ModifyData(Util::stringStrip(motion_path_input.buf_str), fix_blink.value, reflect.value);
                }
            }
            virtual void renderFull() override {
                if (begin()) render();
                end();
            }
            virtual ImGuiWindowFlags reqWindowFlags() { return 0; }
        }; // ImGuiWidgetLoadFile END
    }; // Event END
}; // Simple END

#endif