#ifndef SVKFW_LINALG_H
#define SVKFW_LINALG_H

#include "math/vectors.h"


/*  ==============================================  //
                y
                │
                │   z
                │  /
                │ /
                │/
                └──────────── x


    Using left-handed coordinate system with:
    * up      vector { 0, 1, 0 } - Y axis
    * right   vector { 1, 0, 0 } - X axis
    * forward vector { 0, 0, 1 } - Z axis

    Rotations are applied in the same order (YXZ).

    Corresponding Euler (Tait–Bryan) angles:
    * yaw   - angle around Y axis
    * pitch - angle around X axis
    * roll  - angle around Z axis

    Positive angle direction - clockwise.

//  ==============================================  */
#define SVKFW_COORDINATE_SYSTEM_AGREEMENTS

// TODO? Extract transforms (translation, scale etc.) from 4x4 transform matrix

namespace Simple {
    namespace Affine {
        template <typename T>
        Mat3Base<Util::remove_ref_c<T>> mat2to3(const Mat2Base<T> &_m) {
            return { _m.M[0][0], _m.M[0][1], 0,
                     _m.M[1][0], _m.M[1][1], 0,
                      0,          0,         1 };
        }


        template <typename T>
        Mat4Base<Util::remove_ref_c<T>> mat3to4(const Mat3Base<T> &_m) {
            return { _m.M[0][0], _m.M[0][1], _m.M[0][2], 0,
                     _m.M[1][0], _m.M[1][1], _m.M[1][2], 0,
                     _m.M[2][0], _m.M[2][1], _m.M[2][2], 0,
                      0,          0,          0,         1 };
        }


//  ================================  \\
                Rotations
//  ================================  \\


//  ============  Quaternions  ============  \\

    // Forward declaration

        namespace Rotation {
            struct Quaternion;

            mat3 quaternion2rotMat3D  (const Quaternion &_q);
            mat4 quaternion2rotMat3D_4(const Quaternion &_q);

    // Quaternion class

            struct Quaternion {
                float s = 1.f, i = 0.f, j = 0.f, k = 0.f; // Default - zero rotation quaternion

                Quaternion() {}
                Quaternion(const Quaternion &_quat) : s{_quat.s}, i{_quat.i}, j{_quat.j}, k{_quat.k} {}
                Quaternion(float _s, float _i, float _j, float _k) :   s{_s  }, i{_i  }, j{_j  }, k{_k  } {}
                template <class Y> Quaternion(const Vec3Base<Y> &_v) : s{ 0.f}, i{_v.x}, j{_v.y}, k{_v.z} {}
                template <class Y> Quaternion(const Vec4Base<Y> &_v) : s{_v.x}, i{_v.y}, j{_v.z}, k{_v.w} {}
                template <class Y> Quaternion(float _scal, const Vec3Base<Y> &_vec) :
                        s{_scal}, i{_vec.x}, j{_vec.y}, k{_vec.z} {}
                template <class Y> Quaternion(const Vec3Base<Y> &_axis, float _a) {
                    vec3 __axis{_axis.normalize()};
                    _a *= 0.5f;

                    s  = Math::cos(_a);
                    _a = Math::sin(_a);
                    i = _a * __axis.x;
                    j = _a * __axis.y;
                    k = _a * __axis.z;
                }


                vec3 vecPart() const { return { i, j, k }; }
                Quaternion conjugate() const { return { s,-i,-j,-k }; }
                Quaternion scalar()    const { return { s, 0, 0, 0 }; }
                Quaternion vector()    const { return { 0, i, j, k }; }
                void toConjugate() { i *= -1.f; j *= -1.f; k *= -1.f; }
                void toScalar()    { i  =  0.f; j  =  0.f; k  =  0.f; }
                void toVector()    { s  =  0.f; }

                float norm () const { return sqrt(s * s + i * i + j * j + k * k); }
                float norm2() const { return      s * s + i * i + j * j + k * k ; }

                Quaternion normalize() const {
                    float __n = norm();

                    if (Math::closeToZero(__n)) {
                        fprintf(outswarn, "%s\n", SVKFW_WRAPWARN("Affine :: Rotation :: Quaternion :: normalize", "cannot normalize"));
                        return {};
                    }

                    __n = 1.f / __n;
                    return { s * __n, i * __n, j * __n, k * __n };
                }

                void toNormalized() {
                    float __n = norm();

                    if (Math::closeToZero(__n)) {
                        fprintf(outswarn, "%s\n", SVKFW_WRAPWARN("Affine :: Rotation :: Quaternion :: toNormalized", "cannot normalize"));
                        s = 1.f;
                        i = 0.f;
                        j = 0.f;
                        k = 0.f;
                        return;
                    }

                    __n = 1.f / __n;
                    s *= __n;
                    i *= __n;
                    j *= __n;
                    k *= __n;
                }

