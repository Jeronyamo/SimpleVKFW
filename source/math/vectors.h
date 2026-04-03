#ifndef SVKFW_VECTORS_H
#define SVKFW_VECTORS_H

#include <cstring>

#include "../common/basic_math.h"
#include "../common/utilities.h"
#include "../common/terminal.h"


namespace Simple {
//  ===========================================  \\
                Classes declaration
//  ===========================================  \\

    typedef float flt_t; // Generic floating-point type for vectors/matrices


//  ============  Vector classes declaration  ============  \\

    template <class C> struct Vec2Base;
    template <class C> struct Vec3Base;
    template <class C> struct Vec4Base;

    typedef Vec2Base<int8_t>   vec2i8;
    typedef Vec2Base<int16_t>  vec2i16;
    typedef Vec2Base<int32_t>  vec2i32;
    typedef Vec2Base<uint8_t>  vec2u8;
    typedef Vec2Base<uint16_t> vec2u16;
    typedef Vec2Base<uint32_t> vec2u32;

    typedef Vec2Base<int>      vec2i;
    typedef Vec2Base<bool>     vec2b;
    typedef Vec2Base<long>     vec2l;
    typedef Vec2Base<unsigned> vec2u;
    typedef Vec2Base<float>    vec2f;
    typedef Vec2Base<double>   vec2d;
    typedef Vec2Base<flt_t>    vec2 ;

    typedef Vec3Base<int8_t>   vec3i8;
    typedef Vec3Base<int16_t>  vec3i16;
    typedef Vec3Base<int32_t>  vec3i32;
    typedef Vec3Base<uint8_t>  vec3u8;
    typedef Vec3Base<uint16_t> vec3u16;
    typedef Vec3Base<uint32_t> vec3u32;

    typedef Vec3Base<int>      vec3i;
    typedef Vec3Base<bool>     vec3b;
    typedef Vec3Base<long>     vec3l;
    typedef Vec3Base<unsigned> vec3u;
    typedef Vec3Base<float>    vec3f;
    typedef Vec3Base<double>   vec3d;
    typedef Vec3Base<flt_t>    vec3 ;

    typedef Vec4Base<int8_t>   vec4i8;
    typedef Vec4Base<int16_t>  vec4i16;
    typedef Vec4Base<int32_t>  vec4i32;
    typedef Vec4Base<uint8_t>  vec4u8;
    typedef Vec4Base<uint16_t> vec4u16;
    typedef Vec4Base<uint32_t> vec4u32;

    typedef Vec4Base<int>      vec4i;
    typedef Vec4Base<bool>     vec4b;
    typedef Vec4Base<long>     vec4l;
    typedef Vec4Base<unsigned> vec4u;
    typedef Vec4Base<float>    vec4f;
    typedef Vec4Base<double>   vec4d;
    typedef Vec4Base<flt_t>    vec4 ;


//  ============  Matrix classes declaration  ============  \\

    template <class C> struct Mat2Base;
    template <class C> struct Mat2RefBase;
    template <class C> struct Mat3Base;
    template <class C> struct Mat3RefBase;
    template <class C> struct Mat4Base;
    template <class C> struct Mat4RefBase;

    typedef Mat2Base<int>      mat2i;
    typedef Mat2Base<bool>     mat2b;
    typedef Mat2Base<unsigned> mat2u;
    typedef Mat2Base<float>    mat2f;
    typedef Mat2Base<double>   mat2d;
    typedef Mat2Base<flt_t>    mat2 ;

    typedef Mat3Base<int>      mat3i;
    typedef Mat3Base<bool>     mat3b;
    typedef Mat3Base<unsigned> mat3u;
    typedef Mat3Base<float>    mat3f;
    typedef Mat3Base<double>   mat3d;
    typedef Mat3Base<flt_t>    mat3 ;

    typedef Mat4Base<int>      mat4i;
    typedef Mat4Base<bool>     mat4b;
    typedef Mat4Base<unsigned> mat4u;
    typedef Mat4Base<float>    mat4f;
    typedef Mat4Base<double>   mat4d;
    typedef Mat4Base<flt_t>    mat4 ;


//  ===============================  \\
                Vectors
//  ===============================  \\

//  ============  Vec 2  ============  \\

    template <class C>
    struct Vec2Base {
        C x, y;

        Vec2Base() : x{0}, y{0} {}
        Vec2Base(C _val) : x{_val}, y{_val} {}
        Vec2Base(C _x, C _y) : x(_x), y(_y) {}
        template <class Y> Vec2Base(Y _x, Y _y) : x{_x}, y{_y} {}
        template <class Y> Vec2Base(const Vec2Base<Y> &_vec) : x{_vec.x}, y{_vec.y} {}

        inline flt_t norm()  const { return sqrt(x * x + y * y); }
        inline flt_t norm2() const { return      x * x + y * y ; }
        inline Vec2Base<flt_t> inverse() const
            { return { Math::notZero(x) ? 1. / x : x, Math::notZero(y) ? 1. / y : y }; }
        inline Vec2Base<flt_t> normalize() const {
            flt_t __norm = norm();
            if (Math::notZero(__norm)) __norm = 1. / __norm;
            return { x * __norm, y * __norm };
        }

        template <typename AllT = C>
        typename std::enable_if<std::is_same<AllT, bool>::value, bool>::type all() const { return x && y; }
        template <typename AllT = C>
        typename std::enable_if<std::is_same<AllT, bool>::value, bool>::type any() const { return x || y; }
        template <typename AllT = C>
        typename std::enable_if<std::is_same<AllT, bool>::value, bool>::type non() const { return !(x || y); }


        template <char ind0, char ind1>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y') &&
                                (ind1 == 'x' || ind1 == 'y'),
        Vec2Base<Util::remove_ref<C>&>>::type subvec() {
            return { ind0 == 'x' ? x : y,
                     ind1 == 'x' ? x : y };
        }

        template <char ind0, char ind1>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y') &&
                                (ind1 == 'x' || ind1 == 'y'),
        Vec2Base<Util::remove_ref<C>>>::type subvec() const {
            return { ind0 == 'x' ? x : y,
                     ind1 == 'x' ? x : y };
        }

        template <char ind0, char ind1, char ind2>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y') &&
                                (ind1 == 'x' || ind1 == 'y') &&
                                (ind2 == 'x' || ind2 == 'y'),
        Vec3Base<Util::remove_ref<C>&>>::type subvec() {
            return { ind0 == 'x' ? x : y,
                     ind1 == 'x' ? x : y,
                     ind2 == 'x' ? x : y };
        }

        template <char ind0, char ind1, char ind2>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y') &&
                                (ind1 == 'x' || ind1 == 'y') &&
                                (ind2 == 'x' || ind2 == 'y'),
        Vec3Base<Util::remove_ref<C>>>::type subvec() const {
            return { ind0 == 'x' ? x : y,
                     ind1 == 'x' ? x : y,
                     ind2 == 'x' ? x : y };
        }

        template <char ind0, char ind1, char ind2, char ind3>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y') &&
                                (ind1 == 'x' || ind1 == 'y') &&
                                (ind2 == 'x' || ind2 == 'y') &&
                                (ind3 == 'x' || ind3 == 'y'),
        Vec4Base<Util::remove_ref<C>&>>::type subvec() {
            return { ind0 == 'x' ? x : y,
                     ind1 == 'x' ? x : y,
                     ind2 == 'x' ? x : y,
                     ind3 == 'x' ? x : y };
        }

        template <char ind0, char ind1, char ind2, char ind3>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y') &&
                                (ind1 == 'x' || ind1 == 'y') &&
                                (ind2 == 'x' || ind2 == 'y') &&
                                (ind3 == 'x' || ind3 == 'y'),
        Vec4Base<Util::remove_ref<C>>>::type subvec() const {
            return { ind0 == 'x' ? x : y,
                     ind1 == 'x' ? x : y,
                     ind2 == 'x' ? x : y,
                     ind3 == 'x' ? x : y };
        }

        template <typename Y>
        Vec2Base<Util::remove_ref<Y>> cast() const {
            return { static_cast<Y>(x), static_cast<Y>(y) };
        }

        template <class Y>
        Y castClass() const { return { x, y }; }

        template <typename Y>
        const Vec2Base<C> &operator=(const Vec2Base<Y> &ref)
            { x = ref.x; y = ref.y; return *this; }
        const Vec2Base<C> &operator=(const Vec2Base<C> &ref)
            { Vec2Base<Util::remove_ref<C>> __v{ref}; x = __v.x; y = __v.y; return *this; }
        template <typename Y> const Vec2Base<C> &operator=(const std::initializer_list<Y> &ref)
            { x = ref.begin()[0]; y = ref.begin()[1]; return *this; }
        template <typename Y> typename std::enable_if<std::is_fundamental<Y>::value, const Vec2Base<C>>::type &operator=(Y ref)
            { x = ref; y = ref; return *this; }

        template <class Y> void operator+=(const Vec2Base<Y> &_op) { x += _op.x; y += _op.y; }
        template <class Y> void operator-=(const Vec2Base<Y> &_op) { x -= _op.x; y -= _op.y; }
        template <class Y> void operator*=(const Vec2Base<Y> &_op) { x *= _op.x; y *= _op.y; }
        template <class Y> void operator/=(const Vec2Base<Y> &_op) { x /= _op.x; y /= _op.y; }
        template <class Y> void operator+=(const Y &_op) { x += _op; y += _op; }
        template <class Y> void operator-=(const Y &_op) { x -= _op; y -= _op; }
        template <class Y> void operator*=(const Y &_op) { x *= _op; y *= _op; }
        template <class Y> void operator/=(const Y &_op) { x /= _op; y /= _op; }

        Vec2Base<Util::remove_ref<C>> operator+() const { return {abs(x), abs(y)}; }
        Vec2Base<Util::remove_ref<C>> operator-() const { return {-x, -y}; }

        C& operator[](uint32_t index)       { return getElem(index, std::is_reference<C>{}); }
        C  operator[](uint32_t index) const { return getElem(index, std::is_reference<C>{}); }
        Vec2Base<C&> operator[](const char (&_a)[2 + 1]) {
            if (_a[0] < 'x' || _a[0] > 'y' || _a[1] < 'x' || _a[1] > 'y')
                throw std::invalid_argument(SVKFW_WRAPERR("vec2 :: op[] (2)","index out of bounds"));
            Util::remove_ref<C> *ptr[2]{&x, &y};
            return {*ptr[_a[0] - 'x'], *ptr[_a[1] - 'x']};
        }
        Vec3Base<C&> operator[](const char (&_a)[3 + 1]) {
            for (uint32_t i = 0; i < 3; ++i)
                if (_a[i] < 'x' || _a[i] > 'y')
                    throw std::invalid_argument(SVKFW_WRAPERR("vec2 :: op[] (3)","index out of bounds"));
            Util::remove_ref<C> *ptr[2]{&x, &y};
            return {*ptr[_a[0] - 'x'], *ptr[_a[1] - 'x'], *ptr[_a[2] - 'x']};
        }
        Vec4Base<C&> operator[](const char (&_a)[4 + 1]) {
            for (uint32_t i = 0; i < 4; ++i)
                if (_a[i] < 'x' || _a[i] > 'y')
                    throw std::invalid_argument(SVKFW_WRAPERR("vec2 :: op[] (4)","index out of bounds"));
            Util::remove_ref<C> *ptr[2]{&x, &y};
            return {*ptr[_a[0] - 'x'], *ptr[_a[1] - 'x'], *ptr[_a[2] - 'x'], *ptr[_a[3] - 'x']};
        }
        Vec2Base<C> operator[](const char (&_a)[2 + 1]) const {
            if (_a[0] < 'x' || _a[0] > 'y' || _a[1] < 'x' || _a[1] > 'y')
                throw std::invalid_argument(SVKFW_WRAPERR("vec2 :: op[] const (2)","index out of bounds"));
            const Util::remove_ref<C> *ptr[2]{&x, &y};
            return {*ptr[_a[0] - 'x'], *ptr[_a[1] - 'x']};
        }
        Vec3Base<C> operator[](const char (&_a)[3 + 1]) const {
            for (uint32_t i = 0; i < 3; ++i)
                if (_a[i] < 'x' || _a[i] > 'y')
                    throw std::invalid_argument(SVKFW_WRAPERR("vec2 :: op[] const (3)","index out of bounds"));
            const Util::remove_ref<C> *ptr[2]{&x, &y};
            return {*ptr[_a[0] - 'x'], *ptr[_a[1] - 'x'], *ptr[_a[2] - 'x']};
        }
        Vec4Base<C> operator[](const char (&_a)[4 + 1]) const {
            for (uint32_t i = 0; i < 4; ++i)
                if (_a[i] < 'x' || _a[i] > 'y')
                    throw std::invalid_argument(SVKFW_WRAPERR("vec2 :: op[] const (4)","index out of bounds"));
            const Util::remove_ref<C> *ptr[2]{&x, &y};
            return {*ptr[_a[0] - 'x'], *ptr[_a[1] - 'x'], *ptr[_a[2] - 'x'], *ptr[_a[3] - 'x']};
        }

