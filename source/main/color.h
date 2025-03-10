#ifndef SVKFW_COLOR_H
#define SVKFW_COLOR_H

// #define SVKFW_COL_FTOU 255u
// #define SVKFW_COL_UTOF 0.003921568859f     // 1 / 255

#include <type_traits>
#include <stdexcept>


namespace Simple {
    typedef unsigned char uchar;

//  ============  Color classes declaration  ============  \\

    template <typename T>
    struct  ColClampedType;
    template <typename T>
    struct  Color2Base;
    template <typename T>
    struct  Color3Base;
    template <typename T>
    struct  Color4Base;

    typedef ColClampedType<float> Color1f;
    typedef ColClampedType<uchar> Color1u;
    typedef Color2Base  <Color1f> Color2f;
    typedef Color2Base  <Color1u> Color2u;
    typedef Color3Base  <Color1f> Color3f;
    typedef Color3Base  <Color1u> Color3u;
    typedef Color4Base  <Color1f> Color4f;
    typedef Color4Base  <Color1u> Color4u;


//  ============  Utilities  ============  \\

    namespace UtilCol {
        const float MaxFloat =  1.f;
        const unsigned MaxUChar = 255u;

        const float UtoF = (1. / 255);
        const unsigned FtoU = 255u;


        // <base> types:       uchar, float
        // <base ops> types:   int(for uchar arithmetics), float
        // <base color> types: Color1f or Color1u - <base> types wrappers
        //                      - these and <base ops> types are the template class params for ColorNBase
        // <ops> types:        <base ops>   types or (ColorNBase<int> or ColorNBase<float>), N = 2,3,4
        // <color> types:      <base color> types or (ColorNu         or ColorNf),           N = 2,3,4

        template <typename T>
        struct _BaseToBaseOpT { using type = typename std::conditional<
                                                         std::is_same<T,           char>::value ||
                                                         std::is_same<T,          uchar>::value ||
                                                         std::is_same<T,          short>::value ||
                                                         std::is_same<T,   signed  char>::value ||
                                                         std::is_same<T, unsigned short>::value, int, T>::type; };
        template <typename T>
        struct _BaseToBaseOpT<ColClampedType<T>>
            { using type = typename _BaseToBaseOpT<T>::type; };

        template <typename T>
        // Returns <base ops> type for the <base>, <base ops>, <base color> type
        // (int for uchar/Color1u/int, float for float/Color1f)
        using BaseToBOpType = typename _BaseToBaseOpT<T>::type;


        template <typename T>
        struct _AnyToBaseOpT
            { using type = typename _BaseToBaseOpT<T>::type; };
        template <typename T>
        struct _AnyToBaseOpT<Color2Base<T>>
            { using type = typename _BaseToBaseOpT<T>::type; };
        template <typename T>
        struct _AnyToBaseOpT<Color3Base<T>>
            { using type = typename _BaseToBaseOpT<T>::type; };
        template <typename T>
        struct _AnyToBaseOpT<Color4Base<T>>
            { using type = typename _BaseToBaseOpT<T>::type; };

        template <typename T>
        // Returns <base ops> type for the <base>, <ops>, <color> type
        // (int for uchar-based, float for float-based)
        using AnyToBOpType = typename _AnyToBaseOpT<T>::type;


        template <typename T>
        struct Max { static const T val = 0; };
        template <> struct Max<  int>   { static const   int val = MaxUChar; };
        template <> struct Max<Color1u> { static const   int val = MaxUChar; };
        template <> struct Max<float>   { static const float val; };
        const float Max<  float>::val = MaxFloat;
        template <> struct Max<Color1f> { static const float val; };
        const float Max<Color1f>::val = MaxFloat;


        // The number of channels for <ops>, <color> type
        template <class C>
        struct Channels                    { static const int num = 0; };
        template <>
        struct Channels<float>             { static const int num = 1; };
        template <>
        struct Channels<int>               { static const int num = 1; };
        template <typename T>
        struct Channels<ColClampedType<T>> { static const int num = 1; };
        template <typename T>
        struct Channels<Color2Base<T>>     { static const int num = 2; };
        template <typename T>
        struct Channels<Color3Base<T>>     { static const int num = 3; };
        template <typename T>
        struct Channels<Color4Base<T>>     { static const int num = 4; };


