#include "scene/scene_culling.h"

#include <algorithm>
#include <cmath>
#include <cstring>


namespace Simple {
    namespace SceneGraph {
        namespace {
            float surfaceArea(const AABB &_bounds) {
                if (!_bounds.valid()) return 0.f;
                const vec3 d = _bounds.max - _bounds.min;
                return 2.f * (d.x * d.y + d.y * d.z + d.z * d.x);
            }

            AABB joined(const AABB &_a, const AABB &_b) {
                AABB __res = _a;
                __res.add(_b);
                return __res;
            }

            bool contains(const AABB &_outer, const AABB &_inner) {
                return _outer.min.x <= _inner.min.x && _outer.min.y <= _inner.min.y && _outer.min.z <= _inner.min.z &&
                       _outer.max.x >= _inner.max.x && _outer.max.y >= _inner.max.y && _outer.max.z >= _inner.max.z;
            }

            AABB enlarged(const AABB &_bounds, float _part) {
                AABB __res = _bounds;
                if (!__res.valid()) return __res;
                const vec3 d = (_bounds.max - _bounds.min) * _part;
                const float __m = std::max(std::max(d.x, d.y), std::max(d.z, 1e-3f));
                __res.min = __res.min - vec3{__m, __m, __m};
                __res.max = __res.max + vec3{__m, __m, __m};
                return __res;
            }

            float axisOf(const vec3 &_v, int _axis) { return _axis == 0 ? _v.x : _axis == 1 ? _v.y : _v.z; }

            // The box around a transformed box (Arvo): the center is transformed, the half size goes through |M|.
            // Plain floats: it runs for every bone box of every moving skinned mesh
            void transformBox(const float _min[3], const float _max[3], const mat4 &_m, float _out_min[3], float _out_max[3]) {
                float c[3], e[3];
                for (int i = 0; i < 3; ++i) { c[i] = 0.5f * (_min[i] + _max[i]); e[i] = 0.5f * (_max[i] - _min[i]); }
                for (int j = 0; j < 3; ++j) {
                    const float __c = _m.M[3][j] + c[0] * _m.M[0][j] + c[1] * _m.M[1][j] + c[2] * _m.M[2][j];
                    const float __e = e[0] * std::fabs(_m.M[0][j]) + e[1] * std::fabs(_m.M[1][j]) + e[2] * std::fabs(_m.M[2][j]);
                    _out_min[j] = __c - __e;
                    _out_max[j] = __c + __e;
                }
            }

            AABB transformedBox(const AABB &_bounds, const mat4 &_m) {
                AABB __res;
                if (!_bounds.valid()) return __res;
                const float __min[3] = { _bounds.min.x, _bounds.min.y, _bounds.min.z }, __max[3] = { _bounds.max.x, _bounds.max.y, _bounds.max.z };
                float __out_min[3], __out_max[3];
                transformBox(__min, __max, _m, __out_min, __out_max);
                __res.min = { __out_min[0], __out_min[1], __out_min[2] };
                __res.max = { __out_max[0], __out_max[1], __out_max[2] };
                return __res;
            }

            bool sameMatrix(const mat4 &_a, const mat4 &_b) { return std::memcmp(&_a, &_b, sizeof(mat4)) == 0; }
        } // anonymous


//  == === ==== >  Frustum

        Frustum Frustum::fromMatrix(const mat4 &_m) {
            // Note: row-vector style, clip = p & M, so a clip coordinate is a dot product with a column of M
            Frustum __f;
            vec4 c[4];
            for (int i = 0; i < 4; ++i) c[i] = { _m.M[0][i], _m.M[1][i], _m.M[2][i], _m.M[3][i] };
            const vec4 __planes[6] = { c[3] + c[0], c[3] - c[0], c[3] + c[1], c[3] - c[1], c[2], c[3] - c[2] };
            for (int i = 0; i < 6; ++i) {
                const float __len = std::sqrt(__planes[i].x * __planes[i].x + __planes[i].y * __planes[i].y + __planes[i].z * __planes[i].z);
                if (__len < 1e-12f) continue; // e.g. the far plane of an infinite projection
                __f.planes[__f.count++] = __planes[i] * (1.f / __len);
            }
            return __f;
        }

