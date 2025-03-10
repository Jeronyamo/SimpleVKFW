#ifndef SVKFW_ECSBASE_H
#define SVKFW_ECSBASE_H

#include "main/dbops.h"

#include <atomic>
#include <vector>

#ifdef SVKFW_CHAIN_LEN_EXTRA
typedef unsigned ChainLenType;
#else
typedef unsigned char ChainLenType;
#endif


namespace Simple {
    //  ============  ECS utilities  ============  \\

    using std::pair;
    using std::vector;

    enum struct EntityID : unsigned;
    enum struct TagID : unsigned;

    template <bool IsTypeEnum>
    using TypeSwitchForTag = typename std::conditional<IsTypeEnum, TagID, unsigned>::type;

    template <bool IsTypeEnum>
    using TypeSwitchForEntity = typename std::conditional<IsTypeEnum, EntityID, unsigned>::type;

    typedef bool (*EntitiesWhereFunc)(const un_mulset<TagID> &_mset);
    typedef bool (*EntitiesWhereFunc2)(EntityID _ent, const un_mulset<TagID> &_mset);

// Example of predefined WhereFunction
    template <TagID tag>
    EntitiesWhereFunc CondWithTag = [](const un_mulset<TagID> &t) { return t.find(tag) != t.end(); };

// TODO: REDO
    std::atomic<unsigned> EntityIDgenerator = 1;
    std::atomic<unsigned> TagIDgenerator = 1;


// Assuming _m1.size() <= _m2.size()
    un_map<EntityID, un_mulset<TagID>> _UnionMaxOf2(const un_map<EntityID, un_mulset<TagID>> &_m1,
                                                    const un_map<EntityID, un_mulset<TagID>> &_m2) {
        un_map<EntityID, un_mulset<TagID>> _tmp{_m2};
        for (auto& elem : _m1) {
            if (_tmp.find(elem.first) != _tmp.end())
                _tmp[elem.first] = UNION_MAX_OF_TWO(_tmp[elem.first], elem.second);
            else _tmp[elem.first] = elem.second;
        }
        return _tmp;
    }

// Assuming _m1.size() <= _m2.size()
    un_map<EntityID, un_mulset<TagID>> _InJoinMinOf2(const un_map<EntityID, un_mulset<TagID>> &_m1,
                                                     const un_map<EntityID, un_mulset<TagID>> &_m2) {
        un_map<EntityID, un_mulset<TagID>> _tmp{_m2};
        for (auto& elem : _m1) {
            if (_tmp.find(elem.first) != _tmp.end())
                _tmp[elem.first] = UNION_MIN_OF_TWO(_tmp[elem.first], elem.second);
        }
        return _tmp;
    }


    //  ============  Entities  ============  \\

    struct Entities {
        un_map<EntityID, un_mulset<TagID>> map;


        Entities operator|(const Entities &_ent) const {
            if (this->map.size() <= _ent.map.size()) {
                return { _UnionMaxOf2(this->map, _ent.map) };
            }
            else {
                return { _UnionMaxOf2(_ent.map, this->map) };
            }
        }

        Entities operator&(const Entities &_ent) const {
            if (this->map.size() <= _ent.map.size()) {
                return { _InJoinMinOf2(this->map, _ent.map) };
            }
            else {
                return { _InJoinMinOf2(_ent.map, this->map) };
            }
        }

        Entities operator|(const un_set<EntityID> &_ent) const {
            un_map<EntityID, un_mulset<TagID>> _tmp{ map };

            for (auto& elem : _ent)
                _tmp[elem];
            return { _tmp };
        }

