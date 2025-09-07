#ifndef SVKFW_TERM_HANDLER_H
#define SVKFW_TERM_HANDLER_H

#include <vector>
#include <string>

#include "common/terminal.h"


namespace Simple {
    namespace TermCtrl {
    // Escape sequences

        std::string genPropEscSeq(Emphasis _property) {
            switch (_property) {
                case Emphasis::      Off: return SGR_EMPH_OFF;
                case Emphasis::     Bold: return SGR_EMPH_BOLD;
                case Emphasis::    Faint: return SGR_EMPH_FAINT;
                case Emphasis::BoldFaint: return SGR_EMPH_BOLD ";" SGR_EMPH_FAINT;
            }
            return "";
        }
        std::string genPropEscSeq(Underline _property) {
            switch (_property) {
                case Underline::    On: return SGR_ULINE;
                case Underline::   Off: return SGR_ULINE_OFF;
                case Underline::Double: return SGR_ULINE_DOUBLE;
            }
            return "";
        }
        std::string genPropEscSeq(Blinking _property) {
            switch (_property) {
                case Blinking:: Off: return SGR_BLINK_OFF;
                case Blinking::Slow: return SGR_BLINK_SLOW;
                case Blinking::Fast: return SGR_BLINK_FAST;
            }
            return "";
        }
        std::string genPropEscSeq(FontStyle _property) {
            switch (_property) {
                case FontStyle::Default: return SGR_STYLE_OFF;
                case FontStyle:: Italic: return SGR_STYLE_ITALIC;
                case FontStyle:: Gothic: return SGR_STYLE_GOTHIC;
            }
            return "";
        }
        std::string genPropEscSeq(ColorMode _property) {
            switch (_property) {
                case ColorMode:: Default: return SGR_COL_INVERT_OFF;
                case ColorMode::Inverted: return SGR_COL_INVERT;
            }
            return "";
        }
        std::string genPropEscSeq(Strikeout _property) {
            switch (_property) {
                case Strikeout:: On: return SGR_STRIKEOUT;
                case Strikeout::Off: return SGR_STRIKEOUT_OFF;
            }
            return "";
        }
        std::string genPropEscSeq(RGBModeBG _property, TColor _term_col, TextStyle::Col24 _term_col3) {
            switch (_property) {
                case RGBModeBG::Default: return SGR_COL_BG_DEFAULT;
                case RGBModeBG::   RGB8: return SGR_COL_BG_SET8(+ std::to_string(int(_term_col)));
                case RGBModeBG::  RGB24: return SGR_COL_BG_SET( + std::to_string(_term_col3.r) +,
                                                                          + std::to_string(_term_col3.g) +,
                                                                          + std::to_string(_term_col3.b));
            }
            return "";
        }
        std::string genPropEscSeq(RGBModeFG _property, TColor _term_col, TextStyle::Col24 _term_col3) {
            switch (_property) {
                case RGBModeFG::Default: return SGR_COL_FG_DEFAULT;
                case RGBModeFG::   RGB8: return SGR_COL_FG_SET8(+ std::to_string(int(_term_col)));
                case RGBModeFG::  RGB24: return SGR_COL_FG_SET( + std::to_string(_term_col3.r) +,
                                                                          + std::to_string(_term_col3.g) +,
                                                                          + std::to_string(_term_col3.b));
            }
            return "";
        }
        std::string genPropEscSeq(HideMode _property) {
            switch (_property) {
                case HideMode::Show: return SGR_HIDE_OFF;
                case HideMode::Hide: return SGR_HIDE;
            }
            return "";
        }
        std::string genPropEscSeq(Overline _property) {
            switch (_property) {
                case Overline::Off: return SGR_OLINE_OFF;
                case Overline:: On: return SGR_OLINE;
            }
            return "";
        }

