#include "scene/scene_graph.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>


namespace Simple {
    namespace SceneGraph {

        mat4 translation(const vec3 &_offset) {
            mat4 __m = Mat::Eye4;
            __m.M[3][0] = _offset.x; __m.M[3][1] = _offset.y; __m.M[3][2] = _offset.z;
            return __m;
        }

        mat4 scaling(const vec3 &_scale) {
            mat4 __m = Mat::Eye4;
            __m.M[0][0] = _scale.x; __m.M[1][1] = _scale.y; __m.M[2][2] = _scale.z;
            return __m;
        }

        mat4 lookAlong(const vec3 &_dir, const vec3 &_up) {
            vec3 __z = _dir.normalize();
            vec3 __x = Math::cross(_up, __z);
            if (__x.norm() < 1e-6f) // '_dir' is parallel to '_up'
                __x = Math::cross(vec3{1.f, 0.f, 0.f}, __z);
            __x = __x.normalize();
            vec3 __y = Math::cross(__z, __x);
            // Note: rows are the images of the local axes (row-vector style)
            return { __x.x, __x.y, __x.z, 0.f,
                     __y.x, __y.y, __y.z, 0.f,
                     __z.x, __z.y, __z.z, 0.f,
                     0.f,   0.f,   0.f,   1.f };
        }

        vec3 transformPoint(const mat4 &_m, const vec3 &_p) {
            return { _p.x * _m.M[0][0] + _p.y * _m.M[1][0] + _p.z * _m.M[2][0] + _m.M[3][0],
                     _p.x * _m.M[0][1] + _p.y * _m.M[1][1] + _p.z * _m.M[2][1] + _m.M[3][1],
                     _p.x * _m.M[0][2] + _p.y * _m.M[1][2] + _p.z * _m.M[2][2] + _m.M[3][2] };
        }

        vec3 transformDir(const mat4 &_m, const vec3 &_d) {
            return { _d.x * _m.M[0][0] + _d.y * _m.M[1][0] + _d.z * _m.M[2][0],
                     _d.x * _m.M[0][1] + _d.y * _m.M[1][1] + _d.z * _m.M[2][1],
                     _d.x * _m.M[0][2] + _d.y * _m.M[1][2] + _d.z * _m.M[2][2] };
        }


        mat4 rotation(const Quat &_q) {
            const Quat  __q = _q.normalize();
            const float s = __q.s, i = __q.i, j = __q.j, k = __q.k;
            // Note: the transpose of the usual column-vector style matrix
            return { 1.f - 2.f * (j * j + k * k),       2.f * (i * j + s * k),       2.f * (i * k - s * j), 0.f,
                           2.f * (i * j - s * k), 1.f - 2.f * (i * i + k * k),       2.f * (j * k + s * i), 0.f,
                           2.f * (i * k + s * j),       2.f * (j * k - s * i), 1.f - 2.f * (i * i + j * j), 0.f,
                     0.f, 0.f, 0.f, 1.f };
        }

        Quat rotationOf(const mat4 &_m) {
            // Note: c(r, c) is the usual column-vector style matrix (the transpose of '_m'), rows normalized to drop the scale
            float __len[3];
            for (int r = 0; r < 3; ++r) {
                __len[r] = std::sqrt(_m.M[r][0] * _m.M[r][0] + _m.M[r][1] * _m.M[r][1] + _m.M[r][2] * _m.M[r][2]);
                if (__len[r] < 1e-12f) __len[r] = 1.f;
            }
            float c[3][3];
            for (int r = 0; r < 3; ++r)
                for (int col = 0; col < 3; ++col) c[r][col] = _m.M[col][r] / __len[col];
            const float __trace = c[0][0] + c[1][1] + c[2][2];
            Quat __q;
            if (__trace > 0.f) {
                float __t = std::sqrt(__trace + 1.f) * 2.f;
                __q = Quat(0.25f * __t, (c[2][1] - c[1][2]) / __t, (c[0][2] - c[2][0]) / __t, (c[1][0] - c[0][1]) / __t);
            } else if (c[0][0] > c[1][1] && c[0][0] > c[2][2]) {
                float __t = std::sqrt(1.f + c[0][0] - c[1][1] - c[2][2]) * 2.f;
                __q = Quat((c[2][1] - c[1][2]) / __t, 0.25f * __t, (c[0][1] + c[1][0]) / __t, (c[0][2] + c[2][0]) / __t);
            } else if (c[1][1] > c[2][2]) {
                float __t = std::sqrt(1.f + c[1][1] - c[0][0] - c[2][2]) * 2.f;
                __q = Quat((c[0][2] - c[2][0]) / __t, (c[0][1] + c[1][0]) / __t, 0.25f * __t, (c[1][2] + c[2][1]) / __t);
            } else {
                float __t = std::sqrt(1.f + c[2][2] - c[0][0] - c[1][1]) * 2.f;
                __q = Quat((c[1][0] - c[0][1]) / __t, (c[0][2] + c[2][0]) / __t, (c[1][2] + c[2][1]) / __t, 0.25f * __t);
            }
            return __q.normalize();
        }