                Quaternion inverse() const {
                    float __n2 = -norm2();

                    if (Math::closeToZero(__n2)) {
                        fprintf(outswarn, "%s\n", SVKFW_WRAPWARN("Affine :: Rotation :: Quaternion :: inverse", "cannot invert"));
                        return {};
                    }

                    __n2 = 1.f / __n2;
                    return { s * -__n2, i * __n2, j * __n2, k * __n2 };
                }

                void toInverted() {
                    float __n2 = -norm2();

                    if (Math::closeToZero(__n2)) {
                        fprintf(outswarn, "%s\n", SVKFW_WRAPWARN("Affine :: Rotation :: Quaternion :: toInverted", "cannot invert"));
                        s = 1.f;
                        i = 0.f;
                        j = 0.f;
                        k = 0.f;
                        return;
                    }

                    __n2 = 1.f / __n2;
                    s *= -__n2;
                    i *=  __n2;
                    j *=  __n2;
                    k *=  __n2;
                }

                vec3 rotateCanonical(const vec3 &_vec) const {
                    Quaternion __tmp = this->normalize();
                    return (__tmp * _vec * __tmp.conjugate()).vecPart();
                }

                vec3 rotateByUnit(const vec3 &_vec) const {
                    return _vec + Math::cross(vec3{ 2 * i, 2 * j, 2 * k },
                                  Math::cross(vec3{ i, j, k }, _vec) + s * _vec);
                }

                vec3 rotate(const vec3 &_vec) const {
                    Quaternion __n = this->normalize();
                    return _vec + Math::cross(vec3{ 2 * __n.i, 2 * __n.j, 2 * __n.k },
                                  Math::cross(vec3{ __n.i, __n.j, __n.k }, _vec) + __n.s * _vec);
                }

                inline mat3 rotMat3D() const {
                    return Rotation::quaternion2rotMat3D  (*this);
                }

                inline mat4 rotMat3D_4() const {
                    return Rotation::quaternion2rotMat3D_4(*this);
                }

                mat4 rotMat4DLeft()  const { return { s, -i, -j, -k,
                                                      i,  s, -k,  j,
                                                      j,  k,  s, -i,
                                                      k, -j,  i,  s }; }

                mat4 rotMat4DRight() const { return { s, -i, -j, -k,
                                                      i,  s,  k, -j,
                                                      j, -k,  s,  i,
                                                      k,  j, -i,  s }; }

                vec4 axisAngle() const {
                    float __n = Math::sqrt(i * i + j * j + k * k);

                    if (Math::closeToZero(__n))
                        return { 0.f, 1.f, 0.f, 0.f };

                    __n = 1.f / __n;
                    return { i * __n, j * __n, k * __n, (2 * Math::atan2(__n, s)) };
                }

                Quaternion operator+(const Quaternion &op2) const
                    { return { s + op2.s, i + op2.i, j + op2.j, k + op2.k }; }
                Quaternion operator-(const Quaternion &op2) const
                    { return { s - op2.s, i - op2.i, j - op2.j, k - op2.k }; }
                Quaternion operator*(const Quaternion &op2) const {
                    return { s * op2.s - i * op2.i - j * op2.j - k * op2.k,
                             s * op2.i + i * op2.s + j * op2.k - k * op2.j,
                             s * op2.j - i * op2.k + j * op2.s + k * op2.i,
                             s * op2.k + i * op2.j - j * op2.i + k * op2.s };
                }
                template <class C> Quaternion operator*(const Vec3Base <C> &op2) const {
                    return { -i * op2.x - j * op2.y - k * op2.z,
                              s * op2.x + j * op2.z - k * op2.y,
                              s * op2.y + k * op2.x - i * op2.z,
                              s * op2.z + i * op2.y - j * op2.x };
                }
            }; // Quaternion END

            Quaternion operator+(float _s, const Quaternion &_q) { return {_s + _q.s, _s + _q.i, _s + _q.j, _s + _q.k}; }
            Quaternion operator-(float _s, const Quaternion &_q) { return {_s - _q.s, _s - _q.i, _s - _q.j, _s - _q.k}; }
            Quaternion operator*(float _s, const Quaternion &_q) { return {_s * _q.s, _s * _q.i, _s * _q.j, _s * _q.k}; }

            Quaternion operator+(const Quaternion &_q, float _s) { return {_q.s + _s, _q.i + _s, _q.j + _s, _q.k + _s}; }
            Quaternion operator-(const Quaternion &_q, float _s) { return {_q.s - _s, _q.i - _s, _q.j - _s, _q.k - _s}; }
            Quaternion operator*(const Quaternion &_q, float _s) { return {_q.s * _s, _q.i * _s, _q.j * _s, _q.k * _s}; }
            Quaternion operator/(const Quaternion &_q, float _s) { return {_q.s / _s, _q.i / _s, _q.j / _s, _q.k / _s}; }


//  ============  Euler (Tait–Bryan) angles  ============  \\

