#ifndef SVKFW_ETCS_BASE_H
#define SVKFW_ETCS_BASE_H

#include <cstdint>
#include <vector>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <string>

#include "common/terminal.h"
#include "fun/etcs_tags.h"


namespace Simple {
//  == === ==== =================================== ==== === ==  \\
                    Entity tag-component system
//  == === ==== =================================== ==== === ==  \\

    // Note: an entity is an id with tags (flags, 'TagID') and components (data of any type, at most one of each type).
    //       Tags are bits of one 'TagMask' per entity, components live in one pool per type (a sparse set: the data of all
    //       entities that have the component is packed in one array, plus entity -> slot and slot -> entity tables), so
    //       systems run over arrays of the components they need.
    //
    //       ComponentStorage ecs;
    //       EntityID ball = ecs.create(tagMask(TagID::TAG_PHYSICS_DYNAMIC));
    //       ecs.add<Physics::Body>(ball).mass = 2.f;
    //       ecs.each<Physics::Body>([](EntityID _e, Physics::Body &_b) { ... }, TagFilter().all(TagID::TAG_PHYSICS_DYNAMIC));
    //
    //       An entity id keeps the slot index and its generation: after 'destroy' the slot is reused with the next generation,
    //       so old ids become invalid ('isValid') instead of pointing to the new entity.
    //       Component references and pointers stay valid until a component of the same type is added or removed.

    struct EntityID {
        uint32_t index = UINT32_MAX;
        uint32_t generation = 0u;

        EntityID() {}
        EntityID(uint32_t _index, uint32_t _generation) : index(_index), generation(_generation) {}

        bool isNone() const { return index == UINT32_MAX; }
        bool operator==(const EntityID &_other) const { return index == _other.index && generation == _other.generation; }
        bool operator!=(const EntityID &_other) const { return !(*this == _other); }
        bool operator< (const EntityID &_other) const { return index < _other.index || (index == _other.index && generation < _other.generation); }
        uint64_t hash() const { return (uint64_t(generation) << 32) | index; }

        static const EntityID None;
    }; // EntityID END


    typedef uint64_t TagMask;

    inline TagMask tagMask(TagID _tag) { return TagMask(1u) << (uint32_t)_tag; }
    inline TagMask tagMask(TagID _tag1, TagID _tag2) { return tagMask(_tag1) | tagMask(_tag2); }
    inline TagMask tagMask(TagID _tag1, TagID _tag2, TagID _tag3) { return tagMask(_tag1) | tagMask(_tag2) | tagMask(_tag3); }

    // Note: selects entities by tags: all of 'all_tags', at least one of 'any_tags' (if not empty), none of 'no_tags'
    struct TagFilter {
        TagMask all_tags = 0u, any_tags = 0u, no_tags = 0u;

        TagFilter() {}
        TagFilter(TagMask _all, TagMask _any = 0u, TagMask _no = 0u) : all_tags(_all), any_tags(_any), no_tags(_no) {}

        TagFilter &all (TagID _tag) { all_tags |= tagMask(_tag); return *this; }
        TagFilter &any (TagID _tag) { any_tags |= tagMask(_tag); return *this; }
        TagFilter &none(TagID _tag) { no_tags  |= tagMask(_tag); return *this; }

        bool matches(TagMask _tags) const {
            return (_tags & all_tags) == all_tags && (any_tags == 0u || (_tags & any_tags) != 0u) && (_tags & no_tags) == 0u;
        }
    }; // TagFilter END


    namespace ETCS {
        const uint32_t NO_SLOT = UINT32_MAX;

        // A small number per component type, given in the order of first use
        uint32_t nextComponentType();
        template <class T> uint32_t componentType() {
            static const uint32_t __type = nextComponentType();
            return __type;
        }

        // Note: the type-independent part of a pool: entity index -> slot ('sparse') and slot -> entity ('dense')
        struct PoolBase {
            std::vector<uint32_t> sparse;
            std::vector<EntityID> dense;

            virtual ~PoolBase() {}

            bool     has (uint32_t _index) const { return _index < sparse.size() && sparse[_index] != NO_SLOT; }
            uint32_t slot(uint32_t _index) const { return _index < sparse.size() ? sparse[_index] : NO_SLOT; }
            uint32_t size() const { return (uint32_t)dense.size(); }

            virtual void remove(uint32_t _index) = 0;
            virtual void clear() = 0;
        }; // PoolBase END

        template <class T>
        struct Pool : PoolBase {
            std::vector<T> data;

            T &add(EntityID _entity, const T &_value) {
                if (sparse.size() <= _entity.index) sparse.resize(_entity.index + 1u, NO_SLOT);
                uint32_t &__slot = sparse[_entity.index];
                if (__slot != NO_SLOT) { dense[__slot] = _entity; data[__slot] = _value; return data[__slot]; }
                __slot = (uint32_t)dense.size();
                dense.push_back(_entity);
                data.push_back(_value);
                return data.back();
            }

            T       *get(uint32_t _index)       { const uint32_t __s = slot(_index); return __s == NO_SLOT ? nullptr : &data[__s]; }
            const T *get(uint32_t _index) const { const uint32_t __s = slot(_index); return __s == NO_SLOT ? nullptr : &data[__s]; }

            // The last component moves into the freed slot
            void remove(uint32_t _index) override {
                const uint32_t __s = slot(_index);
                if (__s == NO_SLOT) return;
                const uint32_t __last = (uint32_t)dense.size() - 1u;
                if (__s != __last) {
                    dense[__s] = dense[__last];
                    data [__s] = std::move(data[__last]);
                    sparse[dense[__s].index] = __s;
                }
                dense.pop_back();
                data.pop_back();
                sparse[_index] = NO_SLOT;
            }

