#ifndef SVKFW_COMPLEX_H
#define SVKFW_COMPLEX_H

#include "vectors.h"



namespace Simple {
    //  ============  Complex numbers  ============  \\

    struct Imaginary1 {}; // Imaginary1 END

    namespace ComplIm1 {
        static const Imaginary1 I, J;
    }; // ComplIm1 END

    struct Complex {
        float a = 0.f, b = 0.f;

        inline Complex conj() const { return { a, -b }; }
        inline   float abs2() const { return a * a + b * b; }

        void operator=(float _val) { a = _val; b = 0.f; }
        template <typename T>
        void operator=(const Vec2Base<T> &_vec) { a = _vec.x; b = _vec.y; }
        operator vec2() const { return { a, b }; }
    }; // Complex END


    //  ============  Complex operations  ============  \\

    Complex&operator++(Complex &_c)      { ++_c.a; return _c; }
    Complex&operator--(Complex &_c)      { --_c.a; return _c; }
    Complex&operator++(Complex &_c, int) { ++_c.b; return _c; }
    Complex&operator--(Complex &_c, int) { --_c.b; return _c; }

    Complex operator+(Complex _c) { return { Math::abs(_c.a), Math::abs(_c.b) }; }
    Complex operator-(Complex _c) { return { -_c.a, -_c.b }; }

    Complex operator+(Imaginary1) { return { 0.f, 1.f }; }
    Complex operator-(Imaginary1) { return { 0.f,-1.f }; }


    Complex operator+(Complex _c, float _val) { return { _c.a + _val, _c.b }; }
    Complex operator-(Complex _c, float _val) { return { _c.a - _val, _c.b }; }
    Complex operator*(Complex _c, float _val) { return { _c.a * _val, _c.b * _val }; }
    Complex operator/(Complex _c, float _val) { return { _c.a / _val, _c.b / _val }; }

    Complex operator+(Complex _c, Imaginary1) { return { _c.a, _c.b + 1.f }; }
    Complex operator-(Complex _c, Imaginary1) { return { _c.a, _c.b - 1.f }; }
    Complex operator*(Complex _c, Imaginary1) { return {-_c.b, _c.a }; }
    Complex operator/(Complex _c, Imaginary1) { return { _c.b,-_c.a }; }

    Complex operator+(Complex _c, Complex _i) { return { _c.a + _i.a, _c.b + _i.b }; }
    Complex operator-(Complex _c, Complex _i) { return { _c.a - _i.a, _c.b - _i.b }; }
    Complex operator*(Complex _c, Complex _i) { return { _c.a * _i.a - _c.b * _i.b, _c.a * _i.b + _c.b * _i.a }; }
    Complex operator/(Complex _c, Complex _i) { return Complex{ _c.a * _i.a + _c.b * _i.b, _c.b * _i.a - _c.a * _i.b } / _i.abs2(); }

    Complex operator+(Imaginary1, float _val) { return { (float)_val, 1.f }; }
    Complex operator-(Imaginary1, float _val) { return {-(float)_val, 1.f }; }
    Complex operator*(Imaginary1, float _val) { return { 0.f, (float)_val }; }
    Complex operator/(Imaginary1, float _val) { return { 0.f,  1.f / _val }; }

    Complex operator+(Imaginary1, Imaginary1) { return { 0.f, 2.f }; }
    Complex operator-(Imaginary1, Imaginary1) { return {}; }
    Complex operator*(Imaginary1, Imaginary1) { return {-1.f }; }
    Complex operator/(Imaginary1, Imaginary1) { return { 1.f }; }

    Complex operator+(Imaginary1, Complex _c) { return { _c.a, _c.b + 1.f }; }
    Complex operator-(Imaginary1, Complex _c) { return {-_c.a, 1.f - _c.b }; }
    Complex operator*(Imaginary1, Complex _c) { return {-_c.b, _c.a }; }
    Complex operator/(Imaginary1, Complex _c) { return Complex{_c.b, _c.a} / _c.abs2(); }

    Complex operator+(float _val, Imaginary1) { return { (float)_val, 1.f }; }
    Complex operator-(float _val, Imaginary1) { return { (float)_val,-1.f }; }
    Complex operator*(float _val, Imaginary1) { return { 0.f, (float)_val }; }
    Complex operator/(float _val, Imaginary1) { return { 0.f,-(float)_val }; }

    Complex operator+(float _val, Complex _c) { return { _val + _c.a, _c.b }; }
    Complex operator-(float _val, Complex _c) { return { _val - _c.a,-_c.b }; }
    Complex operator*(float _val, Complex _c) { return { _val * _c.a, _val * _c.b }; }
    Complex operator/(float _val, Complex _c) { return Complex{_val * _c.a, _val * -_c.b} / _c.abs2(); }


    void operator+=(Complex &_c, float _val) { _c.a += _val; }
    void operator-=(Complex &_c, float _val) { _c.a -= _val; }
    void operator*=(Complex &_c, float _val) { _c.a *= _val; _c.b *= _val; }
    void operator/=(Complex &_c, float _val) { _c.a /= _val; _c.b /= _val; }

    void operator+=(Complex &_c, Imaginary1) { _c.b += 1.f; }
    void operator-=(Complex &_c, Imaginary1) { _c.b -= 1.f; }
    void operator*=(Complex &_c, Imaginary1) { float _t = -_c.b; _c.b = _c.a; _c.a = _t; }
    void operator/=(Complex &_c, Imaginary1) { float _t = -_c.a; _c.a = _c.b; _c.b = _t; }

    void operator+=(Complex &_c, Complex _i) { _c.a += _i.a; _c.b += _i.b; }
    void operator-=(Complex &_c, Complex _i) { _c.a -= _i.a; _c.b -= _i.b; }
    void operator*=(Complex &_c, Complex _i)
        { float _tmp(_c.a); _c.a = _c.a * _i.a - _c.b * _i.b; _c.b = _tmp * _i.b + _c.b * _i.a; }
    void operator/=(Complex &_c, Complex _i)
        { float _tmp(_c.a); _c.a = _c.a * _i.a + _c.b * _i.b; _c.b = _c.b * _i.a - _tmp * _i.b; _c / _i.abs2(); }
}; //  Simple END

#endif