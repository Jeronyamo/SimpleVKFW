#ifndef SVKFW_SCENE_GRAPH_H
#define SVKFW_SCENE_GRAPH_H

#include <cstdint>
#include <string>
#include <vector>

#include "math/vectors.h"
#include "main/image.h"


namespace Simple {
    namespace SceneGraph {

//  == === ==== ================================== ==== === ==  \\
                    Scene graph
//  == === ==== ================================== ==== === ==  \\

        // Note: a scene is a set of flat arrays; the objects refer to each other by indices ('SCENE_NONE': no object).
        //       Nodes form a tree (each node has one parent, the roots have none) and carry a local transform;
        //       meshes, lights (and later cameras, skins...) are attached to nodes, so one mesh can be drawn by several nodes.
        //
        //       Conventions (the same as the camera in 'main/camera.h'):
        //         - left-handed world, Y up; positions in scene units (e.g. one PMX unit is ~8 cm);
        //         - matrices are row-vector style like 'mat4' everywhere in the engine: 'p_world = p_local & local & parent...',
        //           so the translation is in M[3][0..2] and a matrix goes to GLSL as is (GLSL reads it column-major);
        //         - texture coordinates have their origin in the top-left corner of the image (Vulkan style);
        //         - front faces are clockwise when looking at them in the (left-handed) world, like in Direct3D and MMD
        //           (see 'SceneRenderer::front_face' for the pipeline side).
        //
        //       Scene scene;
        //       uint32_t stage = loadAssimp(scene, "resources/assets/stage.pmx");                 // see 'scene_assimp.h'
        //       uint32_t chara = loadAssimp(scene, "resources/assets/chara.pmx", ImportOptions(), scene.addNode("chara"));
        //       scene.node(chara).local = translation({0.f, 0.f, 5.f});
        //       scene.addLight(Light::directional({1.f, 0.95f, 0.9f}, 2.f), scene.addNode("sun", SCENE_NONE, lookAlong(...)));
        //       scene.updateTransforms();                                                         // after changing 'local'

        const uint32_t SCENE_NONE = UINT32_MAX;

        mat4 translation(const vec3 &_offset);
        mat4 scaling(const vec3 &_scale);
        // Rotation that turns +Z into '_dir' (normalized inside); '_up' picks the roll. For lights and cameras that look along +Z
        mat4 lookAlong(const vec3 &_dir, const vec3 &_up = {0.f, 1.f, 0.f});
        // Transforms a point (w = 1) or a direction (w = 0) with a row-vector style matrix
        vec3 transformPoint(const mat4 &_m, const vec3 &_p);
        vec3 transformDir  (const mat4 &_m, const vec3 &_d);

        struct AABB {
            vec3 min = { 1e30f,  1e30f,  1e30f};
            vec3 max = {-1e30f, -1e30f, -1e30f};

            bool valid() const;
            void add(const vec3 &_p);
            void add(const AABB &_box);
            vec3 center() const;
            vec3 size() const;
            // The box around the transformed corners
            AABB transformed(const mat4 &_m) const;
        }; // AABB END

        // Note: one interleaved vertex layout for all meshes (48 bytes); 'tangent.w' is the handedness of the bitangent
        //       (bitangent = cross(normal, tangent.xyz) * tangent.w). Meshes without UVs get zero tangents
        struct Vertex {
            vec3 position;
            vec3 normal;
            vec2 uv;
            vec4 tangent;
        }; // Vertex END

        // Note: triangles only (indexed); one material per mesh
        struct Mesh {
            std::string           name;
            std::vector<Vertex>   vertices;
            std::vector<uint32_t> indices;
            uint32_t              material = SCENE_NONE; // SCENE_NONE: the default material (white, opaque)
            AABB                  bounds;                // in the mesh space

            void computeBounds();
        }; // Mesh END

        // Note: decoded RGBA8 texels, top row first. Textures with the same source are shared by the materials
        struct Texture {
            std::string    name;   // the file path (as found) or "*<index>" for embedded textures
            Img::Image4u   image;
            bool           srgb = true;         // color data; normal maps and other data are linear
            bool           has_alpha = false;   // some texel has alpha < 255
            bool           soft_alpha = false;  // some texel has alpha strictly between 0 and 255 (blending needed, not just a cutout)

            // Sets 'has_alpha' and 'soft_alpha' from the texels
            void scanAlpha();
        }; // Texture END

        enum AlphaMode {
            SCENE_ALPHA_OPAQUE = 0, // alpha is ignored
            SCENE_ALPHA_MASK,       // fragments with alpha < 'alpha_cutoff' are discarded, the rest is opaque
            SCENE_ALPHA_BLEND       // blended over what is behind, drawn after the opaque objects, back to front
        };