        mat4 TRS::matrix() const {
            mat4 __m = SceneGraph::rotation(rotation);
            const float __s[3] = { scale.x, scale.y, scale.z };
            for (int r = 0; r < 3; ++r)
                for (int c = 0; c < 3; ++c) __m.M[r][c] *= __s[r];
            __m.M[3][0] = translation.x; __m.M[3][1] = translation.y; __m.M[3][2] = translation.z;
            return __m;
        }

        TRS TRS::fromMatrix(const mat4 &_m) {
            TRS __trs;
            __trs.translation = { _m.M[3][0], _m.M[3][1], _m.M[3][2] };
            float __s[3];
            for (int r = 0; r < 3; ++r) __s[r] = std::sqrt(_m.M[r][0] * _m.M[r][0] + _m.M[r][1] * _m.M[r][1] + _m.M[r][2] * _m.M[r][2]);
            // A mirroring matrix: the mirror goes into the X scale
            const float __det = _m.M[0][0] * (_m.M[1][1] * _m.M[2][2] - _m.M[1][2] * _m.M[2][1])
                              - _m.M[0][1] * (_m.M[1][0] * _m.M[2][2] - _m.M[1][2] * _m.M[2][0])
                              + _m.M[0][2] * (_m.M[1][0] * _m.M[2][1] - _m.M[1][1] * _m.M[2][0]);
            mat4 __r = _m;
            if (__det < 0.f) {
                __s[0] = -__s[0];
                __r.M[0][0] = -__r.M[0][0]; __r.M[0][1] = -__r.M[0][1]; __r.M[0][2] = -__r.M[0][2];
            }
            __trs.scale    = { __s[0], __s[1], __s[2] };
            __trs.rotation = rotationOf(__r);
            return __trs;
        }

//  == === ==== >  AABB

        bool AABB::valid() const { return min.x <= max.x && min.y <= max.y && min.z <= max.z; }

        void AABB::add(const vec3 &_p) {
            min.x = std::min(min.x, _p.x); min.y = std::min(min.y, _p.y); min.z = std::min(min.z, _p.z);
            max.x = std::max(max.x, _p.x); max.y = std::max(max.y, _p.y); max.z = std::max(max.z, _p.z);
        }

        void AABB::add(const AABB &_box) {
            if (!_box.valid()) return;
            add(_box.min);
            add(_box.max);
        }

        vec3 AABB::center() const { return (min + max) * 0.5f; }

        vec3 AABB::size() const { return valid() ? max - min : vec3{0.f, 0.f, 0.f}; }

        AABB AABB::transformed(const mat4 &_m) const {
            AABB __res;
            if (!valid()) return __res;
            for (uint32_t i = 0u; i < 8u; ++i)
                __res.add(transformPoint(_m, { (i & 1u) ? max.x : min.x, (i & 2u) ? max.y : min.y, (i & 4u) ? max.z : min.z }));
            return __res;
        }


//  == === ==== >  Mesh, Texture, Light

        void Mesh::computeBounds() {
            bounds = AABB();
            for (size_t i = 0u; i < vertices.size(); ++i)
                bounds.add(vertices[i].position);
        }

        void Texture::scanAlpha() {
            has_alpha = soft_alpha = false;
            for (size_t i = 0u; i < image.img.size(); ++i) {
                uint8_t __a = image.img[i].w;
                if (__a != 255u) {
                    has_alpha = true;
                    if (__a != 0u) { soft_alpha = true; return; }
                }
            }
        }

        Light Light::directional(const vec3 &_color, float _intensity) {
            Light __l;
            __l.type = SCENE_LIGHT_DIRECTIONAL; __l.color = _color; __l.intensity = _intensity;
            return __l;
        }

        Light Light::point(const vec3 &_color, float _intensity, float _range) {
            Light __l;
            __l.type = SCENE_LIGHT_POINT; __l.color = _color; __l.intensity = _intensity; __l.range = _range;
            return __l;
        }

        Light Light::spot(const vec3 &_color, float _intensity, float _inner, float _outer, float _range) {
            Light __l;
            __l.type = SCENE_LIGHT_SPOT; __l.color = _color; __l.intensity = _intensity;
            __l.inner_cone = _inner; __l.outer_cone = _outer; __l.range = _range;
            return __l;
        }


//  == === ==== >  Skeleton, Pose

        uint32_t Skeleton::addBone(const Bone &_bone) {
            SVKFW_ASSERT(_bone.parent == SCENE_NONE || _bone.parent < bones.size(), std::out_of_range, "SceneGraph::Skeleton :: addBone",
                            "the parent of '" + _bone.name + "' must be added first");
            bones.push_back(_bone);
            return (uint32_t)bones.size() - 1u;
        }

