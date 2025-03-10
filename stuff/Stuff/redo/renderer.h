#ifndef SOGLFW_RENDERER_H
#define SOGLFW_RENDERER_H

#include "primitives.h"
#include "randvec.h"

#include <vector>


namespace Simple {
    struct RayCaster2D {
        Vec2 retval;
        UVec2 curr, size;
        Vec2 start, offset;

        RayCaster2D(const UVec2 &_size, const Vec2 &_start, const Vec2 &_stop) :
            retval{_start}, curr{0u}, size{_size}, start{_start}, offset{(_stop - _start) / _size} {}
        RayCaster2D(const Vec2 &_start, const Vec2 &_offset, const UVec2 &_size) :
            retval{_start}, curr{0u}, size{_size}, start{_start}, offset{_offset} {}
        RayCaster2D(const RayCaster2D &_rc) : retval{_rc.retval}, curr{_rc.curr}, size{_rc.size}, start{_rc.start}, offset{_rc.offset} {}

        RayCaster2D& operator++() {
            curr.x = ++curr.x == size.x ? 0u : curr.x;
            curr.y += !curr.x;
            retval = curr.x ? Vec2(retval.x + offset.x, retval.y) : Vec2(start.x, retval.y + offset.y);
            return *this;
        }
        RayCaster2D  operator++(int) {RayCaster2D _ret = *this; ++(*this); return _ret;}
        bool operator==(const RayCaster2D &_rc) const { return curr.y == _rc.size.y; }
        bool operator!=(const RayCaster2D &_rc) const { return curr.y != _rc.size.y; }
        Vec2 operator*() const { return retval; }
        RayCaster2D begin() { return *this; }
        RayCaster2D end() { return *this; }
    };

    struct Sampler2D {
        Vec2 pixelPt, pixelSize;
        unsigned count;

        Sampler2D(const Vec2 &_pixel, const Vec2 &_pixSize, const unsigned _count) :
                pixelPt{_pixel}, pixelSize{_pixSize}, count{_count} {}
        Sampler2D(const Sampler2D &_s) : pixelPt{_s.pixelPt}, pixelSize{_s.pixelSize}, count{_s.count} {}

        Sampler2D& operator++() { --count; return *this; }
        Sampler2D  operator++(int) {Sampler2D _ret = *this; --count; return _ret;}
        bool operator==(const Sampler2D &_rc) const { return !count; }
        bool operator!=(const Sampler2D &_rc) const { return count; }
        Vec2 operator*() const { return pixelPt + pixelSize * rand2f(); }
        Sampler2D begin() { return *this; }
        Sampler2D end() { return *this; }
    };

    struct Layer2D {
        std::vector <Primitive2D*> objects;

        Layer2D() {}
        Layer2D(const Layer2D &_l) : objects{_l.objects} {}
        Layer2D(const std::initializer_list<Primitive2D*> &_obj) : objects{_obj} {}
    };

    struct Renderer2D {
        Vec2 ULcorner, step;
        UVec2 size;
        std::vector<Layer2D> layers;

        Renderer2D(const Renderer2D &_rend) : ULcorner{_rend.ULcorner}, step{_rend.step}, size{_rend.size}, layers{_rend.layers} {}
        Renderer2D(Vec2 _ul, Vec2 _step, UVec2 _size, const std::vector<Layer2D> &_l) : ULcorner{_ul}, step{_step}, size{_size}, layers{_l} {}

        Image4 render() {
            const int Nsamples = 7;
            const float sampleCoef = 1.f / Nsamples;
            Image4 res{size.x, size.y};
            Image4 layerimg{size.x, size.y};

            for (auto layer : layers) {
                ColorA *ptr = layerimg.image;

                for (auto ray : RayCaster2D(ULcorner, step, size)) {
                    Vec4 rescol{0.f};

                    for (auto sample : Sampler2D(ray, step, Nsamples)) {
                        for (auto obj : layer.objects) {
                            ColorA tmp = obj->intersect(sample);
                            rescol += Vec4{tmp.r, tmp.g, tmp.b, tmp.a};
                        }
                    }

                    rescol *= sampleCoef;
                    *ptr++ = {rescol.x, rescol.y, rescol.z, rescol.w};
                }
                res = res << layerimg;
            }
            return res;
        }
    };
};

#endif