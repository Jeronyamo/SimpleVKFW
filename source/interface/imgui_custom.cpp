#include "interface/imgui_custom.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>


namespace ImGuiCustom {
    static const float Pi = 3.14159265f;

    static ImVec2 operator+(const ImVec2 &_a, const ImVec2 &_b) { return ImVec2(_a.x + _b.x, _a.y + _b.y); }
    static ImVec2 operator-(const ImVec2 &_a, const ImVec2 &_b) { return ImVec2(_a.x - _b.x, _a.y - _b.y); }

    static float clampf(float _v, float _min, float _max) { return _v < _min ? _min : (_v > _max ? _max : _v); }

    // Fills zero size components: the full available width, '_default_h' text lines of height
    static ImVec2 itemSize(const ImVec2 &_size, float _default_h) {
        return ImVec2(_size.x > 0.f ? _size.x : ImGui::GetContentRegionAvail().x,
                      _size.y > 0.f ? _size.y : ImGui::GetTextLineHeight() * _default_h);
    }


    bool Knob(const char *_label, float *_value, float _min, float _max, const char *_format, float _radius) {
        ImGuiIO &__io = ImGui::GetIO();
        const ImGuiStyle &__style = ImGui::GetStyle();
        const float __radius = _radius > 0.f ? _radius : ImGui::GetFontSize() * 1.1f;
        const ImVec2 __pos = ImGui::GetCursorScreenPos();
        const ImVec2 __center = __pos + ImVec2(__radius, __radius);
        // Note: text after "##" is the hidden id part
        const char *__label_end = std::strstr(_label, "##");
        if (__label_end == nullptr) __label_end = _label + std::strlen(_label);
        const ImVec2 __label_size = ImGui::CalcTextSize(_label, __label_end);
        const float __width = std::max(__radius * 2.f, __label_size.x);

        ImGui::InvisibleButton(_label, ImVec2(__width, __radius * 2.f + __style.ItemInnerSpacing.y + __label_size.y));
        bool __changed = false;
        if (ImGui::IsItemActive() && __io.MouseDelta.y != 0.f) {
            const float __speed = (_max - _min) / (__io.KeyShift ? 1000.f : 200.f);
            *_value = clampf(*_value - __io.MouseDelta.y * __speed, _min, _max);
            __changed = true;
        }
        if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text(_format, *_value);
            ImGui::EndTooltip();
        }

