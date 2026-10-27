#ifndef SVKFW_CAMERA_H
#define SVKFW_CAMERA_H

#include "math/linalg.h"


namespace Simple {
//  ============  Projection interface and functions  ============  \\

    // Note: all projections map depth into Vulkan clip space range [0, 1] (near -> 0, far -> 1),
    //       camera looks along -Z axis of the view space
    struct Projection {
        virtual ~Projection();

        virtual mat4 proj() const;

//TODO: check if current top-bottom signs make the result upside-down, if so, also change ProjOrtho
        static mat4 Orthographic(float _w, float _h, float _n, float _f, float _l = 0.f, float _t = 0.f);

        static mat4 Perspective(float _fovy_deg, float _aspect_ratio, float _near, float _far);

        static mat4 PerspectiveGeneral(float _t, float _b, float _l, float _r, float _near, float _far);
    }; // Projection END


//  ============  Projection classes  ============  \\

    struct ProjOrtho : Projection {
        float w_right, h_bot;
        float p_near, p_far, left, top;

        ProjOrtho(const ProjOrtho &proj) = default;
        ProjOrtho(float _w, float _h, float _n, float _f, float _l = 0.f, float _t = 0.f);

        mat4 proj() const override;
    }; // ProjOrtho END

    struct ProjPerspective : Projection {
        float p_near, p_far, fovy;
        vec2 fb_begin, fb_now;

        ProjPerspective(const ProjPerspective &_proj) = default;
        ProjPerspective(vec2 _reso, float _fovy_deg, float _n, float _f);

        mat4 proj() const override;

        void updateReso(vec2 _reso);
    }; // ProjPerspective END


//  ============  View interface and functions  ============  \\

    struct View {
        virtual ~View();

        virtual mat4 view() const;


        static mat4 ViewMat(const vec3 &_cam_pos, const vec3 &_look_at, const vec3 &_up_vect);
    }; // View END


//  ============  View classes  ============  \\

    struct ViewGeneral : View {
        vec3 cam_pos;
        vec3 look_at;
        vec3 up_vect;

        ViewGeneral(const ViewGeneral &_view) = default;
        ViewGeneral(const vec3 &_cam_pos, const vec3 &_look_at, const vec3 &_up_vect);

        mat4 view() const override;
    }; // ViewGeneral END


    struct ViewCentered : View {
	    float yaw, pitch, dist;
        vec3f look_at, up_vect;

        ViewCentered(const ViewCentered &_view) = default;
        ViewCentered(float _yaw = 0.f, float _pitch = 0.f, float _dist = 5.f, const vec3 &_look_at = { 0.f, 0.f, 0.f }, const vec3 &_up_vect = { 0.f, 1.f, 0.f });

        ViewCentered(const vec3 &_cam_pos, const vec3 &_look_at, const vec3 &_up_vect);


        vec3 calcCamPos() const;

        mat4 view() const override;
    }; // ViewCentered END


    struct ViewPOV : View {
        vec3f cam_pos;
	    float yaw;
        vec3f up_vect;
        float pitch;

        ViewPOV(const ViewPOV &_view) = default;
        ViewPOV(const vec3 &_cam_pos = {}, float _yaw = 0.f,
                float _pitch = 0.f, const vec3 &_up_vect = {0.f, 1.f, 0.f});
        ViewPOV(const vec3 &_cam_pos, const vec3 &_look_at, const vec3 &_up_vect);

        vec3 calcLookAt() const;

        mat4 view() const override;

    }; // ViewPOV END


//  ============  Camera class: projection + view  ============  \\

    template <class P, class V>
    struct Camera {
        P proj_class;
        V view_class;

        static_assert(std::is_base_of<Projection, P>::value && std::is_base_of<View, V>::value, "Camera: P must be a Projection and V must be a View");

        Camera(const P &_proj, const V &_view) : proj_class{_proj}, view_class{_view} {}

        mat4 getProj() const { return proj_class.proj(); }
        mat4 getView() const { return view_class.view(); }
    }; // Camera END
};

#endif