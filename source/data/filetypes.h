#ifndef SVKFW_FILETYPES_H
#define SVKFW_FILETYPES_H

#include <map>
#include <vector>

#include "terminal.h"
#include "linalg.h"

#include "platform_dependent/pd_filetypes.h"


namespace Simple {
// Utilities

    enum Filetype : uint32_t {
        NONE,
        BIN,
        VMD,
        OGG,

        UNKNOWN = UINT32_MAX
    }; // Filetype END


// Interfaces

    struct FiletypeContentItf { virtual ~FiletypeContentItf() {} };

    // Tries to read all
    struct FiletypeReaderWriterItf {
        virtual ~FiletypeReaderWriterItf() {}

        virtual bool  read(const std::string &_fpath) = 0;
        virtual void write(const std::string &_fpath) = 0;
        // Used when file reader has to recognize a file without extension
        virtual bool tryHeaders(FileBinReader &_ftype_file) = 0;
    }; // FiletypeReaderWriter END


// Implementations

    struct ContentVMD : FiletypeContentItf {
        struct BoneKeyframe {
            vec3f position;
            uint32_t frame_id;
            Affine::Rotation::Quaternion rotation;
            char   bone_name[16]; // Actually 15 + '\0'
            uint8_t int_info[64]; // interpolation info
            // Interpolation info (in bytes) in range [0,127]:
            //  0-15: MovX.0x,MovY.0x,      0,     0, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
            // 16-30:         MovY.0x,MovZ.0x,Rot.0x, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
            // 31-45:               0,MovZ.0x,Rot.0x, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
            // 46-60:               0,      0,Rot.0x, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
            // 61-63:               0,      0,     0
        };

        struct MorphKeyframe {
            uint32_t frame_id;
            float weight;
            char morph_name[16]; // Actually 15 + '\0'
        };

        struct CameraKeyframe {
            vec3f position;
            uint32_t frame_id;
            vec3f rotation;
            float distance;
            uint32_t fov;
            uint32_t perspect; // 1 byte, uint32_t for padding
            uint8_t int_info[24]{};
            // Interpolation info (in bytes) in range [0,127]:
            //  0- 3:  moveX.0x, moveX.1x, moveX.0y, moveX.1y
            //  4- 7:  moveY.0x, moveY.1x, moveY.0y, moveY.1y
            //  8-11:  moveZ.0x, moveZ.1x, moveZ.0y, moveZ.1y
            // 12-15:    Rot.0x,   Rot.1x,   Rot.0y,   Rot.1y
            // 16-19:   Dist.0x,  Dist.1x,  Dist.0y,  Dist.1y
            // 20-23:    FOV.0x,   FOV.1x,   FOV.0y,   FOV.1y
        };

        struct AttrAdjust {
            vec4f pos_dist;
            vec4f rot_fov; // xyz - yaw/pitch/roll, in degrees

            AttrAdjust(bool _is_offset_scale = true) {
                _is_offset_scale ? defOffset() : defScale();
            }
            AttrAdjust(const vec3f &_pos, const vec3f &_rot,
                       float _cam_dist = 0.f, float _cam_fov = 0.f) : pos_dist{_pos, _cam_dist}, rot_fov{_rot, _cam_fov} {}

            void defScale() {
                pos_dist = 1.f;
                rot_fov  = 1.f;
            }
            void defOffset() {
                pos_dist = 0.f;
                rot_fov  = 0.f;
            }
        };

        struct  BoneAttrClampMinmax {
            vec3f pos_min{ 1e6f};
            vec3f rot_min{ 1e6f}; // yaw/pitch/roll, in degrees
            vec3f pos_max{-1e6f};
            vec3f rot_max{-1e6f}; // yaw/pitch/roll, in degrees

            BoneAttrClampMinmax(const vec3f &_rot_min = {1e6f}, const vec3f &_rot_max = {-1e6f},
                                const vec3f &_pos_min = {1e6f}, const vec3f &_pos_max = {-1e6f}) : rot_min{_rot_min}, rot_max{_rot_max},
                                                                                                   pos_min{_pos_min}, pos_max{_pos_max} {}
        };
        struct MorphAttrClampMinmax {
            float val_min = 1e6f,
                  val_max =-1e6f;

            MorphAttrClampMinmax(float _val_min = 1e6f, float _val_max = -1e6f) : val_min{_val_min}, val_max{_val_max} {}
        };

        struct InterpolationInfo {
            vec4u8 mov_x{(uint8_t)20, (uint8_t)107, (uint8_t)20, (uint8_t)107};
            vec4u8 mov_y{(uint8_t)20, (uint8_t)107, (uint8_t)20, (uint8_t)107};
            vec4u8 mov_z{(uint8_t)20, (uint8_t)107, (uint8_t)20, (uint8_t)107};
            vec4u8   rot{(uint8_t)20, (uint8_t)107, (uint8_t)20, (uint8_t)107};
            vec4u8  dist{(uint8_t)20, (uint8_t)107, (uint8_t)20, (uint8_t)107};
            vec4u8   fov{(uint8_t)20, (uint8_t)107, (uint8_t)20, (uint8_t)107};
        };

    // Data

        char header[30];
        char model[20];
        bool is_old_format;

        static const std::map<std::string, std::string> bone_name_mappings;
        static const std::map<std::string, std::string> morph_name_mappings;
        static const std::map<std::string, std::string> bone_name_mappings_fromraw;
        static const std::map<std::string, std::string> morph_name_mappings_fromraw;

        std::vector<BoneKeyframe> keyframes_bone;
        std::vector<MorphKeyframe> keyframes_morph;
        std::vector<CameraKeyframe> keyframes_camera;

    // Methods

        static bool       BoneMapped(const std::string & _bone_name) { return  bone_name_mappings_fromraw.find( _bone_name) !=  bone_name_mappings_fromraw.end(); }
        static bool      MorphMapped(const std::string &_morph_name) { return morph_name_mappings_fromraw.find(_morph_name) != morph_name_mappings_fromraw.end(); }
        static bool  EqToBoneMapping(const std::string & _bone_name, const std::string &_comp_name) { return  BoneMapped( _bone_name) &&
                                                                                                              bone_name_mappings_fromraw.at( _bone_name) == _comp_name; }
        static bool EqToMorphMapping(const std::string &_morph_name, const std::string &_comp_name) { return MorphMapped(_morph_name) &&
                                                                                                             morph_name_mappings_fromraw.at(_morph_name) == _comp_name; }
        static bool IsLeftBone  (const std::string & _bone_name) { return  _bone_name.substr(0, 2) == "\215\266"; }
        static bool IsRightBone (const std::string & _bone_name) { return  _bone_name.substr(0, 2) == "\211\105"; }
        static bool IsLeftMorph (const std::string &_morph_name) { return _morph_name.substr(0, 2) == "\215\266"; }
        static bool IsRightMorph(const std::string &_morph_name) { return _morph_name.substr(0, 2) == "\211\105"; }

