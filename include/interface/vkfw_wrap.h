#ifndef SVKFW_VKFW_WRAP_H
#define SVKFW_VKFW_WRAP_H

#include <fstream>

#include "vkfw_base.h"
#include "vkfw_enum.h"
#include "vkfw_func.h"


namespace Simple {
    namespace VKFW {
        typedef enum PipelineStateBits : uint32_t {
            VKFW_PIPELINE_STATE_VERTEX_INPUT_BIT   = 1u << 0,
            VKFW_PIPELINE_STATE_INPUT_ASSEMBLY_BIT = 1u << 1,
            VKFW_PIPELINE_STATE_TESSELLATION_BIT   = 1u << 2,
            VKFW_PIPELINE_STATE_VIEWPORT_BIT       = 1u << 3,
            VKFW_PIPELINE_STATE_RASTERIZATION_BIT  = 1u << 4,
            VKFW_PIPELINE_STATE_MULTISAMPLE_BIT    = 1u << 5,
            VKFW_PIPELINE_STATE_DEPTH_STENCIL_BIT  = 1u << 6,
            VKFW_PIPELINE_STATE_COLOR_BLEND_BIT    = 1u << 7,
            VKFW_PIPELINE_STATE_DYNAMIC_BIT        = 1u << 8,
        } PipelineStateBits; // PipelineStateBits END
        typedef uint32_t PipelineStateFlags;

        typedef enum PipelineType : uint32_t {
            VKFW_PIPELINE_TYPE_UNDEFINED   = 0u,
            VKFW_PIPELINE_TYPE_GRAPHICS    = 1u,
            VKFW_PIPELINE_TYPE_COMPUTE     = 2u,
            VKFW_PIPELINE_TYPE_RAY_TRACING = 3u,
        } PipelineType; // PipelineType END



//  == === ==== ============================================ ==== === ==  \\
                    Vulkan Function Wrappers (presets)
//  == === ==== ============================================ ==== === ==  \\

        // Note: thin wrappers of every Vulkan command are generated into 'vkfw_func.h' (see 'source/meta/vkfw_func_gen.py');
        //       only the helpers that do more than one Vulkan call (or build the create info) are written by hand here
        namespace Func {
            std::vector<VkCommandBuffer> AllocateCommandBuffers(VkDevice _device, VkCommandPool _command_pool,
                                                                VkCommandBufferLevel _level, uint32_t _buffer_count,
                                                                const void *_p_next = nullptr);

            // Creates 'semaphore_count' semaphores using the same 'create_info' config.
            // Note: Windows headers define 'CreateSemaphore' as a macro, so the preset has a plural name
            std::vector<VkSemaphore> CreateSemaphores(VkDevice _device, const VkSemaphoreCreateInfo *_create_info, uint32_t _semaphore_count = 1u, const VkAllocationCallbacks *_allocator = nullptr);

            // Creates 'fence_count' fences using the same 'create_info' config. Requires separate calls for signaled/not signaled fences.
            std::vector<VkFence> CreateFences(VkDevice _device, const VkFenceCreateInfo *_create_info, uint32_t _fence_count = 1u, const VkAllocationCallbacks *_allocator = nullptr);

            // Creates 'event_count' events using the same 'create_info' config. Requires separate calls for device-only events.
            // Note: Windows headers define 'CreateEvent' as a macro, so the preset has a plural name
            std::vector<VkEvent> CreateEvents(VkDevice _device, const VkEventCreateInfo *_create_info, uint32_t _event_count = 1u, const VkAllocationCallbacks *_allocator = nullptr);

            // Queries the capture replay handle size and returns the handles of 'group_count' shader groups
            std::vector<uint8_t> GetRayTracingCaptureReplayShaderGroupHandlesKHR(VkPhysicalDevice _phys_device, VkDevice _device, VkPipeline _pipeline,
                                                                                 uint32_t _first_group, uint32_t _group_count);
        }; // Func END



//  == === ==== ======================== ==== === ==  \\
                    Vulkan Utilities
//  == === ==== ======================== ==== === ==  \\

        namespace Util {
            using Simple::Util::StringVec;

        // Default VKFW debug message callback
            VKAPI_ATTR VkBool32 VKAPI_CALL vkfwDebugMessageCallback(
                        VkDebugUtilsMessageSeverityFlagBitsEXT _message_severity,
                        VkDebugUtilsMessageTypeFlagsEXT _message_type,
                        const VkDebugUtilsMessengerCallbackDataEXT* _callback_data,
                        void* _user_data);


//  == === ==== >   Extension/Layer checks for Instance/Device

            bool allLayersAvailable(const StringVec &_layers, const std::vector<VkLayerProperties> _layer_list);

            bool allExtAvailable(const StringVec &_extensions, const std::vector<VkExtensionProperties> &_ext_list);

            void checkLayers(const StringVec &_layers, bool *_flags, const std::vector<VkLayerProperties> &_layer_list);

            void checkExtensions(const StringVec &_extensions, bool *_flags, const std::vector<VkExtensionProperties> &_ext_list);


            bool allInstanceLayersAvailable(const StringVec &_layers);

            bool allInstanceExtAvailable(const StringVec &_extensions, const char *_layer = nullptr);

            bool allDeviceLayersAvailable(VkPhysicalDevice _phys_device, const StringVec &_layers);

            bool allDeviceExtAvailable(VkPhysicalDevice _phys_device, const StringVec &_extensions, const char *_layer = nullptr);


            void checkInstanceLayers(const StringVec &_layers, bool *_flags);

            void checkInstanceExtensions(const StringVec &_extensions, bool *_flags, const char *_layer = nullptr);

            void checkDeviceLayers(VkPhysicalDevice _phys_device, const StringVec &_layers, bool *_flags);

            void checkDeviceExtensions(VkPhysicalDevice _phys_device, const StringVec &_extensions, bool *_flags, const char *_layer = nullptr);


//  == === ==== >   Physical Device Selection Utilities

            uint32_t checkPhysicalDeviceMemoryProperties(VkPhysicalDevice _phys_device, uint32_t _req_type_properties,
                                                         VkMemoryPropertyFlags _req_memory_properties);

            // TODO: ADD STRUCT CHAIN HERE, LET TEST/RANK FUNCTIONS ACCESS STRUCTS BY POINTERS RETURNED BY THIS CLASS' METHODS.
            // This type contains all properties (retreivable via vulkan functions)
            // about one physical device - element of PhysicalDevice::phys_devices vector.
            // It contains references to phys_devices[i], features[i] and properties[i].
            struct PhysDeviceFullInfo {
                const VkPhysicalDevice &phys_device;
                const VkPhysicalDeviceFeatures &features;
                const VkPhysicalDeviceProperties &properties;

                PhysDeviceFullInfo(const VkPhysicalDevice &_phys_device,
                                   const VkPhysicalDeviceFeatures &_features,
                                   const VkPhysicalDeviceProperties &_properties);
                PhysDeviceFullInfo(const PhysDeviceFullInfo &_elem_view);


                vec4u apiVersion() const;

                std::string infoString() const;

            // Scenarios

                std::vector<VkExtensionProperties> getAllDeviceExtensions(const char *_layer = nullptr) const;

                bool allExtAvailable(const StringVec &_extensions) const;

                void checkExtensions(const StringVec &_extensions, bool *_flags) const;
            }; // PhysDeviceFullInfo END


            typedef bool (*PhysDeviceTestFunc) (const PhysDeviceFullInfo &_phys_device_info);
            typedef  int (*PhysDeviceRankFunc) (const PhysDeviceFullInfo &_phys_device_info);

            namespace SelectPhysDevice {
                bool DefaultGraphicsTest(const PhysDeviceFullInfo &_phys_device_info);
            }; // SelectPhysDevice END


            struct PhysDeviceManager {
                std::vector<VkPhysicalDevice> phys_devices;
                std::vector<VkPhysicalDeviceFeatures> features;
                std::vector<VkPhysicalDeviceProperties> properties;
                PhysDeviceTestFunc test_func = SelectPhysDevice::DefaultGraphicsTest;
                PhysDeviceRankFunc rank_func = nullptr;
                VkBool32 selection_by_rank = false;

                PhysDeviceManager(VkInstance _instance = VK_NULL_HANDLE);

                void clear();

                VkPhysicalDevice selectOne();

                void selectWithTest(PhysDeviceTestFunc _test_func = SelectPhysDevice::DefaultGraphicsTest);

                void selectWithRank(PhysDeviceRankFunc _rank_func = nullptr);

                const PhysDeviceFullInfo getElemInfo(uint32_t _index);

                void loadInfo(VkInstance _instance);


