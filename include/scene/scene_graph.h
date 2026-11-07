#ifndef SVKFW_SCENE_GRAPH_H
#define SVKFW_SCENE_GRAPH_H

#include <cstdint>
#include <string>
#include <vector>

#include "math/vectors.h"
#include "math/linalg.h"
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
        //       scene.setLocal(chara, translation({0.f, 0.f, 5.f}));
        //       scene.addLight(Light::directional({1.f, 0.95f, 0.9f}, 2.f), scene.addNode("sun", SCENE_NONE, lookAlong(...)));
        //       scene.updateTransforms();                       // recomputes only the changed nodes and their descendants

        const uint32_t SCENE_NONE = UINT32_MAX;

        typedef Affine::Rotation::Quaternion Quat;

        mat4 translation(const vec3 &_offset);
        mat4 scaling(const vec3 &_scale);
        // Rotation that turns +Z into '_dir' (normalized inside); '_up' picks the roll. For lights and cameras that look along +Z
        mat4 lookAlong(const vec3 &_dir, const vec3 &_up = {0.f, 1.f, 0.f});
        // Transforms a point (w = 1) or a direction (w = 0) with a row-vector style matrix
        vec3 transformPoint(const mat4 &_m, const vec3 &_p);
        vec3 transformDir  (const mat4 &_m, const vec3 &_d);
        // Row-vector style rotation matrix of a quaternion ('rotation(q1 * q2) == rotation(q2) & rotation(q1)': q2 goes first)
        mat4 rotation(const Quat &_q);
        // The rotation of the upper 3x3 part (the scale is removed first); the inverse of 'rotation'
        Quat rotationOf(const mat4 &_m);
        // Rotation, then translation: 'rotation(_q) & translation(_p)'
        mat4 rigidTransform(const Quat &_q, const vec3 &_p);
        // The translation part (M[3][0..2]) and the uniform scale (the length of the first row) of a matrix
        inline vec3  translationOf (const mat4 &_m) { return { _m.M[3][0], _m.M[3][1], _m.M[3][2] }; }
        inline float uniformScaleOf(const mat4 &_m) {
            return std::sqrt(_m.M[0][0] * _m.M[0][0] + _m.M[0][1] * _m.M[0][1] + _m.M[0][2] * _m.M[0][2]);
        }

        // Note: translation, rotation, scale; the matrix is 'scale & rotation & translation' (scale first)
        struct TRS {
            vec3 translation = {0.f, 0.f, 0.f};
            Quat rotation;
            vec3 scale       = {1.f, 1.f, 1.f};

            mat4 matrix() const;
            // Splits a matrix without shear (with shear the result is approximate)
            static TRS fromMatrix(const mat4 &_m);
        }; // TRS END

        struct AABB {
            vec3 min = { 1e30f,  1e30f,  1e30f};
            vec3 max = {-1e30f, -1e30f, -1e30f};

            bool valid() const;
            void add(const vec3 &_p);
            void add(const AABB &_box);
            vec3 center() const;
            vec3 size() const;
            // The box around the transformed box (exact for affine matrices)
            AABB transformed(const mat4 &_m) const;
        }; // AABB END

        // 'AABB::transformed' on plain floats (Arvo: the center is transformed, the half size goes through |M|), for hot
        // loops (e.g. the bone boxes of skinned meshes in the culling)
        void transformBounds(const float _min[3], const float _max[3], const mat4 &_m, float _out_min[3], float _out_max[3]);

        // Note: one interleaved vertex layout for all meshes (48 bytes); 'tangent.w' is the handedness of the bitangent
        //       (bitangent = cross(normal, tangent.xyz) * tangent.w). Meshes without UVs get zero tangents
        struct Vertex {
            vec3 position;
            vec3 normal;
            vec2 uv;
            vec4 tangent;
        }; // Vertex END

        enum SkinMode : uint32_t {
            SCENE_SKIN_LINEAR = 0,   // linear blend skinning (LBS): the bone matrices are blended (MMD BDEF1/2/4)
            SCENE_SKIN_DUAL_QUATERNION // dual quaternion skinning (DQS): no collapse at bent joints (MMD SDEF and QDEF)
        };

        // Note: the bones (indices in the skeleton of the skin) and their weights of one vertex; unused slots have zero weight
        struct SkinVertex {
            uint16_t bones[4];
            vec4     weights;
            uint32_t mode;           // SkinMode
        }; // SkinVertex END

        // Note: spherical deform (MMD SDEF) of a vertex with two bones (the first two of its 'SkinVertex'): the rotations of the
        //       bones are blended around 'center' instead of the matrices, so joints don't collapse. 'center.w' = 1 marks an SDEF
        //       vertex; 'r0', 'r1' are the corrected PMX R0/R1 points (see 'loadPMX'). Kept for a later SDEF mode, the renderer
        //       draws these vertices with DQS for now
        struct SdefVertex {
            vec4 center;
            vec3 r0, r1;
        }; // SdefVertex END

        // Note: the offset of one vertex in one morph of the skeleton ('Skeleton::morphs'); a vertex morph moves the position,
        //       a UV morph the texture coordinates (MMD keeps the normals). Applied on the GPU with the weights of the pose
        struct MorphTarget {
            uint32_t morph  = SCENE_NONE; // in the skeleton of the skin
            uint32_t vertex = 0u;         // in the mesh
            vec3     position = {0.f, 0.f, 0.f};
            vec2     uv       = {0.f, 0.f};
        }; // MorphTarget END

        // Note: triangles only (indexed); one material per mesh
        struct Mesh {
            std::string             name;
            std::vector<Vertex>     vertices;
            std::vector<uint32_t>   indices;
            std::vector<SkinVertex> skin;                  // empty or one per vertex: a skinned mesh (see 'Skin')
            std::vector<SdefVertex> sdef;                  // empty or one per vertex (skinned meshes with SDEF vertices)
            std::vector<MorphTarget> morphs;               // skinned meshes: the vertex / UV morph offsets, sorted by vertex
            uint32_t                material = SCENE_NONE; // SCENE_NONE: the default material (white, opaque)
            AABB                    bounds;                // in the mesh space (the bind pose for skinned meshes)

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
            // Texture tint (MMD material morphs): the base color texture becomes 'clamp(texel * texture_mul + texture_add)'
            vec4        texture_mul  = {1.f, 1.f, 1.f, 1.f};
            vec4        texture_add  = {0.f, 0.f, 0.f, 0.f};
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

        // Note: the bones of a skeleton are not scene nodes: a skeleton is posed as a whole (animation, IK) and has many bones.
        //       Bones are stored parents first; a bone's 'rest' is its local transform in the bind pose (relative to the parent
        //       bone, or to the skeleton space for root bones). 'inverse_bind' maps the skeleton space to the bone space in
        //       the bind pose; skinned vertices are in the skeleton space
        //       Grant (MMD 付与): the bone adds 'grant_ratio' of the rotation / translation of 'grant_parent' to its own.
        //       Deform order (MMD): bones after physics last, then by 'layer', then by index; grants and IK follow it
        struct Bone {
            std::string name;
            uint32_t    parent = SCENE_NONE;
            TRS         rest;
            mat4        inverse_bind = Mat::Eye4;

            int32_t     layer = 0;
            bool        after_physics = false;
            uint32_t    grant_parent  = SCENE_NONE;
            float       grant_ratio   = 0.f;
            bool        grant_rotation = false, grant_translation = false;
            uint32_t    ik = SCENE_NONE;   // the IK chain solved when this bone is reached in the deform order
        }; // Bone END

        enum IKSolver {
            SCENE_IK_CCD = 0,  // iterative, any chain length, joint limits (MMD)
            SCENE_IK_TWO_BONE  // analytic, for two links (arms, legs); the bend direction comes from the current pose
        };

        // Note: a link with limits keeps its rotation (Euler XYZ, radians) in [limit_min, limit_max]; a link limited around one
        //       axis only (knees) is solved in that plane
        struct IKLink {
            uint32_t bone = SCENE_NONE;
            bool     limited = false;
            vec3     limit_min = {0.f, 0.f, 0.f}, limit_max = {0.f, 0.f, 0.f};
        }; // IKLink END

        // Note: rotates 'links' (from the effector side up, MMD order) so that 'effector' reaches 'goal'
        struct IKChain {
            std::string         name;
            IKSolver            solver = SCENE_IK_CCD;
            uint32_t            goal = SCENE_NONE;     // the IK bone: the position to reach
            uint32_t            effector = SCENE_NONE; // the bone that has to reach it
            uint32_t            iterations = 10u;
            float               limit_angle = 3.14159265f; // radians per link and iteration (CCD)
            std::vector<IKLink> links;
            bool                enabled = true;
        }; // IKChain END

        enum MorphType {
            SCENE_MORPH_VERTEX = 0, // moves vertices ('Mesh::morphs')
            SCENE_MORPH_UV,         // moves texture coordinates ('Mesh::morphs')
            SCENE_MORPH_BONE,       // offsets bones ('bones')
            SCENE_MORPH_GROUP,      // drives other morphs ('group'), e.g. a blink made of the eyes and the eyebrows
            SCENE_MORPH_MATERIAL,   // material colors ('materials', see 'Scene::morphMaterials')
            SCENE_MORPH_OTHER       // not supported (MMD flip, impulse, additional UVs)
        };

        struct MorphBoneOffset {
            uint32_t bone = SCENE_NONE;
            vec3     translation = {0.f, 0.f, 0.f};
            Quat     rotation;
        }; // MorphBoneOffset END

        // Note: MMD material morph of one material: with 'add' the values times the weight are added, otherwise each value is
        //       multiplied by 'lerp(1, value, weight)'. All multiplications go first, then the additions. Ambient, edge, sphere and
        //       toon values of PMX are not used by the renderer
        struct MorphMaterialOffset {
            uint32_t material = SCENE_NONE; // in the scene
            bool     add = false;
            vec4     diffuse  = {1.f, 1.f, 1.f, 1.f}; // base color and alpha
            vec3     specular = {1.f, 1.f, 1.f};
            float    shininess = 1.f;
            vec4     texture_tint = {1.f, 1.f, 1.f, 1.f};
        }; // MorphMaterialOffset END

        struct MorphGroupEntry {
            uint32_t morph  = SCENE_NONE;
            float    weight = 1.f;
        }; // MorphGroupEntry END

        // Note: a named shape of the model (MMD 表情): the weights are in the pose ('Pose::morph_weights', usually 0..1)
        struct Morph {
            std::string                  name;
            MorphType                    type = SCENE_MORPH_VERTEX;
            std::vector<MorphBoneOffset> bones;
            std::vector<MorphGroupEntry> group;
            std::vector<MorphMaterialOffset> materials;
        }; // Morph END

        enum RigidShape { SCENE_RIGID_SPHERE = 0, SCENE_RIGID_BOX, SCENE_RIGID_CAPSULE };

        enum RigidMode {
            SCENE_RIGID_FOLLOW_BONE = 0,       // kinematic: placed by its bone (or the skeleton space without a bone)
            SCENE_RIGID_PHYSICS,               // dynamic: drives its bone
            SCENE_RIGID_PHYSICS_BONE_POSITION  // dynamic: drives the rotation of its bone, the bone keeps its animated position
        };

        // Note: a rigid body of the skeleton (MMD physics), in the skeleton space and the bind pose. Shapes: sphere
        //       (size.x radius), box (size = half extents), capsule along the local Y (size.x radius, size.y length of the
        //       cylinder part). 'collision_mask' bit i: collides with the bodies of group i. Simulated by 'systems/physics_scene.h'
        struct RigidBodyDesc {
            std::string name;
            uint32_t    bone  = SCENE_NONE;
            uint32_t    group = 0u;
            uint32_t    collision_mask = 0xFFFFu;
            RigidShape  shape = SCENE_RIGID_SPHERE;
            vec3        size  = {1.f, 1.f, 1.f};
            vec3        position = {0.f, 0.f, 0.f};
            Quat        rotation;
            float       mass = 1.f, linear_damping = 0.f, angular_damping = 0.f, restitution = 0.f, friction = 0.5f;
            RigidMode   mode = SCENE_RIGID_FOLLOW_BONE;
        }; // RigidBodyDesc END

        // Note: a 6DOF spring joint between two rigid bodies of the skeleton; the frame is in the skeleton space (bind pose).
        //       Limits in the joint frame (angles: Euler XYZ, radians): min > max frees an axis, min == max locks it;
        //       springs: stiffness per axis, 0: none
        struct JointDesc {
            std::string name;
            uint32_t    body_a = SCENE_NONE, body_b = SCENE_NONE;
            vec3        position = {0.f, 0.f, 0.f};
            Quat        rotation;
            vec3        linear_min  = {0.f, 0.f, 0.f}, linear_max  = {0.f, 0.f, 0.f};
            vec3        angular_min = {0.f, 0.f, 0.f}, angular_max = {0.f, 0.f, 0.f};
            vec3        linear_spring = {0.f, 0.f, 0.f}, angular_spring = {0.f, 0.f, 0.f};
        }; // JointDesc END

        struct Skeleton {
            std::string          name;
            std::vector<Bone>    bones;
            std::vector<IKChain> iks;
            std::vector<Morph>   morphs;
            std::vector<RigidBodyDesc> rigid_bodies; // physics (MMD), see 'systems/physics_scene.h'
            std::vector<JointDesc>     joints;

            // Computed by 'finalize'
            std::vector<uint32_t>              order;    // the deform order
            std::vector<std::vector<uint32_t>> children;

            // The parent must already be in the skeleton; returns the index of the bone
            uint32_t addBone(const Bone &_bone);
            // Attaches the chain to its goal bone; returns its index
            uint32_t addIK(const IKChain &_ik);
            uint32_t findBone (const std::string &_name) const; // SCENE_NONE if there is none
            uint32_t findMorph(const std::string &_name) const; // SCENE_NONE if there is none
            // Sets 'inverse_bind' of all bones from the rest transforms
            void computeInverseBind();
            // Inverse bind matrices, deform order, children (after adding bones and IK chains)
            void finalize();
        }; // Skeleton END

        // Note: the state of one posed skeleton. 'local' is the animated pose (starts at the rest transforms); after changing it
        //       call 'Scene::updateSkins' (or 'computeFK') to get 'model' (bone -> skeleton space) and 'skinning'
        //       ('inverse_bind & model', for the vertices). The final local rotation is 'ik * local * morph * grant' (grant
        //       first); 'morph_weights' drive the morphs of the skeleton (bone morphs here, vertex and UV ones in the renderer)
        struct Pose {
            std::vector<TRS>  local;
            std::vector<mat4> model;
            std::vector<mat4> skinning;
            std::vector<Quat> ik_rotation, grant_rotation; // computed
            std::vector<vec3> grant_translation;           // computed
            std::vector<uint8_t> ik_enabled;               // per IK chain of the skeleton (animations switch them); from 'IKChain::enabled'
            std::vector<float>   morph_weights;            // per morph of the skeleton (input)
            std::vector<float>   morph_final;              // computed: the weights with the groups resolved (the renderer reads them)
            std::vector<TRS>     morphed;                  // computed: 'local' with the bone morphs
            // Physics ('systems/physics_scene.h'): with 'use_physics' the bones with 'physics_override' take 'physics_model'
            // (their model matrix) instead of their local transform; their descendants follow them
            std::vector<uint8_t> physics_override;
            std::vector<mat4>    physics_model;
            bool              use_physics = false;
            bool              dirty = true;
            bool              solve_ik = true;
            uint64_t          revision = 0u; // counts 'computeFK' calls (e.g. culling notices a changed pose)

            void reset(const Skeleton &_skeleton);  // back to the rest pose
            // Forward kinematics with grants and IK chains in the deform order
            void computeFK(const Skeleton &_skeleton);

        private:
            // Resolves the groups into 'morph_final' and applies the bone morphs to 'morphed'
            void applyMorphs_(const Skeleton &_skeleton);
            void addMorph_(const Skeleton &_skeleton, uint32_t _morph, float _weight, uint32_t _depth);
            mat4 localMatrix_(const Skeleton &_skeleton, uint32_t _bone) const;
            void updateModel_(const Skeleton &_skeleton, uint32_t _bone);    // the bone and its descendants
            void applyGrant_(const Skeleton &_skeleton, uint32_t _bone);
            void solveCCD_(const Skeleton &_skeleton, const IKChain &_ik);
            void solveTwoBone_(const Skeleton &_skeleton, const IKChain &_ik);
            // Rotates '_bone' by '_angle' around '_axis' (in its own space, after its current rotation) through its IK rotation
            void rotateLink_(const Skeleton &_skeleton, uint32_t _bone, const vec3 &_axis, float _angle);
            // Keeps the IK rotation of a limited link inside its limits (Euler XYZ); '_previous' is the angle of the last step
            void limitLink_(const Skeleton &_skeleton, const IKLink &_link, float _limit_angle, vec3 &_previous);
        }; // Pose END

        // Note: a skeleton instance placed in the scene: the skeleton space is the space of 'node'. The skinned meshes of the
        //       nodes that refer to the skin are drawn in that space (their own node gives only the visibility, like in glTF)
        struct Skin {
            std::string name;
            uint32_t    skeleton = SCENE_NONE;
            uint32_t    node     = SCENE_NONE;
            Pose        pose;
        }; // Skin END

        struct Node {
            std::string           name;
            uint32_t              parent = SCENE_NONE;
            std::vector<uint32_t> children;
            mat4                  local = Mat::Eye4;  // after a direct change call 'Scene::markDirty' (or use 'Scene::setLocal')
            std::vector<uint32_t> meshes;
            uint32_t              light = SCENE_NONE;
            uint32_t              skin  = SCENE_NONE; // the skin of the skinned meshes of this node
            bool                  visible = true; // hides the meshes of the node and all its descendants ('Scene::setVisible')
        }; // Node END

        struct Scene {
            std::vector<Node>     nodes;
            std::vector<Mesh>     meshes;
            std::vector<Material> materials;
            std::vector<Texture>  textures;
            std::vector<Light>    lights;
            std::vector<Skeleton> skeletons;
            std::vector<Skin>     skins;
            vec3                  ambient = {0.03f, 0.03f, 0.03f}; // constant light everywhere

            // Computed by 'updateTransforms': the world matrix and the world visibility of each node
            std::vector<mat4>     world;
            std::vector<uint8_t>  world_visible;
            // The nodes recomputed by the last 'updateTransforms' (parents before children), e.g. for refitting the bounds
            std::vector<uint32_t> changed;

            uint32_t addNode(const std::string &_name, uint32_t _parent = SCENE_NONE, const mat4 &_local = Mat::Eye4);
            uint32_t addMesh(const Mesh &_mesh);
            uint32_t addMaterial(const Material &_material);
            uint32_t addTexture(const Texture &_texture);
            // Attaches the light to '_node' (a node holds one light)
            uint32_t addLight(const Light &_light, uint32_t _node);
            uint32_t addSkeleton(const Skeleton &_skeleton);
            // Places an instance of '_skeleton' in the space of '_node' (in the rest pose)
            uint32_t addSkin(uint32_t _skeleton, uint32_t _node, const std::string &_name = std::string());

            Pose &pose(uint32_t _skin); // marks the pose as changed
            // Recomputes the changed poses (forward kinematics)
            void updateSkins();

            Node &node(uint32_t _index);
            const Node &node(uint32_t _index) const;
            uint32_t findNode(const std::string &_name) const; // the first one with the name, SCENE_NONE if there is none

            // Moves '_node' under '_new_parent' (SCENE_NONE: makes it a root), keeping its local transform
            void reparent(uint32_t _node, uint32_t _new_parent);

            // Change the local transform / the visibility of a node and mark it for 'updateTransforms'
            void setLocal(uint32_t _node, const mat4 &_local);
            void setVisible(uint32_t _node, bool _visible);
            // Marks a node whose 'local' or 'visible' was changed directly; its descendants are recomputed with it
            void markDirty(uint32_t _node);
            // Makes the next 'updateTransforms' recompute the whole tree (e.g. after editing 'nodes' by hand)
            void markAllDirty();

            // Recomputes 'world' and 'world_visible' of the marked nodes and their descendants (the whole tree after
            // 'append', 'clear' or 'markAllDirty'); fills 'changed'. Parents are not required to come before children in 'nodes'
            void updateTransforms();

            // The materials after the material morphs of all skins ('Pose::morph_final', after 'updateSkins'): '_out' gets one per
            // scene material. Returns false (and leaves '_out' untouched) if no material morph has a weight
            bool morphMaterials(std::vector<Material> &_out) const;

            // Bounds of the visible meshes in the world (after 'updateTransforms')
            AABB bounds() const;

            // Copies all objects of '_other' into this scene; its roots go under '_parent'. Returns the index of the first copied node
            uint32_t append(const Scene &_other, uint32_t _parent = SCENE_NONE);

            void clear();

        private:
            // Note: 'depth_' is the number of ancestors; 'stamp_' marks the nodes done by the current update
            std::vector<uint32_t> depth_, dirty_, stamp_;
            uint32_t              stamp_count_ = 0u;
            bool                  full_update_ = true;

            void updateSubtree_(uint32_t _index);
            void updateDepth_(uint32_t _index);
        }; // Scene END

    }; // SceneGraph END
}; // Simple END

#endif // SVKFW_SCENE_GRAPH_H