        uint32_t Skeleton::findBone(const std::string &_name) const {
            for (size_t i = 0u; i < bones.size(); ++i)
                if (bones[i].name == _name) return (uint32_t)i;
            return SCENE_NONE;
        }

        uint32_t Skeleton::addIK(const IKChain &_ik) {
            SVKFW_ASSERT(_ik.goal < bones.size() && _ik.effector < bones.size(), std::out_of_range, "SceneGraph::Skeleton :: addIK",
                            "bad goal or effector bone in '" + _ik.name + "'");
            for (size_t l = 0u; l < _ik.links.size(); ++l)
                SVKFW_ASSERT(_ik.links[l].bone < bones.size(), std::out_of_range, "SceneGraph::Skeleton :: addIK", "bad link in '" + _ik.name + "'");
            iks.push_back(_ik);
            bones[_ik.goal].ik = (uint32_t)iks.size() - 1u;
            return (uint32_t)iks.size() - 1u;
        }

        void Skeleton::finalize() {
            computeInverseBind();
            children.assign(bones.size(), std::vector<uint32_t>());
            for (uint32_t i = 0u; i < (uint32_t)bones.size(); ++i)
                if (bones[i].parent != SCENE_NONE) children[bones[i].parent].push_back(i);
            order.resize(bones.size());
            for (uint32_t i = 0u; i < (uint32_t)bones.size(); ++i) order[i] = i;
            struct DeformOrder {
                const std::vector<Bone> &bones;
                bool operator()(uint32_t _a, uint32_t _b) const {
                    if (bones[_a].after_physics != bones[_b].after_physics) return !bones[_a].after_physics;
                    if (bones[_a].layer != bones[_b].layer) return bones[_a].layer < bones[_b].layer;
                    return _a < _b;
                }
            };
            std::sort(order.begin(), order.end(), DeformOrder{bones});
        }

        void Skeleton::computeInverseBind() {
            std::vector<mat4> __bind(bones.size());
            for (size_t i = 0u; i < bones.size(); ++i) {
                __bind[i] = bones[i].parent == SCENE_NONE ? bones[i].rest.matrix() : bones[i].rest.matrix() & __bind[bones[i].parent];
                bones[i].inverse_bind = __bind[i].inverse<float>();
            }
        }

        namespace {
            // '_t' of the rotation (any real '_t': negative ratios give the inverse direction, like MMD grants)
            Quat scaleRotation(const Quat &_q, float _t) {
                Quat __q = _q.normalize();
                if (__q.s < 0.f) __q = Quat(-__q.s, -__q.i, -__q.j, -__q.k);
                const float __v = std::sqrt(__q.i * __q.i + __q.j * __q.j + __q.k * __q.k);
                if (__v < 1e-7f) return Quat();
                const float __angle = 2.f * std::atan2(__v, __q.s) * _t;
                return Quat(vec3{__q.i / __v, __q.j / __v, __q.k / __v}, __angle);
            }

            vec3 origin(const mat4 &_m) { return { _m.M[3][0], _m.M[3][1], _m.M[3][2] }; }

            float clampf(float _v, float _min, float _max) { return _v < _min ? _min : (_v > _max ? _max : _v); }

            // Euler angles of 'q = qx * qy * qz' (Z is applied first); '_near' picks the solution closest to it
            vec3 eulerXYZ(const Quat &_q, const vec3 &_near) {
                const mat4 m = rotation(_q); // row style: m.M[r][c] is the usual c(c, r)
                const float __sy = clampf(m.M[2][0], -1.f, 1.f);
                vec3 __a, __b;
                if (std::fabs(__sy) > 0.99999f) { // gimbal lock: keep X
                    __a.x = _near.x;
                    __a.y = std::asin(__sy);
                    // with x fixed: z from the remaining rotation
                    const mat4 __rest = rotation(Quat(vec3{1.f, 0.f, 0.f}, __a.x).inverse() * _q);
                    __a.z = std::atan2(__rest.M[0][1], __rest.M[1][1]);
                    return __a;
                }
                __a.y = std::asin(__sy);
                __a.x = std::atan2(-m.M[2][1], m.M[2][2]);
                __a.z = std::atan2(-m.M[1][0], m.M[0][0]);
                const float __pi = 3.14159265f;
                __b.x = __a.x + (__a.x > 0.f ? -__pi : __pi);
                __b.y = (__a.y > 0.f ? __pi : -__pi) - __a.y;
                __b.z = __a.z + (__a.z > 0.f ? -__pi : __pi);
                const vec3 __da = __a - _near, __db = __b - _near;
                return Math::dot(__da, __da) <= Math::dot(__db, __db) ? __a : __b;
            }

            Quat fromEulerXYZ(const vec3 &_a) {
                return Quat(vec3{1.f, 0.f, 0.f}, _a.x) * Quat(vec3{0.f, 1.f, 0.f}, _a.y) * Quat(vec3{0.f, 0.f, 1.f}, _a.z);
            }
        } // anonymous namespace END

