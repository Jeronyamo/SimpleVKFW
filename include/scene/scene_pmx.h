#ifndef SVKFW_SCENE_PMX_H
#define SVKFW_SCENE_PMX_H

#include <string>

#include "scene/scene_graph.h"
#include "scene/scene_assimp.h"


namespace Simple {
    namespace File { struct ContentPMX; };

    namespace SceneGraph {

//  == === ==== ================================== ==== === ==  \\
                    PMX import
//  == === ==== ================================== ==== === ==  \\

        // Note: MMD models through the engine's own PMX reader ('File::ReaderWriterPMX'): assimp drops what posing needs
        //       (IK chains, grants, deform layers, SDEF) and the material flags. The data stays in the MMD space, which is the
        //       engine's one (left-handed, Y up; models face -Z).
        //       Result: one root node (with the model name) that holds one mesh per material and the skin; the skeleton keeps
        //       all PMX bones with their names (parents first, so the indices may differ from the file: see 'findBone').
        //       Used options: scale, detect_alpha, hide_zero_opacity. Morphs: vertex, UV, bone, group and material ones (the
        //       material morphs change diffuse, specular and the texture tint, see 'Scene::morphMaterials'). Rigid bodies and
        //       joints go to the skeleton ('Skeleton::rigid_bodies', 'joints'; simulated by 'systems/physics_scene.h').
        //       Not converted yet: edges, sphere and toon textures

        // Loads a PMX file and appends it to '_scene' under '_parent'; returns the root node of the model.
        // Asserts (std::runtime_error) if the file can't be read
        uint32_t loadPMX(Scene &_scene, const std::string &_path, const ImportOptions &_options = ImportOptions(),
                         uint32_t _parent = SCENE_NONE);

        // Converts read PMX data; '_dir' is the folder of its textures
        uint32_t convertPMX(Scene &_scene, const File::ContentPMX &_pmx, const std::string &_dir,
                            const ImportOptions &_options = ImportOptions(), uint32_t _parent = SCENE_NONE);

        // PMX files with 'loadPMX', everything else with 'loadAssimp'
        uint32_t loadModel(Scene &_scene, const std::string &_path, const ImportOptions &_options = ImportOptions(),
                           uint32_t _parent = SCENE_NONE);

    }; // SceneGraph END
}; // Simple END

#endif // SVKFW_SCENE_PMX_H
