#include "scene/scene_animation.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <stdexcept>

#include "data/filetypes.h"
#include "platform_dependent/pd_encoding.h"


namespace Simple {
    namespace SceneGraph {
        namespace {
            template <class Key>
            bool keyBefore(const Key &_a, const Key &_b) { return _a.frame < _b.frame; }

            template <class Key>
            bool frameBeforeKey(float _frame, const Key &_key) { return _frame < _key.frame; }

            // Sorts by frame and keeps the last of the keys with the same frame
            template <class Key>
            void sortKeys(std::vector<Key> &_keys) {
                std::stable_sort(_keys.begin(), _keys.end(), keyBefore<Key>);
                size_t __out = 0u;
                for (size_t i = 0u; i < _keys.size(); ++i) {
                    if (__out > 0u && _keys[__out - 1u].frame == _keys[i].frame) _keys[__out - 1u] = _keys[i];
                    else _keys[__out++] = _keys[i];
                }
                _keys.resize(__out);
            }

            vec3 lerp3(const vec3 &_a, const vec3 &_b, float _ta, float _tb, float _tc) {
                return { _a.x + (_b.x - _a.x) * _ta, _a.y + (_b.y - _a.y) * _tb, _a.z + (_b.z - _a.z) * _tc };
            }

            // A bone of the skeleton for a track: the exact name, or (for a name cut by the file) the only bone that starts with it
            uint32_t findTrackBone(const Skeleton &_skeleton, const BoneTrack &_track) {
                uint32_t __bone = _skeleton.findBone(_track.bone);
                if (__bone != SCENE_NONE || !_track.name_cut || _track.bone.empty()) return __bone;
                for (uint32_t i = 0u; i < (uint32_t)_skeleton.bones.size(); ++i) {
                    if (_skeleton.bones[i].name.compare(0u, _track.bone.size(), _track.bone) != 0) continue;
                    if (__bone != SCENE_NONE) return SCENE_NONE; // ambiguous
                    __bone = i;
                }
                return __bone;
            }
        } // anonymous


//  == === ==== >  AnimationCurve

        bool AnimationCurve::isLinear() const { return x1 == y1 && x2 == y2; }

        float AnimationCurve::evaluate(float _t) const {
            _t = std::min(std::max(_t, 0.f), 1.f);
            if (isLinear()) return _t;
            return File::ContentVMD::interpolateForMMD(_t, vec2f{float(x1), float(y1)}, vec2f{float(x2), float(y2)});
        }


//  == === ==== >  AnimationClip

        void AnimationClip::finalize() {
            length = 0.f;
            for (BoneTrack &__track : bones) {
                sortKeys(__track.keys);
                if (!__track.keys.empty()) length = std::max(length, __track.keys.back().frame);
            }
            for (IKTrack &__track : iks) {
                sortKeys(__track.keys);
                if (!__track.keys.empty()) length = std::max(length, __track.keys.back().frame);
            }
            for (MorphTrack &__track : morphs) {
                sortKeys(__track.keys);
                if (!__track.keys.empty()) length = std::max(length, __track.keys.back().frame);
            }
        }


//  == === ==== >  Binding, sampling

        AnimationBinding bindAnimation(const AnimationClip &_clip, const Skeleton &_skeleton, bool _warn) {
            AnimationBinding __binding;
            std::string __missing;
            __binding.bones.resize(_clip.bones.size());
            for (size_t i = 0u; i < _clip.bones.size(); ++i) {
                __binding.bones[i] = findTrackBone(_skeleton, _clip.bones[i]);
                if (__binding.bones[i] != SCENE_NONE) continue;
                ++__binding.missing_bones;
                __missing += (__missing.empty() ? "" : ", ") + _clip.bones[i].bone;
            }
            if (_warn && __binding.missing_bones > 0u)
                fprintf(svkfwwarn, SVKFW_WRAPWARN("SceneGraph :: bindAnimation", "'%s' -> '%s': %u bones are missing, skipped: %s\n"),
                        _clip.name.c_str(), _skeleton.name.c_str(), __binding.missing_bones, __missing.c_str());

            __missing.clear();
            __binding.iks.resize(_clip.iks.size());
            for (size_t i = 0u; i < _clip.iks.size(); ++i) {
                __binding.iks[i] = SCENE_NONE;
                for (uint32_t c = 0u; c < (uint32_t)_skeleton.iks.size(); ++c)
                    if (_skeleton.iks[c].name == _clip.iks[i].ik) { __binding.iks[i] = c; break; }
                if (__binding.iks[i] != SCENE_NONE) continue;
                ++__binding.missing_iks;
                __missing += (__missing.empty() ? "" : ", ") + _clip.iks[i].ik;
            }
            if (_warn && __binding.missing_iks > 0u)
                fprintf(svkfwwarn, SVKFW_WRAPWARN("SceneGraph :: bindAnimation", "'%s' -> '%s': %u IK chains are missing, skipped: %s\n"),
                        _clip.name.c_str(), _skeleton.name.c_str(), __binding.missing_iks, __missing.c_str());

            __missing.clear();
            __binding.morphs.resize(_clip.morphs.size());
            for (size_t i = 0u; i < _clip.morphs.size(); ++i) {
                __binding.morphs[i] = _skeleton.findMorph(_clip.morphs[i].morph);
                if (__binding.morphs[i] != SCENE_NONE) continue;
                ++__binding.missing_morphs;
                __missing += (__missing.empty() ? "" : ", ") + _clip.morphs[i].morph;
            }
            if (_warn && __binding.missing_morphs > 0u)
                fprintf(svkfwwarn, SVKFW_WRAPWARN("SceneGraph :: bindAnimation", "'%s' -> '%s': %u morphs are missing, skipped: %s\n"),
                        _clip.name.c_str(), _skeleton.name.c_str(), __binding.missing_morphs, __missing.c_str());
            return __binding;
        }

