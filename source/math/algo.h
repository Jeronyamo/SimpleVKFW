#ifndef SVKFW_ALGO_H
#define SVKFW_ALGO_H

#include "common/basic_math.h"


namespace Simple {
    namespace Algo {
        template <typename T>
        uint32_t binarySearchLeft(const T &_val, const T *_arr, uint32_t _size) {
            if (_arr == nullptr || _size == 0u)
                throw std::invalid_argument(SVKFW_WRAPERR("Algo :: binarySearchLeft", "array of size 0"));

            uint32_t __L = 0u, __R = _size;
            if (!(_arr[0] < _val)) {
                __L = -int(_val < _arr[0]);
                __R = 0;
            }
            else if (_arr[_size - 1] < _val)
                __L = _size;

            while (__L < __R) {
                uint32_t __m = (__L >> 1) + (__R >> 1) + uint32_t((__L & 1) && (__R & 1));

                if (__arr[__m] < _val)
                    __L = __m + 1;
                else __R = __m;
            }

            return __L;
        }

        template <typename T>
        uint32_t binarySearchRight(const T &_val, const T *_arr, uint32_t _size) {
            if (_arr == nullptr || _size == 0u)
                throw std::invalid_argument(SVKFW_WRAPERR("Algo :: binarySearchRight", "array of size 0"));
            if (_val < _arr[0])
                return -1;
            if (!(_val < _arr[_size - 1]))
                return _size + int(_arr[_size - 1] < _val);

            uint32_t __L = 0u, __R = _size;
            while (__L < __R) {
                uint32_t __m = (__L >> 1) + (__R >> 1) + uint32_t((__L & 1) && (__R & 1));

                if (_val < __arr[__m])
                    __R = __m;
                else __L = __m + 1;
            }

            return __R - 1;
        }
    }; // Algo END
}; // Simple END

#endif