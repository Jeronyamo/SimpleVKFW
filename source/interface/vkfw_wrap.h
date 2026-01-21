#ifndef SVKFW_VKFW_WRAP_H
#define SVKFW_VKFW_WRAP_H

#include "vkfw_base.h"

#include <fstream>


namespace Simple {
    namespace VKFW {

// Properly delete ptr members of Vulkan structs returned by getVkStructDCopy() methods

        namespace Deleters {
            void deleteStructPtrs(VkInstanceCreateInfo &_sct) {
                if (_sct.pApplicationInfo != nullptr)
                    delete _sct.pApplicationInfo;
                if (_sct.enabledLayerCount && _sct.ppEnabledLayerNames != nullptr)
                    StringVec::DeleteCharPP(_sct.ppEnabledLayerNames, _sct.enabledLayerCount);
                if (_sct.enabledExtensionCount && _sct.ppEnabledExtensionNames != nullptr)
                    StringVec::DeleteCharPP(_sct.ppEnabledExtensionNames, _sct.enabledExtensionCount);

                _sct.pApplicationInfo = nullptr;
                _sct.ppEnabledLayerNames = nullptr;
                _sct.ppEnabledExtensionNames = nullptr;
            }

            void deleteStructPtrs(VkDeviceQueueCreateInfo &_sct) {
                if (_sct.queueCount && _sct.pQueuePriorities != nullptr)
                    delete[] _sct.pQueuePriorities;

                _sct.pQueuePriorities = nullptr;
            }

            void deleteStructPtrs(VkSwapchainCreateInfoKHR &_sct) {
                if (_sct.queueFamilyIndexCount && _sct.pQueueFamilyIndices != nullptr)
                    delete[] _sct.pQueueFamilyIndices;

                _sct.pQueueFamilyIndices = nullptr;
            }

            void deleteStructPtrsConst(const VkSpecializationInfo &_sct) {
                if (_sct.mapEntryCount) {
                    if (_sct.pMapEntries != nullptr)
                        delete[] _sct.pMapEntries;

                    if (_sct.dataSize && _sct.pData != nullptr)
                        delete[] (unsigned char*)_sct.pData;
                }
            }
            void deleteStructPtrs(VkSpecializationInfo &_sct) {
                deleteStructPtrsConst(_sct);

                _sct.pMapEntries = nullptr;
                _sct.pData = nullptr;
            }

            void deleteStructPtrsConst(const VkSubpassDescription &_sct) {
                if (_sct.inputAttachmentCount && _sct.pInputAttachments != nullptr)
                    delete[] _sct.pInputAttachments;
                if (_sct.colorAttachmentCount) {
                    if (_sct.pColorAttachments != nullptr)
                        delete[] _sct.pColorAttachments;
                    if (_sct.pResolveAttachments != nullptr)
                        delete[] _sct.pResolveAttachments;
                }
                if (_sct.pDepthStencilAttachment != nullptr)
                    delete _sct.pDepthStencilAttachment;
                if (_sct.preserveAttachmentCount && _sct.pPreserveAttachments != nullptr)
                    delete[] _sct.pPreserveAttachments;
            }
            void deleteStructPtrs(VkSubpassDescription &_sct) {
                deleteStructPtrsConst(_sct);

                _sct.pInputAttachments = nullptr;
                _sct.pColorAttachments = nullptr;
                _sct.pResolveAttachments = nullptr;
                _sct.pPreserveAttachments = nullptr;
                _sct.pDepthStencilAttachment = nullptr;
            }

            void deleteStructPtrs(VkRenderPassCreateInfo &_sct) {
                if (_sct.attachmentCount && _sct.pAttachments != nullptr)
                    delete[] _sct.pAttachments;
                if (_sct.subpassCount && _sct.pSubpasses != nullptr) {
                    for (uint32_t i = 0u; i < _sct.subpassCount; ++i)
                        deleteStructPtrsConst(_sct.pSubpasses[i]);
                    delete[] _sct.pSubpasses;
                }
                if (_sct.dependencyCount && _sct.pDependencies != nullptr)
                    delete[] _sct.pDependencies;

                _sct.pAttachments = nullptr;
                _sct.pSubpasses = nullptr;
                _sct.pDependencies = nullptr;
            }

            void deleteStructPtrsConst(const VkPipelineVertexInputStateCreateInfo &_sct) {
                if (_sct.vertexBindingDescriptionCount && _sct.pVertexBindingDescriptions != nullptr)
                    delete[] _sct.pVertexBindingDescriptions;
                if (_sct.vertexAttributeDescriptionCount && _sct.pVertexAttributeDescriptions != nullptr)
                    delete[] _sct.pVertexAttributeDescriptions;
            }
            void deleteStructPtrs(VkPipelineVertexInputStateCreateInfo &_sct) {
                deleteStructPtrsConst(_sct);

                _sct.pVertexBindingDescriptions = nullptr;
                _sct.pVertexAttributeDescriptions = nullptr;
            }

            void deleteStructPtrsConst(const VkPipelineViewportStateCreateInfo &_sct) {
                if (_sct.viewportCount && _sct.pViewports != nullptr)
                    delete[] _sct.pViewports;
                if (_sct.scissorCount && _sct.pScissors != nullptr)
                    delete[] _sct.pScissors;
            }
            void deleteStructPtrs(VkPipelineViewportStateCreateInfo &_sct) {
                deleteStructPtrsConst(_sct);

                _sct.pViewports = nullptr;
                _sct.pScissors = nullptr;
            }

            void deleteStructPtrsConst(const VkPipelineColorBlendStateCreateInfo &_sct) {
                if (_sct.attachmentCount && _sct.pAttachments != nullptr) {
                    delete[] _sct.pAttachments;
                }
            }
            void deleteStructPtrs(VkPipelineColorBlendStateCreateInfo &_sct) {
                deleteStructPtrsConst(_sct);

                _sct.pAttachments = nullptr;
            }

            void deleteStructPtrs(VkPipelineShaderStageCreateInfo &_sct) {
                if (_sct.pSpecializationInfo != nullptr) {
                    deleteStructPtrsConst(*_sct.pSpecializationInfo);
                    delete[] _sct.pSpecializationInfo;
                }

                _sct.pSpecializationInfo = nullptr;
            }

            void deleteStructPtrs(VkPipelineMultisampleStateCreateInfo &_sct) {
                if (_sct.pSampleMask != nullptr)
                    delete[] _sct.pSampleMask;

                _sct.pSampleMask = nullptr;
            }

            void deleteStructPtrs(VkPipelineDynamicStateCreateInfo &_sct) {
                if (_sct.dynamicStateCount && _sct.pDynamicStates != nullptr)
                    delete[] _sct.pDynamicStates;

                _sct.pDynamicStates = nullptr;
            }

            void deleteStructPtrs(VkPipelineLayoutCreateInfo &_sct) {
                if (_sct.setLayoutCount && _sct.pSetLayouts != nullptr)
                    delete[] _sct.pSetLayouts;
                if (_sct.pushConstantRangeCount && _sct.pPushConstantRanges != nullptr)
                    delete[] _sct.pPushConstantRanges;

                _sct.pSetLayouts = nullptr;
                _sct.pPushConstantRanges = nullptr;
            }

            void deleteStructPtrs(VkGraphicsPipelineCreateInfo &_sct) {
                if (_sct.stageCount && _sct.pStages != nullptr) {
                    delete[] _sct.pStages;
                }
                if (_sct.pVertexInputState != nullptr) {
                    deleteStructPtrsConst(*_sct.pVertexInputState);
                    delete _sct.pVertexInputState;
                }
                if (_sct.pInputAssemblyState != nullptr)
                    delete _sct.pInputAssemblyState;
                if (_sct.pTessellationState != nullptr)
                    delete _sct.pTessellationState;
                if (_sct.pViewportState != nullptr) {
                    deleteStructPtrsConst(*_sct.pViewportState);
                    delete _sct.pViewportState;
                }
                if (_sct.pRasterizationState != nullptr)
                    delete _sct.pRasterizationState;
                if (_sct.pMultisampleState != nullptr)
                    delete _sct.pMultisampleState;
                if (_sct.pDepthStencilState != nullptr)
                    delete _sct.pDepthStencilState;
                if (_sct.pColorBlendState != nullptr) {
                    deleteStructPtrsConst(*_sct.pColorBlendState);
                    delete _sct.pColorBlendState;
                }
                if (_sct.pDynamicState != nullptr)
                    delete _sct.pDynamicState;

                _sct.pStages = nullptr;
                _sct.pVertexInputState = nullptr;
                _sct.pInputAssemblyState = nullptr;
                _sct.pTessellationState = nullptr;
                _sct.pViewportState = nullptr;
                _sct.pRasterizationState = nullptr;
                _sct.pMultisampleState = nullptr;
                _sct.pDepthStencilState = nullptr;
                _sct.pColorBlendState = nullptr;
                _sct.pDynamicState = nullptr;
            }

            void deleteStructPtrs(VkFramebufferCreateInfo &_sct) {
                if (_sct.attachmentCount && _sct.pAttachments != nullptr)
                    delete[] _sct.pAttachments;

                _sct.pAttachments = nullptr;
            }

            void deleteStructPtrs(VkSubmitInfo &_sct) {
                if (_sct.waitSemaphoreCount) {
                    if (_sct.pWaitSemaphores != nullptr)
                        delete[] _sct.pWaitSemaphores;

                    if (_sct.pWaitDstStageMask != nullptr)
                        delete[] _sct.pWaitDstStageMask;
                }

                if (_sct.commandBufferCount && _sct.pCommandBuffers != nullptr)
                    delete[] _sct.pCommandBuffers;

                if (_sct.signalSemaphoreCount && _sct.pSignalSemaphores != nullptr) {
                    delete[] _sct.pSignalSemaphores;
                }

                _sct.pWaitSemaphores = nullptr;
                _sct.pWaitDstStageMask = nullptr;
                _sct.pCommandBuffers = nullptr;
                _sct.pSignalSemaphores = nullptr;
            }

            void deleteStructPtrs(VkPresentInfoKHR &_sct) {
                if (_sct.waitSemaphoreCount && _sct.pWaitSemaphores != nullptr)
                    delete[] _sct.pWaitSemaphores;
                if (_sct.swapchainCount) {
                    if (_sct.pSwapchains != nullptr)
                        delete[] _sct.pSwapchains;
                    if (_sct.pImageIndices != nullptr)
                        delete[] _sct.pImageIndices;
                    if (_sct.pResults != nullptr)
                        delete[] _sct.pResults;
                }

                _sct.pWaitSemaphores = nullptr;
                _sct.pSwapchains = nullptr;
                _sct.pImageIndices = nullptr;
                _sct.pResults = nullptr;
            }
        }; // Deleters END


//  ============  Vulkan Function Wrappers  ============  \\

        namespace Func {
            std::vector<VkLayerProperties> enumerateInstanceLayerProperties() {
                uint32_t __layer_count = 0u;
                std::vector<VkLayerProperties> __res;
                vkEnumerateInstanceLayerProperties(&__layer_count, nullptr);

                if (__layer_count != 0u) {
                    __res.resize(__layer_count);
                    vkEnumerateInstanceLayerProperties(&__layer_count, __res.data());
                }
                else
                    printf(SVKFW_WRAPINFO("VKFW :: Func :: enumerateInstanceLayerProperties", "did not find instance layers.\n"));
                return __res;
            }

            std::vector<VkExtensionProperties> enumerateInstanceExtensionProperties(const char *_layer = nullptr) {
                uint32_t __ext_count = 0u;
                std::vector<VkExtensionProperties> __res;
                vkEnumerateInstanceExtensionProperties(_layer, &__ext_count, nullptr);

                if (__ext_count != 0u) {
                    __res.resize(__ext_count);
                    vkEnumerateInstanceExtensionProperties(_layer, &__ext_count, __res.data());
                }
                else
                    printf(SVKFW_WRAPINFO("VKFW :: Func :: enumerateInstanceExtensionProperties", "did not find instance extensions.\n"));
                return __res;
            }

            std::vector<VkLayerProperties> enumerateDeviceLayerProperties(VkPhysicalDevice _phys_device) {
                uint32_t __layer_count = 0u;
                std::vector<VkLayerProperties> __res;
                vkEnumerateDeviceLayerProperties(_phys_device, &__layer_count, nullptr);

                if (__layer_count != 0u) {
                    __res.resize(__layer_count);
                    vkEnumerateDeviceLayerProperties(_phys_device, &__layer_count, __res.data());
                }
                else
                    printf(SVKFW_WRAPINFO("VKFW :: Func :: enumerateDeviceLayerProperties", "did not find device layers.\n"));
                return __res;
            }

            std::vector<VkExtensionProperties> enumerateDeviceExtensionProperties(VkPhysicalDevice _phys_device, const char *_layer = nullptr) {
                uint32_t __ext_count = 0u;
                std::vector<VkExtensionProperties> __res;
                vkEnumerateDeviceExtensionProperties(_phys_device, _layer, &__ext_count, nullptr);

                if (__ext_count != 0u) {
                    __res.resize(__ext_count);
                    vkEnumerateDeviceExtensionProperties(_phys_device, _layer, &__ext_count, __res.data());
                }
                else
                    printf(SVKFW_WRAPINFO("VKFW :: Func :: enumerateDeviceExtensionProperties", "did not find device extensions.\n"));
                return __res;
            }

            std::vector<VkSurfaceFormatKHR> getPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface) {
                uint32_t __format_count = 0u;
                std::vector<VkSurfaceFormatKHR> __res;

                vkGetPhysicalDeviceSurfaceFormatsKHR(_phys_device, _surface, &__format_count, nullptr);
                if (__format_count != 0u) {
                    __res.resize(__format_count);
                    vkGetPhysicalDeviceSurfaceFormatsKHR(_phys_device, _surface, &__format_count, __res.data());
                }
                else
                    printf(SVKFW_WRAPINFO("VKFW :: Func :: getPhysicalDeviceSurfaceFormatsKHR", "did not find surface formats.\n"));
                return __res;
            }

            std::vector<VkPresentModeKHR> getPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface) {
                uint32_t __pres_mode_count = 0u;
                std::vector<VkPresentModeKHR> __res;

                vkGetPhysicalDeviceSurfacePresentModesKHR(_phys_device, _surface, &__pres_mode_count, nullptr);

                if (__pres_mode_count != 0) {
                    __res.resize(__pres_mode_count);
                    vkGetPhysicalDeviceSurfacePresentModesKHR(_phys_device, _surface, &__pres_mode_count, __res.data());
                }
                else
                    printf(SVKFW_WRAPINFO("VKFW :: Func :: getPhysicalDeviceSurfacePresentModesKHR", "did not find presentation modes.\n"));
                return __res;
            }

            std::vector<VkImage> getSwapchainImagesKHR(VkDevice _device, VkSwapchainKHR _swapchain) {
                uint32_t __img_count = 0u;
                std::vector<VkImage> __res;

                vkGetSwapchainImagesKHR(_device, _swapchain, &__img_count, nullptr);

                if (__img_count != 0u) {
                    __res.resize(__img_count);
                    vkGetSwapchainImagesKHR(_device, _swapchain, &__img_count, __res.data());
                }
                else
                    printf(SVKFW_WRAPINFO("VKFW :: Func :: getSwapchainImagesKHR", "did not find swapchain images.\n"));
                return __res;
            }

            // safeDeleteArr may be used to delete returned pointer
            void* getPipelineCacheData(VkDevice _device, VkPipelineCache _pipeline_cache, size_t &_cache_data_size) {
                _cache_data_size = 0u;
                void *__cache_data = nullptr;

                vkGetPipelineCacheData(_device, _pipeline_cache, &_cache_data_size, __cache_data);

                if (_cache_data_size != 0u) {
                    __cache_data = new unsigned char[_cache_data_size];
                    vkGetPipelineCacheData(_device, _pipeline_cache, &_cache_data_size, __cache_data);
                }
                return __cache_data;
            }

            std::vector<unsigned char> getPipelineCacheData(VkDevice _device, VkPipelineCache _pipeline_cache) {
                size_t __cache_data_size = 0ul;
                std::vector<unsigned char> __res;

                vkGetPipelineCacheData(_device, _pipeline_cache, &__cache_data_size, nullptr);

                if (__cache_data_size != 0u) {
                    __res.resize(__cache_data_size);
                    vkGetPipelineCacheData(_device, _pipeline_cache, &__cache_data_size, __res.data());
                }
                return __res;
            }


            void beginCommandBuffer(VkCommandBuffer _command_buffer, const void *_p_next = nullptr,
                                    VkCommandBufferUsageFlags _flags = 0u,
                                    const VkCommandBufferInheritanceInfo *_inheritance_info = nullptr) {
                VkCommandBufferBeginInfo __begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
                __begin_info.pNext = _p_next;
                __begin_info.flags = _flags;
                __begin_info.pInheritanceInfo = _inheritance_info;

                if (vkBeginCommandBuffer(_command_buffer, &__begin_info) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Func :: beginCommandBuffer", "failed to begin recording command buffer"));
            }

            void queueSubmit(VkQueue _queue, const std::vector<VkSubmitInfo> &_submits, VkFence _fence = VK_NULL_HANDLE) {
                if (vkQueueSubmit(_queue, _submits.size(), _submits.data(), _fence) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Func :: queueSubmit", "failed to submit queue"));
            }

            void queuePresentKHR(VkQueue _queue, const VkPresentInfoKHR &_pres_info) {
                VkResult __queue_pres_res = vkQueuePresentKHR(_queue, &_pres_info);

                if (__queue_pres_res != VK_SUCCESS) {
                    printf("Error code: %d\n", __queue_pres_res);
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Func :: queuePresentKHR", "failed to present queue"));
                }
                else if (_pres_info.pResults != nullptr) {
                    for (uint32_t i = 0u; i < _pres_info.swapchainCount; ++i) {
                        if (_pres_info.pResults[i] != VK_SUCCESS)
                            fprintf(svkfwwarn, "%s %d\n", SVKFW_WRAPWARN("VKFW :: Func :: queuePresentKHR", "error with swapchain"), i);
                    }
                }
            }

            void waitForFences(VkDevice _device, const std::vector<VkFence> &_fences,
                               VkBool32 _wait_all = VK_TRUE, uint64_t _wait_time = UINT64_MAX) {
                vkWaitForFences(_device, _fences.size(), _fences.data(), _wait_all, _wait_time);
            }

            void resetFences(VkDevice _device, const std::vector<VkFence> &_fences) {
                vkResetFences(_device, _fences.size(), _fences.data());
            }

            uint32_t acquireNextImageKHR(VkDevice _device, VkSwapchainKHR _swapchain,
                                         VkSemaphore _semaphore, VkFence _fence = VK_NULL_HANDLE,
                                         uint64_t _timeout = UINT64_MAX) {
                uint32_t __image_index = 0u;
                vkAcquireNextImageKHR(_device, _swapchain, _timeout, _semaphore, _fence, &__image_index);

                return __image_index;
            }

            void queuePresentKHR(VkQueue _queue, const std::vector<VkSwapchainKHR> &_swapchains, const std::vector<VkSemaphore> &_wait_sem,
                                 const std::vector<uint32_t> &_image_indices, bool _use_results = false, const void *_p_next = nullptr) {
                VkPresentInfoKHR __info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
                __info.pNext = _p_next;
                __info.waitSemaphoreCount = _wait_sem.size();
                __info.pWaitSemaphores = _wait_sem.data();
                __info.swapchainCount = _swapchains.size();
                __info.pSwapchains = _swapchains.data();
                __info.pImageIndices = _image_indices.data();
                __info.pResults = nullptr;
                if (_use_results && _swapchains.size())
                    __info.pResults = new VkResult[_swapchains.size()]{};

                vkQueuePresentKHR(_queue, &__info);

                if (_use_results && _swapchains.size())
                    delete[] __info.pResults;
            }

            VkMemoryRequirements getBufferMemoryRequirements(VkDevice _device, VkBuffer _buffer) {
                VkMemoryRequirements __requirements;

                vkGetBufferMemoryRequirements(_device, _buffer, &__requirements);
                return __requirements;
            }

            VkPhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties(VkPhysicalDevice _phys_device) {
                VkPhysicalDeviceMemoryProperties __properties;

                vkGetPhysicalDeviceMemoryProperties(_phys_device, &__properties);
                return __properties;
            }

            namespace UpdateDescriptorSets {
                struct Builder;
            };
        }; // Func END


//  ============  Vulkan Utility Functions  ============  \\

        namespace Util {
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
                        fprintf(svkfwwarn, "%s %s\n", SVKFW_WRAPWARN("VKFW :: Util :: checkLayers","could not find layer"), _layers.list[i].c_str());
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
                        fprintf(svkfwwarn, "%s %s\n", SVKFW_WRAPWARN("VKFW :: Util :: checkExtensions","could not find extension"), _extensions.list[i].c_str());
                }
            }


            bool allInstanceLayersAvailable(const StringVec &_layers) {
                std::vector<VkLayerProperties> __layer_list{Func::enumerateInstanceLayerProperties()};
                return allLayersAvailable(_layers, __layer_list);
            }

