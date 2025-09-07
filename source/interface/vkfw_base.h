#ifndef SVKFW_VKFW_BASE_H
#define SVKFW_VKFW_BASE_H

#include <map>
#include <string>
#include <vector>

#include "common/memory.h"
#include "interface/glfwrap.h"


namespace Simple {
// Used to simplify work with all the "const char **" struct members in Vulkan
    struct StringVec {
        std::vector<std::string> list;

        StringVec() {}
        StringVec(const std::vector<std::string> &_l) : list{_l} { updateEntriesPtr(); }
        StringVec(const StringVec &_sv) : StringVec{_sv.list} {}
        StringVec(const char **_arr, uint32_t _count) {
            list.insert(list.end(), _arr, _arr + _count);
            updateEntriesPtr();
        }

        const StringVec &operator=(const StringVec &_sv) {
            setEntries(_sv);
            return *this;
        }

        void clear() {
            list.clear();
            safeDeleteArr(_ptr);
        }

        uint32_t count() const { return list.size(); }


        void setEntries(const StringVec &_sv) {
            list = _sv.list;
            updateEntriesPtr();
        }

        void addEntries(const std::vector<std::string> &_l) {
            list.insert(list.end(), _l.begin(), _l.end());
            updateEntriesPtr();
        }

        const char **getEntries() const {
            return _ptr;
        }

        const char **getEntriesDCopy() const {
            char **__res = nullptr;

            if (!list.empty()) {
                __res = new char*[list.size()]{};

                // TODO: of course it can be done in one new[]
                for (uint32_t i = 0u; i < list.size(); ++i) {
                    __res[i] = new char[list[i].size() + 1]{};
                    memcpy(__res[i], list[i].c_str(), (list[i].size() + 1) * sizeof(char));
                }
            }

            return _ptr;
        }
        // Only for getEntriesDCopy result
        static void DeleteCharPP(const char * const*_char_pp, uint32_t _count) {
            for (uint32_t i = 0u; i < _count; ++i)
                delete[] _char_pp[i];

            delete[] _char_pp;
        }

    private:
        const char **_ptr = nullptr;

        void updateEntriesPtr() {
            safeDeleteArr(_ptr);

            const uint32_t _size = list.size();
            if (_size) {
                _ptr = new const char *[_size];
                for (uint32_t i = 0u; i < _size; ++i)
                    _ptr[i] = list[i].c_str();
            }
        }
    }; // StringVec END


    namespace VKFW {
// Utilities

        enum class CompareOp : uint8_t {
            NE, // not equal
            EQ, // equal
            LT, // less than
            LE, // less than or equal
            GE, // greater than or equal
            GT  // greater than
        }; // VKFWCompareOp END

        namespace VulkanAPIVersion {
            uint32_t available(uint32_t *_variant = nullptr) {
                uint32_t _version = VK_API_VERSION_1_0;

                if (vkEnumerateInstanceVersion != nullptr)
                    vkEnumerateInstanceVersion( &_version);

                if (_variant != nullptr)
                    *_variant = VK_API_VERSION_VARIANT(_version);
                return _version - VK_API_VERSION_VARIANT(_version);
            }

            inline uint32_t make(const vec3u &_version, uint32_t _variant = 0u) {
                return VK_MAKE_API_VERSION(_variant, _version.x, _version.y, _version.z);
            }

            inline vec3u divide(uint32_t _version, uint32_t *_variant = nullptr) {
                if (_variant != nullptr)
                    *_variant = VK_API_VERSION_VARIANT(_version);
                return { VK_API_VERSION_MAJOR(_version),
                         VK_API_VERSION_MINOR(_version),
                         VK_API_VERSION_PATCH(_version) };
            }
        }; // VulkanAPIVersion END

