#ifndef SOGLFW_COLORS_H
#define SOGLFW_COLORS_H

#include "vectors.h"

#define COLORS_FTOU 255u
#define COLORS_UTOF 0.003921568859f     // 1.f / 255


namespace Simple {
    //  ============  Derived color classes declaration  ============  \\

    struct RGB;
    struct RGB256;
    struct RGBA;
    struct RGBA256;
    struct WideRGB;
    struct WideRGBA;
    struct HSV;
    struct HSV256;
    struct HSVA;
    struct HSVA256;
    struct LAB;
    struct LABA;
    struct LMS;
    struct LMSA;
    struct ITP;
    struct ITPA;
    struct XYZ;
    struct XYZA;

    template <class C>
    struct ColRelTypes {
        typedef C RelBaseType;
        typedef C RelAlphaType;
    };


    //  ============  Utilities  ============  \\

    inline float clamp(float val) { return val < 0.f ? 0.f : (val > 1.f ? 1.f : val); }
    inline int clamp(int val) { return val < 0 ? 0 : (val > 255 ? 255 : val); }
    inline unsigned clamp(unsigned val) { return val > 255u ? 255u : val; }

    template <typename T>
    struct ColMax {
        static const T max = std::is_floating_point<T>::value ? 1.f : 255u;
    };

    template <typename T1, typename T2>
    struct ColTransform {
        static constexpr T2 coef = std::is_floating_point<T1>::value ? COLORS_FTOU : COLORS_UTOF;
    };

    template <typename T>
    struct ColTransform<T, T> {
        static constexpr T coef = 1;
    };

    template <typename T>
    struct ColComplementType {
        typedef T type;
    };
    template<> struct ColComplementType<float> {
        typedef unsigned char type;
    };
    template<> struct ColComplementType<unsigned char> {
        typedef float type;
    };

    template <class T>
    struct ColChannels {
        static const int channels =   ( std::is_same<T, float>::value || std::is_same<T, unsigned char>::value)
                                + 3 * (
                                        std::is_same<T, RGB >::value || std::is_same<T,  RGB256>::value ||
                                        std::is_same<T, HSV >::value || std::is_same<T, WideRGB>::value || 
                                        std::is_same<T, LAB >::value || std::is_same<T,     LMS>::value ||
                                        std::is_same<T, ITP >::value || std::is_same<T,     XYZ>::value)
                                + 4 * (
                                        std::is_same<T, RGBA>::value || std::is_same<T,  RGBA256>::value ||
                                        std::is_same<T, HSVA>::value || std::is_same<T, WideRGBA>::value || 
                                        std::is_same<T, LABA>::value || std::is_same<T,     LMSA>::value ||
                                        std::is_same<T, ITPA>::value || std::is_same<T,     XYZA>::value);
    };


    //  ============  Color base class  ============  \\

    template <class C, typename T, int ch>
    struct ColBase {
        using ColClass = C;
        using ColBaseT = T;
    };


    //  ============  3-3 channel operations  ============  \\

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, C1>::type operator+(Y op1, const ColBase<C1, T, 3> &op2)
        { return {op1 + *((T*)&op2), op1 + *((T*)&op2 + 1), op1 + *((T*)&op2 + 2)}; }

    template <class C1, typename T, typename Y>
    C1 operator-(Y op1, const ColBase<C1, T, 3> &op2)
        { return {op1 - *((T*)&op2), op1 - *((T*)&op2 + 1), op1 - *((T*)&op2 + 2)}; }

    template <class C1, typename T, typename Y>
    C1 operator*(Y op1, const ColBase<C1, T, 3> &op2)
        { return {op1 * *((T*)&op2), op1 * *((T*)&op2 + 1), op1 * *((T*)&op2 + 2)}; }

