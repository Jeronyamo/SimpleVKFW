#ifndef SOGLFW_COLORS_H
#define SOGLFW_COLORS_H

#include "vectors.h"

#define COLORS_FTOU 255u
#define COLORS_UTOF 0.003921568859f     // 1.f / 255


namespace Simple {
    //  ============  Utilities  ============  \\

    inline float clamp(float val) { return val < 0.f ? 0.f : (val > 1.f ? 1.f : val); }
    inline int clamp(int val) { return val < 0 ? 0 : (val > 255 ? 255 : val); }
    inline unsigned clamp(unsigned val) { return val > 255u ? 255u : val; }

    template <typename T>
    struct ColMax {
        static const T max = std::is_floating_point<T>::value ? 1.f : 255u;
    };


    //  ============  Base types  ============  \\

    template <typename T, int ch>
    struct ColBase {
        using ColType = T;
    };

    template <typename T, int ch>
    struct ColDBG : ColBase<T, ch> {
        T arr[ch];

        ColDBG(const T *ptr) { std::memcpy(arr, ptr, sizeof(T) * ch); }
        ColDBG(const ColDBG<T, ch> &obj) { std::memcpy(arr, obj.arr, sizeof(T) * ch); }
        ColDBG(const std::initializer_list<T> &lst) { std::memcpy(arr, lst.begin(), sizeof(T) * ch); }
    };


    //  ============  3-3 channel operations  ============  \\

    template <typename T>
    ColDBG<T, 3> operator+(T op1, const ColBase<T, 3> &op2)
        { return {op1 + *((T*)&op2), op1 + *((T*)&op2 + 1), op1 + *((T*)&op2 + 2)}; }

    template <typename T>
    ColDBG<T, 3> operator-(T op1, const ColBase<T, 3> &op2)
        { return {op1 - *((T*)&op2), op1 - *((T*)&op2 + 1), op1 - *((T*)&op2 + 2)}; }

    template <typename T>
    ColDBG<T, 3> operator*(T op1, const ColBase<T, 3> &op2)
        { return {op1 * *((T*)&op2), op1 * *((T*)&op2 + 1), op1 * *((T*)&op2 + 2)}; }

