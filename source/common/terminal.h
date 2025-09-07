#ifndef SVKFW_TERMINAL_H
#define SVKFW_TERMINAL_H

#include <cstdio>
#include <cstdint>
#include <type_traits>
#include <stdexcept>

#include "common/bits.h"



//  =========================================  \\
                ANSI escape codes
//  =========================================  \\

// "not working" - on Ubuntu 22 / Windows 10|11

#define SGR_DEFAULT                    "0"
#define SGR_EMPH_BOLD                  "1"
#define SGR_EMPH_FAINT                 "2"
#define SGR_STYLE_ITALIC               "3"
#define SGR_ULINE                      "4"
#define SGR_BLINK_SLOW                 "5" /* not working */
#define SGR_BLINK_FAST                 "6" /* not working */
#define SGR_COL_INVERT                 "7"
#define SGR_HIDE                       "8"
#define SGR_STRIKEOUT                  "9"

// 10-19 (fonts) doesn't work
#define SGR_FONT_DEFAULT              "10"
#define SGR_FONT_1                    "11"
#define SGR_FONT_2                    "12"
#define SGR_FONT_3                    "13"
#define SGR_FONT_4                    "14"
#define SGR_FONT_5                    "15"
#define SGR_FONT_6                    "16"
#define SGR_FONT_7                    "17"
#define SGR_FONT_8                    "18"
#define SGR_FONT_9                    "19"

#define SGR_STYLE_GOTHIC              "20" /* not working */
#define SGR_ULINE_DOUBLE              "21"
#define SGR_EMPH_OFF                  "22"
#define SGR_STYLE_OFF                 "23"
#define SGR_ULINE_OFF                 "24"
#define SGR_BLINK_OFF                 "25" /* not working */
#define SGR_COL_INVERT_OFF            "27"
#define SGR_HIDE_OFF                  "28"
#define SGR_STRIKEOUT_OFF             "29"

#define SGR_COL_FG_BLACK              "30"
#define SGR_COL_FG_RED                "31"
#define SGR_COL_FG_GREEN              "32"
#define SGR_COL_FG_YELLOW             "33"
#define SGR_COL_FG_BLUE               "34"
#define SGR_COL_FG_MAGENTA            "35"
#define SGR_COL_FG_CYAN               "36"
#define SGR_COL_FG_WHITE              "37"
#define SGR_COL_FG_SET(_R, _G, _B)    "38;2;" _R ";" _G ";" _B
#define SGR_COL_FG_SET8(_COL)         "38;5;" _COL
#define SGR_COL_FG_DEFAULT            "39"

#define SGR_COL_BG_BLACK              "40"
#define SGR_COL_BG_RED                "41"
#define SGR_COL_BG_GREEN              "42"
#define SGR_COL_BG_YELLOW             "43"
#define SGR_COL_BG_BLUE               "44"
#define SGR_COL_BG_MAGENTA            "45"
#define SGR_COL_BG_CYAN               "46"
#define SGR_COL_BG_WHITE              "47"
#define SGR_COL_BG_SET(_R, _G, _B)    "48;2;" _R ";" _G ";" _B
#define SGR_COL_BG_SET8(_COL)         "48;5;" _COL
#define SGR_COL_BG_DEFAULT            "49"

#define SGR_BORD_FRAMED               "51" /* not working */
#define SGR_BORD_ENCIRCLED            "52" /* not working */
#define SGR_OLINE                     "53"
#define SGR_BORD_OFF                  "54" /* not working */
#define SGR_OLINE_OFF                 "55"
#define SGR_COL_ULINE_SET(_R, _G, _B) "58;2;" _R ";" _G ";" _B /* not working */
#define SGR_COL_ULINE_SET8(_COL)      "58;5;" _COL             /* not working */
#define SGR_COL_ULINE_DEFAULT         "59"

// 60-75 SKIP

