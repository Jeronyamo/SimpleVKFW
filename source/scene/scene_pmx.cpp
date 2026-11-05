#include "scene/scene_pmx.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <stdexcept>

#include "data/filetypes.h"


namespace Simple {
    namespace SceneGraph {
        namespace {
            typedef File::ContentPMX PMX;

            vec3 toVec3(const vec3f &_v) { return { _v.x, _v.y, _v.z }; }

            std::string directoryOf(const std::string &_path) {
                size_t __sep = _path.find_last_of("/\\");
                return __sep == std::string::npos ? std::string() : _path.substr(0u, __sep + 1u);
            }

            // PMX texture paths are relative to the model and written on Windows ('tx\body.png')
            std::string texturePath(const std::string &_dir, std::string _file) {
                std::replace(_file.begin(), _file.end(), '\\', '/');
                bool __absolute = !_file.empty() && (_file[0] == '/' || (_file.size() > 1u && _file[1] == ':'));
                return __absolute ? _file : _dir + _file;
            }

            struct PMXConverter {
                Scene               &scene;
                const PMX           &pmx;
                std::string          dir;
                const ImportOptions &options;

                std::vector<uint32_t> textures;  // PMX texture -> scene texture (SCENE_NONE: not loaded yet or failed)
                std::vector<uint8_t>  texture_tried;
                std::vector<uint32_t> bone_map;  // PMX bone -> skeleton bone
                // Vertex and UV morph offsets per PMX vertex (CSR: the offsets of vertex v are [morph_first[v], morph_first[v + 1]))
                std::vector<uint32_t>    morph_first;
                std::vector<MorphTarget> morph_targets; // 'vertex' is the PMX vertex here

                PMXConverter(Scene &_scene, const PMX &_pmx, const std::string &_dir, const ImportOptions &_options)
                    : scene(_scene), pmx(_pmx), dir(_dir), options(_options),
                      textures(_pmx.textures.size(), SCENE_NONE), texture_tried(_pmx.textures.size(), 0u) {}

                uint32_t texture(int32_t _index) {
                    if (_index < 0 || (size_t)_index >= pmx.textures.size()) return SCENE_NONE;
                    if (texture_tried[_index]) return textures[_index];
                    texture_tried[_index] = 1u;
                    Texture __tex;
                    __tex.name = texturePath(dir, pmx.textures[_index]);
                    __tex.srgb = true;
                    if (loadTextureFile(__tex, __tex.name)) textures[_index] = scene.addTexture(__tex);
                    else fprintf(stderr, "SceneGraph::loadPMX: can't load texture '%s'\n", __tex.name.c_str());
                    return textures[_index];
                }

                uint32_t material(const PMX::Material &_m) {
                    Material __mat;
                    __mat.name       = _m.name;
                    __mat.base_color = { _m.diffuse.x, _m.diffuse.y, _m.diffuse.z, _m.diffuse.w };
                    __mat.specular   = toVec3(_m.specular);
                    if (_m.specular_power > 0.f) __mat.shininess = _m.specular_power;
                    __mat.base_color_texture = texture(_m.texture);
                    __mat.two_sided  = (_m.flags & PMX::MATERIAL_NO_CULL) != 0u;
                    if (options.detect_alpha) {
                        const Texture *__tex = __mat.base_color_texture == SCENE_NONE ? nullptr : &scene.textures[__mat.base_color_texture];
                        if (__mat.base_color.w < 1.f || (__tex && __tex->soft_alpha)) __mat.alpha_mode = SCENE_ALPHA_BLEND;
                        else if (__tex && __tex->has_alpha)                            __mat.alpha_mode = SCENE_ALPHA_MASK;
                    }
                    // MMD draws the materials in the file order with depth writes
                    __mat.blend_depth_write = true;
                    // Opacity 0: a part switched off (shown by a morph)
                    if (options.hide_zero_opacity && __mat.base_color.w <= 0.f) __mat.visible = false;
                    return scene.addMaterial(__mat);
                }

