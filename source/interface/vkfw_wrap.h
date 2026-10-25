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
            inline std::vector<VkCommandBuffer> AllocateCommandBuffers(VkDevice _device, VkCommandPool _command_pool,
                                                                VkCommandBufferLevel _level, uint32_t _buffer_count,
                                                                const void *_p_next = nullptr) {
                SVKFW_ASSERT(_buffer_count != 0u, std::invalid_argument, "VKFW::Func :: AllocateCommandBuffers", "'buffer_count' must be positive");

                VkCommandBufferAllocateInfo __info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
                __info.pNext = _p_next;
                __info.commandPool = _command_pool;
                __info.level = _level;
                __info.commandBufferCount = _buffer_count;
                return AllocateCommandBuffers(_device, &__info);
            }

            // Creates 'semaphore_count' semaphores using the same 'create_info' config.
            // Note: Windows headers define 'CreateSemaphore' as a macro, so the preset has a plural name
            inline std::vector<VkSemaphore> CreateSemaphores(VkDevice _device, const VkSemaphoreCreateInfo *_create_info, uint32_t _semaphore_count = 1u, const VkAllocationCallbacks *_allocator = nullptr) {
                std::vector<VkSemaphore> __semaphores((size_t)_semaphore_count, VK_NULL_HANDLE);

                for (uint32_t i = 0u; i < _semaphore_count; ++i)
                    VKFW_CHECK(vkCreateSemaphore(_device, _create_info, _allocator, &__semaphores[i]), "vkCreateSemaphore");
                return __semaphores;
            }

            // Creates 'fence_count' fences using the same 'create_info' config. Requires separate calls for signaled/not signaled fences.
            inline std::vector<VkFence> CreateFences(VkDevice _device, const VkFenceCreateInfo *_create_info, uint32_t _fence_count = 1u, const VkAllocationCallbacks *_allocator = nullptr) {
                std::vector<VkFence> __fences((size_t)_fence_count, VK_NULL_HANDLE);

                for (uint32_t i = 0u; i < _fence_count; ++i)
                    VKFW_CHECK(vkCreateFence(_device, _create_info, _allocator, &__fences[i]), "vkCreateFence");
                return __fences;
            }

            // Creates 'event_count' events using the same 'create_info' config. Requires separate calls for device-only events.
            // Note: Windows headers define 'CreateEvent' as a macro, so the preset has a plural name
            inline std::vector<VkEvent> CreateEvents(VkDevice _device, const VkEventCreateInfo *_create_info, uint32_t _event_count = 1u, const VkAllocationCallbacks *_allocator = nullptr) {
                std::vector<VkEvent> __events((size_t)_event_count, VK_NULL_HANDLE);

                for (uint32_t i = 0u; i < _event_count; ++i)
                    VKFW_CHECK(vkCreateEvent(_device, _create_info, _allocator, &__events[i]), "vkCreateEvent");
                return __events;
            }

            // Queries the capture replay handle size and returns the handles of 'group_count' shader groups
            inline std::vector<uint8_t> GetRayTracingCaptureReplayShaderGroupHandlesKHR(VkPhysicalDevice _phys_device, VkDevice _device, VkPipeline _pipeline,
                                                                                 uint32_t _first_group, uint32_t _group_count) {
                // Note: ray tracing pipeline properties extend VkPhysicalDeviceProperties2 (not memory properties)
                StructChain<VkPhysicalDeviceProperties2, VkPhysicalDeviceRayTracingPipelinePropertiesKHR> __properties;
                vkGetPhysicalDeviceProperties2(_phys_device, &__properties.head());

                const size_t __data_size = size_t(__properties.get<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>().shaderGroupHandleCaptureReplaySize) * _group_count;
                std::vector<uint8_t> __data(__data_size, 0u);
                if (__data_size != 0u)
                    GetRayTracingCaptureReplayShaderGroupHandlesKHR(_device, _pipeline, _first_group, _group_count, __data_size, __data.data());
                return __data;
            }
        }; // Func END



//  == === ==== ======================== ==== === ==  \\
                    Vulkan Utilities
//  == === ==== ======================== ==== === ==  \\

        namespace Util {
            using Simple::Util::StringVec;

        // Default VKFW debug message callback
            inline VKAPI_ATTR VkBool32 VKAPI_CALL vkfwDebugMessageCallback(
                        VkDebugUtilsMessageSeverityFlagBitsEXT _message_severity,
                        VkDebugUtilsMessageTypeFlagsEXT _message_type,
                        const VkDebugUtilsMessengerCallbackDataEXT* _callback_data,
                        void* _user_data) {
                // Severity
                FILE* __outs = stdout;
                const char *_severity = CSISEQ(SGR_COL_FG_MAGENTA) "Untracked severity message" CSISEQ(SGR_COL_FG_DEFAULT);

                switch (_message_severity) {
                    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                        _severity = CSISEQ(SGR_COL_FG_CYAN)   "Verbose" CSISEQ(SGR_COL_FG_DEFAULT); break;
                    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                        _severity = CSISEQ(SGR_COL_FG_CYAN)   "Info"    CSISEQ(SGR_COL_FG_DEFAULT); break;
                    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                        __outs = svkfwwarn;
                        _severity = CSISEQ(SGR_COL_FG_YELLOW) "Warning" CSISEQ(SGR_COL_FG_DEFAULT); break;
                    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                        __outs = stderr;
                        _severity = CSISEQ(SGR_COL_FG_RED)    "Error"   CSISEQ(SGR_COL_FG_DEFAULT); break;
                }

                // Message type
                const char *_msgtype[] = { "General", "Validation", "Performance",
                                           "Device address binding", "Untracked type" };

                bool _typeFlags[] = { bool(_message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT),
                                      bool(_message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT),
                                      bool(_message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT),
                                      bool(_message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT),
                                           _message_type > VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT };
                bool _multFlags = false;
                std::string _typestr{};
                for (uint32_t i = 0u; i < 5u; ++i) {
                    if (_typeFlags[i]) {
                        if (!_typestr.empty()) {
                            _multFlags = true;
                            _typestr += " & ";
                        }
                        _typestr += _msgtype[i];
                    }
                }
                if (_multFlags) _typestr = '(' + _typestr + ')';

                // Output
                if (_message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                    // there are multi-line "Info" messages, adding prefix results in poor readability
                    fprintf(__outs, CSISEQ(SGR_EMPH_BOLD) "[VLayer %s %s]" CSISEQ("") "\n",
                                    _typestr.c_str(), _severity);
                fprintf(__outs, "%s\n", _callback_data->pMessage);

                return VK_FALSE;
            }


//  == === ==== >   Extension/Layer checks for Instance/Device

            inline bool allLayersAvailable(const StringVec &_layers, const std::vector<VkLayerProperties> _layer_list) {
                bool __res = true;
                for (const std::string &layer_name : _layers.list) {
                    bool __layer_found = false;

                    for (const VkLayerProperties &layer_properties : _layer_list) {
                        if (layer_name == layer_properties.layerName) {
                            __layer_found = true;
                            break;
                        }
                    }

                    if (!__layer_found) {
                        __res = false;
                        break;
                    }
                }
                return __res;
            }

            inline bool allExtAvailable(const StringVec &_extensions, const std::vector<VkExtensionProperties> &_ext_list) {
                bool __res = true;
                for (const std::string &ext_name : _extensions.list) {
                    bool __ext_found = false;

                    for (const VkExtensionProperties &ext_properties : _ext_list) {
                        if (ext_name == ext_properties.extensionName) {
                            __ext_found = true;
                            break;
                        }
                    }

                    if (!__ext_found) {
                        __res = false;
                        break;
                    }
                }
                return __res;
            }

            inline void checkLayers(const StringVec &_layers, bool *_flags, const std::vector<VkLayerProperties> &_layer_list) {
                for (uint32_t i = 0u; i < _layers.list.size(); ++i) {
                    bool __layer_found = false;

                    for (const VkLayerProperties &layer_properties : _layer_list) {
                        if (_layers.list[i] == layer_properties.layerName) {
                            __layer_found = true;
                            break;
                        }
                    }

                    _flags[i] = __layer_found;
                    if (!__layer_found)
                        fprintf(svkfwwarn, "%s %s\n", SVKFW_WRAPWARN("VKFW::Util :: checkLayers","could not find layer"), _layers.list[i].c_str());
                }
            }

            inline void checkExtensions(const StringVec &_extensions, bool *_flags, const std::vector<VkExtensionProperties> &_ext_list) {
                for (uint32_t i = 0u; i < _extensions.list.size(); ++i) {
                    bool __ext_found = false;

                    for (const VkExtensionProperties &ext_properties : _ext_list) {
                        if (_extensions.list[i] == ext_properties.extensionName) {
                            __ext_found = true;
                            break;
                        }
                    }

                    _flags[i] = __ext_found;
                    if (!__ext_found)
                        fprintf(svkfwwarn, "%s %s\n", SVKFW_WRAPWARN("VKFW::Util :: checkExtensions","could not find extension"), _extensions.list[i].c_str());
                }
            }


            inline bool allInstanceLayersAvailable(const StringVec &_layers) {
                std::vector<VkLayerProperties> __layer_list{Func::EnumerateInstanceLayerProperties()};
                return allLayersAvailable(_layers, __layer_list);
            }

            inline bool allInstanceExtAvailable(const StringVec &_extensions, const char *_layer = nullptr) {
                std::vector<VkExtensionProperties> __ext_list{Func::EnumerateInstanceExtensionProperties(_layer)};
                return allExtAvailable(_extensions, __ext_list);
            }

            inline bool allDeviceLayersAvailable(VkPhysicalDevice _phys_device, const StringVec &_layers) {
                std::vector<VkLayerProperties> __layer_list{Func::EnumerateDeviceLayerProperties(_phys_device)};
                return allLayersAvailable(_layers, __layer_list);
            }

            inline bool allDeviceExtAvailable(VkPhysicalDevice _phys_device, const StringVec &_extensions, const char *_layer = nullptr) {
                std::vector<VkExtensionProperties> __ext_list{Func::EnumerateDeviceExtensionProperties(_phys_device, _layer)};
                return allExtAvailable(_extensions, __ext_list);
            }


            inline void checkInstanceLayers(const StringVec &_layers, bool *_flags) {
                if (_flags == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW::Util :: checkInstanceLayers",
                                    "'flags' must be a pointer to an array of 'layers.size()' boolean values"));

                std::vector<VkLayerProperties> __layer_list{Func::EnumerateInstanceLayerProperties()};

                checkLayers(_layers, _flags, __layer_list);
            }

            inline void checkInstanceExtensions(const StringVec &_extensions, bool *_flags, const char *_layer = nullptr) {
                if (_flags == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW::Util :: checkInstanceExtensions",
                                    "'flags' must be a pointer to an array of 'extensions.size()' boolean values"));

                std::vector<VkExtensionProperties> __ext_list{Func::EnumerateInstanceExtensionProperties(_layer)};

                checkExtensions(_extensions, _flags, __ext_list);
            }

            inline void checkDeviceLayers(VkPhysicalDevice _phys_device, const StringVec &_layers, bool *_flags) {
                if (_flags == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW::Util :: checkDeviceLayers",
                                    "'flags' must be a pointer to an array of 'layers.size()' boolean values"));

                std::vector<VkLayerProperties> __layer_list{Func::EnumerateDeviceLayerProperties(_phys_device)};

                checkLayers(_layers, _flags, __layer_list);
            }

            inline void checkDeviceExtensions(VkPhysicalDevice _phys_device, const StringVec &_extensions, bool *_flags, const char *_layer = nullptr) {
                if (_flags == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW::Util :: checkDeviceExtensions",
                                    "'flags' must be a pointer to an array of 'extensions.size()' boolean values"));

                std::vector<VkExtensionProperties> __ext_list{Func::EnumerateDeviceExtensionProperties(_phys_device, _layer)};

                checkExtensions(_extensions, _flags, __ext_list);
            }


