#ifndef SVKFW_MESH_H
#define SVKFW_MESH_H

#include "math/vectors.h"
#include <vector>
#include <cstdint>



namespace Simple {
    struct Mesh {
        std::vector<vec3> vertices;
        std::vector<vec3u> indices;
    };

    struct Instance {
        uint32_t MeshID;
        mat4 tform;
    };

    struct Scene {
        std::vector<vec3> vertices;
        std::vector<vec3u> indices;
        std::vector<vec2u> offsets;

        std::vector<Instance> instances;
    };

    struct MeshScene {
        std::vector<Mesh> meshes;
        std::vector<Instance> instances;

    };
};

#endif