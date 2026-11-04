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
        }

        void Scene::updateNode_(uint32_t _index, std::vector<uint8_t> &_done) {
            if (_done[_index]) return;
            const Node &__node = nodes[_index];
            if (__node.parent == SCENE_NONE) {
                world[_index] = __node.local;
                world_visible[_index] = __node.visible ? 1u : 0u;
            } else {
                updateNode_(__node.parent, _done);
                world[_index] = __node.local & world[__node.parent];
                world_visible[_index] = (__node.visible && world_visible[__node.parent]) ? 1u : 0u;
            }
            _done[_index] = 1u;
        }

        void Scene::updateTransforms() {
            world.resize(nodes.size());
            world_visible.resize(nodes.size());
            std::vector<uint8_t> __done(nodes.size(), 0u);
            for (uint32_t i = 0u; i < (uint32_t)nodes.size(); ++i)
                updateNode_(i, __done);
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
                nodes.push_back(__node);
                if (_other.nodes[i].parent == SCENE_NONE && _parent != SCENE_NONE)
                    nodes[_parent].children.push_back((uint32_t)nodes.size() - 1u);
            }
            return __node0;
        }

        void Scene::clear() {
            nodes.clear(); meshes.clear(); materials.clear(); textures.clear(); lights.clear();
            world.clear(); world_visible.clear();
        }

    }; // SceneGraph END
}; // Simple END