        Frustum::Result Frustum::classify(const AABB &_bounds, uint32_t &_mask) const {
            if (!_bounds.valid()) return OUTSIDE;
            Result __res = INSIDE;
            for (uint32_t i = 0u; i < count; ++i) {
                if (!(_mask & (1u << i))) continue;
                const vec4 &p = planes[i];
                // The corner farthest along the normal decides "outside", the nearest one "fully inside"
                const float __far  = p.x * (p.x > 0.f ? _bounds.max.x : _bounds.min.x) + p.y * (p.y > 0.f ? _bounds.max.y : _bounds.min.y) +
                                     p.z * (p.z > 0.f ? _bounds.max.z : _bounds.min.z) + p.w;
                if (__far < 0.f) return OUTSIDE;
                const float __near = p.x * (p.x > 0.f ? _bounds.min.x : _bounds.max.x) + p.y * (p.y > 0.f ? _bounds.min.y : _bounds.max.y) +
                                     p.z * (p.z > 0.f ? _bounds.min.z : _bounds.max.z) + p.w;
                if (__near >= 0.f) _mask &= ~(1u << i);
                else __res = INTERSECTS;
            }
            return __res;
        }

        bool Frustum::intersects(const AABB &_bounds) const {
            uint32_t __mask = allPlanes();
            return classify(_bounds, __mask) != OUTSIDE;
        }


//  == === ==== >  StaticBVH

        void StaticBVH::clear() {
            nodes.clear();
            items.clear();
            item_bounds.clear();
        }

        void StaticBVH::build(const std::vector<AABB> &_bounds, const std::vector<uint32_t> &_ids) {
            clear();
            const uint32_t __count = (uint32_t)std::min(_bounds.size(), _ids.size());
            if (__count == 0u) return;
            items.assign(_ids.begin(), _ids.begin() + __count);
            item_bounds.assign(_bounds.begin(), _bounds.begin() + __count);
            std::vector<vec3> __centers(__count);
            for (uint32_t i = 0u; i < __count; ++i) __centers[i] = item_bounds[i].center();

            nodes.reserve(2u * __count);
            nodes.push_back(Node());
            nodes[0].first = 0u;
            nodes[0].count = __count;
            std::vector<uint32_t> __stack(1u, 0u);
            const int BINS = 12;
            while (!__stack.empty()) {
                const uint32_t __index = __stack.back();
                __stack.pop_back();
                const uint32_t __first = nodes[__index].first, __n = nodes[__index].count;
                AABB __bounds, __centroids;
                for (uint32_t i = __first; i < __first + __n; ++i) { __bounds.add(item_bounds[i]); __centroids.add(__centers[i]); }
                nodes[__index].bounds = __bounds;
                if (__n <= max_leaf_size) continue;

                // Binned SAH along the longest centroid axis
                const vec3 __extent = __centroids.size();
                const int __axis = __extent.x >= __extent.y && __extent.x >= __extent.z ? 0 : __extent.y >= __extent.z ? 1 : 2;
                const float __lo = axisOf(__centroids.min, __axis), __width = axisOf(__extent, __axis);
                uint32_t __split = __first + __n / 2u;
                if (__width > 1e-12f) {
                    AABB     __bin_bounds[BINS];
                    uint32_t __bin_count[BINS] = {};
                    for (uint32_t i = __first; i < __first + __n; ++i) {
                        const int b = std::min(BINS - 1, (int)((axisOf(__centers[i], __axis) - __lo) / __width * BINS));
                        __bin_bounds[b].add(item_bounds[i]);
                        ++__bin_count[b];
                    }
                    float    __right_area[BINS];
                    uint32_t __right_count[BINS];
                    AABB     __acc;
                    uint32_t __acc_count = 0u;
                    for (int b = BINS - 1; b > 0; --b) {
                        __acc.add(__bin_bounds[b]);
                        __acc_count += __bin_count[b];
                        __right_area[b]  = surfaceArea(__acc);
                        __right_count[b] = __acc_count;
                    }
                    float __best_cost = 1e30f;
                    int   __best_bin  = -1;
                    __acc = AABB();
                    __acc_count = 0u;
                    for (int b = 1; b < BINS; ++b) {
                        __acc.add(__bin_bounds[b - 1]);
                        __acc_count += __bin_count[b - 1];
                        if (__acc_count == 0u || __right_count[b] == 0u) continue;
                        const float __cost = surfaceArea(__acc) * __acc_count + __right_area[b] * __right_count[b];
                        if (__cost < __best_cost) { __best_cost = __cost; __best_bin = b; }
                    }
                    if (__best_bin > 0) {
                        // Partition: items with a bin below '__best_bin' first
                        uint32_t i = __first, j = __first + __n;
                        while (i < j) {
                            const int b = std::min(BINS - 1, (int)((axisOf(__centers[i], __axis) - __lo) / __width * BINS));
                            if (b < __best_bin) { ++i; continue; }
                            --j;
                            std::swap(items[i], items[j]);
                            std::swap(item_bounds[i], item_bounds[j]);
                            std::swap(__centers[i], __centers[j]);
                        }
                        __split = i;
                    }
                }
                if (__split == __first || __split == __first + __n) __split = __first + __n / 2u; // same centers: halves

                const uint32_t __left = (uint32_t)nodes.size();
                nodes.push_back(Node());
                nodes.push_back(Node());
                nodes[__left].first      = __first;
                nodes[__left].count      = __split - __first;
                nodes[__left + 1u].first = __split;
                nodes[__left + 1u].count = __first + __n - __split;
                nodes[__index].first = __left;
                nodes[__index].count = 0u;
                __stack.push_back(__left);
                __stack.push_back(__left + 1u);
            }
        }

