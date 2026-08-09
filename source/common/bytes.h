#ifndef SVKFW_BYTES_H
#define SVKFW_BYTES_H

#include <cstdint>
#include <vector>

namespace Simple {
    namespace Bytes {
        struct EndiannessCheckSct {
            const uint32_t byte_check_ = 0x01020304;
            const uint32_t is_big_endian;
            const uint32_t is_little_endian;

            EndiannessCheckSct() : is_big_endian   {reinterpret_cast<const unsigned char*>(&byte_check_)[0] == 0x01},
                                   is_little_endian{reinterpret_cast<const unsigned char*>(&byte_check_)[0] == 0x04} {}

            template <typename T>
            static T SwapBytes(T _in_value) {
                unsigned char *__byte_repr = reinterpret_cast<unsigned char*>(&_in_value);
                for (int i = 0; i < (sizeof(T)>>1); ++i)
                    std::swap(__byte_repr[i], __byte_repr[sizeof(T)-1-i]);

                return _in_value;
            }
            template <typename T>
            static void SwapBytesInplace(T &_in_value) {
                unsigned char *__byte_repr = reinterpret_cast<unsigned char*>(&_in_value);
                for (int i = 0; i < (sizeof(T)>>1); ++i)
                    std::swap(__byte_repr[i], __byte_repr[sizeof(T)-1-i]);
            }

            template <typename T>
            T littleEndianToCurrent(T _in_value) const {
                return is_big_endian ? SwapBytes(_in_value) : _in_value;
            }
            template <typename T>
            void littleEndianToCurrentInplace(T &_in_value) const {
                if (is_big_endian) SwapBytesInplace(_in_value);
            }

            template <typename T>
            T bigEndianToCurrent(T _in_value) const {
                return is_little_endian ? SwapBytes(_in_value) : _in_value;
            }
            template <typename T>
            void bigEndianToCurrentInplace(T &_in_value) const {
                if (is_little_endian) SwapBytesInplace(_in_value);
            }
        } Endianness;


//  ============  Bytestream  ============  \\

    struct BytestreamReader {
        const std::vector<unsigned char> &byte_stream;
        uint32_t byte_offset;
        uint32_t pad;

        BytestreamReader(const std::vector<unsigned char> &_byte_stream) : byte_stream{_byte_stream}, byte_offset{0u} {}

        template <typename T>
        bool readValue(T &_in_value, bool _swap_bytes) {
            bool __res_possible = byte_offset + sizeof(T) <= byte_stream.size();

            if (__res_possible) {
                memcpy(&_in_value, byte_stream.data() + byte_offset, sizeof(T));
                if (_swap_bytes) Util::Endianness.SwapBytesInplace(_in_value);
                byte_offset += sizeof(T);
            }
            return __res_possible;
        }

        template <typename T>
        bool readArray(T *_in_array, uint32_t _len_elems, bool _swap_bytes) {
            bool __res_possible = byte_offset + sizeof(T) * _len_elems <= byte_stream.size();

            if (__res_possible) {
                memcpy(_in_array, byte_stream.data() + byte_offset, sizeof(T) * _len_elems);
                if (_swap_bytes)
                    for (uint32_t i = 0u; i < _len_elems; ++i)
                        Util::Endianness.SwapBytesInplace(_in_array[i]);
                byte_offset += sizeof(T) * _len_elems;
            }
            return __res_possible;
        }

        bool moveOffset(int _offset) {
            int __clamped_offset = int(byte_stream.size()) - int(byte_offset) - 1;
            __clamped_offset =          _offset >=  __clamped_offset ?  __clamped_offset : _offset;
            __clamped_offset = __clamped_offset <  -int(byte_offset) ? -int(byte_offset) : __clamped_offset;
            byte_offset    += __clamped_offset;
            return _offset == __clamped_offset;
        }

        const unsigned char *getData(int _offset = 0u) {
            if (_offset) moveOffset(_offset);
            return byte_stream.data() + byte_offset;
        }
    }; // BytestreamReader END

    struct BytestreamWriter {
        std::vector<unsigned char> &byte_stream;
        uint32_t byte_offset;
        uint32_t pad;

        BytestreamWriter(std::vector<unsigned char> &_byte_stream) : byte_stream{_byte_stream}, byte_offset{0u} {}

        // Returns true if bytes were added to stream
        template <typename T>
        bool writeValue(T _in_value, bool _swap_bytes) {
            bool __res_addnew = byte_offset + sizeof(T) > byte_stream.size();
            if (__res_addnew)
                byte_stream.insert(byte_stream.end(), size_t(byte_offset + sizeof(T) - byte_stream.size()), '\0');

            if (_swap_bytes) Util::Endianness.SwapBytesInplace(_in_value);
            memcpy(byte_stream.data() + byte_offset, &_in_value, sizeof(T));
            byte_offset += sizeof(T);

            return __res_addnew;
        }

        template <typename T>
        bool writeArray(const T *_in_array, uint32_t _len_elems, bool _swap_bytes) {
            bool __res_addnew = byte_offset + sizeof(T) * _len_elems > byte_stream.size();
            if (__res_addnew)
                byte_stream.insert(byte_stream.end(), size_t(byte_offset + sizeof(T) * _len_elems - byte_stream.size()), '\0');

            memcpy(byte_stream.data() + byte_offset, _in_array, sizeof(T) * _len_elems);

            unsigned char *__t_repr = reinterpret_cast<T*>(byte_stream.data() + byte_offset);
            if (_swap_bytes)
                for (uint32_t i = 0u; i < _len_elems; ++i)
                    Util::Endianness.SwapBytesInplace(__t_repr[i]);

            byte_offset += sizeof(T) * _len_elems;
            return __res_addnew;
        }

        bool moveOffset(int _offset) {
            int __clamped_offset = int(byte_stream.size()) - int(byte_offset) - 1;
            __clamped_offset =          _offset >=  __clamped_offset ?  __clamped_offset : _offset;
            __clamped_offset = __clamped_offset <  -int(byte_offset) ? -int(byte_offset) : __clamped_offset;
            byte_offset    += __clamped_offset;
            return _offset == __clamped_offset;
        }

        unsigned char *getData(int _offset = 0u) {
            if (_offset) moveOffset(_offset);
            return byte_stream.data() + byte_offset;
        }
    }; // BytestreamWriter END

    }; // Bytes END
}; // Simple END



#endif