        static std::string GetOppositeBone(const std::string &  _bone_name) {
            if ( IsLeftBone(_bone_name))
                return "\211\105" + _bone_name.substr(2);
            if (IsRightBone(_bone_name))
                return "\215\266" + _bone_name.substr(2);
            return _bone_name;
        }
        static std::string GetOppositeMorph(const std::string & _morph_name) {
            if ( IsLeftMorph(_morph_name))
                return "\211\105" + _morph_name.substr(2);
            if (IsRightMorph(_morph_name))
                return "\215\266" + _morph_name.substr(2);
            return _morph_name;
        }
        static bool GetOppositeNameInplace(char *_bone_morph_name) {
            bool __replaced = true;

            if (_bone_morph_name[0] == '\211' && _bone_morph_name[1] == '\105') {
                _bone_morph_name[0]  = '\215';
                _bone_morph_name[1]  = '\266';
            }
            else if (_bone_morph_name[0] == '\215' && _bone_morph_name[1] == '\266') {
                _bone_morph_name[0]  = '\211';
                _bone_morph_name[1]  = '\105';
            }
            else __replaced = false;

            return __replaced;
        }
        // Reflect through a plane X=0
        static void ReflectBoneKeyframe(BoneKeyframe &_bone_kf) {
            GetOppositeNameInplace(_bone_kf.bone_name);
            _bone_kf.position.x *= -1.f;
            vec3f __euler = Affine::Rotation::quaternion2ypr(_bone_kf.rotation);
            __euler["xz"] *= -1.f;
            _bone_kf.rotation = Affine::Rotation::ypr2quaternion(__euler);
        }
        static void ReflectMorphKeyframe(MorphKeyframe &_morph_kf) {
            GetOppositeNameInplace(_morph_kf.morph_name);
        }
        // Reflect through a plane X=0
        static void ReflectCameraKeyframe(CameraKeyframe &_camera_kf) {
            _camera_kf.position.x *= -1.f;
            _camera_kf.rotation["yz"] *= -1.f;
        }

        void sortBoneKeyframes() {
            std::map<std::string, std::vector<BoneKeyframe>> __bone_keyframes;

            for (const BoneKeyframe& kframe : keyframes_bone) {
                if (__bone_keyframes.find(kframe.bone_name) == __bone_keyframes.end())
                    __bone_keyframes[kframe.bone_name] = {};

                std::vector<BoneKeyframe> &__curr_kframes = __bone_keyframes[kframe.bone_name];
                auto __curr_iterator = __curr_kframes.end();
                for (__curr_iterator; __curr_iterator != __curr_kframes.begin(); --__curr_iterator) {
                    if ((__curr_iterator-1)->frame_id < kframe.frame_id)
                        break;
                }
                __curr_kframes.insert(__curr_iterator, kframe);
            }
            keyframes_bone.clear();
            for (const auto &bone_kframes : __bone_keyframes)
                keyframes_bone.insert(keyframes_bone.end(), bone_kframes.second.begin(), bone_kframes.second.end());
        }

        void sortMorphKeyframes() {
            std::map<std::string, std::vector<MorphKeyframe>> __morph_keyframes;

            for (const MorphKeyframe& kframe : keyframes_morph) {
                if (__morph_keyframes.find(kframe.morph_name) == __morph_keyframes.end())
                    __morph_keyframes[kframe.morph_name] = {};

                std::vector<MorphKeyframe> &__curr_kframes = __morph_keyframes[kframe.morph_name];
                auto __curr_iterator = __curr_kframes.end();
                for (__curr_iterator; __curr_iterator != __curr_kframes.begin(); --__curr_iterator) {
                    if ((__curr_iterator-1)->frame_id < kframe.frame_id)
                        break;
                }
                __curr_kframes.insert(__curr_iterator, kframe);
            }
            keyframes_morph.clear();
            for (const auto &morph_kframes : __morph_keyframes)
                keyframes_morph.insert(keyframes_morph.end(), morph_kframes.second.begin(), morph_kframes.second.end());
        }

        // Reflect through a plane X=0
        void reflectMovement() {
            for (uint32_t i = 0u; i < keyframes_bone.size(); ++i)
                ReflectBoneKeyframe (keyframes_bone[i]);
            for (uint32_t i = 0u; i < keyframes_morph.size(); ++i)
                ReflectMorphKeyframe(keyframes_morph[i]);
            for (uint32_t i = 0u; i < keyframes_camera.size(); ++i)
                ReflectCameraKeyframe(keyframes_camera[i]);
        }

        void multKeyframes(const std::map<std::string, AttrAdjust> &_mult_bones,
                           const std::map<std::string,      float> &_mult_morphs,
                           AttrAdjust _cam_mult = {}) {
            for (auto& bone_kframe : keyframes_bone) {
                if (bone_name_mappings_fromraw.find(bone_kframe.bone_name) != bone_name_mappings_fromraw.end() &&
                    _mult_bones.find(bone_name_mappings_fromraw.at(bone_kframe.bone_name)) != _mult_bones.end()) {
                    AttrAdjust __mult_info = _mult_bones.at(bone_name_mappings_fromraw.at(bone_kframe.bone_name));

                    bone_kframe.position *= __mult_info.pos_dist["xyz"];
                    vec3f __euler = Affine::Rotation::quaternion2ypr(bone_kframe.rotation);
                    __euler *= __mult_info.rot_fov["xyz"];
                    bone_kframe.rotation = Affine::Rotation::ypr2quaternion(__euler);
                }
            }

            for (auto& morph_kframe : keyframes_morph) {
                if (morph_name_mappings_fromraw.find(morph_kframe.morph_name) != morph_name_mappings_fromraw.end() &&
                    _mult_morphs.find(morph_name_mappings_fromraw.at(morph_kframe.morph_name)) != _mult_morphs.end()) {
                    float __mult_info = _mult_morphs.at(morph_name_mappings_fromraw.at(morph_kframe.morph_name));

                    morph_kframe.weight *= __mult_info;
                    morph_kframe.weight = Math::clampCL(morph_kframe.weight, 0.f, 1.f);
                }
            }

            for (auto& cam_kframe : keyframes_camera) {
                cam_kframe.position *= _cam_mult.pos_dist["xyz"];
                cam_kframe.distance *= _cam_mult.pos_dist.w;
                cam_kframe.rotation *= _cam_mult.rot_fov["xyz"];
                cam_kframe.fov *= _cam_mult.rot_fov.w;
            }
        }
        void offsetKeyframes(const std::map<std::string, AttrAdjust> &_offset_bones,
                             const std::map<std::string,      float> &_offset_morphs,
                             AttrAdjust _cam_offset = {}) {
            for (auto& bone_kframe : keyframes_bone) {
                if (bone_name_mappings_fromraw.find(bone_kframe.bone_name) != bone_name_mappings_fromraw.end() &&
                    _offset_bones.find(bone_name_mappings_fromraw.at(bone_kframe.bone_name)) != _offset_bones.end()) {
                    AttrAdjust __offset_info = _offset_bones.at(bone_name_mappings_fromraw.at(bone_kframe.bone_name));

                    bone_kframe.position += __offset_info.pos_dist["xyz"];
                    vec3f __euler = Affine::Rotation::quaternion2ypr(bone_kframe.rotation);
                    __euler += __offset_info.rot_fov["xyz"];
                    bone_kframe.rotation = Affine::Rotation::ypr2quaternion(__euler);
                }
            }

            for (auto& morph_kframe : keyframes_morph) {
                if (morph_name_mappings_fromraw.find(morph_kframe.morph_name) != morph_name_mappings_fromraw.end() &&
                    _offset_morphs.find(morph_name_mappings_fromraw.at(morph_kframe.morph_name)) != _offset_morphs.end()) {
                    float __offset_info = _offset_morphs.at(morph_name_mappings_fromraw.at(morph_kframe.morph_name));

                    morph_kframe.weight += __offset_info;
                    morph_kframe.weight = Math::clampCL(morph_kframe.weight, 0.f, 1.f);
                }
            }

            for (auto& cam_kframe : keyframes_camera) {
                cam_kframe.position += _cam_offset.pos_dist["xyz"];
                cam_kframe.distance += _cam_offset.pos_dist.w;
                cam_kframe.rotation += _cam_offset.rot_fov["xyz"];
                cam_kframe.fov += _cam_offset.rot_fov.w;
            }
        }