        template <typename T>
        struct _OtherBase { using type = T; };
        template <> struct _OtherBase<float>   { using type =   int;   };
        template <> struct _OtherBase<  int>   { using type = float;   };
        template <> struct _OtherBase<Color1f> { using type = Color1u; };
        template <> struct _OtherBase<Color1u> { using type = Color1f; };
        template <typename T>
        struct _OtherBase<Color2Base<T>> { using type = Color2Base<typename _OtherBase<T>::type>; };
        template <typename T>
        struct _OtherBase<Color3Base<T>> { using type = Color3Base<typename _OtherBase<T>::type>; };
        template <typename T>
        struct _OtherBase<Color4Base<T>> { using type = Color4Base<typename _OtherBase<T>::type>; };
        template <typename T>
        // Switches between uchar-based and corresponding float-based <ops>, <color> types
        // (ColorNf <-> ColorNu, N=1,2,3,4) (float <-> int)
        using OtherBType = typename _OtherBase<T>::type;


        template <typename T>
        struct _BaseType { using type = T; };
        template <typename T> struct _BaseType<ColClampedType<T>> { using type = T; };
        template <typename T> struct _BaseType<Color2Base<T>>     { using type = T; };
        template <typename T> struct _BaseType<Color3Base<T>>     { using type = T; };
        template <typename T> struct _BaseType<Color4Base<T>>     { using type = T; };
        template <typename T> struct _BaseType<Color2Base<ColClampedType<T>>> { using type = T; };
        template <typename T> struct _BaseType<Color3Base<ColClampedType<T>>> { using type = T; };
        template <typename T> struct _BaseType<Color4Base<ColClampedType<T>>> { using type = T; };
        template <typename T>
        // For <ops>/<color> type returns its <base ops>/<base> type
        // (uchar, int, float) - to use with <type_traits>
        using BaseType = typename _BaseType<T>::type;

        template <typename T>
        struct _BaseColor { using type = T; };
        template <typename T> struct _BaseColor<Color2Base<T>> { using type = T; };
        template <typename T> struct _BaseColor<Color3Base<T>> { using type = T; };
        template <typename T> struct _BaseColor<Color4Base<T>> { using type = T; };
        template <typename T>
        // For <ops>/<color> type returns its <base ops>/<base color> type
        // (int, float, Color1f, Color1u) - used for ColorNBase<T> -> T[N] conversion
        using BaseColorType = typename _BaseColor<T>::type;


        template <bool FromFlt, bool ToFlt>
        struct _ValConv { static const int coef = 1; };
        template <>
        struct _ValConv<true, false> { static const   int coef = FtoU; };
        template <>
        struct _ValConv<false, true> { static const float coef; };
        const float _ValConv<false, true>::coef = UtoF;

        template <typename T1, typename T2>
        // coefficient for color conversion (uchar-based <-> float-based)
        // takes two <ops>/<color> types, finds <base ops>/<base> types
        // (int, uchar, float) and checks using is_floating_point
        using ValConv = _ValConv<std::is_floating_point<BaseType<T1>>::value,
                                 std::is_floating_point<BaseType<T2>>::value>;


        template <typename T, int Ch>
        struct _BaseChanColS {};

        template <typename T>
        struct _BaseChanColS<T, 0> { using type = T; };
        template <typename T>
        struct _BaseChanColS<T, 1> { using type = T; };
        template <typename T>
        struct _BaseChanColS<T, 2> { using type = Color2Base<T>; };
        template <typename T>
        struct _BaseChanColS<T, 3> { using type = Color3Base<T>; };
        template <typename T>
        struct _BaseChanColS<T, 4> { using type = Color4Base<T>; };

        template <typename T, int Ch>
        // Returns <ops>, <color> type for <base ops>, <base color> type and a number of channels
        // used for T[N] -> ColorNBase<T> conversion
        using BaseChColorT = typename _BaseChanColS<T, Ch>::type;


