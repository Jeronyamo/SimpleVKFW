#ifndef SVKFW_VKFW_BASE_H
#define SVKFW_VKFW_BASE_H

#include <string>
#include <vector>
#include <initializer_list>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#include "common/memory.h"
#include "interface/glfwrap.h"
#include "interface/vkfw_struct_traits.h"

#define VKFW_CI_IGNORE UINT32_MAX


namespace Simple {
    namespace VKFW {
// Common

        typedef uint32_t VkfwQueueFamily;
        typedef uint32_t    ContextIndex;
        typedef std::pair<uint32_t, uint32_t> ContextIndex2;

        bool shouldIgnoreCI(ContextIndex   _ci_check);
        bool shouldIgnoreCI(ContextIndex2 _ci2_check);

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


// Typed Ids

        // Note: typed index into a 'Pool'. 'generation' changes every time the slot is freed,
        //       so the Id of a destroyed object never refers to a newer object that reuses its slot.
        //       A default-constructed Id is "none" (the typed analog of VKFW_CI_IGNORE).
        template <typename Tag>
        struct Id {
            uint32_t index      = UINT32_MAX;
            uint32_t generation = 0u;

            Id() {}
            Id(uint32_t _index, uint32_t _generation) : index{_index}, generation{_generation} {}

            bool isNone() const { return index == UINT32_MAX; }

            bool operator==(const Id &_id) const { return index == _id.index && generation == _id.generation; }
            bool operator!=(const Id &_id) const { return !(*this == _id); }
        }; // Id END

        // Note: slot map. 'add()' reuses freed slots, 'get()' checks that the Id is alive and throws otherwise.
        //       References returned by 'get()' are invalidated by 'add()', because the storage may grow.
        template <typename T, typename Tag>
        class Pool {
        public:
            typedef Id<Tag> IdType;

            IdType add(const T &_value = T()) {
                uint32_t __index;
                if (!free_slots.empty()) {
                    __index = free_slots.back();
                    free_slots.pop_back();
                }
                else {
                    __index = (uint32_t)values.size();
                    values.push_back(T());
                    generations.push_back(1u);
                    alive.push_back(0u);
                }
                values[__index] = _value;
                alive [__index] = 1u;
                return IdType(__index, generations[__index]);
            }

            // Returns false if the Id is none or its object was already removed
            bool remove(IdType _id) {
                if (!contains(_id)) return false;
                values[_id.index] = T();
                alive [_id.index] = 0u;
                ++generations[_id.index];
                free_slots.push_back(_id.index);
                return true;
            }

            // Note: generations are kept, so the Ids of the removed objects stay invalid
            void clear() {
                for (uint32_t i = 0u; i < values.size(); ++i)
                    if (alive[i]) remove(IdType(i, generations[i]));
            }

            bool contains(IdType _id) const {
                return _id.index < values.size() && alive[_id.index] && generations[_id.index] == _id.generation;
            }

                  T& get(IdType _id)       { checkId_(_id); return values[_id.index]; }
            const T& get(IdType _id) const { checkId_(_id); return values[_id.index]; }

            // Calls '_func(IdType, T&)' for every live object
            template <typename F>
            void forEach(F _func) {
                for (uint32_t i = 0u; i < values.size(); ++i)
                    if (alive[i]) _func(IdType(i, generations[i]), values[i]);
            }

            uint32_t size() const { return (uint32_t)(values.size() - free_slots.size()); }
            bool    empty() const { return size() == 0u; }

        private:
            std::vector<T>        values;
            std::vector<uint32_t> generations;
            std::vector<uint8_t>  alive;
            std::vector<uint32_t> free_slots;

            void checkId_(IdType _id) const {
                SVKFW_ASSERT(contains(_id), std::out_of_range, "VKFW::Pool :: get", "Id is none or refers to a destroyed object");
            }
        }; // Pool END

    // Sync
        struct SemaphoreTag {};
        struct FenceTag     {};
        struct EventTag     {};
        typedef Id<SemaphoreTag> SemaphoreId;
        typedef Id<FenceTag>     FenceId;
        typedef Id<EventTag>     EventId;