    template <typename T>
    ColDBG<T, 3> operator/(T op1, const ColBase<T, 3> &op2) {
        return {notCloseToZero(*((T*)&op2    )) ? op1 / *((T*)&op2    ) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 1)) ? op1 / *((T*)&op2 + 1) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 2)) ? op1 / *((T*)&op2 + 2) : ColMax<T>::max};
    }

    template <typename T>
    ColDBG<T, 3> operator+(const ColBase<T, 3> &op1, T op2)
        { return {*((T*)&op1) + op2, *((T*)&op1 + 1) + op2, *((T*)&op1 + 2) + op2}; }

    template <typename T>
    ColDBG<T, 3> operator-(const ColBase<T, 3> &op1, T op2)
        { return {*((T*)&op1) - op2, *((T*)&op1 + 1) - op2, *((T*)&op1 + 2) - op2}; }

    template <typename T>
    ColDBG<T, 3> operator*(const ColBase<T, 3> &op1, T op2)
        { return {*((T*)&op1) * op2, *((T*)&op1 + 1) * op2, *((T*)&op1 + 2) * op2}; }

    template <typename T>
    ColDBG<T, 3> operator/(const ColBase<T, 3> &op1, T op2) {
        return notCloseToZero(op2) ?
                std::initializer_list<T>({*((T*)&op1) / op2, *((T*)&op1 + 1) / op2, *((T*)&op1 + 2) / op2}) :
                std::initializer_list<T>({ColMax<T>::max, ColMax<T>::max, ColMax<T>::max});
    }

    template <typename T>
    void operator+=(ColBase<T, 3> &op1, T op2)
        { op1 = {*((T*)&op1) + op2, *((T*)&op1 + 1) + op2, *((T*)&op1 + 2) + op2}; }

    template <typename T>
    void operator-=(ColBase<T, 3> &op1, T op2)
        { op1 = {*((T*)&op1) - op2, *((T*)&op1 + 1) - op2, *((T*)&op1 + 2) - op2}; }

    template <typename T>
    void operator*=(ColBase<T, 3> &op1, T op2)
        { op1 = {*((T*)&op1) * op2, *((T*)&op1 + 1) * op2, *((T*)&op1 + 2) * op2}; }

    template <typename T>
    void operator/=(ColBase<T, 3> &op1, T op2) {
        op1 = notCloseToZero(op2) ?
                std::initializer_list<T>({*((T*)&op1) / op2, *((T*)&op1 + 1) / op2, *((T*)&op1 + 2) / op2}) :
                std::initializer_list<T>({ColMax<T>::max, ColMax<T>::max, ColMax<T>::max});
    }


    template <typename T>
    ColDBG<T, 3> operator+(const ColBase<T, 3> &op1, const ColBase<T, 3> &op2)
        { return {*((T*)&op1) + *((T*)&op2), *((T*)&op1 + 1) + *((T*)&op2 + 1), *((T*)&op1 + 2) + *((T*)&op2 + 2)}; }

    template <typename T>
    ColDBG<T, 3> operator-(const ColBase<T, 3> &op1, const ColBase<T, 3> &op2)
        { return {*((T*)&op1) - *((T*)&op2), *((T*)&op1 + 1) - *((T*)&op2 + 1), *((T*)&op1 + 2) - *((T*)&op2 + 2)}; }

    template <typename T>
    ColDBG<T, 3> operator*(const ColBase<T, 3> &op1, const ColBase<T, 3> &op2)
        { return {*((T*)&op1) * *((T*)&op2), *((T*)&op1 + 1) * *((T*)&op2 + 1), *((T*)&op1 + 2) * *((T*)&op2 + 2)}; }

    template <typename T>
    ColDBG<T, 3> operator/(const ColBase<T, 3> &op1, const ColBase<T, 3> &op2) {
        return {notCloseToZero(*((T*)&op2    )) ? *((T*)&op1    ) / *((T*)&op2    ) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 1)) ? *((T*)&op1 + 1) / *((T*)&op2 + 1) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 2)) ? *((T*)&op1 + 2) / *((T*)&op2 + 2) : ColMax<T>::max};
    }

    template <typename T>
    void operator+=(ColBase<T, 3> &op1, const ColBase<T, 3> &op2)
        { op1 = {*((T*)&op1) + *((T*)&op2), *((T*)&op1 + 1) + *((T*)&op2 + 1), *((T*)&op1 + 2) + *((T*)&op2 + 2)}; }

    template <typename T>
    void operator-=(ColBase<T, 3> &op1, const ColBase<T, 3> &op2)
        { op1 = {*((T*)&op1) - *((T*)&op2), *((T*)&op1 + 1) - *((T*)&op2 + 1), *((T*)&op1 + 2) - *((T*)&op2 + 2)}; }

    template <typename T>
    void operator*=(ColBase<T, 3> &op1, const ColBase<T, 3> &op2)
        { op1 = {*((T*)&op1) * *((T*)&op2), *((T*)&op1 + 1) * *((T*)&op2 + 1), *((T*)&op1 + 2) * *((T*)&op2 + 2)}; }

    template <typename T>
    void operator/=(ColBase<T, 3> &op1, const ColBase<T, 3> &op2) {
        op1 = {notCloseToZero(*((T*)&op2    )) ? *((T*)&op1    ) / *((T*)&op2    ) : ColMax<T>::max,
               notCloseToZero(*((T*)&op2 + 1)) ? *((T*)&op1 + 1) / *((T*)&op2 + 1) : ColMax<T>::max,
               notCloseToZero(*((T*)&op2 + 2)) ? *((T*)&op1 + 2) / *((T*)&op2 + 2) : ColMax<T>::max};
    }


    template <typename T>
    ColDBG<T, 3> MIN(const ColBase<T, 3> &op1, const ColBase<T, 3> &op2)
        { return {*((T*)&op1    ) < *((T*)&op2    ) ? *((T*)&op1    ) : *((T*)&op2    ),
                  *((T*)&op1 + 1) < *((T*)&op2 + 1) ? *((T*)&op1 + 1) : *((T*)&op2 + 1),
                  *((T*)&op1 + 2) < *((T*)&op2 + 2) ? *((T*)&op1 + 2) : *((T*)&op2 + 2)}; }

    template <typename T>
    ColDBG<T, 3> MAX(const ColBase<T, 3> &op1, const ColBase<T, 3> &op2)
        { return {*((T*)&op1    ) > *((T*)&op2    ) ? *((T*)&op1    ) : *((T*)&op2    ),
                  *((T*)&op1 + 1) > *((T*)&op2 + 1) ? *((T*)&op1 + 1) : *((T*)&op2 + 1),
                  *((T*)&op1 + 2) > *((T*)&op2 + 2) ? *((T*)&op1 + 2) : *((T*)&op2 + 2)}; }

    template <typename T>
    ColDBG<T, 3> DIF(const ColBase<T, 3> &op1, const ColBase<T, 3> &op2)
        { return {*((T*)&op1    ) > *((T*)&op2    ) ? *((T*)&op1    ) - *((T*)&op2    ) : *((T*)&op2    ) - *((T*)&op1    ),
                  *((T*)&op1 + 1) > *((T*)&op2 + 1) ? *((T*)&op1 + 1) - *((T*)&op2 + 1) : *((T*)&op2 + 1) - *((T*)&op1 + 1),
                  *((T*)&op1 + 2) > *((T*)&op2 + 2) ? *((T*)&op1 + 2) - *((T*)&op2 + 2) : *((T*)&op2 + 2) - *((T*)&op1 + 2)}; }

    template <typename T>
    ColDBG<T, 3> AVG(const ColBase<T, 3> &op1, const ColBase<T, 3> &op2)
        { return {(*((T*)&op1) + *((T*)&op2)) / 2, (*((T*)&op1 + 1) + *((T*)&op2 + 1)) / 2, (*((T*)&op1 + 2) + *((T*)&op2 + 2)) / 2}; }


    //  ============  4-4 channel operations  ============  \\

    template <typename T>
    ColDBG<T, 4> operator+(T op1, const ColBase<T, 4> &op2)
        { return {op1 + *((T*)&op2), op1 + *((T*)&op2 + 1), op1 + *((T*)&op2 + 2), op1 + *((T*)&op2 + 3)}; }

    template <typename T>
    ColDBG<T, 4> operator-(T op1, const ColBase<T, 4> &op2)
        { return {op1 - *((T*)&op2), op1 - *((T*)&op2 + 1), op1 - *((T*)&op2 + 2), op1 - *((T*)&op2 + 3)}; }

    template <typename T>
    ColDBG<T, 4> operator*(T op1, const ColBase<T, 4> &op2)
        { return {op1 * *((T*)&op2), op1 * *((T*)&op2 + 1), op1 * *((T*)&op2 + 2), op1 * *((T*)&op2 + 3)}; }

    template <typename T>
    ColDBG<T, 4> operator/(T op1, const ColBase<T, 4> &op2) {
        return {notCloseToZero(*((T*)&op2    )) ? op1 / *((T*)&op2    ) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 1)) ? op1 / *((T*)&op2 + 1) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 2)) ? op1 / *((T*)&op2 + 2) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 3)) ? op1 / *((T*)&op2 + 3) : ColMax<T>::max};
    }

    template <typename T>
    ColDBG<T, 4> operator+(const ColBase<T, 4> &op1, T op2)
        { return {*((T*)&op1) + op2, *((T*)&op1 + 1) + op2, *((T*)&op1 + 2) + op2, *((T*)&op1 + 3) + op2}; }

    template <typename T>
    ColDBG<T, 4> operator-(const ColBase<T, 4> &op1, T op2)
        { return {*((T*)&op1) - op2, *((T*)&op1 + 1) - op2, *((T*)&op1 + 2) - op2, *((T*)&op1 + 3) - op2}; }

    template <typename T>
    ColDBG<T, 4> operator*(const ColBase<T, 4> &op1, T op2)
        { return {*((T*)&op1) * op2, *((T*)&op1 + 1) * op2, *((T*)&op1 + 2) * op2, *((T*)&op1 + 3) * op2}; }

    template <typename T>
    ColDBG<T, 4> operator/(const ColBase<T, 4> &op1, T op2) {
        return notCloseToZero(op2) ?
                std::initializer_list<T>{*((T*)&op1) / op2, *((T*)&op1 + 1) / op2, *((T*)&op1 + 2) / op2, *((T*)&op1 + 3) / op2} :
                std::initializer_list<T>{ColMax<T>::max, ColMax<T>::max, ColMax<T>::max, ColMax<T>::max};
    }

    template <typename T>
    void operator+=(ColBase<T, 4> &op1, T op2)
        { op1 = {*((T*)&op1) + op2, *((T*)&op1 + 1) + op2, *((T*)&op1 + 2) + op2, *((T*)&op1 + 3) + op2}; }

    template <typename T>
    void operator-=(ColBase<T, 4> &op1, T op2)
        { op1 = {*((T*)&op1) - op2, *((T*)&op1 + 1) - op2, *((T*)&op1 + 2) - op2, *((T*)&op1 + 3) - op2}; }

    template <typename T>
    void operator*=(ColBase<T, 4> &op1, T op2)
        { op1 = {*((T*)&op1) * op2, *((T*)&op1 + 1) * op2, *((T*)&op1 + 2) * op2, *((T*)&op1 + 3) * op2}; }

    template <typename T>
    void operator/=(ColBase<T, 4> &op1, T op2) {
        op1 = notCloseToZero(op2) ?
                std::initializer_list<T>{*((T*)&op1) / op2, *((T*)&op1 + 1) / op2, *((T*)&op1 + 2) / op2, *((T*)&op1 + 3) / op2} :
                std::initializer_list<T>{ColMax<T>::max, ColMax<T>::max, ColMax<T>::max, ColMax<T>::max};
    }


    template <typename T>
    ColDBG<T, 4> operator+(const ColBase<T, 4> &op1, const ColBase<T, 4> &op2) {
        return {*((T*)&op1    ) + *((T*)&op2    ), *((T*)&op1 + 1) + *((T*)&op2 + 1),
                *((T*)&op1 + 2) + *((T*)&op2 + 2), *((T*)&op1 + 3) + *((T*)&op2 + 3)};
    }

    template <typename T>
    ColDBG<T, 4> operator-(const ColBase<T, 4> &op1, const ColBase<T, 4> &op2) {
        return {*((T*)&op1    ) - *((T*)&op2    ), *((T*)&op1 + 1) - *((T*)&op2 + 1),
                *((T*)&op1 + 2) - *((T*)&op2 + 2), *((T*)&op1 + 3) - *((T*)&op2 + 3)};
    }

    template <typename T>
    ColDBG<T, 4> operator*(const ColBase<T, 4> &op1, const ColBase<T, 4> &op2) {
        return {*((T*)&op1    ) * *((T*)&op2    ), *((T*)&op1 + 1) * *((T*)&op2 + 1),
                *((T*)&op1 + 2) * *((T*)&op2 + 2), *((T*)&op1 + 3) * *((T*)&op2 + 3)};
    }

    template <typename T>
    ColDBG<T, 4> operator/(const ColBase<T, 4> &op1, const ColBase<T, 4> &op2) {
        return {notCloseToZero(*((T*)&op2    )) ? *((T*)&op1    ) / *((T*)&op2    ) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 1)) ? *((T*)&op1 + 1) / *((T*)&op2 + 1) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 2)) ? *((T*)&op1 + 2) / *((T*)&op2 + 2) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 3)) ? *((T*)&op1 + 3) / *((T*)&op2 + 3) : ColMax<T>::max};
    }

    template <typename T>
    void operator+=(ColBase<T, 4> &op1, const ColBase<T, 4> &op2) {
        op1 = {*((T*)&op1    ) + *((T*)&op2    ), *((T*)&op1 + 1) + *((T*)&op2 + 1),
               *((T*)&op1 + 2) + *((T*)&op2 + 2), *((T*)&op1 + 3) + *((T*)&op2 + 3)};
    }

    template <typename T>
    void operator-=(ColBase<T, 4> &op1, const ColBase<T, 4> &op2) {
        op1 = {*((T*)&op1    ) - *((T*)&op2    ), *((T*)&op1 + 1) - *((T*)&op2 + 1),
               *((T*)&op1 + 2) - *((T*)&op2 + 2), *((T*)&op1 + 3) - *((T*)&op2 + 3)};
    }

    template <typename T>
    void operator*=(ColBase<T, 4> &op1, const ColBase<T, 4> &op2) {
        op1 = {*((T*)&op1    ) * *((T*)&op2    ), *((T*)&op1 + 1) * *((T*)&op2 + 1),
               *((T*)&op1 + 2) * *((T*)&op2 + 2), *((T*)&op1 + 3) * *((T*)&op2 + 3)};
    }

    template <typename T>
    void operator/=(ColBase<T, 4> &op1, const ColBase<T, 4> &op2) {
        op1 = {notCloseToZero(*((T*)&op2    )) ? *((T*)&op1    ) / *((T*)&op2    ) : ColMax<T>::max,
               notCloseToZero(*((T*)&op2 + 1)) ? *((T*)&op1 + 1) / *((T*)&op2 + 1) : ColMax<T>::max,
               notCloseToZero(*((T*)&op2 + 2)) ? *((T*)&op1 + 2) / *((T*)&op2 + 2) : ColMax<T>::max,
               notCloseToZero(*((T*)&op2 + 3)) ? *((T*)&op1 + 3) / *((T*)&op2 + 3) : ColMax<T>::max};
    }



    template <typename T>
    ColDBG<T, 4> MIN(const ColBase<T, 4> &op1, const ColBase<T, 4> &op2)
        { return {*((T*)&op1    ) < *((T*)&op2    ) ? *((T*)&op1    ) : *((T*)&op2    ),
                  *((T*)&op1 + 1) < *((T*)&op2 + 1) ? *((T*)&op1 + 1) : *((T*)&op2 + 1),
                  *((T*)&op1 + 2) < *((T*)&op2 + 2) ? *((T*)&op1 + 2) : *((T*)&op2 + 2),
                  *((T*)&op1 + 3) < *((T*)&op2 + 3) ? *((T*)&op1 + 3) : *((T*)&op2 + 3)}; }

    template <typename T>
    ColDBG<T, 4> MAX(const ColBase<T, 4> &op1, const ColBase<T, 4> &op2)
        { return {*((T*)&op1    ) > *((T*)&op2    ) ? *((T*)&op1    ) : *((T*)&op2    ),
                  *((T*)&op1 + 1) > *((T*)&op2 + 1) ? *((T*)&op1 + 1) : *((T*)&op2 + 1),
                  *((T*)&op1 + 2) > *((T*)&op2 + 2) ? *((T*)&op1 + 2) : *((T*)&op2 + 2),
                  *((T*)&op1 + 3) > *((T*)&op2 + 3) ? *((T*)&op1 + 3) : *((T*)&op2 + 3)}; }

    template <typename T>
    ColDBG<T, 4> DIF(const ColBase<T, 4> &op1, const ColBase<T, 4> &op2)
        { return {*((T*)&op1    ) > *((T*)&op2    ) ? *((T*)&op1    ) - *((T*)&op2    ) : *((T*)&op2    ) - *((T*)&op1    ),
                  *((T*)&op1 + 1) > *((T*)&op2 + 1) ? *((T*)&op1 + 1) - *((T*)&op2 + 1) : *((T*)&op2 + 1) - *((T*)&op1 + 1),
                  *((T*)&op1 + 2) > *((T*)&op2 + 2) ? *((T*)&op1 + 2) - *((T*)&op2 + 2) : *((T*)&op2 + 2) - *((T*)&op1 + 2),
                  *((T*)&op1 + 3) > *((T*)&op2 + 3) ? *((T*)&op1 + 3) - *((T*)&op2 + 3) : *((T*)&op2 + 3) - *((T*)&op1 + 3)}; }

    template <typename T>
    ColDBG<T, 4> AVG(const ColBase<T, 4> &op1, const ColBase<T, 4> &op2)
        { return {(*((T*)&op1    ) + *((T*)&op2    )) / 2, (*((T*)&op1 + 1) + *((T*)&op2 + 1)) / 2,
                  (*((T*)&op1 + 2) + *((T*)&op2 + 2)) / 2, (*((T*)&op1 + 3) + *((T*)&op2 + 3)) / 2}; }



    
    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendOver(const ColBase<T, 4> &c1, const ColBase<T, 4> &c2)
        { return {*((T*)&c1    ) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2    ), *((T*)&c1 + 1) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 1),
                  *((T*)&c1 + 2) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 2), *((T*)&c1 + 3) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 3)}; }

    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendIn  (const ColBase<T, 4> &c1, const ColBase<T, 4> &c2)
        { return {*((T*)&c2 + 3) * *((T*)&c1    ), *((T*)&c2 + 3) * *((T*)&c1 + 1),
                  *((T*)&c2 + 3) * *((T*)&c1 + 2), *((T*)&c2 + 3) * *((T*)&c1 + 3)}; }

    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendOut (const ColBase<T, 4> &c1, const ColBase<T, 4> &c2)
        { return {(1.f - *((T*)&c2 + 3)) * *((T*)&c1    ), (1.f - *((T*)&c2 + 3)) * *((T*)&c1 + 1),
                  (1.f - *((T*)&c2 + 3)) * *((T*)&c1 + 2), (1.f - *((T*)&c2 + 3)) * *((T*)&c1 + 3)}; }

    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendAtop(const ColBase<T, 4> &c1, const ColBase<T, 4> &c2)
        { return {*((T*)&c2 + 3) * *((T*)&c1    ) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2    ),
                  *((T*)&c2 + 3) * *((T*)&c1 + 1) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 1),
                  *((T*)&c2 + 3) * *((T*)&c1 + 2) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 2), *((T*)&c2 + 3)}; }

    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendXor (const ColBase<T, 4> &c1, const ColBase<T, 4> &c2)
        { return {(1.f - *((T*)&c2 + 3)) * *((T*)&c1    ) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2    ),
                  (1.f - *((T*)&c2 + 3)) * *((T*)&c1 + 1) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 1),
                  (1.f - *((T*)&c2 + 3)) * *((T*)&c1 + 2) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 2),
                  (1.f - *((T*)&c2 + 3)) * *((T*)&c1 + 3) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 3)}; }


    //  ============  Mixed channel operations  ============  \\