        void StaticBVH::query(const Frustum &_frustum, std::vector<uint32_t> &_out, const std::vector<uint8_t> *_removed) const {
            if (nodes.empty()) return;
            struct Entry { uint32_t node, mask; };
            std::vector<Entry> __stack(1u, Entry{ 0u, Frustum::allPlanes() });
            while (!__stack.empty()) {
                Entry __e = __stack.back();
                __stack.pop_back();
                const Node &__node = nodes[__e.node];
                const Frustum::Result __r = __e.mask ? _frustum.classify(__node.bounds, __e.mask) : Frustum::INSIDE;
                if (__r == Frustum::OUTSIDE) continue;
                if (__node.count > 0u) {
                    for (uint32_t i = __node.first; i < __node.first + __node.count; ++i) {
                        const uint32_t __id = items[i];
                        if (_removed && __id < _removed->size() && (*_removed)[__id]) continue;
                        uint32_t __mask = __e.mask;
                        if (__node.count == 1u || _frustum.classify(item_bounds[i], __mask) != Frustum::OUTSIDE) _out.push_back(__id);
                    }
                    continue;
                }
                __stack.push_back(Entry{ __node.first,      __e.mask });
                __stack.push_back(Entry{ __node.first + 1u, __e.mask });
            }
        }


//  == === ==== >  DynamicAABBTree

        uint32_t DynamicAABBTree::allocate_() {
            if (!free_nodes.empty()) {
                const uint32_t __n = free_nodes.back();
                free_nodes.pop_back();
                nodes[__n] = Node();
                return __n;
            }
            nodes.push_back(Node());
            return (uint32_t)nodes.size() - 1u;
        }

        void DynamicAABBTree::release_(uint32_t _node) {
            nodes[_node] = Node();
            free_nodes.push_back(_node);
        }

        void DynamicAABBTree::clear() {
            nodes.clear();
            free_nodes.clear();
            root = SCENE_NONE;
            leaf_count = 0u;
        }

        uint32_t DynamicAABBTree::insert(const AABB &_fat, uint32_t _id) {
            const uint32_t __leaf = allocate_();
            nodes[__leaf].bounds = _fat;
            nodes[__leaf].id     = _id;
            insertLeaf_(__leaf);
            ++leaf_count;
            return __leaf;
        }

        void DynamicAABBTree::remove(uint32_t _proxy) {
            if (_proxy >= nodes.size()) return;
            removeLeaf_(_proxy);
            release_(_proxy);
            --leaf_count;
        }

        bool DynamicAABBTree::move(uint32_t _proxy, const AABB &_bounds, float _margin) {
            if (contains(nodes[_proxy].bounds, _bounds)) return false;
            removeLeaf_(_proxy);
            nodes[_proxy].bounds = enlarged(_bounds, _margin);
            insertLeaf_(_proxy);
            return true;
        }

        void DynamicAABBTree::refit_(uint32_t _node) {
            for (uint32_t n = _node; n != SCENE_NONE; n = nodes[n].parent)
                nodes[n].bounds = joined(nodes[nodes[n].left].bounds, nodes[nodes[n].right].bounds);
        }

