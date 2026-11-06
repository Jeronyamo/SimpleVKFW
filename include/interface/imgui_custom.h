#ifndef SVKFW_IMGUI_CUSTOM_H
#define SVKFW_IMGUI_CUSTOM_H

#ifndef IMGUI_IMPL_VULKAN_USE_VOLK
#define IMGUI_IMPL_VULKAN_USE_VOLK
#endif

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include <functional>
#include <vector>


// Custom ImGui widgets in the ImGui style: immediate mode functions, state in ImGui storage, no dependencies on SVKFW
namespace ImGuiCustom {
    // Round knob, drag up/down to change (Shift - fine steps). The label is drawn under the knob, the value in a tooltip.
    bool Knob(const char *_label, float *_value, float _min, float _max, const char *_format = "%.3f", float _radius = 0.f);

    // ADSR envelope graph with draggable points: attack end (time), decay end (time and sustain level), release end (time).
    // Times are in seconds, every segment takes up to a quarter of the width at '_max_time'. Returns true if changed.
    bool EnvelopeADSR(const char *_id, float *_attack, float *_decay, float *_sustain, float *_release,
                      float _max_time, bool _exp_curve, const ImVec2 &_size = ImVec2(0.f, 0.f));

    // Line plot of '_values'; longer arrays are drawn as min/max per pixel column (an envelope)
    void Waveform(const char *_id, const float *_values, int _count, float _min, float _max,
                  const ImVec2 &_size = ImVec2(0.f, 0.f), ImU32 _color = 0u);

    // Step plot editable by drawing with the mouse; values are normalized to [-1, 1] and snapped to '_levels' steps
    bool WavetableEditor(const char *_id, float *_table, int _length, int _levels, const ImVec2 &_size = ImVec2(0.f, 0.f));

    // Piano keyboard of '_octaves' octaves starting at note id '_first_note' (a C). Mouse presses set '_note_on'/'_note_off'
    // (-1 when nothing happened) and return true. '_is_lit' highlights notes, e.g. the playing ones.
    bool PianoKeyboard(const char *_id, int _first_note, int _octaves, const std::function<bool(int)> &_is_lit,
                       int *_note_on, int *_note_off, const ImVec2 &_size = ImVec2(0.f, 0.f));

    // Stereo level meter, levels in [0, 1]
    void VuMeter(const char *_id, float _left, float _right, const ImVec2 &_size = ImVec2(0.f, 0.f));


    // Text grid for trackers: rows x columns of cells, every cell made of sub-columns of fixed character widths
    struct TextGridStyle {
        std::vector<int>   sub_widths; // Characters per sub-column, e.g. {3, 2, 2, 3} for "C-4 01 40 4A6"
        std::vector<ImU32> sub_colors; // Text color per sub-column, 0 - default
        int beat_rows = 4, bar_rows = 16; // Highlighted rows, 0 - none
    }; // TextGridStyle END

    struct TextGridCursor {
        int row = 0, column = 0, sub = 0;
    }; // TextGridCursor END

    // '_sub_text(row, column, sub, buffer, buffer_size)' fills the text of a sub-column, '_column_name(column)' - the header.
    // '_play_row' is highlighted (-1 - none), '_scroll_to' scrolls the row to the middle (-1 - no scrolling).
    // Returns true when a cell was clicked (the cursor moved there).
    bool TextGrid(const char *_id, int _rows, int _columns, const std::function<const char*(int)> &_column_name,
                  const TextGridStyle &_style, const std::function<void(int, int, int, char*, int)> &_sub_text,
                  TextGridCursor &_cursor, int _play_row, int _scroll_to, const ImVec2 &_size = ImVec2(0.f, 0.f));


    // MMD interpolation curve: a cubic Bezier from (0,0) to (1,1) with control points (x1,y1), (x2,y2) in [0, 127] (VMD bytes),
    // the nearer control point follows the mouse while dragging. Returns true if changed
    bool BezierCurve127(const char *_id, int *_x1, int *_y1, int *_x2, int *_y2, const ImVec2 &_size = ImVec2(0.f, 0.f));


    // Dope sheet of keyframes (MMD style): one row per track with its keys as diamonds over a frame ruler.
    // Click: selects a key (or sets the current frame on an empty spot); drag a selected key: moves it (whole frames);
    // drag on the ruler: scrubs. Mouse wheel scrolls the rows, Ctrl + wheel zooms the frames, Shift + wheel (or the scroll bar
    // at the bottom) scrolls them
    struct TimelineRow {
        const char        *name = "";
        std::vector<float> frames;         // the keys of the track, sorted
        ImU32              color = 0u;     // diamond color, 0 - default
        bool               header = false; // a group title row without keys
    }; // TimelineRow END

    struct TimelineState {
        float first_frame = 0.f;     // the frame at the left edge
        float px_per_frame = 6.f;
        int   first_row = 0;         // scrolled rows
        float scroll_to = -1.f;      // >= 0: the next call scrolls the frames to show this one in the middle (then resets it)
        int   selected_row = -1, selected_key = -1;
        // Drag of the selected key (internal)
        bool  dragging = false;
        float drag_frame = 0.f;
        // Drag of the scroll bar (internal): the mouse offset in the grab, < 0 - not dragging
        float scroll_grab = -1.f;
    }; // TimelineState END

    struct TimelineEvent {
        enum Type { TL_EVENT_NONE = 0, TL_EVENT_SEEK, TL_EVENT_SELECT, TL_EVENT_MOVE_KEY, TL_EVENT_DOUBLE_CLICK } type = TL_EVENT_NONE;
        int   row = -1, key = -1;
        float frame = 0.f;           // SEEK: the new current frame; MOVE_KEY: where the key goes; DOUBLE_CLICK: the clicked frame;
                                     // SELECT: the key's frame, or the clicked frame (-1 if the name was clicked)
    }; // TimelineEvent END

    // '_current' is the play head; '_length' (frames) limits the ruler (it extends past the last key anyway).
    // Returns true when '_event' has something
    bool Timeline(const char *_id, const std::vector<TimelineRow> &_rows, float _current, float _length, TimelineState &_state,
                  TimelineEvent &_event, const ImVec2 &_size = ImVec2(0.f, 0.f));
}; // ImGuiCustom END

#endif
