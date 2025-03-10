#ifndef SOGLFW_CAMERA_H
#define SOGLFW_CAMERA_H

#include "../math/linalg.h"


namespace Simple {
    //  ============  Projection class  ============  \\

    struct Projection {
        Mat4 P;

        Projection() : P{} {}
        Projection(const Mat4 &_proj) : P{_proj} {}
        Projection(const Projection &_proj) : P{_proj.P} {}


//TODO: check if current top-bottom signs make the result upside-down
        static Mat4 Orthographic(float _w, float _h, float _n, float _f, float _l = 0.f, float _t = 0.f) {
            return { 2.f / (_w - _l), 0.f, 0.f, 0.f,
                     0.f, 2.f / (_t - _h), 0.f, 0.f,
                     0.f, 0.f, 2.f / (_n - _f), 0.f,
                     (_w + _l) / (_l - _w), (_t + _h) / (_h - _t), (_f + _n) / (_n - _f), 1.f };
        }

        static Mat4 Perspective(unsigned _w, unsigned _h, float _n, float _f, float _fov_deg) {
            return { (_h / (_w * tanf(M_PI / 180. * _fov_deg))), 0.f, 0.f, 0.f,
                      0.f,      (1.f / tanf(M_PI / 180. * _fov_deg)), 0.f, 0.f,
                      0.f, 0.f,                  (-(_f + _n) / (_f - _n)),-1.f,
                      0.f, 0.f,            (-(2.f * _f * _n) / (_f - _n)), 0.f };
        }
    };


    //  ============  Orthographic projection classes  ============  \\

//TODO: check if current top-bottom signs make the result upside-down
    Mat4 OrthoCompute(float _w, float _h, float _n, float _f, float _l = 0.f, float _t = 0.f) {
        return { 2.f / (_w - _l), 0.f, 0.f, 0.f,
                 0.f, 2.f / (_t - _h), 0.f, 0.f,
                 0.f, 0.f, 2.f / (_n - _f), 0.f,
                 (_w + _l) / (_l - _w), (_t + _h) / (_h - _t), (_f + _n) / (_n - _f), 1.f };
    }

    struct OrthoLite {
        Mat4 P;

        OrthoLite(const Mat4 &proj) : P{proj} {}
        OrthoLite(const OrthoLite &proj) : P{proj.P} {}
        OrthoLite(float _w, float _h, float _n, float _f, float _l = 0.f, float _t = 0.f) :
                    P{OrthoCompute(_w, _h, _n, _f, _l, _t)} {}
        virtual ~OrthoLite() {}


        virtual void ProjUpd(bool force_upd = false) {}

        Mat4 Projection() {
            ProjUpd();
            return P;
        }
    };

    struct OrthoWithParams : OrthoLite {
        float w_right, h_bot;
        float near, far, left, top;
        bool matUpd;

        OrthoWithParams(const OrthoWithParams &proj) :
                    OrthoLite{proj.P}, w_right{proj.w_right}, h_bot{proj.h_bot}, near{proj.near}, far{proj.far}, matUpd{proj.matUpd} {}
        OrthoWithParams(float _w, float _h, float _n, float _f, float _l = 0.f, float _t = 0.f) :
                    OrthoLite{OrthoCompute(_w, _h, _n, _f, _l, _t)}, w_right{_w}, h_bot{_h}, near{_n}, far{_f}, matUpd{false} {}


        void ProjUpd(bool force_upd = false) override {
            if (matUpd || force_upd) {
                matUpd = false;
                P = OrthoCompute(w_right, h_bot, near, far, left, top);
            }
        }
    };


    //  ============  Perspective projection classes  ============  \\

    Mat4 PerspectiveCompute(unsigned _w, unsigned _h, float _n, float _f, float _fov_deg) {
        return {(_h / (_w * tanf(M_PI / 180. * _fov_deg))), 0.f, 0.f, 0.f,
                 0.f,      (1.f / tanf(M_PI / 180. * _fov_deg)), 0.f, 0.f,
                 0.f, 0.f,                   (-(_f + _n) / (_f - _n)),-1.f,
                 0.f, 0.f,             (-(2.f * _f * _n) / (_f - _n)), 0.f};
    }

    struct PerspectiveLite {
        Mat4 P;

        PerspectiveLite(const Mat4 &proj) : P{proj} {}
        PerspectiveLite(const PerspectiveLite &proj) : P{proj.P} {}
        PerspectiveLite(unsigned _w, unsigned _h, float _n, float _f, float _fov_deg) :
                    P{PerspectiveCompute(_w, _h, _n, _f, _fov_deg)} {}
        virtual ~PerspectiveLite() {}


        virtual void ProjUpd(bool force_upd = false) {}

        Mat4 Projection() {
            ProjUpd();
            return P;
        }
    };

    struct PerspectiveWithParams : PerspectiveLite {
        unsigned width, height;
        float near, far, fov;
        bool matUpd;