        void clampFrames(const std::vector<std::pair<std::string,  BoneAttrClampMinmax>> &_clamp_bones,
                         const std::vector<std::pair<std::string, MorphAttrClampMinmax>> &_clamp_morphs) {
            std::vector<uint32_t> __count_bone(_clamp_bones.size(), 0u), __count_morph(_clamp_morphs.size(), 0u);

            for (auto &bone_frame : keyframes_bone) {
                for (uint32_t i = 0u; i < _clamp_bones.size(); ++i) {
                    if (EqToBoneMapping(bone_frame.bone_name, _clamp_bones[i].first)) {
                        auto __angles = Affine::Rotation::quaternion2ypr(bone_frame.rotation);
                        __angles.yaw   = Math::clampCL(__angles.yaw,   Math::toRad(_clamp_bones[i].second.rot_min.x), Math::toRad(_clamp_bones[i].second.rot_max.x));
                        __angles.pitch = Math::clampCL(__angles.pitch, Math::toRad(_clamp_bones[i].second.rot_min.y), Math::toRad(_clamp_bones[i].second.rot_max.y));
                        __angles.roll  = Math::clampCL(__angles.roll,  Math::toRad(_clamp_bones[i].second.rot_min.z), Math::toRad(_clamp_bones[i].second.rot_max.z));
                        bone_frame.rotation = Affine::Rotation::ypr2quaternion(__angles.yaw, __angles.pitch, __angles.roll);

                        bone_frame.position.x = Math::clampCL(bone_frame.position.x, _clamp_bones[i].second.pos_min.x, _clamp_bones[i].second.pos_max.x);
                        bone_frame.position.y = Math::clampCL(bone_frame.position.y, _clamp_bones[i].second.pos_min.y, _clamp_bones[i].second.pos_max.y);
                        bone_frame.position.z = Math::clampCL(bone_frame.position.z, _clamp_bones[i].second.pos_min.z, _clamp_bones[i].second.pos_max.z);
                        __count_bone[i] += 1;
                    }
                }
            }
            for (auto &morph_frame : keyframes_morph) {
                for (uint32_t i = 0u; i < _clamp_morphs.size(); ++i) {
                    if (EqToMorphMapping(morph_frame.morph_name, _clamp_morphs[i].first)) {
                        morph_frame.weight = Math::clampCL(morph_frame.weight, _clamp_morphs[i].second.val_min, _clamp_morphs[i].second.val_max);
                        __count_morph[i] += 1;
                    }
                }
            }

            if (!_clamp_bones.empty())
                printf("Bone frames:\n");
            for (uint32_t i = 0u; i < _clamp_bones.size(); ++i) {
                printf("%s: %d\n", _clamp_bones[i].first.c_str(), __count_bone[i]);
            }

            if (!_clamp_morphs.empty())
                printf("Morph frames:\n");
            for (uint32_t i = 0u; i < _clamp_morphs.size(); ++i) {
                printf("%s: %d\n", _clamp_morphs[i].first.c_str(), __count_morph[i]);
            }
        }

        void mixCamera(const ContentVMD &_other_camera, const std::vector<uint32_t> &_switch_frame_ids) {
            std::vector<CameraKeyframe> __tmp_keyframes_camera;

            if (keyframes_camera.empty()) return;
            uint32_t __ind_from = 0, __ind_to = 0;

            for (uint32_t __ind_switch = 0; __ind_switch < _switch_frame_ids.size(); ++__ind_switch) {
                __ind_from = __ind_to;
                __ind_to = _switch_frame_ids[__ind_switch];

                bool __correct_kframe = false;
                for (const auto& cam_kframe : (__ind_switch & 1 ? _other_camera.keyframes_camera : keyframes_camera)) {
                    if (cam_kframe.frame_id == __ind_from-1)
                        __correct_kframe = true;
                    if (__ind_from <= cam_kframe.frame_id && cam_kframe.frame_id < __ind_to)
                        __tmp_keyframes_camera.push_back(cam_kframe);
                }
                if (!__correct_kframe && __ind_from)
                    fprintf(svkfwwarn, "%s%d\n", SVKFW_WRAPWARN("ContentVMD :: mixCamera", "Incorrect keyframe: "), __ind_from);
            }

            keyframes_camera = std::move(__tmp_keyframes_camera);
        }

        // Interpolation example - mov_x: x = 0.x, y = 0.y, z = 1.x, w = 1.y 
        static void SetInterpolation(BoneKeyframe &_bone_kframe, const InterpolationInfo &_int_info) {
            // Interpolation info (in bytes) in range [0,127]:
            //  0-15: MovX.0x,MovY.0x,      0,     0, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
            // 16-30:         MovY.0x,MovZ.0x,Rot.0x, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
            // 31-45:               0,MovZ.0x,Rot.0x, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
            // 46-60:               0,      0,Rot.0x, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
            // 61-63:               0,      0,     0
            for (uint8_t &byte : _bone_kframe.int_info) byte = 0;

            _bone_kframe.int_info[ 0] = _int_info.mov_x.x;
            _bone_kframe.int_info[ 1] = _int_info.mov_y.x;

            _bone_kframe.int_info[16] = _int_info.mov_y.x;
            _bone_kframe.int_info[17] = _int_info.mov_z.x;
            _bone_kframe.int_info[18] =   _int_info.rot.x;

            _bone_kframe.int_info[32] = _int_info.mov_z.x;
            _bone_kframe.int_info[33] =   _int_info.rot.x;

            _bone_kframe.int_info[48] =   _int_info.rot.x;

            for (uint8_t offset : {4, 19, 34, 49}) {
                _bone_kframe.int_info[offset   ] = _int_info.mov_x.y;
                _bone_kframe.int_info[offset+ 1] = _int_info.mov_y.y;
                _bone_kframe.int_info[offset+ 2] = _int_info.mov_z.y;
                _bone_kframe.int_info[offset+ 3] =   _int_info.rot.y;

                _bone_kframe.int_info[offset+ 4] = _int_info.mov_x.z;
                _bone_kframe.int_info[offset+ 5] = _int_info.mov_y.z;
                _bone_kframe.int_info[offset+ 6] = _int_info.mov_z.z;
                _bone_kframe.int_info[offset+ 7] =   _int_info.rot.z;

                _bone_kframe.int_info[offset+ 8] = _int_info.mov_x.w;
                _bone_kframe.int_info[offset+ 9] = _int_info.mov_y.w;
                _bone_kframe.int_info[offset+10] = _int_info.mov_z.w;
                _bone_kframe.int_info[offset+11] =   _int_info.rot.w;
            }
        }
        // mov_x: x = 0.x, y = 0.y, z = 1.x, w = 1.y 
        static void SetInterpolation(CameraKeyframe &_cam_kframe, const InterpolationInfo &_int_info) {
            // Interpolation info (in bytes) in range [0,127]:
            //  0- 3:  moveX.0x, moveX.1x, moveX.0y, moveX.1y
            //  4- 7:  moveY.0x, moveY.1x, moveY.0y, moveY.1y
            //  8-11:  moveZ.0x, moveZ.1x, moveZ.0y, moveZ.1y
            // 12-15:    Rot.0x,   Rot.1x,   Rot.0y,   Rot.1y
            // 16-19:   Dist.0x,  Dist.1x,  Dist.0y,  Dist.1y
            // 20-23:    FOV.0x,   FOV.1x,   FOV.0y,   FOV.1y
            vec4u8 *__int_info = (vec4u8*) _cam_kframe.int_info;
            __int_info[0] = _int_info.mov_x["xzyw"];
            __int_info[1] = _int_info.mov_y["xzyw"];
            __int_info[2] = _int_info.mov_z["xzyw"];
            __int_info[3] =   _int_info.rot["xzyw"];
            __int_info[4] =  _int_info.dist["xzyw"];
            __int_info[5] =   _int_info.fov["xzyw"];
        }