        void Pose::reset(const Skeleton &_skeleton) {
            const size_t __count = _skeleton.bones.size();
            local.resize(__count);
            for (size_t i = 0u; i < __count; ++i) local[i] = _skeleton.bones[i].rest;
            ik_rotation.assign(__count, Quat());
            grant_rotation.assign(__count, Quat());
            grant_translation.assign(__count, vec3{0.f, 0.f, 0.f});
            model.resize(__count, Mat::Eye4);
            skinning.resize(__count, Mat::Eye4);
            dirty = true;
        }

        mat4 Pose::localMatrix_(const Skeleton &_skeleton, uint32_t _bone) const {
            TRS __trs = local[_bone];
            __trs.rotation    = ik_rotation[_bone] * local[_bone].rotation * grant_rotation[_bone];
            __trs.translation = __trs.translation + grant_translation[_bone];
            const uint32_t __parent = _skeleton.bones[_bone].parent;
            return __parent == SCENE_NONE ? __trs.matrix() : __trs.matrix() & model[__parent];
        }

        void Pose::updateModel_(const Skeleton &_skeleton, uint32_t _bone) {
            std::vector<uint32_t> __stack(1u, _bone);
            while (!__stack.empty()) {
                const uint32_t __b = __stack.back();
                __stack.pop_back();
                model[__b] = localMatrix_(_skeleton, __b);
                if (__b < _skeleton.children.size())
                    __stack.insert(__stack.end(), _skeleton.children[__b].begin(), _skeleton.children[__b].end());
            }
        }

        void Pose::applyGrant_(const Skeleton &_skeleton, uint32_t _bone) {
            const Bone &__bone = _skeleton.bones[_bone];
            const uint32_t g = __bone.grant_parent;
            const Bone &__source = _skeleton.bones[g];
            // Note: a grant of a granted bone takes its grant only (MMD), otherwise the animated rotation with the IK
            if (__bone.grant_rotation) {
                Quat __rotation = __source.grant_rotation ? grant_rotation[g] : local[g].rotation * __source.rest.rotation.inverse();
                __rotation = ik_rotation[g] * __rotation;
                grant_rotation[_bone] = scaleRotation(__rotation, __bone.grant_ratio);
            }
            if (__bone.grant_translation) {
                const vec3 __translation = __source.grant_translation ? grant_translation[g] : local[g].translation - __source.rest.translation;
                grant_translation[_bone] = __translation * __bone.grant_ratio;
            }
        }

        void Pose::rotateLink_(const Skeleton &_skeleton, uint32_t _bone, const vec3 &_axis, float _angle) {
            const Quat __animated = local[_bone].rotation;
            const Quat __total = ik_rotation[_bone] * __animated * Quat(_axis, _angle);
            ik_rotation[_bone] = (__total * __animated.inverse()).normalize();
            updateModel_(_skeleton, _bone);
        }

        void Pose::limitLink_(const Skeleton &_skeleton, const IKLink &_link, float _limit_angle, vec3 &_previous) {
            const Quat __animated = local[_link.bone].rotation;
            const Quat __total = ik_rotation[_link.bone] * __animated;
            vec3 __angles = eulerXYZ(__total, _previous);
            __angles = { clampf(__angles.x, _link.limit_min.x, _link.limit_max.x),
                         clampf(__angles.y, _link.limit_min.y, _link.limit_max.y),
                         clampf(__angles.z, _link.limit_min.z, _link.limit_max.z) };
            __angles = { _previous.x + clampf(__angles.x - _previous.x, -_limit_angle, _limit_angle),
                         _previous.y + clampf(__angles.y - _previous.y, -_limit_angle, _limit_angle),
                         _previous.z + clampf(__angles.z - _previous.z, -_limit_angle, _limit_angle) };
            _previous = __angles;
            ik_rotation[_link.bone] = (fromEulerXYZ(__angles) * __animated.inverse()).normalize();
            updateModel_(_skeleton, _link.bone);
        }