        // Note: Id of an object that lives inside a parent object and as long as it
        //       (e.g. a buffer inside its memory chunk, a command buffer inside its allocated group),
        //       so it holds the parent's Id (with its generation) and the object's index inside the parent.
        template <typename ParentTag, typename Tag>
        struct SubId {
            Id<ParentTag> parent;
            uint32_t index = UINT32_MAX;

            SubId() {}
            SubId(Id<ParentTag> _parent, uint32_t _index) : parent{_parent}, index{_index} {}

            bool isNone() const { return parent.isNone() || index == UINT32_MAX; }

            bool operator==(const SubId &_id) const { return parent == _id.parent && index == _id.index; }
            bool operator!=(const SubId &_id) const { return !(*this == _id); }
        }; // SubId END

    // Core
        struct QueueFamilyTag {};
        struct QueueTag       {};
        typedef Id   <QueueFamilyTag>           QueueFamilyId;
        typedef SubId<QueueFamilyTag, QueueTag> QueueId;

    // Descriptor
        struct DescriptorSetLayoutTag  {};
        struct DescriptorLayoutBindingTag {};
        struct DescriptorPoolTag       {};
        struct DescriptorSetsTag       {};
        struct DescriptorSetTag        {};
        struct SamplerTag              {};
        typedef Id   <DescriptorSetLayoutTag>                             DescriptorSetLayoutId;
        typedef SubId<DescriptorSetLayoutTag, DescriptorLayoutBindingTag> DescriptorLayoutBindingId; // binding inside the layout create info
        typedef Id   <DescriptorPoolTag>                                  DescriptorPoolId;
        typedef Id   <DescriptorSetsTag>                                  DescriptorSetsId;          // group of sets allocated together
        typedef SubId<DescriptorSetsTag, DescriptorSetTag>                DescriptorSetId;
        typedef Id   <SamplerTag>                                         SamplerId;

    // Pipeline
        struct PipelineTag       {};
        struct PipelineLayoutTag {};
        struct PipelineCacheTag  {};
        struct ShaderModuleTag   {};
        struct ShaderStageTag    {};
        typedef Id   <PipelineTag>                 PipelineId;
        typedef Id   <PipelineLayoutTag>           PipelineLayoutId;
        typedef Id   <PipelineCacheTag>            PipelineCacheId;
        typedef Id   <ShaderModuleTag>             ShaderModuleId;
        typedef SubId<PipelineTag, ShaderStageTag> ShaderStageId; // stage inside the pipeline create info

    // Command
        struct CommandPoolTag    {};
        struct CommandBuffersTag {};
        struct CommandBufferTag  {};
        typedef Id   <CommandPoolTag>                      CommandPoolId;
        typedef Id   <CommandBuffersTag>                   CommandBuffersId; // group of command buffers allocated together
        typedef SubId<CommandBuffersTag, CommandBufferTag> CommandBufferId;

    // Memory
        struct MemoryTag     {};
        struct BufferTag     {};
        struct ImageTag      {};
        struct BufferViewTag {};
        struct ImageViewTag  {};
        typedef Id   <MemoryTag>                MemoryId;
        typedef SubId<MemoryTag, BufferTag>     BufferId;
        typedef SubId<MemoryTag, ImageTag>      ImageId;
        typedef SubId<MemoryTag, BufferViewTag> BufferViewId;
        typedef SubId<MemoryTag, ImageViewTag>  ImageViewId;


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
            uint32_t available(uint32_t *_variant = nullptr);

            uint32_t join(const vec3u &_version, uint32_t _variant = 0u);

            vec3u split(uint32_t _version, uint32_t *_variant = nullptr);
        }; // VulkanAPIVersion END


        // Note: All chained struct pointers are destroyed by the object itself. Add structures by calling 'new ()' or 'addStructBack()'.
        struct StructPtrChain {
        // Static

            static std::vector<VkBaseOutStructure*> ExpandPtrChain(VkBaseOutStructure *_chain_begin);
            static std::vector<VkStructureType> GetAllTypes(VkBaseOutStructure *_chain_begin);
            static VkBaseOutStructure* GetChainElem(VkBaseOutStructure *_chain_begin, uint32_t _id);
            static VkBaseOutStructure* GetChainLastElem(VkBaseOutStructure *_chain_begin);

