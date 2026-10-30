#include "data/filetypes.h"
#include <algorithm>

namespace Simple {
    namespace File {

        FiletypeContentItf::~FiletypeContentItf() {}

        FiletypeReaderWriterItf::~FiletypeReaderWriterItf() {}

        bool pathHasExtension(const std::string &_fpath) {
            bool __res = true;
            if ( _fpath.rfind('.') == _fpath.npos || _fpath == ".") __res = false;
            if ((_fpath.rfind( '/') < _fpath.npos && _fpath.rfind('.') <= _fpath.rfind( '/'))  ||
                (_fpath.rfind('\\') < _fpath.npos && _fpath.rfind('.') <= _fpath.rfind('\\'))) __res = false;
            return __res;
        }

        std::string pathGetExtension(const std::string &_fpath) {
            return pathHasExtension(_fpath) ? _fpath.substr(_fpath.rfind('.') + 1) : "";
        }

        std::string pathRemoveExtension(const std::string &_fpath) {
            return pathHasExtension(_fpath) ? _fpath.substr(0u, _fpath.rfind('.')) : _fpath;
        }

        ContentVMD::AttrAdjust::AttrAdjust(bool _is_offset_scale) {
            _is_offset_scale ? defOffset() : defScale();
        }

        ContentVMD::AttrAdjust::AttrAdjust(const vec3f &_pos, const vec3f &_rot,
                float _cam_dist, float _cam_fov) : pos_dist{_pos, _cam_dist}, rot_fov{_rot, _cam_fov} {}

        void ContentVMD::AttrAdjust::defScale() {
            pos_dist = 1.f;
            rot_fov  = 1.f;
        }

        void ContentVMD::AttrAdjust::defOffset() {
            pos_dist = 0.f;
            rot_fov  = 0.f;
        }

        ContentVMD::BoneAttrClampMinmax::BoneAttrClampMinmax(const vec3f &_rot_min, const vec3f &_rot_max,
                            const vec3f &_pos_min, const vec3f &_pos_max) : rot_min{_rot_min}, rot_max{_rot_max},
                                                                                               pos_min{_pos_min}, pos_max{_pos_max} {}

        ContentVMD::MorphAttrClampMinmax::MorphAttrClampMinmax(float _val_min, float _val_max) : val_min{_val_min}, val_max{_val_max} {}

        bool       ContentVMD::BoneMapped(const std::string & _bone_name) { return  bone_name_mappings_fromraw.find( _bone_name) !=  bone_name_mappings_fromraw.end(); }

        bool      ContentVMD::MorphMapped(const std::string &_morph_name) { return morph_name_mappings_fromraw.find(_morph_name) != morph_name_mappings_fromraw.end(); }

        bool  ContentVMD::EqToBoneMapping(const std::string & _bone_name, const std::string &_comp_name) { return  BoneMapped( _bone_name) &&
                                                                                                              bone_name_mappings_fromraw.at( _bone_name) == _comp_name; }

        bool ContentVMD::EqToMorphMapping(const std::string &_morph_name, const std::string &_comp_name) { return MorphMapped(_morph_name) &&
                                                                                                             morph_name_mappings_fromraw.at(_morph_name) == _comp_name; }

        bool ContentVMD::IsLeftBone  (const std::string & _bone_name) { return  _bone_name.substr(0, 2) == "\215\266"; }

        bool ContentVMD::IsRightBone (const std::string & _bone_name) { return  _bone_name.substr(0, 2) == "\211\105"; }

        bool ContentVMD::IsLeftMorph (const std::string &_morph_name) { return _morph_name.substr(0, 2) == "\215\266"; }

        bool ContentVMD::IsRightMorph(const std::string &_morph_name) { return _morph_name.substr(0, 2) == "\211\105"; }

        std::string ContentVMD::GetBoneNameSafe(const std::string &_raw_bone_name) {
            return bone_name_mappings_fromraw.find(_raw_bone_name) != bone_name_mappings_fromraw.end() ?
                   bone_name_mappings_fromraw.  at(_raw_bone_name) :  "0000000000000000000000000000000";
        }

        std::string ContentVMD::GetMorphNameSafe(const std::string &_raw_morph_name) {
            return morph_name_mappings_fromraw.find(_raw_morph_name) != morph_name_mappings_fromraw.end() ?
                   morph_name_mappings_fromraw.  at(_raw_morph_name) :  "0000000000000000000000000000000";
        }

        std::string ContentVMD::GetOppositeBone(const std::string &  _bone_name) {
            if ( IsLeftBone(_bone_name))
                return "\211\105" + _bone_name.substr(2);
            if (IsRightBone(_bone_name))
                return "\215\266" + _bone_name.substr(2);
            return _bone_name;
        }

        std::string ContentVMD::GetOppositeMorph(const std::string & _morph_name) {
            if ( IsLeftMorph(_morph_name))
                return "\211\105" + _morph_name.substr(2);
            if (IsRightMorph(_morph_name))
                return "\215\266" + _morph_name.substr(2);
            return _morph_name;
        }