        Entities operator&(const un_set<EntityID> &_ent) const {
            un_map<EntityID, un_mulset<TagID>> _tmp;

            if (_ent.size() < map.size()) {
                for (auto& elem : _ent) {
                    auto iter = map.find(elem);
                    if (iter != map.end())
                        _tmp.insert(*iter);
                }
            }
            else {
                for (auto& elem : map) {
                    if (_ent.find(elem.first) != _ent.end())
                        _tmp.insert(elem);
                }
            }
            return { _tmp };
        }

// Returns a set of map keys
        un_set<EntityID> keys() {
            un_set<EntityID> _tmp(map.size());

            for (auto& elem : map)
                _tmp.insert(elem.first);
            return _tmp;
        }


// Returns submap of entities-attributes such that Where Function of (multiset<TagID>) returns true
        Entities Where(EntitiesWhereFunc _func) const {
            un_map<EntityID, un_mulset<TagID>> _tmp;

            for (auto& elem : map) {
                if (_func(elem.second))
                    _tmp.insert(elem);
            }
            return { _tmp };
        }

// Returns submap of entities-attributes such that EntitiesWhereFunc2 of (EntityID, multiset<TagID>) returns true
        Entities Where(EntitiesWhereFunc2 _func) const {
            un_map<EntityID, un_mulset<TagID>> _tmp;

            for (auto& elem : map) {
                if (_func(elem.first, elem.second))
                    _tmp.insert(elem);
            }
            return { _tmp };
        }

// Returns submap of entities-attributes that contain "tag"
        Entities With(TagID tag) const {
            un_map<EntityID, un_mulset<TagID>> _tmp;

            for (auto& elem : map) {
                if (elem.second.find(tag) != elem.second.end())
                    _tmp.insert(elem);
            }
            return { _tmp };
        }

// Returns submap of entities-attributes that contain all tags from "tags" argument
        Entities WithALL(const un_set<TagID> &tags) const {
            un_map<EntityID, un_mulset<TagID>> _tmp;

            for (auto& elem : map) {
                bool flag = false;

                for (auto tag : tags)
                    if (flag = elem.second.find(tag) == elem.second.end())
                        break;

                if (!flag) _tmp.insert(elem);
            }
            return { _tmp };
        }
// Returns submap of entities-attributes that contain any tag from "tags" argument
        Entities WithANY(const un_set<TagID> &tags) const {
            un_map<EntityID, un_mulset<TagID>> _tmp;

            for (auto& elem : map) {
                bool flag = false;

                for (auto tag : tags)
                    if (flag = elem.second.find(tag) != elem.second.end())
                        break;

                if (flag) _tmp.insert(elem);
            }
            return { _tmp };
        }

// If entity has attribute multiset, adds _tag to the multiset
        int AddTag(EntityID _ent, TagID _tag) {
            auto elem = map.find(_ent);
            if (elem != map.end()) {
                elem->second.insert(_tag);
                return 0;
            }
            return -1;
        }

// If entity has attribute multiset, ensures there's exactly one occurence of the _tag in the multiset
        int AddTagUnique(EntityID _ent, TagID _tag) {
            auto elem = map.find(_ent);
            if (elem != map.end()) {
                unsigned cnt = elem->second.count(_tag);
                if (!cnt)
                    elem->second.insert(_tag);
                while (cnt > 1)
                    elem->second.erase(elem->second.find(_tag));
                return 0;
            }
            return -1;
        }

        int DelTag(EntityID _ent, TagID _tag) {
            auto elem = map.find(_ent);
            if (elem != map.end()) {
                auto _tmp = elem->second.equal_range(_tag);
                elem->second.erase(_tmp.first, _tmp.second);
                return 0;
            }
            return -1;
        }

        int DelTagOnce(EntityID _ent, TagID _tag) {
            auto elem = map.find(_ent);
            if (elem != map.end()) {
                elem->second.erase(_tag);
                return 0;
            }
            return -1;
        }
    };

    Entities operator|(const un_set<EntityID> &_ent, const Entities &_ent2) {
        return _ent2 | _ent;
    }

    Entities operator&(const un_set<EntityID> &_ent, const Entities &_ent2) {
        return _ent2 & _ent;
    }


    //  ============  Components  ============  \\

    struct Component {
        virtual ~Component() {}
    };

    struct Components {
        un_map<TagID, Component*> comp_map;
        un_map<std::pair<TagID, EntityID>, TagID> tag_map;


        Component* ComponentMappingGet(TagID _tag) {
            auto elem = comp_map.find(_tag);
            return elem != comp_map.end() ? elem->second : nullptr;
        }

        template <bool ReturnTypeEnum = true, bool ReturnInputTag = false>
        TypeSwitchForTag<ReturnTypeEnum>
        TagMappingGet(TagID _tag, EntityID _ent) {
            auto elem = tag_map.find(pair<TagID, EntityID>{_tag, _ent});
            return elem != tag_map.end() ? (TypeSwitchForTag<ReturnTypeEnum>) elem->second :
                                           (ReturnInputTag ? (TypeSwitchForTag<ReturnTypeEnum>) _tag :
                                                             (TypeSwitchForTag<ReturnTypeEnum>) 0);
        }

