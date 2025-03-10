#ifndef SOGLFW_TEXTURE_H
#define SOGLFW_TEXTURE_H

#include "image.h"


namespace Simple {
    struct TextureBase2D {
        virtual ColorA texcolor(const Vec2 &texcoord) { return 0.f; }
    };

    //TODO: full OpenGL TEXTURE2D stuff support
    struct Texture2D : TextureBase2D {
        Image4 tex;

        Texture2D(const char *filename) : tex(filename) {}

        ColorA texcolor(const Vec2 &texcoord) { return tex[texcoord]; }
    };

    struct SingleColor : TextureBase2D {
        ColorA color;

        SingleColor(const ColorA &_col) : color{_col} {}

        ColorA texcolor(const Vec2 &texcoord) { return color; }
    };

    struct GradientLinear : TextureBase2D {
        ColorA col1, col2;
        Vec2 pt1, pt2;

        GradientLinear(const ColorA &_c1, const ColorA &_c2, const Vec2 &_p1, const Vec2 &_p2) :
            col1{_c1}, col2{_c2}, pt1{_p1}, pt2{_p2} {}

        ColorA texcolor(const Vec2 &texcoord) {}
    };

    struct GradientCenter : TextureBase2D {
        ColorA col1, col2;
        Vec2 center;
        float radius, radinv2;

        GradientCenter(const ColorA &_c1, const ColorA &_c2, const Vec2 &_c, float _rad) :
            col1{_c1}, col2{_c2}, center{_c}, radius{_rad}, radinv2{2.f / _rad} {}

        ColorA texcolor(const Vec2 &texcoord) {
            auto dist = (texcoord - center).norm() * radinv2;
            auto alpha = 2.f * (dist - int(dist));
            alpha = alpha > 1.f ? 2.f - alpha : alpha;
            return alpha * col1 + (1.f - alpha) * col2;
        }
    };

    struct TexBaseMakerInterface {
        TextureBase2D *ptr = nullptr;

        TexBaseMakerInterface(TextureBase2D *_ptr) : ptr{_ptr} {}
        TexBaseMakerInterface(TexBaseMakerInterface &_texbase) : ptr{_texbase.ptr} {}
        TexBaseMakerInterface(const char *fname) : ptr{new Texture2D{fname}} {}
        TexBaseMakerInterface(const ColorA &_col) : ptr{new SingleColor{_col}} {}
        TexBaseMakerInterface(const ColorA &_c1, const ColorA &_c2, const Vec2 &_p1, const Vec2 &_p2) : ptr{new GradientLinear{_c1, _c2, _p1, _p2}} {}
        TexBaseMakerInterface(const ColorA &_c1, const ColorA &_c2, const Vec2 &_p1, float radius) : ptr{new GradientCenter{_c1, _c2, _p1, radius}} {}
    };
};


#endif