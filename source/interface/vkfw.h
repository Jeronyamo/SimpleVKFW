#ifndef SVKFW_VKFW_H
#define SVKFW_VKFW_H

#include "vkfw_wrap.h"


namespace Simple {
    namespace VKFW {
// Common

        typedef enum PipelineStateBits {
            VKFW_PIPELINE_VERTEX_INPUT_STATE_BIT   = 0x00000001,
            VKFW_PIPELINE_INPUT_ASSEMBLY_STATE_BIT = 0x00000002,
            VKFW_PIPELINE_TESSELLATION_STATE_BIT   = 0x00000004,
            VKFW_PIPELINE_VIEWPORT_STATE_BIT       = 0x00000008,
            VKFW_PIPELINE_RASTERIZATION_STATE_BIT  = 0x00000010,
            VKFW_PIPELINE_MULTISAMPLE_STATE_BIT    = 0x00000020,
            VKFW_PIPELINE_DEPTH_STENCIL_STATE_BIT  = 0x00000040,
            VKFW_PIPELINE_COLOR_BLEND_STATE_BIT    = 0x00000080,
            VKFW_PIPELINE_DYNAMIC_STATE_BIT        = 0x00000100,
            VKFW_PIPELINE_STATE_BITS_MAX_ENUM      = 0x7FFFFFFF
        } PipelineStateBits;
        typedef uint32_t PipelineStateFlags;

// Result handler

        class VulkanContextFactory;

        struct ResultHandler {
            VkResult result = VK_RESULT_MAX_ENUM;
            ResultHandler(VkResult _result = VK_RESULT_MAX_ENUM) : result{_result} {}

            void handle(VkResult _result, VulkanContextFactory *_vk_factory = nullptr);
        }; // ResultHandler END


// Swapchain

    // Context

        struct SwapchainContext {
            VkSwapchainKHR swapchain = VK_NULL_HANDLE;
            std::vector<VkImageView> image_views;
            std::vector <VkFramebuffer> framebuffers;

            GLFWwindow *window = nullptr;
        }; // SwapchainContext END

        void destroySwapchainContext(VkDevice _device, const SwapchainContext &_context, const VkAllocationCallbacks *_destroy_callback = nullptr) {
            for (uint32_t i = 0u; i < _context.framebuffers.size(); ++i)
                vkDestroyFramebuffer(_device, _context.framebuffers[i], _destroy_callback);
            for (uint32_t i = 0u; i < _context.image_views.size(); ++i)
                vkDestroyImageView(_device, _context.image_views[i], _destroy_callback);
            if (_context.swapchain != VK_NULL_HANDLE)
                vkDestroySwapchainKHR(_device, _context.swapchain, _destroy_callback);
        }


    // Factory

        class SwapchainContextFactory {
            SwapchainContext *swapchain_context = nullptr;

            SwapchainKHR::Builder *swapchain = nullptr;
            ImageView::Builder *image_view = nullptr;
            Framebuffer::Builder *framebuffer = nullptr;

        public:

            SwapchainContextFactory(SwapchainContext *_swapchain_context = nullptr) : swapchain_context{_swapchain_context} {}
            ~SwapchainContextFactory() {
                deleteBuilderFramebuffer();
                deleteBuilderImageView();
                deleteBuilderSwapchain();
                swapchain_context = nullptr;
            }

        // Set context

            void setContext(SwapchainContext *_swapchain_context) {
                swapchain_context = _swapchain_context;
            }

        // Destroy context objects

            void destroyObjects(VkDevice _device, const VkAllocationCallbacks *_destroy_callback = nullptr) {
                if (swapchain_context != nullptr) {
                    destroySwapchainContext(_device, *swapchain_context, _destroy_callback);
                    *swapchain_context = SwapchainContext{};
                }
            }

        // Allocate builder

            void newBuilderSwapchain() {
                deleteBuilderSwapchain();
                swapchain = new SwapchainKHR::Builder{};
            }
            void newBuilderImageView() {
                deleteBuilderImageView();
                image_view = new ImageView::Builder{};
            }
            void newBuilderFramebuffer() {
                deleteBuilderFramebuffer();
                framebuffer = new Framebuffer::Builder{};
            }

        // Deallocate builder

            void deleteBuilderSwapchain() {
                safeDelete(swapchain);
            }
            void deleteBuilderImageView() {
                safeDelete(image_view);
            }
            void deleteBuilderFramebuffer() {
                safeDelete(framebuffer);
            }

        // Get builders

            SwapchainKHR::Builder* getBuilderSwapchainKHR() { return swapchain; }
            ImageView::Builder* getBuilderImageView() { return image_view; }
            Framebuffer::Builder* getBuilderFramebuffer() { return framebuffer; }

        // Create objects

            void createSwapchain(VkDevice _device, VkSurfaceKHR _surface, void *_p_next = nullptr,
                                 VkSwapchainCreateFlagsKHR _flags = 0u, const VkAllocationCallbacks *_create_cb = nullptr) {
                swapchain_context->swapchain = swapchain->createObject(_device, _surface, _p_next, _flags, _create_cb);
            }

            void createImageViews(VkDevice _device) {
                if (image_view == nullptr)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: SwapchainContext :: createImageViews", "ImageView builder was not created and configured"));

                std::vector<VkImage> __swapchain_images{Func::getSwapchainImagesKHR(_device, swapchain_context->swapchain)};

                swapchain_context->image_views.resize(__swapchain_images.size());
                for (uint32_t i = 0u; i < __swapchain_images.size(); ++i)
                    swapchain_context->image_views[i] = image_view->createObject(_device, __swapchain_images[i]);
            }

            void createFramebuffers(VkDevice _device, VkRenderPass _render_pass) {
                if (framebuffer == nullptr)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: SwapchainContext :: createFramebuffers", "Framebuffer builder was not created and configured"));

                swapchain_context->framebuffers.resize(swapchain_context->image_views.size());

                for (uint32_t i = 0u; i < swapchain_context->image_views.size(); ++i) {
                    framebuffer->setAttachments({swapchain_context->image_views[i]});
                    swapchain_context->framebuffers[i] = framebuffer->createObject(_device, _render_pass);
                }
            }

        // Scenarios

            void setWindowCurrentExtent(const Window &_window = nullptr) {
                if (_window.window != nullptr)
                    swapchain_context->window = _window.window;
                swapchain->chooseCurrentExtent(swapchain_context->window);

                setFramebufferSwapchainBufferSize();
            }

            void setImageViewSwapchainFormat() {
                image_view->setFormat(swapchain->image_info.format);
            }

            void setFramebufferSwapchainBufferSize() {
                framebuffer->setBufferSize(swapchain->image_info.extent);
            }

            // All neccessary Builder fields are assumed to be updated before calling this function
            void recreate(VkDevice _device, VkRenderPass _render_pass, VkSurfaceKHR _surface, const VkAllocationCallbacks *_destroy_callback = nullptr) {
                destroyObjects(_device, _destroy_callback);

                setWindowCurrentExtent();
                swapchain_context->swapchain = swapchain->createObject(_device, _surface);

                createImageViews(_device);
                createFramebuffers(_device, _render_pass);
            }
        }; // SwapchainContextFactory END


// Descriptor

    // Context
        struct DescriptorContext {
            std::vector<VkDescriptorSetLayout> layouts;
            std::vector<VkDescriptorPool> pools;
            std::vector<VkDescriptorSet> sets;
            std::vector<uint32_t> ci_set_to_pool; // returns ci_pool for ci_set
        }; // DescriptorContext END

        void destroyDescriptorContext(VkDevice _device, const DescriptorContext &_context, const VkAllocationCallbacks *_destroy_callback = nullptr) {
            for (uint32_t i = 0u; i < _context.pools.size(); ++i)
                vkDestroyDescriptorPool(_device, _context.pools[i], _destroy_callback);

            for (uint32_t i = 0u; i < _context.layouts.size(); ++i)
                vkDestroyDescriptorSetLayout(_device, _context.layouts[i], _destroy_callback);
        }


    // Factory

        class DescriptorContextFactory {
            DescriptorContext *descriptor_context = nullptr;

            DescriptorSetLayout::Builder *b_layout = nullptr;

        public:

            DescriptorContextFactory(DescriptorContext *_descriptor_context = nullptr) : descriptor_context{_descriptor_context} {}
            ~DescriptorContextFactory() {
                deleteBuilderLayout();
                descriptor_context = nullptr;
            }

        // Set context

            void setContext(DescriptorContext *_descriptor_context) {
                descriptor_context = _descriptor_context;
            }

        // Destroy context objects

            void destroyObjects(VkDevice _device, const VkAllocationCallbacks *_destroy_callback = nullptr) {
                if (descriptor_context != nullptr) {
                    destroyDescriptorContext(_device, *descriptor_context, _destroy_callback);
                    *descriptor_context = DescriptorContext{};
                }
            }

        // Allocate builder

            void newBuilderLayout() {
                deleteBuilderLayout();
                b_layout = new DescriptorSetLayout::Builder{};
            }

        // Deallocate builder

            void deleteBuilderLayout() {
                safeDelete(b_layout);
            }

        // Get builders

            DescriptorSetLayout::Builder* getBuilderLayout() { return b_layout; }

        // Create objects

            uint32_t createLayout(VkDevice _device, const void *_p_next = nullptr,
                                  VkDescriptorSetLayoutCreateFlags _flags = 0u,
                                  const VkAllocationCallbacks *_create_cb = nullptr) {
                descriptor_context->layouts.push_back(b_layout->createObject(_device, _p_next, _flags, _create_cb));
                return descriptor_context->layouts.size() - 1;
            }

            uint32_t createDescriptorPool(VkDevice _device, uint32_t _max_sets, const std::vector<VkDescriptorPoolSize> &_pool_sizes = {},
                                          const void *_p_next = nullptr, VkDescriptorPoolCreateFlags _flags = 0u, const VkAllocationCallbacks *_create_cb = nullptr) {
                descriptor_context->pools.push_back(DescriptorPool::CreateObject(_device, _max_sets, _pool_sizes, _p_next, _flags, _create_cb));
                return descriptor_context->pools.size() - 1;
            }
            // If successful, returns ci_set and guarantees that [ ci_set, ci_set + ci_layouts.size() ) are correct descriptor set context indices.
            uint32_t createDescriptorSets(VkDevice _device, uint32_t _ci_pool, const std::vector<uint32_t> &_ci_layouts, void *_p_next = nullptr) {
                VkDescriptorSetAllocateInfo __info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
                __info.pNext = _p_next;
                __info.descriptorPool = descriptor_context->pools[_ci_pool];

                std::vector<VkDescriptorSetLayout> __layouts(_ci_layouts.size());
                for (uint32_t i = 0u; i < _ci_layouts.size(); ++i)
                    __layouts[i] = descriptor_context->layouts[_ci_layouts[i]];
                __info.descriptorSetCount = (uint32_t) __layouts.size();
                __info.pSetLayouts = __layouts.data();

                uint32_t __res_ci_set = descriptor_context->sets.size();
                descriptor_context->sets.resize(__res_ci_set + __layouts.size(), VK_NULL_HANDLE);

                if (vkAllocateDescriptorSets(_device, &__info, descriptor_context->sets.data() + __res_ci_set) != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("DescriptorContextFactory :: createDescriptorSets", "could not allocate descriptor sets"));
                descriptor_context->ci_set_to_pool.resize(descriptor_context->ci_set_to_pool.size() + __layouts.size(), _ci_pool);
                return __res_ci_set;
            }
        }; // DescriptorContextFactory END


// Pipeline

