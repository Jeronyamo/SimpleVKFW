#ifndef SVKFW_VKFW_H
#define SVKFW_VKFW_H

#include "vkfw_wrap.h"


namespace Simple {
    namespace VKFW {

//  == === ==== ====================== ==== === ==  \\
                    Vulkan Context
//  == === ==== ====================== ==== === ==  \\

        struct VulkanContext {
            struct ContextDebug {
                VulkanContext *cxt_ptr = nullptr;

                Object::DebugUtilsMessengerEXT debug_messenger;


                ContextDebug(VulkanContext* _cxt_ptr) : cxt_ptr{_cxt_ptr} {}
               ~ContextDebug() {}

               ContextDebug& operator=(VkResult _vk_func_result) { /* TODO: */ return *this; }
            }; // ContextDebug END

            struct ContextCore {
                Object::Instance instance;
                Object::PhysicalDevice phys_device;
                Object::Device device; // TODO: Explore DeviceGroup for multiple GPUs support
                std::vector<Object::QueueFamily> queue_families; // TODO: USE ONE QUEUE_FAMILY WRAP! DATA IS DUPLICATED
                std::vector<Object::Queue>       queues;
                Object::RenderPass render_pass;

                Struct::StructWrap<VkAllocationCallbacks> alloc_cb;
                ContextDebug cxt_debug;

                ContextCore(VulkanContext* _cxt_ptr) : cxt_debug{_cxt_ptr} {
                    SVKFW_ASSERT(volkInitialize() == VK_SUCCESS, std::runtime_error, "VKFW::VulkanContext :: ContextCore constructor",
                                    "volk initialization failed");
                }
               ~ContextCore() { destroyContext(); }

                void destroyContext() {
                    destroyRenderPass();
                    destroyDevice();
                    destroyPhysicalDevice();
                    destroyDebugUtilsMessenger();
                    destroyInstance();
                }

            // Manage Objects
                // Automatically adds a queue vector handler of the added queue family
                ContextIndex addQueueFamily() { queue_families.emplace_back(); queues.emplace_back(); return queue_families.size()-1; }

                void createInstance() {
                    instance.create_info.updateWrap();
                    instance.createObject(alloc_cb.wrap);
                }
                void createDebugUtilsMessenger() {
                    cxt_debug.debug_messenger.create_info.updateWrap();
                    cxt_debug.debug_messenger.createObject(instance.handle, alloc_cb.wrap);
                }
                void loadPhysicalDeviceInfo() {
                    phys_device.select_info.loadInfo(instance.handle);
                }
                void findPhysicalDevice() {
                    phys_device.createObject();
                }
                // TODO: USE ONE QUEUE_FAMILY WRAP! DATA IS DUPLICATED
                void loadQueueFamiliesInfo() {
                    for (auto &qfam : queue_families)
                        qfam.select_info.loadInfo(instance.handle, phys_device.handle);
                }
                void findQueueFamilies() {
                    for (ContextIndex i = 0u; i < queue_families.size(); ++i)
                        queue_families[i].createObject();
                }
                void createDevice() {
                    device.create_info.updateWrap();
                    device.createObject(phys_device.handle, alloc_cb.wrap);
                }
                void acquireQueues() {
                    for (ContextIndex i = 0u; i < queues.size(); ++i)
                        // queues[i].create_info.updateWrap(); // not needed; queues are acquired from the device
                        queues[i].createObject(device.handle);
                }
                void createRenderPass() {
                    render_pass.create_info.updateWrap();
                    render_pass.createObject(device.handle, alloc_cb.wrap);
                }


                void destroyInstance() {
                    instance.destroyObject(alloc_cb.wrap);
                }
                void destroyDebugUtilsMessenger() {
                    cxt_debug.debug_messenger.destroyObject(instance.handle, alloc_cb.wrap);
                }
                void destroyPhysicalDevice() {
                    phys_device.destroyObject();
                }
                void destroyDevice() {
                    device.destroyObject(alloc_cb.wrap);
                }
                void destroyRenderPass() {
                    render_pass.destroyObject(device.handle, alloc_cb.wrap);
                }

            // Scenarios

                ContextIndex addSctRenderPassAttachment        () { return render_pass.create_info.addAttachment        (); }
                ContextIndex addSctRenderPassSubpassDescription() { return render_pass.create_info.addSubpassDescription(); }
                ContextIndex addSctRenderPassSubpassDependency () { return render_pass.create_info.addSubpassDependency (); }

                void setDefaultApplicationInfo() {
                    instance.create_info.app_info_wrap.setWholeStruct();
                }

                void activateDebug() {
                    instance.create_info.addExtDebugUtils();
                    instance.create_info.addValidationLayer();
                    if (VulkanAPIVersion::join(instance.create_info.app_info_wrap.api_version["xyz"],
                                               instance.create_info.app_info_wrap.api_version.w) <= VK_API_VERSION_1_0)
                        device.create_info.addValidationLayer(); // physical device isn't found at this point
                }

                void setQueueFamilyQueueCount(const ContextIndex& _ci_queue_family, uint32_t _queue_count, const std::vector<float> &_priorities = {1.0f}) {
                    queues[_ci_queue_family].create_info.queue_family = queue_families[_ci_queue_family].handle;
                    queues[_ci_queue_family].create_info.setQueueCount(_queue_count, _priorities);
                    queues[_ci_queue_family].create_info.updateWrap();
                }

                void setDeviceQueueInfo() {
                    device.create_info.queue_info.resize(queue_families.size());
                    for (ContextIndex i = 0u; i < queue_families.size(); ++i)
                        device.create_info.queue_info[i] = queues[i].create_info.wrap;
                }
            } cxt_core; // ContextCore END

            struct ContextSwapchain {
                Object::SwapchainKHR swapchain;
                Object::  SurfaceKHR   surface;
                std::vector<VkImageView> image_views;
                std::vector<VkFramebuffer> framebuffers;
                GLFWwindow *window = nullptr;

                Struct::StructWrap<  VkImageViewCreateInfo>  image_view_wrap;
                Struct::StructWrap<VkFramebufferCreateInfo> framebuffer_wrap;

                ContextCore *cxt_core = nullptr;

                ContextSwapchain(ContextCore *_cxt_core) { cxt_core = _cxt_core; }
               ~ContextSwapchain() { destroyContext(); }

                void destroyContext() {
                    destroySwapchainWithDependencies();
                    destroySurface();
                }

            // Manage Objects

                void createSurface() {
                    surface.createObject(cxt_core->instance.handle, window, cxt_core->alloc_cb.wrap);
                }

                void createSwapchain() {
                    swapchain.create_info.updateWrap();
                    swapchain.create_info.wrap.surface = surface.handle;
                    swapchain.createObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                    image_view_wrap.wrap.format = swapchain.create_info.wrap.imageFormat;
                    framebuffer_wrap.setBufferSize(swapchain.create_info.wrap.imageExtent);
                }
                void createImageViews() {
                    std::vector<VkImage> __swapchain_images{Func::GetSwapchainImagesKHR(cxt_core->device.handle, swapchain.handle)};

                    image_views.resize(__swapchain_images.size());
                    for (ContextIndex ci = 0u; ci < __swapchain_images.size(); ++ci) {
                        image_view_wrap.wrap.image = __swapchain_images[ci];
                        image_views[ci] = Func::CreateImageView(&image_view_wrap.wrap, cxt_core->device.handle, __swapchain_images[ci], cxt_core->alloc_cb.wrap);
                    }
                }
                void createFramebuffers() {
                    framebuffers.resize(image_views.size());
                    framebuffer_wrap.wrap.renderPass = cxt_core->render_pass.handle;

                    for (ContextIndex ci = 0u; ci < framebuffers.size(); ++ci) {
                        framebuffer_wrap.attachments = {image_views[ci]};
                        framebuffer_wrap.updateWrap();
                        framebuffers[ci] = Func::CreateFramebuffer(&framebuffer_wrap.wrap, cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                    }
                }


                void destroySurface() {
                    surface.destroyObject(cxt_core->instance.handle, cxt_core->alloc_cb.wrap);
                }
                void destroySwapchain() {
                    swapchain.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }
                void destroyImageViews() {
                    for (auto    img_view :  image_views)
                        vkDestroyImageView(cxt_core->device.handle, img_view, cxt_core->alloc_cb.wrap);
                }
                void destroyFramebuffers() {
                    for (auto framebuffer : framebuffers)
                        vkDestroyFramebuffer(cxt_core->device.handle, framebuffer, cxt_core->alloc_cb.wrap);
                }

            // Scenarios

                void setWindow(const Window &_window) {
                    window = _window.window;
                }

                void  createSwapchainWithDependencies() {
                    createSwapchain();
                    createImageViews();
                    createFramebuffers();
                }

                void destroySwapchainWithDependencies() {
                    destroyFramebuffers();
                    destroyImageViews();
                    destroySwapchain();
                }

                void pickSwapchainImageFormat(const Util::SurfaceFormatTestFunc _test_func = Util::SelectSurfaceFormat::DefaultFormatTest) {
                    swapchain.create_info.pickFormat(cxt_core->phys_device.handle, surface.handle, _test_func);
                }

                void setSwapchainExtentFromWindow() {
                    swapchain.create_info.chooseCurrentExtent(window);
                }

                void setSwapchainSurfaceCapabilities() {
                    swapchain.create_info.setSurfaceCapabilities(cxt_core->phys_device.handle, surface.handle);
                }

                void setRenderPassAttachmentImageFormat(ContextIndex _attachment_ci) {
                    cxt_core->render_pass.create_info.attachments_wrap[_attachment_ci].wrap.format = swapchain.create_info.wrap.imageFormat;
                }

                void recreate() {
                    destroySwapchainWithDependencies();
                    setSwapchainSurfaceCapabilities();
                    while (swapchain.create_info.capabilities.currentExtent.width  == 0 ||
                           swapchain.create_info.capabilities.currentExtent.height == 0) {
                        glfwWaitEvents();
                        setSwapchainSurfaceCapabilities();
                    }
                    setSwapchainExtentFromWindow();
                    createSwapchainWithDependencies();
                }
            } cxt_swapchain; // ContextSwapchain END

            struct ContextDescriptor {
                std::vector<Object::DescriptorSetLayout> layouts;
                std::vector<Object::DescriptorPool>        pools;
                std::vector<Object::DescriptorSet>          sets;
                // Note: '1' Pool AND 'N' Layouts --> 'N' Sets

                ContextCore *cxt_core = nullptr;

                ContextDescriptor(ContextCore *_cxt_core) { cxt_core = _cxt_core; }
               ~ContextDescriptor() { destroyContext(); }

                // Note: Only Core and Swapchain contexts' logic allows creation of all objects at once
                void  createContext() {}

                void destroyContext() {
                    // Note: sets can be ignored, they are freed when layouts are destroyed
                    for (ContextIndex ci = 0u; ci < pools.size(); ++ci)
                        pools  [ci].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                    for (ContextIndex ci = 0u; ci < layouts.size(); ++ci)
                        layouts[ci].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }

            // Manage Objects

                ContextIndex addLayout() { layouts.emplace_back(); return layouts.size()-1; }
                ContextIndex addPool  () {   pools.emplace_back(); return   pools.size()-1; }
                ContextIndex addSet   () {    sets.emplace_back(); return    sets.size()-1; }

                void createLayout(ContextIndex _layout_ci) {
                    layouts[_layout_ci].create_info.updateWrap();
                    layouts[_layout_ci].createObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }
                void createPool(ContextIndex _pool_ci) {
                    pools[_pool_ci].create_info.updateWrap();
                    pools[_pool_ci].createObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }
                void createSets(ContextIndex _set_ci) {
                    sets[_set_ci].alloc_info.updateWrap();
                    sets[_set_ci].createObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }


