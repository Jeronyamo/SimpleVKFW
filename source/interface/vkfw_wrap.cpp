#include "interface/vkfw_wrap.h"

namespace Simple {
    namespace VKFW {
        namespace Detail {

            void unlinkNewerVersions(DeviceFeatures &_chain, uint32_t _api_version) {
                if (_api_version < VK_API_VERSION_1_3) _chain.unlink<VkPhysicalDeviceVulkan13Features>();
                if (_api_version < VK_API_VERSION_1_2) {
                    _chain.unlink<VkPhysicalDeviceVulkan11Features>();
                    _chain.unlink<VkPhysicalDeviceVulkan12Features>();
                }
            }

            void unlinkNewerVersions(DeviceProperties &_chain, uint32_t _api_version) {
                if (_api_version < VK_API_VERSION_1_3) _chain.unlink<VkPhysicalDeviceVulkan13Properties>();
                if (_api_version < VK_API_VERSION_1_2) {
                    _chain.unlink<VkPhysicalDeviceVulkan11Properties>();
                    _chain.unlink<VkPhysicalDeviceVulkan12Properties>();
                }
            }

            // Note: feature structures are 'sType', 'pNext' and then only VkBool32 members ('FeatureNames<T>::count' of them);
            //       VkPhysicalDeviceFeatures has only the VkBool32 members
            template <typename T>
            static VkBool32 *featureBools_(T &_s) {
                static_assert(FeatureNames<T>::count > 0u && sizeof(T) >= sizeof(VkBaseOutStructure) + FeatureNames<T>::count * sizeof(VkBool32) &&
                              sizeof(T) <  sizeof(VkBaseOutStructure) + FeatureNames<T>::count * sizeof(VkBool32) + 8u,
                              "VKFW::Detail :: featureBools_ - not a feature structure of VkBool32 members");
                return reinterpret_cast<VkBool32*>(reinterpret_cast<uint8_t*>(&_s) + sizeof(VkBaseOutStructure));
            }
            static VkBool32 *featureBools_(VkPhysicalDeviceFeatures &_s) {
                static_assert(sizeof(VkPhysicalDeviceFeatures) == FeatureNames<VkPhysicalDeviceFeatures>::count * sizeof(VkBool32),
                              "VKFW::Detail :: featureBools_ - VkPhysicalDeviceFeatures must hold only VkBool32 members");
                return &_s.robustBufferAccess;
            }

            template <typename T>
            static void unsupported_(const T &_requested, const T &_supported, std::vector<std::string> &_res) {
                const VkBool32 *__req = featureBools_(const_cast<T&>(_requested)), *__sup = featureBools_(const_cast<T&>(_supported));
                for (uint32_t i = 0u; i < FeatureNames<T>::count; ++i)
                    if (__req[i] && !__sup[i]) _res.push_back(std::string(FeatureNames<T>::name()) + "::" + FeatureNames<T>::member(i));
            }

            template <typename T>
            static void addSupported_(T &_dst, const T &_requested, const T &_supported) {
                VkBool32 *__dst = featureBools_(_dst);
                const VkBool32 *__req = featureBools_(const_cast<T&>(_requested)), *__sup = featureBools_(const_cast<T&>(_supported));
                for (uint32_t i = 0u; i < FeatureNames<T>::count; ++i)
                    if (__req[i] && __sup[i]) __dst[i] = VK_TRUE;
            }

            std::vector<std::string> unsupportedFeatures(const DeviceFeatures &_requested, const DeviceFeatures &_supported) {
                std::vector<std::string> __res;
                unsupported_(_requested.head().features, _supported.head().features, __res);
                unsupported_(_requested.get<VkPhysicalDeviceVulkan11Features>(), _supported.get<VkPhysicalDeviceVulkan11Features>(), __res);
                unsupported_(_requested.get<VkPhysicalDeviceVulkan12Features>(), _supported.get<VkPhysicalDeviceVulkan12Features>(), __res);
                unsupported_(_requested.get<VkPhysicalDeviceVulkan13Features>(), _supported.get<VkPhysicalDeviceVulkan13Features>(), __res);
                return __res;
            }

            void addSupportedFeatures(DeviceFeatures &_dst, const DeviceFeatures &_requested, const DeviceFeatures &_supported) {
                addSupported_(_dst.head().features, _requested.head().features, _supported.head().features);
                addSupported_(_dst.get<VkPhysicalDeviceVulkan11Features>(), _requested.get<VkPhysicalDeviceVulkan11Features>(), _supported.get<VkPhysicalDeviceVulkan11Features>());
                addSupported_(_dst.get<VkPhysicalDeviceVulkan12Features>(), _requested.get<VkPhysicalDeviceVulkan12Features>(), _supported.get<VkPhysicalDeviceVulkan12Features>());
                addSupported_(_dst.get<VkPhysicalDeviceVulkan13Features>(), _requested.get<VkPhysicalDeviceVulkan13Features>(), _supported.get<VkPhysicalDeviceVulkan13Features>());
            }
        }; // Detail END

        namespace Func {

            std::vector<VkCommandBuffer> AllocateCommandBuffers(VkDevice _device, VkCommandPool _command_pool,
                                                                VkCommandBufferLevel _level, uint32_t _buffer_count,
                                                                const void *_p_next) {
                SVKFW_ASSERT(_buffer_count != 0u, std::invalid_argument, "VKFW::Func :: AllocateCommandBuffers", "'buffer_count' must be positive");

                VkCommandBufferAllocateInfo __info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
                __info.pNext = _p_next;
                __info.commandPool = _command_pool;
                __info.level = _level;
                __info.commandBufferCount = _buffer_count;
                return AllocateCommandBuffers(_device, &__info);
            }

            std::vector<VkSemaphore> CreateSemaphores(VkDevice _device, const VkSemaphoreCreateInfo *_create_info, uint32_t _semaphore_count, const VkAllocationCallbacks *_allocator) {
                std::vector<VkSemaphore> __semaphores((size_t)_semaphore_count, VK_NULL_HANDLE);

                for (uint32_t i = 0u; i < _semaphore_count; ++i)
                    VKFW_CHECK(vkCreateSemaphore(_device, _create_info, _allocator, &__semaphores[i]), "vkCreateSemaphore");
                return __semaphores;
            }

            std::vector<VkFence> CreateFences(VkDevice _device, const VkFenceCreateInfo *_create_info, uint32_t _fence_count, const VkAllocationCallbacks *_allocator) {
                std::vector<VkFence> __fences((size_t)_fence_count, VK_NULL_HANDLE);

                for (uint32_t i = 0u; i < _fence_count; ++i)
                    VKFW_CHECK(vkCreateFence(_device, _create_info, _allocator, &__fences[i]), "vkCreateFence");
                return __fences;
            }

            std::vector<VkEvent> CreateEvents(VkDevice _device, const VkEventCreateInfo *_create_info, uint32_t _event_count, const VkAllocationCallbacks *_allocator) {
                std::vector<VkEvent> __events((size_t)_event_count, VK_NULL_HANDLE);

                for (uint32_t i = 0u; i < _event_count; ++i)
                    VKFW_CHECK(vkCreateEvent(_device, _create_info, _allocator, &__events[i]), "vkCreateEvent");
                return __events;
            }