        void DynamicAABBTree::insertLeaf_(uint32_t _leaf) {
            nodes[_leaf].parent = SCENE_NONE;
            if (root == SCENE_NONE) { root = _leaf; return; }

            // The sibling: descend while it is cheaper (surface area heuristic with the cost inherited from the enlarged parents)
            const AABB &__bounds = nodes[_leaf].bounds;
            uint32_t __sibling = root;
            while (nodes[__sibling].left != SCENE_NONE) {
                const Node &__n = nodes[__sibling];
                const float __area      = surfaceArea(__n.bounds);
                const float __joined    = surfaceArea(joined(__n.bounds, __bounds));
                const float __here      = 2.f * __joined;            // a new parent of this node
                const float __inherited = 2.f * (__joined - __area); // what going down adds to this node
                float __child_cost[2];
                const uint32_t __children[2] = { __n.left, __n.right };
                for (int c = 0; c < 2; ++c) {
                    const Node &__child = nodes[__children[c]];
                    const float __with = surfaceArea(joined(__child.bounds, __bounds));
                    __child_cost[c] = (__child.left == SCENE_NONE ? __with : __with - surfaceArea(__child.bounds)) + __inherited;
                }
                if (__here < __child_cost[0] && __here < __child_cost[1]) break;
                __sibling = __child_cost[0] <= __child_cost[1] ? __n.left : __n.right;
            }

            const uint32_t __old_parent = nodes[__sibling].parent;
            const uint32_t __parent = allocate_();
            nodes[__parent].parent = __old_parent;
            nodes[__parent].left   = __sibling;
            nodes[__parent].right  = _leaf;
            nodes[__parent].bounds = joined(nodes[__sibling].bounds, __bounds);
            nodes[__sibling].parent = __parent;
            nodes[_leaf].parent     = __parent;
            if (__old_parent == SCENE_NONE) root = __parent;
            else {
                if (nodes[__old_parent].left == __sibling) nodes[__old_parent].left = __parent;
                else nodes[__old_parent].right = __parent;
                refit_(__old_parent);
            }
        }

        void DynamicAABBTree::removeLeaf_(uint32_t _leaf) {
            if (_leaf == root) { root = SCENE_NONE; return; }
            const uint32_t __parent = nodes[_leaf].parent;
            const uint32_t __grand  = nodes[__parent].parent;
            const uint32_t __sibling = nodes[__parent].left == _leaf ? nodes[__parent].right : nodes[__parent].left;
            if (__grand == SCENE_NONE) {
                root = __sibling;
                nodes[__sibling].parent = SCENE_NONE;
            } else {
                if (nodes[__grand].left == __parent) nodes[__grand].left = __sibling;
                else nodes[__grand].right = __sibling;
                nodes[__sibling].parent = __grand;
                refit_(__grand);
            }
            release_(__parent);
            nodes[_leaf].parent = SCENE_NONE;
        }

        void DynamicAABBTree::query(const Frustum &_frustum, std::vector<uint32_t> &_out) const {
            if (root == SCENE_NONE) return;
            struct Entry { uint32_t node, mask; };
            std::vector<Entry> __stack(1u, Entry{ root, Frustum::allPlanes() });
            while (!__stack.empty()) {
                Entry __e = __stack.back();
                __stack.pop_back();
                const Node &__node = nodes[__e.node];
                if (__e.mask && _frustum.classify(__node.bounds, __e.mask) == Frustum::OUTSIDE) continue;
                if (__node.left == SCENE_NONE) { _out.push_back(__node.id); continue; }
                __stack.push_back(Entry{ __node.left,  __e.mask });
                __stack.push_back(Entry{ __node.right, __e.mask });
            }
        }


//  == === ==== >  SceneCulling

        void SceneCulling::clear() {
            instances.clear();
            bone_bounds.clear();
            static_tree.clear();
            dynamic_tree.clear();
            removed.clear();
            sleeping.clear();
            static_removed = 0u;
            stats = Stats();
        }