                // Returns the first physical device for which _testFunc({device, features, properties}) == true
                VkPhysicalDevice testPickOne(const PhysDeviceTestFunc _testFunc);

                // Returns all physical devices for which _testFunc({device, features, properties}) == true
                std::vector<VkPhysicalDevice> testPickAll(const PhysDeviceTestFunc _testFunc);

                // Ranks all physical devices and returns one with max _rankFunc({device, features, properties})
                VkPhysicalDevice rankPickOne(const PhysDeviceRankFunc _rankFunc);

                // Ranks all physical devices and returns one which passes the comparison to _threshold (default: >=)
                template <CompareOp COMP_OP = CompareOp::GE>
                VkPhysicalDevice rankPickOne(const PhysDeviceRankFunc _rankFunc, int _threshold) {
                    for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                        switch (COMP_OP) {
                            case CompareOp::NE: {
                                if (_rankFunc({phys_devices[i], features[i], properties[i]}) != _threshold)
                                    return phys_devices[i];
                                break;
                            }
                            case CompareOp::EQ: {
                                if (_rankFunc({phys_devices[i], features[i], properties[i]}) == _threshold)
                                    return phys_devices[i];
                                break;
                            }
                            case CompareOp::LT: {
                                if (_rankFunc({phys_devices[i], features[i], properties[i]}) <  _threshold)
                                    return phys_devices[i];
                                break;
                            }
                            case CompareOp::LE: {
                                if (_rankFunc({phys_devices[i], features[i], properties[i]}) <= _threshold)
                                    return phys_devices[i];
                                break;
                            }
                            case CompareOp::GE: {
                                if (_rankFunc({phys_devices[i], features[i], properties[i]}) >= _threshold)
                                    return phys_devices[i];
                                break;
                            }
                            case CompareOp::GT: {
                                if (_rankFunc({phys_devices[i], features[i], properties[i]}) >  _threshold)
                                    return phys_devices[i];
                                break;
                            }
                        }
                    }
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW::Util::PhysDeviceManager :: rankPickOne (2)", "Failed to find a suitable GPU"));
                    return VK_NULL_HANDLE; // TODO: Maybe return first GPU from the list
                }

                // Ranks all physical devices and returns all which pass the comparison to _threshold (default: >=)
                template <CompareOp COMP_OP = CompareOp::GE>
                std::vector<VkPhysicalDevice> rankPickAll(const PhysDeviceRankFunc _rankFunc, int _threshold) {
                    std::vector<VkPhysicalDevice> __res_devices;

                    for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                        switch (COMP_OP) {
                            case CompareOp::NE: {
                                if (_rankFunc({phys_devices[i], features[i], properties[i]}) != _threshold)
                                    __res_devices.push_back(phys_devices[i]);
                                break;
                            }
                            case CompareOp::EQ: {
                                if (_rankFunc({phys_devices[i], features[i], properties[i]}) == _threshold)
                                    __res_devices.push_back(phys_devices[i]);
                                break;
                            }
                            case CompareOp::LT: {
                                if (_rankFunc({phys_devices[i], features[i], properties[i]}) <  _threshold)
                                    __res_devices.push_back(phys_devices[i]);
                                break;
                            }
                            case CompareOp::LE: {
                                if (_rankFunc({phys_devices[i], features[i], properties[i]}) <= _threshold)
                                    __res_devices.push_back(phys_devices[i]);
                                break;
                            }
                            case CompareOp::GE: {
                                if (_rankFunc({phys_devices[i], features[i], properties[i]}) >= _threshold)
                                    __res_devices.push_back(phys_devices[i]);
                                break;
                            }
                            case CompareOp::GT: {
                                if (_rankFunc({phys_devices[i], features[i], properties[i]}) >  _threshold)
                                    __res_devices.push_back(phys_devices[i]);
                                break;
                            }
                        }
                    }

                    return __res_devices;
                }

                // Ranks all physical devices and returns all which pass the comparison to _threshold (default: >=)
                // Adds ranks of selected physical devices to _ranks.
                template <CompareOp COMP_OP = CompareOp::GE>
                std::vector<VkPhysicalDevice> rankPickAll(const PhysDeviceRankFunc _rankFunc, std::vector<int> &_ranks, int _threshold) {
                    std::vector<VkPhysicalDevice> __res_devices;

                    for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                        int __rank = _rankFunc({phys_devices[i], features[i], properties[i]});
                        switch (COMP_OP) {
                            case CompareOp::NE: {
                                if (__rank != _threshold) {
                                    _ranks.push_back(__rank);
                                    __res_devices.push_back(phys_devices[i]);
                                }
                                break;
                            }
                            case CompareOp::EQ: {
                                if (__rank == _threshold) {
                                    _ranks.push_back(__rank);
                                    __res_devices.push_back(phys_devices[i]);
                                }
                                break;
                            }
                            case CompareOp::LT: {
                                if (__rank <  _threshold) {
                                    _ranks.push_back(__rank);
                                    __res_devices.push_back(phys_devices[i]);
                                }
                                break;
                            }
                            case CompareOp::LE: {
                                if (__rank <= _threshold) {
                                    _ranks.push_back(__rank);
                                    __res_devices.push_back(phys_devices[i]);
                                }
                                break;
                            }
                            case CompareOp::GE: {
                                if (__rank >= _threshold) {
                                    _ranks.push_back(__rank);
                                    __res_devices.push_back(phys_devices[i]);
                                }
                                break;
                            }
                            case CompareOp::GT: {
                                if (__rank >  _threshold) {
                                    _ranks.push_back(__rank);
                                    __res_devices.push_back(phys_devices[i]);
                                }
                                break;
                            }
                        }
                    }

                    return __res_devices;
                }
            }; // PhysDeviceManager END