        float clamp(float _v) {
            // if (_v <= 0.f || std::isnan(_v)) return 0.f; // from <cmath> - removed for debugging
            if (_v <= 0.f) return 0.f;
            if (_v >= MaxFloat) return MaxFloat;
            return _v;
        }
        uchar clamp(int _v) {
            if (_v <= 0) return 0;
            if (_v >= MaxUChar) return MaxUChar;
            return _v;
        }
        uchar clamp(unsigned _v) {
            if (_v >= MaxUChar) return MaxUChar;
            return _v;
        }
        inline uchar clamp(uchar _v) {
            return _v;
        }
    };



//  =====================================  \\
                Color classes
//  =====================================  \\

//  ============  Color base class  ============  \\

// Base class for Color1-4
    struct ColorBase {};


// A namespace for color-related functions
    namespace Color {
          float convert(    int _v);
            int convert(  float _v);
        Color1f convert(Color1u _v);
        Color1u convert(Color1f _v);
        template <class T>
        Color2Base<UtilCol::OtherBType<T>> convert(Color2Base<T> _v);
        template <class T>
        Color3Base<UtilCol::OtherBType<T>> convert(Color3Base<T> _v);
        template <class T>
        Color4Base<UtilCol::OtherBType<T>> convert(Color4Base<T> _v);
    };


//  ============  Derived Color1-4 classes  ============  \\

// Color1 - clamped thin wrapper
    template <typename T>
    class ColClampedType : ColorBase {
        T _c = 0u;

    public:
        ColClampedType() {}
        ColClampedType(UtilCol::AnyToBOpType<T> _v) : _c{UtilCol::clamp(_v)} {}
        ColClampedType(const ColClampedType<T> &_f) : _c{_f._c} {}

        operator T() const { return _c; }


        ColClampedType<T>& operator=(ColClampedType<T> _u) {
            _c = _u._c;
            return *this;
        }

        ColClampedType<T>& operator+=(UtilCol::AnyToBOpType<T> _i) {
            _c = UtilCol::clamp(_c + _i);
            return *this;
        }

        ColClampedType<T>& operator-=(UtilCol::AnyToBOpType<T> _i) {
            _c = UtilCol::clamp(_c - _i);
            return *this;
        }

        ColClampedType<T>& operator*=(UtilCol::AnyToBOpType<T> _i) {
            _c = UtilCol::clamp(_c * _i);
            return *this;
        }

        ColClampedType<T>& operator/=(UtilCol::AnyToBOpType<T> _i) {
            _c = UtilCol::clamp(_c / _i);
            return *this;
        }
    }; // ColClampedType END


// Color2
    template <typename T>
    struct Color2Base : ColorBase {
        T g, a;

        Color2Base() : g{0}, a{0} {}
        Color2Base(T _g, T _a = UtilCol::Max<T>::val) : g{_g}, a{_a} {}
        template <typename Y>
        Color2Base(const Color2Base<Y> &_c) : g{_c.g * UtilCol::ValConv<Y, T>::coef},
                                              a{_c.a * UtilCol::ValConv<Y, T>::coef} {}


        T& operator[](unsigned _ind) {
            if (_ind > 1u) throw std::out_of_range("Error: Color2[] - index out of range");
            return _ind == 0u ? g : a;
        }

        T operator[](unsigned _ind) const {
            if (_ind > 1u) throw std::out_of_range("Error: Color2[] const - index out of range");
            return _ind == 0u ? g : a;
        }

        Color2Base<T>& operator =(const Color2Base<T> &_c) {
            g  = _c.g; a  = _c.a;
            return *this;
        }

        Color2Base<T>& operator+=(const Color2Base<T> &_c) {
            g += _c.g; a += _c.a;
            return *this;
        }

        Color2Base<T>& operator-=(const Color2Base<T> &_c) {
            g -= _c.g; a -= _c.a;
            return *this;
        }

        Color2Base<T>& operator*=(const Color2Base<T> &_c) {
            g *= _c.g; a *= _c.a;
            return *this;
        }

        Color2Base<T>& operator/=(const Color2Base<T> &_c) {
            g /= _c.g; a /= _c.a;
            return *this;
        }
    };


// Color3
    template <typename T>
    struct Color3Base : ColorBase {
        union { T r, x, h,  i,    L; };
        union { T g, y, s, Ct, M, A; };
        union { T b, z, v, Cp, S, B; };