                void destroyLayout(ContextIndex _layout_ci) {
                    layouts[_layout_ci].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }
                void destroyPool(ContextIndex _pool_ci) {
                    pools[_pool_ci].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }
                void destroySets(ContextIndex _set_ci) {
                    sets[_set_ci].destroyObject(cxt_core->device.handle);
                }

            // Scenarios

                ContextIndex2 addSctLayoutBinding(ContextIndex _layout_ci) { return {_layout_ci, layouts[_layout_ci].create_info.addBinding()}; }

                void setAllForSet(ContextIndex _set_ci, ContextIndex _pool_ci, const std::vector<ContextIndex> &_layout_cis) {
                    std::vector<VkDescriptorSetLayout> __layouts(_layout_cis.size());
                    for (ContextIndex ci = 0u; ci < _layout_cis.size(); ++ci)
                        __layouts[ci] = layouts[_layout_cis[ci]].handle;

                    sets[_set_ci].alloc_info.setWholeStruct(pools[_pool_ci].handle, __layouts);
                }

                void setPoolForSet(ContextIndex _set_ci, ContextIndex _pool_ci) {
                    sets[_set_ci].alloc_info.wrap.descriptorPool = pools[_pool_ci].handle;
                }

                void setLayoutsForSet(ContextIndex _set_ci, const std::vector<ContextIndex> &_layout_cis) {
                    std::vector<VkDescriptorSetLayout> __layouts(_layout_cis.size());
                    for (ContextIndex ci = 0u; ci < _layout_cis.size(); ++ci)
                        __layouts[ci] = layouts[_layout_cis[ci]].handle;

                    sets[_set_ci].alloc_info.set_layouts = __layouts;
                    sets[_set_ci].alloc_info.updateWrap();
                }
            } cxt_descriptor; // ContextDescriptors END

            struct ContextPipeline {
                std::vector<Object::Pipeline>     pipelines;
                std::vector<Object::PipelineLayout> layouts;
                std::vector<Object::PipelineCache>   caches;
                std::vector<Object::ShaderModule>   shaders;

                ContextCore *cxt_core = nullptr;

                ContextPipeline(ContextCore *_cxt_core) { cxt_core = _cxt_core; }
               ~ContextPipeline() { destroyContext(); }

                // Note: Only Core and Swapchain contexts' logic allows creation of all objects at once
                void  createContext() {}

                void destroyContext() {
                    for (auto& pipeline : pipelines)
                        pipeline.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                    for (auto& layout   :   layouts)
                        layout  .destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                    for (auto& cache    :    caches)
                        cache   .destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                    for (auto& shader   :   shaders)
                        shader  .destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }

            // Manage Objects

                ContextIndex addPipeline() { pipelines.emplace_back(); return pipelines.size() - 1; }
                ContextIndex addShader  () {   shaders.emplace_back(); return   shaders.size() - 1; }
                ContextIndex addLayout  () {   layouts.emplace_back(); return   layouts.size() - 1; }
                ContextIndex addCache   () {    caches.emplace_back(); return    caches.size() - 1; }

                void createPipeline(ContextIndex _pipeline_ci, uint32_t _subpass, ContextIndex _layout_ci, ContextIndex _cache_ci = VKFW_CI_IGNORE,
                                    VkDeferredOperationKHR _deferred_op = VK_NULL_HANDLE) {
                    switch (pipelines[_pipeline_ci].pipeline_type) {
                        case VKFW_PIPELINE_TYPE_GRAPHICS:
                            pipelines[_pipeline_ci].  graphics_create_info_wrap.wrap.renderPass = cxt_core->render_pass.handle;
                            pipelines[_pipeline_ci].  graphics_create_info_wrap.wrap.layout = layouts[_layout_ci].handle;
                            pipelines[_pipeline_ci].  graphics_create_info_wrap.updateWrap(); break;
                        case VKFW_PIPELINE_TYPE_COMPUTE:
                            pipelines[_pipeline_ci].   compute_create_info_wrap.wrap.layout = layouts[_layout_ci].handle;
                            pipelines[_pipeline_ci].   compute_create_info_wrap.updateWrap(); break;
                        case VKFW_PIPELINE_TYPE_RAY_TRACING:
                            pipelines[_pipeline_ci].raytracing_create_info_wrap.wrap.layout = layouts[_layout_ci].handle;
                            pipelines[_pipeline_ci].raytracing_create_info_wrap.updateWrap(); break;
                    }
                    VkPipelineCache __pipeline_cache = _cache_ci != VKFW_CI_IGNORE ? caches[_cache_ci].handle : VK_NULL_HANDLE;
                    pipelines[_pipeline_ci].createObject(cxt_core->device.handle, cxt_core->render_pass.handle, _subpass,
                                                         _deferred_op, __pipeline_cache, cxt_core->alloc_cb.wrap);
                }
                void createCache (ContextIndex _cache_ci) {
                    caches [ _cache_ci].create_info.updateWrap();
                    caches [ _cache_ci].createObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }
                void createLayout(ContextIndex _layout_ci) {
                    layouts[_layout_ci].create_info.updateWrap();
                    layouts[_layout_ci].createObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }
                void createShader(ContextIndex _ci_shader) {
                    shaders[_ci_shader].create_info.updateWrap();
                    shaders[_ci_shader].createObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }


                void destroyPipeline(ContextIndex _pipeline_ci) {
                    pipelines[_pipeline_ci].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }
                void destroyCache   (ContextIndex    _cache_ci) {
                    caches      [_cache_ci].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }
                void destroyLayout  (ContextIndex   _layout_ci) {
                    layouts    [_layout_ci].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }
                void destroyShader  (ContextIndex   _ci_shader) {
                    shaders    [_ci_shader].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }

            // Scenarios

                void setLayoutDescriptorSetLayouts(ContextIndex _ci_layout, const std::vector<ContextIndex> &_ci_descr_set_layouts) {
                    layouts[_ci_layout].create_info.layouts.resize(_ci_descr_set_layouts.size(), VK_NULL_HANDLE);
                    for (uint32_t i = 0u; i < _ci_descr_set_layouts.size(); ++i)
                        layouts[_ci_layout].create_info.layouts[i] = cxt_core->cxt_debug.cxt_ptr->cxt_descriptor.layouts[_ci_descr_set_layouts[0]].handle;
                }

                void addLayoutPushConstant(ContextIndex _ci_layout, VkShaderStageFlags _stage_flags, uint32_t _offset, uint32_t _size) {
                    layouts[_ci_layout].create_info.constants.push_back(VkPushConstantRange{_stage_flags, _offset, _size});
                }

                ContextIndex2 addShaderStage(ContextIndex _ci_pipeline) {
                    return { _ci_pipeline, pipelines[_ci_pipeline].addShaderStage() };
                }
            } cxt_pipeline; // ContextPipeline END

            struct ContextCommand {
                std::vector<Object::CommandPool>   cmd_pools;
                std::vector<Object::CommandBuffer> cmd_buffers;

                ContextCore *cxt_core = nullptr;

                ContextCommand(ContextCore *_cxt_core) { cxt_core = _cxt_core; }
               ~ContextCommand() { destroyContext(); }

                // Note: Only Core and Swapchain contexts' logic allows creation of all objects at once
                void  createContext() {}

                void destroyContext() {
                    for (auto& cmd_buff : cmd_buffers)
                        cmd_buff.destroyObject(cxt_core->device.handle);
                    for (auto& cmd_pool :   cmd_pools)
                        cmd_pool.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }

            // Manage Objects

                ContextIndex addCmdPool  () {   cmd_pools.emplace_back(); return   cmd_pools.size() - 1; }
                ContextIndex addCmdBuffer() { cmd_buffers.emplace_back(); return cmd_buffers.size() - 1; }

                void createCmdPool(ContextIndex _cmd_pool_ci) {
                    cmd_pools[_cmd_pool_ci].create_info.updateWrap();
                    cmd_pools[_cmd_pool_ci].createObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }
                void createCmdBuffers(ContextIndex _cmd_buffer_ci) {
                    cmd_buffers[_cmd_buffer_ci].create_info.updateWrap();
                    cmd_buffers[_cmd_buffer_ci].createObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                }

            // Scenarios

                void createCmdPool(ContextIndex _cmd_pool_ci, ContextIndex _queue_family_ci, VkCommandPoolCreateFlags _flags = 0u) {
                    cmd_pools[_cmd_pool_ci].create_info.updateWrap();
                    cmd_pools[_cmd_pool_ci].createObject(cxt_core->device.handle, cxt_core->queue_families[_queue_family_ci].handle, _flags, cxt_core->alloc_cb.wrap);
                }

                void createCmdBuffersPrimary(ContextIndex _cmd_buffer_ci, ContextIndex _cmd_pool_ci, uint32_t _cmd_buffers_count) {
                    cmd_buffers[_cmd_buffer_ci].create_info.updateWrap();
                    cmd_buffers[_cmd_buffer_ci].createPrimary(cxt_core->device.handle, cmd_pools[_cmd_pool_ci].handle, _cmd_buffers_count, cxt_core->alloc_cb.wrap);
                }
                void createCmdBuffersSecondary(ContextIndex _cmd_buffer_ci, ContextIndex _cmd_pool_ci, uint32_t _cmd_buffers_count) {
                    cmd_buffers[_cmd_buffer_ci].create_info.updateWrap();
                    cmd_buffers[_cmd_buffer_ci].createSecondary(cxt_core->device.handle, cmd_pools[_cmd_pool_ci].handle, _cmd_buffers_count, cxt_core->alloc_cb.wrap);
                }
            } cxt_command; // ContextCommand END

            struct ContextMemory {
                struct MemoryMapping {
                    VkDeviceSize offset; // offset in the allocated device memory
                    VkDeviceSize  range; // required device memory size in bytes
                };

                struct MemoryChunk {
                    Object::DeviceMemory memory;
                    std::vector<Object::Buffer> buffers;
                    std::vector<Object::Image>   images;
                    std::vector<Object::BufferView> buffer_views; // Note: Only used in vkUpdateDescriptorSets()
                    std::vector<Object::ImageView>   image_views;

                    VkMemoryPropertyFlags required_flags; // Flags required by user for this memory chunk
                    std::vector<MemoryMapping> buffer_maps;
                    std::vector<MemoryMapping>  image_maps;
                    // 48 bytes left (2 vectors) for 256 bytes per chunk

                    ContextCore *cxt_core = nullptr;

                    MemoryChunk(ContextCore *_cxt_core) { cxt_core = _cxt_core; }
                   ~MemoryChunk() {}

                    void destroyChunk() {
                        for (auto& buf_view : buffer_views)
                            buf_view.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                        for (auto& img_view :  image_views)
                            img_view.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                        for (auto& buf : buffers)
                            buf.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                        for (auto& img :  images)
                            img.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);

                        memory.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                    }

