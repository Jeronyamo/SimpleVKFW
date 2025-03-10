#ifndef SVKFW_SAMPLER_H
#define SVKFW_SAMPLER_H

#include "math/vectors.h"


namespace Simple {
    struct Sampler2D {
        
    };


    struct TileSampler3D {
        Vec3 pos, dir, offsetH, offsetV;
        UVec2 wsize, tsize;

        TileSampler3D(const Vec3 &_pos, const Vec3 &_dir, float _fovy_deg, const Vec3 &_up,
                      const UVec2 &_window, const UVec2 &_tile = 8u) :
                      pos{_pos}, dir{_dir}, wsize{_window}, tsize{_tile} {
            
        }
    };

    struct Sampler3D {

    };
};

#endif