        template <bool ReturnTypeEnum = true, bool ReturnInputTag = false>
        TypeSwitchForTag<ReturnTypeEnum>
        TagMappingGetRecurrent(TagID _tag, EntityID _ent, ChainLenType chain_len = -1) {
            pair<TagID, EntityID> last_key{_tag, _ent};
            auto new_key = tag_map.find(last_key);

            while (new_key != tag_map.end() && chain_len--) {
                last_key.first = new_key->second;
                new_key = tag_map.find(last_key);
            }
            return (ReturnInputTag ? (TypeSwitchForTag<ReturnTypeEnum>) last_key.first :
                                     (TypeSwitchForTag<ReturnTypeEnum>) 0);
        }

        Component* ComponentMappingGetRecurrent(TagID _tag, EntityID _ent, ChainLenType chain_len = -1) {
            if (auto tag = TagMappingGetRecurrent<false>(_tag, _ent, chain_len)) {
                auto elem = comp_map.find((TagID)tag);
                if (elem != comp_map.end())
                    return elem->second;
            }
            return nullptr;
        }



        //  ============  tag_map tag chains  ============  \\ 

// Checks if a tag chain _tagFrom -> ... -> _tagTo for entity _ent exists.
        template <bool CheckForLoops = true>
        bool TagMappingChainExists(TagID _tagFrom, TagID _tagTo, EntityID _ent, ChainLenType chain_len = -1) {
            pair<TagID, EntityID> _key{_tagFrom, _ent};
            un_set<TagID> _tmp;

            for (auto elem = tag_map.find(_key);
                 elem != tag_map.end() && (CheckForLoops ? _tmp.find(elem->second) == _tmp.end() : true) && chain_len--;
                 elem  = tag_map.find(_key)) {
                _key.first = elem->second;
                if (CheckForLoops) _tmp.insert(_key.first);
                if (unsigned(_key.first) == unsigned(_tagTo)) return true;
            }
            return false;
        }

// Checks if a tag chain _tagFrom -> ... -> _tagTo for entity _ent exists and adds chain tags (except _tagFrom and _tagTo) to inner_links
        template <bool CheckForLoops = true>
        bool TagMappingChainToSet(TagID _tagFrom, TagID _tagTo, EntityID _ent, un_set<TagID> *inner_links, ChainLenType chain_len = -1) {
            if (!inner_links) return TagMappingChainExists<CheckForLoops>(_tagFrom, _tagTo, _ent);
            pair<TagID, EntityID> _key{_tagFrom, _ent};
            un_set<TagID> _tmp;

            for (auto elem = tag_map.find(_key);
                 elem != tag_map.end() && (CheckForLoops ? _tmp.find(elem->second) == _tmp.end() : true) && chain_len--;
                 elem  = tag_map.find(_key)) {
                _key.first = elem->second;
                if (unsigned(_key.first) == unsigned(_tagTo)) {
                    inner_links->insert(_tmp);
                    return true;
                }
                _tmp.insert(_key.first);
            }
            return false;
        }

// Checks if a tag chain _tagFrom -> ... -> _tagTo for entity _ent exists and adds chain tags (except _tagFrom and _tagTo) to inner_links
        template <bool CheckForLoops = false>
        bool TagMappingChainToVec(TagID _tagFrom, TagID _tagTo, EntityID _ent, vector<TagID> *inner_links, ChainLenType chain_len = -1) {
            if (!inner_links) return TagMappingChainExists<CheckForLoops>(_tagFrom, _tagTo, _ent);
            pair<TagID, EntityID> _key{_tagFrom, _ent};
            vector<TagID> _tmp;

            for (auto elem = tag_map.find(_key);
                 elem != tag_map.end() && chain_len--;
                 elem  = tag_map.find(_key)) {
                _key.first = elem->second;
                if (unsigned(_key.first) == unsigned(_tagTo)) {
                    inner_links->insert(inner_links->end(), _tmp.begin(), _tmp.end());
                    return true;
                }
                if (CheckForLoops) {
                    for (auto& elem2 : _tmp)
                        if (_key.first == elem2) return false;
                }
                _tmp.push_back(_key.first);
            }
            return false;
        }


// separate function with un_set<TagID> _tagTo, chain _tagFrom -> any of _tagTo, inserts the last element into inner_links
        template <bool CheckForLoops = false>
        bool TagMappingChainAnyExists(TagID _tagFrom, const un_set<TagID> &_tagTo, EntityID _ent, ChainLenType chain_len = -1) {
            pair<TagID, EntityID> _key{_tagFrom, _ent};
            un_set<TagID> _tmp;

            for (auto elem = tag_map.find(_key);
                 elem != tag_map.end() && (CheckForLoops ? _tmp.find(elem->second) == _tmp.end() : true) && chain_len--;
                 elem  = tag_map.find(_key)) {
                _key.first = elem->second;
                if (CheckForLoops) _tmp.insert(_key.first);
                if (_tagTo.find(_key.first) != _tagTo.end()) return true;
            }
            return false;
        }