        Color3Base() : r{0}, g{0}, b{0} {}
        Color3Base(T _v) : r{_v}, g{_v}, b{_v} {}
        Color3Base(T _x, T _y, T _z) : r{_x}, g{_y}, b{_z} {}
        Color3Base(const Color3Base<T> &_c) : r{_c.r}, g{_c.g}, b{_c.b} {}
        template <typename Y>
        Color3Base(const Color3Base<Y> &_c) : r{_c.r * UtilCol::ValConv<Y, T>::coef},
                                              g{_c.g * UtilCol::ValConv<Y, T>::coef},
                                              b{_c.b * UtilCol::ValConv<Y, T>::coef} {}


        Color4Base<T> col4(T _a = UtilCol::Max<T>::val) {
            return { r, g, b, _a };
        }


        T& operator[](unsigned _ind) {
            if (_ind > 2u) throw std::out_of_range("Error: Color3[] - index out of range");
            switch (_ind) {
                case 0: return r;
                case 1: return g;
                case 2: return b;
            }
        }

        T operator[](unsigned _ind) const {
            if (_ind > 2u) throw std::out_of_range("Error: Color3[] - index out of range");
            switch (_ind) {
                case 0: return r;
                case 1: return g;
                case 2: return b;
            }
        }

        Color3Base<T>& operator =(const Color3Base<T> &_c) {
            r  = _c.r; g  = _c.g; b  = _c.b;
            return *this;
        }

        Color3Base<T>& operator+=(const Color3Base<T> &_c) {
            r += _c.r; g += _c.g; b += _c.b;
            return *this;
        }

        Color3Base<T>& operator-=(const Color3Base<T> &_c) {
            r -= _c.r; g -= _c.g; b -= _c.b;
            return *this;
        }

        Color3Base<T>& operator*=(const Color3Base<T> &_c) {
            r *= _c.r; g *= _c.g; b *= _c.b;
            return *this;
        }

        Color3Base<T>& operator/=(const Color3Base<T> &_c) {
            r /= _c.r; g /= _c.g; b /= _c.b;
            return *this;
        }
    };



    template <typename T>
    struct Color4Base : ColorBase {
        union { T r, x, h,  i,    L; };
        union { T g, y, s, Ct, M, A; };
        union { T b, z, v, Cp, S, B; };
        T a;

        Color4Base() : r{0}, g{0}, b{0}, a{0} {}
        Color4Base(T _v, T _a = UtilCol::Max<T>::val) : r{_v}, g{_v}, b{_v}, a{_a} {}
        Color4Base(T _x, T _y, T _z, T _a = UtilCol::Max<T>::val) : r{_x}, g{_y}, b{_z}, a{_a} {}
        Color4Base(const Color4Base<T> &_c) : r{_c.r}, g{_c.g}, b{_c.b}, a{_c.a} {}
        template <typename Y>
        Color4Base(const Color3Base<Y> &_c, T _a = UtilCol::Max<T>::val) :
                                              r{_c.r * UtilCol::ValConv<Y, T>::coef},
                                              g{_c.g * UtilCol::ValConv<Y, T>::coef},
                                              b{_c.b * UtilCol::ValConv<Y, T>::coef},
                                              a{_a} {}
        template <typename Y>
        Color4Base(const Color4Base<Y> &_c) : r{_c.r * UtilCol::ValConv<Y, T>::coef},
                                              g{_c.g * UtilCol::ValConv<Y, T>::coef},
                                              b{_c.b * UtilCol::ValConv<Y, T>::coef},
                                              a{_c.a * UtilCol::ValConv<Y, T>::coef} {}


        Color3Base<T> col() const {
            return { r, g, b };
        }


        T& operator[](unsigned _ind) {
            if (_ind > 3u) throw std::out_of_range("Error: Color4[] - index out of range");
            switch (_ind) {
                case 0: return r;
                case 1: return g;
                case 2: return b;
                case 3: return a;
            }
        }

        T operator[](unsigned _ind) const {
            if (_ind > 3u) throw std::out_of_range("Error: Color4[] - index out of range");
            switch (_ind) {
                case 0: return r;
                case 1: return g;
                case 2: return b;
                case 3: return a;
            }
        }

        Color4Base<T>& operator =(const Color4Base<T> &_c) {
            r  = _c.r; g  = _c.g; b  = _c.b; a  = _c.a;
            return *this;
        }