//  == === ==== >   Queue Family Selection Utilities

            struct QueueFamilyFullInfo {
                const VkQueueFamilyProperties &properties;

                QueueFamilyFullInfo(const VkQueueFamilyProperties &_props);

                bool supportsGraphics()        const;
                bool supportsCompute()         const;
                bool supportsTransfer()        const;
                bool supportsSparseBinding()   const;
                bool supportsProtectedMemory() const;
                bool supportsVideoDecode()     const;
                bool supportsVideoEncode()     const;
                bool supportsOpticalFlow()     const;

                uint32_t queuesNum()                const;
                uint32_t timestampValidRangeBits()  const;
                vec3u minImageTransferGranularity() const;

                std::string infoString() const;
            }; // QueueFamilyFullInfo END


            typedef bool (*QueueFamilyTestFunc) (const QueueFamilyFullInfo &_info, bool _surf_support);
            typedef  int (*QueueFamilyRankFunc) (const QueueFamilyFullInfo &_info, bool _surf_support);

            namespace SelectQueueFamily {
                bool testGraphics(const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testCompute (const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testTransfer(const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testSparseBinding(const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testProtectedMemory(const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testVideoDecode(const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testVideoEncode(const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testOpticalFlow(const QueueFamilyFullInfo &_info, bool _surf_support);

                bool testSurfAndGraphics(const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testSurfAndCompute (const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testSurfAndTransfer(const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testSurfAndSparseBinding(const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testSurfAndProtectedMemory(const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testSurfAndVideoDecode(const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testSurfAndVideoEncode(const QueueFamilyFullInfo &_info, bool _surf_support);
                bool testSurfAndOpticalFlow(const QueueFamilyFullInfo &_info, bool _surf_support);


            // TODO: practice, then redo these rank functions
            // For now, result >0 means score; if <=0, then queue family doesn't have required feature.

                int rankGraphics(const QueueFamilyFullInfo &_info, bool _surf_support);
                int rankCompute (const QueueFamilyFullInfo &_info, bool _surf_support);
                int rankTransfer(const QueueFamilyFullInfo &_info, bool _surf_support);
                int rankSparseBinding  (const QueueFamilyFullInfo &_info, bool _surf_support);
                int rankProtectedMemory(const QueueFamilyFullInfo &_info, bool _surf_support);
                int rankVideoDecode(const QueueFamilyFullInfo &_info, bool _surf_support);
                int rankVideoEncode(const QueueFamilyFullInfo &_info, bool _surf_support);
                int rankOpticalFlow(const QueueFamilyFullInfo &_info, bool _surf_support);
            }; // SelectQueueFamily END


            struct QueueFamilyManager {
                std::vector<VkQueueFamilyProperties> properties;
                std::vector<bool> surface_support;
                QueueFamilyTestFunc test_func = nullptr;
                QueueFamilyRankFunc rank_func = nullptr;
                VkBool32 selection_by_rank = false;


                uint32_t selectOne();

                void selectWithTest(QueueFamilyTestFunc _test_func);

                void selectWithRank(QueueFamilyRankFunc _rank_func);

                QueueFamilyManager(VkInstance _instance = VK_NULL_HANDLE, VkPhysicalDevice _phys_device = VK_NULL_HANDLE);

                void clear();

                const QueueFamilyFullInfo getElemInfo(uint32_t _index) const;


                void loadInfo(VkInstance _instance, VkPhysicalDevice _phys_device);


                uint32_t testPickOne(QueueFamilyTestFunc _test_func);

                std::vector<uint32_t> testPickAll(QueueFamilyTestFunc _test_func);

                uint32_t rankPickOne(QueueFamilyRankFunc _rank_func);

                template <CompareOp COMP_OP = CompareOp::GE>
                std::vector<uint32_t> rankPickAll(QueueFamilyRankFunc _rank_func, int _threshold) {
                    std::vector<uint32_t> __indices;

                    for (uint32_t i = 0u; i < properties.size(); ++i) {
                        int __rank = _rank_func(properties[i], surface_support[i]);
                        switch (COMP_OP) {
                            case CompareOp::NE: {
                                if (__rank != _threshold)
                                    __indices.push_back(i);
                                break;
                            }
                            case CompareOp::EQ: {
                                if (__rank == _threshold)
                                    __indices.push_back(i);
                                break;
                            }
                            case CompareOp::LT: {
                                if (__rank < _threshold)
                                    __indices.push_back(i);
                                break;
                            }
                            case CompareOp::LE: {
                                if (__rank <= _threshold)
                                    __indices.push_back(i);
                                break;
                            }
                            case CompareOp::GE: {
                                if (__rank >= _threshold)
                                    __indices.push_back(i);
                                break;
                            }
                            case CompareOp::GT: {
                                if (__rank > _threshold)
                                    __indices.push_back(i);
                                break;
                            }
                        }
                    }

                    if (__indices.empty())
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::QueueFamilyManager :: rankPickAll", "could not find queue family\n"));
                    return __indices;
                }
            }; // QueueFamilyManager END


//  == === ==== >   Surface Format Selection Utilities

            typedef bool (*SurfaceFormatTestFunc) (const VkSurfaceFormatKHR &_format);
            typedef int  (*SurfaceFormatRankFunc) (const VkSurfaceFormatKHR &_format);

            namespace SelectSurfaceFormat {
                bool DefaultFormatTest(const VkSurfaceFormatKHR &_format);
            }; // SelectSurfaceFormat END
        }; // Util END



//  == === ==== ============================== ==== === ==  \\
                    Vulkan Struct Wrappers
//  == === ==== ============================== ==== === ==  \\

        namespace Struct {
        // Note: 'wrap' is private. Fill the structure through 'edit()' and pass it to Vulkan through 'get()'.
        //       'get()' calls 'updateWrap()' first, so the counts and pointers that the wrapper manages are always current;
        //       writing such a field through 'edit()' has no effect, use the wrapper's members instead.
        //       'peek()' reads the structure as it is, without updating it.
        // Note: 'setNext' puts a StructChain into 'pNext' (checked at compile time); the chain must outlive the wrap's use.
        //       'edit().pNext = nullptr' removes it.
        #define VKFW_STRUCT_WRAP_ACCESS(T_)                                   \
            public:                                                           \
                typedef T_ WrapType;                                          \
                const WrapType &get()        { updateWrap(); return wrap; }   \
                const WrapType &peek() const { return wrap; }                 \
                      WrapType &edit()       { return wrap; }                 \
                template <typename C_>                                        \
                void setNext(C_ &_chain) { Detail::setWrapNext(wrap, _chain); }

            // Template interface
            template <typename T> struct StructWrap {
            private:
                T wrap;
                VKFW_STRUCT_WRAP_ACCESS(T)
                void clear() {}
                void updateWrap() {}
            };


//  == === ==== >   Vulkan: Simple Structures

        // Note: wrappers for structures that:
        //       1) Do  not have a 'VK_STRUCTURE_TYPE_*' tag
        //       2) Are not used as a config for a Vulkan API function

            template <>
            struct StructWrap<VkViewport> {
            private:
                VkViewport wrap{ 0.f, 0.f, 0.f, 0.f, 0.f, 1.f };
                VKFW_STRUCT_WRAP_ACCESS(VkViewport)

                // Note: VkViewport fields order - x, y, width, height, minDepth, maxDepth
                StructWrap(const VkExtent2D &_extent, float _x = 0.f, float _y = 0.f, float _depthMin = 0.f, float _depthMax = 1.f);
                StructWrap(const vec2f &_extent = 0.f, const vec2f &_offset = 0.f, const vec2f &_depth = { 0.f, 1.f });
                StructWrap(const StructWrap &_viewport);

                void clear();

                void updateWrap();

            // Scenarios

                void setOffset(const VkOffset2D &_offset);
                void setExtent(const VkExtent2D &_extent);

                void setOffset(const vec2 &_offset);
                void setExtent(const vec2 &_extent);
                void setDepth (const vec2 &_depth);

                vec2 offset() const;
                vec2 extent() const;
                vec2  depth() const;
            }; // StructWrap<VkViewport> END

            template <>
            struct StructWrap<VkRect2D> {
            private:
                VkRect2D wrap{ {0, 0}, {0u, 0u} };
                VKFW_STRUCT_WRAP_ACCESS(VkRect2D)

                StructWrap();
                StructWrap(const StructWrap &_sct);
                // Note: VkRect2D fields order - offset, extent
                StructWrap(const VkExtent2D &_extent, const VkOffset2D &_offset = {0, 0});
                StructWrap(const vec2u &_extent, const vec2i &_offset = 0);

                void clear();

                void updateWrap();

            // Scenarios

                void setOffset(const VkOffset2D &_offset);
                void setExtent(const VkExtent2D &_extent);

                void setOffset(const vec2i &_offset);
                void setExtent(const vec2u &_extent);

                vec2i offset() const;
                vec2u extent() const;
            }; // StructWrap<VkRect2D> END

            template <>
            struct StructWrap<VkClearValue> {
            private:
                VkClearValue wrap{};
                VKFW_STRUCT_WRAP_ACCESS(VkClearValue)

                StructWrap();
                StructWrap(float    _v0, float    _v1, float    _v2, float    _v3);
                StructWrap(int32_t  _v0, int32_t  _v1, int32_t  _v2, int32_t  _v3);
                StructWrap(uint32_t _v0, uint32_t _v1, uint32_t _v2, uint32_t _v3);
                StructWrap(float _depth, uint32_t _stencil);

                void clear();

                void updateWrap();

            // Scenarios

                void fromColorVec4f(const vec4f &_col);
                void fromColorVec4i(const vec4i &_col);
                void fromColorVec4u(const vec4u &_col);
                void fromDepthStencil(float _depth, uint32_t _stencil);

            // Static

                static VkClearValue ColorVec4f(const vec4f &_col);
                static VkClearValue ColorVec4i(const vec4i &_col);
                static VkClearValue ColorVec4u(const vec4u &_col);
                static VkClearValue DepthStencil(float _depth, uint32_t _stencil);
            }; // StructWrap<VkClearValue> END

            template <>
            struct StructWrap<VkAttachmentDescription> {
            private:
                VkAttachmentDescription wrap{};
                VKFW_STRUCT_WRAP_ACCESS(VkAttachmentDescription)

                StructWrap();
                StructWrap(const StructWrap &_sct);
                StructWrap(const VkAttachmentDescription &_description);

                void clear();

                void updateWrap();

            // Scenarios

                void setColorOps(VkAttachmentLoadOp _loadOp, VkAttachmentStoreOp _storeOp);
                void setStencilOps(VkAttachmentLoadOp _loadOp, VkAttachmentStoreOp _storeOp);
                void setLayouts(VkImageLayout _initLayout, VkImageLayout _finalLayout);
                void setFormatAndSize(VkFormat _format, VkSampleCountFlagBits _samples);
            }; // StructWrap<VkAttachmentDescription> END

            template <>
            struct StructWrap<VkSubpassDescription> {
            private:
                VkSubpassDescription wrap{};
                VKFW_STRUCT_WRAP_ACCESS(VkSubpassDescription)

                VkAttachmentReference      depth_stencil_attachment;
                std::vector<VkAttachmentReference> color_attachments,
                                                   input_attachments,
                                                 resolve_attachments;
                std::vector<uint32_t>           preserve_attachments;

                StructWrap();

                void clear();

                // Note: pointers to members are set here (and not in 'clear()'), because the structure can be copied/moved (e.g. by std::vector)
                void updateWrap();

            // Scenarios

                void setDepthStencilAttachment(uint32_t _attachment_id, VkImageLayout _layout);

                ContextIndex addColorAttachment(uint32_t _attachment_id, VkImageLayout _layout);

                ContextIndex addInputAttachment(uint32_t _attachment_id, VkImageLayout _layout);

                ContextIndex addResolveAttachment(uint32_t _attachment_id, VkImageLayout _layout);
            }; // StructWrap<VkSubpassDescription> END

            template <>
            struct StructWrap<VkSubpassDependency> {
            private:
                VkSubpassDependency wrap{};
                VKFW_STRUCT_WRAP_ACCESS(VkSubpassDependency)

                StructWrap();
                StructWrap(const StructWrap &_sct);
                StructWrap(const VkSubpassDependency &_subpass_dependency);

                void clear();

                void updateWrap();

            // Scenarios

                void setSubpasses(uint32_t _src_subpass, uint32_t _dst_subpass);

                void setSrcMasks(VkPipelineStageFlags _src_stage_mask, VkAccessFlags _src_acc_mask);

                void setDstMasks(VkPipelineStageFlags _dst_stage_mask, VkAccessFlags _dst_acc_mask);
            }; // StructWrap<VkSubpassDependency> END

            template <>
            struct StructWrap<VkSpecializationInfo> {
            private:
                VkSpecializationInfo wrap{};
                VKFW_STRUCT_WRAP_ACCESS(VkSpecializationInfo)

                std::vector<VkSpecializationMapEntry> map_entries;
                std::vector<unsigned char> data;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setData(const void* _data, size_t _data_size);
            }; // StructWrap<VkSpecializationInfo> END

            template <>
            struct StructWrap<VkDescriptorSetLayoutBinding> {
            private:
                VkDescriptorSetLayoutBinding wrap{};
                VKFW_STRUCT_WRAP_ACCESS(VkDescriptorSetLayoutBinding)

                std::vector<VkSampler> immutable_samplers;

                StructWrap();
                // StructWrap(const Builder &_builder) = default;

                void clear();

                void updateWrap();

            // Scenarios

                void setWholeStruct(uint32_t _binding, VkDescriptorType _descriptor_type, uint32_t _descriptor_count,
                                    VkShaderStageFlags _stage_flags, const std::vector<VkSampler> &_immutable_samplers = {});
            }; // StructWrap<VkDescriptorSetLayoutBinding> END


//  == === ==== >   Vulkan: CreateInfo (General)

            template <>
            struct StructWrap<VkApplicationInfo> {
            private:
                VkApplicationInfo wrap{VK_STRUCTURE_TYPE_APPLICATION_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkApplicationInfo)

                std::string app_name, engine_name;
                vec3u app_version, engine_version;
                vec4u api_version;

                StructWrap();

                void clear();

                void updateWrap();

                // Scenarios

                void setAvailableVersionAPI();

                void setWholeStruct(const std::string &   _app_name = "SimpleVKFW_MainApp", const vec3u &   _app_version = {0u, 0u, 1u},
                                    const std::string &_engine_name = "SimpleVKFW",         const vec3u &_engine_version = {0u, 1u, 0u}, const vec4u &_api_version = {0u});
            }; // StructWrap<VkApplicationInfo> END

            template <>
            struct StructWrap<VkInstanceCreateInfo> {
            private:
                VkInstanceCreateInfo wrap{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkInstanceCreateInfo)

                Util::StringVec layers, extensions;

                StructWrap<VkApplicationInfo> app_info_wrap;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void addLayers(const Util::StringVec &_layers);

                void addExtensions(const Util::StringVec &_extensions);

                void addValidationLayer();

                void addExtGLFW();

                void addExtDebugUtils();
            }; // StructWrap<VkInstanceCreateInfo> END

            template<>
            struct StructWrap<VkDebugUtilsMessengerCreateInfoEXT> {
            private:
                VkDebugUtilsMessengerCreateInfoEXT wrap{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
                VKFW_STRUCT_WRAP_ACCESS(VkDebugUtilsMessengerCreateInfoEXT)

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                // If 'user_cb' is nullptr, the default VKFW callback is used
                void setWholeStruct(VkDebugUtilsMessageSeverityFlagsEXT _msg_severity,
                                    VkDebugUtilsMessageTypeFlagsEXT _msg_type,
                                    PFN_vkDebugUtilsMessengerCallbackEXT _user_cb = nullptr,
                                    void* _user_data = nullptr);
            }; // StructWrap<VkDebugUtilsMessengerCreateInfoEXT> END

            template <>
            struct StructWrap<VkDeviceQueueCreateInfo> {
            private:
                VkDeviceQueueCreateInfo wrap{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkDeviceQueueCreateInfo)

                VkfwQueueFamily queue_family = UINT32_MAX;
                std::vector<float> priorities;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setQueueCount(uint32_t _queue_count, const std::vector<float> &_priorities = {1.0f});
            }; // StructWrap<VkDeviceQueueCreateInfo> END

            template <>
            struct StructWrap<VkDeviceCreateInfo> {
            private:
                VkDeviceCreateInfo wrap{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkDeviceCreateInfo)

                VkPhysicalDeviceFeatures req_features{};
                std::vector<VkDeviceQueueCreateInfo> queue_info;
                Util::StringVec layers, extensions;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void addLayers(const Util::StringVec &_layers);

                void addExtensions(const Util::StringVec &_extensions);

                void addValidationLayer(VkPhysicalDevice _phys_device = VK_NULL_HANDLE);
            }; // StructWrap<VkDeviceCreateInfo> END

            template <>
            struct StructWrap<VkSwapchainCreateInfoKHR> {
            private:
                VkSwapchainCreateInfoKHR wrap{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
                VKFW_STRUCT_WRAP_ACCESS(VkSwapchainCreateInfoKHR)

                VkSurfaceCapabilitiesKHR capabilities;
                std::vector<uint32_t>  queue_families;

                StructWrap();
                // StructWrap(const StructWrap &_builder) = default;

                void clear();

                // Note: concurrent sharing mode requires at least 2 unique queue families
                void updateWrap();

            // Scenarios

                void setSurfaceCapabilities(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface);

                void setCurrentExtent();

                // Returns input bits after applying capabilities.supportedUsageFlags as a mask.
                // If not equal to input, then some usage flags are not supported
                // and the parameter is not set.
                VkImageUsageFlags checkSetImageUsage(VkImageUsageFlags _usage);

                void checkSetImageCount(uint32_t _count);

                bool checkSetExtent(vec2u _extent);

                void chooseCurrentExtent(const Window &_window);


                void pickFormat(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface, const Util::SurfaceFormatTestFunc _test_func);

                void pickFormat(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface, const Util::SurfaceFormatRankFunc _rank_func);

                bool checkSetPresentationMode(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface, VkPresentModeKHR _pres_mode);
            }; // StructWrap<VkSwapchainCreateInfoKHR> END

            template <>
            struct StructWrap<VkImageCreateInfo> {
            private:
                VkImageCreateInfo wrap{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkImageCreateInfo)

                std::vector<uint32_t> queue_families;

                StructWrap();
                // StructWrap(const StructWrap &_builder) = default;

                void clear();

                void updateWrap();

            // Scenarios

                // Note: '_extent.depth' > 1 makes a 3D image, otherwise a 2D one (set 'imageType' through 'edit' for 1D)
                void setWholeStruct(const std::vector<uint32_t> &_queue_families, VkSharingMode _sharing_mode, VkImageUsageFlags _usage_flags,
                                    VkFormat _format, VkExtent3D _extent, uint32_t _mip_levels = 1u, uint32_t _array_layers = 1u,
                                    VkSampleCountFlagBits _samples = VK_SAMPLE_COUNT_1_BIT, VkImageTiling _tiling = VK_IMAGE_TILING_OPTIMAL,
                                    VkImageCreateFlags _flags = 0u);
            }; // StructWrap<VkImageCreateInfo> END

            template <>
            struct StructWrap<VkImageViewCreateInfo> {
            private:
                VkImageViewCreateInfo wrap{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkImageViewCreateInfo)

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setRange(VkImageAspectFlags _aspect_mask, uint32_t _base_mip_level,
                              uint32_t _level_count, uint32_t _base_array_layer, uint32_t _array_level_count);

                void setComponents(VkComponentSwizzle _r = VK_COMPONENT_SWIZZLE_IDENTITY, VkComponentSwizzle _g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                   VkComponentSwizzle _b = VK_COMPONENT_SWIZZLE_IDENTITY, VkComponentSwizzle _a = VK_COMPONENT_SWIZZLE_IDENTITY);
            }; // StructWrap<VkImageViewCreateInfo> END

            template <>
            struct StructWrap<VkBufferCreateInfo> {
            private:
                VkBufferCreateInfo wrap{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkBufferCreateInfo)

                std::vector<VkfwQueueFamily> queue_families;

                StructWrap();
                // StructWrap(const StructWrap &_builder) = default;

                void clear();

                void updateWrap();

            // Scenarios

                void setWholeStruct(const std::vector<VkfwQueueFamily> &_queue_families, VkSharingMode _sharing_mode,
                                    VkBufferUsageFlags _usage_flags, VkDeviceSize _size_bytes);
            }; // StructWrap<VkBufferCreateInfo> END

            template <>
            struct StructWrap<VkBufferViewCreateInfo> {
            private:
                VkBufferViewCreateInfo wrap{VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkBufferViewCreateInfo)

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setWholeStruct(VkBuffer _buffer, VkFormat _format, VkDeviceSize _offset, VkDeviceSize _range);
            }; // StructWrap<VkBufferViewCreateInfo> END

            template <>
            struct StructWrap<VkMemoryAllocateInfo> {
            private:
                VkMemoryAllocateInfo wrap{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkMemoryAllocateInfo)

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setForBuffer(VkPhysicalDevice _phys_device, VkDevice _device, VkBuffer _buffer,
                                  VkMemoryPropertyFlags _req_memory_properties);

                void setForImage(VkPhysicalDevice _phys_device, VkDevice _device, VkImage _image,
                                  VkMemoryPropertyFlags _req_memory_properties);
            }; // StructWrap<VkMemoryAllocateInfo> END

            template <>
            struct StructWrap<VkShaderModuleCreateInfo> {
            private:
                VkShaderModuleCreateInfo wrap{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkShaderModuleCreateInfo)
                std::vector<char> buffer;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void fromFile(const char *_fpath);

                void fromString(const char *_sh_str, size_t _length = 0u);

                void fromString(const std::string &_str_buffer);
            }; // StructWrap<VkShaderModuleCreateInfo> END

            template <>
            struct StructWrap<VkRenderPassCreateInfo> {
            private:
                VkRenderPassCreateInfo wrap{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkRenderPassCreateInfo)

                std::vector<StructWrap<VkAttachmentDescription>> attachments_wrap;
                std::vector<StructWrap<VkSubpassDescription>>      subpasses_wrap;
                std::vector<StructWrap<VkSubpassDependency>>    dependencies_wrap;

                std::vector<VkAttachmentDescription> attachments;
                std::vector<VkSubpassDescription>      subpasses;
                std::vector<VkSubpassDependency>    dependencies;

                StructWrap();
                // StructWrap(const StructWrap &_create_info) = default;

                void clear();

                void updateWrap();

            // Scenarios

                ContextIndex addAttachment();
                ContextIndex addSubpassDescription();
                ContextIndex addSubpassDependency();

                ContextEntity<StructWrap<VkAttachmentDescription>> shortcutAttachment        (ContextIndex _sct_ci);
                ContextEntity<StructWrap<   VkSubpassDescription>> shortcutSubpassDescription(ContextIndex _sct_ci);
                ContextEntity<StructWrap<    VkSubpassDependency>> shortcutSubpassDependency (ContextIndex _sct_ci);
            }; // StructWrap<VkRenderPassCreateInfo> END

            template <>
            struct StructWrap<VkFramebufferCreateInfo> {
            private:
                VkFramebufferCreateInfo wrap{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkFramebufferCreateInfo)

                std::vector<VkImageView> attachments;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setBufferSize(VkExtent2D _buffer_size);

                void setBufferSize(vec2u _buffer_size);

            // Scenarios

                void setBufferSizeAndLayers(VkExtent2D _buffer_size, uint32_t _layers);

                void setBufferSizeAndLayers(vec3u _buf_size_layers);
            }; // StructWrap<VkFramebufferCreateInfo> END

            template <>
            struct StructWrap<VkSemaphoreCreateInfo> {
            private:
                VkSemaphoreCreateInfo wrap{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkSemaphoreCreateInfo)

                StructWrap();

                void clear();

                void updateWrap();
            }; // StructWrap<VkSemaphoreCreateInfo>

            template <>
            struct StructWrap<VkFenceCreateInfo> {
            private:
                VkFenceCreateInfo wrap{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkFenceCreateInfo)

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setWholeStruct(bool _is_signaled);
            }; // StructWrap<VkFenceCreateInfo>

            template <>
            struct StructWrap<VkEventCreateInfo> {
            private:
                VkEventCreateInfo wrap{VK_STRUCTURE_TYPE_EVENT_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkEventCreateInfo)

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setWholeStruct(bool _is_device_only);
            }; // StructWrap<VkEventCreateInfo>


//  == === ==== >   Vulkan: CreateInfo (Descriptor-related)

            template <>
            struct StructWrap<VkWriteDescriptorSet> {
            private:
                VkWriteDescriptorSet wrap{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                VKFW_STRUCT_WRAP_ACCESS(VkWriteDescriptorSet)

                std::vector<VkDescriptorBufferInfo> buffer_infos;
                std::vector<VkDescriptorImageInfo >  image_infos;
                std::vector<VkBufferView>        texel_buf_views;

                // If 0, 'descriptorCount' is the size of the info array that 'descriptorType' uses
                uint32_t descriptor_count = 0u;

                StructWrap();

                void clear();

                // Note: Vulkan reads only one info array, chosen by 'descriptorType', and ignores the other two.
                //       Types that take their data through 'pNext' (e.g. inline uniform blocks, acceleration structures)
                //       use no info array, so 'descriptor_count' must be set for them.
                void updateWrap();

            // Scenarios

                // If 'descriptor_count' is 0, 'descriptorCount' is the size of the info array that 'descriptorType' uses
                void setScalars(VkDescriptorSet _dst_descr_set, VkDescriptorType _descr_type, uint32_t _dst_binding,
                                uint32_t _descriptor_count = 0u, uint32_t _dst_arr_elem = 0u);

                void addBufferInfo(VkBuffer _buffer, VkDeviceSize _offset = 0ul, VkDeviceSize _range = VK_WHOLE_SIZE);

                void addImageInfo(VkSampler _sampler, VkImageView _image_view, VkImageLayout _image_layout);

                void addTexelBufferView(VkBufferView _buffer_view);
            }; // StructWrap<VkWriteDescriptorSet> END

            template <>
            struct StructWrap<VkCopyDescriptorSet> {
            private:
                VkCopyDescriptorSet wrap{VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET};
                VKFW_STRUCT_WRAP_ACCESS(VkCopyDescriptorSet)

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setSrc(VkDescriptorSet _src_set, uint32_t _src_binding, uint32_t _src_arr_elem);

                void setDst(VkDescriptorSet _dst_set, uint32_t _dst_binding, uint32_t _dst_arr_elem);
            }; // StructWrap<VkCopyDescriptorSet> END

            template <>
            struct StructWrap<VkDescriptorSetLayoutCreateInfo> {
            private:
                VkDescriptorSetLayoutCreateInfo wrap{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkDescriptorSetLayoutCreateInfo)

                std::vector<Struct::StructWrap<VkDescriptorSetLayoutBinding>> bindings_wrap;

                std::vector<VkDescriptorSetLayoutBinding> bindings;

                StructWrap();
                // StructWrap(const Builder &_builder) = default;

                void clear();

                ContextIndex addBinding();
                ContextEntity<Struct::StructWrap<VkDescriptorSetLayoutBinding>> shortcutBinding(ContextIndex _sct_ci);

                void updateWrap();
            }; // StructWrap<VkDescriptorSetLayoutCreateInfo> END

            template <>
            struct StructWrap<VkDescriptorPoolCreateInfo> {
            private:
                VkDescriptorPoolCreateInfo wrap{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkDescriptorPoolCreateInfo)

                std::vector<VkDescriptorPoolSize> pool_sizes;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void addPoolSize(VkDescriptorType _descr_type, uint32_t _descr_count);

                void setWholeStruct(uint32_t _max_sets, const std::vector<VkDescriptorPoolSize> &_pool_sizes = {});
            }; // StructWrap<VkDescriptorPoolCreateInfo> END

            template <>
            struct StructWrap<VkDescriptorSetAllocateInfo> {
            private:
                VkDescriptorSetAllocateInfo wrap{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkDescriptorSetAllocateInfo)

                std::vector<VkDescriptorSetLayout> set_layouts;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setWholeStruct(VkDescriptorPool _descr_pool, const std::vector<VkDescriptorSetLayout> &_set_layouts);
            }; // StructWrap<VkDescriptorSetAllocateInfo> END


//  == === ==== >   Vulkan: CreateInfo (Pipeline-related)

            template <>
            struct StructWrap<VkPipelineLayoutCreateInfo> {
            private:
                VkPipelineLayoutCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineLayoutCreateInfo)

                std::vector<VkPushConstantRange> constants;
                std::vector<VkDescriptorSetLayout> layouts;

                StructWrap();

                void clear();

                void updateWrap();
            }; // StructWrap<VkPipelineLayoutCreateInfo> END

            template <>
            struct StructWrap<VkPipelineCacheCreateInfo> {
            private:
                VkPipelineCacheCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineCacheCreateInfo)

                std::vector<unsigned char> cache_data;

                StructWrap();

                void clear();


                void updateWrap();

            // Scenarios

                // Get data from previously created cache object
                void getCacheData(VkDevice _device, VkPipelineCache _pipeline_cache);
            }; // StructWrap<VkPipelineCacheCreateInfo> END

            template <>
            struct StructWrap<VkPipelineLibraryCreateInfoKHR> {
            private:
                VkPipelineLibraryCreateInfoKHR wrap{VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineLibraryCreateInfoKHR)

                std::vector<VkPipeline> libraries;

                StructWrap();

                void clear();

                void updateWrap();
            }; // StructWrap<VkPipelineLibraryCreateInfoKHR> END

            template <>
            struct StructWrap<VkRayTracingShaderGroupCreateInfoKHR> {
            private:
                VkRayTracingShaderGroupCreateInfoKHR wrap{VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
                VKFW_STRUCT_WRAP_ACCESS(VkRayTracingShaderGroupCreateInfoKHR)

                // Note: data is owned by vector, so the structure can be safely copied (previously raw 'new[]' pointer was double-deleted on copies)
                std::vector<unsigned char> capture_replay_handle;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void getData(VkPhysicalDevice _phys_device, VkDevice _device, VkPipeline _pipeline, uint32_t _first_group, uint32_t _group_count);
            }; // StructWrap<VkRayTracingShaderGroupCreateInfoKHR> END

            template <>
            struct StructWrap<VkRayTracingPipelineInterfaceCreateInfoKHR> {
            private:
                VkRayTracingPipelineInterfaceCreateInfoKHR wrap{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR};
                VKFW_STRUCT_WRAP_ACCESS(VkRayTracingPipelineInterfaceCreateInfoKHR)
                // Note: from Gemini, minimal supported 'maxPipelineRayHitAttributeSize' value is 32 bytes
                StructWrap();

                void clear();

                void updateWrap();
            }; // StructWrap<VkRayTracingPipelineInterfaceCreateInfoKHR> END


//  == === ==== >   Vulkan: CreateInfo (Pipeline States)

            template <>
            struct StructWrap<VkPipelineShaderStageCreateInfo> {
            private:
                VkPipelineShaderStageCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineShaderStageCreateInfo)

                StructWrap<VkSpecializationInfo> spec_info_wrap;

                std::string name;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setShaderInfo(VkShaderModule _shader_module, VkShaderStageFlagBits _stage, const std::string &_name);

                void setShaderSpecInfo(const void* _data, size_t _data_size, const std::vector<VkSpecializationMapEntry> &_map_entries);
            }; // StructWrap<VkPipelineShaderStageCreateInfo> END

            template <>
            struct StructWrap<VkPipelineDynamicStateCreateInfo> {
            private:
                VkPipelineDynamicStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineDynamicStateCreateInfo)

                std::vector<VkDynamicState> dynamic_states;

                StructWrap();
                // StructWrap(const StructBuilder &_builder) : dynamic_states{_builder.dynamic_states} {}

                void clear();

                void updateWrap();

            // Scenarios

                // returns false if state is already included in "dynamic_states", true if successfully added
                bool addDynamicState(VkDynamicState _state);

                // returns false if state is not in "dynamic_states", true if successfully removed entries
                bool delDynamicState(VkDynamicState _state);
            }; // StructBuilder<VkPipelineDynamicStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineVertexInputStateCreateInfo> {
            private:
                VkPipelineVertexInputStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineVertexInputStateCreateInfo)

                std::vector<VkVertexInputBindingDescription> bindings;
                std::vector<VkVertexInputAttributeDescription> attributes;

                StructWrap();

                void clear();

                void updateWrap();
            }; // StructWrap<VkPipelineVertexInputStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineInputAssemblyStateCreateInfo> {
            private:
                VkPipelineInputAssemblyStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineInputAssemblyStateCreateInfo)

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setWholeStruct(VkPrimitiveTopology _topology, VkBool32 _primitive_restart);
            }; // StructWrap<VkPipelineInputAssemblyStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineViewportStateCreateInfo> {
            private:
                VkPipelineViewportStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineViewportStateCreateInfo)

                std::vector<StructWrap<VkViewport>> viewports_wrap;
                std::vector<StructWrap<VkRect2D>>    scissors_wrap;

                std::vector<VkViewport> viewports;
                std::vector<VkRect2D>    scissors;
                uint32_t viewports_dynamic_count = 0u, scissors_dynamic_count = 0u;
                // counts can be set with nullptr pointers - dynamic mode

                StructWrap();
                // StructWrap(const StructBuilder &_builder) : viewports{_builder.viewports}, scissors{_builder.scissors} {}

                void clear();

                void updateWrap();

            // Scenarios

                void setViewportsDynamic(uint32_t _viewports_count);

                void setScissorsDynamic(uint32_t _scissors_count);

                void setFromSwapchainExtent(VkExtent2D _extent);
            }; // StructWrap<VkPipelineViewportStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineRasterizationStateCreateInfo> {
            private:
                VkPipelineRasterizationStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineRasterizationStateCreateInfo)

                StructWrap();
                // StructWrap(const StructWrap &_sct) = default;

                void clear();

                void updateWrap();

            // Scenarios

                void setDepthBias(bool _enable, float _const_factor = 0.f, float _clamp = 0.f, float _slope_factor = 0.f);
            }; // StructWrap<VkPipelineRasterizationStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineMultisampleStateCreateInfo> {
            private:
                VkPipelineMultisampleStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineMultisampleStateCreateInfo)

                VkSampleMask sample_mask[2] = {VkSampleMask(-1), VkSampleMask(-1)};

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                // Assuming 'VkSampleMask' is 'uint32_t', use 'mask' when 'rasterizationSamples' <= VK_SAMPLE_COUNT_32_BIT, and use 'mask2' when 'rasterizationSamples' == VK_SAMPLE_COUNT_64_BIT.
                void setSampleMask(VkSampleMask _mask = VkSampleMask(-1), VkSampleMask _mask2 = VkSampleMask(-1));
            }; // StructWrap<VkPipelineMultisampleStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineDepthStencilStateCreateInfo> {
            private:
                VkPipelineDepthStencilStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineDepthStencilStateCreateInfo)

                StructWrap();

                void clear();

                void updateWrap();
            }; // StructWrap<VkPipelineDepthStencilStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineColorBlendAttachmentState> {
            private:
                VkPipelineColorBlendAttachmentState wrap{};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineColorBlendAttachmentState)

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setColorBlend(VkBlendOp _op, VkBlendFactor _src_factor, VkBlendFactor _dst_factor);

                void setAlphaBlend(VkBlendOp _op, VkBlendFactor _src_factor, VkBlendFactor _dst_factor);
            }; // StructWrap<VkPipelineColorBlendAttachmentState> END

            template <>
            struct StructWrap<VkPipelineColorBlendStateCreateInfo> {
            private:
                VkPipelineColorBlendStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineColorBlendStateCreateInfo)

                std::vector<StructWrap<VkPipelineColorBlendAttachmentState>> attachments_wrap;

                std::vector<VkPipelineColorBlendAttachmentState> attachments;

                StructWrap();
                // StructWrap(const StructBuilder &_builder) = default;

                void clear();

                void updateWrap();

            // Scenarios

                void setLogicOp(VkLogicOp _op = VK_LOGIC_OP_MAX_ENUM);

                void setBlendConstants(const vec4 &_blend_constants);
            }; // StructStructWrap<VkPipelineColorBlendStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineTessellationStateCreateInfo> {
            private:
                VkPipelineTessellationStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineTessellationStateCreateInfo)

                StructWrap();
                // StructWrap(const StructBuilder &_sct) = default;
                StructWrap(uint32_t _points_num);

                void clear();

                void updateWrap();
            }; // StructWrap<VkPipelineTessellationStateCreateInfo> END

            // Note: attachment formats of a graphics pipeline used with dynamic rendering (instead of a render pass)
            template <>
            struct StructWrap<VkPipelineRenderingCreateInfo> {
            private:
                VkPipelineRenderingCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineRenderingCreateInfo)

                std::vector<VkFormat> color_formats;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setFormats(Span<VkFormat> _color_formats, VkFormat _depth_format = VK_FORMAT_UNDEFINED, VkFormat _stencil_format = VK_FORMAT_UNDEFINED);
            }; // StructWrap<VkPipelineRenderingCreateInfo> END