        void applyAnimation(const AnimationClip &_clip, const AnimationBinding &_binding, const Skeleton &_skeleton,
                            float _frame, Pose &_pose) {
            if (_pose.local.size() != _skeleton.bones.size() || _pose.ik_enabled.size() != _skeleton.iks.size() ||
                _pose.morph_weights.size() != _skeleton.morphs.size())
                _pose.reset(_skeleton);
            _frame = std::min(std::max(_frame, 0.f), _clip.length);

            const size_t __tracks = std::min(_clip.bones.size(), _binding.bones.size());
            for (size_t t = 0u; t < __tracks; ++t) {
                const uint32_t __b = _binding.bones[t];
                const std::vector<BoneKey> &__keys = _clip.bones[t].keys;
                if (__b == SCENE_NONE || __b >= _pose.local.size() || __keys.empty()) continue;

                // The first key after the frame: interpolate from the one before it
                const size_t __next = size_t(std::upper_bound(__keys.begin(), __keys.end(), _frame, frameBeforeKey<BoneKey>) - __keys.begin());
                vec3 __translation;
                Quat __rotation;
                if (__next == 0u || __next == __keys.size()) {
                    const BoneKey &__key = __keys[__next == 0u ? 0u : __keys.size() - 1u];
                    __translation = __key.translation;
                    __rotation    = __key.rotation;
                }
                else {
                    const BoneKey &__a = __keys[__next - 1u], &__b_key = __keys[__next];
                    const float __t = (_frame - __a.frame) / (__b_key.frame - __a.frame);
                    __translation = lerp3(__a.translation, __b_key.translation,
                                          __b_key.curves[ANIM_TRANSLATION_X].evaluate(__t),
                                          __b_key.curves[ANIM_TRANSLATION_Y].evaluate(__t),
                                          __b_key.curves[ANIM_TRANSLATION_Z].evaluate(__t));
                    __rotation = Interpolation::Slerp::Interpolate(__b_key.curves[ANIM_ROTATION].evaluate(__t), __a.rotation, __b_key.rotation);
                }
                const TRS &__rest = _skeleton.bones[__b].rest;
                _pose.local[__b].translation = __rest.translation + __translation;
                _pose.local[__b].rotation    = __rest.rotation * __rotation;
            }

            const size_t __ik_tracks = std::min(_clip.iks.size(), _binding.iks.size());
            for (size_t t = 0u; t < __ik_tracks; ++t) {
                const uint32_t __c = _binding.iks[t];
                const std::vector<IKKey> &__keys = _clip.iks[t].keys;
                if (__c == SCENE_NONE || __c >= _pose.ik_enabled.size() || __keys.empty()) continue;
                const size_t __next = size_t(std::upper_bound(__keys.begin(), __keys.end(), _frame, frameBeforeKey<IKKey>) - __keys.begin());
                _pose.ik_enabled[__c] = __keys[__next == 0u ? 0u : __next - 1u].enabled ? 1u : 0u;
            }

            const size_t __morph_tracks = std::min(_clip.morphs.size(), _binding.morphs.size());
            for (size_t t = 0u; t < __morph_tracks; ++t) {
                const uint32_t __m = _binding.morphs[t];
                const std::vector<MorphKey> &__keys = _clip.morphs[t].keys;
                if (__m == SCENE_NONE || __m >= _pose.morph_weights.size() || __keys.empty()) continue;
                const size_t __next = size_t(std::upper_bound(__keys.begin(), __keys.end(), _frame, frameBeforeKey<MorphKey>) - __keys.begin());
                if (__next == 0u || __next == __keys.size()) {
                    _pose.morph_weights[__m] = __keys[__next == 0u ? 0u : __keys.size() - 1u].weight;
                    continue;
                }
                const MorphKey &__a = __keys[__next - 1u], &__b = __keys[__next];
                _pose.morph_weights[__m] = __a.weight + (__b.weight - __a.weight) * (_frame - __a.frame) / (__b.frame - __a.frame);
            }
            _pose.dirty = true;
        }


//  == === ==== >  VMD