        template <bool CheckForLoops = true>
        bool TagMappingChainAnyToSet(TagID _tagFrom, const un_set<TagID> &_tagTo, EntityID _ent, un_set<TagID> *inner_links, ChainLenType chain_len = -1) {
            if (!inner_links) return TagMappingChainAnyExists<CheckForLoops>(_tagFrom, _tagTo, _ent);
            pair<TagID, EntityID> _key{_tagFrom, _ent};
            un_set<TagID> _tmp;

            for (auto elem = tag_map.find(_key);
                 elem != tag_map.end() && (CheckForLoops ? _tmp.find(elem->second) == _tmp.end() : true) && chain_len--;
                 elem  = tag_map.find(_key)) {
                _key.first = elem->second;
                if (_tagTo.find(_key.first) != _tagTo.end()) {
                    inner_links->insert(_tmp);
                    return true;
                }
                _tmp.insert(_key.first);
            }
            return false;
        }

        template <bool CheckForLoops = false>
        bool TagMappingChainAnyToVec(TagID _tagFrom, const un_set<TagID> &_tagTo, EntityID _ent, vector<TagID> *inner_links, ChainLenType chain_len = -1) {
            if (!inner_links) return TagMappingChainAnyExists<CheckForLoops>(_tagFrom, _tagTo, _ent);
            pair<TagID, EntityID> _key{_tagFrom, _ent};
            vector<TagID> _tmp;

            for (auto elem = tag_map.find(_key);
                 elem != tag_map.end() && chain_len--;
                 elem  = tag_map.find(_key)) {
                _key.first = elem->second;
                if (_tagTo.find(_key.first) != _tagTo.end()) {
                    inner_links->insert(inner_links->end(), _tmp.begin(), _tmp.end());
                    return true;
                }
                if (CheckForLoops) {
                    for (auto& elem2 : _tmp)
                        if (_key.first == elem2) return false;
                }
                _tmp.push_back(_key.first);
            }
            return false;
        }



// Checks if there is a chain with all elements from un_set<TagID> _tagTo
        template <bool CheckForLoops = false>
        bool TagMappingChainAllExists(TagID _tagFrom, const un_set<TagID> &_tagTo, EntityID _ent, ChainLenType chain_len = -1) {
            pair<TagID, EntityID> _key{_tagFrom, _ent};
            un_set<TagID> _tmp, _tmp2;

            for (auto elem = tag_map.find(_key);
                 elem != tag_map.end() && (CheckForLoops ? _tmp.find(elem->second) == _tmp.end() : true) && chain_len--;
                 elem  = tag_map.find(_key)) {
                _key.first = elem->second;
                if (_tagTo.find(_key.first) != _tagTo.end()) {
                    _tmp2.insert(_key.first);
                    if (_tmp2.size() == _tagTo.size()) return true;
                }
                if (CheckForLoops) _tmp.insert(_key.first);
            }
            return false;
        }

        template <bool CheckForLoops = false>
        bool TagMappingChainAllToSet(TagID _tagFrom, const un_set<TagID> &_tagTo, EntityID _ent, un_set<TagID> *inner_links, ChainLenType chain_len = -1) {
            if (!inner_links) return TagMappingChainAllExists<CheckForLoops>(_tagFrom, _tagTo, _ent);
            pair<TagID, EntityID> _key{_tagFrom, _ent};
            un_set<TagID> _tmp, _tmp2;

            for (auto elem = tag_map.find(_key);
                 elem != tag_map.end() && (CheckForLoops ? _tmp.find(elem->second) == _tmp.end() : true) && chain_len--;
                 elem  = tag_map.find(_key)) {
                _key.first = elem->second;
                if (_tagTo.find(_key.first) != _tagTo.end()) {
                    _tmp2.insert(_key.first);
                    if (_tmp2.size() == _tagTo.size()) {
                        inner_links->insert(_tmp);
                        return true;
                    }
                }
                _tmp.insert(_key.first);
            }
            return false;
        }