                void addBone(uint32_t _index, Skeleton &_skeleton, std::vector<uint8_t> &_state) {
                    if (_state[_index] == 2u) return;
                    SVKFW_ASSERT(_state[_index] == 0u, std::runtime_error, "SceneGraph :: loadPMX", "a loop in the bone parents at '" + pmx.bones[_index].name + "'");
                    _state[_index] = 1u;
                    const PMX::Bone &__b = pmx.bones[_index];
                    const bool __has_parent = __b.parent >= 0 && (size_t)__b.parent < pmx.bones.size();
                    if (__has_parent) addBone((uint32_t)__b.parent, _skeleton, _state);
                    Bone __bone;
                    __bone.name   = __b.name;
                    __bone.parent = __has_parent ? bone_map[__b.parent] : SCENE_NONE;
                    __bone.rest.translation = toVec3(__b.position) - (__has_parent ? toVec3(pmx.bones[__b.parent].position) : vec3{0.f, 0.f, 0.f});
                    __bone.layer         = __b.layer;
                    __bone.after_physics = (__b.flags & PMX::BONE_AFTER_PHYSICS) != 0u;
                    bone_map[_index] = _skeleton.addBone(__bone);
                    _state[_index] = 2u;
                }

                uint32_t skeleton() {
                    Skeleton __skeleton;
                    __skeleton.name = pmx.name;
                    bone_map.assign(pmx.bones.size(), SCENE_NONE);
                    std::vector<uint8_t> __state(pmx.bones.size(), 0u);
                    for (uint32_t i = 0u; i < (uint32_t)pmx.bones.size(); ++i) addBone(i, __skeleton, __state);
                    // Grants and IK chains (the indices are known now)
                    for (size_t i = 0u; i < pmx.bones.size(); ++i) {
                        const PMX::Bone &__b = pmx.bones[i];
                        Bone &__bone = __skeleton.bones[bone_map[i]];
                        if ((__b.flags & (PMX::BONE_GRANT_ROTATION | PMX::BONE_GRANT_TRANSLATION)) && __b.grant_parent >= 0 &&
                            (size_t)__b.grant_parent < pmx.bones.size() && (size_t)__b.grant_parent != i) {
                            __bone.grant_parent      = bone_map[__b.grant_parent];
                            __bone.grant_ratio       = __b.grant_ratio;
                            __bone.grant_rotation    = (__b.flags & PMX::BONE_GRANT_ROTATION) != 0u;
                            __bone.grant_translation = (__b.flags & PMX::BONE_GRANT_TRANSLATION) != 0u;
                        }
                    }
                    for (size_t i = 0u; i < pmx.bones.size(); ++i) {
                        const PMX::Bone &__b = pmx.bones[i];
                        if (!(__b.flags & PMX::BONE_IK) || __b.ik_target < 0 || (size_t)__b.ik_target >= pmx.bones.size()) continue;
                        IKChain __ik;
                        __ik.name        = __b.name;
                        __ik.goal        = bone_map[i];
                        __ik.effector    = bone_map[__b.ik_target];
                        __ik.iterations  = (uint32_t)std::max(__b.ik_loops, 1);
                        __ik.limit_angle = __b.ik_limit_angle > 0.f ? __b.ik_limit_angle : 3.14159265f;
                        for (size_t l = 0u; l < __b.ik_links.size(); ++l) {
                            const PMX::IKLink &__l = __b.ik_links[l];
                            if (__l.bone < 0 || (size_t)__l.bone >= pmx.bones.size()) continue;
                            IKLink __link;
                            __link.bone      = bone_map[__l.bone];
                            __link.limited   = __l.limited;
                            __link.limit_min = toVec3(__l.limit_min);
                            __link.limit_max = toVec3(__l.limit_max);
                            __ik.links.push_back(__link);
                        }
                        if (!__ik.links.empty()) __skeleton.addIK(__ik);
                    }
                    morphs(__skeleton);
                    physics(__skeleton);
                    __skeleton.finalize();
                    return scene.addSkeleton(__skeleton);
                }

                // PMX Euler angles (radians): q = qY * qX * qZ (Z first)
                static Quat eulerPMX(const vec3f &_r) {
                    return Quat(vec3{0.f, 1.f, 0.f}, _r.y) * Quat(vec3{1.f, 0.f, 0.f}, _r.x) * Quat(vec3{0.f, 0.f, 1.f}, _r.z);
                }

