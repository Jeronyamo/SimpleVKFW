#ifndef SVKFW_SCENE_CULLING_H
#define SVKFW_SCENE_CULLING_H

#include <cstdint>
#include <vector>

#include "scene/scene_graph.h"


namespace Simple {
    namespace SceneGraph {

//  == === ==== ================================== ==== === ==  \\
                    Culling
//  == === ==== ================================== ==== === ==  \\

        // Note: the view volume of a camera as 6 planes (normals point inside). Built from 'view & proj' (row-vector style,
        //       Vulkan clip space: -w <= x, y <= w, 0 <= z <= w); planes that degenerate (e.g. an infinite far plane) are dropped
        struct Frustum {
            enum Result { OUTSIDE = 0, INTERSECTS, INSIDE };

            vec4     planes[6];  // xyz: unit normal, w: distance (a point p is inside when dot(n, p) + w >= 0)
            uint32_t count = 0u;

            static Frustum fromMatrix(const mat4 &_view_proj);
            static uint32_t allPlanes() { return 0x3Fu; }

            // Tests the planes in '_mask' only and clears the bits of the planes the box is fully inside of,
            // so the children of a node fully inside a plane skip it
            Result classify(const AABB &_bounds, uint32_t &_mask) const;
            bool   intersects(const AABB &_bounds) const;
        }; // Frustum END

        // Note: a bounding volume hierarchy built once (binned SAH) over a fixed set of boxes; items can be marked removed
        //       (they stay in the tree and are skipped), so the tree is rebuilt only when many are gone. Flat arrays: a node
        //       with 'count' > 0 is a leaf with items [first, first + count), otherwise its children are 'first' and 'first' + 1
        struct StaticBVH {
            struct Node {
                AABB     bounds;
                uint32_t first = 0u, count = 0u;
            };

            std::vector<Node>     nodes;
            std::vector<uint32_t> items;       // the ids, in leaf order
            std::vector<AABB>     item_bounds; // per entry of 'items'
            uint32_t              max_leaf_size = 4u;

            void build(const std::vector<AABB> &_bounds, const std::vector<uint32_t> &_ids);
            void clear();
            // Appends the ids of the items that intersect the frustum; '_removed' (indexed by id) skips items if not null
            void query(const Frustum &_frustum, std::vector<uint32_t> &_out, const std::vector<uint8_t> *_removed = nullptr) const;
        }; // StaticBVH END

        // Note: an AABB tree with insertion and removal for moving objects (like 'b2DynamicTree' in Box2D): the leaves keep
        //       "fat" boxes (enlarged by a margin), so an object that moves inside its fat box doesn't change the tree
        struct DynamicAABBTree {
            struct Node {
                AABB     bounds;
                uint32_t parent = SCENE_NONE, left = SCENE_NONE, right = SCENE_NONE; // leaves: 'left' == SCENE_NONE
                uint32_t id = SCENE_NONE;                                          // leaves: the object
            };

            std::vector<Node>     nodes;
            std::vector<uint32_t> free_nodes;
            uint32_t              root = SCENE_NONE;
            uint32_t              leaf_count = 0u;

            // Returns the proxy (the leaf node); '_fat' is the box stored in the tree
            uint32_t insert(const AABB &_fat, uint32_t _id);
            void     remove(uint32_t _proxy);
            // Re-inserts the proxy if '_bounds' left its fat box ('_margin' enlarges the new one); returns true if it did
            bool     move(uint32_t _proxy, const AABB &_bounds, float _margin);
            const AABB &fatBounds(uint32_t _proxy) const { return nodes[_proxy].bounds; }
            void     clear();
            void     query(const Frustum &_frustum, std::vector<uint32_t> &_out) const;

        private:
            uint32_t allocate_();
            void     release_(uint32_t _node);
            void     insertLeaf_(uint32_t _leaf);
            void     removeLeaf_(uint32_t _leaf);
            void     refit_(uint32_t _node);
        }; // DynamicAABBTree END

        // Note: what a camera can see in a scene, per instance (a node and one of its meshes). Instances start in the static
        //       tree; when one moves (its world matrix or its skin's pose changed) it "wakes up": it is marked removed in the
        //       static tree and goes into the dynamic one. After 'sleep_frames' updates without moving it "falls asleep" and
        //       waits for the next rebuild of the static tree (tested one by one until then).
        //       Skinned meshes: at 'build' each bone gets the box of the vertices it moves (in the bind pose, with the morph
        //       offsets at full weight); posed, the boxes are transformed by the skinning matrices and joined. A bit larger
        //       than the exact bounds, with no pass over the vertices.
        //
        //       SceneCulling culling;
        //       culling.build(scene);                     // after loading and 'updateTransforms' (again if meshes/nodes are added)
        //       renderer.culling = &culling;              // the renderer updates it and draws the visible instances only
        //       // or by hand, each frame after 'updateTransforms' and 'updateSkins':
        //       culling.update(scene);
        //       culling.cull(Frustum::fromMatrix(view & proj), visible);
        struct SceneCulling {
            struct Instance {
                uint32_t node = SCENE_NONE, mesh = SCENE_NONE;
                uint32_t skin = SCENE_NONE;    // skinned: the skin whose pose bounds it
                AABB     bounds;               // world
                bool     dynamic = false;
                uint32_t proxy = SCENE_NONE;   // in the dynamic tree
                uint32_t still_frames = 0u;
                mat4     last_world;           // to notice movement
                uint64_t last_pose = 0u;       // 'Pose::revision' of the skin
            };
            struct BoneBounds {
                uint32_t bone;
                AABB     bounds;               // bind pose, skeleton space
            };
            struct Stats {
                uint32_t instances = 0u, visible = 0u, dynamic = 0u, sleeping = 0u, woke = 0u, rebuilds = 0u;
            };

            // Options
            uint32_t sleep_frames = 60u;   // updates without movement before an instance goes back to the static tree
            float    fat_margin   = 0.1f;  // dynamic boxes are enlarged by this part of their size
            float    rebuild_ratio = 0.25f; // rebuild the static tree when removed + sleeping > this part of it (or > 64)

            std::vector<Instance>                instances; // in node order, then mesh order (the draw order)
            std::vector<std::vector<BoneBounds>> bone_bounds; // per mesh (skinned meshes only)
            StaticBVH                            static_tree;
            DynamicAABBTree                      dynamic_tree;
            std::vector<uint8_t>                 removed;   // per instance: not in the static tree (moving or sleeping)
            std::vector<uint32_t>                sleeping;  // asleep, waiting for the static rebuild
            uint32_t                             static_removed = 0u;
            Stats                                stats;

            void build(const Scene &_scene);
            // Refreshes the bounds of the instances that moved, wakes and puts them to sleep
            void update(const Scene &_scene);
            // The visible instances (sorted, so the draw order is kept)
            void cull(const Frustum &_frustum, std::vector<uint32_t> &_visible);
            void clear();

        private:
            AABB worldBounds_(const Scene &_scene, const Instance &_instance) const;
            void rebuildStatic_();
        }; // SceneCulling END
    }; // SceneGraph END
}; // Simple END

#endif