                    void allocateChunkMemory(VkPhysicalDeviceMemoryProperties &_allowed_size) {
                        VkMemoryRequirements __mem_req{ 0u, 0u, UINT32_MAX }, __mem_req_i{};

                        // Compute all buffer offsets
                        for (uint32_t ci = 0u; ci < buffers.size(); ++ci) {
                            __mem_req_i = Func::GetBufferMemoryRequirements(cxt_core->device.handle, buffers[ci].handle);
                            SVKFW_ASSERT(__mem_req.memoryTypeBits & __mem_req_i.memoryTypeBits, std::runtime_error, "VKFW :: ContextMemory :: allocateChunkMemory",
                                            "No suitable memory type left for buffer " + std::to_string(ci));

                            // printf("Buffer %d alignment: %d, CPU size: %d, GPU size: %d\n", ci, __mem_req_i.alignment, buffer_maps[ci].range, __mem_req_i.size);
                            __mem_req.memoryTypeBits &= __mem_req_i.memoryTypeBits;
                            uint32_t __additional_offset = (__mem_req_i.alignment - (__mem_req.size % __mem_req_i.alignment)) % __mem_req_i.alignment;
                            __mem_req.size += __additional_offset;
                            buffer_maps[ci].offset = __mem_req.size;

                            // Note: from VkMemoryRequirements description, I believe this offset is always 0
                            // __additional_offset = (__mem_req_i.alignment - (buffer_maps[ci].range % __mem_req_i.alignment)) % __mem_req_i.alignment;
                            __mem_req.size += buffer_maps[ci].range;
                        }
                        // Align last buffer
                        if (!buffers.empty()) {
                            uint32_t __additional_offset = (__mem_req_i.alignment - (__mem_req.size % __mem_req_i.alignment)) % __mem_req_i.alignment;
                            __mem_req.size += __additional_offset;
                        }

                        // Compute all image offsets
                        for (uint32_t ci = 0u; ci <  images.size(); ++ci) {
                            __mem_req_i = Func::GetImageMemoryRequirements(cxt_core->device.handle, images[ci].handle);
                            SVKFW_ASSERT(__mem_req.memoryTypeBits & __mem_req_i.memoryTypeBits, std::runtime_error, "VKFW :: ContextMemory :: allocateChunkMemory",
                                            "No suitable memory type left for image " + std::to_string(ci));

                            __mem_req.memoryTypeBits &= __mem_req_i.memoryTypeBits;
                            uint32_t __additional_offset = (__mem_req_i.alignment - (__mem_req.size % __mem_req_i.alignment)) % __mem_req_i.alignment;
                            __mem_req.size += __additional_offset;
                            image_maps[ci].offset = __mem_req.size;

                            // Note: from VkMemoryRequirements description, I believe this offset is always 0
                            // __additional_offset = (__mem_req_i.alignment - (image_maps[ci].range % __mem_req_i.alignment)) % __mem_req_i.alignment;
                            __mem_req.size += buffer_maps[ci].range;
                        }
                        // Align last image
                        if (!images.empty()) {
                            uint32_t __additional_offset = (__mem_req_i.alignment - (__mem_req.size % __mem_req_i.alignment)) % __mem_req_i.alignment;
                            __mem_req.size += __additional_offset;
                        }

                        uint32_t __supported_mem_type_index = VK_MAX_MEMORY_TYPES;
                        for (uint32_t i = 0u; i < _allowed_size.memoryTypeCount; ++i) {
                            bool __use_memtype = true;
                            __use_memtype &= (__mem_req.memoryTypeBits & (1 << i)) != 0u; // can represent
                            __use_memtype &= (_allowed_size.memoryTypes[i].propertyFlags & required_flags) == required_flags; // complies the user requirements
                            __use_memtype &= __mem_req.size <= _allowed_size.memoryHeaps[_allowed_size.memoryTypes[i].heapIndex].size; // can fit

                            if (__use_memtype) {
                                __supported_mem_type_index = i;
                                break;
                            }
                        }

                        SVKFW_ASSERT(__supported_mem_type_index < VK_MAX_MEMORY_TYPES, std::runtime_error, "VKFW :: ContextMemory :: allocateChunkMemory",
                                        "No device memory types to represent and fit buffers/images");

                        _allowed_size.memoryHeaps[_allowed_size.memoryTypes[__supported_mem_type_index].heapIndex].size -= __mem_req.size;
                        memory.allocate_info.wrap.memoryTypeIndex = __supported_mem_type_index;
                        memory.allocate_info.wrap.allocationSize  = __mem_req.size;
                        memory.createObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);

                        // Bind all buffers to created memory
                        VkResult __func_res = VK_SUCCESS;
                        for (ContextIndex ci = 0u; ci < buffers.size(); ++ci) {
                            __func_res = vkBindBufferMemory(cxt_core->device.handle, buffers[ci].handle, memory.handle, buffer_maps[ci].offset);
                            SVKFW_ASSERT(__func_res == VK_SUCCESS, std::runtime_error, "VKFW::MemoryChunk", "bind buffer error, code %d" + std::to_string(__func_res));
                        }

                        // Bind all images to created memory
                        for (ContextIndex ci = 0u; ci < images.size(); ++ci) {
                            __func_res = vkBindImageMemory(cxt_core->device.handle, images[ci].handle, memory.handle, image_maps[ci].offset);
                            SVKFW_ASSERT(__func_res == VK_SUCCESS, std::runtime_error, "VKFW::MemoryChunk", "bind image error, code %d" + std::to_string(__func_res));
                        }
                    }

                // Manage Objects

                    ContextIndex addBuffer(uint32_t _buf_size = 0u) { buffers.emplace_back(); buffer_maps.push_back(MemoryMapping{0u, _buf_size}); return buffers.size() - 1; }
                    ContextIndex addImage (uint32_t _img_size = 0u) {  images.emplace_back();  image_maps.push_back(MemoryMapping{0u, _img_size}); return  images.size() - 1; }
                    ContextIndex addBufferView() { buffer_views.emplace_back(); return buffer_views.size() - 1; }
                    ContextIndex addImageView () {  image_views.emplace_back(); return  image_views.size() - 1; }

                    void createBuffer(ContextIndex _buffer_ci) {
                        buffers[_buffer_ci].create_info.updateWrap();
                        buffers[_buffer_ci].createObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                    }
                    void createImage (ContextIndex _image_ci) {
                        images [ _image_ci].create_info.updateWrap();
                        images [ _image_ci].createObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                    }
                    void createBufferView(ContextIndex _buffer_view_ci, ContextIndex _buffer_ci) {
                        buffer_views[_buffer_view_ci].create_info.updateWrap();
                        buffer_views[_buffer_view_ci].createObject(cxt_core->device.handle, buffers[_buffer_ci].handle, cxt_core->alloc_cb.wrap);
                    }
                    void createImageView (ContextIndex  _image_view_ci, ContextIndex  _image_ci) {
                        image_views [ _image_view_ci].create_info.updateWrap();
                        image_views [ _image_view_ci].createObject(cxt_core->device.handle,  images[ _image_ci].handle, cxt_core->alloc_cb.wrap);
                    }

                    void destroyBuffer(ContextIndex _buffer_ci) {
                        buffers[_buffer_ci].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                    }
                    void destroyImage (ContextIndex _image_ci) {
                        images [ _image_ci].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                    }
                    void destroyBufferView(ContextIndex _buffer_view_ci) {
                        buffer_views[_buffer_view_ci].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                    }
                    void destroyImageView (ContextIndex  _image_view_ci) {
                        image_views [ _image_view_ci].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.wrap);
                    }
                }; // MemoryChunk END
                std::vector<MemoryChunk> memory_chunks;

                Struct::StructWrap<VkPhysicalDeviceMemoryProperties2> memory_properties;

                ContextCore *cxt_core = nullptr;

                ContextMemory(ContextCore *_cxt_core) { cxt_core = _cxt_core; }
               ~ContextMemory() { destroyContext(); }

                void initContext() {
                    memory_properties.loadForPhysicalDevice(cxt_core->phys_device.handle);
                }

                void destroyContext() {
                    for (auto& chunk : memory_chunks)
                        chunk.destroyChunk();
                }

            // Manage objects

                // Returns DeviceMemory context index (one per MemoryChunk, so it's used to access both)
                ContextIndex addMemoryChunk(VkMemoryPropertyFlags _required_flags) {
                    memory_chunks.emplace_back(cxt_core);
                    memory_chunks.back().required_flags = _required_flags;
                    return memory_chunks.size() - 1;
                }

                void allocateChunkMemory(ContextIndex _memory_ci) {
                    memory_chunks[_memory_ci].allocateChunkMemory(memory_properties.wrap.memoryProperties);
                }

                ContextIndex2 addChunkBuffer(ContextIndex _ci_memory, uint32_t _buf_size = 0u) { return { _ci_memory, memory_chunks[_ci_memory].addBuffer(_buf_size) }; }
                ContextIndex2 addChunkImage (ContextIndex _ci_memory, uint32_t _img_size = 0u) { return { _ci_memory, memory_chunks[_ci_memory].addImage (_img_size) }; }
                ContextIndex2 addChunkBufferView(ContextIndex _ci_memory) { return { _ci_memory, memory_chunks[_ci_memory].addBufferView() }; }
                ContextIndex2 addChunkImageView (ContextIndex _ci_memory) { return { _ci_memory, memory_chunks[_ci_memory].addImageView () }; }

                void createChunkBuffer(ContextIndex2 _ci2_buffer) { memory_chunks[_ci2_buffer.first].createBuffer(_ci2_buffer.second); }
                void createChunkImage (ContextIndex2 _ci2_image ) { memory_chunks[_ci2_image .first].createImage (_ci2_image .second); }
                void createChunkBufferView(ContextIndex2 _ci2_buf_view) { memory_chunks[_ci2_buf_view.first].createBuffer(_ci2_buf_view.second); }
                void createChunkImageView (ContextIndex2 _ci2_img_view) { memory_chunks[_ci2_img_view.first].createImage (_ci2_img_view.second); }

            // Scenarios

                void setWholeBufferCreateInfoStruct(ContextIndex2 _ci2_buffer, const std::vector<ContextIndex> &_ci_qfamilies,
                                                    VkSharingMode _sharing_mode, VkBufferUsageFlags _usage_flags, VkDeviceSize _size_bytes = 0u) {
                    if (_size_bytes > 0u)
                        memory_chunks[_ci2_buffer.first].buffer_maps[_ci2_buffer.second].range = _size_bytes;

                    std::vector<VkfwQueueFamily> __qfamilies((size_t)_ci_qfamilies.size());
                    for (uint32_t i = 0u; i < _ci_qfamilies.size(); ++i)
                        __qfamilies[i] = cxt_core->queue_families[i].handle;
                    memory_chunks[_ci2_buffer.first].buffers[_ci2_buffer.second].create_info.setWholeStruct(__qfamilies, _sharing_mode, _usage_flags, memory_chunks[_ci2_buffer.first].buffer_maps[_ci2_buffer.second].range);
                }
            } cxt_memory; // ContextMemory END

            struct ContextBarrier {
                std::vector<VkSemaphore> semaphores;
                std::vector<VkFence>         fences;
                std::vector<VkEvent>         events;

                Struct::StructWrap<VkSemaphoreCreateInfo> semaphore_wrap;
                Struct::StructWrap<VkFenceCreateInfo>         fence_wrap;
                Struct::StructWrap<VkEventCreateInfo>         event_wrap;

                ContextCore *cxt_core = nullptr;

                ContextBarrier(ContextCore *_cxt_core) { cxt_core = _cxt_core; }
               ~ContextBarrier() { destroyContext(); }

                void createContext() {}

                void destroyContext() {
                    for (ContextIndex ci = 0u; ci < semaphores.size(); ++ci)
                        destroySemaphore(ci);
                    for (ContextIndex ci = 0u; ci < fences.size(); ++ci)
                        destroyFence(ci);
                    for (ContextIndex ci = 0u; ci < events.size(); ++ci)
                        destroyEvent(ci);
                }

            // Manage Objects