                // Rigid bodies and joints keep the PMX order (joints refer to it); bones are remapped
                void physics(Skeleton &_skeleton) {
                    _skeleton.rigid_bodies.reserve(pmx.rigid_bodies.size());
                    for (size_t i = 0u; i < pmx.rigid_bodies.size(); ++i) {
                        const PMX::RigidBody &__b = pmx.rigid_bodies[i];
                        RigidBodyDesc __d;
                        __d.name   = __b.name;
                        __d.bone   = __b.bone >= 0 && (size_t)__b.bone < pmx.bones.size() ? bone_map[__b.bone] : SCENE_NONE;
                        __d.group  = std::min<uint32_t>(__b.group, 15u);
                        __d.collision_mask = __b.collision_mask;
                        __d.shape  = (RigidShape)__b.shape;
                        __d.size   = toVec3(__b.size);
                        __d.position = toVec3(__b.position);
                        __d.rotation = eulerPMX(__b.rotation);
                        __d.mass            = __b.mass;
                        __d.linear_damping  = __b.linear_damping;
                        __d.angular_damping = __b.angular_damping;
                        __d.restitution     = __b.restitution;
                        __d.friction        = __b.friction;
                        __d.mode   = (RigidMode)__b.mode;
                        _skeleton.rigid_bodies.push_back(__d);
                    }
                    const int32_t __count = (int32_t)pmx.rigid_bodies.size();
                    for (size_t i = 0u; i < pmx.joints.size(); ++i) {
                        const PMX::Joint &__j = pmx.joints[i];
                        if (__j.body_a < 0 || __j.body_a >= __count || __j.body_b < 0 || __j.body_b >= __count || __j.body_a == __j.body_b) continue;
                        JointDesc __d;
                        __d.name     = __j.name;
                        __d.body_a   = (uint32_t)__j.body_a;
                        __d.body_b   = (uint32_t)__j.body_b;
                        __d.position = toVec3(__j.position);
                        __d.rotation = eulerPMX(__j.rotation);
                        __d.linear_min  = toVec3(__j.linear_min);  __d.linear_max  = toVec3(__j.linear_max);
                        __d.angular_min = toVec3(__j.angular_min); __d.angular_max = toVec3(__j.angular_max);
                        __d.linear_spring  = toVec3(__j.linear_spring);
                        __d.angular_spring = toVec3(__j.angular_spring);
                        _skeleton.joints.push_back(__d);
                    }
                }

                // Morphs keep the PMX order (group entries refer to it); vertex and UV offsets go to the meshes ('mesh')
                void morphs(Skeleton &_skeleton) {
                    const size_t __vertices = pmx.vertices.size();
                    std::vector<uint32_t> __counts(__vertices + 1u, 0u);
                    for (size_t i = 0u; i < pmx.morphs.size(); ++i) {
                        const PMX::Morph &__m = pmx.morphs[i];
                        for (size_t o = 0u; o < __m.vertices.size(); ++o) if (__m.vertices[o].vertex < __vertices) ++__counts[__m.vertices[o].vertex];
                        if (__m.type == PMX::MORPH_UV)
                            for (size_t o = 0u; o < __m.uvs.size(); ++o) if (__m.uvs[o].vertex < __vertices) ++__counts[__m.uvs[o].vertex];
                    }
                    morph_first.assign(__vertices + 1u, 0u);
                    for (size_t v = 0u; v < __vertices; ++v) morph_first[v + 1u] = morph_first[v] + __counts[v];
                    morph_targets.resize(morph_first[__vertices]);
                    std::vector<uint32_t> __fill(morph_first.begin(), morph_first.end() - 1);

                    for (size_t i = 0u; i < pmx.morphs.size(); ++i) {
                        const PMX::Morph &__m = pmx.morphs[i];
                        Morph __morph;
                        __morph.name = __m.name;
                        switch (__m.type) {
                            case PMX::MORPH_VERTEX:   __morph.type = SCENE_MORPH_VERTEX;   break;
                            case PMX::MORPH_UV:       __morph.type = SCENE_MORPH_UV;       break;
                            case PMX::MORPH_BONE:     __morph.type = SCENE_MORPH_BONE;     break;
                            case PMX::MORPH_GROUP:    __morph.type = SCENE_MORPH_GROUP;    break;
                            case PMX::MORPH_MATERIAL: __morph.type = SCENE_MORPH_MATERIAL; break;
                            default:                  __morph.type = SCENE_MORPH_OTHER;
                        }
                        for (size_t o = 0u; o < __m.vertices.size(); ++o) {
                            const uint32_t __v = __m.vertices[o].vertex;
                            if (__v >= __vertices) continue;
                            MorphTarget &__t = morph_targets[__fill[__v]++];
                            __t.morph    = (uint32_t)i;
                            __t.vertex   = __v;
                            __t.position = toVec3(__m.vertices[o].position);
                        }
                        if (__m.type == PMX::MORPH_UV)
                            for (size_t o = 0u; o < __m.uvs.size(); ++o) {
                                const uint32_t __v = __m.uvs[o].vertex;
                                if (__v >= __vertices) continue;
                                MorphTarget &__t = morph_targets[__fill[__v]++];
                                __t.morph  = (uint32_t)i;
                                __t.vertex = __v;
                                __t.uv     = { __m.uvs[o].uv.x, __m.uvs[o].uv.y };
                            }
                        for (size_t o = 0u; o < __m.bones.size(); ++o) {
                            if (__m.bones[o].bone < 0 || (size_t)__m.bones[o].bone >= pmx.bones.size()) continue;
                            MorphBoneOffset __offset;
                            __offset.bone        = bone_map[__m.bones[o].bone];
                            __offset.translation = toVec3(__m.bones[o].translation);
                            __offset.rotation    = __m.bones[o].rotation;
                            __morph.bones.push_back(__offset);
                        }
                        if (__m.type == PMX::MORPH_GROUP)
                            for (size_t o = 0u; o < __m.group.size(); ++o) {
                                if (__m.group[o].morph < 0 || (size_t)__m.group[o].morph >= pmx.morphs.size()) continue;
                                MorphGroupEntry __entry;
                                __entry.morph  = (uint32_t)__m.group[o].morph;
                                __entry.weight = __m.group[o].weight;
                                __morph.group.push_back(__entry);
                            }
                        _skeleton.morphs.push_back(__morph);
                    }
                }