    // Forward declarations

            struct EulerAngles;

            Quaternion ypr2quaternion(float _y, float _p, float _r);
            mat3 ypr2rotMat3D(float _y, float _p, float _r);


    // Euler angles class

            struct EulerAngles {
                float yaw = 0, pitch = 0, roll = 0;

                EulerAngles() {}
                EulerAngles(float _y, float _p, float _r) : yaw{_y}, pitch{_p}, roll{_r} {}
                EulerAngles(const vec3 &_a) : yaw{_a.x}, pitch{_a.y}, roll{_a.z} {}
                EulerAngles(const vec3 &_axis, float _angle) {
                    vec3 __a{_axis.normalize()};
                    _angle *= 0.5f;

                    float __s = Math::cos(_angle);
                    __a *= Math::sin(_angle);

                    pitch = Math::asin(2.f*(__a.x*__s - __a.y*__a.z));

                    if (Math::closeToZero(pitch - (float)M_PI_2, SVKFW_EPS5)) {
                        yaw  = 2.f * Math::atan2(__a.y, __s);
                        roll = 0.f;
                        return;
                    }
                    if (Math::closeToZero(pitch + (float)M_PI_2, SVKFW_EPS5)) {
                        yaw  =-2.f * Math::atan2(__a.y, __s);
                        roll = 0.f;
                        return;
                    }

                    yaw  = Math::atan2(2.f*(__a.x*__a.z + __a.y*__s), 1.f - 2.f*(__a.x*__a.x + __a.y*__a.y));
                    roll = Math::atan2(2.f*(__a.x*__a.y + __a.z*__s), 1.f - 2.f*(__a.x*__a.x + __a.z*__a.z));
                }
                EulerAngles(const mat3 &_rot) {
                    // Don't use this conversion

                    yaw =  Math::atan2( _rot.M[0][2], _rot.M[2][2]);
                    pitch = Math::asin(-_rot.M[1][2]);
                    roll = Math::atan2(-_rot.M[1][0], _rot.M[1][1]);
                }
                EulerAngles(const mat4 &_rot) {
                    // Don't use this conversion

                    yaw =  Math::atan2( _rot.M[0][2], _rot.M[2][2]);
                    pitch = Math::asin(-_rot.M[1][2]);
                    roll = Math::atan2(-_rot.M[1][0], _rot.M[1][1]);
                }
                EulerAngles(const Quaternion &_q) {
                    pitch = Math::asin(2*(_q.i*_q.s - _q.j*_q.k));

                    if (Math::closeToZero(pitch - (float)M_PI_2, SVKFW_EPS5)) {
                        yaw  = 2.f * Math::atan2(_q.j, _q.s);
                        roll = 0.f;
                        return;
                    }
                    if (Math::closeToZero(pitch + (float)M_PI_2, SVKFW_EPS5)) {
                        yaw  =-2.f * Math::atan2(_q.j, _q.s);
                        roll = 0.f;
                        return;
                    }

                    yaw  = Math::atan2(2*(_q.i*_q.k + _q.j*_q.s), 1 - 2*(_q.i*_q.i + _q.j*_q.j));
                    roll = Math::atan2(2*(_q.i*_q.j + _q.k*_q.s), 1 - 2*(_q.i*_q.i + _q.k*_q.k));
                }


                inline mat3 rotMat3D() const {
                    return ypr2rotMat3D(yaw, pitch, roll);
                }

                inline mat4 rotMat3D_4() const {
                    return mat3to4(ypr2rotMat3D(yaw, pitch, roll));
                }

                inline Quaternion quaternion() const {
                    return ypr2quaternion(yaw, pitch, roll);
                }
            }; // EulerAngles END


//  ============  Rotation-related functions  ============  \\

            vec4 axisAngleBetweenVecs(const vec3 &_v1, const vec3 &_v2) {
                vec3 __V1 = _v1.normalize(), __V2 = _v2.normalize();
                float __angle = Math::acos(Math::dot(__V1, __V2));

                if (Math::closeToZero(__angle + (float)M_PI, SVKFW_EPS5))
                    return { vec3{__V2.y, -__V2.x, __V2.z}, float(M_PI) };

                if (Math::closeToZero(__angle, SVKFW_EPS5))
                    return { vec3{__V2.y, -__V2.x, __V2.z}, 0.f };

                return { Math::cross(__V1, __V2), __angle };
            }


//  ============  Quaternion-based rotation functions  ============  \\