        void Pose::solveCCD_(const Skeleton &_skeleton, const IKChain &_ik) {
            const size_t __links = _ik.links.size();
            std::vector<vec3>  __previous(__links, vec3{0.f, 0.f, 0.f});
            std::vector<float> __plane_angle(__links, 0.f);
            std::vector<Quat>  __best(__links);
            float __best_distance = 1e30f;
            for (uint32_t it = 0u; it < _ik.iterations; ++it) {
                for (size_t l = 0u; l < __links; ++l) {
                    const IKLink &__link = _ik.links[l];
                    if (__link.bone == _ik.effector) continue;
                    const mat4 __inverse = model[__link.bone].inverse<float>();
                    vec3 __to_goal     = transformPoint(__inverse, origin(model[_ik.goal]));
                    vec3 __to_effector = transformPoint(__inverse, origin(model[_ik.effector]));
                    if (__to_goal.norm() < 1e-7f || __to_effector.norm() < 1e-7f) continue;
                    __to_goal = __to_goal.normalize();
                    __to_effector = __to_effector.normalize();
                    float __angle = std::acos(clampf(Math::dot(__to_effector, __to_goal), -1.f, 1.f));

                    // A link limited around one axis only (knees): rotate in that plane, within the limits
                    int __axis = -1;
                    if (__link.limited) {
                        const bool __x = __link.limit_min.x != 0.f || __link.limit_max.x != 0.f;
                        const bool __y = __link.limit_min.y != 0.f || __link.limit_max.y != 0.f;
                        const bool __z = __link.limit_min.z != 0.f || __link.limit_max.z != 0.f;
                        if (__x && !__y && !__z) __axis = 0; else if (!__x && __y && !__z) __axis = 1; else if (!__x && !__y && __z) __axis = 2;
                    }
                    if (__axis >= 0) {
                        const vec3  __a   = { __axis == 0 ? 1.f : 0.f, __axis == 1 ? 1.f : 0.f, __axis == 2 ? 1.f : 0.f };
                        const float __min = __axis == 0 ? __link.limit_min.x : __axis == 1 ? __link.limit_min.y : __link.limit_min.z;
                        const float __max = __axis == 0 ? __link.limit_max.x : __axis == 1 ? __link.limit_max.y : __link.limit_max.z;
                        __angle = clampf(__angle, -_ik.limit_angle, _ik.limit_angle);
                        // Which direction brings the effector closer
                        const float __plus  = Math::dot(Quat(__a,  __angle).rotate(__to_effector), __to_goal);
                        const float __minus = Math::dot(Quat(__a, -__angle).rotate(__to_effector), __to_goal);
                        float __new = __plane_angle[l] + (__plus > __minus ? __angle : -__angle);
                        if (it == 0u && (__new < __min || __new > __max)) {
                            if (-__new > __min && -__new < __max) __new = -__new;
                            else {
                                const float __half = (__min + __max) * 0.5f;
                                if (std::fabs(__half - __new) > std::fabs(__half + __new)) __new = -__new;
                            }
                        }
                        __new = clampf(__new, __min, __max);
                        __plane_angle[l] = __new;
                        ik_rotation[__link.bone] = (Quat(__a, __new) * local[__link.bone].rotation.inverse()).normalize();
                        updateModel_(_skeleton, __link.bone);
                        continue;
                    }

                    if (__angle < 1e-5f) continue;
                    __angle = clampf(__angle, -_ik.limit_angle, _ik.limit_angle);
                    vec3 __axis_v = Math::cross(__to_effector, __to_goal);
                    if (__axis_v.norm() < 1e-7f) continue;
                    rotateLink_(_skeleton, __link.bone, __axis_v.normalize(), __angle);
                    if (__link.limited) limitLink_(_skeleton, __link, _ik.limit_angle, __previous[l]);
                }
                // Keep the best iteration (MMD): stop when the distance doesn't improve
                const vec3  __d = origin(model[_ik.goal]) - origin(model[_ik.effector]);
                const float __distance = Math::dot(__d, __d);
                if (__distance < __best_distance) {
                    __best_distance = __distance;
                    for (size_t l = 0u; l < __links; ++l) __best[l] = ik_rotation[_ik.links[l].bone];
                } else {
                    for (size_t l = 0u; l < __links; ++l) {
                        ik_rotation[_ik.links[l].bone] = __best[l];
                        updateModel_(_skeleton, _ik.links[l].bone);
                    }
                    break;
                }
                if (__distance < 1e-10f) break;
            }
        }

        void Pose::solveTwoBone_(const Skeleton &_skeleton, const IKChain &_ik) {
            if (_ik.links.size() != 2u) { solveCCD_(_skeleton, _ik); return; }
            const uint32_t __mid = _ik.links[0].bone, __root = _ik.links[1].bone;
            const vec3 a = origin(model[__root]), b = origin(model[__mid]), c = origin(model[_ik.effector]), t = origin(model[_ik.goal]);
            const float __upper = (b - a).norm(), __lower = (c - b).norm();
            if (__upper < 1e-7f || __lower < 1e-7f) return;
            // 1. The middle joint: the angle that makes the root-effector distance equal to the root-goal distance
            const float __d = clampf((t - a).norm(), std::fabs(__upper - __lower) + 1e-5f, __upper + __lower - 1e-5f);
            const vec3  __u = (a - b).normalize(), __w = (c - b).normalize();
            const float __current = std::acos(clampf(Math::dot(__u, __w), -1.f, 1.f));
            const float __wanted  = std::acos(clampf((__upper * __upper + __lower * __lower - __d * __d) / (2.f * __upper * __lower), -1.f, 1.f));
            vec3 __bend = Math::cross(__w, __u); // the bend plane of the current pose
            if (__bend.norm() < 1e-6f) __bend = Math::cross(__w, t - a);    // a straight limb: bend towards the goal
            if (__bend.norm() < 1e-6f) __bend = Math::cross(__w, vec3{0.f, 0.f, 1.f});
            if (__bend.norm() > 1e-7f) {
                const vec3 __local = transformDir(model[__mid].inverse<float>(), __bend.normalize());
                if (__local.norm() > 1e-7f) rotateLink_(_skeleton, __mid, __local.normalize(), __current - __wanted);
            }
            // 2. The root joint: turn the effector onto the goal
            const mat4 __inverse = model[__root].inverse<float>();
            vec3 __to_effector = transformPoint(__inverse, origin(model[_ik.effector])), __to_goal = transformPoint(__inverse, t);
            if (__to_effector.norm() < 1e-7f || __to_goal.norm() < 1e-7f) return;
            __to_effector = __to_effector.normalize();
            __to_goal = __to_goal.normalize();
            const vec3 __axis = Math::cross(__to_effector, __to_goal);
            if (__axis.norm() < 1e-7f) return;
            rotateLink_(_skeleton, __root, __axis.normalize(), std::acos(clampf(Math::dot(__to_effector, __to_goal), -1.f, 1.f)));
        }

