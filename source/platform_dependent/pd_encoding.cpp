#include "platform_dependent/pd_encoding.h"
#include "common/terminal.h"

#include <cstring>
#include <vector>

#ifdef WIN32
#include <Windows.h>

        static std::string utf8ToShiftJISFull_(const std::string &_str) {
            if (_str.empty()) return std::string();
            const int __wide_size = MultiByteToWideChar(CP_UTF8, 0, _str.data(), int(_str.size()), NULL, 0);
            if (__wide_size <= 0) return std::string();
            std::vector<wchar_t> __wide(__wide_size);
            MultiByteToWideChar(CP_UTF8, 0, _str.data(), int(_str.size()), __wide.data(), __wide_size);
            const int __size = WideCharToMultiByte(932, 0, __wide.data(), __wide_size, NULL, 0, NULL, NULL);
            if (__size <= 0) return std::string();
            std::string __res(size_t(__size), '\0');
            WideCharToMultiByte(932, 0, __wide.data(), __wide_size, &__res[0], __size, NULL, NULL);
            return __res;
        }
#else
#include <iconv.h>
#include <cerrno>
#endif


namespace Simple {
    namespace Enc {
        namespace {
            size_t zeroTerminatedSize(const char *_str, size_t _size) {
                const void *__zero = memchr(_str, 0, _size);
                return __zero ? size_t(static_cast<const char*>(__zero) - _str) : _size;
            }
        } // anonymous

        std::string shiftJISToUTF8(const std::string &_str) { return shiftJISToUTF8(_str.data(), _str.size()); }

#ifdef WIN32
        std::string shiftJISToUTF8(const char *_str, size_t _size) {
            if (_str == nullptr) return std::string();
            int __size = int(zeroTerminatedSize(_str, _size));
            if (__size == 0) return std::string();

            // A lead byte at the very end has no trail byte: drop it, MultiByteToWideChar would make it '?'
            int __end = 0;
            for (int i = 0; i < __size; __end = i) {
                const unsigned char ch = static_cast<unsigned char>(_str[i]);
                const bool __lead = (ch >= 0x81u && ch <= 0x9Fu) || (ch >= 0xE0u && ch <= 0xFCu);
                if (__lead && i + 1 >= __size) break;
                i += __lead ? 2 : 1;
            }
            if (__end == 0) return std::string();

            const int __wide_size = MultiByteToWideChar(932, 0, _str, __end, NULL, 0);
            if (__wide_size <= 0) return std::string();
            std::vector<wchar_t> __wide(__wide_size);
            MultiByteToWideChar(932, 0, _str, __end, __wide.data(), __wide_size);

            const int __utf8_size = WideCharToMultiByte(CP_UTF8, 0, __wide.data(), __wide_size, NULL, 0, NULL, NULL);
            if (__utf8_size <= 0) return std::string();
            std::string __res(size_t(__utf8_size), '\0');
            WideCharToMultiByte(CP_UTF8, 0, __wide.data(), __wide_size, &__res[0], __utf8_size, NULL, NULL);
            return __res;
        }
#else
        std::string shiftJISToUTF8(const char *_str, size_t _size) {
            if (_str == nullptr) return std::string();
            size_t __in_left = zeroTerminatedSize(_str, _size);
            if (__in_left == 0u) return std::string();

            // CP932 is Microsoft's Shift-JIS (what MMD writes); some iconv builds know only the plain name
            iconv_t __conv = iconv_open("UTF-8", "CP932");
            if (__conv == (iconv_t)-1) __conv = iconv_open("UTF-8", "SHIFT_JIS");
            if (__conv == (iconv_t)-1) {
                fprintf(svkfwwarn, "%s", SVKFW_WRAPWARN("Enc :: shiftJISToUTF8", "iconv has no Shift-JIS converter\n"));
                return std::string(_str, __in_left);
            }

            std::string __res(__in_left * 3u, '\0'); // a CP932 character (1-2 bytes) takes at most 3 bytes in UTF-8
            char  *__in  = const_cast<char*>(_str);
            char  *__out = &__res[0];
            size_t __out_left = __res.size();
            while (__in_left > 0u && iconv(__conv, &__in, &__in_left, &__out, &__out_left) == (size_t)-1) {
                if (errno != EILSEQ) break;  // EINVAL: a character cut by the end of the field
                ++__in; --__in_left;         // skip an invalid byte
            }
            iconv_close(__conv);
            __res.resize(__res.size() - __out_left);
            return __res;
        }

        static std::string utf8ToShiftJISFull_(const std::string &_str) {
            if (_str.empty()) return std::string();
            iconv_t __conv = iconv_open("CP932", "UTF-8");
            if (__conv == (iconv_t)-1) __conv = iconv_open("SHIFT_JIS", "UTF-8");
            if (__conv == (iconv_t)-1) {
                fprintf(svkfwwarn, "%s", SVKFW_WRAPWARN("Enc :: utf8ToShiftJIS", "iconv has no Shift-JIS converter\n"));
                return _str;
            }
            std::string __res(_str.size() * 2u + 4u, '\0');
            char  *__in  = const_cast<char*>(_str.data());
            size_t __in_left = _str.size();
            char  *__out = &__res[0];
            size_t __out_left = __res.size();
            while (__in_left > 0u && iconv(__conv, &__in, &__in_left, &__out, &__out_left) == (size_t)-1) {
                if (errno != EILSEQ || __out_left == 0u) break;
                // A character CP932 doesn't have: skip its UTF-8 bytes, write '?'
                do { ++__in; --__in_left; } while (__in_left > 0u && (static_cast<unsigned char>(*__in) & 0xC0u) == 0x80u);
                *__out++ = '?'; --__out_left;
            }
            iconv_close(__conv);
            __res.resize(__res.size() - __out_left);
            return __res;
        }
#endif

        std::string utf8ToShiftJIS(const std::string &_str, size_t _max_bytes) {
            std::string __res = utf8ToShiftJISFull_(_str);
            if (_max_bytes == 0u || __res.size() <= _max_bytes) return __res;
            size_t __end = 0u;
            for (size_t i = 0u; i < __res.size();) {
                const unsigned char ch = static_cast<unsigned char>(__res[i]);
                const size_t __len = ((ch >= 0x81u && ch <= 0x9Fu) || (ch >= 0xE0u && ch <= 0xFCu)) ? 2u : 1u;
                if (i + __len > _max_bytes) break;
                i += __len;
                __end = i;
            }
            __res.resize(__end);
            return __res;
        }
    }; // Enc END
}; // Simple END
