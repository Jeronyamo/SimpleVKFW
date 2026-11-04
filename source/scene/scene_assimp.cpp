#include "scene/scene_assimp.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <stdexcept>

#include <assimp/Importer.hpp>
#include <assimp/DefaultIOSystem.h>
#include <assimp/IOStream.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>

#include "STBI/stb_image.h"


namespace Simple {
    namespace SceneGraph {
        namespace {

            // Note: aiMatrix4x4 is column-vector style (translation in a4, b4, c4), 'mat4' is row-vector style: transposed
            mat4 toMat4(const aiMatrix4x4 &_m) {
                return { _m.a1, _m.b1, _m.c1, _m.d1,
                         _m.a2, _m.b2, _m.c2, _m.d2,
                         _m.a3, _m.b3, _m.c3, _m.d3,
                         _m.a4, _m.b4, _m.c4, _m.d4 };
            }

            std::string lower(std::string _s) {
                for (size_t i = 0u; i < _s.size(); ++i) _s[i] = (char)std::tolower((unsigned char)_s[i]);
                return _s;
            }

            std::string extension(const std::string &_path) {
                size_t __dot = _path.find_last_of('.');
                size_t __sep = _path.find_last_of("/\\");
                if (__dot == std::string::npos || (__sep != std::string::npos && __dot < __sep)) return std::string();
                return lower(_path.substr(__dot + 1u));
            }

            std::string directory(const std::string &_path) {
                size_t __sep = _path.find_last_of("/\\");
                return __sep == std::string::npos ? std::string() : _path.substr(0u, __sep + 1u);
            }

            // Converts the separators of a path written on Windows ('tx\body.png') and joins it with the model folder
            std::string resolvePath(const std::string &_dir, std::string _file) {
                std::replace(_file.begin(), _file.end(), '\\', '/');
                bool __absolute = !_file.empty() && (_file[0] == '/' || (_file.size() > 1u && _file[1] == ':'));
                return __absolute ? _file : _dir + _file;
            }

            struct Converter {
                Scene                &scene;
                const aiScene        &ai;
                std::string           dir;
                bool                  left_handed;
                const ImportOptions  &options;
                bool                  mmd;
                std::string           ext;      // of the model file, lowercase (empty if unknown)

                std::map<std::string, uint32_t> texture_cache; // source + color space -> texture index
                std::vector<uint32_t> mesh_map;                // aiMesh index -> mesh index (SCENE_NONE: skipped)
                std::vector<uint32_t> material_map;            // aiMaterial index -> material index
                Assimp::DefaultIOSystem io;                    // reads UTF-8 paths on Windows too

                Converter(Scene &_scene, const aiScene &_ai, const std::string &_dir, bool _left_handed, const ImportOptions &_options,
                          const std::string &_ext)
                    : scene(_scene), ai(_ai), dir(_dir), left_handed(_left_handed), options(_options), mmd(_ext == "pmx" || _ext == "pmd"), ext(_ext) {}

                // Mirrors Z for right-handed data (the conjugation by diag(1, 1, -1) for matrices)
                vec3 fixVec(const aiVector3D &_v) const { return { _v.x, _v.y, left_handed ? _v.z : -_v.z }; }
                mat4 fixMat(const aiMatrix4x4 &_m) const {
                    mat4 __m = toMat4(_m);
                    if (!left_handed) {
                        __m.M[0][2] = -__m.M[0][2]; __m.M[1][2] = -__m.M[1][2]; __m.M[3][2] = -__m.M[3][2];
                        __m.M[2][0] = -__m.M[2][0]; __m.M[2][1] = -__m.M[2][1]; __m.M[2][3] = -__m.M[2][3];
                    }
                    return __m;
                }

                bool decode(Texture &_tex, const unsigned char *_data, int _size) {
                    int __w = 0, __h = 0, __ch = 0;
                    unsigned char *__px = stbi_load_from_memory(_data, _size, &__w, &__h, &__ch, 4);
                    if (!__px) return false;
                    _tex.image.width  = __w;
                    _tex.image.height = __h;
                    _tex.image.img.resize((size_t)__w * __h);
                    std::memcpy(_tex.image.img.data(), __px, (size_t)__w * __h * 4u);
                    stbi_image_free(__px);
                    _tex.scanAlpha();
                    return true;
                }