        static void SetNameFromMapping(BoneKeyframe &_bone_kframe, std::string _mapping_str);
        static void CreateReflected(const std::string &_vmd_path_no_ext, bool _fix_blink = true);
        static void CreateMixed(const std::string &_motion_vmd_path_no_ext, const std::string &_camera_vmd_path_no_ext,
                                const std::vector<uint32_t> &_split_frame_ids, const AttrAdjust &_motion1_offset,
                                const AttrAdjust &_camera1_scale, const AttrAdjust &_camera1_offset,
                                const AttrAdjust &_camera2_scale, const AttrAdjust &_camera2_offset, bool _fix_blink = true);
    }; // ContentVMD END


    struct ReaderWriterVMD : FiletypeReaderWriterItf {
        ContentVMD file_content{};

        ReaderWriterVMD(const std::string &_fpath = "") { if (!_fpath.empty()) read(_fpath); }

        bool read(const std::string &_fpath) override {
            // Open file
            FileBinReader __ftype_file{_fpath};

            // Header
            const uint32_t header_len = 30;
            __ftype_file.read(file_content.header, header_len);
            std::string __res_str = std::string(file_content.header, file_content.header + (header_len - 5)); // 5 chars are ignored

            if (__res_str == "Vocaloid Motion Data file") {
                file_content.is_old_format = true;
            }
            else if (__res_str != "Vocaloid Motion Data 0002") {
                fprintf(svkfwwarn, SVKFW_WRAPWARN("ReaderWriterVMD :: read", "not a VMD file (header does not match): '%s'"), __res_str.c_str());
                return false;
            }

            printf(SVKFW_WRAPINFO("VMD Reader : Header", "'%s'\n"), __res_str.c_str());
            printf(SVKFW_WRAPINFO("VMD Reader : Format", " %s\n" ), file_content.is_old_format ? "old" : "new");

            // Model
            const uint32_t __model_name_len = file_content.is_old_format ? 10 : 20;
            __ftype_file.read(file_content.model, __model_name_len);

            printf(SVKFW_WRAPINFO("VMD Reader : Model name", "'%s'\n"), file_content.model);

            // Bone/Motion keyframes
            const uint32_t __bone_name_len = 15;
            const uint32_t __bone_interp_info_len = 64;
            uint32_t __keyframes_count = __ftype_file.readBinary<uint32_t>();
            file_content.keyframes_bone.resize(__keyframes_count);
            printf("\n" SVKFW_WRAPINFO("VMD Reader : Bone keyframes total", "%d\n"), __keyframes_count);
            std::string curr_name = "None";

            for (uint32_t i = 0u; i < __keyframes_count; ++i) {
                __ftype_file.read(file_content.keyframes_bone[i].bone_name, __bone_name_len);
                __ftype_file.readBinary(file_content.keyframes_bone[i].frame_id);
                __ftype_file.readBinary(file_content.keyframes_bone[i].position);
                __ftype_file.readBinary(file_content.keyframes_bone[i].rotation);
                // Different order, swap elements
                file_content.keyframes_bone[i].rotation = { file_content.keyframes_bone[i].rotation.k,
                                                            file_content.keyframes_bone[i].rotation.s,
                                                            file_content.keyframes_bone[i].rotation.i,
                                                            file_content.keyframes_bone[i].rotation.j };
                __ftype_file.readBinaryArray(file_content.keyframes_bone[i].int_info, __bone_interp_info_len);

                if (ContentVMD::bone_name_mappings_fromraw.find(file_content.keyframes_bone[i].bone_name) !=
                    ContentVMD::bone_name_mappings_fromraw.end()) {
                    std::string tmp_name = ContentVMD::bone_name_mappings_fromraw.at(file_content.keyframes_bone[i].bone_name);

                    if (tmp_name != curr_name) {
                        curr_name = tmp_name;
                        // printf(SVKFW_WRAPINFO("VMD Reader : Bone", "%s\n"), curr_name.c_str());
                    }
                }
                else {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("ReaderWriterVMD :: read", "Unknown bone: "));
                    for (char ch : std::string(file_content.keyframes_bone[i].bone_name))
                        printf("\\%o", uint8_t(ch));
                    printf("\n");
                }
            }

            // Face/Morph keyframes
            const uint32_t __morph_name_len = 15;
            __ftype_file.readBinary(__keyframes_count);
            file_content.keyframes_morph.resize(__keyframes_count);
            printf("\n" SVKFW_WRAPINFO("VMD Reader : Morph keyframes total", "%d\n"), __keyframes_count);

            for (uint32_t i = 0u; i < __keyframes_count; ++i) {
                __ftype_file.read(file_content.keyframes_morph[i].morph_name, __morph_name_len);
                __ftype_file.readBinary(file_content.keyframes_morph[i].frame_id);
                __ftype_file.readBinary(file_content.keyframes_morph[i].weight  );

                if (ContentVMD::morph_name_mappings_fromraw.find(file_content.keyframes_morph[i].morph_name) !=
                    ContentVMD::morph_name_mappings_fromraw.end()) {
                    std::string tmp_name = ContentVMD::morph_name_mappings_fromraw.at(file_content.keyframes_morph[i].morph_name);

                    if (tmp_name != curr_name) {
                        curr_name = tmp_name;
                        // printf(SVKFW_WRAPINFO("VMD Reader : Morph", "%s\n"), curr_name.c_str());
                    }
                }
                else {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("ReaderWriterVMD :: read", "Unknown morph: "));
                    for (char ch : std::string(file_content.keyframes_morph[i].morph_name))
                        printf("\\%o", uint8_t(ch));
                    printf("\n");
                }
            }

