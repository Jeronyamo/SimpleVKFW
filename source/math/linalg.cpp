#include "math/linalg.h"

namespace Simple {
    namespace Affine {
        namespace Rotation {

            Quaternion::Quaternion() {}

            Quaternion::Quaternion(const Quaternion &_quat) : s{_quat.s}, i{_quat.i}, j{_quat.j}, k{_quat.k} {}

            Quaternion::Quaternion(float _s, float _i, float _j, float _k)   : s{_s  }, i{_i  }, j{_j  }, k{_k  } {}

            vec3 Quaternion::vecPart() const { return { i, j, k }; }

            Quaternion Quaternion::conjugate() const { return { s,-i,-j,-k }; }

            Quaternion Quaternion::scalar()    const { return { s, 0, 0, 0 }; }

            Quaternion Quaternion::vector()    const { return { 0, i, j, k }; }

            void Quaternion::toConjugate() { i *= -1.f; j *= -1.f; k *= -1.f; }

            void Quaternion::toScalar()    { i  =  0.f; j  =  0.f; k  =  0.f; }

            void Quaternion::toVector()    { s  =  0.f; }

            float Quaternion::norm () const { return sqrt(s * s + i * i + j * j + k * k); }

            float Quaternion::norm2() const { return      s * s + i * i + j * j + k * k ; }

            Quaternion Quaternion::normalize() const {
                float __n = norm();

                if (Math::closeToZero(__n)) {
                    fprintf(svkfwwarn, "%s\n", SVKFW_WRAPWARN("Affine :: Rotation :: Quaternion :: normalize", "cannot normalize"));
                    return {};
                }

                __n = 1.f / __n;
                return { s * __n, i * __n, j * __n, k * __n };
            }

            void Quaternion::toNormalized() {
                float __n = norm();

                if (Math::closeToZero(__n)) {
                    fprintf(svkfwwarn, "%s\n", SVKFW_WRAPWARN("Affine :: Rotation :: Quaternion :: toNormalized", "cannot normalize"));
                    s = 1.f; i = 0.f; j = 0.f; k = 0.f;
                    return;
                }

                __n = 1.f / __n;
                s *= __n; i *= __n; j *= __n; k *= __n;
            }

            Quaternion Quaternion::inverse() const {
                float __n2 = -norm2();

                if (Math::closeToZero(__n2)) {
                    fprintf(svkfwwarn, "%s\n", SVKFW_WRAPWARN("Affine :: Rotation :: Quaternion :: inverse", "cannot invert"));
                    return {};
                }

                __n2 = 1.f / __n2;
                return { s * -__n2, i * __n2, j * __n2, k * __n2 };
            }

            void Quaternion::toInverted() {
                float __n2 = -norm2();

                if (Math::closeToZero(__n2)) {
                    fprintf(svkfwwarn, "%s\n", SVKFW_WRAPWARN("Affine :: Rotation :: Quaternion :: toInverted", "cannot invert"));
                    s = 1.f; i = 0.f; j = 0.f; k = 0.f;
                    return;
                }

                __n2 = 1.f / __n2;
                s *= -__n2; i *= __n2; j *= __n2; k *= __n2;
            }

            vec3 Quaternion::rotateCanonical(const vec3 &_vec) const {
                Quaternion __tmp = this->normalize();
                return (__tmp * _vec * __tmp.conjugate()).vecPart();
            }

            vec3 Quaternion::rotateByUnit(const vec3 &_vec) const {
                return _vec + Math::cross(vec3{ 2 * i, 2 * j, 2 * k },
                              Math::cross(vec3{ i, j, k }, _vec) + s * _vec);
            }

            vec3 Quaternion::rotate(const vec3 &_vec) const {
                Quaternion __n = this->normalize();
                return _vec + Math::cross(vec3{ 2 * __n.i, 2 * __n.j, 2 * __n.k },
                              Math::cross(vec3{ __n.i, __n.j, __n.k }, _vec) + __n.s * _vec);
            }

            mat3 Quaternion::rotMat3D() const {
                return Rotation::quaternion2rotMat3D  (*this);
            }

            mat4 Quaternion::rotMat3D_4() const {
                return Rotation::quaternion2rotMat3D_4(*this);
            }

            mat4 Quaternion::rotMat4DLeft()  const { return { s, -i, -j, -k,
                                                  i,  s, -k,  j,
                                                  j,  k,  s, -i,
                                                  k, -j,  i,  s }; }

            mat4 Quaternion::rotMat4DRight() const { return { s, -i, -j, -k,
                                                  i,  s,  k, -j,
                                                  j, -k,  s,  i,
                                                  k,  j, -i,  s }; }