            bool allInstanceExtAvailable(const StringVec &_extensions, const char *_layer = nullptr) {
                std::vector<VkExtensionProperties> __ext_list{Func::enumerateInstanceExtensionProperties(_layer)};
                return allExtAvailable(_extensions, __ext_list);
            }

            bool allDeviceLayersAvailable(VkPhysicalDevice _phys_device, const StringVec &_layers) {
                std::vector<VkLayerProperties> __layer_list{Func::enumerateDeviceLayerProperties(_phys_device)};
                return allLayersAvailable(_layers, __layer_list);
            }

            bool allDeviceExtAvailable(VkPhysicalDevice _phys_device, const StringVec &_extensions, const char *_layer = nullptr) {
                std::vector<VkExtensionProperties> __ext_list{Func::enumerateDeviceExtensionProperties(_phys_device, _layer)};
                return allExtAvailable(_extensions, __ext_list);
            }


            void checkInstanceLayers(const StringVec &_layers, bool *_flags) {
                if (_flags == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: Util :: checkInstanceLayers",
                                    "'flags' must be a pointer to an array of 'layers.size()' boolean values"));

                std::vector<VkLayerProperties> __layer_list{Func::enumerateInstanceLayerProperties()};

                checkLayers(_layers, _flags, __layer_list);
            }

            void checkInstanceExtensions(const StringVec &_extensions, bool *_flags, const char *_layer = nullptr) {
                if (_flags == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: Util :: checkInstanceExtensions",
                                    "'flags' must be a pointer to an array of 'extensions.size()' boolean values"));

                std::vector<VkExtensionProperties> __ext_list{Func::enumerateInstanceExtensionProperties(_layer)};

                checkExtensions(_extensions, _flags, __ext_list);
            }

            void checkDeviceLayers(VkPhysicalDevice _phys_device, const StringVec &_layers, bool *_flags) {
                if (_flags == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: Util :: checkDeviceLayers",
                                    "'flags' must be a pointer to an array of 'layers.size()' boolean values"));

                std::vector<VkLayerProperties> __layer_list{Func::enumerateDeviceLayerProperties(_phys_device)};

                checkLayers(_layers, _flags, __layer_list);
            }

            void checkDeviceExtensions(VkPhysicalDevice _phys_device, const StringVec &_extensions, bool *_flags, const char *_layer = nullptr) {
                if (_flags == nullptr)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: Util :: checkDeviceExtensions",
                                    "'flags' must be a pointer to an array of 'extensions.size()' boolean values"));

                std::vector<VkExtensionProperties> __ext_list{Func::enumerateDeviceExtensionProperties(_phys_device, _layer)};

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

                fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW :: Util :: checkPhysicalDeviceMemoryProperties", "Could not find suitable memory type"));
                return UINT32_MAX;
            }
        }; // Util END