        AnimationClip loadVMD(const std::string &_path) {
            File::ReaderWriterVMD __reader;
            SVKFW_ASSERT(__reader.read(_path), std::runtime_error, "SceneGraph :: loadVMD", "could not read '" + _path + "'");
            const size_t __sep = _path.find_last_of("/\\");
            return convertVMD(__reader.file_content, __sep == std::string::npos ? _path : _path.substr(__sep + 1u));
        }

        AnimationClip convertVMD(const File::ContentVMD &_vmd, const std::string &_name) {
            AnimationClip __clip;
            __clip.name = _name;

            // Bone keys: one track per name, in the order of the first appearance
            const size_t __bone_name_len = 15u;
            std::map<std::string, size_t> __tracks; // raw name -> track
            for (File::ContentVMD::BoneKeyframe __kf : _vmd.keyframes_bone) { // a copy: 'GetInterpolation' takes a reference
                const std::string __raw(__kf.bone_name, strnlen(__kf.bone_name, __bone_name_len));
                std::map<std::string, size_t>::iterator __found = __tracks.find(__raw);
                if (__found == __tracks.end()) {
                    __found = __tracks.insert(std::make_pair(__raw, __clip.bones.size())).first;
                    BoneTrack __new;
                    __new.bone     = Enc::shiftJISToUTF8(__raw);
                    __new.name_cut = __raw.size() >= __bone_name_len - 1u;
                    __clip.bones.push_back(__new);
                }

                BoneKey __key;
                __key.frame       = float(__kf.frame_id);
                __key.translation = { __kf.position.x, __kf.position.y, __kf.position.z };
                __key.rotation    = __kf.rotation.normalize();
                // 'mov_x' etc.: (x1, y1, x2, y2)
                const File::ContentVMD::InterpolationInfo __info = File::ContentVMD::GetInterpolation(__kf);
                const vec4u8 *__channels[ANIM_CHANNEL_COUNT] = { &__info.mov_x, &__info.mov_y, &__info.mov_z, &__info.rot };
                for (int c = 0; c < ANIM_CHANNEL_COUNT; ++c) {
                    __key.curves[c].x1 = __channels[c]->x;
                    __key.curves[c].y1 = __channels[c]->y;
                    __key.curves[c].x2 = __channels[c]->z;
                    __key.curves[c].y2 = __channels[c]->w;
                }
                __clip.bones[__found->second].keys.push_back(__key);
            }

            // Morph keys: one track per name
            const size_t __morph_name_len = 15u;
            std::map<std::string, size_t> __morph_tracks;
            for (const File::ContentVMD::MorphKeyframe &__kf : _vmd.keyframes_morph) {
                const std::string __raw(__kf.morph_name, strnlen(__kf.morph_name, __morph_name_len));
                std::map<std::string, size_t>::iterator __found = __morph_tracks.find(__raw);
                if (__found == __morph_tracks.end()) {
                    __found = __morph_tracks.insert(std::make_pair(__raw, __clip.morphs.size())).first;
                    __clip.morphs.push_back(MorphTrack());
                    __clip.morphs.back().morph = Enc::shiftJISToUTF8(__raw);
                }
                MorphKey __key;
                __key.frame  = float(__kf.frame_id);
                __key.weight = __kf.weight;
                __clip.morphs[__found->second].keys.push_back(__key);
            }

            // IK switches: a key per IK keyframe of each listed chain
            const size_t __ik_name_len = 20u;
            for (const File::ContentVMD::IKKeyframe &__kf : _vmd.keyframes_ik) {
                for (const File::ContentVMD::IKKeyframe::IKState &__state : __kf.iks) {
                    const std::string __name = Enc::shiftJISToUTF8(__state.ik_name, __ik_name_len);
                    size_t __track = 0u;
                    while (__track < __clip.iks.size() && __clip.iks[__track].ik != __name) ++__track;
                    if (__track == __clip.iks.size()) {
                        __clip.iks.push_back(IKTrack());
                        __clip.iks.back().ik = __name;
                    }
                    IKKey __key;
                    __key.frame   = float(__kf.frame_id);
                    __key.enabled = __state.enabled;
                    __clip.iks[__track].keys.push_back(__key);
                }
            }

            __clip.finalize();
            return __clip;
        }


//  == === ==== >  MMD camera