            std::vector<uint8_t> GetRayTracingCaptureReplayShaderGroupHandlesKHR(VkPhysicalDevice _phys_device, VkDevice _device, VkPipeline _pipeline,
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


        namespace Util {

            VKAPI_ATTR VkBool32 VKAPI_CALL vkfwDebugMessageCallback(
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

            bool allLayersAvailable(const StringVec &_layers, const std::vector<VkLayerProperties> _layer_list) {
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

            bool allExtAvailable(const StringVec &_extensions, const std::vector<VkExtensionProperties> &_ext_list) {
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

            void checkLayers(const StringVec &_layers, bool *_flags, const std::vector<VkLayerProperties> &_layer_list) {
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

            void checkExtensions(const StringVec &_extensions, bool *_flags, const std::vector<VkExtensionProperties> &_ext_list) {
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

            bool allInstanceLayersAvailable(const StringVec &_layers) {
                std::vector<VkLayerProperties> __layer_list{Func::EnumerateInstanceLayerProperties()};
                return allLayersAvailable(_layers, __layer_list);
            }

            bool allInstanceExtAvailable(const StringVec &_extensions, const char *_layer) {
                std::vector<VkExtensionProperties> __ext_list{Func::EnumerateInstanceExtensionProperties(_layer)};
                return allExtAvailable(_extensions, __ext_list);
            }

            bool allDeviceLayersAvailable(VkPhysicalDevice _phys_device, const StringVec &_layers) {
                std::vector<VkLayerProperties> __layer_list{Func::EnumerateDeviceLayerProperties(_phys_device)};
                return allLayersAvailable(_layers, __layer_list);
            }

            bool allDeviceExtAvailable(VkPhysicalDevice _phys_device, const StringVec &_extensions, const char *_layer) {
                std::vector<VkExtensionProperties> __ext_list{Func::EnumerateDeviceExtensionProperties(_phys_device, _layer)};
                return allExtAvailable(_extensions, __ext_list);
            }

            void checkInstanceLayers(const StringVec &_layers, bool *_flags) {
                if (_flags == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW::Util :: checkInstanceLayers",
                                    "'flags' must be a pointer to an array of 'layers.size()' boolean values"));

                std::vector<VkLayerProperties> __layer_list{Func::EnumerateInstanceLayerProperties()};

                checkLayers(_layers, _flags, __layer_list);
            }

            void checkInstanceExtensions(const StringVec &_extensions, bool *_flags, const char *_layer) {
                if (_flags == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW::Util :: checkInstanceExtensions",
                                    "'flags' must be a pointer to an array of 'extensions.size()' boolean values"));

                std::vector<VkExtensionProperties> __ext_list{Func::EnumerateInstanceExtensionProperties(_layer)};

                checkExtensions(_extensions, _flags, __ext_list);
            }

            void checkDeviceLayers(VkPhysicalDevice _phys_device, const StringVec &_layers, bool *_flags) {
                if (_flags == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW::Util :: checkDeviceLayers",
                                    "'flags' must be a pointer to an array of 'layers.size()' boolean values"));

                std::vector<VkLayerProperties> __layer_list{Func::EnumerateDeviceLayerProperties(_phys_device)};

                checkLayers(_layers, _flags, __layer_list);
            }

            void checkDeviceExtensions(VkPhysicalDevice _phys_device, const StringVec &_extensions, bool *_flags, const char *_layer) {
                if (_flags == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW::Util :: checkDeviceExtensions",
                                    "'flags' must be a pointer to an array of 'extensions.size()' boolean values"));

                std::vector<VkExtensionProperties> __ext_list{Func::EnumerateDeviceExtensionProperties(_phys_device, _layer)};

                checkExtensions(_extensions, _flags, __ext_list);
            }

            uint32_t checkPhysicalDeviceMemoryProperties(VkPhysicalDevice _phys_device, uint32_t _req_type_properties,
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

            VkFormat pickSupportedFormat(VkPhysicalDevice _phys_device, Span<VkFormat> _candidates, VkFormatFeatureFlags _features,
                                         VkImageTiling _tiling) {
                for (uint32_t i = 0u; i < _candidates.size(); ++i) {
                    VkFormatProperties __props;
                    vkGetPhysicalDeviceFormatProperties(_phys_device, _candidates[i], &__props);
                    const VkFormatFeatureFlags __have = _tiling == VK_IMAGE_TILING_LINEAR ? __props.linearTilingFeatures : __props.optimalTilingFeatures;
                    if ((__have & _features) == _features) return _candidates[i];
                }
                return VK_FORMAT_UNDEFINED;
            }

            VkFormat pickDepthStencilFormat(VkPhysicalDevice _phys_device, VkFormatFeatureFlags _features) {
                const VkFormat __candidates[] = { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT };
                const VkFormat __format = pickSupportedFormat(_phys_device, __candidates, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | _features);
                SVKFW_ASSERT(__format != VK_FORMAT_UNDEFINED, std::runtime_error, "VKFW::Util :: pickDepthStencilFormat",
                                "the device has no depth/stencil attachment format with the requested features");
                return __format;
            }

            // TODO: make pickColorFormat later

            VkStencilOpState stencilOpState(VkCompareOp _compare_op, VkStencilOp _pass_op, VkStencilOp _fail_op, VkStencilOp _depth_fail_op,
                                            uint32_t _reference, uint32_t _compare_mask, uint32_t _write_mask) {
                VkStencilOpState __state{};
                __state.failOp      = _fail_op;
                __state.passOp      = _pass_op;
                __state.depthFailOp = _depth_fail_op;
                __state.compareOp   = _compare_op;
                __state.compareMask = _compare_mask;
                __state.writeMask   = _write_mask;
                __state.reference   = _reference;
                return __state;
            }

            PhysDeviceFullInfo::PhysDeviceFullInfo(const VkPhysicalDevice &_phys_device,
                               const DeviceFeatures   &_features,
                               const DeviceProperties &_properties)
                            : phys_device{_phys_device}, features{_features.head().features}, properties{_properties.head().properties},
                              features2{_features}, properties2{_properties} {}

            PhysDeviceFullInfo::PhysDeviceFullInfo(const PhysDeviceFullInfo &_elem_view)
                            : phys_device{_elem_view.phys_device}, features{_elem_view.features},
                              properties{_elem_view.properties}, features2{_elem_view.features2}, properties2{_elem_view.properties2} {}

            vec4u PhysDeviceFullInfo::apiVersion() const {
                vec4u __ver;

                __ver["xyz"] = VulkanAPIVersion::split(properties.apiVersion, &__ver.w);
                return __ver;
            }

            std::string PhysDeviceFullInfo::infoString() const {
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

            std::vector<VkExtensionProperties> PhysDeviceFullInfo::getAllDeviceExtensions(const char *_layer) const {
                return Func::EnumerateDeviceExtensionProperties(phys_device, _layer);
            }

            bool PhysDeviceFullInfo::allExtAvailable(const StringVec &_extensions) const {
                return Util::allDeviceExtAvailable(phys_device, _extensions);
            }

            void PhysDeviceFullInfo::checkExtensions(const StringVec &_extensions, bool *_flags) const {
                Util::checkDeviceExtensions(phys_device, _extensions, _flags);
            }

            namespace SelectPhysDevice {

                bool DefaultGraphicsTest(const PhysDeviceFullInfo &_phys_device_info) {
                    return _phys_device_info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                           _phys_device_info.allExtAvailable({ {VK_KHR_SWAPCHAIN_EXTENSION_NAME} });
                }
            }; // SelectPhysDevice END


            PhysDeviceManager::PhysDeviceManager(VkInstance _instance) {
                if (_instance != VK_NULL_HANDLE) loadInfo(_instance);
            }

            void PhysDeviceManager::clear() {
                features.clear();
                properties.clear();
                phys_devices.clear();
            }

            VkPhysicalDevice PhysDeviceManager::selectOne() {
                return selection_by_rank ? rankPickOne(rank_func) : testPickOne(test_func);
            }

            void PhysDeviceManager::selectWithTest(PhysDeviceTestFunc _test_func) {
                test_func = _test_func;
                selection_by_rank = false;
            }

            void PhysDeviceManager::selectWithRank(PhysDeviceRankFunc _rank_func) {
                rank_func = _rank_func;
                selection_by_rank = true;
            }

            const PhysDeviceFullInfo PhysDeviceManager::getElemInfo(uint32_t _index) { return { phys_devices[_index], features[_index], properties[_index] }; }

            void PhysDeviceManager::loadInfo(VkInstance _instance, uint32_t _api_version) {
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
                    // Note: the structures of newer versions than the device's must not be chained
                    const uint32_t __api_version = std::min(_api_version, Func::GetPhysicalDeviceProperties(phys_devices[i]).apiVersion);
                    Detail::unlinkNewerVersions(properties[i], __api_version);
                    Detail::unlinkNewerVersions(  features[i], __api_version);
                    vkGetPhysicalDeviceProperties2(phys_devices[i], &properties[i].head());
                    vkGetPhysicalDeviceFeatures2  (phys_devices[i], &  features[i].head());
                }
            }

            uint32_t PhysDeviceManager::indexOf(VkPhysicalDevice _phys_device) const {
                for (uint32_t i = 0u; i < phys_devices.size(); ++i)
                    if (phys_devices[i] == _phys_device) return i;
                throw std::out_of_range(SVKFW_WRAPERR("VKFW::Util::PhysDeviceManager :: indexOf", "the physical device is not loaded"));
            }

            VkPhysicalDevice PhysDeviceManager::testPickOne(const PhysDeviceTestFunc _testFunc) {
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

            std::vector<VkPhysicalDevice> PhysDeviceManager::testPickAll(const PhysDeviceTestFunc _testFunc) {
                std::vector<VkPhysicalDevice> __res_devices;
                for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                    if (_testFunc({phys_devices[i], features[i], properties[i]}))
                        __res_devices.push_back(phys_devices[i]);
                }

                return __res_devices;
            }

            VkPhysicalDevice PhysDeviceManager::rankPickOne(const PhysDeviceRankFunc _rankFunc) {
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

            QueueFamilyFullInfo::QueueFamilyFullInfo(const VkQueueFamilyProperties &_props) : properties{_props} {}

            bool QueueFamilyFullInfo::supportsGraphics()        const { return properties.queueFlags &  VK_QUEUE_GRAPHICS_BIT;         }

            bool QueueFamilyFullInfo::supportsCompute()         const { return properties.queueFlags &  VK_QUEUE_COMPUTE_BIT;          }

            bool QueueFamilyFullInfo::supportsTransfer()        const { return properties.queueFlags & (VK_QUEUE_GRAPHICS_BIT |
                                                                                   VK_QUEUE_COMPUTE_BIT  |
                                                                                   VK_QUEUE_TRANSFER_BIT);        }

            bool QueueFamilyFullInfo::supportsSparseBinding()   const { return properties.queueFlags &  VK_QUEUE_SPARSE_BINDING_BIT;   }

            bool QueueFamilyFullInfo::supportsProtectedMemory() const { return properties.queueFlags &  VK_QUEUE_PROTECTED_BIT;        }

            bool QueueFamilyFullInfo::supportsVideoDecode()     const { return properties.queueFlags &  VK_QUEUE_VIDEO_DECODE_BIT_KHR; }

            bool QueueFamilyFullInfo::supportsVideoEncode()     const { return properties.queueFlags &  VK_QUEUE_VIDEO_ENCODE_BIT_KHR; }

            bool QueueFamilyFullInfo::supportsOpticalFlow()     const { return properties.queueFlags &  VK_QUEUE_OPTICAL_FLOW_BIT_NV;  }

            uint32_t QueueFamilyFullInfo::queuesNum()                const { return   properties.queueCount; }

            uint32_t QueueFamilyFullInfo::timestampValidRangeBits()  const { return   properties.timestampValidBits; }

            vec3u QueueFamilyFullInfo::minImageTransferGranularity() const { return { properties.minImageTransferGranularity.width,
                                                                 properties.minImageTransferGranularity.height,
                                                                 properties.minImageTransferGranularity.depth };  }

            std::string QueueFamilyFullInfo::infoString() const {
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

            namespace SelectQueueFamily {

                bool testGraphics(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsGraphics(); }

                bool testCompute (const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsCompute (); }

                bool testTransfer(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsTransfer(); }

                bool testSparseBinding(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsSparseBinding(); }

                bool testProtectedMemory(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsProtectedMemory(); }

                bool testVideoDecode(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsVideoDecode(); }

                bool testVideoEncode(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsVideoEncode(); }

                bool testOpticalFlow(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _info.supportsOpticalFlow(); }

                bool testSurfAndGraphics(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsGraphics(); }

                bool testSurfAndCompute (const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsCompute (); }

                bool testSurfAndTransfer(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsTransfer(); }

                bool testSurfAndSparseBinding(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsSparseBinding(); }

                bool testSurfAndProtectedMemory(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsProtectedMemory(); }

                bool testSurfAndVideoDecode(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsVideoDecode(); }

                bool testSurfAndVideoEncode(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsVideoEncode(); }

                bool testSurfAndOpticalFlow(const QueueFamilyFullInfo &_info, bool _surf_support)
                    { return _surf_support && _info.supportsOpticalFlow(); }

                int rankGraphics(const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 5 * _info.supportsGraphics()
                             - _info.supportsCompute()
                             - _info.supportsVideoDecode()
                             - _info.supportsVideoEncode()
                             - _info.supportsOpticalFlow();
                }

                int rankCompute (const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 5 * _info.supportsCompute()
                             - _info.supportsGraphics()
                             - _info.supportsVideoDecode()
                             - _info.supportsVideoEncode()
                             - _info.supportsOpticalFlow();
                }

                int rankTransfer(const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 6 * _info.supportsTransfer()
                             - _info.supportsCompute()
                             - _info.supportsGraphics()
                             - _info.supportsVideoDecode()
                             - _info.supportsVideoEncode()
                             - _info.supportsOpticalFlow();
                }

                int rankSparseBinding  (const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 6 * _info.supportsSparseBinding()
                             - _info.supportsCompute()
                             - _info.supportsGraphics()
                             - _info.supportsVideoDecode()
                             - _info.supportsVideoEncode()
                             - _info.supportsOpticalFlow();
                }

                int rankProtectedMemory(const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return _info.supportsProtectedMemory() ? 1 : -1;
                }

                int rankVideoDecode(const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 5 * _info.supportsVideoDecode()
                             - _info.supportsGraphics()
                             - _info.supportsCompute()
                             - _info.supportsVideoEncode()
                             - _info.supportsOpticalFlow();
                }

                int rankVideoEncode(const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 5 * _info.supportsVideoEncode()
                             - _info.supportsGraphics()
                             - _info.supportsCompute()
                             - _info.supportsVideoDecode()
                             - _info.supportsOpticalFlow();
                }

                int rankOpticalFlow(const QueueFamilyFullInfo &_info, bool _surf_support) {
                    return 5 * _info.supportsOpticalFlow()
                             - _info.supportsGraphics()
                             - _info.supportsCompute()
                             - _info.supportsVideoDecode()
                             - _info.supportsVideoEncode();
                }
            }; // SelectQueueFamily END


            uint32_t QueueFamilyManager::selectOne() {
                return selection_by_rank ? rankPickOne(rank_func) : testPickOne(test_func);
            }

            void QueueFamilyManager::selectWithTest(QueueFamilyTestFunc _test_func) {
                test_func = _test_func;
                selection_by_rank = false;
            }

            void QueueFamilyManager::selectWithRank(QueueFamilyRankFunc _rank_func) {
                rank_func = _rank_func;
                selection_by_rank = true;
            }

            QueueFamilyManager::QueueFamilyManager(VkInstance _instance, VkPhysicalDevice _phys_device) {
                if (_instance != VK_NULL_HANDLE && _phys_device != VK_NULL_HANDLE)
                    loadInfo(_instance, _phys_device);
            }

            void QueueFamilyManager::clear() {
                properties.clear();
                surface_support.clear();
            }

            const QueueFamilyFullInfo QueueFamilyManager::getElemInfo(uint32_t _index) const { return { properties[_index] }; }

            void QueueFamilyManager::loadInfo(VkInstance _instance, VkPhysicalDevice _phys_device) {
                uint32_t __qfamily_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(_phys_device, &__qfamily_count, nullptr);

                properties.resize(__qfamily_count);
                vkGetPhysicalDeviceQueueFamilyProperties(_phys_device, &__qfamily_count, properties.data());

                surface_support.resize(properties.size());
                for (uint32_t i = 0u; i < properties.size(); ++i)
                    surface_support[i] = glfwGetPhysicalDevicePresentationSupport(_instance, _phys_device, i);
            }

            uint32_t QueueFamilyManager::testPickOne(QueueFamilyTestFunc _test_func) {
                for (uint32_t i = 0u; i < properties.size(); ++i)
                    if (_test_func(properties[i], surface_support[i]))
                        return i;

                fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::QueueFamilyManager :: testPickOne", "could not find queue family\n"));
                return UINT32_MAX;
            }

            std::vector<uint32_t> QueueFamilyManager::testPickAll(QueueFamilyTestFunc _test_func) {
                std::vector<uint32_t> __indices;

                for (uint32_t i = 0u; i < properties.size(); ++i)
                    if (_test_func(properties[i], surface_support[i]))
                        __indices.push_back(i);

                if (__indices.empty())
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::QueueFamilyManager :: testPickAll", "could not find queue family\n"));
                return __indices;
            }

            uint32_t QueueFamilyManager::rankPickOne(QueueFamilyRankFunc _rank_func) {
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

            namespace SelectSurfaceFormat {

                bool DefaultFormatTest(const VkSurfaceFormatKHR &_format) {
                    return _format.format == VK_FORMAT_B8G8R8A8_SRGB && _format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR;
                }
            }; // SelectSurfaceFormat END
        }; // Util END


        namespace Struct {

            StructWrap<VkViewport>::StructWrap(const VkExtent2D &_extent, float _x, float _y, float _depthMin, float _depthMax) :
                        wrap{_x, _y, (float)_extent.width, (float)_extent.height, _depthMin, _depthMax} {}

            StructWrap<VkViewport>::StructWrap(const vec2f &_extent, const vec2f &_offset, const vec2f &_depth) :
                        wrap{_offset.x, _offset.y, _extent.x, _extent.y, _depth.x, _depth.y} {}

            StructWrap<VkViewport>::StructWrap(const StructWrap &_viewport) : wrap{_viewport.wrap} {}

            void StructWrap<VkViewport>::clear() {
                wrap = VkViewport{ 0.f, 0.f, 0.f, 0.f, 0.f, 1.f };
            }

            void StructWrap<VkViewport>::updateWrap() {}

            void StructWrap<VkViewport>::setOffset(const VkOffset2D &_offset) { wrap.x = _offset.x; wrap.y = _offset.y; }

            void StructWrap<VkViewport>::setExtent(const VkExtent2D &_extent) { wrap.width = _extent.width; wrap.height = _extent.height; }

            void StructWrap<VkViewport>::setOffset(const vec2 &_offset) { wrap.x       = _offset.x; wrap.y       = _offset.y; }

            void StructWrap<VkViewport>::setExtent(const vec2 &_extent) { wrap.width   = _extent.x; wrap.height  = _extent.y; }

            void StructWrap<VkViewport>::setDepth (const vec2 &_depth)  { wrap.minDepth = _depth.x; wrap.maxDepth = _depth.y; }

            vec2 StructWrap<VkViewport>::offset() const { return { wrap.x,        wrap.y        }; }

            vec2 StructWrap<VkViewport>::extent() const { return { wrap.width,    wrap.height   }; }

            vec2  StructWrap<VkViewport>::depth() const { return { wrap.minDepth, wrap.maxDepth }; }

            StructWrap<VkRect2D>::StructWrap() {}

            StructWrap<VkRect2D>::StructWrap(const StructWrap &_sct) : wrap{_sct.wrap} {}

            StructWrap<VkRect2D>::StructWrap(const VkExtent2D &_extent, const VkOffset2D &_offset)
                            : wrap{ {_offset.x, _offset.y}, {_extent.width, _extent.height} } {}

            StructWrap<VkRect2D>::StructWrap(const vec2u &_extent, const vec2i &_offset)
                            : wrap{ {_offset.x, _offset.y}, {_extent.x, _extent.y} } {}

            void StructWrap<VkRect2D>::clear() {
                wrap = VkRect2D{ {0, 0}, {0u, 0u} };
            }

            void StructWrap<VkRect2D>::updateWrap() {}

            void StructWrap<VkRect2D>::setOffset(const VkOffset2D &_offset) { wrap.offset.x = _offset.x; wrap.offset.y = _offset.y; }

            void StructWrap<VkRect2D>::setExtent(const VkExtent2D &_extent) { wrap.extent.width = _extent.width; wrap.extent.height = _extent.height; }

            void StructWrap<VkRect2D>::setOffset(const vec2i &_offset) { wrap.offset.x     = _offset.x; wrap.offset.y      = _offset.y; }

            void StructWrap<VkRect2D>::setExtent(const vec2u &_extent) { wrap.extent.width = _extent.x; wrap.extent.height = _extent.y; }

            vec2i StructWrap<VkRect2D>::offset() const { return { wrap.offset.x,     wrap.offset.y      }; }

            vec2u StructWrap<VkRect2D>::extent() const { return { wrap.extent.width, wrap.extent.height }; }

            StructWrap<VkClearValue>::StructWrap() {}

            StructWrap<VkClearValue>::StructWrap(float    _v0, float    _v1, float    _v2, float    _v3) { fromColorVec4f({_v0, _v1, _v2, _v3}); }

            StructWrap<VkClearValue>::StructWrap(int32_t  _v0, int32_t  _v1, int32_t  _v2, int32_t  _v3) { fromColorVec4i({_v0, _v1, _v2, _v3}); }

            StructWrap<VkClearValue>::StructWrap(uint32_t _v0, uint32_t _v1, uint32_t _v2, uint32_t _v3) { fromColorVec4u({_v0, _v1, _v2, _v3}); }

            StructWrap<VkClearValue>::StructWrap(float _depth, uint32_t _stencil) { fromDepthStencil(_depth, _stencil); }

            void StructWrap<VkClearValue>::clear() {
                wrap = VkClearValue{};
            }

            void StructWrap<VkClearValue>::updateWrap() {}

            void StructWrap<VkClearValue>::fromColorVec4f(const vec4f &_col) {
                wrap.color.float32[0] = _col.x;
                wrap.color.float32[1] = _col.y;
                wrap.color.float32[2] = _col.z;
                wrap.color.float32[3] = _col.w;
            }

            void StructWrap<VkClearValue>::fromColorVec4i(const vec4i &_col) {
                wrap.color.int32[0] = _col.x;
                wrap.color.int32[1] = _col.y;
                wrap.color.int32[2] = _col.z;
                wrap.color.int32[3] = _col.w;
            }

            void StructWrap<VkClearValue>::fromColorVec4u(const vec4u &_col) {
                wrap.color.uint32[0] = _col.x;
                wrap.color.uint32[1] = _col.y;
                wrap.color.uint32[2] = _col.z;
                wrap.color.uint32[3] = _col.w;
            }

            void StructWrap<VkClearValue>::fromDepthStencil(float _depth, uint32_t _stencil) {
                wrap = VkClearValue{};
                wrap.depthStencil.depth   = _depth;
                wrap.depthStencil.stencil = _stencil;
            }

            VkClearValue StructWrap<VkClearValue>::ColorVec4f(const vec4f &_col) {
                VkClearValue __res{};
                __res.color.float32[0] = _col.x;
                __res.color.float32[1] = _col.y;
                __res.color.float32[2] = _col.z;
                __res.color.float32[3] = _col.w;
                return __res;
            }

            VkClearValue StructWrap<VkClearValue>::ColorVec4i(const vec4i &_col) {
                VkClearValue __res{};
                __res.color.int32[0] = _col.x;
                __res.color.int32[1] = _col.y;
                __res.color.int32[2] = _col.z;
                __res.color.int32[3] = _col.w;
                return __res;
            }

            VkClearValue StructWrap<VkClearValue>::ColorVec4u(const vec4u &_col) {
                VkClearValue __res{};
                __res.color.uint32[0] = _col.x;
                __res.color.uint32[1] = _col.y;
                __res.color.uint32[2] = _col.z;
                __res.color.uint32[3] = _col.w;
                return __res;
            }

            VkClearValue StructWrap<VkClearValue>::DepthStencil(float _depth, uint32_t _stencil) {
                VkClearValue __res{};
                __res.depthStencil.depth   = _depth;
                __res.depthStencil.stencil = _stencil;
                return __res;
            }

            StructWrap<VkAttachmentDescription>::StructWrap() {}

            StructWrap<VkAttachmentDescription>::StructWrap(const StructWrap &_sct) : wrap{_sct.wrap} {}

            StructWrap<VkAttachmentDescription>::StructWrap(const VkAttachmentDescription &_description) : wrap{_description} {}

            void StructWrap<VkAttachmentDescription>::clear() {
                wrap = VkAttachmentDescription{};
            }

            void StructWrap<VkAttachmentDescription>::updateWrap() {}

            void StructWrap<VkAttachmentDescription>::setColorOps(VkAttachmentLoadOp _loadOp, VkAttachmentStoreOp _storeOp) {
                wrap.loadOp = _loadOp;
                wrap.storeOp = _storeOp;
            }

            void StructWrap<VkAttachmentDescription>::setStencilOps(VkAttachmentLoadOp _loadOp, VkAttachmentStoreOp _storeOp) {
                wrap.stencilLoadOp = _loadOp;
                wrap.stencilStoreOp = _storeOp;
            }

            void StructWrap<VkAttachmentDescription>::setLayouts(VkImageLayout _initLayout, VkImageLayout _finalLayout) {
                wrap.initialLayout = _initLayout;
                wrap.finalLayout = _finalLayout;
            }

            void StructWrap<VkAttachmentDescription>::setFormatAndSize(VkFormat _format, VkSampleCountFlagBits _samples) {
                wrap.format = _format;
                wrap.samples = _samples;
            }

            StructWrap<VkSubpassDescription>::StructWrap() { clear(); }

            void StructWrap<VkSubpassDescription>::clear() {
                wrap = VkSubpassDescription{};
                depth_stencil_attachment = VkAttachmentReference{VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED};

                color_attachments   .clear();
                input_attachments   .clear();
                resolve_attachments .clear();
                preserve_attachments.clear();
            }

            void StructWrap<VkSubpassDescription>::updateWrap() {
                wrap.    colorAttachmentCount =    color_attachments.size();
                wrap.   pColorAttachments     =    color_attachments.data();
                wrap.    inputAttachmentCount =    input_attachments.size();
                wrap.   pInputAttachments     =    input_attachments.data();
                wrap. preserveAttachmentCount = preserve_attachments.size();
                wrap.pPreserveAttachments     = preserve_attachments.data();
                wrap. pResolveAttachments     =  resolve_attachments.empty() ? nullptr : resolve_attachments.data();
                wrap.pDepthStencilAttachment  = depth_stencil_attachment.attachment != VK_ATTACHMENT_UNUSED ? &depth_stencil_attachment : nullptr;
            }

            void StructWrap<VkSubpassDescription>::setDepthStencilAttachment(uint32_t _attachment_id, VkImageLayout _layout) {
                depth_stencil_attachment.attachment = _attachment_id;
                depth_stencil_attachment.layout     = _layout;
            }

            ContextIndex StructWrap<VkSubpassDescription>::addColorAttachment(uint32_t _attachment_id, VkImageLayout _layout) {
                color_attachments.push_back(VkAttachmentReference{_attachment_id, _layout});
                return color_attachments.size()-1;
            }

            ContextIndex StructWrap<VkSubpassDescription>::addInputAttachment(uint32_t _attachment_id, VkImageLayout _layout) {
                input_attachments.push_back(VkAttachmentReference{_attachment_id, _layout});
                return input_attachments.size()-1;
            }

            ContextIndex StructWrap<VkSubpassDescription>::addResolveAttachment(uint32_t _attachment_id, VkImageLayout _layout) {
                resolve_attachments.push_back(VkAttachmentReference{_attachment_id, _layout});
                return resolve_attachments.size()-1;
            }

            StructWrap<VkSubpassDependency>::StructWrap() {}

            StructWrap<VkSubpassDependency>::StructWrap(const StructWrap &_sct) : wrap{_sct.wrap} {}

            StructWrap<VkSubpassDependency>::StructWrap(const VkSubpassDependency &_subpass_dependency) : wrap{_subpass_dependency} {}

            void StructWrap<VkSubpassDependency>::clear() {
                wrap = VkSubpassDependency{};
            }

            void StructWrap<VkSubpassDependency>::updateWrap() {}

            void StructWrap<VkSubpassDependency>::setSubpasses(uint32_t _src_subpass, uint32_t _dst_subpass) {
                wrap.srcSubpass = _src_subpass;
                wrap.dstSubpass = _dst_subpass;
            }

            void StructWrap<VkSubpassDependency>::setSrcMasks(VkPipelineStageFlags _src_stage_mask, VkAccessFlags _src_acc_mask) {
                wrap.srcStageMask  = _src_stage_mask;
                wrap.srcAccessMask =   _src_acc_mask;
            }

            void StructWrap<VkSubpassDependency>::setDstMasks(VkPipelineStageFlags _dst_stage_mask, VkAccessFlags _dst_acc_mask) {
                wrap. dstStageMask = _dst_stage_mask;
                wrap.dstAccessMask =   _dst_acc_mask;
            }

            StructWrap<VkSpecializationInfo>::StructWrap() {}

            void StructWrap<VkSpecializationInfo>::clear() {
                wrap = VkSpecializationInfo{};
                map_entries.clear();
                data.clear();
            }

            void StructWrap<VkSpecializationInfo>::updateWrap() {
                wrap.mapEntryCount = map_entries.size();
                wrap.pMapEntries   = map_entries.data();
                wrap.dataSize = data.size();
                wrap.pData    = data.data();
            }

            void StructWrap<VkSpecializationInfo>::setData(const void* _data, size_t _data_size) {
                data.resize(_data_size);
                memcpy(data.data(), _data, _data_size);
            }

            StructWrap<VkDescriptorSetLayoutBinding>::StructWrap() {}

            void StructWrap<VkDescriptorSetLayoutBinding>::clear() {
                wrap = VkDescriptorSetLayoutBinding{};
                immutable_samplers.clear();
            }

            void StructWrap<VkDescriptorSetLayoutBinding>::updateWrap() {
                wrap.descriptorCount    = immutable_samplers.size() ? immutable_samplers.size() : wrap.descriptorCount;
                wrap.pImmutableSamplers = immutable_samplers.size() ? immutable_samplers.data() : nullptr;
            }

            void StructWrap<VkDescriptorSetLayoutBinding>::setWholeStruct(uint32_t _binding, VkDescriptorType _descriptor_type, uint32_t _descriptor_count,
                                VkShaderStageFlags _stage_flags, const std::vector<VkSampler> &_immutable_samplers) {
                wrap.binding = _binding;
                wrap.descriptorType = _descriptor_type;
                wrap.descriptorCount = _descriptor_count;
                wrap.stageFlags = _stage_flags;
                immutable_samplers = _immutable_samplers;
                updateWrap();
            }

            StructWrap<VkApplicationInfo>::StructWrap() { clear(); }

            void StructWrap<VkApplicationInfo>::clear() {
                wrap = VkApplicationInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
                app_name    = "SimpleVKFW_MainApp";
                engine_name = "SimpleVKFW";
                app_version = {0u, 0u, 1u};
                engine_version = {0u, 1u, 0u};
                setAvailableVersionAPI();
            }

            void StructWrap<VkApplicationInfo>::updateWrap() {
                wrap.pApplicationName = app_name.c_str();
                wrap.applicationVersion = VulkanAPIVersion::join(app_version);
                wrap.pEngineName = engine_name.c_str();
                wrap.engineVersion = VulkanAPIVersion::join(engine_version);
                wrap.apiVersion = VulkanAPIVersion::join(api_version["xyz"], api_version.w);
            }

            void StructWrap<VkApplicationInfo>::setAvailableVersionAPI() {
                api_version["xyz"] = VulkanAPIVersion::split(VulkanAPIVersion::available(), &api_version.w);
            }

            void StructWrap<VkApplicationInfo>::setWholeStruct(const std::string &   _app_name, const vec3u &   _app_version,
                                const std::string &_engine_name,         const vec3u &_engine_version, const vec4u &_api_version) {
                api_version = _api_version;
                if ((_api_version == 0u).all())  setAvailableVersionAPI();

                engine_version = _engine_version;
                app_version = _app_version;
                engine_name = _engine_name;
                app_name = _app_name;
            }

            StructWrap<VkInstanceCreateInfo>::StructWrap() {}

            void StructWrap<VkInstanceCreateInfo>::clear() {
                wrap = VkInstanceCreateInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};

                layers.clear();
                extensions.clear();

                app_info_wrap.clear();
            }

            void StructWrap<VkInstanceCreateInfo>::updateWrap() {
                wrap.pApplicationInfo = &app_info_wrap.get();
                wrap.enabledLayerCount = layers.count();
                wrap.ppEnabledLayerNames = layers.getEntries();
                wrap.enabledExtensionCount = extensions.count();
                wrap.ppEnabledExtensionNames = extensions.getEntries();
            }

            void StructWrap<VkInstanceCreateInfo>::addLayers(const Util::StringVec &_layers) {
                layers.addEntries(_layers.list);
            }

            void StructWrap<VkInstanceCreateInfo>::addExtensions(const Util::StringVec &_extensions) {
                extensions.addEntries(_extensions.list);
            }

            void StructWrap<VkInstanceCreateInfo>::addValidationLayer() {
                if (!Util::allInstanceLayersAvailable({{ "VK_LAYER_KHRONOS_validation" }}))
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW::StructWrap<VkInstanceCreateInfo> :: addValidationLayer","Validation layer unavailable"));

                addLayers({{ "VK_LAYER_KHRONOS_validation" }});
            }

            void StructWrap<VkInstanceCreateInfo>::addExtGLFW() { // Vulkan + GLFW is the main stack for now
                uint32_t _count = 0u;
                const char **_ext = glfwGetRequiredInstanceExtensions(&_count);
                addExtensions({_ext, _count});
            }

            void StructWrap<VkInstanceCreateInfo>::addExtDebugUtils() {
                if (!Util::allInstanceExtAvailable({{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME }}))
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW::StructWrap<VkInstanceCreateInfo> :: addExtDebugUtils","Debug message callback extension is unavailable"));

                addExtensions({{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME }});
            }

            StructWrap<VkDebugUtilsMessengerCreateInfoEXT>::StructWrap() { clear(); }

            void StructWrap<VkDebugUtilsMessengerCreateInfoEXT>::clear() {
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

            void StructWrap<VkDebugUtilsMessengerCreateInfoEXT>::updateWrap() {}

            void StructWrap<VkDebugUtilsMessengerCreateInfoEXT>::setWholeStruct(VkDebugUtilsMessageSeverityFlagsEXT _msg_severity,
                                VkDebugUtilsMessageTypeFlagsEXT _msg_type,
                                PFN_vkDebugUtilsMessengerCallbackEXT _user_cb,
                                void* _user_data) {
                wrap.messageSeverity = _msg_severity;
                wrap.messageType     = _msg_type;
                wrap.pfnUserCallback = _user_cb != nullptr ? _user_cb : Util::vkfwDebugMessageCallback;
                wrap.pUserData       = _user_data;
            }

            StructWrap<VkDeviceQueueCreateInfo>::StructWrap() {}

            void StructWrap<VkDeviceQueueCreateInfo>::clear() {
                wrap = VkDeviceQueueCreateInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
                priorities.clear();
            }

            void StructWrap<VkDeviceQueueCreateInfo>::updateWrap() {
                wrap.queueCount       = priorities.size();
                wrap.pQueuePriorities = priorities.data();
                wrap.queueFamilyIndex = queue_family;
            }

            void StructWrap<VkDeviceQueueCreateInfo>::setQueueCount(uint32_t _queue_count, const std::vector<float> &_priorities) {
                priorities = _priorities;
                priorities.resize(_queue_count, _priorities.empty() ? 1.0f : _priorities.back());
            }

            StructWrap<VkDeviceCreateInfo>::StructWrap() {}

            void StructWrap<VkDeviceCreateInfo>::clear() {
                wrap = VkDeviceCreateInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
                features          = DeviceFeatures();
                optional_features = DeviceFeatures();
                api_version       = VK_API_VERSION_1_3;
                queue_info.clear();
                layers.clear();
                extensions.clear();
            }

            void StructWrap<VkDeviceCreateInfo>::updateWrap() {
                wrap.enabledLayerCount   = layers.count();
                wrap.ppEnabledLayerNames = layers.getEntries();
                wrap.enabledExtensionCount   = extensions.count();
                wrap.ppEnabledExtensionNames = extensions.getEntries();
                wrap.pQueueCreateInfos    = queue_info.data();
                wrap.queueCreateInfoCount = queue_info.size();

                // Note: with VkPhysicalDeviceFeatures2 in the pNext chain 'pEnabledFeatures' must be null
                features.link();
                Detail::unlinkNewerVersions(features, api_version);
                wrap.pNext            = &features.head();
                wrap.pEnabledFeatures = nullptr;
            }

            void StructWrap<VkDeviceCreateInfo>::enableOptionalFeatures(const DeviceFeatures &_supported) {
                Detail::addSupportedFeatures(features, optional_features, _supported);
            }

            std::vector<std::string> StructWrap<VkDeviceCreateInfo>::unsupportedFeatures(const DeviceFeatures &_supported) const {
                return Detail::unsupportedFeatures(features, _supported);
            }

            void StructWrap<VkDeviceCreateInfo>::addLayers(const Util::StringVec &_layers) {
                layers.addEntries(_layers.list);
            }

            void StructWrap<VkDeviceCreateInfo>::addExtensions(const Util::StringVec &_extensions) {
                extensions.addEntries(_extensions.list);
            }

            void StructWrap<VkDeviceCreateInfo>::addValidationLayer(VkPhysicalDevice _phys_device) {
                if (_phys_device != VK_NULL_HANDLE && !Util::allDeviceLayersAvailable(_phys_device, {{ "VK_LAYER_KHRONOS_validation" }}))
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW::StructWrap<VkDeviceCreateInfo> :: addValidationLayer", "Validation layer unavailable"));

                addLayers({{ "VK_LAYER_KHRONOS_validation" }});
            }

            StructWrap<VkSwapchainCreateInfoKHR>::StructWrap() {}

            void StructWrap<VkSwapchainCreateInfoKHR>::clear() {
                wrap = VkSwapchainCreateInfoKHR{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
                // wrap.imageArrayLayers = 1;
                // wrap.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
                // wrap.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                // wrap.presentMode = VK_PRESENT_MODE_FIFO_KHR;
                capabilities = VkSurfaceCapabilitiesKHR{};
                queue_families.clear();
            }

            void StructWrap<VkSwapchainCreateInfoKHR>::updateWrap() {
                const bool __concurrent = queue_families.size() > 1u;
                wrap.imageSharingMode = __concurrent ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
                wrap.queueFamilyIndexCount = __concurrent ? queue_families.size() : 0u;
                wrap.pQueueFamilyIndices   = __concurrent ? queue_families.data() : nullptr;
            }

            void StructWrap<VkSwapchainCreateInfoKHR>::setSurfaceCapabilities(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface) {
                VkResult __res_call = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_phys_device, _surface, &capabilities);

                if (__res_call != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW::StructWrap<VkSwapchainCreateInfoKHR> :: setSurfaceCapabilities", "failed, error: ") + std::to_string(__res_call));
                wrap.preTransform = capabilities.currentTransform;
            }

            void StructWrap<VkSwapchainCreateInfoKHR>::setCurrentExtent() {
                wrap.imageExtent = capabilities.currentExtent;
            }

            VkImageUsageFlags StructWrap<VkSwapchainCreateInfoKHR>::checkSetImageUsage(VkImageUsageFlags _usage) {
                VkImageUsageFlags __res = _usage & capabilities.supportedUsageFlags;

                // Note: the supported flags are set anyway, the caller checks the result for the optional ones
                wrap.imageUsage = __res;
                if (__res != _usage)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::Struct::StructWrap<VkSwapchainCreateInfoKHR> :: checkSetImageUsage",
                                                            "some usage flags are not supported\n"));
                return __res;
            }

            void StructWrap<VkSwapchainCreateInfoKHR>::checkSetImageCount(uint32_t _count) {
                wrap.minImageCount = Math::clampCL(_count,
                                                    capabilities.minImageCount,
                                                    capabilities.maxImageCount ? capabilities.maxImageCount : UINT32_MAX); // (== 0) means no max

                if (wrap.minImageCount != _count)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW::Struct::StructWrap<VkSwapchainCreateInfoKHR> :: checkSetImageCount",
                                    "image count (%d) was clamped to %d to fit the interval [%d, %d]\n"),
                                    _count, wrap.minImageCount, capabilities.minImageCount, capabilities.maxImageCount);
            }

            bool StructWrap<VkSwapchainCreateInfoKHR>::checkSetExtent(vec2u _extent) {
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

            void StructWrap<VkSwapchainCreateInfoKHR>::chooseCurrentExtent(const Window &_window) {
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

            void StructWrap<VkSwapchainCreateInfoKHR>::pickFormat(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface, const Util::SurfaceFormatTestFunc _test_func) {
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

            void StructWrap<VkSwapchainCreateInfoKHR>::pickFormat(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface, const Util::SurfaceFormatRankFunc _rank_func) {
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

            bool StructWrap<VkSwapchainCreateInfoKHR>::checkSetPresentationMode(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface, VkPresentModeKHR _pres_mode) {
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

            StructWrap<VkImageCreateInfo>::StructWrap() {}

            void StructWrap<VkImageCreateInfo>::clear() {
                wrap = VkImageCreateInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
                queue_families.clear();
            }

            void StructWrap<VkImageCreateInfo>::updateWrap() {
                wrap.queueFamilyIndexCount = queue_families.size();
                wrap.pQueueFamilyIndices = queue_families.data();
            }

            void StructWrap<VkImageCreateInfo>::setWholeStruct(const std::vector<uint32_t> &_queue_families, VkSharingMode _sharing_mode, VkImageUsageFlags _usage_flags,
                                VkFormat _format, VkExtent3D _extent, uint32_t _mip_levels, uint32_t _array_layers,
                                VkSampleCountFlagBits _samples, VkImageTiling _tiling,
                                VkImageCreateFlags _flags) {
                queue_families = _queue_families;
                wrap.flags         = _flags;
                wrap.imageType     = _extent.depth > 1u ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
                wrap.format        = _format;
                wrap.extent        = _extent;
                wrap.mipLevels     = _mip_levels;
                wrap.arrayLayers   = _array_layers;
                wrap.samples       = _samples;
                wrap.tiling        = _tiling;
                wrap.usage         = _usage_flags;
                wrap.sharingMode   = _sharing_mode;
                wrap.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                updateWrap();
            }

            StructWrap<VkImageViewCreateInfo>::StructWrap() {}

            void StructWrap<VkImageViewCreateInfo>::clear() {
                wrap = VkImageViewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            }

            void StructWrap<VkImageViewCreateInfo>::updateWrap() {}

            void StructWrap<VkImageViewCreateInfo>::setRange(VkImageAspectFlags _aspect_mask, uint32_t _base_mip_level,
                          uint32_t _level_count, uint32_t _base_array_layer, uint32_t _array_level_count) {
                wrap.subresourceRange.aspectMask = _aspect_mask;
                wrap.subresourceRange.baseMipLevel = _base_mip_level;
                wrap.subresourceRange.levelCount = _level_count;
                wrap.subresourceRange.baseArrayLayer = _base_array_layer;
                wrap.subresourceRange.layerCount = _array_level_count;
            }

            void StructWrap<VkImageViewCreateInfo>::setComponents(VkComponentSwizzle _r, VkComponentSwizzle _g,
                               VkComponentSwizzle _b, VkComponentSwizzle _a) {
                wrap.components.r = _r;
                wrap.components.g = _g;
                wrap.components.b = _b;
                wrap.components.a = _a;
            }

            StructWrap<VkBufferCreateInfo>::StructWrap() {}

            void StructWrap<VkBufferCreateInfo>::clear() {
                wrap = VkBufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
                queue_families.clear();
            }

            void StructWrap<VkBufferCreateInfo>::updateWrap() {
                wrap.queueFamilyIndexCount = queue_families.size();
                wrap.pQueueFamilyIndices   = queue_families.data();
            }

            void StructWrap<VkBufferCreateInfo>::setWholeStruct(const std::vector<VkfwQueueFamily> &_queue_families, VkSharingMode _sharing_mode,
                                VkBufferUsageFlags _usage_flags, VkDeviceSize _size_bytes) {
                queue_families = _queue_families;
                wrap.sharingMode = _sharing_mode;
                wrap.size  =  _size_bytes;
                wrap.usage = _usage_flags;
                updateWrap();
            }

            StructWrap<VkBufferViewCreateInfo>::StructWrap() {}

            void StructWrap<VkBufferViewCreateInfo>::clear() {
                wrap = VkBufferViewCreateInfo{VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO};
            }

            void StructWrap<VkBufferViewCreateInfo>::updateWrap() {}

            void StructWrap<VkBufferViewCreateInfo>::setWholeStruct(VkBuffer _buffer, VkFormat _format, VkDeviceSize _offset, VkDeviceSize _range) {
                wrap.buffer = _buffer;
                wrap.format = _format;
                wrap.offset = _offset;
                wrap. range = _range ;
            }

            StructWrap<VkMemoryAllocateInfo>::StructWrap() {}

            void StructWrap<VkMemoryAllocateInfo>::clear() {
                wrap = VkMemoryAllocateInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
            }

            void StructWrap<VkMemoryAllocateInfo>::updateWrap() {}

            void StructWrap<VkMemoryAllocateInfo>::setForBuffer(VkPhysicalDevice _phys_device, VkDevice _device, VkBuffer _buffer,
                              VkMemoryPropertyFlags _req_memory_properties) {
                VkMemoryRequirements __mem_requirements = Func::GetBufferMemoryRequirements(_device, _buffer);

                wrap.allocationSize = __mem_requirements.size;
                wrap.memoryTypeIndex = Util::checkPhysicalDeviceMemoryProperties(_phys_device, __mem_requirements.memoryTypeBits, _req_memory_properties);

                if (wrap.memoryTypeIndex == UINT32_MAX)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW::Struct::StructWrap<VkMemoryAllocateInfo> :: setForBuffer", "Could not find suitable memory type"));
            }

            void StructWrap<VkMemoryAllocateInfo>::setForImage(VkPhysicalDevice _phys_device, VkDevice _device, VkImage _image,
                              VkMemoryPropertyFlags _req_memory_properties) {
                VkMemoryRequirements __mem_requirements = Func::GetImageMemoryRequirements(_device, _image);

                wrap.allocationSize = __mem_requirements.size;
                wrap.memoryTypeIndex = Util::checkPhysicalDeviceMemoryProperties(_phys_device, __mem_requirements.memoryTypeBits, _req_memory_properties);

                if (wrap.memoryTypeIndex == UINT32_MAX)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW::Struct::StructWrap<VkMemoryAllocateInfo> :: setForImage", "Could not find suitable memory type"));
            }

            StructWrap<VkSamplerCreateInfo>::StructWrap() { clear(); }

            void StructWrap<VkSamplerCreateInfo>::clear() {
                wrap = VkSamplerCreateInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
                setWholeStruct();
            }

            void StructWrap<VkSamplerCreateInfo>::updateWrap() {}

            void StructWrap<VkSamplerCreateInfo>::setFilter(VkFilter _mag_filter, VkFilter _min_filter, VkSamplerMipmapMode _mipmap_mode) {
                wrap.magFilter  = _mag_filter;
                wrap.minFilter  = _min_filter;
                wrap.mipmapMode = _mipmap_mode;
            }

            void StructWrap<VkSamplerCreateInfo>::setAddressMode(VkSamplerAddressMode _u, VkSamplerAddressMode _v, VkSamplerAddressMode _w,
                                                                 VkBorderColor _border_color) {
                wrap.addressModeU = _u;
                wrap.addressModeV = _v == VK_SAMPLER_ADDRESS_MODE_MAX_ENUM ? _u : _v;
                wrap.addressModeW = _w == VK_SAMPLER_ADDRESS_MODE_MAX_ENUM ? _u : _w;
                wrap.borderColor  = _border_color;
            }

            void StructWrap<VkSamplerCreateInfo>::setAnisotropy(float _max_anisotropy) {
                wrap.anisotropyEnable = _max_anisotropy > 1.f ? VK_TRUE : VK_FALSE;
                wrap.maxAnisotropy    = _max_anisotropy > 1.f ? _max_anisotropy : 1.f;
            }

            void StructWrap<VkSamplerCreateInfo>::setLod(float _min_lod, float _max_lod, float _mip_lod_bias) {
                wrap.minLod     = _min_lod;
                wrap.maxLod     = _max_lod;
                wrap.mipLodBias = _mip_lod_bias;
            }

            void StructWrap<VkSamplerCreateInfo>::setCompare(VkCompareOp _compare_op) {
                wrap.compareEnable = _compare_op == VK_COMPARE_OP_MAX_ENUM ? VK_FALSE : VK_TRUE;
                wrap.compareOp     = _compare_op == VK_COMPARE_OP_MAX_ENUM ? VK_COMPARE_OP_NEVER : _compare_op;
            }

            void StructWrap<VkSamplerCreateInfo>::setWholeStruct(VkFilter _filter, VkSamplerAddressMode _address_mode,
                                                                 float _max_anisotropy, float _max_lod) {
                setFilter(_filter, _filter, _filter == VK_FILTER_LINEAR ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST);
                setAddressMode(_address_mode);
                setAnisotropy(_max_anisotropy);
                setLod(0.f, _max_lod);
                setCompare(VK_COMPARE_OP_MAX_ENUM);
            }

            StructWrap<VkShaderModuleCreateInfo>::StructWrap() {}

            void StructWrap<VkShaderModuleCreateInfo>::clear() {
                wrap = VkShaderModuleCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
                buffer.clear();
            }

            void StructWrap<VkShaderModuleCreateInfo>::updateWrap() {
                wrap.codeSize = buffer.size();
                wrap.pCode = reinterpret_cast<const uint32_t *>(buffer.data());
            }

            void StructWrap<VkShaderModuleCreateInfo>::fromFile(const char *_fpath) {
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

            void StructWrap<VkShaderModuleCreateInfo>::fromString(const char *_sh_str, size_t _length) {
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

            void StructWrap<VkShaderModuleCreateInfo>::fromString(const std::string &_str_buffer) {
                if (_str_buffer.length() == 0u)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW::Struct::StructWrap<VkShaderModuleCreateInfo> :: fromString (2)", "string is empty"));

                buffer.clear();
                buffer.insert(buffer.end(), _str_buffer.begin(), _str_buffer.end());
            }

            StructWrap<VkRenderPassCreateInfo>::StructWrap() {}

            void StructWrap<VkRenderPassCreateInfo>::clear() {
                attachments.clear();
                subpasses.clear();
                dependencies.clear();
                attachments_wrap.clear();
                subpasses_wrap.clear();
                dependencies_wrap.clear();
            }

            void StructWrap<VkRenderPassCreateInfo>::updateWrap() {
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

            ContextIndex StructWrap<VkRenderPassCreateInfo>::addAttachment()         {  attachments_wrap.emplace_back(); return  attachments_wrap.size()-1; }

            ContextIndex StructWrap<VkRenderPassCreateInfo>::addSubpassDescription() {    subpasses_wrap.emplace_back(); return    subpasses_wrap.size()-1; }

            ContextIndex StructWrap<VkRenderPassCreateInfo>::addSubpassDependency()  { dependencies_wrap.emplace_back(); return dependencies_wrap.size()-1; }

            ContextEntity<StructWrap<VkAttachmentDescription>> StructWrap<VkRenderPassCreateInfo>::shortcutAttachment        (ContextIndex _sct_ci) { return {  attachments_wrap.data() + _sct_ci, _sct_ci}; }

            ContextEntity<StructWrap<   VkSubpassDescription>> StructWrap<VkRenderPassCreateInfo>::shortcutSubpassDescription(ContextIndex _sct_ci) { return {    subpasses_wrap.data() + _sct_ci, _sct_ci}; }

            ContextEntity<StructWrap<    VkSubpassDependency>> StructWrap<VkRenderPassCreateInfo>::shortcutSubpassDependency (ContextIndex _sct_ci) { return { dependencies_wrap.data() + _sct_ci, _sct_ci}; }

            StructWrap<VkFramebufferCreateInfo>::StructWrap() {}

            void StructWrap<VkFramebufferCreateInfo>::clear() {
                wrap = VkFramebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
                attachments.clear();
            }

            void StructWrap<VkFramebufferCreateInfo>::updateWrap() {
                wrap.attachmentCount = attachments.size();
                wrap.pAttachments    = attachments.data();
            }

            void StructWrap<VkFramebufferCreateInfo>::setBufferSize(VkExtent2D _buffer_size) {
                wrap. width = _buffer_size. width;
                wrap.height = _buffer_size.height;
            }

            void StructWrap<VkFramebufferCreateInfo>::setBufferSize(vec2u _buffer_size) {
                wrap. width = _buffer_size.x;
                wrap.height = _buffer_size.y;
            }

            void StructWrap<VkFramebufferCreateInfo>::setBufferSizeAndLayers(VkExtent2D _buffer_size, uint32_t _layers) {
                wrap. width = _buffer_size. width;
                wrap.height = _buffer_size.height;
                wrap.layers = _layers;
            }

            void StructWrap<VkFramebufferCreateInfo>::setBufferSizeAndLayers(vec3u _buf_size_layers) {
                wrap. width = _buf_size_layers.x;
                wrap.height = _buf_size_layers.y;
                wrap.layers = _buf_size_layers.z;
            }

            StructWrap<VkSemaphoreCreateInfo>::StructWrap() {}

            void StructWrap<VkSemaphoreCreateInfo>::clear() {
                wrap = VkSemaphoreCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            }

            void StructWrap<VkSemaphoreCreateInfo>::updateWrap() {}

            StructWrap<VkFenceCreateInfo>::StructWrap() {}

            void StructWrap<VkFenceCreateInfo>::clear() {
                wrap = VkFenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            }

            void StructWrap<VkFenceCreateInfo>::updateWrap() {}

            void StructWrap<VkFenceCreateInfo>::setWholeStruct(bool _is_signaled) {
                wrap.flags = _is_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : VkFenceCreateFlags(0u);
                updateWrap();
            }

            StructWrap<VkEventCreateInfo>::StructWrap() {}

            void StructWrap<VkEventCreateInfo>::clear() {
                wrap = VkEventCreateInfo{VK_STRUCTURE_TYPE_EVENT_CREATE_INFO};
            }

            void StructWrap<VkEventCreateInfo>::updateWrap() {}

            void StructWrap<VkEventCreateInfo>::setWholeStruct(bool _is_device_only) {
                wrap.flags = _is_device_only ? VK_EVENT_CREATE_DEVICE_ONLY_BIT : VkEventCreateFlags(0u);
                updateWrap();
            }

            StructWrap<VkWriteDescriptorSet>::StructWrap() {}

            void StructWrap<VkWriteDescriptorSet>::clear() {
                wrap = VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                descriptor_count = 0u;

                buffer_infos.clear();
                image_infos.clear();
                texel_buf_views.clear();
            }

            void StructWrap<VkWriteDescriptorSet>::updateWrap() {
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

            void StructWrap<VkWriteDescriptorSet>::setScalars(VkDescriptorSet _dst_descr_set, VkDescriptorType _descr_type, uint32_t _dst_binding,
                            uint32_t _descriptor_count, uint32_t _dst_arr_elem) {
                descriptor_count     = _descriptor_count;
                wrap.dstSet          = _dst_descr_set;
                wrap.descriptorType  =    _descr_type;
                wrap.dstBinding      =   _dst_binding;
                wrap.dstArrayElement =  _dst_arr_elem;
            }

            void StructWrap<VkWriteDescriptorSet>::addBufferInfo(VkBuffer _buffer, VkDeviceSize _offset, VkDeviceSize _range) {
                buffer_infos.push_back(VkDescriptorBufferInfo{ _buffer, _offset, _range });
            }

            void StructWrap<VkWriteDescriptorSet>::addImageInfo(VkSampler _sampler, VkImageView _image_view, VkImageLayout _image_layout) {
                image_infos.push_back(VkDescriptorImageInfo{ _sampler, _image_view, _image_layout });
            }

            void StructWrap<VkWriteDescriptorSet>::addTexelBufferView(VkBufferView _buffer_view) {
                texel_buf_views.push_back(_buffer_view);
            }

            StructWrap<VkCopyDescriptorSet>::StructWrap() {}

            void StructWrap<VkCopyDescriptorSet>::clear() {
                wrap = VkCopyDescriptorSet{VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET};
            }

            void StructWrap<VkCopyDescriptorSet>::updateWrap() {}

            void StructWrap<VkCopyDescriptorSet>::setSrc(VkDescriptorSet _src_set, uint32_t _src_binding, uint32_t _src_arr_elem) {
                wrap.srcSet          = _src_set;
                wrap.srcBinding      = _src_binding;
                wrap.srcArrayElement = _src_arr_elem;
            }

            void StructWrap<VkCopyDescriptorSet>::setDst(VkDescriptorSet _dst_set, uint32_t _dst_binding, uint32_t _dst_arr_elem) {
                wrap.dstSet          = _dst_set;
                wrap.dstBinding      = _dst_binding;
                wrap.dstArrayElement = _dst_arr_elem;

            }

            StructWrap<VkDescriptorSetLayoutCreateInfo>::StructWrap() {}

            void StructWrap<VkDescriptorSetLayoutCreateInfo>::clear() {
                wrap = VkDescriptorSetLayoutCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
                bindings.clear();
            }

            ContextIndex StructWrap<VkDescriptorSetLayoutCreateInfo>::addBinding() { bindings_wrap.emplace_back(); return bindings_wrap.size()-1; }

            ContextEntity<Struct::StructWrap<VkDescriptorSetLayoutBinding>> StructWrap<VkDescriptorSetLayoutCreateInfo>::shortcutBinding(ContextIndex _sct_ci) { return {bindings_wrap.data() + _sct_ci, _sct_ci}; }

            void StructWrap<VkDescriptorSetLayoutCreateInfo>::updateWrap() {
                bindings.resize(bindings_wrap.size());
                for (uint32_t i = 0u; i < bindings.size(); ++i) {
                    bindings[i] = bindings_wrap[i].get();
                }

                wrap.bindingCount = bindings.size();
                wrap.pBindings    = bindings.data();
            }

            StructWrap<VkDescriptorPoolCreateInfo>::StructWrap() {}

            void StructWrap<VkDescriptorPoolCreateInfo>::clear() {
                wrap = VkDescriptorPoolCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
                pool_sizes.clear();
            }

            void StructWrap<VkDescriptorPoolCreateInfo>::updateWrap() {
                wrap.poolSizeCount = pool_sizes.size();
                wrap.pPoolSizes    = pool_sizes.data();
            }

            void StructWrap<VkDescriptorPoolCreateInfo>::addPoolSize(VkDescriptorType _descr_type, uint32_t _descr_count) {
                pool_sizes.push_back(VkDescriptorPoolSize{_descr_type, _descr_count});
            }

            void StructWrap<VkDescriptorPoolCreateInfo>::setWholeStruct(uint32_t _max_sets, const std::vector<VkDescriptorPoolSize> &_pool_sizes) {
                wrap.maxSets = _max_sets;
                pool_sizes = _pool_sizes;
                updateWrap();
            }

            StructWrap<VkDescriptorSetAllocateInfo>::StructWrap() {}

            void StructWrap<VkDescriptorSetAllocateInfo>::clear() {
                wrap = VkDescriptorSetAllocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
                set_layouts.clear();
            }

            void StructWrap<VkDescriptorSetAllocateInfo>::updateWrap() {
                wrap.descriptorSetCount = set_layouts.size();
                wrap.pSetLayouts        = set_layouts.data();
            }

            void StructWrap<VkDescriptorSetAllocateInfo>::setWholeStruct(VkDescriptorPool _descr_pool, const std::vector<VkDescriptorSetLayout> &_set_layouts) {
                wrap.descriptorPool = _descr_pool;
                set_layouts = _set_layouts;
                updateWrap();
            }

            StructWrap<VkPipelineLayoutCreateInfo>::StructWrap() {}

            void StructWrap<VkPipelineLayoutCreateInfo>::clear() {
                wrap = VkPipelineLayoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
                constants.clear();
                layouts  .clear();
            }

            void StructWrap<VkPipelineLayoutCreateInfo>::updateWrap() {
                wrap.setLayoutCount = layouts.size();
                wrap.pSetLayouts    = layouts.data();
                wrap.pushConstantRangeCount = constants.size();
                wrap.pPushConstantRanges    = constants.data();
            }

            StructWrap<VkPipelineCacheCreateInfo>::StructWrap() {}

            void StructWrap<VkPipelineCacheCreateInfo>::clear() {
                wrap = VkPipelineCacheCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
                cache_data.clear();
            }

            void StructWrap<VkPipelineCacheCreateInfo>::updateWrap() {
                wrap.initialDataSize = cache_data.size();
                wrap.pInitialData    = cache_data.data();
            }

            void StructWrap<VkPipelineCacheCreateInfo>::getCacheData(VkDevice _device, VkPipelineCache _pipeline_cache) {
                cache_data = Func::GetPipelineCacheData(_device, _pipeline_cache);
            }

            StructWrap<VkPipelineLibraryCreateInfoKHR>::StructWrap() {}

            void StructWrap<VkPipelineLibraryCreateInfoKHR>::clear() {
                wrap = VkPipelineLibraryCreateInfoKHR{VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR};
                libraries.clear();
            }

            void StructWrap<VkPipelineLibraryCreateInfoKHR>::updateWrap() {
                wrap.libraryCount = libraries.size();
                wrap.pLibraries   = libraries.data();
            }

            StructWrap<VkRayTracingShaderGroupCreateInfoKHR>::StructWrap() {}

            void StructWrap<VkRayTracingShaderGroupCreateInfoKHR>::clear() {
                capture_replay_handle.clear();
                wrap = VkRayTracingShaderGroupCreateInfoKHR{VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
            }

            void StructWrap<VkRayTracingShaderGroupCreateInfoKHR>::updateWrap() {
                wrap.pShaderGroupCaptureReplayHandle = capture_replay_handle.empty() ? nullptr : capture_replay_handle.data();
            }

            void StructWrap<VkRayTracingShaderGroupCreateInfoKHR>::getData(VkPhysicalDevice _phys_device, VkDevice _device, VkPipeline _pipeline, uint32_t _first_group, uint32_t _group_count) {
                capture_replay_handle = Func::GetRayTracingCaptureReplayShaderGroupHandlesKHR(_phys_device, _device, _pipeline, _first_group, _group_count);
                updateWrap();
            }

            StructWrap<VkRayTracingPipelineInterfaceCreateInfoKHR>::StructWrap() {}

            void StructWrap<VkRayTracingPipelineInterfaceCreateInfoKHR>::clear() {
                wrap = VkRayTracingPipelineInterfaceCreateInfoKHR{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR};
            }

            void StructWrap<VkRayTracingPipelineInterfaceCreateInfoKHR>::updateWrap() {}

            StructWrap<VkPipelineShaderStageCreateInfo>::StructWrap() {}

            void StructWrap<VkPipelineShaderStageCreateInfo>::clear() {
                wrap = VkPipelineShaderStageCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};

                name.clear();

                spec_info_wrap.clear();
            }

            void StructWrap<VkPipelineShaderStageCreateInfo>::updateWrap() {
                wrap.pName = name.c_str();
                wrap.pSpecializationInfo = spec_info_wrap.get().mapEntryCount ? &spec_info_wrap.peek() : nullptr;
            }

            void StructWrap<VkPipelineShaderStageCreateInfo>::setShaderInfo(VkShaderModule _shader_module, VkShaderStageFlagBits _stage, const std::string &_name) {
                wrap.module = _shader_module;
                wrap.stage  = _stage;
                name = _name;
            }

            void StructWrap<VkPipelineShaderStageCreateInfo>::setShaderSpecInfo(const void* _data, size_t _data_size, const std::vector<VkSpecializationMapEntry> &_map_entries) {
                spec_info_wrap.setData(_data, _data_size);
                spec_info_wrap.map_entries = _map_entries;
            }

            StructWrap<VkPipelineDynamicStateCreateInfo>::StructWrap() {}

            void StructWrap<VkPipelineDynamicStateCreateInfo>::clear() {
                wrap = VkPipelineDynamicStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
                dynamic_states.clear();
            }

            void StructWrap<VkPipelineDynamicStateCreateInfo>::updateWrap() {
                wrap.dynamicStateCount = dynamic_states.size();
                wrap.pDynamicStates    = dynamic_states.data();
            }

            bool StructWrap<VkPipelineDynamicStateCreateInfo>::addDynamicState(VkDynamicState _state) {
                for (const VkDynamicState &st : dynamic_states)
                    if (st == _state) return false;

                dynamic_states.push_back(_state);
                return true;
            }

            bool StructWrap<VkPipelineDynamicStateCreateInfo>::delDynamicState(VkDynamicState _state) {
                bool __res = false;
                for (uint32_t i = 0u; i < dynamic_states.size(); ++i) {
                    if (dynamic_states[i] == _state) {
                        dynamic_states.erase(dynamic_states.begin() + i);
                        __res = true;
                    }
                }
                return __res;
            }

            StructWrap<VkPipelineVertexInputStateCreateInfo>::StructWrap() {}

            void StructWrap<VkPipelineVertexInputStateCreateInfo>::clear() {
                wrap = VkPipelineVertexInputStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

                bindings.clear();
                attributes.clear();
            }

            void StructWrap<VkPipelineVertexInputStateCreateInfo>::updateWrap() {
                wrap.vertexBindingDescriptionCount = bindings.size();
                wrap.pVertexBindingDescriptions    = bindings.data();
                wrap.vertexAttributeDescriptionCount = attributes.size();
                wrap.pVertexAttributeDescriptions    = attributes.data();
            }

            StructWrap<VkPipelineInputAssemblyStateCreateInfo>::StructWrap() {}

            void StructWrap<VkPipelineInputAssemblyStateCreateInfo>::clear() {
                wrap = VkPipelineInputAssemblyStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
            }

            void StructWrap<VkPipelineInputAssemblyStateCreateInfo>::updateWrap() {}

            void StructWrap<VkPipelineInputAssemblyStateCreateInfo>::setWholeStruct(VkPrimitiveTopology _topology, VkBool32 _primitive_restart) {
                wrap.topology = _topology;
                wrap.primitiveRestartEnable = _primitive_restart;
                updateWrap();
            }

            StructWrap<VkPipelineViewportStateCreateInfo>::StructWrap() {}

            void StructWrap<VkPipelineViewportStateCreateInfo>::clear() {
                wrap = VkPipelineViewportStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

                viewports_dynamic_count = 0u;
                scissors_dynamic_count  = 0u;
                viewports.clear();
                scissors .clear();

                viewports_wrap.clear();
                scissors_wrap .clear();
            }

            void StructWrap<VkPipelineViewportStateCreateInfo>::updateWrap() {
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

            void StructWrap<VkPipelineViewportStateCreateInfo>::setViewportsDynamic(uint32_t _viewports_count) {
                viewports_dynamic_count = _viewports_count;
            }

            void StructWrap<VkPipelineViewportStateCreateInfo>::setScissorsDynamic(uint32_t _scissors_count) {
                scissors_dynamic_count = _scissors_count;
            }

            void StructWrap<VkPipelineViewportStateCreateInfo>::setFromSwapchainExtent(VkExtent2D _extent) {
                viewports_wrap.resize(1);
                viewports_wrap[0].clear();
                viewports_wrap[0].setExtent(_extent);

                scissors_wrap.resize(1);
                scissors_wrap[0].clear();
                scissors_wrap[0].setExtent(_extent);
            }

            StructWrap<VkPipelineRasterizationStateCreateInfo>::StructWrap() {}

            void StructWrap<VkPipelineRasterizationStateCreateInfo>::clear() {
                wrap = VkPipelineRasterizationStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
                wrap.lineWidth = 1.f;
            }

            void StructWrap<VkPipelineRasterizationStateCreateInfo>::updateWrap() {}

            void StructWrap<VkPipelineRasterizationStateCreateInfo>::setDepthBias(bool _enable, float _const_factor, float _clamp, float _slope_factor) {
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

            StructWrap<VkPipelineMultisampleStateCreateInfo>::StructWrap() {}

            void StructWrap<VkPipelineMultisampleStateCreateInfo>::clear() {
                wrap = VkPipelineMultisampleStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
                sample_mask[0] = VkSampleMask(-1);
                sample_mask[1] = VkSampleMask(-1);
            }

            void StructWrap<VkPipelineMultisampleStateCreateInfo>::updateWrap() {}

            void StructWrap<VkPipelineMultisampleStateCreateInfo>::setSampleMask(VkSampleMask _mask, VkSampleMask _mask2) {
                sample_mask[0] = _mask;
                sample_mask[1] = _mask2;
                wrap.pSampleMask = sample_mask;
            }

            StructWrap<VkPipelineDepthStencilStateCreateInfo>::StructWrap() {}

            void StructWrap<VkPipelineDepthStencilStateCreateInfo>::clear() {
                wrap = VkPipelineDepthStencilStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
            }

            void StructWrap<VkPipelineDepthStencilStateCreateInfo>::updateWrap() {}

            void StructWrap<VkPipelineDepthStencilStateCreateInfo>::setDepth(bool _test, bool _write, VkCompareOp _compare_op) {
                wrap.depthTestEnable  = _test;
                wrap.depthWriteEnable = _write;
                wrap.depthCompareOp   = _compare_op;
            }

            void StructWrap<VkPipelineDepthStencilStateCreateInfo>::setStencil(bool _test, const VkStencilOpState &_front, const VkStencilOpState &_back) {
                wrap.stencilTestEnable = _test;
                wrap.front = _front;
                wrap.back  = _back;
            }

            void StructWrap<VkPipelineDepthStencilStateCreateInfo>::setStencil(bool _test, const VkStencilOpState &_state) {
                setStencil(_test, _state, _state);
            }

            void StructWrap<VkPipelineDepthStencilStateCreateInfo>::setDepthBounds(bool _test, float _min, float _max) {
                wrap.depthBoundsTestEnable = _test;
                wrap.minDepthBounds = _min;
                wrap.maxDepthBounds = _max;
            }

            StructWrap<VkPipelineColorBlendAttachmentState>::StructWrap() { clear(); }

            void StructWrap<VkPipelineColorBlendAttachmentState>::clear() {
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

            void StructWrap<VkPipelineColorBlendAttachmentState>::updateWrap() {}

            void StructWrap<VkPipelineColorBlendAttachmentState>::setColorBlend(VkBlendOp _op, VkBlendFactor _src_factor, VkBlendFactor _dst_factor) {
                wrap.colorBlendOp        = _op;
                wrap.srcColorBlendFactor = _src_factor;
                wrap.dstColorBlendFactor = _dst_factor;
            }

            void StructWrap<VkPipelineColorBlendAttachmentState>::setAlphaBlend(VkBlendOp _op, VkBlendFactor _src_factor, VkBlendFactor _dst_factor) {
                wrap.alphaBlendOp        = _op;
                wrap.srcAlphaBlendFactor = _src_factor;
                wrap.dstAlphaBlendFactor = _dst_factor;
            }

            StructWrap<VkPipelineColorBlendStateCreateInfo>::StructWrap() {}

            void StructWrap<VkPipelineColorBlendStateCreateInfo>::clear() {
                attachments.clear();

                attachments_wrap.clear();
            }

            void StructWrap<VkPipelineColorBlendStateCreateInfo>::updateWrap() {
                attachments.resize(attachments_wrap.size());
                for (uint32_t i = 0u; i < attachments.size(); ++i) {
                    attachments[i] = attachments_wrap[i].get();
                }
                wrap.attachmentCount = attachments.size();
                wrap.pAttachments    = attachments.data();
            }

            void StructWrap<VkPipelineColorBlendStateCreateInfo>::setLogicOp(VkLogicOp _op) {
                wrap.logicOpEnable = VkBool32(_op != VK_LOGIC_OP_MAX_ENUM);
                wrap.logicOp = _op;
            }

            void StructWrap<VkPipelineColorBlendStateCreateInfo>::setBlendConstants(const vec4 &_blend_constants) {
                wrap.blendConstants[0] = _blend_constants.x;
                wrap.blendConstants[1] = _blend_constants.y;
                wrap.blendConstants[2] = _blend_constants.z;
                wrap.blendConstants[3] = _blend_constants.w;
            }

            StructWrap<VkPipelineTessellationStateCreateInfo>::StructWrap() {}

            StructWrap<VkPipelineTessellationStateCreateInfo>::StructWrap(uint32_t _points_num) { wrap.patchControlPoints = _points_num; }

            void StructWrap<VkPipelineTessellationStateCreateInfo>::clear() {
                wrap = VkPipelineTessellationStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
            }

            void StructWrap<VkPipelineTessellationStateCreateInfo>::updateWrap() {}

            StructWrap<VkPipelineRenderingCreateInfo>::StructWrap() {}

            void StructWrap<VkPipelineRenderingCreateInfo>::clear() {
                wrap = VkPipelineRenderingCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
                color_formats.clear();
            }

            void StructWrap<VkPipelineRenderingCreateInfo>::updateWrap() {
                wrap.colorAttachmentCount    = color_formats.size();
                wrap.pColorAttachmentFormats = color_formats.data();
            }

            void StructWrap<VkPipelineRenderingCreateInfo>::setFormats(Span<VkFormat> _color_formats, VkFormat _depth_format, VkFormat _stencil_format) {
                color_formats.assign(_color_formats.begin(), _color_formats.end());
                wrap.depthAttachmentFormat   = _depth_format;
                wrap.stencilAttachmentFormat = _stencil_format;
            }

            StructWrap<VkGraphicsPipelineCreateInfo>::StructWrap() {}

            void StructWrap<VkGraphicsPipelineCreateInfo>::clear() {
                wrap = VkGraphicsPipelineCreateInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

                shader_stages.clear();

                active_state_flags = PipelineStateBits(0u);
                shader_stages_wrap.clear();

                rendering_wrap.clear();
                use_dynamic_rendering = false;
            }

            void StructWrap<VkGraphicsPipelineCreateInfo>::updateWrap() {
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

            void StructWrap<VkGraphicsPipelineCreateInfo>::setPipelineStates(PipelineStateFlags _state_bitmask) {
                active_state_flags = _state_bitmask;
            }

            void StructWrap<VkGraphicsPipelineCreateInfo>::setBasePipeline(int32_t _base_pipeline_index, VkPipeline _base_pipeline_handle) {
                wrap.basePipelineHandle = _base_pipeline_handle;
                wrap.basePipelineIndex  = _base_pipeline_index;
            }

            StructWrap<VkComputePipelineCreateInfo>::StructWrap() {}

            void StructWrap<VkComputePipelineCreateInfo>::clear() {
                wrap = VkComputePipelineCreateInfo{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};

                shader_stage_wrap.clear();
            }

            void StructWrap<VkComputePipelineCreateInfo>::updateWrap() {
                wrap.stage = shader_stage_wrap.get();
            }

            void StructWrap<VkComputePipelineCreateInfo>::setBasePipeline(int32_t _base_pipeline_index, VkPipeline _base_pipeline_handle) {
                wrap.basePipelineHandle = _base_pipeline_handle;
                wrap.basePipelineIndex  = _base_pipeline_index;
            }

            StructWrap<VkRayTracingPipelineCreateInfoKHR>::StructWrap() {}

            void StructWrap<VkRayTracingPipelineCreateInfoKHR>::clear() {
                wrap = VkRayTracingPipelineCreateInfoKHR{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};

                shader_stages.clear();

                active_state_flags = PipelineStateBits(0u);
                shader_stages_wrap.clear();
            }

            void StructWrap<VkRayTracingPipelineCreateInfoKHR>::updateWrap() {
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

            void StructWrap<VkRayTracingPipelineCreateInfoKHR>::setPipelineStates(PipelineStateFlags _state_bitmask) {
                active_state_flags = _state_bitmask;
            }

            void StructWrap<VkRayTracingPipelineCreateInfoKHR>::setBasePipeline(int32_t _base_pipeline_index, VkPipeline _base_pipeline_handle) {
                wrap.basePipelineHandle = _base_pipeline_handle;
                wrap.basePipelineIndex  = _base_pipeline_index;
            }

            StructWrap<VkCommandPoolCreateInfo>::StructWrap() {}

            void StructWrap<VkCommandPoolCreateInfo>::clear() {
                wrap = VkCommandPoolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
            }

            void StructWrap<VkCommandPoolCreateInfo>::updateWrap() {}

            StructWrap<VkCommandBufferAllocateInfo>::StructWrap() {}

            void StructWrap<VkCommandBufferAllocateInfo>::clear() {
                wrap = VkCommandBufferAllocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
            }

            void StructWrap<VkCommandBufferAllocateInfo>::updateWrap() {}

            StructWrap<VkAllocationCallbacks>::StructWrap() {}

            StructWrap<VkAllocationCallbacks>::~StructWrap() { clear(); }

            void StructWrap<VkAllocationCallbacks>::clear() {
                safeDelete(wrap);
            }

            void StructWrap<VkAllocationCallbacks>::updateWrap() {}

            StructWrap<VkPhysicalDeviceMemoryProperties2>::StructWrap() {}

            void StructWrap<VkPhysicalDeviceMemoryProperties2>::clear() {
                wrap = VkPhysicalDeviceMemoryProperties2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
            }

            void StructWrap<VkPhysicalDeviceMemoryProperties2>::updateWrap() {}

            void StructWrap<VkPhysicalDeviceMemoryProperties2>::loadForPhysicalDevice(VkPhysicalDevice _phys_device) {
                vkGetPhysicalDeviceMemoryProperties2(_phys_device, &wrap);
                printInfo();
            }

            void StructWrap<VkPhysicalDeviceMemoryProperties2>::printInfo() const {
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

            StructWrap<VkCommandBufferInheritanceInfo>::StructWrap() {}

            void StructWrap<VkCommandBufferInheritanceInfo>::clear() {
                wrap = VkCommandBufferInheritanceInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
            }

            void StructWrap<VkCommandBufferInheritanceInfo>::updateWrap() {}

            StructWrap<VkSubmitInfo>::StructWrap() {}

            StructWrap<VkSubmitInfo>::StructWrap(const StructWrap<VkSubmitInfo> &_wrap) : wait_semaphores{_wrap.wait_semaphores}, wait_stages{_wrap.wait_stages},
                                                                command_buffers{_wrap.command_buffers}, signal_semaphores{_wrap.signal_semaphores} {
                wrap = _wrap.wrap;
                updateWrap();
            }

            void StructWrap<VkSubmitInfo>::clear() {
                wrap = VkSubmitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};

                wait_stages.clear();
                wait_semaphores.clear();
                command_buffers.clear();
                signal_semaphores.clear();
            }

            void StructWrap<VkSubmitInfo>::updateWrap() {
                wrap.waitSemaphoreCount   = wait_semaphores.size();
                wrap.pWaitSemaphores      = wait_semaphores.data();
                wrap.pWaitDstStageMask    = wait_stages.data(); // size == .waitSemaphoreCount
                wrap.commandBufferCount   = command_buffers.size();
                wrap.pCommandBuffers      = command_buffers.data();
                wrap.signalSemaphoreCount = signal_semaphores.size();
                wrap.pSignalSemaphores    = signal_semaphores.data();
            }

            StructWrap<VkPresentInfoKHR>::StructWrap() {}

            void StructWrap<VkPresentInfoKHR>::clear() {
                wrap = VkPresentInfoKHR{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};

                wait_semaphores.clear();
                image_indices.clear();
                swapchains.clear();
                results.clear();
                use_results = false;
            }

            void StructWrap<VkPresentInfoKHR>::updateWrap() {
                results.resize(use_results * swapchains.size());
                for (auto& res : results) res = VK_SUCCESS;

                wrap.waitSemaphoreCount = wait_semaphores.size();
                wrap.pWaitSemaphores = wait_semaphores.data();
                wrap.swapchainCount = swapchains.size();
                wrap.pSwapchains = swapchains.data();
                wrap.pImageIndices = image_indices.data();
                wrap.pResults = results.empty() ? nullptr : results.data();
            }

            void StructWrap<VkPresentInfoKHR>::setWholeStruct(Span<VkSemaphore> _wait_semaphores,
                                Span<VkSwapchainKHR> _swapchains,
                                Span<uint32_t> _image_indices, bool _use_results) {
                wait_semaphores.assign(_wait_semaphores.begin(), _wait_semaphores.end());
                swapchains     .assign(     _swapchains.begin(),      _swapchains.end());
                image_indices  .assign(  _image_indices.begin(),   _image_indices.end());
                use_results = _use_results;
                updateWrap();
            }

            StructWrap<VkRenderPassBeginInfo>::StructWrap() {}

            void StructWrap<VkRenderPassBeginInfo>::clear() {
                wrap = VkRenderPassBeginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};

                clear_values.clear();

                clear_values_wrap.clear();
                render_area_wrap .clear();
            }

            void StructWrap<VkRenderPassBeginInfo>::updateWrap() {
                clear_values.resize(clear_values_wrap.size());
                for (uint32_t i = 0u; i < clear_values.size(); ++i) {
                    clear_values[i] = clear_values_wrap[i].get();
                }

                wrap.renderArea = render_area_wrap.get();
                wrap.clearValueCount = clear_values.size();
                wrap.pClearValues = clear_values.data();
            }

            void StructWrap<VkRenderPassBeginInfo>::addClearValue4f(const vec4f &_color) {
                clear_values_wrap.emplace_back();
                clear_values_wrap.back().fromColorVec4f(_color);
            }

            void StructWrap<VkRenderPassBeginInfo>::addClearValue4i(const vec4i &_color) {
                clear_values_wrap.emplace_back();
                clear_values_wrap.back().fromColorVec4i(_color);
            }

            void StructWrap<VkRenderPassBeginInfo>::addClearValue4u(const vec4u &_color) {
                clear_values_wrap.emplace_back();
                clear_values_wrap.back().fromColorVec4u(_color);
            }

            void StructWrap<VkRenderPassBeginInfo>::addClearValueDepthStencil(float _depth, uint32_t _stencil) {
                clear_values_wrap.emplace_back();
                clear_values_wrap.back().fromDepthStencil(_depth, _stencil);
            }

            StructWrap<VkCommandBufferBeginInfo>::StructWrap() {}

            void StructWrap<VkCommandBufferBeginInfo>::clear() {
                wrap = VkCommandBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

                inheritance_info_wrap.clear();
            }

            void StructWrap<VkCommandBufferBeginInfo>::updateWrap() {
                wrap.pInheritanceInfo = nullptr;

                if (use_inheritance_info) {
                    wrap.pInheritanceInfo = &inheritance_info_wrap.get();
                }
            }
        }; // Struct END


        namespace Object {

            void Instance::createObject(const VkAllocationCallbacks *_create_cb, bool _volk_instance_only) {
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

            void Instance::destroyObject(const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyInstance(handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void DebugUtilsMessengerEXT::createObject(VkInstance _instance, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreateDebugUtilsMessengerEXT(_instance, &create_info.get(), _create_cb);
            }

            void DebugUtilsMessengerEXT::destroyObject(VkInstance _instance, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyDebugUtilsMessengerEXT(_instance, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void PhysicalDevice::createObject() {
                handle = select_info.selectOne();
            }

            void PhysicalDevice::destroyObject() {
                handle = VK_NULL_HANDLE;
            }

            const DeviceFeatures   &PhysicalDevice::features()   const { return select_info.features  [select_info.indexOf(handle)]; }

            const DeviceProperties &PhysicalDevice::properties() const { return select_info.properties[select_info.indexOf(handle)]; }

            void QueueFamily::createObject() {
                handle = select_info.selectOne();
            }

            void QueueFamily::destroyObject() {
                handle = UINT32_MAX;
            }

            void Queue::createObject(VkDevice _device) {
                handles.resize(create_info.priorities.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < handles.size(); ++i)
                    handles[i] = Func::GetDeviceQueue(_device, create_info.queue_family, i);
            }

            void Queue::destroyObject() {
                handles.clear();
            }

            void Device::createObject(VkPhysicalDevice _phys_device, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreateDevice(_phys_device, &create_info.get(), _create_cb);
            }

            void Device::destroyObject(const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyDevice(handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void SurfaceKHR::createObject(VkInstance _instance, GLFWwindow *_glfw_window, const VkAllocationCallbacks *_create_cb) {
                if (glfwCreateWindowSurface(_instance, _glfw_window, _create_cb, &handle) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW::Object::SurfaceKHR :: createObject", "could not create surface"));
            }

            void SurfaceKHR::destroyObject(VkInstance _instance, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroySurfaceKHR(_instance, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void SwapchainKHR::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreateSwapchainKHR(_device, &create_info.get(), _create_cb);
            }

            void SwapchainKHR::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroySwapchainKHR(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void ShaderModule::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreateShaderModule(_device, &create_info.get(), _create_cb);
            }

            void ShaderModule::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyShaderModule(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void Buffer::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreateBuffer(_device, &create_info.get(), _create_cb);
            }

            void Buffer::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyBuffer(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void BufferView::createObject(VkDevice _device, VkBuffer _buffer, const VkAllocationCallbacks *_create_cb) {
                create_info.edit().buffer = _buffer;
                handle = Func::CreateBufferView(_device, &create_info.get(), _create_cb);
            }

            void BufferView::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyBufferView(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void Image::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreateImage(_device, &create_info.get(), _create_cb);
            }

            void Image::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyImage(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void ImageView::createObject(VkDevice _device, VkImage _image, const VkAllocationCallbacks *_create_cb) {
                create_info.edit().image = _image;
                handle = Func::CreateImageView(_device, &create_info.get(), _create_cb);
            }

            void ImageView::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyImageView(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void DeviceMemory::createObject(VkDevice _device, const VkAllocationCallbacks *_allocate_cb) {
                handle = Func::AllocateMemory(_device, &allocate_info.get(), _allocate_cb);
            }

            void DeviceMemory::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkFreeMemory(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void RenderPass::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreateRenderPass(_device, &create_info.get(), _create_cb);
            }

            void RenderPass::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyRenderPass(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void DescriptorSetLayout::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreateDescriptorSetLayout(_device, &create_info.get(), _create_cb);
            }

            void DescriptorSetLayout::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void DescriptorPool::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreateDescriptorPool(_device, &create_info.get(), _create_cb);
            }

            void DescriptorPool::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyDescriptorPool(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void Sampler::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreateSampler(_device, &create_info.get(), _create_cb);
            }

            void Sampler::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroySampler(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void DescriptorSet::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handles = Func::AllocateDescriptorSets(_device, &alloc_info.get());
            }

            void DescriptorSet::destroyObject(VkDevice _device) {
                if (!handles.empty()) vkFreeDescriptorSets(_device, alloc_info.peek().descriptorPool, handles.size(), handles.data());
                handles.clear();
            }

            void PipelineLayout::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreatePipelineLayout(_device, &create_info.get(), _create_cb);
            }

            void PipelineLayout::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyPipelineLayout(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void PipelineCache::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreatePipelineCache(_device, &create_info.get(), _create_cb);
            }

            void PipelineCache::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyPipelineCache(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            Pipeline::Pipeline() {}

            Pipeline::Pipeline(const Pipeline &_obj_pip) : handle{_obj_pip.handle} {
                setPipelineType(_obj_pip.pipeline_type);
                setPipelineFromCopy(_obj_pip);
            }

            Pipeline::Pipeline(Pipeline &&_obj_pip) : handle{_obj_pip.handle} {
                setPipelineType(_obj_pip.pipeline_type);
                setPipelineFromCopy(_obj_pip);
                _obj_pip.deletePipelineWrap();
                _obj_pip.handle = VK_NULL_HANDLE;
            }

            Pipeline::~Pipeline() { deletePipelineWrap(); }

            Pipeline& Pipeline::operator=(const Pipeline &_obj_pip) {
                if (this == &_obj_pip) return *this;
                handle = _obj_pip.handle;
                setPipelineType(_obj_pip.pipeline_type);
                setPipelineFromCopy(_obj_pip);
                return *this;
            }

            VkResult Pipeline::createObject(VkDevice _device, VkRenderPass _render_pass, uint32_t _subpass,
                                  VkDeferredOperationKHR _deferred_op, VkPipelineCache _pipeline_cache,
                                  const VkAllocationCallbacks *_create_cb) {
                (void)_render_pass; (void)_subpass;
                switch (pipeline_type) {
                    case VKFW_PIPELINE_TYPE_GRAPHICS: {
                        VkGraphicsPipelineCreateInfo __info = graphics_create_info_wrap.get();
                        VkPipelineRenderingCreateInfo __rendering;
                        if (graphics_create_info_wrap.use_dynamic_rendering) {
                            __rendering       = graphics_create_info_wrap.rendering_wrap.get();
                            __rendering.pNext = __info.pNext;
                            __info.pNext      = &__rendering;
                            __info.renderPass = VK_NULL_HANDLE;
                            __info.subpass    = 0u;
                        }
                        return Func::CreateGraphicsPipelines(_device, _pipeline_cache, __info, _create_cb, &handle);
                    }
                    case VKFW_PIPELINE_TYPE_COMPUTE:     return Func::CreateComputePipelines      (_device,               _pipeline_cache,    compute_create_info_wrap.get(), _create_cb, &handle);
                    case VKFW_PIPELINE_TYPE_RAY_TRACING: return Func::CreateRayTracingPipelinesKHR(_device, _deferred_op, _pipeline_cache, raytracing_create_info_wrap.get(), _create_cb, &handle);
                    case VKFW_PIPELINE_TYPE_UNDEFINED:  return VK_SUCCESS;
                    default: throw std::runtime_error(SVKFW_WRAPERR("VKFW::Object::Pipeline :: createObject", "unknown pipeline type: " + std::to_string(pipeline_type)));
                };
            }

            void Pipeline::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyPipeline(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void Pipeline::setPipelineFromCopy(const Pipeline &_obj_pip) {
                switch (pipeline_type) {
                    case VKFW_PIPELINE_TYPE_GRAPHICS:      graphics_create_info_wrap = _obj_pip.  graphics_create_info_wrap; break;
                    case VKFW_PIPELINE_TYPE_COMPUTE:        compute_create_info_wrap = _obj_pip.   compute_create_info_wrap; break;
                    case VKFW_PIPELINE_TYPE_RAY_TRACING: raytracing_create_info_wrap = _obj_pip.raytracing_create_info_wrap; break;
                    default: break;
                };
            }

            void Pipeline::setPipelineType(PipelineType _pipeline_type) {
                deletePipelineWrap();
                pipeline_type = _pipeline_type;
                activatePipelineWrap();
            }

            void Pipeline::deletePipelineWrap() {
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

            void Pipeline::activatePipelineWrap() {
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

            ContextIndex Pipeline::addShaderStage() {
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

            Struct::StructWrap<VkPipelineShaderStageCreateInfo>* Pipeline::shortcutShaderStage(ContextIndex _ci_shader_stage) {
                switch(pipeline_type) {
                    case VKFW_PIPELINE_TYPE_GRAPHICS:    return &  graphics_create_info_wrap.shader_stages_wrap[_ci_shader_stage];
                    case VKFW_PIPELINE_TYPE_COMPUTE:     return &    compute_create_info_wrap.shader_stage_wrap;
                    case VKFW_PIPELINE_TYPE_RAY_TRACING: return &raytracing_create_info_wrap.shader_stages_wrap[_ci_shader_stage];
                    case VKFW_PIPELINE_TYPE_UNDEFINED: throw std::runtime_error(SVKFW_WRAPERR("VKFW::Object::Pipeline :: shortcutShaderStage", "undefined pipeline type: " + std::to_string(pipeline_type)));
                    default:                           throw std::runtime_error(SVKFW_WRAPERR("VKFW::Object::Pipeline :: shortcutShaderStage",   "unknown pipeline type: " + std::to_string(pipeline_type)));
                }
                return nullptr;
            }

            void Framebuffer::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreateFramebuffer(_device, &create_info.get(), _create_cb);
            }

            void Framebuffer::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyFramebuffer(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void CommandPool::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handle = Func::CreateCommandPool(_device, &create_info.get(), _create_cb);
            }

            void CommandPool::createObject(VkDevice _device, VkfwQueueFamily _queue_family, VkCommandPoolCreateFlags _flags, const VkAllocationCallbacks *_create_cb) {
                create_info.edit().flags =  _flags;
                create_info.edit().queueFamilyIndex = _queue_family;
                handle = Func::CreateCommandPool(_device, &create_info.get(), _create_cb);
            }

            void CommandPool::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                if (handle != VK_NULL_HANDLE) vkDestroyCommandPool(_device, handle, _destroy_cb);
                handle = VK_NULL_HANDLE;
            }

            void CommandBuffer::createObject(VkDevice _device, const VkAllocationCallbacks *_create_cb) {
                handles = Func::AllocateCommandBuffers(_device, &create_info.get());
            }

            void CommandBuffer::destroyObject(VkDevice _device) {
                if (!handles.empty()) vkFreeCommandBuffers(_device, create_info.peek().commandPool, (uint32_t)handles.size(), handles.data());
                handles.clear();
            }

            void CommandBuffer::createPrimary  (VkDevice _device, VkCommandPool _command_pool, uint32_t _buffer_count, const VkAllocationCallbacks *_create_cb) {
                create_info.edit().level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                create_info.edit().commandPool = _command_pool;
                create_info.edit().commandBufferCount = _buffer_count;
                handles = Func::AllocateCommandBuffers(_device, &create_info.get());
            }

            void CommandBuffer::createSecondary(VkDevice _device, VkCommandPool _command_pool, uint32_t _buffer_count, const VkAllocationCallbacks *_create_cb) {
                create_info.edit().level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                create_info.edit().commandPool = _command_pool;
                create_info.edit().commandBufferCount = _buffer_count;
                handles = Func::AllocateCommandBuffers(_device, &create_info.get());
            }

            void Semaphore::createObject(VkDevice _device, uint32_t _semaphore_count, const VkAllocationCallbacks *_create_cb) {
                handles = Func::CreateSemaphores(_device, &create_info.get(), _semaphore_count, _create_cb);
            }

            void Semaphore::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                for (auto handle : handles)
                    if (handle != VK_NULL_HANDLE) vkDestroySemaphore(_device, handle, _destroy_cb);
                handles.clear();
            }

            void Fence::createObject(VkDevice _device, uint32_t _fence_count, const VkAllocationCallbacks *_create_cb) {
                handles = Func::CreateFences(_device, &create_info.get(), _fence_count, _create_cb);
            }

            void Fence::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                for (auto handle : handles)
                    vkDestroyFence(_device, handle, _destroy_cb);
                handles.clear();
            }

            void Fence::createSignaled(VkDevice _device, uint32_t _fence_count, const VkAllocationCallbacks *_create_cb) {
                create_info.setWholeStruct(true);
                handles = Func::CreateFences(_device, &create_info.get(), _fence_count, _create_cb);
            }

            void Fence::createNotSignaled(VkDevice _device, uint32_t _fence_count, const VkAllocationCallbacks *_create_cb) {
                create_info.setWholeStruct(false);
                handles = Func::CreateFences(_device, &create_info.get(), _fence_count, _create_cb);
            }

            void Event::createObject(VkDevice _device, uint32_t _event_count, const VkAllocationCallbacks *_create_cb) {
                handles = Func::CreateEvents(_device, &create_info.get(), _event_count, _create_cb);
            }

            void Event::destroyObject(VkDevice _device, const VkAllocationCallbacks *_destroy_cb) {
                for (auto handle : handles)
                    vkDestroyEvent(_device, handle, _destroy_cb);
                handles.clear();
            }

            void Event::createSignaled(VkDevice _device, uint32_t _event_count, const VkAllocationCallbacks *_create_cb) {
                create_info.setWholeStruct(true);
                handles = Func::CreateEvents(_device, &create_info.get(), _event_count, _create_cb);
            }

            void Event::createNotSignaled(VkDevice _device, uint32_t _event_count, const VkAllocationCallbacks *_create_cb) {
                create_info.setWholeStruct(false);
                handles = Func::CreateEvents(_device, &create_info.get(), _event_count, _create_cb);
            }
        }; // Object END
    }; // VKFW END
}; // Simple END
