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

        void Skeleton::computeInverseBind() {
            std::vector<mat4> __bind(bones.size());
            for (size_t i = 0u; i < bones.size(); ++i) {
                __bind[i] = bones[i].parent == SCENE_NONE ? bones[i].rest.matrix() : bones[i].rest.matrix() & __bind[bones[i].parent];
                bones[i].inverse_bind = __bind[i].inverse<float>();
            }
        }

        void Pose::reset(const Skeleton &_skeleton) {
            local.resize(_skeleton.bones.size());
            for (size_t i = 0u; i < local.size(); ++i) local[i] = _skeleton.bones[i].rest;
            dirty = true;
        }

        void Pose::computeFK(const Skeleton &_skeleton) {
            const size_t __count = _skeleton.bones.size();
            if (local.size() != __count) reset(_skeleton);
            model.resize(__count);
            skinning.resize(__count);
            for (size_t i = 0u; i < __count; ++i) {
                const Bone &__bone = _skeleton.bones[i];
                model[i]    = __bone.parent == SCENE_NONE ? local[i].matrix() : local[i].matrix() & model[__bone.parent];
                skinning[i] = __bone.inverse_bind & model[i];
            }
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