            void clear() override { sparse.clear(); dense.clear(); data.clear(); }
        }; // Pool END
    }; // ETCS END


    class ComponentStorage {
        std::vector<uint32_t> generations_;  // per entity slot
        std::vector<TagMask>  tags_;         // per entity slot
        std::vector<uint8_t>  alive_;        // per entity slot
        std::vector<uint32_t> free_;         // destroyed slots, reused first
        std::vector<ETCS::PoolBase*> pools_; // per component type (nullptr: none created yet)
        uint32_t alive_count_ = 0u;

        template <class T> ETCS::Pool<T> *findPool_() const {
            const uint32_t __type = ETCS::componentType<T>();
            return __type < pools_.size() ? static_cast<ETCS::Pool<T>*>(pools_[__type]) : nullptr;
        }

        // 'each' helpers: the component of every type in 'Rest' (or false if one is missing)
        template <class F, class T, class... Rest>
        struct Each_ {
            template <class... Got>
            static void call(ComponentStorage &_s, F &_f, EntityID _e, Got&... _got) {
                ETCS::Pool<T> *__pool = _s.findPool_<T>();
                T *__c = __pool ? __pool->get(_e.index) : nullptr;
                if (__c) Each_<F, Rest...>::call(_s, _f, _e, _got..., *__c);
            }
        };
        template <class F, class T>
        struct Each_<F, T> {
            template <class... Got>
            static void call(ComponentStorage &_s, F &_f, EntityID _e, Got&... _got) {
                ETCS::Pool<T> *__pool = _s.findPool_<T>();
                T *__c = __pool ? __pool->get(_e.index) : nullptr;
                if (__c) _f(_e, _got..., *__c);
            }
        };

    public:
        ComponentStorage() {}
        ~ComponentStorage();
        ComponentStorage(const ComponentStorage&) = delete;
        ComponentStorage &operator=(const ComponentStorage&) = delete;

    // Entities

        EntityID create(TagMask _tags = 0u);
        // Removes all components; the id (and copies of it) become invalid
        void     destroy(EntityID _entity);
        bool     isValid(EntityID _entity) const;
        uint32_t count() const { return alive_count_; }
        // All entities and components
        void     clear();

    // Tags

        bool    hasTag    (EntityID _entity, TagID _tag) const;
        bool    hasAllTags(EntityID _entity, TagMask _tags) const;
        bool    hasAnyTags(EntityID _entity, TagMask _tags) const;
        bool    hasNoTags (EntityID _entity, TagMask _tags) const;
        TagMask tags      (EntityID _entity) const; // 0 for invalid entities
        void    addTag (EntityID _entity, TagID _tag);
        void    addTags(EntityID _entity, TagMask _tags);
        // Return 'true' if the entity had the tag(s)
        bool    delTag (EntityID _entity, TagID _tag);
        bool    delTags(EntityID _entity, TagMask _tags);

    // Components

        // Adds the component (or overwrites the one the entity has); returns it
        template <class T> T &add(EntityID _entity, const T &_value = T()) {
            SVKFW_ASSERT(isValid(_entity), std::invalid_argument, "ComponentStorage :: add", "invalid entity");
            const uint32_t __type = ETCS::componentType<T>();
            if (pools_.size() <= __type) pools_.resize(__type + 1u, nullptr);
            if (!pools_[__type]) pools_[__type] = new ETCS::Pool<T>();
            return static_cast<ETCS::Pool<T>*>(pools_[__type])->add(_entity, _value);
        }

        // nullptr if the entity is invalid or has no such component
        template <class T> T *get(EntityID _entity) {
            ETCS::Pool<T> *__pool = findPool_<T>();
            return __pool && isValid(_entity) ? __pool->get(_entity.index) : nullptr;
        }
        template <class T> const T *get(EntityID _entity) const {
            const ETCS::Pool<T> *__pool = findPool_<T>();
            return __pool && isValid(_entity) ? __pool->get(_entity.index) : nullptr;
        }

        template <class T> bool has(EntityID _entity) const {
            const ETCS::Pool<T> *__pool = findPool_<T>();
            return __pool && isValid(_entity) && __pool->has(_entity.index);
        }

        template <class T> bool remove(EntityID _entity) {
            ETCS::Pool<T> *__pool = findPool_<T>();
            if (!__pool || !isValid(_entity) || !__pool->has(_entity.index)) return false;
            __pool->remove(_entity.index);
            return true;
        }

        // The packed components of one type (e.g. for a system that runs over all of them); nullptr if none were added yet
        template <class T> ETCS::Pool<T> *pool() { return findPool_<T>(); }

    // Queries

        // All entities whose tags match '_filter'
        std::vector<EntityID> gather(const TagFilter &_filter = TagFilter()) const;

        // Calls '_func(EntityID, T&, Rest&...)' for every entity that has all the components and matches '_filter'.
        // Runs over the pool of 'T' (put the rarest component first). Don't add or remove components of these types
        // inside '_func' (collect the entities and change them afterwards)
        template <class T, class... Rest, class F>
        void each(F _func, const TagFilter &_filter = TagFilter()) {
            ETCS::Pool<T> *__pool = findPool_<T>();
            if (!__pool) return;
            for (uint32_t i = 0u; i < __pool->size(); ++i) {
                const EntityID __e = __pool->dense[i];
                if (!_filter.matches(tags_[__e.index])) continue;
                Each_<F, T, Rest...>::call(*this, _func, __e);
            }
        }
    }; // ComponentStorage END
}; // Simple END

#endif