        namespace StructurePtrChain {
            std::vector<VkStructureType> GetAllTypes(const VkBaseOutStructure *_chain_begin) {
                std::vector<VkStructureType> __res;

                for (const VkBaseOutStructure *p = _chain_begin; p != nullptr; p = p->pNext) {
                    if (p == _chain_begin && __res.size())
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: StructurePtrChain :: GetAllTypes", "structure chain is looped"));

                    __res.push_back(p->sType);
                }
                return __res;
            }
            // '_return_last' - if '_index' >= length of the chain: if true, returns last element. If false, returns nullptr.
            VkBaseOutStructure* GetChainElem(VkBaseOutStructure *_chain_begin, uint32_t _index, bool _return_last = false) {
                VkBaseOutStructure *__res = _chain_begin;

                for (uint32_t i = 0u; i < _index; ++i) {
                    if (__res != nullptr) {
                        if (__res->pNext == nullptr && _return_last)
                            return __res;

                        __res = __res->pNext;
                    }

                    if (__res == _chain_begin)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: StructurePtrChain :: GetChainElem", "structure chain is looped"));
                }
                return __res;
            }

            VkBaseOutStructure* GetChainLastElem(VkBaseOutStructure *_chain_begin) {
                VkBaseOutStructure *__res = _chain_begin;

                if (__res == nullptr)
                    return nullptr;

                while (__res->pNext != nullptr) {
                    __res = __res->pNext;

                    if (__res == _chain_begin)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: StructurePtrChain :: GetChainLastElem", "structure chain is looped"));
                }
                return __res;
            }
            // '_offset' is equal to "insert before this index". If offset is greater than length of the chain, inserts to the back.
            void InsertStructs(VkBaseOutStructure *_chain_begin, VkBaseOutStructure *_subchain_begin, uint32_t _offset = UINT32_MAX) {
                if (_subchain_begin == nullptr || _chain_begin == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: StructurePtrChain :: InsertStructs", "null pointer(s) passed"));

                // General:                chain_begin -> ... -> chain_before -> subchain_begin -> ... -> subchain_end -> chain_after -> ... -> chain_end
                // offset 0:            subchain_begin -> ... -> subchain_end ->    chain_begin -> ... ->    chain_end
                // offset >= chain len:    chain_begin -> ... ->    chain_end -> subchain_begin -> ... -> subchain_end

                VkBaseOutStructure *_chain_before = _offset > 0u ? GetChainElem(_chain_begin, _offset - 1, true) : nullptr;
                VkBaseOutStructure *_subchain_end = GetChainLastElem(_subchain_begin);
                VkBaseOutStructure *_chain_after  = _offset > 0u ? _chain_before->pNext : _chain_begin;

                if (_chain_before != nullptr)
                    _chain_before = _subchain_begin;
                _subchain_end->pNext = _chain_after;
            }


            class Handler {
                std::vector<VkStructureType> chain_structures;
                VkBaseOutStructure *chain_begin = nullptr;

            public:
                Handler(VkBaseOutStructure *_chain_begin = nullptr)
                                : chain_structures{GetAllTypes(_chain_begin)}, chain_begin{_chain_begin} {}
                Handler(const Handler &_chain)
                                : chain_structures{_chain.chain_structures}, chain_begin{_chain.chain_begin} {}
                ~Handler() {
                    chain_structures.clear();
                    chain_begin = nullptr;
                }

                void setChain(VkBaseOutStructure *_chain_begin) {
                    chain_begin = _chain_begin;
                    chain_structures = GetAllTypes(chain_begin);
                }

                std::vector<VkStructureType> getAllTypes() const { return chain_structures; }

                VkBaseOutStructure* getChainElem(uint32_t _index, bool _return_last = false) {
                    return GetChainElem(chain_begin, _index, _return_last);
                }

                VkBaseOutStructure* getChainLastElem() {
                    return GetChainLastElem(chain_begin);
                }

                uint32_t size() const {
                    return chain_structures.size();
                }

                void insertStructs(VkBaseOutStructure *_subchain_begin, uint32_t _offset = UINT32_MAX) {
                    if (_subchain_begin == nullptr)
                        throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: StructurePtrChain Handler :: insertStructs", "subchain is nullptr"));

                    InsertStructs(chain_begin, _subchain_begin, _offset);
                    if (_offset == 0)
                        chain_begin = _subchain_begin;
                    chain_structures = GetAllTypes(chain_begin);
                }
            }; // Handler END
        }; // StructurePtrChain END
    }; // VKFW END
}; // Simple END

#endif