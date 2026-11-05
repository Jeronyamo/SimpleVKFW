#include "data/bytestring.h"

namespace Simple {
    namespace Enc {
        namespace {
            uint16_t readUnit16(const unsigned char *_bytes, bool _big_endian) {
                uint16_t __unit;
                memcpy(&__unit, _bytes, sizeof(__unit));
                return _big_endian ? Bytes::Endianness.bigEndianToCurrent(__unit) : Bytes::Endianness.littleEndianToCurrent(__unit);
            }

            void writeUnit16(unsigned char *_bytes, uint16_t _unit, bool _big_endian) {
                // the conversion is symmetric: swapping 'current' order to the requested one
                _unit = _big_endian ? Bytes::Endianness.bigEndianToCurrent(_unit) : Bytes::Endianness.littleEndianToCurrent(_unit);
                memcpy(_bytes, &_unit, sizeof(_unit));
            }

            // Returns the number of bytes taken by a byte order mark (0 or 2) and updates the byte order
            uint32_t checkBOM16(const std::vector<unsigned char> &_bytes, bool &_big_endian) {
                if (_bytes.size() < 2u) return 0u;
                if (_bytes[0] == 0xFF && _bytes[1] == 0xFE) { _big_endian = false; return 2u; }
                if (_bytes[0] == 0xFE && _bytes[1] == 0xFF) { _big_endian = true;  return 2u; }
                return 0u;
            }
        } // anonymous


        Unicode::Unicode() {}

        Unicode::Unicode(const char *_i_str, uint32_t _size_bytes, EncType _encoding) {
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

        Unicode::Unicode(const std::string &_i_str, EncType _encoding) {
            bytestring.resize(_i_str.size()+1);
            memcpy(bytestring.data(), _i_str.data(), _i_str.size());
            bytestring[_i_str.size()] = '\0';

            if (_encoding == ENC_UNKNOWN)
                determineEncoding();
        }

        const char *Unicode::c_str() { return (const char*) bytestring.data(); }

        EncType Unicode::determineEncoding() {
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

        bool Unicode::isASCII() {
            for (unsigned char ch : bytestring)
                if (ch >= 128) return false;
            return true;
        }

        bool Unicode::isASCII(uint32_t *_len_char) {
            for (*_len_char = 0u; *_len_char < bytestring.size(); ++*_len_char)
                if (bytestring[*_len_char] >= 128) return false;
            return true;
        }

        bool Unicode::isUTF8(uint32_t *_len_char) {
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

        bool Unicode::isUTF16(uint32_t *_len_char, bool _big_endian) {
            if ((bytestring.size() & 1) != 0)
                return false;

            bool __prev_high_surrogate = false;
            for (uint32_t i = checkBOM16(bytestring, _big_endian); i < bytestring.size(); i += 2) {
                uint16_t __val = readUnit16(bytestring.data() + i, _big_endian);
                bool  __high_surrogate = __val >= 0xD800 && __val <= 0xDBFF;
                bool   __low_surrogate = __val >= 0xDC00 && __val <= 0xDFFF;

                if (__prev_high_surrogate && !__low_surrogate) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Enc :: Unicode :: isUTF16", "unpaired high surrogate on byte %d\n"), i - 2);
                    return false;
                }
                if (!__prev_high_surrogate && __low_surrogate) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Enc :: Unicode :: isUTF16", "unpaired low surrogate on byte %d\n"), i);
                    return false;
                }
                __prev_high_surrogate = __high_surrogate;
                if (!__high_surrogate && _len_char) ++*_len_char;
            }
            if (__prev_high_surrogate) {
                fprintf(svkfwwarn, "%s", SVKFW_WRAPWARN("Enc :: Unicode :: isUTF16", "unpaired high surrogate at the end\n"));
                return false;
            }
            return true;
        }

        bool Unicode::isUTF32(uint32_t *_len_char) {
            if ((bytestring.size() & 3) != 0)
                return false;

            for (uint32_t i = 0u; i < bytestring.size(); i += 4)
                if (bytestring[i] != 0)
                    return false;

            if (_len_char) *_len_char = bytestring.size() >> 2;
            return true;
        }

        bool Unicode::decode(EncType _curr_encoding) {
            if (_curr_encoding == ENC_UNKNOWN) {
                if (curr_encoding == ENC_UNKNOWN)
                    determineEncoding();
                _curr_encoding = curr_encoding;
            }

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
                default:;
            }
            return false;
        }