        PerspectiveWithParams(const PerspectiveWithParams &proj) :
                    PerspectiveLite{proj.P}, width{proj.width}, height{proj.height},
                    near{proj.near}, far{proj.far}, fov{proj.fov}, matUpd{proj.matUpd} {}
        PerspectiveWithParams(unsigned _w, unsigned _h, float _n, float _f, float _fov_deg) :
                    PerspectiveLite{PerspectiveCompute(_w, _h, _n, _f, _fov_deg)},
                    width{_w}, height{_h}, near{_n}, far{_f}, fov{_fov_deg}, matUpd{false} {}


        void ProjUpd(bool force_upd = false) override {
            if (matUpd || force_upd) {
                matUpd = false;
                float tanHalfFovyInv = 1.f / tanf(M_PI / 180. * fov);
                P[0] = height * tanHalfFovyInv / width;
                P[5] = tanHalfFovyInv;
                P[10] = -(far + near) / (far - near);
                P[14] = -(2.f * far * near) / (far - near);
            }
        }
    };


    //  ============  Camera classes  ============  \\

    Mat4 ViewCompute(const Vec3 &_CamPos, const Vec3 &_LookAt, const Vec3 &_UpVect) {
            Vec3 f = Vec3(_LookAt.x - _CamPos.x, _LookAt.y - _CamPos.y, _LookAt.z - _CamPos.z).normalize();
            Vec3 s = cross(f, _UpVect).normalize();
            Vec3 u = cross(s, f);

            return { s.x, u.x, -f.x, 0.f,
                     s.y, u.y, -f.y, 0.f,
                     s.z, u.z, -f.z, 0.f,
                   -(s.x * _CamPos.x + s.y * _CamPos.y + s.z * _CamPos.z),
                   -(u.x * _CamPos.x + u.y * _CamPos.y + u.z * _CamPos.z),
                     f.x * _CamPos.x + f.y * _CamPos.y + f.z * _CamPos.z, 1.f};
    }

    struct CameraLite {
        Mat4 V;

        CameraLite(const Mat4 &view) : V{view} {}
        CameraLite(const CameraLite &cam) : V{cam.V} {}
        CameraLite(const Vec3 &_CamPos, const Vec3 &_LookAt, const Vec3 &_UpVect) : V{ViewCompute(_CamPos, _LookAt, _UpVect)} {}
        virtual ~CameraLite() {}

        virtual void ViewUpd(bool force_upd = false) {}

        Mat4 View() {
            ViewUpd();
            return V;
        }
    };

    struct CameraWithParams : CameraLite {
        Vec3 CamPos;
        Vec3 LookAt;
        Vec3 UpVect;
        bool matUpd;

        CameraWithParams(const CameraWithParams &cam) :
                    CameraLite{cam.V}, CamPos{cam.CamPos}, LookAt{cam.LookAt}, UpVect{cam.UpVect}, matUpd{cam.matUpd} {}
        CameraWithParams(const Vec3 &_CamPos, const Vec3 &_LookAt, const Vec3 &_UpVect) :
                    CameraLite{ViewCompute(_CamPos, _LookAt, _UpVect)}, CamPos{_CamPos}, LookAt{_LookAt}, UpVect{_UpVect}, matUpd{false} {}


        void ViewUpd(bool force_upd = false) {
            if (matUpd || force_upd)  {
                matUpd = false;
                V = ViewCompute(CamPos, LookAt, UpVect);
            }
        }
    };

    struct CameraCentered : CameraLite {
        Vec3 CamPos;
        Vec3 LookAt;
        Vec3 UpVect;
	    float yaw, pitch, dist;
        bool matUpd;

        CameraCentered(const CameraCentered &cam) :
                    CameraLite{cam.V}, CamPos{cam.CamPos}, LookAt{cam.LookAt}, UpVect{cam.UpVect},
                    yaw{cam.yaw}, pitch{cam.pitch}, dist{cam.dist}, matUpd{cam.matUpd} {}
        //TODO: Pitch might need "-atan2f()"
        CameraCentered(const Vec3 &_CamPos, const Vec3 &_LookAt, const Vec3 &_UpVect) :
                    CameraLite{ViewCompute(_CamPos, _LookAt, _UpVect)}, CamPos{_CamPos}, LookAt{_LookAt}, UpVect{_UpVect},
                    yaw{asinf(_CamPos.y - _LookAt.y)}, pitch{atan2f(_LookAt.x - _CamPos.x, _LookAt.z - _CamPos.z)},
                    dist{(_CamPos - _LookAt).norm()}, matUpd{false} {}
        CameraCentered(float _yaw, float _pitch, float _dist, const Vec3 &_LookAt, const Vec3 &_UpVect) :
                    CameraLite{Mat4()}, LookAt{_LookAt}, UpVect{_UpVect}, yaw{_yaw}, pitch{_pitch}, dist{_dist}, matUpd{false} {
            ViewUpd(true);
        }


        void CamPosUpd() {
            CamPos = dist * Vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)).normalize() + LookAt;
        }

        void ViewUpd(bool force_upd = false) {
            if (matUpd || force_upd)  {
                CamPosUpd();
                matUpd = false;
                V = ViewCompute(CamPos, LookAt, UpVect);
            }
        }
    };
    //TODO: MAYBE if CamPos/LookAt roles in CameraCentered are reversed, we will get FPS camera
};

#endif