        AABB SceneCulling::worldBounds_(const Scene &_scene, const Instance &_instance) const {
            const Mesh &__mesh = _scene.meshes[_instance.mesh];
            if (_instance.skin == SCENE_NONE) return transformedBox(__mesh.bounds, _scene.world[_instance.node]);
            const Skin &__skin = _scene.skins[_instance.skin];
            const std::vector<BoneBounds> &__bones = bone_bounds[_instance.mesh];
            float __lo[3] = { 1e30f, 1e30f, 1e30f }, __hi[3] = { -1e30f, -1e30f, -1e30f };
            for (size_t b = 0u; b < __bones.size(); ++b) {
                if (__bones[b].bone >= __skin.pose.skinning.size()) continue;
                const AABB &__src = __bones[b].bounds;
                const float __min[3] = { __src.min.x, __src.min.y, __src.min.z }, __max[3] = { __src.max.x, __src.max.y, __src.max.z };
                float __out_min[3], __out_max[3];
                transformBox(__min, __max, __skin.pose.skinning[__bones[b].bone], __out_min, __out_max);
                for (int i = 0; i < 3; ++i) { __lo[i] = std::min(__lo[i], __out_min[i]); __hi[i] = std::max(__hi[i], __out_max[i]); }
            }
            AABB __posed;
            if (__lo[0] <= __hi[0]) {
                __posed.min = { __lo[0], __lo[1], __lo[2] };
                __posed.max = { __hi[0], __hi[1], __hi[2] };
            }
            // Note: like the renderer, a skinned mesh is placed by its skin's node
            const uint32_t __space = __skin.node != SCENE_NONE ? __skin.node : _instance.node;
            return transformedBox(__posed, _scene.world[__space]);
        }

        void SceneCulling::build(const Scene &_scene) {
            clear();
            // Bone boxes of the skinned meshes (bind pose): the vertices each bone moves, with their morph offsets
            bone_bounds.resize(_scene.meshes.size());
            for (size_t m = 0u; m < _scene.meshes.size(); ++m) {
                const Mesh &__mesh = _scene.meshes[m];
                if (__mesh.skin.size() != __mesh.vertices.size() || __mesh.skin.empty()) continue;
                std::vector<AABB> __boxes;
                std::vector<uint32_t> __bones;
                std::vector<uint32_t> __slot; // bone -> index in '__boxes'
                // The morphed positions of a vertex extend its box
                std::vector<uint32_t> __morph_first(__mesh.vertices.size() + 1u, 0u);
                for (size_t t = 0u; t < __mesh.morphs.size(); ++t)
                    if (__mesh.morphs[t].vertex < __mesh.vertices.size()) ++__morph_first[__mesh.morphs[t].vertex + 1u];
                for (size_t v = 0u; v < __mesh.vertices.size(); ++v) __morph_first[v + 1u] += __morph_first[v];
                std::vector<vec3> __morph_offsets(__morph_first.back());
                {
                    std::vector<uint32_t> __fill(__morph_first.begin(), __morph_first.end() - 1);
                    for (size_t t = 0u; t < __mesh.morphs.size(); ++t)
                        if (__mesh.morphs[t].vertex < __mesh.vertices.size())
                            __morph_offsets[__fill[__mesh.morphs[t].vertex]++] = __mesh.morphs[t].position;
                }
                for (size_t v = 0u; v < __mesh.vertices.size(); ++v) {
                    AABB __vertex;
                    __vertex.add(__mesh.vertices[v].position);
                    for (uint32_t t = __morph_first[v]; t < __morph_first[v + 1u]; ++t)
                        __vertex.add(__mesh.vertices[v].position + __morph_offsets[t]);
                    const SkinVertex &__skin = __mesh.skin[v];
                    const float __w[4] = { __skin.weights.x, __skin.weights.y, __skin.weights.z, __skin.weights.w };
                    for (int b = 0; b < 4; ++b) {
                        if (__w[b] <= 0.f) continue;
                        const uint32_t __bone = __skin.bones[b];
                        if (__bone >= __slot.size()) __slot.resize(__bone + 1u, SCENE_NONE);
                        if (__slot[__bone] == SCENE_NONE) {
                            __slot[__bone] = (uint32_t)__boxes.size();
                            __boxes.push_back(AABB());
                            __bones.push_back(__bone);
                        }
                        __boxes[__slot[__bone]].add(__vertex);
                    }
                }
                bone_bounds[m].resize(__boxes.size());
                for (size_t i = 0u; i < __boxes.size(); ++i) {
                    bone_bounds[m][i].bone   = __bones[i];
                    bone_bounds[m][i].bounds = __boxes[i];
                }
            }

            // Instances in node order, then mesh order
            for (uint32_t n = 0u; n < (uint32_t)_scene.nodes.size(); ++n) {
                const Node &__node = _scene.nodes[n];
                for (size_t m = 0u; m < __node.meshes.size(); ++m) {
                    Instance __instance;
                    __instance.node = n;
                    __instance.mesh = __node.meshes[m];
                    if (__instance.mesh >= _scene.meshes.size()) continue;
                    if (__node.skin < _scene.skins.size() && !bone_bounds[__instance.mesh].empty()) __instance.skin = __node.skin;
                    __instance.bounds     = worldBounds_(_scene, __instance);
                    const uint32_t __space = __instance.skin != SCENE_NONE && _scene.skins[__instance.skin].node != SCENE_NONE ?
                                             _scene.skins[__instance.skin].node : n;
                    __instance.last_world = _scene.world[__space];
                    if (__instance.skin != SCENE_NONE) __instance.last_pose = _scene.skins[__instance.skin].pose.revision;
                    instances.push_back(__instance);
                }
            }
            removed.assign(instances.size(), 0u);
            rebuildStatic_();
            stats.instances = (uint32_t)instances.size();
        }