    private:
        Util::remove_ref<C>& getElem(uint32_t index, std::false_type)       { return *((Util::remove_ref<C>*)this + index); }
        Util::remove_ref<C>  getElem(uint32_t index, std::false_type) const { return *((Util::remove_ref<C>*)this + index); }
        Util::remove_ref<C>& getElem(uint32_t index, std:: true_type) {
            switch (index) {
                case 0: return x;
                case 1: return y;
                default:
                    throw std::invalid_argument(SVKFW_WRAPERR("vec2 :: op[] (1)","index out of bounds"));
            }
        }
        Util::remove_ref<C>  getElem(uint32_t index, std:: true_type) const {
            switch (index) {
                case 0: return x;
                case 1: return y;
                default:
                    throw std::invalid_argument(SVKFW_WRAPERR("vec2 :: op[] const (1)","index out of bounds"));
            }
        }
    }; // Vec2Base END

    template <typename C, typename Y> Vec2Base<Util::comm_type<C,Y>> operator+(const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x + op2.x, op1.y + op2.y}; }
    template <typename C, typename Y> Vec2Base<Util::comm_vecsub_type<C,Y>> operator-(const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x - op2.x, op1.y - op2.y}; }
    template <typename C, typename Y> Vec2Base<Util::comm_type<C,Y>> operator*(const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x * op2.x, op1.y * op2.y}; }
    template <typename C, typename Y> Vec2Base<Util::comm_type<C,Y>> operator/(const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x / op2.x, op1.y / op2.y}; }

    template <typename C, typename Y> Vec2Base<Util::comm_type<C,Y>> operator+(const Vec2Base<C> &op1, Y op2)
        { return {op1.x + op2, op1.y + op2}; }
    template <typename C, typename Y> Vec2Base<Util::comm_vecsub_type<C,Y>> operator-(const Vec2Base<C> &op1, Y op2)
        { return {op1.x - op2, op1.y - op2}; }
    template <typename C, typename Y> Vec2Base<Util::comm_type<C,Y>> operator*(const Vec2Base<C> &op1, Y op2)
        { return {op1.x * op2, op1.y * op2}; }
    template <typename C, typename Y> Vec2Base<Util::comm_type<C,Y>> operator/(const Vec2Base<C> &op1, Y op2)
        { return {op1.x / op2, op1.y / op2}; }

    template <typename C, typename Y> Vec2Base<Util::comm_type<C,Y>> operator+(Y op1, const Vec2Base<C> &op2)
        { return {op1 + op2.x, op1 + op2.y}; }
    template <typename C, typename Y> Vec2Base<Util::comm_vecsub_type<C,Y>> operator-(Y op1, const Vec2Base<C> &op2)
        { return {op1 - op2.x, op1 - op2.y}; }
    template <typename C, typename Y> Vec2Base<Util::comm_type<C,Y>> operator*(Y op1, const Vec2Base<C> &op2)
        { return {op1 * op2.x, op1 * op2.y}; }
    template <typename C, typename Y> Vec2Base<Util::comm_type<C,Y>> operator/(Y op1, const Vec2Base<C> &op2)
        { return {op1 / op2.x, op1 / op2.y}; }


    template <typename C, typename Y> Vec2Base<bool> operator> (const C &op1, const Vec2Base<Y> &op2)
        { return {op1 >  op2.x, op1 >  op2.y}; }
    template <typename C, typename Y> Vec2Base<bool> operator< (const C &op1, const Vec2Base<Y> &op2)
        { return {op1 <  op2.x, op1 <  op2.y}; }
    template <typename C, typename Y> Vec2Base<bool> operator>=(const C &op1, const Vec2Base<Y> &op2)
        { return {op1 >= op2.x, op1 >= op2.y}; }
    template <typename C, typename Y> Vec2Base<bool> operator<=(const C &op1, const Vec2Base<Y> &op2)
        { return {op1 <= op2.x, op1 <= op2.y}; }
    template <typename C, typename Y> Vec2Base<bool> operator==(const C &op1, const Vec2Base<Y> &op2)
        { return {op1 == op2.x, op1 == op2.y}; }
    template <typename C, typename Y> Vec2Base<bool> operator!=(const C &op1, const Vec2Base<Y> &op2)
        { return {op1 != op2.x, op1 != op2.y}; }

    template <typename C, typename Y> Vec2Base<bool> operator> (const Vec2Base<C> &op1, const Y &op2)
        { return {op1.x >  op2, op1.y >  op2}; }
    template <typename C, typename Y> Vec2Base<bool> operator< (const Vec2Base<C> &op1, const Y &op2)
        { return {op1.x <  op2, op1.y <  op2}; }
    template <typename C, typename Y> Vec2Base<bool> operator>=(const Vec2Base<C> &op1, const Y &op2)
        { return {op1.x >= op2, op1.y >= op2}; }
    template <typename C, typename Y> Vec2Base<bool> operator<=(const Vec2Base<C> &op1, const Y &op2)
        { return {op1.x <= op2, op1.y <= op2}; }
    template <typename C, typename Y> Vec2Base<bool> operator==(const Vec2Base<C> &op1, const Y &op2)
        { return {op1.x == op2, op1.y == op2}; }
    template <typename C, typename Y> Vec2Base<bool> operator!=(const Vec2Base<C> &op1, const Y &op2)
        { return {op1.x != op2, op1.y != op2}; }

    template <typename C, typename Y> Vec2Base<bool> operator> (const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x >  op2.x, op1.y >  op2.y}; }
    template <typename C, typename Y> Vec2Base<bool> operator< (const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x <  op2.x, op1.y <  op2.y}; }
    template <typename C, typename Y> Vec2Base<bool> operator>=(const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x >= op2.x, op1.y >= op2.y}; }
    template <typename C, typename Y> Vec2Base<bool> operator<=(const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x <= op2.x, op1.y <= op2.y}; }
    template <typename C, typename Y> Vec2Base<bool> operator==(const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x == op2.x, op1.y == op2.y}; }
    template <typename C, typename Y> Vec2Base<bool> operator!=(const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
        { return {op1.x != op2.x, op1.y != op2.y}; }


//  ============  Vec 3  ============  \\

    template <class C>
    struct Vec3Base {
        C x, y, z;

        Vec3Base() : x{0}, y{0}, z{0} {}
        Vec3Base(C _val) : x{_val}, y{_val}, z{_val} {}
        Vec3Base(C _x, C _y, C _z) : x(_x), y(_y), z(_z) {}
        template <class Y> Vec3Base(Y _x, Y _y, Y _z) : x{_x}, y{_y}, z{_z} {}
        template <class Y> Vec3Base(const Vec3Base<Y> &_vec) : x{_vec.x}, y{_vec.y}, z{_vec.z} {}
        template <class X, class Y> Vec3Base(const Vec2Base<X> &_vec, Y _val) : x{_vec.x}, y{_vec.y}, z{_val} {}
        template <class X, class Y> Vec3Base(Y _val, const Vec2Base<X> &_vec) : x{_val}, y{_vec.x}, z{_vec.y} {}

        flt_t norm() const  { return sqrt(x * x + y * y + z * z); }
        flt_t norm2() const { return      x * x + y * y + z * z ; }
        Vec3Base<flt_t> inverse() const
            { return { Math::notZero(x) ? 1. / x : x, Math::notZero(y) ? 1. / y : y, Math::notZero(z) ? 1. / z : z }; }
        Vec3Base<flt_t> normalize() const {
            flt_t __norm = norm();
            if (Math::notZero(__norm)) __norm = 1. / __norm;
            return { x * __norm, y * __norm, z * __norm };
        }

        template <typename AllT = C>
        typename std::enable_if<std::is_same<AllT, bool>::value, bool>::type all() const { return x && y && z; }
        template <typename AllT = C>
        typename std::enable_if<std::is_same<AllT, bool>::value, bool>::type any() const { return x || y || z; }


        template <char ind0, char ind1>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y' || ind0 == 'z') &&
                                (ind1 == 'x' || ind1 == 'y' || ind1 == 'z'),
        Vec2Base<Util::remove_ref<C>&>>::type subvec() {
            return { ind0 == 'x' ? x : ind0 == 'y' ? y : z,
                     ind1 == 'x' ? x : ind1 == 'y' ? y : z };
        }

        template <char ind0, char ind1>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y' || ind0 == 'z') &&
                                (ind1 == 'x' || ind1 == 'y' || ind1 == 'z'),
        Vec2Base<Util::remove_ref<C>>>::type subvec() const {
            return { ind0 == 'x' ? x : ind0 == 'y' ? y : z,
                     ind1 == 'x' ? x : ind1 == 'y' ? y : z };
        }

        template <char ind0, char ind1, char ind2>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y' || ind0 == 'z') &&
                                (ind1 == 'x' || ind1 == 'y' || ind1 == 'z') &&
                                (ind2 == 'x' || ind2 == 'y' || ind2 == 'z'),
        Vec3Base<Util::remove_ref<C>&>>::type subvec() {
            return { ind0 == 'x' ? x : ind0 == 'y' ? y : z,
                     ind1 == 'x' ? x : ind1 == 'y' ? y : z,
                     ind2 == 'x' ? x : ind2 == 'y' ? y : z };
        }

        template <char ind0, char ind1, char ind2>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y' || ind0 == 'z') &&
                                (ind1 == 'x' || ind1 == 'y' || ind1 == 'z') &&
                                (ind2 == 'x' || ind2 == 'y' || ind2 == 'z'),
        Vec3Base<Util::remove_ref<C>>>::type subvec() const {
            return { ind0 == 'x' ? x : ind0 == 'y' ? y : z,
                     ind1 == 'x' ? x : ind1 == 'y' ? y : z,
                     ind2 == 'x' ? x : ind2 == 'y' ? y : z };
        }

        template <char ind0, char ind1, char ind2, char ind3>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y' || ind0 == 'z') &&
                                (ind1 == 'x' || ind1 == 'y' || ind1 == 'z') &&
                                (ind2 == 'x' || ind2 == 'y' || ind2 == 'z') &&
                                (ind3 == 'x' || ind3 == 'y' || ind3 == 'z'),
        Vec4Base<Util::remove_ref<C>&>>::type subvec() {
            return { ind0 == 'x' ? x : ind0 == 'y' ? y : z,
                     ind1 == 'x' ? x : ind1 == 'y' ? y : z,
                     ind2 == 'x' ? x : ind2 == 'y' ? y : z,
                     ind3 == 'x' ? x : ind3 == 'y' ? y : z };
        }

        template <char ind0, char ind1, char ind2, char ind3>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y' || ind0 == 'z') &&
                                (ind1 == 'x' || ind1 == 'y' || ind1 == 'z') &&
                                (ind2 == 'x' || ind2 == 'y' || ind2 == 'z') &&
                                (ind3 == 'x' || ind3 == 'y' || ind3 == 'z'),
        Vec4Base<Util::remove_ref<C>>>::type subvec() const {
            return { ind0 == 'x' ? x : ind0 == 'y' ? y : z,
                     ind1 == 'x' ? x : ind1 == 'y' ? y : z,
                     ind2 == 'x' ? x : ind2 == 'y' ? y : z,
                     ind3 == 'x' ? x : ind3 == 'y' ? y : z };
        }

        template <typename Y>
        Vec3Base<Util::remove_ref<Y>> cast() const {
            return { static_cast<Y>(x), static_cast<Y>(y), static_cast<Y>(z) };
        }

        template <class Y>
        Y castClass() const { return { x, y, z }; }

        template <typename Y>
        const Vec3Base<C> &operator=(const Vec3Base<Y> &ref)
            { x = ref.x; y = ref.y; z = ref.z; return *this; }
        const Vec3Base<C> &operator=(const Vec3Base<C> &ref)
            { Vec3Base<Util::remove_ref<C>> __v{ref}; x = __v.x; y = __v.y; z = __v.z; return *this; }
        template <typename Y> const Vec3Base<C> &operator=(const std::initializer_list<Y> &ref)
            { x = ref.begin()[0]; y = ref.begin()[1]; z = ref.begin()[2]; return *this; }
        template <typename Y> typename std::enable_if<std::is_fundamental<Y>::value, const Vec3Base<C>>::type &operator=(Y ref)
            { x = ref; y = ref; z = ref; return *this; }

        template <class Y> void operator+=(const Vec3Base<Y> &_op) { x += _op.x; y += _op.y; z += _op.z; }
        template <class Y> void operator-=(const Vec3Base<Y> &_op) { x -= _op.x; y -= _op.y; z -= _op.z; }
        template <class Y> void operator*=(const Vec3Base<Y> &_op) { x *= _op.x; y *= _op.y; z *= _op.z; }
        template <class Y> void operator/=(const Vec3Base<Y> &_op) { x /= _op.x; y /= _op.y; z /= _op.z; }
        template <class Y> void operator+=(const Y &_op) { x += _op; y += _op; z += _op; }
        template <class Y> void operator-=(const Y &_op) { x -= _op; y -= _op; z -= _op; }
        template <class Y> void operator*=(const Y &_op) { x *= _op; y *= _op; z *= _op; }
        template <class Y> void operator/=(const Y &_op) { x /= _op; y /= _op; z /= _op; }

        Vec3Base<Util::remove_ref<C>> operator+() const { return {abs(x), abs(y), abs(z)}; }
        Vec3Base<Util::remove_ref<C>> operator-() const { return {-x, -y, -z}; }

        C& operator[](uint32_t index)       { return getElem(index, std::is_reference<C>{}); }
        C  operator[](uint32_t index) const { return getElem(index, std::is_reference<C>{}); }
        Vec2Base<C&> operator[](const char (&_a)[2 + 1]) {
            if (_a[0] < 'x' || _a[0] > 'z' || _a[1] < 'x' || _a[1] > 'z')
                throw std::invalid_argument(SVKFW_WRAPERR("vec3 :: op[] (2)","index out of bounds"));
            Util::remove_ref<C> *ptr[3]{&x, &y, &z};
            return {*ptr[_a[0] - 'x'], *ptr[_a[1] - 'x']};
        }
        Vec3Base<C&> operator[](const char (&_a)[3 + 1]) {
            for (uint32_t i = 0; i < 3; ++i)
                if (_a[i] < 'x' || _a[i] > 'z')
                    throw std::invalid_argument(SVKFW_WRAPERR("vec3 :: op[] (3)","index out of bounds"));
            Util::remove_ref<C> *ptr[3]{&x, &y, &z};
            return {*ptr[_a[0] - 'x'], *ptr[_a[1] - 'x'], *ptr[_a[2] - 'x']};
        }
        Vec4Base<C&> operator[](const char (&_a)[4 + 1]) {
            for (uint32_t i = 0; i < 4; ++i)
                if (_a[i] < 'x' || _a[i] > 'z')
                    throw std::invalid_argument(SVKFW_WRAPERR("vec3 :: op[] (4)","index out of bounds"));
            Util::remove_ref<C> *ptr[3]{&x, &y, &z};
            return {*ptr[_a[0] - 'x'], *ptr[_a[1] - 'x'], *ptr[_a[2] - 'x'], *ptr[_a[3] - 'x']};
        }
        Vec2Base<C> operator[](const char (&_a)[2 + 1]) const {
            if (_a[0] < 'x' || _a[0] > 'z' || _a[1] < 'x' || _a[1] > 'z')
                throw std::invalid_argument(SVKFW_WRAPERR("vec3 :: op[] const (2)","index out of bounds"));
            const Util::remove_ref<C> *ptr[3]{&x, &y, &z};
            return {*ptr[_a[0] - 'x'], *ptr[_a[1] - 'x']};
        }
        Vec3Base<C> operator[](const char (&_a)[3 + 1]) const {
            for (uint32_t i = 0; i < 3; ++i)
                if (_a[i] < 'x' || _a[i] > 'z')
                    throw std::invalid_argument(SVKFW_WRAPERR("vec3 :: op[] const (3)","index out of bounds"));
            const Util::remove_ref<C> *ptr[3]{&x, &y, &z};
            return {*ptr[_a[0] - 'x'], *ptr[_a[1] - 'x'], *ptr[_a[2] - 'x']};
        }
        Vec4Base<C> operator[](const char (&_a)[4 + 1]) const {
            for (uint32_t i = 0; i < 4; ++i)
                if (_a[i] < 'x' || _a[i] > 'z')
                    throw std::invalid_argument(SVKFW_WRAPERR("vec3 :: op[] const (4)","index out of bounds"));
            const Util::remove_ref<C> *ptr[3]{&x, &y, &z};
            return {*ptr[_a[0] - 'x'], *ptr[_a[1] - 'x'], *ptr[_a[2] - 'x'], *ptr[_a[3] - 'x']};
        }

    private:
        Util::remove_ref<C>& getElem(uint32_t index, std::false_type)       { return *((Util::remove_ref<C>*)this + index); }
        Util::remove_ref<C>  getElem(uint32_t index, std::false_type) const { return *((Util::remove_ref<C>*)this + index); }
        Util::remove_ref<C>& getElem(uint32_t index, std:: true_type) {
            switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                default:
                    throw std::invalid_argument(SVKFW_WRAPERR("vec3 :: op[] (1)","index out of bounds"));
            }
        }
        Util::remove_ref<C>  getElem(uint32_t index, std:: true_type) const {
            switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                default:
                    throw std::invalid_argument(SVKFW_WRAPERR("vec3 :: op[] const (1)","index out of bounds"));
            }
        }
    }; // Vec3Base END

    template <typename C, typename Y> Vec3Base<Util::comm_type<C,Y>> operator+(const Vec3Base<C> &op1, const Vec3Base<Y> &op2)
        { return {op1.x + op2.x, op1.y + op2.y, op1.z + op2.z}; }
    template <typename C, typename Y> Vec3Base<Util::comm_vecsub_type<C,Y>> operator-(const Vec3Base<C> &op1, const Vec3Base<Y> &op2)
        { return {op1.x - op2.x, op1.y - op2.y, op1.z - op2.z}; }
    template <typename C, typename Y> Vec3Base<Util::comm_type<C,Y>> operator*(const Vec3Base<C> &op1, const Vec3Base<Y> &op2)
        { return {op1.x * op2.x, op1.y * op2.y, op1.z * op2.z}; }
    template <typename C, typename Y> Vec3Base<Util::comm_type<C,Y>> operator/(const Vec3Base<C> &op1, const Vec3Base<Y> &op2)
        { return {op1.x / op2.x, op1.y / op2.y, op1.z / op2.z}; }

    template <typename C, typename Y> Vec3Base<Util::comm_type<C,Y>> operator+(const Vec3Base<C> &op1, Y op2)
        { return {op1.x + op2, op1.y + op2, op1.z + op2}; }
    template <typename C, typename Y> Vec3Base<Util::comm_vecsub_type<C,Y>> operator-(const Vec3Base<C> &op1, Y op2)
        { return {op1.x - op2, op1.y - op2, op1.z - op2}; }
    template <typename C, typename Y> Vec3Base<Util::comm_type<C,Y>> operator*(const Vec3Base<C> &op1, Y op2)
        { return {op1.x * op2, op1.y * op2, op1.z * op2}; }
    template <typename C, typename Y> Vec3Base<Util::comm_type<C,Y>> operator/(const Vec3Base<C> &op1, Y op2)
        { return {op1.x / op2, op1.y / op2, op1.z / op2}; }

    template <typename C, typename Y> Vec3Base<Util::comm_type<C,Y>> operator+(Y op1, const Vec3Base<C> &op2)
        { return {op1 + op2.x, op1 + op2.y, op1 + op2.z}; }
    template <typename C, typename Y> Vec3Base<Util::comm_vecsub_type<C,Y>> operator-(Y op1, const Vec3Base<C> &op2)
        { return {op1 - op2.x, op1 - op2.y, op1 - op2.z}; }
    template <typename C, typename Y> Vec3Base<Util::comm_type<C,Y>> operator*(Y op1, const Vec3Base<C> &op2)
        { return {op1 * op2.x, op1 * op2.y, op1 * op2.z}; }
    template <typename C, typename Y> Vec3Base<Util::comm_type<C,Y>> operator/(Y op1, const Vec3Base<C> &op2)
        { return {op1 / op2.x, op1 / op2.y, op1 / op2.z}; }


    template <typename C, typename Y> Vec3Base<bool> operator> (const C &op1, const Vec3Base<Y> &op2)
        { return {op1 >  op2.x, op1 >  op2.y, op1 >  op2.z}; }
    template <typename C, typename Y> Vec3Base<bool> operator< (const C &op1, const Vec3Base<Y> &op2)
        { return {op1 <  op2.x, op1 <  op2.y, op1 <  op2.z}; }
    template <typename C, typename Y> Vec3Base<bool> operator>=(const C &op1, const Vec3Base<Y> &op2)
        { return {op1 >= op2.x, op1 >= op2.y, op1 >= op2.z}; }
    template <typename C, typename Y> Vec3Base<bool> operator<=(const C &op1, const Vec3Base<Y> &op2)
        { return {op1 <= op2.x, op1 <= op2.y, op1 <= op2.z}; }
    template <typename C, typename Y> Vec3Base<bool> operator==(const C &op1, const Vec3Base<Y> &op2)
        { return {op1 == op2.x, op1 == op2.y, op1 == op2.z}; }
    template <typename C, typename Y> Vec3Base<bool> operator!=(const C &op1, const Vec3Base<Y> &op2)
        { return {op1 != op2.x, op1 != op2.y, op1 != op2.z}; }

    template <typename C, typename Y> Vec3Base<bool> operator> (const Vec3Base<C> &op1, const Y &op2)
        { return {op1.x >  op2, op1.y >  op2, op1.z >  op2}; }
    template <typename C, typename Y> Vec3Base<bool> operator< (const Vec3Base<C> &op1, const Y &op2)
        { return {op1.x <  op2, op1.y <  op2, op1.z <  op2}; }
    template <typename C, typename Y> Vec3Base<bool> operator>=(const Vec3Base<C> &op1, const Y &op2)
        { return {op1.x >= op2, op1.y >= op2, op1.z >= op2}; }
    template <typename C, typename Y> Vec3Base<bool> operator<=(const Vec3Base<C> &op1, const Y &op2)
        { return {op1.x <= op2, op1.y <= op2, op1.z <= op2}; }
    template <typename C, typename Y> Vec3Base<bool> operator==(const Vec3Base<C> &op1, const Y &op2)
        { return {op1.x == op2, op1.y == op2, op1.z == op2}; }
    template <typename C, typename Y> Vec3Base<bool> operator!=(const Vec3Base<C> &op1, const Y &op2)
        { return {op1.x != op2, op1.y != op2, op1.z != op2}; }

    template <typename C, typename Y> Vec3Base<bool> operator> (const Vec3Base<C> &op1, const Vec3Base<Y> &op2)
        { return {op1.x >  op2.x, op1.y >  op2.y, op1.z >  op2.z}; }
    template <typename C, typename Y> Vec3Base<bool> operator< (const Vec3Base<C> &op1, const Vec3Base<Y> &op2)
        { return {op1.x <  op2.x, op1.y <  op2.y, op1.z <  op2.z}; }
    template <typename C, typename Y> Vec3Base<bool> operator>=(const Vec3Base<C> &op1, const Vec3Base<Y> &op2)
        { return {op1.x >= op2.x, op1.y >= op2.y, op1.z >= op2.z}; }
    template <typename C, typename Y> Vec3Base<bool> operator<=(const Vec3Base<C> &op1, const Vec3Base<Y> &op2)
        { return {op1.x <= op2.x, op1.y <= op2.y, op1.z <= op2.z}; }
    template <typename C, typename Y> Vec3Base<bool> operator==(const Vec3Base<C> &op1, const Vec3Base<Y> &op2)
        { return {op1.x == op2.x, op1.y == op2.y, op1.z == op2.z}; }
    template <typename C, typename Y> Vec3Base<bool> operator!=(const Vec3Base<C> &op1, const Vec3Base<Y> &op2)
        { return {op1.x != op2.x, op1.y != op2.y, op1.z != op2.z}; }