//  ============  Vulkan Instance  ============  \\

        namespace Instance {
            struct Builder {
                VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
                StringVec layers, extensions;
                VkBool32 volk_load_inst_only = VK_FALSE;


                Builder() {
                    app_info.apiVersion = VulkanAPIVersion::available();
                }
                Builder(const Builder &_builder) = default;
                ~Builder() {
                    clear();
                }


                void clear() {
                    app_info = VkApplicationInfo{};
                    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                    app_info.apiVersion = VulkanAPIVersion::available();

                    layers.clear();
                    extensions.clear();

                    volk_load_inst_only = VK_FALSE;
                }


                VkInstance createObject(const void *_p_next = nullptr, VkInstanceCreateFlags _flags = 0u,
                                        const VkAllocationCallbacks * _create_cb = nullptr) const {
                    VkInstanceCreateInfo __info = getVkStructView(_p_next, _flags);
                    VkInstance __instance = VK_NULL_HANDLE;

                    if (vkCreateInstance(&__info, _create_cb, &__instance) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Instance Builder :: createObject", "could not create instance"));

                    if (volk_load_inst_only) {
                        volkLoadInstanceOnly(__instance);
                        printf(SVKFW_WRAPINFO("VKFW :: Instance Builder :: createObject", "only instance-based function pointers are loaded. Call volkLoadDevice() after VkDevice creation.\n"));
                    }
                    else volkLoadInstance(__instance);

                    return __instance;
                }

                VkInstanceCreateInfo getVkStructView(const void *_p_next = nullptr, VkInstanceCreateFlags _flags = 0u) const {
                    VkInstanceCreateInfo __res{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
                    __res.pNext = _p_next;
                    __res.flags = _flags;
                    __res.pApplicationInfo = &app_info;
                    __res.enabledLayerCount = layers.count();
                    __res.ppEnabledLayerNames = layers.getEntries();
                    __res.enabledExtensionCount = extensions.count();
                    __res.ppEnabledExtensionNames = extensions.getEntries();

                    return __res;
                }

                VkInstanceCreateInfo getVkStructDCopy(const void *_p_next = nullptr, VkInstanceCreateFlags _flags = 0u) const {
                    VkInstanceCreateInfo __res = getVkStructView(_p_next, _flags);
                    __res.ppEnabledLayerNames = layers.getEntriesDCopy();
                    __res.ppEnabledExtensionNames = extensions.getEntriesDCopy();

                    return __res;
                }

            // Setters

                void setApplicationInfo(const char *_app_name, vec3u _app_version,
                                        const char * _ng_name, vec3u  _ng_version,
                                        uint32_t _api_version = 0u, const void *_p_next = nullptr) {
                    app_info.pNext = _p_next;
                    app_info.pApplicationName = _app_name;
                    app_info.applicationVersion = VulkanAPIVersion::make(_app_version);
                    app_info.pEngineName = _ng_name;
                    app_info.engineVersion = VulkanAPIVersion::make(_ng_version);
                    app_info.apiVersion = _api_version ? _api_version : VulkanAPIVersion::available();
                }

                void setVolkInitInstanceOnly(bool _inst_only = true) {
                    volk_load_inst_only = _inst_only;
                }

                void setLayers(const StringVec &_layers) {
                    layers = _layers;
                }

                void setExtensions(const StringVec &_extensions) {
                    extensions = _extensions;
                }

            // Scenarios

                void addLayers(const StringVec &_layers) {
                    layers.addEntries(_layers.list);
                }

                void addExtensions(const StringVec &_extensions) {
                    extensions.addEntries(_extensions.list);
                }

                void addValLayers(const StringVec &_val_layers = {{ "VK_LAYER_KHRONOS_validation" }}) {
                    if (!Util::allInstanceLayersAvailable(_val_layers))
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Instance Builder :: addValLayers","has unavailable layers"));

                    addLayers(_val_layers);
                }

                void addGLFWExtensions() { // As Vulkan + GLFW is the main stack for now
                    uint32_t _count = 0u;
                    const char **_ext = glfwGetRequiredInstanceExtensions(&_count);
                    addExtensions({_ext, _count});
                }

                void addDebugUtilsExtension() {
                    if (!Util::allInstanceExtAvailable({{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME }}))
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Instance Builder :: addDUExtension","debug message callback extension is unavailable"));

                    addExtensions({{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME }});
                }
            }; // Builder END
        }; // Instance END


//  ============  Vulkan Debug Messenger Callback  ============  \\

        namespace DebugUtilsMessengerEXT {
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


            struct Builder {
                VkDebugUtilsMessageSeverityFlagsEXT message_severity = 0;
                VkDebugUtilsMessageTypeFlagsEXT message_type = 0;
                PFN_vkDebugUtilsMessengerCallbackEXT user_callback = nullptr;
                void* user_data = nullptr;


                Builder() {
                    message_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                    message_type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                    user_callback = vkfwDebugMessageCallback;
                    user_data = nullptr;
                }
                Builder(const Builder &_builder) = default;
                ~Builder() {
                    clear();
                }


                void clear() {
                    message_severity = 0;
                    message_type = 0;
                    user_callback = nullptr;
                    user_data = nullptr;
                }

                VkDebugUtilsMessengerEXT createObject(VkInstance _instance, const void *_p_next = nullptr,
                                                      VkDebugUtilsMessengerCreateFlagsEXT _flags = 0u,
                                                      const VkAllocationCallbacks * _create_cb = nullptr) const {
                    VkDebugUtilsMessengerCreateInfoEXT __info = getVkStruct(_p_next, _flags);
                    VkDebugUtilsMessengerEXT __messenger = VK_NULL_HANDLE;

                    if (vkCreateDebugUtilsMessengerEXT(_instance, &__info, _create_cb, &__messenger) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: DebugMessenger Builder :: createObject","could not create debug messenger"));

                    return __messenger;
                }

                VkDebugUtilsMessengerCreateInfoEXT getVkStruct(const void *_p_next = nullptr, VkDebugUtilsMessengerCreateFlagsEXT _flags = 0u) const {
                    VkDebugUtilsMessengerCreateInfoEXT __res{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
                    __res.pNext = _p_next;
                    __res.flags = _flags;
                    __res.messageSeverity = message_severity;
                    __res.messageType = message_type;
                    __res.pfnUserCallback = user_callback;
                    __res.pUserData = user_data;

                    return __res;
                }

            // Setters

                void setInfo(VkDebugUtilsMessageSeverityFlagsEXT _msg_severity,
                             VkDebugUtilsMessageTypeFlagsEXT _msg_type,
                             PFN_vkDebugUtilsMessengerCallbackEXT _user_cb = nullptr,
                             void* _user_data = nullptr) {
                    message_severity = _msg_severity;
                    message_type = _msg_type;
                    user_callback = _user_cb != nullptr ? _user_cb : user_callback;
                    user_data = _user_data;
                }
            }; // Builder END
        }; // DebugMessengerEXT END


//  ============  Vulkan Physical Device  ============  \\

        namespace PhysicalDevice {
            // This type contains all properties (retreivable via vulkan functions)
            // about one physical device - element of Manager::phys_devices vector.
            // It contains references to phys_devices[i], features[i] and properties[i].
            struct InfoElement {
                const VkPhysicalDevice &phys_device;
                const VkPhysicalDeviceFeatures &features;
                const VkPhysicalDeviceProperties &properties;

                InfoElement(const VkPhysicalDevice &_phys_device,
                            const VkPhysicalDeviceFeatures &_features,
                            const VkPhysicalDeviceProperties &_properties)
                                : phys_device{_phys_device}, features{_features}, properties{_properties} {}
                InfoElement(const InfoElement &_elem_view)
                                : phys_device{_elem_view.phys_device}, features{_elem_view.features},
                                    properties{_elem_view.properties} {}


                vec4u apiVersion() const {
                    vec4u __ver;

                    __ver["xyz"] = VulkanAPIVersion::divide(properties.apiVersion, &__ver.w);
                    return __ver;
                }

                std::string infoString() const {
                    std::string _out = "Physical Device Info:\n";
                    vec4u _ver = apiVersion();
                    _out = _out + "Device name: " + properties.deviceName + '\n'
                                + "API version: " + std::to_string(_ver.x) + '.'
                                                  + std::to_string(_ver.y) + '.'
                                                  + std::to_string(_ver.z) +
                                     " (Variant " + std::to_string(_ver.w) + ")\n";
                    return _out;
                }

            // Scenarios

                std::vector<VkExtensionProperties> getAllDeviceExtensions(const char *_layer = nullptr) const {
                    return Func::enumerateDeviceExtensionProperties(phys_device, _layer);
                }

                bool allExtAvailable(const StringVec &_extensions) const {
                    return Util::allDeviceExtAvailable(phys_device, _extensions);
                }

                void checkExtensions(const StringVec &_extensions, bool *_flags) const {
                    Util::checkDeviceExtensions(phys_device, _extensions, _flags);
                }
            }; // InfoElement END


            namespace Suitability {
                typedef bool (*TestFunc) (const InfoElement &_phys_device_info);
                typedef  int (*RankFunc) (const InfoElement &_phys_device_info);
                // typedef  int (*CompFunc) (const void *_pd1, const void *_pd2);

                // TODO: more useful defaults

                bool defaultGraphicsTest(const InfoElement &_phys_device_info) {
                    printf("%s\n", _phys_device_info.infoString().c_str());
                    return _phys_device_info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                        _phys_device_info.features.geometryShader && _phys_device_info.allExtAvailable({ {VK_KHR_SWAPCHAIN_EXTENSION_NAME} });
                }
            }; // Suitability END


            struct Manager {
                std::vector<VkPhysicalDevice> phys_devices;
                std::vector<VkPhysicalDeviceFeatures> features;
                std::vector<VkPhysicalDeviceProperties> properties;

                Manager(VkInstance _instance = VK_NULL_HANDLE) {
                    if (_instance != VK_NULL_HANDLE)
                        loadInfo(_instance);
                }
                ~Manager() {
                    clearManager();
                }

                void clearManager() {
                    features.clear();
                    properties.clear();
                    phys_devices.clear();
                }

                const InfoElement getElemInfo(uint32_t _index) { return { phys_devices[_index], features[_index], properties[_index] }; }

                void loadInfo(VkInstance _instance) {
                    if (_instance == VK_NULL_HANDLE)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: PhysicalDevice Manager :: loadInfo", "instance is not set"));

                    uint32_t __devCount = 0;
                    vkEnumeratePhysicalDevices(_instance, &__devCount, nullptr);

                    if (__devCount == 0)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: PhysicalDevice Manager :: loadInfo", "no GPUs with Vulkan support available"));

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
                VkPhysicalDevice testPickOne(const Suitability::TestFunc _testFunc) {
                    VkPhysicalDevice __res = 0;
                    for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                        if (_testFunc({phys_devices[i], features[i], properties[i]}))
                            __res = phys_devices[i];
                    }
                    return __res;

                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: PhysicalDevice Manager :: testPickOne", "failed to find a suitable GPU"));
                    return VK_NULL_HANDLE;
                }

                // Returns all physical devices for which _testFunc({device, features, properties}) == true
                std::vector<VkPhysicalDevice> testPickAll(const Suitability::TestFunc _testFunc) {
                    std::vector<VkPhysicalDevice> __res_devices;
                    for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                        if (_testFunc({phys_devices[i], features[i], properties[i]}))
                            __res_devices.push_back(phys_devices[i]);
                    }

                    return __res_devices;
                }

                // Ranks all physical devices and returns one with max _rankFunc({device, features, properties})
                VkPhysicalDevice rankPickOne(const Suitability::RankFunc _rankFunc) {
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
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: PhysicalDevice Manager :: rankPickOne", "failed to find a suitable GPU"));
                    return phys_devices[__ind];
                }

                // Ranks all physical devices and returns one which passes the comparison to _threshold (default: >=)
                template <CompareOp COMP_OP = CompareOp::GE>
                VkPhysicalDevice rankPickOne(const Suitability::RankFunc _rankFunc, int _threshold) {
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
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: PhysicalDevice Manager :: rankPickOne (with threshold)", "failed to find a suitable GPU"));
                    return VK_NULL_HANDLE; // TODO: Maybe return first GPU from the list
                }

                // Ranks all physical devices and returns all which pass the comparison to _threshold (default: >=)
                template <CompareOp COMP_OP = CompareOp::GE>
                std::vector<VkPhysicalDevice> rankPickAll(const Suitability::RankFunc _rankFunc, int _threshold) {
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
                std::vector<VkPhysicalDevice> rankPickAll(const Suitability::RankFunc _rankFunc, std::vector<int> &_ranks, int _threshold) {
                    std::vector<VkPhysicalDevice> __res_devices;

                    for (uint32_t i = 0u; i < phys_devices.size(); ++i) {
                        int __rank = _rankFunc({phys_devices[i], features[i], properties[i]});
                        switch (COMP_OP) {
                            case CompareOp::NE: {
                                if (__rank != _threshold)
                                    _ranks.push_back(__rank);
                                    __res_devices.push_back(phys_devices[i]);
                                break;
                            }
                            case CompareOp::EQ: {
                                if (__rank == _threshold)
                                    _ranks.push_back(__rank);
                                    __res_devices.push_back(phys_devices[i]);
                                break;
                            }
                            case CompareOp::LT: {
                                if (__rank <  _threshold)
                                    _ranks.push_back(__rank);
                                    __res_devices.push_back(phys_devices[i]);
                                break;
                            }
                            case CompareOp::LE: {
                                if (__rank <= _threshold)
                                    _ranks.push_back(__rank);
                                    __res_devices.push_back(phys_devices[i]);
                                break;
                            }
                            case CompareOp::GE: {
                                if (__rank >= _threshold)
                                    _ranks.push_back(__rank);
                                    __res_devices.push_back(phys_devices[i]);
                                break;
                            }
                            case CompareOp::GT: {
                                if (__rank >  _threshold)
                                    _ranks.push_back(__rank);
                                    __res_devices.push_back(phys_devices[i]);
                                break;
                            }
                        }
                    }

                    return __res_devices;
                }
            }; // Manager END
        }; // PhysicalDevice END


//  ============  Vulkan Queue Family  ============  \\

        namespace QueueFamily {
            struct InfoElement {
                const VkQueueFamilyProperties &properties;

                InfoElement(const VkQueueFamilyProperties &_props) : properties{_props} {}

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
            }; // InfoElement END


            namespace Suitability {
                typedef bool (*TestFunc) (const InfoElement &_info, bool _surf_support);
                typedef  int (*RankFunc) (const InfoElement &_info, bool _surf_support);

                bool testGraphics(const InfoElement &_info, bool _surf_support)
                    { return _info.supportsGraphics(); }
                bool testCompute (const InfoElement &_info, bool _surf_support)
                    { return _info.supportsCompute (); }
                bool testTransfer(const InfoElement &_info, bool _surf_support)
                    { return _info.supportsTransfer(); }
                bool testSparseBinding(const InfoElement &_info, bool _surf_support)
                    { return _info.supportsSparseBinding(); }
                bool testProtectedMemory(const InfoElement &_info, bool _surf_support)
                    { return _info.supportsProtectedMemory(); }
                bool testVideoDecode(const InfoElement &_info, bool _surf_support)
                    { return _info.supportsVideoDecode(); }
                bool testVideoEncode(const InfoElement &_info, bool _surf_support)
                    { return _info.supportsVideoEncode(); }
                bool testOpticalFlow(const InfoElement &_info, bool _surf_support)
                    { return _info.supportsOpticalFlow(); }

                bool testSurfAndGraphics(const InfoElement &_info, bool _surf_support)
                    { return _surf_support && _info.supportsGraphics(); }
                bool testSurfAndCompute (const InfoElement &_info, bool _surf_support)
                    { return _surf_support && _info.supportsCompute (); }
                bool testSurfAndTransfer(const InfoElement &_info, bool _surf_support)
                    { return _surf_support && _info.supportsTransfer(); }
                bool testSurfAndSparseBinding(const InfoElement &_info, bool _surf_support)
                    { return _surf_support && _info.supportsSparseBinding(); }
                bool testSurfAndProtectedMemory(const InfoElement &_info, bool _surf_support)
                    { return _surf_support && _info.supportsProtectedMemory(); }
                bool testSurfAndVideoDecode(const InfoElement &_info, bool _surf_support)
                    { return _surf_support && _info.supportsVideoDecode(); }
                bool testSurfAndVideoEncode(const InfoElement &_info, bool _surf_support)
                    { return _surf_support && _info.supportsVideoEncode(); }
                bool testSurfAndOpticalFlow(const InfoElement &_info, bool _surf_support)
                    { return _surf_support && _info.supportsOpticalFlow(); }


            // TODO: practice, then redo these rank functions
            // For now, result >0 means score; if <=0, then queue family doesn't have required feature.

                int rankGraphics(const InfoElement &_info, bool _surf_support) {
                    return 5 * _info.supportsGraphics()
                            - _info.supportsCompute()
                            - _info.supportsVideoDecode()
                            - _info.supportsVideoEncode()
                            - _info.supportsOpticalFlow();
                }
                int rankCompute (const InfoElement &_info, bool _surf_support) {
                    return 5 * _info.supportsCompute()
                            - _info.supportsGraphics()
                            - _info.supportsVideoDecode()
                            - _info.supportsVideoEncode()
                            - _info.supportsOpticalFlow();
                }
                int rankTransfer(const InfoElement &_info, bool _surf_support) {
                    return 6 * _info.supportsTransfer()
                            - _info.supportsCompute()
                            - _info.supportsGraphics()
                            - _info.supportsVideoDecode()
                            - _info.supportsVideoEncode()
                            - _info.supportsOpticalFlow();
                }
                int rankSparseBinding  (const InfoElement &_info, bool _surf_support) {
                    return 6 * _info.supportsSparseBinding()
                            - _info.supportsCompute()
                            - _info.supportsGraphics()
                            - _info.supportsVideoDecode()
                            - _info.supportsVideoEncode()
                            - _info.supportsOpticalFlow();
                }
                int rankProtectedMemory(const InfoElement &_info, bool _surf_support) {
                    return _info.supportsProtectedMemory() ? 1 : -1;
                }
                int rankVideoDecode(const InfoElement &_info, bool _surf_support) {
                    return 5 * _info.supportsVideoDecode()
                            - _info.supportsGraphics()
                            - _info.supportsCompute()
                            - _info.supportsVideoEncode()
                            - _info.supportsOpticalFlow();
                }
                int rankVideoEncode(const InfoElement &_info, bool _surf_support) {
                    return 5 * _info.supportsVideoEncode()
                            - _info.supportsGraphics()
                            - _info.supportsCompute()
                            - _info.supportsVideoDecode()
                            - _info.supportsOpticalFlow();
                }
                int rankOpticalFlow(const InfoElement &_info, bool _surf_support) {
                    return 5 * _info.supportsOpticalFlow()
                            - _info.supportsGraphics()
                            - _info.supportsCompute()
                            - _info.supportsVideoDecode()
                            - _info.supportsVideoEncode();
                }
            }; // Suitability END


            struct Manager {
                std::vector<VkQueueFamilyProperties> properties;
                std::vector<bool> surface_support;

                Manager(VkInstance _instance = VK_NULL_HANDLE, VkPhysicalDevice _phys_device = VK_NULL_HANDLE) {
                    if (_instance != VK_NULL_HANDLE && _phys_device != VK_NULL_HANDLE)
                        loadInfo(_instance, _phys_device);
                }
                ~Manager() {
                    clearManager();
                }

                void clearManager() {
                    properties.clear();
                    surface_support.clear();
                }

                const InfoElement getElemInfo(uint32_t _index) const { return { properties[_index] }; }


                void loadInfo(VkInstance _instance, VkPhysicalDevice _phys_device) {
                    uint32_t __qfamily_count = 0;
                    vkGetPhysicalDeviceQueueFamilyProperties(_phys_device, &__qfamily_count, nullptr);

                    properties.resize(__qfamily_count);
                    vkGetPhysicalDeviceQueueFamilyProperties(_phys_device, &__qfamily_count, properties.data());

                    surface_support.resize(properties.size());
                    for (uint32_t i = 0u; i < properties.size(); ++i)
                        surface_support[i] = glfwGetPhysicalDevicePresentationSupport(_instance, _phys_device, i);
                }


                uint32_t testPickOne(Suitability::TestFunc _test_func) {
                    for (uint32_t i = 0u; i < properties.size(); ++i)
                        if (_test_func(properties[i], surface_support[i]))
                            return i;

                    fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW :: QueueFamily Manager :: testPickOne", "could not find queue family"));
                    return UINT32_MAX;
                }

                std::vector<uint32_t> testPickAll(Suitability::TestFunc _test_func) {
                    std::vector<uint32_t> __indices;

                    for (uint32_t i = 0u; i < properties.size(); ++i)
                        if (_test_func(properties[i], surface_support[i]))
                            __indices.push_back(i);

                    if (__indices.empty())
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW :: QueueFamily Manager :: testPickAll", "could not find queue family"));
                    return __indices;
                }

                uint32_t rankPickOne(Suitability::RankFunc _rank_func) {
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
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW :: QueueFamily Manager :: rankPickOne", "could not find queue family"));
                        return UINT32_MAX;
                    }
                    return __maxI;
                }

                template <CompareOp COMP_OP = CompareOp::GE>
                std::vector<uint32_t> rankPickAll(Suitability::RankFunc _rank_func, int _threshold) {
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
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW :: QueueFamily Manager :: rankPickAll", "could not find queue family"));
                    return __indices;
                }
            }; // Manager END
        }; // QueueFamily END


        namespace Queue {
            VkQueue CreateObject(VkDevice _device, uint32_t _queue_family, uint32_t _queue_index) {
                VkQueue __queue = VK_NULL_HANDLE;
                vkGetDeviceQueue(_device, _queue_family, _queue_index, &__queue);

                return __queue;
            }

            VkDeviceQueueCreateInfo GetVkStructDCopy(uint32_t _queue_family, uint32_t _queue_count, float _eq_priorities_val = 1.f, const void *_p_next = nullptr, VkDeviceQueueCreateFlags _flags = 0u) {
                VkDeviceQueueCreateInfo __res{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
                __res.pNext = _p_next;
                __res.flags = _flags;
                __res.queueFamilyIndex = _queue_family;
                __res.queueCount = _queue_count;
                __res.pQueuePriorities = nullptr;

                if (_queue_count) {
                    float *__priorities_ptr = new float[_queue_count];
                    for (uint32_t i = 0u; i < _queue_count; ++i)
                        __priorities_ptr[i] = _eq_priorities_val;
                    __res.pQueuePriorities = __priorities_ptr;
                }

                return __res;
            }

            VkDeviceQueueCreateInfo GetVkStructDCopy(uint32_t _queue_family, const std::vector<float> _priorities, const void *_p_next = nullptr, VkDeviceQueueCreateFlags _flags = 0u) {
                VkDeviceQueueCreateInfo __res{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
                __res.pNext = _p_next;
                __res.flags = _flags;
                __res.queueFamilyIndex = _queue_family;
                __res.queueCount = _priorities.size();
                __res.pQueuePriorities = nullptr;

                if (_priorities.size()) {
                    float *__priorities_ptr = new float[_priorities.size()];
                    memcpy(__priorities_ptr, _priorities.data(), _priorities.size() * sizeof(float));
                    __res.pQueuePriorities = __priorities_ptr;
                }

                return __res;
            }

            struct Builder {
                std::vector<float> priorities;

                Builder() {}
                ~Builder() {
                    clear();
                }

                void clear() {
                    priorities.clear();
                }

                VkQueue createObject(VkDevice _device, uint32_t _queue_family, uint32_t _queue_index) const {
                    VkQueue __queue = VK_NULL_HANDLE;
                    vkGetDeviceQueue(_device, _queue_family, _queue_index, &__queue);

                    return __queue;
                }

                VkDeviceQueueCreateInfo getVkStructView(uint32_t _queue_family, const void *_p_next = nullptr, VkDeviceQueueCreateFlags _flags = 0u) const {
                    VkDeviceQueueCreateInfo __res{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
                    __res.pNext = _p_next;
                    __res.flags = _flags;
                    __res.queueFamilyIndex = _queue_family;
                    __res.queueCount = priorities.size();
                    __res.pQueuePriorities = priorities.data();

                    return __res;
                }

                VkDeviceQueueCreateInfo getVkStructDCopy(uint32_t _queue_family, const void *_p_next = nullptr, VkDeviceQueueCreateFlags _flags = 0u) const {
                    VkDeviceQueueCreateInfo __res = getVkStructView(_queue_family, _p_next, _flags);

                    __res.pQueuePriorities = nullptr;
                    if (priorities.size()) {
                        float *__priorities_ptr = new float[priorities.size()];
                        memcpy(__priorities_ptr, priorities.data(), priorities.size() * sizeof(float));
                        __res.pQueuePriorities = __priorities_ptr;
                    }

                    return __res;
                }

            // Setters

                // queue count is set as the number of priorities
                void setPriorities(const std::vector<float> &_priorities) {
                    priorities = _priorities;
                }

                void setQueueCountAndEqualPriorities(uint32_t _queue_count, float _eq_priorities_val = 1.f) {
                    priorities.clear();
                    priorities.resize(_queue_count, _eq_priorities_val);
                }
            }; // Builder END
        }; // Queue END


//  ============  Vulkan Logical Device  ============  \\

        namespace Device {
            struct Builder {
                VkPhysicalDeviceFeatures *req_features = nullptr;
                std::vector<VkDeviceQueueCreateInfo> queue_info;
                StringVec layers, extensions;

                Builder() {}
                Builder(const Builder &_builder)
                                : req_features{_builder.req_features}, queue_info{_builder.queue_info},
                                  layers{_builder.layers}, extensions{_builder.extensions} {
                    for (uint32_t i = 0u; i < queue_info.size(); ++i) {
                        float *__ptr = new float[queue_info[i].queueCount];
                        memcpy(__ptr, queue_info[i].pQueuePriorities, queue_info[i].queueCount * sizeof(float));
                        queue_info[i].pQueuePriorities = __ptr;
                    }
                    if (_builder.req_features != nullptr) {
                        req_features = new VkPhysicalDeviceFeatures{*_builder.req_features};
                    }
                }
                ~Builder() {
                    clear();
                }

                void clear() {
                    safeDelete(req_features);

                    for (uint32_t i = 0u; i < queue_info.size(); ++i)
                        Deleters::deleteStructPtrs(queue_info[i]);
                    queue_info.clear();

                    layers.clear();
                    extensions.clear();
                }

                VkDevice createObject(VkPhysicalDevice _phys_device, const void *_p_next = nullptr, VkDeviceCreateFlags _flags = 0u,
                            const VkAllocationCallbacks * _create_cb = nullptr) {
                    VkDeviceCreateInfo __info = getVkStructView(_p_next, _flags);
                    VkDevice __device = VK_NULL_HANDLE;

                    if (vkCreateDevice(_phys_device, &__info, _create_cb, &__device) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Device Builder :: createObject","could not create device"));
                    return __device;
                }

                VkDeviceCreateInfo getVkStructView(const void *_p_next = nullptr, VkDeviceCreateFlags _flags = 0u) const {
                    VkDeviceCreateInfo __res{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
                    __res.flags = _flags;
                    __res.pNext = _p_next;
                    __res.enabledLayerCount = layers.count();
                    __res.ppEnabledLayerNames = layers.getEntries();
                    __res.enabledExtensionCount = extensions.count();
                    __res.ppEnabledExtensionNames = extensions.getEntries();
                    __res.pQueueCreateInfos = queue_info.data();
                    __res.queueCreateInfoCount = queue_info.size();
                    __res.pEnabledFeatures = req_features;

                    return __res;
                }

                // TODO: getVkStructDCopy()

            // Setters

                void setRequiredFeatures(const VkPhysicalDeviceFeatures &_features) {
                    if (req_features == nullptr)
                        req_features = new VkPhysicalDeviceFeatures;
                    *req_features = _features;
                }

                void setLayers(const StringVec &_layers) {
                    layers = _layers;
                }

                void setExtensions(const StringVec &_extensions) {
                    extensions = _extensions;
                }

                void setQueueInfo(const std::vector<VkDeviceQueueCreateInfo> &_info) {
                    queue_info = _info;
                }

            // Scenarios

                void addQueueInfo(const std::vector<VkDeviceQueueCreateInfo> &_info) {
                    queue_info.insert(queue_info.end(), _info.begin(), _info.end());
                }

                void deleteQueueInfo(uint32_t _index) {
                    delete[] queue_info[_index].pQueuePriorities;
                    queue_info.erase(queue_info.begin() + _index);
                }

                void addValLayers(VkPhysicalDevice _phys_device, const StringVec &_val_layers = {{ "VK_LAYER_KHRONOS_validation" }}) {
                    if (!Util::allDeviceLayersAvailable(_phys_device, _val_layers))
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Device Builder :: addValLayers","has unavailable device layers"));

                    addLayers(_val_layers);
                }

                void addLayers(const StringVec &_layers) {
                    layers.addEntries(_layers.list);
                }

                void addExtensions(const StringVec &_extensions) {
                    extensions.addEntries(_extensions.list);
                }
            }; // Builder END
        }; // Device END


//  ============  Vulkan Surface  ============  \\

        namespace SurfaceKHR {
            VkSurfaceKHR CreateObject(VkInstance _instance, Window &_glfw_window, const VkAllocationCallbacks * _create_cb = nullptr) {
                VkSurfaceKHR __surface = VK_NULL_HANDLE;
                if (glfwCreateWindowSurface(_instance, _glfw_window.window, _create_cb, &__surface) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Surface Builder :: CreateObject","could not create surface"));
                return __surface;
            }
        }; // SurfaceKHR END


//  ============ Vulkan Swapchain  ============  \\

        namespace SwapchainKHR {
            namespace Suitability {
                typedef bool (*FormatTestFunc)(const VkSurfaceFormatKHR &_format);
                typedef int  (*FormatRankFunc)(const VkSurfaceFormatKHR &_format);

                static bool defaultFormatTest(const VkSurfaceFormatKHR &_format) {
                    return _format.format == VK_FORMAT_B8G8R8A8_SRGB && _format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR;
                }
            }; // Suitability END


            struct Builder {
                VkSurfaceCapabilitiesKHR capabilities;
                VkSwapchainKHR old_swapchain = VK_NULL_HANDLE;

                uint32_t image_count = 0u;
                struct ImageInfo {
                    VkFormat format = VK_FORMAT_UNDEFINED;
                    VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                    VkExtent2D extent = { 0u, 0u };
                    uint32_t array_layers = 1u;
                    VkImageUsageFlags usage = 0u;
                    // VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE; // set automatically by queue_families (it's non-emptiness)
                } image_info;

                std::vector<uint32_t> queue_families;
                VkSurfaceTransformFlagBitsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
                VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Alpha among windows - almost always opaque
                VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
                bool is_clipped = false;


                Builder() {}
                Builder(const Builder &_builder) = default;
                ~Builder() {
                    clear();
                }

                void clear() {
                    capabilities = VkSurfaceCapabilitiesKHR{};
                }

                VkSwapchainKHR createObject(VkDevice _device, VkSurfaceKHR _surface, const void *_p_next = nullptr, VkSwapchainCreateFlagsKHR _flags = 0u,
                                    const VkAllocationCallbacks * _create_cb = nullptr) const {
                    VkSwapchainCreateInfoKHR __info = getVkStructView(_surface, _p_next, _flags);
                    VkSwapchainKHR __swapchain = VK_NULL_HANDLE;

                    // Can return an error depending on current state of the surface (?) TODO: proper error handling
                    if (vkCreateSwapchainKHR(_device, &__info, _create_cb, &__swapchain) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Swapchain Builder :: createObject","could not create swapchain"));
                    return __swapchain;
                }

                VkSwapchainCreateInfoKHR getVkStructView(VkSurfaceKHR _surface, const void *_p_next = nullptr, VkSwapchainCreateFlagsKHR _flags = 0u) const {
                    VkSwapchainCreateInfoKHR __res{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
                    __res.pNext = _p_next;
                    __res.flags = _flags;
                    __res.surface = _surface;
                    __res.minImageCount = image_count;
                    __res.imageFormat = image_info.format;
                    __res.imageColorSpace = image_info.color_space;
                    __res.imageExtent = image_info.extent;
                    __res.imageArrayLayers = image_info.array_layers;
                    __res.imageUsage = image_info.usage;
                    __res.imageSharingMode = queue_families.size() ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
                    __res.queueFamilyIndexCount = queue_families.size();
                    __res.pQueueFamilyIndices = queue_families.data();
                    __res.preTransform = transform;
                    __res.compositeAlpha = composite_alpha;
                    __res.presentMode = present_mode;
                    __res.clipped = is_clipped;
                    __res.oldSwapchain = old_swapchain;

                    return __res;
                }

                VkSwapchainCreateInfoKHR getVkStructDCopy(VkSurfaceKHR _surface, const void *_p_next = nullptr, VkSwapchainCreateFlagsKHR _flags = 0u) const {
                    VkSwapchainCreateInfoKHR __res = getVkStructView(_surface, _p_next, _flags);

                    __res.pQueueFamilyIndices = nullptr;
                    if (queue_families.size()) {
                        uint32_t *__queue_family_indices = new uint32_t[queue_families.size()];
                        memcpy(__queue_family_indices, queue_families.data(), queue_families.size() * sizeof(uint32_t));
                        __res.pQueueFamilyIndices = __queue_family_indices;
                    }

                    return __res;
                }

                // TODO: some "createObjects" method with vkCreateSharedSwapchainsKHR

            // Setters

                void setOldSwapchain(VkSwapchainKHR _swapchain) {
                    old_swapchain = _swapchain;
                }

                void setSurfaceCapabilities(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface) {
                    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_phys_device, _surface, &capabilities);
                }

                void setSurfaceCapabilities(const VkSurfaceCapabilitiesKHR &_capabilities) {
                    capabilities = _capabilities;
                }

                void setImageCount(uint32_t _count) {
                    image_count = _count;
                }

                void setImageExtent(VkExtent2D _extent) {
                    image_info.extent = _extent;
                }

                void setCurrentExtent() {
                    image_info.extent = capabilities.currentExtent;
                }

                void setImageUsage(VkImageUsageFlags _usage) {
                    image_info.usage = _usage;
                }

                void setQueueFaimilies(const std::vector<uint32_t> &_queue_families) {
                    queue_families = _queue_families;
                }

                void setTransform(VkSurfaceTransformFlagBitsKHR _transform) {
                    transform = _transform;
                }

                void setCurrentTransform() {
                    transform = capabilities.currentTransform;
                }

                void setCompositeAlpha(VkCompositeAlphaFlagBitsKHR _composite_alpha) {
                    composite_alpha = _composite_alpha;
                }

                void setClipped(bool _is_clipped) {
                    is_clipped = _is_clipped;
                }

            // Scenarios

                // Returns input bits after applying capabilities.supportedUsageFlags as a mask.
                // If not equal to input, then some usage flags are not supported
                // and the parameter is not set.
                VkImageUsageFlags checkSetImageUsage(VkImageUsageFlags _usage) {
                    VkImageUsageFlags __res = _usage & capabilities.supportedUsageFlags;

                    if (__res == _usage)
                        image_info.usage = _usage;
                    else
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW :: SwapchainKHR Builder :: checkAndSetImageUsage",
                                                        "some usage flags are not supported\n"));
                    return __res;
                }

                void checkSetImageCount(uint32_t _count) {
                    image_count = Math::clampCL(_count,
                                                 capabilities.minImageCount,
                                                 capabilities.maxImageCount ? // (== 0) means no max
                                                 capabilities.maxImageCount : UINT32_MAX);

                    if (image_count != _count)
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW :: Swapchain Builder :: checkSetImageCount",
                                        "image count (%d) was clamped to %d to fit the interval [%d, %d]\n"),
                                            _count, image_count, capabilities.minImageCount, capabilities.maxImageCount);
                    // else printf(SVKFW_WRAPINFO("VKFW :: Swapchain Builder :: checkSetImageCount", "imageCount: %d, the interval: [%d, %d]\n"),
                    //                         image_count, capabilities.minImageCount, capabilities.maxImageCount);
                }

                bool checkSetExtent(vec2u _extent) {
                    image_info.extent = { _extent.x, _extent.y };

                    if (capabilities.currentExtent.width  != UINT32_MAX &&
                        capabilities.currentExtent.height != UINT32_MAX) {
                        image_info.extent = { Math::clampCL(_extent.x,
                                                capabilities.minImageExtent.width,
                                                capabilities.maxImageExtent.width),
                                            Math::clampCL(_extent.y,
                                                capabilities.minImageExtent.height,
                                                capabilities.maxImageExtent.height)  };
                        // printf(SVKFW_WRAPINFO("VKFW :: Swapchain Builder :: checkSetExtent", "resulting swap extent: [%d, %d]\n"),
                        //                         image_info.extent.width, image_info.extent.height);
                    }
                    return image_info.extent.width  == _extent.x &&
                        image_info.extent.height == _extent.y;
                }

                void chooseCurrentExtent(const Window &_window) {
                    if (capabilities.currentExtent.width != UINT32_MAX) {
                        image_info.extent = capabilities.currentExtent;
                        // printf(SVKFW_WRAPINFO("VKFW :: Swapchain Builder :: chooseCurrentExtent", "from window's current swap extent: [%d, %d]\n"),
                        //                         image_info.extent.width, image_info.extent.height);
                        return;
                    }
                    else {
                        vec2i __fbsize{_window.getFramebufferSize()};

                        image_info.extent = { Math::clampCL((uint32_t)__fbsize.x,
                                                capabilities.minImageExtent.width,
                                                capabilities.maxImageExtent.width),
                                              Math::clampCL((uint32_t)__fbsize.y,
                                                capabilities.minImageExtent.height,
                                                capabilities.maxImageExtent.height)  };
                        // printf(SVKFW_WRAPINFO("VKFW :: Swapchain Builder :: chooseCurrentExtent", "from framebuffer size: [%d, %d]\n"),
                        //                         image_info.extent.width, image_info.extent.height);
                    }
                }


                void pickFormat(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface, const Suitability::FormatTestFunc _test_func) {
                    std::vector<VkSurfaceFormatKHR> __formats{Func::getPhysicalDeviceSurfaceFormatsKHR(_phys_device, _surface)};

                    for (VkSurfaceFormatKHR __format : __formats) {
                        if (_test_func(__format)) {
                            image_info.format      = __format.format;
                            image_info.color_space = __format.colorSpace;
                            printf(SVKFW_WRAPINFO("VKFW :: Swapchain Builder :: pickFormat (1)", "format: %d, color space: %d\n"), uint32_t(__format.format), uint32_t(__format.colorSpace));
                            return;
                        }
                    }
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Swapchain Builder :: pickFormat (1)", "could not choose format"));
                }

                void pickFormat(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface, const Suitability::FormatRankFunc _rank_func) {
                    std::vector<VkSurfaceFormatKHR> __formats{Func::getPhysicalDeviceSurfaceFormatsKHR(_phys_device, _surface)};

                    int __max_rank = INT_MIN;
                    uint32_t __curr_ind = UINT32_MAX;
                    for (uint32_t i = 0u; i < __formats.size(); ++i) {
                        int __rank = _rank_func(__formats[i]);
                        if (__rank > __max_rank) {
                            __max_rank = __rank;
                            __curr_ind = i;
                        }
                    }

                    if (__curr_ind > __formats.size())
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Swapchain Builder :: pickFormat (2)", "could not choose format"));
                    printf(SVKFW_WRAPINFO("VKFW :: Swapchain Builder :: pickFormat (2)", "format: %d, color space: %d\n"),
                            uint32_t(__formats[__curr_ind].format), uint32_t(__formats[__curr_ind].colorSpace));
                    image_info.format      = __formats[__curr_ind].format;
                    image_info.color_space = __formats[__curr_ind].colorSpace;
                }

                bool checkSetPresentationMode(VkPhysicalDevice _phys_device, VkSurfaceKHR _surface, VkPresentModeKHR _pres_mode) {
                    std::vector<VkPresentModeKHR> __pres_modes{Func::getPhysicalDeviceSurfacePresentModesKHR(_phys_device, _surface)};

                    for (VkPresentModeKHR __mode : __pres_modes)
                        if (__mode == _pres_mode) {
                            present_mode = _pres_mode;
                            printf(SVKFW_WRAPINFO("VKFW :: Swapchain Builder :: checkSetPresentationMode", "mode %d\n"), uint32_t(__mode));
                            return true;
                        }

                    fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW :: Swapchain Builder :: checkSetPresentationMode",
                                                        "presentation mode %d not supported\n"), uint32_t(_pres_mode));
                    return false;
                }
            }; // Builder END
        }; // SwapchainKHR END


//  ============  Vulkan Image  ============  \\

        namespace Image {
            // TODO:
            struct Builder {
                VkImageType              imageType;
                VkFormat                 format;
                VkExtent3D               extent;
                uint32_t                 mipLevels;
                uint32_t                 arrayLayers;
                VkSampleCountFlagBits    samples;
                VkImageTiling            tiling;
                VkImageUsageFlags        usage;
                VkSharingMode            sharingMode;
                std::vector<uint32_t> queue_families;
                VkImageLayout            initialLayout;
            }; // Builder END
        };  // Image END


//  ============  Vulkan Image View  ============  \\

        namespace ImageView {
            struct Builder {
                VkImageViewType view_type = VkImageViewType(-1);
                VkFormat format = VK_FORMAT_UNDEFINED;
                VkComponentMapping components{};
                VkImageSubresourceRange subresource_range{};

                Builder() {}
                ~Builder() {
                    clear();
                }

                void clear() {
                    view_type = VkImageViewType(-1);
                    format = VK_FORMAT_UNDEFINED;
                    components = VkComponentMapping{};
                    subresource_range = VkImageSubresourceRange{};
                }

                VkImageView createObject(VkDevice _device, VkImage _image, const void *_p_next = nullptr,
                                        VkImageViewCreateFlags _flags = 0u, const VkAllocationCallbacks * _create_cb = nullptr) const {
                    VkImageViewCreateInfo __info = getVkStruct(_image, _p_next, _flags);
                    VkImageView __image_view = VK_NULL_HANDLE;
                    if (vkCreateImageView(_device, &__info, _create_cb, &__image_view) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: ImageView Builder :: createObject",
                                                                "failed to create image view"));

                    return __image_view;
                }

                VkImageViewCreateInfo getVkStruct(VkImage _image, const void *_p_next = nullptr, VkImageViewCreateFlags _flags = 0u) const {
                    VkImageViewCreateInfo __res{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
                    __res.pNext = _p_next;
                    __res.flags = _flags;
                    __res.image = _image;
                    __res.viewType = view_type;
                    __res.format = format;
                    __res.components = components;
                    __res.subresourceRange = subresource_range;

                    return __res;
                }

            // Setters

                void setRange(VkImageAspectFlags _aspect_mask, uint32_t _base_mip_level,
                              uint32_t _level_count, uint32_t _base_array_layer, uint32_t _array_level_count) {
                    subresource_range.aspectMask = _aspect_mask;
                    subresource_range.baseMipLevel = _base_mip_level;
                    subresource_range.levelCount = _level_count;
                    subresource_range.baseArrayLayer = _base_array_layer;
                    subresource_range.layerCount = _array_level_count;
                }

                void setComponents(VkComponentSwizzle _r = VK_COMPONENT_SWIZZLE_IDENTITY, VkComponentSwizzle _g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                   VkComponentSwizzle _b = VK_COMPONENT_SWIZZLE_IDENTITY, VkComponentSwizzle _a = VK_COMPONENT_SWIZZLE_IDENTITY) {
                    components.r = _r;
                    components.g = _g;
                    components.b = _b;
                    components.a = _a;
                }

                void setViewType(VkImageViewType _view_type) {
                    view_type = _view_type;
                }

                void setFormat(VkFormat _format) {
                    format = _format;
                }
            }; // Builder END
        }; // ImageView END


//  ============  Vulkan Shader Module  ============  \\

        namespace ShaderModule {
            struct Builder {
                std::vector<char> buffer;

                Builder() {}
                Builder(const Builder &_builder) = default;
                Builder(const char *_shader_fpath) {
                    fromFile(_shader_fpath);
                }
                Builder(const char *_buffer_str, size_t _buffer_size) {
                    fromString(_buffer_str, _buffer_size);
                }
                ~Builder() {
                    clear();
                }

                void clear() {
                    buffer.clear();
                }

                VkShaderModule createObject(VkDevice _device, const void *_p_next = nullptr, VkShaderModuleCreateFlags _flags = 0u,
                                        const VkAllocationCallbacks * _create_cb = nullptr) const {
                    VkShaderModuleCreateInfo __info = getVkStructView(_p_next, _flags);
                    VkShaderModule __shader = VK_NULL_HANDLE;
                    if (vkCreateShaderModule(_device, &__info, _create_cb, &__shader) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: ShaderModule Builder :: createObject",
                                                                "failed to create shader module"));

                    return __shader;
                }

                VkShaderModuleCreateInfo getVkStructView(const void *_p_next = nullptr, VkShaderModuleCreateFlags _flags = 0u) const {
                    VkShaderModuleCreateInfo __res{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
                    __res.pNext = _p_next;
                    __res.flags = _flags;
                    __res.codeSize = buffer.size();
                    __res.pCode = reinterpret_cast<const uint32_t *>(buffer.data());

                    return __res;
                }

            // Setters

                void setBuffer(const std::vector<char> &_buffer) {
                    buffer = _buffer;
                }

            // Scenarios

                void fromFile(const char *_fpath) {
                    std::ifstream __shader_file{_fpath, std::ios::ate | std::ios::binary};

                    if (!__shader_file.is_open()) {
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: ShaderModule Builder :: fromFile",
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
                        throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: ShaderModule Builder :: fromString (1)", "no string provided"));
                    if (_length == 0u) {
                        while (_sh_str[_length] != '\0') ++_length;
                        if (_length == 0u)
                            throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: ShaderModule Builder :: fromString (1)", "string is empty"));
                    }

                    buffer.clear();
                    buffer.insert(buffer.end(), _sh_str, _sh_str + _length);
                }

                void fromString(const std::string &_str_buffer) {
                    if (_str_buffer.length() == 0u)
                        throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: ShaderModule Builder :: fromString (2)", "string is empty"));

                    buffer.clear();
                    buffer.insert(buffer.end(), _str_buffer.begin(), _str_buffer.end());
                }
            }; // Builder END
        }; // ShaderModule END


//  ============  Vulkan Specialization Info  ============  \\

        namespace SpecializationInfo {
            struct Builder {
                std::vector<VkSpecializationMapEntry> map_entries;
                std::vector<unsigned char> data;

                Builder() {}
                Builder(const Builder &_builder) = default;
                ~Builder() {
                    clear();
                }

                void clear() {
                    map_entries.clear();
                    data.clear();
                }

                VkSpecializationInfo getVkStructView() const {
                    VkSpecializationInfo __res{};
                    __res.mapEntryCount = map_entries.size();

                    if (__res.mapEntryCount) {
                        __res.pMapEntries = map_entries.data();
                        __res.dataSize = map_entries.back().offset + map_entries.back().size;
                        __res.pData = data.data();
                    }

                    return __res;
                }

                VkSpecializationInfo getVkStructDCopy() const {
                    VkSpecializationInfo __res = getVkStructView();

                    if (__res.mapEntryCount) {
                        VkSpecializationMapEntry *__tmp_entries = new VkSpecializationMapEntry[__res.mapEntryCount];
                        memcpy(__tmp_entries, map_entries.data(), sizeof(VkSpecializationMapEntry) * __res.mapEntryCount);
                        __res.pMapEntries = __tmp_entries;

                        void *__tmp_data_ptr = new unsigned char[__res.dataSize];
                        memcpy(__tmp_data_ptr, data.data(), __res.dataSize);
                        __res.pData = data.data();
                    }

                    return __res;
                }

            // Scenarios

                void addEntry(uint32_t _const_id, void *_data, size_t _size_bytes) {
                    if (data.size() > UINT32_MAX) // hypothetical error
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: SpecializationInfo :: addEntry", "data size makes offset invalid"));

                    VkSpecializationMapEntry __new_entry{ .constantID= _const_id, .offset= (uint32_t) data.size(), .size= _size_bytes };
                    const char *__data_reint = reinterpret_cast<const char*>(_data);

                    data.insert(data.end(), __data_reint, __data_reint + _size_bytes);
                    map_entries.push_back(__new_entry);
                }

            // not implemented
                void delEntry(uint32_t _index) {
                    // TODO:
                }
            }; // Builder END
        };

        namespace Buffer {
            VkBuffer CreateObject(VkDevice _device, const std::vector<uint32_t> &_queue_families, VkSharingMode _sharing_mode, VkDeviceSize _size_bytes,
                                    VkBufferUsageFlags _usage_flags, const void *_p_next = nullptr, VkBufferCreateFlags _flags = 0u, const VkAllocationCallbacks * _create_cb = nullptr) {
                VkBufferCreateInfo __info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
                __info.pNext = _p_next;
                __info.flags = _flags;
                __info.size  = _size_bytes;
                __info.usage = _usage_flags;
                __info.sharingMode = _sharing_mode;
                __info.queueFamilyIndexCount = _queue_families.size();
                __info.pQueueFamilyIndices   = _queue_families.data();

                VkBuffer __buffer = VK_NULL_HANDLE;
                if (vkCreateBuffer(_device, &__info, _create_cb, &__buffer) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Buffer :: CreateObject", "failed to create buffer"));

                return __buffer;
            }

            struct Builder {
                std::vector<uint32_t> queue_families;
                VkSharingMode sharing_mode{VkSharingMode(-1)};
                VkDeviceSize size_bytes = 0ul;
                VkBufferUsageFlags usage_flags = 0u;

                Builder() {}
                Builder(const Builder &_builder) = default;
                ~Builder() {
                    clear();
                }

                void clear() {
                    queue_families.clear();
                    sharing_mode = VkSharingMode(-1);
                    size_bytes = 0ul;
                    usage_flags = 0u;
                }

                VkBuffer createObject(VkDevice _device, const void *_p_next = nullptr, VkBufferCreateFlags _flags = 0u, const VkAllocationCallbacks * _create_cb = nullptr) const {
                    VkBufferCreateInfo __info = getVkStructView(_p_next, _flags);

                    VkBuffer __buffer = VK_NULL_HANDLE;
                    if (vkCreateBuffer(_device, &__info, _create_cb, &__buffer) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Buffer Builder :: createObject", "failed to create buffer"));

                    return __buffer;
                }

                VkBufferCreateInfo getVkStructView(const void *_p_next = nullptr, VkBufferCreateFlags _flags = 0u) const {
                    VkBufferCreateInfo __res{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
                    __res.pNext = _p_next;
                    __res.flags = _flags;
                    __res.size  = size_bytes;
                    __res.usage = usage_flags;
                    __res.sharingMode = sharing_mode;
                    __res.queueFamilyIndexCount = queue_families.size();
                    __res.pQueueFamilyIndices   = queue_families.data();

                    return __res;
                }

                // TODO: getVkStructDCopy

            // Setters

                void setQueueFamilies(const std::vector<uint32_t> &_queue_families) {
                    queue_families = _queue_families;
                }

                void setSharingMode(VkSharingMode _sharing_mode) {
                    sharing_mode = _sharing_mode;
                }

                void setBufferSize(VkDeviceSize _size_bytes) {
                    size_bytes = _size_bytes;
                }

                void setUsageFlags(VkBufferUsageFlags _usage_flags) {
                    usage_flags = _usage_flags;
                }

            // Scenarios

                void setInfo(const std::vector<uint32_t> &_queue_families, VkSharingMode _sharing_mode,
                             VkDeviceSize _size_bytes, VkBufferUsageFlags _usage_flags) {
                    queue_families = _queue_families;
                    sharing_mode = _sharing_mode;
                    size_bytes = _size_bytes;
                    usage_flags = _usage_flags;
                }
            }; // Builder END
        }; // Buffer END


        namespace DeviceMemory {
            // Struct with memory info for one buffer
            VkMemoryAllocateInfo GetVkStruct(VkPhysicalDevice _phys_device, VkDevice _device, VkBuffer _buffer,
                                             VkMemoryPropertyFlags _req_memory_properties, void *_p_next = nullptr) {
                VkMemoryRequirements __mem_requirements;
                vkGetBufferMemoryRequirements(_device, _buffer, &__mem_requirements);

                VkMemoryAllocateInfo __res{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
                __res.pNext = _p_next;
                __res.allocationSize = __mem_requirements.size;
                __res.memoryTypeIndex = Util::checkPhysicalDeviceMemoryProperties(_phys_device, __mem_requirements.memoryTypeBits, _req_memory_properties);

                if (__res.memoryTypeIndex == UINT32_MAX)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: DeviceMemory :: GetVkStruct (1)", "Could not find suitable memory type"));

                return __res;
            }

            VkMemoryAllocateInfo GetVkStruct(VkDeviceSize _alloc_size, uint32_t _mem_type_index, void *_p_next = nullptr) {
                VkMemoryAllocateInfo __res{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
                __res.pNext = _p_next;
                __res.allocationSize = _alloc_size;
                __res.memoryTypeIndex = _mem_type_index;

                return __res;
            }
            // Memory for one buffer
            VkDeviceMemory CreateObject(VkPhysicalDevice _phys_device, VkDevice _device, VkBuffer _buffer, VkMemoryPropertyFlags _req_memory_properties,
                                        void *_p_next = nullptr, const VkAllocationCallbacks * _create_cb = nullptr) {
                VkMemoryAllocateInfo __info = GetVkStruct(_phys_device, _device, _buffer, _req_memory_properties, _p_next);

                VkDeviceMemory __memory;
                vkAllocateMemory(_device, &__info, _create_cb, &__memory);

                return __memory;
            }

            VkDeviceMemory CreateObject(VkDevice _device, VkDeviceSize _alloc_size, uint32_t _mem_type_index,
                                        void *_p_next = nullptr, const VkAllocationCallbacks * _create_cb = nullptr) {
                VkMemoryAllocateInfo __info = GetVkStruct(_alloc_size, _mem_type_index, _p_next);

                VkDeviceMemory __memory;
                vkAllocateMemory(_device, &__info, _create_cb, &__memory);

                return __memory;
            }
        }; // DeviceMemory END



//  ================================================  \\
                Vulkan graphics pipeline
//  ================================================  \\

        namespace Viewport {
            struct StructWrap {
                VkViewport viewport{};

                StructWrap(const VkExtent2D &_extent, float _x = 0.f, float _y = 0.f, float _depthMin = 0.f, float _depthMax = 1.f) :
                            viewport{.x= (float)_x, .y= (float)_y, .width= (float)_extent.width, .height= (float)_extent.height,
                                                    .minDepth= _depthMin, .maxDepth= _depthMax} {}
                StructWrap(const vec2 &_extent, const vec2 &_offset = 0.f, const vec2 &_depth = { 0.f, 1.f }) :
                            viewport{.x= _offset.x, .y= _offset.y, .width= _extent.x, .height= _extent.y,
                                                    .minDepth= _depth.x, .maxDepth= _depth.y} {}
                StructWrap(const StructWrap &_viewport) : viewport{_viewport.viewport} {}
                ~StructWrap() {
                    clear();
                }

                void clear() {
                    viewport = VkViewport{};
                }

                const VkViewport* ptr() const { return &viewport; }

            // Setters

                void setOffset(const VkOffset2D &_offset) { viewport.x = _offset.x; viewport.y = _offset.y; }
                void setExtent(const VkExtent2D &_extent) { viewport.width = _extent.width; viewport.height = _extent.height; }

                void setOffset(const vec2 &_offset) { viewport.x = _offset.x; viewport.y = _offset.y; }
                void setExtent(const vec2 &_extent) { viewport.width = _extent.x; viewport.height = _extent.y; }
                void setDepth (const vec2 &_depth)  { viewport.minDepth = _depth.x; viewport.maxDepth = _depth.y; }

            // Scenarios

                vec2 offset() const { return { viewport.x, viewport.y }; }
                vec2 extent() const { return { viewport.width, viewport.height }; }
                vec2  depth() const { return { viewport.minDepth, viewport.maxDepth }; }
            }; // StructWrap END
        }; // Viewport END


        namespace Rect2D {
            struct StructWrap {
                VkRect2D rectangle{};

                StructWrap() {}
                StructWrap(const StructWrap &_sct) : rectangle{_sct.rectangle} {}
                StructWrap(const VkExtent2D &_extent, const VkOffset2D &_offset = {0, 0})
                                : rectangle{ .offset= {_offset.x, _offset.y}, .extent= {_extent.width, _extent.height} } {}
                StructWrap(const vec2u &_extent, const vec2i &_offset = 0)
                                : rectangle{ .offset= {_offset.x, _offset.y}, .extent= {_extent.x, _extent.y} } {}
                ~StructWrap() {
                    clear();
                }

                void clear() {
                    rectangle = VkRect2D{};
                }

                const VkRect2D* ptr() const { return &rectangle; }

            // Setters

                void setOffset(const vec2i &_offset) { rectangle.offset.x = _offset.x; rectangle.offset.y = _offset.y; }
                void setExtent(const vec2u &_extent) { rectangle.extent.width = _extent.x; rectangle.extent.height = _extent.y; }

            // Scenarios

                vec2i offset() const { return { rectangle.offset.x, rectangle.offset.y }; }
                vec2u extent() const { return { rectangle.extent.width, rectangle.extent.height }; }
            }; // StructWrap END
        }; // Rect2D END


        namespace ClearValue {
            VkClearValue build(float _v0, float _v1, float _v2, float _v3) {
                VkClearValue __res{};
                __res.color.float32[0] = _v0;
                __res.color.float32[1] = _v1;
                __res.color.float32[2] = _v2;
                __res.color.float32[3] = _v3;
                return __res;
            }
            VkClearValue build(int32_t _v0, int32_t _v1, int32_t _v2, int32_t _v3) {
                VkClearValue __res{};
                __res.color.int32[0] = _v0;
                __res.color.int32[1] = _v1;
                __res.color.int32[2] = _v2;
                __res.color.int32[3] = _v3;
                return __res;
            }
            VkClearValue build(uint32_t _v0, uint32_t _v1, uint32_t _v2, uint32_t _v3) {
                VkClearValue __res{};
                __res.color.uint32[0] = _v0;
                __res.color.uint32[1] = _v1;
                __res.color.uint32[2] = _v2;
                __res.color.uint32[3] = _v3;
                return __res;
            }
            VkClearValue build(float _depth, uint32_t _stencil) {
                VkClearValue __res{ .depthStencil = {_depth, _stencil} };
                return __res;
            }
        }; // ClearValue END


        namespace AttachmentDescription {
            struct StructWrap {
                VkAttachmentDescription description{};

                StructWrap() {}
                StructWrap(const StructWrap &_sct) : description{_sct.description} {}
                StructWrap(const VkAttachmentDescription &_description) : description{_description} {}
                ~StructWrap() {
                    clear();
                }

                void clear() {
                    description = VkAttachmentDescription{};
                }

                const VkAttachmentDescription *ptr() const { return &description; }

            // Setters

                void setColorOps(VkAttachmentLoadOp _loadOp, VkAttachmentStoreOp _storeOp) {
                    description.loadOp = _loadOp;
                    description.storeOp = _storeOp;
                }
                void setStencilOps(VkAttachmentLoadOp _loadOp, VkAttachmentStoreOp _storeOp) {
                    description.stencilLoadOp = _loadOp;
                    description.stencilStoreOp = _storeOp;
                }
                void setLayouts(VkImageLayout _initLayout, VkImageLayout _finalLayout) {
                    description.initialLayout = _initLayout;
                    description.finalLayout = _finalLayout;
                }
                void setFormatAndSize(VkFormat _format, VkSampleCountFlagBits _samples) {
                    description.format = _format;
                    description.samples = _samples;
                }
            }; // StructWrap END
        }; // AttachmentDescription END


        namespace SubpassDescription {
            struct StructBuilder {
                VkPipelineBindPoint binding_point{VkPipelineBindPoint(-1)};
                VkAttachmentReference depth_stencil_attachment;
                std::vector<VkAttachmentReference> color_attachments,
                                                   input_attachments,
                                                   resolve_attachments;
                std::vector<uint32_t> preserve_attachments;

                StructBuilder() {}
                ~StructBuilder() {
                    clear();
                }

                void clear() {
                    binding_point = VkPipelineBindPoint(-1);
                    depth_stencil_attachment = VkAttachmentReference{};
                    color_attachments.clear();
                    input_attachments.clear();
                    resolve_attachments.clear();
                    preserve_attachments.clear();
                }

                VkSubpassDescription getVkStructView(VkSubpassDescriptionFlags _flags = 0u) const {
                    VkSubpassDescription __res{};
                    __res.flags = _flags;
                    __res.colorAttachmentCount = color_attachments.size();
                    __res.pColorAttachments    = color_attachments.data();
                    __res.inputAttachmentCount = input_attachments.size();
                    __res.pInputAttachments    = input_attachments.data();
                    __res.preserveAttachmentCount = preserve_attachments.size();
                    __res.pPreserveAttachments    = preserve_attachments.data();
                    __res.pResolveAttachments  = resolve_attachments.data();
                    return __res;
                }

                VkSubpassDescription getVkStructDCopy(VkSubpassDescriptionFlags _flags = 0u) const {
                    VkSubpassDescription __res = getVkStructView(_flags);

                    if (color_attachments.size()) {
                        VkAttachmentReference *__attach_memory = new VkAttachmentReference[color_attachments.size()];
                        memcpy(__attach_memory, color_attachments.data(), sizeof(VkAttachmentReference) * color_attachments.size());
                        __res.pColorAttachments = __attach_memory;
                    }
                    if (input_attachments.size()) {
                        VkAttachmentReference *__attach_memory = new VkAttachmentReference[input_attachments.size()];
                        memcpy(__attach_memory, input_attachments.data(), sizeof(VkAttachmentReference) * input_attachments.size());
                        __res.pColorAttachments = __attach_memory;
                    }
                    if (resolve_attachments.size()) {
                        VkAttachmentReference *__attach_memory = new VkAttachmentReference[resolve_attachments.size()];
                        memcpy(__attach_memory, resolve_attachments.data(), sizeof(VkAttachmentReference) * resolve_attachments.size());
                        __res.pColorAttachments = __attach_memory;
                    }
                    if (preserve_attachments.size()) {
                        uint32_t *__arr_memory = new uint32_t[preserve_attachments.size()];
                        memcpy(__arr_memory, preserve_attachments.data(), sizeof(uint32_t) * preserve_attachments.size());
                        __res.pPreserveAttachments = __arr_memory;
                    }

                    return __res;
                }

            // Setters

                void setColorAttachments(const std::vector<VkAttachmentReference> &_color_attachments) {
                    color_attachments = _color_attachments;
                }

                void setInputAttachments(const std::vector<VkAttachmentReference> &_input_attachments) {
                    input_attachments = _input_attachments;
                }

                void setResolveAttachments(const std::vector<VkAttachmentReference> &_resolveAttachments) {
                    resolve_attachments = _resolveAttachments;
                }

                void setDepthStencilAttachment(VkAttachmentReference _ds_attachment) {
                    depth_stencil_attachment = _ds_attachment;
                }

                void setPreservedAttachments(const std::vector<uint32_t> &_preserve_attachments) {
                    preserve_attachments = _preserve_attachments;
                }

                void setBindingPoint(VkPipelineBindPoint _binding_point) {
                    binding_point = _binding_point;
                }
            }; // Builder END
        }; // SubpassDescription END


        namespace SubpassDependency {
            struct StructWrap {
                VkSubpassDependency subpass_dependency{};

                StructWrap() {}
                StructWrap(const StructWrap &_sct) : subpass_dependency{_sct.subpass_dependency} {}
                StructWrap(const VkSubpassDependency &_subpass_dependency) : subpass_dependency{_subpass_dependency} {}
                ~StructWrap() {
                    clear();
                }

                void clear() {
                    subpass_dependency = VkSubpassDependency{};
                }

                const VkSubpassDependency *ptr() const { return &subpass_dependency; }

            // Setters

                void setSrcSubpass(uint32_t _src_subpass) {
                    subpass_dependency.srcSubpass = _src_subpass;
                }

                void setDstSubpass(uint32_t _dst_subpass) {
                    subpass_dependency.dstSubpass = _dst_subpass;
                }

                void setSrcAccessMask(VkAccessFlags _src_acc_mask) {
                    subpass_dependency.srcAccessMask = _src_acc_mask;
                }

                void setDstAccessMask(VkAccessFlags _dst_acc_mask) {
                    subpass_dependency.dstAccessMask = _dst_acc_mask;
                }

                void setSrcStageMask(VkPipelineStageFlags _src_stage_mask) {
                    subpass_dependency.srcStageMask = _src_stage_mask;
                }

                void setDstStageMask(VkPipelineStageFlags _dst_stage_mask) {
                    subpass_dependency.dstStageMask = _dst_stage_mask;
                }

                void setFlags(VkDependencyFlags _flags) {
                    subpass_dependency.dependencyFlags = _flags;
                }

                void setSubpasses(uint32_t _src_subpass, uint32_t _dst_subpass) {
                    subpass_dependency.srcSubpass = _src_subpass;
                    subpass_dependency.dstSubpass = _dst_subpass;
                }

                void setSrcMasks(VkPipelineStageFlags _src_stage_mask, VkAccessFlags _src_acc_mask) {
                    subpass_dependency.srcStageMask = _src_stage_mask;
                    subpass_dependency.srcAccessMask = _src_acc_mask;
                }

                void setDstMasks(VkPipelineStageFlags _dst_stage_mask, VkAccessFlags _dst_acc_mask) {
                    subpass_dependency.dstStageMask = _dst_stage_mask;
                    subpass_dependency.dstAccessMask = _dst_acc_mask;
                }
            }; // StructWrap END
        }; // SubpassDependency END


//  ============  Vulkan Render Pass  ============  \\

        namespace RenderPass {
            struct Builder {
                std::vector<VkAttachmentDescription> attachments;
                std::vector<VkSubpassDescription> subpasses;
                std::vector<VkSubpassDependency> dependencies;

                Builder() {}
                Builder(const Builder &_builder) = default;
                ~Builder() {
                    clear();
                }


                void clear() {
                    attachments.clear();
                    subpasses.clear();
                    dependencies.clear();
                }

                VkRenderPass createObject(VkDevice _device, const void *_p_next = nullptr, VkRenderPassCreateFlags _flags = 0u,
                                          const VkAllocationCallbacks * _create_cb = nullptr) const {
                    VkRenderPassCreateInfo __info = getVkStructView(_p_next, _flags);
                    VkRenderPass __render_pass = VK_NULL_HANDLE;
                    if (vkCreateRenderPass(_device, &__info, _create_cb, &__render_pass) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: RenderPass Builder :: createObject",
                                                                "failed to create render pass"));

                    return __render_pass;
                }

                VkRenderPassCreateInfo getVkStructView(const void *_p_next = nullptr, VkRenderPassCreateFlags _flags = 0u) const {
                    VkRenderPassCreateInfo __res{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
                    __res.pNext = _p_next;
                    __res.flags = _flags;
                    __res.attachmentCount = attachments.size();
                    __res.pAttachments = attachments.data();
                    __res.subpassCount = subpasses.size();
                    __res.pSubpasses = subpasses.data();
                    __res.dependencyCount = dependencies.size();
                    __res.pDependencies = dependencies.data();

                    return __res;
                }

                VkRenderPassCreateInfo getVkStructDCopy(const void *_p_next = nullptr, VkRenderPassCreateFlags _flags = 0u) const {
                    VkRenderPassCreateInfo __res = getVkStructView(_p_next, _flags);

                    if (attachments.size()) {
                        VkAttachmentDescription *__attachments = new VkAttachmentDescription[attachments.size()];
                        memcpy(__attachments, attachments.data(), attachments.size() * sizeof(VkAttachmentDescription));
                        __res.pAttachments = __attachments;
                    }

                    if (subpasses.size()) {
                        VkSubpassDescription *__subpasses = new VkSubpassDescription[subpasses.size()];
                        memcpy(__subpasses, subpasses.data(), subpasses.size() * sizeof(VkSubpassDescription));
                        __res.pSubpasses = __subpasses;
                    }

                    if (dependencies.size()) {
                        VkSubpassDependency *__dependencies = new VkSubpassDependency[dependencies.size()];
                        memcpy(__dependencies, dependencies.data(), dependencies.size() * sizeof(VkSubpassDependency));
                        __res.pDependencies = __dependencies;
                    }

                    return __res;
                }

            // Setters

                void setAttachments(const std::vector<VkAttachmentDescription> &_attachments) {
                    attachments = _attachments;
                }

                // void setAttachments(const std::vector<AttachmentDescription::StructWrap> &_attachments) {
                //     attachments.resize(_attachments.size());
                //     for (uint32_t i = 0u; i < _attachments.size(); ++i)
                //         attachments[i] = _attachments[i].description;
                // }

                void setSubpasses(const std::vector<VkSubpassDescription> &_subpasses) {
                    subpasses = _subpasses;
                }

                void setDependencies(const std::vector<VkSubpassDependency> &_dependencies) {
                    dependencies = _dependencies;
                }
            }; // Builder END
        }; // RenderPass END


        namespace WriteDescriptorSet {
            struct Builder {
                VkDescriptorSet descriptor_set;
                VkDescriptorType descriptor_type;
                uint32_t descriptor_count = 0u;
                uint32_t dst_binding;
                uint32_t dst_arr_element;
                std::vector<VkDescriptorBufferInfo> buffer_infos;
                std::vector<VkDescriptorImageInfo >  image_infos;
                std::vector<VkBufferView> texel_buf_views;

                Builder() {}

                // If 'descriptor_count' is 0, using max size among 'buffer_infos', 'image_infos' and 'texel_buf_views' vectors
                VkWriteDescriptorSet getVkStructView(void *_p_next = nullptr) {
                    VkWriteDescriptorSet __res{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                    __res.pNext = _p_next;
                    __res.dstSet = descriptor_set;
                    __res.dstBinding = dst_binding;
                    __res.dstArrayElement = dst_arr_element;
                    __res.descriptorCount = descriptor_count ? descriptor_count : std::max(std::max(buffer_infos.size(), image_infos.size()), texel_buf_views.size());
                    __res.descriptorType = descriptor_type;
                    __res.pImageInfo = image_infos.size() ? image_infos.data() : nullptr;
                    __res.pBufferInfo = buffer_infos.size() ? buffer_infos.data() : nullptr;
                    __res.pTexelBufferView = texel_buf_views.size() ? texel_buf_views.data() : nullptr;

                    return __res;
                }


            // Scenarios
                // If 'descriptor_count' is 0, using max size among 'buffer_infos', 'image_infos' and 'texel_buf_views' vectors
                void setInfo(VkDescriptorSet _descr_set, VkDescriptorType _descriptor_type, uint32_t _dst_binding, uint32_t _descriptor_count = 0u, uint32_t _dst_arr_element = 0u) {
                    descriptor_set = _descr_set;
                    descriptor_type = _descriptor_type;
                    dst_binding = _dst_binding;
                    dst_arr_element = _dst_arr_element;
                }

                void addBufferInfo(VkBuffer _buffer, VkDeviceSize _offset = 0ul, VkDeviceSize _range = VK_WHOLE_SIZE) {
                    buffer_infos.push_back(VkDescriptorBufferInfo{ .buffer = _buffer, .offset = _offset, .range = _range });
                }

                void addImageInfo(VkSampler _sampler, VkImageView _image_view, VkImageLayout _image_layout) {
                    image_infos.push_back(VkDescriptorImageInfo{ .sampler = _sampler, .imageView = _image_view, .imageLayout = _image_layout });
                }

                void setTexelBufferViews(const std::vector<VkBufferView> &_texel_buf_views) {
                    texel_buf_views = _texel_buf_views;
                }
            }; // Builder END
        }; // WriteDescriptorSet END


        namespace DescriptorSetLayoutBinding {
            struct Builder {
                uint32_t binding;
                VkDescriptorType descriptor_type;
                VkShaderStageFlags stage_flags;
                std::vector<VkSampler> immutable_samplers;

                Builder() {}
                Builder(const Builder &_builder) = default;
                ~Builder() {
                    clear();
                }

                void clear() {
                    binding = -1;
                    descriptor_type = VkDescriptorType(-1);
                    stage_flags = 0;
                    immutable_samplers.clear();
                }

                VkDescriptorSetLayoutBinding getVkStructView() const {
                    VkDescriptorSetLayoutBinding __res;
                    __res.binding = binding;
                    __res.descriptorType = descriptor_type;
                    __res.descriptorCount = immutable_samplers.size();
                    __res.stageFlags = stage_flags;
                    __res.pImmutableSamplers = immutable_samplers.data();

                    return __res;
                }

                // TODO: getVkStructDCopy()

                void setInfo(uint32_t _binding, VkDescriptorType _descriptor_type, VkShaderStageFlags _stage_flags,
                             const std::vector<VkSampler> &_immutable_samplers = {}) {
                    binding = _binding;
                    descriptor_type = _descriptor_type;
                    stage_flags = _stage_flags;
                    immutable_samplers = _immutable_samplers;
                }
            }; // Builder END
        }; // DescriptorSetLayoutBinding END

        namespace DescriptorSetLayout {
            struct Builder {
                std::vector<std::vector<VkSampler>> immutable_samplers;
                std::vector<VkDescriptorSetLayoutBinding> bindings;

                Builder() {}
                Builder(const Builder &_builder) = default;
                ~Builder() {
                    clear();
                }

                void clear() {
                    bindings.clear();
                }

                VkDescriptorSetLayout createObject(VkDevice _device, const void *_p_next = nullptr,
                                                   VkDescriptorSetLayoutCreateFlags _flags = 0u,
                                                   const VkAllocationCallbacks *_create_cb = nullptr) const {
                    VkDescriptorSetLayoutCreateInfo __info = getVkStructView(_p_next, _flags);
                    VkDescriptorSetLayout __layout = VK_NULL_HANDLE;
                    if (vkCreateDescriptorSetLayout(_device, &__info, _create_cb, &__layout) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: DescriptorSetLayout Builder :: createObject", "could not create descriptor set layout"));

                    return __layout;
                }

                VkDescriptorSetLayoutCreateInfo getVkStructView(const void *_p_next = nullptr, VkDescriptorSetLayoutCreateFlags _flags = 0u) const {
                    VkDescriptorSetLayoutCreateInfo __res{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
                    __res.pNext = _p_next;
                    __res.flags = _flags;
                    __res.bindingCount = bindings.size();
                    __res.pBindings = bindings.data();

                    return __res;
                }

                // TODO: getVkStructDCopy()

            // Setters

                // TODO: maybe rewrite the logic

                void setBindingsSize(uint32_t _size) {
                    immutable_samplers.resize(_size);
                    bindings.resize(_size);
                }

                // 'builder_binding_index' - index in Builder's array of bindings
                // 'binding' is a member of VkDescriptorSetLayoutBinding struct
                void setBinding(uint32_t _builder_binding_index, uint32_t _binding, VkDescriptorType _descr_type, uint32_t _descr_count,
                                VkShaderStageFlags _stage_flags, const std::vector<VkSampler> &_immutable_samplers = {}) {
                    immutable_samplers[_builder_binding_index] = _immutable_samplers;
                    bindings[_builder_binding_index] = VkDescriptorSetLayoutBinding{ .binding= _binding,
                                                                                     .descriptorType= _descr_type,
                                                                                     .descriptorCount= _descr_count,
                                                                                     .stageFlags= _stage_flags,
                                                                                     .pImmutableSamplers= immutable_samplers[_builder_binding_index].data() };
                }
            }; // Builder END
        }; // DescriptorSetLayout END

        namespace DescriptorPool {
            VkDescriptorPoolCreateInfo GetVkStructView(uint32_t _max_sets, const std::vector<VkDescriptorPoolSize> &_pool_sizes = {},
                                                       const void *_p_next = nullptr, VkDescriptorPoolCreateFlags _flags = 0u) {
                VkDescriptorPoolCreateInfo __res{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
                __res.pNext = _p_next;
                __res.flags = _flags;
                __res.maxSets = _max_sets;
                __res.poolSizeCount = (uint32_t)_pool_sizes.size();
                __res.pPoolSizes = _pool_sizes.data();

                return __res;
            }

            VkDescriptorPool CreateObject(VkDevice _device, uint32_t _max_sets, const std::vector<VkDescriptorPoolSize> &_pool_sizes = {},
                                          const void *_p_next = nullptr, VkDescriptorPoolCreateFlags _flags = 0u, const VkAllocationCallbacks *_create_cb = nullptr) {
                VkDescriptorPoolCreateInfo __info = GetVkStructView(_max_sets, _pool_sizes, _p_next, _flags);
                VkDescriptorPool __pool;
                if (vkCreateDescriptorPool(_device, &__info, _create_cb, &__pool) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: DescriptorPool :: CreateObject", "could not create descriptor pool"));

                return __pool;
            }
        }; // DescriptorPool END
        // TODO:
        // namespace DescriptorSet {
        //     VkDescriptorSetAllocateInfo GetVkStruct() {

        //     }
        // }; // DescriptorSet END


//  ============  Vulkan Pipeline  ============  \\

        namespace Pipeline {

//  ============  Pipeline Layout  ============  \\

            namespace Layout {
                VkPipelineLayoutCreateInfo GetVkStructView(const std::vector<VkPushConstantRange> &_constants = {},
                                                           const std::vector<VkDescriptorSetLayout> &_layouts = {},
                                                           const void *_p_next = nullptr, VkPipelineLayoutCreateFlags _flags = 0u) {
                    VkPipelineLayoutCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
                    __res.pNext = _p_next;
                    __res.flags = _flags;
                    __res.setLayoutCount = _layouts.size();
                    __res.pSetLayouts = _layouts.data();
                    __res.pushConstantRangeCount = _constants.size();
                    __res.pPushConstantRanges = _constants.data();

                    return __res;
                }

                VkPipelineLayoutCreateInfo GetVkStructDCopy(const std::vector<VkPushConstantRange> &_constants = {},
                                                            const std::vector<VkDescriptorSetLayout> &_layouts = {},
                                                            const void *_p_next = nullptr, VkPipelineLayoutCreateFlags _flags = 0u) {
                    VkPipelineLayoutCreateInfo __res = GetVkStructView(_constants, _layouts, _p_next, _flags);

                    if (_layouts.size()) {
                        VkDescriptorSetLayout *__set_layouts = new VkDescriptorSetLayout[_layouts.size()];
                        memcpy(__set_layouts, _layouts.data(), _layouts.size() * sizeof(VkDescriptorSetLayout));
                        __res.pSetLayouts = __set_layouts;
                    }
                    if (_constants.size()) {
                        VkPushConstantRange *__push_constant_ranges = new VkPushConstantRange[_constants.size()];
                        memcpy(__push_constant_ranges, _constants.data(), _constants.size() * sizeof(VkPushConstantRange));
                        __res.pPushConstantRanges = __push_constant_ranges;
                    }

                    return __res;
                }

                VkPipelineLayout CreateObject(VkDevice _device, const std::vector<VkPushConstantRange> &_constants = {},
                                              const std::vector<VkDescriptorSetLayout> &_layouts = {}, const void *_p_next = nullptr,
                                              VkPipelineLayoutCreateFlags _flags = 0u, const VkAllocationCallbacks * _create_cb = nullptr) {
                    VkPipelineLayoutCreateInfo __info = GetVkStructView(_constants, _layouts, _p_next, _flags);
                    VkPipelineLayout __pipeline_layout = VK_NULL_HANDLE;
                    if (vkCreatePipelineLayout(_device, &__info, _create_cb, &__pipeline_layout) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Pipeline :: Layout :: CreateObject", "could not create pipeline layout"));

                    return __pipeline_layout;
                }


                struct Builder {
                    std::vector<VkPushConstantRange> constants;

                    Builder() {}
                    Builder(const Builder &_builder) = default;
                    ~Builder() {
                        clear();
                    }

                    void clear() {
                        constants.clear();
                    }

                    VkPipelineLayout createObject(VkDevice _device, const std::vector<VkDescriptorSetLayout> &_layouts = {}, const void *_p_next = nullptr, VkPipelineLayoutCreateFlags _flags = 0u,
                                                  const VkAllocationCallbacks * _create_cb = nullptr) const {
                        VkPipelineLayoutCreateInfo __info = getVkStructView(_layouts, _p_next, _flags);
                        VkPipelineLayout __pipeline_layout = VK_NULL_HANDLE;
                        if (vkCreatePipelineLayout(_device, &__info, _create_cb, &__pipeline_layout) != VK_SUCCESS)
                            throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Pipeline :: Layout Builder :: createObject", "could not create pipeline layout"));

                        return __pipeline_layout;
                    }

                    VkPipelineLayoutCreateInfo getVkStructView(const std::vector<VkDescriptorSetLayout> &_layouts = {}, const void *_p_next = nullptr, VkPipelineLayoutCreateFlags _flags = 0u) const {
                        VkPipelineLayoutCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
                        __res.pNext = _p_next;
                        __res.flags = _flags;
                        __res.setLayoutCount = _layouts.size();
                        __res.pSetLayouts = _layouts.data();
                        __res.pushConstantRangeCount = constants.size();
                        __res.pPushConstantRanges = constants.data();

                        return __res;
                    }

                    VkPipelineLayoutCreateInfo getVkStructDCopy(const std::vector<VkDescriptorSetLayout> &_layouts = {}, const void *_p_next = nullptr, VkPipelineLayoutCreateFlags _flags = 0u) const {
                        VkPipelineLayoutCreateInfo __res = getVkStructView(_layouts, _p_next, _flags);

                        if (_layouts.size()) {
                            VkDescriptorSetLayout *__set_layouts = new VkDescriptorSetLayout[_layouts.size()];
                            memcpy(__set_layouts, _layouts.data(), _layouts.size() * sizeof(VkDescriptorSetLayout));
                            __res.pSetLayouts = __set_layouts;
                        }
                        if (constants.size()) {
                            VkPushConstantRange *__push_constant_ranges = new VkPushConstantRange[constants.size()];
                            memcpy(__push_constant_ranges, constants.data(), constants.size() * sizeof(VkPushConstantRange));
                            __res.pPushConstantRanges = __push_constant_ranges;
                        }

                        return __res;
                    }

                // Setters

                    void setConstRanges(const std::vector<VkPushConstantRange> &_constants) {
                        constants = _constants;
                    }
                }; // Builder END
            }; // Layout END


//  ============  Pipeline Cache  ============  \\

            namespace Cache {
                struct Builder {
                    size_t cache_data_size = 0u;
                    void  *cache_data = nullptr;

                    Builder() {}
                    ~Builder() {
                        clear();
                    }

                    void clear() {
                        if (cache_data_size && cache_data != nullptr)
                            delete[] (unsigned char*)cache_data;

                        cache_data = nullptr;
                        cache_data_size = 0u;
                    }

                    VkPipelineCache createObject(VkDevice _device, const void *_p_next = nullptr, VkPipelineCacheCreateFlags _flags = 0u,
                                                 const VkAllocationCallbacks * _create_cb = nullptr) const {
                        VkPipelineCacheCreateInfo __info = getVkStructView(_p_next, _flags);
                        VkPipelineCache __cache = VK_NULL_HANDLE;
                        if (vkCreatePipelineCache(_device, &__info, _create_cb, &__cache) != VK_SUCCESS)
                            throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Pipeline :: Cache Builder :: createObject", "could not create pipeline cache"));

                        return __cache;
                    }

                    VkPipelineCacheCreateInfo getVkStructView(const void *_p_next = nullptr, VkPipelineCacheCreateFlags _flags = 0u) const {
                        VkPipelineCacheCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
                        __res.pNext = _p_next;
                        __res.flags = _flags;
                        __res.initialDataSize = cache_data_size;
                        __res.pInitialData = cache_data;

                        return __res;
                    }

                // Scenarios

                    void getCacheData(VkDevice _device, VkPipelineCache _pipeline_cache) {
                        cache_data = Func::getPipelineCacheData(_device, _pipeline_cache, cache_data_size);
                    }
                }; // Builder END
            }; // Cache END

//  ============  Pipeline Shader Stage  ============  \\

            namespace ShaderStage {
                struct StructBuilder {
                    VkShaderModule shader_module = VK_NULL_HANDLE; // exception from main idea, no reason to allow for (shader_module + entry_point_name + stage) to be set separately
                    const char *entry_point_name = nullptr;
                    VkShaderStageFlagBits stage;
                    VkSpecializationInfo spec_info{ 0u, nullptr, 0u, nullptr };

                    StructBuilder() {}
                    StructBuilder(const StructBuilder &_builder)
                                    : entry_point_name{_builder.entry_point_name},
                                    stage{_builder.stage}, spec_info{_builder.spec_info} {}
                    ~StructBuilder() {
                        clear();
                    }

                    void clear() {
                        entry_point_name = nullptr;
                        stage = VkShaderStageFlagBits(0);
                        spec_info = { 0u, nullptr, 0u, nullptr };
                    }

                    VkPipelineShaderStageCreateInfo getVkStructView(const void *_p_next = nullptr, VkPipelineShaderStageCreateFlags _flags = 0u) const {
                        VkPipelineShaderStageCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
                        __res.pNext = _p_next;
                        __res.flags = _flags;
                        __res.stage = stage;
                        __res.module = shader_module;
                        __res.pName = entry_point_name;
                        __res.pSpecializationInfo = spec_info.mapEntryCount ? &spec_info : nullptr;

                        return __res;
                    }

                    VkPipelineShaderStageCreateInfo getVkStructDCopy(const void *_p_next = nullptr, VkPipelineShaderStageCreateFlags _flags = 0u) const {
                        VkPipelineShaderStageCreateInfo __res = getVkStructView(_p_next, _flags);
                        __res.pSpecializationInfo = spec_info.mapEntryCount ? new VkSpecializationInfo(spec_info) : nullptr;

                        return __res;
                    }

                // Setters

                    void setShaderInfo(VkShaderModule _shader_module, VkShaderStageFlagBits _stage, const char *_name) {
                        shader_module = _shader_module;
                        stage = _stage;
                        entry_point_name = _name;
                    }

                    void setSpecializationInfo(const VkSpecializationInfo &_spec_info) {
                        spec_info = _spec_info;
                    }
                }; // StructBuilder END
            }; // ShaderStage END


//  ============  Pipeline Dynamic State  ============  \\

            namespace DynamicState {
                struct StructBuilder {
                    std::vector<VkDynamicState> dynamic_states;

                    StructBuilder() {}
                    StructBuilder(const StructBuilder &_builder) : dynamic_states{_builder.dynamic_states} {}
                    ~StructBuilder() {
                        clear();
                    }

                    void clear() {
                        dynamic_states.clear();
                    }

                    VkPipelineDynamicStateCreateInfo getVkStructView(const void *_p_next = nullptr, VkPipelineDynamicStateCreateFlags _flags = 0u) const {
                        VkPipelineDynamicStateCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
                        __res.pNext = _p_next;
                        __res.flags = _flags;
                        __res.dynamicStateCount = dynamic_states.size();
                        __res.pDynamicStates = dynamic_states.data();

                        return __res;
                    }

                    VkPipelineDynamicStateCreateInfo getVkStructDCopy(const void *_p_next = nullptr, VkPipelineDynamicStateCreateFlags _flags = 0u) const {
                        VkPipelineDynamicStateCreateInfo __res = getVkStructView(_p_next, _flags);

                        if (dynamic_states.size()) {
                            VkDynamicState *__dynamic_states = new VkDynamicState[dynamic_states.size()];
                            memcpy(__dynamic_states, dynamic_states.data(), dynamic_states.size() * sizeof(VkDynamicState));
                            __res.pDynamicStates = __dynamic_states;
                        }

                        return __res;
                    }

                // Setters

                    void setDynamicStates(const std::vector<VkDynamicState> &_dynamic_states) {
                        dynamic_states = _dynamic_states;
                    }

                // Scenarios

                    // returns false if state is already included in "dynamic_states", true if successfully added
                    bool addDynamicState(VkDynamicState _state) {
                        for (const VkDynamicState &st : dynamic_states)
                            if (st == _state) return false;

                        dynamic_states.push_back(_state);
                        return true;
                    }

                    // returns false if state is not in "dynamic_states", true if successfully removed first entry
                    bool delDynamicState(VkDynamicState _state) {
                        for (uint32_t i = 0u; i < dynamic_states.size(); ++i) {
                            if (dynamic_states[i] == _state) {
                                dynamic_states.erase(dynamic_states.begin() + i);
                                return true;
                            }
                        }
                        return false;
                    }
                }; // StructBuilder END
            }; // DynamicState END


//  ============  Pipeline Vertex Input State  ============  \\

            namespace VertexInputState {
                struct StructBuilder {
                    std::vector<VkVertexInputBindingDescription> bindings;
                    std::vector<VkVertexInputAttributeDescription> attributes;

                    StructBuilder() {}
                    ~StructBuilder() {
                        clear();
                    }

                    void clear() {
                        bindings.clear();
                        attributes.clear();
                    }

                    VkPipelineVertexInputStateCreateInfo getVkStructView(const void *_p_next = nullptr, VkPipelineVertexInputStateCreateFlags _flags = 0u) const {
                        VkPipelineVertexInputStateCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
                        __res.flags = _flags;
                        __res.pNext = _p_next;
                        __res.vertexBindingDescriptionCount = bindings.size();
                        __res.pVertexBindingDescriptions    = bindings.data();
                        __res.vertexAttributeDescriptionCount = attributes.size();
                        __res.pVertexAttributeDescriptions    = attributes.data();

                        return __res;
                    }

                    VkPipelineVertexInputStateCreateInfo getVkStructDCopy(const void *_p_next = nullptr, VkPipelineVertexInputStateCreateFlags _flags = 0u) const {
                        VkPipelineVertexInputStateCreateInfo __res = getVkStructView(_p_next, _flags);

                        if (bindings.size()) {
                            VkVertexInputBindingDescription *__descr_ptr = new VkVertexInputBindingDescription[bindings.size()];
                            memcpy(__descr_ptr, bindings.data(), bindings.size() * sizeof(VkVertexInputBindingDescription));
                            __res.pVertexBindingDescriptions = __descr_ptr;
                        }
                        if (attributes.size()) {
                            VkVertexInputAttributeDescription *__descr_ptr = new VkVertexInputAttributeDescription[attributes.size()];
                            memcpy(__descr_ptr, attributes.data(), attributes.size() * sizeof(VkVertexInputAttributeDescription));
                            __res.pVertexAttributeDescriptions = __descr_ptr;
                        }

                        return __res;
                    }

                // Setters

                    void setBindings(const std::vector<VkVertexInputBindingDescription> &_bindings) {
                        bindings = _bindings;
                    }

                    void setAttributes(const std::vector<VkVertexInputAttributeDescription> &_attributes) {
                        attributes = _attributes;
                    }
                }; // StructBuilder END
            }; // VertexInputState END


//  ============  Pipeline Input Assembly State  ============  \\

            namespace InputAssemblyState {
                struct StructBuilder {
                    VkPrimitiveTopology topology = VkPrimitiveTopology(-1);
                    VkBool32 primitive_restart = VK_FALSE;

                    StructBuilder() {}
                    StructBuilder(const StructBuilder &_sct)
                                    : topology{_sct.topology}, primitive_restart{_sct.primitive_restart} {}
                    StructBuilder(VkPrimitiveTopology _topology, VkBool32 _primitive_restart)
                                    : topology{_topology}, primitive_restart{_primitive_restart} {}
                    ~StructBuilder() {
                        clear();
                    }

                    void clear() {
                        topology = VkPrimitiveTopology(-1);
                        primitive_restart = VK_FALSE;
                    }

                    VkPipelineInputAssemblyStateCreateInfo getVkStruct(const void *_p_next = nullptr, VkPipelineInputAssemblyStateCreateFlags _flags = 0u) const {
                        VkPipelineInputAssemblyStateCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
                        __res.pNext = _p_next;
                        __res.flags = _flags;
                        __res.topology = topology;
                        __res.primitiveRestartEnable = primitive_restart;

                        return __res;
                    }

                // Setters

                    void setTopology(VkPrimitiveTopology _topology_type) {
                        topology = _topology_type;
                    }

                    void setRestart(bool _primitive_restart) {
                        primitive_restart = VkBool32(_primitive_restart);
                    }
                }; // StructBuilder END
            }; // InputAssemblyState END


//  ============  Pipeline Viewport State  ============  \\

            namespace ViewportState {
                struct StructBuilder {
                    std::vector<VkViewport> viewports;
                    std::vector<VkRect2D> scissors;
                    uint32_t viewports_dynamic_count = 0u, scissors_dynamic_count = 0u;
                    // counts can be set with nullptr pointers - dynamic mode

                    StructBuilder() {}
                    StructBuilder(const StructBuilder &_builder) : viewports{_builder.viewports}, scissors{_builder.scissors} {}
                    ~StructBuilder() {
                        clear();
                    }

                    void clear() {
                        viewports.clear();
                        scissors.clear();
                    }

                    VkPipelineViewportStateCreateInfo getVkStructView(const void *_p_next = nullptr, VkPipelineViewportStateCreateFlags _flags = 0u) const {
                        VkPipelineViewportStateCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
                        __res.flags = _flags;
                        __res.pNext = _p_next;
                        __res.viewportCount = viewports_dynamic_count;
                        if (!viewports_dynamic_count) {
                            __res.viewportCount = viewports.size();
                            __res.pViewports    = viewports.data();
                        }
                        __res.scissorCount  = scissors_dynamic_count;
                        if (!scissors_dynamic_count) {
                            __res.scissorCount  =  scissors.size();
                            __res.pScissors     =  scissors.data();
                        }

                        return __res;
                    }

                    VkPipelineViewportStateCreateInfo getVkStructDCopy(const void *_p_next = nullptr, VkPipelineViewportStateCreateFlags _flags = 0u) const {
                        VkPipelineViewportStateCreateInfo __res = getVkStructView(_p_next, _flags);

                        if (!viewports_dynamic_count && viewports.size()) {
                            VkViewport *__viewport_ptr = new VkViewport[__res.viewportCount];
                            memcpy(__viewport_ptr, viewports.data(), viewports.size() * sizeof(VkViewport));
                            __res.pViewports = __viewport_ptr;
                        }

                        if (!scissors_dynamic_count && scissors.size()) {
                            VkRect2D  *__scissor_ptr = new VkRect2D[__res.scissorCount];
                            memcpy(__scissor_ptr, scissors.data(), scissors.size() * sizeof(VkRect2D));
                            __res.pScissors = __scissor_ptr;
                        }

                        return __res;
                    }

                // Setters

                    void setViewportsDynamic(uint32_t _viewports_count) {
                        viewports.clear();
                        viewports_dynamic_count = _viewports_count;
                    }

                    void setScissorsDynamic(uint32_t _scissors_count) {
                        scissors.clear();
                        scissors_dynamic_count = _scissors_count;
                    }

                    void setViewports(const std::vector<Viewport::StructWrap> &_viewports) {
                        viewports.resize(_viewports.size());
                        for (uint32_t i = 0u; i < _viewports.size(); ++i)
                            viewports[i] = _viewports[i].viewport;
                    }

                    void setScissors(const std::vector<Rect2D::StructWrap> &_scissors) {
                        scissors.resize(_scissors.size());
                        for (uint32_t i = 0u; i < _scissors.size(); ++i)
                            scissors[i] = _scissors[i].rectangle;
                    }

                    void setFromSwapchainExtent(VkExtent2D _extent) {
                        setViewports({ {_extent, 0.f, 0.f, 0.f, 1.f} });
                        setScissors ({ {_extent, {0, 0}} });
                    }

                // Scenarios

                    void addViewports(const std::vector<Viewport::StructWrap> &_viewports) {
                        viewports.reserve(viewports.size() + _viewports.size());
                        for (const auto &__viewport : _viewports)
                            viewports.push_back(__viewport.viewport);
                    }

                    void addScissors(const std::vector<Rect2D::StructWrap> &_scissors) {
                        scissors.reserve(scissors.size() + _scissors.size());
                        for (const auto &__scissor : _scissors)
                            scissors.push_back(__scissor.rectangle);
                    }
                }; // StructBuilder END
            }; // ViewportState END


//  ============  Pipeline Rasterization State  ============  \\

            namespace RasterizationState {
                struct StructBuilder {
                    bool depth_clamp_enable = false;
                    bool rasterizer_discard_enable = false;
                    VkPolygonMode polygon_mode = VK_POLYGON_MODE_FILL;
                    VkCullModeFlags cull_mode = VK_CULL_MODE_NONE;
                    VkFrontFace front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
                    bool  depth_bias_enable = false;
                    float depth_bias_constant_factor = 0.f;
                    float depth_bias_clamp = 0.f;
                    float depth_bias_slope_factor = 0.f;
                    float line_width = 0.f;

                    StructBuilder() {}
                    StructBuilder(const StructBuilder &_sct) = default;
                    ~StructBuilder() {
                        clear();
                    }

                    void clear() {
                        depth_clamp_enable = false;
                        rasterizer_discard_enable = false;
                        polygon_mode = VK_POLYGON_MODE_FILL;
                        cull_mode = VK_CULL_MODE_NONE;
                        front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
                        depth_bias_enable = false;
                        depth_bias_constant_factor = 0.f;
                        depth_bias_clamp = 0.f;
                        depth_bias_slope_factor = 0.f;
                        line_width = 0.f;
                    }

                    VkPipelineRasterizationStateCreateInfo getVkStruct(const void *_p_next = nullptr, VkPipelineRasterizationStateCreateFlags _flags = 0u) const {
                        VkPipelineRasterizationStateCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
                        __res.pNext = _p_next;
                        __res.flags = _flags;
                        __res.depthClampEnable = depth_clamp_enable;
                        __res.rasterizerDiscardEnable = rasterizer_discard_enable;
                        __res.polygonMode = polygon_mode;
                        __res.cullMode = cull_mode;
                        __res.frontFace = front_face;
                        __res.depthBiasEnable = depth_bias_enable;
                        __res.depthBiasConstantFactor = depth_bias_constant_factor;
                        __res.depthBiasClamp = depth_bias_clamp;
                        __res.depthBiasSlopeFactor = depth_bias_slope_factor;
                        __res.lineWidth = line_width;
                        return __res;
                    }

                // Setters

                    void setDepthClamp(bool _enable) {
                        depth_clamp_enable = VkBool32(_enable);
                    }

                    void setRasterizerDiscard(bool _enable) {
                        rasterizer_discard_enable = VkBool32(_enable);
                    }

                    void setPolygonMode(VkPolygonMode _mode) {
                        polygon_mode = _mode;
                    }

                    void setCullMode(VkCullModeFlags _mode) {
                        cull_mode = _mode;
                    }

                    void setFrontFace(VkFrontFace _front) {
                        front_face = _front;
                    }

                    void setDepthBias(bool _enable, float _const_factor = 0.f, float _clamp = 0.f, float _slope_factor = 0.f) {
                        if (!_enable) {
                            _const_factor = 0.f;
                            _clamp = 0.f;
                            _slope_factor = 0.f;
                        }

                        depth_bias_enable = _enable;
                        depth_bias_constant_factor = _const_factor;
                        depth_bias_clamp = _clamp;
                        depth_bias_slope_factor = _slope_factor;
                    }

                    void setLineWidth(float _width) {
                        line_width = _width;
                    }

                // Scenarios

                    // TODO:
                    void defStruct() {
                        depth_clamp_enable = false;
                        rasterizer_discard_enable = false;
                        polygon_mode = VK_POLYGON_MODE_FILL;
                        cull_mode = VK_CULL_MODE_NONE;
                        front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
                        depth_bias_enable = false;
                        depth_bias_constant_factor = 0.f;
                        depth_bias_clamp = 0.f;
                        depth_bias_slope_factor = 0.f;
                        line_width = 1.f;
                    }
                }; // StructBuilder END
            }; // RasterizationState END


//  ============  Pipeline Multisample State  ============  \\

            namespace MultisampleState {
                struct StructBuilder {
                    VkSampleCountFlagBits rasterization_samples;
                    VkBool32 sample_shading_enable;
                    float min_sample_shading;
                    std::vector<VkSampleMask> sample_mask;
                    VkBool32 alpha_to_coverage_enable;
                    VkBool32 alpha_to_one_enable;

                    StructBuilder() {}
                    ~StructBuilder() {
                        clear();
                    }

                    void clear() {
                        sample_shading_enable = VK_FALSE;
                        rasterization_samples = VkSampleCountFlagBits(0u);
                        min_sample_shading = 0.0f;
                        sample_mask.clear();
                        alpha_to_coverage_enable = VK_FALSE;
                        alpha_to_one_enable = VK_FALSE;
                    }

                    VkPipelineMultisampleStateCreateInfo getVkStructView(const void *_p_next = nullptr, VkPipelineMultisampleStateCreateFlags _flags = 0u) const {
                        VkPipelineMultisampleStateCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
                        __res.pNext = _p_next;
                        __res.flags = _flags;
                        __res.rasterizationSamples = rasterization_samples;
                        __res.sampleShadingEnable = sample_shading_enable;
                        __res.minSampleShading = min_sample_shading;
                        __res.pSampleMask = sample_mask.size() ? sample_mask.data() : nullptr; // no direct count data member
                        __res.alphaToCoverageEnable = alpha_to_coverage_enable;
                        __res.alphaToOneEnable = alpha_to_one_enable;

                        // from documentation
                        // if (rasterization_samples  < VK_SAMPLE_COUNT_64_BIT)
                            // assert(sample_mask.size() < 2);
                        // if (rasterization_samples == VK_SAMPLE_COUNT_64_BIT)
                            // assert(sample_mask.size() == 0u || sample_mask.size() == 2u);

                        return __res;
                    }

                    VkPipelineMultisampleStateCreateInfo getVkStructDCopy(const void *_p_next = nullptr, VkPipelineMultisampleStateCreateFlags _flags = 0u) const {
                        VkPipelineMultisampleStateCreateInfo __res = getVkStructView(_p_next, _flags);

                        if (sample_mask.size()) {
                            uint32_t *__sample_masks = new uint32_t[sample_mask.size()];
                            memcpy(__sample_masks, sample_mask.data(), sample_mask.size() * sizeof(uint32_t));
                            __res.pSampleMask = __sample_masks;
                        }

                        return __res;
                    }

                // Scenarios

                    // TODO:
                    void defStruct() {
                        sample_shading_enable = VK_FALSE;
                        rasterization_samples = VK_SAMPLE_COUNT_1_BIT;
                        min_sample_shading = 1.0f; // Optional
                        sample_mask.clear(); // Optional
                        alpha_to_coverage_enable = VK_FALSE; // Optional
                        alpha_to_one_enable = VK_FALSE; // Optional
                    }
                }; // StructBuilder END
            }; // MultisampleState END


//  ============  Pipeline Depth Stencil State  ============  \\

            namespace DepthStencilState {
                // TODO:
                struct StructBuilder {
                    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

                    StructBuilder() {}
                    ~StructBuilder() {
                        clear();
                    }

                    void clear() {
                        depth_stencil_state_create_info = VkPipelineDepthStencilStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
                    }

                    const VkPipelineDepthStencilStateCreateInfo* ptr() const { return &depth_stencil_state_create_info; }

                    VkPipelineDepthStencilStateCreateInfo getVkStruct(const void *_p_next = nullptr, VkPipelineDepthStencilStateCreateFlags _flags = 0u) const {
                        VkPipelineDepthStencilStateCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
                        __res = depth_stencil_state_create_info;
                        __res.pNext = _p_next;
                        __res.flags = _flags;

                        return __res;
                    }

                // Scenarios

                    // TODO:
                    void defStruct() {
                        depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                        depth_stencil_state_create_info.pNext = nullptr;
                        depth_stencil_state_create_info.flags = 0u;
                        depth_stencil_state_create_info.depthTestEnable = VK_FALSE;
                        depth_stencil_state_create_info.depthWriteEnable = VK_FALSE;
                        depth_stencil_state_create_info.depthCompareOp;
                        depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
                        depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
                        depth_stencil_state_create_info.front;
                        depth_stencil_state_create_info.back;
                        depth_stencil_state_create_info.minDepthBounds;
                        depth_stencil_state_create_info.maxDepthBounds;
                    }
                }; // StructBuilder END
            }; // DepthStencilState END


//  ============  Pipeline Color Blend Attachment State  ============  \\

            namespace ColorBlendAttachmentState {
                struct StructWrap {
                    VkPipelineColorBlendAttachmentState attachment{};

                    StructWrap() {}
                    ~StructWrap() {
                        clear();
                    }

                    void clear() {
                        attachment = VkPipelineColorBlendAttachmentState{};
                    }

                    const VkPipelineColorBlendAttachmentState* ptr() const { return &attachment; }

                // Setters

                    void setBlendEnable(bool _enable) {
                        attachment.blendEnable = VkBool32(_enable);
                    }

                    void setColorWriteMask(VkColorComponentFlags _colors) {
                        attachment.colorWriteMask = _colors;
                    }

                    void setColorBlend(VkBlendOp _op, VkBlendFactor _src_factor, VkBlendFactor _dst_factor) {
                        attachment.colorBlendOp = _op;
                        attachment.srcColorBlendFactor = _src_factor;
                        attachment.dstColorBlendFactor = _dst_factor;
                    }

                    void setAlphaBlend(VkBlendOp _op, VkBlendFactor _src_factor, VkBlendFactor _dst_factor) {
                        attachment.alphaBlendOp = _op;
                        attachment.srcAlphaBlendFactor = _src_factor;
                        attachment.dstAlphaBlendFactor = _dst_factor;
                    }

                // Scenarios

                    void confBlendOver() {
                        // Default: Porter-Duff blend over
                        attachment.blendEnable = VK_TRUE;
                        attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                        attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                        attachment.colorBlendOp = VK_BLEND_OP_ADD;
                        attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                        attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                        attachment.alphaBlendOp = VK_BLEND_OP_ADD;
                        attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                    }
                }; // StructWrap END
            }; // ColorBlendAttachmentState END


//  ============  Pipeline Color Blend State  ============  \\

            namespace ColorBlendState {
                struct StructBuilder {
                    std::vector<VkPipelineColorBlendAttachmentState> attachments;
                    vec4 blend_constants;
                    VkLogicOp logic_op{VkLogicOp(-1)};

                    StructBuilder() {}
                    StructBuilder(const StructBuilder &_builder) = default;
                    ~StructBuilder() {
                        clear();
                    }

                    void clear() {
                        attachments.clear();
                        blend_constants = 0.f;
                        logic_op = VkLogicOp(-1);
                    }

                    VkPipelineColorBlendStateCreateInfo getVkStructView(const void *_p_next = nullptr, VkPipelineColorBlendStateCreateFlags _flags = 0u) const {
                        VkPipelineColorBlendStateCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
                        __res.flags = _flags;
                        __res.pNext = _p_next;
                        __res.logicOpEnable = VkBool32(logic_op != VkLogicOp(-1));
                        __res.logicOp = logic_op;
                        __res.attachmentCount = attachments.size();
                        __res.pAttachments = attachments.data();
                        __res.blendConstants[0] = blend_constants[0];
                        __res.blendConstants[1] = blend_constants[1];
                        __res.blendConstants[2] = blend_constants[2];
                        __res.blendConstants[3] = blend_constants[3];

                        return __res;
                    }

                    VkPipelineColorBlendStateCreateInfo getVkStructDCopy(const void *_p_next = nullptr, VkPipelineColorBlendStateCreateFlags _flags = 0u) const {
                        VkPipelineColorBlendStateCreateInfo __res = getVkStructView(_p_next, _flags);

                        if (attachments.size()) {
                            VkPipelineColorBlendAttachmentState *__attach_ptr = new VkPipelineColorBlendAttachmentState[attachments.size()];
                            memcpy(__attach_ptr, attachments.data(), attachments.size() * sizeof(VkPipelineColorBlendAttachmentState));
                            __res.pAttachments = __attach_ptr;
                        }

                        return __res;
                    }

                // Setters

                    void setLogicOp(VkLogicOp _op) {
                        logic_op = _op;
                    }

                    void setBlendConstants(const vec4 &_blend_constants) {
                        blend_constants = _blend_constants;
                    }

                    void setAttachments(const std::vector<ColorBlendAttachmentState::StructWrap> &_states) {
                        attachments.resize(_states.size());
                        for (uint32_t i = 0u; i < _states.size(); ++i)
                            attachments[i] = _states[i].attachment;
                    }

                // Scenarios

                    void addAttachments(const std::vector<ColorBlendAttachmentState::StructWrap> &_states) {
                        attachments.reserve(attachments.size() + _states.size());
                        for (uint32_t i = 0u; i < _states.size(); ++i)
                            attachments.push_back(_states[i].attachment);
                    }
                }; // StructBuilder END
            }; // ColorBlendState END


//  ============  Pipeline Tessellation State  ============  \\

            namespace TessellationState {
                VkPipelineTessellationStateCreateInfo GetVkStruct(uint32_t _patch_control_points, const void *_p_next = nullptr, VkPipelineTessellationStateCreateFlags _flags = 0u) {
                    VkPipelineTessellationStateCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
                    __res.pNext = _p_next;
                    __res.flags = _flags;
                    __res.patchControlPoints = _patch_control_points;

                    return __res;
                }

                struct StructBuilder {
                    uint32_t patch_control_points = 0u;

                    StructBuilder() {}
                    StructBuilder(const StructBuilder &_sct) = default;
                    StructBuilder(uint32_t _points_num) : patch_control_points{_points_num} {}
                    ~StructBuilder() {
                        clear();
                    }

                    void clear() {
                        patch_control_points = 0u;
                    }

                    VkPipelineTessellationStateCreateInfo getVkStruct(const void *_p_next = nullptr, VkPipelineTessellationStateCreateFlags _flags = 0u) const {
                        VkPipelineTessellationStateCreateInfo __res{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
                        __res.pNext = _p_next;
                        __res.flags = _flags;
                        __res.patchControlPoints = patch_control_points;

                        return __res;
                    }

                // Setters

                    void setPatchControlPoints(uint32_t _points_num) {
                        patch_control_points = _points_num;
                    }
                }; // StructBuilder END
            }; // TessellationState END


//  ============  Vulkan Graphics Pipeline  ============  \\

            namespace Graphics {
                struct Builder {
                    const VkPipelineVertexInputStateCreateInfo   *   vertex_input_state = VK_NULL_HANDLE;
                    const VkPipelineInputAssemblyStateCreateInfo * input_assembly_state = VK_NULL_HANDLE;
                    const VkPipelineTessellationStateCreateInfo  *   tessellation_state = VK_NULL_HANDLE;
                    const VkPipelineViewportStateCreateInfo      *       viewport_state = VK_NULL_HANDLE;
                    const VkPipelineRasterizationStateCreateInfo *  rasterization_state = VK_NULL_HANDLE;
                    const VkPipelineMultisampleStateCreateInfo   *    multisample_state = VK_NULL_HANDLE;
                    const VkPipelineDepthStencilStateCreateInfo  *  depth_btencil_state = VK_NULL_HANDLE;
                    const VkPipelineColorBlendStateCreateInfo    *    color_blend_state = VK_NULL_HANDLE;
                    const VkPipelineDynamicStateCreateInfo       *        dynamic_state = VK_NULL_HANDLE;
                    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
                    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
                    VkPipeline base_pipeline_handle = VK_NULL_HANDLE;
                    int32_t base_pipeline_index = 0;

                    Builder(VkPipelineLayout _layout = VK_NULL_HANDLE) : pipeline_layout{_layout} {}
                    ~Builder() {
                        clear();
                    }

                    void clear() {
                        vertex_input_state   = nullptr;
                        input_assembly_state = nullptr;
                        tessellation_state   = nullptr;
                        viewport_state       = nullptr;
                        rasterization_state  = nullptr;
                        multisample_state    = nullptr;
                        depth_btencil_state  = nullptr;
                        color_blend_state    = nullptr;
                        dynamic_state        = nullptr;
                        pipeline_layout = VK_NULL_HANDLE;
                        base_pipeline_handle = VK_NULL_HANDLE;
                        base_pipeline_index = 0;
                        shader_stages.clear();
                    }

                    VkPipeline createObject(VkDevice _device, VkRenderPass _render_pass, uint32_t _subpass, VkPipelineCache _pipeline_cache = VK_NULL_HANDLE, const void *_p_next = nullptr, VkPipelineCreateFlags _flags = 0u,
                                            const VkAllocationCallbacks * _create_cb = nullptr) const {
                        VkGraphicsPipelineCreateInfo __info = getVkStructView(_render_pass, _subpass, _p_next, _flags);
                        VkPipeline __pipeline = VK_NULL_HANDLE;
                        if (vkCreateGraphicsPipelines(_device, _pipeline_cache, 1, &__info, _create_cb, &__pipeline) != VK_SUCCESS)
                            throw std::runtime_error(SVKFW_WRAPERR("VKFW :: GraphicsPipeline Builder :: createObject", "failed to create graphics pipeline"));

                        return __pipeline;
                    }

                    VkGraphicsPipelineCreateInfo getVkStructView(VkRenderPass _render_pass, uint32_t _subpass, const void *_p_next = nullptr, VkPipelineCreateFlags _flags = 0u) const {
                        VkGraphicsPipelineCreateInfo __res{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
                        __res.pNext = _p_next;
                        __res.flags = _flags;
                        __res.stageCount = shader_stages.size();
                        __res.pStages = shader_stages.data();
                        __res.pVertexInputState = vertex_input_state;
                        __res.pInputAssemblyState = input_assembly_state;
                        __res.pTessellationState = tessellation_state;
                        __res.pViewportState = viewport_state;
                        __res.pRasterizationState = rasterization_state;
                        __res.pMultisampleState = multisample_state;
                        __res.pDepthStencilState = depth_btencil_state;
                        __res.pColorBlendState = color_blend_state;
                        __res.pDynamicState = dynamic_state;
                        __res.layout = pipeline_layout;
                        __res.renderPass = _render_pass;
                        __res.subpass = _subpass;

                        return __res;
                    }

                    VkGraphicsPipelineCreateInfo getVkStructDCopy(VkRenderPass _render_pass, uint32_t _subpass, const void *_p_next = nullptr, VkPipelineCreateFlags _flags = 0u) const {
                        VkGraphicsPipelineCreateInfo __res = getVkStructView(_render_pass, _subpass, _p_next, _flags);

                        if (shader_stages.size()) {
                            VkPipelineShaderStageCreateInfo *__shader_stages = new VkPipelineShaderStageCreateInfo[shader_stages.size()];
                            memcpy(__shader_stages, shader_stages.data(), shader_stages.size() * sizeof(VkPipelineShaderStageCreateInfo));
                            __res.pStages = __shader_stages;
                        }
                        if (vertex_input_state != nullptr)
                            __res.pVertexInputState = new VkPipelineVertexInputStateCreateInfo{*vertex_input_state};
                        if (input_assembly_state != nullptr)
                            __res.pInputAssemblyState = new VkPipelineInputAssemblyStateCreateInfo{*input_assembly_state};
                        if (tessellation_state != nullptr)
                            __res.pTessellationState = new VkPipelineTessellationStateCreateInfo{*tessellation_state};
                        if (viewport_state != nullptr)
                            __res.pViewportState = new VkPipelineViewportStateCreateInfo{*viewport_state};
                        if (rasterization_state != nullptr)
                            __res.pRasterizationState = new VkPipelineRasterizationStateCreateInfo{*rasterization_state};
                        if (multisample_state != nullptr)
                            __res.pMultisampleState = new VkPipelineMultisampleStateCreateInfo{*multisample_state};
                        if (depth_btencil_state != nullptr)
                            __res.pDepthStencilState = new VkPipelineDepthStencilStateCreateInfo{*depth_btencil_state};
                        if (color_blend_state != nullptr)
                            __res.pColorBlendState = new VkPipelineColorBlendStateCreateInfo{*color_blend_state};
                        if (dynamic_state != nullptr)
                            __res.pDynamicState = new VkPipelineDynamicStateCreateInfo{*dynamic_state};

                        return __res;
                    }

                // Setters

                    void setShaderStages(const std::vector<VkPipelineShaderStageCreateInfo> &_shader_stages) {
                        shader_stages = _shader_stages;
                    }

                    void setVertexInputState(const VkPipelineVertexInputStateCreateInfo *_vertex_input_state) {
                        vertex_input_state = _vertex_input_state;
                    }

                    void setInputAssemblyState(const VkPipelineInputAssemblyStateCreateInfo *_input_assembly_state) {
                        input_assembly_state = _input_assembly_state;
                    }

                    void setTessellationState(const VkPipelineTessellationStateCreateInfo *_tessellation_state) {
                        tessellation_state = _tessellation_state;
                    }

                    void setViewportState(const VkPipelineViewportStateCreateInfo *_viewport_state) {
                        viewport_state = _viewport_state;
                    }

                    void setRasterizationState(const VkPipelineRasterizationStateCreateInfo *_rasterization_state) {
                        rasterization_state = _rasterization_state;
                    }

                    void setMultisampleState(const VkPipelineMultisampleStateCreateInfo *_multisample_state) {
                        multisample_state = _multisample_state;
                    }

                    void setDepthStencilState(const VkPipelineDepthStencilStateCreateInfo *_depth_btencil_state) {
                        depth_btencil_state = _depth_btencil_state;
                    }

                    void setColorBlendState(const VkPipelineColorBlendStateCreateInfo *_color_blend_state) {
                        color_blend_state = _color_blend_state;
                    }

                    void setDynamicState(const VkPipelineDynamicStateCreateInfo *_dynamic_state) {
                        dynamic_state = _dynamic_state;
                    }

                    void setBasePipeline(int32_t _base_pipeline_index = 0, VkPipeline _base_pipeline_handle = VK_NULL_HANDLE) {
                        base_pipeline_handle = _base_pipeline_handle;
                        base_pipeline_index = _base_pipeline_index;
                    }

                    void setPipelineLayout(VkPipelineLayout _layout) {
                        pipeline_layout = _layout;
                    }

                // Scenarios

                    void addShaderStages(const std::vector<VkPipelineShaderStageCreateInfo> &_shader_stages) {
                        shader_stages.insert(shader_stages.end(), _shader_stages.begin(), _shader_stages.end());
                    }
                }; // Builder END
            }; // Graphics END


//  ============  Vulkan Compute Pipeline  ============  \\

            namespace Compute {
                struct Builder {
                    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
                    VkPipeline base_pipeline_handle = VK_NULL_HANDLE;
                    int32_t base_pipeline_index = 0;
                    VkPipelineShaderStageCreateInfo compute_shader_stage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};

                    Builder(VkPipelineLayout _layout = VK_NULL_HANDLE) : pipeline_layout{_layout} {}
                    Builder(const Builder &_builder) = default;
                    ~Builder() {
                        clear();
                    }

                    void clear() {
                        pipeline_layout = VK_NULL_HANDLE;
                        base_pipeline_handle = VK_NULL_HANDLE;
                        base_pipeline_index = 0;
                        compute_shader_stage = VkPipelineShaderStageCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
                    }

                    VkPipeline createObject(VkDevice _device, VkPipelineCache _pipeline_cache = VK_NULL_HANDLE, const void *_p_next = nullptr, VkPipelineCreateFlags _flags = 0u,
                                            const VkAllocationCallbacks * _create_cb = nullptr) const {
                        VkComputePipelineCreateInfo __info = getVkStruct(_p_next, _flags);
                        VkPipeline __pipeline = VK_NULL_HANDLE;
                        if (vkCreateComputePipelines(_device, _pipeline_cache, 1, &__info, _create_cb, &__pipeline) != VK_SUCCESS)
                            throw std::runtime_error(SVKFW_WRAPERR("VKFW :: ComputePipeline Builder :: createObject", "failed to create compute pipeline"));

                        return __pipeline;
                    }

                    VkComputePipelineCreateInfo getVkStruct(const void *_p_next = nullptr, VkPipelineCreateFlags _flags = 0u) const {
                        VkComputePipelineCreateInfo __res{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
                        __res.pNext = _p_next;
                        __res.flags = _flags;
                        __res.stage = compute_shader_stage;
                        __res.layout = pipeline_layout;
                        __res.basePipelineHandle = base_pipeline_handle;
                        __res.basePipelineIndex = base_pipeline_index;

                        return __res;
                    }

                // Setters

                    void setShaderStage(const VkPipelineShaderStageCreateInfo &_shader_stage) {
                        compute_shader_stage = _shader_stage;
                    }

                    void setPipelineLayout(VkPipelineLayout _layout) {
                        pipeline_layout = _layout;
                    }

                    void setBasePipeline(int32_t _base_pipeline_index = 0, VkPipeline _base_pipeline_handle = VK_NULL_HANDLE) {
                        base_pipeline_handle = _base_pipeline_handle;
                        base_pipeline_index = _base_pipeline_index;
                    }
                }; // Builder END
            }; // Compute END


//  ============  Vulkan Ray Tracing Pipeline  ============  \\
            // TODO:
            namespace RayTracing {
                struct Builder {

                    VkPipeline createObject() {
                        // vkCreateRayTracingPipelinesKHR();
                        return VK_NULL_HANDLE;
                    }

                    VkRayTracingPipelineCreateInfoKHR getVkStruct() {
                        VkRayTracingPipelineCreateInfoKHR __res{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};

                        return __res;
                    }
                }; // Builder END
            }; // RayTracing END
        }; // Pipeline END


//  ============  Vulkan Framebuffer  ============  \\

        namespace Framebuffer {
            struct Builder {
                std::vector<VkImageView> attachments;
                VkExtent2D buffer_size{0u, 0u};
                uint32_t layers = 0u;

                Builder() {}
                ~Builder() {
                    clear();
                }

                void clear() {
                    attachments.clear();
                    buffer_size = {0u, 0u};
                    layers = 0u;
                }

                VkFramebuffer createObject(VkDevice _device, VkRenderPass _render_pass, const void *_p_next = nullptr, VkFramebufferCreateFlags _flags = 0u,
                                        const VkAllocationCallbacks * _create_cb = nullptr) const {
                    VkFramebufferCreateInfo __info = getVkStructView(_render_pass, _p_next, _flags);
                    VkFramebuffer __framebuffer = VK_NULL_HANDLE;
                    if (vkCreateFramebuffer(_device, &__info, _create_cb, &__framebuffer) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Framebuffer Builder :: createObject", "failed to create framebuffer"));

                    return __framebuffer;
                }

                VkFramebufferCreateInfo getVkStructView(VkRenderPass _render_pass, const void *_p_next = nullptr, VkFramebufferCreateFlags _flags = 0u) const {
                    VkFramebufferCreateInfo __res{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
                    __res.pNext = _p_next;
                    __res.flags = _flags;
                    __res.renderPass = _render_pass;
                    __res.attachmentCount = attachments.size();
                    __res.pAttachments = attachments.data();
                    __res.width  = buffer_size.width;
                    __res.height = buffer_size.height;
                    __res.layers = layers;

                    return __res;
                }

                VkFramebufferCreateInfo getVkStructDCopy(VkRenderPass _render_pass, const void *_p_next = nullptr, VkFramebufferCreateFlags _flags = 0u) const {
                    VkFramebufferCreateInfo __res = getVkStructView(_render_pass, _p_next, _flags);

                    if (attachments.size()) {
                        VkImageView *__attachments = new VkImageView[attachments.size()];
                        memcpy(__attachments, attachments.data(), attachments.size() * sizeof(VkImageView));
                        __res.pAttachments = __attachments;
                    }

                    return __res;
                }

            // Setters

                void setAttachments(const std::vector<VkImageView> &_attachments) {
                    attachments = _attachments;
                }

                void setBufferSize(VkExtent2D _buffer_size) {
                    buffer_size = _buffer_size;
                }

                void setLayers(uint32_t _layers) {
                    layers = _layers;
                }

            // Scenarios

                void setBufferSizeAndLayers(VkExtent2D _buffer_size, uint32_t _layers) {
                    buffer_size = _buffer_size;
                    layers = _layers;
                }
            }; // Builder END
        }; // Framebuffer END


    // ============  Vulkan Command Pool  ============  \\

        namespace CommandPool {
            VkCommandPool CreateObject(VkDevice _device, uint32_t _queue_family, const void *_p_next = nullptr,
                                        VkCommandPoolCreateFlags _flags = 0u, const VkAllocationCallbacks * _create_cb = nullptr) {
                VkCommandPoolCreateInfo __res{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
                __res.pNext = _p_next;
                __res.flags = _flags;
                __res.queueFamilyIndex = _queue_family;

                VkCommandPool __framebuffer = VK_NULL_HANDLE;
                if (vkCreateCommandPool(_device, &__res, _create_cb, &__framebuffer) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: CommandPool :: CreateObject", "failed to create command pool"));

                return __framebuffer;
            }

            VkCommandPoolCreateInfo GetVkStruct(uint32_t _queue_family, const void *_p_next = nullptr, VkCommandPoolCreateFlags _flags = 0u) {
                VkCommandPoolCreateInfo __res{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
                __res.pNext = _p_next;
                __res.flags = _flags;
                __res.queueFamilyIndex = _queue_family;

                return __res;
            }
        }; // CommandPool END


        namespace CommandBufferInheritance {
            struct Builder {
                uint32_t subpass = 0u;
                VkBool32 occlusion_query = VK_FALSE;
                VkQueryControlFlags query = 0u;
                VkQueryPipelineStatisticFlags pipeline_statistics = 0u;

                Builder() {}
                ~Builder() {
                    clear();
                }

                void clear() {
                    subpass = 0u;
                    occlusion_query = VK_FALSE;
                    query = 0u;
                    pipeline_statistics = 0u;
                }

                VkCommandBufferInheritanceInfo getVkStruct(VkRenderPass _render_pass, VkFramebuffer _framebuffer, const void *_p_next = nullptr) const {
                    VkCommandBufferInheritanceInfo __res{VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
                    __res.pNext = _p_next;
                    __res.renderPass = _render_pass;
                    __res.subpass = subpass;
                    __res.framebuffer = _framebuffer;
                    __res.occlusionQueryEnable = occlusion_query;
                    __res.queryFlags = query;
                    __res.pipelineStatistics = pipeline_statistics;

                    return __res;
                }

            // Setters

                void setInfo(uint32_t _subpass = 0u, VkBool32 _occlusion_query = VK_FALSE,
                            VkQueryControlFlags _query = 0u, VkQueryPipelineStatisticFlags _pipeline_statistics = 0u) {
                    subpass = _subpass;
                    occlusion_query = _occlusion_query;
                    query = _query;
                    pipeline_statistics = _pipeline_statistics;
                }
            }; // Builder END
        }; // CommandBufferInheritance


        namespace CommandBuffer {
            VkCommandBufferAllocateInfo GetVkStruct(VkCommandPool _command_pool, VkCommandBufferLevel _level,
                                                    uint32_t _buffer_count, const void *_p_next = nullptr) {
                VkCommandBufferAllocateInfo __res;
                __res.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                __res.pNext = _p_next;
                __res.commandPool = _command_pool;
                __res.level = _level;
                __res.commandBufferCount = _buffer_count;

                return __res;
            }

            std::vector<VkCommandBuffer> CreateObjects(VkDevice _device, VkCommandPool _command_pool, VkCommandBufferLevel _level,
                                          uint32_t _buffer_count, const void *_p_next = nullptr, VkFlags _flags = 0u,
                                          const VkAllocationCallbacks * _create_cb = nullptr) {
                if (_buffer_count == 0u)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: CommandBuffer :: CreateObjects", "'buffer_count' must be positive"));

                VkCommandBufferAllocateInfo __info = GetVkStruct(_command_pool, _level, _buffer_count, _p_next);

                std::vector<VkCommandBuffer> __command_buffer{size_t(_buffer_count)};
                if (vkAllocateCommandBuffers(_device, &__info, __command_buffer.data()) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: CommandBuffer :: CreateObjects", "failed to allocate command buffers"));

                return __command_buffer;
            }
        }; // CommandBuffer END


        namespace SubmitInfo {
            struct StructBuilder {
                std::vector<VkSemaphore> wait_semaphores;
                std::vector<VkPipelineStageFlags> wait_stages;
                std::vector<VkCommandBuffer> command_buffers;
                std::vector<VkSemaphore> signal_semaphores;

                StructBuilder(const std::vector<VkCommandBuffer> &_command_buffers = {},
                            const std::vector<VkSemaphore> &_wait_semaphores = {},
                            const std::vector<VkSemaphore> &_signal_semaphores = {})
                                : command_buffers{_command_buffers}, wait_semaphores{_wait_semaphores}, signal_semaphores{_signal_semaphores} {}
                ~StructBuilder() {
                    clear();
                }

                void clear() {
                    wait_stages.clear();
                    wait_semaphores.clear();
                    command_buffers.clear();
                    signal_semaphores.clear();
                }

                VkSubmitInfo getVkStructView(const void *_p_next = nullptr) const {
                    VkSubmitInfo __res{VK_STRUCTURE_TYPE_SUBMIT_INFO};
                    __res.pNext = _p_next;
                    __res.waitSemaphoreCount = wait_semaphores.size();
                    __res.pWaitSemaphores = wait_semaphores.data();
                    __res.pWaitDstStageMask = wait_stages.data(); // size == .waitSemaphoreCount
                    __res.commandBufferCount = command_buffers.size();
                    __res.pCommandBuffers = command_buffers.data();
                    __res.signalSemaphoreCount = signal_semaphores.size();
                    __res.pSignalSemaphores = signal_semaphores.data();

                    return __res;
                }

                VkSubmitInfo getVkStructDCopy(const void *_p_next = nullptr) const {
                    VkSubmitInfo __res = getVkStructView(_p_next);

                    if (wait_semaphores.size()) {
                        VkSemaphore *__wait_semaphores = new VkSemaphore[wait_semaphores.size()];
                        memcpy(__wait_semaphores, wait_semaphores.data(), wait_semaphores.size() * sizeof(VkSemaphore));
                        __res.pWaitSemaphores = __wait_semaphores;

                        VkPipelineStageFlags *__stage_flags = new VkPipelineStageFlags[wait_stages.size()];
                        memcpy(__stage_flags, wait_stages.data(), wait_stages.size() * sizeof(VkPipelineStageFlags));
                        __res.pWaitDstStageMask = __stage_flags;
                    }

                    if (command_buffers.size()) {
                        VkCommandBuffer *__command_buffers = new VkCommandBuffer[command_buffers.size()];
                        memcpy(__command_buffers, command_buffers.data(), command_buffers.size() * sizeof(VkCommandBuffer));
                        __res.pCommandBuffers = __command_buffers;
                    }

                    if (signal_semaphores.size()) {
                        VkSemaphore *__signal_semaphores = new VkSemaphore[signal_semaphores.size()];
                        memcpy(__signal_semaphores, signal_semaphores.data(), signal_semaphores.size() * sizeof(VkSemaphore));
                        __res.pSignalSemaphores = __signal_semaphores;
                    }

                    return __res;
                }

            // Setters

                void setCommandBuffers(const std::vector<VkCommandBuffer> &_command_buffers) {
                    command_buffers = _command_buffers;
                }

                void setWaitSemaphores(const std::vector<VkSemaphore> &_wait_semaphores) {
                    wait_semaphores = _wait_semaphores;
                }

                void setWaitStages(const std::vector<VkPipelineStageFlags> &_wait_stages) {
                    wait_stages = _wait_stages;
                }

                void setSignalSemaphores(const std::vector<VkSemaphore> &_signal_semaphores) {
                    signal_semaphores = _signal_semaphores;
                }
            }; // StructBuilder END
        }; // SubmitInfo END


        namespace PresentInfoKHR {
            struct Builder {
                std::vector<VkSemaphore> wait_semaphores;
                std::vector<VkSwapchainKHR> swapchains;
                std::vector<uint32_t> image_indices;
                VkResult *results = nullptr;
                bool use_results = false;

                Builder(const std::vector<VkSemaphore> &_wait_semaphores = {},
                        const std::vector<VkSwapchainKHR> &_swapchains = {})
                                : wait_semaphores{_wait_semaphores}, swapchains{_swapchains} {}
                ~Builder() {
                    clear();
                }

                void clear() {
                    wait_semaphores.clear();
                    swapchains.clear();
                    image_indices.clear();
                    use_results = false;
                    safeDeleteArr(results);
                }

                VkPresentInfoKHR getVkStructView(const void *_p_next = nullptr) const {
                    VkPresentInfoKHR __res{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
                    __res.pNext = _p_next;
                    __res.waitSemaphoreCount = wait_semaphores.size();
                    __res.pWaitSemaphores = wait_semaphores.data();
                    __res.swapchainCount = swapchains.size();
                    __res.pSwapchains = swapchains.data();
                    __res.pImageIndices = image_indices.data();
                    __res.pResults = results;

                    return __res;
                }

                VkPresentInfoKHR getVkStructDCopy(const void *_p_next = nullptr) const {
                    VkPresentInfoKHR __res = getVkStructView(_p_next);

                    if (wait_semaphores.size()) {
                        VkSemaphore *__semaphores = new VkSemaphore[wait_semaphores.size()];
                        memcpy(__semaphores, wait_semaphores.data(), wait_semaphores.size() * sizeof(VkSemaphore));
                        __res.pWaitSemaphores = __semaphores;
                    }

                    if (swapchains.size()) {
                        VkSwapchainKHR *__swapchains = new VkSwapchainKHR[swapchains.size()];
                        memcpy(__swapchains, swapchains.data(), swapchains.size() * sizeof(VkSwapchainKHR));
                        __res.pSwapchains = __swapchains;
                    }

                    if (image_indices.size()) {
                        uint32_t *__image_indices = new uint32_t[image_indices.size()];
                        memcpy(__image_indices, image_indices.data(), image_indices.size() * sizeof(uint32_t));
                        __res.pImageIndices = __image_indices;
                    }

                    if (use_results && swapchains.size()) {
                        __res.pResults = new VkResult[swapchains.size()]{};
                    }

                    return __res;
                }

            // Setters

                void setWaitSemaphores(const std::vector<VkSemaphore> &_wait_semaphores) {
                    wait_semaphores = _wait_semaphores;
                }

                void setSwapchains(const std::vector<VkSwapchainKHR> &_swapchains) {
                    swapchains = _swapchains;
                }

                void setImageIndices(const std::vector<uint32_t> &_image_indices) {
                    image_indices = _image_indices;
                }

                void setUseResults(bool _use_results) {
                    use_results = _use_results;
                    updateResultsArray();
                }

            // Scenarios

                void updateResultsArray() {
                    if (results != nullptr)
                        delete[] results;
                    results = use_results && swapchains.size() ? new VkResult[swapchains.size()]{} : nullptr;
                }

                void setSwapchainsAndResults(const std::vector<VkSwapchainKHR> &_swapchains, bool _use_results = false) {
                    swapchains = _swapchains;
                    use_results = _use_results;

                    updateResultsArray();
                }

                void setInfo(const std::vector<VkSemaphore> &_wait_semaphores,
                             const std::vector<VkSwapchainKHR> &_swapchains,
                             const std::vector<uint32_t> &_image_indices, bool _use_results = false) {
                    wait_semaphores = _wait_semaphores;
                    swapchains = _swapchains;
                    image_indices = _image_indices;
                    use_results = _use_results;
                    updateResultsArray();
                }
            }; // Builder END
        }; // PresentInfoKHR END


//  ============  Vulkan syncronization  ============  \\

        namespace Semaphore {
            VkSemaphore CreateObject(VkDevice _device, const void *_p_next = nullptr, VkSemaphoreCreateFlags _flags = 0u,
                                    const VkAllocationCallbacks *_create_cb = nullptr) {
                VkSemaphoreCreateInfo __info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
                __info.pNext = _p_next;
                __info.flags = _flags;

                VkSemaphore __semaphore = VK_NULL_HANDLE;
                if (vkCreateSemaphore(_device, &__info, _create_cb, &__semaphore) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Semaphore :: CreateObject", "failed to create semaphore"));

                return __semaphore;
            }

            VkSemaphoreCreateInfo GetVkStruct(const void *_p_next = nullptr, VkSemaphoreCreateFlags _flags = 0u) {
                VkSemaphoreCreateInfo __res{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
                __res.pNext = _p_next;
                __res.flags = _flags;

                return __res;
            }
        }; // Semaphore END


        namespace Fence {
            VkFence CreateObject(VkDevice _device, const void *_p_next = nullptr, VkFenceCreateFlags _flags = 0u,
                                const VkAllocationCallbacks *_create_cb = nullptr) {
                VkFenceCreateInfo __info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
                __info.pNext = _p_next;
                __info.flags = _flags;

                VkFence __fence = VK_NULL_HANDLE;
                if (vkCreateFence(_device, &__info, _create_cb, &__fence) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: Fence :: CreateObject", "failed to create fence"));

                return __fence;
            }

            VkFenceCreateInfo GetVkStruct(const void *_p_next = nullptr, VkFenceCreateFlags _flags = 0u) {
                VkFenceCreateInfo __res{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
                __res.pNext = _p_next;
                __res.flags = _flags;

                return __res;
            }
        }; // Fence END


//  ============  Vulkan Function Wrappers (definition)  ============  \\
        
        namespace Func {
            namespace UpdateDescriptorSets {
                struct Builder {
                    std::vector<WriteDescriptorSet::Builder> write_sets;
                    std::vector<VkCopyDescriptorSet> copy_sets;

                // Call

                    void call(VkDevice _device, const std::vector<uint32_t> &_write_sets_i,
                            const std::vector<uint32_t> &_copy_sets_i, void *_p_next = nullptr) {
                        std::vector<VkWriteDescriptorSet> __writes(_write_sets_i.size());
                        std::vector<VkCopyDescriptorSet > __copies( _copy_sets_i.size());
                        for (uint32_t i = 0u; i < _write_sets_i.size(); ++i)
                            __writes[i] = write_sets[_write_sets_i[i]].getVkStructView(_p_next);
                        for (uint32_t i = 0u; i < _copy_sets_i.size(); ++i)
                            __copies[i] =  copy_sets[ _copy_sets_i[i]];

                        vkUpdateDescriptorSets(_device, (uint32_t)__writes.size(), __writes.data(), (uint32_t)__copies.size(), __copies.data());
                    }

                    void call(VkDevice _device, void *_p_next = nullptr) {
                        std::vector<VkWriteDescriptorSet> __writes(write_sets.size());
                        for (uint32_t i = 0u; i < write_sets.size(); ++i)
                            __writes[i] = write_sets[i].getVkStructView(_p_next);
                        vkUpdateDescriptorSets(_device, (uint32_t)__writes.size(), __writes.data(), (uint32_t)copy_sets.size(), copy_sets.data());
                    }

                // Scenarios

                    uint32_t addSetWrite() {
                        write_sets.emplace_back();
                        return write_sets.size() - 1;
                    }

                    WriteDescriptorSet::Builder& getSetWrite(uint32_t _ci_sct_write_set) {
                        return write_sets[_ci_sct_write_set];
                    }

                    uint32_t addSetCopy(VkDescriptorSet _src_set, VkDescriptorSet _dst_set, uint32_t _descr_count, uint32_t _src_binding, uint32_t _dst_binding, uint32_t _src_arr_elem = 0, uint32_t _dst_arr_elem = 0, void *_p_next = nullptr) {
                        VkCopyDescriptorSet __set_copy{VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET};
                        __set_copy.pNext = _p_next;
                        __set_copy.srcSet = _src_set;
                        __set_copy.srcBinding = _src_binding;
                        __set_copy.srcArrayElement = _src_arr_elem;
                        __set_copy.dstSet = _dst_set;
                        __set_copy.dstBinding = _dst_binding;
                        __set_copy.dstArrayElement = _dst_arr_elem;
                        __set_copy.descriptorCount = _descr_count;
                        copy_sets.push_back(__set_copy);
                        return copy_sets.size() - 1;
                    }
                }; // Builder END
            }; // UpdateDescriptorSets END
        };

    // "vkCmd*" call wrappers
        namespace Cmd {
            namespace BeginRenderPass {
                struct Builder {
                    std::vector<VkClearValue> clear_colors;
                    VkRect2D render_area{};

                    Builder() {}
                    ~Builder() {
                        clear();
                    }

                    void clear() {
                        clear_colors.clear();
                    }

                    void cmdCall(VkCommandBuffer _command_buffer, VkRenderPass _render_pass,
                                 VkFramebuffer _framebuffer, VkSubpassContents _contents, const void *_p_next = nullptr) const {
                        VkRenderPassBeginInfo __info = getVkStructView(_render_pass, _framebuffer, _p_next);

                        vkCmdBeginRenderPass(_command_buffer, &__info, _contents);
                    }

                    VkRenderPassBeginInfo getVkStructView(VkRenderPass _render_pass, VkFramebuffer _framebuffer, const void *_p_next = nullptr) const {
                        VkRenderPassBeginInfo __res{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
                        __res.pNext = _p_next;
                        __res.renderPass = _render_pass;
                        __res.framebuffer = _framebuffer;
                        __res.renderArea = render_area;
                        __res.clearValueCount = clear_colors.size();
                        __res.pClearValues = clear_colors.data();

                        return __res;
                    }

                    // TODO: getVkStructDCopy()

                // Setters

                    void setClearColors(const std::vector<VkClearValue> &_clear_colors) {
                        clear_colors = _clear_colors;
                    }

                    void setRenderArea(const Rect2D::StructWrap &_area) {
                        render_area = _area.rectangle;
                    }

                // Scenarios

                    // Example of adding color
                    void addBlackColor() {
                        clear_colors.push_back(ClearValue::build(0.f, 0.f, 0.f, 1.f));
                    }
                }; // Builder END
            }; // BeginRenderPass END


            namespace SetViewport {
                // _count == 0 means [offset, _viewports.size() - 1]
                void CmdCall(VkCommandBuffer _command_buffer, const std::vector<VkViewport> &_viewports,
                             uint32_t _offset = 0u, uint32_t _count = 0u) {
                    if (_count == 0u)
                        _count = _viewports.size() - _offset;
                    vkCmdSetViewport(_command_buffer, _offset, _count, _viewports.data());
                }
            }; // SetViewport END


            namespace SetScissor {
                // _count == 0 means [offset, _viewports.size() - 1]
                void CmdCall(VkCommandBuffer _command_buffer, const std::vector<VkRect2D> &_scissors,
                             uint32_t _offset = 0u, uint32_t _count = 0u) {
                    if (_count == 0u)
                        _count = _scissors.size() - _offset;
                    vkCmdSetScissor(_command_buffer, _offset, _count, _scissors.data());
                }
            }; // SetScissors END


            namespace Draw {
                void CmdCall(VkCommandBuffer _command_buffer, uint32_t _vert_count, uint32_t _inst_count,
                             uint32_t _vert_offset = 0u, uint32_t _inst_offset = 0u) {
                    vkCmdDraw(_command_buffer, _vert_count, _inst_count, _vert_offset, _inst_offset);
                }
            }; // Draw END
        }; // Cmd END
    }; // VKFW END
}; // Simple END

#endif