    // Context

        struct PipelineContext {
            VkPipeline pipeline = VK_NULL_HANDLE;
            VkPipelineLayout layout = VK_NULL_HANDLE;
            VkPipelineCache cache = VK_NULL_HANDLE;
            VkPipelineBindPoint pipeline_type = VkPipelineBindPoint(-1);
        }; // PipelineContext END

        void destroyPipelineContext(VkDevice _device, const PipelineContext &_context, const VkAllocationCallbacks *_destroy_callback = nullptr) {
            if (_context.pipeline != VK_NULL_HANDLE)
                vkDestroyPipeline(_device, _context.pipeline, _destroy_callback);
            if (_context.cache != VK_NULL_HANDLE)
                vkDestroyPipelineCache(_device, _context.cache, _destroy_callback);
            if (_context.layout != VK_NULL_HANDLE)
                vkDestroyPipelineLayout(_device, _context.layout, _destroy_callback);
        }


    // Factory (Graphics)

        class PipelineGraphicsContextFactory {
            PipelineContext *pipeline_context = nullptr;

            Pipeline::Graphics::Builder *pipeline = nullptr;
            Pipeline::Cache::Builder *cache = nullptr;

            Pipeline::VertexInputState::StructBuilder   *   vertex_input_state = nullptr;
            Pipeline::InputAssemblyState::StructBuilder * input_assembly_state = nullptr;
            Pipeline::TessellationState::StructBuilder  *   tessellation_state = nullptr;
            Pipeline::ViewportState::StructBuilder      *       viewport_state = nullptr;
            Pipeline::RasterizationState::StructBuilder *  rasterization_state = nullptr;
            Pipeline::MultisampleState::StructBuilder   *    multisample_state = nullptr;
            Pipeline::DepthStencilState::StructBuilder  *  depth_stencil_state = nullptr;
            Pipeline::ColorBlendState::StructBuilder    *    color_blend_state = nullptr;
            Pipeline::DynamicState::StructBuilder       *        dynamic_state = nullptr;
            Pipeline::ShaderStage::StructBuilder        *         shader_stage = nullptr;

            std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

        public:
            PipelineGraphicsContextFactory(PipelineContext *_pipeline_context = nullptr) : pipeline_context{_pipeline_context} {}
            ~PipelineGraphicsContextFactory() {
                deleteBuilderPipeline();
                // deleteBuilderLayout();
                deleteBuilderCache();
                deleteBuilderAllStates();
                deleteBuilderPipelineShaderStage();
                pipeline_context = nullptr;
            }

        // Set context

            void setContext(PipelineContext *_pipeline_context) {
                pipeline_context = _pipeline_context;
            }

        // Destroy context objects

            void destroyObjects(VkDevice _device, const VkAllocationCallbacks *_destroy_callback = nullptr) {
                if (pipeline_context != nullptr) {
                    destroyPipelineContext(_device, *pipeline_context, _destroy_callback);
                    *pipeline_context = PipelineContext{};
                }
            }

        // Allocate builder

            void newBuilderPipeline() {
                deleteBuilderPipeline();
                pipeline = new Pipeline::Graphics::Builder{};
            }
            // void newBuilderLayout() {
            //     deleteBuilderLayout();
            //     layout = new Pipeline::Layout::Builder{};
            // }
            void newBuilderCache() {
                deleteBuilderCache();
                cache = new Pipeline::Cache::Builder{};
            }
            void newBuilderPipelineVertexInputState() {
                deleteBuilderPipelineVertexInputState();
                vertex_input_state = new Pipeline::VertexInputState::StructBuilder{};
            }
            void newBuilderPipelineInputAssemblyState() {
                deleteBuilderPipelineInputAssemblyState();
                input_assembly_state = new Pipeline::InputAssemblyState::StructBuilder{};
            }
            void newBuilderPipelineTessellationState() {
                deleteBuilderPipelineTessellationState();
                tessellation_state = new Pipeline::TessellationState::StructBuilder{};
            }
            void newBuilderPipelineViewportState() {
                deleteBuilderPipelineViewportState();
                viewport_state = new Pipeline::ViewportState::StructBuilder{};
            }
            void newBuilderPipelineRasterizationState() {
                deleteBuilderPipelineRasterizationState();
                rasterization_state = new Pipeline::RasterizationState::StructBuilder{};
            }
            void newBuilderPipelineMultisampleState() {
                deleteBuilderPipelineMultisampleState();
                multisample_state = new Pipeline::MultisampleState::StructBuilder{};
            }
            void newBuilderPipelineDepthStencilState() {
                deleteBuilderPipelineDepthStencilState();
                depth_stencil_state = new Pipeline::DepthStencilState::StructBuilder{};
            }
            void newBuilderPipelineColorBlendState() {
                deleteBuilderPipelineColorBlendState();
                color_blend_state = new Pipeline::ColorBlendState::StructBuilder{};
            }
            void newBuilderPipelineDynamicState() {
                deleteBuilderPipelineDynamicState();
                dynamic_state = new Pipeline::DynamicState::StructBuilder{};
            }
            void newBuilderPipelineShaderStage() {
                deleteBuilderPipelineShaderStage();
                shader_stage = new Pipeline::ShaderStage::StructBuilder{};
            }

        // Deallocate builder
            // Also deletes shader stages' structures, which are assumed to be deep copied
            void deleteBuilderPipeline() {
                deleteAndClearShaderStages();
                safeDelete(pipeline);
            }
            // void deleteBuilderLayout() {
            //     safeDelete(layout);
            // }
            void deleteBuilderCache() {
                safeDelete(cache);
            }
            void deleteBuilderPipelineVertexInputState() {
                safeDelete(vertex_input_state);
            }
            void deleteBuilderPipelineInputAssemblyState() {
                safeDelete(input_assembly_state);
            }
            void deleteBuilderPipelineTessellationState() {
                safeDelete(tessellation_state);
            }
            void deleteBuilderPipelineViewportState() {
                safeDelete(viewport_state);
            }
            void deleteBuilderPipelineRasterizationState() {
                safeDelete(rasterization_state);
            }
            void deleteBuilderPipelineMultisampleState() {
                safeDelete(multisample_state);
            }
            void deleteBuilderPipelineDepthStencilState() {
                safeDelete(depth_stencil_state);
            }
            void deleteBuilderPipelineColorBlendState() {
                safeDelete(color_blend_state);
            }
            void deleteBuilderPipelineDynamicState() {
                safeDelete(dynamic_state);
            }
            void deleteBuilderPipelineShaderStage() {
                safeDelete(shader_stage);
            }

        // Get builders

            Pipeline::Graphics::Builder *getBuilderPipeline() { return pipeline; }
            // Pipeline::Layout::Builder *getBuilderLayout() { return layout; }
            Pipeline::Cache::Builder *getBuilderCache() { return cache; }
            Pipeline::VertexInputState::StructBuilder *getBuilderPipelineVertexInputState() {
                return vertex_input_state;
            }
            Pipeline::InputAssemblyState::StructBuilder *getBuilderPipelineInputAssemblyState() {
                return input_assembly_state;
            }
            Pipeline::TessellationState::StructBuilder *getBuilderPipelineTessellationState() {
                return tessellation_state;
            }
            Pipeline::ViewportState::StructBuilder *getBuilderPipelineViewportState() {
                return viewport_state;
            }
            Pipeline::RasterizationState::StructBuilder *getBuilderPipelineRasterizationState() {
                return rasterization_state;
            }
            Pipeline::MultisampleState::StructBuilder *getBuilderPipelineMultisampleState() {
                return multisample_state;
            }
            Pipeline::DepthStencilState::StructBuilder *getBuilderPipelineDepthStencilState() {
                return depth_stencil_state;
            }
            Pipeline::ColorBlendState::StructBuilder *getBuilderPipelineColorBlendState() {
                return color_blend_state;
            }
            Pipeline::DynamicState::StructBuilder *getBuilderPipelineDynamicState() {
                return dynamic_state;
            }
            Pipeline::ShaderStage::StructBuilder *getBuilderPipelineShaderStage() {
                return shader_stage;
            }

        // Create objects

            void createPipeline(VkDevice _device, VkRenderPass _render_pass, uint32_t _subpass, bool _use_pipeline_cache,
                                PipelineStateFlags _state_flags = 0x000001ff, void *_p_next = nullptr,
                                VkPipelineCreateFlags _flags = 0u, const VkAllocationCallbacks *_create_cb = nullptr) {
                VkPipelineVertexInputStateCreateInfo __vertex_input_state;
                VkPipelineInputAssemblyStateCreateInfo __input_assembly_state;
                VkPipelineTessellationStateCreateInfo __tessellation_state;
                VkPipelineViewportStateCreateInfo __viewport_state;
                VkPipelineRasterizationStateCreateInfo __rasterization_state;
                VkPipelineMultisampleStateCreateInfo __multisample_state;
                VkPipelineDepthStencilStateCreateInfo __depth_stencil_state;
                VkPipelineColorBlendStateCreateInfo __color_blend_state;
                VkPipelineDynamicStateCreateInfo __dynamic_state;

                if ((_state_flags | VKFW_PIPELINE_VERTEX_INPUT_STATE_BIT) && vertex_input_state != nullptr) {
                    __vertex_input_state = vertex_input_state->getVkStructView();
                    pipeline->setVertexInputState(&__vertex_input_state);
                }
                if ((_state_flags | VKFW_PIPELINE_INPUT_ASSEMBLY_STATE_BIT) && input_assembly_state != nullptr) {
                    __input_assembly_state = input_assembly_state->getVkStruct();
                    pipeline->setInputAssemblyState(&__input_assembly_state);
                }
                if ((_state_flags | VKFW_PIPELINE_TESSELLATION_STATE_BIT) && tessellation_state != nullptr) {
                    __tessellation_state = tessellation_state->getVkStruct();
                    pipeline->setTessellationState(&__tessellation_state);
                }
                if ((_state_flags | VKFW_PIPELINE_VIEWPORT_STATE_BIT) && viewport_state != nullptr) {
                    __viewport_state = viewport_state->getVkStructView();
                    pipeline->setViewportState(&__viewport_state);
                }
                if ((_state_flags | VKFW_PIPELINE_RASTERIZATION_STATE_BIT) && rasterization_state != nullptr) {
                    __rasterization_state = rasterization_state->getVkStruct();
                    pipeline->setRasterizationState(&__rasterization_state);
                }
                if ((_state_flags | VKFW_PIPELINE_MULTISAMPLE_STATE_BIT) && multisample_state != nullptr) {
                    __multisample_state = multisample_state->getVkStructView();
                    pipeline->setMultisampleState(&__multisample_state);
                }
                if ((_state_flags | VKFW_PIPELINE_DEPTH_STENCIL_STATE_BIT) && depth_stencil_state != nullptr) {
                    __depth_stencil_state = depth_stencil_state->getVkStruct();
                    pipeline->setDepthStencilState(&__depth_stencil_state);
                }
                if ((_state_flags | VKFW_PIPELINE_COLOR_BLEND_STATE_BIT) && color_blend_state != nullptr) {
                    __color_blend_state = color_blend_state->getVkStructView();
                    pipeline->setColorBlendState(&__color_blend_state);
                }
                if ((_state_flags | VKFW_PIPELINE_DYNAMIC_STATE_BIT) && dynamic_state != nullptr) {
                    __dynamic_state = dynamic_state->getVkStructView();
                    pipeline->setDynamicState(&__dynamic_state);
                }

                if (_use_pipeline_cache && pipeline_context == nullptr)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: PipelineGraphicsContextFactory :: createPipeline", "cannot use pipeline cache: is not set"));

                pipeline->setPipelineLayout(pipeline_context->layout);
                pipeline->setShaderStages(shader_stages);
                pipeline_context->pipeline = pipeline->createObject(_device, _render_pass, _subpass, _use_pipeline_cache ? pipeline_context->cache : VK_NULL_HANDLE, _p_next, _flags, _create_cb);
                pipeline_context->pipeline_type = VK_PIPELINE_BIND_POINT_GRAPHICS;
            }
            // void createLayout(VkDevice _device, void *_p_next = nullptr, VkPipelineLayoutCreateFlags _flags = 0u,
            //                   const VkAllocationCallbacks *_create_cb = nullptr) {
            //     pipeline_context->layout = layout->createObject(_device, _p_next, _flags, _create_cb);
            // }
            void createLayout(VkDevice _device, const std::vector<VkPushConstantRange> &_constants = {},
                              const std::vector<VkDescriptorSetLayout> &_layouts = {}, void *_p_next = nullptr,
                              VkPipelineLayoutCreateFlags _flags = 0u, const VkAllocationCallbacks *_create_cb = nullptr) {
                pipeline_context->layout = Pipeline::Layout::CreateObject(_device, _constants, _layouts, _p_next, _flags, _create_cb);
            }
            void createCache(VkDevice _device, void *_p_next = nullptr, VkPipelineCacheCreateFlags _flags = 0u,
                             const VkAllocationCallbacks * _create_cb = nullptr) {
                pipeline_context->cache = cache->createObject(_device, _p_next, _flags, _create_cb);
            }