//  ============  Vec 4  ============  \\

    template <class C>
    struct Vec4Base {
        C x, y, z, w;

        Vec4Base() : x{0}, y{0}, z{0}, w{0} {}
        Vec4Base(C _val) : x{_val}, y{_val}, z{_val}, w{_val} {}
        Vec4Base(C _x, C _y, C _z, C _w) : x(_x), y(_y), z(_z), w(_w) {}
        template <class Y> Vec4Base(Y _x, Y _y, Y _z, Y _w) : x{_x}, y{_y}, z{_z}, w{_w} {}
        template <class X, class Y> Vec4Base(const Vec2Base<X> &_vec, Y _z, Y _w) : x{_vec.x}, y{_vec.y}, z{_z}, w{_w} {}
        template <class X, class Y> Vec4Base(Y _x, const Vec2Base<X> &_vec, Y _w) : x{_x}, y{_vec.x}, z{_vec.y}, w{_w} {}
        template <class X, class Y> Vec4Base(Y _x, Y _y, const Vec2Base<X> &_vec) : x{_x}, y{_y}, z{_vec.x}, w{_vec.y} {}
        template <class X, class Y> Vec4Base(const Vec2Base<X> &_vec1, const Vec2Base<Y> &_vec2) : x{_vec1.x}, y{_vec1.y}, z{_vec2.x}, w{_vec2.y} {}
        template <class X, class Y> Vec4Base(const Vec3Base<X> &_vec, Y _y) : x{_vec.x}, y{_vec.y}, z{_vec.z}, w{_y} {}
        template <class X, class Y> Vec4Base(Y _x, const Vec3Base<X> &_vec) : x{_x}, y{_vec.x}, z{_vec.y}, w{_vec.z} {}
        template <class Y> Vec4Base(const Vec4Base<Y> &_vec) : x{_vec.x}, y{_vec.y}, z{_vec.z}, w{_vec.w} {}

        flt_t norm()  const { return sqrt(x * x + y * y + z * z + w * w); }
        flt_t norm2() const { return      x * x + y * y + z * z + w * w ; }
        Vec4Base<flt_t> inverse() const
            { return { Math::notZero(x) ? 1. / x : x, Math::notZero(y) ? 1. / y : y,
                       Math::notZero(z) ? 1. / z : z, Math::notZero(w) ? 1. / w : w }; }
        Vec4Base<flt_t> normalize() const {
            flt_t __norm = norm();
            if (Math::notZero(__norm)) __norm = 1. / __norm;
            return { x * __norm, y * __norm, z * __norm, w * __norm };
        }

        template <typename AllT = C>
        typename std::enable_if<std::is_same<AllT, bool>::value, bool>::type all() const { return x && y && z && w; }
        template <typename AllT = C>
        typename std::enable_if<std::is_same<AllT, bool>::value, bool>::type any() const { return x || y || z || w; }


        template <char ind0, char ind1>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y' || ind0 == 'z' || ind0 == 'w') &&
                                (ind1 == 'x' || ind1 == 'y' || ind1 == 'z' || ind1 == 'w'),
        Vec2Base<Util::remove_ref<C>&>>::type subvec() {
            return { ind0 == 'x' ? x : ind0 == 'y' ? y : ind0 == 'z' ? z : w,
                     ind1 == 'x' ? x : ind1 == 'y' ? y : ind1 == 'z' ? z : w };
        }

        template <char ind0, char ind1>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y' || ind0 == 'z' || ind0 == 'w') &&
                                (ind1 == 'x' || ind1 == 'y' || ind1 == 'z' || ind1 == 'w'),
        Vec2Base<Util::remove_ref<C>>>::type subvec() const {
            return { ind0 == 'x' ? x : ind0 == 'y' ? y : ind0 == 'z' ? z : w,
                     ind1 == 'x' ? x : ind1 == 'y' ? y : ind1 == 'z' ? z : w };
        }

        template <char ind0, char ind1, char ind2>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y' || ind0 == 'z' || ind0 == 'w') &&
                                (ind1 == 'x' || ind1 == 'y' || ind1 == 'z' || ind1 == 'w') &&
                                (ind2 == 'x' || ind2 == 'y' || ind2 == 'z' || ind2 == 'w'),
        Vec3Base<Util::remove_ref<C>&>>::type subvec() {
            return { ind0 == 'x' ? x : ind0 == 'y' ? y : ind0 == 'z' ? z : w,
                     ind1 == 'x' ? x : ind1 == 'y' ? y : ind1 == 'z' ? z : w,
                     ind2 == 'x' ? x : ind2 == 'y' ? y : ind2 == 'z' ? z : w };
        }

        template <char ind0, char ind1, char ind2>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y' || ind0 == 'z' || ind0 == 'w') &&
                                (ind1 == 'x' || ind1 == 'y' || ind1 == 'z' || ind1 == 'w') &&
                                (ind2 == 'x' || ind2 == 'y' || ind2 == 'z' || ind2 == 'w'),
        Vec3Base<Util::remove_ref<C>>>::type subvec() const {
            return { ind0 == 'x' ? x : ind0 == 'y' ? y : ind0 == 'z' ? z : w,
                     ind1 == 'x' ? x : ind1 == 'y' ? y : ind1 == 'z' ? z : w,
                     ind2 == 'x' ? x : ind2 == 'y' ? y : ind2 == 'z' ? z : w };
        }

        template <char ind0, char ind1, char ind2, char ind3>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y' || ind0 == 'z' || ind0 == 'w') &&
                                (ind1 == 'x' || ind1 == 'y' || ind1 == 'z' || ind1 == 'w') &&
                                (ind2 == 'x' || ind2 == 'y' || ind2 == 'z' || ind2 == 'w') &&
                                (ind3 == 'x' || ind3 == 'y' || ind3 == 'z' || ind3 == 'w'),
        Vec4Base<Util::remove_ref<C>&>>::type subvec() {
            return { ind0 == 'x' ? x : ind0 == 'y' ? y : ind0 == 'z' ? z : w,
                     ind1 == 'x' ? x : ind1 == 'y' ? y : ind1 == 'z' ? z : w,
                     ind2 == 'x' ? x : ind2 == 'y' ? y : ind2 == 'z' ? z : w,
                     ind3 == 'x' ? x : ind3 == 'y' ? y : ind3 == 'z' ? z : w };
        }

        template <char ind0, char ind1, char ind2, char ind3>
        typename std::enable_if<(ind0 == 'x' || ind0 == 'y' || ind0 == 'z' || ind0 == 'w') &&
                                (ind1 == 'x' || ind1 == 'y' || ind1 == 'z' || ind1 == 'w') &&
                                (ind2 == 'x' || ind2 == 'y' || ind2 == 'z' || ind2 == 'w') &&
                                (ind3 == 'x' || ind3 == 'y' || ind3 == 'z' || ind3 == 'w'),
        Vec4Base<Util::remove_ref<C>>>::type subvec() const {
            return { ind0 == 'x' ? x : ind0 == 'y' ? y : ind0 == 'z' ? z : w,
                     ind1 == 'x' ? x : ind1 == 'y' ? y : ind1 == 'z' ? z : w,
                     ind2 == 'x' ? x : ind2 == 'y' ? y : ind2 == 'z' ? z : w,
                     ind3 == 'x' ? x : ind3 == 'y' ? y : ind3 == 'z' ? z : w };
        }

        template <typename Y>
        Vec4Base<Util::remove_ref<Y>> cast() const {
            return { static_cast<Y>(x), static_cast<Y>(y), static_cast<Y>(z), static_cast<Y>(w) };
        }

        template <class Y>
        Y castClass() const {
            return { x, y, z, w };
        }

        template <typename Y>
        const Vec4Base<C> &operator=(const Vec4Base<Y> &ref)
            { x = ref.x; y = ref.y; z = ref.z; w = ref.w; return *this; }
        const Vec4Base<C> &operator=(const Vec4Base<C> &ref)
            { Vec4Base<Util::remove_ref<C>> __v{ref}; x = __v.x; y = __v.y; z = __v.z; w = __v.w; return *this; }
        template <typename Y> const Vec4Base<C> &operator=(const std::initializer_list<Y> &ref)
            { x = ref.begin()[0]; y = ref.begin()[1]; z = ref.begin()[2]; w = ref.begin()[3]; return *this; }
        template <typename Y> typename std::enable_if<std::is_fundamental<Y>::value, const Vec4Base<C>>::type &operator=(Y ref)
            { x = ref; y = ref; z = ref; w = ref; return *this; }

        template <class Y> void operator+=(const Vec4Base<Y> &_op) { x += _op.x; y += _op.y; z += _op.z; w += _op.w; }
        template <class Y> void operator-=(const Vec4Base<Y> &_op) { x -= _op.x; y -= _op.y; z -= _op.z; w -= _op.w; }
        template <class Y> void operator*=(const Vec4Base<Y> &_op) { x *= _op.x; y *= _op.y; z *= _op.z; w *= _op.w; }
        template <class Y> void operator/=(const Vec4Base<Y> &_op) { x /= _op.x; y /= _op.y; z /= _op.z; w /= _op.w; }
        template <class Y> void operator+=(Y _op) { x += _op; y += _op; z += _op; w += _op; }
        template <class Y> void operator-=(Y _op) { x -= _op; y -= _op; z -= _op; w -= _op; }
        template <class Y> void operator*=(Y _op) { x *= _op; y *= _op; z *= _op; w *= _op; }
        template <class Y> void operator/=(Y _op) { x /= _op; y /= _op; z /= _op; w /= _op; }

        Vec4Base<Util::remove_ref<C>> operator+() const { return {abs(x), abs(y), abs(z), abs(w)}; }
        Vec4Base<Util::remove_ref<C>> operator-() const { return {-x, -y, -z, -w}; }

        C& operator[](uint32_t index)       { return getElem(index, std::is_reference<C>{}); }
        C  operator[](uint32_t index) const { return getElem(index, std::is_reference<C>{}); }
        Vec2Base<C&> operator[](const char (&_a)[2 + 1]) {
            if (_a[0] < 'w' || _a[0] > 'z' || _a[1] < 'w' || _a[1] > 'z')
                throw std::invalid_argument(SVKFW_WRAPERR("vec4 :: op[] (2)","index out of bounds"));
            Util::remove_ref<C> *ptr[4]{&w, &x, &y, &z};
            return {*ptr[_a[0] - 'w'], *ptr[_a[1] - 'w']};
        }
        Vec3Base<C&> operator[](const char (&_a)[3 + 1]) {
            for (uint32_t i = 0; i < 3; ++i)
                if (_a[i] < 'w' || _a[i] > 'z')
                    throw std::invalid_argument(SVKFW_WRAPERR("vec4 :: op[] (3)","index out of bounds"));
            Util::remove_ref<C> *ptr[4]{&w, &x, &y, &z};
            return {*ptr[_a[0] - 'w'], *ptr[_a[1] - 'w'], *ptr[_a[2] - 'w']};
        }
        Vec4Base<C&> operator[](const char (&_a)[4 + 1]) {
            for (uint32_t i = 0; i < 4; ++i)
                if (_a[i] < 'w' || _a[i] > 'z')
                    throw std::invalid_argument(SVKFW_WRAPERR("vec4 :: op[] (4)","index out of bounds"));
            Util::remove_ref<C> *ptr[4]{&w, &x, &y, &z};
            return {*ptr[_a[0] - 'w'], *ptr[_a[1] - 'w'], *ptr[_a[2] - 'w'], *ptr[_a[3] - 'w']};
        }
        Vec2Base<C> operator[](const char (&_a)[2 + 1]) const {
            if (_a[0] < 'w' || _a[0] > 'z' || _a[1] < 'w' || _a[1] > 'z')
                throw std::invalid_argument(SVKFW_WRAPERR("vec4 :: op[] const (2)","index out of bounds"));
            const Util::remove_ref<C> *ptr[4]{&w, &x, &y, &z};
            return {*ptr[_a[0] - 'w'], *ptr[_a[1] - 'w']};
        }
        Vec3Base<C> operator[](const char (&_a)[3 + 1]) const {
            for (uint32_t i = 0; i < 3; ++i)
                if (_a[i] < 'w' || _a[i] > 'z')
                    throw std::invalid_argument(SVKFW_WRAPERR("vec4 :: op[] const (3)","index out of bounds"));
            const Util::remove_ref<C> *ptr[4]{&w, &x, &y, &z};
            return {*ptr[_a[0] - 'w'], *ptr[_a[1] - 'w'], *ptr[_a[2] - 'w']};
        }
        Vec4Base<C> operator[](const char (&_a)[4 + 1]) const {
            for (uint32_t i = 0; i < 4; ++i)
                if (_a[i] < 'w' || _a[i] > 'z')
                    throw std::invalid_argument(SVKFW_WRAPERR("vec4 :: op[] const (4)","index out of bounds"));
            const Util::remove_ref<C> *ptr[4]{&w, &x, &y, &z};
            return {*ptr[_a[0] - 'w'], *ptr[_a[1] - 'w'], *ptr[_a[2] - 'w'], *ptr[_a[3] - 'w']};
        }

    private:
        Util::remove_ref<C>& getElem(uint32_t index, std::false_type)       { return *((Util::remove_ref<C>*)this + index); }
        Util::remove_ref<C>  getElem(uint32_t index, std::false_type) const { return *((Util::remove_ref<C>*)this + index); }
        Util::remove_ref<C>& getElem(uint32_t index, std:: true_type) {
            switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                default:
                    throw std::invalid_argument(SVKFW_WRAPERR("vec4 :: op[] (1)","index out of bounds"));
            }
        }
        Util::remove_ref<C>  getElem(uint32_t index, std:: true_type) const {
            switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                default:
                    throw std::invalid_argument(SVKFW_WRAPERR("vec4 :: op[] const (1)","index out of bounds"));
            }
        }
    }; // Vec4Base END

    template <typename C, typename Y> Vec4Base<Util::comm_type<C,Y>> operator+(const Vec4Base<C> &op1, const Vec4Base<Y> &op2)
        { return {op1.x + op2.x, op1.y + op2.y, op1.z + op2.z, op1.w + op2.w}; }
    template <typename C, typename Y> Vec4Base<Util::comm_vecsub_type<C,Y>> operator-(const Vec4Base<C> &op1, const Vec4Base<Y> &op2)
        { return {op1.x - op2.x, op1.y - op2.y, op1.z - op2.z, op1.w - op2.w}; }
    template <typename C, typename Y> Vec4Base<Util::comm_type<C,Y>> operator*(const Vec4Base<C> &op1, const Vec4Base<Y> &op2)
        { return {op1.x * op2.x, op1.y * op2.y, op1.z * op2.z, op1.w * op2.w}; }
    template <typename C, typename Y> Vec4Base<Util::comm_type<C,Y>> operator/(const Vec4Base<C> &op1, const Vec4Base<Y> &op2)
        { return {op1.x / op2.x, op1.y / op2.y, op1.z / op2.z, op1.w / op2.w}; }

    template <typename C, typename Y> Vec4Base<Util::comm_type<C,Y>> operator+(const Vec4Base<C> &op1, Y op2)
        { return {op1.x + op2, op1.y + op2, op1.z + op2, op1.w + op2}; }
    template <typename C, typename Y> Vec4Base<Util::comm_vecsub_type<C,Y>> operator-(const Vec4Base<C> &op1, Y op2)
        { return {op1.x - op2, op1.y - op2, op1.z - op2, op1.w - op2}; }
    template <typename C, typename Y> Vec4Base<Util::comm_type<C,Y>> operator*(const Vec4Base<C> &op1, Y op2)
        { return {op1.x * op2, op1.y * op2, op1.z * op2, op1.w * op2}; }
    template <typename C, typename Y> Vec4Base<Util::comm_type<C,Y>> operator/(const Vec4Base<C> &op1, Y op2)
        { return {op1.x / op2, op1.y / op2, op1.z / op2, op1.w / op2}; }

    template <typename C, typename Y> Vec4Base<Util::comm_type<C,Y>> operator+(Y op1, const Vec4Base<C> &op2)
        { return {op1 + op2.x, op1 + op2.y, op1 + op2.z, op1 + op2.w}; }
    template <typename C, typename Y> Vec4Base<Util::comm_vecsub_type<C,Y>> operator-(Y op1, const Vec4Base<C> &op2)
        { return {op1 - op2.x, op1 - op2.y, op1 - op2.z, op1 - op2.w}; }
    template <typename C, typename Y> Vec4Base<Util::comm_type<C,Y>> operator*(Y op1, const Vec4Base<C> &op2)
        { return {op1 * op2.x, op1 * op2.y, op1 * op2.z, op1 * op2.w}; }
    template <typename C, typename Y> Vec4Base<Util::comm_type<C,Y>> operator/(Y op1, const Vec4Base<C> &op2)
        { return {op1 / op2.x, op1 / op2.y, op1 / op2.z, op1 / op2.w}; }


    template <typename C, typename Y> Vec4Base<bool> operator> (const C &op1, const Vec4Base<Y> &op2)
        { return {op1 >  op2.x, op1 >  op2.y, op1 >  op2.z, op1 >  op2.w}; }
    template <typename C, typename Y> Vec4Base<bool> operator< (const C &op1, const Vec4Base<Y> &op2)
        { return {op1 <  op2.x, op1 <  op2.y, op1 <  op2.z, op1 <  op2.w}; }
    template <typename C, typename Y> Vec4Base<bool> operator>=(const C &op1, const Vec4Base<Y> &op2)
        { return {op1 >= op2.x, op1 >= op2.y, op1 >= op2.z, op1 >= op2.w}; }
    template <typename C, typename Y> Vec4Base<bool> operator<=(const C &op1, const Vec4Base<Y> &op2)
        { return {op1 <= op2.x, op1 <= op2.y, op1 <= op2.z, op1 <= op2.w}; }
    template <typename C, typename Y> Vec4Base<bool> operator==(const C &op1, const Vec4Base<Y> &op2)
        { return {op1 == op2.x, op1 == op2.y, op1 == op2.z, op1 == op2.w}; }
    template <typename C, typename Y> Vec4Base<bool> operator!=(const C &op1, const Vec4Base<Y> &op2)
        { return {op1 != op2.x, op1 != op2.y, op1 != op2.z, op1 != op2.w}; }

    template <typename C, typename Y> Vec4Base<bool> operator> (const Vec4Base<C> &op1, const Y &op2)
        { return {op1.x >  op2, op1.y >  op2, op1.z >  op2, op1.w >  op2}; }
    template <typename C, typename Y> Vec4Base<bool> operator< (const Vec4Base<C> &op1, const Y &op2)
        { return {op1.x <  op2, op1.y <  op2, op1.z <  op2, op1.w <  op2}; }
    template <typename C, typename Y> Vec4Base<bool> operator>=(const Vec4Base<C> &op1, const Y &op2)
        { return {op1.x >= op2, op1.y >= op2, op1.z >= op2, op1.w >= op2}; }
    template <typename C, typename Y> Vec4Base<bool> operator<=(const Vec4Base<C> &op1, const Y &op2)
        { return {op1.x <= op2, op1.y <= op2, op1.z <= op2, op1.w <= op2}; }
    template <typename C, typename Y> Vec4Base<bool> operator==(const Vec4Base<C> &op1, const Y &op2)
        { return {op1.x == op2, op1.y == op2, op1.z == op2, op1.w == op2}; }
    template <typename C, typename Y> Vec4Base<bool> operator!=(const Vec4Base<C> &op1, const Y &op2)
        { return {op1.x != op2, op1.y != op2, op1.z != op2, op1.w != op2}; }

    template <typename C, typename Y> Vec4Base<bool> operator> (const Vec4Base<C> &op1, const Vec4Base<Y> &op2)
        { return {op1.x >  op2.x, op1.y >  op2.y, op1.z >  op2.z, op1.w >  op2.w}; }
    template <typename C, typename Y> Vec4Base<bool> operator< (const Vec4Base<C> &op1, const Vec4Base<Y> &op2)
        { return {op1.x <  op2.x, op1.y <  op2.y, op1.z <  op2.z, op1.w <  op2.w}; }
    template <typename C, typename Y> Vec4Base<bool> operator>=(const Vec4Base<C> &op1, const Vec4Base<Y> &op2)
        { return {op1.x >= op2.x, op1.y >= op2.y, op1.z >= op2.z, op1.w >= op2.w}; }
    template <typename C, typename Y> Vec4Base<bool> operator<=(const Vec4Base<C> &op1, const Vec4Base<Y> &op2)
        { return {op1.x <= op2.x, op1.y <= op2.y, op1.z <= op2.z, op1.w <= op2.w}; }
    template <typename C, typename Y> Vec4Base<bool> operator==(const Vec4Base<C> &op1, const Vec4Base<Y> &op2)
        { return {op1.x == op2.x, op1.y == op2.y, op1.z == op2.z, op1.w == op2.w}; }
    template <typename C, typename Y> Vec4Base<bool> operator!=(const Vec4Base<C> &op1, const Vec4Base<Y> &op2)
        { return {op1.x != op2.x, op1.y != op2.y, op1.z != op2.z, op1.w != op2.w}; }