        const float __t = _max > _min ? (*_value - _min) / (_max - _min) : 0.f;
        const float __angle_min = Pi * 0.75f, __angle_max = Pi * 2.25f;
        const float __angle = __angle_min + (__angle_max - __angle_min) * __t;
        const ImVec2 __knob_center = ImVec2(__pos.x + __width * 0.5f, __center.y);
        ImDrawList *__draw = ImGui::GetWindowDrawList();
        const ImU32 __col_bg = ImGui::GetColorU32(ImGui::IsItemActive() ? ImGuiCol_FrameBgActive : (ImGui::IsItemHovered() ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg));
        __draw->AddCircleFilled(__knob_center, __radius, __col_bg, 24);
        __draw->PathArcTo(__knob_center, __radius * 0.85f, __angle_min, __angle, 24);
        __draw->PathStroke(ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 0, __radius * 0.15f);
        __draw->AddLine(ImVec2(__knob_center.x + std::cos(__angle) * __radius * 0.3f, __knob_center.y + std::sin(__angle) * __radius * 0.3f),
                        ImVec2(__knob_center.x + std::cos(__angle) * __radius * 0.8f, __knob_center.y + std::sin(__angle) * __radius * 0.8f),
                        ImGui::GetColorU32(ImGuiCol_Text), 2.f);
        __draw->AddText(ImVec2(__pos.x + (__width - __label_size.x) * 0.5f, __pos.y + __radius * 2.f + __style.ItemInnerSpacing.y),
                        ImGui::GetColorU32(ImGuiCol_Text), _label, __label_end);
        return __changed;
    }


    static float envelopeLevel(float _t, float _seg_time, float _from, float _to, bool _exp) {
        if (_seg_time <= 0.f) return _to;
        const float __x = clampf(_t / _seg_time, 0.f, 1.f);
        if (!_exp) return _from + (_to - _from) * __x;
        // Same shape as the synth: -60 dB at the end of the segment
        return _to + (_from - _to) * std::exp(-6.9077553f * __x);
    }

    bool EnvelopeADSR(const char *_id, float *_attack, float *_decay, float *_sustain, float *_release,
                      float _max_time, bool _exp_curve, const ImVec2 &_size) {
        const ImVec2 __size = itemSize(_size, 6.f);
        const ImVec2 __pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton(_id, __size);

        ImGuiStorage *__storage = ImGui::GetStateStorage();
        const ImGuiID __handle_id = ImGui::GetID(_id);
        const float __pad = 6.f;
        const float __w = __size.x - 2.f * __pad, __h = __size.y - 2.f * __pad;
        const float __seg = __w * 0.25f / std::max(_max_time, 1e-3f); // pixels per second
        const float __x0 = __pos.x + __pad, __y_top = __pos.y + __pad, __y_bottom = __pos.y + __pad + __h;
        auto __level_y = [&](float _level) { return __y_bottom - clampf(_level, 0.f, 1.f) * __h; };

        ImVec2 __points[4] = {
            ImVec2(__x0 + *_attack * __seg, __y_top),                                       // Attack end
            ImVec2(__x0 + (*_attack + *_decay) * __seg, __level_y(*_sustain)),              // Decay end
            ImVec2(__x0 + (*_attack + *_decay) * __seg + __w * 0.25f, __level_y(*_sustain)), // Sustain end
            ImVec2(__x0 + (*_attack + *_decay + *_release) * __seg + __w * 0.25f, __y_bottom) // Release end
        };

        // Picks the nearest handle on click (0 - attack, 1 - decay/sustain, 3 - release), keeps it while dragging
        bool __changed = false;
        int __active = __storage->GetInt(__handle_id, -1);
        const ImVec2 __mouse = ImGui::GetIO().MousePos;
        if (ImGui::IsItemActivated()) {
            float __best = 1e9f;
            __active = -1;
            const int __handles[3] = {0, 1, 3};
            for (int i = 0; i < 3; ++i) {
                const ImVec2 __d = __points[__handles[i]] - __mouse;
                const float __dist = __d.x * __d.x + __d.y * __d.y;
                if (__dist < __best) { __best = __dist; __active = __handles[i]; }
            }
        }
        if (!ImGui::IsItemActive()) __active = -1;
        __storage->SetInt(__handle_id, __active);

        if (__active >= 0 && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.f)) {
            if (__active == 0)
                *_attack = clampf((__mouse.x - __x0) / __seg, 0.f, _max_time);
            else if (__active == 1) {
                *_decay   = clampf((__mouse.x - __points[0].x) / __seg, 0.f, _max_time);
                *_sustain = clampf((__y_bottom - __mouse.y) / __h, 0.f, 1.f);
            }
            else
                *_release = clampf((__mouse.x - __points[2].x) / __seg, 0.f, _max_time);
            __changed = true;
        }

        ImDrawList *__draw = ImGui::GetWindowDrawList();
        __draw->AddRectFilled(__pos, __pos + __size, ImGui::GetColorU32(ImGuiCol_FrameBg), ImGui::GetStyle().FrameRounding);
        const ImU32 __col_line = ImGui::GetColorU32(ImGuiCol_PlotLines);
        const int __steps = 24;
        __draw->PathLineTo(ImVec2(__x0, __y_bottom));
        __draw->PathLineTo(__points[0]);
        for (int i = 1; i <= __steps; ++i) {
            const float __t = *_decay * i / __steps;
            __draw->PathLineTo(ImVec2(__points[0].x + __t * __seg, __level_y(envelopeLevel(__t, *_decay, 1.f, *_sustain, _exp_curve))));
        }
        __draw->PathLineTo(__points[2]);
        for (int i = 1; i <= __steps; ++i) {
            const float __t = *_release * i / __steps;
            __draw->PathLineTo(ImVec2(__points[2].x + __t * __seg, __level_y(envelopeLevel(__t, *_release, *_sustain, 0.f, _exp_curve))));
        }
        __draw->PathStroke(__col_line, 0, 2.f);

        const char *const __names[4] = {"A", "D", "", "R"};
        for (int i = 0; i < 4; ++i) {
            if (i == 2) continue;
            const bool __hot = __active == i;
            __draw->AddCircleFilled(__points[i], __hot ? 6.f : 4.5f, ImGui::GetColorU32(__hot ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab));
            __draw->AddText(__points[i] + ImVec2(5.f, i == 3 ? -ImGui::GetTextLineHeight() : 2.f), ImGui::GetColorU32(ImGuiCol_TextDisabled), __names[i]);
        }
        return __changed;
    }


    void Waveform(const char *_id, const float *_values, int _count, float _min, float _max, const ImVec2 &_size, ImU32 _color) {
        const ImVec2 __size = itemSize(_size, 3.f);
        const ImVec2 __pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton(_id, __size);
        ImDrawList *__draw = ImGui::GetWindowDrawList();
        __draw->AddRectFilled(__pos, __pos + __size, ImGui::GetColorU32(ImGuiCol_FrameBg), ImGui::GetStyle().FrameRounding);
        const float __range = _max > _min ? _max - _min : 1.f;
        auto __y = [&](float _v) { return __pos.y + __size.y - (clampf(_v, _min, _max) - _min) / __range * __size.y; };
        if (_min < 0.f && _max > 0.f)
            __draw->AddLine(ImVec2(__pos.x, __y(0.f)), ImVec2(__pos.x + __size.x, __y(0.f)), ImGui::GetColorU32(ImGuiCol_Border));
        if (_count < 2) return;

        const ImU32 __col = _color ? _color : ImGui::GetColorU32(ImGuiCol_PlotLines);
        const int __columns = std::max(int(__size.x), 1);
        if (_count <= __columns) {
            for (int i = 0; i < _count; ++i)
                __draw->PathLineTo(ImVec2(__pos.x + __size.x * i / (_count - 1), __y(_values[i])));
            __draw->PathStroke(__col, 0, 1.5f);
            return;
        }
        for (int c = 0; c < __columns; ++c) {
            const int __begin = int(int64_t(_count) * c / __columns), __end = std::max(int(int64_t(_count) * (c + 1) / __columns), __begin + 1);
            float __lo = _values[__begin], __hi = _values[__begin];
            for (int i = __begin + 1; i < __end; ++i) {
                __lo = std::min(__lo, _values[i]);
                __hi = std::max(__hi, _values[i]);
            }
            const float __x = __pos.x + c + 0.5f;
            __draw->AddLine(ImVec2(__x, __y(__hi)), ImVec2(__x, __y(__lo) + 1.f), __col);
        }
    }


    bool WavetableEditor(const char *_id, float *_table, int _length, int _levels, const ImVec2 &_size) {
        const ImVec2 __size = itemSize(_size, 5.f);
        const ImVec2 __pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton(_id, __size);
        if (_length <= 0 || _levels < 2) return false;

        bool __changed = false;
        if (ImGui::IsItemActive()) {
            const ImVec2 __mouse = ImGui::GetIO().MousePos;
            const int __id = int(clampf((__mouse.x - __pos.x) / __size.x, 0.f, 0.9999f) * _length);
            const int __level = int(clampf(1.f - (__mouse.y - __pos.y) / __size.y, 0.f, 0.9999f) * _levels);
            const float __val = 2.f * float(__level) / float(_levels - 1) - 1.f;
            if (_table[__id] != __val) {
                _table[__id] = __val;
                __changed = true;
            }
        }

        ImDrawList *__draw = ImGui::GetWindowDrawList();
        __draw->AddRectFilled(__pos, __pos + __size, ImGui::GetColorU32(ImGuiCol_FrameBg), ImGui::GetStyle().FrameRounding);
        const float __mid = __pos.y + __size.y * 0.5f, __bar_w = __size.x / _length;
        for (int i = 0; i < _length; ++i) {
            const float __top = __pos.y + (1.f - (clampf(_table[i], -1.f, 1.f) + 1.f) * 0.5f) * __size.y;
            __draw->AddRectFilled(ImVec2(__pos.x + i * __bar_w + 1.f, std::min(__top, __mid)),
                                  ImVec2(__pos.x + (i + 1) * __bar_w - 1.f, std::max(__top, __mid) + 1.f), ImGui::GetColorU32(ImGuiCol_PlotHistogram));
        }
        return __changed;
    }


    bool PianoKeyboard(const char *_id, int _first_note, int _octaves, const std::function<bool(int)> &_is_lit,
                       int *_note_on, int *_note_off, const ImVec2 &_size) {
        static const int White_Offsets[7] = {0, 2, 4, 5, 7, 9, 11};
        static const int Black_After[5]   = {0, 1, 3, 4, 5}; // White key index the black key follows (C#, D#, F#, G#, A#)
        static const int Black_Offsets[5] = {1, 3, 6, 8, 10};

        *_note_on = *_note_off = -1;
        const ImVec2 __size = itemSize(_size, 4.f);
        const ImVec2 __pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton(_id, __size);

        const int __num_white = std::max(_octaves, 1) * 7;
        const float __white_w = __size.x / __num_white, __black_w = __white_w * 0.6f, __black_h = __size.y * 0.6f;
        auto __black_rect_x = [&](int _octave, int _i) { return __pos.x + (_octave * 7 + Black_After[_i] + 1) * __white_w - __black_w * 0.5f; };

        // Note under the mouse: black keys first, they lie on top
        int __hovered = -1;
        const ImVec2 __mouse = ImGui::GetIO().MousePos;
        if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
            for (int o = 0; o < _octaves && __hovered < 0; ++o)
                for (int i = 0; i < 5 && __hovered < 0; ++i) {
                    const float __x = __black_rect_x(o, i);
                    if (__mouse.x >= __x && __mouse.x < __x + __black_w && __mouse.y >= __pos.y && __mouse.y < __pos.y + __black_h)
                        __hovered = _first_note + o * 12 + Black_Offsets[i];
                }
            if (__hovered < 0 && __mouse.x >= __pos.x && __mouse.x < __pos.x + __size.x && __mouse.y >= __pos.y && __mouse.y < __pos.y + __size.y) {
                const int __white = std::min(int((__mouse.x - __pos.x) / __white_w), __num_white - 1);
                __hovered = _first_note + (__white / 7) * 12 + White_Offsets[__white % 7];
            }
        }

        // The held note is kept in the storage, sliding over the keys plays them in turn
        ImGuiStorage *__storage = ImGui::GetStateStorage();
        const ImGuiID __held_id = ImGui::GetID(_id);
        const int __held = __storage->GetInt(__held_id, -1);
        int __new_held = ImGui::IsItemActive() ? __hovered : -1;
        if (__new_held < 0 && ImGui::IsItemActive()) __new_held = __held; // Dragged outside: keep the note
        if (__new_held != __held) {
            *_note_off = __held;
            *_note_on  = __new_held;
            __storage->SetInt(__held_id, __new_held);
        }

        ImDrawList *__draw = ImGui::GetWindowDrawList();
        const ImU32 __col_lit = ImGui::GetColorU32(ImGuiCol_ButtonActive);
        for (int w = 0; w < __num_white; ++w) {
            const int __note = _first_note + (w / 7) * 12 + White_Offsets[w % 7];
            const bool __lit = __note == __new_held || (_is_lit && _is_lit(__note));
            const ImVec2 __a(__pos.x + w * __white_w, __pos.y), __b(__pos.x + (w + 1) * __white_w - 1.f, __pos.y + __size.y);
            __draw->AddRectFilled(__a, __b, __lit ? __col_lit : IM_COL32(235, 235, 235, 255), 2.f);
            if (w % 7 == 0) {
                char __label[16];
                snprintf(__label, sizeof(__label), "C%d", __note / 12);
                __draw->AddText(ImVec2(__a.x + 3.f, __b.y - ImGui::GetTextLineHeight() - 2.f), IM_COL32(60, 60, 60, 255), __label);
            }
        }
        for (int o = 0; o < _octaves; ++o)
            for (int i = 0; i < 5; ++i) {
                const int __note = _first_note + o * 12 + Black_Offsets[i];
                const bool __lit = __note == __new_held || (_is_lit && _is_lit(__note));
                const float __x = __black_rect_x(o, i);
                __draw->AddRectFilled(ImVec2(__x, __pos.y), ImVec2(__x + __black_w, __pos.y + __black_h), __lit ? __col_lit : IM_COL32(25, 25, 25, 255), 2.f);
            }
        return *_note_on >= 0 || *_note_off >= 0;
    }


    void VuMeter(const char *_id, float _left, float _right, const ImVec2 &_size) {
        const ImVec2 __size = itemSize(_size, 0.6f);
        const ImVec2 __pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton(_id, __size);
        ImDrawList *__draw = ImGui::GetWindowDrawList();
        __draw->AddRectFilled(__pos, __pos + __size, ImGui::GetColorU32(ImGuiCol_FrameBg));
        const float __bar_h = (__size.y - 1.f) * 0.5f;
        const float __levels[2] = {_left, _right};
        for (int i = 0; i < 2; ++i) {
            const float __level = clampf(__levels[i], 0.f, 1.f);
            const ImU32 __col = __level > 0.9f ? IM_COL32(230, 60, 50, 255) : (__level > 0.6f ? IM_COL32(230, 200, 50, 255) : IM_COL32(80, 200, 90, 255));
            const float __y = __pos.y + i * (__bar_h + 1.f);
            __draw->AddRectFilled(ImVec2(__pos.x, __y), ImVec2(__pos.x + __size.x * __level, __y + __bar_h), __col);
        }
    }


    bool TextGrid(const char *_id, int _rows, int _columns, const std::function<const char*(int)> &_column_name,
                  const TextGridStyle &_style, const std::function<void(int, int, int, char*, int)> &_sub_text,
                  TextGridCursor &_cursor, int _play_row, int _scroll_to, const ImVec2 &_size) {
        const ImGuiTableFlags __flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV |
                                        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX;
        if (_columns <= 0 || _rows <= 0 || !ImGui::BeginTable(_id, _columns + 1, __flags, _size)) return false;

        const float __char_w = ImGui::CalcTextSize("0").x;
        const float __space = __char_w;
        int __cell_chars = 0;
        for (size_t s = 0u; s < _style.sub_widths.size(); ++s)
            __cell_chars += _style.sub_widths[s];
        const float __cell_w = __cell_chars * __char_w + (float(_style.sub_widths.size()) - 1.f) * __space;

        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableSetupColumn("##row", ImGuiTableColumnFlags_WidthFixed, __char_w * 3.f);
        for (int c = 0; c < _columns; ++c)
            ImGui::TableSetupColumn(_column_name ? _column_name(c) : "", ImGuiTableColumnFlags_WidthFixed, __cell_w);
        ImGui::TableHeadersRow();

        bool __clicked = false;
        char __buf[32];
        ImDrawList *__draw = ImGui::GetWindowDrawList();
        const ImU32 __col_cursor = ImGui::GetColorU32(ImGuiCol_TextSelectedBg);
        const ImU32 __col_play   = ImGui::GetColorU32(ImGuiCol_HeaderActive);
        const ImU32 __col_bar    = ImGui::GetColorU32(ImGuiCol_TableRowBgAlt, 2.f);
        const ImU32 __col_beat   = ImGui::GetColorU32(ImGuiCol_TableRowBgAlt);
        const ImU32 __col_rowcur = ImGui::GetColorU32(ImGuiCol_Header, 0.35f);

        ImGuiListClipper __clipper;
        __clipper.Begin(_rows);
        if (_scroll_to >= 0 && _scroll_to < _rows) __clipper.IncludeItemByIndex(_scroll_to);
        while (__clipper.Step())
            for (int r = __clipper.DisplayStart; r < __clipper.DisplayEnd; ++r) {
                ImGui::TableNextRow();
                if (r == _play_row)
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, __col_play);
                else if (r == _cursor.row)
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, __col_rowcur);
                if (_style.bar_rows > 0 && r % _style.bar_rows == 0)
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, __col_bar);
                else if (_style.beat_rows > 0 && r % _style.beat_rows == 0)
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, __col_beat);

                ImGui::TableSetColumnIndex(0);
                ImGui::TextDisabled("%02X", r);
                if (r == _scroll_to) ImGui::SetScrollHereY(0.5f);

                for (int c = 0; c < _columns; ++c) {
                    ImGui::TableSetColumnIndex(c + 1);
                    ImVec2 __p = ImGui::GetCursorScreenPos();
                    const float __line_h = ImGui::GetTextLineHeight();
                    for (size_t s = 0u; s < _style.sub_widths.size(); ++s) {
                        const float __w = _style.sub_widths[s] * __char_w;
                        const bool __is_cursor = r == _cursor.row && c == _cursor.column && int(s) == _cursor.sub;
                        if (__is_cursor)
                            __draw->AddRectFilled(__p, ImVec2(__p.x + __w, __p.y + __line_h), __col_cursor);
                        if (ImGui::IsMouseHoveringRect(__p, ImVec2(__p.x + __w + __space, __p.y + __line_h)) &&
                            ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
                            _cursor.row = r;
                            _cursor.column = c;
                            _cursor.sub = int(s);
                            __clicked = true;
                        }
                        __buf[0] = '\0';
                        _sub_text(r, c, int(s), __buf, int(sizeof(__buf)));
                        const ImU32 __col = s < _style.sub_colors.size() && _style.sub_colors[s] ? _style.sub_colors[s] : ImGui::GetColorU32(ImGuiCol_Text);
                        const bool __empty = __buf[0] == '.';
                        __draw->AddText(__p, __empty ? ImGui::GetColorU32(ImGuiCol_TextDisabled) : __col, __buf);
                        __p.x += __w + __space;
                    }
                    ImGui::Dummy(ImVec2(__cell_w, ImGui::GetTextLineHeight()));
                }
            }
        ImGui::EndTable();
        return __clicked;
    }
    static float bezierAxis(float _u, float _p1, float _p2) {
        const float __v = 1.f - _u;
        return 3.f * __v * __v * _u * _p1 + 3.f * __v * _u * _u * _p2 + _u * _u * _u;
    }

    bool BezierCurve127(const char *_id, int *_x1, int *_y1, int *_x2, int *_y2, const ImVec2 &_size) {
        const float __side = _size.x > 0.f ? _size.x : ImGui::GetTextLineHeight() * 8.f;
        const ImVec2 __size(__side, _size.y > 0.f ? _size.y : __side);
        const ImVec2 __pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton(_id, __size);
        const float __pad = ImGui::GetFontSize() * 0.5f;
        const ImVec2 __a(__pos.x + __pad, __pos.y + __size.y - __pad), __b(__pos.x + __size.x - __pad, __pos.y + __pad);
        const float __w = __b.x - __a.x, __h = __a.y - __b.y;
        // (0..127, 0..127) -> screen
        struct Map {
            ImVec2 a; float w, h;
            ImVec2 operator()(float _x, float _y) const { return ImVec2(a.x + _x / 127.f * w, a.y - _y / 127.f * h); }
        } __map = { __a, __w, __h };

        ImGuiStorage *__storage = ImGui::GetStateStorage();
        const ImGuiID __drag_id = ImGui::GetID("##bezier_drag");
        int __drag = __storage->GetInt(__drag_id, -1);
        const ImVec2 __mouse = ImGui::GetIO().MousePos;
        const ImVec2 __p1 = __map(float(*_x1), float(*_y1)), __p2 = __map(float(*_x2), float(*_y2));
        if (ImGui::IsItemActivated()) {
            const float d1 = (__mouse.x - __p1.x) * (__mouse.x - __p1.x) + (__mouse.y - __p1.y) * (__mouse.y - __p1.y);
            const float d2 = (__mouse.x - __p2.x) * (__mouse.x - __p2.x) + (__mouse.y - __p2.y) * (__mouse.y - __p2.y);
            __drag = d1 <= d2 ? 0 : 1;
        }
        bool __changed = false;
        if (ImGui::IsItemActive() && __drag >= 0) {
            const int __x = (int)std::lround(clampf((__mouse.x - __a.x) / __w, 0.f, 1.f) * 127.f);
            const int __y = (int)std::lround(clampf((__a.y - __mouse.y) / __h, 0.f, 1.f) * 127.f);
            int *__px = __drag == 0 ? _x1 : _x2, *__py = __drag == 0 ? _y1 : _y2;
            __changed = *__px != __x || *__py != __y;
            *__px = __x; *__py = __y;
        }
        if (!ImGui::IsItemActive()) __drag = -1;
        __storage->SetInt(__drag_id, __drag);

        ImDrawList *__draw = ImGui::GetWindowDrawList();
        __draw->AddRectFilled(__pos, __pos + __size, ImGui::GetColorU32(ImGuiCol_FrameBg));
        __draw->AddRect(__map(0.f, 0.f), __map(127.f, 127.f), ImGui::GetColorU32(ImGuiCol_Border));
        __draw->AddLine(__map(0.f, 0.f), __map(127.f, 127.f), ImGui::GetColorU32(ImGuiCol_TextDisabled));
        const ImU32 __col = ImGui::GetColorU32(ImGuiCol_PlotLines);
        ImVec2 __prev = __map(0.f, 0.f);
        for (int i = 1; i <= 32; ++i) {
            const float u = i / 32.f;
            const ImVec2 __pt = __map(bezierAxis(u, float(*_x1), float(*_x2)), bezierAxis(u, float(*_y1), float(*_y2)));
            __draw->AddLine(__prev, __pt, __col, 2.f);
            __prev = __pt;
        }
        const ImVec2 __q1 = __map(float(*_x1), float(*_y1)), __q2 = __map(float(*_x2), float(*_y2));
        __draw->AddLine(__map(0.f, 0.f), __q1, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        __draw->AddLine(__map(127.f, 127.f), __q2, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        __draw->AddCircleFilled(__q1, __pad * 0.6f, ImGui::GetColorU32(__drag == 0 ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab));
        __draw->AddCircleFilled(__q2, __pad * 0.6f, ImGui::GetColorU32(__drag == 1 ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab));
        return __changed;
    }


    bool Timeline(const char *_id, const std::vector<TimelineRow> &_rows, float _current, float _length, TimelineState &_state,
                  TimelineEvent &_event, const ImVec2 &_size) {
        _event = TimelineEvent();
        ImGuiIO &__io = ImGui::GetIO();
        const ImVec2 __full = itemSize(_size, 12.f);
        const ImVec2 __pos  = ImGui::GetCursorScreenPos();
        const float  __line = ImGui::GetTextLineHeightWithSpacing();
        // The keys area, the horizontal scroll bar under it
        const float  __scroll_h = ImGui::GetStyle().ScrollbarSize;
        const ImVec2 __size(__full.x, std::max(__full.y - __scroll_h, __line * 2.f));
        const float  __name_w = std::min(ImGui::GetFontSize() * 9.f, __size.x * 0.35f);
        const float  __ruler_h = __line;
        const float  __keys_x = __pos.x + __name_w;
        const float  __keys_w = std::max(__size.x - __name_w, 1.f);
        const int    __visible_rows = std::max(int((__size.y - __ruler_h) / __line), 1);
        _state.px_per_frame = clampf(_state.px_per_frame, 0.05f, 64.f);
        const float  __visible_frames = __keys_w / _state.px_per_frame;
        if (_state.scroll_to >= 0.f) {
            _state.first_frame = std::max(_state.scroll_to - __visible_frames * 0.5f, 0.f);
            _state.scroll_to = -1.f;
        }

        ImGui::InvisibleButton(_id, __size);
        const bool __hovered = ImGui::IsItemHovered();
        // Wheel: rows; Ctrl - zoom around the mouse; Shift - frames
        if (__hovered && __io.MouseWheel != 0.f) {
            if (__io.KeyCtrl) {
                const float __mouse_frame = _state.first_frame + (__io.MousePos.x - __keys_x) / _state.px_per_frame;
                _state.px_per_frame = clampf(_state.px_per_frame * (__io.MouseWheel > 0.f ? 1.25f : 0.8f), 0.05f, 64.f);
                _state.first_frame = __mouse_frame - (__io.MousePos.x - __keys_x) / _state.px_per_frame;
            } else if (__io.KeyShift) _state.first_frame -= __io.MouseWheel * 40.f / _state.px_per_frame;
            else _state.first_row -= int(__io.MouseWheel * 3.f);
        }
        if (__hovered && __io.MouseWheelH != 0.f) _state.first_frame -= __io.MouseWheelH * 40.f / _state.px_per_frame;
        _state.first_frame = std::max(_state.first_frame, 0.f);
        _state.first_row = std::max(std::min(_state.first_row, int(_rows.size()) - __visible_rows), 0);

        const float __frame_end = std::max(_length, 1.f);
        struct Map {
            float x0, first, px;
            float x(float _f) const { return x0 + (_f - first) * px; }
            float frame(float _x) const { return first + (_x - x0) / px; }
        } __map = { __keys_x, _state.first_frame, _state.px_per_frame };

        // Hit test: the row and the nearest key under the mouse
        const ImVec2 __mouse = __io.MousePos;
        const bool __in_ruler = __mouse.y < __pos.y + __ruler_h;
        const bool __in_keys  = __mouse.x >= __keys_x;
        int __hit_row = -1, __hit_key = -1;
        if (!__in_ruler) {
            const int __r = _state.first_row + int((__mouse.y - __pos.y - __ruler_h) / __line);
            if (__r >= 0 && __r < (int)_rows.size()) __hit_row = __r;
        }
        if (__hit_row >= 0 && __in_keys) {
            float __best = __line * 0.5f;
            const std::vector<float> &__frames = _rows[__hit_row].frames;
            for (size_t k = 0u; k < __frames.size(); ++k) {
                const float d = std::fabs(__map.x(__frames[k]) - __mouse.x);
                if (d <= __best) { __best = d; __hit_key = (int)k; }
            }
        }
        const float __mouse_frame = std::max(std::floor(__map.frame(__mouse.x) + 0.5f), 0.f);

        if (ImGui::IsItemActivated()) {
            if (__in_ruler && __in_keys) { _event.type = TimelineEvent::TL_EVENT_SEEK; _event.frame = __mouse_frame; }
            else if (__hit_key >= 0) {
                _state.selected_row = __hit_row; _state.selected_key = __hit_key;
                _state.dragging = true; _state.drag_frame = _rows[__hit_row].frames[__hit_key];
                _event.type = TimelineEvent::TL_EVENT_SELECT; _event.row = __hit_row; _event.key = __hit_key;
                _event.frame = _state.drag_frame;
            } else if (__hit_row >= 0) {
                _state.selected_row = __hit_row; _state.selected_key = -1;
                _event.type = TimelineEvent::TL_EVENT_SELECT; _event.row = __hit_row;
                _event.frame = __in_keys ? __mouse_frame : -1.f;
            }
        }
        if (ImGui::IsItemActive() && __in_ruler && !_state.dragging && __in_keys && _event.type == TimelineEvent::TL_EVENT_NONE &&
            ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.f)) {
            _event.type = TimelineEvent::TL_EVENT_SEEK; _event.frame = __mouse_frame;
        }
        if (_state.dragging) {
            if (ImGui::IsItemActive()) _state.drag_frame = __mouse_frame;
            else {
                _state.dragging = false;
                if (_state.selected_row >= 0 && _state.selected_row < (int)_rows.size() && _state.selected_key >= 0 &&
                    _state.selected_key < (int)_rows[_state.selected_row].frames.size() &&
                    _rows[_state.selected_row].frames[_state.selected_key] != _state.drag_frame) {
                    _event.type = TimelineEvent::TL_EVENT_MOVE_KEY; _event.row = _state.selected_row; _event.key = _state.selected_key;
                    _event.frame = _state.drag_frame;
                }
            }
        }
        if (__hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && __hit_row >= 0 && __in_keys) {
            _event.type = TimelineEvent::TL_EVENT_DOUBLE_CLICK; _event.row = __hit_row; _event.key = __hit_key; _event.frame = __mouse_frame;
        }

        // Horizontal scroll bar: the range covers the motion (and the play head) plus half a view, and the current view
        const float __range = std::max(std::max(__frame_end, _current) + __visible_frames * 0.5f, _state.first_frame + __visible_frames);
        const float __grab_w = std::min(std::max(__keys_w * __visible_frames / __range, __scroll_h), __keys_w);
        const float __track_w = std::max(__keys_w - __grab_w, 1.f);
        const float __scroll_max = std::max(__range - __visible_frames, 1e-3f);
        const ImVec2 __bar_pos(__keys_x, __pos.y + __size.y);
        ImGui::SetCursorScreenPos(__bar_pos);
        ImGui::PushID(_id);
        ImGui::InvisibleButton("##hscroll", ImVec2(__keys_w, __scroll_h));
        ImGui::PopID();
        const bool __bar_hovered = ImGui::IsItemHovered(), __bar_active = ImGui::IsItemActive();
        float __grab_x = __keys_x + _state.first_frame / __scroll_max * __track_w;
        if (ImGui::IsItemActivated()) {
            // On the grab: drag it; elsewhere: center the grab on the mouse, then drag
            if (__mouse.x >= __grab_x && __mouse.x <= __grab_x + __grab_w) _state.scroll_grab = __mouse.x - __grab_x;
            else _state.scroll_grab = __grab_w * 0.5f;
        }
        if (__bar_active && _state.scroll_grab >= 0.f) {
            _state.first_frame = clampf((__mouse.x - _state.scroll_grab - __keys_x) / __track_w, 0.f, 1.f) * __scroll_max;
            __grab_x = __keys_x + _state.first_frame / __scroll_max * __track_w;
            __map.first = _state.first_frame;
        } else _state.scroll_grab = -1.f;
        if (__bar_hovered && __io.MouseWheel != 0.f && !__io.KeyCtrl) {
            _state.first_frame = std::max(_state.first_frame - __io.MouseWheel * 40.f / _state.px_per_frame, 0.f);
            __map.first = _state.first_frame;
        }

        // Drawing
        ImDrawList *__draw = ImGui::GetWindowDrawList();
        __draw->PushClipRect(__pos, __pos + __size, true);
        __draw->AddRectFilled(__pos, __pos + __size, ImGui::GetColorU32(ImGuiCol_FrameBg));
        // Ruler: a tick every 'step' frames with a label every 'label' frames
        const float __steps[] = { 1.f, 2.f, 5.f, 10.f, 15.f, 30.f, 60.f, 150.f, 300.f, 600.f, 1500.f, 3000.f };
        float __step = 1.f;
        for (size_t i = 0u; i < sizeof(__steps) / sizeof(__steps[0]); ++i) { __step = __steps[i]; if (__step * __map.px >= 8.f) break; }
        const float __label_step = __step * 5.f;
        const float __last_visible = __map.frame(__keys_x + __keys_w);
        const ImU32 __col_tick = ImGui::GetColorU32(ImGuiCol_TextDisabled), __col_text = ImGui::GetColorU32(ImGuiCol_Text);
        for (float f = std::floor(_state.first_frame / __step) * __step; f <= __last_visible; f += __step) {
            const float x = __map.x(f);
            if (x < __keys_x) continue;
            const bool __major = std::fmod(f, __label_step) < 0.5f;
            __draw->AddLine(ImVec2(x, __pos.y + __ruler_h * (__major ? 0.4f : 0.7f)), ImVec2(x, __pos.y + __size.y),
                            __major ? ImGui::GetColorU32(ImGuiCol_Border) : (__col_tick & 0x30FFFFFFu));
            if (__major) {
                char __buf[16];
                snprintf(__buf, sizeof(__buf), "%d", int(f));
                __draw->AddText(ImVec2(x + 2.f, __pos.y), __col_tick, __buf);
            }
        }
        // The end of the motion
        if (__map.x(__frame_end) >= __keys_x)
            __draw->AddRectFilled(ImVec2(std::max(__map.x(__frame_end), __keys_x), __pos.y + __ruler_h), __pos + __size,
                                  ImGui::GetColorU32(ImGuiCol_WindowBg) & 0x80FFFFFFu);
        // Rows
        const float __half = __line * 0.3f;
        for (int r = _state.first_row; r < (int)_rows.size() && r < _state.first_row + __visible_rows; ++r) {
            const TimelineRow &__row = _rows[r];
            const float y = __pos.y + __ruler_h + (r - _state.first_row) * __line;
            if (r == _state.selected_row)
                __draw->AddRectFilled(ImVec2(__pos.x, y), ImVec2(__pos.x + __size.x, y + __line), ImGui::GetColorU32(ImGuiCol_Header));
            else if (r % 2 == 1)
                __draw->AddRectFilled(ImVec2(__pos.x, y), ImVec2(__pos.x + __size.x, y + __line), ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
            __draw->PushClipRect(ImVec2(__pos.x, y), ImVec2(__keys_x - 2.f, y + __line), true);
            __draw->AddText(ImVec2(__pos.x + (__row.header ? 2.f : 10.f), y), __row.header ? __col_tick : __col_text, __row.name);
            __draw->PopClipRect();
            const float cy = y + __line * 0.5f;
            const ImU32 __col_key = __row.color ? __row.color : ImGui::GetColorU32(ImGuiCol_PlotLines);
            for (size_t k = 0u; k < __row.frames.size(); ++k) {
                const bool __selected = r == _state.selected_row && (int)k == _state.selected_key;
                const float __f = __selected && _state.dragging ? _state.drag_frame : __row.frames[k];
                const float x = __map.x(__f);
                if (x < __keys_x - __half || x > __keys_x + __keys_w + __half) continue;
                const ImVec2 __pts[4] = { ImVec2(x, cy - __half), ImVec2(x + __half, cy), ImVec2(x, cy + __half), ImVec2(x - __half, cy) };
                __draw->AddConvexPolyFilled(__pts, 4, __selected ? ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered) : __col_key);
                if (__selected) __draw->AddPolyline(__pts, 4, __col_text, ImDrawFlags_Closed, 1.5f);
            }
        }
        __draw->AddLine(ImVec2(__keys_x, __pos.y), ImVec2(__keys_x, __pos.y + __size.y), ImGui::GetColorU32(ImGuiCol_Border));
        // Play head
        const float __head = __map.x(_current);
        if (__head >= __keys_x)
            __draw->AddLine(ImVec2(__head, __pos.y), ImVec2(__head, __pos.y + __size.y), ImGui::GetColorU32(ImGuiCol_PlotHistogram), 2.f);
        __draw->PopClipRect();
        // The scroll bar
        __draw->AddRectFilled(ImVec2(__pos.x, __bar_pos.y), ImVec2(__pos.x + __full.x, __bar_pos.y + __scroll_h),
                              ImGui::GetColorU32(ImGuiCol_ScrollbarBg));
        const ImGuiCol __grab_col = __bar_active ? ImGuiCol_ScrollbarGrabActive : (__bar_hovered ? ImGuiCol_ScrollbarGrabHovered : ImGuiCol_ScrollbarGrab);
        __draw->AddRectFilled(ImVec2(__grab_x, __bar_pos.y + 2.f), ImVec2(__grab_x + __grab_w, __bar_pos.y + __scroll_h - 2.f),
                              ImGui::GetColorU32(__grab_col), ImGui::GetStyle().ScrollbarRounding);
        // The play head on the scroll bar
        const float __head_bar = __keys_x + clampf(_current / __range, 0.f, 1.f) * __keys_w;
        __draw->AddLine(ImVec2(__head_bar, __bar_pos.y), ImVec2(__head_bar, __bar_pos.y + __scroll_h), ImGui::GetColorU32(ImGuiCol_PlotHistogram), 2.f);
        return _event.type != TimelineEvent::TL_EVENT_NONE;
    }
}; // ImGuiCustom END