#define SGR_COL_FG_BLACK_BR           "90"
#define SGR_COL_FG_RED_BR             "91"
#define SGR_COL_FG_GREEN_BR           "92"
#define SGR_COL_FG_YELLOW_BR          "93"
#define SGR_COL_FG_BLUE_BR            "94"
#define SGR_COL_FG_MAGENTA_BR         "95"
#define SGR_COL_FG_CYAN_BR            "96"
#define SGR_COL_FG_WHITE_BR           "97"

#define SGR_COL_BG_BLACK_BR          "100"
#define SGR_COL_BG_RED_BR            "101"
#define SGR_COL_BG_GREEN_BR          "102"
#define SGR_COL_BG_YELLOW_BR         "103"
#define SGR_COL_BG_BLUE_BR           "104"
#define SGR_COL_BG_MAGENTA_BR        "105"
#define SGR_COL_BG_CYAN_BR           "106"
#define SGR_COL_BG_WHITE_BR          "107"

//  =============================================  \\
                ANSI escape codes END
//  =============================================  \\

#include "platform_dependent/pd_terminal.h"

// Note: for the love of God, don't add space before '(' to align SVKFW_WRAPERR - macros won't work correctly

#define SVKFW_ENCLOSE(_SVKFW_SEQ, _SVKFW_WHAT) CSISEQ(_SVKFW_SEQ) _SVKFW_WHAT CSISEQ("")

#define SVKFW_WRAPINFO(_SVKFW_WHERE, _SVKFW_MESSG) "[" SVKFW_ENCLOSE(SGR_COL_FG_CYAN,   "Info"   ) "]>["  SVKFW_ENCLOSE(SGR_COL_FG_CYAN,   _SVKFW_WHERE) "]> " _SVKFW_MESSG
#define SVKFW_WRAPWARN(_SVKFW_WHERE, _SVKFW_MESSG) "[" SVKFW_ENCLOSE(SGR_COL_FG_YELLOW, "Warning") "]>["  SVKFW_ENCLOSE(SGR_COL_FG_YELLOW, _SVKFW_WHERE) "]> " _SVKFW_MESSG
#define SVKFW_WRAPERR( _SVKFW_WHERE, _SVKFW_MESSG) "[" SVKFW_ENCLOSE(SGR_COL_FG_RED,    "Error"  ) "]>["  SVKFW_ENCLOSE(SGR_COL_FG_RED,    _SVKFW_WHERE) "]> " _SVKFW_MESSG

#define SVKFW_ASSERT(_SVKFW_EXPR, _SVKFW_ERR_TYPE, _SVKFW_WHERE, _SVKFW_MESSG) if (!(_SVKFW_EXPR)) throw _SVKFW_ERR_TYPE(SVKFW_WRAPERR(_SVKFW_WHERE, _SVKFW_MESSG))


namespace Simple {
    FILE *svkfwwarn = stderr;

    namespace TermCtrl {
        enum class TFlag : uint8_t {
            BOLD, FAINT, COLORS_INV, FG_RGB8, FG_RGB24, BG_RGB8, BG_RGB24, HIDDEN, STRIKEOUT,
            OVERLINE, SINGLE_UNDERLINE, DOUBLE_UNDERLINE, SLOW_BLINK, FAST_BLINK, ITALIC, GOTHIC,
            HAS_UNDERLINE, HAS_FONTSTYLE, HAS_FG_COLOR, HAS_BG_COLOR, HAS_BLINK, BOLDFAINT
        }; // TFlag END

        enum class FontStyle : uint8_t {  Default, Italic, Gothic  };
        enum class ColorMode : uint8_t {  Default, Inverted  };
        enum class RGBModeFG : uint8_t {  Default, RGB8, RGB24  };
        enum class RGBModeBG : uint8_t {  Default, RGB8, RGB24  };
        enum class Emphasis  : uint8_t {  Off, Bold, Faint, BoldFaint  };
        enum class Blinking  : uint8_t {  Off, Slow, Fast  };
        enum class Underline : uint8_t {  Off, On, Double  };
        enum class Overline  : uint8_t {  Off, On  };
        enum class Strikeout : uint8_t {  Off, On  };
        enum class HideMode  : uint8_t {  Show, Hide  };