                ContextIndex addSemaphore() {
                    ContextIndex __res_semaphore_ci = semaphores.size();
                    for (ContextIndex ci = 0u; ci < semaphores.size(); ++ci)
                        if (semaphores[ci] == VK_NULL_HANDLE) { __res_semaphore_ci = ci; break; }
                    if (__res_semaphore_ci >= semaphores.size()) semaphores.emplace_back();

                    vkCreateSemaphore(cxt_core->device.handle, &semaphore_wrap.wrap, cxt_core->alloc_cb.wrap, &semaphores[__res_semaphore_ci]);
                    return __res_semaphore_ci;
                }
                ContextIndex addFence(bool _is_signaled) {
                    ContextIndex __res_fence_ci = fences.size();
                    for (ContextIndex ci = 0u; ci < fences.size(); ++ci)
                        if (fences[ci] == VK_NULL_HANDLE) { __res_fence_ci = ci; break; }
                    if (__res_fence_ci >= fences.size()) fences.emplace_back();

                    fence_wrap.setWholeStruct(_is_signaled);
                    vkCreateFence(cxt_core->device.handle, &fence_wrap.wrap, cxt_core->alloc_cb.wrap, &fences[__res_fence_ci]);
                    return __res_fence_ci;
                }
                ContextIndex addEvent(bool _is_signaled) {
                    ContextIndex __res_event_ci = events.size();
                    for (ContextIndex ci = 0u; ci < events.size(); ++ci)
                        if (events[ci] == VK_NULL_HANDLE) { __res_event_ci = ci; break; }
                    if (__res_event_ci >= events.size()) events.emplace_back();

                    event_wrap.setWholeStruct(_is_signaled);
                    vkCreateEvent(cxt_core->device.handle, &event_wrap.wrap, cxt_core->alloc_cb.wrap, &events[__res_event_ci]);
                    return __res_event_ci;
                }

                void destroySemaphore(ContextIndex _semaphore_ci) {
                    if (semaphores[_semaphore_ci] != VK_NULL_HANDLE)
                        vkDestroySemaphore(cxt_core->device.handle, semaphores[_semaphore_ci], cxt_core->alloc_cb.wrap);
                    semaphores[_semaphore_ci] = VK_NULL_HANDLE;
                }
                void destroyFence(ContextIndex _fence_ci) {
                    if (fences[_fence_ci] != VK_NULL_HANDLE)
                        vkDestroyFence(cxt_core->device.handle, fences[_fence_ci], cxt_core->alloc_cb.wrap);
                    fences[_fence_ci] = VK_NULL_HANDLE;
                }
                void destroyEvent(ContextIndex _event_ci) {
                    if (events[_event_ci] != VK_NULL_HANDLE)
                        vkDestroyEvent(cxt_core->device.handle, events[_event_ci], cxt_core->alloc_cb.wrap);
                    events[_event_ci] = VK_NULL_HANDLE;
                }
            } cxt_barrier; // ContextBarrier END


            VulkanContext() : cxt_core{this},
                              cxt_swapchain{&cxt_core}, cxt_descriptor{&cxt_core}, cxt_pipeline{&cxt_core},
                              cxt_command  {&cxt_core}, cxt_memory    {&cxt_core}, cxt_barrier {&cxt_core} {}
           ~VulkanContext() { /* destroyContext();  */ }

            void destroyContext() {
                cxt_barrier.destroyContext();
                cxt_memory.destroyContext();
                cxt_command.destroyContext();
                cxt_pipeline.destroyContext();
                cxt_swapchain.destroyContext();
                cxt_descriptor.destroyContext();
                cxt_core.destroyContext();
            }

        // Scenarios

            void initNoncoreContexts() {
                cxt_memory.initContext();
            }

            void handleResult(VkResult _result) {
                // TODO: handle VkResult
            }
        } cxt_vkfw; // VulkanContext END



//  == === ==== ================================== ==== === ==  \\
                    Vulkan Additional Handlers
//  == === ==== ================================== ==== === ==  \\

    // Note: Handlers with an access to VulkanContext