        // Scenarios

            uint32_t addShaderStage(void *_p_next = nullptr, VkPipelineShaderStageCreateFlags _flags = 0u) {
                shader_stages.push_back(shader_stage->getVkStructDCopy(_p_next, _flags));
                return shader_stages.size() - 1;
            }

            bool updateShaderStage(uint32_t _index, void *_p_next = nullptr, VkPipelineShaderStageCreateFlags _flags = 0u) {
                if (deleteShaderStage(_index)) {
                    shader_stages[_index] = shader_stage->getVkStructDCopy(_p_next, _flags);
                    return true;
                }
                return false;
            }

            bool deleteShaderStage(uint32_t _index) {
                bool __res = pipeline != nullptr && _index < shader_stages.size();
                if (__res)
                    Deleters::deleteStructPtrs(shader_stages[_index]);
                return __res;
            }

            void deleteAndClearShaderStages() {
                if (pipeline != nullptr) {
                    for (uint32_t i = 0u; i < shader_stages.size(); ++i)
                        Deleters::deleteStructPtrs(shader_stages[i]);
                    shader_stages.clear();
                }
            }

            void deleteBuilderAllStates() {
                deleteBuilderPipelineVertexInputState();
                deleteBuilderPipelineInputAssemblyState();
                deleteBuilderPipelineTessellationState();
                deleteBuilderPipelineViewportState();
                deleteBuilderPipelineRasterizationState();
                deleteBuilderPipelineMultisampleState();
                deleteBuilderPipelineDepthStencilState();
                deleteBuilderPipelineColorBlendState();
                deleteBuilderPipelineDynamicState();
            }
        }; // PipelineGraphicsContextFactory END


    // Factory (Compute)

        class PipelineComputeContextFactory {
            PipelineContext *pipeline_context = nullptr;

            Pipeline::Compute::Builder *pipeline = nullptr;
            Pipeline::Cache::Builder *cache = nullptr;

            Pipeline::ShaderStage::StructBuilder *b_shader_stage = nullptr;

        public:
            PipelineComputeContextFactory(PipelineContext *_pipeline_context = nullptr) : pipeline_context{_pipeline_context} {}
            ~PipelineComputeContextFactory() {
                deleteBuilderPipeline();
                deleteBuilderCache();
                deleteBuilderPipelineShaderStage();
                pipeline_context = nullptr;
            }

        // Set context

            void setContext(PipelineContext *_pipeline_context) {
                pipeline_context = _pipeline_context;
            }

        // Destroy context objects

            void destroyObjects(VkDevice _device, const VkAllocationCallbacks *_destroy_callback = nullptr) {
                if (pipeline_context != nullptr) {
                    destroyPipelineContext(_device, *pipeline_context, _destroy_callback);
                    *pipeline_context = PipelineContext{};
                }
            }

        // Allocate builder

            void newBuilderPipeline() {
                deleteBuilderPipeline();
                pipeline = new Pipeline::Compute::Builder{};
            }
            void newBuilderCache() {
                deleteBuilderCache();
                cache = new Pipeline::Cache::Builder{};
            }
            void newBuilderPipelineShaderStage() {
                deleteBuilderPipelineShaderStage();
                b_shader_stage = new Pipeline::ShaderStage::StructBuilder{};
            }

        // Deallocate builder
            // Also deletes shader stages' structures, which are assumed to be deep copied
            void deleteBuilderPipeline() {
                safeDelete(pipeline);
            }
            // void deleteBuilderLayout() {
            //     safeDelete(layout);
            // }
            void deleteBuilderCache() {
                safeDelete(cache);
            }
            void deleteBuilderPipelineShaderStage() {
                safeDelete(b_shader_stage);
            }

        // Get builders

            Pipeline::Compute::Builder *getBuilderPipeline() { return pipeline; }
            Pipeline::Cache::Builder *getBuilderCache() { return cache; }
            Pipeline::ShaderStage::StructBuilder *getBuilderPipelineShaderStage() { return b_shader_stage; }

        // Create objects

            void createPipeline(VkDevice _device, bool _use_pipeline_cache, void *_p_next = nullptr,
                                VkPipelineCreateFlags _flags = 0u, const VkAllocationCallbacks *_create_cb = nullptr) {
                if (_use_pipeline_cache && pipeline_context == nullptr)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: PipelineComputeContextFactory :: createPipeline", "cannot use pipeline cache: not set"));

                pipeline->setPipelineLayout(pipeline_context->layout);
                pipeline->setShaderStage(b_shader_stage->getVkStructView());
                pipeline_context->pipeline = pipeline->createObject(_device, _use_pipeline_cache ? pipeline_context->cache : VK_NULL_HANDLE, _p_next, _flags, _create_cb);
                pipeline_context->pipeline_type = VK_PIPELINE_BIND_POINT_COMPUTE;
            }
            void createLayout(VkDevice _device, const std::vector<VkPushConstantRange> &_constants = {},
                              const std::vector<VkDescriptorSetLayout> &_layouts = {}, void *_p_next = nullptr,
                              VkPipelineLayoutCreateFlags _flags = 0u, const VkAllocationCallbacks *_create_cb = nullptr) {
                pipeline_context->layout = Pipeline::Layout::CreateObject(_device, _constants, _layouts, _p_next, _flags, _create_cb);
            }
            void createCache(VkDevice _device, void *_p_next = nullptr, VkPipelineCacheCreateFlags _flags = 0u,
                             const VkAllocationCallbacks * _create_cb = nullptr) {
                pipeline_context->cache = cache->createObject(_device, _p_next, _flags, _create_cb);
            }

        // Scenarios

            void setShaderStage(VkShaderModule _shader_module, const char *_name) {
                b_shader_stage->setShaderInfo(_shader_module, VK_SHADER_STAGE_COMPUTE_BIT, _name);
            }

            // uint32_t addShaderStage(void *_p_next = nullptr, VkPipelineShaderStageCreateFlags _flags = 0u) {
            //     shader_stages.push_back(shader_stage->getVkStructDCopy(_p_next, _flags));
            //     return shader_stages.size() - 1;
            // }

            // bool updateShaderStage(uint32_t _index, void *_p_next = nullptr, VkPipelineShaderStageCreateFlags _flags = 0u) {
            //     if (deleteShaderStage(_index)) {
            //         shader_stages[_index] = shader_stage->getVkStructDCopy(_p_next, _flags);
            //         return true;
            //     }
            //     return false;
            // }
        }; // PipelineComputeContextFactory END


// Command pool/buffer
// TODO: if there's no new functionality to implement, delete command context and add its contents directly to VulkanContext

    // Context

        struct CommandContext {
            std::vector<VkCommandPool> pools;
            std::vector<VkCommandBuffer> buffers;
            std::vector<uint32_t> buffer_ci_pools;
        }; // CommandContext END

        void destroyCommandContext(VkDevice _device, const CommandContext &_context, const VkAllocationCallbacks *_destroy_callback = nullptr) {
            // Note: command buffers are freed automatically on command pool destruction
            for (uint32_t i = 0u; i < _context.pools.size(); ++i)
                vkDestroyCommandPool(_device, _context.pools[i], _destroy_callback);
        }


    // Factory

        class CommandContextFactory {
            CommandContext *command_context = nullptr;

        public:
            CommandContextFactory(CommandContext *_command_context = nullptr) : command_context{_command_context} {}
            ~CommandContextFactory() {
                command_context = nullptr;
            }

        // Set context

            void setContext(CommandContext *_command_context) {
                command_context = _command_context;
            }

        // Destroy context objects

            void destroyObjects(VkDevice _device, const VkAllocationCallbacks *_destroy_callback = nullptr) {
                if (command_context != nullptr) {
                    destroyCommandContext(_device, *command_context, _destroy_callback);
                    *command_context = CommandContext{};
                }
            }

        // Create objects

            uint32_t createCommandPool(VkDevice _device, uint32_t _queue_family, void *_p_next = nullptr,
                                   VkCommandPoolCreateFlags _flags = 0u, const VkAllocationCallbacks *_create_cb = nullptr) {
                command_context->pools.push_back(CommandPool::CreateObject(_device, _queue_family, _p_next, _flags, _create_cb));
                return command_context->pools.size() - 1;
            }
            // Returns context index of the first buffer from created range. Indices [res_first, res_first + buffer_count) are correct.
            uint32_t createCommandBuffers(VkDevice _device, uint32_t _ci_pool, VkCommandBufferLevel _level, uint32_t _buffer_count,
                                          void *_p_next = nullptr, VkFlags _flags = 0u, const VkAllocationCallbacks *_create_cb = nullptr) {
                std::vector<VkCommandBuffer> __new_buffers{CommandBuffer::CreateObjects(_device, command_context->pools[_ci_pool], _level, _buffer_count, _p_next, _flags, _create_cb)};
                uint32_t __res_first = command_context->buffers.size();

                command_context->buffers.insert(command_context->buffers.end(), __new_buffers.begin(), __new_buffers.end());
                command_context->buffer_ci_pools.resize(command_context->buffer_ci_pools.size() + _buffer_count, _ci_pool);
                return __res_first;
            }
        }; // CommandContextFactory END


// Device memory