        Color4Base<T>& operator+=(const Color4Base<T> &_c) {
            r += _c.r; g += _c.g; b += _c.b; a += _c.a;
            return *this;
        }

        Color4Base<T>& operator-=(const Color4Base<T> &_c) {
            r -= _c.r; g -= _c.g; b -= _c.b; a -= _c.a;
            return *this;
        }

        Color4Base<T>& operator*=(const Color4Base<T> &_c) {
            r *= _c.r; g *= _c.g; b *= _c.b; a *= _c.a;
            return *this;
        }

        Color4Base<T>& operator/=(const Color4Base<T> &_c) {
            r /= _c.r; g /= _c.g; b /= _c.b; a /= _c.a;
            return *this;
        }
    };


//  ============  Color functions definition  ============  \\

    float   Color::convert(  int _v)   { return _v * UtilCol::UtoF; }
      int   Color::convert(float _v)   { return _v * UtilCol::FtoU; }
    Color1f Color::convert(Color1u _v) { return _v * UtilCol::UtoF; }
    Color1u Color::convert(Color1f _v) { return _v * UtilCol::FtoU; }
    template <class T>
    Color2Base<UtilCol::OtherBType<T>> Color::convert(Color2Base<T> _v)
        { return { convert(_v.g), convert(_v.a) }; }
    template <class T>
    Color3Base<UtilCol::OtherBType<T>> Color::convert(Color3Base<T> _v)
        { return { convert(_v.r), convert(_v.g), convert(_v.b) }; }
    template <class T>
    Color4Base<UtilCol::OtherBType<T>> Color::convert(Color4Base<T> _v)
        { return { convert(_v.r), convert(_v.g), convert(_v.b), convert(_v.a) }; }


//  ========================================  \\
                Color operations
//  ========================================  \\

//  ============  Color-scalar / Scalar-color operations  ============  \\

// Color2
    template <typename T>
    Color2Base<UtilCol::BaseToBOpType<T>> operator+(const Color2Base<T> &_c, UtilCol::BaseToBOpType<T> _v)
            { return { _c.g + _v, _c.a + _v }; }
    template <typename T>
    Color2Base<UtilCol::BaseToBOpType<T>> operator-(const Color2Base<T> &_c, UtilCol::BaseToBOpType<T> _v)
            { return { _c.g - _v, _c.a - _v }; }
    template <typename T>
    Color2Base<UtilCol::BaseToBOpType<T>> operator*(const Color2Base<T> &_c, float _v)
            { return { _c.g * _v, _c.a * _v }; }
    template <typename T>
    Color2Base<UtilCol::BaseToBOpType<T>> operator/(const Color2Base<T> &_c, float _v)
            { return { _c.g / _v, _c.a / _v }; }

    template <typename T>
    Color2Base<UtilCol::BaseToBOpType<T>> operator+(UtilCol::BaseToBOpType<T> _v, const Color2Base<T> &_c)
            { return { _v + _c.g, _v + _c.a }; }
    template <typename T>
    Color2Base<UtilCol::BaseToBOpType<T>> operator-(UtilCol::BaseToBOpType<T> _v, const Color2Base<T> &_c)
            { return { _v - _c.g, _v - _c.a }; }
    template <typename T>
    Color2Base<UtilCol::BaseToBOpType<T>> operator*(float _v, const Color2Base<T> &_c)
            { return { _v * _c.g, _v * _c.a }; }
    template <typename T>
    Color2Base<UtilCol::BaseToBOpType<T>> operator/(float _v, const Color2Base<T> &_c)
            { return { _v / _c.g, _v / _c.a }; }


// Color3
    template <typename T>
    Color3Base<UtilCol::BaseToBOpType<T>> operator+(const Color3Base<T> &_c, UtilCol::BaseToBOpType<T> _v)
            { return { _c.r + _v, _c.g + _v, _c.b + _v }; }
    template <typename T>
    Color3Base<UtilCol::BaseToBOpType<T>> operator-(const Color3Base<T> &_c, UtilCol::BaseToBOpType<T> _v)
            { return { _c.r - _v, _c.g - _v, _c.b - _v }; }
    template <typename T>
    Color3Base<UtilCol::BaseToBOpType<T>> operator*(const Color3Base<T> &_c, float _v)
            { return { _c.r * _v, _c.g * _v, _c.b * _v }; }
    template <typename T>
    Color3Base<UtilCol::BaseToBOpType<T>> operator/(const Color3Base<T> &_c, float _v)
            { return { _c.r / _v, _c.g / _v, _c.b / _v }; }

