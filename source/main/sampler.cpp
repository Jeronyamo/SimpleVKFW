#include "main/sampler.h"

namespace Simple {

    TileSampler3D::TileSampler3D(const vec3 &_pos, const vec3 &_dir, float _fovy_deg, const vec3 &_up,
                  const vec2u &_window, const vec2u &_tile) :
                  pos{_pos}, dir{_dir}, wsize{_window}, tsize{_tile} {

    }
}; // Simple END