//  == === ==== >   Physical Device Selection Utilities

            inline uint32_t checkPhysicalDeviceMemoryProperties(VkPhysicalDevice _phys_device, uint32_t _req_type_properties,
                                                         VkMemoryPropertyFlags _req_memory_properties) {
                VkPhysicalDeviceMemoryProperties __mem_properties;
                vkGetPhysicalDeviceMemoryProperties(_phys_device, &__mem_properties);

                for (uint32_t i = 0u; i < __mem_properties.memoryTypeCount; i++) {
                    bool __type_req_satisfied = _req_type_properties & (1 << i);
                    bool  __mem_req_satisfied = (__mem_properties.memoryTypes[i].propertyFlags & _req_memory_properties) == _req_memory_properties;

                    if (__type_req_satisfied && __mem_req_satisfied)
                        return i;
                }

                fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::Util :: checkPhysicalDeviceMemoryProperties", "Could not find suitable memory type"));
                return UINT32_MAX;
            }

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
                                   const VkPhysicalDeviceProperties &_properties)
                                : phys_device{_phys_device}, features{_features}, properties{_properties} {}
                PhysDeviceFullInfo(const PhysDeviceFullInfo &_elem_view)
                                : phys_device{_elem_view.phys_device}, features{_elem_view.features},
                                  properties{_elem_view.properties} {}


                vec4u apiVersion() const {
                    vec4u __ver;

                    __ver["xyz"] = VulkanAPIVersion::split(properties.apiVersion, &__ver.w);
                    return __ver;
                }

                std::string infoString() const {
                    std::string _out = "Physical Device Info:\n";
                    vec4u _ver = apiVersion();
                    _out = _out + "Device name: " + properties.deviceName + '\n'
                                + "API version: " + std::to_string(_ver.x) + '.'
                                                  + std::to_string(_ver.y) + '.'
                                                  + std::to_string(_ver.z) +
                                     " (Variant " + std::to_string(_ver.w) + ")\n"
                                // + "Limit: UB min alignment = " + std::to_string(properties.limits.minUniformBufferOffsetAlignment) + '\n'
                                ;
                    return _out;
                }

            // Scenarios

                std::vector<VkExtensionProperties> getAllDeviceExtensions(const char *_layer = nullptr) const {
                    return Func::EnumerateDeviceExtensionProperties(phys_device, _layer);
                }

                bool allExtAvailable(const StringVec &_extensions) const {
                    return Util::allDeviceExtAvailable(phys_device, _extensions);
                }

                void checkExtensions(const StringVec &_extensions, bool *_flags) const {
                    Util::checkDeviceExtensions(phys_device, _extensions, _flags);
                }
            }; // PhysDeviceFullInfo END


            typedef bool (*PhysDeviceTestFunc) (const PhysDeviceFullInfo &_phys_device_info);
            typedef  int (*PhysDeviceRankFunc) (const PhysDeviceFullInfo &_phys_device_info);

            namespace SelectPhysDevice {
                inline bool DefaultGraphicsTest(const PhysDeviceFullInfo &_phys_device_info) {
                    return _phys_device_info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                           _phys_device_info.allExtAvailable({ {VK_KHR_SWAPCHAIN_EXTENSION_NAME} });
                }
            }; // SelectPhysDevice END


            struct PhysDeviceManager {
                std::vector<VkPhysicalDevice> phys_devices;
                std::vector<VkPhysicalDeviceFeatures> features;
                std::vector<VkPhysicalDeviceProperties> properties;
                PhysDeviceTestFunc test_func = SelectPhysDevice::DefaultGraphicsTest;
                PhysDeviceRankFunc rank_func = nullptr;
                VkBool32 selection_by_rank = false;

                PhysDeviceManager(VkInstance _instance = VK_NULL_HANDLE) {
                    if (_instance != VK_NULL_HANDLE) loadInfo(_instance);
                }

                void clear() {
                    features.clear();
                    properties.clear();
                    phys_devices.clear();
                }

                VkPhysicalDevice selectOne() {
                    return selection_by_rank ? rankPickOne(rank_func) : testPickOne(test_func);
                }

                void selectWithTest(PhysDeviceTestFunc _test_func = SelectPhysDevice::DefaultGraphicsTest) {
                    test_func = _test_func;
                    selection_by_rank = false;
                }

                void selectWithRank(PhysDeviceRankFunc _rank_func = nullptr) {
                    rank_func = _rank_func;
                    selection_by_rank = true;
                }

                const PhysDeviceFullInfo getElemInfo(uint32_t _index) { return { phys_devices[_index], features[_index], properties[_index] }; }

                void loadInfo(VkInstance _instance) {
                    if (_instance == VK_NULL_HANDLE)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::Util::PhysDeviceManager :: loadInfo", "instance is not set"));

                    uint32_t __devCount = 0;
                    vkEnumeratePhysicalDevices(_instance, &__devCount, nullptr);

                    if (__devCount == 0)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::Util::PhysDeviceManager :: loadInfo", "no GPUs with Vulkan support available"));

                    features.resize(__devCount);
                    properties.resize(__devCount);
                    phys_devices.resize(__devCount);
                    vkEnumeratePhysicalDevices(_instance, &__devCount, phys_devices.data());

                    for (uint32_t i = 0u; i < __devCount; ++i) {
                        vkGetPhysicalDeviceProperties(phys_devices[i], &properties[i]);
                        vkGetPhysicalDeviceFeatures  (phys_devices[i], &  features[i]);
                    }
                }


                // Returns the first physical device for which _testFunc({device, features, properties}) == true
                VkPhysicalDevice testPickOne(const PhysDeviceTestFunc _testFunc) {
                    uint32_t __res_index = phys_devices.size();
                    VkPhysicalDevice __res = VK_NULL_HANDLE;

                    for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                        printf(SVKFW_WRAPINFO("VKFW::Util::PhysDeviceManager :: testPickOne", "Found GPU %d:\n%s\n"), i, PhysDeviceFullInfo{phys_devices[i], features[i], properties[i]}.infoString().c_str());
                        if (__res == VK_NULL_HANDLE && _testFunc({phys_devices[i], features[i], properties[i]})) {
                            __res_index = i;
                            __res = phys_devices[i];
                        }
                    }

                    if (__res != VK_NULL_HANDLE) {
                        printf("\n");
                        printf(SVKFW_WRAPINFO("VKFW::Util::PhysDeviceManager :: testPickOne", "Selected GPU %d:\n%s\n"), __res_index, PhysDeviceFullInfo{phys_devices[__res_index], features[__res_index], properties[__res_index]}.infoString().c_str());
                        return __res;
                    }

                    throw std::runtime_error(SVKFW_WRAPERR("VKFW::Util::PhysDeviceManager :: testPickOne", "Failed to find a suitable GPU"));
                    return VK_NULL_HANDLE;
                }

                // Returns all physical devices for which _testFunc({device, features, properties}) == true
                std::vector<VkPhysicalDevice> testPickAll(const PhysDeviceTestFunc _testFunc) {
                    std::vector<VkPhysicalDevice> __res_devices;
                    for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                        if (_testFunc({phys_devices[i], features[i], properties[i]}))
                            __res_devices.push_back(phys_devices[i]);
                    }

                    return __res_devices;
                }

                // Ranks all physical devices and returns one with max _rankFunc({device, features, properties})
                VkPhysicalDevice rankPickOne(const PhysDeviceRankFunc _rankFunc) {
                    int __maxRank = 0;
                    uint32_t __ind = phys_devices.size();
                    for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                        int __rank = _rankFunc({phys_devices[i], features[i], properties[i]});
                        if (__rank > __maxRank) {
                            __maxRank = __rank;
                            __ind = i;
                        }
                    }
                    if (__ind >= phys_devices.size())
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::Util::PhysDeviceManager :: rankPickOne (1)", "Failed to find a suitable GPU"));
                    return phys_devices[__ind];
                }

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

                QueueFamilyFullInfo(const VkQueueFamilyProperties &_props) : properties{_props} {}

                bool supportsGraphics()        const { return properties.queueFlags &  VK_QUEUE_GRAPHICS_BIT;         }
                bool supportsCompute()         const { return properties.queueFlags &  VK_QUEUE_COMPUTE_BIT;          }
                bool supportsTransfer()        const { return properties.queueFlags & (VK_QUEUE_GRAPHICS_BIT |
                                                                                       VK_QUEUE_COMPUTE_BIT  |
                                                                                       VK_QUEUE_TRANSFER_BIT);        }
                bool supportsSparseBinding()   const { return properties.queueFlags &  VK_QUEUE_SPARSE_BINDING_BIT;   }
                bool supportsProtectedMemory() const { return properties.queueFlags &  VK_QUEUE_PROTECTED_BIT;        }
                bool supportsVideoDecode()     const { return properties.queueFlags &  VK_QUEUE_VIDEO_DECODE_BIT_KHR; }
                bool supportsVideoEncode()     const { return properties.queueFlags &  VK_QUEUE_VIDEO_ENCODE_BIT_KHR; }
                bool supportsOpticalFlow()     const { return properties.queueFlags &  VK_QUEUE_OPTICAL_FLOW_BIT_NV;  }

                uint32_t queuesNum()                const { return   properties.queueCount; }
                uint32_t timestampValidRangeBits()  const { return   properties.timestampValidBits; }
                vec3u minImageTransferGranularity() const { return { properties.minImageTransferGranularity.width,
                                                                     properties.minImageTransferGranularity.height,
                                                                     properties.minImageTransferGranularity.depth };  }

                std::string infoString() const {
                    std::string __res{"Queue family info:\n"};
                    vec3u __gran{minImageTransferGranularity()};
                    return __res +
                            "supports graphics:          " + (supportsGraphics()        ? "yes" : "no") + '\n' +
                            "supports compute:           " + (supportsCompute()         ? "yes" : "no") + '\n' +
                            "supports transfer:          " + (supportsTransfer()        ? "yes" : "no") + '\n' +
                            "supports sparse binding:    " + (supportsSparseBinding()   ? "yes" : "no") + '\n' +
                            "supports protected memory:  " + (supportsProtectedMemory() ? "yes" : "no") + '\n' +
                            "supports video decode:      " + (supportsVideoDecode()     ? "yes" : "no") + '\n' +
                            "supports video encode:      " + (supportsVideoEncode()     ? "yes" : "no") + '\n' +
                            "supports optical flow:      " + (supportsOpticalFlow()     ? "yes" : "no") + '\n' +
                            "number of queues:           " + std::to_string(queuesNum())                + '\n' +
                            "timestamp valid range bits: " + std::to_string(timestampValidRangeBits())  + '\n' +
                            "minimal granularity:       (" + std::to_string(__gran.x) + ", "
                                                           + std::to_string(__gran.y) + ", "
                                                           + std::to_string(__gran.z) + ")\n";
                }
            }; // QueueFamilyFullInfo END


            typedef bool (*QueueFamilyTestFunc) (const QueueFamilyFullInfo &_info, bool _surf_support);
            typedef  int (*QueueFamilyRankFunc) (const QueueFamilyFullInfo &_info, bool _surf_support);

            namespace SelectQueueFamily {
                inline bool testGraphics(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsGraphics(); }
                inline bool testCompute (const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsCompute (); }
                inline bool testTransfer(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsTransfer(); }
                inline bool testSparseBinding(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsSparseBinding(); }
                inline bool testProtectedMemory(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsProtectedMemory(); }
                inline bool testVideoDecode(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsVideoDecode(); }
                inline bool testVideoEncode(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsVideoEncode(); }
                inline bool testOpticalFlow(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsOpticalFlow(); }

                inline bool testSurfAndGraphics(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsGraphics(); }
                inline bool testSurfAndCompute (const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsCompute (); }
                inline bool testSurfAndTransfer(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsTransfer(); }
                inline bool testSurfAndSparseBinding(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsSparseBinding(); }
                inline bool testSurfAndProtectedMemory(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsProtectedMemory(); }
                inline bool testSurfAndVideoDecode(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsVideoDecode(); }
                inline bool testSurfAndVideoEncode(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsVideoEncode(); }
                inline bool testSurfAndOpticalFlow(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsOpticalFlow(); }


            // TODO: practice, then redo these rank functions
            // For now, result >0 means score; if <=0, then queue family doesn't have required feature.

                inline int rankGraphics(const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 5 * _info.supportsGraphics()
                             - _info.supportsCompute()
                             - _info.supportsVideoDecode()
                             - _info.supportsVideoEncode()
                             - _info.supportsOpticalFlow();
                }
                inline int rankCompute (const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 5 * _info.supportsCompute()
                             - _info.supportsGraphics()
                             - _info.supportsVideoDecode()
                             - _info.supportsVideoEncode()
                             - _info.supportsOpticalFlow();
                }
                inline int rankTransfer(const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 6 * _info.supportsTransfer()
                             - _info.supportsCompute()
                             - _info.supportsGraphics()
                             - _info.supportsVideoDecode()
                             - _info.supportsVideoEncode()
                             - _info.supportsOpticalFlow();
                }
                inline int rankSparseBinding  (const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 6 * _info.supportsSparseBinding()
                             - _info.supportsCompute()
                             - _info.supportsGraphics()
                             - _info.supportsVideoDecode()
                             - _info.supportsVideoEncode()
                             - _info.supportsOpticalFlow();
                }
                inline int rankProtectedMemory(const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return _info.supportsProtectedMemory() ? 1 : -1;
                }
                inline int rankVideoDecode(const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 5 * _info.supportsVideoDecode()
                             - _info.supportsGraphics()
                             - _info.supportsCompute()
                             - _info.supportsVideoEncode()
                             - _info.supportsOpticalFlow();
                }
                inline int rankVideoEncode(const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 5 * _info.supportsVideoEncode()
                             - _info.supportsGraphics()
                             - _info.supportsCompute()
                             - _info.supportsVideoDecode()
                             - _info.supportsOpticalFlow();
                }
                inline int rankOpticalFlow(const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 5 * _info.supportsOpticalFlow()
                             - _info.supportsGraphics()
                             - _info.supportsCompute()
                             - _info.supportsVideoDecode()
                             - _info.supportsVideoEncode();
                }
            }; // SelectQueueFamily END


            struct QueueFamilyManager {
                std::vector<VkQueueFamilyProperties> properties;
                std::vector<bool> surface_support;
                QueueFamilyTestFunc test_func = nullptr;
                QueueFamilyRankFunc rank_func = nullptr;
                VkBool32 selection_by_rank = false;


                uint32_t selectOne() {
                    return selection_by_rank ? rankPickOne(rank_func) : testPickOne(test_func);
                }

                void selectWithTest(QueueFamilyTestFunc _test_func) {
                    test_func = _test_func;
                    selection_by_rank = false;
                }

                void selectWithRank(QueueFamilyRankFunc _rank_func) {
                    rank_func = _rank_func;
                    selection_by_rank = true;
                }

                QueueFamilyManager(VkInstance _instance = VK_NULL_HANDLE, VkPhysicalDevice _phys_device = VK_NULL_HANDLE) {
                    if (_instance != VK_NULL_HANDLE && _phys_device != VK_NULL_HANDLE)
                        loadInfo(_instance, _phys_device);
                }

                void clear() {
                    properties.clear();
                    surface_support.clear();
                }

                const QueueFamilyFullInfo getElemInfo(uint32_t _index) const { return { properties[_index] }; }


                void loadInfo(VkInstance _instance, VkPhysicalDevice _phys_device) {
                    uint32_t __qfamily_count = 0;
                    vkGetPhysicalDeviceQueueFamilyProperties(_phys_device, &__qfamily_count, nullptr);

                    properties.resize(__qfamily_count);
                    vkGetPhysicalDeviceQueueFamilyProperties(_phys_device, &__qfamily_count, properties.data());

                    surface_support.resize(properties.size());
                    for (uint32_t i = 0u; i < properties.size(); ++i)
                        surface_support[i] = glfwGetPhysicalDevicePresentationSupport(_instance, _phys_device, i);
                }


                uint32_t testPickOne(QueueFamilyTestFunc _test_func) {
                    for (uint32_t i = 0u; i < properties.size(); ++i)
                        if (_test_func(properties[i], surface_support[i]))
                            return i;

                    fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::QueueFamilyManager :: testPickOne", "could not find queue family\n"));
                    return UINT32_MAX;
                }

                std::vector<uint32_t> testPickAll(QueueFamilyTestFunc _test_func) {
                    std::vector<uint32_t> __indices;

                    for (uint32_t i = 0u; i < properties.size(); ++i)
                        if (_test_func(properties[i], surface_support[i]))
                            __indices.push_back(i);

                    if (__indices.empty())
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::QueueFamilyManager :: testPickAll", "could not find queue family\n"));
                    return __indices;
                }

                uint32_t rankPickOne(QueueFamilyRankFunc _rank_func) {
                    uint32_t __maxI = properties.size();
                    int  __maxR = INT_MIN;

                    for (uint32_t i = 0u; i < properties.size(); ++i) {
                        int __rank = _rank_func(properties[i], surface_support[i]);
                        if (__rank > __maxR) {
                            __maxI = i;
                            __maxR = __rank;
                        }
                    }

                    if (__maxI >= properties.size()) {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::QueueFamilyManager :: rankPickOne", "could not find queue family\n"));
                        return UINT32_MAX;
                    }
                    return __maxI;
                }

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
                inline bool DefaultFormatTest(const VkSurfaceFormatKHR &_format) {
                    return _format.format == VK_FORMAT_B8G8R8A8_SRGB && _format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR;
                }
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
                StructWrap(const VkExtent2D &_extent, float _x = 0.f, float _y = 0.f, float _depthMin = 0.f, float _depthMax = 1.f) :
                            wrap{_x, _y, (float)_extent.width, (float)_extent.height, _depthMin, _depthMax} {}
                StructWrap(const vec2f &_extent = 0.f, const vec2f &_offset = 0.f, const vec2f &_depth = { 0.f, 1.f }) :
                            wrap{_offset.x, _offset.y, _extent.x, _extent.y, _depth.x, _depth.y} {}
                StructWrap(const StructWrap &_viewport) : wrap{_viewport.wrap} {}

                void clear() {
                    wrap = VkViewport{ 0.f, 0.f, 0.f, 0.f, 0.f, 1.f };
                }

                void updateWrap() {}

            // Scenarios

                void setOffset(const VkOffset2D &_offset) { wrap.x = _offset.x; wrap.y = _offset.y; }
                void setExtent(const VkExtent2D &_extent) { wrap.width = _extent.width; wrap.height = _extent.height; }

                void setOffset(const vec2 &_offset) { wrap.x       = _offset.x; wrap.y       = _offset.y; }
                void setExtent(const vec2 &_extent) { wrap.width   = _extent.x; wrap.height  = _extent.y; }
                void setDepth (const vec2 &_depth)  { wrap.minDepth = _depth.x; wrap.maxDepth = _depth.y; }

                vec2 offset() const { return { wrap.x,        wrap.y        }; }
                vec2 extent() const { return { wrap.width,    wrap.height   }; }
                vec2  depth() const { return { wrap.minDepth, wrap.maxDepth }; }
            }; // StructWrap<VkViewport> END

            template <>
            struct StructWrap<VkRect2D> {
            private:
                VkRect2D wrap{ {0, 0}, {0u, 0u} };
                VKFW_STRUCT_WRAP_ACCESS(VkRect2D)

                StructWrap() {}
                StructWrap(const StructWrap &_sct) : wrap{_sct.wrap} {}
                // Note: VkRect2D fields order - offset, extent
                StructWrap(const VkExtent2D &_extent, const VkOffset2D &_offset = {0, 0})
                                : wrap{ {_offset.x, _offset.y}, {_extent.width, _extent.height} } {}
                StructWrap(const vec2u &_extent, const vec2i &_offset = 0)
                                : wrap{ {_offset.x, _offset.y}, {_extent.x, _extent.y} } {}

                void clear() {
                    wrap = VkRect2D{ {0, 0}, {0u, 0u} };
                }

                void updateWrap() {}

            // Scenarios

                void setOffset(const VkOffset2D &_offset) { wrap.offset.x = _offset.x; wrap.offset.y = _offset.y; }
                void setExtent(const VkExtent2D &_extent) { wrap.extent.width = _extent.width; wrap.extent.height = _extent.height; }

                void setOffset(const vec2i &_offset) { wrap.offset.x     = _offset.x; wrap.offset.y      = _offset.y; }
                void setExtent(const vec2u &_extent) { wrap.extent.width = _extent.x; wrap.extent.height = _extent.y; }

                vec2i offset() const { return { wrap.offset.x,     wrap.offset.y      }; }
                vec2u extent() const { return { wrap.extent.width, wrap.extent.height }; }
            }; // StructWrap<VkRect2D> END

            template <>
            struct StructWrap<VkClearValue> {
            private:
                VkClearValue wrap{};
                VKFW_STRUCT_WRAP_ACCESS(VkClearValue)

                StructWrap() {}
                StructWrap(float    _v0, float    _v1, float    _v2, float    _v3) { fromColorVec4f({_v0, _v1, _v2, _v3}); }
                StructWrap(int32_t  _v0, int32_t  _v1, int32_t  _v2, int32_t  _v3) { fromColorVec4i({_v0, _v1, _v2, _v3}); }
                StructWrap(uint32_t _v0, uint32_t _v1, uint32_t _v2, uint32_t _v3) { fromColorVec4u({_v0, _v1, _v2, _v3}); }
                StructWrap(float _depth, uint32_t _stencil) { fromDepthStencil(_depth, _stencil); }

                void clear() {
                    wrap = VkClearValue{};
                }

                void updateWrap() {}

            // Scenarios

                void fromColorVec4f(const vec4f &_col) {
                    wrap.color.float32[0] = _col.x;
                    wrap.color.float32[1] = _col.y;
                    wrap.color.float32[2] = _col.z;
                    wrap.color.float32[3] = _col.w;
                }
                void fromColorVec4i(const vec4i &_col) {
                    wrap.color.int32[0] = _col.x;
                    wrap.color.int32[1] = _col.y;
                    wrap.color.int32[2] = _col.z;
                    wrap.color.int32[3] = _col.w;
                }
                void fromColorVec4u(const vec4u &_col) {
                    wrap.color.uint32[0] = _col.x;
                    wrap.color.uint32[1] = _col.y;
                    wrap.color.uint32[2] = _col.z;
                    wrap.color.uint32[3] = _col.w;
                }
                void fromDepthStencil(float _depth, uint32_t _stencil) {
                    wrap = VkClearValue{};
                    wrap.depthStencil.depth   = _depth;
                    wrap.depthStencil.stencil = _stencil;
                }

            // Static

                static VkClearValue ColorVec4f(const vec4f &_col) {
                    VkClearValue __res{};
                    __res.color.float32[0] = _col.x;
                    __res.color.float32[1] = _col.y;
                    __res.color.float32[2] = _col.z;
                    __res.color.float32[3] = _col.w;
                    return __res;
                }
                static VkClearValue ColorVec4i(const vec4i &_col) {
                    VkClearValue __res{};
                    __res.color.int32[0] = _col.x;
                    __res.color.int32[1] = _col.y;
                    __res.color.int32[2] = _col.z;
                    __res.color.int32[3] = _col.w;
                    return __res;
                }
                static VkClearValue ColorVec4u(const vec4u &_col) {
                    VkClearValue __res{};
                    __res.color.uint32[0] = _col.x;
                    __res.color.uint32[1] = _col.y;
                    __res.color.uint32[2] = _col.z;
                    __res.color.uint32[3] = _col.w;
                    return __res;
                }
                static VkClearValue DepthStencil(float _depth, uint32_t _stencil) {
                    VkClearValue __res{};
                    __res.depthStencil.depth   = _depth;
                    __res.depthStencil.stencil = _stencil;
                    return __res;
                }
            }; // StructWrap<VkClearValue> END

            template <>
            struct StructWrap<VkAttachmentDescription> {
            private:
                VkAttachmentDescription wrap{};
                VKFW_STRUCT_WRAP_ACCESS(VkAttachmentDescription)

                StructWrap() {}
                StructWrap(const StructWrap &_sct) : wrap{_sct.wrap} {}
                StructWrap(const VkAttachmentDescription &_description) : wrap{_description} {}

                void clear() {
                    wrap = VkAttachmentDescription{};
                }

                void updateWrap() {}

            // Scenarios

                void setColorOps(VkAttachmentLoadOp _loadOp, VkAttachmentStoreOp _storeOp) {
                    wrap.loadOp = _loadOp;
                    wrap.storeOp = _storeOp;
                }
                void setStencilOps(VkAttachmentLoadOp _loadOp, VkAttachmentStoreOp _storeOp) {
                    wrap.stencilLoadOp = _loadOp;
                    wrap.stencilStoreOp = _storeOp;
                }
                void setLayouts(VkImageLayout _initLayout, VkImageLayout _finalLayout) {
                    wrap.initialLayout = _initLayout;
                    wrap.finalLayout = _finalLayout;
                }
                void setFormatAndSize(VkFormat _format, VkSampleCountFlagBits _samples) {
                    wrap.format = _format;
                    wrap.samples = _samples;
                }
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

                StructWrap() { clear(); }

                void clear() {
                    wrap = VkSubpassDescription{};
                    depth_stencil_attachment = VkAttachmentReference{VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED};

                    color_attachments   .clear();
                    input_attachments   .clear();
                    resolve_attachments .clear();
                    preserve_attachments.clear();
                }

                // Note: pointers to members are set here (and not in 'clear()'), because the structure can be copied/moved (e.g. by std::vector)
                void updateWrap() {
                    wrap.    colorAttachmentCount =    color_attachments.size();
                    wrap.   pColorAttachments     =    color_attachments.data();
                    wrap.    inputAttachmentCount =    input_attachments.size();
                    wrap.   pInputAttachments     =    input_attachments.data();
                    wrap. preserveAttachmentCount = preserve_attachments.size();
                    wrap.pPreserveAttachments     = preserve_attachments.data();
                    wrap. pResolveAttachments     =  resolve_attachments.empty() ? nullptr : resolve_attachments.data();
                    wrap.pDepthStencilAttachment  = depth_stencil_attachment.attachment != VK_ATTACHMENT_UNUSED ? &depth_stencil_attachment : nullptr;
                }

            // Scenarios

                void setDepthStencilAttachment(uint32_t _attachment_id, VkImageLayout _layout) {
                    depth_stencil_attachment.attachment = _attachment_id;
                    depth_stencil_attachment.layout     = _layout;
                }

                ContextIndex addColorAttachment(uint32_t _attachment_id, VkImageLayout _layout) {
                    color_attachments.push_back(VkAttachmentReference{_attachment_id, _layout});
                    return color_attachments.size()-1;
                }

                ContextIndex addInputAttachment(uint32_t _attachment_id, VkImageLayout _layout) {
                    input_attachments.push_back(VkAttachmentReference{_attachment_id, _layout});
                    return input_attachments.size()-1;
                }

                ContextIndex addResolveAttachment(uint32_t _attachment_id, VkImageLayout _layout) {
                    resolve_attachments.push_back(VkAttachmentReference{_attachment_id, _layout});
                    return resolve_attachments.size()-1;
                }
            }; // StructWrap<VkSubpassDescription> END

            template <>
            struct StructWrap<VkSubpassDependency> {
            private:
                VkSubpassDependency wrap{};
                VKFW_STRUCT_WRAP_ACCESS(VkSubpassDependency)

                StructWrap() {}
                StructWrap(const StructWrap &_sct) : wrap{_sct.wrap} {}
                StructWrap(const VkSubpassDependency &_subpass_dependency) : wrap{_subpass_dependency} {}

                void clear() {
                    wrap = VkSubpassDependency{};
                }

                void updateWrap() {}

            // Scenarios

                void setSubpasses(uint32_t _src_subpass, uint32_t _dst_subpass) {
                    wrap.srcSubpass = _src_subpass;
                    wrap.dstSubpass = _dst_subpass;
                }

                void setSrcMasks(VkPipelineStageFlags _src_stage_mask, VkAccessFlags _src_acc_mask) {
                    wrap.srcStageMask  = _src_stage_mask;
                    wrap.srcAccessMask =   _src_acc_mask;
                }

                void setDstMasks(VkPipelineStageFlags _dst_stage_mask, VkAccessFlags _dst_acc_mask) {
                    wrap. dstStageMask = _dst_stage_mask;
                    wrap.dstAccessMask =   _dst_acc_mask;
                }
            }; // StructWrap<VkSubpassDependency> END

            template <>
            struct StructWrap<VkSpecializationInfo> {
            private:
                VkSpecializationInfo wrap{};
                VKFW_STRUCT_WRAP_ACCESS(VkSpecializationInfo)

                std::vector<VkSpecializationMapEntry> map_entries;
                std::vector<unsigned char> data;

                StructWrap() {}

                void clear() {
                    wrap = VkSpecializationInfo{};
                    map_entries.clear();
                    data.clear();
                }

                void updateWrap() {
                    wrap.mapEntryCount = map_entries.size();
                    wrap.pMapEntries   = map_entries.data();
                    wrap.dataSize = data.size();
                    wrap.pData    = data.data();
                }

            // Scenarios

                void setData(const void* _data, size_t _data_size) {
                    data.resize(_data_size);
                    memcpy(data.data(), _data, _data_size);
                }
            }; // StructWrap<VkSpecializationInfo> END

            template <>
            struct StructWrap<VkDescriptorSetLayoutBinding> {
            private:
                VkDescriptorSetLayoutBinding wrap{};
                VKFW_STRUCT_WRAP_ACCESS(VkDescriptorSetLayoutBinding)

                std::vector<VkSampler> immutable_samplers;

                StructWrap() {}
                // StructWrap(const Builder &_builder) = default;

                void clear() {
                    wrap = VkDescriptorSetLayoutBinding{};
                    immutable_samplers.clear();
                }

                void updateWrap() {
                    wrap.descriptorCount    = immutable_samplers.size() ? immutable_samplers.size() : wrap.descriptorCount;
                    wrap.pImmutableSamplers = immutable_samplers.size() ? immutable_samplers.data() : nullptr;
                }

            // Scenarios

                void setWholeStruct(uint32_t _binding, VkDescriptorType _descriptor_type, uint32_t _descriptor_count,
                                    VkShaderStageFlags _stage_flags, const std::vector<VkSampler> &_immutable_samplers = {}) {
                    wrap.binding = _binding;
                    wrap.descriptorType = _descriptor_type;
                    wrap.descriptorCount = _descriptor_count;
                    wrap.stageFlags = _stage_flags;
                    immutable_samplers = _immutable_samplers;
                    updateWrap();
                }
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

                StructWrap() { clear(); }

                void clear() {
                    wrap = VkApplicationInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
                    app_name    = "SimpleVKFW_MainApp";
                    engine_name = "SimpleVKFW";
                    app_version = {0u, 0u, 1u};
                    engine_version = {0u, 1u, 0u};
                    setAvailableVersionAPI();
                }

                void updateWrap() {
                    wrap.pApplicationName = app_name.c_str();
                    wrap.applicationVersion = VulkanAPIVersion::join(app_version);
                    wrap.pEngineName = engine_name.c_str();
                    wrap.engineVersion = VulkanAPIVersion::join(engine_version);
                    wrap.apiVersion = VulkanAPIVersion::join(api_version["xyz"], api_version.w);
                }

                // Scenarios

                void setAvailableVersionAPI() {
                    api_version["xyz"] = VulkanAPIVersion::split(VulkanAPIVersion::available(), &api_version.w);
                }

                void setWholeStruct(const std::string &   _app_name = "SimpleVKFW_MainApp", const vec3u &   _app_version = {0u, 0u, 1u},
                                    const std::string &_engine_name = "SimpleVKFW",         const vec3u &_engine_version = {0u, 1u, 0u}, const vec4u &_api_version = {0u}) {
                    api_version = _api_version;
                    if ((_api_version == 0u).all())  setAvailableVersionAPI();

                    engine_version = _engine_version;
                    app_version = _app_version;
                    engine_name = _engine_name;
                    app_name = _app_name;
                }
            }; // StructWrap<VkApplicationInfo> END

            template <>
            struct StructWrap<VkInstanceCreateInfo> {
            private:
                VkInstanceCreateInfo wrap{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkInstanceCreateInfo)

                Util::StringVec layers, extensions;

                StructWrap<VkApplicationInfo> app_info_wrap;

                StructWrap() {}

                void clear() {
                    wrap = VkInstanceCreateInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};

                    layers.clear();
                    extensions.clear();

                    app_info_wrap.clear();
                }

                void updateWrap() {
                    wrap.pApplicationInfo = &app_info_wrap.get();
                    wrap.enabledLayerCount = layers.count();
                    wrap.ppEnabledLayerNames = layers.getEntries();
                    wrap.enabledExtensionCount = extensions.count();
                    wrap.ppEnabledExtensionNames = extensions.getEntries();
                }

            // Scenarios

                void addLayers(const Util::StringVec &_layers) {
                    layers.addEntries(_layers.list);
                }

                void addExtensions(const Util::StringVec &_extensions) {
                    extensions.addEntries(_extensions.list);
                }

                void addValidationLayer() {
                    if (!Util::allInstanceLayersAvailable({{ "VK_LAYER_KHRONOS_validation" }}))
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::StructWrap<VkInstanceCreateInfo> :: addValidationLayer","Validation layer unavailable"));

                    addLayers({{ "VK_LAYER_KHRONOS_validation" }});
                }

                void addExtGLFW() { // Vulkan + GLFW is the main stack for now
                    uint32_t _count = 0u;
                    const char **_ext = glfwGetRequiredInstanceExtensions(&_count);
                    addExtensions({_ext, _count});
                }

                void addExtDebugUtils() {
                    if (!Util::allInstanceExtAvailable({{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME }}))
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::StructWrap<VkInstanceCreateInfo> :: addExtDebugUtils","Debug message callback extension is unavailable"));

                    addExtensions({{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME }});
                }
            }; // StructWrap<VkInstanceCreateInfo> END

            template<>
            struct StructWrap<VkDebugUtilsMessengerCreateInfoEXT> {
            private:
                VkDebugUtilsMessengerCreateInfoEXT wrap{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
                VKFW_STRUCT_WRAP_ACCESS(VkDebugUtilsMessengerCreateInfoEXT)

                StructWrap() { clear(); }

                void clear() {
                    wrap = VkDebugUtilsMessengerCreateInfoEXT{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
                    wrap.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
                                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                    wrap.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                    wrap.pfnUserCallback = Util::vkfwDebugMessageCallback;
                    wrap.pUserData       = nullptr;
                }

                void updateWrap() {}

            // Scenarios

                // If 'user_cb' is nullptr, the default VKFW callback is used
                void setWholeStruct(VkDebugUtilsMessageSeverityFlagsEXT _msg_severity,
                                    VkDebugUtilsMessageTypeFlagsEXT _msg_type,
                                    PFN_vkDebugUtilsMessengerCallbackEXT _user_cb = nullptr,
                                    void* _user_data = nullptr) {
                    wrap.messageSeverity = _msg_severity;
                    wrap.messageType     = _msg_type;
                    wrap.pfnUserCallback = _user_cb != nullptr ? _user_cb : Util::vkfwDebugMessageCallback;
                    wrap.pUserData       = _user_data;
                }
            }; // StructWrap<VkDebugUtilsMessengerCreateInfoEXT> END

            template <>
            struct StructWrap<VkDeviceQueueCreateInfo> {
            private:
                VkDeviceQueueCreateInfo wrap{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkDeviceQueueCreateInfo)

                VkfwQueueFamily queue_family = UINT32_MAX;
                std::vector<float> priorities;

                StructWrap() {}

                void clear() {
                    wrap = VkDeviceQueueCreateInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
                    priorities.clear();
                }

                void updateWrap() {
                    wrap.queueCount       = priorities.size();
                    wrap.pQueuePriorities = priorities.data();
                    wrap.queueFamilyIndex = queue_family;
                }

            // Scenarios

                void setQueueCount(uint32_t _queue_count, const std::vector<float> &_priorities = {1.0f}) {
                    priorities = _priorities;
                    priorities.resize(_queue_count, _priorities.empty() ? 1.0f : _priorities.back());
                }
            }; // StructWrap<VkDeviceQueueCreateInfo> END

            template <>
            struct StructWrap<VkDeviceCreateInfo> {
            private:
                VkDeviceCreateInfo wrap{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkDeviceCreateInfo)

                VkPhysicalDeviceFeatures req_features{};
                std::vector<VkDeviceQueueCreateInfo> queue_info;
                Util::StringVec layers, extensions;

                StructWrap() {}

                void clear() {
                    wrap = VkDeviceCreateInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
                    req_features = VkPhysicalDeviceFeatures{};
                    queue_info.clear();
                    layers.clear();
                    extensions.clear();
                }

                void updateWrap() {
                    wrap.enabledLayerCount   = layers.count();
                    wrap.ppEnabledLayerNames = layers.getEntries();
                    wrap.enabledExtensionCount   = extensions.count();
                    wrap.ppEnabledExtensionNames = extensions.getEntries();
                    wrap.pQueueCreateInfos    = queue_info.data();
                    wrap.queueCreateInfoCount = queue_info.size();
                    wrap.pEnabledFeatures = &req_features;

                    // Note: with VkPhysicalDeviceFeatures2 in the pNext chain 'pEnabledFeatures' must be null,
                    //       so 'req_features' is copied into the chained structure instead
                    for (VkBaseOutStructure *p = (VkBaseOutStructure*)wrap.pNext; p != nullptr; p = p->pNext) {
                        if (p->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2) {
                            ((VkPhysicalDeviceFeatures2*)p)->features = req_features;
                            wrap.pEnabledFeatures = nullptr;
                            break;
                        }
                    }
                }

            // Scenarios

                void addLayers(const Util::StringVec &_layers) {
                    layers.addEntries(_layers.list);
                }

                void addExtensions(const Util::StringVec &_extensions) {
                    extensions.addEntries(_extensions.list);
                }

                void addValidationLayer(VkPhysicalDevice _phys_device = VK_NULL_HANDLE) {
                    if (_phys_device != VK_NULL_HANDLE && !Util::allDeviceLayersAvailable(_phys_device, {{ "VK_LAYER_KHRONOS_validation" }}))
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::StructWrap<VkDeviceCreateInfo> :: addValidationLayer", "Validation layer unavailable"));

                    addLayers({{ "VK_LAYER_KHRONOS_validation" }});
                }
            }; // StructWrap<VkDeviceCreateInfo> END

            template <>
            struct StructWrap<VkSwapchainCreateInfoKHR> {
            private:
                VkSwapchainCreateInfoKHR wrap{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
                VKFW_STRUCT_WRAP_ACCESS(VkSwapchainCreateInfoKHR)

                VkSurfaceCapabilitiesKHR capabilities;
                std::vector<uint32_t>  queue_families;

                StructWrap() {}
                // StructWrap(const StructWrap &_builder) = default;

                void clear() {
                    wrap = VkSwapchainCreateInfoKHR{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
                    // wrap.imageArrayLayers = 1;
                    // wrap.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
                    // wrap.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                    // wrap.presentMode = VK_PRESENT_MODE_FIFO_KHR;
                    capabilities = VkSurfaceCapabilitiesKHR{};
                    queue_families.clear();
                }

                // Note: concurrent sharing mode requires at least 2 unique queue families
                void updateWrap() {
                    const bool __concurrent = queue_families.size() > 1u;
                    wrap.imageSharingMode = __concurrent ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
                    wrap.queueFamilyIndexCount = __concurrent ? queue_families.size() : 0u;
                    wrap.pQueueFamilyIndices   = __concurrent ? queue_families.data() : nullptr;
                }

            // Scenarios

                void setSurfaceCapabilities(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface) {
                    VkResult __res_call = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_phys_device, _surface, &capabilities);

                    if (__res_call != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::StructWrap<VkSwapchainCreateInfoKHR> :: setSurfaceCapabilities", "failed, error: ") + std::to_string(__res_call));
                    wrap.preTransform = capabilities.currentTransform;
                }

                void setCurrentExtent() {
                    wrap.imageExtent = capabilities.currentExtent;
                }

                // Returns input bits after applying capabilities.supportedUsageFlags as a mask.
                // If not equal to input, then some usage flags are not supported
                // and the parameter is not set.
                VkImageUsageFlags checkSetImageUsage(VkImageUsageFlags _usage) {
                    VkImageUsageFlags __res = _usage & capabilities.supportedUsageFlags;

                    if (__res == _usage)
                        wrap.imageUsage = _usage;
                    else
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::Struct::StructWrap<VkSwapchainCreateInfoKHR> :: checkSetImageUsage",
                                                                "some usage flags are not supported\n"));
                    return __res;
                }

                void checkSetImageCount(uint32_t _count) {
                    wrap.minImageCount = Math::clampCL(_count,
                                                        capabilities.minImageCount,
                                                        capabilities.maxImageCount ? capabilities.maxImageCount : UINT32_MAX); // (== 0) means no max

                    if (wrap.minImageCount != _count)
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::Struct::StructWrap<VkSwapchainCreateInfoKHR> :: checkSetImageCount",
                                        "image count (%d) was clamped to %d to fit the interval [%d, %d]\n"),
                                        _count, wrap.minImageCount, capabilities.minImageCount, capabilities.maxImageCount);
                }

                bool checkSetExtent(vec2u _extent) {
                    wrap.imageExtent = { _extent.x, _extent.y };

                    // Note: (UINT32_MAX, UINT32_MAX) current extent means that the extent is determined by the swapchain, so it must be clamped
                    if (capabilities.currentExtent.width  == UINT32_MAX &&
                        capabilities.currentExtent.height == UINT32_MAX) {
                        wrap.imageExtent = { Math::clampCL(_extent.x, capabilities.minImageExtent.width,  capabilities.maxImageExtent.width ),
                                             Math::clampCL(_extent.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height) };
                    }
                    return wrap.imageExtent.width  == _extent.x &&
                           wrap.imageExtent.height == _extent.y;
                }

                void chooseCurrentExtent(const Window &_window) {
                    if (capabilities.currentExtent.width != UINT32_MAX) {
                        wrap.imageExtent = capabilities.currentExtent;
                        return;
                    }
                    else {
                        vec2i __fbsize{_window.getFramebufferSize()};
                        wrap.imageExtent = { Math::clampCL((uint32_t)__fbsize.x, capabilities.minImageExtent.width,  capabilities.maxImageExtent.width ),
                                             Math::clampCL((uint32_t)__fbsize.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height) };
                    }
                }


                void pickFormat(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface, const Util::SurfaceFormatTestFunc _test_func) {
                                std::vector<VkSurfaceFormatKHR> __formats{Func::GetPhysicalDeviceSurfaceFormatsKHR(_phys_device, _surface)};
                    for (VkSurfaceFormatKHR __format : __formats) {
                        if (_test_func(__format)) {
                            wrap.imageFormat     = __format.format;
                            wrap.imageColorSpace = __format.colorSpace;
                            printf(SVKFW_WRAPINFO("VKFW::Struct::StructWrap<VkSwapchainCreateInfoKHR> :: pickFormat (1)", "Format: %d, color space: %d\n"), uint32_t(__format.format), uint32_t(__format.colorSpace));
                            return;
                        }
                    }
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW::Struct::StructWrap<VkSwapchainCreateInfoKHR> :: pickFormat (1)", "could not choose format"));
                }

                void pickFormat(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface, const Util::SurfaceFormatRankFunc _rank_func) {
                    std::vector<VkSurfaceFormatKHR> __formats{Func::GetPhysicalDeviceSurfaceFormatsKHR(_phys_device, _surface)};

                    int __max_rank = INT_MIN;
                    uint32_t __curr_ind = UINT32_MAX;
                    for (uint32_t i = 0u; i < __formats.size(); ++i) {
                        int __rank = _rank_func(__formats[i]);
                        if (__rank > __max_rank) {
                            __max_rank = __rank;
                            __curr_ind = i;
                        }
                    }

                    if (__curr_ind >= __formats.size())
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::Struct::StructWrap<VkSwapchainCreateInfoKHR> :: pickFormat (2)", "could not choose format"));
                    printf(SVKFW_WRAPINFO("VKFW::Struct::StructWrap<VkSwapchainCreateInfoKHR> :: pickFormat (2)", "Format: %d, color space: %d\n"),
                            uint32_t(__formats[__curr_ind].format), uint32_t(__formats[__curr_ind].colorSpace));

                    wrap.imageFormat     = __formats[__curr_ind].format;
                    wrap.imageColorSpace = __formats[__curr_ind].colorSpace;
                }

                bool checkSetPresentationMode(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface, VkPresentModeKHR _pres_mode) {
                    std::vector<VkPresentModeKHR> __pres_modes{Func::GetPhysicalDeviceSurfacePresentModesKHR(_phys_device, _surface)};

                    for (VkPresentModeKHR __mode : __pres_modes)
                        if (__mode == _pres_mode) {
                            wrap.presentMode = _pres_mode;
                            printf(SVKFW_WRAPINFO("VKFW::Struct::StructWrap<VkSwapchainCreateInfoKHR> :: checkSetPresentationMode", "mode %d\n"), uint32_t(__mode));
                            return true;
                        }

                    fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::Struct::StructWrap<VkSwapchainCreateInfoKHR> :: checkSetPresentationMode",
                                                        "presentation mode %d not supported\n"), uint32_t(_pres_mode));
                    return false;
                }
            }; // StructWrap<VkSwapchainCreateInfoKHR> END

            template <>
            struct StructWrap<VkImageCreateInfo> {
            private:
                VkImageCreateInfo wrap{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkImageCreateInfo)

                std::vector<uint32_t> queue_families;

                StructWrap() {}
                // StructWrap(const StructWrap &_builder) = default;

                void clear() {
                    wrap = VkImageCreateInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
                    queue_families.clear();
                }

                void updateWrap() {
                    wrap.queueFamilyIndexCount = queue_families.size();
                    wrap.pQueueFamilyIndices = queue_families.data();
                }
            }; // StructWrap<VkImageCreateInfo> END

            template <>
            struct StructWrap<VkImageViewCreateInfo> {
            private:
                VkImageViewCreateInfo wrap{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkImageViewCreateInfo)

                StructWrap() {}

                void clear() {
                    wrap = VkImageViewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
                }

                void updateWrap() {}

            // Scenarios

                void setRange(VkImageAspectFlags _aspect_mask, uint32_t _base_mip_level,
                              uint32_t _level_count, uint32_t _base_array_layer, uint32_t _array_level_count) {
                    wrap.subresourceRange.aspectMask = _aspect_mask;
                    wrap.subresourceRange.baseMipLevel = _base_mip_level;
                    wrap.subresourceRange.levelCount = _level_count;
                    wrap.subresourceRange.baseArrayLayer = _base_array_layer;
                    wrap.subresourceRange.layerCount = _array_level_count;
                }

                void setComponents(VkComponentSwizzle _r = VK_COMPONENT_SWIZZLE_IDENTITY, VkComponentSwizzle _g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                   VkComponentSwizzle _b = VK_COMPONENT_SWIZZLE_IDENTITY, VkComponentSwizzle _a = VK_COMPONENT_SWIZZLE_IDENTITY) {
                    wrap.components.r = _r;
                    wrap.components.g = _g;
                    wrap.components.b = _b;
                    wrap.components.a = _a;
                }
            }; // StructWrap<VkImageViewCreateInfo> END

            template <>
            struct StructWrap<VkBufferCreateInfo> {
            private:
                VkBufferCreateInfo wrap{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkBufferCreateInfo)

                std::vector<VkfwQueueFamily> queue_families;

                StructWrap() {}
                // StructWrap(const StructWrap &_builder) = default;

                void clear() {
                    wrap = VkBufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
                    queue_families.clear();
                }

                void updateWrap() {
                    wrap.queueFamilyIndexCount = queue_families.size();
                    wrap.pQueueFamilyIndices   = queue_families.data();
                }

            // Scenarios

                void setWholeStruct(const std::vector<VkfwQueueFamily> &_queue_families, VkSharingMode _sharing_mode,
                                    VkBufferUsageFlags _usage_flags, VkDeviceSize _size_bytes) {
                    queue_families = _queue_families;
                    wrap.sharingMode = _sharing_mode;
                    wrap.size  =  _size_bytes;
                    wrap.usage = _usage_flags;
                    updateWrap();
                }
            }; // StructWrap<VkBufferCreateInfo> END

            template <>
            struct StructWrap<VkBufferViewCreateInfo> {
            private:
                VkBufferViewCreateInfo wrap{VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkBufferViewCreateInfo)

                StructWrap() {}

                void clear() {
                    wrap = VkBufferViewCreateInfo{VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO};
                }

                void updateWrap() {}

            // Scenarios

                void setWholeStruct(VkBuffer _buffer, VkFormat _format, VkDeviceSize _offset, VkDeviceSize _range) {
                    wrap.buffer = _buffer;
                    wrap.format = _format;
                    wrap.offset = _offset;
                    wrap. range = _range ;
                }
            }; // StructWrap<VkBufferViewCreateInfo> END

            template <>
            struct StructWrap<VkMemoryAllocateInfo> {
            private:
                VkMemoryAllocateInfo wrap{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkMemoryAllocateInfo)

                StructWrap() {}

                void clear() {
                    wrap = VkMemoryAllocateInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
                }

                void updateWrap() {}

            // Scenarios

                void setForBuffer(VkPhysicalDevice _phys_device, VkDevice _device, VkBuffer _buffer,
                                  VkMemoryPropertyFlags _req_memory_properties) {
                    VkMemoryRequirements __mem_requirements = Func::GetBufferMemoryRequirements(_device, _buffer);

                    wrap.allocationSize = __mem_requirements.size;
                    wrap.memoryTypeIndex = Util::checkPhysicalDeviceMemoryProperties(_phys_device, __mem_requirements.memoryTypeBits, _req_memory_properties);

                    if (wrap.memoryTypeIndex == UINT32_MAX)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::Struct::StructWrap<VkMemoryAllocateInfo> :: setForBuffer", "Could not find suitable memory type"));
                }

                void setForImage(VkPhysicalDevice _phys_device, VkDevice _device, VkImage _image,
                                  VkMemoryPropertyFlags _req_memory_properties) {
                    VkMemoryRequirements __mem_requirements = Func::GetImageMemoryRequirements(_device, _image);

                    wrap.allocationSize = __mem_requirements.size;
                    wrap.memoryTypeIndex = Util::checkPhysicalDeviceMemoryProperties(_phys_device, __mem_requirements.memoryTypeBits, _req_memory_properties);

                    if (wrap.memoryTypeIndex == UINT32_MAX)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::Struct::StructWrap<VkMemoryAllocateInfo> :: setForImage", "Could not find suitable memory type"));
                }
            }; // StructWrap<VkMemoryAllocateInfo> END

            template <>
            struct StructWrap<VkShaderModuleCreateInfo> {
            private:
                VkShaderModuleCreateInfo wrap{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkShaderModuleCreateInfo)
                std::vector<char> buffer;

                StructWrap() {}

                void clear() {
                    wrap = VkShaderModuleCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
                    buffer.clear();
                }

                void updateWrap() {
                    wrap.codeSize = buffer.size();
                    wrap.pCode = reinterpret_cast<const uint32_t *>(buffer.data());
                }

            // Scenarios

                void fromFile(const char *_fpath) {
                    std::ifstream __shader_file{_fpath, std::ios::ate | std::ios::binary};

                    if (!__shader_file.is_open()) {
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::Struct::StructWrap<VkShaderModuleCreateInfo> :: fromFile",
                                                                "failed to open file for reading"));
                    }

                    size_t __shader_file_size = (size_t) __shader_file.tellg();
                    buffer.resize(__shader_file_size);

                    __shader_file.seekg(0);
                    __shader_file.read(buffer.data(), __shader_file_size);
                    __shader_file.close();
                }

                void fromString(const char *_sh_str, size_t _length = 0u) {
                    if (_sh_str == nullptr)
                        throw std::invalid_argument(SVKFW_WRAPERR("VKFW::Struct::StructWrap<VkShaderModuleCreateInfo> :: fromString (1)", "no string provided"));
                    if (_length == 0u) {
                        while (_sh_str[_length] != '\0') ++_length;
                        if (_length == 0u)
                            throw std::invalid_argument(SVKFW_WRAPERR("VKFW::Struct::StructWrap<VkShaderModuleCreateInfo> :: fromString (1)", "string is empty"));
                    }

                    buffer.clear();
                    buffer.insert(buffer.end(), _sh_str, _sh_str + _length);
                }

                void fromString(const std::string &_str_buffer) {
                    if (_str_buffer.length() == 0u)
                        throw std::invalid_argument(SVKFW_WRAPERR("VKFW::Struct::StructWrap<VkShaderModuleCreateInfo> :: fromString (2)", "string is empty"));

                    buffer.clear();
                    buffer.insert(buffer.end(), _str_buffer.begin(), _str_buffer.end());
                }
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

                StructWrap() {}
                // StructWrap(const StructWrap &_create_info) = default;

                void clear() {
                    attachments.clear();
                    subpasses.clear();
                    dependencies.clear();
                    attachments_wrap.clear();
                    subpasses_wrap.clear();
                    dependencies_wrap.clear();
                }

                void updateWrap() {
                    attachments.resize(attachments_wrap.size());
                    for (uint32_t i = 0u; i < attachments.size(); ++i) {
                        attachments[i] = attachments_wrap[i].get();
                    }

                    subpasses.resize(subpasses_wrap.size());
                    for (uint32_t i = 0u; i < subpasses.size(); ++i) {
                        subpasses[i] = subpasses_wrap[i].get();
                    }

                    dependencies.resize(dependencies_wrap.size());
                    for (uint32_t i = 0u; i < dependencies.size(); ++i) {
                        dependencies[i] = dependencies_wrap[i].get();
                    }

                    wrap.attachmentCount =  attachments.size();
                    wrap.pAttachments    =  attachments.data();
                    wrap.subpassCount    =    subpasses.size();
                    wrap.pSubpasses      =    subpasses.data();
                    wrap.dependencyCount = dependencies.size();
                    wrap.pDependencies   = dependencies.data();
                }

            // Scenarios

                ContextIndex addAttachment()         {  attachments_wrap.emplace_back(); return  attachments_wrap.size()-1; }
                ContextIndex addSubpassDescription() {    subpasses_wrap.emplace_back(); return    subpasses_wrap.size()-1; }
                ContextIndex addSubpassDependency()  { dependencies_wrap.emplace_back(); return dependencies_wrap.size()-1; }

                ContextEntity<StructWrap<VkAttachmentDescription>> shortcutAttachment        (ContextIndex _sct_ci) { return {  attachments_wrap.data() + _sct_ci, _sct_ci}; }
                ContextEntity<StructWrap<   VkSubpassDescription>> shortcutSubpassDescription(ContextIndex _sct_ci) { return {    subpasses_wrap.data() + _sct_ci, _sct_ci}; }
                ContextEntity<StructWrap<    VkSubpassDependency>> shortcutSubpassDependency (ContextIndex _sct_ci) { return { dependencies_wrap.data() + _sct_ci, _sct_ci}; }
            }; // StructWrap<VkRenderPassCreateInfo> END

            template <>
            struct StructWrap<VkFramebufferCreateInfo> {
            private:
                VkFramebufferCreateInfo wrap{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkFramebufferCreateInfo)

                std::vector<VkImageView> attachments;

                StructWrap() {}

                void clear() {
                    wrap = VkFramebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
                    attachments.clear();
                }

                void updateWrap() {
                    wrap.attachmentCount = attachments.size();
                    wrap.pAttachments    = attachments.data();
                }

            // Scenarios

                void setBufferSize(VkExtent2D _buffer_size) {
                    wrap. width = _buffer_size. width;
                    wrap.height = _buffer_size.height;
                }

                void setBufferSize(vec2u _buffer_size) {
                    wrap. width = _buffer_size.x;
                    wrap.height = _buffer_size.y;
                }

            // Scenarios

                void setBufferSizeAndLayers(VkExtent2D _buffer_size, uint32_t _layers) {
                    wrap. width = _buffer_size. width;
                    wrap.height = _buffer_size.height;
                    wrap.layers = _layers;
                }

                void setBufferSizeAndLayers(vec3u _buf_size_layers) {
                    wrap. width = _buf_size_layers.x;
                    wrap.height = _buf_size_layers.y;
                    wrap.layers = _buf_size_layers.z;
                }
            }; // StructWrap<VkFramebufferCreateInfo> END

            template <>
            struct StructWrap<VkSemaphoreCreateInfo> {
            private:
                VkSemaphoreCreateInfo wrap{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkSemaphoreCreateInfo)

                StructWrap() {}

                void clear() {
                    wrap = VkSemaphoreCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
                }

                void updateWrap() {}
            }; // StructWrap<VkSemaphoreCreateInfo>

            template <>
            struct StructWrap<VkFenceCreateInfo> {
            private:
                VkFenceCreateInfo wrap{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkFenceCreateInfo)

                StructWrap() {}

                void clear() {
                    wrap = VkFenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
                }

                void updateWrap() {}

            // Scenarios

                void setWholeStruct(bool _is_signaled) {
                    wrap.flags = _is_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : VkFenceCreateFlags(0u);
                    updateWrap();
                }
            }; // StructWrap<VkFenceCreateInfo>

            template <>
            struct StructWrap<VkEventCreateInfo> {
            private:
                VkEventCreateInfo wrap{VK_STRUCTURE_TYPE_EVENT_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkEventCreateInfo)

                StructWrap() {}

                void clear() {
                    wrap = VkEventCreateInfo{VK_STRUCTURE_TYPE_EVENT_CREATE_INFO};
                }

                void updateWrap() {}

            // Scenarios

                void setWholeStruct(bool _is_device_only) {
                    wrap.flags = _is_device_only ? VK_EVENT_CREATE_DEVICE_ONLY_BIT : VkEventCreateFlags(0u);
                    updateWrap();
                }
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

                StructWrap() {}

                void clear() {
                    wrap = VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                    descriptor_count = 0u;

                    buffer_infos.clear();
                    image_infos.clear();
                    texel_buf_views.clear();
                }

                // Note: Vulkan reads only one info array, chosen by 'descriptorType', and ignores the other two.
                //       Types that take their data through 'pNext' (e.g. inline uniform blocks, acceleration structures)
                //       use no info array, so 'descriptor_count' must be set for them.
                void updateWrap() {
                    wrap.pImageInfo       = nullptr;
                    wrap.pBufferInfo      = nullptr;
                    wrap.pTexelBufferView = nullptr;

                    bool     __uses_infos  = true;
                    uint32_t __infos_count = 0u;
                    switch (wrap.descriptorType) {
                        case VK_DESCRIPTOR_TYPE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                            __infos_count   = image_infos.size();
                            wrap.pImageInfo = image_infos.empty() ? nullptr : image_infos.data();
                            break;
                        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                            __infos_count         = texel_buf_views.size();
                            wrap.pTexelBufferView = texel_buf_views.empty() ? nullptr : texel_buf_views.data();
                            break;
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                            __infos_count    = buffer_infos.size();
                            wrap.pBufferInfo = buffer_infos.empty() ? nullptr : buffer_infos.data();
                            break;
                        default:
                            __uses_infos = false;
                    }

                    wrap.descriptorCount = descriptor_count ? descriptor_count : __infos_count;

                    if (wrap.descriptorCount == 0u)
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::Struct::StructWrap<VkWriteDescriptorSet> :: updateWrap", "descriptorCount is 0\n"));
                    else if (__uses_infos && wrap.descriptorCount > __infos_count)
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::Struct::StructWrap<VkWriteDescriptorSet> :: updateWrap",
                                                          "descriptorCount (%u) is larger than the info array (%u)\n"), wrap.descriptorCount, __infos_count);
                }

            // Scenarios

                // If 'descriptor_count' is 0, 'descriptorCount' is the size of the info array that 'descriptorType' uses
                void setScalars(VkDescriptorSet _dst_descr_set, VkDescriptorType _descr_type, uint32_t _dst_binding,
                                uint32_t _descriptor_count = 0u, uint32_t _dst_arr_elem = 0u) {
                    descriptor_count     = _descriptor_count;
                    wrap.dstSet          = _dst_descr_set;
                    wrap.descriptorType  =    _descr_type;
                    wrap.dstBinding      =   _dst_binding;
                    wrap.dstArrayElement =  _dst_arr_elem;
                }

                void addBufferInfo(VkBuffer _buffer, VkDeviceSize _offset = 0ul, VkDeviceSize _range = VK_WHOLE_SIZE) {
                    buffer_infos.push_back(VkDescriptorBufferInfo{ _buffer, _offset, _range });
                }

                void addImageInfo(VkSampler _sampler, VkImageView _image_view, VkImageLayout _image_layout) {
                    image_infos.push_back(VkDescriptorImageInfo{ _sampler, _image_view, _image_layout });
                }

                void addTexelBufferView(VkBufferView _buffer_view) {
                    texel_buf_views.push_back(_buffer_view);
                }
            }; // StructWrap<VkWriteDescriptorSet> END

            template <>
            struct StructWrap<VkCopyDescriptorSet> {
            private:
                VkCopyDescriptorSet wrap{VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET};
                VKFW_STRUCT_WRAP_ACCESS(VkCopyDescriptorSet)

                StructWrap() {}

                void clear() {
                    wrap = VkCopyDescriptorSet{VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET};
                }

                void updateWrap() {}

            // Scenarios

                void setSrc(VkDescriptorSet _src_set, uint32_t _src_binding, uint32_t _src_arr_elem) {
                    wrap.srcSet          = _src_set;
                    wrap.srcBinding      = _src_binding;
                    wrap.srcArrayElement = _src_arr_elem;
                }

                void setDst(VkDescriptorSet _dst_set, uint32_t _dst_binding, uint32_t _dst_arr_elem) {
                    wrap.dstSet          = _dst_set;
                    wrap.dstBinding      = _dst_binding;
                    wrap.dstArrayElement = _dst_arr_elem;
                    
                }
            }; // StructWrap<VkCopyDescriptorSet> END

            template <>
            struct StructWrap<VkDescriptorSetLayoutCreateInfo> {
            private:
                VkDescriptorSetLayoutCreateInfo wrap{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkDescriptorSetLayoutCreateInfo)

                std::vector<Struct::StructWrap<VkDescriptorSetLayoutBinding>> bindings_wrap;

                std::vector<VkDescriptorSetLayoutBinding> bindings;

                StructWrap() {}
                // StructWrap(const Builder &_builder) = default;

                void clear() {
                    wrap = VkDescriptorSetLayoutCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
                    bindings.clear();
                }

                ContextIndex addBinding() { bindings_wrap.emplace_back(); return bindings_wrap.size()-1; }
                ContextEntity<Struct::StructWrap<VkDescriptorSetLayoutBinding>> shortcutBinding(ContextIndex _sct_ci) { return {bindings_wrap.data() + _sct_ci, _sct_ci}; }

                void updateWrap() {
                    bindings.resize(bindings_wrap.size());
                    for (uint32_t i = 0u; i < bindings.size(); ++i) {
                        bindings[i] = bindings_wrap[i].get();
                    }

                    wrap.bindingCount = bindings.size();
                    wrap.pBindings    = bindings.data();
                }
            }; // StructWrap<VkDescriptorSetLayoutCreateInfo> END

            template <>
            struct StructWrap<VkDescriptorPoolCreateInfo> {
            private:
                VkDescriptorPoolCreateInfo wrap{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkDescriptorPoolCreateInfo)

                std::vector<VkDescriptorPoolSize> pool_sizes;

                StructWrap() {}

                void clear() {
                    wrap = VkDescriptorPoolCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
                    pool_sizes.clear();
                }

                void updateWrap() {
                    wrap.poolSizeCount = pool_sizes.size();
                    wrap.pPoolSizes    = pool_sizes.data();
                }

            // Scenarios

                void addPoolSize(VkDescriptorType _descr_type, uint32_t _descr_count) {
                    pool_sizes.push_back(VkDescriptorPoolSize{_descr_type, _descr_count});
                }

                void setWholeStruct(uint32_t _max_sets, const std::vector<VkDescriptorPoolSize> &_pool_sizes = {}) {
                    wrap.maxSets = _max_sets;
                    pool_sizes = _pool_sizes;
                    updateWrap();
                }
            }; // StructWrap<VkDescriptorPoolCreateInfo> END

            template <>
            struct StructWrap<VkDescriptorSetAllocateInfo> {
            private:
                VkDescriptorSetAllocateInfo wrap{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkDescriptorSetAllocateInfo)

                std::vector<VkDescriptorSetLayout> set_layouts;

                StructWrap() {}

                void clear() {
                    wrap = VkDescriptorSetAllocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
                    set_layouts.clear();
                }

                void updateWrap() {
                    wrap.descriptorSetCount = set_layouts.size();
                    wrap.pSetLayouts        = set_layouts.data();
                }

            // Scenarios

                void setWholeStruct(VkDescriptorPool _descr_pool, const std::vector<VkDescriptorSetLayout> &_set_layouts) {
                    wrap.descriptorPool = _descr_pool;
                    set_layouts = _set_layouts;
                    updateWrap();
                }
            }; // StructWrap<VkDescriptorSetAllocateInfo> END