        std::string genStyleEscSeqExplicit(const TextStyle &_style_st) {
            std::string __res = CSISEQ(SGR_DEFAULT);

            __res += CSISEQ_BEG;
            __res += genPropEscSeq(_style_st.get<FontStyle>()) + ';';
            __res += genPropEscSeq(_style_st.get<ColorMode>()) + ';';
            __res += genPropEscSeq(_style_st.get<Emphasis >()) + ';';
            __res += genPropEscSeq(_style_st.get<Blinking >()) + ';';
            __res += genPropEscSeq(_style_st.get<Underline>()) + ';';
            __res += genPropEscSeq(_style_st.get<Overline >()) + ';';
            __res += genPropEscSeq(_style_st.get<Strikeout>()) + ';';
            __res += genPropEscSeq(_style_st.get<HideMode >());
            __res += CSISEQ_END;

            __res += CSISEQ_BEG + genPropEscSeq(_style_st.get<RGBModeFG>(), _style_st.fgcolor, _style_st.rgbFG) + CSISEQ_END;
            __res += CSISEQ_BEG + genPropEscSeq(_style_st.get<RGBModeBG>(), _style_st.bgcolor, _style_st.rgbBG) + CSISEQ_END;

            return __res;
        }
        std::string genStyleEscSeq(const TextStyle &_style_st) {
            std::string __res;
            bool __has_custom_props = false;

            __res += CSISEQ_BEG;
            if (_style_st.getActive<TFlag::HAS_FONTSTYLE>()) {
                __res  += genPropEscSeq(_style_st.get<FontStyle>()); __has_custom_props = true;
            }
            if (_style_st.getActive<TFlag::COLORS_INV>()) {
                __res  += (__has_custom_props ? ";" : "") + genPropEscSeq(_style_st.get<ColorMode>());
                __has_custom_props = true;
            }
            if (_style_st.getActive<TFlag::BOLDFAINT>()) {
                __res  += (__has_custom_props ? ";" : "") + genPropEscSeq(_style_st.get<Emphasis>());
                __has_custom_props = true;
            }
            if (_style_st.getActive<TFlag::HAS_BLINK>()) {
                __res  += (__has_custom_props ? ";" : "") + genPropEscSeq(_style_st.get<Blinking>());
                __has_custom_props = true;
            }
            if (_style_st.getActive<TFlag::HAS_UNDERLINE>()) {
                __res  += (__has_custom_props ? ";" : "") + genPropEscSeq(_style_st.get<Underline>());
                __has_custom_props = true;
            }
            if (_style_st.getActive<TFlag::OVERLINE>()) {
                __res  += (__has_custom_props ? ";" : "") + genPropEscSeq(_style_st.get<Overline>());
                __has_custom_props = true;
            }
            if (_style_st.getActive<TFlag::STRIKEOUT>()) {
                __res  += (__has_custom_props ? ";" : "") + genPropEscSeq(_style_st.get<Strikeout>());
                __has_custom_props = true;
            }
            if (_style_st.getActive<TFlag::HIDDEN>()) {
                __res  += (__has_custom_props ? ";" : "") + genPropEscSeq(_style_st.get<HideMode>());
                __has_custom_props = true;
            }
            __res += CSISEQ_END;

            __res += CSISEQ_BEG + genPropEscSeq(_style_st.get<RGBModeFG>(), _style_st.fgcolor, _style_st.rgbFG) + CSISEQ_END;
            __res += CSISEQ_BEG + genPropEscSeq(_style_st.get<RGBModeBG>(), _style_st.bgcolor, _style_st.rgbBG) + CSISEQ_END;

            return __res;
        }

    // Stylized string

        struct StylizedString {
            std::string   f_str;
            std::string esc_seq;

            StylizedString(const std::string &  _f_str = "",
                           const std::string &_esc_seq = "") : f_str{_f_str}, esc_seq{_esc_seq} {}
        }; // StylizedString END

        uint32_t compileStylizedStrings(const std::vector<StylizedString> &_s_strings, std::string &_res_str) {
            uint32_t __total_len = 0u;
            for (const auto &p_str : _s_strings) {
                __total_len += p_str.f_str.size();
                _res_str += p_str.esc_seq.empty() ? p_str.f_str : p_str.esc_seq + p_str.f_str + CSISEQ(SGR_DEFAULT);
            }
            return __total_len;
        }

    // Terminal handler

        struct TerminalHandler {
            struct StyleConfig {
                std::vector<TextStyle> states;
                uint32_t curr_state_id = 0u;
                bool is_active =  true;
            } style; // StyleConfig END

            struct  LineConfig {
                char  border_l = '|';
                char  border_r = '|';
                uint16_t width = 64u;

                struct LineElements {
                    uint8_t pretty_l, pretty_r;
                    uint8_t  space_l,  space_r;
                };

                LineElements getCenteredWindowLineElementsLen(int _empty_len) {
                    LineElements __res{};
                    if (_empty_len == width) {
                        __res.pretty_l = _empty_len;
                    }
                    else if (_empty_len >= 8) {
                        _empty_len -= 6;
                        __res.space_l  = __res.space_r = 3;
                        __res.pretty_l = _empty_len >> 1;
                        __res.pretty_r = _empty_len - __res.pretty_l;
                    }
                    else if (_empty_len >= 4) {
                        _empty_len -= 2;
                        __res.pretty_l  = __res.pretty_r = 1;
                        __res.space_l = _empty_len >> 1;
                        __res.space_r = _empty_len - __res.space_l;
                    }
                    else if (_empty_len >= 2) {
                        __res.space_l = _empty_len >> 1;
                        __res.space_r = _empty_len - __res.space_l;
                    }
                    return __res;
                }
            } line; //  LineConfig END