            vec4 Quaternion::axisAngle() const {
                float __n = Math::sqrt(i * i + j * j + k * k);

                if (Math::closeToZero(__n))
                    return { 0.f, 1.f, 0.f, 0.f };

                return { i / __n, j / __n, k / __n, (2 * Math::atan2(__n, s)) };
            }

            Quaternion Quaternion::operator+(const Quaternion &op2) const
                { return { s + op2.s, i + op2.i, j + op2.j, k + op2.k }; }

            Quaternion Quaternion::operator-(const Quaternion &op2) const
                { return { s - op2.s, i - op2.i, j - op2.j, k - op2.k }; }

            Quaternion Quaternion::operator*(const Quaternion &op2) const {
                return { s * op2.s - i * op2.i - j * op2.j - k * op2.k,
                         s * op2.i + i * op2.s + j * op2.k - k * op2.j,
                         s * op2.j - i * op2.k + j * op2.s + k * op2.i,
                         s * op2.k + i * op2.j - j * op2.i + k * op2.s };
            }

            Quaternion operator+(float _s, const Quaternion &_q) { return {_s + _q.s,  _q.i,  _q.j,  _q.k}; }

            Quaternion operator-(float _s, const Quaternion &_q) { return {_s - _q.s, -_q.i, -_q.j, -_q.k}; }

            Quaternion operator*(float _s, const Quaternion &_q) { return {_s * _q.s, _s * _q.i, _s * _q.j, _s * _q.k}; }

            Quaternion operator+(const Quaternion &_q, float _s) { return {_q.s + _s, _q.i, _q.j, _q.k}; }

            Quaternion operator-(const Quaternion &_q, float _s) { return {_q.s - _s, _q.i, _q.j, _q.k}; }

            Quaternion operator*(const Quaternion &_q, float _s) { return {_q.s * _s, _q.i * _s, _q.j * _s, _q.k * _s}; }

            Quaternion operator/(const Quaternion &_q, float _s) { return {_q.s / _s, _q.i / _s, _q.j / _s, _q.k / _s}; }

            EulerAngles::EulerAngles() {}

            EulerAngles::EulerAngles(float _y, float _p, float _r) : yaw{_y}, pitch{_p}, roll{_r} {}

            EulerAngles::EulerAngles(const vec3 &_a) : yaw{_a.x}, pitch{_a.y}, roll{_a.z} {}

