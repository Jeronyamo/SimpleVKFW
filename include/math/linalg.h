#ifndef SVKFW_LINALG_H
#define SVKFW_LINALG_H

#include "math/vectors.h"


/*  ==============================================  *\
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

\*  ==============================================  */
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
            vec3 quaternion2tangent(const Quaternion &_q);
            Quaternion tangent2quaternion(const vec3 &_v);

    // Quaternion class

            struct Quaternion {
                float s = 1.f, i = 0.f, j = 0.f, k = 0.f; // Default - zero rotation quaternion

                Quaternion();
                Quaternion(const Quaternion &_quat);
                Quaternion(float _s, float _i, float _j, float _k);
                template <class Y> Quaternion(const Vec3Base<Y> &_v) : s{ 0.f}, i{_v.x}, j{_v.y}, k{_v.z} {}
                template <class Y> Quaternion(const Vec4Base<Y> &_v) : s{_v.x}, i{_v.y}, j{_v.z}, k{_v.w} {}
                template <class Y> Quaternion(float _scal, const Vec3Base<Y> &_vec)
                                : s{_scal}, i{_vec.x}, j{_vec.y}, k{_vec.z} {}
                template <class Y> Quaternion(const Vec3Base<Y> &_axis, float _a) {
                    vec3 __axis{_axis.normalize()};
                    _a *= 0.5f;

                    s  = Math::cos(_a);
                    _a = Math::sin(_a);
                    i = _a * __axis.x;
                    j = _a * __axis.y;
                    k = _a * __axis.z;
                }


                vec3 vecPart() const;
                Quaternion conjugate() const;
                Quaternion scalar()    const;
                Quaternion vector()    const;
                void toConjugate();
                void toScalar();
                void toVector();

                float norm () const;
                float norm2() const;

                Quaternion normalize() const;

                void toNormalized();

                Quaternion inverse() const;

                void toInverted();

                vec3 rotateCanonical(const vec3 &_vec) const;

                vec3 rotateByUnit(const vec3 &_vec) const;

                vec3 rotate(const vec3 &_vec) const;

                mat3 rotMat3D() const;

                mat4 rotMat3D_4() const;

                mat4 rotMat4DLeft()  const;

                mat4 rotMat4DRight() const;

                vec4 axisAngle() const;

                Quaternion operator+(const Quaternion &op2) const;
                Quaternion operator-(const Quaternion &op2) const;
                Quaternion operator*(const Quaternion &op2) const;
                template <class C> Quaternion operator*(const Vec3Base <C> &op2) const {
                    return { -i * op2.x - j * op2.y - k * op2.z,
                              s * op2.x + j * op2.z - k * op2.y,
                              s * op2.y + k * op2.x - i * op2.z,
                              s * op2.z + i * op2.y - j * op2.x };
                }
            }; // Quaternion END

            // Note: a scalar is a quaternion with zero vector part, so only the scalar part is affected by +/-
            Quaternion operator+(float _s, const Quaternion &_q);
            Quaternion operator-(float _s, const Quaternion &_q);
            Quaternion operator*(float _s, const Quaternion &_q);

            Quaternion operator+(const Quaternion &_q, float _s);
            Quaternion operator-(const Quaternion &_q, float _s);
            Quaternion operator*(const Quaternion &_q, float _s);
            Quaternion operator/(const Quaternion &_q, float _s);


//  ============  Euler (Tait–Bryan) angles  ============  \\

    // Forward declarations

            struct EulerAngles;

            Quaternion ypr2quaternion(float _y, float _p, float _r);
            mat3 ypr2rotMat3D(float _y, float _p, float _r);


    // Euler angles class

            struct EulerAngles {
                float yaw = 0, pitch = 0, roll = 0;

                EulerAngles();
                EulerAngles(float _y, float _p, float _r);
                EulerAngles(const vec3 &_a);
                EulerAngles(const vec3 &_axis, float _angle);
                EulerAngles(const mat3 &_rot);
                EulerAngles(const mat4 &_rot);
                EulerAngles(const Quaternion &_q);

                operator vec3();


                mat3 rotMat3D() const;

                mat4 rotMat3D_4() const;

                Quaternion quaternion() const;
            }; // EulerAngles END


//  ============  Rotation-related functions  ============  \\

            // Returns a unit vector perpendicular to '_v' (any of them)
            vec3 anyPerpendicular(const vec3 &_v);

            // Returns { axis.x, axis.y, axis.z, angle }
            vec4 axisAngleBetweenVecs(const vec3 &_v1, const vec3 &_v2);


//  ============  Quaternion-based rotation functions  ============  \\

    // Returns quaternion's tangent vector used in SLERP
            vec3 quaternion2tangent(const Quaternion &_q);
            Quaternion tangent2quaternion(const vec3 &_v);

    // Returns quaternion representing the shortest rotation between two vectors
            Quaternion rotationBetweenVecs(const vec3 &_v1, const vec3 &_v2);

    // Returns quaternion representing shortest rotation between two unit vectors
            Quaternion rotationBetweenUnitVecs(const vec3 &_v1, const vec3 &_v2);


//  ============  Quaternion -> Rotation matrix  ============  \\

            mat3 quaternion2rotMat3D(const Quaternion &_q);

            mat4 quaternion2rotMat3D_4(const Quaternion &_q);


//  ============  Quaternion <-> Euler angles  ============  \\

            Quaternion ypr2quaternion(const EulerAngles &_ypr);

            Quaternion ypr2quaternion(float _y, float _p, float _r);

            EulerAngles quaternion2ypr(const Quaternion &_q);