                // The vertices of one material (only the ones its faces use)
                uint32_t mesh(uint32_t _first_index, const PMX::Material &_m, uint32_t _material, std::vector<uint32_t> &_remap) {
                    Mesh __mesh;
                    __mesh.name     = _m.name;
                    __mesh.material = _material;
                    bool __has_sdef = false;
                    const uint32_t __end = std::min<uint32_t>(_first_index + _m.index_count, (uint32_t)pmx.indices.size());
                    for (uint32_t i = _first_index; i < __end; ++i) {
                        const uint32_t __v = pmx.indices[i];
                        if (__v >= pmx.vertices.size()) { __mesh.indices.push_back(0u); continue; }
                        if (_remap[__v] == SCENE_NONE) {
                            _remap[__v] = (uint32_t)__mesh.vertices.size();
                            const PMX::Vertex &__p = pmx.vertices[__v];
                            Vertex __vertex;
                            __vertex.position = toVec3(__p.position);
                            __vertex.normal   = toVec3(__p.normal);
                            __vertex.uv       = { __p.uv.x, __p.uv.y };
                            __vertex.tangent  = { 0.f, 0.f, 0.f, 0.f }; // no normal maps in PMX
                            __mesh.vertices.push_back(__vertex);

                            SkinVertex __skin;
                            float __w[4] = { 0.f, 0.f, 0.f, 0.f };
                            const int __count = __p.deform == PMX::DEFORM_BDEF1 ? 1 : (__p.deform == PMX::DEFORM_BDEF2 || __p.deform == PMX::DEFORM_SDEF) ? 2 : 4;
                            for (int b = 0; b < 4; ++b) {
                                const bool __valid = b < __count && __p.bones[b] >= 0 && (size_t)__p.bones[b] < pmx.bones.size();
                                __skin.bones[b] = (uint16_t)(__valid ? bone_map[__p.bones[b]] : 0u);
                                __w[b] = __valid ? std::max(__p.weights[b], 0.f) : 0.f;
                            }
                            const float __sum = __w[0] + __w[1] + __w[2] + __w[3];
                            __skin.weights = __sum > 0.f ? vec4{ __w[0] / __sum, __w[1] / __sum, __w[2] / __sum, __w[3] / __sum } : vec4{1.f, 0.f, 0.f, 0.f};
                            // BDEF: linear; SDEF and QDEF: dual quaternions (SDEF approximated, see 'SdefVertex')
                            __skin.mode = (__p.deform == PMX::DEFORM_SDEF || __p.deform == PMX::DEFORM_QDEF) ? SCENE_SKIN_DUAL_QUATERNION : SCENE_SKIN_LINEAR;
                            __mesh.skin.push_back(__skin);

                            // SDEF: the R0/R1 correction of the MMD runtime, so the blended rotation pivots on the weighted center
                            SdefVertex __sdef;
                            __sdef.center = {0.f, 0.f, 0.f, 0.f};
                            __sdef.r0 = __sdef.r1 = {0.f, 0.f, 0.f};
                            if (__p.deform == PMX::DEFORM_SDEF) {
                                const float __w0 = __skin.weights.x, __w1 = 1.f - __w0;
                                const vec3 c = toVec3(__p.sdef_c), r0 = toVec3(__p.sdef_r0), r1 = toVec3(__p.sdef_r1);
                                const vec3 __rw = r0 * __w0 + r1 * __w1;
                                __sdef.center = { c.x, c.y, c.z, 1.f };
                                __sdef.r0 = (c + (c + r0 - __rw)) * 0.5f;
                                __sdef.r1 = (c + (c + r1 - __rw)) * 0.5f;
                                __has_sdef = true;
                            }
                            __mesh.sdef.push_back(__sdef);

                            for (uint32_t t = morph_first[__v]; t < morph_first[__v + 1u]; ++t) {
                                MorphTarget __target = morph_targets[t];
                                __target.vertex = _remap[__v];
                                __mesh.morphs.push_back(__target);
                            }
                        }
                        __mesh.indices.push_back(_remap[__v]);
                    }
                    // Reset the remap for the next material
                    for (uint32_t i = _first_index; i < __end; ++i)
                        if (pmx.indices[i] < pmx.vertices.size()) _remap[pmx.indices[i]] = SCENE_NONE;
                    if (!__has_sdef) __mesh.sdef.clear();
                    if (__mesh.indices.empty()) return SCENE_NONE;
                    __mesh.computeBounds();
                    return scene.addMesh(__mesh);
                }

