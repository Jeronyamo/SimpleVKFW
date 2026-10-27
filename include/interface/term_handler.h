#ifndef SVKFW_TERM_HANDLER_H
#define SVKFW_TERM_HANDLER_H

#include <vector>
#include <string>

#include "common/terminal.h"


namespace Simple {
    namespace TermCtrl {
    // Escape sequences

        std::string genPropEscSeq(Emphasis _property);
        std::string genPropEscSeq(Underline _property);
        std::string genPropEscSeq(Blinking _property);
        std::string genPropEscSeq(FontStyle _property);
        std::string genPropEscSeq(ColorMode _property);
        std::string genPropEscSeq(Strikeout _property);
        std::string genPropEscSeq(RGBModeBG _property, TColor _term_col, TextStyle::Col24 _term_col3);
        std::string genPropEscSeq(RGBModeFG _property, TColor _term_col, TextStyle::Col24 _term_col3);
        std::string genPropEscSeq(HideMode _property);
        std::string genPropEscSeq(Overline _property);

        std::string genStyleEscSeqExplicit(const TextStyle &_style_st);
        std::string genStyleEscSeq(const TextStyle &_style_st);

    // Stylized string

        struct StylizedString {
            std::string   f_str;
            std::string esc_seq;

            StylizedString(const std::string &  _f_str = "",
                           const std::string &_esc_seq = "");
        }; // StylizedString END

        uint32_t compileStylizedStrings(const std::vector<StylizedString> &_s_strings, std::string &_res_str);

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

                LineElements getCenteredWindowLineElementsLen(int _empty_len);
            } line; //  LineConfig END

            TerminalHandler(const std::vector<TextStyle> &_styles = {});

        // Manage style states

            uint32_t addState(const TextStyle &_style);
            std::string genStyleEscSeq(uint32_t _state_id);
            StylizedString encloseWithStyle(uint32_t _state_id, const std::string &_e_str);


        // Printers

            void print(const std::string &_f_str, const std::string &_p_str);
            // print fixed-width window line (no error produced if the line is too wide)
            void print_w(const std::string &_p_str = "");
            // print fixed-width window line using a style from 'style.states' (no error produced if the line is too wide)
            void print_w(uint32_t _style_id, const std::string &_p_str = "");
            // print fixed-width window line (no error produced if the line is too wide)
            void print_w(const std::vector<StylizedString> &_s_strings);
            // print centered fixed-width window line (no error produced if the line is too wide)
            void print_wcent(std::string _str, char _pretty_sym = ' ');
            // print centered fixed-width window line (no error produced if the line is too wide)
            void print_wcent(uint32_t _style_id, std::string _str, char _pretty_sym = ' ');
            // print centered fixed-width window line (no error produced if the line is too wide)
            void print_wcent(const std::vector<StylizedString> &_s_strings, char _pretty_sym = ' ');
        }; // TerminalHandler END
    }; // TermCtrl END

// General string handling

    void splitString(const std::string &_str_to_split, uint32_t _width, std::vector<std::string> &_res);
}; // Simple END

#endif