            // Note: 'offset' means "insert before this index". If offset is equal to/greater than length of the chain, inserts as last element.
            //     General:                chain_begin -> ... -> chain_before -> subchain_begin -> ... -> subchain_end -> chain_after -> ... -> chain_end
            //     offset == 0:         subchain_begin -> ... -> subchain_end ->    chain_begin -> ... ->    chain_end
            //     offset >= chain len:    chain_begin -> ... ->    chain_end -> subchain_begin -> ... -> subchain_end
            // Returns the new beginning of the chain ('subchain_begin' if offset == 0, otherwise 'chain_begin').
            static VkBaseOutStructure* InsertStructs(VkBaseOutStructure *_chain_begin, VkBaseOutStructure *_subchain_begin, uint32_t _offset = UINT32_MAX);

        // StructPtrChain class

            std::vector<VkBaseOutStructure*> chain;

            StructPtrChain(VkBaseOutStructure *_chain_begin = nullptr);
            StructPtrChain(const std::vector<void *> &_chain_vec);
            StructPtrChain(StructPtrChain &&_chain);
           ~StructPtrChain();

            // Note: Vulkan structures are trivially destructible, so it's enough to deallocate the memory.
            //       Calling 'delete' through 'VkBaseOutStructure*' for an object of another type is UB, while
            //       '::operator delete' accepts any pointer returned by 'new T' (for non-overaligned 'T').
            void clear();

            void updateChainLinks();

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

            void setChain(VkBaseOutStructure *_chain_begin);

            std::vector<VkStructureType> getAllTypes() const;

            void insertStructs(VkBaseOutStructure *_subchain_begin, uint32_t _offset = UINT32_MAX);

            template <typename T>
            void addStructBack(VkStructureType _s_type) {
                static_assert(std::is_trivially_destructible<T>::value, "VKFW :: StructPtrChain :: addStructBack - type must be a Vulkan structure");
                chain.push_back(reinterpret_cast<VkBaseOutStructure*>(new T{_s_type}));
                updateChainLinks();
            }

        }; // StructPtrChain END


        namespace Detail {
            template <bool... Bs> struct AnyOf;
            template <> struct AnyOf<> { static const bool value = false; };
            template <bool B, bool... Bs> struct AnyOf<B, Bs...> { static const bool value = B || AnyOf<Bs...>::value; };

            template <typename T, typename... Ts> struct TypeCount;
            template <typename T> struct TypeCount<T> { static const uint32_t value = 0u; };
            template <typename T, typename U, typename... Ts> struct TypeCount<T, U, Ts...> {
                static const uint32_t value = (std::is_same<T, U>::value ? 1u : 0u) + TypeCount<T, Ts...>::value;
            };

            // Note: index of the first 'T' in 'Ts' ('TypeCount' guards against a missing type)
            template <typename T, typename... Ts> struct TypeIndex;
            template <typename T, typename... Ts> struct TypeIndex<T, T, Ts...> { static const uint32_t value = 0u; };
            template <typename T, typename U, typename... Ts> struct TypeIndex<T, U, Ts...> { static const uint32_t value = 1u + TypeIndex<T, Ts...>::value; };

            template <typename... Ts> struct TypeList {};

            // Note: every structure must appear once and extend one of the structures before it ('StructExtends')
            template <typename Prev, typename... Rest> struct ChainValid;
            template <typename... Prev> struct ChainValid<TypeList<Prev...>> { static const bool value = true; };
            template <typename... Prev, typename T, typename... Rest> struct ChainValid<TypeList<Prev...>, T, Rest...> {
                static const bool value = TypeCount<T, Prev...>::value == 0u && AnyOf<StructExtends<T, Prev>::value...>::value &&
                                          ChainValid<TypeList<Prev..., T>, Rest...>::value;
            };

            template <typename W, typename C>
            void setWrapNext(W &_wrap, C &_chain) {
                static_assert(StructExtends<typename C::HeadType, W>::value, "VKFW :: StructWrap :: setNext - the chain head can't extend this structure (see StructExtends)");
                _wrap.pNext = &_chain.head();
            }
        }; // Detail END