//  ============  Vector utilities  ============  \\

    namespace Util {
        template <typename T, int Ch>
        struct VecConstructorSct {};
        template <typename T>
        struct VecConstructorSct<T, 2> { using type = Vec2Base<T>; };
        template <typename T>
        struct VecConstructorSct<T, 3> { using type = Vec3Base<T>; };
        template <typename T>
        struct VecConstructorSct<T, 4> { using type = Vec4Base<T>; };
        
        template <typename T, int Ch>
        using VecConstructor = typename VecConstructorSct<T, Ch>::type;
    }; // Util END
    

    namespace Math {
        template <typename Y>
        inline Util::remove_ref_c<Y> minOf(const Vec2Base<Y> &op1)
            { return std::min(op1.x, op1.y); }
        template <typename Y>
        inline Util::remove_ref_c<Y> maxOf(const Vec2Base<Y> &op1)
            { return std::max(op1.x, op1.y); }

        template <typename Y>
        inline Util::remove_ref_c<Y> minOf(const Vec3Base<Y> &op1)
            { return std::min(std::min(op1.x, op1.y), op1.z); }
        template <typename Y>
        inline Util::remove_ref_c<Y> maxOf(const Vec3Base<Y> &op1)
            { return std::min(std::min(op1.x, op1.y), op1.z); }

        template <typename Y>
        inline Util::remove_ref_c<Y> minOf(const Vec4Base<Y> &op1)
            { return std::min(std::min(std::min(op1.x, op1.y), op1.z), op1.w); }
        template <typename Y>
        inline Util::remove_ref_c<Y> maxOf(const Vec4Base<Y> &op1)
            { return std::max(std::max(std::max(op1.x, op1.y), op1.z), op1.w); }



        template <typename Y, typename Z>
        inline typename std::enable_if<Util::is_same_base<Y, Z>::value, Vec2Base<Util::remove_ref_c<Y>>>::type
        min(const Vec2Base<Y> &op1, const Vec2Base<Z> &op2)
            { return {op1.x < op2.x ? op1.x : op2.x, op1.y < op2.y ? op1.y : op2.y}; }
        template <typename Y, typename Z>
        inline typename std::enable_if<Util::is_same_base<Y, Z>::value, Vec2Base<Util::remove_ref_c<Y>>>::type
        max(const Vec2Base<Y> &op1, const Vec2Base<Z> &op2)
            { return {op1.x > op2.x ? op1.x : op2.x, op1.y > op2.y ? op1.y : op2.y}; }

        template <typename Y, typename Z>
        inline typename std::enable_if<Util::is_same_base<Y, Z>::value, Vec3Base<Util::remove_ref_c<Y>>>::type
        min(const Vec3Base<Y> &op1, const Vec3Base<Z> &op2)
            { return {op1.x < op2.x ? op1.x : op2.x, op1.y < op2.y ? op1.y : op2.y, op1.z < op2.z ? op1.z : op2.z}; }
        template <typename Y, typename Z>
        inline typename std::enable_if<Util::is_same_base<Y, Z>::value, Vec3Base<Util::remove_ref_c<Y>>>::type
        max(const Vec3Base<Y> &op1, const Vec3Base<Z> &op2)
            { return {op1.x > op2.x ? op1.x : op2.x, op1.y > op2.y ? op1.y : op2.y, op1.z > op2.z ? op1.z : op2.z}; }

        template <typename Y, typename Z>
        inline typename std::enable_if<Util::is_same_base<Y, Z>::value, Vec4Base<Util::remove_ref_c<Y>>>::type
        min(const Vec4Base<Y> &op1, const Vec4Base<Z> &op2)
            { return {op1.x < op2.x ? op1.x : op2.x, op1.y < op2.y ? op1.y : op2.y, op1.z < op2.z ? op1.z : op2.z, op1.w < op2.w ? op1.w : op2.w}; }
        template <typename Y, typename Z>
        inline typename std::enable_if<Util::is_same_base<Y, Z>::value, Vec4Base<Util::remove_ref_c<Y>>>::type
        max(const Vec4Base<Y> &op1, const Vec4Base<Z> &op2)
            { return {op1.x > op2.x ? op1.x : op2.x, op1.y > op2.y ? op1.y : op2.y, op1.z > op2.z ? op1.z : op2.z, op1.w > op2.w ? op1.w : op2.w}; }


        template <typename C>
        flt_t sum(const C &op1)
            { return op1; }
        template <typename C>
        flt_t sum(const Vec2Base<C> &op1)
            { return op1.x + op1.y; }
        template <typename C>
        flt_t sum(const Vec3Base<C> &op1)
            { return op1.x + op1.y + op1.z; }
        template <typename C>
        flt_t sum(const Vec4Base<C> &op1)
            { return op1.x + op1.y + op1.z + op1.w; }

        template <typename C>
        flt_t pow(const C &op1, float _pow)
            { return std::pow(op1, _pow); }
        template <typename C>
        Vec2Base<float> pow(const Vec2Base<C> &op1, float _pow)
            { return {std::pow(op1.x, _pow), std::pow(op1.y, _pow)}; }
        template <typename C>
        Vec3Base<float> pow(const Vec3Base<C> &op1, float _pow)
            { return {std::pow(op1.x, _pow), std::pow(op1.y, _pow), std::pow(op1.z, _pow)}; }
        template <typename C>
        Vec4Base<float> pow(const Vec4Base<C> &op1, float _pow)
            { return {std::pow(op1.x, _pow), std::pow(op1.y, _pow), std::pow(op1.z, _pow), std::pow(op1.w, _pow)}; }

        template <typename C, typename Y>
        flt_t dot(const C &op1, const Y &op2)
            { return op1 * op2; }
        template <typename C, typename Y>
        flt_t dot(const Vec2Base<C> &op1, const Vec2Base<Y> &op2)
            { return op1.x * op2.x + op1.y * op2.y; }
        template <typename C, typename Y>
        flt_t dot(const Vec3Base<C> &op1, const Vec3Base<Y> &op2)
            { return op1.x * op2.x + op1.y * op2.y + op1.z * op2.z; }
        template <typename C, typename Y>
        flt_t dot(const Vec4Base<C> &op1, const Vec4Base<Y> &op2)
            { return op1.x * op2.x + op1.y * op2.y + op1.z * op2.z + op1.w * op2.w; }

        template <typename C, typename Y>
        flt_t dist(const C &op1, const Y &op2) { return std::abs(op1 - op2); }
        template <typename C, typename Y>
        flt_t dist(const Vec2Base<C> &op1, const Vec2Base<Y> &op2) { return (op1 - op2).norm(); }
        template <typename C, typename Y>
        flt_t dist(const Vec3Base<C> &op1, const Vec3Base<Y> &op2) { return (op1 - op2).norm(); }
        template <typename C, typename Y>
        flt_t dist(const Vec4Base<C> &op1, const Vec4Base<Y> &op2) { return (op1 - op2).norm(); }

        template <typename C, typename Y>
        Vec3Base<Util::comm_vecsub_type<C,Y>> cross(const Vec3Base<C> &op1, const Vec3Base<Y> &op2)
            { return {op1.y * op2.z - op1.z * op2.y, op1.z * op2.x - op1.x * op2.z, op1.x * op2.y - op1.y * op2.x}; }


        template <typename T>
        Vec2Base<T> clampCL(Vec2Base<T> _val, Vec2Base<T> _left, Vec2Base<T> _right) { return { clampCL(_val.x, _left.x, _right.x),
                                                                                                clampCL(_val.y, _left.y, _right.y) }; }
        template <typename T>
        Vec3Base<T> clampCL(Vec3Base<T> _val, Vec3Base<T> _left, Vec3Base<T> _right) { return { clampCL(_val.x, _left.x, _right.x),
                                                                                                clampCL(_val.y, _left.y, _right.y),
                                                                                                clampCL(_val.z, _left.z, _right.z) }; }
        template <typename T>
        Vec4Base<T> clampCL(Vec4Base<T> _val, Vec4Base<T> _left, Vec4Base<T> _right) { return { clampCL(_val.x, _left.x, _right.x),
                                                                                                clampCL(_val.y, _left.y, _right.y),
                                                                                                clampCL(_val.z, _left.z, _right.z),
                                                                                                clampCL(_val.w, _left.w, _right.w) }; }
        template <typename T>
        Vec2Base<T> clampRO(Vec2Base<T> _val, Vec2Base<T> _left, Vec2Base<T> _right, Vec2Base<T> _step = 1) { return { clampRO(_val.x, _left.x, _right.x, _step.x),
                                                                                                                       clampRO(_val.y, _left.y, _right.y, _step.y) }; }
        template <typename T>
        Vec3Base<T> clampRO(Vec3Base<T> _val, Vec3Base<T> _left, Vec3Base<T> _right, Vec2Base<T> _step = 1) { return { clampRO(_val.x, _left.x, _right.x, _step.x),
                                                                                                                       clampRO(_val.y, _left.y, _right.y, _step.y),
                                                                                                                       clampRO(_val.z, _left.z, _right.z, _step.z) }; }
        template <typename T>
        Vec4Base<T> clampRO(Vec4Base<T> _val, Vec4Base<T> _left, Vec4Base<T> _right, Vec2Base<T> _step = 1) { return { clampRO(_val.x, _left.x, _right.x, _step.x),
                                                                                                                       clampRO(_val.y, _left.y, _right.y, _step.y),
                                                                                                                       clampRO(_val.z, _left.z, _right.z, _step.z),
                                                                                                                       clampRO(_val.w, _left.w, _right.w, _step.w) }; }
        template <typename T>
        Vec2Base<T> clampLO(Vec2Base<T> _val, Vec2Base<T> _left, Vec2Base<T> _right, Vec2Base<T> _step = 1) { return { clampLO(_val.x, _left.x, _right.x, _step.x),
                                                                                                                       clampLO(_val.y, _left.y, _right.y, _step.y) }; }
        template <typename T>
        Vec3Base<T> clampLO(Vec3Base<T> _val, Vec3Base<T> _left, Vec3Base<T> _right, Vec2Base<T> _step = 1) { return { clampLO(_val.x, _left.x, _right.x, _step.x),
                                                                                                                       clampLO(_val.y, _left.y, _right.y, _step.y),
                                                                                                                       clampLO(_val.z, _left.z, _right.z, _step.z) }; }
        template <typename T>
        Vec4Base<T> clampLO(Vec4Base<T> _val, Vec4Base<T> _left, Vec4Base<T> _right, Vec2Base<T> _step = 1) { return { clampLO(_val.x, _left.x, _right.x, _step.x),
                                                                                                                       clampLO(_val.y, _left.y, _right.y, _step.y),
                                                                                                                       clampLO(_val.z, _left.z, _right.z, _step.z),
                                                                                                                       clampLO(_val.w, _left.w, _right.w, _step.w) }; }
        template <typename T>
        Vec2Base<T> clampOP(Vec2Base<T> _val, Vec2Base<T> _left, Vec2Base<T> _right, Vec2Base<T> _step = 1) { return { clampOP(_val.x, _left.x, _right.x, _step.x),
                                                                                                                       clampOP(_val.y, _left.y, _right.y, _step.y) }; }
        template <typename T>
        Vec3Base<T> clampOP(Vec3Base<T> _val, Vec3Base<T> _left, Vec3Base<T> _right, Vec2Base<T> _step = 1) { return { clampOP(_val.x, _left.x, _right.x, _step.x),
                                                                                                                       clampOP(_val.y, _left.y, _right.y, _step.y),
                                                                                                                       clampOP(_val.z, _left.z, _right.z, _step.z) }; }
        template <typename T>
        Vec4Base<T> clampOP(Vec4Base<T> _val, Vec4Base<T> _left, Vec4Base<T> _right, Vec2Base<T> _step = 1) { return { clampOP(_val.x, _left.x, _right.x, _step.x),
                                                                                                                       clampOP(_val.y, _left.y, _right.y, _step.y),
                                                                                                                       clampOP(_val.z, _left.z, _right.z, _step.z),
                                                                                                                       clampOP(_val.w, _left.w, _right.w, _step.w) }; }
    }; // Math END


