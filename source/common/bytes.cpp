#include "common/bytes.h"

namespace Simple {
    namespace Bytes {

        EndiannessCheckSct::EndiannessCheckSct() : is_big_endian   {reinterpret_cast<const unsigned char*>(&byte_check_)[0] == 0x01},
                               is_little_endian{reinterpret_cast<const unsigned char*>(&byte_check_)[0] == 0x04} {}

        EndiannessCheckSct Endianness;

        BytestreamReader::BytestreamReader(const std::vector<unsigned char> &_byte_stream) : byte_stream{_byte_stream}, byte_offset{0u} {}

        bool BytestreamReader::moveOffset(int _offset) {
            int __clamped_offset = int(byte_stream.size()) - int(byte_offset);
            __clamped_offset =          _offset >=  __clamped_offset ?  __clamped_offset : _offset;
            __clamped_offset = __clamped_offset <  -int(byte_offset) ? -int(byte_offset) : __clamped_offset;
            byte_offset    += __clamped_offset;
            return _offset == __clamped_offset;
        }

        const unsigned char *BytestreamReader::getData(int _offset) {
            if (_offset) moveOffset(_offset);
            return byte_stream.data() + byte_offset;
        }

        BytestreamWriter::BytestreamWriter(std::vector<unsigned char> &_byte_stream) : byte_stream{_byte_stream}, byte_offset{0u} {}

        bool BytestreamWriter::moveOffset(int _offset) {
            int __clamped_offset = int(byte_stream.size()) - int(byte_offset);
            __clamped_offset =          _offset >=  __clamped_offset ?  __clamped_offset : _offset;
            __clamped_offset = __clamped_offset <  -int(byte_offset) ? -int(byte_offset) : __clamped_offset;
            byte_offset    += __clamped_offset;
            return _offset == __clamped_offset;
        }

        unsigned char *BytestreamWriter::getData(int _offset) {
            if (_offset) moveOffset(_offset);
            return byte_stream.data() + byte_offset;
        }
    }; // Bytes END
}; // Simple END