        void SceneCulling::rebuildStatic_() {
            std::vector<AABB>     __bounds;
            std::vector<uint32_t> __ids;
            for (uint32_t i = 0u; i < (uint32_t)instances.size(); ++i) {
                if (instances[i].dynamic) continue;
                removed[i] = 0u;
                __bounds.push_back(instances[i].bounds);
                __ids.push_back(i);
            }
            static_tree.build(__bounds, __ids);
            sleeping.clear();
            static_removed = 0u;
            ++stats.rebuilds;
        }

        void SceneCulling::update(const Scene &_scene) {
            stats.woke = 0u;
            for (uint32_t i = 0u; i < (uint32_t)instances.size(); ++i) {
                Instance &__inst = instances[i];
                if (__inst.node >= _scene.nodes.size() || __inst.node >= _scene.world.size()) continue;
                const uint32_t __space = __inst.skin != SCENE_NONE && _scene.skins[__inst.skin].node != SCENE_NONE ?
                                         _scene.skins[__inst.skin].node : __inst.node;
                const uint64_t __pose = __inst.skin != SCENE_NONE ? _scene.skins[__inst.skin].pose.revision : 0u;
                const bool __moved = !sameMatrix(_scene.world[__space], __inst.last_world) || __pose != __inst.last_pose;
                if (!__moved) {
                    if (__inst.dynamic && ++__inst.still_frames >= sleep_frames) {
                        // Falls asleep: out of the dynamic tree, into the static one at its next rebuild
                        dynamic_tree.remove(__inst.proxy);
                        __inst.proxy   = SCENE_NONE;
                        __inst.dynamic = false;
                        sleeping.push_back(i);
                    }
                    continue;
                }
                __inst.last_world   = _scene.world[__space];
                __inst.last_pose    = __pose;
                __inst.still_frames = 0u;
                __inst.bounds       = worldBounds_(_scene, __inst);
                if (__inst.dynamic) {
                    dynamic_tree.move(__inst.proxy, __inst.bounds, fat_margin);
                    continue;
                }
                // Wakes up: out of the static tree (or the sleeping list), into the dynamic one
                if (!removed[i]) { removed[i] = 1u; ++static_removed; }
                else {
                    std::vector<uint32_t>::iterator __it = std::find(sleeping.begin(), sleeping.end(), i);
                    if (__it != sleeping.end()) sleeping.erase(__it);
                }
                __inst.dynamic = true;
                __inst.proxy   = dynamic_tree.insert(enlarged(__inst.bounds, fat_margin), i);
                ++stats.woke;
            }
            // Many holes in the static tree, or many sleepers tested one by one: rebuild it
            const uint32_t __pending = static_removed + (uint32_t)sleeping.size();
            if (__pending > 64u || (__pending > 0u && __pending > rebuild_ratio * (float)static_tree.items.size() && !sleeping.empty()))
                rebuildStatic_();
            stats.dynamic  = dynamic_tree.leaf_count;
            stats.sleeping = (uint32_t)sleeping.size();
        }

        void SceneCulling::cull(const Frustum &_frustum, std::vector<uint32_t> &_visible) {
            _visible.clear();
            static_tree.query(_frustum, _visible, &removed);
            dynamic_tree.query(_frustum, _visible);
            for (size_t s = 0u; s < sleeping.size(); ++s)
                if (_frustum.intersects(instances[sleeping[s]].bounds)) _visible.push_back(sleeping[s]);
            std::sort(_visible.begin(), _visible.end());
            stats.visible = (uint32_t)_visible.size();
        }
    }; // SceneGraph END
}; // Simple END