    // Context
        // Contains device memory and all device-memory-dependant objects (VkBuffer, VkImage)
        // Note: a good practice is to use one VkBuffer for as many arrays as possible
        //       (i.e. same one for indices, vertices and uniforms)
        struct DeviceMemoryContext {
            std::vector<VkDeviceMemory> memory;
            std::vector<VkBuffer>      buffers;
            std::vector<VkImage>        images;

            // Info about where objects are bound (TODO: move to Factory?)
            struct BindInfo {
                uint32_t  ci_memory;
                uint32_t  ci_object; // buffer/image index, depends on IMAGE_FLAG
                VkDeviceSize offset;
                VkDeviceSize   size; // aligned, from memoryRequirements

                static const uint32_t LASTBIT_FLAG = UINT32_MAX ^ (UINT32_MAX >> 1);

                bool         isInUse() const { return ci_memory &  LASTBIT_FLAG; }
                bool         isImage() const { return ci_object &  LASTBIT_FLAG; }
                uint32_t getMemoryCI() const { return ci_memory & ~LASTBIT_FLAG; }
                uint32_t getObjectCI() const { return ci_object & ~LASTBIT_FLAG; }
            }; // BindInfo END

            std::vector<BindInfo> memory_mapping;
            // TODO: info about device memory properties? Add info about alignment! Save memoryType/memoryHeap indices?
        }; // DeviceMemoryContext END

        void destroyDeviceMemoryContext(VkDevice _device, const DeviceMemoryContext &_context, const VkAllocationCallbacks *_destroy_callback = nullptr) {
            for (uint32_t i = 0u; i < _context.buffers.size(); ++i)
                vkDestroyBuffer(_device, _context.buffers[i], _destroy_callback);
            for (uint32_t i = 0u; i < _context.images.size(); ++i)
                vkDestroyImage(_device, _context.images[i], _destroy_callback);
            for (uint32_t i = 0u; i < _context.memory.size(); ++i)
                vkFreeMemory(_device, _context.memory[i], _destroy_callback);
        }


    // Factory

        class DeviceMemoryContextFactory {
            DeviceMemoryContext *device_memory_context = nullptr;

        public:
            DeviceMemoryContextFactory(DeviceMemoryContext *_device_memory_context = nullptr) : device_memory_context{_device_memory_context} {}
            ~DeviceMemoryContextFactory() {
                device_memory_context = nullptr;
            }

        // Set context

            void setContext(DeviceMemoryContext *_device_memory_context) {
                device_memory_context = _device_memory_context;
            }

        // Destroy context objects

            void destroyObjects(VkDevice _device, const VkAllocationCallbacks *_destroy_callback = nullptr) {
                if (device_memory_context != nullptr) {
                    destroyDeviceMemoryContext(_device, *device_memory_context, _destroy_callback);
                    *device_memory_context = DeviceMemoryContext{};
                }
            }

        // Create objects

            uint32_t createBuffer(VkDevice _device, const std::vector<uint32_t> &_queue_families, VkSharingMode _sharing_mode,
                                  VkDeviceSize _size_bytes, VkBufferUsageFlags _usage_flags, const void *_p_next = nullptr,
                                  VkBufferCreateFlags _flags = 0u, const VkAllocationCallbacks *_create_cb = nullptr) {
                VkBuffer __buffer = Buffer::CreateObject(_device, _queue_families, _sharing_mode, _size_bytes, _usage_flags, _p_next, _flags, _create_cb);
                device_memory_context->buffers.push_back(__buffer);
                return device_memory_context->buffers.size() - 1;
            }
            // For one memory buffer
            uint32_t createDeviceMemory(VkPhysicalDevice _phys_device, VkDevice _device, uint32_t _ci_buffer, VkMemoryPropertyFlags _req_memory_properties,
                                        void *_p_next = nullptr, const VkAllocationCallbacks * _create_cb = nullptr) {
                VkMemoryRequirements __mem_requirements;
                vkGetBufferMemoryRequirements(_device, device_memory_context->buffers[_ci_buffer], &__mem_requirements);

                VkMemoryAllocateInfo __info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
                __info.pNext = _p_next;
                __info.allocationSize = __mem_requirements.size;
                __info.memoryTypeIndex = Util::checkPhysicalDeviceMemoryProperties(_phys_device, __mem_requirements.memoryTypeBits, _req_memory_properties);

                if (__info.memoryTypeIndex == UINT32_MAX)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW :: DeviceMemoryContextFactory :: createDeviceMemory", "Could not find suitable memory type"));

                VkDeviceMemory __memory;
                vkAllocateMemory(_device, &__info, _create_cb, &__memory);

                DeviceMemoryContext::BindInfo __memory_mapping{ .ci_memory = (uint32_t)device_memory_context->memory.size(),
                                                                .ci_object = _ci_buffer, .offset = 0ul, .size = __info.allocationSize };
                device_memory_context->memory_mapping.push_back(__memory_mapping);

                device_memory_context->memory.push_back(__memory);

                // TODO: this function is only called once and there is no vkUnbindBufferMemory, but if for some reason there's a need to prevent from calling it now, add bool flag
                vkBindBufferMemory(_device, device_memory_context->buffers[_ci_buffer], __memory, __memory_mapping.offset);
                return __memory_mapping.ci_memory;
            }

        // Scenarios

            void bMemoryFillBase(VkDevice _device, uint32_t _ci_buffer, const void *_data, VkDeviceSize _size, VkDeviceSize _offset = 0ul, VkMemoryMapFlags _flags = 0u) {
                void* __device_memory_ptr = nullptr;
                DeviceMemoryContext::BindInfo __info = bBufferGetBindInfo(_ci_buffer);
                if (_size == 0ul)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: DeviceMemoryContextFactory :: bMemoryFillBase", "'size' must be greater than 0"));

                if (_offset + _size > __info.size)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW :: DeviceMemoryContextFactory :: bMemoryFillBase", "data size + offset are out of bounds for current object's memory. \
                                                                                                               It will overwrite other object's memory or result in memory mapping error."));

                VkDeviceMemory __memory_handle = device_memory_context->memory[__info.getMemoryCI()];
                vkMapMemory(_device, __memory_handle, __info.offset, __info.size, _flags, &__device_memory_ptr);

                memcpy((char*)__device_memory_ptr + _offset, _data, _size);

                vkUnmapMemory(_device, __memory_handle);
            }

            // size and offset are for this buffer specifically (from the beginning of this buffer's memory at BindInfo::offset)
            // _size is THE SIZE OF *_data IN BYTES. The function always maps all of the buffer's memory.
            void bBufferMemoryFill(VkDevice _device, uint32_t _ci_buffer, const void *_data, VkDeviceSize _size, VkDeviceSize _offset = 0ul, VkMemoryMapFlags _flags = 0u) {
                bMemoryFillBase(_device, _ci_buffer, _data, _size, _offset, _flags);
            }

            // size and offset are for this image specifically (from the beginning of this image's memory at BindInfo::offset)
            // _size is THE SIZE OF *_data IN BYTES. The function always maps all of the image's memory.
            void bImageMemoryFill(VkDevice _device, uint32_t _ci_image, const void *_data, VkDeviceSize _size, VkDeviceSize _offset = 0ul, VkMemoryMapFlags _flags = 0u) {
                _ci_image |= DeviceMemoryContext::BindInfo::LASTBIT_FLAG;
                bMemoryFillBase(_device, _ci_image, _data, _size, _offset, _flags);
            }

            DeviceMemoryContext::BindInfo bBufferGetBindInfo(uint32_t _ci_buffer) {
                DeviceMemoryContext::BindInfo __res{};

                for (uint32_t i = 0u; i < device_memory_context->memory_mapping.size(); ++i) {
                    if (device_memory_context->memory_mapping[i].ci_object == _ci_buffer) {
                        __res = device_memory_context->memory_mapping[i];
                        break;
                    }
                }
                return __res;
            }

            DeviceMemoryContext::BindInfo bImageGetBindInfo(uint32_t _ci_image) {
                // This hack is due to how data in BindInfo is stored
                return bBufferGetBindInfo(_ci_image | DeviceMemoryContext::BindInfo::LASTBIT_FLAG);
            }
        }; // DeviceMemoryContextFactory END


// Vulkan
// TODO: Think about moving Instance, PhysicalDevice and Devices (std::vector) to a small InstanceContext.

    // Context

        struct VulkanContext {
            VkInstance instance = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            VkPhysicalDevice phys_device = VK_NULL_HANDLE;
            std::vector<uint32_t> queue_families;
            std::vector<VkQueue> queues;

            // std::vector<VkImageView> image_views;

            VkSurfaceKHR surface = VK_NULL_HANDLE;
            SwapchainContext swapchain_context;

            VkRenderPass render_pass = VK_NULL_HANDLE;
            std::vector<VkShaderModule> shader_modules;

            PipelineContext pipeline_context;
            CommandContext command_context;
            DeviceMemoryContext device_memory_context;
            DescriptorContext descriptor_context;

            std::vector<VkSemaphore> semaphores;
            std::vector<VkFence> fences;

            VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;

        // Get objects

            VkFence getObjectFence(uint32_t _ci_fence) const {
                return fences[_ci_fence];
            }
            VkSemaphore getObjectSemaphore(uint32_t _ci_semaphore) const {
                return semaphores[_ci_semaphore];
            }
            VkCommandPool getObjectCommandPool(uint32_t _ci_pool) const {
                return command_context.pools[_ci_pool];
            }
            VkCommandBuffer getObjectCommandBuffer(uint32_t _ci_command_buffer) const {
                return command_context.buffers[_ci_command_buffer];
            }
            VkFramebuffer getObjectFramebuffer(uint32_t _ci_framebuffer) const {
                return swapchain_context.framebuffers[_ci_framebuffer];
            }
            VkQueue getObjectQueue(uint32_t _ci_queue) const {
                return queues[_ci_queue];
            }
            uint32_t getObjectQueueFamily(uint32_t _ci_qfamily) const {
                return queue_families[_ci_qfamily];
            }
            VkBuffer getObjectBuffer(uint32_t _ci_buffer) const {
                return device_memory_context.buffers[_ci_buffer];
            }
            VkImage getObjectImage(uint32_t _ci_image) const {
                return device_memory_context.images[_ci_image];
            }
            VkDeviceMemory getObjectDeviceMemory(uint32_t _ci_device_memory) const {
                return device_memory_context.memory[_ci_device_memory];
            }
            VkDescriptorSetLayout getObjectDescriptorSetLayout(uint32_t _ci_layout) {
                return descriptor_context.layouts[_ci_layout];
            }
            VkDescriptorPool getObjectDescriptorPool(uint32_t _ci_pool) {
                return descriptor_context.pools[_ci_pool];
            }
            VkDescriptorSet getObjectDescriptorSet(uint32_t _ci_set) {
                return descriptor_context.sets[_ci_set];
            }