    template <class C1, typename T, typename Y>
    C1 operator/(Y op1, const ColBase<C1, T, 3> &op2) {
        return {notCloseToZero(*((T*)&op2    )) ? op1 / *((T*)&op2    ) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 1)) ? op1 / *((T*)&op2 + 1) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 2)) ? op1 / *((T*)&op2 + 2) : ColMax<T>::max};
    }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, C1>::type operator+(const ColBase<C1, T, 3> &op1, Y op2)
        { return {*((T*)&op1) + op2, *((T*)&op1 + 1) + op2, *((T*)&op1 + 2) + op2}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, C1>::type operator-(const ColBase<C1, T, 3> &op1, Y op2)
        { return {*((T*)&op1) - op2, *((T*)&op1 + 1) - op2, *((T*)&op1 + 2) - op2}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, C1>::type operator*(const ColBase<C1, T, 3> &op1, Y op2)
        { return {*((T*)&op1) * op2, *((T*)&op1 + 1) * op2, *((T*)&op1 + 2) * op2}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, C1>::type operator/(const ColBase<C1, T, 3> &op1, Y op2) {
        return notCloseToZero(op2) ?
                C1(*((T*)&op1) / op2, *((T*)&op1 + 1) / op2, *((T*)&op1 + 2) / op2) :
                C1(ColMax<T>::max);
    }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, void>::type operator+=(ColBase<C1, T, 3> &op1, Y op2)
        { op1 = {T(*((T*)&op1) + op2), T(*((T*)&op1 + 1) + op2), T(*((T*)&op1 + 2) + op2)}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, void>::type operator-=(ColBase<C1, T, 3> &op1, Y op2)
        { op1 = {T(*((T*)&op1) - op2), T(*((T*)&op1 + 1) - op2), T(*((T*)&op1 + 2) - op2)}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, void>::type operator*=(ColBase<C1, T, 3> &op1, Y op2)
        { op1 = {T(*((T*)&op1) * op2), T(*((T*)&op1 + 1) * op2), T(*((T*)&op1 + 2) * op2)}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, void>::type operator/=(ColBase<C1, T, 3> &op1, Y op2)
        { op1 = notCloseToZero(op2) ? C1(T(*((T*)&op1) / op2), T(*((T*)&op1 + 1) / op2), T(*((T*)&op1 + 2) / op2)) : C1(ColMax<T>::max); }


    template <class C1, class C2, typename T>
    C1 operator+(const ColBase<C1, T, 3> &op1, const ColBase<C2, T, 3> &o2)
        { C1 op2{o2}; return {*((T*)&op1) + *((T*)&op2), *((T*)&op1 + 1) + *((T*)&op2 + 1), *((T*)&op1 + 2) + *((T*)&op2 + 2)}; }

    template <class C1, class C2, typename T>
    C1 operator-(const ColBase<C1, T, 3> &op1, const ColBase<C2, T, 3> &o2)
        { C1 op2{o2}; return {*((T*)&op1) - *((T*)&op2), *((T*)&op1 + 1) - *((T*)&op2 + 1), *((T*)&op1 + 2) - *((T*)&op2 + 2)}; }

    template <class C1, class C2, typename T>
    C1 operator*(const ColBase<C1, T, 3> &op1, const ColBase<C2, T, 3> &o2)
        { C1 op2{o2}; return {*((T*)&op1) * *((T*)&op2), *((T*)&op1 + 1) * *((T*)&op2 + 1), *((T*)&op1 + 2) * *((T*)&op2 + 2)}; }

    template <class C1, class C2, typename T>
    C1 operator/(const ColBase<C1, T, 3> &op1, const ColBase<C2, T, 3> &o2) {
        C1 op2{o2};
        return {notCloseToZero(*((T*)&op2    )) ? *((T*)&op1    ) / *((T*)&op2    ) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 1)) ? *((T*)&op1 + 1) / *((T*)&op2 + 1) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 2)) ? *((T*)&op1 + 2) / *((T*)&op2 + 2) : ColMax<T>::max};
    }

    template <class C1, class C2, typename T>
    void operator+=(ColBase<C1, T, 3> &op1, const ColBase<C2, T, 3> &op2)
        { C1 o2{op2}; op1 = {*((T*)&op1) + *((T*)&o2), *((T*)&op1 + 1) + *((T*)&o2 + 1), *((T*)&op1 + 2) + *((T*)&o2 + 2)}; }

    template <class C1, class C2, typename T>
    void operator-=(ColBase<C1, T, 3> &op1, const ColBase<C2, T, 3> &op2)
        { C1 o2{op2}; op1 = {*((T*)&op1) - *((T*)&o2), *((T*)&op1 + 1) - *((T*)&o2 + 1), *((T*)&op1 + 2) - *((T*)&o2 + 2)}; }

    template <class C1, class C2, typename T>
    void operator*=(ColBase<C1, T, 3> &op1, const ColBase<C2, T, 3> &op2)
        { C1 o2{op2}; op1 = {*((T*)&op1) * *((T*)&o2), *((T*)&op1 + 1) * *((T*)&o2 + 1), *((T*)&op1 + 2) * *((T*)&o2 + 2)}; }

    template <class C1, class C2, typename T>
    void operator/=(ColBase<C1, T, 3> &op1, const ColBase<C2, T, 3> &op2) {
        C1 o2{op2};
        *((T*)&op1    ) = notCloseToZero(*((T*)&o2    )) ? clamp(*((T*)&op1    ) / *((T*)&o2    )) : ColMax<T>::max;
        *((T*)&op1 + 1) = notCloseToZero(*((T*)&o2 + 1)) ? clamp(*((T*)&op1 + 1) / *((T*)&o2 + 1)) : ColMax<T>::max;
        *((T*)&op1 + 2) = notCloseToZero(*((T*)&o2 + 2)) ? clamp(*((T*)&op1 + 2) / *((T*)&o2 + 2)) : ColMax<T>::max;
    }


    template <class C1, typename T>
    inline C1 MIN(const ColBase<C1, T, 3> &op1, const ColBase<C1, T, 3> &op2)
        { return {*((T*)&op1    ) < *((T*)&op2    ) ? *((T*)&op1    ) : *((T*)&op2    ),
                  *((T*)&op1 + 1) < *((T*)&op2 + 1) ? *((T*)&op1 + 1) : *((T*)&op2 + 1),
                  *((T*)&op1 + 2) < *((T*)&op2 + 2) ? *((T*)&op1 + 2) : *((T*)&op2 + 2)}; }

    template <class C1, typename T>
    inline C1 MAX(const ColBase<C1, T, 3> &op1, const ColBase<C1, T, 3> &op2)
        { return {*((T*)&op1    ) > *((T*)&op2    ) ? *((T*)&op1    ) : *((T*)&op2    ),
                  *((T*)&op1 + 1) > *((T*)&op2 + 1) ? *((T*)&op1 + 1) : *((T*)&op2 + 1),
                  *((T*)&op1 + 2) > *((T*)&op2 + 2) ? *((T*)&op1 + 2) : *((T*)&op2 + 2)}; }

    template <class C1, typename T>
    inline C1 DIF(const ColBase<C1, T, 3> &op1, const ColBase<C1, T, 3> &op2)
        { return {*((T*)&op1    ) > *((T*)&op2    ) ? *((T*)&op1    ) - *((T*)&op2    ) : *((T*)&op2    ) - *((T*)&op1    ),
                  *((T*)&op1 + 1) > *((T*)&op2 + 1) ? *((T*)&op1 + 1) - *((T*)&op2 + 1) : *((T*)&op2 + 1) - *((T*)&op1 + 1),
                  *((T*)&op1 + 2) > *((T*)&op2 + 2) ? *((T*)&op1 + 2) - *((T*)&op2 + 2) : *((T*)&op2 + 2) - *((T*)&op1 + 2)}; }

    template <class C1, typename T>
    inline C1 AVG(const ColBase<C1, T, 3> &op1, const ColBase<C1, T, 3> &op2)
        { return {(*((T*)&op1) + *((T*)&op2)) / 2, (*((T*)&op1 + 1) + *((T*)&op2 + 1)) / 2, (*((T*)&op1 + 2) + *((T*)&op2 + 2)) / 2}; }


    //  ============  4-4 channel operations  ============  \\

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, C1>::type operator+(Y op1, const ColBase<C1, T, 4> &op2)
        { return {T(op1 + *((T*)&op2)), T(op1 + *((T*)&op2 + 1)), T(op1 + *((T*)&op2 + 2)), T(op1 + *((T*)&op2 + 3))}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, C1>::type operator-(Y op1, const ColBase<C1, T, 4> &op2)
        { return {T(op1 - *((T*)&op2)), T(op1 - *((T*)&op2 + 1)), T(op1 - *((T*)&op2 + 2)), T(op1 - *((T*)&op2 + 3))}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, C1>::type operator*(Y op1, const ColBase<C1, T, 4> &op2)
        { return {T(op1 * *((T*)&op2)), T(op1 * *((T*)&op2 + 1)), T(op1 * *((T*)&op2 + 2)), T(op1 * *((T*)&op2 + 3))}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, C1>::type operator/(Y op1, const ColBase<C1, T, 4> &op2) {
        return {notCloseToZero(*((T*)&op2    )) ? T(op1 / *((T*)&op2    )) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 1)) ? T(op1 / *((T*)&op2 + 1)) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 2)) ? T(op1 / *((T*)&op2 + 2)) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 3)) ? T(op1 / *((T*)&op2 + 3)) : ColMax<T>::max};
    }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, C1>::type operator+(const ColBase<C1, T, 4> &op1, Y op2)
        { return {*((T*)&op1) + op2, *((T*)&op1 + 1) + op2, *((T*)&op1 + 2) + op2, *((T*)&op1 + 3) + op2}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, C1>::type operator-(const ColBase<C1, T, 4> &op1, Y op2)
        { return {*((T*)&op1) - op2, *((T*)&op1 + 1) - op2, *((T*)&op1 + 2) - op2, *((T*)&op1 + 3) - op2}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, C1>::type operator*(const ColBase<C1, T, 4> &op1, Y op2)
        { return {*((T*)&op1) * op2, *((T*)&op1 + 1) * op2, *((T*)&op1 + 2) * op2, *((T*)&op1 + 3) * op2}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, C1>::type operator/(const ColBase<C1, T, 4> &op1, Y op2) {
        return notCloseToZero(op2) ?
                C1(*((T*)&op1) / op2, *((T*)&op1 + 1) / op2, *((T*)&op1 + 2) / op2, *((T*)&op1 + 3) / op2) :
                C1(ColMax<T>::max);
    }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, void>::type operator+=(ColBase<C1, T, 4> &op1, Y op2)
        { op1 = {*((T*)&op1) + op2, *((T*)&op1 + 1) + op2, *((T*)&op1 + 2) + op2, *((T*)&op1 + 3) + op2}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, void>::type operator-=(ColBase<C1, T, 4> &op1, Y op2)
        { op1 = {*((T*)&op1) - op2, *((T*)&op1 + 1) - op2, *((T*)&op1 + 2) - op2, *((T*)&op1 + 3) - op2}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, void>::type operator*=(ColBase<C1, T, 4> &op1, Y op2)
        { op1 = {*((T*)&op1) * op2, *((T*)&op1 + 1) * op2, *((T*)&op1 + 2) * op2, *((T*)&op1 + 3) * op2}; }

    template <class C1, typename T, typename Y>
    typename std::enable_if<std::is_fundamental<Y>::value, void>::type operator/=(ColBase<C1, T, 4> &op1, Y op2) {
        op1 = notCloseToZero(op2) ?
                C1(*((T*)&op1) / op2, *((T*)&op1 + 1) / op2, *((T*)&op1 + 2) / op2, *((T*)&op1 + 3) / op2) :
                C1(ColMax<T>::max);
    }


    template <class C1, class C2, typename T>
    C1 operator+(const ColBase<C1, T, 4> &op1, const ColBase<C2, T, 4> &op2) {
        return {*((T*)&op1    ) + *((T*)&op2    ), *((T*)&op1 + 1) + *((T*)&op2 + 1),
                *((T*)&op1 + 2) + *((T*)&op2 + 2), *((T*)&op1 + 3) + *((T*)&op2 + 3)};
    }

    template <class C1, class C2, typename T>
    C1 operator-(const ColBase<C1, T, 4> &op1, const ColBase<C2, T, 4> &op2) {
        return {*((T*)&op1    ) - *((T*)&op2    ), *((T*)&op1 + 1) - *((T*)&op2 + 1),
                *((T*)&op1 + 2) - *((T*)&op2 + 2), *((T*)&op1 + 3) - *((T*)&op2 + 3)};
    }

    template <class C1, class C2, typename T>
    C1 operator*(const ColBase<C1, T, 4> &op1, const ColBase<C2, T, 4> &op2) {
        return {*((T*)&op1    ) * *((T*)&op2    ), *((T*)&op1 + 1) * *((T*)&op2 + 1),
                *((T*)&op1 + 2) * *((T*)&op2 + 2), *((T*)&op1 + 3) * *((T*)&op2 + 3)};
    }

    template <class C1, class C2, typename T>
    C1 operator/(const ColBase<C1, T, 4> &op1, const ColBase<C2, T, 4> &op2) {
        return {notCloseToZero(*((T*)&op2    )) ? *((T*)&op1    ) / *((T*)&op2    ) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 1)) ? *((T*)&op1 + 1) / *((T*)&op2 + 1) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 2)) ? *((T*)&op1 + 2) / *((T*)&op2 + 2) : ColMax<T>::max,
                notCloseToZero(*((T*)&op2 + 3)) ? *((T*)&op1 + 3) / *((T*)&op2 + 3) : ColMax<T>::max};
    }

    template <class C1, class C2, typename T>
    void operator+=(ColBase<C1, T, 4> &op1, const ColBase<C2, T, 4> &op2) {
        C1 o2{op2};
        op1 = {*((T*)&op1    ) + *((T*)&o2    ), *((T*)&op1 + 1) + *((T*)&o2 + 1),
               *((T*)&op1 + 2) + *((T*)&o2 + 2), *((T*)&op1 + 3) + *((T*)&o2 + 3)};
    }

    template <class C1, class C2, typename T>
    void operator-=(ColBase<C1, T, 4> &op1, const ColBase<C2, T, 4> &op2) {
        C1 o2{op2};
        op1 = {*((T*)&op1    ) - *((T*)&o2    ), *((T*)&op1 + 1) - *((T*)&o2 + 1),
               *((T*)&op1 + 2) - *((T*)&o2 + 2), *((T*)&op1 + 3) - *((T*)&o2 + 3)};
    }

    template <class C1, class C2, typename T>
    void operator*=(ColBase<C1, T, 4> &op1, const ColBase<C2, T, 4> &op2) {
        C1 o2{op2};
        op1 = {*((T*)&op1    ) * *((T*)&o2    ), *((T*)&op1 + 1) * *((T*)&o2 + 1),
               *((T*)&op1 + 2) * *((T*)&o2 + 2), *((T*)&op1 + 3) * *((T*)&o2 + 3)};
    }

    template <class C1, class C2, typename T>
    void operator/=(ColBase<C1, T, 4> &op1, const ColBase<C2, T, 4> &op2) {
        C1 o2{op2};
        *((T*)&op1    ) = notCloseToZero(*((T*)&o2    )) ? clamp(*((T*)&op1    ) / *((T*)&o2    )) : ColMax<T>::max;
        *((T*)&op1 + 1) = notCloseToZero(*((T*)&o2 + 1)) ? clamp(*((T*)&op1 + 1) / *((T*)&o2 + 1)) : ColMax<T>::max;
        *((T*)&op1 + 2) = notCloseToZero(*((T*)&o2 + 2)) ? clamp(*((T*)&op1 + 2) / *((T*)&o2 + 2)) : ColMax<T>::max;
        *((T*)&op1 + 3) = notCloseToZero(*((T*)&o2 + 3)) ? clamp(*((T*)&op1 + 3) / *((T*)&o2 + 3)) : ColMax<T>::max;
    }



    template <class C1, typename T>
    inline C1 MIN(const ColBase<C1, T, 4> &op1, const ColBase<C1, T, 4> &op2)
        { return {*((T*)&op1    ) < *((T*)&op2    ) ? *((T*)&op1    ) : *((T*)&op2    ),
                  *((T*)&op1 + 1) < *((T*)&op2 + 1) ? *((T*)&op1 + 1) : *((T*)&op2 + 1),
                  *((T*)&op1 + 2) < *((T*)&op2 + 2) ? *((T*)&op1 + 2) : *((T*)&op2 + 2),
                  *((T*)&op1 + 3) < *((T*)&op2 + 3) ? *((T*)&op1 + 3) : *((T*)&op2 + 3)}; }

    template <class C1, typename T>
    inline C1 MAX(const ColBase<C1, T, 4> &op1, const ColBase<C1, T, 4> &op2)
        { return {*((T*)&op1    ) > *((T*)&op2    ) ? *((T*)&op1    ) : *((T*)&op2    ),
                  *((T*)&op1 + 1) > *((T*)&op2 + 1) ? *((T*)&op1 + 1) : *((T*)&op2 + 1),
                  *((T*)&op1 + 2) > *((T*)&op2 + 2) ? *((T*)&op1 + 2) : *((T*)&op2 + 2),
                  *((T*)&op1 + 3) > *((T*)&op2 + 3) ? *((T*)&op1 + 3) : *((T*)&op2 + 3)}; }

    template <class C1, typename T>
    inline C1 DIF(const ColBase<C1, T, 4> &op1, const ColBase<C1, T, 4> &op2)
        { return {*((T*)&op1    ) > *((T*)&op2    ) ? *((T*)&op1    ) - *((T*)&op2    ) : *((T*)&op2    ) - *((T*)&op1    ),
                  *((T*)&op1 + 1) > *((T*)&op2 + 1) ? *((T*)&op1 + 1) - *((T*)&op2 + 1) : *((T*)&op2 + 1) - *((T*)&op1 + 1),
                  *((T*)&op1 + 2) > *((T*)&op2 + 2) ? *((T*)&op1 + 2) - *((T*)&op2 + 2) : *((T*)&op2 + 2) - *((T*)&op1 + 2),
                  *((T*)&op1 + 3) > *((T*)&op2 + 3) ? *((T*)&op1 + 3) - *((T*)&op2 + 3) : *((T*)&op2 + 3) - *((T*)&op1 + 3)}; }

    template <class C1, typename T>
    inline C1 AVG(const ColBase<C1, T, 4> &op1, const ColBase<C1, T, 4> &op2)
        { return {(*((T*)&op1    ) + *((T*)&op2    )) / 2, (*((T*)&op1 + 1) + *((T*)&op2 + 1)) / 2,
                  (*((T*)&op1 + 2) + *((T*)&op2 + 2)) / 2, (*((T*)&op1 + 3) + *((T*)&op2 + 3)) / 2}; }



    template <class C1, typename T>
    inline C1 PorterDuffBlendOver(const ColBase<C1, T, 4> &c1, const ColBase<C1, T, 4> &c2)
        { return {*((T*)&c1    ) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2    ), *((T*)&c1 + 1) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 1),
                  *((T*)&c1 + 2) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 2), *((T*)&c1 + 3) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 3)}; }

    template <class C1, typename T>
    inline C1 PorterDuffBlendIn  (const ColBase<C1, T, 4> &c1, const ColBase<C1, T, 4> &c2)
        { return {*((T*)&c2 + 3) * *((T*)&c1    ), *((T*)&c2 + 3) * *((T*)&c1 + 1),
                  *((T*)&c2 + 3) * *((T*)&c1 + 2), *((T*)&c2 + 3) * *((T*)&c1 + 3)}; }

    template <class C1, typename T>
    inline C1 PorterDuffBlendOut (const ColBase<C1, T, 4> &c1, const ColBase<C1, T, 4> &c2)
        { return {(ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1    ), (ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1 + 1),
                  (ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1 + 2), (ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1 + 3)}; }

    template <class C1, typename T>
    inline C1 PorterDuffBlendAtop(const ColBase<C1, T, 4> &c1, const ColBase<C1, T, 4> &c2)
        { return {*((T*)&c2 + 3) * *((T*)&c1    ) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2    ),
                  *((T*)&c2 + 3) * *((T*)&c1 + 1) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 1),
                  *((T*)&c2 + 3) * *((T*)&c1 + 2) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 2), *((T*)&c2 + 3)}; }

    template <class C1, typename T>
    inline C1 PorterDuffBlendXor (const ColBase<C1, T, 4> &c1, const ColBase<C1, T, 4> &c2)
        { return {(ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1    ) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2    ),
                  (ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1 + 1) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 1),
                  (ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1 + 2) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 2),
                  (ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1 + 3) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 3)}; }


    //  ============  Mixed-channel operations  ============  \\

    template <class C1, typename T>
    inline C1 PorterDuffBlendOver(const ColBase<C1, T, 4> &c1, const ColBase<typename ColRelTypes<C1>::RelAlphaType, T, 3> &c2)
        { return {*((T*)&c1    ) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2    ),
                  *((T*)&c1 + 1) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 1),
                  *((T*)&c1 + 2) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 2), ColMax<T>::max}; }

    template <class C1, typename T>
    inline C1 PorterDuffBlendIn  (const ColBase<C1, T, 4> &c1, const ColBase<typename ColRelTypes<C1>::RelAlphaType, T, 3> &c2)
        { return c1; }

    template <class C1, typename T>
    inline C1 PorterDuffBlendOut (const ColBase<C1, T, 4> &c1, const ColBase<typename ColRelTypes<C1>::RelAlphaType, T, 3> &c2)
        { return 0; }

    template <class C1, typename T>
    inline C1 PorterDuffBlendAtop(const ColBase<C1, T, 4> &c1, const ColBase<typename ColRelTypes<C1>::RelAlphaType, T, 3> &c2)
        { return {*((T*)&c1    ) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2    ),
                  *((T*)&c1 + 1) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 1),
                  *((T*)&c1 + 2) + (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 2), ColMax<T>::max}; }

    template <class C1, typename T>
    inline C1 PorterDuffBlendXor (const ColBase<C1, T, 4> &c1, const ColBase<typename ColRelTypes<C1>::RelAlphaType, T, 3> &c2)
        { return {(ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2    ), (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 1),
                  (ColMax<T>::max - *((T*)&c1 + 3)) * *((T*)&c2 + 2),  ColMax<T>::max - *((T*)&c1 + 3)}; }


    template <class C1, typename T>
    inline C1 PorterDuffBlendOver(const ColBase<typename ColRelTypes<C1>::RelAlphaType, T, 3> &c1, const ColBase<C1, T, 4> &c2)
        { return c1; }

    template <class C1, typename T>
    inline C1 PorterDuffBlendIn  (const ColBase<typename ColRelTypes<C1>::RelAlphaType, T, 3> &c1, const ColBase<C1, T, 4> &c2)
        { return {*((T*)&c2 + 3) * *((T*)&c1), *((T*)&c2 + 3) * *((T*)&c1 + 1), *((T*)&c2 + 3) * *((T*)&c1 + 2), *((T*)&c2 + 3)}; }

    template <class C1, typename T>
    inline C1 PorterDuffBlendOut (const ColBase<typename ColRelTypes<C1>::RelAlphaType, T, 3> &c1, const ColBase<C1, T, 4> &c2)
        { return {(ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1    ), (ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1 + 1),
                  (ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1 + 2),  ColMax<T>::max - *((T*)&c2 + 3)}; }

    template <class C1, typename T>
    inline C1 PorterDuffBlendAtop(const ColBase<typename ColRelTypes<C1>::RelAlphaType, T, 3> &c1, const ColBase<C1, T, 4> &c2)
        { return {*((T*)&c2 + 3) * *((T*)&c1), *((T*)&c2 + 3) * *((T*)&c1 + 1), *((T*)&c2 + 3) * *((T*)&c1 + 2), *((T*)&c2 + 3)}; }

    template <class C1, typename T>
    inline C1 PorterDuffBlendXor (const ColBase<typename ColRelTypes<C1>::RelAlphaType, T, 3> &c1, const ColBase<C1, T, 4> &c2)
        { return {(ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1    ), (ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1 + 1),
                  (ColMax<T>::max - *((T*)&c2 + 3)) * *((T*)&c1 + 2),  ColMax<T>::max - *((T*)&c2 + 3)}; }


    //  ============  RGB color model  ============  \\

    template <> struct ColRelTypes<RGB> {
        typedef RGBA RelAlphaType;
        typedef RGB256 RelBaseType;
    };

    template <> struct ColRelTypes<RGB256> {
        typedef RGBA256 RelAlphaType;
        typedef RGB RelBaseType;
    };

    template <class C1, typename T>
    struct RGBBase : ColBase<C1, T, 3> {
        T r = 0, g = 0, b = 0;

        RGBBase() {}
        RGBBase(T _val) : r{_val}, g{_val}, b{_val} {}
        RGBBase(T _v1, T _v2, T _v3) : r{_v1}, g{_v2}, b{_v3} {}
    };

    struct RGB : RGBBase<RGB, float> {
        RGB() {}
        RGB(ColBaseT _val) : RGBBase(_val) {}
        RGB(const ColClass &_col) : RGBBase(_col.r, _col.g, _col.b) {}
        RGB(ColBaseT _v1, ColBaseT _v2, ColBaseT _v3) : RGBBase(_v1, _v2, _v3) {}
    };
    struct RGB256 : RGBBase<RGB256, unsigned char> {
        RGB256() {}
        RGB256(ColBaseT _val) : RGBBase(_val) {}
        RGB256(const ColClass &_col) : RGBBase(_col.r, _col.g, _col.b) {}
        RGB256(ColBaseT _v1, ColBaseT _v2, ColBaseT _v3) : RGBBase(_v1, _v2, _v3) {}
    };


    //  ============  RGBA color model  ============  \\

    template <> struct ColRelTypes<RGBA> {
        typedef RGB RelAlphaType;
        typedef RGBA256 RelBaseType;
    };

    template <> struct ColRelTypes<RGBA256> {
        typedef RGB256 RelAlphaType;
        typedef RGBA RelBaseType;
    };

    template <class C1, typename T>
    struct RGBABase : ColBase<C1, T, 4> {
        T r = 0, g = 0, b = 0, a = 0;

        RGBABase() {}
        RGBABase(T _val) : r{_val}, g{_val}, b{_val}, a{_val} {}
        RGBABase(T _v1, T _v2, T _v3, T _v4) : r{_v1}, g{_v2}, b{_v3}, a{_v4} {}
    };

    struct RGBA : RGBABase<RGBA, float> {
        RGBA() {}
        RGBA(ColBaseT _val) : RGBABase(_val) {}
        RGBA(const ColClass &_col) : RGBABase(_col.r, _col.g, _col.b, _col.a) {}
        RGBA(ColBaseT _v1, ColBaseT _v2, ColBaseT _v3, ColBaseT _v4) : RGBABase(_v1, _v2, _v3, _v4) {}
        RGBA(const typename ColRelTypes<ColClass>::RelAlphaType &_col, ColBaseT _a = 0) : RGBABase(_col.r, _col.g, _col.b, _a) {}
        RGBA(const typename ColRelTypes<typename ColRelTypes<ColClass>::RelBaseType>::RelAlphaType &_col, ColBaseT _a = 0) :
                RGBABase(ColBaseT(_col.r * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef),
                         ColBaseT(_col.g * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef),
                         ColBaseT(_col.b * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef), _a) {}
        RGBA(const typename ColRelTypes<ColClass>::RelBaseType &_col) :
                RGBABase(ColBaseT(_col.r * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef),
                         ColBaseT(_col.g * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef),
                         ColBaseT(_col.b * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef),
                         ColBaseT(_col.a * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef)) {}
    };
    struct RGBA256 : RGBABase<RGBA256, unsigned char> {
        RGBA256() {}
        RGBA256(ColBaseT _val) : RGBABase(_val) {}
        RGBA256(const ColClass &_col) : RGBABase(_col.r, _col.g, _col.b, _col.a) {}
        RGBA256(ColBaseT _v1, ColBaseT _v2, ColBaseT _v3, ColBaseT _v4) : RGBABase(_v1, _v2, _v3, _v4) {}
        RGBA256(const typename ColRelTypes<ColClass>::RelAlphaType &_col, ColBaseT _a = 0) : RGBABase(_col.r, _col.g, _col.b, _a) {}
        RGBA256(const typename ColRelTypes<typename ColRelTypes<ColClass>::RelBaseType>::RelAlphaType &_col, ColBaseT _a = 0) :
                RGBABase(ColBaseT(_col.r * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef),
                         ColBaseT(_col.g * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef),
                         ColBaseT(_col.b * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef), _a) {}
        RGBA256(const typename ColRelTypes<ColClass>::RelBaseType &_col) :
                RGBABase(ColBaseT(_col.r * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef),
                         ColBaseT(_col.g * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef),
                         ColBaseT(_col.b * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef),
                         ColBaseT(_col.a * ColTransform<ColComplementType<ColBaseT>::type, ColBaseT>::coef)) {}
    };


    //  ============  HSV color model  ============  \\

    template <> struct ColRelTypes<HSV> {
        typedef HSVA RelAlphaType;
        typedef HSV256 RelBaseType;
    };

    template <> struct ColRelTypes<HSV256> {
        typedef HSVA256 RelAlphaType;
        typedef HSV RelBaseType;
    };

    template <class C1, typename T>
    struct HSVBase : ColBase<C1, T, 3> {
        T h = 0, s = 0, v = 0;

        HSVBase() {}
        HSVBase(T _val) : h{_val}, s{_val}, v{_val} {}
        HSVBase(T _v1, T _v2, T _v3) : h{_v1}, s{_v2}, v{_v3} {}
    };

    struct HSV : HSVBase<HSV, float> {
        HSV() {}
        HSV(ColBaseT _val) : HSVBase(_val) {}
        HSV(const ColClass &_col) : HSVBase(*((ColBaseT*)&_col), *((ColBaseT*)&_col + 1), *((ColBaseT*)&_col + 2)) {}
        HSV(ColBaseT _v1, ColBaseT _v2, ColBaseT _v3) : HSVBase(_v1, _v2, _v3) {}
    };
    struct HSV256 : HSVBase<HSV256, unsigned char> {
        HSV256() {}
        HSV256(ColBaseT _val) : HSVBase(_val) {}
        HSV256(const ColClass &_col) : HSVBase(*((ColBaseT*)&_col), *((ColBaseT*)&_col + 1), *((ColBaseT*)&_col + 2)) {}
        HSV256(ColBaseT _v1, ColBaseT _v2, ColBaseT _v3) : HSVBase(_v1, _v2, _v3) {}
    };


    //  ============  HSVA color model  ============  \\

    template <> struct ColRelTypes<HSVA> {
        typedef HSV RelAlphaType;
        typedef HSVA256 RelBaseType;
    };

    template <> struct ColRelTypes<HSVA> {
        typedef HSV256 RelAlphaType;
        typedef HSVA RelBaseType;
    };

    template <class C1, typename T>
    struct HSVABase : ColBase<C1, T, 4> {
        T h = 0, s = 0, v = 0, a = 0;

        HSVABase() {}
        HSVABase(T _val) : h{_val}, s{_val}, v{_val}, a{_val} {}
        HSVABase(T _v1, T _v2, T _v3, T _v4) : h{_v1}, s{_v2}, v{_v3}, a{_v4} {}
    };

    struct HSVA : HSVABase<HSVA, float> {
        HSVA() {}
        HSVA(ColBaseT _val) : HSVABase(_val) {}
        HSVA(const ColClass &_col) : HSVABase(*((ColBaseT*)&_col), *((ColBaseT*)&_col + 1), *((ColBaseT*)&_col + 2), *((ColBaseT*)&_col + 3)) {}
        HSVA(ColBaseT _v1, ColBaseT _v2, ColBaseT _v3, ColBaseT _v4) : HSVABase(_v1, _v2, _v3, _v4) {}
    };
    struct HSVA256 : HSVABase<HSVA256, unsigned char> {
        HSVA256() {}
        HSVA256(ColBaseT _val) : HSVABase(_val) {}
        HSVA256(const ColClass &_col) : HSVABase(_col.h, _col.s, _col.v, _col.a) {}
        HSVA256(ColBaseT _v1, ColBaseT _v2, ColBaseT _v3, ColBaseT _v4) : HSVABase(_v1, _v2, _v3, _v4) {}
    };

};


#endif