    // Returns quaternion representing the shortest rotation between two vectors
            Quaternion rotationBetweenVecs(const vec3 &_v1, const vec3 &_v2) {
                float __KxCos = Math::dot(_v1, _v2);
                float __K = Math::sqrt(_v1.norm2() * _v2.norm2());

                if (Math::closeToZero(__K - __KxCos))
                    return {}; // Zero rotation - default
                if (Math::closeToZero(__K + __KxCos))
                    return { 0, vec3{ -_v1.y, _v1.x, _v1.z }.normalize() };

                return Quaternion{ __K + __KxCos, Math::cross(_v1, _v2) }.normalize();
            }

    // Returns quaternion representing shortest rotation between two unit vectors
            Quaternion rotationBetweenUnitVecs(const vec3 &_v1, const vec3 &_v2) {
                float __Cos = Math::dot(_v1, _v2);

                if (Math::closeToZero(1.f - __Cos))
                    return {}; // Zero rotation - default
                if (Math::closeToZero(1.f + __Cos))
                    return { 0, vec3{ -_v1.y, _v1.x, _v1.z } };

                return Quaternion{ 1.f + __Cos, Math::cross(_v1, _v2) }.normalize();
            }


//  ============  Quaternion -> Rotation matrix  ============  \\

            mat3 quaternion2rotMat3D(const Quaternion &_q) {
                float __n = _q.norm2();

                if (Math::closeToZero(__n)) {
                    fprintf(outswarn, "%s\n", SVKFW_WRAPWARN("Affine :: Rotation :: quaternion2rotMat3D", "cannot calculate rotation matrix"));
                    return Mat::Eye3;
                }

                __n = 2.f / __n;
                float _i = _q.i*__n, _j = _q.j*__n, _k = _q.k*__n;

                float si = _q.s* _i, sj = _q.s* _j, sk = _q.s* _k;
                float ii = _q.i* _i, ij = _q.i* _j, ik = _q.i* _k;
                float jj = _q.j* _j, jk = _q.j* _k, kk = _q.k* _k;

                return { 1-(jj + kk),   (ij - sk),   (ik + sj),
                           (ij + sk), 1-(ii + kk),   (jk - si),
                           (ik - sj),   (jk + si), 1-(ii + jj) };
            }

            inline mat4 quaternion2rotMat3D_4(const Quaternion &_q) {
                return mat3to4(quaternion2rotMat3D(_q));
            }


//  ============  Quaternion <-> Euler angles  ============  \\

            Quaternion ypr2quaternion(float _y, float _p, float _r) {
                float s1 = Math::sin(_y * 0.5f), c1 = Math::cos(_y * 0.5f),
                      s2 = Math::sin(_p * 0.5f), c2 = Math::cos(_p * 0.5f),
                      s3 = Math::sin(_r * 0.5f), c3 = Math::cos(_r * 0.5f);

                return { c3*c2*c1 + s3*s2*s1,
                         c3*s2*c1 + s3*c2*s1,
                         c3*c2*s1 - s3*s2*c1,
                         s3*c2*c1 - c3*s2*s1 };
            }

            EulerAngles quaternion2ypr(const Quaternion &_q) {
                float _p = Math::asin(2.f * (_q.i*_q.s - _q.j*_q.k));

                if (Math::closeToZero(_p - (float)M_PI_2, SVKFW_EPS5))
                    return {  2.f * Math::atan2(_q.j, _q.s), _p, 0.f };
                if (Math::closeToZero(_p + (float)M_PI_2, SVKFW_EPS5))
                    return { -2.f * Math::atan2(_q.j, _q.s), _p, 0.f };

                return { Math::atan2(2.f*(_q.i*_q.k + _q.j*_q.s), 1.f - 2.f*(_q.i*_q.i + _q.j*_q.j)),
                         _p,
                         Math::atan2(2.f*(_q.i*_q.j + _q.k*_q.s), 1.f - 2.f*(_q.i*_q.i + _q.k*_q.k)) };
            }


//  ============  Euler angles -> Rotation matrix  ============  \\

            // YXZ
            mat3 ypr2rotMat3D(float _y, float _p, float _r) {
                float s1 = Math::sin(_y), c1 = Math::cos(_y),
                      s2 = Math::sin(_p), c2 = Math::cos(_p),
                      s3 = Math::sin(_r), c3 = Math::cos(_r);

                return { c1*c3 + s1*s2*s3, s1*s2*c3 - c1*s3, s1*c2,
                         c2*s3,            c2*c3,           -s2,   
                         c1*s2*s3 - c3*s1, c1*c3*s2 + s1*s3, c1*c2 };
            }

            inline mat3 ypr2rotMat3D(const vec3 &_a) {
                return  ypr2rotMat3D(_a.x, _a.y, _a.z);
            }