        // Note: typed pNext chain stored in one object (like vk::StructureChain): 'sType' and 'pNext' are set in the constructor,
        //       'get<T>()' is resolved at compile time and there are no allocations. The chain is checked at compile time:
        //       every structure must appear once and extend one of the structures before it.
        //
        //       StructChain<VkPhysicalDeviceFeatures2, VkPhysicalDeviceVulkan12Features> features;
        //       vkGetPhysicalDeviceFeatures2(phys_device, &features.head());
        //       if (features.get<VkPhysicalDeviceVulkan12Features>().timelineSemaphore) ...
        //
        //       The chain must outlive every Vulkan call that reads it through 'pNext'.
        template <typename Head, typename... Tail>
        struct StructChain {
            typedef Head HeadType;
            static const uint32_t SIZE = 1u + sizeof...(Tail);

            static_assert(Detail::ChainValid<Detail::TypeList<Head>, Tail...>::value,
                          "VKFW :: StructChain - every structure must appear once and extend an earlier structure of the chain (see StructExtends)");

            StructChain() : data{} { link(); }
            StructChain(const StructChain &_chain) : data{_chain.data} { link(); }
            StructChain& operator=(const StructChain &_chain) { data = _chain.data; link(); return *this; }

                  Head& head()       { return std::get<0>(data); }
            const Head& head() const { return std::get<0>(data); }

            template <typename T>
            T& get() {
                static_assert(Detail::TypeCount<T, Head, Tail...>::value == 1u, "VKFW :: StructChain :: get - the type is not in the chain");
                return std::get<Detail::TypeIndex<T, Head, Tail...>::value>(data);
            }
            template <typename T>
            const T& get() const {
                static_assert(Detail::TypeCount<T, Head, Tail...>::value == 1u, "VKFW :: StructChain :: get - the type is not in the chain");
                return std::get<Detail::TypeIndex<T, Head, Tail...>::value>(data);
            }

            // Restores the order of the template arguments, linking all structures (keeps their contents)
            void link() {
                VkBaseOutStructure *__ptrs[SIZE];
                fillPtrs_(__ptrs);
                const VkStructureType __types[SIZE] = { StructType<Head>::value, StructType<Tail>::value... };
                for (uint32_t i = 0u; i < SIZE; ++i) {
                    __ptrs[i]->sType = __types[i];
                    __ptrs[i]->pNext = i + 1u < SIZE ? __ptrs[i + 1u] : nullptr;
                }
            }

            // Removes 'T' from the pNext chain (Vulkan won't see it), its contents are kept
            template <typename T>
            void unlink() {
                static_assert(!std::is_same<T, Head>::value, "VKFW :: StructChain :: unlink - the head can't be unlinked");
                VkBaseOutStructure *__target = reinterpret_cast<VkBaseOutStructure*>(&get<T>());
                for (VkBaseOutStructure *p = reinterpret_cast<VkBaseOutStructure*>(&head()); p != nullptr; p = p->pNext)
                    if (p->pNext == __target) { p->pNext = __target->pNext; __target->pNext = nullptr; return; }
            }
            // Links an unlinked 'T' back, right after the head
            template <typename T>
            void relink() {
                if (isLinked<T>()) return;
                VkBaseOutStructure *__head   = reinterpret_cast<VkBaseOutStructure*>(&head());
                VkBaseOutStructure *__target = reinterpret_cast<VkBaseOutStructure*>(&get<T>());
                __target->pNext = __head->pNext;
                __head->pNext   = __target;
            }
            template <typename T>
            bool isLinked() const {
                const VkBaseOutStructure *__target = reinterpret_cast<const VkBaseOutStructure*>(&get<T>());
                for (const VkBaseOutStructure *p = reinterpret_cast<const VkBaseOutStructure*>(&head()); p != nullptr; p = p->pNext)
                    if (p == __target) return true;
                return false;
            }

        private:
            std::tuple<Head, Tail...> data;

            void fillPtrs_(VkBaseOutStructure **_ptrs) {
                VkBaseOutStructure *__ptrs[SIZE] = { reinterpret_cast<VkBaseOutStructure*>(&std::get<0>(data)),
                                                     reinterpret_cast<VkBaseOutStructure*>(&get<Tail>())... };
                for (uint32_t i = 0u; i < SIZE; ++i) _ptrs[i] = __ptrs[i];
            }
        }; // StructChain END
    }; // VKFW END
}; // Simple END

#endif