        void Pose::computeFK(const Skeleton &_skeleton) {
            const size_t __count = _skeleton.bones.size();
            if (local.size() != __count || ik_rotation.size() != __count) reset(_skeleton);
            model.resize(__count);
            skinning.resize(__count);
            ik_rotation.assign(__count, Quat());
            grant_rotation.assign(__count, Quat());
            grant_translation.assign(__count, vec3{0.f, 0.f, 0.f});
            for (uint32_t i = 0u; i < (uint32_t)__count; ++i) model[i] = localMatrix_(_skeleton, i);
            // Grants and IK in the deform order (without 'finalize' there is none: plain FK)
            for (size_t o = 0u; o < _skeleton.order.size(); ++o) {
                const uint32_t __b = _skeleton.order[o];
                const Bone &__bone = _skeleton.bones[__b];
                if (__bone.grant_parent != SCENE_NONE && (__bone.grant_rotation || __bone.grant_translation)) {
                    applyGrant_(_skeleton, __b);
                    updateModel_(_skeleton, __b);
                }
                if (__bone.ik != SCENE_NONE && solve_ik && _skeleton.iks[__bone.ik].enabled) {
                    const IKChain &__ik = _skeleton.iks[__bone.ik];
                    if (__ik.solver == SCENE_IK_TWO_BONE) solveTwoBone_(_skeleton, __ik);
                    else solveCCD_(_skeleton, __ik);
                }
            }
            for (size_t i = 0u; i < __count; ++i) skinning[i] = _skeleton.bones[i].inverse_bind & model[i];
            dirty = false;
        }


//  == === ==== >  Scene

        uint32_t Scene::addNode(const std::string &_name, uint32_t _parent, const mat4 &_local) {
            SVKFW_ASSERT(_parent == SCENE_NONE || _parent < nodes.size(), std::out_of_range, "SceneGraph::Scene :: addNode",
                            "no parent node " + std::to_string(_parent));
            Node __node;
            __node.name   = _name;
            __node.parent = _parent;
            __node.local  = _local;
            nodes.push_back(__node);
            uint32_t __index = (uint32_t)nodes.size() - 1u;
            if (_parent != SCENE_NONE)
                nodes[_parent].children.push_back(__index);
            if (!full_update_ && depth_.size() == __index) {
                depth_.push_back(_parent == SCENE_NONE ? 0u : depth_[_parent] + 1u);
                dirty_.push_back(__index);
            } else
                full_update_ = true;
            return __index;
        }

        uint32_t Scene::addMesh(const Mesh &_mesh) {
            meshes.push_back(_mesh);
            return (uint32_t)meshes.size() - 1u;
        }

        uint32_t Scene::addMaterial(const Material &_material) {
            materials.push_back(_material);
            return (uint32_t)materials.size() - 1u;
        }

        uint32_t Scene::addTexture(const Texture &_texture) {
            textures.push_back(_texture);
            return (uint32_t)textures.size() - 1u;
        }

        uint32_t Scene::addLight(const Light &_light, uint32_t _node) {
            Node &__node = node(_node);
            lights.push_back(_light);
            uint32_t __index = (uint32_t)lights.size() - 1u;
            lights.back().node = _node;
            __node.light = __index;
            return __index;
        }

        uint32_t Scene::addSkeleton(const Skeleton &_skeleton) {
            skeletons.push_back(_skeleton);
            return (uint32_t)skeletons.size() - 1u;
        }

