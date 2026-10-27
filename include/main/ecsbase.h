#ifndef SVKFW_ECSBASE_H
#define SVKFW_ECSBASE_H

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

#include "math/vectors.h"
#include "fun/etcs_tags.h"

#define TAG_NO_COMPONENT UINT32_MAX


namespace Simple {
//  == === ==== =================================== ==== === ==  \\
                    Entity tag-component system
//  == === ==== =================================== ==== === ==  \\

    enum struct EntityID : uint32_t;
    typedef     uint8_t DataArr[16]; // TODO: 16 or 24?

    struct Component {
        EntityID entity;
        TagID       tag;
        DataArr  p_data;

        template <typename SetT>
        void   setData(const SetT &_data_fill) {  std::memcpy((void*)p_data, (void*) &_data_fill, sizeof(SetT));  }
        template <typename GetT>
        GetT   getData() const { return *((GetT*)p_data); }

        bool   isValid() const;
        uint64_t getHash() const;

        bool operator<(const Component& _comp);

        template <typename T>
        static Component Create(EntityID _ent, TagID _tag, const T &_data_fill) {
            Component __res{ _ent, _tag };
            __res.setData<T>(_data_fill);
            return __res;
        }
        static const Component Invalid;
    }; // Component END


    struct ComponentStorage {
        std::vector<std::unordered_map<TagID, uint32_t>> entity_tags; // entity -> set of tag-offset pairs (some tags correspond to components -> store its offset)
        std::vector<Component> components;
        std::unordered_set<EntityID> deleted_entities;

        static std::unordered_map<TagID, uint32_t> TagMapFromVector(const std::vector<TagID> &_tags);

        static std::unordered_map<TagID, uint32_t> TagMapFromComponents(const std::vector<Component> &_components);

    // Manage Entities, Tags and Components

        EntityID addEntity(const std::vector<TagID> &_tags = {}, const std::vector<Component> &_components = {});

        bool hasTag(EntityID _ent, TagID _tag) const;
        bool hasComponent(EntityID _ent, TagID _tag) const;
        uint32_t getComponentOffset(EntityID _ent, TagID _tag) const;

        bool hasAllTags(EntityID _ent, const std::vector<TagID> &_tags) const;

        bool hasAnyTags(EntityID _ent, const std::vector<TagID> &_tags) const;

        bool hasNoTags(EntityID _ent, const std::vector<TagID> &_tags) const;

        uint32_t addTag(EntityID _ent, TagID _tag);
        void addTags(EntityID _ent, const std::vector<TagID> &_tags);
        void addComponent(const Component &_comp);
        void addEntityComponents(EntityID _ent, const std::vector<Component> &_components);
        bool tryReplaceDeletedComponent(const Component &_comp);

        // Deletes tag and marks component deleted (if exists), returns 'true' if tag was deleted
        bool delTag(EntityID _ent, TagID _tag);

        // Deletes tags and marks components deleted (if exist)
        void delTags(EntityID _ent, const std::vector<TagID> &_tags);

        void deleteEntity(EntityID _ent);

        void defragComponents();

    // Tag, Component lookup

        template <typename GetT>
        GetT getComponentData(EntityID _ent, TagID _tag) const {
            auto   __comp_offset  = entity_tags[(uint32_t)_ent].find(_tag);
            return __comp_offset != entity_tags[(uint32_t)_ent].end() && __comp_offset->second != TAG_NO_COMPONENT ?
                        components[__comp_offset->second].getData<GetT>() : GetT();
        }

        template <typename SetT>
        void setComponentData(EntityID _ent, TagID _tag, const SetT &_data_fill) {
            auto __comp_offset = entity_tags[(uint32_t)_ent].find(_tag);
            if (__comp_offset != entity_tags[(uint32_t)_ent].end() && __comp_offset->second != TAG_NO_COMPONENT)
                components[__comp_offset->second].setData(_data_fill);
        }

        // Gather all entities that have all 'all_tags', any of 'any_tags', and no 'no_tags'
        std::vector<EntityID> gatherEntities(const std::vector<TagID> &_all_tags, const std::vector<TagID> &_any_tags, const std::vector<TagID> &_no_tags) const;
    }; // ComponentStorage END
}; // Simple END

#endif