//  == === ==== >   VKFW Object Getter Handler

        struct ObjGetterHandler {
            VKFW::VulkanContext *cxt_ptr = nullptr;

            ObjGetterHandler(VKFW::VulkanContext *_cxt_ptr = &cxt_vkfw) : cxt_ptr{_cxt_ptr} {}

        // Single object getters

        // Getters (Core)

            VkInstance               getObjInstance()                        const { return cxt_ptr->cxt_core.instance.handle; }
            VkDebugUtilsMessengerEXT getObjDebugUtilsMessengerEXT()          const { return cxt_ptr->cxt_core.cxt_debug.debug_messenger.handle;}
            VkPhysicalDevice         getObjPhysicalDevice()                  const { return cxt_ptr->cxt_core.phys_device.handle; }
            VkDevice                 getObjDevice()                          const { return cxt_ptr->cxt_core.device.handle; }
            VkfwQueueFamily          getObjQueueFamily(ContextIndex _obj_ci) const { return shouldIgnoreCI(_obj_ci ) ? UINT32_MAX     : cxt_ptr->cxt_core.queue_families[_obj_ci].handle; }
            VkQueue                  getObjQueue(ContextIndex2 _obj_ci2)     const { return shouldIgnoreCI(_obj_ci2) ? VK_NULL_HANDLE : cxt_ptr->cxt_core.queues[_obj_ci2.first].handles[_obj_ci2.second]; }
            VkRenderPass             getObjRenderPass()                      const { return cxt_ptr->cxt_core.render_pass.handle; }
            VkAllocationCallbacks*   getSctAllocationCallbacks()                   { return cxt_ptr->cxt_core.alloc_cb.wrap; }

            ContextEntity<Object::QueueFamily> shortcutObjQueueFamily(ContextIndex  _obj_ci ) { return {cxt_ptr->cxt_core.queue_families.data() + _obj_ci       , _obj_ci       }; }
            ContextEntity<Object::Queue      > shortcutObjQueue      (ContextIndex2 _obj_ci2) { return {cxt_ptr->cxt_core.queues        .data() + _obj_ci2.first, _obj_ci2.first}; }

            ContextEntity<Struct::StructWrap<VkAttachmentDescription>> shortcutSctRenderPassAttachment        (ContextIndex _sct_ci) { return cxt_ptr->cxt_core.render_pass.create_info.shortcutAttachment        (_sct_ci); }
            ContextEntity<Struct::StructWrap<   VkSubpassDescription>> shortcutSctRenderPassSubpassDescription(ContextIndex _sct_ci) { return cxt_ptr->cxt_core.render_pass.create_info.shortcutSubpassDescription(_sct_ci); }
            ContextEntity<Struct::StructWrap<    VkSubpassDependency>> shortcutSctRenderPassSubpassDependency (ContextIndex _sct_ci) { return cxt_ptr->cxt_core.render_pass.create_info.shortcutSubpassDependency (_sct_ci); }

        // Getters (Swapchain)

            VkSwapchainKHR getObjSwapchainKHR() const { return cxt_ptr->cxt_swapchain.swapchain.handle; }
            VkSurfaceKHR   getObjSurfaceKHR()   const { return cxt_ptr->cxt_swapchain.surface  .handle; }
            VkImageView    getObjSwapchainImageView  (ContextIndex _obj_ci) const { return shouldIgnoreCI(_obj_ci) ? VK_NULL_HANDLE : cxt_ptr->cxt_swapchain.image_views [_obj_ci]; }
            VkFramebuffer  getObjSwapchainFramebuffer(ContextIndex _obj_ci) const { return shouldIgnoreCI(_obj_ci) ? VK_NULL_HANDLE : cxt_ptr->cxt_swapchain.framebuffers[_obj_ci]; }

        // Getters (Descriptor)

            VkDescriptorSetLayout getObjDescriptorSetLayout(ContextIndex  _obj_ci ) const { return shouldIgnoreCI(_obj_ci ) ? VK_NULL_HANDLE : cxt_ptr->cxt_descriptor.layouts[_obj_ci].handle                      ; }
            VkDescriptorPool      getObjDescriptorPool     (ContextIndex  _obj_ci ) const { return shouldIgnoreCI(_obj_ci ) ? VK_NULL_HANDLE : cxt_ptr->cxt_descriptor.pools  [_obj_ci].handle                      ; }
            VkDescriptorSet       getObjDescriptorSet      (ContextIndex2 _obj_ci2) const { return shouldIgnoreCI(_obj_ci2) ? VK_NULL_HANDLE : cxt_ptr->cxt_descriptor.sets[_obj_ci2.first].handles[_obj_ci2.second]; }

            ContextEntity<Object::DescriptorSetLayout> shortcutObjDescriptorSetLayout(ContextIndex _obj_ci) { return {cxt_ptr->cxt_descriptor.layouts.data() + _obj_ci, _obj_ci}; }
            ContextEntity<Object::DescriptorPool     > shortcutObjDescriptorPool     (ContextIndex _obj_ci) { return {cxt_ptr->cxt_descriptor.  pools.data() + _obj_ci, _obj_ci}; }
            ContextEntity<Object::DescriptorSet      > shortcutObjDescriptorSet      (ContextIndex _obj_ci) { return {cxt_ptr->cxt_descriptor.   sets.data() + _obj_ci, _obj_ci}; }

            ContextEntity<Struct::StructWrap<VkDescriptorSetLayoutBinding>> shortcutSctDescriptorSetLayoutBinding(ContextIndex2 _sct_ci2) {
                auto __ce_sct_binding = cxt_ptr->cxt_descriptor.layouts[_sct_ci2.first].create_info.shortcutBinding(_sct_ci2.second);
                __ce_sct_binding.first = _sct_ci2.first;
                return __ce_sct_binding;
            }

        // Getters (Pipeline)

            VkPipeline       getObjPipeline      (ContextIndex _obj_ci) const { return shouldIgnoreCI(_obj_ci) ? VK_NULL_HANDLE : cxt_ptr->cxt_pipeline.pipelines[_obj_ci].handle; }
            VkPipelineLayout getObjPipelineLayout(ContextIndex _obj_ci) const { return shouldIgnoreCI(_obj_ci) ? VK_NULL_HANDLE : cxt_ptr->cxt_pipeline.layouts  [_obj_ci].handle; }
            VkPipelineCache  getObjPipelineCache (ContextIndex _obj_ci) const { return shouldIgnoreCI(_obj_ci) ? VK_NULL_HANDLE : cxt_ptr->cxt_pipeline.caches   [_obj_ci].handle; }
            VkShaderModule   getObjShaderModule  (ContextIndex _obj_ci) const { return shouldIgnoreCI(_obj_ci) ? VK_NULL_HANDLE : cxt_ptr->cxt_pipeline.shaders  [_obj_ci].handle; }

            ContextEntity<Object::Pipeline      > shortcutObjPipeline      (ContextIndex _obj_ci) { return {cxt_ptr->cxt_pipeline.pipelines.data() + _obj_ci, _obj_ci }; }
            ContextEntity<Object::PipelineLayout> shortcutObjPipelineLayout(ContextIndex _obj_ci) { return {cxt_ptr->cxt_pipeline.layouts  .data() + _obj_ci, _obj_ci }; }
            ContextEntity<Object::PipelineCache > shortcutObjPipelineCache (ContextIndex _obj_ci) { return {cxt_ptr->cxt_pipeline.caches   .data() + _obj_ci, _obj_ci }; }
            ContextEntity<Object::ShaderModule  > shortcutObjShaderModule  (ContextIndex _obj_ci) { return {cxt_ptr->cxt_pipeline.shaders  .data() + _obj_ci, _obj_ci }; }

            ContextEntity<Struct::StructWrap<VkPipelineShaderStageCreateInfo>> shortcutSctShaderStage(ContextIndex2 _sct_ci2) { return { cxt_ptr->cxt_pipeline.pipelines[_sct_ci2.first].shortcutShaderStage(_sct_ci2.second), _sct_ci2 }; }

        // Getters (Command)

            VkCommandPool   getObjCommandPool  (ContextIndex  _obj_ci ) const { return shouldIgnoreCI(_obj_ci ) ? VK_NULL_HANDLE : cxt_ptr->cxt_command.cmd_pools[_obj_ci].handle                           ; }
            VkCommandBuffer getObjCommandBuffer(ContextIndex2 _obj_ci2) const { return shouldIgnoreCI(_obj_ci2) ? VK_NULL_HANDLE : cxt_ptr->cxt_command.cmd_buffers[_obj_ci2.first].handles[_obj_ci2.second]; }

            ContextEntity<Object::CommandPool  > shortcutObjCommandPool  (ContextIndex  _obj_ci ) { return {cxt_ptr->cxt_command.cmd_pools  .data() + _obj_ci       , _obj_ci       }; }
            ContextEntity<Object::CommandBuffer> shortcutObjCommandBuffer(ContextIndex  _obj_ci ) { return {cxt_ptr->cxt_command.cmd_buffers.data() + _obj_ci       , _obj_ci       }; }
            ContextEntity<Object::CommandBuffer> shortcutObjCommandBuffer(ContextIndex2 _obj_ci2) { return {cxt_ptr->cxt_command.cmd_buffers.data() + _obj_ci2.first, _obj_ci2.first}; }

        // Getters (Device Memory)
        
            VkDeviceMemory getObjDeviceMemory(ContextIndex  _obj_ci ) const { return shouldIgnoreCI(_obj_ci ) ? VK_NULL_HANDLE : cxt_ptr->cxt_memory.memory_chunks[_obj_ci].memory.handle                              ; }
            VkBuffer       getObjBuffer      (ContextIndex2 _obj_ci2) const { return shouldIgnoreCI(_obj_ci2) ? VK_NULL_HANDLE : cxt_ptr->cxt_memory.memory_chunks[_obj_ci2.first].buffers     [_obj_ci2.second].handle; }
            VkImage        getObjImage       (ContextIndex2 _obj_ci2) const { return shouldIgnoreCI(_obj_ci2) ? VK_NULL_HANDLE : cxt_ptr->cxt_memory.memory_chunks[_obj_ci2.first].images      [_obj_ci2.second].handle; }
            VkBufferView   getObjBufferView  (ContextIndex2 _obj_ci2) const { return shouldIgnoreCI(_obj_ci2) ? VK_NULL_HANDLE : cxt_ptr->cxt_memory.memory_chunks[_obj_ci2.first].buffer_views[_obj_ci2.second].handle; }
            VkImageView    getObjImageView   (ContextIndex2 _obj_ci2) const { return shouldIgnoreCI(_obj_ci2) ? VK_NULL_HANDLE : cxt_ptr->cxt_memory.memory_chunks[_obj_ci2.first].image_views [_obj_ci2.second].handle; }

            VKFW::VulkanContext::ContextMemory::MemoryMapping getSctMemoryMapping(ContextIndex2 _obj_ci2) { return shouldIgnoreCI(_obj_ci2) ? VKFW::VulkanContext::ContextMemory::MemoryMapping{ VK_WHOLE_SIZE, VK_WHOLE_SIZE } : cxt_ptr->cxt_memory.memory_chunks[_obj_ci2.first].buffer_maps[_obj_ci2.second]; }

            ContextEntity<VulkanContext::ContextMemory::MemoryChunk> shortcutSctMemoryChunk(ContextIndex  _obj_ci ) { return { cxt_ptr->cxt_memory.memory_chunks.data() + _obj_ci, _obj_ci}; }
            ContextEntity<Object::DeviceMemory> shortcutObjDeviceMemory(ContextIndex  _obj_ci ) { return {&cxt_ptr->cxt_memory.memory_chunks[_obj_ci       ].memory, _obj_ci}; }
            ContextEntity<Object::Buffer      > shortcutObjBuffer      (ContextIndex2 _obj_ci2) { return { cxt_ptr->cxt_memory.memory_chunks[_obj_ci2.first].buffers     .data() + _obj_ci2.second, _obj_ci2}; }
            ContextEntity<Object::Image       > shortcutObjImage       (ContextIndex2 _obj_ci2) { return { cxt_ptr->cxt_memory.memory_chunks[_obj_ci2.first].images      .data() + _obj_ci2.second, _obj_ci2}; }
            ContextEntity<Object::BufferView  > shortcutObjBufferView  (ContextIndex2 _obj_ci2) { return { cxt_ptr->cxt_memory.memory_chunks[_obj_ci2.first].buffer_views.data() + _obj_ci2.second, _obj_ci2}; }
            ContextEntity<Object::ImageView   > shortcutObjImageView   (ContextIndex2 _obj_ci2) { return { cxt_ptr->cxt_memory.memory_chunks[_obj_ci2.first].image_views .data() + _obj_ci2.second, _obj_ci2}; }

        // Getters (Barrier)

            VkSemaphore getObjSemaphore(ContextIndex _obj_ci) const { return shouldIgnoreCI(_obj_ci) ? VK_NULL_HANDLE : cxt_ptr->cxt_barrier.semaphores[_obj_ci]; }
            VkFence     getObjFence    (ContextIndex _obj_ci) const { return shouldIgnoreCI(_obj_ci) ? VK_NULL_HANDLE : cxt_ptr->cxt_barrier.fences    [_obj_ci]; }
            VkEvent     getObjEvent    (ContextIndex _obj_ci) const { return shouldIgnoreCI(_obj_ci) ? VK_NULL_HANDLE : cxt_ptr->cxt_barrier.events    [_obj_ci]; }

        // Multiple object getters

        // Getters (Core)

            std::vector<VkfwQueueFamily> getObjsQueueFamily(const std::vector<ContextIndex> &_objs_ci) const {
                std::vector<VkfwQueueFamily> __res(_objs_ci.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_core.queue_families[_objs_ci[ci]].handle;
                return __res;
            }

            std::vector<VkQueue> getObjsQueue(const std::vector<ContextIndex2> &_objs_ci2) const {
                std::vector<VkQueue> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_core.queues[_objs_ci2[ci].first].handles[_objs_ci2[ci].second];
                return __res;
            }
            std::vector<VkQueue> getObjsQueue(ContextIndex _objs_ci1, const std::vector<ContextIndex> &_objs_ci2) const {
                std::vector<VkQueue> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_core.queues[_objs_ci1].handles[_objs_ci2[ci]];
                return __res;
            }

        // Getters (Swapchain)

            std::vector<VkImageView> getObjsSwapchainImageView(const std::vector<ContextIndex> &_objs_ci) const {
                std::vector<VkImageView> __res(_objs_ci.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_swapchain.image_views[_objs_ci[ci]];
                return __res;
            }

            std::vector<VkFramebuffer> getObjsSwapchainFramebuffer(const std::vector<ContextIndex> &_objs_ci) const {
                std::vector<VkFramebuffer> __res(_objs_ci.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_swapchain.framebuffers[_objs_ci[ci]];
                return __res;
            }

        // Getters (Descriptor)

            std::vector<VkDescriptorSetLayout> getObjsDescriptorSetLayout(const std::vector<ContextIndex> &_objs_ci) const {
                std::vector<VkDescriptorSetLayout> __res(_objs_ci.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_descriptor.layouts[_objs_ci[ci]].handle;
                return __res;
            }

            std::vector<VkDescriptorPool> getObjsDescriptorPool(const std::vector<ContextIndex> &_objs_ci) const {
                std::vector<VkDescriptorPool> __res(_objs_ci.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_descriptor.pools[_objs_ci[ci]].handle;
                return __res;
            }

            std::vector<VkDescriptorSet> getObjsDescriptorSet(const std::vector<ContextIndex2> &_objs_ci2) const {
                std::vector<VkDescriptorSet> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_descriptor.sets[_objs_ci2[ci].first].handles[_objs_ci2[ci].second];
                return __res;
            }
            std::vector<VkDescriptorSet> getObjsDescriptorSet(ContextIndex _objs_ci1, const std::vector<ContextIndex> &_objs_ci2) const {
                std::vector<VkDescriptorSet> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_descriptor.sets[_objs_ci1].handles[_objs_ci2[ci]];
                return __res;
            }

        // Getters (Pipeline)

            std::vector<VkPipeline> getObjsPipeline(const std::vector<ContextIndex> &_objs_ci) const {
                std::vector<VkPipeline> __res(_objs_ci.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_pipeline.pipelines[_objs_ci[ci]].handle;
                return __res;
            }

            std::vector<VkPipelineCache> getObjsPipelineCache(const std::vector<ContextIndex> &_objs_ci) const {
                std::vector<VkPipelineCache> __res(_objs_ci.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_pipeline.caches[_objs_ci[ci]].handle;
                return __res;
            }

            std::vector<VkPipelineLayout> getObjsPipelineLayout(const std::vector<ContextIndex> &_objs_ci) const {
                std::vector<VkPipelineLayout> __res(_objs_ci.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_pipeline.layouts[_objs_ci[ci]].handle;
                return __res;
            }

        // Getters (Command)

            std::vector<VkCommandPool> getObjsCommandPool(const std::vector<ContextIndex> &_objs_ci) const {
                std::vector<VkCommandPool> __res(_objs_ci.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_command.cmd_pools[_objs_ci[ci]].handle;
                return __res;
            }

            std::vector<VkCommandBuffer> getObjsCommandBuffer(const std::vector<ContextIndex2> &_objs_ci2) const {
                std::vector<VkCommandBuffer> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_command.cmd_buffers[_objs_ci2[ci].first].handles[_objs_ci2[ci].second];
                return __res;
            }
            std::vector<VkCommandBuffer> getObjsCommandBuffer(ContextIndex _objs_ci1, const std::vector<ContextIndex> &_objs_ci2) const {
                std::vector<VkCommandBuffer> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_command.cmd_buffers[_objs_ci1].handles[_objs_ci2[ci]];
                return __res;
            }

        // Getters (Device Memory)

            std::vector<VkDeviceMemory> getObjsDeviceMemory(const std::vector<ContextIndex> &_objs_ci) const {
                std::vector<VkDeviceMemory> __res(_objs_ci.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_memory.memory_chunks[_objs_ci[ci]].memory.handle;
                return __res;
            }

            std::vector<VkBuffer> getObjsBuffer(const std::vector<ContextIndex2> &_objs_ci2) const {
                std::vector<VkBuffer> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_memory.memory_chunks[_objs_ci2[ci].first].buffers[_objs_ci2[ci].second].handle;
                return __res;
            }
            std::vector<VkBuffer> getObjsBuffer(ContextIndex _objs_ci1, const std::vector<ContextIndex> &_objs_ci2) const {
                std::vector<VkBuffer> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_memory.memory_chunks[_objs_ci1].buffers[_objs_ci2[ci]].handle;
                return __res;
            }

            std::vector<VkImage> getObjsImage(const std::vector<ContextIndex2> &_objs_ci2) const {
                std::vector<VkImage> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_memory.memory_chunks[_objs_ci2[ci].first].images[_objs_ci2[ci].second].handle;
                return __res;
            }
            std::vector<VkImage> getObjsImage(ContextIndex _objs_ci1, const std::vector<ContextIndex> &_objs_ci2) const {
                std::vector<VkImage> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_memory.memory_chunks[_objs_ci1].images[_objs_ci2[ci]].handle;
                return __res;
            }

            std::vector<VkBufferView> getObjsBufferView(const std::vector<ContextIndex2> &_objs_ci2) const {
                std::vector<VkBufferView> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_memory.memory_chunks[_objs_ci2[ci].first].buffer_views[_objs_ci2[ci].second].handle;
                return __res;
            }
            std::vector<VkBufferView> getObjsBufferView(ContextIndex _objs_ci1, const std::vector<ContextIndex> &_objs_ci2) const {
                std::vector<VkBufferView> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_memory.memory_chunks[_objs_ci1].buffer_views[_objs_ci2[ci]].handle;
                return __res;
            }

            std::vector<VkImageView> getObjsImageView(const std::vector<ContextIndex2> &_objs_ci2) const {
                std::vector<VkImageView> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_memory.memory_chunks[_objs_ci2[ci].first].image_views[_objs_ci2[ci].second].handle;
                return __res;
            }
            std::vector<VkImageView> getObjsImageView(ContextIndex _objs_ci1, const std::vector<ContextIndex> &_objs_ci2) const {
                std::vector<VkImageView> __res(_objs_ci2.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci2.size(); ++ci) __res[ci] = cxt_ptr->cxt_memory.memory_chunks[_objs_ci1].image_views[_objs_ci2[ci]].handle;
                return __res;
            }

        // Getters (Barrier)

            std::vector<VkSemaphore> getObjsSemaphore(const std::vector<ContextIndex> &_objs_ci) const {
                std::vector<VkSemaphore> __res(_objs_ci.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_barrier.semaphores[_objs_ci[ci]];
                return __res;
            }

            std::vector<VkFence> getObjsFence(const std::vector<ContextIndex> &_objs_ci) const {
                std::vector<VkFence> __res(_objs_ci.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_barrier.fences[_objs_ci[ci]];
                return __res;
            }

            std::vector<VkEvent> getObjsEvent(const std::vector<ContextIndex> &_objs_ci) const {
                std::vector<VkEvent> __res(_objs_ci.size(), 0u);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_barrier.events[_objs_ci[ci]];
                return __res;
            }
        }; // ObjGetterHandler END


//  == === ==== >   VKFW Barrier Handler

        struct BarrierHandler {
            ObjGetterHandler cxt_handler;

            BarrierHandler(VKFW::VulkanContext *_cxt_ptr = &cxt_vkfw) : cxt_handler{_cxt_ptr} {}

        // Functions

            void WaitForFences(const std::vector<ContextIndex> &_fences_ci, VkBool32 _wait_all = VK_TRUE, uint64_t _wait_time = UINT64_MAX) {
                std::vector<VkFence> __fences = cxt_handler.getObjsFence(_fences_ci);
                Func::WaitForFences(cxt_handler.getObjDevice(), __fences, _wait_all, _wait_time);
            }

            void ResetFences(const std::vector<ContextIndex> &_fences_ci) {
                std::vector<VkFence> __fences = cxt_handler.getObjsFence(_fences_ci);
                Func::ResetFences(cxt_handler.getObjDevice(), __fences);
            }
        }; // BarrierHandler END


//  == === ==== >   VKFW Device Memory Map Handler

        struct MemoryMapHandler {
            std::pair<void *, ContextIndex> mem_handle{nullptr, VKFW_CI_IGNORE};
            std::vector<std::pair<ContextIndex2, VulkanContext::ContextMemory::MemoryMapping>> buf_maps, img_maps;

            ObjGetterHandler cxt_handler;

            MemoryMapHandler(VKFW::VulkanContext *_cxt_ptr = &cxt_vkfw) : cxt_handler{_cxt_ptr} {}
           ~MemoryMapHandler() {
                if (cxt_handler.getObjDevice() != VK_NULL_HANDLE) UnmapMemory();
                clear();
            }

            void MapMemory(ContextIndex _ci_memory, VkDeviceSize _offset = 0u, VkDeviceSize _size = VK_WHOLE_SIZE) {
                mem_handle = { Func::MapMemory(cxt_handler.getObjDevice(), cxt_handler.getObjDeviceMemory(_ci_memory), _offset, _size), _ci_memory };
            }

            void UnmapMemory() {
                if (mem_handle.second != VKFW_CI_IGNORE)
                    Func::UnmapMemory(cxt_handler.getObjDevice(), cxt_handler.getObjDeviceMemory(mem_handle.second));
            }

            void clear() {
                mem_handle = {nullptr, VKFW_CI_IGNORE};
                buf_maps.clear();
                img_maps.clear();
            }

        // Memory Functions

            void FlushMappedMemoryRanges(const std::vector<VulkanContext::ContextMemory::MemoryMapping> &_mem_ranges) const {
                std::vector<VkMappedMemoryRange> __mem_ranges(_mem_ranges.size(), VkMappedMemoryRange{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE});
                for (uint32_t i = 0u; i < __mem_ranges.size(); ++i) {
                    __mem_ranges[i].memory =  cxt_handler.getObjDeviceMemory(mem_handle.second);
                    __mem_ranges[i].offset = _mem_ranges[i].offset;
                    __mem_ranges[i].size   = _mem_ranges[i].range;
                }

                Func::FlushMappedMemoryRanges(cxt_handler.getObjDevice(), __mem_ranges);
            }

            void InvalidateMappedMemoryRanges(const std::vector<VulkanContext::ContextMemory::MemoryMapping> &_mem_ranges) const {
                std::vector<VkMappedMemoryRange> __mem_ranges(_mem_ranges.size(), VkMappedMemoryRange{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE});
                for (uint32_t i = 0u; i < __mem_ranges.size(); ++i) {
                    __mem_ranges[i].memory =  cxt_handler.getObjDeviceMemory(mem_handle.second);
                    __mem_ranges[i].offset = _mem_ranges[i].offset;
                    __mem_ranges[i].size   = _mem_ranges[i].range;
                }

                Func::InvalidateMappedMemoryRanges(cxt_handler.getObjDevice(), __mem_ranges);
            }

        // Scenarios
        
            void mapMemoryFromBuffer(ContextIndex2 _buffer_ci, VkDeviceSize _offset = 0u, VkDeviceSize _size = VK_WHOLE_SIZE) {
                auto __buffer_mapping = cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_buffer_ci.first].buffer_maps[_buffer_ci.second];
                SVKFW_ASSERT(_offset         < __buffer_mapping.range, std::invalid_argument, "VKFW::MemoryMapHandle :: mapMemoryFromBuffer",          "'offset' is out of buffer range: " + std::to_string(_offset        ) + " < " + std::to_string(__buffer_mapping.range));
                if (_size != VK_WHOLE_SIZE)
                    SVKFW_ASSERT(_offset + _size < __buffer_mapping.range, std::invalid_argument, "VKFW::MemoryMapHandle :: mapMemoryFromBuffer", "'size' + 'offset' is out of buffer range: " + std::to_string(_offset + _size) + " < " + std::to_string(__buffer_mapping.range));

                __buffer_mapping.offset += _offset;
                __buffer_mapping.range   =   _size;

                MapMemory(_buffer_ci.first, __buffer_mapping.offset, __buffer_mapping.range);
            }

            void mapMemoryFromImage(ContextIndex2 _image_ci, VkDeviceSize _offset = 0u, VkDeviceSize _size = VK_WHOLE_SIZE) {
                auto __image_mapping = cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_image_ci.first].image_maps[_image_ci.second];
                SVKFW_ASSERT(_offset         < __image_mapping.range, std::invalid_argument, "VKFW::MemoryMapHandle :: mapMemoryFromImage",          "'offset' is out of image range: " + std::to_string(_offset        ) + " < " + std::to_string(__image_mapping.range));
                if (_size != VK_WHOLE_SIZE)
                    SVKFW_ASSERT(_offset + _size < __image_mapping.range, std::invalid_argument, "VKFW::MemoryMapHandle :: mapMemoryFromImage", "'size' + 'offset' is out of image range: " + std::to_string(_offset + _size) + " < " + std::to_string(__image_mapping.range));

                __image_mapping.offset += _offset;
                __image_mapping.range   =   _size;

                MapMemory(_image_ci.first, __image_mapping.offset, __image_mapping.range);
            }

            void mapMemoryFromResources(const std::vector<ContextIndex2> &_ci_bufs, const std::vector<ContextIndex2> &_ci_imgs) {
                SVKFW_ASSERT(!_ci_bufs.empty() || !_ci_imgs.empty(), std::invalid_argument, "VKFW::MemoryMapHandler :: mapMemoryFromResources",
                                "Both buffer and image arrays are empty");

                vec2u __mapping_loc{~0u, 0u};
                ContextIndex __ci_device_mem = VKFW_CI_IGNORE;
                if (!_ci_bufs.empty()) __ci_device_mem = _ci_bufs[0].first;
                if (!_ci_imgs.empty()) __ci_device_mem = _ci_imgs[0].first;

                buf_maps.resize(_ci_bufs.size());
                for (uint32_t i = 0u; i < _ci_bufs.size(); ++i) {
                    SVKFW_ASSERT(_ci_bufs[i].first == __ci_device_mem, std::invalid_argument, "VKFW::MemoryMapHandler :: mapMemoryFromResources",
                                    "buffer memory chunk mismatch: " + std::to_string(_ci_bufs[i].first) + " and " + std::to_string(__ci_device_mem));
                    buf_maps[i].first  = _ci_bufs[i];
                    buf_maps[i].second = cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_ci_bufs[i].first].buffer_maps[_ci_bufs[i].second];
                    __mapping_loc.x = std::min(__mapping_loc.x, uint32_t(buf_maps[i].second.offset));
                    __mapping_loc.y = std::max(__mapping_loc.y, uint32_t(buf_maps[i].second.offset + buf_maps[i].second.range));
                }

                img_maps.resize(_ci_imgs.size());
                for (uint32_t i = 0u; i < _ci_imgs.size(); ++i) {
                    SVKFW_ASSERT(_ci_imgs[i].first == __ci_device_mem, std::invalid_argument, "VKFW::MemoryMapHandler :: mapMemoryFromResources",
                                    "image memory chunk mismatch: " + std::to_string(_ci_imgs[i].first) + " and " + std::to_string(__ci_device_mem) + (_ci_bufs.empty() ? " (image)" : " (buffer)"));
                    img_maps[i].first  = _ci_imgs[i];
                    img_maps[i].second = cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_ci_imgs[i].first].image_maps[_ci_imgs[i].second];
                    __mapping_loc.x = std::min(__mapping_loc.x, uint32_t(img_maps[i].second.offset));
                    __mapping_loc.y = std::max(__mapping_loc.y, uint32_t(img_maps[i].second.offset + img_maps[i].second.range));
                }

                for (auto& buf_map : buf_maps)
                    buf_map.second.offset -= __mapping_loc.x;
                for (auto& img_map : img_maps)
                    img_map.second.offset -= __mapping_loc.x;
                __mapping_loc.y -= __mapping_loc.x;

                MapMemory(__ci_device_mem, __mapping_loc.x, __mapping_loc.y);
            }

            void copyToBufferMap(ContextIndex2 _ci_buf, const void* _data_ptr, int64_t _mem_offset = 0, VkDeviceSize _mem_size = VK_WHOLE_SIZE) const {
                bool __success = false;
                for (const auto &buf_map : buf_maps) {
                    if (buf_map.first == _ci_buf) {
                        if (_mem_size == VK_WHOLE_SIZE) _mem_size = buf_map.second.range;
                        // printf("Copying %ld bytes to mapped buffer: [%d, %d)\n", _mem_size, buf_map.second.offset, buf_map.second.offset + buf_map.second.range);
                        memcpy((uint8_t*)mem_handle.first + buf_map.second.offset + _mem_offset, _data_ptr, _mem_size);
                        if (!(cxt_handler.cxt_ptr->cxt_memory.memory_chunks[mem_handle.second].required_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
                            FlushMappedMemoryRanges({ {buf_map.second.offset + _mem_offset, _mem_size} });
                        __success = true;
                        break;
                    }
                }
            }

            void copyToImageMap(ContextIndex2 _ci_img, const void* _data_ptr, int64_t _mem_offset = 0, VkDeviceSize _mem_size = VK_WHOLE_SIZE) const {
                bool __success = false;
                for (const auto &img_map : img_maps) {
                    if (img_map.first == _ci_img) {
                        if (_mem_size == VK_WHOLE_SIZE) _mem_size = img_map.second.range;
                        memcpy((uint8_t*)mem_handle.first + img_map.second.offset + _mem_offset, _data_ptr, _mem_size);
                        if (!(cxt_handler.cxt_ptr->cxt_memory.memory_chunks[mem_handle.second].required_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
                            FlushMappedMemoryRanges({ {img_map.second.offset + _mem_offset, _mem_size} });
                        __success = true;
                        break;
                    }
                }
            }

            void copyFromBufferMap(ContextIndex2 _ci_buf, void* _data_ptr, int64_t _mem_offset = 0, VkDeviceSize _mem_size = VK_WHOLE_SIZE) const {
                bool __success = false;
                for (const auto &buf_map : buf_maps) {
                    if (buf_map.first == _ci_buf) {
                        if (!(cxt_handler.cxt_ptr->cxt_memory.memory_chunks[mem_handle.second].required_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
                            InvalidateMappedMemoryRanges({ {buf_map.second.offset + _mem_offset, _mem_size} });
                        memcpy(_data_ptr, (uint8_t*)mem_handle.first + buf_map.second.offset + _mem_offset, _mem_size);
                        __success = true;
                    }
                }
            }

            void copyFromImageMap(ContextIndex2 _ci_img, void* _data_ptr, int64_t _mem_offset = 0, VkDeviceSize _mem_size = VK_WHOLE_SIZE) const {
                bool __success = false;
                for (const auto &img_map : img_maps) {
                    if (img_map.first == _ci_img) {
                        if (!(cxt_handler.cxt_ptr->cxt_memory.memory_chunks[mem_handle.second].required_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
                            InvalidateMappedMemoryRanges({ {img_map.second.offset + _mem_offset, _mem_size} });
                        memcpy(_data_ptr, (uint8_t*)mem_handle.first + img_map.second.offset + _mem_offset, _mem_size);
                        __success = true;
                    }
                }
            }
        }; // MemoryMapHandler END


//  == === ==== >   VKFW Command Buffer Handler

        struct CommandHandler {
            VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;

            Callable::CmdBeginRenderPass begin_render_pass_wrap;
            Callable::BeginCommandBuffer  begin_cmd_buffer_wrap;

            ObjGetterHandler cxt_handler;

            CommandHandler(VKFW::VulkanContext *_cxt_ptr = &cxt_vkfw) : cxt_handler{_cxt_ptr} { setBeginRenderPassRenderPass(); }
           ~CommandHandler() { clear(); }

            void setCmdBuffer(ContextIndex2 _cmd_buffer_ci2) {
                cmd_buffer = cxt_handler.getObjCommandBuffer(_cmd_buffer_ci2);
            }

            void clear() {
                cmd_buffer = VK_NULL_HANDLE;
            }

        // CmdBuffer-related

            void BeginCommandBuffer() {
                begin_cmd_buffer_wrap.callFunction(cmd_buffer);
            }

            void EndCommandBuffer() {
                vkEndCommandBuffer(cmd_buffer);
            }

            void ResetCommandBuffer(bool _set_release_resources_bit = false) {
                vkResetCommandBuffer(cmd_buffer, _set_release_resources_bit ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : VkCommandBufferResetFlags(0u));
            }

        // Commands

            void CmdBeginRenderPass(VkSubpassContents _contents) {
                begin_render_pass_wrap.updateWrap();
                begin_render_pass_wrap.callFunction(cmd_buffer, _contents);
            }

            void CmdEndRenderPass() {
                vkCmdEndRenderPass(cmd_buffer);
            }

            void CmdBindDescriptorSets(ContextIndex _ci_pip_layout, VkPipelineBindPoint _pip_bind_pt, const std::vector<ContextIndex2> &_ci2_descr_sets, const std::vector<uint32_t> &_dynamic_descr = {}) {
                auto __obj_descr_sets = cxt_handler.getObjsDescriptorSet(_ci2_descr_sets);
                vkCmdBindDescriptorSets(cmd_buffer, _pip_bind_pt, cxt_handler.getObjPipelineLayout(_ci_pip_layout), 0u, __obj_descr_sets.size(), __obj_descr_sets.data(), _dynamic_descr.size(), _dynamic_descr.data());
            }

            void CmdBindIndexBuffer(ContextIndex2 _index_buffer_ci2, VkDeviceSize _offset = 0u, VkIndexType _index_type = VK_INDEX_TYPE_UINT32) {
                vkCmdBindIndexBuffer(cmd_buffer, cxt_handler.getObjBuffer(_index_buffer_ci2), _offset, _index_type);
            }

            void CmdBindPipeline(ContextIndex _pipeline_ci) {
                VkPipelineBindPoint __pipeline_type;
                switch (cxt_handler.cxt_ptr->cxt_pipeline.pipelines[_pipeline_ci].pipeline_type) {
                    case VKFW_PIPELINE_TYPE_GRAPHICS   : __pipeline_type = VK_PIPELINE_BIND_POINT_GRAPHICS       ; break;
                    case VKFW_PIPELINE_TYPE_COMPUTE    : __pipeline_type = VK_PIPELINE_BIND_POINT_COMPUTE        ; break;
                    case VKFW_PIPELINE_TYPE_RAY_TRACING: __pipeline_type = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR; break;
                }
                vkCmdBindPipeline(cmd_buffer, __pipeline_type, cxt_handler.getObjPipeline(_pipeline_ci));
            }

            void CmdBindVertexBuffers(const std::vector<ContextIndex2> &_ci_buffers, std::vector<VkDeviceSize> &&_offsets = {}) {
                std::vector<VkBuffer> __obj_buffers = cxt_handler.getObjsBuffer(_ci_buffers);
                _offsets.resize(_ci_buffers.size(), 0u);

                vkCmdBindVertexBuffers(cmd_buffer, 0u, __obj_buffers.size(), __obj_buffers.data(), _offsets.empty() ? nullptr : _offsets.data());
            }

            void CmdClearAttachments() {
                VkClearAttachment attach{};
                attach.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                attach.clearValue.color = {1.f, 0.f, 1.f, 1.f};
                attach.colorAttachment = 0u;
                VkClearRect rect{};
                rect.rect.offset = {0, 0};
                rect.rect.extent = cxt_handler.cxt_ptr->cxt_swapchain.swapchain.create_info.wrap.imageExtent;
                rect.baseArrayLayer = 0;
                rect.layerCount = 1;

                vkCmdClearAttachments(cmd_buffer, 1, &attach, 1, &rect);
            }

            // void CmdClearColorImage() {
            //     vkCmdClearColorImage()
            // }

            void CmdCopyBuffer(ContextIndex2 _src_buffer_ci2, ContextIndex2 _dst_buffer_ci2, const std::vector<VkBufferCopy> &_copy_regions) {
                VkBuffer __src_buffer = cxt_handler.getObjBuffer(_src_buffer_ci2);
                VkBuffer __dst_buffer = cxt_handler.getObjBuffer(_dst_buffer_ci2);

                vkCmdCopyBuffer(cmd_buffer, __src_buffer, __dst_buffer, (uint32_t) _copy_regions.size(), _copy_regions.data());
            }

            void CmdCopyImage(ContextIndex2 _src_image_ci2, ContextIndex2 _dst_image_ci2, VkImageLayout _layout1, VkImageLayout _layout2, const std::vector<VkImageCopy> &_copy_regions) {
                VkImage __src_image = cxt_handler.getObjImage(_src_image_ci2);
                VkImage __dst_image = cxt_handler.getObjImage(_dst_image_ci2);

                vkCmdCopyImage(cmd_buffer, __src_image, _layout1, __dst_image, _layout2, (uint32_t) _copy_regions.size(), _copy_regions.data());
            }

            void CmdCopyBufferToImage(ContextIndex2 _src_buffer_ci2, ContextIndex2 _dst_image_ci2, VkImageLayout _layout, const std::vector<VkBufferImageCopy> &_copy_regions) {
                VkBuffer __src_buffer = cxt_handler.getObjBuffer(_src_buffer_ci2);
                VkImage  __dst_image  = cxt_handler.getObjImage ( _dst_image_ci2);

                vkCmdCopyBufferToImage(cmd_buffer, __src_buffer, __dst_image, _layout, (uint32_t) _copy_regions.size(), _copy_regions.data());
            }

            void CmdDraw(uint32_t _vert_count, uint32_t _inst_count, uint32_t _first_vert = 0u, uint32_t _first_inst = 0u) {
                vkCmdDraw(cmd_buffer, _vert_count, _inst_count, _first_vert, _first_inst);
            }
            // Note: each index is 'index + vert_offset'
            void CmdDrawIndexed(uint32_t _ind_count, uint32_t _inst_count, int32_t _vert_offset = 0, uint32_t _first_ind = 0u, uint32_t _first_inst = 0u) {
                vkCmdDrawIndexed(cmd_buffer, _ind_count, _inst_count, _first_ind, _vert_offset, _first_inst);
            }

            void CmdPipelineBarrier() {
                // VkMemoryBarrier;
                // VkImageMemoryBarrier;
                // VkBufferMemoryBarrier;
                // vkCmdPipelineBarrier(cmd_buffer, );
            }

            void CmdSetLineWidth(float _line_width) {
               vkCmdSetLineWidth(cmd_buffer, _line_width); 
            }

            void CmdSetScissor(const std::vector<VkRect2D> &_scissors) {
                vkCmdSetScissor(cmd_buffer, 0, _scissors.size(), _scissors.data());
            }

            void CmdSetViewport(const std::vector<VkViewport> &_viewports) {
                vkCmdSetViewport(cmd_buffer, 0, _viewports.size(), _viewports.data());
            }

        // Scenarios

            void setBeginRenderPassRenderPass() {
                begin_render_pass_wrap.wrap.wrap.renderPass = cxt_handler.getObjRenderPass();
            }

            void setBeginRenderPassSwapchainRenderArea() {
                begin_render_pass_wrap.wrap.render_area_wrap.setOffset(vec2i(0));
                begin_render_pass_wrap.wrap.render_area_wrap.setExtent(cxt_handler.cxt_ptr->cxt_swapchain.swapchain.create_info.wrap.imageExtent);
            }

            void setBeginRenderPassSwapchainFramebuffer(ContextIndex _ci_framebuffer) {
                begin_render_pass_wrap.wrap.wrap.framebuffer = cxt_handler.getObjSwapchainFramebuffer(_ci_framebuffer);
            }

            void cmdSetViewportScissorFromSwapchainExtent() {
                VkViewport __swapchain_viewport{};
                VkRect2D   __swapchain_scissor {};

                __swapchain_scissor.extent  = cxt_handler.cxt_ptr->cxt_swapchain.swapchain.create_info.wrap.imageExtent;
                __swapchain_viewport.width  = __swapchain_scissor.extent.width ;
                __swapchain_viewport.height = __swapchain_scissor.extent.height;

                vkCmdSetViewport(cmd_buffer, 0, 1, &__swapchain_viewport);
                vkCmdSetScissor (cmd_buffer, 0, 1, &__swapchain_scissor );
            }
        }; // CommandHandler END


//  == === ==== >   VKFW Function Call Handler

        struct FuncHandler {
            Callable::UpdateDescriptorSets update_descr_sets;
            Callable::QueuePresentKHR     queue_present_wrap;
            Callable::QueueSubmit          queue_submit_wrap;

            std::vector<ContextIndex> wait_fences_for_swapchain; // fences to wait before recreating swapchain

            ObjGetterHandler cxt_handler;

            FuncHandler(VKFW::VulkanContext *_cxt_ptr = &cxt_vkfw) : cxt_handler{_cxt_ptr} {}
           ~FuncHandler() {}


            void setFencesToWait(const std::vector<ContextIndex> &_ci_fences) {
                wait_fences_for_swapchain = _ci_fences;
            }

        // Functions

            void QueuePresentKHR(ContextIndex2 _ci2_queue_pres, uint64_t _recreate_wait_fences_timeout = UINT64_MAX) {
                queue_present_wrap.updateWrap();
                if (queue_present_wrap.callFunction(cxt_handler.getObjQueue(_ci2_queue_pres)) == UINT32_MAX) {
                    waitForRequiredFences(_recreate_wait_fences_timeout);
                    cxt_handler.cxt_ptr->cxt_swapchain.recreate();
                }
            }

            inline void QueueSubmit(ContextIndex2 _ci2_queue, const std::vector<ContextIndex2> &_ci2_cmdbuf, const std::vector<uint32_t> &_active_submits, ContextIndex _fence_ci = VKFW_CI_IGNORE) {
                QueueSubmit(cxt_handler.getObjQueue(_ci2_queue), cxt_handler.getObjsCommandBuffer(_ci2_cmdbuf), _active_submits, _fence_ci);
            }
            void QueueSubmit(VkQueue _obj_queue, const std::vector<VkCommandBuffer> &_obj_cmdbuf, const std::vector<uint32_t> &_active_submits, ContextIndex _fence_ci = VKFW_CI_IGNORE) {
                for (uint32_t i : _active_submits)
                    queue_submit_wrap.wraps[i].command_buffers = _obj_cmdbuf;

                queue_submit_wrap.setActiveSubmits(_active_submits);
                queue_submit_wrap.callFunction(_obj_queue, cxt_handler.getObjFence(_fence_ci));
            }

            uint32_t AcquireNextImageKHR(ContextIndex _semaphore_ci, ContextIndex _fence_ci = VKFW_CI_IGNORE, uint64_t _timeout = UINT64_MAX) {
                VkFence __fence = cxt_handler.getObjFence(_fence_ci);
                uint32_t __image_index = UINT32_MAX;

                VkResult __result = vkAcquireNextImageKHR(cxt_handler.getObjDevice(), cxt_handler.getObjSwapchainKHR(),
                                                          _timeout, cxt_handler.getObjSemaphore(_semaphore_ci), __fence, &__image_index);

                if (__result == VK_SUBOPTIMAL_KHR || __result == VK_ERROR_OUT_OF_DATE_KHR) {
                    waitForRequiredFences(_timeout);
                    cxt_handler.cxt_ptr->cxt_swapchain.recreate();
                    __image_index = UINT32_MAX;
                }
                else if (__result != VK_SUCCESS)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW::SwapchainHandler :: AcquireNextImageKHR", "Failed to acquire next swapchain image"));

                return __image_index;
            }

            void UpdateDescriptorSets() {
                update_descr_sets.callFunction(cxt_handler.getObjDevice());
            }

        // Scenarios

            void waitForRequiredFences(uint64_t _timeout = UINT64_MAX) {
                auto __obj_fences = cxt_handler.getObjsFence(wait_fences_for_swapchain);
                if (!__obj_fences.empty())
                    vkWaitForFences(cxt_handler.getObjDevice(), __obj_fences.size(), __obj_fences.data(), VK_TRUE, _timeout);
            }

            void setWholeQueuePresentStruct(const std::vector<ContextIndex> &_ci_wait_semaphores,
                                            const std::vector<uint32_t> &_image_indices, bool _use_results = false) {
                auto __obj_semaphores = cxt_handler.getObjsSemaphore(_ci_wait_semaphores);
                queue_present_wrap.wrap.setWholeStruct(__obj_semaphores, {cxt_handler.getObjSwapchainKHR()}, _image_indices, _use_results);
            }

            void writeBufferMemory(CommandHandler &_cmd_handler, ContextIndex2 _ci2_queue_transfer, ContextIndex2 _buffer_ci2, ContextIndex2 _staging_buffer_ci2, const void *_copy_data, size_t _copy_size, size_t _dst_offset = 0u) {
                if ((cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_buffer_ci2.first].buffers[_buffer_ci2.second].create_info.wrap.usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0u)
                    fprintf(svkfwwarn, "%s (%d, %d) %s\n", SVKFW_WRAPWARN("VKFW::CommandHandler :: writeBufferMemory", "Buffer with context index"), _buffer_ci2.first, _buffer_ci2.second,
                                                                            "is not set as transfer operation destination point");

                if ((cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_staging_buffer_ci2.first].buffers[_staging_buffer_ci2.second].create_info.wrap.usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0u)
                    fprintf(svkfwwarn, "%s (%d, %d) %s\n", SVKFW_WRAPWARN("VKFW::CommandHandler :: writeBufferMemory", "Staging buffer with context index"), _staging_buffer_ci2.first, _staging_buffer_ci2.second,
                                                                            "is not set as transfer operation source point");

                if ((cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_staging_buffer_ci2.first].required_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0u)
                    fprintf(svkfwwarn, "%s (%d, %d) %s\n", SVKFW_WRAPWARN("VKFW::CommandHandler :: writeBufferMemory", "Staging buffer with context index"), _staging_buffer_ci2.first, _staging_buffer_ci2.second,
                                                                            "cannot be mapped to CPU");

                bool   __call_flush = (cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_staging_buffer_ci2.first].required_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0u;
                size_t __staging_buffer_map_size = std::min(_copy_size, cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_staging_buffer_ci2.first].buffer_maps[_staging_buffer_ci2.second].range);
                BarrierHandler      __barrier_h{cxt_handler.cxt_ptr};
                MemoryMapHandler __buffer_map_h{cxt_handler.cxt_ptr};
                __buffer_map_h.mapMemoryFromBuffer(_staging_buffer_ci2, 0u, __staging_buffer_map_size);

                ContextIndex  __fence_ci = cxt_handler.cxt_ptr->cxt_barrier.addFence(false);
                ContextIndex __submit_ci = queue_submit_wrap.wraps.size();

                queue_submit_wrap.wraps.emplace_back();
                queue_submit_wrap.wraps[__submit_ci].command_buffers = { _cmd_handler.cmd_buffer };
                queue_submit_wrap.wraps[__submit_ci].wait_stages = {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};

                VkMappedMemoryRange __mem_range{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
                __mem_range.memory = cxt_handler.getObjDeviceMemory(_staging_buffer_ci2.first);
                VkBufferCopy __copy_region{ .dstOffset = _dst_offset };

                while (_copy_size > 0u) {
                    size_t __curr_copy_size = std::min(__staging_buffer_map_size, _copy_size);
                    __copy_region.size = __curr_copy_size;
                    _copy_size -= __curr_copy_size;

                    _cmd_handler.BeginCommandBuffer();

                    memcpy(__buffer_map_h.mem_handle.first, (uint8_t*)_copy_data + (__copy_region.dstOffset - _dst_offset), __curr_copy_size);
                    if (__call_flush) __buffer_map_h.FlushMappedMemoryRanges({ { 0u, __curr_copy_size } });
                    _cmd_handler.CmdCopyBuffer(_staging_buffer_ci2, _buffer_ci2, { __copy_region });

                    _cmd_handler.EndCommandBuffer();

                    QueueSubmit(cxt_handler.getObjQueue(_ci2_queue_transfer), {_cmd_handler.cmd_buffer}, {__submit_ci}, __fence_ci);
                    __barrier_h.WaitForFences({__fence_ci});
                    __barrier_h.  ResetFences({__fence_ci});

                    _cmd_handler.ResetCommandBuffer();
                    __copy_region.dstOffset += __curr_copy_size;
                }
                cxt_handler.cxt_ptr->cxt_barrier.destroyFence(__fence_ci);
                queue_submit_wrap.wraps.pop_back();
            }

            // TODO: writeImageMemory for image types

            // void writeImageMemory(ContextIndex2 _image_ci2, ContextIndex2 _staging_buffer_ci2, VkImageLayout _layout, const void *_copy_data, size_t _copy_size, size_t _dst_offset = 0u) {
            //     if ((cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_image_ci2.first].buffers[_image_ci2.second].create_info.wrap.usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0u)
            //         fprintf(svkfwwarn, "%s (%d, %d) %s\n", SVKFW_WRAPWARN("VKFW::CommandHandler :: writeImageMemory", "Image with context index"), _image_ci2.first, _image_ci2.second,
            //                                                                 "is not set as transfer operation destination point");

            //     if ((cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_staging_buffer_ci2.first].buffers[_staging_buffer_ci2.second].create_info.wrap.usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0u)
            //         fprintf(svkfwwarn, "%s (%d, %d) %s\n", SVKFW_WRAPWARN("VKFW::CommandHandler :: writeImageMemory", "Staging buffer with context index"), _staging_buffer_ci2.first, _staging_buffer_ci2.second,
            //                                                                 "is not set as transfer operation source point");

            //     if ((cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_staging_buffer_ci2.first].required_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0u)
            //         fprintf(svkfwwarn, "%s (%d, %d) %s\n", SVKFW_WRAPWARN("VKFW::CommandHandler :: writeImageMemory", "Staging buffer with context index"), _staging_buffer_ci2.first, _staging_buffer_ci2.second,
            //                                                                 "cannot be mapped to CPU");

            //     bool   __call_flush = (cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_staging_buffer_ci2.first].required_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0u;
            //     size_t __staging_buffer_map_size = std::min(_copy_size, cxt_handler.cxt_ptr->cxt_memory.memory_chunks[_staging_buffer_ci2.first].buffer_maps[_staging_buffer_ci2.second].range);
            //     BarrierHandler      __barrier_h{cxt_handler.cxt_ptr};
            //     MemoryMapHandler __buffer_map_h{cxt_handler.cxt_ptr};
            //     __buffer_map_h.setMemoryChunk(_staging_buffer_ci2.first);

            //     ContextIndex __memory_map_ci = __buffer_map_h.mapMemoryFromBuffer(_staging_buffer_ci2.second, 0u, __staging_buffer_map_size);
            //     ContextIndex      __fence_ci = __barrier_h.cxt_handler.cxt_ptr->cxt_barrier.addFence(false);
            //     ContextIndex     __submit_ci =   queue_submit_wrap.wraps.size();

            //     queue_submit_wrap.wraps.emplace_back();
            //     queue_submit_wrap.wraps[__submit_ci].command_buffers = {cmd_buffer};
            //     queue_submit_wrap.wraps[__submit_ci].wait_stages = {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};

            //     VkMappedMemoryRange __mem_range{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
            //     __mem_range.memory = cxt_handler.getObjDeviceMemory(_staging_buffer_ci2.first);
            //     VkBufferImageCopy __copy_region{};
            //     __copy_region.dstOffset = _dst_offset;

            //     while (_copy_size > 0u) {
            //         size_t __curr_copy_size = std::min(__staging_buffer_map_size, _copy_size);
            //         _copy_size -= __curr_copy_size;

            //         BeginCommandBuffer();

            //         memcpy(__buffer_map_h.handles[__memory_map_ci], _copy_data + (__copy_region.dstOffset - _dst_offset), __curr_copy_size);
            //         if (__call_flush) __buffer_map_h.FlushMappedMemoryRanges({ { 0u, __curr_copy_size } });
            //         CmdCopyBufferToImage(_staging_buffer_ci2, _buffer_ci2, { __copy_region });

            //         EndCommandBuffer();

            //         QueueSubmit({__submit_ci}, __fence_ci);
            //         __barrier_h.WaitForFences({__fence_ci});
            //         __barrier_h.  ResetFences({__fence_ci});

            //         __copy_region.dstOffset += __curr_copy_size;
            //     }
            // }
        }; // FuncHandler END
    }; // VKFW END
}; // Simple END

#endif