//  ============  Euler angles -> Rotation matrix  ============  \\

            // YXZ
            mat3 ypr2rotMat3D(float _y, float _p, float _r);

            mat3 ypr2rotMat3D(const vec3 &_a);

            // YXZ
            mat4 ypr2rotMat3D_4(float _y, float _p, float _r);

            mat4 ypr2rotMat3D_4(const vec3 &_a);


//  ============  XYZ/Any convention angles -> Rotation matrix  ============  \\

    // clockwise rotation
            mat2 rotMat2D(float _theta);

    // clockwise rotation
            mat3 rotMat2D_3(float _theta);

    // rotation matrix around axis 'x', 'y' or 'z'
            mat3 axisRotMat3D(float _theta, char _axis);

    // rotation matrix around axes 'x', 'y' or 'z'
            mat3 axesRotMat3D(const vec2 &_theta, char _axis1, char _axis2);

    // rotation matrix around axes 'x', 'y' or 'z'
            mat3 axesRotMat3D(const vec3 &_theta, char _axis1, char _axis2, char _axis3);

    // rotation matrix around axis 'x', 'y' or 'z'
            mat4 axisRotMat3D_4(float _theta, char _axis);

    // rotation matrix around axes 'x', 'y' or 'z'
            mat4 axesRotMat3D_4(const vec2 &_theta, char _axis1, char _axis2);

    // rotation matrix around axes 'x', 'y' or 'z'
            mat4 axesRotMat3D_4(const vec3 &_theta, char _axis1, char _axis2, char _axis3);

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

        mat3 translateMat2D(const vec2 &_o);

        mat4 translateMat3D(const vec3 &_o);


//  ============  Scaling  ============  \\

        mat3 scaleMat2D(const vec2 &_o);

        mat4 scaleMat3D(const vec3 &_o);


//  ============  Shear  ============  \\

        mat4 shearMat3D(float xy, float xz, float yx, float yz, float zx, float zy);

        mat4 shearAngleMat3D(float xy, float xz, float yx, float yz, float zx, float zy);


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

        mat3 reflectMat2D(char _axis);

        mat4 reflectMat3D(char _axis);

        mat4 reflectMat3D(char _axis1, char _axis2);


//  ============  Transform class  ============  \\

        struct Transform3D {
            mat4 tform{Mat::Eye4};


            Transform3D();
            Transform3D(const mat4 &_tf);
            Transform3D(const Transform3D &_tf);


            Transform3D& translate(const vec3 &_o);

            Transform3D& scale(const vec3 &_s);


            Transform3D& rotate(const Affine::Rotation::Quaternion  &_s);

            Transform3D& rotate(const Affine::Rotation::EulerAngles &_e);


            Transform3D& reflect();

            Transform3D& reflect(char _axis);

            Transform3D& reflect(char _axis1, char _axis2);


            Transform3D& shear(float xy, float xz, float yx, float yz, float zx, float zy);

            Transform3D& shearAngle(float xy, float xz, float yx, float yz, float zx, float zy);
        }; // Transform3D END
    }; // Affine END



//  General quaternion functions

    namespace Math {
        float dot(const Affine::Rotation::Quaternion &_q1, const Affine::Rotation::Quaternion &_q2);

        Affine::Rotation::Quaternion pow(const Affine::Rotation::Quaternion &_q, float _p);

        Affine::Rotation::Quaternion powUnitQuat(const Affine::Rotation::Quaternion &_q, float _p);
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
                _t = Math::clampCL(_t, 0.f, 1.f);
                return P0 + _t * (P1 - P0);
            }

            static V Interpolate(float _t, const V &_p0, const V &_p1) {
                _t = Math::clampCL(_t, 0.f, 1.f);
                return _p0 + _t * (_p1 - _p0);
            }
        }; // BezierLinear END

        template <class V>
        struct BezierQuadratic : Bezier<V> {
            V P0, P1, P2;

            BezierQuadratic(const V &_p0, const V &_p1, const V &_p2) : P0{_p0}, P1{_p1}, P2{_p2} {}
            BezierQuadratic(const BezierLinear<V> &_l) : P0{_l.P0}, P1{0.5f * (_l.P0 + _l.P1)}, P2{_l.P1} {}
            BezierQuadratic(const BezierQuadratic<V> &_c) : P0{_c.P0}, P1{_c.P1}, P2{_c.P2} {}

            V interpolate(float _t) const override {
                _t = Math::clampCL(_t, 0.f, 1.f);
                return P1 + Math::sqr(1.f - _t) * (P0 - P1) + (_t * _t) * (P2 - P1);
            }

            static V Interpolate(float _t, const V &_p0, const V &_p1, const V &_p2) {
                _t = Math::clampCL(_t, 0.f, 1.f);
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
                _t = Math::clampCL(_t, 0.f, 1.f);
                return Math::cube(1.f - _t) *  P0 +
                             3 * (1.f - _t) * _t * ( P1 + _t * ( P2 -  P1)) + (_t * _t * _t) *  P3;
            }

            static V Interpolate(float _t, const V &_p0, const V &_p1, const V &_p2, const V &_p3) {
                _t = Math::clampCL(_t, 0.f, 1.f);
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

            Slerp(const Affine::Rotation::Quaternion &_q1, const Affine::Rotation::Quaternion &_q2);
            Slerp(const Slerp &_s);

            Affine::Rotation::Quaternion interpolate(float _u) const;

            // Note: interpolates along the shortest path ('q' and '-q' represent the same rotation)
            static Affine::Rotation::Quaternion Interpolate(float _u, const Affine::Rotation::Quaternion &_q1,
                                                                      const Affine::Rotation::Quaternion &_q2);

        }; // Slerp END
    }; // Interpolation EnD
}; // Simple END

#endif