#ifndef SVKFW_BYTESTRING_H
#define SVKFW_BYTESTRING_H

#include <vector>
#include <string>

#include "common/terminal.h"


namespace Simple {
    namespace Enc {
        typedef enum EncType : uint32_t { ENC_UNKNOWN, ENC_RAW, ENC_ASCII, ENC_UTF8, ENC_UTF16, ENC_UTF32 } EncType; // EncType

        struct Unicode {
            std::vector<unsigned char> bytestring;
            EncType curr_encoding;

            Unicode() {}

            Unicode(const char *_i_str, uint32_t _size_bytes = 0u, EncType _encoding = ENC_UNKNOWN) {
                if (_i_str == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("Enc :: Unicode Consructor", "no string (nullptr)"));
                if (_size_bytes && _i_str[_size_bytes] != '\0')
                    throw std::invalid_argument(SVKFW_WRAPERR("Enc :: Unicode Consructor", "incorrect size, check length in bytes"));

                if (_size_bytes == 0u)
                    for (; _i_str[_size_bytes++] != '\0';);
                bytestring.resize(_size_bytes);
                memcpy(bytestring.data(), _i_str, _size_bytes);

                if (_encoding == ENC_UNKNOWN)
                    determineEncoding();
            }

            Unicode(const std::string &_i_str = {}, EncType _encoding = ENC_UNKNOWN) {
                bytestring.resize(_i_str.size()+1);
                memcpy(bytestring.data(), _i_str.data(), _i_str.size());
                bytestring[_i_str.size()] = '\0';

                if (_encoding == ENC_UNKNOWN)
                    determineEncoding();
            }



            const char *c_str() { return (const char*) bytestring.data(); }


        // Encoding checks

            EncType determineEncoding() {
                // TODO:
                if (isASCII())
                    curr_encoding = ENC_ASCII;
                else if (isUTF8())
                    curr_encoding = ENC_UTF8;
                else if (isUTF32())
                    curr_encoding = ENC_UTF32;
                else if (isUTF16())
                    curr_encoding = ENC_UTF16;
                return curr_encoding;
            }

            bool isASCII() {
                for (unsigned char ch : bytestring)
                    if (ch >= 128) return false;
                return true;
            }
            // the first len_char-1 characters are guaranteed to be ASCII-complied
            bool isASCII(uint32_t *_len_char) {
                for (*_len_char = 0u; *_len_char < bytestring.size(); ++*_len_char)
                    if (bytestring[*_len_char] >= 128) return false;
                return true;
            }

            // the first len_char-1 characters (counting multi-byte as 1) are guaranteed to be UTF8-complied
            bool isUTF8(uint32_t *_len_char = nullptr) {
                uint8_t __chars_with_10 = 0;
                uint8_t __multi_size = 0; // size of multi-byte character in bytes, for checks
                bool __ole1 = false, __ole2 = false, __tbe1 = false;

                for (unsigned char ch : bytestring) {
                    // Checks
                    if (__ole1 && ch <  0xA0) {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("Enc :: Unicode :: isUTF8", "overlong-encoded 3-byte\n"));
                        return false;
                    }
                    if (__ole2 && ch <  0x90) {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("Enc :: Unicode :: isUTF8", "overlong-encoded 4-byte\n"));
                        return false;
                    }
                    if (__tbe1 && ch >= 0x90) {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("Enc :: Unicode :: isUTF8", "encoded value is too big to be a unicode character\n"));
                        return false;
                    }
                    __ole1 = ch == 0xE0; // 3-byte, 1110....
                    __ole2 = ch == 0xF0; // 4-byte, 11110...
                    __tbe1 = ch == 0xF4;

                    // Parsing
                    if (ch < 128) { // 0..., ASCII character
                        __multi_size = 0;
                        if (_len_char) ++*_len_char;
                        continue;
                    }