        bool Unicode::decodeBytesASCII(bool _check_ascii) {
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

        bool Unicode::decodeUTF8() {
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

        // Note: unpaired surrogates become U+FFFD
        bool Unicode::decodeUTF16(bool _big_endian) {
            if ((bytestring.size() & 1u) != 0u) {
                fprintf(svkfwwarn, "%s", SVKFW_WRAPWARN("Enc :: Unicode :: decodeUTF16", "odd number of bytes\n"));
                return false;
            }
            std::vector<unsigned char> __bytestring{bytestring};
            const uint32_t __bom = checkBOM16(__bytestring, _big_endian);
            bytestring.clear();
            bytestring.reserve(2u * __bytestring.size());
            for (size_t i = __bom; i < __bytestring.size(); i += 2u) {
                uint32_t __ch = readUnit16(__bytestring.data() + i, _big_endian);
                if (__ch >= 0xD800u && __ch <= 0xDBFFu && i + 2u < __bytestring.size()) {
                    const uint32_t __low = readUnit16(__bytestring.data() + i + 2u, _big_endian);
                    if (__low >= 0xDC00u && __low <= 0xDFFFu) { __ch = 0x10000u + ((__ch - 0xD800u) << 10u) + (__low - 0xDC00u); i += 2u; }
                    else __ch = 0xFFFDu;
                } else if (__ch >= 0xD800u && __ch <= 0xDFFFu) __ch = 0xFFFDu;
                bytestring.push_back((unsigned char)( __ch >> 24));
                bytestring.push_back((unsigned char)((__ch >> 16) & 0xFFu));
                bytestring.push_back((unsigned char)((__ch >>  8) & 0xFFu));
                bytestring.push_back((unsigned char)( __ch        & 0xFFu));
            }
            curr_encoding = ENC_UTF32;
            return true;
        }

        bool Unicode::encodeUTF8() {
            if (curr_encoding != ENC_UTF32) {
                fprintf(svkfwwarn, "%s", SVKFW_WRAPWARN("Enc :: Unicode :: encodeUTF8", "the string is not decoded\n"));
                return false;
            }
            std::vector<unsigned char> __bytestring{bytestring};
            bytestring.clear();
            bytestring.reserve(__bytestring.size());
            for (size_t i = 0u; i + 3u < __bytestring.size(); i += 4u) {
                const uint32_t __ch = (uint32_t(__bytestring[i]) << 24) | (uint32_t(__bytestring[i + 1u]) << 16) |
                                      (uint32_t(__bytestring[i + 2u]) << 8) | __bytestring[i + 3u];
                if (__ch < 0x80u) bytestring.push_back((unsigned char)__ch);
                else if (__ch < 0x800u) {
                    bytestring.push_back((unsigned char)(0xC0u | (__ch >> 6)));
                    bytestring.push_back((unsigned char)(0x80u | (__ch & 0x3Fu)));
                } else if (__ch < 0x10000u) {
                    bytestring.push_back((unsigned char)(0xE0u | (__ch >> 12)));
                    bytestring.push_back((unsigned char)(0x80u | ((__ch >> 6) & 0x3Fu)));
                    bytestring.push_back((unsigned char)(0x80u | (__ch & 0x3Fu)));
                } else {
                    bytestring.push_back((unsigned char)(0xF0u | (__ch >> 18)));
                    bytestring.push_back((unsigned char)(0x80u | ((__ch >> 12) & 0x3Fu)));
                    bytestring.push_back((unsigned char)(0x80u | ((__ch >> 6) & 0x3Fu)));
                    bytestring.push_back((unsigned char)(0x80u | (__ch & 0x3Fu)));
                }
            }
            curr_encoding = ENC_UTF8;
            return true;
        }

        bool Unicode::encodeUTF16(bool _big_endian) {
            if (curr_encoding != ENC_UTF32) {
                fprintf(svkfwwarn, SVKFW_WRAPWARN("Enc :: Unicode :: encodeUTF16", "the string is not decoded\n"));
                return false;
            }

            // In place: a UTF-16 character never takes more bytes than its UTF-32 form
            uint32_t __utf16_i = 0u;
            for (uint32_t i = 0u; i + 3u < bytestring.size(); i += 4u) {
                uint32_t __char32 = (uint32_t(bytestring[i  ]) << 24) + (uint32_t(bytestring[i+1]) << 16) +
                                    (uint32_t(bytestring[i+2]) <<  8) +  uint32_t(bytestring[i+3]);

                if (__char32 >= 0x10000u) {
                    __char32 -= 0x10000u;
                    writeUnit16(bytestring.data() + __utf16_i     , uint16_t(0xD800u + ((__char32 >> 10) & 0x3FFu)), _big_endian);
                    writeUnit16(bytestring.data() + __utf16_i + 2u, uint16_t(0xDC00u + ( __char32        & 0x3FFu)), _big_endian);
                    __utf16_i += 4u;
                }
                else {
                    writeUnit16(bytestring.data() + __utf16_i, uint16_t(__char32), _big_endian);
                    __utf16_i += 2u;
                }
            }
            bytestring.resize(__utf16_i);
            curr_encoding = ENC_UTF16;
            return true;
        }
    }; // Enc END
}; // Simple END