            // YXZ
            mat4 ypr2rotMat3D_4(float _y, float _p, float _r) {
                float s1 = Math::sin(_y), c1 = Math::cos(_y),
                      s2 = Math::sin(_p), c2 = Math::cos(_p),
                      s3 = Math::sin(_r), c3 = Math::cos(_r);

                return { c1*c3 + s1*s2*s3, s1*s2*c3 - c1*s3, s1*c2, 0,
                         c2*s3,            c2*c3,              -s2, 0,
                         c1*s2*s3 - c3*s1, c1*c3*s2 + s1*s3, c1*c2, 0,
                         0,                0,                0,     1 };
            }

            inline mat4 ypr2rotMat3D_4(const vec3 &_a) {
                return  ypr2rotMat3D_4(_a.x, _a.y, _a.z);
            }


//  ============  XYZ/Any convention angles -> Rotation matrix  ============  \\

    // clockwise rotation
            mat2 rotMat2D(float _theta) {
                return { Math::cos(_theta), Math::sin(_theta),
                        -Math::sin(_theta), Math::cos(_theta) };
            }

    // clockwise rotation
            mat3 rotMat2D_3(float _theta) {
                return { Math::cos(_theta), Math::sin(_theta), 0.f,
                        -Math::sin(_theta), Math::cos(_theta), 0.f,
                         0.f,                       0.f,                       1.f };
            }

    // rotation matrix around axis 'x', 'y' or 'z'
            mat3 axisRotMat3D(float _theta, char _axis) {
                if (_axis < 'x' || _axis > 'z')
                    throw std::invalid_argument(SVKFW_WRAPERR("Simple :: Affine :: Rotation :: axisRotMat3D", "invalid axis name"));

                mat3 __res{Mat::Eye3};
                float __s = Math::sin(_theta), __c = Math::cos(_theta);

                switch (_axis) {
                    case 'x': {
                        __res.M[1][1] = __c;
                        __res.M[2][2] = __c;
                        __res.M[1][2] = __s;
                        __res.M[2][1] =-__s;
                        break;
                    }
                    case 'y': {
                        __res.M[0][0] = __c;
                        __res.M[2][2] = __c;
                        __res.M[0][2] = __s;
                        __res.M[2][0] =-__s;
                        break;
                    }
                    case 'z': {
                        __res.M[0][0] = __c;
                        __res.M[1][1] = __c;
                        __res.M[0][1] = __s;
                        __res.M[1][0] =-__s;
                        break;
                    }
                }

                return __res;
            }

    // rotation matrix around axes 'x', 'y' or 'z'
            inline mat3 axesRotMat3D(const vec2 &_theta, char _axis1, char _axis2) {
                return axisRotMat3D(_theta.y, _axis2) &
                       axisRotMat3D(_theta.x, _axis1);
            }

    // rotation matrix around axes 'x', 'y' or 'z'
            inline mat3 axesRotMat3D(const vec3 &_theta, char _axis1, char _axis2, char _axis3) {
                return axisRotMat3D(_theta.z, _axis3) &
                       axisRotMat3D(_theta.y, _axis2) &
                       axisRotMat3D(_theta.x, _axis1);
            }

    // rotation matrix around axis 'x', 'y' or 'z'
            inline mat4 axisRotMat3D_4(float _theta, char _axis) {
                return mat3to4(axisRotMat3D(_theta, _axis));
            }

    // rotation matrix around axes 'x', 'y' or 'z'
            inline mat4 axesRotMat3D_4(const vec2 &_theta, char _axis1, char _axis2) {
                return mat3to4(axisRotMat3D(_theta.y, _axis2) &
                               axisRotMat3D(_theta.x, _axis1));
            }

    // rotation matrix around axes 'x', 'y' or 'z'
            inline mat4 axesRotMat3D_4(const vec3 &_theta, char _axis1, char _axis2, char _axis3) {
                return mat3to4(axisRotMat3D(_theta.z, _axis3) &
                               axisRotMat3D(_theta.y, _axis2) &
                               axisRotMat3D(_theta.x, _axis1));
            }

/*
    // _x - angle around axis X, _y - angle around axis Y, _z - angle around axis Z, cw
            mat3 xyz2rotMat3D(float _x, float _y, float _z) {
                float s1 = Math::sin(_x), c1 = Math::cos(_x),
                      s2 = Math::sin(_y), c2 = Math::cos(_y),
                      s3 = Math::sin(_z), c3 = Math::cos(_z);

                return { c2*c3,           -c2*s3,            s2,
                         c1*s3 + c3*s1*s2, c1*c3 - s1*s2*s3,-c2*s1,
                         s1*s3 - c1*c3*s2, c3*s1 + c1*s2*s3, c1*c2 };
            }

    // .x - angle around axis X, .y - angle around axis Y, .z - angle around axis Z, cw
            inline mat3 xyz2rotMat3D(const vec3 &_a) {
                return  xyz2rotMat3D(_a.x, _a.y, _a.z);
            }
*/
        }; // Rotation END


//  ============  Translation  ============  \\