            std::vector<VkFence> getObjectVecFence(const std::vector<uint32_t> &_ci_fences) const {
                std::vector<VkFence> __res;
                __res.resize(_ci_fences.size());
                for (uint32_t i = 0u; i < _ci_fences.size(); ++i)
                    __res[i] = fences[_ci_fences[i]];
                return __res;
            }
            std::vector<VkSemaphore> getObjectVecSemaphore(const std::vector<uint32_t> &_ci_semaphores) const {
                std::vector<VkSemaphore> __res;
                __res.resize(_ci_semaphores.size());
                for (uint32_t i = 0u; i < _ci_semaphores.size(); ++i)
                    __res[i] = semaphores[_ci_semaphores[i]];
                return __res;
            }
            std::vector<VkCommandBuffer> getObjectVecCommandBuffer(const std::vector<uint32_t> &_ci_command_buffers) const {
                std::vector<VkCommandBuffer> __res;
                __res.resize(_ci_command_buffers.size());
                for (uint32_t i = 0u; i < _ci_command_buffers.size(); ++i)
                    __res[i] = command_context.buffers[_ci_command_buffers[i]];
                return __res;
            }
            std::vector<VkFramebuffer> getObjectVecFramebuffer(const std::vector<uint32_t> &_ci_framebuffers) const {
                std::vector<VkFramebuffer> __res;
                __res.resize(_ci_framebuffers.size());
                for (uint32_t i = 0u; i < _ci_framebuffers.size(); ++i)
                    __res[i] = swapchain_context.framebuffers[_ci_framebuffers[i]];
                return __res;
            }
            std::vector<VkQueue> getObjectVecQueue(const std::vector<uint32_t> &_ci_queues) const {
                std::vector<VkQueue> __res;
                __res.resize(_ci_queues.size());
                for (uint32_t i = 0u; i < _ci_queues.size(); ++i)
                    __res[i] = queues[_ci_queues[i]];
                return __res;
            }
            std::vector<uint32_t> getObjectVecQueueFamily(const std::vector<uint32_t> &_ci_qfamilies) const {
                std::vector<uint32_t> __res;
                __res.resize(_ci_qfamilies.size());
                for (uint32_t i = 0u; i < _ci_qfamilies.size(); ++i)
                    __res[i] = queue_families[_ci_qfamilies[i]];
                return __res;
            }
            std::vector<VkBuffer> getObjectVecBuffer(const std::vector<uint32_t> &_ci_buffers) const {
                std::vector<VkBuffer> __res;
                __res.resize(_ci_buffers.size());
                for (uint32_t i = 0u; i < _ci_buffers.size(); ++i)
                    __res[i] = device_memory_context.buffers[_ci_buffers[i]];
                return __res;
            }
            std::vector<VkDeviceMemory> getObjectVecDeviceMemory(const std::vector<uint32_t> &_ci_device_memory) const {
                std::vector<VkDeviceMemory> __res;
                __res.resize(_ci_device_memory.size());
                for (uint32_t i = 0u; i < _ci_device_memory.size(); ++i)
                    __res[i] = device_memory_context.memory[_ci_device_memory[i]];
                return __res;
            }
            std::vector<VkDescriptorSetLayout> getObjectVecDescriptorSetLayout(const std::vector<uint32_t> &_ci_layout) const {
                std::vector<VkDescriptorSetLayout> __res;
                __res.resize(_ci_layout.size());
                for (uint32_t i = 0u; i < _ci_layout.size(); ++i)
                    __res[i] = descriptor_context.layouts[_ci_layout[i]];
                return __res;
            }
            std::vector<VkDescriptorPool> getObjectVecDescriptorPool(const std::vector<uint32_t> &_ci_pool) const {
                std::vector<VkDescriptorPool> __res;
                __res.resize(_ci_pool.size());
                for (uint32_t i = 0u; i < _ci_pool.size(); ++i)
                    __res[i] = descriptor_context.pools[_ci_pool[i]];
                return __res;
            }
            std::vector<VkDescriptorSet> getObjectVecDescriptorSet(const std::vector<uint32_t> &_ci_set) const {
                std::vector<VkDescriptorSet> __res;
                __res.resize(_ci_set.size());
                for (uint32_t i = 0u; i < _ci_set.size(); ++i)
                    __res[i] = descriptor_context.sets[_ci_set[i]];
                return __res;
            }
        } Vulkan_global_context; // VulkanContext END

        void destroyVulkanContext(const VulkanContext &_context, const VkAllocationCallbacks *_destroy_callback = nullptr) {
            vkDeviceWaitIdle(_context.device);

            destroyCommandContext(_context.device, _context.command_context, _destroy_callback);
            destroyPipelineContext(_context.device, _context.pipeline_context, _destroy_callback);
            destroySwapchainContext(_context.device, _context.swapchain_context, _destroy_callback);
            destroyDescriptorContext(_context.device, _context.descriptor_context, _destroy_callback);
            destroyDeviceMemoryContext(_context.device, _context.device_memory_context, _destroy_callback);

            for (uint32_t i = 0u; i < _context.semaphores.size(); ++i)
                vkDestroySemaphore(_context.device, _context.semaphores[i], _destroy_callback);
            for (uint32_t i = 0u; i < _context.fences.size(); ++i)
                vkDestroyFence(_context.device, _context.fences[i], _destroy_callback);
            if (_context.render_pass != VK_NULL_HANDLE)
                vkDestroyRenderPass(_context.device, _context.render_pass, _destroy_callback);
            for (uint32_t i = 0u; i < _context.shader_modules.size(); ++i)
                vkDestroyShaderModule(_context.device, _context.shader_modules[i], _destroy_callback);
            if (_context.surface != nullptr)
                vkDestroySurfaceKHR(_context.instance, _context.surface, _destroy_callback);
            // no need/impossible to destroy queues, queue families, physical devices
            if (_context.device != VK_NULL_HANDLE)
                vkDestroyDevice(_context.device, _destroy_callback);
            if (_context.messenger != VK_NULL_HANDLE)
                vkDestroyDebugUtilsMessengerEXT(_context.instance, _context.messenger, _destroy_callback);
            if (_context.instance != VK_NULL_HANDLE)
                vkDestroyInstance(_context.instance, _destroy_callback);
        }


    // Factory

        class VulkanContextFactory {
            VulkanContext *vk_context = nullptr;

            Instance::Builder *b_instance = nullptr;
            Device::Builder *b_device = nullptr;
            PhysicalDevice::Manager *m_phys_device = nullptr;
            QueueFamily::Manager *m_queue_family = nullptr;
            Queue::Builder *b_queue = nullptr;

            ImageView::Builder *b_image_view = nullptr;

            RenderPass::Builder *b_render_pass = nullptr;
            ShaderModule::Builder *b_shader_module = nullptr;

            PipelineGraphicsContextFactory c_graphics_pipeline;
            PipelineComputeContextFactory  c_compute_pipeline = nullptr;
            // PipelineRayTracingContextFactory *c_raytracing_pipeline = nullptr;
            SwapchainContextFactory c_swapchain;
            DeviceMemoryContextFactory c_device_memory;
            CommandContextFactory c_command;
            DescriptorContextFactory c_descriptor;

            DebugUtilsMessengerEXT::Builder *b_messenger = nullptr;

            VkAllocationCallbacks *alloc_callback = nullptr;