//  ================================  \\
                Matrices
//  ================================  \\

    namespace Mat {
        const struct Placeholder { const char _ = '_'; } _;
    }; // Mat END


//  ============  2x2 Matrix  ============  \\

    template <class C>
    struct Mat2Base {
        C M[2][2];

        Mat2Base() : M{ {0, 0}, {0, 0} } {}
        Mat2Base(C m0, C m1, C m2, C m3) : M{{ m0, m1 }, { m2, m3 }} {}
        template <class Y> Mat2Base(const Mat2Base<Y> &_mat) {
            for (uint32_t i = 0u; i < 4u; ++i)
                M[i >> 1][i & 1] = static_cast<C>(_mat.M[i >> 1][i & 1]);
        }
        Mat2Base(C _val) {
            for (uint32_t i = 0u; i < 4u; ++i)
                M[i >> 1][i & 1] = _val;
        }


        // template <typename Y> Mat2Base<Y> cast() { return *this; }
        template <typename Y> Mat2Base<Util::remove_ref<Y>> cast() const { return *this; }

        void transpose() { swap(M[0][1], M[1][0]); swap(M[0][2], M[2][0]); swap(M[1][2], M[2][1]); }
        Mat2Base <C> T() const { return { M[0][0], M[1][0], M[0][1], M[1][1] }; }

        template <typename Y = double>
        Y det() const { return M[0][0] * M[1][1] - M[0][1] * M[1][0]; }

        bool invertible() const { return !Math::closeToZero(this->det(), SVKFW_EPS4); }

        template <typename Y = flt_t>
        Mat2Base <Y> inverse() const {
            Y determinant = this->det<Y>();
            if (!Math::closeToZero(determinant, SVKFW_EPS4)) {
                determinant = 1. / determinant;
                return { M[1][1] * determinant, -M[0][1] * determinant,
                        -M[1][0] * determinant,  M[0][0] * determinant };
            }
            return *this;
        }

        Mat2Base<C> operator+() const
            { return { abs(M[0][0]), abs(M[0][1]), abs(M[1][0]), abs(M[1][1]) }; }
        Mat2Base<C> operator-() const
            { return { -M[0][0], -M[0][1], -M[1][0], -M[1][1] }; }

        template <class Y> Mat2Base<Util::comm_type<C,Y>> operator+(const Mat2Base<Y> &_m) const
            { return { M[0][0] + _m.M[0][0], M[0][1] + _m.M[0][1], M[1][0] + _m.M[1][0], M[1][1] + _m.M[1][1] }; }
        template <class Y> Mat2Base<Util::comm_vecsub_type<C,Y>> operator-(const Mat2Base<Y> &_m) const
            { return { M[0][0] - _m.M[0][0], M[0][1] - _m.M[0][1], M[1][0] - _m.M[1][0], M[1][1] - _m.M[1][1] }; }
        template <class Y> Mat2Base<Util::comm_type<C,Y>> operator*(const Mat2Base<Y> &_m) const
            { return { M[0][0] * _m.M[0][0], M[0][1] * _m.M[0][1], M[1][0] * _m.M[1][0], M[1][1] * _m.M[1][1] }; }
        template <class Y> Mat2Base<Util::comm_type<C,Y>> operator/(const Mat2Base<Y> &_m) const
            { return { M[0][0] / _m.M[0][0], M[0][1] / _m.M[0][1], M[1][0] / _m.M[1][0], M[1][1] / _m.M[1][1] }; }

        template <class Y> void operator+=(const Mat2Base<Y> &_m) { for (uint32_t i=0; i<2u; ++i) for (uint32_t j=0; j<2u; ++j) M[i][j] += _m.M[i][j]; }
        template <class Y> void operator-=(const Mat2Base<Y> &_m) { for (uint32_t i=0; i<2u; ++i) for (uint32_t j=0; j<2u; ++j) M[i][j] -= _m.M[i][j]; }
        template <class Y> void operator*=(const Mat2Base<Y> &_m) { for (uint32_t i=0; i<2u; ++i) for (uint32_t j=0; j<2u; ++j) M[i][j] *= _m.M[i][j]; }
        template <class Y> void operator/=(const Mat2Base<Y> &_m) { for (uint32_t i=0; i<2u; ++i) for (uint32_t j=0; j<2u; ++j) M[i][j] /= _m.M[i][j]; }
        void operator+=(C _op) { for (uint32_t i=0; i<2u; ++i) for (uint32_t j=0; j<2u; ++j) M[i][j] += _op; }
        void operator-=(C _op) { for (uint32_t i=0; i<2u; ++i) for (uint32_t j=0; j<2u; ++j) M[i][j] -= _op; }
        void operator*=(C _op) { for (uint32_t i=0; i<2u; ++i) for (uint32_t j=0; j<2u; ++j) M[i][j] *= _op; }
        void operator/=(C _op) { for (uint32_t i=0; i<2u; ++i) for (uint32_t j=0; j<2u; ++j) M[i][j] /= _op; }

        Mat2Base<C> operator+(C _op) const
            { return { M[0][0] + _op, M[0][1] + _op, M[1][0] + _op, M[1][1] + _op }; }
        Mat2Base<C> operator-(C _op) const
            { return { M[0][0] - _op, M[0][1] - _op, M[1][0] - _op, M[1][1] - _op }; }
        Mat2Base<C> operator*(C _op) const
            { return { M[0][0] * _op, M[0][1] * _op, M[1][0] * _op, M[1][1] * _op }; }
        Mat2Base<C> operator/(C _op) const
            { return { M[0][0] / _op, M[0][1] / _op, M[1][0] / _op, M[1][1] / _op }; }

        template <class Y> Vec2Base<Util::comm_type<C,Y>> operator&(const Vec2Base<Y> &vec) const {
            return { vec.x * M[0][0] + vec.y * M[0][1], vec.x * M[1][0] + vec.y * M[1][1] };
        }
        template <class Y> Mat2Base<Util::comm_type<C,Y>> operator&(const Mat2Base<Y> &_m) const {
            return { _m.M[0][0] * M[0][0] + _m.M[1][0] * M[0][1],
                     _m.M[0][1] * M[0][0] + _m.M[1][1] * M[0][1],
                     _m.M[0][0] * M[1][0] + _m.M[1][0] * M[1][1],
                     _m.M[0][1] * M[1][0] + _m.M[1][1] * M[1][1] };
        }
        template <class Y> void operator&=(const Mat2Base<Y> &_m) {
            *this= { _m.M[0][0] * M[0][0] + _m.M[1][0] * M[0][1],
                     _m.M[0][1] * M[0][0] + _m.M[1][1] * M[0][1],
                     _m.M[0][0] * M[1][0] + _m.M[1][0] * M[1][1],
                     _m.M[0][1] * M[1][0] + _m.M[1][1] * M[1][1] };
        }

        C& operator[](vec2i ind)       { return M[ind.x][ind.y]; }
        C  operator[](vec2i ind) const { return M[ind.x][ind.y]; }
        Vec2Base<C&> operator[](uint32_t ind)       { return { M[ind][0], M[ind][1] }; }
        Vec2Base<C > operator[](uint32_t ind) const { return { M[ind][0], M[ind][1] }; }
        Mat2RefBase<C> operator[](Mat::Placeholder) { return {  &M[0][0],  &M[1][1] }; }
        const Mat2RefBase<const C> operator[](Mat::Placeholder) const { return { &M[0][0], &M[1][1] }; }
    }; // Mat2Base END

    template <class C>
    struct Mat2RefBase {
        C *c[2];

        Mat2RefBase(C *c1, C *c2) : c{c1, c2} {}

        Vec2Base<C&> operator[](uint32_t ind) { return { *c[ind], *(c[ind] + 2) }; }
        Vec2Base<typename std::remove_const<C>::type> operator[](uint32_t ind) const { return { *c[ind], *(c[ind] + 2) }; }
    }; // Mat2RefBase END