            TerminalHandler(const std::initializer_list<TextStyle> &_styles = {}) {
                style.states = _styles;
            }

        // Manage style states

            uint32_t addState(const TextStyle &_style) { style.states.push_back(_style); return style.states.size() - 1; }
            std::string genStyleEscSeq(uint32_t _state_id) { return TermCtrl::genStyleEscSeq(style.states[_state_id]); }
            StylizedString encloseWithStyle(uint32_t _state_id, const std::string &_e_str) { 
                return { _e_str, genStyleEscSeq(_state_id) };
            }


        // Printers

            inline void print(const std::string &_f_str, const std::string &_p_str) {
                printf(_f_str.c_str(), _p_str.c_str());
            }
            // print fixed-width window line (no error produced if the line is too wide)
            void print_w(const std::string &_p_str = "") {
                std::string __f_str = line.border_l + ("  %-" + std::to_string(line.width) + "s  ") + line.border_r + '\n';
                print(__f_str, _p_str);
            }
            // print fixed-width window line using a style from 'style.states' (no error produced if the line is too wide)
            void print_w(uint32_t _style_id, const std::string &_p_str = "") {
                std::string __f_str = line.border_l + ("  " + TermCtrl::genStyleEscSeq(style.states[_style_id]) + "%-" + std::to_string(line.width) + "s" + CSISEQ(SGR_DEFAULT) + "  ") + line.border_r + '\n';
                print(__f_str, _p_str);
            }
            // print fixed-width window line (no error produced if the line is too wide)
            void print_w(const std::vector<StylizedString> &_s_strings) {
                std::string __res_str;
                uint32_t __total_len = compileStylizedStrings(_s_strings, __res_str);

                if (__total_len < line.width)
                    __res_str += std::string(line.width - __total_len, ' ');
                __res_str = line.border_l + ("  " + __res_str + "  ") + line.border_r + '\n';

                print("%s", __res_str);
            }
            // print centered fixed-width window line (no error produced if the line is too wide)
            void print_wcent(std::string _str, char _pretty_sym = ' ') {
                auto __line_elems = line.getCenteredWindowLineElementsLen(int(line.width) - _str.size());
                print_w(std::string(__line_elems.pretty_l, _pretty_sym) + std::string(__line_elems.space_l, ' ') + _str +
                        std::string(__line_elems.space_r, ' ') + std::string(__line_elems.pretty_r, _pretty_sym));
            }
            // print centered fixed-width window line (no error produced if the line is too wide)
            void print_wcent(uint32_t _style_id, std::string _str, char _pretty_sym = ' ') {
                auto __line_elems = line.getCenteredWindowLineElementsLen(int(line.width) - _str.size());
                print_w(std::string(__line_elems.pretty_l, _pretty_sym) + std::string(__line_elems.space_l, ' ') +
                        TermCtrl::genStyleEscSeq(style.states[_style_id]) + _str + CSISEQ(SGR_DEFAULT) +
                        std::string(__line_elems.space_r, ' ') + std::string(__line_elems.pretty_r, _pretty_sym));
            }
            // print centered fixed-width window line (no error produced if the line is too wide)
            void print_wcent(const std::vector<StylizedString> &_s_strings, char _pretty_sym = ' ') {
                std::string __res_str;
                uint32_t __total_len = compileStylizedStrings(_s_strings, __res_str);
                auto __line_elems = line.getCenteredWindowLineElementsLen(int(line.width) - __total_len);

                print_w(std::string(__line_elems.pretty_l, _pretty_sym) + std::string(__line_elems.space_l, ' ') +
                        __res_str + std::string(__line_elems.space_r, ' ') + std::string(__line_elems.pretty_r, _pretty_sym));
            }
        }; // TerminalHandler END
    }; // TermCtrl END

// General string handling

    void splitString(const std::string &_str_to_split, uint32_t _width, std::vector<std::string> &_res) {
        uint32_t __curr_id = 0u, __last_id = 0u;

        if (_width == 0u) {
            _res.push_back(_str_to_split);
            return;
        }

        while (int(_str_to_split.size()) - __curr_id > _width) {
            __last_id = __curr_id;

            uint32_t __pos = _str_to_split.substr(__last_id, _width).rfind(' ');
            bool __use_pos = __pos > (_width >> 1) && __pos < _width;

            _res.push_back(_str_to_split.substr(__last_id, __use_pos ? __pos : _width));
            __curr_id += __use_pos ? __pos+1 : _width;
        }
        _res.push_back(_str_to_split.substr(__curr_id, _width));
    }
}; // Simple END

#endif