        namespace {
            // Rotations of a column vector (right-hand rule matrices, applied in the MMD space)
            vec3 rotX(const vec3 &_v, float _a) { const float c = std::cos(_a), s = std::sin(_a); return { _v.x, _v.y * c - _v.z * s, _v.y * s + _v.z * c }; }
            vec3 rotY(const vec3 &_v, float _a) { const float c = std::cos(_a), s = std::sin(_a); return { _v.x * c + _v.z * s, _v.y, -_v.x * s + _v.z * c }; }
            vec3 rotZ(const vec3 &_v, float _a) { const float c = std::cos(_a), s = std::sin(_a); return { _v.x * c - _v.y * s, _v.x * s + _v.y * c, _v.z }; }
            vec3 cameraRotate(const vec3 &_v, const vec3 &_r) { return rotY(rotX(rotZ(_v, -_r.z), -_r.x), -_r.y); }
        } // anonymous

        // Note: MMD rotates the offset (0, 0, -distance) and the up vector by the roll first, then the pitch, then the yaw, all with
        //       the VMD angles negated: Ry(-y) Rx(-x) Rz(-z) (checked against MMD on a camera VMD)
        void CameraPose::lookAt(vec3 &_eye, vec3 &_up) const {
            _eye = center + cameraRotate({0.f, 0.f, -distance}, rotation);
            _up  = cameraRotate({0.f, 1.f, 0.f}, rotation);
        }

        bool CameraClip::empty() const { return keys.empty(); }

        void CameraClip::finalize() {
            sortKeys(keys);
            length = keys.empty() ? 0.f : keys.back().frame;
        }

        CameraPose CameraClip::evaluate(float _frame) const {
            if (keys.empty()) return CameraPose();
            _frame = std::min(std::max(_frame, 0.f), length);
            const size_t __next = size_t(std::upper_bound(keys.begin(), keys.end(), _frame, frameBeforeKey<CameraKey>) - keys.begin());
            if (__next == 0u) return keys.front().pose;
            if (__next >= keys.size()) return keys.back().pose;
            const CameraKey &a = keys[__next - 1u], &b = keys[__next];
            // Keys on adjacent frames: MMD ignores the curves and cuts at the next key (a camera cut)
            if (b.frame - a.frame <= 1.f) return a.pose;
            const float t = (_frame - a.frame) / (b.frame - a.frame);
            float e[CAMERA_CHANNEL_COUNT];
            for (int c = 0; c < CAMERA_CHANNEL_COUNT; ++c) e[c] = b.curves[c].evaluate(t);
            CameraPose __pose;
            __pose.center   = lerp3(a.pose.center, b.pose.center, e[CAMERA_X], e[CAMERA_Y], e[CAMERA_Z]);
            __pose.rotation = a.pose.rotation + (b.pose.rotation - a.pose.rotation) * e[CAMERA_ROTATION];
            __pose.distance = a.pose.distance + (b.pose.distance - a.pose.distance) * e[CAMERA_DISTANCE];
            __pose.fov      = a.pose.fov + (b.pose.fov - a.pose.fov) * e[CAMERA_FOV];
            __pose.perspective = a.pose.perspective;
            return __pose;
        }

        CameraClip convertVMDCamera(const File::ContentVMD &_vmd, const std::string &_name) {
            CameraClip __clip;
            __clip.name = _name;
            for (File::ContentVMD::CameraKeyframe __kf : _vmd.keyframes_camera) {
                CameraKey __key;
                __key.frame = float(__kf.frame_id);
                __key.pose.center   = { __kf.position.x, __kf.position.y, __kf.position.z };
                __key.pose.rotation = { __kf.rotation.x, __kf.rotation.y, __kf.rotation.z };
                __key.pose.distance = -__kf.distance; // the VMD stores it negated
                __key.pose.fov      = float(__kf.fov);
                __key.pose.perspective = __kf.perspect == 0u; // VMD stores "perspective off"
                const File::ContentVMD::InterpolationInfo __info = File::ContentVMD::GetInterpolation(__kf);
                const vec4u8 *__channels[CAMERA_CHANNEL_COUNT] = { &__info.mov_x, &__info.mov_y, &__info.mov_z, &__info.rot, &__info.dist, &__info.fov };
                for (int c = 0; c < CAMERA_CHANNEL_COUNT; ++c) {
                    __key.curves[c].x1 = __channels[c]->x;
                    __key.curves[c].y1 = __channels[c]->y;
                    __key.curves[c].x2 = __channels[c]->z;
                    __key.curves[c].y2 = __channels[c]->w;
                }
                __clip.keys.push_back(__key);
            }
            __clip.finalize();
            return __clip;
        }
    }; // SceneGraph END
}; // Simple END