        // Note: a simple Blinn-Phong material; the base color multiplies the base color texture, the alpha is the opacity.
        //       Lighting = ambient + sum of lights (diffuse + specular) + emissive
        struct Material {
            std::string name;
            vec4        base_color = {1.f, 1.f, 1.f, 1.f};
            vec3        specular   = {0.f, 0.f, 0.f};
            float       shininess  = 32.f;   // Blinn-Phong exponent
            vec3        emissive   = {0.f, 0.f, 0.f};
            float       normal_scale = 1.f;  // strength of the normal map

            uint32_t    base_color_texture = SCENE_NONE;
            uint32_t    normal_texture     = SCENE_NONE; // tangent space, +Y up (OpenGL style)
            uint32_t    emissive_texture   = SCENE_NONE;

            AlphaMode   alpha_mode   = SCENE_ALPHA_OPAQUE;
            float       alpha_cutoff = 0.5f;
            // Blended materials don't write depth by default (sorted back to front instead); MMD models are drawn with depth
            // writes in the material order, fully transparent texels are discarded
            bool        blend_depth_write = false;
            bool        two_sided    = false; // no back face culling (the back faces are lit with the flipped normal)
            bool        visible      = true;  // e.g. parts that are switched off (PMX materials with zero alpha)
        }; // Material END

        enum LightType {
            SCENE_LIGHT_DIRECTIONAL = 0, // along the +Z of its node, no position, no falloff
            SCENE_LIGHT_POINT,           // at the origin of its node
            SCENE_LIGHT_SPOT             // at the origin of its node, along its +Z
        };

        // Note: position and direction come from the node (origin and +Z axis), so lights move with the scene.
        //       Radiance at distance d: color * intensity / max(d^2, 0.01) * window(d / range) (range == 0: no window)
        struct Light {
            std::string name;
            LightType   type      = SCENE_LIGHT_POINT;
            vec3        color     = {1.f, 1.f, 1.f};
            float       intensity = 1.f;
            float       range     = 0.f;    // distance where the light fades out completely, 0: unlimited
            float       inner_cone = 0.4f;  // spot lights: half angles in radians; full intensity inside 'inner_cone'
            float       outer_cone = 0.6f;
            uint32_t    node      = SCENE_NONE;

            static Light directional(const vec3 &_color, float _intensity);
            static Light point(const vec3 &_color, float _intensity, float _range = 0.f);
            static Light spot(const vec3 &_color, float _intensity, float _inner, float _outer, float _range = 0.f);
        }; // Light END

        struct Node {
            std::string           name;
            uint32_t              parent = SCENE_NONE;
            std::vector<uint32_t> children;
            mat4                  local = Mat::Eye4;
            std::vector<uint32_t> meshes;
            uint32_t              light = SCENE_NONE;
            bool                  visible = true; // hides the meshes of the node and all its descendants
        }; // Node END

        struct Scene {
            std::vector<Node>     nodes;
            std::vector<Mesh>     meshes;
            std::vector<Material> materials;
            std::vector<Texture>  textures;
            std::vector<Light>    lights;
            vec3                  ambient = {0.03f, 0.03f, 0.03f}; // constant light everywhere

            // Computed by 'updateTransforms': the world matrix and the world visibility of each node
            std::vector<mat4>     world;
            std::vector<uint8_t>  world_visible;

            uint32_t addNode(const std::string &_name, uint32_t _parent = SCENE_NONE, const mat4 &_local = Mat::Eye4);
            uint32_t addMesh(const Mesh &_mesh);
            uint32_t addMaterial(const Material &_material);
            uint32_t addTexture(const Texture &_texture);
            // Attaches the light to '_node' (a node holds one light)
            uint32_t addLight(const Light &_light, uint32_t _node);

            Node &node(uint32_t _index);
            const Node &node(uint32_t _index) const;
            uint32_t findNode(const std::string &_name) const; // the first one with the name, SCENE_NONE if there is none

            // Moves '_node' under '_new_parent' (SCENE_NONE: makes it a root), keeping its local transform
            void reparent(uint32_t _node, uint32_t _new_parent);

            // Recomputes 'world' and 'world_visible' from the local transforms (parents are not required to come first)
            void updateTransforms();

            // Bounds of the visible meshes in the world (after 'updateTransforms')
            AABB bounds() const;

            // Copies all objects of '_other' into this scene; its roots go under '_parent'. Returns the index of the first copied node
            uint32_t append(const Scene &_other, uint32_t _parent = SCENE_NONE);

            void clear();

        private:
            void updateNode_(uint32_t _index, std::vector<uint8_t> &_done);
        }; // Scene END

    }; // SceneGraph END
}; // Simple END

#endif // SVKFW_SCENE_GRAPH_H
