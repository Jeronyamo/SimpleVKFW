#include "data/vmd_mocap.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>


namespace Simple {
    namespace File {
        namespace VMDMocap {
            namespace {
                typedef ContentVMD::BoneKeyframe BoneKeyframe;

                const uint32_t CHANNEL_COUNT = 7u; // position x, y, z; rotation s, i, j, k
                const size_t   BONE_NAME_LEN = 15u;

                std::string rawName(const BoneKeyframe &_kf) { return std::string(_kf.bone_name, strnlen(_kf.bone_name, BONE_NAME_LEN)); }

                void toChannels(const BoneKeyframe &_kf, float *_out) {
                    _out[0] = _kf.position.x;  _out[1] = _kf.position.y;  _out[2] = _kf.position.z;
                    _out[3] = _kf.rotation.s;  _out[4] = _kf.rotation.i;  _out[5] = _kf.rotation.j;  _out[6] = _kf.rotation.k;
                }

                // MMD curve (x1, y1, x2, y2) is a straight line when both control points lie on the diagonal
                bool curveLinear(const vec4u8 &_c) { return _c.x == _c.y && _c.z == _c.w; }

                // Quintic Hermite basis on u in [0, 1]: value, first and second derivative at both ends (scaled by H, H^2)
                inline float quintic(float _u, float _x0, float _v0, float _a0, float _x1, float _v1, float _a1) {
                    const float u2 = _u * _u, u3 = u2 * _u, u4 = u3 * _u, u5 = u4 * _u;
                    const float h0 = 1.f - 10.f * u3 + 15.f * u4 - 6.f * u5;
                    const float h1 = _u - 6.f * u3 + 8.f * u4 - 3.f * u5;
                    const float h2 = 0.5f * (u2 - 3.f * u3 + 3.f * u4 - u5);
                    const float h3 = 0.5f * (u3 - 2.f * u4 + u5);
                    const float h4 = -4.f * u3 + 7.f * u4 - 3.f * u5;
                    const float h5 = 10.f * u3 - 15.f * u4 + 6.f * u5;
                    return h0 * _x0 + h1 * _v0 + h2 * _a0 + h3 * _a1 + h4 * _v1 + h5 * _x1;
                }
            }; // anonymous END


            void smoothChannel(const std::vector<uint32_t> &_frames, const std::vector<float> &_values,
                               const std::vector<uint8_t> &_hold, const std::vector<uint8_t> &_cut,
                               const SmoothSettings &_settings, std::vector<float> &_out) {
                const size_t __keys = _frames.size();
                if (__keys == 0u) { _out.clear(); return; }
                const uint32_t __first = _frames.front();
                _out.assign(_frames.back() - __first + 1u, _values.front());
                if (__keys < 2u) return;

                std::vector<float> __slope(__keys - 1u), __vel(__keys, 0.f), __acc(__keys, 0.f);
                for (size_t i = 0u; i + 1u < __keys; ++i)
                    __slope[i] = (_values[i + 1u] - _values[i]) / float(_frames[i + 1u] - _frames[i]);

                // Key velocities and accelerations (per frame): non-uniform Catmull-Rom velocity, second divided difference.
                // A key next to a hold, a cut or an end of the track starts/stops from rest
                for (size_t i = 1u; i + 1u < __keys; ++i) {
                    if (_cut[i - 1u] || _cut[i] || _hold[i - 1u] || _hold[i]) continue;
                    const float __hl = float(_frames[i] - _frames[i - 1u]), __hr = float(_frames[i + 1u] - _frames[i]);
                    const float __sl = __slope[i - 1u], __sr = __slope[i];
                    float __v = (__sl * __hr + __sr * __hl) / (__hl + __hr);
                    if (_settings.limit_overshoot) {
                        // Fritsch-Carlson style: zero at an extremum of the channel, at most 3x the smaller slope otherwise
                        if (__sl * __sr <= 0.f) __v = 0.f;
                        else {
                            const float __lim = 3.f * std::min(std::fabs(__sl), std::fabs(__sr));
                            __v = std::max(-__lim, std::min(__lim, __v));
                        }
                    }
                    __vel[i] = __v;
                    __acc[i] = 2.f * (__sr - __sl) / (__hl + __hr);
                }

                for (size_t i = 0u; i + 1u < __keys; ++i) {
                    const uint32_t __t0 = _frames[i], __t1 = _frames[i + 1u];
                    const float __h = float(__t1 - __t0), __x0 = _values[i], __x1 = _values[i + 1u];
                    float *__dst = &_out[__t0 - __first];
                    if (_hold[i] || _cut[i]) {
                        for (uint32_t f = 0u; f < __t1 - __t0; ++f) __dst[f] = __x0 + (__x1 - __x0) * (float(f) / __h);
                        continue;
                    }
                    const float __v0 = __vel[i] * __h, __v1 = __vel[i + 1u] * __h;
                    float __a0 = __acc[i] * __h * __h, __a1 = __acc[i + 1u] * __h * __h;
                    if (_settings.acc_bound > 0.f) {
                        const float __lim = _settings.acc_bound * std::fabs(__x1 - __x0);
                        __a0 = std::max(-__lim, std::min(__lim, __a0));
                        __a1 = std::max(-__lim, std::min(__lim, __a1));
                    }
                    const float __inv_h = 1.f / __h;
                    for (uint32_t f = 0u; f < __t1 - __t0; ++f)
                        __dst[f] = quintic(float(f) * __inv_h, __x0, __v0, __a0, __x1, __v1, __a1);
                }
                _out.back() = _values.back();
            }