        public:
            VulkanContextFactory(bool _volk_init = true, VulkanContext *_vk_context = nullptr) : vk_context{_vk_context} {
                if (_volk_init) {
                    if (volkInitialize() != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW :: VulkanContextFactory", "could not initialize Volk"));
                }
            }
            ~VulkanContextFactory() {
                deleteContextCommand();
                deleteContextPipelineGraphics();
                deleteContextPipelineCompute();
                deleteContextSwapchain();
                deleteContextDescriptor();
                deleteBuilderRenderPass();
                deleteBuilderShaderModule();
                deleteBuilderImageView();
                deleteBuilderQueue();
                deleteBuilderQueueFamilies();
                deleteBuilderDevice();
                deleteBuilderPhysicalDevice();
                deleteBuilderMessenger();
                deleteBuilderInstance();
            }

        // Set context

            void setContext(VulkanContext *_vk_context) {
                vk_context = _vk_context;
                c_swapchain.setContext(&_vk_context->swapchain_context);
                c_graphics_pipeline.setContext(&_vk_context->pipeline_context);
                c_command.setContext(&_vk_context->command_context);
                c_device_memory.setContext(&_vk_context->device_memory_context);
                c_descriptor.setContext(&vk_context->descriptor_context);
            }

        // Allocate builder

            void newBuilderInstance() {
                deleteBuilderInstance();
                b_instance = new Instance::Builder;
            }
            void newBuilderDevice() {
                deleteBuilderDevice();
                b_device = new Device::Builder;
            }
            void newBuilderPhysicalDevice() {
                deleteBuilderPhysicalDevice();
                m_phys_device = new PhysicalDevice::Manager;
            }
            void newBuilderQueueFamilies() {
                deleteBuilderQueueFamilies();
                m_queue_family = new QueueFamily::Manager;
            }
            void newBuilderQueue() {
                deleteBuilderQueue();
                b_queue = new Queue::Builder;
            }
            void newBuilderImageView() {
                deleteBuilderImageView();
                b_image_view = new ImageView::Builder;
            }
            void newBuilderShaderModule() {
                deleteBuilderShaderModule();
                b_shader_module = new ShaderModule::Builder;
            }
            void newBuilderMessenger() {
                deleteBuilderMessenger();
                b_messenger = new DebugUtilsMessengerEXT::Builder;
            }
            void newBuilderRenderPass() {
                deleteBuilderRenderPass();
                b_render_pass = new RenderPass::Builder;
            }


        // Deallocate builder

            void deleteBuilderInstance() {
                safeDelete(b_instance);
            }
            void deleteBuilderDevice() {
                if (b_device != nullptr) {
                    for (uint32_t i = 0u; i < b_device->queue_info.size(); ++i)
                        Deleters::deleteStructPtrs(b_device->queue_info[i]);
                    b_device->queue_info.clear();
                }
                safeDelete(b_device);
            }
            void deleteBuilderPhysicalDevice() {
                safeDelete(m_phys_device);
            }
            void deleteBuilderQueueFamilies() {
                safeDelete(m_queue_family);
            }
            void deleteBuilderQueue() {
                safeDelete(b_queue);
            }
            void deleteBuilderImageView() {
                safeDelete(b_image_view);
            }
            void deleteBuilderShaderModule() {
                safeDelete(b_shader_module);
            }
            void deleteBuilderMessenger() {
                safeDelete(b_messenger);
            }
            void deleteBuilderRenderPass() {
                if (b_render_pass != nullptr) {
                    for (uint32_t i = 0u; i < b_render_pass->subpasses.size(); ++i)
                        Deleters::deleteStructPtrs(b_render_pass->subpasses[i]);
                }
                safeDelete(b_render_pass);
            }
            void deleteContextSwapchain() {
                c_swapchain.~SwapchainContextFactory();
            }
            void deleteContextPipelineGraphics() {
                c_graphics_pipeline.~PipelineGraphicsContextFactory();
            }
            void deleteContextPipelineCompute() {
                c_compute_pipeline.~PipelineComputeContextFactory();
            }
            void deleteContextCommand() {
                c_command.~CommandContextFactory();
            }
            void deleteContextDescriptor() {
                c_descriptor.~DescriptorContextFactory();
            }


        // Get builders

            Instance::Builder* getBuilderInstance() { return b_instance; }
            Device::Builder* getBuilderDevice() { return b_device; }
            PhysicalDevice::Manager* getBuilderPhysicalDevice() { return m_phys_device; }
            QueueFamily::Manager* getBuilderQueueFamilies() { return m_queue_family; }
            Queue::Builder* getBuilderQueue() { return b_queue; }
            ImageView::Builder* getBuilderImageView() { return b_image_view; }
            DebugUtilsMessengerEXT::Builder* getBuilderMessenger() { return b_messenger; }
            SwapchainContextFactory* getContextSwapchain() { return &c_swapchain; }
            RenderPass::Builder* getBuilderRenderPass() { return b_render_pass; }
            ShaderModule::Builder* getBuilderShaderModule() { return b_shader_module; }
            PipelineGraphicsContextFactory* getContextPipelineGraphics() { return &c_graphics_pipeline; }
            PipelineComputeContextFactory* getContextPipelineCompute() { return &c_compute_pipeline; }
            CommandContextFactory* getContextCommand() { return &c_command; }
            DeviceMemoryContextFactory* getContextDeviceMemory() { return &c_device_memory; }
            DescriptorContextFactory* getContextDescriptor() { return &c_descriptor; }

        // Create objects

            void createInstance(void *_p_next = nullptr, VkInstanceCreateFlags _flags = 0u) {
                vk_context->instance = b_instance->createObject(_p_next, _flags, alloc_callback);
            }
            void pickPhysicalDevice(Simple::VKFW::PhysicalDevice::Suitability::TestFunc _test_func) {
                vk_context->phys_device = PhysicalDevice::Manager{vk_context->instance}.testPickOne(_test_func);
            }
            void createDevice(void *_p_next = nullptr, VkDeviceCreateFlags _flags = 0u) {
                vk_context->device = b_device->createObject(vk_context->phys_device, _p_next, _flags, alloc_callback);
            }
            // Returns "Queue family index"'s index in vk_context->queue_families or -1 on error (could not find queue family)
            // Indices of Context vector members are called that type's "context indices".
            uint32_t pickQueueFamily(Simple::VKFW::QueueFamily::Suitability::TestFunc _test_func) {
                uint32_t __res = QueueFamily::Manager{vk_context->instance, vk_context->phys_device}.testPickOne(_test_func);
                if (__res != -1) {
                    vk_context->queue_families.push_back(__res);
                    __res = vk_context->queue_families.size() - 1;
                }
                return __res;
            }
            // Returns "Queue family index"'s index in vk_context->queue_families or -1 on error (could not find queue family)
            // Indices of Context vector members are called that type's "context indices".
            uint32_t pickQueueFamily(Simple::VKFW::QueueFamily::Suitability::RankFunc _test_func) {
                uint32_t __res = QueueFamily::Manager{vk_context->instance, vk_context->phys_device}.rankPickOne(_test_func);
                if (__res != -1) {
                    vk_context->queue_families.push_back(__res);
                    __res = vk_context->queue_families.size() - 1;
                }
                return __res;
            }
            // Returns queue context index of the first added queue
            uint32_t addFamilyQueues(uint32_t _qfamily_context_index, uint32_t _queue_count) {
                uint32_t __queue_family_index = vk_context->queue_families[_qfamily_context_index];
                uint32_t __offset = vk_context->queues.size();

                for (uint32_t i = 0u; i < _queue_count; ++i)
                    vk_context->queues.push_back(Queue::CreateObject(vk_context->device, __queue_family_index, i));

                return __offset;
            }

            // Returns shader module context index
            uint32_t createShaderModuleFromFile(const std::string &_fpath, void *_p_next = nullptr, VkShaderModuleCreateFlags _flags = 0u,
                                                const VkAllocationCallbacks * _create_cb = nullptr) {
                b_shader_module->fromFile(_fpath.c_str());
                vk_context->shader_modules.push_back(b_shader_module->createObject(vk_context->device, _p_next, _flags, alloc_callback));
                return vk_context->shader_modules.size() - 1;
            }
            // Returns shader module context index
            uint32_t createShaderModuleFromString(const std::string &_shader, void *_p_next = nullptr, VkShaderModuleCreateFlags _flags = 0u,
                                                  const VkAllocationCallbacks * _create_cb = nullptr) {
                b_shader_module->fromString(_shader);
                vk_context->shader_modules.push_back(b_shader_module->createObject(vk_context->device, _p_next, _flags, alloc_callback));
                return vk_context->shader_modules.size() - 1;
            }

            void createSurface(Window &_glfw_window) {
                vk_context->surface = SurfaceKHR::CreateObject(vk_context->instance, _glfw_window, alloc_callback);
            }

            void createSwapchain(void *_p_next = nullptr, VkSwapchainCreateFlagsKHR _flags = 0u) {
                c_swapchain.createSwapchain(vk_context->device, vk_context->surface, _p_next, _flags, alloc_callback);
            }
            void createSwapchainImageViews() {
                c_swapchain.createImageViews(vk_context->device);
            }
            void createSwapchainFramebuffers() {
                c_swapchain.createFramebuffers(vk_context->device, vk_context->render_pass);
            }

            void createRenderPass(void *_p_next = nullptr, VkRenderPassCreateFlags _flags = 0u) {
                vk_context->render_pass = b_render_pass->createObject(vk_context->device, _p_next, _flags, alloc_callback);
            }

            void createPipelineGraphics(uint32_t _subpass, bool _use_pipeline_cache,
                                        PipelineStateFlags _state_flags = 0x000001ff, void *_p_next = nullptr,
                                        VkPipelineCreateFlags _flags = 0u) {
                c_graphics_pipeline.createPipeline(vk_context->device, vk_context->render_pass, _subpass, _use_pipeline_cache, _state_flags, _p_next, _flags, alloc_callback);
            }
            void createPipelineGraphicsCache(void *_p_next = nullptr, VkPipelineCacheCreateFlags _flags = 0u) {
                c_graphics_pipeline.createCache(vk_context->device, _p_next, _flags, alloc_callback);
            }
            // void createPipelineGraphicsLayout(void *_p_next = nullptr, VkPipelineLayoutCreateFlags _flags = 0u) {
            //     c_graphics_pipeline.createLayout(vk_context->device, _p_next, _flags, alloc_callback);
            // }
            void createPipelineGraphicsLayout(const std::vector<VkPushConstantRange> &_constants = {},
                                              const std::vector<uint32_t> &_ci_layouts = {},
                                              void *_p_next = nullptr, VkPipelineLayoutCreateFlags _flags = 0u) {
                c_graphics_pipeline.createLayout(vk_context->device, _constants, vk_context->getObjectVecDescriptorSetLayout(_ci_layouts), _p_next, _flags, alloc_callback);
            }

            uint32_t createCommandPool(uint32_t _ci_qfamily, void *_p_next = nullptr, VkCommandPoolCreateFlags _flags = 0u) {
                return c_command.createCommandPool(vk_context->device, vk_context->queue_families[_ci_qfamily], _p_next, _flags, alloc_callback);
            }
            uint32_t createCommandBuffers(uint32_t _ci_pool, VkCommandBufferLevel _level, uint32_t _buffer_count, void *_p_next = nullptr, VkFlags _flags = 0u) {
                return c_command.createCommandBuffers(vk_context->device, _ci_pool, _level, _buffer_count, _p_next, _flags, alloc_callback);
            }

            uint32_t createBuffer(const std::vector<uint32_t> &_ci_qfamilies, VkSharingMode _sharing_mode, VkDeviceSize _size_bytes,
                                  VkBufferUsageFlags _usage_flags, const void *_p_next = nullptr, VkBufferCreateFlags _flags = 0u) {
                return c_device_memory.createBuffer(vk_context->device, vk_context->getObjectVecQueueFamily(_ci_qfamilies), _sharing_mode, _size_bytes, _usage_flags, _p_next, _flags, alloc_callback);
            }

            uint32_t createSemaphore(void *_p_next = nullptr, VkSemaphoreCreateFlags _flags = 0u) {
                vk_context->semaphores.push_back(Semaphore::CreateObject(vk_context->device, _p_next, _flags, alloc_callback));
                return vk_context->semaphores.size() - 1;
            }

            uint32_t createFence(void *_p_next = nullptr, VkFenceCreateFlags _flags = 0u) {
                vk_context->fences.push_back(Fence::CreateObject(vk_context->device, _p_next, _flags, alloc_callback));
                return vk_context->fences.size() - 1;
            }

            uint32_t createDeviceMemory(uint32_t _ci_buffer, VkMemoryPropertyFlags _req_memory_properties, void *_p_next = nullptr) {
                return c_device_memory.createDeviceMemory(vk_context->phys_device, vk_context->device, _ci_buffer, _req_memory_properties, _p_next, alloc_callback);
            }

            uint32_t createDescriptorSetLayout(const void *_p_next = nullptr, VkDescriptorSetLayoutCreateFlags _flags = 0u) {
                return c_descriptor.createLayout(vk_context->device, _p_next, _flags, alloc_callback);
            }

            uint32_t createDescriptorPool(uint32_t _max_sets, const std::vector<VkDescriptorPoolSize> &_pool_sizes = {},
                                          const void *_p_next = (const void *)nullptr, VkDescriptorPoolCreateFlags _flags = 0u) {
                return c_descriptor.createDescriptorPool(vk_context->device, _max_sets, _pool_sizes, _p_next, _flags, alloc_callback);
            }

            uint32_t createDescriptorSets(uint32_t _ci_pool, const std::vector<uint32_t> &_ci_layouts, void *_p_next = nullptr) {
                return c_descriptor.createDescriptorSets(vk_context->device, _ci_pool, _ci_layouts, _p_next);
            }

            void createMessenger(void *_p_next = nullptr, VkDebugUtilsMessengerCreateFlagsEXT _flags = 0u) {
                vk_context->messenger = b_messenger->createObject(vk_context->instance, _p_next, _flags, alloc_callback);
            }

        // Scenarios

            // Note: prefixes: c* - if context-related, b* - if builder-related, o* - if object has neither context nor builder

            void bDeviceIncludeValLayersLegacy() {
                if (Simple::VKFW::VulkanAPIVersion::available() <= VK_API_VERSION_1_0)
                    b_device->addValLayers(vk_context->phys_device);
            }

            void bDeviceAddQueueInfo(uint32_t _qfamily_context_index, const std::vector<float> _priorities,
                                     void *_p_next = nullptr, VkDeviceQueueCreateFlags _flags = 0u) {
                b_device->addQueueInfo({Queue::GetVkStructDCopy(vk_context->queue_families[_qfamily_context_index], _priorities, _p_next, _flags)});
            }

            void bDeviceAddQueueInfo(uint32_t _qfamily_context_index, uint32_t _queue_count, float _eq_priorities_val = 1.0f,
                                     void *_p_next = nullptr, VkDeviceQueueCreateFlags _flags = 0u) {
                b_device->addQueueInfo({Queue::GetVkStructDCopy(vk_context->queue_families[_qfamily_context_index], _queue_count, _eq_priorities_val, _p_next, _flags)});
            }

            void cSwapchainSetCapabilities() {
                c_swapchain.getBuilderSwapchainKHR()->setSurfaceCapabilities(vk_context->phys_device, vk_context->surface);
            }
            void cSwapchainSetPresentationMode(VkPresentModeKHR _pres_mode) {
                c_swapchain.getBuilderSwapchainKHR()->checkSetPresentationMode(vk_context->phys_device, vk_context->surface, _pres_mode);
            }
            void cSwapchainPickFormat(const Simple::VKFW::SwapchainKHR::Suitability::FormatTestFunc _test_func) {
                c_swapchain.getBuilderSwapchainKHR()->pickFormat(vk_context->phys_device, vk_context->surface, _test_func);
            }
            void cSwapchainPickFormat(const Simple::VKFW::SwapchainKHR::Suitability::FormatRankFunc _rank_func) {
                c_swapchain.getBuilderSwapchainKHR()->pickFormat(vk_context->phys_device, vk_context->surface, _rank_func);
            }

            void cSwapchainRecreate(Window &_glfw_window) {
                int width = 0, height = 0;

                glfwGetFramebufferSize(_glfw_window.window, &width, &height);
                while (width == 0 || height == 0) {
                    glfwGetFramebufferSize(_glfw_window.window, &width, &height);
                    glfwWaitEvents();
                }

                cSwapchainSetCapabilities();
                vkDeviceWaitIdle(vk_context->device);
                c_swapchain.recreate(vk_context->device, vk_context->render_pass, vk_context->surface, alloc_callback);
            }

            // Returns graphics_pipeline shader_stage context index
            uint32_t cPipelineGraphicsAddShaderStage(uint32_t _shader_module_context_index, VkShaderStageFlagBits _stage, const char *_name, const VkSpecializationInfo &_spec_info = {}) {
                c_graphics_pipeline.getBuilderPipelineShaderStage()->setShaderInfo(vk_context->shader_modules[_shader_module_context_index], _stage, _name);
                c_graphics_pipeline.getBuilderPipelineShaderStage()->setSpecializationInfo(_spec_info);
                return c_graphics_pipeline.addShaderStage();
            }

            // TODO: Returns compute_pipeline shader_stage context index
            uint32_t cPipelineComputeAddShaderStage(uint32_t _shader_module_context_index, const char *_name, const VkSpecializationInfo &_spec_info = {}) {
                c_compute_pipeline.setShaderStage(vk_context->shader_modules[_shader_module_context_index], _name);
                c_compute_pipeline.getBuilderPipelineShaderStage()->setSpecializationInfo(_spec_info);
                return 0u;
            }

            // TODO: This function must not replace the functionality of mapping/unmapping VkDeviceMemory by its context index
            //       So, wrap vkMapMemory/vkUnmapMemory.
            void cBufferMemoryFill(uint32_t _ci_buffer, const void *_data, VkDeviceSize _size, VkDeviceSize _offset = 0ul, VkMemoryMapFlags _flags = 0u) {
                c_device_memory.bBufferMemoryFill(vk_context->device, _ci_buffer, _data, _size, _offset, _flags);
            }

            void cImageMemoryFill(uint32_t _ci_image, const void *_data, VkDeviceSize _size, VkDeviceSize _offset = 0ul, VkMemoryMapFlags _flags = 0u) {
                c_device_memory.bImageMemoryFill(vk_context->device, _ci_image, _data, _size, _offset, _flags);
            }
        } Vulkan_global_context_factory; // VulkanContextFactory END


// Context-based wrapper for function calls