/*
    template <typename T, int ch1, int ch2>
    ColDBG<T, ((ch1 < ch2) ? ch2 : ch1)> operator+(const ColBase<T, ch1> &op1, const ColBase<T, ch2> &op2) {
        T tarr[(ch1 < ch2) ? ch2 : ch1]{0, 0, 0, ColMax<T>::max};
        for (unsigned i = 0u; i < ((ch1 > ch2) ? ch2 : ch1); ++i)
            tarr[i] = clamp(*((T*)&op1 + i) + *((T*)&op2 + i));
        return tarr;
    }

    template <typename T, int ch1, int ch2>
    ColDBG<T, ((ch1 < ch2) ? ch2 : ch1)> operator-(const ColBase<T, ch1> &op1, const ColBase<T, ch2> &op2) {
        T tarr[(ch1 < ch2) ? ch2 : ch1]{0, 0, 0, ColMax<T>::max};
        for (unsigned i = 0u; i < ((ch1 > ch2) ? ch2 : ch1); ++i)
            tarr[i] = clamp(*((T*)&op1 + i) - *((T*)&op2 + i));
        return tarr;
    }

    template <typename T, int ch1, int ch2>
    ColDBG<T, ((ch1 < ch2) ? ch2 : ch1)> operator*(const ColBase<T, ch1> &op1, const ColBase<T, ch2> &op2) {
        T tarr[(ch1 < ch2) ? ch2 : ch1]{0, 0, 0, ColMax<T>::max};
        for (unsigned i = 0u; i < ((ch1 > ch2) ? ch2 : ch1); ++i)
            tarr[i] = clamp(*((T*)&op1 + i) * *((T*)&op2 + i));
        return tarr;
    }

    template <typename T, int ch1, int ch2>
    ColDBG<T, ((ch1 < ch2) ? ch2 : ch1)> operator/(const ColBase<T, ch1> &op1, const ColBase<T, ch2> &op2) {
        T tarr[(ch1 < ch2) ? ch2 : ch1]{0, 0, 0, ColMax<T>::max};
        for (unsigned i = 0u; i < ((ch1 > ch2) ? ch2 : ch1); ++i)
            tarr[i] = notCloseToZero(*((T*)&op2 + i)) ? clamp(*((T*)&op1 + i) / *((T*)&op2 + i)) : ColMax<T>::max;
        return tarr;
    }
*/


    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendOver(const ColBase<T, 4> &c1, const ColBase<T, 3> &c2)
        { return {*((T*)&c1    ) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2    ),
                  *((T*)&c1 + 1) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 1),
                  *((T*)&c1 + 2) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 2), 1.f}; }

    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendIn  (const ColBase<T, 4> &c1, const ColBase<T, 3> &c2)
        { return c1; }

    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendOut (const ColBase<T, 4> &c1, const ColBase<T, 3> &c2)
        { return {0.f, 0.f, 0.f, 0.f}; }

    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendAtop(const ColBase<T, 4> &c1, const ColBase<T, 3> &c2)
        { return {*((T*)&c1    ) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2    ),
                  *((T*)&c1 + 1) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 1),
                  *((T*)&c1 + 2) + (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 2), 1.f}; }

    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendXor (const ColBase<T, 4> &c1, const ColBase<T, 3> &c2)
        { return {(1.f - *((T*)&c1 + 3)) * *((T*)&c2    ), (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 1),
                  (1.f - *((T*)&c1 + 3)) * *((T*)&c2 + 2),  1.f - *((T*)&c1 + 3)}; }


    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendOver(const ColBase<T, 3> &c1, const ColBase<T, 4> &c2)
        { return c1; }

    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendIn  (const ColBase<T, 3> &c1, const ColBase<T, 4> &c2)
        { return {*((T*)&c2 + 3) * *((T*)&c1), *((T*)&c2 + 3) * *((T*)&c1 + 1), *((T*)&c2 + 3) * *((T*)&c1 + 2), *((T*)&c2 + 3)}; }

    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendOut (const ColBase<T, 3> &c1, const ColBase<T, 4> &c2)
        { return {(1.f - *((T*)&c2 + 3)) * *((T*)&c1    ), (1.f - *((T*)&c2 + 3)) * *((T*)&c1 + 1),
                  (1.f - *((T*)&c2 + 3)) * *((T*)&c1 + 2),  1.f - *((T*)&c2 + 3)}; }

    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendAtop(const ColBase<T, 3> &c1, const ColBase<T, 4> &c2)
        { return {*((T*)&c2 + 3) * *((T*)&c1), *((T*)&c2 + 3) * *((T*)&c1 + 1), *((T*)&c2 + 3) * *((T*)&c1 + 2), *((T*)&c2 + 3)}; }

    template <typename T>
    inline ColDBG<T, 4> PorterDuffBlendXor (const ColBase<T, 3> &c1, const ColBase<T, 4> &c2)
        { return {(1.f - *((T*)&c2 + 3)) * *((T*)&c1    ), (1.f - *((T*)&c2 + 3)) * *((T*)&c1 + 1),
                  (1.f - *((T*)&c2 + 3)) * *((T*)&c1 + 2),  1.f - *((T*)&c2 + 3)}; }


    //  ============  Color models  ============  \\

    struct RGB;
    struct RGB256;
    struct RGBA;
    struct RGBA256;
    struct WideRGB;
    struct WideRGBA;
    struct HSV;
    struct HSVA;
    struct LAB;
    struct LABA;
    struct LMS;
    struct LMSA;
    struct ITP;
    struct ITPA;
    struct XYZ;
    struct XYZA;

    template <class T>
    struct ColChannels {
        static const unsigned channels = (std::is_same<T, float>::value || std::is_same<T, unsigned char>::value)
                                + 3u * (
                                        std::is_same<T, RGB>::value || std::is_same<T, RGB256>::value ||
                                        std::is_same<T, HSV>::value || std::is_same<T, WideRGB>::value || 
                                        std::is_same<T, LAB>::value || std::is_same<T, LMS>::value ||
                                        std::is_same<T, ITP>::value || std::is_same<T, XYZ>::value)
                                + 4u * (
                                        std::is_same<T, RGBA>::value || std::is_same<T, RGBA256>::value ||
                                        std::is_same<T, HSVA>::value || std::is_same<T, WideRGBA>::value || 
                                        std::is_same<T, LABA>::value || std::is_same<T, LMSA>::value ||
                                        std::is_same<T, ITPA>::value || std::is_same<T, XYZA>::value);
    };

    struct RGB : ColBase<float, 3> {
        ColType r = 0.f, g = 0.f, b = 0.f;

        RGB() {}
        template <typename T> RGB(T v1) : r{float(v1)}, g{float(v1)}, b{float(v1)} {}
        template <typename T1, typename T2, typename T3> RGB(T1 v1, T2 v2, T3 v3) : r{float(v1)}, g{float(v2)}, b{float(v3)} {}
        RGB(const ColDBG<float, 3> &col) : r{clamp(col.arr[0])}, g{clamp(col.arr[1])}, b{clamp(col.arr[2])} {}
        RGB(const Vec3 &vec) : r{clamp(vec.x)}, g{clamp(vec.y)}, b{clamp(vec.z)} {}
        RGB(const RGB  &col) : r{col.r}, g{col.g}, b{col.b} {}

        RGB(const RGBA &col);
        RGB(const RGB256 &col);
        RGB(const RGBA256 &col);
    };

    struct RGB256 : ColBase<unsigned char, 3> {
        ColType r = 0u, g = 0u, b = 0u;

        RGB256() {}
        template <typename T> RGB256(T v1) : r{(unsigned char)(v1)}, g{(unsigned char)(v1)}, b{(unsigned char)(v1)} {}
        template <typename T1, typename T2, typename T3> RGB256(T1 v1, T2 v2, T3 v3) :
                                             r{(unsigned char)(v1)}, g{(unsigned char)(v2)}, b{(unsigned char)(v3)} {}
        RGB256(const ColDBG<unsigned char, 3> &col) : r{col.arr[0]}, g{col.arr[1]}, b{col.arr[2]} {}
        //RGB256(const UVec3  &vec) : r{vec.x}, g{vec.y}, b{vec.z} {}
        RGB256(const RGB256 &col) : r{col.r}, g{col.g}, b{col.b} {}

        RGB256(const RGB &col);
        RGB256(const RGBA &col);
        RGB256(const RGBA256 &col);
    };

    struct RGBA : ColBase<float, 4> {
        ColType r = 0.f, g = 0.f, b = 0.f, a = 0.f;

        RGBA() {}
        template <typename T1> RGBA(T1 v1) : r{float(v1)}, g{float(v1)}, b{float(v1)} {}
        template <typename T1, typename T2, typename T3, typename T4> RGBA(T1 v1, T2 v2, T3 v3, T4 v4) : r{float(v1)}, g{float(v2)}, b{float(v3)}, a{float(v4)} {}
        RGBA(const ColDBG<float, 4> &col) : r{clamp(col.arr[0])}, g{clamp(col.arr[1])}, b{clamp(col.arr[2])}, a{clamp(col.arr[3])} {}
        RGBA(const Vec4 &vec) : r{vec.x}, g{vec.y}, b{vec.z}, a{vec.w} {}
        RGBA(const RGBA &col) : r{col.r}, g{col.g}, b{col.b}, a{col.a} {}

        RGBA(const RGB &col);
        RGBA(const RGB256 &col);
        RGBA(const RGBA256 &col);
    };

    struct RGBA256 : ColBase<unsigned char, 4> {
        ColType r = 0u, g = 0u, b = 0u, a = 0u;

        RGBA256() {}
        template <typename T1> RGBA256(T1 v1) : r{(unsigned char)(v1)}, g{(unsigned char)(v1)}, b{(unsigned char)(v1)}, a{(unsigned char)(v1)} {}
        template <typename T1, typename T2, typename T3, typename T4> RGBA256(T1 v1, T2 v2, T3 v3, T4 v4) :
                                                r{(unsigned char)(v1)}, g{(unsigned char)(v2)}, b{(unsigned char)(v3)}, a{(unsigned char)(v4)} {}
        RGBA256(const ColDBG<unsigned char, 4> &col) : r{col.arr[0]}, g{col.arr[1]}, b{col.arr[2]}, a{col.arr[3]} {}
        //RGBA256(const UVec4   &vec) : r{vec.x}, g{vec.y}, b{vec.z}, a{vec.w} {}
        RGBA256(const RGBA256 &col) : r{col.r}, g{col.g}, b{col.b}, a{col.a} {}

        RGBA256(const RGB &col);
        RGBA256(const RGB256 &col);
        RGBA256(const RGBA &col);
    };


    struct WideRGB : ColBase<float, 3> {
        ColType r = 0.f, g = 0.f, b = 0.f;

        WideRGB() {}
        template <typename T> WideRGB(T v1) : r{float(v1)}, g{float(v1)}, b{float(v1)} {}
        template <typename T1, typename T2, typename T3> WideRGB(T1 v1, T2 v2, T3 v3) : r{float(v1)}, g{float(v2)}, b{float(v3)} {}
        WideRGB(const ColDBG<float, 3> &col) : r{clamp(col.arr[0])}, g{clamp(col.arr[1])}, b{clamp(col.arr[2])} {}
        WideRGB(const Vec3    &vec) : r{vec.x}, g{vec.y}, b{vec.z} {}
        WideRGB(const WideRGB &col) : r{col.r}, g{col.g}, b{col.b} {}
    };

    struct WideRGBA : ColBase<float, 4> {
        ColType r = 0.f, g = 0.f, b = 0.f, a = 0.f;

        WideRGBA() {}
        template <typename T1> WideRGBA(T1 v1) : r{float(v1)}, g{float(v1)}, b{float(v1)} {}
        template <typename T1, typename T2, typename T3, typename T4> WideRGBA(T1 v1, T2 v2, T3 v3, T4 v4) : r{float(v1)}, g{float(v2)}, b{float(v3)}, a{float(v4)} {}
        WideRGBA(const ColDBG<float, 4> &col) : r{clamp(col.arr[0])}, g{clamp(col.arr[1])}, b{clamp(col.arr[2])}, a{clamp(col.arr[3])} {}
        WideRGBA(const Vec4     &vec) : r{vec.x}, g{vec.y}, b{vec.z}, a{vec.w} {}
        WideRGBA(const WideRGBA &col) : r{col.r}, g{col.g}, b{col.b}, a{col.a} {}
    };


    struct HSV : ColBase<float, 3> {
        ColType h = 0.f, s = 0.f, v = 0.f;

        HSV() {}
        template <typename T1, typename T2, typename T3> HSV(T1 v1, T2 v2, T3 v3) : h{float(v1)}, s{float(v2)}, v{float(v3)} {}
        HSV(const ColDBG<float, 3> &col) : h{clamp(col.arr[0])}, s{clamp(col.arr[1])}, v{clamp(col.arr[2])} {}
        HSV(const Vec3 &vec) : h{vec.x}, s{vec.y}, v{vec.z} {}
        HSV(const HSV  &col) : h{col.h}, s{col.s}, v{col.v} {}
    };

    struct HSVA : ColBase<float, 4> {
        ColType h = 0.f, s = 0.f, v = 0.f, a = 0.f;

        HSVA() {}
        template <typename T1, typename T2, typename T3, typename T4> HSVA(T1 v1, T2 v2, T3 v3, T4 v4) : h{float(v1)}, s{float(v2)}, v{float(v3)}, a{float(v4)} {}
        HSVA(const ColDBG<float, 4> &col) : h{clamp(col.arr[0])}, s{clamp(col.arr[1])}, v{clamp(col.arr[2])}, a{clamp(col.arr[3])} {}
        HSVA(const Vec4 &vec) : h{vec.x}, s{vec.y}, v{vec.z}, a{vec.w} {}
        HSVA(const HSVA &col) : h{col.h}, s{col.s}, v{col.v}, a{col.a} {}
    };


    struct LAB : ColBase<float, 3> {
        ColType l = 0.f, a = 0.f, b = 0.f;

        LAB() {}
        template <typename T1, typename T2, typename T3> LAB(T1 v1, T2 v2, T3 v3) : l{float(v1)}, a{float(v2)}, b{float(v3)} {}
        LAB(const ColDBG<float, 3> &col) : l{clamp(col.arr[0])}, a{clamp(col.arr[1])}, b{clamp(col.arr[2])} {}
        LAB(const Vec3 &vec) : l{vec.x}, a{vec.y}, b{vec.z} {}
        LAB(const LAB  &col) : l{col.l}, a{col.a}, b{col.b} {}
    };

    struct LABA : ColBase<float, 4> {
        ColType l = 0.f, a = 0.f, b = 0.f, al = 0.f;

        LABA() {}
        template <typename T1, typename T2, typename T3, typename T4> LABA(T1 v1, T2 v2, T3 v3, T4 v4) : l{float(v1)}, a{float(v2)}, b{float(v3)}, al{float(v4)} {}
        LABA(const ColDBG<float, 4> &col) : l{clamp(col.arr[0])}, a{clamp(col.arr[1])}, b{clamp(col.arr[2])}, al{clamp(col.arr[3])} {}
        LABA(const Vec4 &vec) : l{vec.x}, a{vec.y}, b{vec.z}, al{vec.w } {}
        LABA(const LABA &col) : l{col.l}, a{col.a}, b{col.b}, al{col.al} {}
    };


    struct LMS : ColBase<float, 3> {
        ColType l = 0.f, m = 0.f, s = 0.f;

        LMS() {}
        template <typename T1, typename T2, typename T3> LMS(T1 v1, T2 v2, T3 v3) : l{float(v1)}, m{float(v2)}, s{float(v3)} {}
        LMS(const ColDBG<float, 3> &col) : l{clamp(col.arr[0])}, m{clamp(col.arr[1])}, s{clamp(col.arr[2])} {}
        LMS(const Vec3 &vec) : l{vec.x}, m{vec.y}, s{vec.z} {}
        LMS(const LMS  &col) : l{col.l}, m{col.m}, s{col.s} {}
    };

    struct LMSA : ColBase<float, 4> {
        ColType l = 0.f, m = 0.f, s = 0.f, a = 0.f;

        LMSA() {}
        template <typename T1, typename T2, typename T3, typename T4> LMSA(T1 v1, T2 v2, T3 v3, T4 v4) : l{float(v1)}, m{float(v2)}, s{float(v3)}, a{float(v4)} {}
        LMSA(const ColDBG<float, 4> &col) : l{clamp(col.arr[0])}, m{clamp(col.arr[1])}, s{clamp(col.arr[2])}, a{clamp(col.arr[3])} {}
        LMSA(const Vec4 &vec) : l{vec.x}, m{vec.y}, s{vec.z}, a{vec.w} {}
        LMSA(const LMSA &col) : l{col.l}, m{col.m}, s{col.s}, a{col.a} {}
    };


    struct ITP : ColBase<float, 3> {
        ColType i = 0.f, t = 0.f, p = 0.f;

        ITP() {}
        template <typename T1, typename T2, typename T3> ITP(T1 v1, T2 v2, T3 v3) : i{float(v1)}, t{float(v2)}, p{float(v3)} {}
        ITP(const ColDBG<float, 3> &col) : i{clamp(col.arr[0])}, t{clamp(col.arr[1])}, p{clamp(col.arr[2])} {}
        ITP(const Vec3 &vec) : i{vec.x}, t{vec.y}, p{vec.z} {}
        ITP(const ITP  &col) : i{col.i}, t{col.t}, p{col.p} {}
    };

    struct ITPA : ColBase<float, 4> {
        ColType i = 0.f, t = 0.f, p = 0.f, a = 0.f;

        ITPA() {}
        template <typename T1, typename T2, typename T3, typename T4> ITPA(T1 v1, T2 v2, T3 v3, T4 v4) : i{float(v1)}, t{float(v2)}, p{float(v3)}, a{float(v4)} {}
        ITPA(const ColDBG<float, 4> &col) : i{clamp(col.arr[0])}, t{clamp(col.arr[1])}, p{clamp(col.arr[2])}, a{clamp(col.arr[3])} {}
        ITPA(const Vec4 &vec) : i{vec.x}, t{vec.y}, p{vec.z}, a{vec.w} {}
        ITPA(const ITPA &col) : i{col.i}, t{col.t}, p{col.p}, a{col.a} {}
    };


    struct XYZ : ColBase<float, 3> {
        ColType x = 0.f, y = 0.f, z = 0.f;

        XYZ() {}
        template <typename T1, typename T2, typename T3> XYZ(T1 v1, T2 v2, T3 v3) : x{float(v1)}, y{float(v2)}, z{float(v3)} {}
        XYZ(const ColDBG<float, 3> &col) : x{clamp(col.arr[0])}, y{clamp(col.arr[1])}, z{clamp(col.arr[2])} {}
        XYZ(const Vec3 &vec) : x{vec.x}, y{vec.y}, z{vec.z} {}
        XYZ(const XYZ  &col) : x{col.x}, y{col.y}, z{col.z} {}
    };

    struct XYZA : ColBase<float, 4> {
        ColType x = 0.f, y = 0.f, z = 0.f, a = 0.f;

        XYZA() {}
        template <typename T1, typename T2, typename T3, typename T4> XYZA(T1 v1, T2 v2, T3 v3, T4 v4) : x{float(v1)}, y{float(v2)}, z{float(v3)}, a{float(v4)} {}
        XYZA(const ColDBG<float, 4> &col) : x{clamp(col.arr[0])}, y{clamp(col.arr[1])}, z{clamp(col.arr[2])}, a{clamp(col.arr[3])} {}
        XYZA(const Vec4 &vec) : x{vec.x}, y{vec.y}, z{vec.z}, a{vec.w} {}
        XYZA(const XYZA &col) : x{col.x}, y{col.y}, z{col.z}, a{col.a} {}
    };


    //  ============  Reference white  ============  \\

    const XYZ D50 = { 0.96422f, 1.f, 0.82521f };
    const XYZ D65 = { 0.95047f, 1.f, 1.08883f };
    const XYZ D502D65 = { 0.9857398f, 1.f, 1.3194581f };
    const XYZ D652D50 = { 1.0144665f, 1.f, 0.7578869f };


    //  ============  Color model transforms (constructors)  ============  \\

    //E
    const Mat3 RGB2XYZ_E { 0.4887180f, 0.3106803f, 0.2006017f,
                           0.1762044f, 0.8129847f, 0.0108109f,
                           0.0f,       0.0102048f, 0.9897952f };
    //E
    const Mat3 XYZ2RGB_E { 2.3706743f,-0.9000405f,-0.4706338f,
                          -0.5138850f, 1.4253036f, 0.0885814f,
                           0.0052982f,-0.0146949f, 1.0093968f };

    //D50
    const Mat3 RGB2562XYZ_E = RGB2XYZ_E * COLORS_UTOF;

    //D50
    const Mat3 XYZ2RGB256_E = XYZ2RGB_E * COLORS_FTOU;

    //D50
    const Mat3 RGB2XYZ { 0.4868870f, 0.3062984f, 0.1710347f,
                         0.1746583f, 0.8247541f, 0.0005877f,
                        -0.0012563f, 0.0169832f, 0.8094831f };
    //D50
    const Mat3 XYZ2RGB { 2.3638081f,-0.8676030f,-0.4988161f,
                        -0.5005940f, 1.3962369f, 0.1047562f,
                         0.0141712f,-0.0306400f, 1.2323842f };

    //D50
    const Mat3 RGB2562XYZ = RGB2XYZ * COLORS_UTOF;

    //D50
    const Mat3 XYZ2RGB256 = XYZ2RGB * COLORS_FTOU;

    //D50
    const Mat3 WideRGB2XYZ { 0.7161046f, 0.1009296f, 0.1471858f,
                             0.2581874f, 0.7249378f, 0.0168748f,
                             0.0f,       0.0517813f, 0.7734287f };
    //D50
    const Mat3 XYZ2WideRGB { 1.4628067f,-0.1840623f,-0.2743606f,
                            -0.5217933f, 1.4472381f, 0.0677227f,
                             0.0349342f,-0.0968930f, 1.2884099f };

    //CAT02
    const Mat3 XYZ2LMS { 1.4628067f,-0.1840623f,-0.2743606f,
                        -0.5217933f, 1.4472381f, 0.0677227f,
                         0.0349342f,-0.0968930f, 1.2884099f };
    //CAT02
    const Mat3 LMS2XYZ = XYZ2LMS.Inv();
};


#endif