                bool loadEmbedded(Texture &_tex, const aiTexture &_ai_tex) {
                    if (_ai_tex.mHeight == 0u) // compressed (png, jpg...) in memory
                        return decode(_tex, reinterpret_cast<const unsigned char*>(_ai_tex.pcData), (int)_ai_tex.mWidth);
                    _tex.image.width  = (int)_ai_tex.mWidth;
                    _tex.image.height = (int)_ai_tex.mHeight;
                    _tex.image.img.resize((size_t)_ai_tex.mWidth * _ai_tex.mHeight);
                    for (size_t i = 0u; i < _tex.image.img.size(); ++i) { // aiTexel is BGRA
                        const aiTexel &__t = _ai_tex.pcData[i];
                        _tex.image.img[i].x = __t.r; _tex.image.img[i].y = __t.g; _tex.image.img[i].z = __t.b; _tex.image.img[i].w = __t.a;
                    }
                    _tex.scanAlpha();
                    return true;
                }

                bool loadFile(Texture &_tex, const std::string &_path) {
                    std::unique_ptr<Assimp::IOStream> __file(io.Exists(_path.c_str()) ? io.Open(_path.c_str(), "rb") : nullptr);
                    if (!__file) return false;
                    std::vector<unsigned char> __bytes(__file->FileSize());
                    if (__bytes.empty() || __file->Read(__bytes.data(), 1u, __bytes.size()) != __bytes.size()) return false;
                    return decode(_tex, __bytes.data(), (int)__bytes.size());
                }

                uint32_t texture(const aiMaterial &_mat, aiTextureType _type, bool _srgb) {
                    if (_mat.GetTextureCount(_type) == 0u) return SCENE_NONE;
                    aiString __ai_path;
                    if (_mat.GetTexture(_type, 0u, &__ai_path) != AI_SUCCESS || __ai_path.length == 0u) return SCENE_NONE;
                    std::string __src = __ai_path.C_Str();

                    const aiTexture *__embedded = ai.GetEmbeddedTexture(__src.c_str());
                    std::string __name = __embedded ? __src : resolvePath(dir, __src);
                    std::string __key  = __name + (_srgb ? "|srgb" : "|linear");
                    std::map<std::string, uint32_t>::const_iterator __it = texture_cache.find(__key);
                    if (__it != texture_cache.end()) return __it->second;

                    Texture __tex;
                    __tex.name = __name;
                    __tex.srgb = _srgb;
                    bool __ok = __embedded ? loadEmbedded(__tex, *__embedded) : loadFile(__tex, __name);
                    uint32_t __index = SCENE_NONE;
                    if (__ok)
                        __index = scene.addTexture(__tex);
                    else
                        fprintf(stderr, "SceneGraph::loadAssimp: can't load texture '%s'\n", __name.c_str());
                    texture_cache[__key] = __index;
                    return __index;
                }