        class VulkanContextFunc {
            VulkanContext *vk_context = nullptr;
            VulkanContextFactory *vk_factory = nullptr;

            Cmd::BeginRenderPass::Builder *b_begin_render_pass = nullptr;
            Func::UpdateDescriptorSets::Builder *b_update_descriptor_sets = nullptr;

        public:
            VulkanContextFunc(VulkanContext *_vk_context = nullptr,
                              VulkanContextFactory *_vk_factory = nullptr) : vk_context{_vk_context}, vk_factory{_vk_factory} {}

        // Set context/factory

            void setContext(VulkanContext *_vk_context) {
                vk_context = _vk_context;
            }
            void setFactory(VulkanContextFactory *_vk_factory) {
                vk_factory = _vk_factory;
            }
            void setFactoryContext(VulkanContext *_vk_context, VulkanContextFactory *_vk_factory) {
                vk_context = _vk_context;
                vk_factory = _vk_factory;
            }

        // New command/func info builders

            void newBuilderCmdBeginRenderPass() {
                deleteBuilderCmdBeginRenderPass();
                b_begin_render_pass = new Cmd::BeginRenderPass::Builder;
            }

            void newBuilderUpdateDescriptorSets() {
                deleteBuilderUpdateDescriptorSets();
                b_update_descriptor_sets = new Func::UpdateDescriptorSets::Builder;
            }

        // Deallocate command/func info builders

            void deleteBuilderCmdBeginRenderPass() {
                safeDelete(b_begin_render_pass);
            }

            void deleteBuilderUpdateDescriptorSets() {
                safeDelete(b_update_descriptor_sets);
            }

        // Get command/func info builders

            Cmd::BeginRenderPass::Builder *getBuilderCmdBeginRenderPass() { return b_begin_render_pass; }
            Func::UpdateDescriptorSets::Builder *getBuilderUpdateDescriptorSets() { return b_update_descriptor_sets; }

        // Functions

            void waitForFences(const std::vector<uint32_t> _ci_fences, VkBool32 _wait_all = VK_TRUE, uint64_t _wait_time = UINT64_MAX) {
                std::vector<VkFence> __fences{vk_context->getObjectVecFence(_ci_fences)};
                vkWaitForFences(vk_context->device, __fences.size(), __fences.data(), _wait_all, _wait_time);
            }

            void resetFences(const std::vector<uint32_t> _ci_fences) {
                std::vector<VkFence> __fences{vk_context->getObjectVecFence(_ci_fences)};
                vkResetFences(vk_context->device, __fences.size(), __fences.data());
            }

            void beginCommandBuffer(uint32_t _ci_command_buffer, void *_p_next = nullptr, VkCommandBufferUsageFlags _flags = 0u,
                                    const VkCommandBufferInheritanceInfo *_inheritance_info = nullptr) {
                Func::beginCommandBuffer(vk_context->getObjectCommandBuffer(_ci_command_buffer), _p_next, _flags, _inheritance_info);
            }

            void resetCommandBuffer(uint32_t _ci_command_buffer, VkCommandBufferResetFlags _flags = 0u) {
                vkResetCommandBuffer(vk_context->getObjectCommandBuffer(_ci_command_buffer), _flags);
            }

            void endCommandBuffer(uint32_t _ci_command_buffer) {
                vkEndCommandBuffer(vk_context->getObjectCommandBuffer(_ci_command_buffer));
            }

            void queueSubmit(uint32_t _ci_queue, const std::vector<VkSubmitInfo> &_submits, uint32_t _ci_fence = UINT32_MAX) {
                vkQueueSubmit(vk_context->getObjectQueue(_ci_queue), _submits.size(), _submits.data(),
                              _ci_fence != UINT32_MAX ? vk_context->getObjectFence(_ci_fence) : VK_NULL_HANDLE);
            }
            // TODO: might pass more than 1 SubmitInfo, this overload always creates one, previous takes a vector of already created structs
            void queueSubmit(uint32_t _ci_queue,
                             const std::vector<VkPipelineStageFlags> &_wait_stages,
                             const std::vector<uint32_t> &_ci_command_buffers,
                             const std::vector<uint32_t> &_ci_wait_semaphores,
                             const std::vector<uint32_t> &_ci_signal_semaphores,
                             uint32_t _ci_fence = UINT32_MAX, void *_p_next = nullptr) {
                SubmitInfo::StructBuilder __b_info{vk_context->getObjectVecCommandBuffer(_ci_command_buffers),
                                                   vk_context->getObjectVecSemaphore(_ci_wait_semaphores),
                                                   vk_context->getObjectVecSemaphore(_ci_signal_semaphores)};
                __b_info.setWaitStages(_wait_stages);

                VkSubmitInfo __submit_info = __b_info.getVkStructView(_p_next);
                vkQueueSubmit(vk_context->getObjectQueue(_ci_queue), 1u, &__submit_info,
                              _ci_fence != UINT32_MAX ? vk_context->getObjectFence(_ci_fence) : VK_NULL_HANDLE);
            }

            // Returns next framebuffer's context index or UINT32_MAX when swapchain recreation is required (or in case of VK_SUBOPTIMAL_KHR)
            uint32_t acquireNextImageKHR(uint32_t _ci_semaphore = UINT32_MAX, uint32_t _ci_fence = UINT32_MAX, uint64_t _timeout = UINT64_MAX) {
                uint32_t __image_index = 0u;
                VkResult __func_res = vkAcquireNextImageKHR(vk_context->device, vk_context->swapchain_context.swapchain, _timeout,
                                                            _ci_semaphore != UINT32_MAX ? vk_context->getObjectSemaphore(_ci_semaphore) : VK_NULL_HANDLE,
                                                            _ci_fence != UINT32_MAX ? vk_context->getObjectFence(_ci_fence) : VK_NULL_HANDLE, &__image_index);
                if (__func_res != VK_SUCCESS) {
                    if ((__func_res == VK_ERROR_OUT_OF_DATE_KHR || __func_res == VK_SUBOPTIMAL_KHR) && vk_factory != nullptr) {
                        return UINT32_MAX;
                    }
                    else throw std::runtime_error(SVKFW_WRAPERR("VKFW :: VulkanContextFunc :: acquireNextImageKHR", "unhandled function error"));
                }
                return __image_index;
            }

