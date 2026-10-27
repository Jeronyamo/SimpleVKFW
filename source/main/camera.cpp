#include "main/camera.h"

namespace Simple {

    Projection::~Projection() {}

    mat4 Projection::proj() const { return Mat::Eye4; }

    mat4 Projection::Orthographic(float _w, float _h, float _n, float _f, float _l, float _t) {
        return { 2.f / (_w - _l), 0.f, 0.f, 0.f,
                 0.f, 2.f / (_t - _h), 0.f, 0.f,
                 0.f, 0.f, 1.f / (_n - _f), 0.f,
                 (_w + _l) / (_l - _w), (_t + _h) / (_h - _t), _n / (_n - _f), 1.f };
    }

    mat4 Projection::Perspective(float _fovy_deg, float _aspect_ratio, float _near, float _far) {
        float _inv_tan = 1.f / Math::tan(_fovy_deg * (M_PI / 360));
        float _inv_nf  = 1.f / (_near - _far);

        // Note: '_aspect_ratio' is width / height
        return { _inv_tan / _aspect_ratio, 0.f,     0.f,  0.f,
                 0.f, _inv_tan,                     0.f,  0.f,
                 0.f, 0.f,              _far * _inv_nf, -1.f,
                 0.f, 0.f,      _far * _near * _inv_nf,  0.f };
    }

    mat4 Projection::PerspectiveGeneral(float _t, float _b, float _l, float _r, float _near, float _far) {
        return { 2 * _near / (_r - _l), 0.f, 0.f, 0.f,
                 0.f, 2 * _near / (_t - _b), 0.f, 0.f,
                 (_r+_l) / (_r-_l), (_t+_b) / (_t-_b), _far / (_near-_far), -1.f,
                 0.f, 0.f, _far * _near / (_near-_far), 0.f };
    }

    ProjOrtho::ProjOrtho(float _w, float _h, float _n, float _f, float _l, float _t)
                    : w_right{_w}, h_bot{_h}, p_near{_n}, p_far{_f}, left{_l}, top{_t} {}

    mat4 ProjOrtho::proj() const {
        mat4 __res{};

        float __v1 = 1.f / (left - w_right),
              __v2 = 1.f / (h_bot - top),
              __v3 = 1.f / (p_near - p_far);

        __res[{0,0}] = -2.f * __v1;
        __res[{1,1}] = -2.f * __v2;
        __res[{2,2}] =          __v3;
        __res[{3,0}] = (w_right + left) * __v1;
        __res[{3,1}] = (top + h_bot)   * __v2;
        __res[{3,2}] =  p_near          * __v3;
        __res[{3,3}] =  1.f;

        return __res;
    }

    ProjPerspective::ProjPerspective(vec2 _reso, float _fovy_deg, float _n, float _f)
                    : p_near{_n}, p_far{_f}, fovy{_fovy_deg}, fb_begin{1024.f, 1024.f}, fb_now{_reso} {}

    mat4 ProjPerspective::proj() const {
        mat4 __res{};

        float _inv_tan = 1.f / Math::tan((M_PI / 360) * fovy);
        float _inv_nf = 1.f / (p_near - p_far);
        vec2 _coef = fb_begin / fb_now;

        __res[{0,0}] = _coef.x * _inv_tan;
        __res[{1,1}] = _coef.y * _inv_tan;
        __res[{2,2}] =  p_far * _inv_nf;
        __res[{2,3}] = -1.f;
        __res[{3,2}] = (p_far * p_near) * _inv_nf;

        return __res;
    }

    void ProjPerspective::updateReso(vec2 _reso) {
        fb_now = _reso;
    }

    View::~View() {}

    mat4 View::view() const { return Mat::Eye4; }

