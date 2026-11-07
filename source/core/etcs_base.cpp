#include "core/etcs_base.h"

namespace Simple {

    const EntityID EntityID::None;

    namespace ETCS {
        uint32_t nextComponentType() {
            static uint32_t __count = 0u;
            return __count++;
        }
    }; // ETCS END

    ComponentStorage::~ComponentStorage() {
        for (ETCS::PoolBase *pool : pools_) delete pool;
    }

    EntityID ComponentStorage::create(TagMask _tags) {
        uint32_t __index;
        if (!free_.empty()) {
            __index = free_.back();
            free_.pop_back();
        } else {
            __index = (uint32_t)generations_.size();
            generations_.push_back(0u);
            tags_.push_back(0u);
            alive_.push_back(0u);
        }
        tags_ [__index] = _tags;
        alive_[__index] = 1u;
        ++alive_count_;
        return EntityID(__index, generations_[__index]);
    }

    void ComponentStorage::destroy(EntityID _entity) {
        if (!isValid(_entity)) return;
        for (ETCS::PoolBase *pool : pools_)
            if (pool) pool->remove(_entity.index);
        tags_ [_entity.index] = 0u;
        alive_[_entity.index] = 0u;
        ++generations_[_entity.index];
        free_.push_back(_entity.index);
        --alive_count_;
    }

    bool ComponentStorage::isValid(EntityID _entity) const {
        return _entity.index < alive_.size() && alive_[_entity.index] && generations_[_entity.index] == _entity.generation;
    }

    void ComponentStorage::clear() {
        for (ETCS::PoolBase *pool : pools_)
            if (pool) pool->clear();
        // Generations go on, so ids of the cleared entities stay invalid
        free_.clear();
        for (uint32_t i = (uint32_t)generations_.size(); i-- > 0u; ) {
            if (alive_[i]) ++generations_[i];
            alive_[i] = 0u;
            tags_ [i] = 0u;
            free_.push_back(i);
        }
        alive_count_ = 0u;
    }

    bool ComponentStorage::hasTag(EntityID _entity, TagID _tag) const {
        return isValid(_entity) && (tags_[_entity.index] & tagMask(_tag)) != 0u;
    }

    bool ComponentStorage::hasAllTags(EntityID _entity, TagMask _tags) const {
        return isValid(_entity) && (tags_[_entity.index] & _tags) == _tags;
    }

    bool ComponentStorage::hasAnyTags(EntityID _entity, TagMask _tags) const {
        return isValid(_entity) && (tags_[_entity.index] & _tags) != 0u;
    }

    bool ComponentStorage::hasNoTags(EntityID _entity, TagMask _tags) const {
        return isValid(_entity) && (tags_[_entity.index] & _tags) == 0u;
    }

    TagMask ComponentStorage::tags(EntityID _entity) const {
        return isValid(_entity) ? tags_[_entity.index] : 0u;
    }

    void ComponentStorage::addTag(EntityID _entity, TagID _tag) { addTags(_entity, tagMask(_tag)); }

    void ComponentStorage::addTags(EntityID _entity, TagMask _tags) {
        if (isValid(_entity)) tags_[_entity.index] |= _tags;
    }

    bool ComponentStorage::delTag(EntityID _entity, TagID _tag) { return delTags(_entity, tagMask(_tag)); }

    bool ComponentStorage::delTags(EntityID _entity, TagMask _tags) {
        if (!isValid(_entity) || (tags_[_entity.index] & _tags) == 0u) return false;
        tags_[_entity.index] &= ~_tags;
        return true;
    }

    std::vector<EntityID> ComponentStorage::gather(const TagFilter &_filter) const {
        std::vector<EntityID> __res;
        for (uint32_t i = 0u; i < (uint32_t)alive_.size(); ++i)
            if (alive_[i] && _filter.matches(tags_[i])) __res.push_back(EntityID(i, generations_[i]));
        return __res;
    }
}; // Simple END