        template <bool CheckForLoops = false>
        bool TagMappingChainAllToVec(TagID _tagFrom, const un_set<TagID> &_tagTo, EntityID _ent, vector<TagID> *inner_links, ChainLenType chain_len = -1) {
            if (!inner_links) return TagMappingChainAllExists<CheckForLoops>(_tagFrom, _tagTo, _ent);
            pair<TagID, EntityID> _key{_tagFrom, _ent};
            vector<TagID> _tmp;
            un_set<TagID> _tmp2;

            for (auto elem = tag_map.find(_key);
                 elem != tag_map.end() && (CheckForLoops ? _tmp.find(elem->second) == _tmp.end() : true) && chain_len--;
                 elem  = tag_map.find(_key)) {
                _key.first = elem->second;
                if (_tagTo.find(_key.first) != _tagTo.end()) {
                    _tmp2.insert(_key.first);
                    if (_tmp2.size() == _tagTo.size()) {
                        inner_links->insert(inner_links->end(), _tmp.begin(), _tmp.end());
                        return true;
                    }
                }
                _tmp.push_back(_key.first);
            }
            return false;
        }



// If a tag chain _tagFrom -> ... -> _tagTo for entity _ent exists, deletes all inner links (resulting in _tagFrom -> _tagTo chain)
        bool TagMappingChainCollapse(TagID _tagFrom, TagID _tagTo, EntityID _ent) {
            un_set<TagID> inner_links;

            if (TagMappingChainToSet(_tagFrom, _tagTo, _ent, &inner_links)) {
                for (auto& elem : inner_links)
                    tag_map.erase({elem, _ent});
                return true;
            }
            return false;
        }

// If a tag chain _tagFrom -> ... -> _tagTo for entity _ent exists, deletes all inner links
// except the ones in "_except_tags" (resulting in _tagFrom -> _except_tags -> _tagTo chain)
        bool TagMappingChainCollapseExcept(TagID _tagFrom, TagID _tagTo, EntityID _ent, const un_set<TagID> &_except_tags) {
            if (!_except_tags.size()) return TagMappingChainCollapse(_tagFrom, _tagTo, _ent);
            vector<TagID> inner_links;
            TagID last_tag = _tagFrom;

            if (TagMappingChainToVec(_tagFrom, _tagTo, _ent, &inner_links)) {
                for (auto& elem : inner_links) {
                    if (_except_tags.count(elem)) {
                        tag_map[{last_tag, _ent}] = elem;
                        last_tag = elem;
                    }
                    else tag_map.erase({elem, _ent});
                }
                tag_map[{last_tag, _ent}] = _tagTo;
                return true;
            }
            return false;
        }

// Adds "(_tagFrom, _ent) : _tagTo" to tag_map and deletes a (_tagFrom -> ... -> _tagTo) chain if exists
        void TagMappingInsertDeletingChain(TagID _tagFrom, TagID _tagTo, EntityID _ent) {
            TagMappingChainCollapse(_tagFrom, _tagTo, _ent);
            tag_map[{_tagFrom, _ent}] = _tagTo;
        }


        //  ============  Component* deletion  ============  \\

        void DeleteComponents(const un_set<TagID> &tags_to_elems) {
            for (auto elem : tags_to_elems) {
                auto elem2 = comp_map.find(elem);

                if (elem2 != comp_map.end())
                    delete   elem2->second;
                comp_map.erase(elem2);
            }
        }

        void DeleteComponentArrays(const un_set<TagID> &tags_to_arrs) {
            for (auto elem : tags_to_arrs) {
                auto elem2 = comp_map.find(elem);

                if (elem2 != comp_map.end())
                    delete[] elem2->second;
                comp_map.erase(elem2);
            }
        }

        void DeleteComponentsAll() {
            for (auto elem : comp_map)
                delete elem.second;
            comp_map.clear();
        }

        void DeleteComponentArraysAll() {
            for (auto elem : comp_map)
                delete[] elem.second;
            comp_map.clear();
        }
    };
};

#endif