//  ============  3x3 Matrix  ============  \\

    template <class C>
    struct Mat3Base {
        C M[3][3];

        Mat3Base() : M{ {0, 0, 0}, {0, 0, 0}, {0, 0, 0} } {}
        Mat3Base(C m0, C m1, C m2, C m3, C m4, C m5, C m6, C m7, C m8) :
            M{{ m0, m1, m2 }, { m3, m4, m5 }, { m6, m7, m8 }} {}
        template <class Y> Mat3Base(const Mat3Base<Y> &_mat) {
            for (uint32_t i = 0u; i < 3u; ++i)
                for (uint32_t j = 0u; j < 3u; ++j)
                    M[i][j] = static_cast<C>(_mat.M[i][j]);
        }
        Mat3Base(C _val) {
            for (uint32_t i = 0u; i < 3u; ++i)
                for (uint32_t j = 0u; j < 3u; ++j)
                    M[i][j] = _val;
        }


        // template <typename Y> Mat3Base<Y> cast() { return *this; }
        template <typename Y> Mat3Base<Util::remove_ref<Y>> cast() const { return *this; }

        void transpose() { swap(M[0][1], M[1][0]); swap(M[0][2], M[2][0]); swap(M[1][2], M[2][1]); }
        Mat3Base <C> T() const { return { M[0][0], M[1][0], M[2][0], M[0][1], M[1][1], M[2][1], M[0][2], M[1][2], M[2][2] }; }

        template <typename Y = double>
        Y det() const { return M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1]) -
                               M[0][1] * (M[1][0] * M[2][2] - M[1][2] * M[2][0]) +
                               M[0][2] * (M[1][0] * M[2][1] - M[1][1] * M[2][0]); }

        bool invertible() const { return !Math::closeToZero(this->det(), SVKFW_EPS4); }

        template <typename Y = flt_t>
        Mat3Base<Y> inverse() const {
            Y tmp[3]{M[1][1] * M[2][2] - M[1][2] * M[2][1], M[1][2] * M[2][0] - M[1][0] * M[2][2], M[1][0] * M[2][1] - M[1][1] * M[2][0]};
            Y determinant = M[0][0] * tmp[0] + M[0][1] * tmp[1] + M[0][2] * tmp[2];

            if (!Math::closeToZero(determinant, SVKFW_EPS4)) {
                determinant = 1. / determinant;
                return Mat3Base<Y>(tmp[0], M[0][2] * M[2][1] - M[0][1] * M[2][2], M[0][1] * M[1][2] - M[0][2] * M[1][1],
                                   tmp[1], M[0][0] * M[2][2] - M[0][2] * M[2][0], M[0][2] * M[1][0] - M[0][0] * M[1][2],
                                   tmp[2], M[0][1] * M[2][0] - M[0][0] * M[2][1], M[0][0] * M[1][1] - M[0][1] * M[1][0])
                    * determinant;
            }

            return *this;
        }

        Mat3Base<C> operator+() const
            { return {abs(M[0][0]), abs(M[0][1]), abs(M[0][2]),
                      abs(M[1][0]), abs(M[1][1]), abs(M[1][2]),
                      abs(M[2][0]), abs(M[2][1]), abs(M[2][2])}; }
        Mat3Base<C> operator-() const
            { return { -M[0][0], -M[0][1], -M[0][2],
                       -M[1][0], -M[1][1], -M[1][2],
                       -M[2][0], -M[2][1], -M[2][2] }; }

        template <class Y> Mat3Base<Util::comm_type<C,Y>> operator+(const Mat3Base<Y> &_m) const
            { return { M[0][0] + _m.M[0][0], M[0][1] + _m.M[0][1], M[0][2] + _m.M[0][2],
                       M[1][0] + _m.M[1][0], M[1][1] + _m.M[1][1], M[1][2] + _m.M[1][2],
                       M[2][0] + _m.M[2][0], M[2][1] + _m.M[2][1], M[2][2] + _m.M[2][2] }; }
        template <class Y> Mat3Base<Util::comm_vecsub_type<C,Y>> operator-(const Mat3Base<Y> &_m) const
            { return { M[0][0] - _m.M[0][0], M[0][1] - _m.M[0][1], M[0][2] - _m.M[0][2],
                       M[1][0] - _m.M[1][0], M[1][1] - _m.M[1][1], M[1][2] - _m.M[1][2],
                       M[2][0] - _m.M[2][0], M[2][1] - _m.M[2][1], M[2][2] - _m.M[2][2] }; }
        template <class Y> Mat3Base<Util::comm_type<C,Y>> operator*(const Mat3Base<Y> &_m) const
            { return { M[0][0] * _m.M[0][0], M[0][1] * _m.M[0][1], M[0][2] * _m.M[0][2],
                       M[1][0] * _m.M[1][0], M[1][1] * _m.M[1][1], M[1][2] * _m.M[1][2],
                       M[2][0] * _m.M[2][0], M[2][1] * _m.M[2][1], M[2][2] * _m.M[2][2] }; }
        template <class Y> Mat3Base<Util::comm_type<C,Y>> operator/(const Mat3Base<Y> &_m) const
            { return { M[0][0] / _m.M[0][0], M[0][1] / _m.M[0][1], M[0][2] / _m.M[0][2],
                       M[1][0] / _m.M[1][0], M[1][1] / _m.M[1][1], M[1][2] / _m.M[1][2],
                       M[2][0] / _m.M[2][0], M[2][1] / _m.M[2][1], M[2][2] / _m.M[2][2] }; }

        template <class Y> void operator+=(const Mat3Base<Y> &_m) { for (uint32_t i=0; i<3u; ++i) for (uint32_t j=0; j<3u; ++j) M[i][j] += _m.M[i][j]; }
        template <class Y> void operator-=(const Mat3Base<Y> &_m) { for (uint32_t i=0; i<3u; ++i) for (uint32_t j=0; j<3u; ++j) M[i][j] -= _m.M[i][j]; }
        template <class Y> void operator*=(const Mat3Base<Y> &_m) { for (uint32_t i=0; i<3u; ++i) for (uint32_t j=0; j<3u; ++j) M[i][j] *= _m.M[i][j]; }
        template <class Y> void operator/=(const Mat3Base<Y> &_m) { for (uint32_t i=0; i<3u; ++i) for (uint32_t j=0; j<3u; ++j) M[i][j] /= _m.M[i][j]; }
        void operator+=(C _op) { for (uint32_t i=0; i<3u; ++i) for (uint32_t j=0; j<3u; ++j) M[i][j] += _op; }
        void operator-=(C _op) { for (uint32_t i=0; i<3u; ++i) for (uint32_t j=0; j<3u; ++j) M[i][j] -= _op; }
        void operator*=(C _op) { for (uint32_t i=0; i<3u; ++i) for (uint32_t j=0; j<3u; ++j) M[i][j] *= _op; }
        void operator/=(C _op) { for (uint32_t i=0; i<3u; ++i) for (uint32_t j=0; j<3u; ++j) M[i][j] /= _op; }

        Mat3Base<C> operator+(C _op) const
            { return { M[0][0] + _op, M[0][1] + _op, M[0][2] + _op,
                       M[1][0] + _op, M[1][1] + _op, M[1][2] + _op,
                       M[2][0] + _op, M[2][1] + _op, M[2][2] + _op }; }
        Mat3Base<C> operator-(C _op) const
            { return { M[0][0] - _op, M[0][1] - _op, M[0][2] - _op,
                       M[1][0] - _op, M[1][1] - _op, M[1][2] - _op,
                       M[2][0] - _op, M[2][1] - _op, M[2][2] - _op }; }
        Mat3Base<C> operator*(C _op) const
            { return { M[0][0] * _op, M[0][1] * _op, M[0][2] * _op,
                       M[1][0] * _op, M[1][1] * _op, M[1][2] * _op,
                       M[2][0] * _op, M[2][1] * _op, M[2][2] * _op }; }
        Mat3Base<C> operator/(C _op) const
            { return { M[0][0] / _op, M[0][1] / _op, M[0][2] / _op,
                       M[1][0] / _op, M[1][1] / _op, M[1][2] / _op,
                       M[2][0] / _op, M[2][1] / _op, M[2][2] / _op }; }

        template <class Y> Vec3Base<Util::comm_type<C,Y>> operator&(const Vec3Base<Y> &vec) const {
            return { vec.x * M[0][0] + vec.y * M[0][1] + vec.z * M[0][2],
                     vec.x * M[1][0] + vec.y * M[1][1] + vec.z * M[1][2],
                     vec.x * M[2][0] + vec.y * M[2][1] + vec.z * M[2][2] };
        }
        template <class Y> Mat3Base<Util::comm_type<C,Y>> operator&(const Mat3Base<Y> &_m) const {
            Mat3Base<Util::comm_type<C,Y>> tmp;
            for (uint32_t i = 0; i <= 6; i += 3)
                for (uint32_t j = 0; j < 3; ++j)
                    (*tmp.M)[i + j] = (*_m.M)[j] * (*M)[i] + (*_m.M)[j + 3] * (*M)[i + 1] + (*_m.M)[j + 6] * (*M)[i + 2];
            return tmp;
        }
        template <class Y> void operator&=(const Mat3Base<Y> &_m) {
            for (uint32_t i = 0; i <= 6; i += 3)
                for (uint32_t j = 0; j < 3; ++j)
                    (*M)[i + j] = (*_m.M)[j] * (*M)[i] + (*_m.M)[j + 3] * (*M)[i + 1] + (*_m.M)[j + 6] * (*M)[i + 2];
        }

        C& operator[](vec2i ind)       { return M[ind.x][ind.y]; }
        C  operator[](vec2i ind) const { return M[ind.x][ind.y]; }
        Vec3Base<C&> operator[](uint32_t ind)       { return { M[ind][0], M[ind][1], M[ind][2] }; }
        Vec3Base<C > operator[](uint32_t ind) const { return { M[ind][0], M[ind][1], M[ind][2] }; }
        Mat3RefBase<C> operator[](Mat::Placeholder) { return {  &M[0][0],  &M[0][1],  &M[0][2] }; }
        const Mat3RefBase<const C> operator[](Mat::Placeholder) const { return { &M[0][0], &M[0][1], &M[0][2] }; }
    }; // Mat3Base END

    template <class C>
    struct Mat3RefBase {
        C *c[3];

        Mat3RefBase(C *c1, C *c2, C *c3) : c{c1, c2, c3} {}

        Vec3Base<C&> operator[](uint32_t ind) { return { *c[ind], *(c[ind] + 3), *(c[ind] + 6) }; }
        Vec3Base<typename std::remove_const<C>::type> operator[](uint32_t ind) const { return { *c[ind], *(c[ind] + 3), *(c[ind] + 6) }; }
    }; // Mat3RefBase END


