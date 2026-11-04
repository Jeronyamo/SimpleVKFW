#ifndef SVKFW_MODEL_H
#define SVKFW_MODEL_H

#include "math/vectors.h"
#include <vector>
#include <cstdint>


namespace Simple {
    // Point/Line mesh;
    // 'vertices' contains free points first, then line points, starting from 'vertices[indices[0].x]'
    struct Mesh2v {
        std::vector<vec3> vertices;
        std::vector<vec2u> indices;
    };
    // Triangle-only mesh
    struct Mesh3v {
        std::vector<vec3> vertices;
        std::vector<vec3u> indices;
    };
};

#endif