        uint32_t Scene::addSkin(uint32_t _skeleton, uint32_t _node, const std::string &_name) {
            SVKFW_ASSERT(_skeleton < skeletons.size(), std::out_of_range, "SceneGraph::Scene :: addSkin", "no skeleton " + std::to_string(_skeleton));
            node(_node);
            Skin __skin;
            __skin.name     = _name.empty() ? skeletons[_skeleton].name : _name;
            __skin.skeleton = _skeleton;
            __skin.node     = _node;
            __skin.pose.reset(skeletons[_skeleton]);
            __skin.pose.computeFK(skeletons[_skeleton]);
            skins.push_back(__skin);
            return (uint32_t)skins.size() - 1u;
        }

        Pose &Scene::pose(uint32_t _skin) {
            SVKFW_ASSERT(_skin < skins.size(), std::out_of_range, "SceneGraph::Scene :: pose", "no skin " + std::to_string(_skin));
            skins[_skin].pose.dirty = true;
            return skins[_skin].pose;
        }

        void Scene::updateSkins() {
            for (size_t i = 0u; i < skins.size(); ++i)
                if (skins[i].pose.dirty) skins[i].pose.computeFK(skeletons[skins[i].skeleton]);
        }

        Node &Scene::node(uint32_t _index) {
            SVKFW_ASSERT(_index < nodes.size(), std::out_of_range, "SceneGraph::Scene :: node", "no node " + std::to_string(_index));
            return nodes[_index];
        }

        const Node &Scene::node(uint32_t _index) const {
            SVKFW_ASSERT(_index < nodes.size(), std::out_of_range, "SceneGraph::Scene :: node", "no node " + std::to_string(_index));
            return nodes[_index];
        }

        uint32_t Scene::findNode(const std::string &_name) const {
            for (size_t i = 0u; i < nodes.size(); ++i)
                if (nodes[i].name == _name) return (uint32_t)i;
            return SCENE_NONE;
        }

        void Scene::reparent(uint32_t _node, uint32_t _new_parent) {
            Node &__node = node(_node);
            for (uint32_t p = _new_parent; p != SCENE_NONE; p = node(p).parent)
                SVKFW_ASSERT(p != _node, std::invalid_argument, "SceneGraph::Scene :: reparent", "the new parent is a descendant of the node");
            if (__node.parent != SCENE_NONE) {
                std::vector<uint32_t> &__siblings = nodes[__node.parent].children;
                __siblings.erase(std::remove(__siblings.begin(), __siblings.end(), _node), __siblings.end());
            }
            __node.parent = _new_parent;
            if (_new_parent != SCENE_NONE)
                nodes[_new_parent].children.push_back(_node);
            if (!full_update_ && depth_.size() == nodes.size()) {
                updateDepth_(_node);
                dirty_.push_back(_node);
            } else
                full_update_ = true;
        }

        void Scene::setLocal(uint32_t _node, const mat4 &_local) {
            node(_node).local = _local;
            markDirty(_node);
        }

        void Scene::setVisible(uint32_t _node, bool _visible) {
            node(_node).visible = _visible;
            markDirty(_node);
        }

        void Scene::markDirty(uint32_t _node) {
            SVKFW_ASSERT(_node < nodes.size(), std::out_of_range, "SceneGraph::Scene :: markDirty", "no node " + std::to_string(_node));
            if (!full_update_) dirty_.push_back(_node);
        }

        void Scene::markAllDirty() {
            full_update_ = true;
        }

        void Scene::updateDepth_(uint32_t _index) {
            std::vector<uint32_t> __stack(1u, _index);
            while (!__stack.empty()) {
                uint32_t __n = __stack.back();
                __stack.pop_back();
                depth_[__n] = nodes[__n].parent == SCENE_NONE ? 0u : depth_[nodes[__n].parent] + 1u;
                __stack.insert(__stack.end(), nodes[__n].children.begin(), nodes[__n].children.end());
            }
        }

        void Scene::updateSubtree_(uint32_t _index) {
            // Note: depth first; a node is computed before its children are pushed, so the parent's world is always ready
            std::vector<uint32_t> __stack(1u, _index);
            while (!__stack.empty()) {
                uint32_t __n = __stack.back();
                __stack.pop_back();
                const Node &__node = nodes[__n];
                if (__node.parent == SCENE_NONE) {
                    world[__n]         = __node.local;
                    world_visible[__n] = __node.visible ? 1u : 0u;
                    depth_[__n]        = 0u;
                } else {
                    world[__n]         = __node.local & world[__node.parent];
                    world_visible[__n] = (__node.visible && world_visible[__node.parent]) ? 1u : 0u;
                    depth_[__n]        = depth_[__node.parent] + 1u;
                }
                stamp_[__n] = stamp_count_;
                changed.push_back(__n);
                __stack.insert(__stack.end(), __node.children.begin(), __node.children.end());
            }
        }