    mat4 View::ViewMat(const vec3 &_cam_pos, const vec3 &_look_at, const vec3 &_up_vect) {
        vec3 f = (_look_at - _cam_pos).normalize();
        vec3 s = Math::cross(_up_vect, f).normalize();
        vec3 u = Math::cross(s, f);

        return { s.x, u.x, -f.x, 0.f,
                 s.y, u.y, -f.y, 0.f,
                 s.z, u.z, -f.z, 0.f,
                -Math::dot(s, _cam_pos),
                -Math::dot(u, _cam_pos),
                 Math::dot(f, _cam_pos), 1.f};
    }

    ViewGeneral::ViewGeneral(const vec3 &_cam_pos, const vec3 &_look_at, const vec3 &_up_vect)
                    : cam_pos{_cam_pos}, look_at{_look_at}, up_vect{_up_vect} {}

    mat4 ViewGeneral::view() const {
        return ViewMat(cam_pos, look_at, up_vect);
    }

    ViewCentered::ViewCentered(float _yaw, float _pitch, float _dist, const vec3 &_look_at, const vec3 &_up_vect)
                    : yaw{_yaw}, pitch{_pitch}, dist{_dist}, look_at{_look_at}, up_vect{_up_vect} {}

    ViewCentered::ViewCentered(const vec3 &_cam_pos, const vec3 &_look_at, const vec3 &_up_vect)
                    : dist{(_look_at - _cam_pos).norm()}, look_at{_look_at}, up_vect{_up_vect} {
        vec3 __front_vec = (_look_at - _cam_pos).normalize();
        pitch = Math::asin(-__front_vec.y) * float(180.f / M_PI);
        yaw = atan2f(__front_vec.x, __front_vec.z) * float(180.f / M_PI);
    }

    vec3 ViewCentered::calcCamPos() const {
        return dist * vec3(-Math::cos(pitch * float(M_PI / 180)) * Math::sin(yaw * float(M_PI / 180)),
                            Math::sin(pitch * float(M_PI / 180)),
                           -Math::cos(pitch * float(M_PI / 180)) * Math::cos(yaw * float(M_PI / 180))).normalize() + look_at;
    }

    mat4 ViewCentered::view() const {
        int __test_pitch = int((Math::abs(pitch) + 90.f) / 180);
        if (__test_pitch < 0) __test_pitch ^= 1;
        bool __inverse_upvec = __test_pitch & 1;

        return ViewMat(calcCamPos(), look_at, __inverse_upvec ? -up_vect : up_vect);
    }

    ViewPOV::ViewPOV(const vec3 &_cam_pos, float _yaw,
            float _pitch, const vec3 &_up_vect)
                    : cam_pos{_cam_pos}, yaw{_yaw}, pitch{_pitch}, up_vect{_up_vect} {}

    ViewPOV::ViewPOV(const vec3 &_cam_pos, const vec3 &_look_at, const vec3 &_up_vect)
                    : cam_pos{_cam_pos}, up_vect{_up_vect} {
        vec3 __front_vec = (_look_at - _cam_pos).normalize();
        pitch = Math::asin(__front_vec.y) * float(180.f / M_PI);
        yaw = atan2f(__front_vec.x, __front_vec.z) * float(180.f / M_PI);
    }

    vec3 ViewPOV::calcLookAt() const {
        return vec3(Math::cos(pitch * float(M_PI / 180)) * Math::sin(yaw * float(M_PI / 180)),
                    Math::sin(pitch * float(M_PI / 180)),
                    Math::cos(pitch * float(M_PI / 180)) * Math::cos(yaw * float(M_PI / 180))).normalize() + cam_pos;
    }

    mat4 ViewPOV::view() const {
        int __test_pitch = int((Math::abs(pitch) + 90.f) / 180);
        if (__test_pitch < 0) __test_pitch ^= 1;
        bool __inverse_upvec = __test_pitch & 1;

        return ViewMat(cam_pos, calcLookAt(), __inverse_upvec ? -up_vect : up_vect);
    }
}; // Simple END