//  == === ==== >   Vulkan: CreateInfo (Pipeline)

            template <>
            struct StructWrap<VkGraphicsPipelineCreateInfo> {
            private:
                VkGraphicsPipelineCreateInfo wrap{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkGraphicsPipelineCreateInfo)

                StructWrap<VkPipelineVertexInputStateCreateInfo>     vertex_input_state_wrap;
                StructWrap<VkPipelineInputAssemblyStateCreateInfo> input_assembly_state_wrap;
                StructWrap<VkPipelineTessellationStateCreateInfo>    tessellation_state_wrap;
                StructWrap<VkPipelineViewportStateCreateInfo>            viewport_state_wrap;
                StructWrap<VkPipelineRasterizationStateCreateInfo>  rasterization_state_wrap;
                StructWrap<VkPipelineMultisampleStateCreateInfo>      multisample_state_wrap;
                StructWrap<VkPipelineDepthStencilStateCreateInfo>   depth_stencil_state_wrap;
                StructWrap<VkPipelineColorBlendStateCreateInfo>       color_blend_state_wrap;
                StructWrap<VkPipelineDynamicStateCreateInfo>              dynamic_state_wrap;
                std::vector<StructWrap<VkPipelineShaderStageCreateInfo>>  shader_stages_wrap;
                PipelineStateFlags active_state_flags = PipelineStateBits(0u);

                // Note: used instead of 'renderPass'/'subpass' if 'use_dynamic_rendering' is set (see 'Object::Pipeline::createObject')
                StructWrap<VkPipelineRenderingCreateInfo> rendering_wrap;
                bool use_dynamic_rendering = false;

                std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setPipelineStates(PipelineStateFlags _state_bitmask);

                void setBasePipeline(int32_t _base_pipeline_index = 0, VkPipeline _base_pipeline_handle = VK_NULL_HANDLE);
            }; // StructWrap<VkGraphicsPipelineCreateInfo> END

            template <>
            struct StructWrap<VkComputePipelineCreateInfo> {
            private:
                VkComputePipelineCreateInfo wrap{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkComputePipelineCreateInfo)

                StructWrap<VkPipelineShaderStageCreateInfo>  shader_stage_wrap;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setBasePipeline(int32_t _base_pipeline_index = 0, VkPipeline _base_pipeline_handle = VK_NULL_HANDLE);
            }; // StructWrap<VkComputePipelineCreateInfo> END

            template <>
            struct StructWrap<VkRayTracingPipelineCreateInfoKHR> {
            private:
                VkRayTracingPipelineCreateInfoKHR wrap{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};
                VKFW_STRUCT_WRAP_ACCESS(VkRayTracingPipelineCreateInfoKHR)

                StructWrap<VkPipelineDynamicStateCreateInfo>                  dynamic_state_wrap;
                std::vector<StructWrap<VkPipelineShaderStageCreateInfo>>      shader_stages_wrap;
                std::vector<StructWrap<VkRayTracingShaderGroupCreateInfoKHR>> shader_groups_wrap;
                StructWrap<VkPipelineLibraryCreateInfoKHR>                         lib_info_wrap;
                StructWrap<VkRayTracingPipelineInterfaceCreateInfoKHR>        lib_interface_wrap;
                PipelineStateFlags active_state_flags = PipelineStateBits(0u);

                std::vector<VkPipelineShaderStageCreateInfo>      shader_stages;
                std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_groups;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void setPipelineStates(PipelineStateFlags _state_bitmask);

                void setBasePipeline(int32_t _base_pipeline_index = 0, VkPipeline _base_pipeline_handle = VK_NULL_HANDLE);
            }; // StructWrap<VkRayTracingPipelineCreateInfoKHR>


