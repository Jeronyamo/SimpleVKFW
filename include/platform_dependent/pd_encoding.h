#ifndef SVKFW_PD_ENCODING_H
#define SVKFW_PD_ENCODING_H

#include <cstddef>
#include <string>


namespace Simple {
    namespace Enc {
        // Shift-JIS (code page 932, as in MMD files: VMD names, PMD) to UTF-8, through the system converter
        // (WinAPI on Windows, iconv elsewhere). Stops at the first zero byte or after '_size' bytes; a character cut by
        // the end of a fixed-size field (VMD names are 15/20 bytes) is dropped
        std::string shiftJISToUTF8(const char *_str, size_t _size);
        std::string shiftJISToUTF8(const std::string &_str);
        // UTF-8 to Shift-JIS (CP932), e.g. names written to VMD files; characters CP932 lacks become '?'. With '_max_bytes' > 0
        // the result is cut to at most that many bytes without splitting a two-byte character (VMD bone names: 15 bytes)
        std::string utf8ToShiftJIS(const std::string &_str, size_t _max_bytes = 0u);
    }; // Enc END
}; // Simple END

#endif
