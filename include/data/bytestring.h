#ifndef SVKFW_BYTESTRING_H
#define SVKFW_BYTESTRING_H

#include <vector>
#include <cstring>
#include <string>

#include "common/terminal.h"
#include "common/bytes.h"


namespace Simple {
    namespace Enc {
        typedef enum EncType : uint32_t { ENC_UNKNOWN, ENC_RAW, ENC_ASCII, ENC_UTF8, ENC_UTF16, ENC_UTF32 } EncType; // EncType

        struct Unicode {
            std::vector<unsigned char> bytestring;
            EncType curr_encoding;

            Unicode();

            Unicode(const char *_i_str, uint32_t _size_bytes = 0u, EncType _encoding = ENC_UNKNOWN);

            Unicode(const std::string &_i_str, EncType _encoding = ENC_UNKNOWN);



            const char *c_str();


        // Encoding checks

            EncType determineEncoding();

            bool isASCII();
            // the first len_char-1 characters are guaranteed to be ASCII-complied
            bool isASCII(uint32_t *_len_char);

            // the first len_char-1 characters (counting multi-byte as 1) are guaranteed to be UTF8-complied
            bool isUTF8(uint32_t *_len_char = nullptr);

            // UTF-16 code units are read/written in the given byte order (little-endian by default, as in PMX, Windows);
            // a byte order mark at the start overrides it when checking/decoding

            // the first len_char-1 characters (counting multi-byte as 1) are guaranteed to be UTF16-complied
            bool isUTF16(uint32_t *_len_char = nullptr, bool _big_endian = false);

            bool isUTF32(uint32_t *_len_char = nullptr);


        // Decoding

            bool decode(EncType _curr_encoding = ENC_UNKNOWN);

            bool decodeBytesASCII(bool _check_ascii);

            bool decodeUTF8();

            bool decodeUTF16(bool _big_endian = false);


        // Encoding

            bool encodeUTF16(bool _big_endian = false);

            // From the decoded form (UTF-32); 'bytestring' gets no terminating zero
            bool encodeUTF8();
        }; // Unicode END
    }; // Enc END
}; // Simple END

#endif