//  == === ==== >   Vulkan: CreateInfo (Command Buffer-related)

            template <>
            struct StructWrap<VkCommandPoolCreateInfo> {
            private:
                VkCommandPoolCreateInfo wrap{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkCommandPoolCreateInfo)

                StructWrap();

                void clear();

                void updateWrap();
            }; // StructWrap<VkCommandPoolCreateInfo> END

            template <>
            struct StructWrap<VkCommandBufferAllocateInfo> {
            private:
                VkCommandBufferAllocateInfo wrap{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkCommandBufferAllocateInfo)

                StructWrap();


                void clear();

                void updateWrap();
            }; // StructWrap<VkCommandBufferAllocateInfo> END


//  == === ==== >   Vulkan: Other Function Configs

            template <>
            struct StructWrap<VkAllocationCallbacks> {
            private:
                VkAllocationCallbacks *wrap = nullptr;
                VKFW_STRUCT_WRAP_ACCESS(VkAllocationCallbacks*)

                StructWrap();
               ~StructWrap();

                void clear();

                void updateWrap();
            }; // StructWrap<VkAllocationCallbacks> END

            template <>
            struct StructWrap<VkPhysicalDeviceMemoryProperties2> {
            private:
                VkPhysicalDeviceMemoryProperties2 wrap{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
                VKFW_STRUCT_WRAP_ACCESS(VkPhysicalDeviceMemoryProperties2)

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void loadForPhysicalDevice(VkPhysicalDevice _phys_device);

                void printInfo() const;
            }; // StructWrap<VkPhysicalDeviceMemoryProperties2> END

            template <>
            struct StructWrap<VkCommandBufferInheritanceInfo> {
            private:
                VkCommandBufferInheritanceInfo wrap{VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkCommandBufferInheritanceInfo)

                StructWrap();

                void clear();

                void updateWrap();
            }; // StructWrap<VkCommandBufferInheritanceInfo> END

            template <>
            struct StructWrap<VkSubmitInfo> {
            private:
                VkSubmitInfo wrap{VK_STRUCTURE_TYPE_SUBMIT_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkSubmitInfo)

                std::vector<VkSemaphore>      wait_semaphores;
                std::vector<VkPipelineStageFlags> wait_stages;
                std::vector<VkCommandBuffer>  command_buffers;
                std::vector<VkSemaphore>    signal_semaphores;

                StructWrap();
                StructWrap(const StructWrap<VkSubmitInfo> &_wrap);

                void clear();

                void updateWrap();
            }; // StructWrap<VkSubmitInfo> END

            template <>
            struct StructWrap<VkPresentInfoKHR> {
            private:
                VkPresentInfoKHR wrap{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
                VKFW_STRUCT_WRAP_ACCESS(VkPresentInfoKHR)

                std::vector<VkSemaphore> wait_semaphores;
                std::vector<VkSwapchainKHR> swapchains;
                std::vector<uint32_t> image_indices;
                std::vector<VkResult> results;
                bool use_results = false;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                // Note: 'assign' reuses the vector capacity, so per-frame calls don't allocate memory
                void setWholeStruct(Span<VkSemaphore> _wait_semaphores,
                                    Span<VkSwapchainKHR> _swapchains,
                                    Span<uint32_t> _image_indices, bool _use_results = false);
            }; // StructWrap<VkPresentInfoKHR> END

            template <>
            struct StructWrap<VkRenderPassBeginInfo> {
            private:
                VkRenderPassBeginInfo wrap{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkRenderPassBeginInfo)

                std::vector<StructWrap<VkClearValue>> clear_values_wrap;
                StructWrap<VkRect2D> render_area_wrap;

                std::vector<VkClearValue> clear_values;

                StructWrap();

                void clear();

                void updateWrap();

            // Scenarios

                void addClearValue4f(const vec4f &_color);

                void addClearValue4i(const vec4i &_color);

                void addClearValue4u(const vec4u &_color);

                void addClearValueDepthStencil(float _depth, uint32_t _stencil);
            }; // StructWrap<VkRenderPassBeginInfo> END

            template <>
            struct StructWrap<VkCommandBufferBeginInfo> {
            private:
                VkCommandBufferBeginInfo wrap{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkCommandBufferBeginInfo)

                StructWrap<VkCommandBufferInheritanceInfo> inheritance_info_wrap;
                VkBool32 use_inheritance_info = false;

                StructWrap();

                void clear();

                void updateWrap();
            }; // StructWrap<VkCommandBufferBeginInfo> END
        #undef VKFW_STRUCT_WRAP_ACCESS
        }; // Struct END



//  == === ==== ============================== ==== === ==  \\
                    Vulkan Object Wrappers
//  == === ==== ============================== ==== === ==  \\

        namespace Object {

//  == === ==== >   Vulkan Instance

            struct Instance {
                VkInstance handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkInstanceCreateInfo> create_info;

                void createObject(const VkAllocationCallbacks *_create_cb = nullptr, bool _volk_instance_only = false);

                void destroyObject(const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // Instance END


//  == === ==== >   Vulkan Debug Messenger Callback

            struct DebugUtilsMessengerEXT {
                VkDebugUtilsMessengerEXT handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkDebugUtilsMessengerCreateInfoEXT> create_info;

                void createObject(VkInstance _instance, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkInstance _instance, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // DebugMessengerEXT END


//  == === ==== >   Vulkan Physical Device

            struct PhysicalDevice {
                VkPhysicalDevice handle = VK_NULL_HANDLE;
                Util::PhysDeviceManager select_info;

                void createObject();

                void destroyObject();
            }; // PhysicalDevice END


//  == === ==== >   Vulkan Queue Family

            struct QueueFamily {
                VkfwQueueFamily handle = UINT32_MAX;
                Util::QueueFamilyManager select_info;

                void createObject();

                void destroyObject();
            }; // QueueFamily END


//  == === ==== >   Vulkan Queue

            struct Queue {
                std::vector<VkQueue> handles;
                Struct::StructWrap<VkDeviceQueueCreateInfo> create_info;

                void createObject(VkDevice _device);

                void destroyObject();
            }; // Queue END


//  == === ==== >   Vulkan Logical Device

            struct Device {
                VkDevice handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkDeviceCreateInfo> create_info;

                void createObject(VkPhysicalDevice _phys_device, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // Device END


//  == === ==== >   Vulkan Surface

            struct SurfaceKHR {
                VkSurfaceKHR handle = VK_NULL_HANDLE;

                void createObject(VkInstance _instance, GLFWwindow *_glfw_window, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkInstance _instance, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // SurfaceKHR END


//  == === ==== >   Vulkan Swapchain

            struct SwapchainKHR {
                VkSwapchainKHR handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkSwapchainCreateInfoKHR> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // SwapchainKHR END


//  == === ==== >   Vulkan Shader Module

            struct ShaderModule {
                VkShaderModule handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkShaderModuleCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // ShaderModule END


//  == === ==== >   Vulkan Buffer

            struct Buffer {
                VkBuffer handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkBufferCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // Buffer END


//  == === ==== >   Vulkan Buffer View

            struct BufferView {
                VkBufferView handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkBufferViewCreateInfo> create_info;

                void createObject(VkDevice _device, VkBuffer _buffer, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // BufferView END


//  == === ==== >   Vulkan Image

            struct Image {
                VkImage handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkImageCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            };  // Image END


//  == === ==== >   Vulkan Image View

            struct ImageView {
                VkImageView handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkImageViewCreateInfo> create_info;

                void createObject(VkDevice _device, VkImage _image, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // ImageView END


//  == === ==== >   Vulkan Device Memory

            struct DeviceMemory {
                VkDeviceMemory handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkMemoryAllocateInfo> allocate_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_allocate_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // DeviceMemory END


//  == === ==== >   Vulkan Render Pass

            struct RenderPass {
                VkRenderPass handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkRenderPassCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // RenderPass END


//  == === ==== >   Vulkan Descriptor Set Layout

            struct DescriptorSetLayout {
                VkDescriptorSetLayout handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkDescriptorSetLayoutCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // DescriptorSetLayout END


//  == === ==== >   Vulkan Descriptor Pool

            struct DescriptorPool {
                VkDescriptorPool handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkDescriptorPoolCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // DescriptorPool END


//  == === ==== >   Vulkan Descriptor Set

            struct DescriptorSet {
                std::vector<VkDescriptorSet> handles;
                Struct::StructWrap<VkDescriptorSetAllocateInfo> alloc_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device);
            }; // DescriptorSet END


//  == === ==== >   Vulkan Pipeline Layout

            struct PipelineLayout {
                VkPipelineLayout handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkPipelineLayoutCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // PipelineLayout END


//  == === ==== >   Vulkan Pipeline Cache

            struct PipelineCache {
                VkPipelineCache handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkPipelineCacheCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // PipelineCache END


//  == === ==== >   Vulkan Pipeline

            struct Pipeline {
                VkPipeline handle = VK_NULL_HANDLE;

                PipelineType pipeline_type = VKFW_PIPELINE_TYPE_UNDEFINED;
                union {
                    Struct::StructWrap<  VkGraphicsPipelineCreateInfo   >   graphics_create_info_wrap;
                    Struct::StructWrap<   VkComputePipelineCreateInfo   >    compute_create_info_wrap;
                    Struct::StructWrap<VkRayTracingPipelineCreateInfoKHR> raytracing_create_info_wrap;
                };

                Pipeline();
                Pipeline(const Pipeline &_obj_pip);
                Pipeline(Pipeline &&_obj_pip);
               ~Pipeline();

                // Note: the union makes the implicit assignment deleted, it's needed to store pipelines in a 'Pool'
                Pipeline& operator=(const Pipeline &_obj_pip);

                // Returns VK_SUCCESS or another success code (e.g. VK_PIPELINE_COMPILE_REQUIRED, VK_OPERATION_DEFERRED_KHR), throws on errors
                // Note: the render pass and the subpass are taken from the create info;
                //       with 'use_dynamic_rendering' the attachment formats ('rendering_wrap') are chained first and the render pass is ignored
                VkResult createObject(VkDevice _device, VkRenderPass _render_pass, uint32_t _subpass,
                                      VkDeferredOperationKHR _deferred_op, VkPipelineCache _pipeline_cache = VK_NULL_HANDLE,
                                      const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);

            // Scenarios

                void setPipelineFromCopy(const Pipeline &_obj_pip);

                void setPipelineType(PipelineType _pipeline_type);

                void deletePipelineWrap();

                void activatePipelineWrap();

                ContextIndex addShaderStage();

                Struct::StructWrap<VkPipelineShaderStageCreateInfo>* shortcutShaderStage(ContextIndex _ci_shader_stage);
            }; // Pipeline END


//  == === ==== >   Vulkan Framebuffer

            struct Framebuffer {
                VkFramebuffer handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkFramebufferCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // Framebuffer END


//  == === ==== >   Vulkan Command Pool

            struct CommandPool {
                VkCommandPool handle = VK_NULL_HANDLE;

                Struct::StructWrap<VkCommandPoolCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr);

                void createObject(VkDevice _device, VkfwQueueFamily _queue_family, VkCommandPoolCreateFlags _flags = 0u, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // CommandPool END


//  == === ==== >   Vulkan Command Buffer

            struct CommandBuffer {
                std::vector<VkCommandBuffer> handles;
                Struct::StructWrap<VkCommandBufferAllocateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device);

            // Scenarios

                void createPrimary  (VkDevice _device, VkCommandPool _command_pool, uint32_t _buffer_count, const VkAllocationCallbacks *_create_cb = nullptr);

                void createSecondary(VkDevice _device, VkCommandPool _command_pool, uint32_t _buffer_count, const VkAllocationCallbacks *_create_cb = nullptr);
            }; // CommandBuffer END


//  == === ==== >   Vulkan Semaphore

            struct Semaphore {
                std::vector<VkSemaphore> handles;
                Struct::StructWrap<VkSemaphoreCreateInfo> create_info;

                void createObject(VkDevice _device, uint32_t _semaphore_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);
            }; // Semaphore END


//  == === ==== >   Vulkan Fence

            struct Fence {
                std::vector<VkFence> handles;
                Struct::StructWrap<VkFenceCreateInfo> create_info;

                void createObject(VkDevice _device, uint32_t _fence_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);

            // Scenarios

                void createSignaled(VkDevice _device, uint32_t _fence_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr);

                void createNotSignaled(VkDevice _device, uint32_t _fence_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr);
            }; // Fence END


//  == === ==== >   Vulkan Event

            struct Event {
                std::vector<VkEvent> handles;
                Struct::StructWrap<VkEventCreateInfo> create_info;

                void createObject(VkDevice _device, uint32_t _event_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr);

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr);

            // Scenarios

                void createSignaled(VkDevice _device, uint32_t _event_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr);

                void createNotSignaled(VkDevice _device, uint32_t _event_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr);
            }; // Event END

        }; // Object END
    }; // VKFW END
}; // Simple END

#endif