                uint32_t material(const aiMaterial &_ai_mat) {
                    Material __mat;
                    aiString __name;
                    if (_ai_mat.Get(AI_MATKEY_NAME, __name) == AI_SUCCESS) __mat.name = __name.C_Str();

                    aiColor4D __color(1.f, 1.f, 1.f, 1.f);
                    if (_ai_mat.Get(AI_MATKEY_BASE_COLOR, __color) != AI_SUCCESS) {
                        aiColor3D __diffuse(1.f, 1.f, 1.f);
                        _ai_mat.Get(AI_MATKEY_COLOR_DIFFUSE, __diffuse);
                        __color = aiColor4D(__diffuse.r, __diffuse.g, __diffuse.b, 1.f);
                    }
                    float __opacity = 1.f;
                    if (_ai_mat.Get(AI_MATKEY_OPACITY, __opacity) == AI_SUCCESS) __color.a *= __opacity;
                    __mat.base_color = { __color.r, __color.g, __color.b, __color.a };

                    aiColor3D __specular(0.f, 0.f, 0.f), __emissive(0.f, 0.f, 0.f);
                    _ai_mat.Get(AI_MATKEY_COLOR_SPECULAR, __specular);
                    _ai_mat.Get(AI_MATKEY_COLOR_EMISSIVE, __emissive);
                    __mat.specular = { __specular.r, __specular.g, __specular.b };
                    __mat.emissive = { __emissive.r, __emissive.g, __emissive.b };
                    float __shininess = 0.f;
                    // Note: the MMD importer stores the PMX specular power as the shininess strength
                    if (_ai_mat.Get(AI_MATKEY_SHININESS, __shininess) != AI_SUCCESS && mmd)
                        _ai_mat.Get(AI_MATKEY_SHININESS_STRENGTH, __shininess);
                    if (__shininess > 0.f) __mat.shininess = __shininess;

                    __mat.base_color_texture = texture(_ai_mat, aiTextureType_BASE_COLOR, true);
                    if (__mat.base_color_texture == SCENE_NONE) __mat.base_color_texture = texture(_ai_mat, aiTextureType_DIFFUSE, true);
                    __mat.normal_texture = texture(_ai_mat, aiTextureType_NORMALS, false);
                    // Note: OBJ files give normal maps as 'bump' (assimp: height); real height maps can't be told apart, so only for OBJ
                    if (__mat.normal_texture == SCENE_NONE && ext == "obj")
                        __mat.normal_texture = texture(_ai_mat, aiTextureType_HEIGHT, false);
                    __mat.emissive_texture = texture(_ai_mat, aiTextureType_EMISSIVE, true);
                    if (__mat.emissive_texture != SCENE_NONE && __mat.emissive.x + __mat.emissive.y + __mat.emissive.z == 0.f)
                        __mat.emissive = {1.f, 1.f, 1.f};
                    _ai_mat.Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), __mat.normal_scale);

                    int __two_sided = 0;
                    if (_ai_mat.Get(AI_MATKEY_TWOSIDED, __two_sided) == AI_SUCCESS) __mat.two_sided = __two_sided != 0;
                    // Note: assimp drops the PMX material flags (culling, edges, toon), and MMD models rely on drawing both sides
                    if (mmd) __mat.two_sided = true;

                    // Alpha mode: explicit in glTF, otherwise guessed from the opacity and the base color texture
                    aiString __alpha_mode;
                    if (_ai_mat.Get(AI_MATKEY_GLTF_ALPHAMODE, __alpha_mode) == AI_SUCCESS) {
                        std::string __mode = __alpha_mode.C_Str();
                        __mat.alpha_mode = __mode == "BLEND" ? SCENE_ALPHA_BLEND : __mode == "MASK" ? SCENE_ALPHA_MASK : SCENE_ALPHA_OPAQUE;
                        _ai_mat.Get(AI_MATKEY_GLTF_ALPHACUTOFF, __mat.alpha_cutoff);
                    } else if (options.detect_alpha) {
                        const Texture *__tex = __mat.base_color_texture == SCENE_NONE ? nullptr : &scene.textures[__mat.base_color_texture];
                        if (__mat.base_color.w < 1.f || (__tex && __tex->soft_alpha)) __mat.alpha_mode = SCENE_ALPHA_BLEND;
                        else if (__tex && __tex->has_alpha)                            __mat.alpha_mode = SCENE_ALPHA_MASK;
                    }
                    if (mmd) __mat.blend_depth_write = true;
                    if (options.hide_zero_opacity && __mat.base_color.w <= 0.f)
                        __mat.visible = false;
                    return scene.addMaterial(__mat);
                }