        mat3 translateMat2D(const vec2 &_o) {
            return { 1.f, 0.f, _o.x,
                     0.f, 1.f, _o.y,
                     0.f, 0.f,  1.f };
        }

        mat4 translateMat3D(const vec3 &_o) {
            return { 1.f, 0.f, 0.f, _o.x,
                     0.f, 1.f, 0.f, _o.y,
                     0.f, 0.f, 1.f, _o.z,
                     0.f, 0.f, 0.f,  1.f };
        }


//  ============  Scaling  ============  \\

        mat3 scaleMat2D(const vec2 &_o) {
            return { _o.x,  0.f, 0.f,
                      0.f, _o.y, 0.f,
                      0.f,  0.f, 1.f };
        }

        mat4 scaleMat3D(const vec3 &_o) {
            return { _o.x,  0.f,  0.f, 0.f,
                      0.f, _o.y,  0.f, 0.f,
                      0.f,  0.f, _o.z, 0.f,
                      0.f,  0.f,  0.f, 1.f };
        }


//  ============  Shear  ============  \\

        mat4 shearMat3D(float xy, float xz, float yx, float yz, float zx, float zy) {
            return { 1.f,  xy,  xz, 0.f,
                      yx, 1.f,  yz, 0.f,
                      zx,  zy, 1.f, 0.f,
                     0.f, 0.f, 0.f, 1.f };
        }

        mat4 shearAngleMat3D(float xy, float xz, float yx, float yz, float zx, float zy) {
            return { 1.f,           Math::tan(xy),  Math::tan(xz), 0.f,
                     Math::tan(yx), 1.f,            Math::tan(yz), 0.f,
                     Math::tan(zx), Math::tan(zy),  1.f,           0.f,
                     0.f,           0.f,            0.f,           1.f };
        }


//  ============  Reflection  ============  \\

        namespace Reflection {
            const mat3 x_2D{ -1, 0, 0,
                              0, 1, 0,
                              0, 0, 1 };
            const mat3 y_2D{  1, 0, 0,
                              0,-1, 0,
                              0, 0, 1 };
            const mat3 xy_2D{-1, 0, 0,
                              0,-1, 0,
                              0, 0, 1 };

            const mat4 x_3D{ -1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              0, 0, 0, 1 };
            const mat4 y_3D{  1, 0, 0, 0,
                              0,-1, 0, 0,
                              0, 0, 1, 0,
                              0, 0, 0, 1 };
            const mat4 z_3D{  1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0,-1, 0,
                              0, 0, 0, 1 };

            const mat4 xy_3D{ -1, 0, 0, 0,
                               0,-1, 0, 0,
                               0, 0, 1, 0,
                               0, 0, 0, 1 };
            const mat4 xz_3D{ -1, 0, 0, 0,
                               0, 1, 0, 0,
                               0, 0,-1, 0,
                               0, 0, 0, 1 };
            const mat4 yz_3D{  1, 0, 0, 0,
                               0,-1, 0, 0,
                               0, 0,-1, 0,
                               0, 0, 0, 1 };

            const mat4 xyz_3D{ -1, 0, 0, 0,
                                0,-1, 0, 0,
                                0, 0,-1, 0,
                                0, 0, 0, 1 };
        }; // Reflection END

        mat3 reflectMat2D(char _axis) {
            if (_axis < 'x' || _axis > 'y')
                throw std::invalid_argument(SVKFW_WRAPERR("Simple :: Affine :: reflectMat2D", "invalid axis name"));

            if (_axis == 'x') return Reflection::x_2D;
            return Reflection::y_2D;
        }

        mat4 reflectMat3D(char _axis) {
            if (_axis < 'x' || _axis > 'z')
                throw std::invalid_argument(SVKFW_WRAPERR("Simple :: Affine :: reflectMat3D (1)", "invalid axis name"));

            if (_axis == 'x') return Reflection::x_3D;
            if (_axis == 'y') return Reflection::y_3D;
            return Reflection::z_3D;
        }

        mat4 reflectMat3D(char _axis1, char _axis2) {
            if (_axis1 < 'x' || _axis1 > 'z' ||
                _axis2 < 'x' || _axis2 > 'z')
                throw std::invalid_argument(SVKFW_WRAPERR("Simple :: Affine :: reflectMat3D (2)", "invalid axis name"));

            vec3 __v{1.f};
            __v[_axis1 - 'x'] = -1.f;
            __v[_axis2 - 'x'] = -1.f;
            return {__v.x,  0.f,  0.f, 0.f,
                      0.f,__v.y,  0.f, 0.f,
                      0.f,  0.f,__v.z, 0.f,
                      0.f,  0.f,  0.f, 1.f };
        }


//  ============  Transform class  ============  \\