            uint32_t queuePresentKHR(uint32_t _ci_queue, const std::vector<uint32_t> &_ci_wait_sem,
                                     const std::vector<uint32_t> &_image_indices, uint32_t _ci_fence = UINT32_MAX, void *_p_next = nullptr) {
                std::vector<VkSemaphore> __wait_sem = vk_context->getObjectVecSemaphore(_ci_wait_sem);
                VkSwapchainKHR __swapchain = vk_context->swapchain_context.swapchain;
                VkQueue __queue = vk_context->getObjectQueue(_ci_queue);
                VkResult __result = VK_SUCCESS;

                VkPresentInfoKHR __info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
                __info.pNext = _p_next;
                __info.waitSemaphoreCount = __wait_sem.size();
                __info.pWaitSemaphores = __wait_sem.data();
                __info.swapchainCount = 1u;
                __info.pSwapchains = &__swapchain;
                __info.pImageIndices = _image_indices.data();
                __info.pResults = &__result;

                uint32_t __func_res = vkQueuePresentKHR(__queue, &__info);
                if (__func_res != VK_SUCCESS) {
                    if ((__func_res == VK_ERROR_OUT_OF_DATE_KHR || __func_res == VK_SUBOPTIMAL_KHR) && vk_factory != nullptr) {
                        return UINT32_MAX;
                    }
                    else throw std::runtime_error(SVKFW_WRAPERR("VKFW :: VulkanContextFunc :: queuePresentKHR","unhandled function error"));
                }
                return 0u;
            }
            // Buffers are automatically bound after DeviceMemory allocation, so no need to call this function
            void bindBufferMemory(uint32_t _ci_buffer) {
                auto __buffer_bind_info = vk_factory->getContextDeviceMemory()->bBufferGetBindInfo(_ci_buffer);
                vkBindBufferMemory(vk_context->device, vk_context->getObjectBuffer(_ci_buffer),
                                   vk_context->getObjectDeviceMemory(__buffer_bind_info.ci_memory), __buffer_bind_info.offset);
            }
            // If 'size' is zero, maps memory from 'offset' to the end of buffer memory.
            void* mapMemory(uint32_t _ci_buffer_or_image, bool _is_image, VkDeviceSize _offset = 0ul, VkDeviceSize _size = 0ul, VkMemoryMapFlags _flags = 0u) {
                if (_is_image)
                    _ci_buffer_or_image |= Simple::VKFW::DeviceMemoryContext::BindInfo::LASTBIT_FLAG; // Flag used in BindInfo
                auto __buffer_bind_info = vk_factory->getContextDeviceMemory()->bBufferGetBindInfo(_ci_buffer_or_image);
                if (_offset + _size > __buffer_bind_info.size)
                    throw std::invalid_argument(SVKFW_WRAPERR("VKFW :: VulkanContextFunc :: mapBufferMemory", "'offset' + 'size' exceed buffer size"));
                if (_size == 0ul)
                    _size = __buffer_bind_info.size - _offset;
                
                void *__res = nullptr;
                vkMapMemory(vk_context->device, vk_context->getObjectDeviceMemory(__buffer_bind_info.ci_memory), __buffer_bind_info.offset + _offset, _size, _flags, &__res);
                return __res;
            }
            void unmapMemory(uint32_t _ci_buffer_or_image, bool _is_image) {
                if (_is_image)
                    _ci_buffer_or_image |= Simple::VKFW::DeviceMemoryContext::BindInfo::LASTBIT_FLAG; // Flag used in BindInfo
                uint32_t __ci_memory = vk_factory->getContextDeviceMemory()->bBufferGetBindInfo(_ci_buffer_or_image).ci_memory;
                vkUnmapMemory(vk_context->device, vk_context->getObjectDeviceMemory(__ci_memory));
            }

            void updateDescriptorSets(const std::vector<uint32_t> &_write_upd, const std::vector<uint32_t> &_copy_upd, void *_p_next = nullptr) {
                b_update_descriptor_sets->call(vk_context->device, _write_upd, _copy_upd, _p_next);
            }
            void updateDescriptorSets(void *_p_next = nullptr) {
                b_update_descriptor_sets->call(vk_context->device, _p_next);
            }
            void bUpdateDescrSetsWriteSetInfo(uint32_t _ci_sct_write_set, uint32_t _ci_descr_set,  VkDescriptorType _descriptor_type, uint32_t _dst_binding, uint32_t _descriptor_count = 0U, uint32_t _dst_arr_element = 0U) {
                b_update_descriptor_sets->getSetWrite(_ci_sct_write_set).setInfo(vk_context->getObjectDescriptorSet(_ci_descr_set), _descriptor_type, _dst_binding, _descriptor_count, _dst_arr_element);
            }
            void bUpdateDescrSetsWriteAddBufferInfo(uint32_t _ci_sct_write_set, uint32_t _ci_buffer, VkDeviceSize _offset = 0ul, VkDeviceSize _range = VK_WHOLE_SIZE) {
                b_update_descriptor_sets->getSetWrite(_ci_sct_write_set).addBufferInfo(vk_context->getObjectBuffer(_ci_buffer), _offset, _range);
            }
            // TODO:
            // void bUpdateDescriptorSetsWriteSetAddImageInfo() {

            // }


        // Commands

            void cmdBeginRenderPass(uint32_t _ci_command_buffer, uint32_t _ci_framebuffer,
                                    VkSubpassContents _contents, void *_p_next = nullptr) {
                b_begin_render_pass->cmdCall(vk_context->getObjectCommandBuffer(_ci_command_buffer), vk_context->render_pass,
                                             vk_context->getObjectFramebuffer(_ci_framebuffer), _contents, _p_next);
            }

            void cmdBindPipeline(uint32_t _ci_command_buffer, VkPipelineBindPoint _bind_point) {
                vkCmdBindPipeline(vk_context->getObjectCommandBuffer(_ci_command_buffer), _bind_point, vk_context->pipeline_context.pipeline);
            }

            void cmdBindVertexBuffers(uint32_t _ci_command_buffer, uint32_t _binding_first, const std::vector<uint32_t> &_ci_buffers) {
                std::vector<VkBuffer> __buffers = vk_context->getObjectVecBuffer(_ci_buffers);
                std::vector<VkDeviceSize> __offsets;

                __offsets.resize(_ci_buffers.size());
                for (uint32_t i = 0; i < _ci_buffers.size(); ++i)
                    __offsets[i] = vk_factory->getContextDeviceMemory()->bBufferGetBindInfo(_ci_buffers[i]).offset;

                vkCmdBindVertexBuffers(vk_context->getObjectCommandBuffer(_ci_command_buffer), _binding_first, (uint32_t)__buffers.size(), __buffers.data(), __offsets.data());
            }

            void cmdBindIndexBuffer(uint32_t _ci_command_buffer, uint32_t _ci_buffer, VkIndexType _index_type) {
                vkCmdBindIndexBuffer(vk_context->getObjectCommandBuffer(_ci_command_buffer), vk_context->getObjectBuffer(_ci_buffer),
                                     vk_factory->getContextDeviceMemory()->bBufferGetBindInfo(_ci_buffer).offset, _index_type);
            }

            void cmdBindDescriptorSets(uint32_t _ci_command_buffer, VkPipelineBindPoint _pip_bind_point, uint32_t _first_set, const std::vector<uint32_t> &_ci_descr_set, const std::vector<uint32_t> &_ci_descr = {}) {
                std::vector<VkDescriptorSet> __descr_sets{vk_context->getObjectVecDescriptorSet(_ci_descr_set)};
                vkCmdBindDescriptorSets(vk_context->getObjectCommandBuffer(_ci_command_buffer), _pip_bind_point, vk_context->pipeline_context.layout, _first_set, __descr_sets.size(), __descr_sets.data(), _ci_descr.size(), _ci_descr.data());
            }

            void cmdSetViewport(uint32_t _ci_command_buffer, const std::vector<VkViewport> &_viewports,
                                uint32_t _offset = 0u, uint32_t _count = 0u) {
                if (_count == 0u)
                    _count = _viewports.size() - _offset;
                vkCmdSetViewport(vk_context->getObjectCommandBuffer(_ci_command_buffer), _offset, _count, _viewports.data());
            }
            // This overload sets viewport from swapchain_extent
            void cmdSetViewport(uint32_t _ci_command_buffer) {
                Viewport::StructWrap __viewport{vk_factory->getContextSwapchain()->getBuilderSwapchainKHR()->image_info.extent};
                vkCmdSetViewport(vk_context->getObjectCommandBuffer(_ci_command_buffer), 0u, 1u, &__viewport.viewport);
            }

            void cmdSetScissor(uint32_t _ci_command_buffer, const std::vector<VkRect2D> &_scissors,
                                uint32_t _offset = 0u, uint32_t _count = 0u) {
                if (_count == 0u)
                    _count = _scissors.size() - _offset;
                vkCmdSetScissor(vk_context->getObjectCommandBuffer(_ci_command_buffer), _offset, _count, _scissors.data());
            }
            // This overload sets scissor from swapchain_extent
            void cmdSetScissor(uint32_t _ci_command_buffer) {
                Rect2D::StructWrap __scissor{vk_factory->getContextSwapchain()->getBuilderSwapchainKHR()->image_info.extent};
                vkCmdSetScissor(vk_context->getObjectCommandBuffer(_ci_command_buffer), 0u, 1u, &__scissor.rectangle);
            }

            void cmdDraw(uint32_t _ci_command_buffer, uint32_t _vert_count, uint32_t _inst_count,
                         uint32_t _vert_offset = 0u, uint32_t _inst_offset = 0u) {
                vkCmdDraw(vk_context->getObjectCommandBuffer(_ci_command_buffer), _vert_count, _inst_count, _vert_offset, _inst_offset);
            }

            void cmdDrawIndexed(uint32_t _ci_command_buffer, uint32_t _index_count, uint32_t _inst_count, uint32_t _first_index = 0u, int32_t _vert_offset = 0u, uint32_t _first_inst = 0u) {
                vkCmdDrawIndexed(vk_context->getObjectCommandBuffer(_ci_command_buffer), _index_count, _inst_count, _first_index, _vert_offset, _first_inst);
            }

            void cmdEndRenderPass(uint32_t _ci_command_buffer) {
                vkCmdEndRenderPass(vk_context->getObjectCommandBuffer(_ci_command_buffer));
            }

            void cmdCopyBuffer(uint32_t _ci_command_buffer, uint32_t _ci_buffer_src, uint32_t _ci_buffer_dst, const std::vector<VkBufferCopy> &_copy_regions) {
                vkCmdCopyBuffer(vk_context->getObjectCommandBuffer(_ci_command_buffer), vk_context->getObjectBuffer(_ci_buffer_src),
                                vk_context->getObjectBuffer(_ci_buffer_dst), _copy_regions.size(), _copy_regions.data());                
            }

            // By default, one region with the size of min(src buffer, dst buffer)
            // is copied to the beginning of dst buffer. It can be done by passing size=0.
            void cmdCopyBuffer(uint32_t _ci_command_buffer, uint32_t _ci_buffer_src, uint32_t _ci_buffer_dst, VkDeviceSize _size = 0ul, uint32_t _offset_src = 0u, uint32_t _offset_dst = 0u) {
                if (_size == 0u) {
                    _size = std::min(vk_factory->getContextDeviceMemory()->bBufferGetBindInfo(_ci_buffer_src).size,
                                     vk_factory->getContextDeviceMemory()->bBufferGetBindInfo(_ci_buffer_dst).size);
                }
                VkBufferCopy __region_info{ _offset_src, _offset_dst, _size };

                vkCmdCopyBuffer(vk_context->getObjectCommandBuffer(_ci_command_buffer),
                                vk_context->getObjectBuffer(_ci_buffer_src),
                                vk_context->getObjectBuffer(_ci_buffer_dst), 1u, &__region_info);
            }


        // Scenarios

            // Sets current swapchain extent (and zero offset) as render area (so that it matches the framebuffer)
            void bCmdBeginRenderPassSetRenderAreaFull() {
                b_begin_render_pass->setRenderArea(vk_factory->getContextSwapchain()->getBuilderSwapchainKHR()->image_info.extent);
            }
        } Vulkan_global_context_func; // VulkanContextFunc END


        void initContext(VulkanContext* _vk_context = &Vulkan_global_context, VulkanContextFactory *_vk_factory = &Vulkan_global_context_factory, VulkanContextFunc *_vk_func = &Vulkan_global_context_func) {
            if (_vk_factory != nullptr)
                _vk_factory->setContext(_vk_context);
            if (_vk_func != nullptr)
                _vk_func->setFactoryContext(_vk_context, _vk_factory);
        }


// ResultHandler::handle definition

        void ResultHandler::handle(VkResult _result, VulkanContextFactory *_vk_factory) {
            // TODO:
        }
    }; // VKFW END
}; // Simple END

#endif