        void Scene::updateTransforms() {
            const uint32_t __count = (uint32_t)nodes.size();
            changed.clear();
            if (++stamp_count_ == 0u) { // wrapped: forget the old marks
                stamp_.assign(stamp_.size(), 0u);
                stamp_count_ = 1u;
            }
            stamp_.resize(__count, 0u);

            world.resize(__count);         // nodes added since the last update are marked
            world_visible.resize(__count);
            if (full_update_ || depth_.size() != __count) {
                depth_.assign(__count, 0u);
                changed.reserve(__count);
                for (uint32_t i = 0u; i < __count; ++i)
                    if (nodes[i].parent == SCENE_NONE) updateSubtree_(i);
                full_update_ = false;
                dirty_.clear();
                return;
            }

            // Shallow nodes first: the subtree of a marked node covers its marked descendants, which are then skipped
            struct ByDepth {
                const std::vector<uint32_t> &depth;
                bool operator()(uint32_t _a, uint32_t _b) const { return depth[_a] < depth[_b]; }
            };
            std::sort(dirty_.begin(), dirty_.end(), ByDepth{depth_});
            for (size_t i = 0u; i < dirty_.size(); ++i)
                if (stamp_[dirty_[i]] != stamp_count_) updateSubtree_(dirty_[i]);
            dirty_.clear();
        }

        AABB Scene::bounds() const {
            AABB __res;
            for (size_t n = 0u; n < nodes.size() && n < world.size(); ++n) {
                if (!world_visible[n]) continue;
                for (size_t m = 0u; m < nodes[n].meshes.size(); ++m) {
                    const Mesh &__mesh = meshes[nodes[n].meshes[m]];
                    if (__mesh.material != SCENE_NONE && !materials[__mesh.material].visible) continue;
                    __res.add(__mesh.bounds.transformed(world[n]));
                }
            }
            return __res;
        }

        uint32_t Scene::append(const Scene &_other, uint32_t _parent) {
            const uint32_t __node0 = (uint32_t)nodes.size(),     __mesh0 = (uint32_t)meshes.size();
            const uint32_t __mat0  = (uint32_t)materials.size(), __tex0  = (uint32_t)textures.size();
            const uint32_t __light0 = (uint32_t)lights.size();
            const uint32_t __skel0 = (uint32_t)skeletons.size(), __skin0 = (uint32_t)skins.size();

            skeletons.insert(skeletons.end(), _other.skeletons.begin(), _other.skeletons.end());
            for (size_t i = 0u; i < _other.skins.size(); ++i) {
                skins.push_back(_other.skins[i]);
                skins.back().skeleton += __skel0;
                skins.back().node      = (skins.back().node == SCENE_NONE ? SCENE_NONE : skins.back().node + __node0);
            }

            textures.insert(textures.end(), _other.textures.begin(), _other.textures.end());
            for (size_t i = 0u; i < _other.materials.size(); ++i) {
                Material __mat = _other.materials[i];
                __mat.base_color_texture = (__mat.base_color_texture == SCENE_NONE ? SCENE_NONE : __mat.base_color_texture + __tex0);
                __mat.normal_texture     = (__mat.normal_texture == SCENE_NONE ? SCENE_NONE : __mat.normal_texture + __tex0);
                __mat.emissive_texture   = (__mat.emissive_texture == SCENE_NONE ? SCENE_NONE : __mat.emissive_texture + __tex0);
                materials.push_back(__mat);
            }
            for (size_t i = 0u; i < _other.meshes.size(); ++i) {
                meshes.push_back(_other.meshes[i]);
                meshes.back().material = (meshes.back().material == SCENE_NONE ? SCENE_NONE : meshes.back().material + __mat0);
            }
            for (size_t i = 0u; i < _other.lights.size(); ++i) {
                lights.push_back(_other.lights[i]);
                lights.back().node = (lights.back().node == SCENE_NONE ? SCENE_NONE : lights.back().node + __node0);
            }
            for (size_t i = 0u; i < _other.nodes.size(); ++i) {
                Node __node = _other.nodes[i];
                __node.parent = __node.parent == SCENE_NONE ? _parent : __node.parent + __node0;
                for (size_t c = 0u; c < __node.children.size(); ++c) __node.children[c] += __node0;
                for (size_t m = 0u; m < __node.meshes.size();   ++m) __node.meshes[m]   += __mesh0;
                __node.light = (__node.light == SCENE_NONE ? SCENE_NONE : __node.light + __light0);
                __node.skin  = (__node.skin  == SCENE_NONE ? SCENE_NONE : __node.skin  + __skin0);
                nodes.push_back(__node);
                if (_other.nodes[i].parent == SCENE_NONE && _parent != SCENE_NONE)
                    nodes[_parent].children.push_back((uint32_t)nodes.size() - 1u);
            }
            full_update_ = true;
            return __node0;
        }

        void Scene::clear() {
            nodes.clear(); meshes.clear(); materials.clear(); textures.clear(); lights.clear(); skeletons.clear(); skins.clear();
            world.clear(); world_visible.clear(); changed.clear();
            depth_.clear(); dirty_.clear(); stamp_.clear();
            stamp_count_ = 0u;
            full_update_ = true;
        }

    }; // SceneGraph END
}; // Simple END