//  ============  4x4 Matrix  ============  \\

    template <class C>
    struct Mat4Base {
        C M[4][4];

        Mat4Base() : M{ {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} } {}
        Mat4Base(C m0,  C m1,  C m2,  C m3,
                 C m4,  C m5,  C m6,  C m7,
                 C m8,  C m9,  C m10, C m11,
                 C m12, C m13, C m14, C m15) :
            M{{ m0,  m1,  m2,  m3  },
              { m4,  m5,  m6,  m7  },
              { m8,  m9,  m10, m11 },
              { m12, m13, m14, m15 }} {}
        template <class Y> Mat4Base(const Mat4Base<Y> &_m) {
            for (uint32_t i = 0u; i < 4u; ++i)
                for (uint32_t j = 0u; j < 4u; ++j)
                    M[i][j] = static_cast<C>(_m.M[i][j]);
        }
        Mat4Base(C _val) {
            for (uint32_t i = 0u; i < 4u; ++i)
                for (uint32_t j = 0u; j < 4u; ++j)
                    M[i][j] = _val;
        }


        // template <typename Y> Mat4Base<Y> cast() { return *this; }
        template <typename Y> Mat4Base<Util::remove_ref<Y>> cast() const { return *this; }

        void transpose() { swap(M[0][1], M[1][0]); swap(M[0][2], M[2][0]); swap(M[0][3], M[3][0]);
                           swap(M[1][2], M[2][1]); swap(M[1][3], M[3][1]); swap(M[2][3], M[3][2]); }
        Mat4Base <C> T() const { return { M[0][0], M[1][0], M[2][0], M[3][0],
                                          M[0][1], M[1][1], M[2][1], M[3][1],
                                          M[0][2], M[1][2], M[2][2], M[3][2],
                                          M[0][3], M[1][3], M[2][3], M[3][3] }; }

        template <typename Y = double>
        Y det() const {
            Y A2323 = M[2][2] * M[3][3] - M[2][3] * M[3][2];
            Y A1323 = M[2][1] * M[3][3] - M[2][3] * M[3][1];
            Y A1223 = M[2][1] * M[3][2] - M[2][2] * M[3][1];
            Y A0323 = M[2][0] * M[3][3] - M[2][3] * M[3][0];
            Y A0223 = M[2][0] * M[3][2] - M[2][2] * M[3][0];
            Y A0123 = M[2][0] * M[3][1] - M[2][1] * M[3][0];

            return M[0][0] * ( M[1][1] * A2323 - M[1][2] * A1323 + M[1][3] * A1223 )
                 - M[0][1] * ( M[1][0] * A2323 - M[1][2] * A0323 + M[1][3] * A0223 )
                 + M[0][2] * ( M[1][0] * A1223 - M[1][1] * A0223 + M[1][2] * A0123 )
                 - M[0][3] * ( M[1][0] * A1223 - M[1][1] * A0223 + M[1][2] * A0123 );
        }

        bool invertible() const { return !Math::closeToZero(this->det(), SVKFW_EPS4); }

        template <typename Y = flt_t>
        Mat4Base<Y> inverse() const {
            Y A2323 = M[2][2] * M[3][3] - M[2][3] * M[3][2];
            Y A1323 = M[2][1] * M[3][3] - M[2][3] * M[3][1];
            Y A1223 = M[2][1] * M[3][2] - M[2][2] * M[3][1];
            Y A0323 = M[2][0] * M[3][3] - M[2][3] * M[3][0];
            Y A0223 = M[2][0] * M[3][2] - M[2][2] * M[3][0];
            Y A0123 = M[2][0] * M[3][1] - M[2][1] * M[3][0];
            Y tmp[4]{( M[1][1] * A2323 - M[1][2] * A1323 + M[1][3] * A1223 ),
                     ( M[1][0] * A2323 - M[1][2] * A0323 + M[1][3] * A0223 ),
                     ( M[1][0] * A1323 - M[1][1] * A0323 + M[1][3] * A0123 ),
                     ( M[1][0] * A1223 - M[1][1] * A0223 + M[1][2] * A0123 )};

            Y determinant = M[0][0] * tmp[0] - M[0][1] * tmp[1] + M[0][2] * tmp[2] - M[0][3] * tmp[3];
            if (!Math::closeToZero(determinant, SVKFW_EPS4)) {
                determinant = 1. / determinant;

                Y A2313 = M[1][2] * M[3][3] - M[1][3] * M[3][2];
                Y A1313 = M[1][1] * M[3][3] - M[1][3] * M[3][1];
                Y A1213 = M[1][1] * M[3][2] - M[1][2] * M[3][1];
                Y A2312 = M[1][2] * M[2][3] - M[1][3] * M[2][2];
                Y A1312 = M[1][1] * M[2][3] - M[1][3] * M[2][1];
                Y A1212 = M[1][1] * M[2][2] - M[1][2] * M[2][1];
                Y A0313 = M[1][0] * M[3][3] - M[1][3] * M[3][0];
                Y A0213 = M[1][0] * M[3][2] - M[1][2] * M[3][0];
                Y A0312 = M[1][0] * M[2][3] - M[1][3] * M[2][0];
                Y A0212 = M[1][0] * M[2][2] - M[1][2] * M[2][0];
                Y A0113 = M[1][0] * M[3][1] - M[1][1] * M[3][0];
                Y A0112 = M[1][0] * M[2][1] - M[1][1] * M[2][0];

                return {
                           tmp[0] * determinant,
                        ( M[0][1] * A2323 - M[0][2] * A1323 + M[0][3] * A1223 ) * determinant,
                        ( M[0][1] * A2313 - M[0][2] * A1313 + M[0][3] * A1213 ) * determinant,
                        ( M[0][1] * A2312 - M[0][2] * A1312 + M[0][3] * A1212 ) * determinant,
                           tmp[1] * determinant,
                        ( M[0][0] * A2323 - M[0][2] * A0323 + M[0][3] * A0223 ) * determinant,
                        ( M[0][0] * A2313 - M[0][2] * A0313 + M[0][3] * A0213 ) * determinant,
                        ( M[0][0] * A2312 - M[0][2] * A0312 + M[0][3] * A0212 ) * determinant,
                           tmp[2] * determinant,
                        ( M[0][0] * A1323 - M[0][1] * A0323 + M[0][3] * A0123 ) * determinant,
                        ( M[0][0] * A1313 - M[0][1] * A0313 + M[0][3] * A0113 ) * determinant,
                        ( M[0][0] * A1312 - M[0][1] * A0312 + M[0][3] * A0112 ) * determinant,
                           tmp[3] * determinant,
                        ( M[0][0] * A1223 - M[0][1] * A0223 + M[0][2] * A0123 ) * determinant,
                        ( M[0][0] * A1213 - M[0][1] * A0213 + M[0][2] * A0113 ) * determinant,
                        ( M[0][0] * A1212 - M[0][1] * A0212 + M[0][2] * A0112 ) * determinant
                };
            }

            return *this;
        }

        Mat4Base<C> operator+() const
            { return { abs(M[0][0]), abs(M[0][1]), abs(M[0][2]), abs(M[0][3]),
                       abs(M[1][0]), abs(M[1][1]), abs(M[1][2]), abs(M[1][3]),
                       abs(M[2][0]), abs(M[2][1]), abs(M[2][2]), abs(M[2][3]),
                       abs(M[3][0]), abs(M[3][1]), abs(M[3][2]), abs(M[3][3]) }; }
        Mat4Base<C> operator-() const
            { return { -M[0][0], -M[0][1], -M[0][2], -M[0][3],
                       -M[1][0], -M[1][1], -M[1][2], -M[1][3],
                       -M[2][0], -M[2][1], -M[2][2], -M[2][3],
                       -M[3][0], -M[3][1], -M[3][2], -M[3][3] }; }

        template <class Y> Mat4Base<Util::comm_type<C,Y>> operator+(const Mat4Base<Y> &_m) const
            { return { M[0][0] + _m.M[0][0], M[0][1] + _m.M[0][1], M[0][2] + _m.M[0][2], M[0][3] + _m.M[0][3],
                       M[1][0] + _m.M[1][0], M[1][1] + _m.M[1][1], M[1][2] + _m.M[1][2], M[1][3] + _m.M[1][3],
                       M[2][0] + _m.M[2][0], M[2][1] + _m.M[2][1], M[2][2] + _m.M[2][2], M[2][3] + _m.M[2][3],
                       M[3][0] + _m.M[3][0], M[3][1] + _m.M[3][1], M[3][2] + _m.M[3][2], M[3][3] + _m.M[3][3] }; }
        template <class Y> Mat4Base<Util::comm_vecsub_type<C,Y>> operator-(const Mat4Base<Y> &_m) const
            { return { M[0][0] - _m.M[0][0], M[0][1] - _m.M[0][1], M[0][2] - _m.M[0][2], M[0][3] - _m.M[0][3],
                       M[1][0] - _m.M[1][0], M[1][1] - _m.M[1][1], M[1][2] - _m.M[1][2], M[1][3] - _m.M[1][3],
                       M[2][0] - _m.M[2][0], M[2][1] - _m.M[2][1], M[2][2] - _m.M[2][2], M[2][3] - _m.M[2][3],
                       M[3][0] - _m.M[3][0], M[3][1] - _m.M[3][1], M[3][2] - _m.M[3][2], M[3][3] - _m.M[3][3] }; }
        template <class Y> Mat4Base<Util::comm_type<C,Y>> operator*(const Mat4Base<Y> &_m) const
            { return { M[0][0] * _m.M[0][0], M[0][1] * _m.M[0][1], M[0][2] * _m.M[0][2], M[0][3] * _m.M[0][3],
                       M[1][0] * _m.M[1][0], M[1][1] * _m.M[1][1], M[1][2] * _m.M[1][2], M[1][3] * _m.M[1][3],
                       M[2][0] * _m.M[2][0], M[2][1] * _m.M[2][1], M[2][2] * _m.M[2][2], M[2][3] * _m.M[2][3],
                       M[3][0] * _m.M[3][0], M[3][1] * _m.M[3][1], M[3][2] * _m.M[3][2], M[3][3] * _m.M[3][3] }; }
        template <class Y> Mat4Base<Util::comm_type<C,Y>> operator/(const Mat4Base<Y> &_m) const
            { return { M[0][0] / _m.M[0][0], M[0][1] / _m.M[0][1], M[0][2] / _m.M[0][2], M[0][3] / _m.M[0][3],
                       M[1][0] / _m.M[1][0], M[1][1] / _m.M[1][1], M[1][2] / _m.M[1][2], M[1][3] / _m.M[1][3],
                       M[2][0] / _m.M[2][0], M[2][1] / _m.M[2][1], M[2][2] / _m.M[2][2], M[2][3] / _m.M[2][3],
                       M[3][0] / _m.M[3][0], M[3][1] / _m.M[3][1], M[3][2] / _m.M[3][2], M[3][3] / _m.M[3][3] }; }

        template <class Y> void operator+=(const Mat4Base<Y> &_m) { for (uint32_t i=0; i<4u; ++i) for (uint32_t j=0; j<4u; ++j) M[i][j] += _m.M[i][j]; }
        template <class Y> void operator-=(const Mat4Base<Y> &_m) { for (uint32_t i=0; i<4u; ++i) for (uint32_t j=0; j<4u; ++j) M[i][j] -= _m.M[i][j]; }
        template <class Y> void operator*=(const Mat4Base<Y> &_m) { for (uint32_t i=0; i<4u; ++i) for (uint32_t j=0; j<4u; ++j) M[i][j] *= _m.M[i][j]; }
        template <class Y> void operator/=(const Mat4Base<Y> &_m) { for (uint32_t i=0; i<4u; ++i) for (uint32_t j=0; j<4u; ++j) M[i][j] /= _m.M[i][j]; }
        void operator+=(C _op) { for (uint32_t i=0; i<4u; ++i) for (uint32_t j=0; j<4u; ++j) M[i][j] += _op; }
        void operator-=(C _op) { for (uint32_t i=0; i<4u; ++i) for (uint32_t j=0; j<4u; ++j) M[i][j] -= _op; }
        void operator*=(C _op) { for (uint32_t i=0; i<4u; ++i) for (uint32_t j=0; j<4u; ++j) M[i][j] *= _op; }
        void operator/=(C _op) { for (uint32_t i=0; i<4u; ++i) for (uint32_t j=0; j<4u; ++j) M[i][j] /= _op; }

        Mat4Base<C> operator+(C _op) const
            { return { M[0][0] + _op, M[0][1] + _op, M[0][2] + _op, M[0][3] + _op,
                       M[1][0] + _op, M[1][1] + _op, M[1][2] + _op, M[1][3] + _op,
                       M[2][0] + _op, M[2][1] + _op, M[2][2] + _op, M[2][3] + _op,
                       M[3][0] + _op, M[3][1] + _op, M[3][2] + _op, M[3][3] + _op }; }
        Mat4Base<C> operator-(C _op) const
            { return { M[0][0] - _op, M[0][1] - _op, M[0][2] - _op, M[0][3] - _op,
                       M[1][0] - _op, M[1][1] - _op, M[1][2] - _op, M[1][3] - _op,
                       M[2][0] - _op, M[2][1] - _op, M[2][2] - _op, M[2][3] - _op,
                       M[3][0] - _op, M[3][1] - _op, M[3][2] - _op, M[3][3] - _op }; }
        Mat4Base<C> operator*(C _op) const
            { return { M[0][0] * _op, M[0][1] * _op, M[0][2] * _op, M[0][3] * _op,
                       M[1][0] * _op, M[1][1] * _op, M[1][2] * _op, M[1][3] * _op,
                       M[2][0] * _op, M[2][1] * _op, M[2][2] * _op, M[2][3] * _op,
                       M[3][0] * _op, M[3][1] * _op, M[3][2] * _op, M[3][3] * _op }; }
        Mat4Base<C> operator/(C _op) const
            { return { M[0][0] / _op, M[0][1] / _op, M[0][2] / _op, M[0][3] / _op,
                       M[1][0] / _op, M[1][1] / _op, M[1][2] / _op, M[1][3] / _op,
                       M[2][0] / _op, M[2][1] / _op, M[2][2] / _op, M[2][3] / _op,
                       M[3][0] / _op, M[3][1] / _op, M[3][2] / _op, M[3][3] / _op }; }

        template <class Y> Vec4Base<Util::comm_type<C,Y>> operator&(const Vec4Base<Y> &vec) const {
            return { vec.x * M[0][0] + vec.y * M[0][1] + vec.z * M[0][2] + vec.w * M[0][3],
                     vec.x * M[1][0] + vec.y * M[1][1] + vec.z * M[1][2] + vec.w * M[1][3],
                     vec.x * M[2][0] + vec.y * M[2][1] + vec.z * M[2][2] + vec.w * M[2][3],
                     vec.x * M[3][0] + vec.y * M[3][1] + vec.z * M[3][2] + vec.w * M[3][3] };
        }
        template <class Y> Mat4Base<Util::comm_type<C,Y>> operator&(const Mat4Base<Y> &_m) const {
            Mat4Base<Util::comm_type<C,Y>> tmp;
            for (uint32_t i = 0; i <= 12; i += 4)
                for (uint32_t j = 0; j < 4; ++j)
                    (*tmp.M)[i + j] = (*_m.M)[j    ] * (*M)[i    ] + (*_m.M)[j +  4] * (*M)[i + 1] +
                                      (*_m.M)[j + 8] * (*M)[i + 2] + (*_m.M)[j + 12] * (*M)[i + 3];
            return tmp;
        }
        template <class Y> void operator&=(const Mat4Base<Y> &_m) {
            for (uint32_t i = 0; i <= 12; i += 4)
                for (uint32_t j = 0; j < 4; ++j)
                    (*M)[i + j] = (*_m.M)[j    ] * (*M)[i    ] + (*_m.M)[j +  4] * (*M)[i + 1] +
                                  (*_m.M)[j + 8] * (*M)[i + 2] + (*_m.M)[j + 12] * (*M)[i + 3];
        }

        C& operator[](vec2i ind)       { return M[ind.x][ind.y]; }
        C  operator[](vec2i ind) const { return M[ind.x][ind.y]; }
        Vec4Base<C&> operator[](uint32_t ind)       { return { M[ind][0], M[ind][1], M[ind][2], M[ind][3] }; }
        Vec4Base<C > operator[](uint32_t ind) const { return { M[ind][0], M[ind][1], M[ind][2], M[ind][3] }; }
        Mat4RefBase<C> operator[](Mat::Placeholder) { return {  &M[0][0],  &M[0][1],  &M[0][2],  &M[0][3] }; }
        const Mat4RefBase<const C> operator[](Mat::Placeholder) const { return { &M[0][0], &M[0][1], &M[0][2], &M[0][3] }; }
    }; // Mat4Base END

    template <class C>
    struct Mat4RefBase {
        C *c[4];

        Mat4RefBase(C *c1, C *c2, C *c3, C *c4) : c{c1, c2, c3, c4} {}

        Vec4Base<C&> operator[](uint32_t ind) { return { *c[ind], *(c[ind] + 4), *(c[ind] + 8), *(c[ind] + 12) }; }
        Vec4Base<typename std::remove_const<C>::type> operator[](uint32_t ind) const { return { *c[ind], *(c[ind] + 4), *(c[ind] + 8), *(c[ind] + 12) }; }
    }; // Mat4RefBase END


//  ============  Matrix constants  ============  \\

    namespace Mat {
        const mat2 Zeros2{};
        const mat2 Ones2{1};
        const mat2 Eye2{1, 0,
                        0, 1};

        const mat3 Zeros3{};
        const mat3 Ones3{1};
        const mat3 Eye3{1, 0, 0,
                        0, 1, 0,
                        0, 0, 1};

        const mat4 Zeros4{};
        const mat4 Ones4{1};
        const mat4 Eye4{1, 0, 0, 0,
                        0, 1, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1};
    }; // Mat END
}; // Simple END

#endif