                    if ((ch & 64) == 0) { // 10..., 2nd/3rd/4th byte of a character
                        if (__chars_with_10 == 0)
                            return false;
                        --__chars_with_10;
                    }
                    else { // 11..., new character
                        if (__chars_with_10 > 0)
                            return false;
                        if      ((ch & 32) == 0) { // 110...
                            if ((ch & 0x1E) == 0) { // ASCII char | 110.00001 10.111111 -- the ones after dots represent ASCII character 127 -> check for 0000 in the first byte
                                fprintf(svkfwwarn, SVKFW_WRAPWARN("Enc :: Unicode :: isUTF8", "overlong-encoded ASCII character\n"));
                                return false;
                            }
                            __chars_with_10 = 1;
                        }
                        else if ((ch & 16) == 0) { // 1110...
                            __chars_with_10 = 2;
                        }
                        else {
                            if ((ch & 0x0F) > 5) { // Larger than any unicode character
                                fprintf(svkfwwarn, SVKFW_WRAPWARN("Enc :: Unicode :: isUTF8", "encoded value is larger than any unicode character\n"));
                                return false;
                            }
                            __chars_with_10 = 3; // 11110...
                        }
                        __multi_size = __chars_with_10 + 1;
                        if (_len_char) ++*_len_char;
                    }
                }
                return true;
            }

            // the first len_char-1 characters (counting multi-byte as 1) are guaranteed to be UTF16-complied
            bool isUTF16(uint32_t *_len_char = nullptr) {
                if ((bytestring.size() & 1) != 0)
                    return false;

                bool __prev_high_surrogate = false;
                for (uint32_t i = 0u; i < bytestring.size(); i += 2) {
                    uint16_t __val = (bytestring[i] << 8) + (bytestring[i + 1]);
                    bool  __high_surrogate = __val >= 0xD800 && __val <= 0xDBFF;
                    bool   __low_surrogate = __val >= 0xDC00 && __val <= 0xDFFF;

                    if      ( __prev_high_surrogate &&  __low_surrogate) {
                        __prev_high_surrogate = false;
                        if (_len_char) ++*_len_char;
                    }
                    else if ( __prev_high_surrogate && __high_surrogate) {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("Enc :: Unicode :: isUTF16", "unpaired high surrogate on byte %d\n"), i);
                        if (_len_char) ++*_len_char;
                    }
                    else if (!__prev_high_surrogate &&  __low_surrogate) {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("Enc :: Unicode :: isUTF16", "unpaired low surrogate on byte %d\n"), i);
                        __prev_high_surrogate = false;
                        if (_len_char) ++*_len_char;
                    }
                    else if (!__prev_high_surrogate && __high_surrogate) {
                        __prev_high_surrogate = true;
                    }
                }
                return true;
            }

            bool isUTF32(uint32_t *_len_char = nullptr) {
                if ((bytestring.size() & 3) != 0)
                    return false;

                for (uint32_t i = 0u; i < bytestring.size(); i += 4)
                    if (bytestring[i] != 0)
                        return false;

                if (_len_char) *_len_char = bytestring.size() >> 2;
                return true;
            }


        // Decoding

            bool decode(EncType _curr_encoding = ENC_UNKNOWN) {
                if (_curr_encoding == ENC_UNKNOWN)
                    _curr_encoding = curr_encoding;
                if (_curr_encoding == ENC_UNKNOWN) {
                    determineEncoding();
                    _curr_encoding = curr_encoding;
                }
                if (_curr_encoding == ENC_UNKNOWN)
                    return false;

                switch (_curr_encoding) {
                    case ENC_RAW:
                    case ENC_ASCII: {
                        return decodeBytesASCII(_curr_encoding == ENC_ASCII);
                    }
                    case ENC_UTF8: {
                        return decodeUTF8();
                    }
                    case ENC_UTF16: {
                        return decodeUTF16();
                    }
                }
            }

            bool decodeBytesASCII(bool _check_ascii) {
                if (_check_ascii) {
                    uint32_t __len32 = 0u;
                    if (!isASCII(&__len32)) {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("Enc :: Unicode :: decodeASCII","not ASCII after %d ASCII characters\n"), __len32);
                        return false;
                    }
                }

                std::vector<unsigned char> __bytestring{bytestring};
                bytestring.clear();
                bytestring.resize(4*__bytestring.size(), 0);

                for (uint32_t i = 0u; i < __bytestring.size(); ++i)
                    bytestring[(i << 2) + 3] = __bytestring[i];
                curr_encoding = ENC_UTF32;
                return true;
            }

            bool decodeUTF8() {
                uint32_t __len32 = 0u;
                if (!isUTF8(&__len32)) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Enc :: Unicode :: decodeUTF8","not UTF-8 after %d UTF-8 characters\n"), __len32);
                    return false;
                }

                // printf("Before decoding: size = %d\n", __len32);

                std::vector<unsigned char> __bytestring{bytestring};
                bytestring.clear();
                bytestring.resize(4*__len32, 0);

                uint32_t __utf32_i = 0u;
                uint32_t __utf32_ch = 0u;
                uint8_t  __chars_with_10 = 0u;
                for (unsigned char ch : __bytestring) {
                    if (ch < 128) {
                        bytestring[__utf32_i + 3] = ch;
                        __utf32_i += 4u;
                    }
                    else if ((ch & 0xC0) == 0x80) {
                        __chars_with_10 -= 1;
                        __utf32_ch |= (ch & 0x3F) << (6 * __chars_with_10); // 2nd-4th bytes use 6 least significant bits to store a character
                        if (__chars_with_10 == 0u) {
                            bytestring[__utf32_i + 0] =  __utf32_ch >> 24;
                            bytestring[__utf32_i + 1] = (__utf32_ch >> 16) & 0xFF;
                            bytestring[__utf32_i + 2] = (__utf32_ch >>  8) & 0xFF;
                            bytestring[__utf32_i + 3] =  __utf32_ch        & 0xFF;
                            __utf32_ch = 0u;
                            __utf32_i += 4u;
                        }
                    }
                    else if ((ch & 0xE0) == 0xC0) { // 110...
                        __chars_with_10 = 1;
                        __utf32_ch |= (ch & 0x1F) << (6 * __chars_with_10);
                    }
                    else if ((ch & 0xF0) == 0xE0) { // 1110...
                        __chars_with_10 = 2;
                        __utf32_ch |= (ch & 0x0F) << (6 * __chars_with_10);
                    }
                    else if ((ch & 0xF8) == 0xF0) { // 11110...
                        __chars_with_10 = 3;
                        __utf32_ch |= (ch & 0x07) << (6 * __chars_with_10);
                    }
                    else throw std::runtime_error(SVKFW_WRAPERR("Enc :: Unicode :: decodeUTF8", "unexpected decoding error on byte with ord ") + std::to_string(uint32_t(uint8_t(ch))));
                }
                curr_encoding = ENC_UTF32;
                return true;
            }

            bool decodeUTF16() {
                return false;
            }


        // Encoding

            bool encodeUTF16() {
                if (curr_encoding != ENC_UTF32) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Enc :: Unicode :: encodeUTF16", "the string is not decoded\n"));
                    return false;
                }

                // printf("Before encoding: size = %d\n", bytestring.size());

                uint32_t __utf16_i = 0u;
                for (uint32_t i = 0u; i < bytestring.size(); i += 4u) {
                    uint32_t __char32 = (bytestring[i  ] << 24) + (bytestring[i+1] << 16) +
                                        (bytestring[i+2] <<  8) +  bytestring[i+3];

                    if (bytestring[i+1]) {
                        __char32 -= (1u << 16);
                        uint16_t __high_surrogate = 0xD800 + ((__char32 >> 10) & 0x3FF);
                        uint16_t  __low_surrogate = 0xDC00 + ((__char32      ) & 0x3FF);
                        bytestring[__utf16_i+1  ] = __high_surrogate >>   8;
                        bytestring[__utf16_i] = __high_surrogate & 0xFF;
                        bytestring[__utf16_i+3] =  __low_surrogate >>   8;
                        bytestring[__utf16_i+2] =  __low_surrogate & 0xFF;
                        __utf16_i += 4;
                    }
                    else {
                        bytestring[__utf16_i+1  ] = bytestring[i+2];
                        bytestring[__utf16_i] = bytestring[i+3];
                        __utf16_i += 2;
                    }
                }
                bytestring.resize(__utf16_i);
                curr_encoding = ENC_UTF16;
                return true;
            }
        }; // Unicode END
    }; // Enc END
}; // Simple END

#endif