            EulerAngles::EulerAngles(const vec3 &_axis, float _angle) {
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

            EulerAngles::EulerAngles(const mat3 &_rot) {
                // Don't use this conversion

                yaw =  Math::atan2( _rot.M[0][2], _rot.M[2][2]);
                pitch = Math::asin(-_rot.M[1][2]);
                roll = Math::atan2(-_rot.M[1][0], _rot.M[1][1]);
            }

            EulerAngles::EulerAngles(const mat4 &_rot) {
                // Don't use this conversion

                yaw =  Math::atan2( _rot.M[0][2], _rot.M[2][2]);
                pitch = Math::asin(-_rot.M[1][2]);
                roll = Math::atan2(-_rot.M[1][0], _rot.M[1][1]);
            }

            EulerAngles::EulerAngles(const Quaternion &_q) {
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

            EulerAngles::operator vec3() { return { yaw, pitch, roll }; }

            mat3 EulerAngles::rotMat3D() const {
                return ypr2rotMat3D(yaw, pitch, roll);
            }

            mat4 EulerAngles::rotMat3D_4() const {
                return mat3to4(ypr2rotMat3D(yaw, pitch, roll));
            }

            Quaternion EulerAngles::quaternion() const {
                return ypr2quaternion(yaw, pitch, roll);
            }

            vec3 anyPerpendicular(const vec3 &_v) {
                // Cross product with the axis that is "most perpendicular" to '_v'
                vec3 __abs_v{Math::abs(_v.x), Math::abs(_v.y), Math::abs(_v.z)};
                vec3 __axis{0.f};
                __axis[Math::aminOf(__abs_v)] = 1.f;
                return Math::cross(_v, __axis).normalize();
            }

            vec4 axisAngleBetweenVecs(const vec3 &_v1, const vec3 &_v2) {
                vec3 __V1 = _v1.normalize(), __V2 = _v2.normalize();
                float __angle = Math::acos(Math::clampCL(Math::dot(__V1, __V2), -1.f, 1.f));

                if (Math::closeToZero(__angle - (float)M_PI, SVKFW_EPS5))
                    return { anyPerpendicular(__V1), float(M_PI) };

                if (Math::closeToZero(__angle, SVKFW_EPS5))
                    return { anyPerpendicular(__V1), 0.f };

                return { Math::cross(__V1, __V2).normalize(), __angle };
            }

            vec3 quaternion2tangent(const Quaternion &_q) {
                float __scalar_acos = std::acos(Math::clampCL(_q.s, -1.f, 1.f));
                vec3 __res = _q.vecPart();
                if (std::abs(__scalar_acos) > 1e-4f)
                    __res *= __scalar_acos / std::sin(__scalar_acos);
                return __res;
            }

            Quaternion tangent2quaternion(const vec3 &_v) {
                float __vec_norm = _v.norm();
                Quaternion __res = { std::cos(__vec_norm), _v };
                if (__vec_norm > 1e-4f)
                    __res = { __res.s, std::sin(__vec_norm) / __vec_norm * _v };
                return __res;
            }

            Quaternion rotationBetweenVecs(const vec3 &_v1, const vec3 &_v2) {
                float __KxCos = Math::dot(_v1, _v2);
                float __K = Math::sqrt(_v1.norm2() * _v2.norm2());

                if (Math::closeToZero(__K - __KxCos))
                    return {}; // Zero rotation - default
                if (Math::closeToZero(__K + __KxCos))
                    return { 0.f, anyPerpendicular(_v1) }; // 180 degrees around any perpendicular axis

                return Quaternion{ __K + __KxCos, Math::cross(_v1, _v2) }.normalize();
            }

            Quaternion rotationBetweenUnitVecs(const vec3 &_v1, const vec3 &_v2) {
                float __Cos = Math::dot(_v1, _v2);

                if (Math::closeToZero(1.f - __Cos))
                    return {}; // Zero rotation - default
                if (Math::closeToZero(1.f + __Cos))
                    return { 0.f, anyPerpendicular(_v1) }; // 180 degrees around any perpendicular axis

                return Quaternion{ 1.f + __Cos, Math::cross(_v1, _v2) }.normalize();
            }

            mat3 quaternion2rotMat3D(const Quaternion &_q) {
                float __n = _q.norm2();

                if (Math::closeToZero(__n)) {
                    fprintf(svkfwwarn, "%s\n", SVKFW_WRAPWARN("Affine :: Rotation :: quaternion2rotMat3D", "cannot calculate rotation matrix"));
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

            mat4 quaternion2rotMat3D_4(const Quaternion &_q) {
                return mat3to4(quaternion2rotMat3D(_q));
            }

            Quaternion ypr2quaternion(const EulerAngles &_ypr) {
                return ypr2quaternion(_ypr.yaw, _ypr.pitch, _ypr.roll);
            }

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

            mat3 ypr2rotMat3D(float _y, float _p, float _r) {
                float s1 = Math::sin(_y), c1 = Math::cos(_y),
                      s2 = Math::sin(_p), c2 = Math::cos(_p),
                      s3 = Math::sin(_r), c3 = Math::cos(_r);

                return { c1*c3 + s1*s2*s3, s1*s2*c3 - c1*s3, s1*c2,
                         c2*s3,            c2*c3,           -s2,   
                         c1*s2*s3 - c3*s1, c1*c3*s2 + s1*s3, c1*c2 };
            }

            mat3 ypr2rotMat3D(const vec3 &_a) {
                return  ypr2rotMat3D(_a.x, _a.y, _a.z);
            }

            mat4 ypr2rotMat3D_4(float _y, float _p, float _r) {
                float s1 = Math::sin(_y), c1 = Math::cos(_y),
                      s2 = Math::sin(_p), c2 = Math::cos(_p),
                      s3 = Math::sin(_r), c3 = Math::cos(_r);

                return { c1*c3 + s1*s2*s3, s1*s2*c3 - c1*s3, s1*c2, 0,
                         c2*s3,            c2*c3,              -s2, 0,
                         c1*s2*s3 - c3*s1, c1*c3*s2 + s1*s3, c1*c2, 0,
                         0,                0,                0,     1 };
            }

            mat4 ypr2rotMat3D_4(const vec3 &_a) {
                return  ypr2rotMat3D_4(_a.x, _a.y, _a.z);
            }

            mat2 rotMat2D(float _theta) {
                return { Math::cos(_theta), Math::sin(_theta),
                        -Math::sin(_theta), Math::cos(_theta) };
            }

            mat3 rotMat2D_3(float _theta) {
                return { Math::cos(_theta), Math::sin(_theta), 0.f,
                        -Math::sin(_theta), Math::cos(_theta), 0.f,
                         0.f,               0.f,               1.f };
            }

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

            mat3 axesRotMat3D(const vec2 &_theta, char _axis1, char _axis2) {
                return axisRotMat3D(_theta.y, _axis2) &
                       axisRotMat3D(_theta.x, _axis1);
            }

            mat3 axesRotMat3D(const vec3 &_theta, char _axis1, char _axis2, char _axis3) {
                return axisRotMat3D(_theta.z, _axis3) &
                       axisRotMat3D(_theta.y, _axis2) &
                       axisRotMat3D(_theta.x, _axis1);
            }

            mat4 axisRotMat3D_4(float _theta, char _axis) {
                return mat3to4(axisRotMat3D(_theta, _axis));
            }

            mat4 axesRotMat3D_4(const vec2 &_theta, char _axis1, char _axis2) {
                return mat3to4(axisRotMat3D(_theta.y, _axis2) &
                               axisRotMat3D(_theta.x, _axis1));
            }

            mat4 axesRotMat3D_4(const vec3 &_theta, char _axis1, char _axis2, char _axis3) {
                return mat3to4(axisRotMat3D(_theta.z, _axis3) &
                               axisRotMat3D(_theta.y, _axis2) &
                               axisRotMat3D(_theta.x, _axis1));
            }
        }; // Rotation END


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

        Transform3D::Transform3D() {}

        Transform3D::Transform3D(const mat4 &_tf) : tform{_tf} {}

        Transform3D::Transform3D(const Transform3D &_tf) : tform{_tf.tform} {}

        Transform3D& Transform3D::translate(const vec3 &_o) {
            tform &= translateMat3D(_o);
            return *this;
        }

        Transform3D& Transform3D::scale(const vec3 &_s) {
            tform &= scaleMat3D(_s);
            return *this;
        }

        Transform3D& Transform3D::rotate(const Affine::Rotation::Quaternion  &_s) {
            tform &= _s.rotMat3D_4();
            return *this;
        }

        Transform3D& Transform3D::rotate(const Affine::Rotation::EulerAngles &_e) {
            tform &= _e.rotMat3D_4();
            return *this;
        }

        Transform3D& Transform3D::reflect() {
            tform &= Reflection::xyz_3D;
            return *this;
        }

        Transform3D& Transform3D::reflect(char _axis) {
            tform &= reflectMat3D(_axis);
            return *this;
        }

        Transform3D& Transform3D::reflect(char _axis1, char _axis2) {
            tform &= reflectMat3D(_axis1, _axis2);
            return *this;
        }

        Transform3D& Transform3D::shear(float xy, float xz, float yx, float yz, float zx, float zy) {
            tform &= shearMat3D(xy, xz, yx, yz, zx, zy);
            return *this;
        }

        Transform3D& Transform3D::shearAngle(float xy, float xz, float yx, float yz, float zx, float zy) {
            tform &= shearAngleMat3D(xy, xz, yx, yz, zx, zy);
            return *this;
        }
    }; // Affine END


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


    namespace Interpolation {

        Slerp::Slerp(const Affine::Rotation::Quaternion &_q1, const Affine::Rotation::Quaternion &_q2) :
                Q1{_q1.normalize()}, Q2{_q2.normalize()} {}

        Slerp::Slerp(const Slerp &_s) : Q1{_s.Q1}, Q2{_s.Q2} {}

        Affine::Rotation::Quaternion Slerp::interpolate(float _u) const {
            return Interpolate(_u, Q1, Q2);
        }

        Affine::Rotation::Quaternion Slerp::Interpolate(float _u, const Affine::Rotation::Quaternion &_q1,
                                                                  const Affine::Rotation::Quaternion &_q2) {
            _u = Math::clampCL(_u, 0.f, 1.f);
            // if greedy, use one variable instead of "norm" and "theta"
            float __norm = Math::sqrt(_q1.norm2() * _q2.norm2());
            if (Math::closeToZero(__norm )) return _q1;

            float __cos = Math::dot(_q1, _q2) / __norm;
            Affine::Rotation::Quaternion __q2 = _q2;
            if (__cos < 0.f) {
                __cos = -__cos;
                __q2  = -1.f * __q2;
            }

            // acos is undefined for |cos| > 1 (possible due to rounding); for small angles linear interpolation is used
            float __theta = Math::acos(Math::clampCL(__cos, -1.f, 1.f));
            if (Math::closeToZero(__theta, SVKFW_EPS5))
                return _q1 * (1.f - _u) + __q2 * _u;

            float __denom = 1.f / Math::sin(__theta);
            return _q1  * (Math::sin((1.f - _u) * __theta) * __denom) +
                   __q2 * (Math::sin(       _u  * __theta) * __denom);
        }
    }; // Interpolation END
}; // Simple END
