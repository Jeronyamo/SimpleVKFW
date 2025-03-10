#ifndef SOGLFW_DEC64_H
#define SOGLFW_DEC64_H

#include <cmath>
#include <cstdint>
#include <type_traits>


namespace Simple {
    struct DEC64 {
        int64_t d = 0;

        DEC64() {}
        DEC64(const DEC64 &_val) : d{_val.d} {}
        DEC64(int64_t _coef, int _expo) {
            d = (_coef << EXPO_BITS) + (unsigned char)_expo;
        }

        template <typename T>
        DEC64(const typename std::enable_if<std::is_integral<T>::value, T>::type &_val) : d{int64_t(_val) << EXPO_BITS} {
            // TODO: check if _val has >=56 significant bits
        }

        template <typename T>
        DEC64(const typename std::enable_if<std::is_floating_point<T>::value, T>::type &_val, int bits) {
            if (bits < 0) bits += COEF_BITS;
            d = int64_t(_val * pow10(bits)) << EXPO_BITS;
            bits += std::floor(std::log10(std::abs(_val)));
            if (bits <= -(1 << EXPO_BITS - 1) ||
                bits >=  (1 << EXPO_BITS - 1))
                throw std::runtime_error("Error: DEC64 - number of bits exceeds the limits");
            d += (unsigned char) bits;
        }

        static const unsigned char EXPO_BITS = 8;
        static const unsigned char COEF_BITS = 8 * sizeof(int64_t) - EXPO_BITS;

        static int64_t pow10(int _val) {
            return std::pow(_val, 10);
        }

        // TODO: operations
    };

    const DEC64 Dec64NegInf{int64_t(1ul << DEC64::COEF_BITS - 1),     -128};
    const DEC64 Dec64PosInf{int64_t(1ul << DEC64::COEF_BITS - 1) - 1, -128};
    const DEC64 Dec64NaN{0, -128};
};

#endif