    template <typename T>
    Color3Base<UtilCol::BaseToBOpType<T>> operator+(UtilCol::BaseToBOpType<T> _v, const Color3Base<T> &_c)
            { return { _v + _c.r, _v + _c.g, _v + _c.b }; }
    template <typename T>
    Color3Base<UtilCol::BaseToBOpType<T>> operator-(UtilCol::BaseToBOpType<T> _v, const Color3Base<T> &_c)
            { return { _v - _c.r, _v - _c.g, _v - _c.b }; }
    template <typename T>
    Color3Base<UtilCol::BaseToBOpType<T>> operator*(float _v, const Color3Base<T> &_c)
            { return { _v * _c.r, _v * _c.g, _v * _c.b }; }
    template <typename T>
    Color3Base<UtilCol::BaseToBOpType<T>> operator/(float _v, const Color3Base<T> &_c)
            { return { _v / _c.r, _v / _c.g, _v / _c.b }; }


// Color4
    template <typename T>
    Color4Base<UtilCol::BaseToBOpType<T>> operator+(const Color4Base<T> &_c, UtilCol::BaseToBOpType<T> _v)
            { return { _c.r + _v, _c.g + _v, _c.b + _v, _c.a + _v }; }
    template <typename T>
    Color4Base<UtilCol::BaseToBOpType<T>> operator-(const Color4Base<T> &_c, UtilCol::BaseToBOpType<T> _v)
            { return { _c.r - _v, _c.g - _v, _c.b - _v, _c.a - _v }; }
    template <typename T>
    Color4Base<UtilCol::BaseToBOpType<T>> operator*(const Color4Base<T> &_c, float _v)
            { return { _c.r * _v, _c.g * _v, _c.b * _v, _c.a * _v }; }
    template <typename T>
    Color4Base<UtilCol::BaseToBOpType<T>> operator/(const Color4Base<T> &_c, float _v)
            { return { _c.r / _v, _c.g / _v, _c.b / _v, _c.a / _v }; }

