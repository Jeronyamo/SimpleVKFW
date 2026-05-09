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

        bool   isValid() const { return tag != TagID::TAG_DELETED && entity != EntityID(TAG_NO_COMPONENT); }
        size_t getHash() const { return (uint32_t(entity) << 32) | (uint32_t)tag; }

        bool operator<(const Component& _comp) { return getHash() < _comp.getHash(); }

        template <typename T>
        static Component Create(EntityID _ent, TagID _tag, const T &_data_fill) {
            Component __res{ _ent, _tag };
            __res.setData<T>(_data_fill);
            return __res;
        }
        static const Component Invalid;
    }; // Component END
    const Component Component::Invalid{ EntityID(TAG_NO_COMPONENT), TagID::TAG_DELETED };


    struct ComponentStorage {
        std::vector<std::unordered_map<TagID, uint32_t>> entity_tags; // entity -> set of tag-offset pairs (some tags correspond to components -> store its offset)
        std::vector<Component> components;
        std::unordered_set<EntityID> deleted_entities;

        static std::unordered_map<TagID, uint32_t> TagMapFromVector(const std::vector<TagID> &_tags) {
            std::unordered_map<TagID, uint32_t> __res(_tags.size());
            for (auto tag : _tags) __res[tag] = TAG_NO_COMPONENT;
            return __res;
        }

        static std::unordered_map<TagID, uint32_t> TagMapFromComponents(const std::vector<Component> &_components) {
            std::unordered_map<TagID, uint32_t> __res(_components.size());
            for (auto component : _components) __res[component.tag] = TAG_NO_COMPONENT;
            return __res;
        }

    // Manage Entities, Tags and Components

        EntityID addEntity(const std::vector<TagID> &_tags = {}, const std::vector<Component> &_components = {}) {
            EntityID __res_new_id;

            // Get EntityID; Set tags
            if (deleted_entities.empty()) {
                __res_new_id = (EntityID) entity_tags.size();
                entity_tags.push_back(TagMapFromVector(_tags));
            }
            else {
                __res_new_id = *deleted_entities.begin();
                deleted_entities.erase(deleted_entities.begin());
                entity_tags[(uint32_t)__res_new_id] = TagMapFromVector(_tags);
            }

            // Set components
            if (!_components.empty()) addEntityComponents(__res_new_id, _components);

            return __res_new_id;
        }

        bool hasTag(EntityID _ent, TagID _tag) const {
            return uint32_t(_ent) < entity_tags.size() && entity_tags[(uint32_t)_ent].find(_tag) != entity_tags[(uint32_t)_ent].end();
        }
        bool hasComponent(EntityID _ent, TagID _tag) const {
            if (uint32_t(_ent) >= entity_tags.size()) return false;
            auto   __comp_offset  = entity_tags[(uint32_t)_ent].find(_tag);
            return __comp_offset != entity_tags[(uint32_t)_ent].end() && __comp_offset->second != TAG_NO_COMPONENT;
        }
        uint32_t getComponentOffset(EntityID _ent, TagID _tag) const {
            if (uint32_t(_ent) >= entity_tags.size()) return TAG_NO_COMPONENT;
            auto   __comp_offset  = entity_tags[(uint32_t)_ent].find(_tag);
            return __comp_offset != entity_tags[(uint32_t)_ent].end()  ? __comp_offset->second  : TAG_NO_COMPONENT;
        }

        bool hasAllTags(EntityID _ent, const std::vector<TagID> &_tags) const {
            if (uint32_t(_ent) >= entity_tags.size()) return false;

            bool __res = true;
            for (auto tag : _tags) {
                if (entity_tags[(uint32_t)_ent].find(tag) == entity_tags[(uint32_t)_ent].end()) {
                    __res = false; break;
                }
            }
            return __res;
        }

        bool hasAnyTags(EntityID _ent, const std::vector<TagID> &_tags) const {
            if (uint32_t(_ent) >= entity_tags.size()) return false;

            bool __res = false;
            for (auto tag : _tags) {
                if (entity_tags[(uint32_t)_ent].find(tag) != entity_tags[(uint32_t)_ent].end()) {
                    __res = true; break;
                }
            }
            return __res;
        }

        bool hasNoTags(EntityID _ent, const std::vector<TagID> &_tags) const {
            if (uint32_t(_ent) >= entity_tags.size()) return false;

            bool __res = true;
            for (auto tag : _tags) {
                if (entity_tags[(uint32_t)_ent].find(tag) != entity_tags[(uint32_t)_ent].end()) {
                    __res = false; break;
                }
            }
            return __res;
        }

        uint32_t addTag(EntityID _ent, TagID _tag) {
            if (uint32_t(_ent) >= entity_tags.size()) return TAG_NO_COMPONENT;
            entity_tags[(uint32_t)_ent].insert({_tag, TAG_NO_COMPONENT});
            return entity_tags[(uint32_t)_ent][_tag];
        }
        void addTags(EntityID _ent, const std::vector<TagID> &_tags) {
            if (uint32_t(_ent) >= entity_tags.size()) return;
            auto _tag_map = TagMapFromVector(_tags);
            entity_tags[(uint32_t)_ent].insert(_tag_map.begin(), _tag_map.end());
        }
        void addComponent(const Component &_comp) {
            if (uint32_t(_comp.entity) >= entity_tags.size() || !_comp.isValid()) return;

            uint32_t __comp_offset = addTag(_comp.entity, _comp.tag);
            if (__comp_offset == TAG_NO_COMPONENT) {
                __comp_offset = (uint32_t) components.size();
                components.emplace_back();
            }
            components[__comp_offset] = _comp;
        }
        void addEntityComponents(EntityID _ent, const std::vector<Component> &_components) {
            if (uint32_t(_ent) >= entity_tags.size()) return;

            auto __comp_map = TagMapFromComponents(_components);
            entity_tags[(uint32_t)_ent].insert(__comp_map.begin(), __comp_map.end());

            uint32_t __comp_offset;
            for (const Component &component : _components) {
                __comp_offset = entity_tags[(uint32_t)_ent][component.tag];

                if (__comp_offset == TAG_NO_COMPONENT) {
                    __comp_offset = (uint32_t) components.size();
                    components.emplace_back();
                }
                components[__comp_offset] = component;
                components[__comp_offset].entity = _ent;
            }
        }
        bool tryReplaceDeletedComponent(const Component &_comp) {
            if (uint32_t(_comp.entity) >= entity_tags.size() || !_comp.isValid()) return false;

            uint32_t __min_deleted_offset = getComponentOffset(_comp.entity, TagID::TAG_DELETED);
            if (__min_deleted_offset != TAG_NO_COMPONENT) {
                components[__min_deleted_offset] = _comp;
                while (++__min_deleted_offset < components.size()) {
                    if (components[__min_deleted_offset].entity == _comp.entity &&
                        components[__min_deleted_offset].tag == TagID::TAG_DELETED) {
                        entity_tags[(uint32_t)_comp.entity][TagID::TAG_DELETED] = __min_deleted_offset;
                        break;
                    }
                }
                if (__min_deleted_offset >= components.size()) entity_tags[(uint32_t)_comp.entity].erase(TagID::TAG_DELETED);
                return true;
            }
            return false;
        }

        // Deletes tag and marks component deleted (if exists), returns 'true' if tag was deleted
        bool delTag(EntityID _ent, TagID _tag) {
            if (uint32_t(_ent) >= entity_tags.size()) return false;
            auto __comp_iter  = entity_tags[(uint32_t)_ent].find(_tag);

            if ( __comp_iter != entity_tags[(uint32_t)_ent].end()) {
                if (__comp_iter->second != TAG_NO_COMPONENT) {
                    components[__comp_iter->second] = Component::Invalid;
                    entity_tags[(uint32_t)_ent].insert({TagID::TAG_DELETED, TAG_NO_COMPONENT});
                    entity_tags[(uint32_t)_ent][TagID::TAG_DELETED] = std::min(__comp_iter->second, entity_tags[(uint32_t)_ent][TagID::TAG_DELETED]);
                }
                entity_tags[(uint32_t)_ent].erase(_tag);
                return true;
            }
            return false;
        }

        // Deletes tags and marks components deleted (if exist)
        void delTags(EntityID _ent, const std::vector<TagID> &_tags) {
            if (uint32_t(_ent) >= entity_tags.size()) return;

            uint32_t __min_comp_offset = TAG_NO_COMPONENT;
            for (TagID tag : _tags) {
                auto __comp_iter = entity_tags[(uint32_t)_ent].find(tag);
                if (__comp_iter != entity_tags[(uint32_t)_ent].end()) {
                    __min_comp_offset = std::min(__min_comp_offset, __comp_iter->second);
                    if (__comp_iter->second != TAG_NO_COMPONENT)
                        components[__comp_iter->second].tag = TagID::TAG_DELETED;
                }
            }
            if (__min_comp_offset < TAG_NO_COMPONENT) {
                entity_tags[(uint32_t)_ent].insert({TagID::TAG_DELETED, __min_comp_offset});
                entity_tags[(uint32_t)_ent][TagID::TAG_DELETED] = std::min(__min_comp_offset, entity_tags[(uint32_t)_ent][TagID::TAG_DELETED]);
            }
        }

        void deleteEntity(EntityID _ent) {
            if (uint32_t(_ent) >= entity_tags.size()) return;

            std::vector<TagID> __tags(entity_tags[(uint32_t)_ent].size());
            auto __tags_iter = entity_tags[(uint32_t)_ent].begin();
            for (uint32_t i = 0u; i < __tags.size(); ++i)
                __tags[i] = (__tags_iter++)->first;
            delTags(_ent, __tags);
            deleted_entities.insert(_ent);
        }

        void defragComponents() {
            std::sort(components.begin(), components.end());
            for (int i = components.size() - 1; i >= 0; --i)
                entity_tags[(uint32_t)components[i].entity][components[i].tag] = i;
        }

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
        std::vector<EntityID> gatherEntities(const std::vector<TagID> &_all_tags, const std::vector<TagID> &_any_tags, const std::vector<TagID> &_no_tags) const {
            std::vector<EntityID> __res_entities;
            for (uint32_t i = 0u; i < entity_tags.size(); ++i) {
                bool __entity_fits = (_all_tags.size() ? hasAllTags(EntityID(i), _all_tags) : true) &&
                                     (_any_tags.size() ? hasAnyTags(EntityID(i), _any_tags) : true) &&
                                     ( _no_tags.size() ? hasNoTags (EntityID(i),  _no_tags) : true);
                if (__entity_fits) __res_entities.push_back(EntityID(i));
            }
            return __res_entities;
        }
    }; // ComponentStorage END
}; // Simple END

#endif