        enum class TColor : uint8_t {
            Black,
            Red,
            Green,
            Yellow,
            Blue,
            Magenta,
            Cyan,
            White,
            BrightBlack,
            BrightRed,
            BrightGreen,
            BrightYellow,
            BrightBlue,
            BrightMagenta,
            BrightCyan,
            BrightWhite,
            Default
        }; // TColor END


        TColor getTermColor(bool _red, bool _green, bool _blue, bool _bright) {
            return TColor((uint8_t(_red       ) | (uint8_t(_green ) << 1) |
                          (uint8_t(_blue) << 2) | (uint8_t(_bright) << 3)));
        }


        struct TextStyle {
            uint16_t flags = 0;
            TColor fgcolor = TColor::Default, bgcolor = TColor::Default;

            struct Col24 {
                uint8_t r, g, b;
            } rgbFG{}, rgbBG{}; // underline color doesn't work on Windows/Ubuntu

            TextStyle(const std::vector<TFlag> &_properties = {},
                      TColor _fg_col8 = TColor::Default, TColor _bg_col8 = TColor::Default,
                      Col24 _fg_col24 = {}, Col24 _bg_col24 = {}) {
                for (TFlag prop : _properties) {
                    if (prop == TFlag::BG_RGB8 || prop == TFlag::BG_RGB24)
                        set(prop, _bg_col8, _bg_col24);
                    else
                        set(prop, _fg_col8, _fg_col24);
                }
            }


            // 0 - bold, 1 - faint, 2 - color mode, 3-4 - fg rgb8/24, 5-6 - bg rgb8/24,
            // 7 - hide, 8 - strikeout, 9 - overline, 10-11 - underline, 12-13 - blinking,
            // 14 - Italic, 15 - Gothic
            template <class T>
            static uint8_t FlagOffset() {
                if (std::is_same<T, Emphasis >::value)
                    return  0;
                if (std::is_same<T, ColorMode>::value)
                    return  2;
                if (std::is_same<T, RGBModeFG>::value)
                    return  3;
                if (std::is_same<T, RGBModeBG>::value)
                    return  5;
                if (std::is_same<T, HideMode >::value)
                    return  7;
                if (std::is_same<T, Strikeout>::value)
                    return  8;
                if (std::is_same<T, Overline >::value)
                    return  9;
                if (std::is_same<T, Underline>::value)
                    return 10;
                if (std::is_same<T, Blinking >::value)
                    return 12;
                if (std::is_same<T, FontStyle>::value)
                    return 14;
                throw std::invalid_argument(SVKFW_WRAPERR("TermCtrl :: TextStyle :: FlagOffset", "template argument isn't a property type"));
            }

            template <class T>
            static typename std::enable_if<
                std::is_same<T, Emphasis >::value || std::is_same<T, Underline>::value ||
                std::is_same<T, Blinking >::value || std::is_same<T, FontStyle>::value ||
                std::is_same<T, ColorMode>::value || std::is_same<T, Strikeout>::value ||
                std::is_same<T, RGBModeBG>::value || std::is_same<T, RGBModeFG>::value ||
                std::is_same<T, HideMode >::value || std::is_same<T, Overline >::value,
            uint8_t>::type FlagSizeBits() {
                if (std::is_same<T,  Emphasis>::value || std::is_same<T, Underline>::value ||
                    std::is_same<T,  Blinking>::value || std::is_same<T, FontStyle>::value ||
                    std::is_same<T, RGBModeBG>::value || std::is_same<T, RGBModeFG>::value)
                    return 2;
                return 1;
            }

            template <class T>
            void set(T _property) {
                flags = Bits::nullify(flags, FlagOffset<T>(), FlagSizeBits<T>()) | (uint16_t(_property) << FlagOffset<T>());
            }
            template <class T>
            T get() const {
                return (T) Bits::unpack(flags, FlagOffset<T>(), FlagSizeBits<T>());
            }