                uint32_t convert(uint32_t _parent) {
                    const float __s = options.scale;
                    const uint32_t __root = scene.addNode(pmx.name, _parent, scaling({__s, __s, __s}));
                    const uint32_t __skeleton = skeleton();
                    std::vector<uint32_t> __remap(pmx.vertices.size(), SCENE_NONE);
                    uint32_t __first = 0u;
                    for (size_t m = 0u; m < pmx.materials.size(); ++m) {
                        const uint32_t __material = material(pmx.materials[m]);
                        const uint32_t __mesh = mesh(__first, pmx.materials[m], __material, __remap);
                        if (__mesh != SCENE_NONE) scene.nodes[__root].meshes.push_back(__mesh);
                        __first += pmx.materials[m].index_count;
                    }
                    if (!scene.skeletons[__skeleton].bones.empty())
                        scene.nodes[__root].skin = scene.addSkin(__skeleton, __root, pmx.name);
                    return __root;
                }
            }; // PMXConverter END
        } // anonymous namespace END

        uint32_t convertPMX(Scene &_scene, const File::ContentPMX &_pmx, const std::string &_dir, const ImportOptions &_options, uint32_t _parent) {
            PMXConverter __conv(_scene, _pmx, _dir, _options);
            return __conv.convert(_parent);
        }

        uint32_t loadPMX(Scene &_scene, const std::string &_path, const ImportOptions &_options, uint32_t _parent) {
            File::ReaderWriterPMX __reader;
            SVKFW_ASSERT(__reader.read(_path), std::runtime_error, "SceneGraph :: loadPMX", "can't load '" + _path + "'");
            return convertPMX(_scene, __reader.file_content, directoryOf(_path), _options, _parent);
        }

        uint32_t loadModel(Scene &_scene, const std::string &_path, const ImportOptions &_options, uint32_t _parent) {
            std::string __ext = _path.size() > 4u ? _path.substr(_path.size() - 4u) : std::string();
            for (size_t i = 0u; i < __ext.size(); ++i) __ext[i] = (char)std::tolower((unsigned char)__ext[i]);
            return __ext == ".pmx" ? loadPMX(_scene, _path, _options, _parent) : loadAssimp(_scene, _path, _options, _parent);
        }

    }; // SceneGraph END
}; // Simple END