        bool ContentVMD::GetOppositeNameInplace(char *_bone_morph_name) {
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

        void ContentVMD::ReflectBoneKeyframe(BoneKeyframe &_bone_kf) {
            GetOppositeNameInplace(_bone_kf.bone_name);
            _bone_kf.position.x *= -1.f;
            vec3f __euler = Affine::Rotation::quaternion2ypr(_bone_kf.rotation);
            __euler["xz"] *= -1.f;
            _bone_kf.rotation = Affine::Rotation::ypr2quaternion(__euler);
        }

        void ContentVMD::ReflectMorphKeyframe(MorphKeyframe &_morph_kf) {
            GetOppositeNameInplace(_morph_kf.morph_name);
        }

        void ContentVMD::ReflectCameraKeyframe(CameraKeyframe &_camera_kf) {
            _camera_kf.position.x *= -1.f;
            _camera_kf.rotation["yz"] *= -1.f;
        }

        void ContentVMD::sortBoneKeyframes() {
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

        void ContentVMD::sortMorphKeyframes() {
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

        void ContentVMD::reflectMovement() {
            for (uint32_t i = 0u; i < keyframes_bone.size(); ++i)
                ReflectBoneKeyframe  (keyframes_bone[i]);
            for (uint32_t i = 0u; i < keyframes_morph.size(); ++i)
                ReflectMorphKeyframe (keyframes_morph[i]);
            for (uint32_t i = 0u; i < keyframes_camera.size(); ++i)
                ReflectCameraKeyframe(keyframes_camera[i]);
        }

        void ContentVMD::multKeyframes(const std::map<std::string, AttrAdjust> &_mult_bones,
                           const std::map<std::string,      float> &_mult_morphs,
                           AttrAdjust _cam_mult) {
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
                cam_kframe.fov      *= _cam_mult.rot_fov.w;
            }
        }

        void ContentVMD::offsetKeyframes(const std::map<std::string, AttrAdjust> &_offset_bones,
                             const std::map<std::string,      float> &_offset_morphs,
                             AttrAdjust _cam_offset) {
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

        void ContentVMD::clampFrames(const std::vector<std::pair<std::string,  BoneAttrClampMinmax>> &_clamp_bones,
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

        ContentVMD::BiquadCoeffs ContentVMD::makeLowpassBiquad(float fs, float fc, float Q) {
            float w0 = 2.f * M_PI * fc / fs;
            float cosw0 = std::cos(w0);
            float alpha = std::sin(w0) / (2.f * Q);

            float a0 = 1.f + alpha;
            BiquadCoeffs c;
            c.b0 = ((1.f - cosw0) / 2.0f) / a0;
            c.b1 =  (1.f - cosw0)         / a0;
            c.b2 = c.b0;
            c.a1 = (-2.0f * cosw0) / a0;
            c.a2 = ( 1.0f - alpha) / a0;
            return c;
        }

        float ContentVMD::biquadStep(const BiquadCoeffs& c, BiquadState& s, float x) {
            float y = c.b0 * x + s.z1;
            s.z1 = c.b1 * x - c.a1 * y + s.z2;
            s.z2 = c.b2 * x - c.a2 * y;
            return y;
        }

        void ContentVMD::butterworthLowpass4(std::vector<vec3f> &data, float _freq_sample_hz, float _freq_cutout_hz, bool _is_forward) {
            if (data.empty()) return;

            const float Q1 = 0.54119610f;
            const float Q2 = 1.30656296f;
            BiquadState s1[3], s2[3];

            // Seed from the first sample in the iteration direction
            BiquadCoeffs c1 = makeLowpassBiquad(_freq_sample_hz, _freq_cutout_hz, Q1);
            BiquadCoeffs c2 = makeLowpassBiquad(_freq_sample_hz, _freq_cutout_hz, Q2);
            vec3f seed = _is_forward ? data.front() : data.back();
            // Steady state for constant input 'seed' (DC gain is 1, so output == input): z2 = (b2-a2)*x, z1 = (b1-a1)*x + z2
            for (int k = 0; k < 3; ++k) {
                s1[k].z2 = (c1.b2 - c1.a2) * seed[k];
                s1[k].z1 = (c1.b1 - c1.a1) * seed[k] + s1[k].z2;
                s2[k].z2 = (c2.b2 - c2.a2) * seed[k];
                s2[k].z1 = (c2.b1 - c2.a1) * seed[k] + s2[k].z2;
            }
            for (int step = 0; step < data.size(); ++step) {
                int i = _is_forward ? step : (data.size() - 1 - step);
                vec3f res;

                for (int k = 0; k < 3; ++k) {
                    float t = biquadStep(c1, s1[k], data[i][k]);
                    res[k]  = biquadStep(c2, s2[k], t);
                }
                data[i] = res;
            }
        }

        float ContentVMD::findU(float _t, float _x1, float _x2) {
            if (_t <= 0.f) return 0.f;
            if (_t >= 1.f) return 1.f;

            float lower = 0.f;
            float upper = 1.f;
            float u     = 0.5f;

            for (int i = 0; i < 12; ++i) {
                float current_x = 3.f * (1.f - u) * (1.f - u) * u * _x1 +
                                  3.f * (1.f - u) *        u  * u * _x2 +
                                               u  *        u  * u;

                if (std::abs(current_x - _t) < 1e-4f) break;
                if (current_x < _t) lower = u;
                else                upper = u;
                u = (lower + upper) * 0.5f;
            }
            return u;
        }

        float ContentVMD::interpolateForMMD(float _t, vec2f _p1, vec2f _p2) {
            _p1 /= 127;
            _p2 /= 127;
            float u = findU(_t, _p1.x, _p2.x);
            float __res = 3.f * (1.f - u) * (1.f - u) * u * _p1.y +
                          3.f * (1.f - u) *        u  * u * _p2.y +
                                       u  *        u  * u;
            return __res;
        }

        void ContentVMD::reinterpolateBone(const std::string &_raw_bone_name) {
            // Reinterpolate existing Bezier curves
            std::vector<uint32_t> __bone_kframe_ids;
            std::vector<vec3f> __kframe_data; // in_vel, out_vel, do_keep

            // for (uint32_t i = 0u; i < keyframes_bone.size(); ++i) {
            //     if (keyframes_bone[i].bone_name == _raw_bone_name)
            //         __bone_kframe_ids.push_back(i);
            // }
            // if (__bone_kframe_ids.size() <= 2) return;
            // __kframe_data.resize(__bone_kframe_ids.size());

            // for (uint32_t i = 1u; i < __kframe_data.size() - 1u; ++i) {
            //     // TODO:
            // }

            __kframe_data.clear();
            __bone_kframe_ids.clear();


            std::vector<BoneKeyframe> __bone_keyframes;

            // Obtain/Interpolate every keyframe (30 FPS)
            for (auto& bone_kframe : keyframes_bone) {
                if (bone_kframe.bone_name != _raw_bone_name) continue;
                if (!__bone_keyframes.empty()) {
                    const uint32_t __first_ind = __bone_keyframes.size() - 1;
                    const uint32_t __new_kframes = bone_kframe.frame_id - __bone_keyframes[__first_ind].frame_id;
                    const vec3b __update_pos = __bone_keyframes[__first_ind].position != bone_kframe.position;
                    // Note: interpolation curve of the segment A->B is stored in the keyframe B
                    const InterpolationInfo __int_info =  GetInterpolation(bone_kframe);

                    __bone_keyframes.resize(__bone_keyframes.size() + __new_kframes);
                    if (Math::dot(__bone_keyframes[__first_ind].rotation, bone_kframe.rotation) < 1e-6f)
                        bone_kframe.rotation = -1.f * bone_kframe.rotation;
                    __bone_keyframes[__first_ind + __new_kframes] = bone_kframe;

                    for (uint32_t i = 1u; i < __new_kframes; ++i) {
                        __bone_keyframes[__first_ind + i] = __bone_keyframes[__first_ind];
                        __bone_keyframes[__first_ind + i].frame_id += i;

                        float __interp_t;
                        if (__update_pos.x) {
                            // Pos X
                            __interp_t = interpolateForMMD(float(i) / __new_kframes, __int_info.mov_x["xy"].cast<float>(), __int_info.mov_x["zw"].cast<float>());
                            __bone_keyframes[__first_ind + i].position.x = Interpolation::BezierLinear<float>::Interpolate(__interp_t, __bone_keyframes[__first_ind].position.x, bone_kframe.position.x);
                        }
                        if (__update_pos.y) {
                            // Pos Y
                            __interp_t = interpolateForMMD(float(i) / __new_kframes, __int_info.mov_y["xy"].cast<float>(), __int_info.mov_y["zw"].cast<float>());
                            __bone_keyframes[__first_ind + i].position.y = Interpolation::BezierLinear<float>::Interpolate(__interp_t, __bone_keyframes[__first_ind].position.y, bone_kframe.position.y);
                        }
                        if (__update_pos.z) {
                            // Pos Z
                            __interp_t = interpolateForMMD(float(i) / __new_kframes, __int_info.mov_z["xy"].cast<float>(), __int_info.mov_z["zw"].cast<float>());
                            __bone_keyframes[__first_ind + i].position.z = Interpolation::BezierLinear<float>::Interpolate(__interp_t, __bone_keyframes[__first_ind].position.z, bone_kframe.position.z);
                        }
                        // Rot
                        __interp_t = interpolateForMMD(float(i) / __new_kframes, __int_info.rot["xy"].cast<float>(), __int_info.rot["zw"].cast<float>());
                        __bone_keyframes[__first_ind + i].rotation = Interpolation::Slerp::Interpolate(__interp_t, __bone_keyframes[__first_ind].rotation, bone_kframe.rotation);
                    }
                }
                else __bone_keyframes.insert(__bone_keyframes.end(), bone_kframe);
            }

            // Remove old keyframes
            for (int i = keyframes_bone.size() - 1; i >= 0; --i)
                if (keyframes_bone[i].bone_name == _raw_bone_name)
                    keyframes_bone.erase(keyframes_bone.begin() + i);

            // Convert quaternion rotations into tangent-space vectors
            std::vector<vec3f> __tangent_vecs(__bone_keyframes.size());
            for (uint32_t i = 0u; i < __tangent_vecs.size(); ++i)
                __tangent_vecs[i] = Affine::Rotation::quaternion2tangent(__bone_keyframes[i].rotation);

            // Apply Butterworth low-pass filter
            butterworthLowpass4(__tangent_vecs, 30.f, 4.f,  true);
            butterworthLowpass4(__tangent_vecs, 30.f, 4.f, false);

            // Convert vectors back to quaternions
            for (uint32_t i = 0u; i < __tangent_vecs.size(); ++i)
                __bone_keyframes[i].rotation = Affine::Rotation::tangent2quaternion(__tangent_vecs[i]).normalize();

            // Add new keyframes
            keyframes_bone.insert(keyframes_bone.end(), __bone_keyframes.begin(), __bone_keyframes.end());
        }

        void ContentVMD::reinterpolateBones() {
            const std::vector<std::string> __ignored_bones = {
                "hand_dummy_r", "hand_dummy_l",
                "f_thumb01_r", "f_thumb02_r", "f_thumb03_r", "f_index01_r", "f_index02_r", "f_index03_r", "f_middle01_r", "f_middle02_r", "f_middle03_r", "f_ring01_r", "f_ring02_r", "f_ring03_r", "f_little01_r", "f_little02_r", "f_little03_r",
                "f_thumb01_l", "f_thumb02_l", "f_thumb03_l", "f_index01_l", "f_index02_l", "f_index03_l", "f_middle01_l", "f_middle02_l", "f_middle03_l", "f_ring01_l", "f_ring02_l", "f_ring03_l", "f_little01_l", "f_little02_l", "f_little03_l",
                "eyes", "eye_r", "eye_l", "mouth_base", "eyebrow_base",
            };

            for (const auto& bone_mapping : bone_name_mappings) {
                bool __ignore_bone = false;
                for (const auto& ignored_bone : __ignored_bones) {
                    __ignore_bone = bone_mapping.first == ignored_bone;
                    if (__ignore_bone) break;
                }
                if (__ignore_bone) continue;

                printf(SVKFW_WRAPINFO("ContentVMD :: reinterpolateBones", "Reinterpolating bone: %s\n"), bone_mapping.first.c_str());
                reinterpolateBone(bone_mapping.second);
            }
            sortBoneKeyframes();
        }

        ContentVMD::CRDataP::CRDataP() {}

        ContentVMD::CRDataP::CRDataP(const CameraKeyframe &_cam_kframe) {
            pos = _cam_kframe.position;
            tangent = Affine::Rotation::quaternion2tangent(Affine::Rotation::ypr2quaternion(_cam_kframe.rotation));
            fov = _cam_kframe.fov;
            dist = _cam_kframe.distance;
        }

        ContentVMD::CRDataP::CRDataP(const vec3f &_pos, const vec3f &_tang, float _fov, float _dist)
                        : pos{_pos}, tangent{_tang}, fov{_fov}, dist{_dist} {}

        ContentVMD::CRDataP ContentVMD::CRDataP::operator+(const ContentVMD::CRDataP&_op)
            { return { pos + _op.pos, tangent + _op.tangent, fov + _op.fov, dist + _op.dist }; }

        ContentVMD::CRDataP ContentVMD::CRDataP::operator-(const ContentVMD::CRDataP&_op)
            { return { pos - _op.pos, tangent - _op.tangent, fov - _op.fov, dist - _op.dist }; }

        ContentVMD::CRDataP ContentVMD::CRDataP::operator*(const ContentVMD::CRDataP&_op)
            { return { pos * _op.pos, tangent * _op.tangent, fov * _op.fov, dist * _op.dist }; }

        ContentVMD::CRDataP ContentVMD::CRDataP::operator/(const ContentVMD::CRDataP&_op)
            { return { pos / _op.pos, tangent / _op.tangent, fov / _op.fov, dist / _op.dist }; }

        ContentVMD::CRDataP ContentVMD::CRDataP::operator+(const ContentVMD::CRDataT&_op)
            { return { pos + _op.x, tangent + _op.y, fov + _op.z, dist + _op.w }; }

        ContentVMD::CRDataP ContentVMD::CRDataP::operator-(const ContentVMD::CRDataT&_op)
            { return { pos - _op.x, tangent - _op.y, fov - _op.z, dist - _op.w }; }

        ContentVMD::CRDataP ContentVMD::CRDataP::operator*(const ContentVMD::CRDataT&_op)
            { return { pos * _op.x, tangent * _op.y, fov * _op.z, dist * _op.w }; }

        ContentVMD::CRDataP ContentVMD::CRDataP::operator/(const ContentVMD::CRDataT&_op)
            { return { pos / _op.x, tangent / _op.y, fov / _op.z, dist / _op.w }; }

        vec4f ContentVMD::DataPtoT(CRDataP _data_new, CRDataP _data_old) {
            return { (_data_new.pos     - _data_old.pos    ).norm(),
                     (_data_new.tangent - _data_old.tangent).norm(),
                     std::sqrt(std::abs(_data_new.fov  - _data_old.fov )),
                     std::sqrt(std::abs(_data_new.dist - _data_old.dist))};
        }

        void ContentVMD::reinterpolateCamera(bool _bake_path) {
            if (keyframes_camera.size() < 2) return;
            const vec2f x{ 1.f/3, 2.f/3 };
            CRDataT t[4]{};
            CRDataP m[2]{}, p[4]{ keyframes_camera[0], keyframes_camera[0],
                                  keyframes_camera[1], keyframes_camera[keyframes_camera.size() < 3 ? 1 : 2], };

            CRDataT __delta0, __delta1;
            __delta0 = DataPtoT(p[2], p[1]);
            __delta1 = DataPtoT(p[3], p[2]);
            t[1] = DataPtoT(p[1], p[0]);
            t[2] = t[1] + __delta0;
            t[3] = t[2] + __delta1;

            // Note: control points' Y can be outside of [0, 1], but VMD stores them in [0, 127]
            auto __toInterpBytes = [&x](float _y1, float _y2) -> vec4u8 {
                return (vec4f(x[0], Math::clampCL(_y1, 0.f, 1.f), x[1], Math::clampCL(_y2, 0.f, 1.f)) * 127.f + 0.5f).cast<uint8_t>();
            };

            // Note: the segment p[1]->p[2] is processed at each iteration; it's curve is stored in its end keyframe (p[2], i+1)
            InterpolationInfo __interp_info;
            for (uint32_t i = 0u; i + 1u < keyframes_camera.size(); ++i) {
                m[0] = (p[2] - p[0]) / (t[2] - t[0]);
                m[1] = (p[3] - p[1]) / (t[3] - t[1]);

                CRDataP y1 = m[0] * (t[2] - t[1]) / ((p[2] - p[1]) * 3.f),
                        y2 = m[1] *-(t[2] - t[1]) / ((p[2] - p[1]) * 3.f) + 1.f;

                __interp_info = GetInterpolation(keyframes_camera[i+1]);
                if (__delta0.x > 1e-4f) {
                    if (std::isfinite(y1.pos.x) && std::isfinite(y2.pos.x))
                        __interp_info.mov_x = __toInterpBytes(y1.pos.x, y2.pos.x);
                    if (std::isfinite(y1.pos.y) && std::isfinite(y2.pos.y))
                        __interp_info.mov_y = __toInterpBytes(y1.pos.y, y2.pos.y);
                    if (std::isfinite(y1.pos.z) && std::isfinite(y2.pos.z))
                        __interp_info.mov_z = __toInterpBytes(y1.pos.z, y2.pos.z);
                }
                if (__delta0.z > 1e-4f)
                    if (std::isfinite(y1.fov ) && std::isfinite(y2.fov ))
                        __interp_info.fov  = __toInterpBytes(y1.fov,  y2.fov );
                if (__delta0.w > 1e-4f)
                    if (std::isfinite(y1.dist) && std::isfinite(y2.dist))
                        __interp_info.dist = __toInterpBytes(y1.dist, y2.dist);
                SetInterpolation(keyframes_camera[i+1], __interp_info);


                // Shift the window: p = { k[i], k[i+1], k[i+2], k[i+3] } for the next iteration
                p[0] = p[1]; p[1] = p[2]; p[2] = p[3];
                p[3] = keyframes_camera[std::min(i+3u, (uint32_t)keyframes_camera.size()-1u)];
                __delta0 = DataPtoT(p[2], p[1]);
                __delta1 = DataPtoT(p[3], p[2]);

                t[0] = t[1]; t[1] = t[2]; t[2] = t[3];
                t[3] = t[2] + __delta1 * (__delta1 > 1e-4f);
            }
        }

        void ContentVMD::mixCamera(const ContentVMD &_other_camera, const std::vector<uint32_t> &_switch_frame_ids) {
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

            // Last segment: from the last switch frame to the end
            __ind_from = __ind_to;
            for (const auto& cam_kframe : (_switch_frame_ids.size() & 1 ? _other_camera.keyframes_camera : keyframes_camera))
                if (__ind_from <= cam_kframe.frame_id)
                    __tmp_keyframes_camera.push_back(cam_kframe);

            keyframes_camera = std::move(__tmp_keyframes_camera);
        }

        ContentVMD::InterpolationInfo ContentVMD::GetInterpolation(BoneKeyframe &_bone_kframe) {
            // Interpolation info (in bytes) in range [0,127]:
            //  0-15: MovX.0x,MovY.0x,      0,     0, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
            // 16-30:         MovY.0x,MovZ.0x,Rot.0x, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
            // 31-45:               0,MovZ.0x,Rot.0x, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
            // 46-60:               0,      0,Rot.0x, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
            // 61-63:               0,      0,     0
            InterpolationInfo __res{};
            __res.mov_x = { _bone_kframe.int_info[ 0], _bone_kframe.int_info[ 4], _bone_kframe.int_info[ 8], _bone_kframe.int_info[12] };
            __res.mov_y = { _bone_kframe.int_info[ 1], _bone_kframe.int_info[ 5], _bone_kframe.int_info[ 9], _bone_kframe.int_info[13] };
            __res.mov_z = { _bone_kframe.int_info[17], _bone_kframe.int_info[ 6], _bone_kframe.int_info[10], _bone_kframe.int_info[14] };
            __res.rot   = { _bone_kframe.int_info[18], _bone_kframe.int_info[ 7], _bone_kframe.int_info[11], _bone_kframe.int_info[15] };
            return __res;
        }

        ContentVMD::InterpolationInfo ContentVMD::GetInterpolation(CameraKeyframe &_cam_kframe) {
            // Interpolation info (in bytes) in range [0,127]:
            //  0- 3:  moveX.0x, moveX.1x, moveX.0y, moveX.1y
            //  4- 7:  moveY.0x, moveY.1x, moveY.0y, moveY.1y
            //  8-11:  moveZ.0x, moveZ.1x, moveZ.0y, moveZ.1y
            // 12-15:    Rot.0x,   Rot.1x,   Rot.0y,   Rot.1y
            // 16-19:   Dist.0x,  Dist.1x,  Dist.0y,  Dist.1y
            // 20-23:    FOV.0x,   FOV.1x,   FOV.0y,   FOV.1y
            // Note: file order is (0x, 1x, 0y, 1y), InterpolationInfo order is (0x, 0y, 1x, 1y) - same as in SetInterpolation()
            InterpolationInfo __res;
            vec4u8 *__res_info[6] = { &__res.mov_x, &__res.mov_y, &__res.mov_z, &__res.rot, &__res.dist, &__res.fov };
            for (uint32_t i = 0u; i < 6u; ++i)
                *__res_info[i] = { _cam_kframe.int_info[4*i + 0], _cam_kframe.int_info[4*i + 2],
                                   _cam_kframe.int_info[4*i + 1], _cam_kframe.int_info[4*i + 3] };
            return __res;
        }

        void ContentVMD::SetInterpolation(BoneKeyframe &_bone_kframe, const InterpolationInfo &_int_info) {
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

        void ContentVMD::SetInterpolation(CameraKeyframe &_cam_kframe, const InterpolationInfo &_int_info) {
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
            __int_info[3] = _int_info.  rot["xzyw"];
            __int_info[4] = _int_info. dist["xzyw"];
            __int_info[5] = _int_info.  fov["xzyw"];
        }

        ReaderWriterVMD::ReaderWriterVMD(const std::string &_fpath) { if (!_fpath.empty()) read(_fpath); }

        bool ReaderWriterVMD::read(const std::string &_fpath) {
            // Open file
            FileReader __ftype_file{_fpath};
            if (!__ftype_file.isOpen()) return false;
            file_content.is_old_format = false;

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

            printf(SVKFW_WRAPINFO("VMD Reader : Model name", "'%.*s'\n"), int(__model_name_len), file_content.model); // not null-terminated

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
            __keyframes_count = 0u; // stays 0 if the section is missing (EOF)
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
            __keyframes_count = 0u; // stays 0 if the section is missing (EOF)
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
            return __ftype_file.isValid() || __ftype_file.isEOF();
        }

        bool ReaderWriterVMD::write(const std::string &_fpath) {
            // Open file (existing file is truncated)
            FileWriter __ftype_file{_fpath, true};
            if (!__ftype_file.isOpen()) return false;

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
            return __ftype_file.isValid();
        }

        bool ReaderWriterVMD::tryHeaders(ReaderItf &_ftype_file) {
            const uint32_t header_len = 30;
            char __ch_buffer[32]{};

            _ftype_file.read(__ch_buffer, header_len);
            std::string __res_str = std::string(__ch_buffer, __ch_buffer + header_len - 5);
            return _ftype_file.isValid() && (__res_str == "Vocaloid Motion Data file" || __res_str == "Vocaloid Motion Data 0002");
        }

        const std::map<std::string, std::string> ContentVMD::bone_name_mappings
                {
                    // Center
                    {"root",      "\221\123\202\304\202\314\220\145",},
                    {"center",    "\203\132\203\223\203\136\201\133",},
                    {"groove",    "\203\117\203\213\201\133\203\165",},
                    {"back_hips", "\215\230",},
                    // Body
                    {"hips",   "\221\314\202\314\217\144\220\123",},
                    {"spine",  "\217\343\224\274\220\147",},
                    {"spine2", "\217\343\224\274\220\147\62",},
                    {"spine3", "\217\343\224\274\220\147\63",},
                    {"neck",   "\216\361",},
                    {"head",   "\223\252",},
                    {"waist",  "\211\272\224\274\220\147",},
                    // Right Arm
                    {"shoulder_p_r",   "\211\105\214\250\120",},
                    {"shoulder_r",     "\211\105\214\250",},
                    {"shoulder_c_r",   "\211\105\214\250\103",},
                    {"uparm_r",        "\211\105\230\162",},
                    {"uparm_twist_r",  "\211\105\230\162\235\200",},
                    {"lowarm_r",       "\211\105\202\320\202\266",},
                    {"lowarm_twist_r", "\211\105\216\350\235\200",},
                    {"hand_r",         "\211\105\216\350\216\361",},
                    {"hand_dummy_r",   "\211\105\203\137\203\176\201\133",},
                    {"ik_hand_r",      "\211\105\230\162\202\150\202\152",},
                    // Left Arm
                    {"shoulder_p_l",   "\215\266\214\250\120",},
                    {"shoulder_l",     "\215\266\214\250",},
                    {"shoulder_c_l",   "\215\266\214\250\103",},
                    {"uparm_l",        "\215\266\230\162",},
                    {"uparm_twist_l",  "\215\266\230\162\235\200",},
                    {"lowarm_l",       "\215\266\202\320\202\266",},
                    {"lowarm_twist_l", "\215\266\216\350\235\200",},
                    {"hand_l",         "\215\266\216\350\216\361",},
                    {"hand_dummy_l",   "\215\266\203\137\203\176\201\133",},
                    {"ik_hand_l",      "\215\266\230\162\202\150\202\152",},
                    // Right Hand
                    {"f_thumb01_r",  "\211\105\220\145\216\167\202\117",},
                    {"f_thumb02_r",  "\211\105\220\145\216\167\202\120",},
                    {"f_thumb03_r",  "\211\105\220\145\216\167\202\121",},
                    {"f_index01_r",  "\211\105\220\154\216\167\202\120",},
                    {"f_index02_r",  "\211\105\220\154\216\167\202\121",},
                    {"f_index03_r",  "\211\105\220\154\216\167\202\122",},
                    {"f_middle01_r", "\211\105\222\206\216\167\202\120",},
                    {"f_middle02_r", "\211\105\222\206\216\167\202\121",},
                    {"f_middle03_r", "\211\105\222\206\216\167\202\122",},
                    {"f_ring01_r",   "\211\105\226\362\216\167\202\120",},
                    {"f_ring02_r",   "\211\105\226\362\216\167\202\121",},
                    {"f_ring03_r",   "\211\105\226\362\216\167\202\122",},
                    {"f_little01_r", "\211\105\217\254\216\167\202\120",},
                    {"f_little02_r", "\211\105\217\254\216\167\202\121",},
                    {"f_little03_r", "\211\105\217\254\216\167\202\122",},
                    // Left Hand
                    {"f_thumb01_l",  "\215\266\220\145\216\167\202\117",},
                    {"f_thumb02_l",  "\215\266\220\145\216\167\202\120",},
                    {"f_thumb03_l",  "\215\266\220\145\216\167\202\121",},
                    {"f_index01_l",  "\215\266\220\154\216\167\202\120",},
                    {"f_index02_l",  "\215\266\220\154\216\167\202\121",},
                    {"f_index03_l",  "\215\266\220\154\216\167\202\122",},
                    {"f_middle01_l", "\215\266\222\206\216\167\202\120",},
                    {"f_middle02_l", "\215\266\222\206\216\167\202\121",},
                    {"f_middle03_l", "\215\266\222\206\216\167\202\122",},
                    {"f_ring01_l",   "\215\266\226\362\216\167\202\120",},
                    {"f_ring02_l",   "\215\266\226\362\216\167\202\121",},
                    {"f_ring03_l",   "\215\266\226\362\216\167\202\122",},
                    {"f_little01_l", "\215\266\217\254\216\167\202\120",},
                    {"f_little02_l", "\215\266\217\254\216\167\202\121",},
                    {"f_little03_l", "\215\266\217\254\216\167\202\122",},
                    // Right Leg
                    {"back_hips_c_r", "\215\230\203\114\203\203\203\223\203\132\203\213\211\105",},
                    {"upleg_r",       "\211\105\221\253",},
                    {"lowleg_r",      "\211\105\202\320\202\264",},
                    {"foot_r",        "\211\105\221\253\216\361",},
                    {"toe_r",         "\211\105\202\302\202\334\220\346",},
                    {"ik_f_root_r",   "\211\105\221\253\111\113\220\145",},
                    {"ik_foot_r",     "\211\105\221\253\202\150\202\152",},
                    {"ik_toe_r",      "\211\105\202\302\202\334\220\346\202\150\202\152",},
                    {"upleg_d_r",     "\211\105\221\253\104",},
                    {"lowleg_d_r",    "\211\105\202\320\202\264\104",},
                    {"foot_d_r",      "\211\105\221\253\216\361\104",},
                    {"heel_ex_r",     "\211\105\346\371\105\130",},
                    {"toe_ex_r",      "\211\105\221\253\220\346\105\130",},
                    // Left Leg
                    {"back_hips_c_l", "\215\230\203\114\203\203\203\223\203\132\203\213\215\266",},
                    {"upleg_l",       "\215\266\221\253",},
                    {"lowleg_l",      "\215\266\202\320\202\264",},
                    {"foot_l",        "\215\266\221\253\216\361",},
                    {"toe_l",         "\215\266\202\302\202\334\220\346",},
                    {"ik_f_root_l",   "\215\266\221\253\111\113\220\145",},
                    {"ik_foot_l",     "\215\266\221\253\202\150\202\152",},
                    {"ik_toe_l",      "\215\266\202\302\202\334\220\346\202\150\202\152",},
                    {"upleg_d_l",     "\215\266\221\253\104",},
                    {"lowleg_d_l",    "\215\266\202\320\202\264\104",},
                    {"foot_d_l",      "\215\266\221\253\216\361\104",},
                    {"heel_ex_l",     "\215\266\346\371\105\130",},
                    {"toe_ex_l",      "\215\266\221\253\220\346\105\130",},
                    // Eyes
                    {"eyes",  "\227\274\226\332",},
                    {"eye_r", "\211\105\226\332",},
                    {"eye_l", "\215\266\226\332",},
                    // Misc
                    {"mouth_base",   "\214\373\203\170\201\133\203\130",},
                    {"eyebrow_base", "\224\373\203\170\201\133\203\130",},
                };

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
                };

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
                };

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
                };

        void ContentVMD::SetNameFromMapping(BoneKeyframe &_bone_kframe, std::string _mapping_str) {
            for (char &byte : _bone_kframe.bone_name) byte = '\0';

            _mapping_str = bone_name_mappings.at(_mapping_str);
            for (int i = 0; i < _mapping_str.size(); ++i)
                _bone_kframe.bone_name[i] = _mapping_str[i];
        }

        void ContentVMD::ModifyData(const std::string &_vmd_path, bool _fix_blink, bool _make_reflected, bool _reinterpolate) {
            ReaderWriterVMD __vmd_file{};
            if (!__vmd_file.read(_vmd_path)) {
                fprintf(svkfwwarn, "%s '%s'\n", SVKFW_WRAPWARN("File :: ContentVMD :: ModifyData", "could not read VMD file"), _vmd_path.c_str());
                return;
            }
            std::string __vmd_path_no_ext = pathRemoveExtension(_vmd_path), __suffix = "";

            if (_fix_blink) {
                ContentVMD::MorphAttrClampMinmax blink_clamp_info{0.01f, 0.99f};
                __vmd_file.file_content.clampFrames({}, {{"blink", blink_clamp_info}});
                __suffix += "2";
                __vmd_file.write(__vmd_path_no_ext + __suffix + ".vmd");
                printf(SVKFW_WRAPINFO("File :: ContentVMD :: ModifyData", "Saved motion with fixed blink to:\n%s%s.vmd\n"), __vmd_path_no_ext.c_str(), __suffix.c_str());
            }
            if (_make_reflected) {
                __vmd_file.file_content.reflectMovement();
                __suffix += "_reflected";
                __vmd_file.write(__vmd_path_no_ext + __suffix + ".vmd");
                printf(SVKFW_WRAPINFO("File :: ContentVMD :: ModifyData", "Saved reflected motion to:\n%s%s.vmd\n"), __vmd_path_no_ext.c_str(), __suffix.c_str());
            }
            if (_reinterpolate) {
                __vmd_file.file_content.reinterpolateBones();
                __suffix += "_reinterp";
                __vmd_file.write(__vmd_path_no_ext + __suffix + ".vmd");
                printf(SVKFW_WRAPINFO("File :: ContentVMD :: ModifyData", "Saved reinterpolated motion to:\n%s%s.vmd\n"), __vmd_path_no_ext.c_str(), __suffix.c_str());
            }
        }

        void ContentVMD::ModifyCamera(const std::string &_vmd_path, bool _reinterpolate) {
            ReaderWriterVMD __vmd_file{};
            if (!__vmd_file.read(_vmd_path)) {
                fprintf(svkfwwarn, "%s '%s'\n", SVKFW_WRAPWARN("File :: ContentVMD :: ModifyCamera", "could not read VMD file"), _vmd_path.c_str());
                return;
            }
            std::string __vmd_path_no_ext = pathRemoveExtension(_vmd_path), __suffix = "";

            if (_reinterpolate) {
                __vmd_file.file_content.reinterpolateCamera();
                __suffix += "_reinterp";
                __vmd_file.write(__vmd_path_no_ext + __suffix + ".vmd");
                printf(SVKFW_WRAPINFO("File :: ContentVMD :: ModifyCamera", "Saved reinterpolated camera to:\n%s%s.vmd\n"), __vmd_path_no_ext.c_str(), __suffix.c_str());
            }
        }

        void ContentVMD::CreateMixed(const std::string &_motion_vmd_path, const std::string &_camera_vmd_path,
                                     const std::vector<uint32_t> &_split_frame_ids, const AttrAdjust &_motion1_offset,
                                     const AttrAdjust &_camera1_scale, const AttrAdjust &_camera1_offset,
                                     const AttrAdjust &_camera2_scale, const AttrAdjust &_camera2_offset, bool _fix_blink) {
            // Reflected motion
            std::string __motion_vmd_path_no_ext = pathRemoveExtension(_motion_vmd_path);
            std::string __camera_vmd_path_no_ext = pathRemoveExtension(_camera_vmd_path);
            ReaderWriterVMD __vmd_file{};
            if (!__vmd_file.read(__motion_vmd_path_no_ext + ".vmd"))
                throw std::invalid_argument(SVKFW_WRAPERR("ContentVMD :: CreateMixed", "could not read motion VMD file"));

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

            __vmd_file.write(__motion_vmd_path_no_ext + "2.vmd");

            __vmd_file.file_content.reflectMovement();
            __vmd_file.write(__motion_vmd_path_no_ext + "2_reflected.vmd");


            // Reflected camera
            ReaderWriterVMD __vmd_file2{};
            if (!__vmd_file.read(__camera_vmd_path_no_ext + ".vmd") || !__vmd_file2.read(__camera_vmd_path_no_ext + ".vmd"))
                throw std::invalid_argument(SVKFW_WRAPERR("ContentVMD :: CreateMixed", "could not read camera VMD file"));

            if (__vmd_file.file_content.keyframes_camera.empty())
                throw std::invalid_argument(SVKFW_WRAPERR("ContentVMD :: CreateMixed", "camera VMD file has no camera keyframes"));

            __vmd_file .file_content.multKeyframes  ({}, {}, _camera1_scale );
            __vmd_file .file_content.offsetKeyframes({}, {}, _camera1_offset);
            __vmd_file2.file_content.multKeyframes  ({}, {}, _camera2_scale );
            __vmd_file2.file_content.offsetKeyframes({}, {}, _camera2_offset);

            __vmd_file2.file_content.reflectMovement();

            __vmd_file .write(__camera_vmd_path_no_ext + "2.vmd");
            __vmd_file2.write(__camera_vmd_path_no_ext + "2_reflected.vmd");

            // Mixed camera
            __vmd_file.file_content.mixCamera(__vmd_file2.file_content, _split_frame_ids);
            __vmd_file .write(__camera_vmd_path_no_ext + "2_mixed.vmd");
        }

        ReaderWriterOGG::ReaderWriterOGG(const std::string &_fpath) { if (!_fpath.empty()) read(_fpath); }

        bool ReaderWriterOGG::read(const std::string &_fpath) {
            // Open file
            FileReader __ftype_file{_fpath};
            if (!__ftype_file.isOpen()) return false;

            while (__ftype_file.isValid()) {
                ContentOGG::PageOGG __tmp_ogg_page{};

                // Find 'OggS'
                char __oggs[5]{};
                bool __is_complete = __ftype_file.readBinaryArray<char>(__oggs, 4);
                if (!__is_complete && __ftype_file.isEOF()) break; // TODO: something smarter
                SVKFW_WASSERT_CMD(__is_complete, "File::ReaderWriterOGG :: read", "Page " + std::to_string(file_content.pages.size()) + " - Error while beginning to read OGG page\n", break);

                while (__ftype_file.isValid() && std::strcmp(__oggs, "OggS")) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File::ReaderWriterOGG :: read", "Page %u - OggS capture position mismatch!\n"), unsigned(file_content.pages.size()));
                    __oggs[0] = __oggs[1];
                    __oggs[1] = __oggs[2];
                    __oggs[2] = __oggs[3];
                    __oggs[3] = __ftype_file.readBinary<char>();
                }
                SVKFW_WASSERT_CMD(__ftype_file.isValid(), "File::ReaderWriterOGG :: read", "Error while finding 'OggS' capture\n", break);
                std::memcpy(__tmp_ogg_page.signature, __oggs, 4);

                // Page found, read rest of the header
                __tmp_ogg_page.version          = __ftype_file.readBinary<uint8_t >();
                __tmp_ogg_page.header_flags     = __ftype_file.readBinary<uint8_t >();
                __tmp_ogg_page.granule_position = __ftype_file.readBinary<uint64_t>();
                __tmp_ogg_page.serial_number    = __ftype_file.readBinary<uint32_t>();
                __tmp_ogg_page.sequence_number  = __ftype_file.readBinary<uint32_t>();
                __tmp_ogg_page.checksum         = __ftype_file.readBinary<uint32_t>();
                __tmp_ogg_page.total_segments   = __ftype_file.readBinary<uint8_t >();
                SVKFW_WASSERT_CMD(__ftype_file.isValid(), "File::ReaderWriterOGG :: read", "Error reading OGG page header\n", break);
                SVKFW_WASSERT_CMD(__ftype_file.readBinaryArray<uint8_t>(__tmp_ogg_page.segment_table, __tmp_ogg_page.total_segments), "File::ReaderWriterOGG :: read", "Error reading segment table", break);

                // Calculate data size, read it and save the page
                // printf(SVKFW_WRAPINFO("OGG Reader : Read", "Page %d - %d segments: "), file_content.pages.size(), __tmp_ogg_page.total_segments);
                uint32_t __page_data_size = 0u;
                for (uint32_t i = 0u; i < __tmp_ogg_page.total_segments; ++i) {
                    __page_data_size += __tmp_ogg_page.segment_table[i];
                    // printf("%d, ", __tmp_ogg_page.segment_table[i]);
                }
                // printf("\n");

                __tmp_ogg_page.segment_data.resize(__page_data_size, 0u);
                SVKFW_WASSERT_CMD(__ftype_file.readBinaryArray<uint8_t>(__tmp_ogg_page.segment_data.data(), __page_data_size), "File::ReaderWriterOGG :: read", "Error reading segment data\n", break);
                file_content.pages.emplace_back(std::move(__tmp_ogg_page));
            }

            printf(SVKFW_WRAPINFO("OGG Reader : Read", "Read END, %u OGG pages\n"), unsigned(file_content.pages.size()));
            return true;
        }

        bool ReaderWriterOGG::write(const std::string &_fpath) {
            // Open file (existing file is truncated)
            FileWriter __ftype_file{_fpath, true};
            if (!__ftype_file.isOpen()) return false;
            file_content.flushPage();

            for (const auto& page : file_content.pages) {
                __ftype_file.writeBinaryArray(page.signature, 4);
                __ftype_file.writeBinary(page.version         );
                __ftype_file.writeBinary(page.header_flags    );
                __ftype_file.writeBinary(page.granule_position);
                __ftype_file.writeBinary(page.serial_number   );
                __ftype_file.writeBinary(page.sequence_number );
                __ftype_file.writeBinary(page.checksum        );
                __ftype_file.writeBinary(page.total_segments  );
                __ftype_file.writeBinaryArray(page.segment_table, page.total_segments);
                __ftype_file.writeBinaryArray(page.segment_data.data(), page.segment_data.size());
            }

            return __ftype_file.isValid();
        }

        bool ReaderWriterOGG::tryHeaders(ReaderItf &_ftype_file) {
            const uint32_t header_len = 4;
            char __ch_buffer[5]{};

            _ftype_file.read(__ch_buffer, header_len);
            std::string __res_str = std::string(__ch_buffer, __ch_buffer + header_len);
            return _ftype_file.isValid() && __res_str == "OggS";
        }

        uint32_t ContentOGG::Checksum(const PageOGG &_page) {
            static uint32_t __table[256]{};
            static bool __table_ready = false;
            if (!__table_ready) {
                for (uint32_t i = 0u; i < 256u; ++i) {
                    uint32_t __crc = i << 24u;
                    for (uint32_t b = 0u; b < 8u; ++b)
                        __crc = (__crc & 0x80000000u) ? (__crc << 1u) ^ 0x04C11DB7u : (__crc << 1u);
                    __table[i] = __crc;
                }
                __table_ready = true;
            }

            uint8_t __header[27]{};
            std::memcpy(__header, _page.signature, 4);
            __header[4] = _page.version;
            __header[5] = _page.header_flags;
            for (uint32_t i = 0u; i < 8u; ++i) __header[ 6u + i] = uint8_t(_page.granule_position >> (8u * i));
            for (uint32_t i = 0u; i < 4u; ++i) __header[14u + i] = uint8_t(_page.serial_number    >> (8u * i));
            for (uint32_t i = 0u; i < 4u; ++i) __header[18u + i] = uint8_t(_page.sequence_number  >> (8u * i));
            // Bytes 22-25 (checksum) stay zero
            __header[26] = _page.total_segments;

            uint32_t __crc = 0u;
            auto __update = [&__crc](const uint8_t *_bytes, size_t _len) {
                for (size_t i = 0u; i < _len; ++i)
                    __crc = (__crc << 8u) ^ __table[((__crc >> 24u) ^ _bytes[i]) & 0xFFu];
            };
            __update(__header, 27u);
            __update(_page.segment_table, _page.total_segments);
            __update(_page.segment_data.data(), _page.segment_data.size());
            return __crc;
        }

        void ContentOGG::addPacket(uint32_t _serial, const std::vector<unsigned char> &_packet,
                                   uint64_t _granule_position, bool _flush, bool _end_of_stream) {
            // Max page data in bytes before the page is closed (libogg uses about 4 KB too)
            const size_t __page_fill = 4096u;

            // Lacing values: 255 for every full segment, the last one is < 255 (0 if the size is a multiple of 255)
            const size_t __num_lacing = _packet.size() / 255u + 1u;
            size_t __offset = 0u;
            for (size_t i = 0u; i < __num_lacing; ++i) {
                if (!page_open || pages.back().serial_number != _serial || pages.back().total_segments == 255u) {
                    if (page_open) flushPage();

                    // New page, its sequence number continues the last page of this stream
                    uint32_t __sequence = 0u;
                    bool __first_page = true;
                    for (size_t p = pages.size(); p-- > 0u;)
                        if (pages[p].serial_number == _serial) { __sequence = pages[p].sequence_number + 1u; __first_page = false; break; }

                    PageOGG __page{};
                    std::memcpy(__page.signature, "OggS", 4);
                    __page.header_flags     = uint8_t((i > 0u ? 0x01u : 0u) | (__first_page ? 0x02u : 0u));
                    __page.granule_position = UINT64_MAX; // No packet ends on this page yet
                    __page.serial_number    = _serial;
                    __page.sequence_number  = __sequence;
                    pages.emplace_back(std::move(__page));
                    page_open = true;
                }
                PageOGG &__page = pages.back();
                const size_t __segment = std::min<size_t>(255u, _packet.size() - __offset);
                __page.segment_table[__page.total_segments++] = uint8_t(__segment);
                __page.segment_data.insert(__page.segment_data.end(), _packet.begin() + __offset, _packet.begin() + __offset + __segment);
                __offset += __segment;
            }

            PageOGG &__page = pages.back();
            __page.granule_position = _granule_position;
            if (_end_of_stream) __page.header_flags |= 0x04u;
            if (_flush || _end_of_stream || __page.total_segments == 255u || __page.segment_data.size() >= __page_fill)
                flushPage();
        }

        void ContentOGG::flushPage() {
            if (!page_open || pages.empty()) return;
            pages.back().checksum = Checksum(pages.back());
            page_open = false;
        }

        // WAV

        WAVCodecItf::~WAVCodecItf() {}

        uint16_t WAVCodecPCM::formatTag() const { return WAV_FORMAT_PCM; }

        bool WAVCodecPCM::supportsBits(uint16_t _bits_per_sample) const {
            return _bits_per_sample == 8u || _bits_per_sample == 16u || _bits_per_sample == 24u || _bits_per_sample == 32u;
        }

        uint16_t WAVCodecPCM::blockAlign(uint16_t _channels, uint16_t _bits_per_sample) const {
            return uint16_t(_channels * (_bits_per_sample / 8u));
        }

        bool WAVCodecPCM::encode(const float *_samples, size_t _num_samples, uint16_t _bits_per_sample, std::vector<uint8_t> &_out_data) const {
            if (!supportsBits(_bits_per_sample)) return false;
            const uint32_t __bytes = _bits_per_sample / 8u;
            const double   __scale = double((uint64_t(1u) << (_bits_per_sample - 1u)) - 1u);

            _out_data.resize(_num_samples * __bytes);
            uint8_t *__dst = _out_data.data();
            for (size_t i = 0u; i < _num_samples; ++i, __dst += __bytes) {
                const double __sample = std::min(std::max(double(_samples[i]), -1.), 1.) * __scale;
                if (__bytes == 1u) { *__dst = uint8_t(128 + int32_t(__sample)); continue; } // 8 bit is unsigned
                const uint32_t __word = uint32_t(int32_t(__sample));
                for (uint32_t b = 0u; b < __bytes; ++b)
                    __dst[b] = uint8_t(__word >> (8u * b));
            }
            return true;
        }

        bool WAVCodecPCM::decode(const uint8_t *_data, size_t _num_bytes, uint16_t _bits_per_sample, std::vector<float> &_out_samples) const {
            if (!supportsBits(_bits_per_sample)) return false;
            const uint32_t __bytes = _bits_per_sample / 8u;
            const double   __scale = 1. / double(uint64_t(1u) << (_bits_per_sample - 1u));

            _out_samples.resize(_num_bytes / __bytes);
            for (size_t i = 0u; i < _out_samples.size(); ++i, _data += __bytes) {
                if (__bytes == 1u) { _out_samples[i] = float((int32_t(*_data) - 128) * __scale); continue; }
                uint32_t __word = 0u;
                for (uint32_t b = 0u; b < __bytes; ++b)
                    __word |= uint32_t(_data[b]) << (8u * b);
                __word <<= 32u - _bits_per_sample; // Sign extension through the top bit
                _out_samples[i] = float(double(int32_t(__word) >> (32u - _bits_per_sample)) * __scale);
            }
            return true;
        }

        static std::map<uint16_t, const WAVCodecItf*>& WAVCodecs_() {
            static WAVCodecPCM __pcm;
            static std::map<uint16_t, const WAVCodecItf*> __codecs{ {__pcm.formatTag(), &__pcm} };
            return __codecs;
        }

        void ContentWAV::RegisterCodec(const WAVCodecItf *_codec) {
            if (_codec != nullptr) WAVCodecs_()[_codec->formatTag()] = _codec;
        }

        const WAVCodecItf* ContentWAV::FindCodec(uint16_t _format_tag) {
            auto __it = WAVCodecs_().find(_format_tag);
            return __it == WAVCodecs_().end() ? nullptr : __it->second;
        }

        uint16_t ContentWAV::blockAlign() const {
            const WAVCodecItf *__codec = FindCodec(format_tag);
            return __codec != nullptr ? __codec->blockAlign(channels, bits_per_sample) : uint16_t(channels * ((bits_per_sample + 7u) / 8u));
        }

        uint32_t ContentWAV::numFrames() const {
            const uint16_t __align = blockAlign();
            return __align ? uint32_t(data.size() / __align) : 0u;
        }

        bool ContentWAV::setSamples(const float *_samples, size_t _num_frames) {
            const WAVCodecItf *__codec = FindCodec(format_tag);
            if (__codec == nullptr || !__codec->supportsBits(bits_per_sample) || channels == 0u) {
                fprintf(svkfwwarn, SVKFW_WRAPWARN("File::ContentWAV :: setSamples", "No codec for format 0x%04X, %u bits\n"), unsigned(format_tag), unsigned(bits_per_sample));
                return false;
            }
            return __codec->encode(_samples, _num_frames * channels, bits_per_sample, data);
        }

        bool ContentWAV::setSamples(const std::vector<vec2f> &_frames) {
            channels = 2u;
            std::vector<float> __samples(_frames.size() * 2u);
            for (size_t i = 0u; i < _frames.size(); ++i) {
                __samples[2u * i     ] = _frames[i].x;
                __samples[2u * i + 1u] = _frames[i].y;
            }
            return setSamples(__samples.data(), _frames.size());
        }

        bool ContentWAV::getSamples(std::vector<float> &_out_samples) const {
            const WAVCodecItf *__codec = FindCodec(format_tag);
            if (__codec == nullptr || !__codec->supportsBits(bits_per_sample)) {
                fprintf(svkfwwarn, SVKFW_WRAPWARN("File::ContentWAV :: getSamples", "No codec for format 0x%04X, %u bits\n"), unsigned(format_tag), unsigned(bits_per_sample));
                return false;
            }
            return __codec->decode(data.data(), size_t(numFrames()) * blockAlign(), bits_per_sample, _out_samples);
        }

        ReaderWriterWAV::ReaderWriterWAV(const std::string &_fpath) { if (!_fpath.empty()) read(_fpath); }

        bool ReaderWriterWAV::read(const std::string &_fpath) {
            FileReader __ftype_file{_fpath};
            if (!__ftype_file.isOpen()) return false;
            SVKFW_WASSERT_CMD(tryHeaders(__ftype_file), "File::ReaderWriterWAV :: read", "'" + _fpath + "' is not a RIFF WAVE file\n", return false);

            bool __has_fmt = false, __has_data = false;
            while (__ftype_file.isValid() && !(__has_fmt && __has_data)) {
                char __chunk_id[4]{};
                uint32_t __chunk_size = 0u;
                if (!__ftype_file.readBinaryArray<char>(__chunk_id, 4) || !__ftype_file.readBinary(__chunk_size)) break;
                const std::string __id(__chunk_id, __chunk_id + 4);

                std::vector<uint8_t> __chunk(__chunk_size);
                SVKFW_WASSERT_CMD(__ftype_file.readBinaryArray<uint8_t>(__chunk.data(), __chunk_size), "File::ReaderWriterWAV :: read",
                                  "Chunk '" + __id + "' is cut off\n", break);
                if (__chunk_size & 1u) __ftype_file.readBinary<uint8_t>(); // Chunks are padded to even sizes

                if (__id == "fmt ") {
                    SVKFW_WASSERT_CMD(__chunk_size >= 16u, "File::ReaderWriterWAV :: read", "'fmt ' chunk is too small\n", return false);
                    auto __u16 = [&__chunk](uint32_t _at) { return uint16_t(__chunk[_at] | (__chunk[_at + 1u] << 8u)); };
                    file_content.format_tag      = __u16(0u);
                    file_content.channels        = __u16(2u);
                    file_content.sample_rate     = uint32_t(__u16(4u)) | (uint32_t(__u16(6u)) << 16u);
                    file_content.bits_per_sample = __u16(14u);
                    // WAVE_FORMAT_EXTENSIBLE: cbSize, valid bits, channel mask, sub-format GUID (starts with the real tag)
                    if (file_content.format_tag == WAV_FORMAT_EXTENSIBLE && __chunk_size >= 40u)
                        file_content.format_tag = __u16(24u);
                    __has_fmt = true;
                }
                else if (__id == "data") {
                    file_content.data = std::move(__chunk);
                    __has_data = true;
                }
            }

            SVKFW_WASSERT_CMD(__has_fmt && __has_data, "File::ReaderWriterWAV :: read", "'" + _fpath + "' has no 'fmt ' or 'data' chunk\n", return false);
            if (ContentWAV::FindCodec(file_content.format_tag) == nullptr)
                fprintf(svkfwwarn, SVKFW_WRAPWARN("File::ReaderWriterWAV :: read", "No codec for format 0x%04X, samples can't be decoded\n"), unsigned(file_content.format_tag));
            return true;
        }

        bool ReaderWriterWAV::write(const std::string &_fpath) {
            const ContentWAV &__wav = file_content;
            const uint16_t __align = __wav.blockAlign();
            const size_t __data_size = size_t(__wav.numFrames()) * __align;
            SVKFW_WASSERT_CMD(__data_size + 80u < size_t(UINT32_MAX), "File::ReaderWriterWAV :: write", "Data doesn't fit into a RIFF file\n", return false);

            // WAVE_FORMAT_EXTENSIBLE is required above 2 channels or 16 bits (except float)
            const bool __extensible = __wav.channels > 2u || (__wav.bits_per_sample > 16u && __wav.format_tag != WAV_FORMAT_IEEE_FLOAT);
            const uint32_t __fmt_size = __extensible ? 40u : 16u;

            FileWriter __ftype_file{_fpath, true};
            if (!__ftype_file.isOpen()) return false;

            __ftype_file.writeBinaryArray("RIFF", 4);
            __ftype_file.writeBinary(uint32_t(4u + 8u + __fmt_size + 8u + __data_size + (__data_size & 1u)));
            __ftype_file.writeBinaryArray("WAVE", 4);

            __ftype_file.writeBinaryArray("fmt ", 4);
            __ftype_file.writeBinary(__fmt_size);
            __ftype_file.writeBinary(uint16_t(__extensible ? WAV_FORMAT_EXTENSIBLE : __wav.format_tag));
            __ftype_file.writeBinary(__wav.channels);
            __ftype_file.writeBinary(__wav.sample_rate);
            __ftype_file.writeBinary(uint32_t(__wav.sample_rate * __align)); // Bytes per second
            __ftype_file.writeBinary(__align);
            __ftype_file.writeBinary(__wav.bits_per_sample);
            if (__extensible) {
                static const uint8_t __guid_tail[14] = { 0x00u, 0x00u, 0x00u, 0x00u, 0x10u, 0x00u, 0x80u, 0x00u, 0x00u, 0xAAu, 0x00u, 0x38u, 0x9Bu, 0x71u };
                __ftype_file.writeBinary(uint16_t(22u));               // cbSize
                __ftype_file.writeBinary(__wav.bits_per_sample);       // Valid bits per sample
                __ftype_file.writeBinary(uint32_t(0u));                // Channel mask: no speaker positions
                __ftype_file.writeBinary(uint16_t(__wav.format_tag)); // Sub-format GUID
                __ftype_file.writeBinaryArray(__guid_tail, 14);
            }

            __ftype_file.writeBinaryArray("data", 4);
            __ftype_file.writeBinary(uint32_t(__data_size));
            __ftype_file.writeBinaryArray(__wav.data.data(), __data_size);
            if (__data_size & 1u) __ftype_file.writeBinary(uint8_t(0u));

            return __ftype_file.isValid();
        }

        bool ReaderWriterWAV::tryHeaders(ReaderItf &_ftype_file) {
            char __header[12]{};
            _ftype_file.read(__header, 12);
            return _ftype_file.isValid() && !std::memcmp(__header, "RIFF", 4) && !std::memcmp(__header + 8, "WAVE", 4);
        }


        namespace Util {

            std::pair<uint32_t, uint32_t> parseWords(const std::vector<std::string> &_words, const char *_json_str) {
                uint32_t __res_index = UINT32_MAX, __res_offset = UINT32_MAX;

                for (uint32_t i = 0u; i < _words.size(); ++i) {
                    bool __matches = false;
                    for (uint32_t j = 0u; j < _words[i].size(); ++j) {
                        if (_json_str[j] == '\0' || _json_str[j] != _words[i][j])
                            break;
                        __matches = j+1 == _words[i].size();
                    }
                    if (__matches) { __res_index = i; __res_offset = _words[i].size(); }
                }

                return { __res_index, __res_offset };
            }

            uint32_t skipWhitespace(const char *_json_str) {
                uint32_t __res_offset = 0u;
                if (_json_str != nullptr)
                    while (std::isspace((unsigned char)_json_str[__res_offset]) && _json_str[__res_offset] != '\0')
                        __res_offset += 1;
                return __res_offset;
            }
        }; // Util END


        ContentJSON::JSONNode::JSONNode() {}

        ContentJSON::JSONNode::JSONNode(const JSONNode &_node) : node_type{_node.node_type} {
            switch (node_type) {
                case JSON_TYPE_ARR :
                case JSON_TYPE_OBJ : new (&value_arr_obj) std::vector<JSONNode>;
                                        value_arr_obj = _node.value_arr_obj; break;
                case JSON_TYPE_STR : new (&value_str) std::string;
                                        value_str  = _node.value_str;  break;
                case JSON_TYPE_BOOL: value_bool = _node.value_bool; break;
                case JSON_TYPE_NUM : value_num  = _node.value_num ; break;
                case JSON_TYPE_NULL: break;
            }
        }

        ContentJSON::JSONNode::~JSONNode() { removeValue(); }

        const ContentJSON::JSONNode& ContentJSON::JSONNode::operator=(const JSONNode& _node) {
            if (this == &_node) return *this;
            removeValue();
            node_type = _node.node_type; // Note: 'removeValue()' resets the type, so it must be set after it

            switch (node_type) {
                case JSON_TYPE_ARR :
                case JSON_TYPE_OBJ : new (&value_arr_obj) std::vector<JSONNode>;
                                        value_arr_obj = _node.value_arr_obj; break;
                case JSON_TYPE_STR : new (&value_str) std::string;
                                        value_str  = _node.value_str;  break;
                case JSON_TYPE_BOOL: value_bool = _node.value_bool; break;
                case JSON_TYPE_NUM : value_num  = _node.value_num ; break;
                case JSON_TYPE_NULL: break;
            }
            return *this;
        }

        void ContentJSON::JSONNode::removeValue() {
            switch (node_type) {
                case JSON_TYPE_ARR :
                case JSON_TYPE_OBJ :   value_arr_obj.~vector(); break;
                case JSON_TYPE_STR : value_str.~basic_string(); break;
                default: break;
            }
            node_type = JSON_TYPE_NULL;
        }

        void ContentJSON::JSONNode::switchValueNull() {
            removeValue();
            node_type = JSON_TYPE_NULL;
        }

        void ContentJSON::JSONNode::switchValueBool() {
            removeValue();
            node_type = JSON_TYPE_BOOL;
            value_bool = false;
        }

        void ContentJSON::JSONNode::switchValueNum () {
            removeValue();
            node_type = JSON_TYPE_NUM;
            value_num = 0.0;
        }

        void ContentJSON::JSONNode::switchValueStr () {
            removeValue();
            node_type = JSON_TYPE_STR;
            new (&value_str) std::string;
        }

        void ContentJSON::JSONNode::switchValueArr () {
            removeValue();
            node_type = JSON_TYPE_ARR;
            new (&value_arr_obj) std::vector<JSONNode>;
        }

        void ContentJSON::JSONNode::switchValueObj () {
            removeValue();
            node_type = JSON_TYPE_OBJ;
            new (&value_arr_obj) std::vector<JSONNode>;
        }

        void ContentJSON::JSONNode::setValueNull() {
            switchValueNull();
        }

        void ContentJSON::JSONNode::setValueBool(bool _value) {
            switchValueBool();
            value_bool = _value;
        }

        void ContentJSON::JSONNode::setValueStr(const std::string &_value) {
            switchValueStr();
            value_str = _value;
        }

        void ContentJSON::JSONNode::setValueNum(double _value) {
            switchValueNum();
            value_num = _value;
        }

        void ContentJSON::JSONNode::setValueArr(const std::vector<JSONNode> &_value) {
            switchValueArr();
            value_arr_obj = _value;
        }

        void ContentJSON::JSONNode::setValueObj(const JSONNode &_key, const JSONNode &_value) {
            switchValueObj();
            value_arr_obj = {_key, _value};
        }

        uint32_t ContentJSON::JSONNode::readValueNull(const char *_json_str) {
            std::pair<uint32_t,uint32_t> __index_offset = Util::parseWords({"null"}, _json_str);
            setValueNull();
            return __index_offset.second;
        }

        uint32_t ContentJSON::JSONNode::readValueBool(const char *_json_str) {
            std::pair<uint32_t,uint32_t> __index_offset = Util::parseWords({"false","true"}, _json_str);
            setValueBool(__index_offset.first);
            return __index_offset.second;
        }

        uint32_t ContentJSON::JSONNode::readValueNum (const char *_json_str) {
            std::size_t __res_offset = UINT32_MAX;
            double __value = 0.0;
            try {
                __value = std::stod(_json_str, &__res_offset);
            }
            catch (const std::exception &) { __res_offset = UINT32_MAX; }
            setValueNum(__value);
            return (uint32_t) __res_offset;

        }

        uint32_t ContentJSON::JSONNode::readValueStr (const char *_json_str) {
            bool __finish = _json_str == nullptr || _json_str[0] != '"';
            uint32_t __res_offset = UINT32_MAX;
            std::string __res_str;

            for (uint32_t i = 1u; !__finish; ++i) {
                switch (_json_str[i]) {
                    case  '"': __res_offset = i+1; __finish = true; break;
                    case '\0':                     __finish = true; break;
                    case '\\': {
                        ++i;
                        switch (_json_str[i]) {
                            case 'b': __res_str += '\b'; break;
                            case 'f': __res_str += '\f'; break;
                            case 'n': __res_str += '\n'; break;
                            case 'r': __res_str += '\r'; break;
                            case 't': __res_str += '\t'; break;
                            case '\0': __finish = true;  break;
                            // TODO: '\uXXXX' sequences are kept as is
                            case 'u': __res_str += "\\u"; break;
                            default : __res_str += _json_str[i]; break; // '"', '\\', '/'
                        }
                        break;
                    }
                    default: __res_str += _json_str[i]; break;
                }
            }
            if (__res_offset != UINT32_MAX)
                setValueStr(__res_str);
            return __res_offset;
        }

        uint32_t ContentJSON::JSONNode::readValueArr (const char *_json_str) {
            uint32_t __res_offset = UINT32_MAX;

            bool __finish = _json_str == nullptr || _json_str[0] != '[';
            uint32_t __tmp_offset = 0u, __sum_offset = __finish ? UINT32_MAX : 1u;
            switchValueArr();
            // Empty: '[' and ']' with only whitespace between
            if (!__finish) {
                __tmp_offset = Util::skipWhitespace(_json_str+__sum_offset);
                if (_json_str[__sum_offset+__tmp_offset] == ']')
                    return __sum_offset + __tmp_offset + 1u;
            }
            while (!__finish) {
                value_arr_obj.emplace_back();
                __tmp_offset = value_arr_obj.back().readValue(_json_str+__sum_offset);
                if (__tmp_offset == UINT32_MAX) {
                    std::string __warn_msg = SVKFW_WRAPWARN("ContentJSON :: JSONNode :: readValueArr", "couldn't read JSON object in array\n");
                    fprintf(svkfwwarn, "%s\n", __warn_msg.c_str());
                    break;
                }
                __sum_offset += __tmp_offset;

                // Finish state
                if (_json_str[__sum_offset] == ']') {
                    __finish = _json_str[__sum_offset] == ']';
                    __sum_offset += 1;
                }
                else if (_json_str[__sum_offset] == ',') {
                    __sum_offset += 1;
                    // Processing trailing comma
                    __tmp_offset = Util::skipWhitespace(_json_str+__sum_offset);
                    __sum_offset += __tmp_offset;

                    // Finish state
                    if (_json_str[__sum_offset] == ']') {
                        __finish = _json_str[__sum_offset] == ']';
                        __sum_offset += 1;
                    }
                }
                else {
                    std::string __warn_msg = SVKFW_WRAPWARN("ContentJSON :: JSONNode :: readValueArr", "unexpected symbol: " + std::to_string(_json_str[__sum_offset]));
                    fprintf(svkfwwarn, "%s\n", __warn_msg.c_str());
                    break;
                }
            }
            if (__finish) __res_offset = __sum_offset;
            return __res_offset;
        }

        uint32_t ContentJSON::JSONNode::readValueObj (const char *_json_str) {
            uint32_t __res_offset = UINT32_MAX;

            bool __finish = _json_str == nullptr || _json_str[0] != '{';
            uint32_t __tmp_offset = 0u, __sum_offset = __finish ? UINT32_MAX : 1u;
            switchValueObj();
            // Empty: '{' and '}' with only whitespace between
            if (!__finish) {
                __tmp_offset = Util::skipWhitespace(_json_str+__sum_offset);
                if (_json_str[__sum_offset+__tmp_offset] == '}')
                    return __sum_offset + __tmp_offset + 1u;
            }
            while (!__finish) {
                // Reading string
                value_arr_obj.emplace_back();
                __tmp_offset = value_arr_obj.back().readValue(_json_str+__sum_offset);
                if (__tmp_offset == UINT32_MAX) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("ContentJSON :: JSONNode :: readValueObj", "couldn't read first JSON object (string) in object\n"));
                    break;
                }
                __sum_offset += __tmp_offset;

                // Reading ':'
                if (_json_str[__sum_offset] != ':') {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("ContentJSON :: JSONNode :: readValueObj", "unexpected symbol instead of ':' - '%c'\n"), _json_str[__sum_offset]);
                    break;
                }
                __sum_offset += 1;

