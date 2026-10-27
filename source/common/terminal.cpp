#include "common/terminal.h"

namespace Simple {

    FILE *svkfwwarn = stderr;

    namespace TermCtrl {

        TColor getTermColor(bool _red, bool _green, bool _blue, bool _bright) {
            return TColor((uint8_t(_red       ) | (uint8_t(_green ) << 1) |
                          (uint8_t(_blue) << 2) | (uint8_t(_bright) << 3)));
        }

        TextStyle::TextStyle(const std::vector<TFlag> &_properties,
                  TColor _fg_col8, TColor _bg_col8,
                  Col24 _fg_col24, Col24 _bg_col24) {
            for (TFlag prop : _properties) {
                if (prop == TFlag::BG_RGB8 || prop == TFlag::BG_RGB24)
                    set(prop, _bg_col8, _bg_col24);
                else
                    set(prop, _fg_col8, _fg_col24);
            }
        }

        void TextStyle::set(TFlag _property, TColor _col8, Col24 _col24) {
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

        void TextStyle::setColorFG(TermCtrl::TColor _fg_color) {
            fgcolor = _fg_color;
            set(TermCtrl::RGBModeFG::RGB8);
        }

        void TextStyle::setColorFG(uint8_t _r, uint8_t _g, uint8_t _b) {
            rgbFG = {_r, _g, _b};
            set(TermCtrl::RGBModeFG::RGB24);
        }

        void TextStyle::setColorBG(TermCtrl::TColor _bg_color) {
            bgcolor = _bg_color;
            set(TermCtrl::RGBModeBG::RGB8);
        }

        void TextStyle::setColorBG(uint8_t _r, uint8_t _g, uint8_t _b) {
            rgbBG = {_r, _g, _b};
            set(TermCtrl::RGBModeBG::RGB24);
        }
    }; // TermCtrl END
}; // Simple END