//  == === ==== >   Vulkan: CreateInfo (Pipeline-related)

            template <>
            struct StructWrap<VkPipelineLayoutCreateInfo> {
            private:
                VkPipelineLayoutCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineLayoutCreateInfo)

                std::vector<VkPushConstantRange> constants;
                std::vector<VkDescriptorSetLayout> layouts;

                StructWrap() {}

                void clear() {
                    wrap = VkPipelineLayoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
                    constants.clear();
                    layouts  .clear();
                }

                void updateWrap() {
                    wrap.setLayoutCount = layouts.size();
                    wrap.pSetLayouts    = layouts.data();
                    wrap.pushConstantRangeCount = constants.size();
                    wrap.pPushConstantRanges    = constants.data();
                }
            }; // StructWrap<VkPipelineLayoutCreateInfo> END

            template <>
            struct StructWrap<VkPipelineCacheCreateInfo> {
            private:
                VkPipelineCacheCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineCacheCreateInfo)

                std::vector<unsigned char> cache_data;

                StructWrap() {}

                void clear() {
                    wrap = VkPipelineCacheCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
                    cache_data.clear();
                }


                void updateWrap() {
                    wrap.initialDataSize = cache_data.size();
                    wrap.pInitialData    = cache_data.data();
                }

            // Scenarios

                // Get data from previously created cache object
                void getCacheData(VkDevice _device, VkPipelineCache _pipeline_cache) {
                    cache_data = Func::GetPipelineCacheData(_device, _pipeline_cache);
                }
            }; // StructWrap<VkPipelineCacheCreateInfo> END

            template <>
            struct StructWrap<VkPipelineLibraryCreateInfoKHR> {
            private:
                VkPipelineLibraryCreateInfoKHR wrap{VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineLibraryCreateInfoKHR)

                std::vector<VkPipeline> libraries;

                StructWrap() {}

                void clear() {
                    wrap = VkPipelineLibraryCreateInfoKHR{VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR};
                    libraries.clear();
                }

                void updateWrap() {
                    wrap.libraryCount = libraries.size();
                    wrap.pLibraries   = libraries.data();
                }
            }; // StructWrap<VkPipelineLibraryCreateInfoKHR> END

            template <>
            struct StructWrap<VkRayTracingShaderGroupCreateInfoKHR> {
            private:
                VkRayTracingShaderGroupCreateInfoKHR wrap{VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
                VKFW_STRUCT_WRAP_ACCESS(VkRayTracingShaderGroupCreateInfoKHR)

                // Note: data is owned by vector, so the structure can be safely copied (previously raw 'new[]' pointer was double-deleted on copies)
                std::vector<unsigned char> capture_replay_handle;

                StructWrap() {}

                void clear() {
                    capture_replay_handle.clear();
                    wrap = VkRayTracingShaderGroupCreateInfoKHR{VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
                }

                void updateWrap() {
                    wrap.pShaderGroupCaptureReplayHandle = capture_replay_handle.empty() ? nullptr : capture_replay_handle.data();
                }

            // Scenarios

                void getData(VkPhysicalDevice _phys_device, VkDevice _device, VkPipeline _pipeline, uint32_t _first_group, uint32_t _group_count) {
                    capture_replay_handle = Func::GetRayTracingCaptureReplayShaderGroupHandlesKHR(_phys_device, _device, _pipeline, _first_group, _group_count);
                    updateWrap();
                }
            }; // StructWrap<VkRayTracingShaderGroupCreateInfoKHR> END

            template <>
            struct StructWrap<VkRayTracingPipelineInterfaceCreateInfoKHR> {
            private:
                VkRayTracingPipelineInterfaceCreateInfoKHR wrap{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR};
                VKFW_STRUCT_WRAP_ACCESS(VkRayTracingPipelineInterfaceCreateInfoKHR)
                // Note: from Gemini, minimal supported 'maxPipelineRayHitAttributeSize' value is 32 bytes
                StructWrap() {}

                void clear() {
                    wrap = VkRayTracingPipelineInterfaceCreateInfoKHR{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR};
                }

                void updateWrap() {}
            }; // StructWrap<VkRayTracingPipelineInterfaceCreateInfoKHR> END


//  == === ==== >   Vulkan: CreateInfo (Pipeline States)

            template <>
            struct StructWrap<VkPipelineShaderStageCreateInfo> {
            private:
                VkPipelineShaderStageCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineShaderStageCreateInfo)

                StructWrap<VkSpecializationInfo> spec_info_wrap;

                std::string name;

                StructWrap() {}

                void clear() {
                    wrap = VkPipelineShaderStageCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};

                    name.clear();

                    spec_info_wrap.clear();
                }

                void updateWrap() {
                    wrap.pName = name.c_str();
                    wrap.pSpecializationInfo = spec_info_wrap.get().mapEntryCount ? &spec_info_wrap.peek() : nullptr;
                }

            // Scenarios

                void setShaderInfo(VkShaderModule _shader_module, VkShaderStageFlagBits _stage, const std::string &_name) {
                    wrap.module = _shader_module;
                    wrap.stage  = _stage;
                    name = _name;
                }

                void setShaderSpecInfo(const void* _data, size_t _data_size, const std::vector<VkSpecializationMapEntry> &_map_entries) {
                    spec_info_wrap.setData(_data, _data_size);
                    spec_info_wrap.map_entries = _map_entries;
                }
            }; // StructWrap<VkPipelineShaderStageCreateInfo> END

            template <>
            struct StructWrap<VkPipelineDynamicStateCreateInfo> {
            private:
                VkPipelineDynamicStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineDynamicStateCreateInfo)

                std::vector<VkDynamicState> dynamic_states;

                StructWrap() {}
                // StructWrap(const StructBuilder &_builder) : dynamic_states{_builder.dynamic_states} {}

                void clear() {
                    wrap = VkPipelineDynamicStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
                    dynamic_states.clear();
                }

                void updateWrap() {
                    wrap.dynamicStateCount = dynamic_states.size();
                    wrap.pDynamicStates    = dynamic_states.data();
                }

            // Scenarios

                // returns false if state is already included in "dynamic_states", true if successfully added
                bool addDynamicState(VkDynamicState _state) {
                    for (const VkDynamicState &st : dynamic_states)
                        if (st == _state) return false;

                    dynamic_states.push_back(_state);
                    return true;
                }

                // returns false if state is not in "dynamic_states", true if successfully removed entries
                bool delDynamicState(VkDynamicState _state) {
                    bool __res = false;
                    for (uint32_t i = 0u; i < dynamic_states.size(); ++i) {
                        if (dynamic_states[i] == _state) {
                            dynamic_states.erase(dynamic_states.begin() + i);
                            __res = true;
                        }
                    }
                    return __res;
                }
            }; // StructBuilder<VkPipelineDynamicStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineVertexInputStateCreateInfo> {
            private:
                VkPipelineVertexInputStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineVertexInputStateCreateInfo)

                std::vector<VkVertexInputBindingDescription> bindings;
                std::vector<VkVertexInputAttributeDescription> attributes;

                StructWrap() {}

                void clear() {
                    wrap = VkPipelineVertexInputStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

                    bindings.clear();
                    attributes.clear();
                }

                void updateWrap() {
                    wrap.vertexBindingDescriptionCount = bindings.size();
                    wrap.pVertexBindingDescriptions    = bindings.data();
                    wrap.vertexAttributeDescriptionCount = attributes.size();
                    wrap.pVertexAttributeDescriptions    = attributes.data();
                }
            }; // StructWrap<VkPipelineVertexInputStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineInputAssemblyStateCreateInfo> {
            private:
                VkPipelineInputAssemblyStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineInputAssemblyStateCreateInfo)

                StructWrap() {}

                void clear() {
                    wrap = VkPipelineInputAssemblyStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
                }

                void updateWrap() {}

            // Scenarios

                void setWholeStruct(VkPrimitiveTopology _topology, VkBool32 _primitive_restart) {
                    wrap.topology = _topology;
                    wrap.primitiveRestartEnable = _primitive_restart;
                    updateWrap();
                }
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

                StructWrap() {}
                // StructWrap(const StructBuilder &_builder) : viewports{_builder.viewports}, scissors{_builder.scissors} {}

                void clear() {
                    wrap = VkPipelineViewportStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

                    viewports_dynamic_count = 0u;
                    scissors_dynamic_count  = 0u;
                    viewports.clear();
                    scissors .clear();

                    viewports_wrap.clear();
                    scissors_wrap .clear();
                }

                void updateWrap() {
                    wrap.pViewports = nullptr;
                    wrap.viewportCount = viewports_dynamic_count;
                    if (!viewports_dynamic_count) {
                        viewports.resize(viewports_wrap.size());
                        for (uint32_t i = 0u; i < viewports.size(); ++i) {
                            viewports[i] = viewports_wrap[i].get();
                        }

                        wrap.viewportCount = viewports.size();
                        wrap.pViewports    = viewports.data();
                    }

                    wrap.pScissors = nullptr;
                    wrap.scissorCount = scissors_dynamic_count;
                    if (!scissors_dynamic_count) {
                        scissors.resize(scissors_wrap.size());
                        for (uint32_t i = 0u; i < scissors.size(); ++i) {
                            scissors[i] = scissors_wrap[i].get();
                        }

                        wrap.scissorCount  =  scissors.size();
                        wrap.pScissors     =  scissors.data();
                    }
                }

            // Scenarios

                void setViewportsDynamic(uint32_t _viewports_count) {
                    viewports_dynamic_count = _viewports_count;
                }

                void setScissorsDynamic(uint32_t _scissors_count) {
                    scissors_dynamic_count = _scissors_count;
                }

                void setFromSwapchainExtent(VkExtent2D _extent) {
                    viewports_wrap.resize(1);
                    viewports_wrap[0].clear();
                    viewports_wrap[0].setExtent(_extent);
                    
                    scissors_wrap.resize(1);
                    scissors_wrap[0].clear();
                    scissors_wrap[0].setExtent(_extent);
                }
            }; // StructWrap<VkPipelineViewportStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineRasterizationStateCreateInfo> {
            private:
                VkPipelineRasterizationStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineRasterizationStateCreateInfo)

                StructWrap() {}
                // StructWrap(const StructWrap &_sct) = default;

                void clear() {
                    wrap = VkPipelineRasterizationStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
                    wrap.lineWidth = 1.f;
                }

                void updateWrap() {}

            // Scenarios

                void setDepthBias(bool _enable, float _const_factor = 0.f, float _clamp = 0.f, float _slope_factor = 0.f) {
                    if (!_enable) {
                        _const_factor = 0.f;
                        _clamp = 0.f;
                        _slope_factor = 0.f;
                    }

                    wrap.depthBiasEnable = _enable;
                    wrap.depthBiasClamp  =  _clamp;
                    wrap.depthBiasConstantFactor = _const_factor;
                    wrap.depthBiasSlopeFactor    = _slope_factor;
                }
            }; // StructWrap<VkPipelineRasterizationStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineMultisampleStateCreateInfo> {
            private:
                VkPipelineMultisampleStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineMultisampleStateCreateInfo)

                VkSampleMask sample_mask[2] = {VkSampleMask(-1), VkSampleMask(-1)};

                StructWrap() {}

                void clear() {
                    wrap = VkPipelineMultisampleStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
                    sample_mask[0] = VkSampleMask(-1);
                    sample_mask[1] = VkSampleMask(-1);
                }

                void updateWrap() {}

            // Scenarios

                // Assuming 'VkSampleMask' is 'uint32_t', use 'mask' when 'rasterizationSamples' <= VK_SAMPLE_COUNT_32_BIT, and use 'mask2' when 'rasterizationSamples' == VK_SAMPLE_COUNT_64_BIT.
                void setSampleMask(VkSampleMask _mask = VkSampleMask(-1), VkSampleMask _mask2 = VkSampleMask(-1)) {
                    sample_mask[0] = _mask;
                    sample_mask[1] = _mask2;
                    wrap.pSampleMask = sample_mask;
                }
            }; // StructWrap<VkPipelineMultisampleStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineDepthStencilStateCreateInfo> {
            private:
                VkPipelineDepthStencilStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineDepthStencilStateCreateInfo)

                StructWrap() {}

                void clear() {
                    wrap = VkPipelineDepthStencilStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
                }

                void updateWrap() {}
            }; // StructWrap<VkPipelineDepthStencilStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineColorBlendAttachmentState> {
            private:
                VkPipelineColorBlendAttachmentState wrap{};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineColorBlendAttachmentState)

                StructWrap() { clear(); }

                void clear() {
                    wrap = VkPipelineColorBlendAttachmentState{};

                    // Default: Porter-Duff blend over
                    wrap.blendEnable = VK_TRUE;
                    wrap.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                    wrap.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                    wrap.colorBlendOp        = VK_BLEND_OP_ADD;
                    wrap.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                    wrap.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                    wrap.alphaBlendOp        = VK_BLEND_OP_ADD;
                    wrap.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                }

                void updateWrap() {}

            // Scenarios

                void setColorBlend(VkBlendOp _op, VkBlendFactor _src_factor, VkBlendFactor _dst_factor) {
                    wrap.colorBlendOp        = _op;
                    wrap.srcColorBlendFactor = _src_factor;
                    wrap.dstColorBlendFactor = _dst_factor;
                }

                void setAlphaBlend(VkBlendOp _op, VkBlendFactor _src_factor, VkBlendFactor _dst_factor) {
                    wrap.alphaBlendOp        = _op;
                    wrap.srcAlphaBlendFactor = _src_factor;
                    wrap.dstAlphaBlendFactor = _dst_factor;
                }
            }; // StructWrap<VkPipelineColorBlendAttachmentState> END

            template <>
            struct StructWrap<VkPipelineColorBlendStateCreateInfo> {
            private:
                VkPipelineColorBlendStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineColorBlendStateCreateInfo)

                std::vector<StructWrap<VkPipelineColorBlendAttachmentState>> attachments_wrap;

                std::vector<VkPipelineColorBlendAttachmentState> attachments;

                StructWrap() {}
                // StructWrap(const StructBuilder &_builder) = default;

                void clear() {
                    attachments.clear();

                    attachments_wrap.clear();
                }

                void updateWrap() {
                    attachments.resize(attachments_wrap.size());
                    for (uint32_t i = 0u; i < attachments.size(); ++i) {
                        attachments[i] = attachments_wrap[i].get();
                    }
                    wrap.attachmentCount = attachments.size();
                    wrap.pAttachments    = attachments.data();
                }

            // Scenarios

                void setLogicOp(VkLogicOp _op = VK_LOGIC_OP_MAX_ENUM) {
                    wrap.logicOpEnable = VkBool32(_op != VK_LOGIC_OP_MAX_ENUM);
                    wrap.logicOp = _op;
                }

                void setBlendConstants(const vec4 &_blend_constants) {
                    wrap.blendConstants[0] = _blend_constants.x;
                    wrap.blendConstants[1] = _blend_constants.y;
                    wrap.blendConstants[2] = _blend_constants.z;
                    wrap.blendConstants[3] = _blend_constants.w;
                }
            }; // StructStructWrap<VkPipelineColorBlendStateCreateInfo> END

            template <>
            struct StructWrap<VkPipelineTessellationStateCreateInfo> {
            private:
                VkPipelineTessellationStateCreateInfo wrap{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkPipelineTessellationStateCreateInfo)

                StructWrap() {}
                // StructWrap(const StructBuilder &_sct) = default;
                StructWrap(uint32_t _points_num) { wrap.patchControlPoints = _points_num; }

                void clear() {
                    wrap = VkPipelineTessellationStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
                }

                void updateWrap() {}
            }; // StructWrap<VkPipelineTessellationStateCreateInfo> END


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

                std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

                StructWrap() {}

                void clear() {
                    wrap = VkGraphicsPipelineCreateInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

                    shader_stages.clear();

                    active_state_flags = PipelineStateBits(0u);
                    shader_stages_wrap.clear();
                }

                void updateWrap() {
                    shader_stages.resize(shader_stages_wrap.size());
                    for (uint32_t i = 0u; i < shader_stages.size(); ++i) {
                        shader_stages[i] = shader_stages_wrap[i].get();
                    }

                    wrap.stageCount = shader_stages.size();
                    wrap.pStages    = shader_stages.data();

                    wrap.pVertexInputState   = (active_state_flags & VKFW_PIPELINE_STATE_VERTEX_INPUT_BIT  ) ?     &vertex_input_state_wrap.get() : nullptr;
                    wrap.pInputAssemblyState = (active_state_flags & VKFW_PIPELINE_STATE_INPUT_ASSEMBLY_BIT) ?   &input_assembly_state_wrap.get() : nullptr;
                    wrap.pTessellationState  = (active_state_flags & VKFW_PIPELINE_STATE_TESSELLATION_BIT  ) ?     &tessellation_state_wrap.get() : nullptr;
                    wrap.pViewportState      = (active_state_flags & VKFW_PIPELINE_STATE_VIEWPORT_BIT      ) ?         &viewport_state_wrap.get() : nullptr;
                    wrap.pRasterizationState = (active_state_flags & VKFW_PIPELINE_STATE_RASTERIZATION_BIT ) ?    &rasterization_state_wrap.get() : nullptr;
                    wrap.pMultisampleState   = (active_state_flags & VKFW_PIPELINE_STATE_MULTISAMPLE_BIT   ) ?      &multisample_state_wrap.get() : nullptr;
                    wrap.pDepthStencilState  = (active_state_flags & VKFW_PIPELINE_STATE_DEPTH_STENCIL_BIT ) ?    &depth_stencil_state_wrap.get() : nullptr;
                    wrap.pColorBlendState    = (active_state_flags & VKFW_PIPELINE_STATE_COLOR_BLEND_BIT   ) ?      &color_blend_state_wrap.get() : nullptr;
                    wrap.pDynamicState       = (active_state_flags & VKFW_PIPELINE_STATE_DYNAMIC_BIT       ) ?          &dynamic_state_wrap.get() : nullptr;
                }

            // Scenarios

                void setPipelineStates(PipelineStateFlags _state_bitmask) {
                    active_state_flags = _state_bitmask;
                }

                void setBasePipeline(int32_t _base_pipeline_index = 0, VkPipeline _base_pipeline_handle = VK_NULL_HANDLE) {
                    wrap.basePipelineHandle = _base_pipeline_handle;
                    wrap.basePipelineIndex  = _base_pipeline_index;
                }
            }; // StructWrap<VkGraphicsPipelineCreateInfo> END

            template <>
            struct StructWrap<VkComputePipelineCreateInfo> {
            private:
                VkComputePipelineCreateInfo wrap{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkComputePipelineCreateInfo)

                StructWrap<VkPipelineShaderStageCreateInfo>  shader_stage_wrap;

                StructWrap() {}

                void clear() {
                    wrap = VkComputePipelineCreateInfo{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};

                    shader_stage_wrap.clear();
                }

                void updateWrap() {
                    wrap.stage = shader_stage_wrap.get();
                }

            // Scenarios

                void setBasePipeline(int32_t _base_pipeline_index = 0, VkPipeline _base_pipeline_handle = VK_NULL_HANDLE) {
                    wrap.basePipelineHandle = _base_pipeline_handle;
                    wrap.basePipelineIndex  = _base_pipeline_index;
                }
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

                StructWrap() {}

                void clear() {
                    wrap = VkRayTracingPipelineCreateInfoKHR{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};

                    shader_stages.clear();

                    active_state_flags = PipelineStateBits(0u);
                    shader_stages_wrap.clear();
                }

                void updateWrap() {
                    shader_stages.resize(shader_stages_wrap.size());
                    for (uint32_t i = 0u; i < shader_stages.size(); ++i) {
                        shader_stages[i] = shader_stages_wrap[i].get();
                    }

                    shader_groups.resize(shader_groups_wrap.size());
                    for (uint32_t i = 0u; i < shader_groups.size(); ++i) {
                        shader_groups[i] = shader_groups_wrap[i].get();
                    }

                    wrap.stageCount = shader_stages.size();
                    wrap.pStages    = shader_stages.data();
                    wrap.groupCount = shader_groups.size();
                    wrap.pGroups    = shader_groups.data();

                    wrap.pLibraryInfo      = (wrap.flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) ? &lib_info_wrap.get() : nullptr;
                    wrap.pLibraryInterface = (wrap.flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) ? &lib_interface_wrap.get() : nullptr;
                    wrap.pDynamicState     = (active_state_flags & VKFW_PIPELINE_STATE_DYNAMIC_BIT) ? &dynamic_state_wrap.get() : nullptr;
                }

            // Scenarios

                void setPipelineStates(PipelineStateFlags _state_bitmask) {
                    active_state_flags = _state_bitmask;
                }

                void setBasePipeline(int32_t _base_pipeline_index = 0, VkPipeline _base_pipeline_handle = VK_NULL_HANDLE) {
                    wrap.basePipelineHandle = _base_pipeline_handle;
                    wrap.basePipelineIndex  = _base_pipeline_index;
                }
            }; // StructWrap<VkRayTracingPipelineCreateInfoKHR>