                // Reading object
                value_arr_obj.emplace_back();
                __tmp_offset = value_arr_obj.back().readValue(_json_str+__sum_offset);
                if (__tmp_offset == UINT32_MAX) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("ContentJSON :: JSONNode :: readValueObj", "couldn't read second JSON object in object\n"));
                    break;
                }
                __sum_offset += __tmp_offset;

                // Finish state
                if (_json_str[__sum_offset] == '}') {
                    __finish = _json_str[__sum_offset] == '}';
                    __sum_offset += 1;
                }
                else if (_json_str[__sum_offset] == ',') {
                    __sum_offset += 1;
                    // Processing trailing comma
                    __tmp_offset = Util::skipWhitespace(_json_str+__sum_offset);
                    __sum_offset += __tmp_offset;

                    // Finish state
                    if (_json_str[__sum_offset] == '}') {
                        __finish = _json_str[__sum_offset] == '}';
                        __sum_offset += 1;
                    }
                }
                else {
                    std::string __warn_msg = SVKFW_WRAPWARN("ContentJSON :: JSONNode :: readValueObj", "unexpected symbol: " + std::to_string(_json_str[__sum_offset]));
                    fprintf(svkfwwarn, "%s", __warn_msg.c_str());
                    break;
                }
            }
            if (__finish) __res_offset = __sum_offset;
            return __res_offset;
        }

        uint32_t ContentJSON::JSONNode::readValue(const char *_json_str) {
            uint32_t __res_offset = 0u;
            if (_json_str == nullptr) return UINT32_MAX;

            uint32_t __tmp_offset = Util::skipWhitespace(_json_str);
            __res_offset += __tmp_offset;

            switch (_json_str[__res_offset]) {
                case 't':
                case 'f': __tmp_offset = readValueBool(_json_str + __res_offset); break;
                case 'n': __tmp_offset = readValueNull(_json_str + __res_offset); break;
                case '"': __tmp_offset = readValueStr (_json_str + __res_offset); break;
                case '[': __tmp_offset = readValueArr (_json_str + __res_offset); break;
                case '{': __tmp_offset = readValueObj (_json_str + __res_offset); break;
                default : __tmp_offset = readValueNum (_json_str + __res_offset); break;
            }
            if (__tmp_offset == UINT32_MAX) return UINT32_MAX;
            __res_offset += __tmp_offset;

            __tmp_offset = Util::skipWhitespace(_json_str+__res_offset);
            __res_offset += __tmp_offset;
            return __res_offset;
        }

        std::vector<std::string> ContentJSON::JSONNode::writeValueNull(uint32_t _indent) {
            return {"null"};
        }

        std::vector<std::string> ContentJSON::JSONNode::writeValueBool(uint32_t _indent) {
            return { value_bool ? "true" : "false" };
        }

        std::vector<std::string> ContentJSON::JSONNode::writeValueNum (uint32_t _indent) {
            // Note: shortest of "%.15g"/"%.17g" that keeps the value, integers are written without fractional part
            char __buf[32]{};
            snprintf(__buf, sizeof(__buf), "%.15g", value_num);
            if (std::strtod(__buf, nullptr) != value_num)
                snprintf(__buf, sizeof(__buf), "%.17g", value_num);
            return { std::string(__buf) };
        }

        std::vector<std::string> ContentJSON::JSONNode::writeValueStr (uint32_t _indent) {
            std::string __res{'"'};
            for (char ch : value_str) {
                switch (ch) {
                    case  '"': __res += "\\\""; break;
                    case '\\': __res += "\\\\"; break;
                    case '\b': __res += "\\b";  break;
                    case '\f': __res += "\\f";  break;
                    case '\n': __res += "\\n";  break;
                    case '\r': __res += "\\r";  break;
                    case '\t': __res += "\\t";  break;
                    default  : __res += ch;     break;
                }
            }
            return { __res + '"' };
        }

        std::vector<std::string> ContentJSON::JSONNode::writeValueArr (uint32_t _indent) {
            const std::string __indent_str = std::string(size_t(_indent), ' ');
            std::vector<std::string> __res, __tmp_res;
            if (value_arr_obj.empty()) _indent = 0u;

            __res.push_back("[");
            for (uint32_t i = 0u; i < value_arr_obj.size(); ++i) {
                __tmp_res = value_arr_obj[i].writeValue(_indent);
                for (auto& ln : __tmp_res)
                    __res.push_back(__indent_str + ln);
                if (i+1 < value_arr_obj.size())
                    __res.back() += ",";
            }
            __res.push_back("]");

            if (_indent == 0) {
                for (uint32_t i = 1u; i < __res.size(); ++i)
                    __res[0] += ' ' + __res[i];
                __res.resize(1);
            }

            return __res;
        }

        std::vector<std::string> ContentJSON::JSONNode::writeValueObj (uint32_t _indent) {
            const std::string __indent_str = std::string(size_t(_indent), ' ');
            std::vector<std::string> __res, __tmp_res;
            if (value_arr_obj.empty()) _indent = 0u;

            __res.push_back("{");
            for (uint32_t i = 0u; i < value_arr_obj.size(); i+=2) {
                __tmp_res = value_arr_obj[i].writeValue(_indent);
                for (auto& ln : __tmp_res)
                    __res.push_back(__indent_str + ln);
                __res.back() += " : ";

                __tmp_res = value_arr_obj[i+1].writeValue(_indent);
                __res.back() += __tmp_res[0];
                for (uint32_t j = 1u; j < __tmp_res.size(); ++j)
                    __res.push_back(__indent_str + __tmp_res[j]);
                if (i+2 < value_arr_obj.size())
                    __res.back() += ",";
            }
            __res.push_back("}");

            if (_indent == 0) {
                for (uint32_t i = 1u; i < __res.size(); ++i)
                    __res[0] += ' ' + __res[i];
                __res.resize(1);
            }

            return __res;
        }

        std::vector<std::string> ContentJSON::JSONNode::writeValue(uint32_t _indent) {
            switch (node_type) {
                case JSON_TYPE_NULL: return writeValueNull(_indent);
                case JSON_TYPE_BOOL: return writeValueBool(_indent);
                case JSON_TYPE_NUM : return writeValueNum (_indent);
                case JSON_TYPE_STR : return writeValueStr (_indent);
                case JSON_TYPE_ARR : return writeValueArr (_indent);
                case JSON_TYPE_OBJ : return writeValueObj (_indent);
            }
            return {};
        }

        const ContentJSON::JSONNode* ContentJSON::JSONNode::find(const std::string &_key) const {
            if (node_type != JSON_TYPE_OBJ) return nullptr;
            // Note: objects keep their keys and values in turn
            for (size_t i = 0u; i + 1u < value_arr_obj.size(); i += 2u)
                if (value_arr_obj[i].node_type == JSON_TYPE_STR && value_arr_obj[i].value_str == _key)
                    return &value_arr_obj[i + 1u];
            return nullptr;
        }

        ContentJSON::JSONNode& ContentJSON::JSONNode::addMember(const std::string &_key) {
            if (node_type != JSON_TYPE_OBJ) switchValueObj();
            value_arr_obj.push_back(JSONNode{});
            value_arr_obj.back().setValueStr(_key);
            value_arr_obj.push_back(JSONNode{});
            return value_arr_obj.back();
        }

        ContentJSON::JSONNode& ContentJSON::JSONNode::pushBack() {
            if (node_type != JSON_TYPE_ARR) switchValueArr();
            value_arr_obj.push_back(JSONNode{});
            return value_arr_obj.back();
        }

        double ContentJSON::JSONNode::getNum(const std::string &_key, double _default) const {
            const JSONNode *__node = find(_key);
            return (__node != nullptr && __node->node_type == JSON_TYPE_NUM) ? __node->value_num : _default;
        }

        bool ContentJSON::JSONNode::getBool(const std::string &_key, bool _default) const {
            const JSONNode *__node = find(_key);
            return (__node != nullptr && __node->node_type == JSON_TYPE_BOOL) ? __node->value_bool : _default;
        }

        std::string ContentJSON::JSONNode::getStr(const std::string &_key, const std::string &_default) const {
            const JSONNode *__node = find(_key);
            return (__node != nullptr && __node->node_type == JSON_TYPE_STR) ? __node->value_str : _default;
        }

        ContentJSON::ContentJSON() {}

        ContentJSON::~ContentJSON() {}

        void ContentJSON::readFromString(const char *_json_str) {
            root_node.readValue(_json_str);
        }

        std::string ContentJSON::writeToString(uint32_t _indent) {
            auto __lines = root_node.writeValue(_indent);
            std::string __res;
            for (auto& ln : __lines) {
                __res += ln + '\n';
                ln.clear();
            }
            return __res;
        }
    }; // File END
}; // Simple END
