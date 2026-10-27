#include "main/ecsbase.h"

namespace Simple {

    bool   Component::isValid() const { return tag != TagID::TAG_DELETED && entity != EntityID(TAG_NO_COMPONENT); }

    uint64_t Component::getHash() const { return (uint64_t(uint32_t(entity)) << 32) | (uint32_t)tag; }

    bool Component::operator<(const Component& _comp) { return getHash() < _comp.getHash(); }

    const Component Component::Invalid{ EntityID(TAG_NO_COMPONENT), TagID::TAG_DELETED };

    std::unordered_map<TagID, uint32_t> ComponentStorage::TagMapFromVector(const std::vector<TagID> &_tags) {
        std::unordered_map<TagID, uint32_t> __res(_tags.size());
        for (auto tag : _tags) __res[tag] = TAG_NO_COMPONENT;
        return __res;
    }

    std::unordered_map<TagID, uint32_t> ComponentStorage::TagMapFromComponents(const std::vector<Component> &_components) {
        std::unordered_map<TagID, uint32_t> __res(_components.size());
        for (auto component : _components) __res[component.tag] = TAG_NO_COMPONENT;
        return __res;
    }

    EntityID ComponentStorage::addEntity(const std::vector<TagID> &_tags, const std::vector<Component> &_components) {
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

    bool ComponentStorage::hasTag(EntityID _ent, TagID _tag) const {
        return uint32_t(_ent) < entity_tags.size() && entity_tags[(uint32_t)_ent].find(_tag) != entity_tags[(uint32_t)_ent].end();
    }

    bool ComponentStorage::hasComponent(EntityID _ent, TagID _tag) const {
        if (uint32_t(_ent) >= entity_tags.size()) return false;
        auto   __comp_offset  = entity_tags[(uint32_t)_ent].find(_tag);
        return __comp_offset != entity_tags[(uint32_t)_ent].end() && __comp_offset->second != TAG_NO_COMPONENT;
    }

    uint32_t ComponentStorage::getComponentOffset(EntityID _ent, TagID _tag) const {
        if (uint32_t(_ent) >= entity_tags.size()) return TAG_NO_COMPONENT;
        auto   __comp_offset  = entity_tags[(uint32_t)_ent].find(_tag);
        return __comp_offset != entity_tags[(uint32_t)_ent].end()  ? __comp_offset->second  : TAG_NO_COMPONENT;
    }

    bool ComponentStorage::hasAllTags(EntityID _ent, const std::vector<TagID> &_tags) const {
        if (uint32_t(_ent) >= entity_tags.size()) return false;

        bool __res = true;
        for (auto tag : _tags) {
            if (entity_tags[(uint32_t)_ent].find(tag) == entity_tags[(uint32_t)_ent].end()) {
                __res = false; break;
            }
        }
        return __res;
    }

    bool ComponentStorage::hasAnyTags(EntityID _ent, const std::vector<TagID> &_tags) const {
        if (uint32_t(_ent) >= entity_tags.size()) return false;

        bool __res = false;
        for (auto tag : _tags) {
            if (entity_tags[(uint32_t)_ent].find(tag) != entity_tags[(uint32_t)_ent].end()) {
                __res = true; break;
            }
        }
        return __res;
    }

    bool ComponentStorage::hasNoTags(EntityID _ent, const std::vector<TagID> &_tags) const {
        if (uint32_t(_ent) >= entity_tags.size()) return false;

        bool __res = true;
        for (auto tag : _tags) {
            if (entity_tags[(uint32_t)_ent].find(tag) != entity_tags[(uint32_t)_ent].end()) {
                __res = false; break;
            }
        }
        return __res;
    }

    uint32_t ComponentStorage::addTag(EntityID _ent, TagID _tag) {
        if (uint32_t(_ent) >= entity_tags.size()) return TAG_NO_COMPONENT;
        entity_tags[(uint32_t)_ent].insert({_tag, TAG_NO_COMPONENT});
        return entity_tags[(uint32_t)_ent][_tag];
    }

    void ComponentStorage::addTags(EntityID _ent, const std::vector<TagID> &_tags) {
        if (uint32_t(_ent) >= entity_tags.size()) return;
        auto _tag_map = TagMapFromVector(_tags);
        entity_tags[(uint32_t)_ent].insert(_tag_map.begin(), _tag_map.end());
    }

    void ComponentStorage::addComponent(const Component &_comp) {
        if (uint32_t(_comp.entity) >= entity_tags.size() || !_comp.isValid()) return;

        uint32_t __comp_offset = addTag(_comp.entity, _comp.tag);
        if (__comp_offset == TAG_NO_COMPONENT) {
            __comp_offset = (uint32_t) components.size();
            components.emplace_back();
        }
        components[__comp_offset] = _comp;
    }

    void ComponentStorage::addEntityComponents(EntityID _ent, const std::vector<Component> &_components) {
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

    bool ComponentStorage::tryReplaceDeletedComponent(const Component &_comp) {
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

    bool ComponentStorage::delTag(EntityID _ent, TagID _tag) {
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

    void ComponentStorage::delTags(EntityID _ent, const std::vector<TagID> &_tags) {
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

    void ComponentStorage::deleteEntity(EntityID _ent) {
        if (uint32_t(_ent) >= entity_tags.size()) return;

        std::vector<TagID> __tags(entity_tags[(uint32_t)_ent].size());
        auto __tags_iter = entity_tags[(uint32_t)_ent].begin();
        for (uint32_t i = 0u; i < __tags.size(); ++i)
            __tags[i] = (__tags_iter++)->first;
        delTags(_ent, __tags);
        deleted_entities.insert(_ent);
    }

    void ComponentStorage::defragComponents() {
        std::sort(components.begin(), components.end());
        for (int i = components.size() - 1; i >= 0; --i)
            entity_tags[(uint32_t)components[i].entity][components[i].tag] = i;
    }

    std::vector<EntityID> ComponentStorage::gatherEntities(const std::vector<TagID> &_all_tags, const std::vector<TagID> &_any_tags, const std::vector<TagID> &_no_tags) const {
        std::vector<EntityID> __res_entities;
        for (uint32_t i = 0u; i < entity_tags.size(); ++i) {
            bool __entity_fits = (_all_tags.size() ? hasAllTags(EntityID(i), _all_tags) : true) &&
                                 (_any_tags.size() ? hasAnyTags(EntityID(i), _any_tags) : true) &&
                                 ( _no_tags.size() ? hasNoTags (EntityID(i),  _no_tags) : true);
            if (__entity_fits) __res_entities.push_back(EntityID(i));
        }
        return __res_entities;
    }
}; // Simple END
