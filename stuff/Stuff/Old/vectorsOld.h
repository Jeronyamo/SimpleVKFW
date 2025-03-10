#ifndef SOGLFW_VECTORS_H
#define SOGLFW_VECTORS_H

#include "common.h"

#include <cmath>
#include <cstring>
#include <algorithm>


#ifdef VECTORS_USE_DOUBLE_PREC
#define _vectors_flt_pt_type double
#define VECTORS_EPS SOGLFW_EPS_DBL
#else
#define _vectors_flt_pt_type float
#define VECTORS_EPS SOGLFW_EPS_FLT
#endif


namespace Simple {
    const struct MatPlaceholder {} _;

    using std::sqrt;
    using std::memcpy;
    using std::swap;
    //using std::min;
    //using std::max;


    template <class C> inline bool notCloseToZero(C  _val) { return _val <= -SOGLFW_1EPS || _val >= SOGLFW_1EPS; }

    //template <typename T> inline const T min(const T op1, const T op2) { return op1 < op2 ? op1 : op2;}
    //template <typename T> inline const T max(const T op1, const T op2) { return op1 > op2 ? op1 : op2;}
    

    template <typename A, bool _val> struct safe_make_signed {};
    template <typename A> struct safe_make_signed<A, true> {
        using type = std::make_signed<A>;
    };
    template <typename A> struct safe_make_signed<A, false> {
        using type = A;
    };
    template <typename A> struct res_type_for_sub {
        using type = safe_make_signed<A, std::is_unsigned<A>::value>;
    };


    //  ============  Vector classes declaration  ============  \\

    template <class C> struct Vec2Base;
    template <class C> struct Vec3Base;
    template <class C> struct Vec4Base;

    template <class C> struct Vec2RefBase;
    template <class C> struct Vec3RefBase;
    template <class C> struct Vec4RefBase;

    typedef Vec2Base<int> IVec2;
    typedef Vec2Base<bool> BVec2;
    typedef Vec2Base<unsigned> UVec2;
    typedef Vec2Base<_vectors_flt_pt_type> Vec2;

    typedef Vec3Base<int> IVec3;
    typedef Vec3Base<bool> BVec3;
    typedef Vec3Base<unsigned> UVec3;
    typedef Vec3Base<_vectors_flt_pt_type> Vec3;

    typedef Vec4Base<int> IVec4;
    typedef Vec4Base<bool> BVec4;
    typedef Vec4Base<unsigned> UVec4;
    typedef Vec4Base<_vectors_flt_pt_type> Vec4;


    //  ============  Matrix classes declaration  ============  \\

    template <class C> struct Mat3Base;
    
    template <class C> struct Mat3RefBase;

    typedef Mat3Base<int> IMat3;
    typedef Mat3Base<bool> BMat3;
    typedef Mat3Base<unsigned> UMat3;
    typedef Mat3Base<_vectors_flt_pt_type> Mat3;


    //  ============  Vec 2  ============  \\

    template <class C>
    struct Vec2Base {
        C x, y;

        Vec2Base() : x(0), y(0) {}
        Vec2Base(C &_x, C &_y) : x(_x), y(_y) {}
        template <class Y> Vec2Base(Y _val) : x(_val), y(_val) {}
        template <class X, class Y> Vec2Base(X _x, Y _y) : x(_x), y(_y) {}
        template <class Y> Vec2Base(const Vec2Base<Y> &_vec) : x(_vec.x), y(_vec.y) {}


        _vectors_flt_pt_type norm() const { return sqrt(x * x + y * y); }
        template <class Y> _vectors_flt_pt_type dot(const Vec2Base<Y> &op) const { return x * op.x + y * op.y; }
        Vec2Base<_vectors_flt_pt_type> inverse() const { return { notCloseToZero(x) ? 1. / x : x, notCloseToZero(y) ? 1. / y : y }; }
        Vec2Base<_vectors_flt_pt_type> normalize() const {
            _vectors_flt_pt_type norm = sqrt(x * x + y * y);
            norm = notCloseToZero(norm) ? 1. / norm : 1.;
            return { x * norm, y * norm };
        }

        template <class Y> const Vec2Base<C> &operator=(const Vec2Base <Y> &ref) { x = ref.x; y = ref.y; return *this; }
        template <class Y> void operator+=(const Vec2Base<Y> &op) { x += op.x; y += op.y; }
        template <class Y> void operator-=(const Vec2Base<Y> &op) { x -= op.x; y -= op.y; }
        template <class Y> void operator*=(const Vec2Base<Y> &op) { x *= op.x; y *= op.y; }
        template <class Y> void operator/=(const Vec2Base<Y> &op) { x /= op.x; y /= op.y; }
        template <class Y> void operator+=(const Y &op) { x += op.x; y += op.y; }
        template <class Y> void operator-=(const Y &op) { x -= op.x; y -= op.y; }
        template <class Y> void operator*=(const Y &op) { x *= op.x; y *= op.y; }
        template <class Y> void operator/=(const Y &op) { x /= op.x; y /= op.y; }