            // Camera keyframes
            const uint32_t __camera_interp_info_len = 24;
            __ftype_file.readBinary(__keyframes_count);
            file_content.keyframes_camera.resize(__keyframes_count);
            printf("\n" SVKFW_WRAPINFO("VMD Reader : Camera keyframes total", "%d\n"), __keyframes_count);

            for (uint32_t i = 0u; i < __keyframes_count; ++i) {
                __ftype_file.readBinary(file_content.keyframes_camera[i].frame_id);
                __ftype_file.readBinary(file_content.keyframes_camera[i].distance);
                __ftype_file.readBinary(file_content.keyframes_camera[i].position);
                __ftype_file.readBinary(file_content.keyframes_camera[i].rotation);
                __ftype_file.readBinaryArray(file_content.keyframes_camera[i].int_info, __camera_interp_info_len);
                __ftype_file.readBinary(file_content.keyframes_camera[i].fov     );
                file_content.keyframes_camera[i].perspect = __ftype_file.readBinary<uint8_t>(); // reading 1 byte is super effective, duh
            }
            printf(SVKFW_WRAPINFO("VMD Reader : Read", "Read END\n"));
            return true;
        }

        void write(const std::string &_fpath) override {
            // Open file
            FileBinWriter __ftype_file{_fpath};

            // Header
            const uint32_t __header_len = 30;
            __ftype_file.write(file_content.header, __header_len);

            // Model
            const uint32_t __model_name_len = file_content.is_old_format ? 10 : 20;
            __ftype_file.write(file_content.model, __model_name_len);

            // Bone/Motion keyframes
            const uint32_t __bone_interp_info_len = 64;
            const uint32_t __bone_name_len = 15;
            uint32_t __keyframes_count = file_content.keyframes_bone.size();
            __ftype_file.writeBinary(__keyframes_count);

            for (uint32_t i = 0u; i < __keyframes_count; ++i) {
                __ftype_file.write(file_content.keyframes_bone[i].bone_name, __bone_name_len);
                __ftype_file.writeBinary(file_content.keyframes_bone[i].frame_id);
                __ftype_file.writeBinary(file_content.keyframes_bone[i].position);
                vec4f __quaternion_data_ordered = { file_content.keyframes_bone[i].rotation.i,
                                                    file_content.keyframes_bone[i].rotation.j,
                                                    file_content.keyframes_bone[i].rotation.k,
                                                    file_content.keyframes_bone[i].rotation.s };
                __ftype_file.writeBinary(__quaternion_data_ordered);
                __ftype_file.writeBinaryArray(file_content.keyframes_bone[i].int_info, __bone_interp_info_len);
            }

            // Face/Morph keyframes
            const uint32_t __morph_name_len = 15;
            __keyframes_count = file_content.keyframes_morph.size();
            __ftype_file.writeBinary(__keyframes_count);

            for (uint32_t i = 0u; i < __keyframes_count; ++i) {
                __ftype_file.write(file_content.keyframes_morph[i].morph_name, __morph_name_len);
                __ftype_file.writeBinary(file_content.keyframes_morph[i].frame_id);
                __ftype_file.writeBinary(file_content.keyframes_morph[i].weight);
            }

            // Camera keyframes
            const uint32_t __camera_interp_info_len = 24;
            __keyframes_count = file_content.keyframes_camera.size();
            __ftype_file.writeBinary(__keyframes_count);

            for (uint32_t i = 0u; i < __keyframes_count; ++i) {
                __ftype_file.writeBinary(file_content.keyframes_camera[i].frame_id);
                __ftype_file.writeBinary(file_content.keyframes_camera[i].distance);
                __ftype_file.writeBinary(file_content.keyframes_camera[i].position);
                __ftype_file.writeBinary(file_content.keyframes_camera[i].rotation);
                __ftype_file.writeBinaryArray(file_content.keyframes_camera[i].int_info, __camera_interp_info_len);
                __ftype_file.writeBinary(file_content.keyframes_camera[i].fov     );
                __ftype_file.writeBinary((uint8_t)file_content.keyframes_camera[i].perspect);
            }
        }

        bool tryHeaders(FileBinReader &_ftype_file) override {
            const uint32_t header_len = 30;
            char __ch_buffer[32]{};

            _ftype_file.read(__ch_buffer, header_len);
            std::string __res_str = std::string(__ch_buffer, __ch_buffer + header_len - 5);
            return __res_str == "Vocaloid Motion Data file" || __res_str == "Vocaloid Motion Data 0002";
        }
    }; // ReaderWriterVMD END

    // VMD: init mappings

    const std::map<std::string, std::string> ContentVMD::bone_name_mappings
    {
        // Center
        {"root", "\221\123\202\304\202\314\220\145",},
        {"center", "\203\132\203\223\203\136\201\133",},
        {"groove", "\203\117\203\213\201\133\203\165",},
        {"back_hips", "\215\230",},
        // Body
        {"hips", "\221\314\202\314\217\144\220\123",},
        {"spine", "\217\343\224\274\220\147",},
        {"spine2", "\217\343\224\274\220\147\62",},
        {"spine3", "\217\343\224\274\220\147\63",},
        {"neck", "\216\361",},
        {"head", "\223\252",},
        {"waist", "\211\272\224\274\220\147",},
        // Right Arm
        {"shoulder_p_r", "\211\105\214\250\120",},
        {"shoulder_r", "\211\105\214\250",},
        {"shoulder_c_r", "\211\105\214\250\103",},
        {"uparm_r", "\211\105\230\162",},
        {"uparm_twist_r", "\211\105\230\162\235\200",},
        {"lowarm_r", "\211\105\202\320\202\266",},
        {"lowarm_twist_r", "\211\105\216\350\235\200",},
        {"hand_r", "\211\105\216\350\216\361",},
        {"hand_dummy_r", "\211\105\203\137\203\176\201\133",},
        {"ik_hand_r", "\211\105\230\162\202\150\202\152",},
        // Left Arm
        {"shoulder_p_l", "\215\266\214\250\120",},
        {"shoulder_l", "\215\266\214\250",},
        {"shoulder_c_l", "\215\266\214\250\103",},
        {"uparm_l", "\215\266\230\162",},
        {"uparm_twist_l", "\215\266\230\162\235\200",},
        {"lowarm_l", "\215\266\202\320\202\266",},
        {"lowarm_twist_l", "\215\266\216\350\235\200",},
        {"hand_l", "\215\266\216\350\216\361",},
        {"hand_dummy_l", "\215\266\203\137\203\176\201\133",},
        {"ik_hand_l", "\215\266\230\162\202\150\202\152",},
        // Right Hand
        {"f_thumb01_r", "\211\105\220\145\216\167\202\117",},
        {"f_thumb02_r", "\211\105\220\145\216\167\202\120",},
        {"f_thumb03_r", "\211\105\220\145\216\167\202\121",},
        {"f_index01_r", "\211\105\220\154\216\167\202\120",},
        {"f_index02_r", "\211\105\220\154\216\167\202\121",},
        {"f_index03_r", "\211\105\220\154\216\167\202\122",},
        {"f_middle01_r", "\211\105\222\206\216\167\202\120",},
        {"f_middle02_r", "\211\105\222\206\216\167\202\121",},
        {"f_middle03_r", "\211\105\222\206\216\167\202\122",},
        {"f_ring01_r", "\211\105\226\362\216\167\202\120",},
        {"f_ring02_r", "\211\105\226\362\216\167\202\121",},
        {"f_ring03_r", "\211\105\226\362\216\167\202\122",},
        {"f_little01_r", "\211\105\217\254\216\167\202\120",},
        {"f_little02_r", "\211\105\217\254\216\167\202\121",},
        {"f_little03_r", "\211\105\217\254\216\167\202\122",},
        // Left Hand
        {"f_thumb01_l", "\215\266\220\145\216\167\202\117",},
        {"f_thumb02_l", "\215\266\220\145\216\167\202\120",},
        {"f_thumb03_l", "\215\266\220\145\216\167\202\121",},
        {"f_index01_l", "\215\266\220\154\216\167\202\120",},
        {"f_index02_l", "\215\266\220\154\216\167\202\121",},
        {"f_index03_l", "\215\266\220\154\216\167\202\122",},
        {"f_middle01_l", "\215\266\222\206\216\167\202\120",},
        {"f_middle02_l", "\215\266\222\206\216\167\202\121",},
        {"f_middle03_l", "\215\266\222\206\216\167\202\122",},
        {"f_ring01_l", "\215\266\226\362\216\167\202\120",},
        {"f_ring02_l", "\215\266\226\362\216\167\202\121",},
        {"f_ring03_l", "\215\266\226\362\216\167\202\122",},
        {"f_little01_l", "\215\266\217\254\216\167\202\120",},
        {"f_little02_l", "\215\266\217\254\216\167\202\121",},
        {"f_little03_l", "\215\266\217\254\216\167\202\122",},
        // Right Leg
        {"back_hips_c_r", "\215\230\203\114\203\203\203\223\203\132\203\213\211\105",},
        {"upleg_r", "\211\105\221\253",},
        {"lowleg_r", "\211\105\202\320\202\264",},
        {"foot_r", "\211\105\221\253\216\361",},
        {"toe_r", "\211\105\202\302\202\334\220\346",},
        {"ik_f_root_r", "\211\105\221\253\111\113\220\145",},
        {"ik_foot_r", "\211\105\221\253\202\150\202\152",},
        {"ik_toe_r", "\211\105\202\302\202\334\220\346\202\150\202\152",},
        {"upleg_d_r", "\211\105\221\253\104",},
        {"lowleg_d_r", "\211\105\202\320\202\264\104",},
        {"foot_d_r", "\211\105\221\253\216\361\104",},
        {"heel_ex_r", "\211\105\346\371\105\130",},
        {"toe_ex_r", "\211\105\221\253\220\346\105\130",},
        // Left Leg
        {"back_hips_c_l", "\215\230\203\114\203\203\203\223\203\132\203\213\215\266",},
        {"upleg_l", "\215\266\221\253",},
        {"lowleg_l", "\215\266\202\320\202\264",},
        {"foot_l", "\215\266\221\253\216\361",},
        {"toe_l", "\215\266\202\302\202\334\220\346",},
        {"ik_f_root_l", "\215\266\221\253\111\113\220\145",},
        {"ik_foot_l", "\215\266\221\253\202\150\202\152",},
        {"ik_toe_l", "\215\266\202\302\202\334\220\346\202\150\202\152",},
        {"upleg_d_l", "\215\266\221\253\104",},
        {"lowleg_d_l", "\215\266\202\320\202\264\104",},
        {"foot_d_l", "\215\266\221\253\216\361\104",},
        {"heel_ex_l", "\215\266\346\371\105\130",},
        {"toe_ex_l", "\215\266\221\253\220\346\105\130",},
        // Eyes
        {"eyes", "\227\274\226\332",},
        {"eye_r", "\211\105\226\332",},
        {"eye_l", "\215\266\226\332",},
        // Misc
        {"mouth_base", "\214\373\203\170\201\133\203\130",},
        {"eyebrow_base", "\224\373\203\170\201\133\203\130",},
    }; // bone_name_mappings END

    const std::map<std::string, std::string> ContentVMD::morph_name_mappings
    {
        // Mouth
        {"mouth_a", "\202\240",},
        {"mouth_i", "\202\242",},
        {"mouth_u", "\202\244",},
        {"mouth_e", "\202\246",},
        {"mouth_o", "\202\250",},
        {"mouth_wa", "\202\355",},
        {"mouth_n", "\202\361",},
        {"mouth_a2", "\202\240\202\121",},
        {"mouth_wa2", "\203\217",},
        {"mouth_smile", "\202\311\202\342\202\350",},
        {"mouth_smile2", "\202\311\202\342\202\350\202\121",},
        {"mouth_smile3", "\202\311\202\301\202\261\202\350",},
        {"mouth_tongueout", "\202\330\202\353\202\301",},
        {"mouth_he", "\201\310",},
        {"mouth_triangle", "\201\243",},
        {"mouth_cat", "\203\326",},
        {"mouth_cat2", "\203\326\201\240",},
        {"mouth_bothup", "\214\373\212\160\217\343\202\260",},
        {"mouth_bothdown", "\214\373\212\160\211\272\202\260",},
        {"mouth_wider", "\214\373\211\241\215\114\202\260",},
        // Eyes
        {"blink", "\202\334\202\316\202\275\202\253",},
        {"eyesmile", "\217\316\202\242",},
        {"eyesmile2", "\217\316\202\242\226\332",},
        {"wink_r", "\203\105\203\102\203\223\203\116\211\105",},
        {"wink_l", "\203\105\203\102\203\223\203\116",},
        {"wink_r2", "\263\250\335\270\202\121\211\105",},
        {"wink_l2", "\203\105\203\102\203\223\203\116\202\121",},
        {"surprise", "\202\321\202\301\202\255\202\350",},
        {"stare", "\202\266\202\306\226\332",},
        {"eye_angry", "\267\330\257",},
        {"nagomi", "\202\310\202\262\202\335",},
        {"hau", "\202\315\202\243",},
        {"hatyu", "\202\315\202\277\202\343\226\332",},
        {"star", "\220\257\226\332",},
        {"heart", "\202\315\202\237\202\306",},
        {"iris_shrink", "\223\265\217\254",},
        {"face_red", "\217\306\202\352",},
        // Eyebrow
        {"serious", "\220\136\226\312\226\332",},
        {"confuse", "\215\242\202\351",},
        {"angry", "\223\173\202\350",},
        {"eyebrowsmile", "\202\311\202\261\202\350",},
        {"eyebrowup", "\217\343",},
        {"eyebrowdown", "\211\272",},
    }; // morph_name_mappings END

    // VMD: init reversed mappings

    const std::map<std::string, std::string> ContentVMD::bone_name_mappings_fromraw
    {
        // Center
        {"\221\123\202\304\202\314\220\145","root",      },
        {"\203\132\203\223\203\136\201\133","center",    },
        {"\203\117\203\213\201\133\203\165","groove",    },
        {"\215\230",                        "back_hips", },
        // Body
        {"\221\314\202\314\217\144\220\123","hips",   },
        {"\217\343\224\274\220\147",        "spine",  },
        {"\217\343\224\274\220\147\62",     "spine2", },
        {"\217\343\224\274\220\147\63",     "spine3", },
        {"\216\361",                        "neck",   },
        {"\223\252",                        "head",   },
        {"\211\272\224\274\220\147",        "waist",  },
        // Right Arm
        {"\211\105\214\250\120",            "shoulder_p_r",   },
        {"\211\105\214\250",                "shoulder_r",     },
        {"\211\105\214\250\103",            "shoulder_c_r",   },
        {"\211\105\230\162",                "uparm_r",        },
        {"\211\105\230\162\235\200",        "uparm_twist_r",  },
        {"\211\105\202\320\202\266",        "lowarm_r",       },
        {"\211\105\216\350\235\200",        "lowarm_twist_r", },
        {"\211\105\216\350\216\361",        "hand_r",         },
        {"\211\105\203\137\203\176\201\133","hand_dummy_r",   },
        {"\211\105\230\162\202\150\202\152","ik_hand_r",      },
        // Left Arm
        {"\215\266\214\250\120",            "shoulder_p_l",   },
        {"\215\266\214\250",                "shoulder_l",     },
        {"\215\266\214\250\103",            "shoulder_c_l",   },
        {"\215\266\230\162",                "uparm_l",        },
        {"\215\266\230\162\235\200",        "uparm_twist_l",  },
        {"\215\266\202\320\202\266",        "lowarm_l",       },
        {"\215\266\216\350\235\200",        "lowarm_twist_l", },
        {"\215\266\216\350\216\361",        "hand_l",         },
        {"\215\266\203\137\203\176\201\133","hand_dummy_l",   },
        {"\215\266\230\162\202\150\202\152","ik_hand_l",      },
        // Right Hand
        {"\211\105\220\145\216\167\202\117","f_thumb01_r",  },
        {"\211\105\220\145\216\167\202\120","f_thumb02_r",  },
        {"\211\105\220\145\216\167\202\121","f_thumb03_r",  },
        {"\211\105\220\154\216\167\202\120","f_index01_r",  },
        {"\211\105\220\154\216\167\202\121","f_index02_r",  },
        {"\211\105\220\154\216\167\202\122","f_index03_r",  },
        {"\211\105\222\206\216\167\202\120","f_middle01_r", },
        {"\211\105\222\206\216\167\202\121","f_middle02_r", },
        {"\211\105\222\206\216\167\202\122","f_middle03_r", },
        {"\211\105\226\362\216\167\202\120","f_ring01_r",   },
        {"\211\105\226\362\216\167\202\121","f_ring02_r",   },
        {"\211\105\226\362\216\167\202\122","f_ring03_r",   },
        {"\211\105\217\254\216\167\202\120","f_little01_r", },
        {"\211\105\217\254\216\167\202\121","f_little02_r", },
        {"\211\105\217\254\216\167\202\122","f_little03_r", },
        // Left Hand
        {"\215\266\220\145\216\167\202\117","f_thumb01_l",  },
        {"\215\266\220\145\216\167\202\120","f_thumb02_l",  },
        {"\215\266\220\145\216\167\202\121","f_thumb03_l",  },
        {"\215\266\220\154\216\167\202\120","f_index01_l",  },
        {"\215\266\220\154\216\167\202\121","f_index02_l",  },
        {"\215\266\220\154\216\167\202\122","f_index03_l",  },
        {"\215\266\222\206\216\167\202\120","f_middle01_l", },
        {"\215\266\222\206\216\167\202\121","f_middle02_l", },
        {"\215\266\222\206\216\167\202\122","f_middle03_l", },
        {"\215\266\226\362\216\167\202\120","f_ring01_l",   },
        {"\215\266\226\362\216\167\202\121","f_ring02_l",   },
        {"\215\266\226\362\216\167\202\122","f_ring03_l",   },
        {"\215\266\217\254\216\167\202\120","f_little01_l", },
        {"\215\266\217\254\216\167\202\121","f_little02_l", },
        {"\215\266\217\254\216\167\202\122","f_little03_l", },
        // Right Leg
        {"\215\230\203\114\203\203\203\223\203\132\203\213\211\105","back_hips_c_r", },
        {"\211\105\221\253",                                        "upleg_r",       },
        {"\211\105\202\320\202\264",                                "lowleg_r",      },
        {"\211\105\221\253\216\361",                                "foot_r",        },
        {"\211\105\202\302\202\334\220\346",                        "toe_r",         },
        {"\211\105\221\253\111\113\220\145",                        "ik_f_root_r",   },
        {"\211\105\221\253\202\150\202\152",                        "ik_foot_r",     },
        {"\211\105\202\302\202\334\220\346\202\150\202\152",        "ik_toe_r",      },
        {"\211\105\221\253\104",                                    "upleg_d_r",     },
        {"\211\105\202\320\202\264\104",                            "lowleg_d_r",    },
        {"\211\105\221\253\216\361\104",                            "foot_d_r",      },
        {"\211\105\346\371\105\130",                                "heel_ex_r",     },
        {"\211\105\221\253\220\346\105\130",                        "toe_ex_r",      },
        // Left Leg
        {"\215\230\203\114\203\203\203\223\203\132\203\213\215\266","back_hips_c_l", },
        {"\215\266\221\253",                                        "upleg_l",       },
        {"\215\266\202\320\202\264",                                "lowleg_l",      },
        {"\215\266\221\253\216\361",                                "foot_l",        },
        {"\215\266\202\302\202\334\220\346",                        "toe_l",         },
        {"\215\266\221\253\111\113\220\145",                        "ik_f_root_l",   },
        {"\215\266\221\253\202\150\202\152",                        "ik_foot_l",     },
        {"\215\266\202\302\202\334\220\346\202\150\202\152",        "ik_toe_l",      },
        {"\215\266\221\253\104",                                    "upleg_d_l",     },
        {"\215\266\202\320\202\264\104",                            "lowleg_d_l",    },
        {"\215\266\221\253\216\361\104",                            "foot_d_l",      },
        {"\215\266\346\371\105\130",                                "heel_ex_l",     },
        {"\215\266\221\253\220\346\105\130",                        "toe_ex_l",      },
        // Eyes
        {"\227\274\226\332","eyes",  },
        {"\211\105\226\332","eye_r", },
        {"\215\266\226\332","eye_l", },
        // Misc
        {"\214\373\203\170\201\133\203\130","mouth_base",   },
        {"\224\373\203\170\201\133\203\130","eyebrow_base", },
    }; // bone_name_mappings END

    const std::map<std::string, std::string> ContentVMD::morph_name_mappings_fromraw
    {
        // Mouth
        {"\202\240",                         "mouth_a",         },
        {"\202\242",                         "mouth_i",         },
        {"\202\244",                         "mouth_u",         },
        {"\202\246",                         "mouth_e",         },
        {"\202\250",                         "mouth_o",         },
        {"\202\355",                         "mouth_wa",        },
        {"\202\361",                         "mouth_n",         },
        {"\202\240\202\121",                 "mouth_a2",        },
        {"\203\217",                         "mouth_wa2",       },
        {"\202\311\202\342\202\350",         "mouth_smile",     },
        {"\202\311\202\342\202\350\202\121", "mouth_smile2",    },
        {"\202\311\202\301\202\261\202\350", "mouth_smile3",    },
        {"\202\330\202\353\202\301",         "mouth_tongueout", },
        {"\201\310",                         "mouth_he",        },
        {"\201\243",                         "mouth_triangle",  },
        {"\203\326",                         "mouth_cat",       },
        {"\203\326\201\240",                 "mouth_cat2",      },
        {"\214\373\212\160\217\343\202\260", "mouth_bothup",    },
        {"\214\373\212\160\211\272\202\260", "mouth_bothdown",  },
        {"\214\373\211\241\215\114\202\260", "mouth_wider",     },
        // Eyes
        {"\202\334\202\316\202\275\202\253",        "blink",       },
        {"\217\316\202\242",                        "eyesmile",    },
        {"\217\316\202\242\226\332",                "eyesmile2",   },
        {"\203\105\203\102\203\223\203\116\211\105","wink_r",      },
        {"\203\105\203\102\203\223\203\116",        "wink_l",      },
        {"\263\250\335\270\202\121\211\105",        "wink_r2",     },
        {"\203\105\203\102\203\223\203\116\202\121","wink_l2",     },
        {"\202\321\202\301\202\255\202\350",        "surprise",    },
        {"\202\266\202\306\226\332",                "stare",       },
        {"\267\330\257",                            "eye_angry",   },
        {"\202\310\202\262\202\335",                "nagomi",      },
        {"\202\315\202\243",                        "hau",         },
        {"\202\315\202\277\202\343\226\332",        "hatyu",       },
        {"\220\257\226\332",                        "star",        },
        {"\202\315\202\237\202\306",                "heart",       },
        {"\223\265\217\254",                        "iris_shrink", },
        {"\217\306\202\352",                        "face_red",    },
        // Eyebrow
        {"\220\136\226\312\226\332","serious",      },
        {"\215\242\202\351",        "confuse",      },
        {"\223\173\202\350",        "angry",        },
        {"\202\311\202\261\202\350","eyebrowsmile", },
        {"\217\343",                "eyebrowup",    },
        {"\211\272",                "eyebrowdown",  },
    }; // morph_name_mappings END


    void ContentVMD::SetNameFromMapping(BoneKeyframe &_bone_kframe, std::string _mapping_str) {
        for (char &byte : _bone_kframe.bone_name) byte = '\0';

        _mapping_str = bone_name_mappings.at(_mapping_str);
        for (int i = 0; i < _mapping_str.size(); ++i)
            _bone_kframe.bone_name[i] = _mapping_str[i];
    }

    void ContentVMD::CreateReflected(const std::string &_vmd_path_no_ext, bool _fix_blink) {
        ReaderWriterVMD __vmd_file{_vmd_path_no_ext + ".vmd"};

        if (_fix_blink) {
            ContentVMD::MorphAttrClampMinmax blink_clamp_info{0.01f, 0.99f};
            __vmd_file.file_content.clampFrames({}, {{"blink", blink_clamp_info}});
            __vmd_file.write(_vmd_path_no_ext + "2.vmd");
        }

        __vmd_file.file_content.reflectMovement();
        __vmd_file.write(_vmd_path_no_ext + (_fix_blink ? "2" : "") + "_reflected.vmd");
    }

    void ContentVMD::CreateMixed(const std::string &_motion_vmd_path_no_ext, const std::string &_camera_vmd_path_no_ext, const std::vector<uint32_t> &_split_frame_ids, const AttrAdjust &_motion1_offset,
                                 const AttrAdjust &_camera1_scale, const AttrAdjust &_camera1_offset, const AttrAdjust &_camera2_scale, const AttrAdjust &_camera2_offset, bool _fix_blink) {
        // Reflected motion
        ReaderWriterVMD __vmd_file{_motion_vmd_path_no_ext + ".vmd"};

        if (!__vmd_file.file_content.keyframes_camera.empty())
            throw std::invalid_argument(SVKFW_WRAPERR("ContentVMD :: CreateMixed", "motion VMD file has camera keyframes"));

        bool __has_root = false;
        for (uint32_t i = 0u; i < __vmd_file.file_content.keyframes_bone.size(); ++i)
            if (ContentVMD::bone_name_mappings_fromraw.find(__vmd_file.file_content.keyframes_bone[i].bone_name) != ContentVMD::bone_name_mappings_fromraw.end() &&
                ContentVMD::bone_name_mappings_fromraw.  at(__vmd_file.file_content.keyframes_bone[i].bone_name) == "root") {
                __has_root = true;
                break;
            }

        if (__has_root)
            __vmd_file.file_content.offsetKeyframes({{"root", _motion1_offset}}, {});
        else {
            InterpolationInfo __root_interp;
            BoneKeyframe      __root_kframe{};

            __root_kframe.position = _motion1_offset.pos_dist["xyz"];
            __root_kframe.rotation = Affine::Rotation::ypr2quaternion(_motion1_offset.rot_fov["xyz"]);

            SetInterpolation  (__root_kframe, __root_interp);
            SetNameFromMapping(__root_kframe,  "root");

            __vmd_file.file_content.keyframes_bone.insert(__vmd_file.file_content.keyframes_bone.begin(), __root_kframe);
        }

        if (_fix_blink) {
            ContentVMD::MorphAttrClampMinmax __blink_clamp_info{0.01f, 0.99f};
            __vmd_file.file_content.clampFrames({}, {{"blink", __blink_clamp_info}});
        }

        __vmd_file.write(_motion_vmd_path_no_ext + "2.vmd");

        __vmd_file.file_content.reflectMovement();
        __vmd_file.write(_motion_vmd_path_no_ext + "2_reflected.vmd");


        // Reflected camera
        __vmd_file.read(_camera_vmd_path_no_ext + ".vmd");
        ReaderWriterVMD __vmd_file2{_camera_vmd_path_no_ext + ".vmd"};

        if (__vmd_file.file_content.keyframes_camera.empty())
            throw std::invalid_argument(SVKFW_WRAPERR("ContentVMD :: CreateMixed", "camera VMD file has no camera keyframes"));

        __vmd_file .file_content.multKeyframes  ({}, {}, _camera1_scale );
        __vmd_file .file_content.offsetKeyframes({}, {}, _camera1_offset);
        __vmd_file2.file_content.multKeyframes  ({}, {}, _camera2_scale );
        __vmd_file2.file_content.offsetKeyframes({}, {}, _camera2_offset);

        __vmd_file2.file_content.reflectMovement();

        __vmd_file .write(_camera_vmd_path_no_ext + "2.vmd");
        __vmd_file2.write(_camera_vmd_path_no_ext + "2_reflected.vmd");

        // Mixed camera
        __vmd_file.file_content.mixCamera(__vmd_file2.file_content, _split_frame_ids);
        __vmd_file .write(_camera_vmd_path_no_ext + "2_mixed.vmd");
    }


    // OGG

    struct ContentOGG : FiletypeContentItf {
        struct PageOGG {
            uint32_t signature; // these 4 bytes must be "OggS"
            uint8_t  version;
            uint8_t  header_flags;
            uint64_t granule_position;
            uint32_t serial_number;
            uint32_t sequence_number;
            uint32_t checksum;
            uint8_t  total_segments;
            char segment_table[256];
        }; // PageOGG END
        std::vector<PageOGG> data_pages;
    }; // ContentOGG END


    // JSON

    struct ContentJSON : FiletypeContentItf {

    }; // ContentJSON END
}; // Simple END

#endif
