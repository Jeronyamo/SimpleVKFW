#ifndef SVKFW_VKFW_BASE_H
#define SVKFW_VKFW_BASE_H

#include <string>
#include <vector>

#include "common/memory.h"
#include "interface/glfwrap.h"

#define VKFW_CI_IGNORE UINT32_MAX


namespace Simple {
    namespace VKFW {
// Common

        typedef uint32_t VkfwQueueFamily;
        typedef uint32_t    ContextIndex;
        typedef std::pair<uint32_t, uint32_t> ContextIndex2;

        bool shouldIgnoreCI(ContextIndex   _ci_check) { return _ci_check == VKFW_CI_IGNORE; }
        bool shouldIgnoreCI(ContextIndex2 _ci2_check) { return _ci2_check.first == VKFW_CI_IGNORE || _ci2_check.second == VKFW_CI_IGNORE; }

        template <typename T>
        struct ContextEntity {
            T* entity_ptr = nullptr;
            ContextIndex first = VKFW_CI_IGNORE, second = VKFW_CI_IGNORE;

            ContextEntity() {}
            ContextEntity(T* _entity_ptr, ContextIndex _first = VKFW_CI_IGNORE, ContextIndex _second = VKFW_CI_IGNORE)
                            : entity_ptr{_entity_ptr}, first{_first}, second{_second} {}
            ContextEntity(T* _entity_ptr, ContextIndex2 _pair)
                            : entity_ptr{_entity_ptr}, first{_pair.first}, second{_pair.second} {}
            ContextEntity(const ContextEntity &_entity)
                            : entity_ptr{_entity.entity_ptr}, first{_entity.first}, second{_entity.second} {}

            T* operator ->() { return entity_ptr; }

            bool shouldIgnoreCI1() const { return first == VKFW_CI_IGNORE; }
            bool shouldIgnoreCI2() const { return first == VKFW_CI_IGNORE && second == VKFW_CI_IGNORE; }

            ContextIndex  getCI1() const { return   first; }
            ContextIndex2 getCI2() const { return { first, second }; }
        }; // ContextEntity END


// Utilities

        enum class CompareOp : uint8_t {
            NE, // not equal
            EQ, // equal
            LT, // less than
            LE, // less than or equal
            GE, // greater than or equal
            GT  // greater than
        }; // CompareOp END

        namespace VulkanAPIVersion {
            uint32_t available(uint32_t *_variant = nullptr) {
                uint32_t _version = VK_API_VERSION_1_0;

                if (vkEnumerateInstanceVersion != nullptr)
                    vkEnumerateInstanceVersion( &_version);

                if (_variant != nullptr)
                    *_variant = VK_API_VERSION_VARIANT(_version);
                return _version - VK_API_VERSION_VARIANT(_version);
            }

            inline uint32_t join(const vec3u &_version, uint32_t _variant = 0u) {
                return VK_MAKE_API_VERSION(_variant, _version.x, _version.y, _version.z);
            }

            inline vec3u split(uint32_t _version, uint32_t *_variant = nullptr) {
                if (_variant != nullptr)
                    *_variant = VK_API_VERSION_VARIANT(_version);
                return { VK_API_VERSION_MAJOR(_version),
                         VK_API_VERSION_MINOR(_version),
                         VK_API_VERSION_PATCH(_version) };
            }
        }; // VulkanAPIVersion END


        // Note: All chained struct pointers are destroyed by the object itself. Add structures by calling 'new ()' or 'addStructBack()'.
        struct StructPtrChain {
        // Static

            static std::vector<VkBaseOutStructure*> ExpandPtrChain(VkBaseOutStructure *_chain_begin) {
                std::vector<VkBaseOutStructure*> __res;

                while (_chain_begin != nullptr) {
                    __res.push_back(_chain_begin);
                    _chain_begin = _chain_begin->pNext;

                    // Safeguard from loops
                    for (uint32_t i = 0u; i < __res.size() - 1; ++i)
                        if (__res[i] == __res.back()) {
                            __res.pop_back();
                            _chain_begin = nullptr;
                            break;
                        }
                }

                return __res;
            }

            static std::vector<VkStructureType> GetAllTypes(VkBaseOutStructure *_chain_begin) {
                auto __all_ptrs = ExpandPtrChain(_chain_begin);
                std::vector<VkStructureType> __res;

                __res.reserve(__all_ptrs.size());
                for (auto p : __all_ptrs) {
                    // TODO: check if having multiple structures of same type is never allowed
                    for (VkStructureType res_t : __res)
                        if (p->sType == res_t)
                            throw std::runtime_error(SVKFW_WRAPERR("VKFW :: StructPtrChain :: GetAllTypes", "structure chain has multiple structures of type " + std::to_string(res_t)));

                    __res.push_back(p->sType);
                }
                return __res;
            }