        C &operator[](const unsigned index) { return *((C*)this + index); }
        const C &operator[](const unsigned index) const { return *((C*)this + index); }
        Vec2Base<C&> operator[](const char (&attrs)[2 + 1])
            { typename std::remove_reference<C>::type *ptr[2]{&x, &y}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x']}; }
        Vec3Base<C&> operator[](const char (&attrs)[3 + 1])
            { typename std::remove_reference<C>::type *ptr[2]{&x, &y}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x'], *ptr[attrs[2] - 'x']}; }
        Vec4Base<C&> operator[](const char (&attrs)[4 + 1])
            { typename std::remove_reference<C>::type *ptr[2]{&x, &y}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x'], *ptr[attrs[2] - 'x'], *ptr[attrs[3] - 'x']}; }
        const Vec2Base<C&> operator[](const char (&attrs)[2 + 1]) const
            { typename std::remove_reference<C>::type *ptr[2]{&x, &y}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x']}; }
        const Vec3Base<C&> operator[](const char (&attrs)[3 + 1]) const
            { typename std::remove_reference<C>::type *ptr[2]{&x, &y}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x'], *ptr[attrs[2] - 'x']}; }
        const Vec4Base<C&> operator[](const char (&attrs)[4 + 1]) const
            { typename std::remove_reference<C>::type *ptr[2]{&x, &y}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x'], *ptr[attrs[2] - 'x'], *ptr[attrs[3] - 'x']}; }
    };


    template <class C>
    struct Vec2RefBase {
        C &x, &y;

        Vec2RefBase(C &_x, C &_y) : x{_x}, y{_y} {}


        _vectors_flt_pt_type norm() const { return sqrt(x * x + y * y); }
        template <class Y> _vectors_flt_pt_type dot(const Vec2Base<Y> &op) const { return x * op.x + y * op.y; }
        Vec2Base<_vectors_flt_pt_type> inverse() const { return { notCloseToZero(x) ? 1. / x : x, notCloseToZero(y) ? 1. / y : y }; }
        Vec2Base<_vectors_flt_pt_type> normalize() const {
            _vectors_flt_pt_type norm = sqrt(x * x + y * y);
            norm = notCloseToZero(norm) ? 1. / norm : 1.;
            return { x * norm, y * norm };
        }

        const Vec2RefBase<C> operator=(const Vec2RefBase <C> &ref) { x = ref.x; y = ref.y; return *this; }
        template <class Y> const Vec2RefBase<C> operator=(const Vec2RefBase <Y> &ref) { x = ref.x; y = ref.y; return *this; }
        template <class Y> const Vec2RefBase<C> operator=(const Vec2Base <Y> &ref) { x = ref.x; y = ref.y; return *this; }

        template <class Y> void operator+=(const Vec2Base<Y>    &op) { x += op.x; y += op.y; }
        template <class Y> void operator-=(const Vec2Base<Y>    &op) { x -= op.x; y -= op.y; }
        template <class Y> void operator*=(const Vec2Base<Y>    &op) { x *= op.x; y *= op.y; }
        template <class Y> void operator/=(const Vec2Base<Y>    &op) { x /= op.x; y /= op.y; }
        template <class Y> void operator+=(const Vec2RefBase<Y> &op) { x += op.x; y += op.y; }
        template <class Y> void operator-=(const Vec2RefBase<Y> &op) { x -= op.x; y -= op.y; }
        template <class Y> void operator*=(const Vec2RefBase<Y> &op) { x *= op.x; y *= op.y; }
        template <class Y> void operator/=(const Vec2RefBase<Y> &op) { x /= op.x; y /= op.y; }
        template <class Y> void operator+=(const Y &op) { x += op.x; y += op.y; }
        template <class Y> void operator-=(const Y &op) { x -= op.x; y -= op.y; }
        template <class Y> void operator*=(const Y &op) { x *= op.x; y *= op.y; }
        template <class Y> void operator/=(const Y &op) { x /= op.x; y /= op.y; }

        template <class Y> Vec2Base<typename std::common_type<C, Y>::type> operator+(const Vec2Base<Y> &op2) const
            { return {x + op2.x, y + op2.y}; }
        template <class Y> Vec2Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Vec2Base<Y> &op2) const
            { return {x - op2.x, y - op2.y}; }
        template <class Y> Vec2Base<typename std::common_type<C, Y>::type> operator*(const Vec2Base<Y> &op2) const
            { return {x * op2.x, y * op2.y}; }
        template <class Y> Vec2Base<typename std::common_type<C, Y>::type> operator/(const Vec2Base<Y> &op2) const
            { return {x / op2.x, y / op2.y}; }

        template <class Y> Vec2Base<typename std::common_type<C, Y>::type> operator+(const Vec2RefBase<Y> &op2) const
            { return {x + op2.x, y + op2.y}; }
        template <class Y> Vec2Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Vec2RefBase<Y> &op2) const
            { return {x - op2.x, y - op2.y}; }
        template <class Y> Vec2Base<typename std::common_type<C, Y>::type> operator*(const Vec2RefBase<Y> &op2) const
            { return {x * op2.x, y * op2.y}; }
        template <class Y> Vec2Base<typename std::common_type<C, Y>::type> operator/(const Vec2RefBase<Y> &op2) const
            { return {x / op2.x, y / op2.y}; }

        template <class Y> Vec2Base<typename std::common_type<C, Y>::type> operator+(const Y op2) const
            { return {x + op2, y + op2}; }
        template <class Y> Vec2Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Y op2) const
            { return {x - op2, y - op2}; }
        template <class Y> Vec2Base<typename std::common_type<C, Y>::type> operator*(const Y op2) const
            { return {x * op2, y * op2}; }
        template <class Y> Vec2Base<typename std::common_type<C, Y>::type> operator/(const Y op2) const
            { return {x / op2, y / op2}; }

        C &operator[](const unsigned index) { C *arr[2]{&x, &y}; return *arr[index]; }
        const C &operator[](const unsigned index) const { C *arr[2]{&x, &y}; return *arr[index]; }
        Vec2RefBase<C> operator[](const char (&attrs)[2 + 1])
            { C *ptr[2]{&x, &y}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x']}; }
        Vec3RefBase<C> operator[](const char (&attrs)[3 + 1])
            { C *ptr[2]{&x, &y}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x'], *ptr[attrs[2] - 'x']}; }
        Vec4RefBase<C> operator[](const char (&attrs)[4 + 1])
            { C *ptr[2]{&x, &y}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x'], *ptr[attrs[2] - 'x'], *ptr[attrs[3] - 'x']}; }
        const Vec2RefBase<C> operator[](const char (&attrs)[2 + 1]) const
            { C *ptr[2]{&x, &y}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x']}; }
        const Vec3RefBase<C> operator[](const char (&attrs)[3 + 1]) const
            { C *ptr[2]{&x, &y}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x'], *ptr[attrs[2] - 'x']}; }
        const Vec4RefBase<C> operator[](const char (&attrs)[4 + 1]) const
            { C *ptr[2]{&x, &y}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x'], *ptr[attrs[2] - 'x'], *ptr[attrs[3] - 'x']}; }
    };

    template <typename C, class Y> const Vec2Base<typename std::common_type<C, Y>::type> operator+(const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x + op2.x, op1.y + op2.y}; }
    template <typename C, class Y> const Vec2Base<typename safe_make_signed<typename std::common_type<C, Y>::type,
                                                                    std::is_unsigned<std::common_type<C, Y>>::value>::type> operator-(const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x - op2.x, op1.y - op2.y}; }
    template <typename C, class Y> const Vec2Base<typename std::common_type<C, Y>::type> operator*(const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x * op2.x, op1.y * op2.y}; }
    template <typename C, class Y> const Vec2Base<typename std::common_type<C, Y>::type> operator/(const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x / op2.x, op1.y / op2.y}; }

    template <typename C, class Y> const Vec2Base<typename std::common_type<C, Y>::type> operator+(const Vec2Base<C> &op1, const Vec2RefBase<Y> &op2)
        { return {op1.x + op2.x, op1.y + op2.y}; }
    template <typename C, class Y> const Vec2Base<typename safe_make_signed<typename std::common_type<C, Y>::type,
                                                                    std::is_unsigned<std::common_type<C, Y>>::value>::type> operator-(const Vec2Base<C> &op1, const Vec2RefBase<Y> &op2)
        { return {op1.x - op2.x, op1.y - op2.y}; }
    template <typename C, class Y> const Vec2Base<typename std::common_type<C, Y>::type> operator*(const Vec2Base<C> &op1, const Vec2RefBase<Y> &op2)
        { return {op1.x * op2.x, op1.y * op2.y}; }
    template <typename C, class Y> const Vec2Base<typename std::common_type<C, Y>::type> operator/(const Vec2Base<C> &op1, const Vec2RefBase<Y> &op2)
        { return {op1.x / op2.x, op1.y / op2.y}; }

    template <typename C, class Y> Vec2Base<typename std::common_type<C, Y>::type> operator+(const Vec2Base<C> &op1, const Y op2)
        { return {op1.x + op2, op1.y + op2}; }
    template <typename C, class Y> Vec2Base<typename safe_make_signed<typename std::common_type<C, Y>::type,
                                                                    std::is_unsigned<std::common_type<C, Y>>::value>::type> operator-(const Vec2Base<C> &op1, const Y op2)
        { return {op1.x - op2, op1.y - op2}; }
    template <typename C, class Y> Vec2Base<typename std::common_type<C, Y>::type> operator*(const Vec2Base<C> &op1, const Y op2)
        { return {op1.x * op2, op1.y * op2}; }
    template <typename C, class Y> Vec2Base<typename std::common_type<C, Y>::type> operator/(const Vec2Base<C> &op1, const Y op2)
        { return {op1.x / op2, op1.y / op2}; }

    template <typename C, typename Y> Vec2Base<typename std::common_type<C, Y>::type> operator+(const Y op1, const Vec2Base<C> &op2)
        { return {op1 + op2.x, op1 + op2.y}; }
    template <typename C, typename Y> Vec2Base<typename safe_make_signed<typename std::common_type<C, Y>::type,
                                                                      std::is_unsigned<std::common_type<C, Y>>::value>::type> operator-(const Y op1, const Vec2Base<C> &op2)
        { return {op1 - op2.x, op1 - op2.y}; }
    template <typename C, typename Y> Vec2Base<typename std::common_type<C, Y>::type> operator*(const Y op1, const Vec2Base<C> &op2)
        { return {op1 * op2.x, op1 * op2.y}; }
    template <typename C, typename Y> Vec2Base<typename std::common_type<C, Y>::type> operator/(const Y op1, const Vec2Base<C> &op2)
        { return {op1 / op2.x, op1 / op2.y}; }

    template <class C, class Y> Vec2Base<typename std::common_type<C, Y>::type> operator+(const Y op1, const Vec2RefBase<C> &op2)
        { return {op1 + op2.x, op1 + op2.y}; }
    template <class C, class Y> Vec2Base<typename safe_make_signed<typename std::common_type<C, Y>::type,
                                                                      std::is_unsigned<std::common_type<C, Y>>::value>::type> operator-(const Y op1, const Vec2RefBase<C> &op2)
        { return {op1 - op2.x, op1 - op2.y}; }
    template <class C, class Y> Vec2Base<typename std::common_type<C, Y>::type> operator*(const Y op1, const Vec2RefBase<C> &op2)
        { return {op1 * op2.x, op1 * op2.y}; }
    template <class C, class Y> Vec2Base<typename std::common_type<C, Y>::type> operator/(const Y op1, const Vec2RefBase<C> &op2)
        { return {op1 / op2.x, op1 / op2.y}; }


    //  ============  Vec 3  ============  \\

    template <class C>
    struct Vec3Base {
        C x, y, z;

        Vec3Base() : x(0), y(0), z(0) {}
        template <class Y> Vec3Base(Y _val) : x(_val), y(_val), z(_val) {}
        template <class X, class Y, class Z> Vec3Base(X _x, Y _y, Z _z) : x(_x), y(_y), z(_z) {}
        template <class Y> Vec3Base(const Vec3Base<Y> &_vec) : x(_vec.x), y(_vec.y), z(_vec.z) {}
        template <class Y> Vec3Base(const Vec3RefBase<Y> &_vec) : x(_vec.x), y(_vec.y), z(_vec.z) {}


        _vectors_flt_pt_type norm() const { return sqrt(x * x + y * y + z * z); }
        template <class Y> _vectors_flt_pt_type dot(const Vec3Base<Y> &op) const { return x * op.x + y * op.y + z * op.z; }
        Vec3Base<_vectors_flt_pt_type> inverse() const
            { return { notCloseToZero(x) ? 1. / x : x, notCloseToZero(y) ? 1. / y : y, notCloseToZero(z) ? 1. / z : z }; }
        Vec3Base<_vectors_flt_pt_type> normalize() const {
            _vectors_flt_pt_type norm = sqrt(x * x + y * y + z * z);
            norm = notCloseToZero(norm) ? 1. / norm : 1.;
            return { x * norm, y * norm, z * norm };
        }


        template <class Y> void operator+=(const Vec3Base<Y>    &op) { x += op.x; y += op.y; z += op.z; }
        template <class Y> void operator-=(const Vec3Base<Y>    &op) { x -= op.x; y -= op.y; z -= op.z; }
        template <class Y> void operator*=(const Vec3Base<Y>    &op) { x *= op.x; y *= op.y; z *= op.z; }
        template <class Y> void operator/=(const Vec3Base<Y>    &op) { x /= op.x; y /= op.y; z /= op.z; }
        template <class Y> void operator+=(const Vec3RefBase<Y> &op) { x += op.x; y += op.y; z += op.z; }
        template <class Y> void operator-=(const Vec3RefBase<Y> &op) { x -= op.x; y -= op.y; z -= op.z; }
        template <class Y> void operator*=(const Vec3RefBase<Y> &op) { x *= op.x; y *= op.y; z *= op.z; }
        template <class Y> void operator/=(const Vec3RefBase<Y> &op) { x /= op.x; y /= op.y; z /= op.z; }
        template <class Y> void operator+=(const Y &op) { x += op.x; y += op.y; z += op.z; }
        template <class Y> void operator-=(const Y &op) { x -= op.x; y -= op.y; z -= op.z; }
        template <class Y> void operator*=(const Y &op) { x *= op.x; y *= op.y; z *= op.z; }
        template <class Y> void operator/=(const Y &op) { x /= op.x; y /= op.y; z /= op.z; }

        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator+(const Vec3Base<Y> &op2) const
            { return {x + op2.x, y + op2.y, z + op2.z}; }
        template <class Y> Vec3Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Vec3Base<Y> &op2) const
            { return {x - op2.x, y - op2.y, z - op2.z}; }
        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator*(const Vec3Base<Y> &op2) const
            { return {x * op2.x, y * op2.y, z * op2.z}; }
        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator/(const Vec3Base<Y> &op2) const
            { return {x / op2.x, y / op2.y, z / op2.z}; }

        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator+(const Vec3RefBase<Y> &op2) const
            { return {x + op2.x, y + op2.y, z + op2.z}; }
        template <class Y> Vec3Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Vec3RefBase<Y> &op2) const
            { return {x - op2.x, y - op2.y, z - op2.z}; }
        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator*(const Vec3RefBase<Y> &op2) const
            { return {x * op2.x, y * op2.y, z * op2.z}; }
        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator/(const Vec3RefBase<Y> &op2) const
            { return {x / op2.x, y / op2.y, z / op2.z}; }

        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator+(const Y op2) const
            { return {x + op2, y + op2, z + op2}; }
        template <class Y> Vec3Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Y op2) const
            { return {x - op2, y - op2, z - op2}; }
        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator*(const Y op2) const
            { return {x * op2, y * op2, z * op2}; }
        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator/(const Y op2) const
            { return {x / op2, y / op2, z / op2}; }

        C &operator[](const unsigned index) { return *((C*)this + index); }
        const C &operator[](const unsigned index) const { return *((C*)this + index); }
        Vec2RefBase<C> operator[](const char (&attrs)[2 + 1])
            { return {*((C*)this + (attrs[0] - 'x')), *((C*)this + (attrs[1] - 'x'))}; }
        Vec3RefBase<C> operator[](const char (&attrs)[3 + 1])
            { return {*((C*)this + (attrs[0] - 'x')), *((C*)this + (attrs[1] - 'x')), *((C*)this + (attrs[2] - 'x'))}; }
        Vec4RefBase<C> operator[](const char (&attrs)[4 + 1])
            { return {*((C*)this + (attrs[0] - 'x')), *((C*)this + (attrs[1] - 'x')), *((C*)this + (attrs[2] - 'x')), *((C*)this + (attrs[3] - 'x'))}; }
        const Vec2RefBase<C> operator[](const char (&attrs)[2 + 1]) const
            { return {*((C*)this + (attrs[0] - 'x')), *((C*)this + (attrs[1] - 'x'))}; }
        const Vec3RefBase<C> operator[](const char (&attrs)[3 + 1]) const
            { return {*((C*)this + (attrs[0] - 'x')), *((C*)this + (attrs[1] - 'x')), *((C*)this + (attrs[2] - 'x'))}; }
        const Vec4RefBase<C> operator[](const char (&attrs)[4 + 1]) const
            { return {*((C*)this + (attrs[0] - 'x')), *((C*)this + (attrs[1] - 'x')), *((C*)this + (attrs[2] - 'x')), *((C*)this + (attrs[3] - 'x'))}; }
    };


    template <class C>
    struct Vec3RefBase {
        C &x, &y, &z;

        Vec3RefBase(C &_x, C &_y, C &_z) : x{_x}, y{_y}, z{_z} {}


        _vectors_flt_pt_type norm() const { return sqrt(x * x + y * y + z * z); }
        template <class Y> _vectors_flt_pt_type dot(const Vec3Base<Y> &op) const { return x * op.x + y * op.y + z * op.z; }
        Vec3Base<_vectors_flt_pt_type> inverse() const
            { return { notCloseToZero(x) ? 1. / x : x, notCloseToZero(y) ? 1. / y : y, notCloseToZero(z) ? 1. / z : z }; }
        Vec3Base<_vectors_flt_pt_type> normalize() const {
            _vectors_flt_pt_type norm = sqrt(x * x + y * y + z * z);
            norm = notCloseToZero(norm) ? 1. / norm : 1.;
            return { x * norm, y * norm, z * norm };
        }


        const Vec3RefBase<C> operator=(const Vec3RefBase <C> &ref) { x = ref.x; y = ref.y; z = ref.z; return *this; }
        template <class Y> const Vec3RefBase<C> operator=(const Vec3RefBase <Y> &ref) { x = ref.x; y = ref.y; z = ref.z; return *this; }
        Vec3RefBase<C> &operator=(const Vec3Base <C> &ref) { x = ref.x; y = ref.y; z = ref.z; return *this; }

        template <class Y> void operator+=(const Vec3Base<Y>    &op) { x += op.x; y += op.y; z += op.z; }
        template <class Y> void operator-=(const Vec3Base<Y>    &op) { x -= op.x; y -= op.y; z -= op.z; }
        template <class Y> void operator*=(const Vec3Base<Y>    &op) { x *= op.x; y *= op.y; z *= op.z; }
        template <class Y> void operator/=(const Vec3Base<Y>    &op) { x /= op.x; y /= op.y; z /= op.z; }
        template <class Y> void operator+=(const Vec3RefBase<Y> &op) { x += op.x; y += op.y; z += op.z; }
        template <class Y> void operator-=(const Vec3RefBase<Y> &op) { x -= op.x; y -= op.y; z -= op.z; }
        template <class Y> void operator*=(const Vec3RefBase<Y> &op) { x *= op.x; y *= op.y; z *= op.z; }
        template <class Y> void operator/=(const Vec3RefBase<Y> &op) { x /= op.x; y /= op.y; z /= op.z; }
        template <class Y> void operator+=(const Y &op) { x += op.x; y += op.y; z += op.z; }
        template <class Y> void operator-=(const Y &op) { x -= op.x; y -= op.y; z -= op.z; }
        template <class Y> void operator*=(const Y &op) { x *= op.x; y *= op.y; z *= op.z; }
        template <class Y> void operator/=(const Y &op) { x /= op.x; y /= op.y; z /= op.z; }

        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator+(const Vec3Base<Y> &op2) const
            { return {x + op2.x, y + op2.y, z + op2.z}; }
        template <class Y> Vec3Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Vec3Base<Y> &op2) const
            { return {x - op2.x, y - op2.y, z - op2.z}; }
        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator*(const Vec3Base<Y> &op2) const
            { return {x * op2.x, y * op2.y, z * op2.z}; }
        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator/(const Vec3Base<Y> &op2) const
            { return {x / op2.x, y / op2.y, z / op2.z}; }

        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator+(const Vec3RefBase<Y> &op2) const
            { return {x + op2.x, y + op2.y, z + op2.z}; }
        template <class Y> Vec3Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Vec3RefBase<Y> &op2) const
            { return {x - op2.x, y - op2.y, z - op2.z}; }
        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator*(const Vec3RefBase<Y> &op2) const
            { return {x * op2.x, y * op2.y, z * op2.z}; }
        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator/(const Vec3RefBase<Y> &op2) const
            { return {x / op2.x, y / op2.y, z / op2.z}; }

        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator+(const Y op2) const
            { return {x + op2, y + op2, z + op2}; }
        template <class Y> Vec3Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Y op2) const
            { return {x - op2, y - op2, z - op2}; }
        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator*(const Y op2) const
            { return {x * op2, y * op2, z * op2}; }
        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator/(const Y op2) const
            { return {x / op2, y / op2, z / op2}; }