    template <typename T>
    Color4Base<UtilCol::BaseToBOpType<T>> operator+(UtilCol::BaseToBOpType<T> _v, const Color4Base<T> &_c)
            { return { _v + _c.r, _v + _c.g, _v + _c.b, _v + _c.a }; }
    template <typename T>
    Color4Base<UtilCol::BaseToBOpType<T>> operator-(UtilCol::BaseToBOpType<T> _v, const Color4Base<T> &_c)
            { return { _v - _c.r, _v - _c.g, _v - _c.b, _v - _c.a }; }
    template <typename T>
    Color4Base<UtilCol::BaseToBOpType<T>> operator*(float _v, const Color4Base<T> &_c)
            { return { _v * _c.r, _v * _c.g, _v * _c.b, _v * _c.a }; }
    template <typename T>
    Color4Base<UtilCol::BaseToBOpType<T>> operator/(float _v, const Color4Base<T> &_c)
            { return { _v / _c.r, _v / _c.g, _v / _c.b, _v / _c.a }; }


//  ============  Color-color operations  ============  \\

// Color2
    template <typename T>
    Color2Base<UtilCol::BaseToBOpType<T>> operator+(const Color2Base<T> &_c1, const Color2Base<T> &_c2)
            { return { _c1.g + _c2.g, _c1.a + _c2.a }; }
    template <typename T>
    Color2Base<UtilCol::BaseToBOpType<T>> operator-(const Color2Base<T> &_c1, const Color2Base<T> &_c2)
            { return { _c1.g - _c2.g, _c1.a - _c2.a }; }
    template <typename T>
    Color2Base<UtilCol::BaseToBOpType<T>> operator*(const Color2Base<T> &_c1, const Color2Base<T> &_c2)
            { return { _c1.g * _c2.g, _c1.a * _c2.a }; }
    template <typename T>
    Color2Base<UtilCol::BaseToBOpType<T>> operator/(const Color2Base<T> &_c1, const Color2Base<T> &_c2)
            { return { _c1.g / _c2.g, _c1.a / _c2.a }; }


// Color3
    template <typename T>
    Color3Base<UtilCol::BaseToBOpType<T>> operator+(const Color3Base<T> &_c1, const Color3Base<T> &_c2)
            { return { _c1.r + _c2.r, _c1.g + _c2.g, _c1.b + _c2.b }; }
    template <typename T>
    Color3Base<UtilCol::BaseToBOpType<T>> operator-(const Color3Base<T> &_c1, const Color3Base<T> &_c2)
            { return { _c1.r - _c2.r, _c1.g - _c2.g, _c1.b - _c2.b }; }
    template <typename T>
    Color3Base<UtilCol::BaseToBOpType<T>> operator*(const Color3Base<T> &_c1, const Color3Base<T> &_c2)
            { return { _c1.r * _c2.r, _c1.g * _c2.g, _c1.b * _c2.b }; }
    template <typename T>
    Color3Base<UtilCol::BaseToBOpType<T>> operator/(const Color3Base<T> &_c1, const Color3Base<T> &_c2)
            { return { _c1.r / _c2.r, _c1.g / _c2.g, _c1.b / _c2.b }; }


// Color4
    template <typename T>
    Color4Base<UtilCol::BaseToBOpType<T>> operator+(const Color4Base<T> &_c1, const Color4Base<T> &_c2)
            { return { _c1.r + _c2.r, _c1.g + _c2.g, _c1.b + _c2.b, _c1.a + _c2.a }; }
    template <typename T>
    Color4Base<UtilCol::BaseToBOpType<T>> operator-(const Color4Base<T> &_c1, const Color4Base<T> &_c2)
            { return { _c1.r - _c2.r, _c1.g - _c2.g, _c1.b - _c2.b, _c1.a - _c2.a }; }
    template <typename T>
    Color4Base<UtilCol::BaseToBOpType<T>> operator*(const Color4Base<T> &_c1, const Color4Base<T> &_c2)
            { return { _c1.r * _c2.r, _c1.g * _c2.g, _c1.b * _c2.b, _c1.a * _c2.a }; }
    template <typename T>
    Color4Base<UtilCol::BaseToBOpType<T>> operator/(const Color4Base<T> &_c1, const Color4Base<T> &_c2)
            { return { _c1.r / _c2.r, _c1.g / _c2.g, _c1.b / _c2.b, _c1.a / _c2.a }; }


//  ============  Porter-Duff blends 4-4, 4-3, 3-4  ============  \\