            static VkBaseOutStructure* GetChainElem(VkBaseOutStructure *_chain_begin, uint32_t _id) {
                auto __res = ExpandPtrChain(_chain_begin);
                return _id < __res.size() ? __res[_id] : nullptr;
            }

            static VkBaseOutStructure* GetChainLastElem(VkBaseOutStructure *_chain_begin) {
                auto __res = ExpandPtrChain(_chain_begin);
                return __res.empty() ? nullptr : __res.back();
            }

            // Note: 'offset' means "insert before this index". If offset is equal to/greater than length of the chain, inserts as last element.
            //     General:                chain_begin -> ... -> chain_before -> subchain_begin -> ... -> subchain_end -> chain_after -> ... -> chain_end
            //     offset == 0:         subchain_begin -> ... -> subchain_end ->    chain_begin -> ... ->    chain_end
            //     offset >= chain len:    chain_begin -> ... ->    chain_end -> subchain_begin -> ... -> subchain_end
            static void InsertStructs(VkBaseOutStructure *_chain_begin, VkBaseOutStructure *_subchain_begin, uint32_t _offset = UINT32_MAX) {
                if (_subchain_begin == nullptr || _chain_begin == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: StructPtrChain :: InsertStructs", "null pointer(s) passed"));

                VkBaseOutStructure *_subchain_end = GetChainLastElem(_subchain_begin);
                VkBaseOutStructure *_chain_before = _offset > 0u ? GetChainElem(_chain_begin, _offset - 1) : nullptr;
                VkBaseOutStructure *_chain_after  = _offset > 0u ? _chain_before->pNext : _chain_begin;

                if (_chain_before != nullptr)
                    _chain_before->pNext = _subchain_begin;
                _subchain_end->pNext = _chain_after;
            }

        // StructPtrChain class

            std::vector<VkBaseOutStructure*> chain;

            StructPtrChain(VkBaseOutStructure *_chain_begin = nullptr) : chain{ExpandPtrChain(_chain_begin)} {}
            StructPtrChain(const std::vector<void *> &_chain_vec) {
                chain.resize(_chain_vec.size());
                for (uint32_t i = 0; i < chain.size(); ++i)
                    chain[i] = (VkBaseOutStructure *) _chain_vec[i];
                updateChainLinks();
            }
            StructPtrChain(StructPtrChain &&_chain) : chain{_chain.chain} { _chain.chain.clear(); }
           ~StructPtrChain() { clear(); }

            void clear() {
                for (auto p : chain) delete p;
                chain.clear();
            }

            void updateChainLinks() {
                for (uint32_t i = 1u; i < chain.size(); ++i)
                    chain[i-1]->pNext = chain[i];
                if (!chain.empty()) chain.back()->pNext = nullptr;
            }

        // Scenarios

            template <typename T = VkBaseOutStructure>
            T* get() {
                return (      T*) (chain.empty() ? nullptr : chain[0]);
            }
            template <typename T = VkBaseOutStructure>
            const T* cget() {
                return (const T*) (chain.empty() ? nullptr : chain[0]);
            }

            template <typename T = VkBaseOutStructure>
            T* getId(uint32_t _chain_id) {
                return (      T*) (_chain_id < chain.size() ? chain[_chain_id] : nullptr);
            }
            template <typename T = VkBaseOutStructure>
            const T* cgetId(uint32_t _chain_id) {
                return (const T*) (_chain_id < chain.size() ? chain[_chain_id] : nullptr);
            }

            template <typename T = VkBaseOutStructure>
            T* getSType(VkStructureType _expected_stype) {
                for (auto p : chain)
                    if (p->sType == _expected_stype) return (      T*) p;
                return nullptr;
            }
            template <typename T = VkBaseOutStructure>
            const T* cgetSType(VkStructureType _expected_stype) {
                for (auto p : chain)
                    if (p->sType == _expected_stype) return (const T*) p;
                return nullptr;
            }

            void setChain(VkBaseOutStructure *_chain_begin) {
                chain = ExpandPtrChain(_chain_begin);
            }

            std::vector<VkStructureType> getAllTypes() const {
                std::vector<VkStructureType> __res(chain.size());
                for (uint32_t i = 0u; i < __res.size(); ++i)
                    __res[i] = chain[i]->sType;
                return __res;
            }

            void insertStructs(VkBaseOutStructure *_subchain_begin, uint32_t _offset = UINT32_MAX) {
                auto __subchain = ExpandPtrChain(_subchain_begin);

                if (__subchain.empty())
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: StructPtrChain :: insertStructs", "subchain is empty"));

                auto __iter_begin = chain.end();
                if (_offset < chain.size())
                    __iter_begin = chain.begin() + _offset;

                chain.insert(__iter_begin, __subchain.begin(), __subchain.end());
            }

            template <typename T>
            void addStructBack(VkStructureType _s_type) {
                chain.push_back(new T{_s_type});
            }
        }; // StructPtrChain END
    }; // VKFW END
}; // Simple END

#endif