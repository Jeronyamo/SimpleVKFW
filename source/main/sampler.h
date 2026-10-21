#ifndef SVKFW_MAIN_SAMPLER_H
#define SVKFW_MAIN_SAMPLER_H // Note: differs from sound/sampler.h guard

#include "math/vectors.h"


namespace Simple {
    struct Sampler2D {
        
    };


    struct TileSampler3D {
        vec3 pos, dir, offsetH, offsetV;
        vec2u wsize, tsize;

        TileSampler3D(const vec3 &_pos, const vec3 &_dir, float _fovy_deg, const vec3 &_up,
                      const vec2u &_window, const vec2u &_tile = 8u) :
                      pos{_pos}, dir{_dir}, wsize{_window}, tsize{_tile} {
            
        }
    };

    struct Sampler3D {

    };
};

#endif