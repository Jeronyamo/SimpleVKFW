#ifndef SVKFW_BYTESTRING_H
#define SVKFW_BYTESTRING_H

#include <vector>
#include <cstring>
#include <string>

#include "common/terminal.h"


namespace Simple {
    namespace Enc {
        typedef enum EncType : uint32_t { ENC_UNKNOWN, ENC_RAW, ENC_ASCII, ENC_UTF8, ENC_UTF16, ENC_UTF32 } EncType; // EncType

        struct Unicode {
            std::vector<unsigned char> bytestring;
            EncType curr_encoding;

            Unicode();

            Unicode(const char *_i_str, uint32_t _size_bytes = 0u, EncType _encoding = ENC_UNKNOWN);

            Unicode(const std::string &_i_str = {}, EncType _encoding = ENC_UNKNOWN);



            const char *c_str();


        // Encoding checks

            EncType determineEncoding();

            bool isASCII();
            // the first len_char-1 characters are guaranteed to be ASCII-complied
            bool isASCII(uint32_t *_len_char);

            // the first len_char-1 characters (counting multi-byte as 1) are guaranteed to be UTF8-complied
            bool isUTF8(uint32_t *_len_char = nullptr);

            // the first len_char-1 characters (counting multi-byte as 1) are guaranteed to be UTF16-complied
            bool isUTF16(uint32_t *_len_char = nullptr);

            bool isUTF32(uint32_t *_len_char = nullptr);


        // Decoding

            bool decode(EncType _curr_encoding = ENC_UNKNOWN);

            bool decodeBytesASCII(bool _check_ascii);

            bool decodeUTF8();

            bool decodeUTF16();


        // Encoding

            bool encodeUTF16();
        }; // Unicode END
    }; // Enc END
}; // Simple END

#endif