        struct Transform3D {
            mat4 tform{Mat::Eye4};


            Transform3D() {}
            Transform3D(const mat4 &_tf) : tform{_tf} {}
            Transform3D(const Transform3D &_tf) : tform{_tf.tform} {}


            Transform3D& translate(const vec3 &_o) {
                tform &= translateMat3D(_o);
                return *this;
            }

            Transform3D& scale(const vec3 &_s) {
                tform &= scaleMat3D(_s);
                return *this;
            }


            Transform3D& rotate(const Affine::Rotation::Quaternion  &_s) {
                tform &= _s.rotMat3D_4();
                return *this;
            }

            Transform3D& rotate(const Affine::Rotation::EulerAngles &_e) {
                tform &= _e.rotMat3D_4();
                return *this;
            }


            Transform3D& reflect() {
                tform &= Reflection::xyz_3D;
                return *this;
            }

            Transform3D& reflect(char _axis) {
                tform &= reflectMat3D(_axis);
                return *this;
            }

            Transform3D& reflect(char _axis1, char _axis2) {
                tform &= reflectMat3D(_axis1, _axis2);
                return *this;
            }


            Transform3D& shear(float xy, float xz, float yx, float yz, float zx, float zy) {
                tform &= shearMat3D(xy, xz, yx, yz, zx, zy);
                return *this;
            }

            Transform3D& shearAngle(float xy, float xz, float yx, float yz, float zx, float zy) {
                tform &= shearAngleMat3D(xy, xz, yx, yz, zx, zy);
                return *this;
            }
        }; // Transform3D END
    }; // Affine END



//  General quaternion functions

    namespace Math {
        float dot(const Affine::Rotation::Quaternion &_q1, const Affine::Rotation::Quaternion &_q2)
            { return _q1.s * _q2.s + _q1.i * _q2.i + _q1.j * _q2.j + _q1.k * _q2.k; }

        Affine::Rotation::Quaternion pow(const Affine::Rotation::Quaternion &_q, float _p) {
            vec3 __v = _q.vecPart();
            float __vn = __v.norm();

            if (Math::closeToZero(__vn) || Math::closeToZero(_p))
                return { Math::pow(_q.s, _p), 0.f, 0.f, 0.f }; // second check is from formula below

            float __n = _q.norm();
            __v *= 1.f / __vn;
            __vn = _p * Math::acos(_q.s / __n);
            // ln_quat = {__s = _p * Math::log(__n), __v * __vn}; __v - unit vector
            return Math::exp(_p * Math::log(__n)) *
                    Affine::Rotation::Quaternion{ Math::cos(__vn), Math::sin(__vn) * __v };
        }

        Affine::Rotation::Quaternion powUnitQuat(const Affine::Rotation::Quaternion &_q, float _p) {
            vec3 __v = _q.vecPart();
            float __vn = __v.norm();

            if (Math::closeToZero(__vn) || Math::closeToZero(_p))
                return { powf(_q.s, _p), 0.f, 0.f, 0.f };

            __v *= 1.f / __vn;
            __vn = _p * Math::acos(_q.s);
            return { Math::cos(__vn), Math::sin(__vn) * __v };
        }
    }; // Math END



//  ====================================  \\
                Interpolation
//  ====================================  \\

    namespace Interpolation {
        template <class V>
        struct Bezier {
            virtual ~Bezier() {}

            virtual V interpolate(float _t) const {
                return {};
            }
        }; // Bezier END

        template <class V>
        struct BezierLinear : Bezier<V> {
            V P0, P1;

            BezierLinear(const V &_p0, const V &_p1) : P0{_p0}, P1{_p1} {}
            BezierLinear(const BezierLinear<V> &_c) : P0{_c.P0}, P1{_c.P1} {}

            V interpolate(float _t) const override {
                _t = Math::clamp_CL(_t, 0.f, 1.f);
                return P0 + _t * (P1 - P0);
            }

            static V interpolate(float _t, const V &_p0, const V &_p1) {
                _t = Math::clamp_CL(_t, 0.f, 1.f);
                return _p0 + _t * (_p1 - _p0);
            }
        }; // BezierLinear END

        template <class V>
        struct BezierQuadratic : Bezier<V> {
            V P0, P1, P2;

