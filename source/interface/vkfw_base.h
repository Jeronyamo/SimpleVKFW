#ifndef SVKFW_VKFW_BASE_H
#define SVKFW_VKFW_BASE_H

#include <string>
#include <vector>
#include <initializer_list>

#include "common/memory.h"
#include "interface/glfwrap.h"

#define VKFW_CI_IGNORE UINT32_MAX


namespace Simple {
    namespace VKFW {
// Common

        typedef uint32_t VkfwQueueFamily;
        typedef uint32_t    ContextIndex;
        typedef std::pair<uint32_t, uint32_t> ContextIndex2;

        inline bool shouldIgnoreCI(ContextIndex   _ci_check) { return _ci_check == VKFW_CI_IGNORE; }
        inline bool shouldIgnoreCI(ContextIndex2 _ci2_check) { return _ci2_check.first == VKFW_CI_IGNORE || _ci2_check.second == VKFW_CI_IGNORE; }

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
            bool shouldIgnoreCI2() const { return first == VKFW_CI_IGNORE || second == VKFW_CI_IGNORE; }

            ContextIndex  getCI1() const { return   first; }
            ContextIndex2 getCI2() const { return { first, second }; }
        }; // ContextEntity END


// Utilities

        // Note: non-owning view of a contiguous array (pointer + size), used instead of 'const std::vector<T>&' parameters,
        //       so calls with '{ a, b }', a single value, a C array or a std::vector don't allocate memory.
        //       The viewed data must outlive the Span: don't store a Span created from a braced list.
        template <typename T>
        struct Span {
            const T *ptr = nullptr;
            uint32_t count = 0u;

            Span() {}
            Span(const T &_one) : ptr{&_one}, count{1u} {}
            Span(const T *_ptr, uint32_t _count) : ptr{_ptr}, count{_count} {}
            Span(std::initializer_list<T> _list) : ptr{_list.begin()}, count{(uint32_t)_list.size()} {}
            template <typename A>
            Span(const std::vector<T, A> &_vec) : ptr{_vec.data()}, count{(uint32_t)_vec.size()} {}
            template <size_t N>
            Span(const T (&_arr)[N]) : ptr{_arr}, count{(uint32_t)N} {}

            uint32_t size() const { return count; }
            bool    empty() const { return count == 0u; }
            const T* data() const { return count ? ptr : nullptr; }
            const T* begin() const { return ptr; }
            const T*   end() const { return ptr + count; }
            const T& operator[](uint32_t _id) const { return ptr[_id]; }
        }; // Span END

        #define VKFW_INLINE_ARRAY_SIZE 32u

        // Note: array of a fixed size, set in the constructor; elements are stored inline (on the stack) if there are
        //       at most 'N' of them, otherwise in a std::vector. Used to convert context indices into Vulkan handles.
        template <typename T, uint32_t N = VKFW_INLINE_ARRAY_SIZE>
        struct InlineArray {
            explicit InlineArray(uint32_t _count) : count{_count} { if (count > N) heap.resize(count); }

            uint32_t size() const { return count; }
                  T* data()       { return count > N ? heap.data() : (count ? local : nullptr); }
            const T* data() const { return count > N ? heap.data() : (count ? local : nullptr); }
                  T& operator[](uint32_t _id)       { return data()[_id]; }
            const T& operator[](uint32_t _id) const { return data()[_id]; }

            operator Span<T>() const { return { data(), count }; }

        private:
            T local[N];
            std::vector<T> heap;
            uint32_t count;

            InlineArray(const InlineArray &);
            InlineArray& operator=(const InlineArray &);
        }; // InlineArray END

        enum class CompareOp : uint8_t {
            NE, // not equal
            EQ, // equal
            LT, // less than
            LE, // less than or equal
            GE, // greater than or equal
            GT  // greater than
        }; // CompareOp END

        namespace VulkanAPIVersion {
            inline uint32_t available(uint32_t *_variant = nullptr) {
                uint32_t _version = VK_API_VERSION_1_0;

                if (vkEnumerateInstanceVersion != nullptr)
                    vkEnumerateInstanceVersion( &_version);

                if (_variant != nullptr)
                    *_variant = VK_API_VERSION_VARIANT(_version);
                return _version & ~VK_MAKE_API_VERSION(0x7u, 0u, 0u, 0u); // variant bits are cleared
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
            // Returns the new beginning of the chain ('subchain_begin' if offset == 0, otherwise 'chain_begin').
            static VkBaseOutStructure* InsertStructs(VkBaseOutStructure *_chain_begin, VkBaseOutStructure *_subchain_begin, uint32_t _offset = UINT32_MAX) {
                if (_subchain_begin == nullptr || _chain_begin == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: StructPtrChain :: InsertStructs", "null pointer(s) passed"));

                VkBaseOutStructure *_subchain_end = GetChainLastElem(_subchain_begin);
                VkBaseOutStructure *_chain_before = nullptr;
                VkBaseOutStructure *_chain_after  = _chain_begin;

                if (_offset > 0u) {
                    std::vector<VkBaseOutStructure*> __chain = ExpandPtrChain(_chain_begin);
                    _chain_before = __chain[std::min<size_t>(_offset, __chain.size()) - 1];
                    _chain_after  = _chain_before->pNext;
                    _chain_before->pNext = _subchain_begin;
                }
                _subchain_end->pNext = _chain_after;
                return _offset > 0u ? _chain_begin : _subchain_begin;
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

            // Note: Vulkan structures are trivially destructible, so it's enough to deallocate the memory.
            //       Calling 'delete' through 'VkBaseOutStructure*' for an object of another type is UB, while
            //       '::operator delete' accepts any pointer returned by 'new T' (for non-overaligned 'T').
            void clear() {
                for (auto p : chain) ::operator delete(static_cast<void*>(p));
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
                updateChainLinks();
            }

            template <typename T>
            void addStructBack(VkStructureType _s_type) {
                static_assert(std::is_trivially_destructible<T>::value, "VKFW :: StructPtrChain :: addStructBack - type must be a Vulkan structure");
                chain.push_back(reinterpret_cast<VkBaseOutStructure*>(new T{_s_type}));
                updateChainLinks();
            }

        }; // StructPtrChain END
    }; // VKFW END
}; // Simple END

#endif