        C &operator[](const unsigned index) { C *arr[3]{&x, &y, &z}; return *arr[index]; }
        const C &operator[](const unsigned index) const { C *arr[3]{&x, &y, &z}; return *arr[index]; }
        Vec2RefBase<C> operator[](const char (&attrs)[2 + 1])
            { C *ptr[3]{&x, &y, &z}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x']}; }
        Vec3RefBase<C> operator[](const char (&attrs)[3 + 1])
            { C *ptr[3]{&x, &y, &z}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x'], *ptr[attrs[2] - 'x']}; }
        Vec4RefBase<C> operator[](const char (&attrs)[4 + 1])
            { C *ptr[3]{&x, &y, &z}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x'], *ptr[attrs[2] - 'x'], *ptr[attrs[3] - 'x']}; }
        const Vec2RefBase<C> operator[](const char (&attrs)[2 + 1]) const
            { C *ptr[3]{&x, &y, &z}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x']}; }
        const Vec3RefBase<C> operator[](const char (&attrs)[3 + 1]) const
            { C *ptr[3]{&x, &y, &z}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x'], *ptr[attrs[2] - 'x']}; }
        const Vec4RefBase<C> operator[](const char (&attrs)[4 + 1]) const
            { C *ptr[3]{&x, &y, &z}; return {*ptr[attrs[0] - 'x'], *ptr[attrs[1] - 'x'], *ptr[attrs[2] - 'x'], *ptr[attrs[3] - 'x']}; }
    };


    template <class C, class Y> Vec3Base<typename std::common_type<C, Y>::type> operator+(const Y op1, const Vec3Base<C> &op2)
        { return {op1 + op2.x, op1 + op2.y, op1 + op2.z}; }
    template <class C, class Y> Vec3Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                         typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                         typename std::common_type<C, Y>::type>::type> operator-(const Y op1, const Vec3Base<C> &op2)
        { return {op1 - op2.x, op1 - op2.y, op1 - op2.z}; }
    template <class C, class Y> Vec3Base<typename std::common_type<C, Y>::type> operator*(const Y op1, const Vec3Base<C> &op2)
        { return {op1 * op2.x, op1 * op2.y, op1 * op2.z}; }
    template <class C, class Y> Vec3Base<typename std::common_type<C, Y>::type> operator/(const Y op1, const Vec3Base<C> &op2)
        { return {op1 / op2.x, op1 / op2.y, op1 / op2.z}; }

    template <class C, class Y> Vec3Base<typename std::common_type<C, Y>::type> operator+(const Y op1, const Vec3RefBase<C> &op2)
        { return {op1 + op2.x, op1 + op2.y, op1 + op2.z}; }
    template <class C, class Y> Vec3Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                         typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                         typename std::common_type<C, Y>::type>::type> operator-(const Y op1, const Vec3RefBase<C> &op2)
        { return {op1 - op2.x, op1 - op2.y, op1 - op2.z}; }
    template <class C, class Y> Vec3Base<typename std::common_type<C, Y>::type> operator*(const Y op1, const Vec3RefBase<C> &op2)
        { return {op1 * op2.x, op1 * op2.y, op1 * op2.z}; }
    template <class C, class Y> Vec3Base<typename std::common_type<C, Y>::type> operator/(const Y op1, const Vec3RefBase<C> &op2)
        { return {op1 / op2.x, op1 / op2.y, op1 / op2.z}; }


    //  ============  Vec 4  ============  \\

    template <class C>
    struct Vec4Base {
        C x, y, z, w;

        Vec4Base() : x(0), y(0), z(0), w(0) {}
        template <class Y> Vec4Base(Y _val) : x(_val), y(_val), z(_val), w(_val) {}
        template <class X, class Y, class Z, class W> Vec4Base(X _x, Y _y, Z _z, W _w) : x(_x), y(_y), z(_z), w(_w) {}
        template <class Y> Vec4Base(const Vec4Base<Y> &_vec) : x(_vec.x), y(_vec.y), z(_vec.z), w(_vec.w) {}
        template <class Y> Vec4Base(const Vec4RefBase<Y> &_vec) : x(_vec.x), y(_vec.y), z(_vec.z), w(_vec.w) {}


        _vectors_flt_pt_type norm() const { return sqrt(x * x + y * y + z * z + w * w); }
        template <class Y> _vectors_flt_pt_type dot(const Vec4Base<Y> &op) const { return x * op.x + y * op.y + z * op.z + w * op.w; }
        Vec4Base<_vectors_flt_pt_type> inverse() const { return { notCloseToZero(x) ? 1. / x : x, notCloseToZero(y) ? 1. / y : y,
                                                                  notCloseToZero(z) ? 1. / z : z, notCloseToZero(w) ? 1. / w : w }; }
        Vec4Base<_vectors_flt_pt_type> normalize() const {
            _vectors_flt_pt_type norm = sqrt(x * x + y * y + z * z + w * w);
            norm = notCloseToZero(norm) ? 1. / norm : 1.;
            return { x * norm, y * norm, z * norm, w * norm };
        }


        template <class Y> void operator+=(const Vec4Base<Y>    &op) { x += op.x; y += op.y; z += op.z; w += op.w; }
        template <class Y> void operator-=(const Vec4Base<Y>    &op) { x -= op.x; y -= op.y; z -= op.z; w -= op.w; }
        template <class Y> void operator*=(const Vec4Base<Y>    &op) { x *= op.x; y *= op.y; z *= op.z; w *= op.w; }
        template <class Y> void operator/=(const Vec4Base<Y>    &op) { x /= op.x; y /= op.y; z /= op.z; w /= op.w; }
        template <class Y> void operator+=(const Vec4RefBase<Y> &op) { x += op.x; y += op.y; z += op.z; w += op.w; }
        template <class Y> void operator-=(const Vec4RefBase<Y> &op) { x -= op.x; y -= op.y; z -= op.z; w -= op.w; }
        template <class Y> void operator*=(const Vec4RefBase<Y> &op) { x *= op.x; y *= op.y; z *= op.z; w *= op.w; }
        template <class Y> void operator/=(const Vec4RefBase<Y> &op) { x /= op.x; y /= op.y; z /= op.z; w /= op.w; }
        template <class Y> void operator+=(const Y op) { x += op; y += op; z += op; w += op; }
        template <class Y> void operator-=(const Y op) { x -= op; y -= op; z -= op; w -= op; }
        template <class Y> void operator*=(const Y op) { x *= op; y *= op; z *= op; w *= op; }
        template <class Y> void operator/=(const Y op) { x /= op; y /= op; z /= op; w /= op; }

        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator+(const Vec4Base<Y> &op2) const
            { return {x + op2.x, y + op2.y, z + op2.z, w + op2.w}; }
        template <class Y> Vec4Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Vec4Base<Y> &op2) const
            { return {x - op2.x, y - op2.y, z - op2.z, w - op2.w}; }
        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator*(const Vec4Base<Y> &op2) const
            { return {x * op2.x, y * op2.y, z * op2.z, w * op2.w}; }
        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator/(const Vec4Base<Y> &op2) const
            { return {x / op2.x, y / op2.y, z / op2.z, w / op2.w}; }

        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator+(const Vec4RefBase<Y> &op2) const
            { return {x + op2.x, y + op2.y, z + op2.z, w + op2.w}; }
        template <class Y> Vec4Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Vec4RefBase<Y> &op2) const
            { return {x - op2.x, y - op2.y, z - op2.z, w - op2.w}; }
        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator*(const Vec4RefBase<Y> &op2) const
            { return {x * op2.x, y * op2.y, z * op2.z, w * op2.w}; }
        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator/(const Vec4RefBase<Y> &op2) const
            { return {x / op2.x, y / op2.y, z / op2.z, w / op2.w}; }

        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator+(const Y op2) const
            { return {x + op2, y + op2, z + op2, w + op2}; }
        template <class Y> Vec4Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Y op2) const
            { return {x - op2, y - op2, z - op2, w - op2}; }
        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator*(const Y op2) const
            { return {x * op2, y * op2, z * op2, w * op2}; }
        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator/(const Y op2) const
            { return {x / op2, y / op2, z / op2, w / op2}; }
        
        C &operator[](const unsigned index) { return *((C*)this + index); }
        const C &operator[](const unsigned index) const { return *((C*)this + index); }
        Vec2RefBase<C> operator[](const char (&attrs)[2 + 1])
            { C *ptr[4]{&w, &x, &y, &z}; return {*ptr[attrs[0] + 1 - 'x'], *ptr[attrs[1] + 1 - 'x']}; }
        Vec3RefBase<C> operator[](const char (&attrs)[3 + 1])
            { C *ptr[4]{&w, &x, &y, &z}; return {*ptr[attrs[0] + 1 - 'x'], *ptr[attrs[1] + 1 - 'x'], *ptr[attrs[2] + 1 - 'x']}; }
        Vec4RefBase<C> operator[](const char (&attrs)[4 + 1])
            { C *ptr[4]{&w, &x, &y, &z}; return {*ptr[attrs[0] + 1 - 'x'], *ptr[attrs[1] + 1 - 'x'], *ptr[attrs[2] + 1 - 'x'], *ptr[attrs[3] + 1 - 'x']}; }
        const Vec2RefBase<C> operator[](const char (&attrs)[2 + 1]) const
            { C *ptr[4]{&w, &x, &y, &z}; return {*ptr[attrs[0] + 1 - 'x'], *ptr[attrs[1] + 1 - 'x']}; }
        const Vec3RefBase<C> operator[](const char (&attrs)[3 + 1]) const
            { C *ptr[4]{&w, &x, &y, &z}; return {*ptr[attrs[0] + 1 - 'x'], *ptr[attrs[1] + 1 - 'x'], *ptr[attrs[2] + 1 - 'x']}; }
        const Vec4RefBase<C> operator[](const char (&attrs)[4 + 1]) const
            { C *ptr[4]{&w, &x, &y, &z}; return {*ptr[attrs[0] + 1 - 'x'], *ptr[attrs[1] + 1 - 'x'], *ptr[attrs[2] + 1 - 'x'], *ptr[attrs[3] + 1 - 'x']}; }
    };


    template <class C>
    struct Vec4RefBase{
        C &x, &y, &z, &w;

        Vec4RefBase(C &_x, C &_y, C &_z, C &_w) : x{_x}, y{_y}, z{_z}, w{_w} {}


        _vectors_flt_pt_type norm() const { return sqrt(x * x + y * y + z * z + w * w); }
        template <class Y> _vectors_flt_pt_type dot(const Vec4Base<Y> &op) const { return x * op.x + y * op.y + z * op.z + w * op.w; }
        Vec4Base<_vectors_flt_pt_type> inverse() const { return { notCloseToZero(x) ? 1. / x : x, notCloseToZero(y) ? 1. / y : y,
                                                                  notCloseToZero(z) ? 1. / z : z, notCloseToZero(w) ? 1. / w : w }; }
        Vec4Base<_vectors_flt_pt_type> normalize() const {
            _vectors_flt_pt_type norm = sqrt(x * x + y * y + z * z + w * w);
            norm = notCloseToZero(norm) ? 1. / norm : 1.;
            return { x * norm, y * norm, z * norm, w * norm };
        }


        const Vec4RefBase<C> operator=(const Vec4RefBase <C> &ref) { x = ref.x; y = ref.y; z = ref.z; w = ref.w; return *this; }
        template <class Y> const Vec4RefBase<C> operator=(const Vec4RefBase <Y> &ref) { x = ref.x; y = ref.y; z = ref.z; w = ref.w; return *this; }
        Vec4RefBase<C> &operator=(const Vec4Base <C> &ref) { x = ref.x; y = ref.y; z = ref.z; w = ref.w; return *this; }

        template <class Y> void operator+=(const Vec4Base<Y>    &op) { x += op.x; y += op.y; z += op.z; w += op.w; }
        template <class Y> void operator-=(const Vec4Base<Y>    &op) { x -= op.x; y -= op.y; z -= op.z; w -= op.w; }
        template <class Y> void operator*=(const Vec4Base<Y>    &op) { x *= op.x; y *= op.y; z *= op.z; w *= op.w; }
        template <class Y> void operator/=(const Vec4Base<Y>    &op) { x /= op.x; y /= op.y; z /= op.z; w /= op.w; }
        template <class Y> void operator+=(const Vec4RefBase<Y> &op) { x += op.x; y += op.y; z += op.z; w += op.w; }
        template <class Y> void operator-=(const Vec4RefBase<Y> &op) { x -= op.x; y -= op.y; z -= op.z; w -= op.w; }
        template <class Y> void operator*=(const Vec4RefBase<Y> &op) { x *= op.x; y *= op.y; z *= op.z; w *= op.w; }
        template <class Y> void operator/=(const Vec4RefBase<Y> &op) { x /= op.x; y /= op.y; z /= op.z; w /= op.w; }
        template <class Y> void operator+=(const Y op) { x += op; y += op; z += op; w += op; }
        template <class Y> void operator-=(const Y op) { x -= op; y -= op; z -= op; w -= op; }
        template <class Y> void operator*=(const Y op) { x *= op; y *= op; z *= op; w *= op; }
        template <class Y> void operator/=(const Y op) { x /= op; y /= op; z /= op; w /= op; }

        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator+(const Vec4Base<Y> &op2) const
            { return {x + op2.x, y + op2.y, z + op2.z, w + op2.w}; }
        template <class Y> Vec4Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value, // && (std::is_signed<C>::value || std::is_signed<Y>::value),
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Vec4Base<Y> &op2) const
            { return {x - op2.x, y - op2.y, z - op2.z, w - op2.w}; }
        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator*(const Vec4Base<Y> &op2) const
            { return {x * op2.x, y * op2.y, z * op2.z, w * op2.w}; }
        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator/(const Vec4Base<Y> &op2) const
            { return {x / op2.x, y / op2.y, z / op2.z, w / op2.w}; }

        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator+(const Vec4RefBase<Y> &op2) const
            { return {x + op2.x, y + op2.y, z + op2.z, w + op2.w}; }
        template <class Y> Vec4Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Vec4RefBase<Y> &op2) const
            { return {x - op2.x, y - op2.y, z - op2.z, w - op2.w}; }
        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator*(const Vec4RefBase<Y> &op2) const
            { return {x * op2.x, y * op2.y, z * op2.z, w * op2.w}; }
        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator/(const Vec4RefBase<Y> &op2) const
            { return {x / op2.x, y / op2.y, z / op2.z, w / op2.w}; }

        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator+(const Y op2) const
            { return {x + op2, y + op2, z + op2, w + op2}; }
        template <class Y> Vec4Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Y op2) const
            { return {x - op2, y - op2, z - op2, w - op2}; }
        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator*(const Y op2) const
            { return {x * op2, y * op2, z * op2, w * op2}; }
        template <class Y> Vec4Base<typename std::common_type<C, Y>::type> operator/(const Y op2) const
            { return {x / op2, y / op2, z / op2, w / op2}; }

        C &operator[](const unsigned index) { C *arr[4]{&x, &y, &z, &w}; return *arr[index]; }
        const C &operator[](const unsigned index) const { C *arr[4]{&x, &y, &z, &w}; return *arr[index]; }
        Vec2RefBase<C> operator[](const char (&attrs)[2 + 1])
            { C *ptr[4]{&w, &x, &y, &z}; return {*ptr[attrs[0] + 1 - 'x'], *ptr[attrs[1] + 1 - 'x']}; }
        Vec3RefBase<C> operator[](const char (&attrs)[3 + 1])
            { C *ptr[4]{&w, &x, &y, &z}; return {*ptr[attrs[0] + 1 - 'x'], *ptr[attrs[1] + 1 - 'x'], *ptr[attrs[2] + 1 - 'x']}; }
        Vec4RefBase<C> operator[](const char (&attrs)[4 + 1])
            { C *ptr[4]{&w, &x, &y, &z}; return {*ptr[attrs[0] + 1 - 'x'], *ptr[attrs[1] + 1 - 'x'], *ptr[attrs[2] + 1 - 'x'], *ptr[attrs[3] + 1 - 'x']}; }
        const Vec2RefBase<C> operator[](const char (&attrs)[2 + 1]) const
            { C *ptr[4]{&w, &x, &y, &z}; return {*ptr[attrs[0] + 1 - 'x'], *ptr[attrs[1] + 1 - 'x']}; }
        const Vec3RefBase<C> operator[](const char (&attrs)[3 + 1]) const
            { C *ptr[4]{&w, &x, &y, &z}; return {*ptr[attrs[0] + 1 - 'x'], *ptr[attrs[1] + 1 - 'x'], *ptr[attrs[2] + 1 - 'x']}; }
        const Vec4RefBase<C> operator[](const char (&attrs)[4 + 1]) const
            { C *ptr[4]{&w, &x, &y, &z}; return {*ptr[attrs[0] + 1 - 'x'], *ptr[attrs[1] + 1 - 'x'], *ptr[attrs[2] + 1 - 'x'], *ptr[attrs[3] + 1 - 'x']}; }
    };


    template <class C, class Y> Vec4Base<typename std::common_type<C, Y>::type> operator+(const Y op1, const Vec4Base<C> &op2)
        { return {op1 + op2.x, op1 + op2.y, op1 + op2.z}; }
    template <class C, class Y> Vec4Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                         typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                         typename std::common_type<C, Y>::type>::type> operator-(const Y op1, const Vec4Base<C> &op2)
        { return {op1 - op2.x, op1 - op2.y, op1 - op2.z}; }
    template <class C, class Y> Vec4Base<typename std::common_type<C, Y>::type> operator*(const Y op1, const Vec4Base<C> &op2)
        { return {op1 * op2.x, op1 * op2.y, op1 * op2.z}; }
    template <class C, class Y> Vec4Base<typename std::common_type<C, Y>::type> operator/(const Y op1, const Vec4Base<C> &op2)
        { return {op1 / op2.x, op1 / op2.y, op1 / op2.z}; }

    template <class C, class Y> Vec4Base<typename std::common_type<C, Y>::type> operator+(const Y op1, const Vec4RefBase<C> &op2)
        { return {op1 + op2.x, op1 + op2.y, op1 + op2.z}; }
    template <class C, class Y> Vec4Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                         typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                         typename std::common_type<C, Y>::type>::type> operator-(const Y op1, const Vec4RefBase<C> &op2)
        { return {op1 - op2.x, op1 - op2.y, op1 - op2.z}; }
    template <class C, class Y> Vec4Base<typename std::common_type<C, Y>::type> operator*(const Y op1, const Vec4RefBase<C> &op2)
        { return {op1 * op2.x, op1 * op2.y, op1 * op2.z}; }
    template <class C, class Y> Vec4Base<typename std::common_type<C, Y>::type> operator/(const Y op1, const Vec4RefBase<C> &op2)
        { return {op1 / op2.x, op1 / op2.y, op1 / op2.z}; }


    //  ============  Matrices  ============  \\

    template <class C>
    struct Mat3Base {
        C M[3][3]{ 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        Mat3Base() {}
        //Mat3Base(const Mat3Base<C> &mat) { memcpy((C*)(M[0]), (C*)(mat.M[0]), 9 * sizeof(C)); }
        template <class Y> Mat3Base(Mat3Base<Y> mat) : M{{ C((*mat.M)[0]), C((*mat.M)[1]), C((*mat.M)[2]) },
                                                         { C((*mat.M)[3]), C((*mat.M)[4]), C((*mat.M)[5]) },
                                                         { C((*mat.M)[6]), C((*mat.M)[7]), C((*mat.M)[8]) }} {}
        template <class Y> Mat3Base(const C *arr) : M{{ arr[0], arr[1], arr[2] },
                                                      { arr[3], arr[4], arr[5] },
                                                      { arr[6], arr[7], arr[8] }} {}
        template <class Y> Mat3Base(const Y &val) : M{{ C(val), C(val), C(val) },
                                                      { C(val), C(val), C(val) },
                                                      { C(val), C(val), C(val) }} {}
        //Mat3Base(C *arr) { memcpy((C*)(M[0]), arr, 9 * sizeof(C)); }
        template <class Y>
        Mat3Base(Y m0, Y m1, Y m2, Y m3, Y m4, Y m5, Y m6, Y m7, Y m8) :
            M{{ C(m0), C(m1), C(m2) }, { C(m3), C(m4), C(m5) }, { C(m6), C(m7), C(m8) }} {}
        template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8> //never use this
        Mat3Base(T0 m0, T1 m1, T2 m2, T3 m3, T4 m4, T5 m5, T6 m6, T7 m7, T8 m8) :
            M{{ C(m0), C(m1), C(m2) }, { C(m3), C(m4), C(m5) }, { C(m6), C(m7), C(m8) }} {}

        void transpose() { swap(M[0][1], M[1][0]); swap(M[0][2], M[2][0]); swap(M[1][2], M[2][1]); }
        Mat3Base <C> T() const { return { (*M)[0], (*M)[3], (*M)[6], (*M)[1], (*M)[4], (*M)[7], (*M)[2], (*M)[5], (*M)[8] }; }

        _vectors_flt_pt_type det() const { return (*M)[0] * ((*M)[4] * (*M)[8] - (*M)[5] * (*M)[7]) -
                                                  (*M)[1] * ((*M)[3] * (*M)[8] - (*M)[5] * (*M)[6]) +
                                                  (*M)[2] * ((*M)[3] * (*M)[7] - (*M)[4] * (*M)[6]); }
        
        bool invertible() const { return notCloseToZero(this->det()); }

        Mat3Base <C> Adj() const {
            return {(*M)[4] * (*M)[8] - (*M)[5] * (*M)[7], (*M)[2] * (*M)[7] - (*M)[1] * (*M)[8], (*M)[1] * (*M)[5] - (*M)[2] * (*M)[4],
                    (*M)[5] * (*M)[6] - (*M)[3] * (*M)[8], (*M)[0] * (*M)[8] - (*M)[2] * (*M)[6], (*M)[2] * (*M)[3] - (*M)[0] * (*M)[5],
                    (*M)[3] * (*M)[7] - (*M)[4] * (*M)[6], (*M)[1] * (*M)[6] - (*M)[0] * (*M)[7], (*M)[0] * (*M)[4] - (*M)[1] * (*M)[3]};
        }

        void inverse() {
            C tmp[3]{(*M)[4] * (*M)[8] - (*M)[5] * (*M)[7], (*M)[5] * (*M)[6] - (*M)[3] * (*M)[8], (*M)[3] * (*M)[7] - (*M)[4] * (*M)[6]};
            double determinant = (*M)[0] * tmp[0] + (*M)[1] * tmp[1] + (*M)[2] * tmp[2];

            *this = notCloseToZero(determinant) ?
                        Mat3Base<C>(Mat3Base<double>(tmp[0], (*M)[2] * (*M)[7] - (*M)[1] * (*M)[8], (*M)[1] * (*M)[5] - (*M)[2] * (*M)[4],
                             tmp[1], (*M)[0] * (*M)[8] - (*M)[2] * (*M)[6], (*M)[2] * (*M)[3] - (*M)[0] * (*M)[5],
                             tmp[2], (*M)[1] * (*M)[6] - (*M)[0] * (*M)[7], (*M)[0] * (*M)[4] - (*M)[1] * (*M)[3])
                              / determinant) : *this;
        }

        Mat3Base <C> Inv() const {
            C tmp[3]{(*M)[4] * (*M)[8] - (*M)[5] * (*M)[7], (*M)[5] * (*M)[6] - (*M)[3] * (*M)[8], (*M)[3] * (*M)[7] - (*M)[4] * (*M)[6]};
            double determinant = (*M)[0] * tmp[0] + (*M)[1] * tmp[1] + (*M)[2] * tmp[2];

            return notCloseToZero(determinant) ?
                        Mat3Base<C>(Mat3Base<double>(tmp[0], (*M)[2] * (*M)[7] - (*M)[1] * (*M)[8], (*M)[1] * (*M)[5] - (*M)[2] * (*M)[4],
                             tmp[1], (*M)[0] * (*M)[8] - (*M)[2] * (*M)[6], (*M)[2] * (*M)[3] - (*M)[0] * (*M)[5],
                             tmp[2], (*M)[1] * (*M)[6] - (*M)[0] * (*M)[7], (*M)[0] * (*M)[4] - (*M)[1] * (*M)[3])
                              / determinant) : *this;
        }


        template <class Y> Mat3Base<typename std::common_type<C, Y>::type> operator+(const Mat3Base<Y> &mat) const
            { return { (*M)[0] + (*mat.M)[0], (*M)[1] + (*mat.M)[1], (*M)[2] + (*mat.M)[2],
                       (*M)[3] + (*mat.M)[3], (*M)[4] + (*mat.M)[4], (*M)[5] + (*mat.M)[5],
                       (*M)[6] + (*mat.M)[6], (*M)[7] + (*mat.M)[7], (*M)[8] + (*mat.M)[8] }; }
        template <class Y> Mat3Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Mat3Base<Y> &mat) const
            { return { (*M)[0] - (*mat.M)[0], (*M)[1] - (*mat.M)[1], (*M)[2] - (*mat.M)[2],
                       (*M)[3] - (*mat.M)[3], (*M)[4] - (*mat.M)[4], (*M)[5] - (*mat.M)[5],
                       (*M)[6] - (*mat.M)[6], (*M)[7] - (*mat.M)[7], (*M)[8] - (*mat.M)[8] }; }
        template <class Y> Mat3Base<typename std::common_type<C, Y>::type> operator*(const Mat3Base<Y> &mat) const
            { return { (*M)[0] * (*mat.M)[0], (*M)[1] * (*mat.M)[1], (*M)[2] * (*mat.M)[2],
                       (*M)[3] * (*mat.M)[3], (*M)[4] * (*mat.M)[4], (*M)[5] * (*mat.M)[5],
                       (*M)[6] * (*mat.M)[6], (*M)[7] * (*mat.M)[7], (*M)[8] * (*mat.M)[8] }; }
        template <class Y> Mat3Base<typename std::common_type<C, Y>::type> operator/(const Mat3Base<Y> &mat) const
            { return { (*M)[0] / (*mat.M)[0], (*M)[1] / (*mat.M)[1], (*M)[2] / (*mat.M)[2],
                       (*M)[3] / (*mat.M)[3], (*M)[4] / (*mat.M)[4], (*M)[5] / (*mat.M)[5],
                       (*M)[6] / (*mat.M)[6], (*M)[7] / (*mat.M)[7], (*M)[8] / (*mat.M)[8] }; }

        template <class Y> void operator+=(const Mat3Base<Y> &mat) { for (int i = 0; i < 9; ++i) (*M)[i] += (*mat.M)[i]; }
        template <class Y> void operator-=(const Mat3Base<Y> &mat) { for (int i = 0; i < 9; ++i) (*M)[i] -= (*mat.M)[i]; }
        template <class Y> void operator*=(const Mat3Base<Y> &mat) { for (int i = 0; i < 9; ++i) (*M)[i] *= (*mat.M)[i]; }
        template <class Y> void operator/=(const Mat3Base<Y> &mat) { for (int i = 0; i < 9; ++i) (*M)[i] /= (*mat.M)[i]; }
        template <class Y> void operator+=(const Y val) { for (int i = 0; i < 9; ++i) (*M)[i] += val; }
        template <class Y> void operator-=(const Y val) { for (int i = 0; i < 9; ++i) (*M)[i] -= val; }
        template <class Y> void operator*=(const Y val) { for (int i = 0; i < 9; ++i) (*M)[i] *= val; }
        template <class Y> void operator/=(const Y val) { for (int i = 0; i < 9; ++i) (*M)[i] /= val; }

        template <class Y> Mat3Base<typename std::common_type<C, Y>::type> operator+(const Y val) const
            { return { (*M)[0] + val, (*M)[1] + val, (*M)[2] + val,
                       (*M)[3] + val, (*M)[4] + val, (*M)[5] + val,
                       (*M)[6] + val, (*M)[7] + val, (*M)[8] + val, }; }
        template <class Y> Mat3Base<typename std::conditional<std::is_integral<C>::value && std::is_integral<Y>::value,
                                    typename std::make_signed<typename std::common_type<C, Y>::type>::type,
                                    typename std::common_type<C, Y>::type>::type> operator-(const Y val) const
            { return { (*M)[0] - val, (*M)[1] - val, (*M)[2] - val,
                       (*M)[3] - val, (*M)[4] - val, (*M)[5] - val,
                       (*M)[6] - val, (*M)[7] - val, (*M)[8] - val, }; }
        template <class Y> Mat3Base<typename std::common_type<C, Y>::type> operator*(const Y val) const
            { return { (*M)[0] * val, (*M)[1] * val, (*M)[2] * val,
                       (*M)[3] * val, (*M)[4] * val, (*M)[5] * val,
                       (*M)[6] * val, (*M)[7] * val, (*M)[8] * val, }; }
        template <class Y> Mat3Base<typename std::common_type<C, Y>::type> operator/(const Y val) const
            { return { (*M)[0] / val, (*M)[1] / val, (*M)[2] / val,
                       (*M)[3] / val, (*M)[4] / val, (*M)[5] / val,
                       (*M)[6] / val, (*M)[7] / val, (*M)[8] / val, }; }

        template <class Y> Vec3Base<typename std::common_type<C, Y>::type> operator&(const Vec3Base<Y> &vec) const {
            return { vec.x * (*M)[0] + vec.y * (*M)[1] + vec.z * (*M)[2],
                     vec.x * (*M)[3] + vec.y * (*M)[4] + vec.z * (*M)[5],
                     vec.x * (*M)[6] + vec.y * (*M)[7] + vec.z * (*M)[8] };
        }
        template <class Y> Mat3Base<typename std::common_type<C, Y>::type> operator&(const Mat3Base<Y> &mat) const {
            return { (*mat.M)[0] * (*M)[0] + (*mat.M)[3] * (*M)[1] + (*mat.M)[6] * (*M)[2],
                     (*mat.M)[1] * (*M)[0] + (*mat.M)[4] * (*M)[1] + (*mat.M)[7] * (*M)[2],
                     (*mat.M)[2] * (*M)[0] + (*mat.M)[5] * (*M)[1] + (*mat.M)[8] * (*M)[2],
                     (*mat.M)[0] * (*M)[3] + (*mat.M)[3] * (*M)[4] + (*mat.M)[6] * (*M)[5],
                     (*mat.M)[1] * (*M)[3] + (*mat.M)[4] * (*M)[4] + (*mat.M)[7] * (*M)[5],
                     (*mat.M)[2] * (*M)[3] + (*mat.M)[5] * (*M)[4] + (*mat.M)[8] * (*M)[5],
                     (*mat.M)[0] * (*M)[6] + (*mat.M)[3] * (*M)[7] + (*mat.M)[6] * (*M)[8],
                     (*mat.M)[1] * (*M)[6] + (*mat.M)[4] * (*M)[7] + (*mat.M)[7] * (*M)[8],
                     (*mat.M)[2] * (*M)[6] + (*mat.M)[5] * (*M)[7] + (*mat.M)[8] * (*M)[8] };
        }
        template <class Y> void operator&=(const Mat3Base<Y> &mat) {
            *this = { (*mat.M)[0] * (*M)[0] + (*mat.M)[3] * (*M)[1] + (*mat.M)[6] * (*M)[2],
                      (*mat.M)[1] * (*M)[0] + (*mat.M)[4] * (*M)[1] + (*mat.M)[7] * (*M)[2],
                      (*mat.M)[2] * (*M)[0] + (*mat.M)[5] * (*M)[1] + (*mat.M)[8] * (*M)[2],
                      (*mat.M)[0] * (*M)[3] + (*mat.M)[3] * (*M)[4] + (*mat.M)[6] * (*M)[5],
                      (*mat.M)[1] * (*M)[3] + (*mat.M)[4] * (*M)[4] + (*mat.M)[7] * (*M)[5],
                      (*mat.M)[2] * (*M)[3] + (*mat.M)[5] * (*M)[4] + (*mat.M)[8] * (*M)[5],
                      (*mat.M)[0] * (*M)[6] + (*mat.M)[3] * (*M)[7] + (*mat.M)[6] * (*M)[8],
                      (*mat.M)[1] * (*M)[6] + (*mat.M)[4] * (*M)[7] + (*mat.M)[7] * (*M)[8],
                      (*mat.M)[2] * (*M)[6] + (*mat.M)[5] * (*M)[7] + (*mat.M)[8] * (*M)[8] };
        }

        Mat3RefBase<C> operator[](const MatPlaceholder sym) { return {&(*(M)[0]), &(*(M)[1]), &(*(M)[2])}; }
        const Mat3RefBase<C> operator[](const MatPlaceholder sym) const { return {&(*(M)[0]), &(*(M)[1]), &(*(M)[2])}; }
        Vec3RefBase<C> operator[](const std::size_t ind) { return { M[ind][0], M[ind][1], M[ind][2] }; }
        const Vec3RefBase<C> operator[](const std::size_t ind) const { return { M[ind][0], M[ind][1], M[ind][2] }; }
    };

    template <class C>
    struct Mat3RefBase {
        C *c[3];

        Mat3RefBase(C *c1, C *c2, C *c3) : c{c1, c2, c3} {}

        Vec3RefBase<C> operator[](const unsigned ind) { return { *c[ind], *(c[ind] + 3), *(c[ind] + 6) }; }
        const Vec3RefBase<C> operator[](const unsigned ind) const { return { *c[ind], *(c[ind] + 3), *(c[ind] + 6) }; }
    };

    const Mat3 Eye3{1.f, 0.f, 0.f,
                    0.f, 1.f, 0.f,
                    0.f, 0.f, 1.f};

    const Mat3 Ones3{1.f, 1.f, 1.f,
                     1.f, 1.f, 1.f,
                     1.f, 1.f, 1.f};

    const Mat3 Zeros3{0.f, 0.f, 0.f,
                      0.f, 0.f, 0.f,
                      0.f, 0.f, 0.f};


    //  ============  Quaternions  ============  \\

    struct Quaternion {
        float s = 0.f, i = 0.f, j = 0.f, k = 0.f;

        Quaternion() {}
        Quaternion(const Quaternion &_quat) : s{_quat.s}, i{_quat.i}, j{_quat.j}, k{_quat.k} {}
        template <class S, class I, class J, class K> Quaternion(S _s, I _i, J _j, K _k) : s{_s}, i{_i}, j{_j}, k{_k} {}
        template <class Y> Quaternion(const Vec3Base<Y> &_vec) : s{0.f}, i{_vec.x}, j{_vec.y}, k{_vec.z} {}
        template <class Y> Quaternion(float angle_deg, const Vec3Base<Y> &axis) {
            Quaternion _tmp = axis;   //TODO
        }

        Quaternion conjugate() const { return {   s,  -i,  -j,  -k }; }
        Quaternion scalar()    const { return {   s, 0.f, 0.f, 0.f }; }
        Quaternion vector()    const { return { 0.f,   i,   j,   k }; }
        void toConjugate() { i *= -1.f; j *= -1.f; k *= -1.f; }
        void toScalar()    { i  =  0.f; j  =  0.f; k  =  0.f; }
        void toVector()    { s  =  0.f; }

        float norm() const { return sqrt(s * s + i * i + j * j + k * k); }
        float dot(const Quaternion &op) const { return s * op.s + i * op.i + j * op.j + k * op.k; }
        Quaternion normalized() const {
            float norm = sqrt(s * s + i * i + j * j + k * k);
            norm = notCloseToZero(norm) ? 1.f / norm : 1.f;
            return { s * norm, i * norm, j * norm, k * norm };
        }
        void toNormalized() {
            float norm = sqrt(s * s + i * i + j * j + k * k);

            if (notCloseToZero(norm)) {
                norm = 1.f / norm;
                s *= norm;
                i *= norm;
                j *= norm;
                k *= norm;
            }
        }

        Quaternion inverted() const {
            float normSqr = s * s + i * i + j * j + k * k;
            normSqr = notCloseToZero(normSqr) ? 1.f / normSqr : 1.f;
            return { s * normSqr, -i * normSqr, -j * normSqr, -k * normSqr };
        }
        void toInverted() {
            float normSqr = s * s + i * i + j * j + k * k;

            if (notCloseToZero(normSqr)) {
                normSqr = 1.f / normSqr;
                s *=  normSqr;
                i *= -normSqr;
                j *= -normSqr;
                k *= -normSqr;
            }
        }

        Quaternion operator+(const Quaternion &op2) const { return { s + op2.s, i + op2.i, j + op2.j, k + op2.k }; }
        Quaternion operator-(const Quaternion &op2) const { return { s - op2.s, i - op2.i, j - op2.j, k - op2.k }; }
        Quaternion operator*(const Quaternion &op2) const {
            return { s * op2.s - i * op2.i - j * op2.j - k * op2.k,
                     s * op2.i + i * op2.s + j * op2.k - k * op2.j,
                     s * op2.j + j * op2.s + k * op2.i - i * op2.k,
                     s * op2.k + k * op2.s + i * op2.j - j * op2.i };
        }
        Quaternion operator/(const Quaternion &op2) const { return this->operator*(op2.inverted()); }
        template <class C> Quaternion operator*(const Vec3Base <C> &op2) { 
            return { -i * op2.x - j * op2.y - k * op2.z,
                      s * op2.x + j * op2.z - op2.y * k,
                      s * op2.y + k * op2.x - op2.z * i,
                      s * op2.z + i * op2.y - op2.i * j };
        }
    };
};

#endif