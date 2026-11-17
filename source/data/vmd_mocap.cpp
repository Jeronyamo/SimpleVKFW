#include "data/vmd_mocap.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>

#include "math/randvec.h"


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

                typedef Affine::Rotation::Quaternion Quaternion;

                // Bone tracks in the order of the first appearance: sorted by frame, one key per frame (the last one in the
                // file wins, as when MMD loads it), each rotation on the hemisphere of the previous one
                void collectTracks(const ContentVMD &_vmd, std::map<std::string, size_t> &_ids, std::vector<std::vector<BoneKeyframe>> &_tracks) {
                    for (size_t i = 0u; i < _vmd.keyframes_bone.size(); ++i) {
                        const std::string __name = rawName(_vmd.keyframes_bone[i]);
                        std::map<std::string, size_t>::iterator __it = _ids.find(__name);
                        if (__it == _ids.end()) {
                            __it = _ids.insert(std::make_pair(__name, _tracks.size())).first;
                            _tracks.push_back(std::vector<BoneKeyframe>());
                        }
                        _tracks[__it->second].push_back(_vmd.keyframes_bone[i]);
                    }
                    for (size_t t = 0u; t < _tracks.size(); ++t) {
                        std::vector<BoneKeyframe> &__keys = _tracks[t];
                        std::stable_sort(__keys.begin(), __keys.end(),
                                         [](const BoneKeyframe &_a, const BoneKeyframe &_b) { return _a.frame_id < _b.frame_id; });
                        size_t __n = 0u;
                        for (size_t i = 0u; i < __keys.size(); ++i) {
                            if (__n > 0u && __keys[__n - 1u].frame_id == __keys[i].frame_id) __keys[__n - 1u] = __keys[i];
                            else __keys[__n++] = __keys[i];
                        }
                        __keys.resize(__n);
                        for (size_t i = 1u; i < __n; ++i)
                            if (Math::dot(__keys[i - 1u].rotation, __keys[i].rotation) < 0.f) __keys[i].rotation = -1.f * __keys[i].rotation;
                    }
                }

                // A key of a full-key track: '_base' (name, physics switch) with linear curves
                BoneKeyframe fullKey(const BoneKeyframe &_base, uint32_t _frame, const vec3f &_position, const Quaternion &_rotation) {
                    ContentVMD::InterpolationInfo __linear;
                    __linear.mov_x = __linear.mov_y = __linear.mov_z = __linear.rot = vec4u8{(uint8_t)20, (uint8_t)20, (uint8_t)107, (uint8_t)107};
                    BoneKeyframe __kf = _base;
                    __kf.frame_id = _frame;
                    __kf.position = _position;
                    __kf.rotation = Affine::Rotation::unitOrIdentity(_rotation);
                    const uint8_t __flags[2] = { __kf.int_info[2], __kf.int_info[3] };
                    ContentVMD::SetInterpolation(__kf, __linear);
                    __kf.int_info[2] = __flags[0];
                    __kf.int_info[3] = __flags[1];
                    return __kf;
                }

                // The weight of the MMD curve '_c' at '_u' (linear curves, including the all-zero ones, are the identity)
                float curveWeight(const vec4u8 &_c, float _u) {
                    if (curveLinear(_c)) return _u;
                    return ContentVMD::interpolateForMMD(_u, vec2f{float(_c.x), float(_c.y)}, vec2f{float(_c.z), float(_c.w)});
                }

                // A baked track: the pose at every frame from 'first' (held before the first and after the last key)
                struct Baked {
                    uint32_t first = 0u, last = 0u;     // the frames of the first and the last key
                    std::vector<vec3f> position;
                    std::vector<Quaternion> rotation;
                };

                void bake(std::vector<BoneKeyframe> &_keys, uint32_t _from, uint32_t _to, Baked &_out) {
                    _out.first = _keys.front().frame_id;
                    _out.last  = _keys.back ().frame_id;
                    const size_t __count = _to - _from + 1u;
                    _out.position.resize(__count);
                    _out.rotation.resize(__count);
                    size_t __k = 0u;
                    for (size_t s = 0u; s < __count; ++s) {
                        const uint32_t __frame = _from + (uint32_t)s;
                        while (__k + 1u < _keys.size() && _keys[__k + 1u].frame_id <= __frame) ++__k;
                        const BoneKeyframe &__a = _keys[__k];
                        if (__frame <= __a.frame_id || __k + 1u == _keys.size()) {
                            _out.position[s] = __a.position;
                            _out.rotation[s] = __a.rotation;
                            continue;
                        }
                        BoneKeyframe &__b = _keys[__k + 1u];
                        const float __u = float(__frame - __a.frame_id) / float(__b.frame_id - __a.frame_id);
                        const ContentVMD::InterpolationInfo __info = ContentVMD::GetInterpolation(__b);
                        _out.position[s] = vec3f{ __a.position.x + (__b.position.x - __a.position.x) * curveWeight(__info.mov_x, __u),
                                                  __a.position.y + (__b.position.y - __a.position.y) * curveWeight(__info.mov_y, __u),
                                                  __a.position.z + (__b.position.z - __a.position.z) * curveWeight(__info.mov_z, __u) };
                        _out.rotation[s] = Interpolation::Slerp::Interpolate(curveWeight(__info.rot, __u), __a.rotation, __b.rotation);
                    }
                    for (size_t s = 1u; s < __count; ++s)
                        if (Math::dot(_out.rotation[s - 1u], _out.rotation[s]) < 0.f) _out.rotation[s] = -1.f * _out.rotation[s];
                }

                // Gaussian lowpass of a rotation track (componentwise, the ends held), normalized
                void lowpass(const std::vector<Quaternion> &_in, float _sigma, std::vector<Quaternion> &_out) {
                    const int32_t __n = (int32_t)_in.size(), __r = std::max(1, (int32_t)std::ceil(3.f * _sigma));
                    std::vector<float> __w(2 * __r + 1);
                    float __sum = 0.f;
                    for (int32_t i = -__r; i <= __r; ++i) __sum += (__w[i + __r] = std::exp(-0.5f * float(i * i) / (_sigma * _sigma)));
                    for (size_t i = 0u; i < __w.size(); ++i) __w[i] /= __sum;
                    _out.resize(_in.size());
                    for (int32_t t = 0; t < __n; ++t) {
                        Quaternion __acc(0.f, 0.f, 0.f, 0.f);
                        for (int32_t i = -__r; i <= __r; ++i)
                            __acc = __acc + __w[i + __r] * _in[std::max(0, std::min(__n - 1, t + i))];
                        _out[t] = Affine::Rotation::unitOrIdentity(__acc);
                    }
                }

                // The deviation of '_in' from its trend (rotation vectors in the bone's frame)
                void deviation(const std::vector<Quaternion> &_in, float _sigma, std::vector<vec3f> &_out) {
                    std::vector<Quaternion> __trend;
                    lowpass(_in, _sigma, __trend);
                    _out.resize(_in.size());
                    for (size_t t = 0u; t < _in.size(); ++t) _out[t] = Affine::Rotation::rotationVector(__trend[t].conjugate() * _in[t]);
                }

                // child(t) = exp(gain * dev(t - delay)) * child(t) over [_from, _to)
                void transfer(const std::vector<vec3f> &_dev, float _gain, uint32_t _delay, size_t _from, size_t _to,
                              const std::vector<Quaternion> &_child, std::vector<Quaternion> &_out) {
                    _out = _child;
                    if (_gain == 0.f) return;
                    for (size_t t = _from; t < _to; ++t) {
                        const vec3f &__d = _dev[t >= _from + _delay ? t - _delay : _from];
                        _out[t] = Affine::Rotation::fromRotationVector(vec3f{ __d.x * _gain, __d.y * _gain, __d.z * _gain }) * _child[t];
                    }
                }

                float rmsDegrees(const std::vector<vec3f> &_v) {
                    double __sum = 0.;
                    for (size_t t = 0u; t < _v.size(); ++t) __sum += double(_v[t].x * _v[t].x + _v[t].y * _v[t].y + _v[t].z * _v[t].z);
                    return _v.empty() ? 0.f : float(std::sqrt(__sum / double(_v.size()))) * 57.29578f;
                }

                float meanSquareDiff(const std::vector<float> &_a, const std::vector<float> &_b) {
                    float __sum = 0.f;
                    for (size_t i = 0u; i < _a.size(); ++i) __sum += (_a[i] - _b[i]) * (_a[i] - _b[i]);
                    return _a.empty() ? 0.f : __sum / float(_a.size());
                }

                std::string rawBoneName(const std::string &_name) {
                    std::map<std::string, std::string>::const_iterator __it = ContentVMD::bone_name_mappings.find(_name);
                    return __it != ContentVMD::bone_name_mappings.end() ? __it->second : _name;
                }

                bool readFloats(const ContentJSON::JSONNode &_node, std::vector<float> &_out) {
                    if (_node.node_type != JSON_TYPE_ARR) return false;
                    _out.clear();
                    for (size_t i = 0u; i < _node.value_arr_obj.size(); ++i) {
                        if (_node.value_arr_obj[i].node_type != JSON_TYPE_NUM) return false;
                        _out.push_back(float(_node.value_arr_obj[i].value_num));
                    }
                    return true;
                }

                bool readVec3(const ContentJSON::JSONNode *_node, vec3f &_out) {
                    std::vector<float> __v;
                    if (_node == nullptr || !readFloats(*_node, __v) || __v.size() != 3u) return false;
                    _out = vec3f{ __v[0], __v[1], __v[2] };
                    return true;
                }

                // Gaussian smoothing of a scalar track (the ends held)
                void smoothScalar(std::vector<float> &_x, float _sigma) {
                    if (_x.empty() || _sigma <= 0.f) return;
                    const int32_t __n = (int32_t)_x.size(), __r = std::max(1, (int32_t)std::ceil(3.f * _sigma));
                    std::vector<float> __w(2 * __r + 1), __in(_x);
                    float __sum = 0.f;
                    for (int32_t i = -__r; i <= __r; ++i) __sum += (__w[i + __r] = std::exp(-0.5f * float(i * i) / (_sigma * _sigma)));
                    for (int32_t t = 0; t < __n; ++t) {
                        float __acc = 0.f;
                        for (int32_t i = -__r; i <= __r; ++i) __acc += __w[i + __r] * __in[std::max(0, std::min(__n - 1, t + i))];
                        _x[t] = __acc / __sum;
                    }
                }

                // Gaussian lowpass of a vector track (the ends held); '_sigma' in frames
                void smoothVec3(const std::vector<vec3f> &_in, float _sigma, std::vector<vec3f> &_out) {
                    const int32_t __n = (int32_t)_in.size(), __r = std::max(1, (int32_t)std::ceil(3.f * _sigma));
                    std::vector<float> __w(2 * __r + 1);
                    float __sum = 0.f;
                    for (int32_t i = -__r; i <= __r; ++i) __sum += (__w[i + __r] = std::exp(-0.5f * float(i * i) / (_sigma * _sigma)));
                    for (size_t i = 0u; i < __w.size(); ++i) __w[i] /= __sum;
                    _out.resize(_in.size());
                    for (int32_t t = 0; t < __n; ++t) {
                        vec3f __acc{ 0.f, 0.f, 0.f };
                        for (int32_t i = -__r; i <= __r; ++i) __acc = __acc + _in[std::max(0, std::min(__n - 1, t + i))] * __w[i + __r];
                        _out[t] = __acc;
                    }
                }

                // Sigma (frames) of the Gaussian lowpass with the gain 1/2 at '_hz'
                float halfGainSigma(float _hz, float _fps) { return _fps * 1.17741f / (6.2831853f * _hz); }

                // Band [_lo, _hi] Hz of a track: the difference of two Gaussian lowpasses
                void bandpass(const std::vector<vec3f> &_in, float _lo, float _hi, float _fps, std::vector<vec3f> &_out) {
                    std::vector<vec3f> __wide;
                    smoothVec3(_in, halfGainSigma(_hi, _fps), _out);
                    smoothVec3(_in, halfGainSigma(_lo, _fps), __wide);
                    for (size_t t = 0u; t < _out.size(); ++t) _out[t] = _out[t] - __wide[t];
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

                std::map<std::string, size_t> __track_ids;
                std::vector<std::vector<BoneKeyframe>> __tracks;
                collectTracks(_vmd, __track_ids, __tracks);

                std::vector<BoneKeyframe> __result;
                __result.reserve(_vmd.keyframes_bone.size());
                std::vector<uint32_t> __frames;
                std::vector<float> __values[CHANNEL_COUNT], __samples[CHANNEL_COUNT];
                std::vector<uint8_t> __hold[2], __cut, __curved; // hold: [0] position, [1] rotation

                for (size_t t = 0u; t < __tracks.size(); ++t) {
                    std::vector<BoneKeyframe> &__keys = __tracks[t];
                    const size_t __n = __keys.size();

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
                    size_t __src = 0u;
                    for (size_t s = 0u; s < __samples[0].size(); ++s) {
                        const uint32_t __frame = __frames.front() + (uint32_t)s;
                        while (__src + 1u < __n && __frames[__src + 1u] <= __frame) ++__src;
                        __result.push_back(fullKey(__keys[__src], __frame, vec3f{ __samples[0][s], __samples[1][s], __samples[2][s] },
                                                   Quaternion(__samples[3][s], __samples[4][s], __samples[5][s], __samples[6][s])));
                    }
                    ++__report.bones_processed;
                }

                _vmd.keyframes_bone.swap(__result);
                __report.keys_after = (uint32_t)_vmd.keyframes_bone.size();
                return __report;
            }

            void coupling(const std::vector<Quaternion> &_parent, const std::vector<Quaternion> &_child, std::vector<float> &_out) {
                const size_t __n = std::min(_parent.size(), _child.size());
                std::vector<vec3f> __wp(__n > 0u ? __n - 1u : 0u), __wc(__wp.size());
                for (size_t t = 0u; t + 1u < __n; ++t) {
                    __wp[t] = Affine::Rotation::rotationVector(_parent[t].conjugate() * _parent[t + 1u]);
                    __wc[t] = _child[t].rotateByUnit(Affine::Rotation::rotationVector(_child[t].conjugate() * _child[t + 1u]));
                }
                for (size_t l = 0u; l < _out.size(); ++l) {
                    double __ab = 0., __aa = 0., __bb = 0.;
                    for (size_t t = 0u; t + l < __wp.size(); ++t) {
                        const vec3f &__a = __wp[t], &__b = __wc[t + l];
                        __ab += double(__a.x * __b.x + __a.y * __b.y + __a.z * __b.z);
                        __aa += double(__a.x * __a.x + __a.y * __a.y + __a.z * __a.z);
                        __bb += double(__b.x * __b.x + __b.y * __b.y + __b.z * __b.z);
                    }
                    _out[l] = __aa > 0. && __bb > 0. ? float(__ab / std::sqrt(__aa * __bb)) : 0.f;
                }
            }


            bool loadChainProfile(const std::string &_path, ChainProfile &_out, std::string &_error) {
                std::ifstream __file(_path.c_str(), std::ios::binary);
                if (!__file) { _error = "Could not open '" + _path + "'"; return false; }
                std::stringstream __text;
                __text << __file.rdbuf();
                const std::string __str = __text.str();
                ContentJSON __json;
                __json.readFromString(__str.c_str());
                const ContentJSON::JSONNode &__root = __json.root_node;
                const ContentJSON::JSONNode *__chain = __root.find("chain");
                const ContentJSON::JSONNode *__pairs = __chain != nullptr ? __chain->find("pairs") : nullptr;
                if (__pairs == nullptr || __pairs->node_type != JSON_TYPE_ARR) { _error = "'" + _path + "' has no \"chain\": {\"pairs\": [...]}"; return false; }

                ChainProfile __profile;
                for (size_t i = 0u; i < __pairs->value_arr_obj.size(); ++i) {
                    const ContentJSON::JSONNode &__p = __pairs->value_arr_obj[i];
                    const ContentJSON::JSONNode *__target = __p.find("target");
                    ChainPair __pair;
                    __pair.parent = __p.getStr("parent", "");
                    __pair.child  = __p.getStr("child",  "");
                    if (__pair.parent.empty() || __pair.child.empty() || __target == nullptr || !readFloats(*__target, __pair.target) || __pair.target.empty()) {
                        _error = "'" + _path + "': chain pair " + std::to_string(i) + " needs \"parent\", \"child\" and \"target\"";
                        return false;
                    }
                    __profile.pairs.push_back(__pair);
                }
                std::vector<float> __values;
                const ContentJSON::JSONNode *__node;
                if ((__node = __chain->find("gains" )) != nullptr && readFloats(*__node, __values) && !__values.empty()) __profile.gains  = __values;
                if ((__node = __chain->find("sigmas")) != nullptr && readFloats(*__node, __values) && !__values.empty()) __profile.sigmas = __values;
                __profile.max_added_deg = float(__chain->getNum("max_added_deg", double(__profile.max_added_deg)));
                if ((__node = __chain->find("delays")) != nullptr && readFloats(*__node, __values) && !__values.empty()) {
                    __profile.delays.clear();
                    for (size_t i = 0u; i < __values.size(); ++i) __profile.delays.push_back((uint32_t)std::max(0.f, __values[i]));
                }
                const ContentJSON::JSONNode *__jitter = __root.find("jitter");
                if (__jitter != nullptr) {
                    __profile.jitter_frames = float(__jitter->getNum("frames", double(__profile.jitter_frames)));
                    __profile.jitter_rate   = float(__jitter->getNum("rate",   double(__profile.jitter_rate)));
                    const ContentJSON::JSONNode *__bones = __jitter->find("bones");
                    if (__bones != nullptr && __bones->node_type == JSON_TYPE_ARR)
                        for (size_t i = 0u; i < __bones->value_arr_obj.size(); ++i)
                            if (__bones->value_arr_obj[i].node_type == JSON_TYPE_STR) __profile.jitter_bones.push_back(__bones->value_arr_obj[i].value_str);
                }
                _out = __profile;
                return true;
            }


            ChainReport chainBones(ContentVMD &_vmd, const ChainProfile &_profile, const ChainSettings &_settings) {
                ChainReport __report;
                __report.keys_before = (uint32_t)_vmd.keyframes_bone.size();

                std::map<std::string, size_t> __track_ids;
                std::vector<std::vector<BoneKeyframe>> __tracks;
                collectTracks(_vmd, __track_ids, __tracks);

                // The tracks involved (raw names) and the frames they cover
                struct Use { size_t track; std::string raw; };
                std::vector<std::pair<size_t, size_t>> __pairs;  // indices in '__used' of parent and child
                std::vector<size_t> __pair_ids;                  // in '_profile.pairs'
                std::vector<size_t> __jitter;                    // indices in '__used'
                std::vector<Use> __used;
                const auto __use = [&](const std::string &_name) -> int64_t {
                    const std::string __raw = rawBoneName(_name);
                    std::map<std::string, size_t>::const_iterator __it = __track_ids.find(__raw);
                    if (__it == __track_ids.end() || __tracks[__it->second].empty()) return -1;
                    for (size_t i = 0u; i < __used.size(); ++i) if (__used[i].track == __it->second) return (int64_t)i;
                    __used.push_back(Use{ __it->second, __raw });
                    return (int64_t)__used.size() - 1;
                };
                for (size_t p = 0u; p < _profile.pairs.size(); ++p) {
                    const int64_t __a = __use(_profile.pairs[p].parent), __b = __use(_profile.pairs[p].child);
                    if (__a < 0 || __b < 0 || __a == __b) continue;
                    __pairs.push_back(std::make_pair((size_t)__a, (size_t)__b));
                    __pair_ids.push_back(p);
                }
                const float __jitter_frames = _profile.jitter_frames * _settings.jitter;
                if (__jitter_frames > 0.f)
                    for (size_t j = 0u; j < _profile.jitter_bones.size(); ++j) {
                        const int64_t __b = __use(_profile.jitter_bones[j]);
                        if (__b >= 0 && std::find(__jitter.begin(), __jitter.end(), (size_t)__b) == __jitter.end()) __jitter.push_back((size_t)__b);
                    }
                if (__used.empty()) { __report.keys_after = __report.keys_before; return __report; }

                uint32_t __from = UINT32_MAX, __to = 0u;
                for (size_t i = 0u; i < __used.size(); ++i) {
                    __from = std::min(__from, __tracks[__used[i].track].front().frame_id);
                    __to   = std::max(__to,   __tracks[__used[i].track].back ().frame_id);
                }
                std::vector<Baked> __orig(__used.size());
                for (size_t i = 0u; i < __used.size(); ++i) bake(__tracks[__used[i].track], __from, __to, __orig[i]);
                std::vector<std::vector<Quaternion>> __rot(__used.size());
                for (size_t i = 0u; i < __used.size(); ++i) __rot[i] = __orig[i].rotation;
                std::vector<uint8_t> __changed(__used.size(), 0u);

                // Pairs from the root: fit on the frames both bones have keys for, against the parent as changed by its own pair
                const float __strength = std::max(0.f, _settings.strength);
                std::vector<float> __coup;
                std::vector<vec3f> __dev;
                std::vector<Quaternion> __trial;
                for (size_t p = 0u; p < __pairs.size(); ++p) {
                    const ChainPair &__pair = _profile.pairs[__pair_ids[p]];
                    const Baked &__parent = __orig[__pairs[p].first];
                    const size_t __child = __pairs[p].second;
                    const size_t __lo = std::max(__parent.first, __orig[__child].first) - __from;
                    const size_t __hi = std::min(__parent.last,  __orig[__child].last ) - __from + 1u;
                    ChainPairReport __rep;
                    __rep.parent = __pair.parent;
                    __rep.child  = __pair.child;
                    __coup.resize(__pair.target.size());
                    if (__hi < __lo + 2u * __coup.size() + 2u) continue; // too short to tell
                    const std::vector<Quaternion> __p_win(__rot[__pairs[p].first].begin() + __lo, __rot[__pairs[p].first].begin() + __hi);
                    const std::vector<Quaternion> __c_win(__rot[__child].begin() + __lo, __rot[__child].begin() + __hi);
                    coupling(__p_win, __c_win, __coup);
                    __rep.error_before = __rep.error_after = meanSquareDiff(__coup, __pair.target);
                    std::vector<vec3f> __best_dev;
                    for (size_t si = 0u; si < _profile.sigmas.size(); ++si) {
                        deviation(__p_win, std::max(1.f, _profile.sigmas[si]), __dev);
                        const float __dev_deg = rmsDegrees(__dev);
                        for (size_t di = 0u; di < _profile.delays.size(); ++di)
                            for (size_t gi = 0u; gi < _profile.gains.size(); ++gi) {
                                if (_profile.gains[gi] <= 0.f || _profile.gains[gi] * __strength * __dev_deg > _profile.max_added_deg) continue;
                                transfer(__dev, _profile.gains[gi], _profile.delays[di], 0u, __dev.size(), __c_win, __trial);
                                coupling(__p_win, __trial, __coup);
                                const float __err = meanSquareDiff(__coup, __pair.target);
                                if (__err < __rep.error_after) {
                                    __rep.error_after = __err;
                                    __rep.gain  = _profile.gains[gi];
                                    __rep.delay = _profile.delays[di];
                                    __rep.sigma = _profile.sigmas[si];
                                    __best_dev  = __dev;
                                }
                            }
                    }
                    __rep.gain *= __strength;
                    if (__rep.gain > 0.f) {
                        // Applied over all the frames of the pair's window; the rotation added is 'gain * dev'
                        transfer(__best_dev, __rep.gain, __rep.delay, 0u, __best_dev.size(), __c_win, __trial);
                        for (size_t t = 0u; t < __trial.size(); ++t) __rot[__child][__lo + t] = __trial[t];
                        __rep.added_deg = rmsDegrees(__best_dev) * __rep.gain;
                        if (__strength != 1.f) {
                            coupling(__p_win, __trial, __coup);
                            __rep.error_after = meanSquareDiff(__coup, __pair.target);
                        }
                        __changed[__child] = 1u;
                    } else __rep.error_after = __rep.error_before;
                    __report.pairs.push_back(__rep);
                }

                // Timing jitter: each bone is sampled at t + tau(t), tau smooth noise with the rms 'jitter_frames'
                std::vector<std::vector<vec3f>> __pos(__used.size());
                for (size_t i = 0u; i < __used.size(); ++i) __pos[i] = __orig[i].position;
                const Noise::NoisePerlin __noise(_settings.seed);
                const size_t __count = __to - __from + 1u;
                std::vector<float> __tau(__count);
                for (size_t j = 0u; j < __jitter.size(); ++j) {
                    const size_t __b = __jitter[j];
                    double __sum = 0.;
                    for (size_t t = 0u; t < __count; ++t) {
                        // Note: off the lattice (gradient noise is 0 on it); every bone along its own line
                        __tau[t] = __noise.sample(vec4f{ float(__from + t) * _profile.jitter_rate / 30.f, 0.5f + 7.31f * float(j), 0.37f, 0.71f });
                        __sum += double(__tau[t]) * double(__tau[t]);
                    }
                    const float __rms = float(std::sqrt(__sum / double(__count)));
                    if (__rms <= 1e-6f) continue;
                    const float __scale = __jitter_frames / __rms;
                    const std::vector<Quaternion> __src_rot = __rot[__b];
                    const std::vector<vec3f>      __src_pos = __pos[__b];
                    for (size_t t = 0u; t < __count; ++t) {
                        const float __at = std::max(0.f, std::min(float(__count - 1u), float(t) + __tau[t] * __scale));
                        const size_t __i0 = std::min((size_t)__at, __count - 1u), __i1 = std::min(__i0 + 1u, __count - 1u);
                        const float __u = __at - float(__i0);
                        __rot[__b][t] = Interpolation::Slerp::Interpolate(__u, __src_rot[__i0], __src_rot[__i1]);
                        __pos[__b][t] = vec3f{ __src_pos[__i0].x + (__src_pos[__i1].x - __src_pos[__i0].x) * __u,
                                               __src_pos[__i0].y + (__src_pos[__i1].y - __src_pos[__i0].y) * __u,
                                               __src_pos[__i0].z + (__src_pos[__i1].z - __src_pos[__i0].z) * __u };
                    }
                    __changed[__b] = 1u;
                    ++__report.bones_jittered;
                }

                // The changed tracks get one key per frame over [__from, __to]
                std::vector<uint8_t> __replace(__tracks.size(), 0u);
                for (size_t i = 0u; i < __used.size(); ++i) __replace[__used[i].track] = __changed[i];
                std::vector<BoneKeyframe> __result;
                __result.reserve(_vmd.keyframes_bone.size() + __count * __used.size());
                for (size_t t = 0u; t < __tracks.size(); ++t)
                    if (!__replace[t]) __result.insert(__result.end(), __tracks[t].begin(), __tracks[t].end());
                for (size_t i = 0u; i < __used.size(); ++i) {
                    if (!__changed[i]) continue;
                    const std::vector<BoneKeyframe> &__keys = __tracks[__used[i].track];
                    size_t __src = 0u;
                    for (size_t t = 0u; t < __count; ++t) {
                        const uint32_t __frame = __from + (uint32_t)t;
                        while (__src + 1u < __keys.size() && __keys[__src + 1u].frame_id <= __frame) ++__src;
                        __result.push_back(fullKey(__keys[__src], __frame, __pos[i][t], __rot[i][t]));
                    }
                    ++__report.bones_written;
                }
                _vmd.keyframes_bone.swap(__result);
                __report.keys_after = (uint32_t)_vmd.keyframes_bone.size();
                return __report;
            }

            bool loadNoiseProfile(const std::string &_path, NoiseProfile &_out, std::string &_error) {
                std::ifstream __file(_path.c_str(), std::ios::binary);
                if (!__file) { _error = "Could not open '" + _path + "'"; return false; }
                std::stringstream __text;
                __text << __file.rdbuf();
                const std::string __str = __text.str();
                ContentJSON __json;
                __json.readFromString(__str.c_str());
                const ContentJSON::JSONNode &__root = __json.root_node;
                const ContentJSON::JSONNode *__noise = __root.find("noise");
                const ContentJSON::JSONNode *__bands = __noise != nullptr ? __noise->find("bands") : nullptr;
                const ContentJSON::JSONNode *__groups = __noise != nullptr ? __noise->find("groups") : nullptr;
                if (__bands == nullptr || __bands->node_type != JSON_TYPE_ARR || __groups == nullptr || __groups->node_type != JSON_TYPE_ARR) {
                    _error = "'" + _path + "' has no \"noise\": {\"bands\": [...], \"groups\": [...]}";
                    return false;
                }
                NoiseProfile __profile;
                for (size_t i = 0u; i < __bands->value_arr_obj.size(); ++i) {
                    std::vector<float> __edges;
                    if (!readFloats(__bands->value_arr_obj[i], __edges) || __edges.size() != 2u || !(__edges[0] > 0.f && __edges[1] > __edges[0])) {
                        _error = "'" + _path + "': noise band " + std::to_string(i) + " must be [low Hz, high Hz]";
                        return false;
                    }
                    __profile.bands_lo.push_back(__edges[0]);
                    __profile.bands_hi.push_back(__edges[1]);
                }
                const size_t __band_count = __profile.bands_lo.size();
                const ContentJSON::JSONNode *__scale = __noise->find("band_scale");
                if (__scale == nullptr || !readFloats(*__scale, __profile.band_scale) || __profile.band_scale.size() != __band_count)
                    __profile.band_scale.assign(__band_count, 1.f);
                __profile.band_follows_speed.assign(__band_count, 1u);
                const ContentJSON::JSONNode *__follows = __noise->find("band_follows_speed");
                if (__follows != nullptr && __follows->node_type == JSON_TYPE_ARR && __follows->value_arr_obj.size() == __band_count)
                    for (size_t b = 0u; b < __band_count; ++b) __profile.band_follows_speed[b] = __follows->value_arr_obj[b].node_type == JSON_TYPE_BOOL && !__follows->value_arr_obj[b].value_bool ? 0u : 1u;
                __profile.speed_window = float(__noise->getNum("speed_window", double(__profile.speed_window)));
                for (size_t i = 0u; i < __groups->value_arr_obj.size(); ++i) {
                    const ContentJSON::JSONNode &__g = __groups->value_arr_obj[i];
                    NoiseGroup __group;
                    __group.name     = __g.getStr("name", "group " + std::to_string(i));
                    __group.position = __g.getBool("position", false);
                    __group.min_speed = float(__g.getNum("min_speed", double(__group.min_speed)));
                    __group.max_speed = float(__g.getNum("max_speed", double(__group.max_speed)));
                    const ContentJSON::JSONNode *__bones = __g.find("bones"), *__rest = __g.find("rest"), *__per = __g.find("per_speed");
                    if (__bones != nullptr && __bones->node_type == JSON_TYPE_ARR)
                        for (size_t b = 0u; b < __bones->value_arr_obj.size(); ++b)
                            if (__bones->value_arr_obj[b].node_type == JSON_TYPE_STR) __group.bones.push_back(__bones->value_arr_obj[b].value_str);
                    if (__group.bones.empty() || __rest == nullptr || __per == nullptr || !readFloats(*__rest, __group.rest) || !readFloats(*__per, __group.per_speed) ||
                        __group.rest.size() != __band_count || __group.per_speed.size() != __band_count) {
                        _error = "'" + _path + "': noise group '" + __group.name + "' needs \"bones\" and \"rest\", \"per_speed\" for every band";
                        return false;
                    }
                    vec3f __axes;
                    if (readVec3(__g.find("axes"), __axes)) {
                        const float __len = std::sqrt(__axes.x * __axes.x + __axes.y * __axes.y + __axes.z * __axes.z);
                        if (__len > 0.f) __group.axes = vec3f{ __axes.x / __len, __axes.y / __len, __axes.z / __len };
                    }
                    __profile.groups.push_back(__group);
                }
                const ContentJSON::JSONNode *__links = __noise->find("links");
                if (__links != nullptr && __links->node_type == JSON_TYPE_ARR)
                    for (size_t i = 0u; i < __links->value_arr_obj.size(); ++i) {
                        const ContentJSON::JSONNode &__l = __links->value_arr_obj[i];
                        NoiseLink __link;
                        __link.parent = __l.getStr("parent", "");
                        __link.child  = __l.getStr("child",  "");
                        __link.share  = std::max(0.f, std::min(1.f, float(__l.getNum("share", 0.))));
                        if (!__link.parent.empty() && !__link.child.empty() && __link.share > 0.f) __profile.links.push_back(__link);
                    }
                const ContentJSON::JSONNode *__breath = __root.find("breathing");
                if (__breath != nullptr) {
                    __profile.breath_rest_hz      = float(__breath->getNum("rest_hz",      double(__profile.breath_rest_hz)));
                    __profile.breath_active_hz    = float(__breath->getNum("active_hz",    double(__profile.breath_active_hz)));
                    __profile.breath_active_speed = float(__breath->getNum("active_speed", double(__profile.breath_active_speed)));
                    const ContentJSON::JSONNode *__bones = __breath->find("bones");
                    if (__bones != nullptr && __bones->node_type == JSON_TYPE_ARR)
                        for (size_t b = 0u; b < __bones->value_arr_obj.size(); ++b) {
                            const ContentJSON::JSONNode &__n = __bones->value_arr_obj[b];
                            BreathBone __bone;
                            __bone.bone = __n.getStr("bone", "");
                            if (!__bone.bone.empty() && readVec3(__n.find("degrees"), __bone.degrees)) __profile.breath.push_back(__bone);
                        }
                }
                _out = __profile;
                return true;
            }


            NoiseReport addNoise(ContentVMD &_vmd, const NoiseProfile &_profile, const NoiseSettings &_settings) {
                NoiseReport __report;
                __report.keys_before = __report.keys_after = (uint32_t)_vmd.keyframes_bone.size();

                std::map<std::string, size_t> __track_ids;
                std::vector<std::vector<BoneKeyframe>> __tracks;
                collectTracks(_vmd, __track_ids, __tracks);
                if (__tracks.empty()) return __report;
                uint32_t __from = UINT32_MAX, __to = 0u;
                for (size_t t = 0u; t < __tracks.size(); ++t) {
                    __from = std::min(__from, __tracks[t].front().frame_id);
                    __to   = std::max(__to,   __tracks[t].back ().frame_id);
                }
                const size_t __count = __to - __from + 1u;
                if (__count < 2u) return __report;

                // The bones changed: noise group (or none) and breath amplitude
                struct Target { size_t track; std::string name; const NoiseGroup *group; vec3f breath; Baked baked; };
                std::vector<Target> __targets;
                const auto __target = [&](const std::string &_name) -> Target* {
                    std::map<std::string, size_t>::const_iterator __it = __track_ids.find(rawBoneName(_name));
                    if (__it == __track_ids.end()) return nullptr;
                    for (size_t i = 0u; i < __targets.size(); ++i) if (__targets[i].track == __it->second) return &__targets[i];
                    Target __t;
                    __t.track  = __it->second;
                    __t.name   = _name;
                    __t.group  = nullptr;
                    __t.breath = vec3f{ 0.f, 0.f, 0.f };
                    __targets.push_back(__t);
                    return &__targets.back();
                };
                const float __strength = std::max(0.f, _settings.strength), __breathing = std::max(0.f, _settings.breathing);
                if (__strength > 0.f)
                    for (size_t g = 0u; g < _profile.groups.size(); ++g)
                        for (size_t b = 0u; b < _profile.groups[g].bones.size(); ++b) {
                            Target *__t = __target(_profile.groups[g].bones[b]);
                            if (__t != nullptr) __t->group = &_profile.groups[g];
                        }
                if (__breathing > 0.f)
                    for (size_t b = 0u; b < _profile.breath.size(); ++b) {
                        Target *__t = __target(_profile.breath[b].bone);
                        if (__t != nullptr) __t->breath = _profile.breath[b].degrees * __breathing;
                    }
                if (__targets.empty()) return __report;
                for (size_t i = 0u; i < __targets.size(); ++i) bake(__tracks[__targets[i].track], __from, __to, __targets[i].baked);

                // Velocities (degrees or units per second, local axes) and speeds averaged over the window; the upper body's
                // speed sets the breath rate
                const float __fps = 30.f, __deg = 57.29578f;
                const float __sigma = 0.5f * _profile.speed_window * __fps;
                std::vector<std::vector<vec3f>> __velocity(__targets.size());
                std::vector<std::vector<float>> __speed(__targets.size());
                std::vector<float> __activity(__count, 0.f);
                uint32_t __active_bones = 0u;
                for (size_t i = 0u; i < __targets.size(); ++i) {
                    const Baked &__b = __targets[i].baked;
                    const bool __pos = __targets[i].group != nullptr && __targets[i].group->position;
                    std::vector<vec3f> &__v = __velocity[i];
                    std::vector<float> &__sp = __speed[i];
                    __v.assign(__count, vec3f{ 0.f, 0.f, 0.f });
                    __sp.assign(__count, 0.f);
                    for (size_t t = 0u; t + 1u < __count; ++t)
                        __v[t] = __pos ? (__b.position[t + 1u] - __b.position[t]) * __fps
                                       : Affine::Rotation::rotationVector(__b.rotation[t].conjugate() * __b.rotation[t + 1u]) * (__fps * __deg);
                    __v[__count - 1u] = __v[__count - 2u];
                    for (size_t t = 0u; t < __count; ++t) __sp[t] = std::sqrt(__v[t].x * __v[t].x + __v[t].y * __v[t].y + __v[t].z * __v[t].z);
                    if (!__pos) {
                        for (size_t t = 0u; t < __count; ++t) __activity[t] += __sp[t];
                        ++__active_bones;
                    }
                    smoothScalar(__sp, __sigma);
                }
                if (__active_bones > 0u) for (size_t t = 0u; t < __count; ++t) __activity[t] /= float(__active_bones);
                smoothScalar(__activity, 2.f * __fps);

                // Breath: 0..1, the inhale faster than the exhale; the rate follows the activity and drifts a little
                const Noise::NoisePerlin __noise(_settings.seed);
                std::vector<float> __breath(__count, 0.f);
                double __phase = 0.;
                for (size_t t = 0u; t < __count; ++t) {
                    const float __act  = std::min(1.f, __activity[t] / std::max(1e-3f, _profile.breath_active_speed));
                    const float __rate = (_profile.breath_rest_hz + (_profile.breath_active_hz - _profile.breath_rest_hz) * __act) *
                                         (1.f + 0.3f * __noise.sample(vec4f{ float(__from + t) * 0.1f / __fps, 91.5f, 0.37f, 0.71f }));
                    const double __x = __phase - std::floor(__phase);
                    const double __u = __x + 0.08 * std::sin(6.283185307 * __x);
                    __breath[t] = float(0.5 - 0.5 * std::cos(6.283185307 * __u)) - 0.5f;
                    __phase += double(__rate) / double(__fps);
                }

                // Noise: per band one Perlin line per axis (its spectrum is centered on 0.24 cycles per cell) limited to the
                // band, scaled so that its velocity in the band has the rms 1, times what the motion lacks in the band:
                // sqrt(target^2 - current^2), both rms over the window
                // Links: the noise source of a child mixes in its parent's (the parent must be noised too)
                std::vector<int64_t> __link_parent(__targets.size(), -1);
                std::vector<float>   __link_share(__targets.size(), 0.f);
                for (size_t l = 0u; l < _profile.links.size(); ++l) {
                    int64_t __p = -1, __c = -1;
                    for (size_t i = 0u; i < __targets.size(); ++i) {
                        if (__targets[i].group == nullptr) continue;
                        if (__targets[i].name == _profile.links[l].parent) __p = (int64_t)i;
                        if (__targets[i].name == _profile.links[l].child)  __c = (int64_t)i;
                    }
                    if (__p < 0 || __c < 0 || __p == __c) continue;
                    __link_parent[__c] = __p;
                    __link_share[__c]  = _profile.links[l].share;
                }
                const auto __source = [&](size_t _target, size_t _frame, uint32_t _axis, size_t _band, float _cells) -> float {
                    return __noise.sample(vec4f{ float(__from + _frame) * _cells, 0.5f + 7.31f * float(_target) + 1.93f * float(_axis), 0.37f + 2.17f * float(_band), 0.71f });
                };

                std::vector<vec3f> __band, __shape, __shape_v;
                std::vector<float> __have(__count), __lack(__count);
                std::vector<vec3f> __added(__count);
                for (size_t i = 0u; i < __targets.size(); ++i) {
                    Target &__t = __targets[i];
                    for (size_t f = 0u; f < __count; ++f) __added[f] = vec3f{ 0.f, 0.f, 0.f };
                    if (__t.group != nullptr) {
                        const NoiseGroup &__g = *__t.group;
                        for (size_t b = 0u; b < _profile.bands_lo.size(); ++b) {
                            const float __scale = __strength * _profile.band_scale[b];
                            if (__scale <= 0.f) continue;
                            const float __lo = _profile.bands_lo[b], __hi = _profile.bands_hi[b];
                            const bool  __follows = b >= _profile.band_follows_speed.size() || _profile.band_follows_speed[b] != 0u;
                            // What the motion has and lacks in the band
                            bandpass(__velocity[i], __lo, __hi, __fps, __band);
                            for (size_t f = 0u; f < __count; ++f) __have[f] = __band[f].x * __band[f].x + __band[f].y * __band[f].y + __band[f].z * __band[f].z;
                            smoothScalar(__have, __sigma);
                            bool __any = false;
                            for (size_t f = 0u; f < __count; ++f) {
                                const float __sp = __follows ? std::max(__g.min_speed, std::min(__g.max_speed, __speed[i][f])) : __g.min_speed;
                                const float __target = __g.rest[b] + __g.per_speed[b] * __sp;
                                __lack[f] = __scale * std::sqrt(std::max(0.f, __target * __target - std::max(0.f, __have[f])));
                                __any = __any || __lack[f] > 0.f;
                            }
                            if (!__any) continue;
                            // The noise shape: band limited, its band velocity of rms 1 (split over the axes by their shares)
                            const float __cells = std::sqrt(__lo * __hi) / 0.243f / __fps;
                            __shape.resize(__count);
                            const int64_t __parent = __link_parent[i];
                            const float   __share  = __link_share[i], __own = std::sqrt(1.f - __share * __share);
                            for (size_t f = 0u; f < __count; ++f)
                                for (uint32_t k = 0u; k < 3u; ++k)
                                    __shape[f][k] = __parent < 0 ? __source(i, f, k, b, __cells)
                                                                 : __own * __source(i, f, k, b, __cells) + __share * __source((size_t)__parent, f, k, b, __cells);
                            bandpass(__shape, __lo, __hi, __fps, __band);
                            __shape_v.resize(__count);
                            for (size_t f = 0u; f + 1u < __count; ++f) __shape_v[f] = (__band[f + 1u] - __band[f]) * __fps;
                            __shape_v[__count - 1u] = __shape_v[__count - 2u];
                            for (uint32_t k = 0u; k < 3u; ++k) {
                                double __sum = 0.;
                                for (size_t f = 0u; f < __count; ++f) __sum += double(__shape_v[f][k]) * double(__shape_v[f][k]);
                                const float __rms = float(std::sqrt(__sum / double(__count)));
                                if (__rms <= 1e-9f || __g.axes[k] <= 0.f) continue;
                                const float __k = __g.axes[k] / __rms;
                                for (size_t f = 0u; f < __count; ++f) __added[f][k] += __k * __lack[f] * __band[f][k];
                            }
                        }
                    }
                    for (size_t f = 0u; f < __count; ++f) __added[f] = __added[f] + __t.breath * __breath[f];

                    double __sum = 0.;
                    const bool __pos = __t.group != nullptr && __t.group->position;
                    for (size_t f = 0u; f < __count; ++f) {
                        const vec3f &__a = __added[f];
                        __sum += double(__a.x * __a.x + __a.y * __a.y + __a.z * __a.z);
                        if (__pos) __t.baked.position[f] = __t.baked.position[f] + __a;
                        else       __t.baked.rotation[f] = __t.baked.rotation[f] * Affine::Rotation::fromRotationVector(__a * (1.f / __deg));
                    }
                    const float __rms = float(std::sqrt(__sum / double(__count)));
                    if (!__pos && __rms > __report.max_rms_deg) { __report.max_rms_deg = __rms; __report.max_bone = __t.name; }
                    if (__t.group != nullptr) ++__report.bones_noised;
                    if (__t.breath.x != 0.f || __t.breath.y != 0.f || __t.breath.z != 0.f) ++__report.bones_breathing;
                }

                // The changed tracks get one key per frame over the frames of the motion
                std::vector<uint8_t> __replace(__tracks.size(), 0u);
                for (size_t i = 0u; i < __targets.size(); ++i) __replace[__targets[i].track] = 1u;
                std::vector<BoneKeyframe> __result;
                __result.reserve(_vmd.keyframes_bone.size() + __count * __targets.size());
                for (size_t t = 0u; t < __tracks.size(); ++t)
                    if (!__replace[t]) __result.insert(__result.end(), __tracks[t].begin(), __tracks[t].end());
                for (size_t i = 0u; i < __targets.size(); ++i) {
                    const std::vector<BoneKeyframe> &__keys = __tracks[__targets[i].track];
                    const Baked &__b = __targets[i].baked;
                    size_t __src = 0u;
                    for (size_t t = 0u; t < __count; ++t) {
                        const uint32_t __frame = __from + (uint32_t)t;
                        while (__src + 1u < __keys.size() && __keys[__src + 1u].frame_id <= __frame) ++__src;
                        __result.push_back(fullKey(__keys[__src], __frame, __b.position[t], __b.rotation[t]));
                    }
                }
                _vmd.keyframes_bone.swap(__result);
                __report.keys_after = (uint32_t)_vmd.keyframes_bone.size();
                return __report;
            }
        }; // VMDMocap END
    }; // File END
}; // Simple END
