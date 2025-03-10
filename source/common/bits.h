#ifndef SVKFW_BITS_H
#define SVKFW_BITS_H

#include <climits>
#include <cstdint>
#include <type_traits>


namespace Simple {
//  ============  Utilities  ============  \\

    namespace Util {
        template <typename T>
        struct PackS {
            T bits;
            uint8_t offset;
        };
    };


//  ============  Bits  ============  \\

    namespace Bits {
        template <typename T>
        struct Mask {
            static_assert(std::is_unsigned<T>::value, "SVKFW Error: Bits :: Mask - template type must be unsigned integer of any size");

            template <uint8_t _BIT1, uint8_t _BIT2, uint8_t... _ARGS>
            static inline T FromBits() {
                static_assert(sizeof(T) * CHAR_BIT >  _BIT1, "SVKFW Error: Bits :: Mask :: FromBits - bit index exceeds max for this type");
                return FromBits<_BIT2, _ARGS...>() | (1u << _BIT1);
            }
            template <uint8_t _BIT>
            static inline T FromBits() {
                static_assert(sizeof(T) * CHAR_BIT >  _BIT , "SVKFW Error: Bits :: Mask :: FromBits - bit index exceeds max for this type");
                return (1u << _BIT);
            }
            template <uint8_t _BITS>
            static inline T FirstBits() {
                static_assert(sizeof(T) * CHAR_BIT >= _BITS, "SVKFW Error: Bits :: Mask :: FirstBits - bit number exceeds max for this type");
                return _BITS ? (1u << _BITS) - 1u : 0u;
            }
            template <uint8_t _BITS>
            static inline T LastBits() {
                static_assert(sizeof(T) * CHAR_BIT >= _BITS, "SVKFW Error: Bits :: Mask :: LastBits - bit number exceeds max for this type");
                return _BITS ? T(-1) ^ ((1u << (sizeof(T) * CHAR_BIT - _BITS)) - 1u) : 0u;
            }


            static inline T FromBits(const std::initializer_list<uint8_t> &_BITS) {
                T __res = 0u;
                for (uint8_t i : _BITS) {
                    if (sizeof(T) * CHAR_BIT > i)
                        throw std::invalid_argument("SVKFW Error: Bits :: Mask :: FromBits - bit index exceeds max for this type");
                    __res |= (1u << i);
                }
                return __res;
            }
            static inline T FirstBits(uint8_t _BITS) {
                if (sizeof(T) * CHAR_BIT >= _BITS)
                    throw std::invalid_argument("SVKFW Error: Bits :: Mask :: FirstBits - bit number exceeds max for this type");
                return _BITS ? (1u << _BITS) - 1u : 0u;
            }
            static inline T LastBits(uint8_t _BITS) {
                if (sizeof(T) * CHAR_BIT >= _BITS)
                    throw std::invalid_argument("SVKFW Error: Bits :: Mask :: LastBits - bit number exceeds max for this type");
                return _BITS ? T(-1) ^ ((1u << (sizeof(T) * CHAR_BIT - _BITS)) - 1u) : 0u;
            }
            static inline T Bit(uint8_t _bit) {
                if (sizeof(T) * CHAR_BIT > _bit)
                    throw std::invalid_argument("SVKFW Error: Bits :: Mask :: Bit - bit index exceeds max for this type");
                return 1u << _bit;
            }
        };

        typedef Mask<uint8_t > Mask8;
        typedef Mask<uint16_t> Mask16;
        typedef Mask<uint32_t> Mask32;
        typedef Mask<uint64_t> Mask64;


        // if _sizebits is 0, returns all bits after _offset, otherwise uses mask
        template <typename T>
        T unpack(T _from, uint8_t _offset, uint8_t _sizebits) {
            _from >>= _offset;
            if (_sizebits) _from &= Mask<T>::FirstBits(_sizebits);
            return _from;
        }

        template <typename Y, typename T>
        Y pack(const std::initializer_list<Util::PackS<T>> &_BITS) {
            Y __res = 0u;
            for (const Util::PackS<T> &_s : _BITS)
                __res |= (_s.bits << _s.offset);
            return __res;
        }

        template <typename T>
        T nullify(T _val, uint8_t _offset, uint8_t _numbits) {
            return _val & (~Mask<T>::FirstBits(_numbits) << _offset);
        }
    };
};

#endif