//  == === ==== >   Vulkan: CreateInfo (Command Buffer-related)

            template <>
            struct StructWrap<VkCommandPoolCreateInfo> {
            private:
                VkCommandPoolCreateInfo wrap{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkCommandPoolCreateInfo)

                StructWrap() {}

                void clear() {
                    wrap = VkCommandPoolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
                }

                void updateWrap() {}
            }; // StructWrap<VkCommandPoolCreateInfo> END

            template <>
            struct StructWrap<VkCommandBufferAllocateInfo> {
            private:
                VkCommandBufferAllocateInfo wrap{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkCommandBufferAllocateInfo)

                StructWrap() {}


                void clear() {
                    wrap = VkCommandBufferAllocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
                }

                void updateWrap() {}
            }; // StructWrap<VkCommandBufferAllocateInfo> END


//  == === ==== >   Vulkan: Other Function Configs

            template <>
            struct StructWrap<VkAllocationCallbacks> {
            private:
                VkAllocationCallbacks *wrap = nullptr;
                VKFW_STRUCT_WRAP_ACCESS(VkAllocationCallbacks*)

                StructWrap() {}
               ~StructWrap() { clear(); }

                void clear() {
                    safeDelete(wrap);
                }

                void updateWrap() {}
            }; // StructWrap<VkAllocationCallbacks> END

            template <>
            struct StructWrap<VkPhysicalDeviceMemoryProperties2> {
            private:
                VkPhysicalDeviceMemoryProperties2 wrap{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
                VKFW_STRUCT_WRAP_ACCESS(VkPhysicalDeviceMemoryProperties2)

                StructWrap() {}

                void clear() {
                    wrap = VkPhysicalDeviceMemoryProperties2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
                }

                void updateWrap() {}

            // Scenarios

                void loadForPhysicalDevice(VkPhysicalDevice _phys_device) {
                    vkGetPhysicalDeviceMemoryProperties2(_phys_device, &wrap);
                    printInfo();
                }

                void printInfo() const {
                    printf("\nPhysical device memory description:\n");
                    for (uint32_t i = 0u; i < wrap.memoryProperties.memoryHeapCount; ++i) {
                        std::string __tmp_enum_str = Enum::memory_heap_flag_bits.getBitNames(wrap.memoryProperties.memoryHeaps[i].flags);
                        printf("Heap %d - size (%.2f MB); flags (%s)\n", i, float(wrap.memoryProperties.memoryHeaps[i].size) / (1024 * 1024), __tmp_enum_str.c_str());
                        for (uint32_t j = 0u; j < wrap.memoryProperties.memoryTypeCount; ++j) {
                            if (wrap.memoryProperties.memoryTypes[j].heapIndex == i) {
                                __tmp_enum_str = Enum::memory_property_flag_bits.getBitNames(wrap.memoryProperties.memoryTypes[j].propertyFlags);
                                printf("    %d.%d Memory type - flags (%s)\n", i, j, __tmp_enum_str.c_str());
                            }
                        }
                    }
                    printf("\n");
                }
            }; // StructWrap<VkPhysicalDeviceMemoryProperties2> END

            template <>
            struct StructWrap<VkCommandBufferInheritanceInfo> {
            private:
                VkCommandBufferInheritanceInfo wrap{VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkCommandBufferInheritanceInfo)

                StructWrap() {}

                void clear() {
                    wrap = VkCommandBufferInheritanceInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
                }

                void updateWrap() {}
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

                StructWrap() {}
                StructWrap(const StructWrap<VkSubmitInfo> &_wrap) : wait_semaphores{_wrap.wait_semaphores}, wait_stages{_wrap.wait_stages},
                                                                    command_buffers{_wrap.command_buffers}, signal_semaphores{_wrap.signal_semaphores} {
                    wrap = _wrap.wrap;
                    updateWrap();
                }

                void clear() {
                    wrap = VkSubmitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};

                    wait_stages.clear();
                    wait_semaphores.clear();
                    command_buffers.clear();
                    signal_semaphores.clear();
                }

                void updateWrap() {
                    wrap.waitSemaphoreCount   = wait_semaphores.size();
                    wrap.pWaitSemaphores      = wait_semaphores.data();
                    wrap.pWaitDstStageMask    = wait_stages.data(); // size == .waitSemaphoreCount
                    wrap.commandBufferCount   = command_buffers.size();
                    wrap.pCommandBuffers      = command_buffers.data();
                    wrap.signalSemaphoreCount = signal_semaphores.size();
                    wrap.pSignalSemaphores    = signal_semaphores.data();
                }
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

                StructWrap() {}

                void clear() {
                    wrap = VkPresentInfoKHR{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};

                    wait_semaphores.clear();
                    image_indices.clear();
                    swapchains.clear();
                    results.clear();
                    use_results = false;
                }

                void updateWrap() {
                    results.resize(use_results * swapchains.size());
                    for (auto& res : results) res = VK_SUCCESS;

                    wrap.waitSemaphoreCount = wait_semaphores.size();
                    wrap.pWaitSemaphores = wait_semaphores.data();
                    wrap.swapchainCount = swapchains.size();
                    wrap.pSwapchains = swapchains.data();
                    wrap.pImageIndices = image_indices.data();
                    wrap.pResults = results.empty() ? nullptr : results.data();
                }

            // Scenarios

                // Note: 'assign' reuses the vector capacity, so per-frame calls don't allocate memory
                void setWholeStruct(Span<VkSemaphore> _wait_semaphores,
                                    Span<VkSwapchainKHR> _swapchains,
                                    Span<uint32_t> _image_indices, bool _use_results = false) {
                    wait_semaphores.assign(_wait_semaphores.begin(), _wait_semaphores.end());
                    swapchains     .assign(     _swapchains.begin(),      _swapchains.end());
                    image_indices  .assign(  _image_indices.begin(),   _image_indices.end());
                    use_results = _use_results;
                    updateWrap();
                }
            }; // StructWrap<VkPresentInfoKHR> END

            template <>
            struct StructWrap<VkRenderPassBeginInfo> {
            private:
                VkRenderPassBeginInfo wrap{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkRenderPassBeginInfo)

                std::vector<StructWrap<VkClearValue>> clear_values_wrap;
                StructWrap<VkRect2D> render_area_wrap;

                std::vector<VkClearValue> clear_values;

                StructWrap() {}

                void clear() {
                    wrap = VkRenderPassBeginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};

                    clear_values.clear();

                    clear_values_wrap.clear();
                    render_area_wrap .clear();
                }

                void updateWrap() {
                    clear_values.resize(clear_values_wrap.size());
                    for (uint32_t i = 0u; i < clear_values.size(); ++i) {
                        clear_values[i] = clear_values_wrap[i].get();
                    }

                    wrap.renderArea = render_area_wrap.get();
                    wrap.clearValueCount = clear_values.size();
                    wrap.pClearValues = clear_values.data();
                }

            // Scenarios

                void addClearValue4f(const vec4f &_color) {
                    clear_values_wrap.emplace_back();
                    clear_values_wrap.back().fromColorVec4f(_color);
                }

                void addClearValue4i(const vec4i &_color) {
                    clear_values_wrap.emplace_back();
                    clear_values_wrap.back().fromColorVec4i(_color);
                }

                void addClearValue4u(const vec4u &_color) {
                    clear_values_wrap.emplace_back();
                    clear_values_wrap.back().fromColorVec4u(_color);
                }

                void addClearValueDepthStencil(float _depth, uint32_t _stencil) {
                    clear_values_wrap.emplace_back();
                    clear_values_wrap.back().fromDepthStencil(_depth, _stencil);
                }
            }; // StructWrap<VkRenderPassBeginInfo> END

            template <>
            struct StructWrap<VkCommandBufferBeginInfo> {
            private:
                VkCommandBufferBeginInfo wrap{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
                VKFW_STRUCT_WRAP_ACCESS(VkCommandBufferBeginInfo)

                StructWrap<VkCommandBufferInheritanceInfo> inheritance_info_wrap;
                VkBool32 use_inheritance_info = false;

                StructWrap() {}

                void clear() {
                    wrap = VkCommandBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

                    inheritance_info_wrap.clear();
                }

                void updateWrap() {
                    wrap.pInheritanceInfo = nullptr;

                    if (use_inheritance_info) {
                        wrap.pInheritanceInfo = &inheritance_info_wrap.get();
                    }
                }
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

                void createObject(const VkAllocationCallbacks *_create_cb = nullptr, bool _volk_instance_only = false) {
                    handle = Func::CreateInstance(&create_info.get(), _create_cb);

                    if (_volk_instance_only) {
                        volkLoadInstanceOnly(handle);
                        printf(SVKFW_WRAPINFO("VKFW::Object::Instance :: createObject", "Only instance-based function pointers are loaded. Call volkLoadDevice() after VkDevice creation.\n"));
                    }
                    else {
                        volkLoadInstance(handle);
                        printf(SVKFW_WRAPINFO("VKFW::Object::Instance :: createObject", "All Vulkan function pointers are loaded.\n"));
                    }
                }

                void destroyObject(const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyInstance(handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // Instance END


//  == === ==== >   Vulkan Debug Messenger Callback

            struct DebugUtilsMessengerEXT {
                VkDebugUtilsMessengerEXT handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkDebugUtilsMessengerCreateInfoEXT> create_info;

                void createObject(VkInstance _instance, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handle = Func::CreateDebugUtilsMessengerEXT(_instance, &create_info.get(), _create_cb);
                }

                void destroyObject(VkInstance _instance, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyDebugUtilsMessengerEXT(_instance, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // DebugMessengerEXT END


//  == === ==== >   Vulkan Physical Device

            struct PhysicalDevice {
                VkPhysicalDevice handle = VK_NULL_HANDLE;
                Util::PhysDeviceManager select_info;

                void createObject() {
                    handle = select_info.selectOne();
                }

                void destroyObject() {
                    handle = VK_NULL_HANDLE;
                }
            }; // PhysicalDevice END


//  == === ==== >   Vulkan Queue Family

            struct QueueFamily {
                VkfwQueueFamily handle = UINT32_MAX;
                Util::QueueFamilyManager select_info;

                void createObject() {
                    handle = select_info.selectOne();
                }

                void destroyObject() {
                    handle = UINT32_MAX;
                }
            }; // QueueFamily END


//  == === ==== >   Vulkan Queue

            struct Queue {
                std::vector<VkQueue> handles;
                Struct::StructWrap<VkDeviceQueueCreateInfo> create_info;

                void createObject(VkDevice _device) {
                    handles.resize(create_info.priorities.size(), VK_NULL_HANDLE);
                    for (uint32_t i = 0u; i < handles.size(); ++i)
                        handles[i] = Func::GetDeviceQueue(_device, create_info.queue_family, i);
                }

                void destroyObject() {
                    handles.clear();
                }
            }; // Queue END


//  == === ==== >   Vulkan Logical Device

            struct Device {
                VkDevice handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkDeviceCreateInfo> create_info;

                void createObject(VkPhysicalDevice _phys_device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handle = Func::CreateDevice(_phys_device, &create_info.get(), _create_cb);
                }

                void destroyObject(const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyDevice(handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // Device END


//  == === ==== >   Vulkan Surface

            struct SurfaceKHR {
                VkSurfaceKHR handle = VK_NULL_HANDLE;

                void createObject(VkInstance _instance, GLFWwindow *_glfw_window, const VkAllocationCallbacks *_create_cb = nullptr) {
                    if (glfwCreateWindowSurface(_instance, _glfw_window, _create_cb, &handle) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::Object::SurfaceKHR :: createObject", "could not create surface"));
                }

                void destroyObject(VkInstance _instance, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroySurfaceKHR(_instance, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // SurfaceKHR END


//  == === ==== >   Vulkan Swapchain

            struct SwapchainKHR {
                VkSwapchainKHR handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkSwapchainCreateInfoKHR> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handle = Func::CreateSwapchainKHR(_device, &create_info.get(), _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroySwapchainKHR(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // SwapchainKHR END


//  == === ==== >   Vulkan Shader Module

            struct ShaderModule {
                VkShaderModule handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkShaderModuleCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handle = Func::CreateShaderModule(_device, &create_info.get(), _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyShaderModule(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // ShaderModule END


//  == === ==== >   Vulkan Buffer

            struct Buffer {
                VkBuffer handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkBufferCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handle = Func::CreateBuffer(_device, &create_info.get(), _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyBuffer(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // Buffer END


//  == === ==== >   Vulkan Buffer View

            struct BufferView {
                VkBufferView handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkBufferViewCreateInfo> create_info;

                void createObject(VkDevice _device, VkBuffer _buffer, const VkAllocationCallbacks *_create_cb = nullptr) {
                    create_info.edit().buffer = _buffer;
                    handle = Func::CreateBufferView(_device, &create_info.get(), _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyBufferView(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // BufferView END


//  == === ==== >   Vulkan Image

            struct Image {
                VkImage handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkImageCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handle = Func::CreateImage(_device, &create_info.get(), _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyImage(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            };  // Image END


//  == === ==== >   Vulkan Image View

            struct ImageView {
                VkImageView handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkImageViewCreateInfo> create_info;

                void createObject(VkDevice _device, VkImage _image, const VkAllocationCallbacks *_create_cb = nullptr) {
                    create_info.edit().image = _image;
                    handle = Func::CreateImageView(_device, &create_info.get(), _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyImageView(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // ImageView END


//  == === ==== >   Vulkan Device Memory

            struct DeviceMemory {
                VkDeviceMemory handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkMemoryAllocateInfo> allocate_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_allocate_cb = nullptr) {
                    handle = Func::AllocateMemory(_device, &allocate_info.get(), _allocate_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkFreeMemory(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // DeviceMemory END


//  == === ==== >   Vulkan Render Pass

            struct RenderPass {
                VkRenderPass handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkRenderPassCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handle = Func::CreateRenderPass(_device, &create_info.get(), _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyRenderPass(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // RenderPass END


//  == === ==== >   Vulkan Descriptor Set Layout

            struct DescriptorSetLayout {
                VkDescriptorSetLayout handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkDescriptorSetLayoutCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handle = Func::CreateDescriptorSetLayout(_device, &create_info.get(), _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // DescriptorSetLayout END


//  == === ==== >   Vulkan Descriptor Pool

            struct DescriptorPool {
                VkDescriptorPool handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkDescriptorPoolCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handle = Func::CreateDescriptorPool(_device, &create_info.get(), _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyDescriptorPool(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // DescriptorPool END


//  == === ==== >   Vulkan Descriptor Set

            struct DescriptorSet {
                std::vector<VkDescriptorSet> handles;
                Struct::StructWrap<VkDescriptorSetAllocateInfo> alloc_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handles = Func::AllocateDescriptorSets(_device, &alloc_info.get());
                }

                void destroyObject(VkDevice _device) {
                    if (!handles.empty()) vkFreeDescriptorSets(_device, alloc_info.peek().descriptorPool, handles.size(), handles.data());
                    handles.clear();
                }
            }; // DescriptorSet END


//  == === ==== >   Vulkan Pipeline Layout

            struct PipelineLayout {
                VkPipelineLayout handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkPipelineLayoutCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handle = Func::CreatePipelineLayout(_device, &create_info.get(), _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyPipelineLayout(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // PipelineLayout END


//  == === ==== >   Vulkan Pipeline Cache

            struct PipelineCache {
                VkPipelineCache handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkPipelineCacheCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handle = Func::CreatePipelineCache(_device, &create_info.get(), _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyPipelineCache(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
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

                Pipeline() {}
                Pipeline(const Pipeline &_obj_pip) : handle{_obj_pip.handle} {
                    setPipelineType(_obj_pip.pipeline_type);
                    setPipelineFromCopy(_obj_pip);
                }
                Pipeline(Pipeline &&_obj_pip) : handle{_obj_pip.handle} {
                    setPipelineType(_obj_pip.pipeline_type);
                    setPipelineFromCopy(_obj_pip);
                    _obj_pip.deletePipelineWrap();
                    _obj_pip.handle = VK_NULL_HANDLE;
                }
               ~Pipeline() { deletePipelineWrap(); }

                // Note: the union makes the implicit assignment deleted, it's needed to store pipelines in a 'Pool'
                Pipeline& operator=(const Pipeline &_obj_pip) {
                    if (this == &_obj_pip) return *this;
                    handle = _obj_pip.handle;
                    setPipelineType(_obj_pip.pipeline_type);
                    setPipelineFromCopy(_obj_pip);
                    return *this;
                }

                // Returns VK_SUCCESS or another success code (e.g. VK_PIPELINE_COMPILE_REQUIRED, VK_OPERATION_DEFERRED_KHR), throws on errors
                // Note: the render pass and the subpass are taken from the create info
                VkResult createObject(VkDevice _device, VkRenderPass _render_pass, uint32_t _subpass,
                                      VkDeferredOperationKHR _deferred_op, VkPipelineCache _pipeline_cache = VK_NULL_HANDLE,
                                      const VkAllocationCallbacks *_create_cb = nullptr) {
                    (void)_render_pass; (void)_subpass;
                    switch (pipeline_type) {
                        case VKFW_PIPELINE_TYPE_GRAPHICS:    return Func::CreateGraphicsPipelines     (_device,               _pipeline_cache,   graphics_create_info_wrap.get(), _create_cb, &handle);
                        case VKFW_PIPELINE_TYPE_COMPUTE:     return Func::CreateComputePipelines      (_device,               _pipeline_cache,    compute_create_info_wrap.get(), _create_cb, &handle);
                        case VKFW_PIPELINE_TYPE_RAY_TRACING: return Func::CreateRayTracingPipelinesKHR(_device, _deferred_op, _pipeline_cache, raytracing_create_info_wrap.get(), _create_cb, &handle);
                        case VKFW_PIPELINE_TYPE_UNDEFINED:  return VK_SUCCESS;
                        default: throw std::runtime_error(SVKFW_WRAPERR("VKFW::Object::Pipeline :: createObject", "unknown pipeline type: " + std::to_string(pipeline_type)));
                    };
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyPipeline(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }

            // Scenarios

                void setPipelineFromCopy(const Pipeline &_obj_pip) {
                    switch (pipeline_type) {
                        case VKFW_PIPELINE_TYPE_GRAPHICS:      graphics_create_info_wrap = _obj_pip.  graphics_create_info_wrap; break;
                        case VKFW_PIPELINE_TYPE_COMPUTE:        compute_create_info_wrap = _obj_pip.   compute_create_info_wrap; break;
                        case VKFW_PIPELINE_TYPE_RAY_TRACING: raytracing_create_info_wrap = _obj_pip.raytracing_create_info_wrap; break;
                        default: break;
                    };
                }

                void setPipelineType(PipelineType _pipeline_type) {
                    deletePipelineWrap();
                    pipeline_type = _pipeline_type;
                    activatePipelineWrap();
                }

                void deletePipelineWrap() {
                    switch (pipeline_type) {
                        case VKFW_PIPELINE_TYPE_GRAPHICS:      graphics_create_info_wrap.~StructWrap(); break;
                        case VKFW_PIPELINE_TYPE_COMPUTE:        compute_create_info_wrap.~StructWrap(); break;
                        case VKFW_PIPELINE_TYPE_RAY_TRACING: raytracing_create_info_wrap.~StructWrap(); break;
                        case VKFW_PIPELINE_TYPE_UNDEFINED:  break;
                        // Note: called from the destructor, so it must not throw ('activatePipelineWrap()' never leaves an unknown type)
                        default: fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::Object::Pipeline :: deletePipelineWrap", "unknown pipeline type: %u\n"), uint32_t(pipeline_type)); break;
                    };
                    pipeline_type = VKFW_PIPELINE_TYPE_UNDEFINED;
                }

                void activatePipelineWrap() {
                    switch (pipeline_type) {
                        case VKFW_PIPELINE_TYPE_GRAPHICS:    new (&  graphics_create_info_wrap) Struct::StructWrap<  VkGraphicsPipelineCreateInfo   >; break;
                        case VKFW_PIPELINE_TYPE_COMPUTE:     new (&   compute_create_info_wrap) Struct::StructWrap<   VkComputePipelineCreateInfo   >; break;
                        case VKFW_PIPELINE_TYPE_RAY_TRACING: new (&raytracing_create_info_wrap) Struct::StructWrap<VkRayTracingPipelineCreateInfoKHR>; break;
                        case VKFW_PIPELINE_TYPE_UNDEFINED:  break;
                        default: {
                            const PipelineType __unknown_type = pipeline_type;
                            pipeline_type = VKFW_PIPELINE_TYPE_UNDEFINED;
                            throw std::runtime_error(SVKFW_WRAPERR("VKFW::Object::Pipeline :: activatePipelineWrap", "unknown pipeline type: " + std::to_string(__unknown_type)));
                        }
                    };
                }

                ContextIndex addShaderStage() {
                    ContextIndex __res_ci_shader_stage = VKFW_CI_IGNORE;

                    switch(pipeline_type) {
                        case VKFW_PIPELINE_TYPE_GRAPHICS:      graphics_create_info_wrap.shader_stages_wrap.emplace_back(); __res_ci_shader_stage =   graphics_create_info_wrap.shader_stages_wrap.size() - 1; break;
                        case VKFW_PIPELINE_TYPE_COMPUTE:     __res_ci_shader_stage = 0u; break;
                        case VKFW_PIPELINE_TYPE_RAY_TRACING: raytracing_create_info_wrap.shader_stages_wrap.emplace_back(); __res_ci_shader_stage = raytracing_create_info_wrap.shader_stages_wrap.size() - 1; break;
                        case VKFW_PIPELINE_TYPE_UNDEFINED:  break;
                        default: throw std::runtime_error(SVKFW_WRAPERR("VKFW::Object::Pipeline :: addShaderStage", "unknown pipeline type: " + std::to_string(pipeline_type)));
                    }
                    return __res_ci_shader_stage;
                }

                Struct::StructWrap<VkPipelineShaderStageCreateInfo>* shortcutShaderStage(ContextIndex _ci_shader_stage) {
                    switch(pipeline_type) {
                        case VKFW_PIPELINE_TYPE_GRAPHICS:    return &  graphics_create_info_wrap.shader_stages_wrap[_ci_shader_stage];
                        case VKFW_PIPELINE_TYPE_COMPUTE:     return &    compute_create_info_wrap.shader_stage_wrap;
                        case VKFW_PIPELINE_TYPE_RAY_TRACING: return &raytracing_create_info_wrap.shader_stages_wrap[_ci_shader_stage];
                        case VKFW_PIPELINE_TYPE_UNDEFINED: throw std::runtime_error(SVKFW_WRAPERR("VKFW::Object::Pipeline :: shortcutShaderStage", "undefined pipeline type: " + std::to_string(pipeline_type)));
                        default:                           throw std::runtime_error(SVKFW_WRAPERR("VKFW::Object::Pipeline :: shortcutShaderStage",   "unknown pipeline type: " + std::to_string(pipeline_type)));
                    }
                    return nullptr;
                }
            }; // Pipeline END


//  == === ==== >   Vulkan Framebuffer

            struct Framebuffer {
                VkFramebuffer handle = VK_NULL_HANDLE;
                Struct::StructWrap<VkFramebufferCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handle = Func::CreateFramebuffer(_device, &create_info.get(), _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyFramebuffer(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // Framebuffer END


//  == === ==== >   Vulkan Command Pool

            struct CommandPool {
                VkCommandPool handle = VK_NULL_HANDLE;

                Struct::StructWrap<VkCommandPoolCreateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handle = Func::CreateCommandPool(_device, &create_info.get(), _create_cb);
                }

                void createObject(VkDevice _device, VkfwQueueFamily _queue_family, VkCommandPoolCreateFlags _flags = 0u, const VkAllocationCallbacks *_create_cb = nullptr) {
                    create_info.edit().flags =  _flags;
                    create_info.edit().queueFamilyIndex = _queue_family;
                    handle = Func::CreateCommandPool(_device, &create_info.get(), _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    if (handle != VK_NULL_HANDLE) vkDestroyCommandPool(_device, handle, _destroy_cb);
                    handle = VK_NULL_HANDLE;
                }
            }; // CommandPool END


//  == === ==== >   Vulkan Command Buffer

            struct CommandBuffer {
                std::vector<VkCommandBuffer> handles;
                Struct::StructWrap<VkCommandBufferAllocateInfo> create_info;

                void createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handles = Func::AllocateCommandBuffers(_device, &create_info.get());
                }

                void destroyObject(VkDevice _device) {
                    if (!handles.empty()) vkFreeCommandBuffers(_device, create_info.peek().commandPool, (uint32_t)handles.size(), handles.data());
                    handles.clear();
                }

            // Scenarios

                void createPrimary  (VkDevice _device, VkCommandPool _command_pool, uint32_t _buffer_count, const VkAllocationCallbacks *_create_cb = nullptr) {
                    create_info.edit().level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                    create_info.edit().commandPool = _command_pool;
                    create_info.edit().commandBufferCount = _buffer_count;
                    handles = Func::AllocateCommandBuffers(_device, &create_info.get());
                }

                void createSecondary(VkDevice _device, VkCommandPool _command_pool, uint32_t _buffer_count, const VkAllocationCallbacks *_create_cb = nullptr) {
                    create_info.edit().level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                    create_info.edit().commandPool = _command_pool;
                    create_info.edit().commandBufferCount = _buffer_count;
                    handles = Func::AllocateCommandBuffers(_device, &create_info.get());
                }
            }; // CommandBuffer END


//  == === ==== >   Vulkan Semaphore

            struct Semaphore {
                std::vector<VkSemaphore> handles;
                Struct::StructWrap<VkSemaphoreCreateInfo> create_info;

                void createObject(VkDevice _device, uint32_t _semaphore_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handles = Func::CreateSemaphores(_device, &create_info.get(), _semaphore_count, _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    for (auto handle : handles)
                        if (handle != VK_NULL_HANDLE) vkDestroySemaphore(_device, handle, _destroy_cb);
                    handles.clear();
                }
            }; // Semaphore END


//  == === ==== >   Vulkan Fence

            struct Fence {
                std::vector<VkFence> handles;
                Struct::StructWrap<VkFenceCreateInfo> create_info;

                void createObject(VkDevice _device, uint32_t _fence_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handles = Func::CreateFences(_device, &create_info.get(), _fence_count, _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    for (auto handle : handles)
                        vkDestroyFence(_device, handle, _destroy_cb);
                    handles.clear();
                }

            // Scenarios

                void createSignaled(VkDevice _device, uint32_t _fence_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr) {
                    create_info.setWholeStruct(true);
                    handles = Func::CreateFences(_device, &create_info.get(), _fence_count, _create_cb);
                }

                void createNotSignaled(VkDevice _device, uint32_t _fence_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr) {
                    create_info.setWholeStruct(false);
                    handles = Func::CreateFences(_device, &create_info.get(), _fence_count, _create_cb);
                }
            }; // Fence END


//  == === ==== >   Vulkan Event

            struct Event {
                std::vector<VkEvent> handles;
                Struct::StructWrap<VkEventCreateInfo> create_info;

                void createObject(VkDevice _device, uint32_t _event_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr) {
                    handles = Func::CreateEvents(_device, &create_info.get(), _event_count, _create_cb);
                }

                void destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb = nullptr) {
                    for (auto handle : handles)
                        vkDestroyEvent(_device, handle, _destroy_cb);
                    handles.clear();
                }

            // Scenarios

                void createSignaled(VkDevice _device, uint32_t _event_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr) {
                    create_info.setWholeStruct(true);
                    handles = Func::CreateEvents(_device, &create_info.get(), _event_count, _create_cb);
                }

                void createNotSignaled(VkDevice _device, uint32_t _event_count = 1u, const VkAllocationCallbacks *_create_cb = nullptr) {
                    create_info.setWholeStruct(false);
                    handles = Func::CreateEvents(_device, &create_info.get(), _event_count, _create_cb);
                }
            }; // Event END

        }; // Object END
    }; // VKFW END
}; // Simple END

#endif