            Report smoothBones(ContentVMD &_vmd, const SmoothSettings &_settings) {
                Report __report;
                __report.keys_before = (uint32_t)_vmd.keyframes_bone.size();

                std::vector<std::string> __skip_raw;
                for (size_t i = 0u; i < _settings.skip_bones.size(); ++i) {
                    std::map<std::string, std::string>::const_iterator __it = ContentVMD::bone_name_mappings.find(_settings.skip_bones[i]);
                    if (__it != ContentVMD::bone_name_mappings.end()) __skip_raw.push_back(__it->second);
                }

                // Tracks in the order of the first appearance
                std::map<std::string, size_t> __track_ids;
                std::vector<std::vector<BoneKeyframe>> __tracks;
                for (size_t i = 0u; i < _vmd.keyframes_bone.size(); ++i) {
                    const std::string __name = rawName(_vmd.keyframes_bone[i]);
                    std::map<std::string, size_t>::iterator __it = __track_ids.find(__name);
                    if (__it == __track_ids.end()) {
                        __it = __track_ids.insert(std::make_pair(__name, __tracks.size())).first;
                        __tracks.push_back(std::vector<BoneKeyframe>());
                    }
                    __tracks[__it->second].push_back(_vmd.keyframes_bone[i]);
                }

                std::vector<BoneKeyframe> __result;
                __result.reserve(_vmd.keyframes_bone.size());
                std::vector<uint32_t> __frames;
                std::vector<float> __values[CHANNEL_COUNT], __samples[CHANNEL_COUNT];
                std::vector<uint8_t> __hold[2], __cut, __curved; // hold: [0] position, [1] rotation

                for (size_t t = 0u; t < __tracks.size(); ++t) {
                    std::vector<BoneKeyframe> &__keys = __tracks[t];
                    std::stable_sort(__keys.begin(), __keys.end(),
                                     [](const BoneKeyframe &_a, const BoneKeyframe &_b) { return _a.frame_id < _b.frame_id; });
                    // One key per frame (the last one in the file wins, as when MMD loads it)
                    size_t __n = 0u;
                    for (size_t i = 0u; i < __keys.size(); ++i) {
                        if (__n > 0u && __keys[__n - 1u].frame_id == __keys[i].frame_id) __keys[__n - 1u] = __keys[i];
                        else __keys[__n++] = __keys[i];
                    }
                    __keys.resize(__n);
                    // Rotation signs: each key on the hemisphere of the previous one
                    for (size_t i = 1u; i < __n; ++i)
                        if (Math::dot(__keys[i - 1u].rotation, __keys[i].rotation) < 0.f) __keys[i].rotation = -1.f * __keys[i].rotation;

                    const std::string __name = rawName(__keys.front());
                    bool __skip = __n < 2u || std::find(__skip_raw.begin(), __skip_raw.end(), __name) != __skip_raw.end();

                    __frames.resize(__n);
                    for (uint32_t c = 0u; c < CHANNEL_COUNT; ++c) __values[c].resize(__n);
                    for (size_t i = 0u; i < __n; ++i) {
                        float __ch[CHANNEL_COUNT];
                        toChannels(__keys[i], __ch);
                        __frames[i] = __keys[i].frame_id;
                        for (uint32_t c = 0u; c < CHANNEL_COUNT; ++c) __values[c][i] = __ch[c];
                    }

                    // Segment flags: hold (no change), cut (keys too close), curved (the animator's curve)
                    const float __eps = 1e-6f;
                    bool __moves = false;
                    const size_t __segments = __n > 0u ? __n - 1u : 0u;
                    __hold[0].assign(__segments, 0u);
                    __hold[1].assign(__segments, 0u);
                    __cut.assign(__segments, 0u);
                    __curved.assign(__segments, 0u);
                    for (size_t i = 0u; i + 1u < __n; ++i) {
                        bool __pos_changes[3] = { false, false, false }, __rot_changes = false;
                        for (uint32_t c = 0u; c < CHANNEL_COUNT; ++c) {
                            if (std::fabs(__values[c][i + 1u] - __values[c][i]) <= __eps) continue;
                            if (c < 3u) __pos_changes[c] = true; else __rot_changes = true;
                        }
                        // Note: position and rotation hold separately (a still foot may turn, a turning bone may stay in place)
                        __hold[0][i] = !(__pos_changes[0] || __pos_changes[1] || __pos_changes[2]);
                        __hold[1][i] = !__rot_changes;
                        __moves = __moves || !__hold[0][i] || !__hold[1][i];
                        __cut[i] = __frames[i + 1u] - __frames[i] <= _settings.cut_gap;
                        if (_settings.keep_curves && !__cut[i]) {
                            // Note: the curve of the segment A->B is stored in the key B
                            const ContentVMD::InterpolationInfo __info = ContentVMD::GetInterpolation(__keys[i + 1u]);
                            __curved[i] = (__pos_changes[0] && !curveLinear(__info.mov_x)) || (__pos_changes[1] && !curveLinear(__info.mov_y)) ||
                                          (__pos_changes[2] && !curveLinear(__info.mov_z)) || (__rot_changes    && !curveLinear(__info.rot  ));
                        }
                    }
                    __skip = __skip || !__moves;
                    if (__skip) {
                        __result.insert(__result.end(), __keys.begin(), __keys.end());
                        ++__report.bones_skipped;
                        continue;
                    }

                    for (uint32_t c = 0u; c < CHANNEL_COUNT; ++c)
                        smoothChannel(__frames, __values[c], __hold[c < 3u ? 0 : 1], __cut, _settings, __samples[c]);

                    // The animator's curves where they are not linear
                    for (size_t i = 0u; i + 1u < __n; ++i) {
                        if (!__curved[i]) continue;
                        const ContentVMD::InterpolationInfo __info = ContentVMD::GetInterpolation(__keys[i + 1u]);
                        const vec4u8 *__pos_curves[3] = { &__info.mov_x, &__info.mov_y, &__info.mov_z };
                        const uint32_t __t0 = __frames[i], __h = __frames[i + 1u] - __t0;
                        for (uint32_t f = 1u; f < __h; ++f) {
                            const float __u = float(f) / float(__h);
                            const size_t __s = __t0 + f - __frames.front();
                            for (uint32_t c = 0u; c < 3u; ++c) {
                                const vec4u8 &__cv = *__pos_curves[c];
                                const float __w = ContentVMD::interpolateForMMD(__u, vec2f{float(__cv.x), float(__cv.y)}, vec2f{float(__cv.z), float(__cv.w)});
                                __samples[c][__s] = __values[c][i] + (__values[c][i + 1u] - __values[c][i]) * __w;
                            }
                            const float __w = ContentVMD::interpolateForMMD(__u, vec2f{float(__info.rot.x), float(__info.rot.y)}, vec2f{float(__info.rot.z), float(__info.rot.w)});
                            const Affine::Rotation::Quaternion __q = Interpolation::Slerp::Interpolate(__w, __keys[i].rotation, __keys[i + 1u].rotation);
                            __samples[3][__s] = __q.s;  __samples[4][__s] = __q.i;  __samples[5][__s] = __q.j;  __samples[6][__s] = __q.k;
                        }
                    }

                    // One key per frame with linear curves; name and the other bytes (physics switch) from the key in effect
                    ContentVMD::InterpolationInfo __linear;
                    __linear.mov_x = __linear.mov_y = __linear.mov_z = __linear.rot = vec4u8{(uint8_t)20, (uint8_t)20, (uint8_t)107, (uint8_t)107};
                    size_t __src = 0u;
                    for (size_t s = 0u; s < __samples[0].size(); ++s) {
                        const uint32_t __frame = __frames.front() + (uint32_t)s;
                        while (__src + 1u < __n && __frames[__src + 1u] <= __frame) ++__src;
                        BoneKeyframe __kf = __keys[__src];
                        __kf.frame_id = __frame;
                        __kf.position = vec3f{ __samples[0][s], __samples[1][s], __samples[2][s] };
                        __kf.rotation = Affine::Rotation::unitOrIdentity(Affine::Rotation::Quaternion(__samples[3][s], __samples[4][s], __samples[5][s], __samples[6][s]));
                        const uint8_t __flags[2] = { __kf.int_info[2], __kf.int_info[3] };
                        ContentVMD::SetInterpolation(__kf, __linear);
                        __kf.int_info[2] = __flags[0];
                        __kf.int_info[3] = __flags[1];
                        __result.push_back(__kf);
                    }
                    ++__report.bones_processed;
                }

                _vmd.keyframes_bone.swap(__result);
                __report.keys_after = (uint32_t)_vmd.keyframes_bone.size();
                return __report;
            }
        }; // VMDMocap END
    }; // File END
}; // Simple END