            void set(TFlag _property, TColor _col8 = TColor::Default, Col24 _col24 = {}) {
                switch (_property) {
                    case TFlag::BOLD:             set(Emphasis::Bold); break;
                    case TFlag::FAINT:            set(Emphasis::Faint); break;
                    case TFlag::COLORS_INV:       set(ColorMode::Inverted); break;
                    case TFlag::FG_RGB8:          if (_col8 != TColor::Default) setColorFG(_col8); break;
                    case TFlag::FG_RGB24:         setColorFG(_col24.r, _col24.g, _col24.b); break;
                    case TFlag::BG_RGB8:          if (_col8 != TColor::Default) setColorBG(_col8); break;
                    case TFlag::BG_RGB24:         setColorBG(_col24.r, _col24.g, _col24.b); break;
                    case TFlag::HIDDEN:           set(HideMode::Hide); break;
                    case TFlag::STRIKEOUT:        set(Strikeout::On); break;
                    case TFlag::OVERLINE:         set(Overline::On); break;
                    case TFlag::SINGLE_UNDERLINE: set(Underline::On); break;
                    case TFlag::DOUBLE_UNDERLINE: set(Underline::Double); break;
                    case TFlag::SLOW_BLINK:       set(Blinking::Slow); break;
                    case TFlag::FAST_BLINK:       set(Blinking::Fast); break;
                    case TFlag::ITALIC:           set(FontStyle::Italic); break;
                    case TFlag::GOTHIC:           set(FontStyle::Gothic); break;
                    case TFlag::BOLDFAINT:        set(Emphasis::BoldFaint); break;
                }
            }
            template <TFlag TERM_PROPERTY>
            bool getActive() const {
                if (TERM_PROPERTY <  TFlag::HAS_UNDERLINE)
                    return flags & (1u << uint8_t(TERM_PROPERTY));
                if (TERM_PROPERTY == TFlag::HAS_UNDERLINE)
                    return flags & (Bits::Mask16::FirstBits<2u>() << FlagOffset<Underline>());
                if (TERM_PROPERTY == TFlag::HAS_FONTSTYLE)
                    return flags & (Bits::Mask16::FirstBits<2u>() << FlagOffset<FontStyle>());
                if (TERM_PROPERTY == TFlag::BOLDFAINT)
                    return flags & (Bits::Mask16::FirstBits<2u>() << FlagOffset<Emphasis >());
                if (TERM_PROPERTY == TFlag::HAS_FG_COLOR)
                    return flags & (Bits::Mask16::FirstBits<2u>() << FlagOffset<RGBModeFG>());
                if (TERM_PROPERTY == TFlag::HAS_BG_COLOR)
                    return flags & (Bits::Mask16::FirstBits<2u>() << FlagOffset<RGBModeBG>());
                if (TERM_PROPERTY == TFlag::HAS_BLINK)
                    return flags & (Bits::Mask16::FirstBits<2u>() << FlagOffset<Blinking >());
                throw std::invalid_argument(SVKFW_WRAPERR("TermCtrl :: TextStyle :: getActive flag property", "TFlag template argument is out of bounds"));
            }

            void setColorFG(TermCtrl::TColor _fg_color) {
                fgcolor = _fg_color;
                set(TermCtrl::RGBModeFG::RGB8);
            }
            void setColorFG(uint8_t _r, uint8_t _g, uint8_t _b) {
                rgbFG = {_r, _g, _b};
                set(TermCtrl::RGBModeFG::RGB24);
            }
            void setColorBG(TermCtrl::TColor _bg_color) {
                bgcolor = _bg_color;
                set(TermCtrl::RGBModeBG::RGB8);
            }
            void setColorBG(uint8_t _r, uint8_t _g, uint8_t _b) {
                rgbBG = {_r, _g, _b};
                set(TermCtrl::RGBModeBG::RGB24);
            }
        }; // TextStyle END
    }; // TermCtrl END
}; // Simple END

#endif