                uint32_t mesh(const aiMesh &_ai_mesh) {
                    if (!(_ai_mesh.mPrimitiveTypes & aiPrimitiveType_TRIANGLE) || _ai_mesh.mNumVertices == 0u) return SCENE_NONE;
                    Mesh __mesh;
                    __mesh.name = _ai_mesh.mName.C_Str();
                    __mesh.material = _ai_mesh.mMaterialIndex < material_map.size() ? material_map[_ai_mesh.mMaterialIndex] : SCENE_NONE;
                    __mesh.vertices.resize(_ai_mesh.mNumVertices);
                    const bool __has_uv = _ai_mesh.HasTextureCoords(0u), __has_tangents = _ai_mesh.HasTangentsAndBitangents() && __has_uv;
                    for (uint32_t i = 0u; i < _ai_mesh.mNumVertices; ++i) {
                        Vertex &__v = __mesh.vertices[i];
                        __v.position = fixVec(_ai_mesh.mVertices[i]);
                        __v.normal   = _ai_mesh.HasNormals() ? fixVec(_ai_mesh.mNormals[i]) : vec3{0.f, 1.f, 0.f};
                        __v.uv       = __has_uv ? vec2{ _ai_mesh.mTextureCoords[0][i].x, _ai_mesh.mTextureCoords[0][i].y } : vec2{0.f, 0.f};
                        __v.tangent  = {0.f, 0.f, 0.f, 0.f};
                        if (__has_tangents) {
                            vec3 __t = fixVec(_ai_mesh.mTangents[i]), __b = fixVec(_ai_mesh.mBitangents[i]);
                            // Handedness: does cross(normal, tangent) point along the bitangent?
                            float __w = Math::dot(Math::cross(__v.normal, __t), __b) < 0.f ? -1.f : 1.f;
                            __v.tangent = { __t.x, __t.y, __t.z, __w };
                        }
                    }
                    __mesh.indices.reserve((size_t)_ai_mesh.mNumFaces * 3u);
                    for (uint32_t f = 0u; f < _ai_mesh.mNumFaces; ++f) {
                        const aiFace &__face = _ai_mesh.mFaces[f];
                        if (__face.mNumIndices != 3u) continue; // points and lines (the mesh is split by type, see 'loadAssimp')
                        __mesh.indices.push_back(__face.mIndices[0]);
                        __mesh.indices.push_back(__face.mIndices[1]);
                        __mesh.indices.push_back(__face.mIndices[2]);
                    }
                    if (__mesh.indices.empty()) return SCENE_NONE;
                    __mesh.computeBounds();
                    return scene.addMesh(__mesh);
                }

                uint32_t node(const aiNode &_ai_node, uint32_t _parent, const mat4 &_extra) {
                    uint32_t __index = scene.addNode(_ai_node.mName.C_Str(), _parent, fixMat(_ai_node.mTransformation) & _extra);
                    for (uint32_t m = 0u; m < _ai_node.mNumMeshes; ++m) {
                        uint32_t __mesh = _ai_node.mMeshes[m] < mesh_map.size() ? mesh_map[_ai_node.mMeshes[m]] : SCENE_NONE;
                        if (__mesh != SCENE_NONE) scene.nodes[__index].meshes.push_back(__mesh);
                    }
                    for (uint32_t c = 0u; c < _ai_node.mNumChildren; ++c)
                        node(*_ai_node.mChildren[c], __index, identity());
                    return __index;
                }