    namespace Color {
        template <typename T>
        Color4Base<T> PorterDuffBlendOver(const Color4Base<T> &_c1, const Color4Base<T> &_c2)
            { return _c1 + (UtilCol::Max<T>::val - _c1.a) * UtilCol::ValConv<T, float>::coef * _c2; }
        template <typename T>
        Color4Base<T> PorterDuffBlendIn  (const Color4Base<T> &_c1, const Color4Base<T> &_c2)
            { return _c2.a * UtilCol::ValConv<T, float>::coef * _c1; }
        template <typename T>
        Color4Base<T> PorterDuffBlendOut (const Color4Base<T> &_c1, const Color4Base<T> &_c2)
            { return (UtilCol::Max<T>::val - _c2.a) * UtilCol::ValConv<T, float>::coef * _c1; }
        template <typename T>
        Color4Base<T> PorterDuffBlendAtop(const Color4Base<T> &_c1, const Color4Base<T> &_c2)
            { return                          _c2.a  * UtilCol::ValConv<T, float>::coef * _c1 +
                    (UtilCol::Max<T>::val - _c1.a) * UtilCol::ValConv<T, float>::coef * _c2; }
        template <typename T>
        Color4Base<T> PorterDuffBlendXor (const Color4Base<T> &_c1, const Color4Base<T> &_c2)
            { return (UtilCol::Max<T>::val - _c2.a) * UtilCol::ValConv<T, float>::coef * _c1 +
                     (UtilCol::Max<T>::val - _c1.a) * UtilCol::ValConv<T, float>::coef * _c2; }

// Do I even need these?
        template <typename T>
        Color4Base<T> PorterDuffBlendOver(const Color4Base<T> &_c1, const Color3Base<T> &_c2)
            { return {_c1.r + (UtilCol::Max<T>::val - _c1.a) * UtilCol::ValConv<T, float>::coef * _c2.r,
                      _c1.g + (UtilCol::Max<T>::val - _c1.a) * UtilCol::ValConv<T, float>::coef * _c2.g,
                      _c1.b + (UtilCol::Max<T>::val - _c1.a) * UtilCol::ValConv<T, float>::coef * _c2.b,
                      UtilCol::Max<T>::val}; }
        template <typename T> inline
        Color4Base<T> PorterDuffBlendIn  (const Color4Base<T> &_c1, const Color3Base<T> &_c2)
            { return _c1; }
        template <typename T> inline
        Color4Base<T> PorterDuffBlendOut (const Color4Base<T> &_c1, const Color3Base<T> &_c2)
            { return {}; }
        template <typename T>
        Color4Base<T> PorterDuffBlendAtop(const Color4Base<T> &_c1, const Color3Base<T> &_c2)
            { return {_c1.r + (UtilCol::Max<T>::val - _c1.a) * UtilCol::ValConv<T, float>::coef * _c2.r,
                      _c1.g + (UtilCol::Max<T>::val - _c1.a) * UtilCol::ValConv<T, float>::coef * _c2.g,
                      _c1.b + (UtilCol::Max<T>::val - _c1.a) * UtilCol::ValConv<T, float>::coef * _c2.b,
                      UtilCol::Max<T>::val}; }
        template <typename T>
        Color4Base<T> PorterDuffBlendXor (const Color4Base<T> &_c1, const Color3Base<T> &_c2)
            { return {(UtilCol::Max<T>::val - _c1.a) * UtilCol::ValConv<T, float>::coef * _c2.r,
                      (UtilCol::Max<T>::val - _c1.a) * UtilCol::ValConv<T, float>::coef * _c2.g,
                      (UtilCol::Max<T>::val - _c1.a) * UtilCol::ValConv<T, float>::coef * _c2.b,
                       UtilCol::Max<T>::val - _c1.a}; }

        template <typename T> inline
        Color4Base<T> PorterDuffBlendOver(const Color3Base<T> &_c1, const Color4Base<T> &_c2)
            { return _c1; }
        template <typename T>
        Color4Base<T> PorterDuffBlendIn  (const Color3Base<T> &_c1, const Color4Base<T> &_c2)
            { return {_c2.a * UtilCol::ValConv<T, float>::coef * _c1.r,
                      _c2.a * UtilCol::ValConv<T, float>::coef * _c1.g,
                      _c2.a * UtilCol::ValConv<T, float>::coef * _c1.b,
                      _c2.a}; }
        template <typename T>
        Color4Base<T> PorterDuffBlendOut (const Color3Base<T> &_c1, const Color4Base<T> &_c2)
            { return {(UtilCol::Max<T>::val - _c2.a) * UtilCol::ValConv<T, float>::coef * _c1.r,
                      (UtilCol::Max<T>::val - _c2.a) * UtilCol::ValConv<T, float>::coef * _c1.g,
                      (UtilCol::Max<T>::val - _c2.a) * UtilCol::ValConv<T, float>::coef * _c1.b,
                       UtilCol::Max<T>::val - _c2.a}; }
        template <typename T>
        Color4Base<T> PorterDuffBlendAtop(const Color3Base<T> &_c1, const Color4Base<T> &_c2)
            { return {_c2.a * UtilCol::ValConv<T, float>::coef * _c1.r,
                      _c2.a * UtilCol::ValConv<T, float>::coef * _c1.g,
                      _c2.a * UtilCol::ValConv<T, float>::coef * _c1.b,
                      _c2.a}; }
        template <typename T>
        Color4Base<T> PorterDuffBlendXor (const Color3Base<T> &_c1, const Color4Base<T> &_c2)
            { return {(UtilCol::Max<T>::val - _c2.a) * UtilCol::ValConv<T, float>::coef * _c1.r,
                      (UtilCol::Max<T>::val - _c2.a) * UtilCol::ValConv<T, float>::coef * _c1.g,
                      (UtilCol::Max<T>::val - _c2.a) * UtilCol::ValConv<T, float>::coef * _c1.b,
                       UtilCol::Max<T>::val - _c2.a}; }
    };
};

#endif