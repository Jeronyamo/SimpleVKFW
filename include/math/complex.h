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

        // Note: in C++11 a struct with default member initializers isn't an aggregate, so constructors are required for '{a, b}'
        Complex();
        Complex(float _a, float _b = 0.f);

        Complex conj() const;
        float abs2() const;

        void operator=(float _val);
        template <typename T>
        void operator=(const Vec2Base<T> &_vec) { a = _vec.x; b = _vec.y; }
        operator vec2() const;
    }; // Complex END


    //  ============  Complex operations  ============  \\

    Complex&operator++(Complex &_c);
    Complex&operator--(Complex &_c);
    Complex&operator++(Complex &_c, int);
    Complex&operator--(Complex &_c, int);

    Complex operator+(Complex _c);
    Complex operator-(Complex _c);

    Complex operator+(Imaginary1);
    Complex operator-(Imaginary1);


    Complex operator+(Complex _c, float _val);
    Complex operator-(Complex _c, float _val);
    Complex operator*(Complex _c, float _val);
    Complex operator/(Complex _c, float _val);

    Complex operator+(Complex _c, Imaginary1);
    Complex operator-(Complex _c, Imaginary1);
    Complex operator*(Complex _c, Imaginary1);
    Complex operator/(Complex _c, Imaginary1);

    Complex operator+(Complex _c, Complex _i);
    Complex operator-(Complex _c, Complex _i);
    Complex operator*(Complex _c, Complex _i);
    Complex operator/(Complex _c, Complex _i);

    Complex operator+(Imaginary1, float _val);
    Complex operator-(Imaginary1, float _val);
    Complex operator*(Imaginary1, float _val);
    Complex operator/(Imaginary1, float _val);

    Complex operator+(Imaginary1, Imaginary1);
    Complex operator-(Imaginary1, Imaginary1);
    Complex operator*(Imaginary1, Imaginary1);
    Complex operator/(Imaginary1, Imaginary1);

    Complex operator+(Imaginary1, Complex _c);
    Complex operator-(Imaginary1, Complex _c);
    Complex operator*(Imaginary1, Complex _c);
    Complex operator/(Imaginary1, Complex _c);

    Complex operator+(float _val, Imaginary1);
    Complex operator-(float _val, Imaginary1);
    Complex operator*(float _val, Imaginary1);
    Complex operator/(float _val, Imaginary1);

    Complex operator+(float _val, Complex _c);
    Complex operator-(float _val, Complex _c);
    Complex operator*(float _val, Complex _c);
    Complex operator/(float _val, Complex _c);


    void operator+=(Complex &_c, float _val);
    void operator-=(Complex &_c, float _val);
    void operator*=(Complex &_c, float _val);
    void operator/=(Complex &_c, float _val);

    void operator+=(Complex &_c, Imaginary1);
    void operator-=(Complex &_c, Imaginary1);
    void operator*=(Complex &_c, Imaginary1);
    void operator/=(Complex &_c, Imaginary1);

    void operator+=(Complex &_c, Complex _i);
    void operator-=(Complex &_c, Complex _i);
    void operator*=(Complex &_c, Complex _i);
    void operator/=(Complex &_c, Complex _i);
}; //  Simple END

#endif