            BezierQuadratic(const V &_p0, const V &_p1, const V &_p2) : P0{_p0}, P1{_p1}, P2{_p2} {}
            BezierQuadratic(const BezierLinear<V> &_l) : P0{_l.P0}, P1{0.5f * (_l.P1 - _l.P0)}, P2{_l.P1} {}
            BezierQuadratic(const BezierQuadratic<V> &_c) : P0{_c.P0}, P1{_c.P1}, P2{_c.P2} {}

            V interpolate(float _t) const override {
                _t = Math::clamp_CL(_t, 0.f, 1.f);
                return P1 + Math::sqr(1.f - _t) * (P0 - P1) + (_t * _t) * (P2 - P1);
            }

            static V interpolate(float _t, const V &_p0, const V &_p1, const V &_p2) {
                _t = Math::clamp_CL(_t, 0.f, 1.f);
                return _p1 + Math::sqr(1.f - _t) * (_p0 - _p1) + (_t * _t) * (_p2 - _p1);
            }
        }; // BezierQuadratic END

        template <class V>
        struct BezierCubic : Bezier<V> {
            V P0, P1, P2, P3;

            BezierCubic(const V &_p0, const V &_p1, const V &_p2, const V &_p3) : P0{_p0}, P1{_p1}, P2{_p2}, P3{_p3} {}
            BezierCubic(const BezierLinear<V> &_l) : BezierCubic(BezierQuadratic<V>{_l}) {}
            BezierCubic(const BezierQuadratic<V> &_q) : P0{_q.P0}, P1{(1.f / 3) * (2 * _q.P1 + _q.P0)},
                                                        P2{(1.f / 3) * (2 * _q.P1 + _q.P2)}, P3{_q.P2} {}
            BezierCubic(const BezierCubic<V> &_c) : P0{_c.P0}, P1{_c.P1}, P2{_c.P2}, P3{_c.P3} {}

            V interpolate(float _t) const override {
                _t = Math::clamp_CL(_t, 0.f, 1.f);
                return Math::cube(1.f - _t) *  P0 +
                            3 * (1.f - _t) * _t * ( P1 + _t * ( P2 -  P1)) + (_t * _t * _t) *  P3;
            }

            static V interpolate(float _t, const V &_p0, const V &_p1, const V &_p2, const V &_p3) {
                _t = Math::clamp_CL(_t, 0.f, 1.f);
                return Math::cube(1.f - _t) * _p0 +
                            3 * (1.f - _t) * _t * (_p1 + _t * (_p2 - _p1)) + (_t * _t * _t) * _p3;
            }
        }; // BezierCubic END


//  ============  Bezier types  ============  \\

        typedef Bezier<vec2>          Bezier2D;
        typedef BezierLinear<vec2>    BezierLinear2D;
        typedef BezierQuadratic<vec2> BezierQuadratic2D;
        typedef BezierCubic<vec2>     BezierCubic2D;

        typedef Bezier<vec3>          Bezier3D;
        typedef BezierLinear<vec3>    BezierLinear3D;
        typedef BezierQuadratic<vec3> BezierQuadratic3D;
        typedef BezierCubic<vec3>     BezierCubic3D;


//  ============  SLERP  ============  \\

        struct Slerp {
            Affine::Rotation::Quaternion Q1, Q2;

            Slerp(const Affine::Rotation::Quaternion &_q1, const Affine::Rotation::Quaternion &_q2) :
                    Q1{_q1.normalize()}, Q2{_q2.normalize()} {}
            Slerp(const Slerp &_s) : Q1{_s.Q1}, Q2{_s.Q2} {}

            Affine::Rotation::Quaternion interpolate(float _u) const {
                _u = Math::clamp_CL(_u, 0.f, 1.f);

                float __theta = Math::acos(Math::dot(Q1, Q2));
                if (Math::closeToZero(__theta)) return Q1;

                float __denom = 1.f / Math::sin(__theta);
                return Q1 * (Math::sin((1.f - _u) * __theta) * __denom) +
                       Q2 * (Math::sin(       _u  * __theta) * __denom);
            }

            static Affine::Rotation::Quaternion interpolate(float _u, const Affine::Rotation::Quaternion &_q1,
                                                                      const Affine::Rotation::Quaternion &_q2) {
                _u = Math::clamp_CL(_u, 0.f, 1.f);
                // if greedy, use one variable instead of "norm" and "theta"
                float __norm = Math::sqrt(_q1.norm2() * _q2.norm2());
                if (Math::closeToZero(__norm )) return _q1;

                float __theta = Math::acos(Math::dot(_q1, _q2) / __norm);
                if (Math::closeToZero(__theta)) return _q1;

                float __denom = 1.f / Math::sin(__theta);
                return _q1 * (Math::sin((1.f - _u) * __theta) * __denom) +
                       _q2 * (Math::sin(       _u  * __theta) * __denom);
            }
        }; // Slerp END
    }; // Interpolation EnD
}; // Simple END

#endif