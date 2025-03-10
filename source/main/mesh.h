#ifndef SVKFW_MESH_H
#define SVKFW_MESH_H

#include "math/vectors.h"
#include <vector>



namespace Simple {
    struct Mesh {
        std::vector<Vec3> vertices;
        std::vector<UVec3> indices;
    };

    struct Instance {
        uint MeshID;
        Mat4 tform;
    };

    struct Scene {
        std::vector<Vec3> vertices;
        std::vector<UVec3> indices;
        std::vector<UVec2> offsets;

        std::vector<Instance> instances;
    };

    struct MeshScene {
        std::vector<Mesh> meshes;
        std::vector<Instance> instances;

    };
};

#endif