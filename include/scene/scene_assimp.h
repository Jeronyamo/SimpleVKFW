#ifndef SVKFW_SCENE_ASSIMP_H
#define SVKFW_SCENE_ASSIMP_H

#include <string>

#include "scene/scene_graph.h"

struct aiScene;


namespace Simple {
    namespace SceneGraph {

//  == === ==== ================================== ==== === ==  \\
                    Assimp import
//  == === ==== ================================== ==== === ==  \\

        // Note: what is converted now: the node tree with transforms, triangle meshes (positions, normals, the first UV set,
        //       tangents), materials (base color, specular, shininess, emissive, opacity, two-sided, the base color / normal /
        //       emissive textures from files or embedded), lights (directional, point, spot, ambient) and the skeleton with the
        //       bone weights (one skeleton per file, see 'Skin'). Other data (animations, morphs, cameras, extra UV sets, vertex
        //       colors) is skipped for now.
        struct ImportOptions {
            // Coordinate system of the data given by assimp: right-handed for all formats (the MMD importer converts the
            // left-handed PMX data itself). AUTO = RIGHT: Z is mirrored back to the engine's left-handed space
            enum Handedness { HANDEDNESS_AUTO = 0, HANDEDNESS_LEFT, HANDEDNESS_RIGHT };
            Handedness handedness = HANDEDNESS_AUTO;

            float  scale = 1.f;              // applied to the root transform
            bool   gen_smooth_normals = true; // for meshes without normals
            bool   optimize = true;          // join identical vertices, improve the vertex cache order
            float  smoothing_angle = 80.f;   // degrees, for generated normals
            // Materials with an opacity below 1 or a texture with soft alpha are blended, textures with only 0/255 alpha are cut out
            bool   detect_alpha = true;
            // Opacity 0 means "switched off" in PMX models (the part is shown by a morph), such materials get 'visible' = false
            bool   hide_zero_opacity = true;
        }; // ImportOptions END

        // Loads a model file and appends it to '_scene' under '_parent'; returns the root node of the model.
        // Throws std::runtime_error if the file can't be read. Texture files are looked up relative to the model file
        // ('\' separators are accepted); a texture that can't be loaded is reported and the material goes without it
        uint32_t loadAssimp(Scene &_scene, const std::string &_path, const ImportOptions &_options = ImportOptions(),
                            uint32_t _parent = SCENE_NONE);

        // Converts an already loaded aiScene (triangulated, with the post-processing the engine needs: see 'loadAssimp')
        // '_dir' is the folder of external textures; '_left_handed' tells if the data needs no conversion
        uint32_t convertAssimp(Scene &_scene, const aiScene &_ai_scene, const std::string &_dir, bool _left_handed,
                               const ImportOptions &_options = ImportOptions(), uint32_t _parent = SCENE_NONE);

    }; // SceneGraph END
}; // Simple END

#endif // SVKFW_SCENE_ASSIMP_H
