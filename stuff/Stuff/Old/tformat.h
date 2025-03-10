#ifndef SVKFW_FMT_H
#define SVKFW_FMT_H

// Some wrapper for now removed fmt library

namespace Simple {
    namespace TFormat {
//         const fmt::text_style Default_style;


//         struct FmtText {
//             const char *text;
//             fmt::text_style style;

//             FmtText(const FmtText &_ftext) : text{_ftext.text}, style{_ftext.style} {}
//             FmtText(const char *_text = "", const fmt::text_style &_style = Default_style) :
//                         text{_text}, style{_style} {}
//         };


//         struct FmtOutput {
//             FILE *_outputs;
//             fmt::text_style style;

//             FmtOutput& operator<<(const fmt::text_style &_ts) {
//                 style = _ts;
//                 return *this;
//             }

//             FmtOutput& operator<<(char _ch) {
//                 char _text[2] = { _ch, '\0'};
//                 fmt::print(_outputs, style, _text);
//                 return *this;
//             }

//             FmtOutput& operator<<(const char *_text) {
//                 fmt::print(_outputs, style, _text);
//                 return *this;
//             }

//             FmtOutput& operator<<(const std::string &_text) {
//                 fmt::print(_outputs, style, _text);
//                 return *this;
//             }

//             template <typename T>
//             FmtOutput& operator<<(const typename std::enable_if<std::is_arithmetic<T>::value, T>::type &_to_text) {
//                 std::string __t{std::to_string(_to_text)};
//                 fmt::print(_outputs, style, __t.c_str());
//                 return *this;
//             }

//             FmtOutput& operator<<(const FmtText &_ft) {
//                 fmt::print(_outputs, _ft.style, _ft.text);
//                 return *this;
//             }
//         };
//         FmtOutput FmtOut{ stdout };
//         FmtOutput FmtWrn{ outswarn };
//         FmtOutput FmtErr{ stderr };


// // TODO: stuff for this function: FMTText gatherer, weak test_style type
//         void printAll(FILE *_f, const FmtText *_texts, uint _count) {
//             for (uint i = 0u; i < _count; ++i)
//                 fmt::print(_f, _texts[i].style, _texts[i].text);
//         }
    };
};

#endif