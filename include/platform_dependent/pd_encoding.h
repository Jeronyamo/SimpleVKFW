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
    }; // Enc END
}; // Simple END

#endif