                void light(const aiLight &_ai_light, uint32_t _first_node) {
                    aiColor3D __c = _ai_light.mColorDiffuse;
                    if (_ai_light.mType == aiLightSource_AMBIENT) {
                        scene.ambient = scene.ambient + vec3{ __c.r, __c.g, __c.b };
                        return;
                    }
                    Light __light;
                    __light.name = _ai_light.mName.C_Str();
                    switch (_ai_light.mType) {
                        case aiLightSource_DIRECTIONAL: __light.type = SCENE_LIGHT_DIRECTIONAL; break;
                        case aiLightSource_SPOT:        __light.type = SCENE_LIGHT_SPOT;        break;
                        case aiLightSource_POINT:       __light.type = SCENE_LIGHT_POINT;       break;
                        default: return; // area lights later
                    }
                    // Note: assimp keeps the intensity in the color
                    float __max = std::max(__c.r, std::max(__c.g, __c.b));
                    __light.intensity = __max > 0.f ? __max : 1.f;
                    __light.color = __max > 0.f ? vec3{ __c.r / __max, __c.g / __max, __c.b / __max } : vec3{1.f, 1.f, 1.f};
                    __light.inner_cone = _ai_light.mAngleInnerCone;
                    __light.outer_cone = std::max(_ai_light.mAngleOuterCone, _ai_light.mAngleInnerCone);
                    // Range: where the attenuation 1 / (c + l d + q d^2) drops to 1/256 (only if the file has a falloff)
                    float __q = _ai_light.mAttenuationQuadratic, __l = _ai_light.mAttenuationLinear, __k = _ai_light.mAttenuationConstant - 256.f;
                    if (__q > 0.f)      __light.range = (-__l + std::sqrt(std::max(0.f, __l * __l - 4.f * __q * __k))) / (2.f * __q);
                    else if (__l > 0.f) __light.range = -__k / __l;

                    // The light node: a child of the node with the light's name, at the light's position, looking along its direction
                    uint32_t __parent = SCENE_NONE;
                    for (uint32_t n = _first_node; n < scene.nodes.size(); ++n)
                        if (scene.nodes[n].name == __light.name) { __parent = n; break; }
                    if (__parent == SCENE_NONE) __parent = _first_node;
                    vec3 __dir = fixVec(_ai_light.mDirection), __up = fixVec(_ai_light.mUp);
                    if (__dir.norm() < 1e-6f) __dir = {0.f, 0.f, 1.f};
                    if (__up.norm()  < 1e-6f) __up  = {0.f, 1.f, 0.f};
                    mat4 __local = lookAlong(__dir, __up) & translation(fixVec(_ai_light.mPosition));
                    scene.addLight(__light, scene.addNode(__light.name + "_light", __parent, __local));
                }

            }; // Converter END

        } // anonymous namespace END


        namespace {
            uint32_t convert(Converter &_conv, uint32_t _parent) {
                const aiScene &__ai = _conv.ai;
                for (uint32_t i = 0u; i < __ai.mNumMaterials; ++i) _conv.material_map.push_back(_conv.material(*__ai.mMaterials[i]));
                for (uint32_t i = 0u; i < __ai.mNumMeshes;    ++i) _conv.mesh_map.push_back(_conv.mesh(*__ai.mMeshes[i]));
                // Note: the scale goes into the root transform, the vertices stay as in the file
                const float __s = _conv.options.scale;
                uint32_t __root = _conv.node(*__ai.mRootNode, _parent, scaling({__s, __s, __s}));
                for (uint32_t i = 0u; i < __ai.mNumLights; ++i) _conv.light(*__ai.mLights[i], __root);
                return __root;
            }
        } // anonymous namespace END

        uint32_t convertAssimp(Scene &_scene, const aiScene &_ai_scene, const std::string &_dir, bool _left_handed,
                               const ImportOptions &_options, uint32_t _parent) {
            Converter __conv(_scene, _ai_scene, _dir, _left_handed, _options, std::string());
            return convert(__conv, _parent);
        }

        uint32_t loadAssimp(Scene &_scene, const std::string &_path, const ImportOptions &_options, uint32_t _parent) {
            const std::string __ext = extension(_path);
            const bool __mmd = __ext == "pmx" || __ext == "pmd";
            const bool __left_handed = _options.handedness == ImportOptions::HANDEDNESS_AUTO ? __mmd
                                     : _options.handedness == ImportOptions::HANDEDNESS_LEFT;

            Assimp::Importer __importer;
            // Note: only triangles are kept (points and lines are split off by SortByPType and skipped);
            //       FlipUVs moves the UV origin to the top-left corner (the MMD importer flips PMX UVs to the bottom-left itself)
            unsigned int __flags = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
                                   aiProcess_ValidateDataStructure;
            if (_options.gen_smooth_normals) {
                __flags |= aiProcess_GenSmoothNormals;
                __importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, _options.smoothing_angle);
            }
            if (_options.optimize) __flags |= aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality;
            // Note: bones are not used yet; without this the MMD importer still gives the meshes in the bind pose
            __importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
            __flags |= aiProcess_FindDegenerates;

            const aiScene *__ai = __importer.ReadFile(_path, __flags);
            if (!__ai || !__ai->mRootNode || (__ai->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
                throw std::runtime_error("SceneGraph::loadAssimp: can't load '" + _path + "': " + __importer.GetErrorString());

            Converter __conv(_scene, *__ai, directory(_path), __left_handed, _options, __ext);
            return convert(__conv, _parent);
        }

    }; // SceneGraph END
}; // Simple END
