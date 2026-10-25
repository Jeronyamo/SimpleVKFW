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
                // Note: a queue family and the queues created from it share one Id
                struct QueueFamilyEntry {
                    Object::QueueFamily family;
                    Object::Queue       queues;
                };
                Pool<QueueFamilyEntry, QueueFamilyTag> queue_families; // TODO: USE ONE QUEUE_FAMILY WRAP! DATA IS DUPLICATED
                Object::RenderPass render_pass;

                Struct::StructWrap<VkAllocationCallbacks> alloc_cb;
                ContextDebug cxt_debug;

                ContextCore(VulkanContext* _cxt_ptr) : cxt_debug{_cxt_ptr} {
                    SVKFW_ASSERT(volkInitialize() == VK_SUCCESS, std::runtime_error, "VKFW::VulkanContext :: ContextCore constructor",
                                    "volk initialization failed");
                    // Note: members are constructed before volkInitialize(), so the version was computed without 'vkEnumerateInstanceVersion'
                    instance.create_info.app_info_wrap.setAvailableVersionAPI();
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
                QueueFamilyId addQueueFamily() { return queue_families.add(); }

            // Access by Id (throws if the Id is none, stale or out of its family)

                Object::QueueFamily &queueFamily(QueueFamilyId _id) { return queue_families.get(_id).family; }
                Object::Queue       &queues     (QueueFamilyId _id) { return queue_families.get(_id).queues; }
                VkQueue queue(QueueId _id) {
                    std::vector<VkQueue> &__handles = queue_families.get(_id.parent).queues.handles;
                    SVKFW_ASSERT(_id.index < __handles.size(), std::out_of_range, "VKFW::ContextCore :: queue", "queue index is out of its family");
                    return __handles[_id.index];
                }

            // Create Objects

                void createInstance() {
                    instance.createObject(alloc_cb.get());
                }
                void createDebugUtilsMessenger() {
                    cxt_debug.debug_messenger.createObject(instance.handle, alloc_cb.get());
                }
                void loadPhysicalDeviceInfo() {
                    phys_device.select_info.loadInfo(instance.handle);
                }
                void findPhysicalDevice() {
                    phys_device.createObject();
                }
                // TODO: USE ONE QUEUE_FAMILY WRAP! DATA IS DUPLICATED
                void loadQueueFamiliesInfo() {
                    queue_families.forEach([this](QueueFamilyId, QueueFamilyEntry &_qfam) {
                        _qfam.family.select_info.loadInfo(instance.handle, phys_device.handle);
                    });
                }
                void findQueueFamilies() {
                    queue_families.forEach([](QueueFamilyId, QueueFamilyEntry &_qfam) { _qfam.family.createObject(); });
                }
                void createDevice() {
                    device.createObject(phys_device.handle, alloc_cb.get());
                }
                void acquireQueues() {
                    queue_families.forEach([this](QueueFamilyId, QueueFamilyEntry &_qfam) { _qfam.queues.createObject(device.handle); });
                }
                void createRenderPass() {
                    render_pass.createObject(device.handle, alloc_cb.get());
                }


                void destroyInstance() {
                    instance.destroyObject(alloc_cb.get());
                }
                void destroyDebugUtilsMessenger() {
                    cxt_debug.debug_messenger.destroyObject(instance.handle, alloc_cb.get());
                }
                void destroyPhysicalDevice() {
                    phys_device.destroyObject();
                }
                void destroyDevice() {
                    device.destroyObject(alloc_cb.get());
                }
                void destroyRenderPass() {
                    render_pass.destroyObject(device.handle, alloc_cb.get());
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

                void setQueueFamilyQueueCount(QueueFamilyId _queue_family_id, uint32_t _queue_count, const std::vector<float> &_priorities = {1.0f}) {
                    QueueFamilyEntry &__qfam = queue_families.get(_queue_family_id);
                    __qfam.queues.create_info.queue_family = __qfam.family.handle;
                    __qfam.queues.create_info.setQueueCount(_queue_count, _priorities);
                }

                void setDeviceQueueInfo() {
                    device.create_info.queue_info.clear();
                    queue_families.forEach([this](QueueFamilyId, QueueFamilyEntry &_qfam) {
                        device.create_info.queue_info.push_back(_qfam.queues.create_info.get());
                    });
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
                    image_views.clear();
                    framebuffers.clear();
                    destroySurface();
                }

            // Manage Objects

                void createSurface() {
                    surface.createObject(cxt_core->instance.handle, window, cxt_core->alloc_cb.get());
                }

                void createSwapchain() {
                    swapchain.create_info.edit().surface = surface.handle;
                    swapchain.createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                    image_view_wrap.edit().format = swapchain.create_info.peek().imageFormat;
                    framebuffer_wrap.setBufferSize(swapchain.create_info.peek().imageExtent);
                }
                void createImageViews() {
                    std::vector<VkImage> __swapchain_images{Func::GetSwapchainImagesKHR(cxt_core->device.handle, swapchain.handle)};

                    image_views.resize(__swapchain_images.size());
                    for (ContextIndex ci = 0u; ci < __swapchain_images.size(); ++ci) {
                        image_view_wrap.edit().image = __swapchain_images[ci];
                        image_views[ci] = Func::CreateImageView(&image_view_wrap.get(), cxt_core->device.handle, __swapchain_images[ci], cxt_core->alloc_cb.get());
                    }
                }
                void createFramebuffers() {
                    framebuffers.resize(image_views.size());
                    framebuffer_wrap.edit().renderPass = cxt_core->render_pass.handle;

                    for (ContextIndex ci = 0u; ci < framebuffers.size(); ++ci) {
                        framebuffer_wrap.attachments = {image_views[ci]};
                        framebuffers[ci] = Func::CreateFramebuffer(&framebuffer_wrap.get(), cxt_core->device.handle, cxt_core->alloc_cb.get());
                    }
                }


                void destroySurface() {
                    surface.destroyObject(cxt_core->instance.handle, cxt_core->alloc_cb.get());
                }
                void destroySwapchain() {
                    swapchain.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }
                void destroyImageViews() {
                    for (auto    img_view :  image_views)
                        vkDestroyImageView(cxt_core->device.handle, img_view, cxt_core->alloc_cb.get());
                }
                void destroyFramebuffers() {
                    for (auto framebuffer : framebuffers)
                        vkDestroyFramebuffer(cxt_core->device.handle, framebuffer, cxt_core->alloc_cb.get());
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

                uint32_t getSwapchainImageCount() {
                    return image_views.size();
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
                    cxt_core->render_pass.create_info.attachments_wrap[_attachment_ci].edit().format = swapchain.create_info.peek().imageFormat;
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
                Pool<Object::DescriptorSetLayout, DescriptorSetLayoutTag> layouts;
                Pool<Object::DescriptorPool,      DescriptorPoolTag>        pools;
                Pool<Object::DescriptorSet,       DescriptorSetsTag>         sets;
                // Note: '1' Pool AND 'N' Layouts --> 'N' Sets

                ContextCore *cxt_core = nullptr;

                ContextDescriptor(ContextCore *_cxt_core) { cxt_core = _cxt_core; }
               ~ContextDescriptor() { destroyContext(); }

                // Note: Only Core and Swapchain contexts' logic allows creation of all objects at once
                void  createContext() {}

                void destroyContext() {
                    // Note: sets are freed when their pools are destroyed
                    VkDevice                     __device   = cxt_core->device.handle;
                    const VkAllocationCallbacks *__alloc_cb = cxt_core->alloc_cb.get();
                    pools  .forEach([&](DescriptorPoolId,      Object::DescriptorPool      &_pool  ) { _pool  .destroyObject(__device, __alloc_cb); });
                    layouts.forEach([&](DescriptorSetLayoutId, Object::DescriptorSetLayout &_layout) { _layout.destroyObject(__device, __alloc_cb); });
                    sets   .clear();
                    pools  .clear();
                    layouts.clear();
                }

            // Access by Id (throws if the Id is none, stale or out of its group)

                VkDescriptorSet set(DescriptorSetId _id) {
                    std::vector<VkDescriptorSet> &__handles = sets.get(_id.parent).handles;
                    SVKFW_ASSERT(_id.index < __handles.size(), std::out_of_range, "VKFW::ContextDescriptor :: set", "set index is out of its group");
                    return __handles[_id.index];
                }

            // Manage Objects

                DescriptorSetLayoutId addLayout() { return layouts.add(); }
                DescriptorPoolId      addPool  () { return   pools.add(); }
                DescriptorSetsId      addSet   () { return    sets.add(); }

                void createLayout(DescriptorSetLayoutId _layout_id) {
                    layouts.get(_layout_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }
                void createPool(DescriptorPoolId _pool_id) {
                    pools.get(_pool_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }
                void createSets(DescriptorSetsId _sets_id) {
                    sets.get(_sets_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }

                // Note: 'destroy*' keeps the Id (the object can be created again), 'remove*' also frees the Id
                void destroyLayout(DescriptorSetLayoutId _layout_id) {
                    layouts.get(_layout_id).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }
                void destroyPool(DescriptorPoolId _pool_id) {
                    pools.get(_pool_id).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }
                void destroySets(DescriptorSetsId _sets_id) {
                    sets.get(_sets_id).destroyObject(cxt_core->device.handle);
                }

                void removeLayout(DescriptorSetLayoutId _layout_id) { if (layouts.contains(_layout_id)) { destroyLayout(_layout_id); layouts.remove(_layout_id); } }
                void removePool  (DescriptorPoolId      _pool_id  ) { if (  pools.contains(_pool_id  )) { destroyPool  (_pool_id  );   pools.remove(_pool_id  ); } }
                void removeSets  (DescriptorSetsId      _sets_id  ) { if (   sets.contains(_sets_id  )) { destroySets  (_sets_id  );    sets.remove(_sets_id  ); } }

            // Scenarios

                DescriptorLayoutBindingId addSctLayoutBinding(DescriptorSetLayoutId _layout_id) {
                    return DescriptorLayoutBindingId(_layout_id, layouts.get(_layout_id).create_info.addBinding());
                }

                void setAllForSet(DescriptorSetsId _sets_id, DescriptorPoolId _pool_id, Span<DescriptorSetLayoutId> _layouts_id) {
                    std::vector<VkDescriptorSetLayout> __layouts(_layouts_id.size());
                    for (uint32_t i = 0u; i < _layouts_id.size(); ++i)
                        __layouts[i] = layouts.get(_layouts_id[i]).handle;

                    sets.get(_sets_id).alloc_info.setWholeStruct(pools.get(_pool_id).handle, __layouts);
                }

                void setPoolForSet(DescriptorSetsId _sets_id, DescriptorPoolId _pool_id) {
                    sets.get(_sets_id).alloc_info.edit().descriptorPool = pools.get(_pool_id).handle;
                }

                void setLayoutsForSet(DescriptorSetsId _sets_id, Span<DescriptorSetLayoutId> _layouts_id) {
                    std::vector<VkDescriptorSetLayout> __layouts(_layouts_id.size());
                    for (uint32_t i = 0u; i < _layouts_id.size(); ++i)
                        __layouts[i] = layouts.get(_layouts_id[i]).handle;

                    sets.get(_sets_id).alloc_info.set_layouts = __layouts;
                }
            } cxt_descriptor; // ContextDescriptors END

            struct ContextPipeline {
                Pool<Object::Pipeline,       PipelineTag>       pipelines;
                Pool<Object::PipelineLayout, PipelineLayoutTag>   layouts;
                Pool<Object::PipelineCache,  PipelineCacheTag>     caches;
                Pool<Object::ShaderModule,   ShaderModuleTag>     shaders;

                ContextCore *cxt_core = nullptr;

                ContextPipeline(ContextCore *_cxt_core) { cxt_core = _cxt_core; }
               ~ContextPipeline() { destroyContext(); }

                // Note: Only Core and Swapchain contexts' logic allows creation of all objects at once
                void  createContext() {}

                void destroyContext() {
                    VkDevice                     __device   = cxt_core->device.handle;
                    const VkAllocationCallbacks *__alloc_cb = cxt_core->alloc_cb.get();
                    pipelines.forEach([&](PipelineId,       Object::Pipeline       &_pipeline) { _pipeline.destroyObject(__device, __alloc_cb); });
                    layouts  .forEach([&](PipelineLayoutId, Object::PipelineLayout &_layout  ) { _layout  .destroyObject(__device, __alloc_cb); });
                    caches   .forEach([&](PipelineCacheId,  Object::PipelineCache  &_cache   ) { _cache   .destroyObject(__device, __alloc_cb); });
                    shaders  .forEach([&](ShaderModuleId,   Object::ShaderModule   &_shader  ) { _shader  .destroyObject(__device, __alloc_cb); });
                    pipelines.clear();
                    layouts  .clear();
                    caches   .clear();
                    shaders  .clear();
                }

            // Manage Objects

                PipelineId       addPipeline() { return pipelines.add(); }
                ShaderModuleId   addShader  () { return   shaders.add(); }
                PipelineLayoutId addLayout  () { return   layouts.add(); }
                PipelineCacheId  addCache   () { return    caches.add(); }

                void createPipeline(PipelineId _pipeline_id, uint32_t _subpass, PipelineLayoutId _layout_id, PipelineCacheId _cache_id = PipelineCacheId(),
                                    VkDeferredOperationKHR _deferred_op = VK_NULL_HANDLE) {
                    Object::Pipeline &__pipeline = pipelines.get(_pipeline_id);
                    VkPipelineLayout  __layout   = layouts  .get(_layout_id).handle;
                    switch (__pipeline.pipeline_type) {
                        case VKFW_PIPELINE_TYPE_GRAPHICS:
                            __pipeline.  graphics_create_info_wrap.edit().renderPass = cxt_core->render_pass.handle;
                            __pipeline.  graphics_create_info_wrap.edit().subpass    = _subpass;

                            __pipeline.  graphics_create_info_wrap.edit().layout = __layout; break;
                        case VKFW_PIPELINE_TYPE_COMPUTE:
                            __pipeline.   compute_create_info_wrap.edit().layout = __layout; break;
                        case VKFW_PIPELINE_TYPE_RAY_TRACING:
                            __pipeline.raytracing_create_info_wrap.edit().layout = __layout; break;
                        default: break;
                    }
                    VkPipelineCache __pipeline_cache = _cache_id.isNone() ? VK_NULL_HANDLE : caches.get(_cache_id).handle;
                    __pipeline.createObject(cxt_core->device.handle, cxt_core->render_pass.handle, _subpass,
                                            _deferred_op, __pipeline_cache, cxt_core->alloc_cb.get());
                }
                void createCache (PipelineCacheId _cache_id) {
                    caches .get(_cache_id ).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }
                void createLayout(PipelineLayoutId _layout_id) {
                    layouts.get(_layout_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }
                void createShader(ShaderModuleId _shader_id) {
                    shaders.get(_shader_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }

                // Note: 'destroy*' keeps the Id (the object can be created again), 'remove*' also frees the Id
                void destroyPipeline(PipelineId _pipeline_id) {
                    pipelines.get(_pipeline_id).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }
                void destroyCache   (PipelineCacheId _cache_id) {
                    caches   .get(_cache_id   ).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }
                void destroyLayout  (PipelineLayoutId _layout_id) {
                    layouts  .get(_layout_id  ).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }
                void destroyShader  (ShaderModuleId _shader_id) {
                    shaders  .get(_shader_id  ).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }

                void removePipeline(PipelineId       _pipeline_id) { if (pipelines.contains(_pipeline_id)) { destroyPipeline(_pipeline_id); pipelines.remove(_pipeline_id); } }
                void removeCache   (PipelineCacheId  _cache_id   ) { if (caches   .contains(_cache_id   )) { destroyCache   (_cache_id   ); caches   .remove(_cache_id   ); } }
                void removeLayout  (PipelineLayoutId _layout_id  ) { if (layouts  .contains(_layout_id  )) { destroyLayout  (_layout_id  ); layouts  .remove(_layout_id  ); } }
                void removeShader  (ShaderModuleId   _shader_id  ) { if (shaders  .contains(_shader_id  )) { destroyShader  (_shader_id  ); shaders  .remove(_shader_id  ); } }

            // Scenarios

                void setLayoutDescriptorSetLayouts(PipelineLayoutId _layout_id, Span<DescriptorSetLayoutId> _descr_set_layouts_id) {
                    Object::PipelineLayout &__layout = layouts.get(_layout_id);
                    __layout.create_info.layouts.resize(_descr_set_layouts_id.size(), VK_NULL_HANDLE);
                    for (uint32_t i = 0u; i < _descr_set_layouts_id.size(); ++i)
                        __layout.create_info.layouts[i] = cxt_core->cxt_debug.cxt_ptr->cxt_descriptor.layouts.get(_descr_set_layouts_id[i]).handle;
                }

                void addLayoutPushConstant(PipelineLayoutId _layout_id, VkShaderStageFlags _stage_flags, uint32_t _offset, uint32_t _size) {
                    layouts.get(_layout_id).create_info.constants.push_back(VkPushConstantRange{_stage_flags, _offset, _size});
                }

                ShaderStageId addShaderStage(PipelineId _pipeline_id) {
                    return ShaderStageId(_pipeline_id, pipelines.get(_pipeline_id).addShaderStage());
                }
            } cxt_pipeline; // ContextPipeline END

            struct ContextCommand {
                Pool<Object::CommandPool,   CommandPoolTag>      cmd_pools;
                Pool<Object::CommandBuffer, CommandBuffersTag> cmd_buffers;

                ContextCore *cxt_core = nullptr;

                ContextCommand(ContextCore *_cxt_core) { cxt_core = _cxt_core; }
               ~ContextCommand() { destroyContext(); }

                // Note: Only Core and Swapchain contexts' logic allows creation of all objects at once
                void  createContext() {}

                void destroyContext() {
                    VkDevice                     __device   = cxt_core->device.handle;
                    const VkAllocationCallbacks *__alloc_cb = cxt_core->alloc_cb.get();
                    cmd_buffers.forEach([&](CommandBuffersId, Object::CommandBuffer &_cmd_buff) { _cmd_buff.destroyObject(__device); });
                    cmd_pools  .forEach([&](CommandPoolId,    Object::CommandPool   &_cmd_pool) { _cmd_pool.destroyObject(__device, __alloc_cb); });
                    cmd_buffers.clear();
                    cmd_pools  .clear();
                }

            // Access by Id (throws if the Id is none, stale or out of its group)

                VkCommandBuffer commandBuffer(CommandBufferId _id) {
                    std::vector<VkCommandBuffer> &__handles = cmd_buffers.get(_id.parent).handles;
                    SVKFW_ASSERT(_id.index < __handles.size(), std::out_of_range, "VKFW::ContextCommand :: commandBuffer", "command buffer index is out of its group");
                    return __handles[_id.index];
                }

            // Manage Objects

                CommandPoolId    addCmdPool  () { return   cmd_pools.add(); }
                CommandBuffersId addCmdBuffer() { return cmd_buffers.add(); }

                void createCmdPool(CommandPoolId _cmd_pool_id) {
                    cmd_pools.get(_cmd_pool_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }
                void createCmdBuffers(CommandBuffersId _cmd_buffers_id) {
                    cmd_buffers.get(_cmd_buffers_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                }

                // Note: removing a pool frees its command buffers on the Vulkan side, remove their groups first
                void removeCmdBuffers(CommandBuffersId _cmd_buffers_id) {
                    if (!cmd_buffers.contains(_cmd_buffers_id)) return;
                    cmd_buffers.get(_cmd_buffers_id).destroyObject(cxt_core->device.handle);
                    cmd_buffers.remove(_cmd_buffers_id);
                }
                void removeCmdPool(CommandPoolId _cmd_pool_id) {
                    if (!cmd_pools.contains(_cmd_pool_id)) return;
                    cmd_pools.get(_cmd_pool_id).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                    cmd_pools.remove(_cmd_pool_id);
                }

            // Scenarios

                void createCmdPool(CommandPoolId _cmd_pool_id, QueueFamilyId _queue_family_id, VkCommandPoolCreateFlags _flags = 0u) {
                    cmd_pools.get(_cmd_pool_id).createObject(cxt_core->device.handle, cxt_core->queueFamily(_queue_family_id).handle, _flags, cxt_core->alloc_cb.get());
                }

                void createCmdBuffersPrimary(CommandBuffersId _cmd_buffers_id, CommandPoolId _cmd_pool_id, uint32_t _cmd_buffers_count) {
                    cmd_buffers.get(_cmd_buffers_id).createPrimary(cxt_core->device.handle, cmd_pools.get(_cmd_pool_id).handle, _cmd_buffers_count, cxt_core->alloc_cb.get());
                }
                void createCmdBuffersSecondary(CommandBuffersId _cmd_buffers_id, CommandPoolId _cmd_pool_id, uint32_t _cmd_buffers_count) {
                    cmd_buffers.get(_cmd_buffers_id).createSecondary(cxt_core->device.handle, cmd_pools.get(_cmd_pool_id).handle, _cmd_buffers_count, cxt_core->alloc_cb.get());
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
                    void *mapped_ptr = nullptr; // HOST_VISIBLE chunks stay mapped from allocation to destruction (persistent mapping)
                    // 48 bytes left (2 vectors) for 256 bytes per chunk

                    ContextCore *cxt_core = nullptr;

                    MemoryChunk() {}
                    MemoryChunk(ContextCore *_cxt_core) { cxt_core = _cxt_core; }

                    void destroyChunk() {
                        if (mapped_ptr != nullptr) Func::UnmapMemory(cxt_core->device.handle, memory.handle);
                        mapped_ptr = nullptr;

                        for (auto& buf_view : buffer_views)
                            buf_view.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                        for (auto& img_view :  image_views)
                            img_view.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                        for (auto& buf : buffers)
                            buf.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                        for (auto& img :  images)
                            img.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());

                        memory.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
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
                            VkDeviceSize __additional_offset = (__mem_req_i.alignment - (__mem_req.size % __mem_req_i.alignment)) % __mem_req_i.alignment;
                            __mem_req.size += __additional_offset;
                            buffer_maps[ci].offset = __mem_req.size;

                            // Note: the device may require more memory than requested by user ('range'), so the max is used
                            __mem_req.size += std::max(buffer_maps[ci].range, __mem_req_i.size);
                        }
                        // Align last buffer
                        if (!buffers.empty()) {
                            VkDeviceSize __additional_offset = (__mem_req_i.alignment - (__mem_req.size % __mem_req_i.alignment)) % __mem_req_i.alignment;
                            __mem_req.size += __additional_offset;
                        }

                        // Compute all image offsets
                        for (uint32_t ci = 0u; ci <  images.size(); ++ci) {
                            __mem_req_i = Func::GetImageMemoryRequirements(cxt_core->device.handle, images[ci].handle);
                            SVKFW_ASSERT(__mem_req.memoryTypeBits & __mem_req_i.memoryTypeBits, std::runtime_error, "VKFW :: ContextMemory :: allocateChunkMemory",
                                            "No suitable memory type left for image " + std::to_string(ci));

                            __mem_req.memoryTypeBits &= __mem_req_i.memoryTypeBits;
                            VkDeviceSize __additional_offset = (__mem_req_i.alignment - (__mem_req.size % __mem_req_i.alignment)) % __mem_req_i.alignment;
                            __mem_req.size += __additional_offset;
                            image_maps[ci].offset = __mem_req.size;

                            // Note: the device may require more memory than requested by user ('range'), so the max is used
                            __mem_req.size += std::max(image_maps[ci].range, __mem_req_i.size);
                        }
                        // Align last image
                        if (!images.empty()) {
                            VkDeviceSize __additional_offset = (__mem_req_i.alignment - (__mem_req.size % __mem_req_i.alignment)) % __mem_req_i.alignment;
                            __mem_req.size += __additional_offset;
                        }

                        uint32_t __supported_mem_type_index = VK_MAX_MEMORY_TYPES;
                        for (uint32_t i = 0u; i < _allowed_size.memoryTypeCount; ++i) {
                            bool __use_memtype = true;
                            __use_memtype &= (__mem_req.memoryTypeBits & (1u << i)) != 0u; // can represent
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
                        memory.allocate_info.edit().memoryTypeIndex = __supported_mem_type_index;
                        memory.allocate_info.edit().allocationSize  = __mem_req.size;
                        memory.createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());

                        // Note: mapping a memory object once and keeping it mapped is allowed and cheaper than mapping it every frame
                        if (required_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                            mapped_ptr = Func::MapMemory(cxt_core->device.handle, memory.handle);

                        // Bind all buffers to created memory
                        VkResult __func_res = VK_SUCCESS;
                        for (ContextIndex ci = 0u; ci < buffers.size(); ++ci) {
                            __func_res = vkBindBufferMemory(cxt_core->device.handle, buffers[ci].handle, memory.handle, buffer_maps[ci].offset);
                            SVKFW_ASSERT(__func_res == VK_SUCCESS, std::runtime_error, "VKFW::MemoryChunk", "bind buffer error, code " + std::to_string(__func_res));
                        }

                        // Bind all images to created memory
                        for (ContextIndex ci = 0u; ci < images.size(); ++ci) {
                            __func_res = vkBindImageMemory(cxt_core->device.handle, images[ci].handle, memory.handle, image_maps[ci].offset);
                            SVKFW_ASSERT(__func_res == VK_SUCCESS, std::runtime_error, "VKFW::MemoryChunk", "bind image error, code " + std::to_string(__func_res));
                        }
                    }

                // Manage Objects

                    // Note: return the object's index inside the chunk ('ContextMemory::addChunk*' wraps it into a typed Id)
                    uint32_t addBuffer(uint32_t _buf_size = 0u) { buffers.emplace_back(); buffer_maps.push_back(MemoryMapping{0u, _buf_size}); return buffers.size() - 1; }
                    uint32_t addImage (uint32_t _img_size = 0u) {  images.emplace_back();  image_maps.push_back(MemoryMapping{0u, _img_size}); return  images.size() - 1; }
                    uint32_t addBufferView() { buffer_views.emplace_back(); return buffer_views.size() - 1; }
                    uint32_t addImageView () {  image_views.emplace_back(); return  image_views.size() - 1; }

                    void createBuffer(uint32_t _buffer_index) {
                        buffers[_buffer_index].createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                    }
                    void createImage (uint32_t _image_index) {
                        images [ _image_index].createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                    }
                    void createBufferView(uint32_t _buffer_view_index, uint32_t _buffer_index) {
                        buffer_views[_buffer_view_index].createObject(cxt_core->device.handle, buffers[_buffer_index].handle, cxt_core->alloc_cb.get());
                    }
                    void createImageView (uint32_t  _image_view_index, uint32_t  _image_index) {
                        image_views [ _image_view_index].createObject(cxt_core->device.handle,  images[ _image_index].handle, cxt_core->alloc_cb.get());
                    }

                    void destroyBuffer(uint32_t _buffer_index) {
                        buffers[_buffer_index].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                    }
                    void destroyImage (uint32_t _image_index) {
                        images [ _image_index].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                    }
                    void destroyBufferView(uint32_t _buffer_view_index) {
                        buffer_views[_buffer_view_index].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                    }
                    void destroyImageView (uint32_t  _image_view_index) {
                        image_views [ _image_view_index].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
                    }
                }; // MemoryChunk END
                Pool<MemoryChunk, MemoryTag> memory_chunks;

                Struct::StructWrap<VkPhysicalDeviceMemoryProperties2> memory_properties;

                ContextCore *cxt_core = nullptr;

                ContextMemory(ContextCore *_cxt_core) { cxt_core = _cxt_core; }
               ~ContextMemory() { destroyContext(); }

                void initContext() {
                    memory_properties.loadForPhysicalDevice(cxt_core->phys_device.handle);
                }

                void destroyContext() {
                    memory_chunks.forEach([](MemoryId, MemoryChunk &_chunk) { _chunk.destroyChunk(); });
                    memory_chunks.clear();
                }

            // Access by Id (throws if the Id is none, stale or out of its chunk)

                      MemoryChunk &chunk(MemoryId _id)       { return memory_chunks.get(_id); }
                const MemoryChunk &chunk(MemoryId _id) const { return memory_chunks.get(_id); }

                Object::Buffer     &buffer    (BufferId     _id) { return at_(chunk(_id.parent).buffers,      _id.index); }
                Object::Image      &image     (ImageId      _id) { return at_(chunk(_id.parent).images,       _id.index); }
                Object::BufferView &bufferView(BufferViewId _id) { return at_(chunk(_id.parent).buffer_views, _id.index); }
                Object::ImageView  &imageView (ImageViewId  _id) { return at_(chunk(_id.parent).image_views,  _id.index); }

                      MemoryMapping &bufferMapping(BufferId _id)       { return at_(chunk(_id.parent).buffer_maps, _id.index); }
                const MemoryMapping &bufferMapping(BufferId _id) const { return at_(chunk(_id.parent).buffer_maps, _id.index); }
                      MemoryMapping & imageMapping(ImageId  _id)       { return at_(chunk(_id.parent). image_maps, _id.index); }
                const MemoryMapping & imageMapping(ImageId  _id) const { return at_(chunk(_id.parent). image_maps, _id.index); }

            // Manage objects

                MemoryId addMemoryChunk(VkMemoryPropertyFlags _required_flags) {
                    MemoryChunk __chunk(cxt_core);
                    __chunk.required_flags = _required_flags;
                    return memory_chunks.add(__chunk);
                }

                // Destroys the chunk with all its buffers, images and views; their Ids become invalid
                void removeMemoryChunk(MemoryId _memory_id) {
                    if (!memory_chunks.contains(_memory_id)) return;
                    memory_chunks.get(_memory_id).destroyChunk();
                    memory_chunks.remove(_memory_id);
                }

                void allocateChunkMemory(MemoryId _memory_id) {
                    chunk(_memory_id).allocateChunkMemory(memory_properties.edit().memoryProperties);
                }

                BufferId     addChunkBuffer    (MemoryId _memory_id, uint32_t _buf_size = 0u) { return BufferId    (_memory_id, chunk(_memory_id).addBuffer(_buf_size)); }
                ImageId      addChunkImage     (MemoryId _memory_id, uint32_t _img_size = 0u) { return ImageId     (_memory_id, chunk(_memory_id).addImage (_img_size)); }
                BufferViewId addChunkBufferView(MemoryId _memory_id) { return BufferViewId(_memory_id, chunk(_memory_id).addBufferView()); }
                ImageViewId  addChunkImageView (MemoryId _memory_id) { return ImageViewId (_memory_id, chunk(_memory_id).addImageView ()); }

                void createChunkBuffer(BufferId _buffer_id) { buffer(_buffer_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get()); }
                void createChunkImage (ImageId  _image_id ) { image (_image_id ).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get()); }
                // Note: the view and the viewed resource may belong to different chunks
                void createChunkBufferView(BufferId _buffer_id, BufferViewId _buf_view_id) {
                    bufferView(_buf_view_id).createObject(cxt_core->device.handle, buffer(_buffer_id).handle, cxt_core->alloc_cb.get());
                }
                void createChunkImageView (ImageId  _image_id , ImageViewId  _img_view_id) {
                    imageView (_img_view_id).createObject(cxt_core->device.handle, image (_image_id ).handle, cxt_core->alloc_cb.get());
                }

                // Returns nullptr if the chunk isn't HOST_VISIBLE or isn't allocated yet
                void* getMappedChunk (MemoryId _memory_id) const { return chunk(_memory_id).mapped_ptr; }
                void* getMappedBuffer(BufferId _buffer_id) const {
                    void *__chunk_ptr = getMappedChunk(_buffer_id.parent);
                    return __chunk_ptr == nullptr ? nullptr : (uint8_t*)__chunk_ptr + bufferMapping(_buffer_id).offset;
                }
                void* getMappedImage (ImageId  _image_id ) const {
                    void *__chunk_ptr = getMappedChunk(_image_id.parent);
                    return __chunk_ptr == nullptr ? nullptr : (uint8_t*)__chunk_ptr + imageMapping(_image_id).offset;
                }

            // Scenarios

                void setWholeBufferCreateInfoStruct(BufferId _buffer_id, Span<QueueFamilyId> _qfamilies_id,
                                                    VkSharingMode _sharing_mode, VkBufferUsageFlags _usage_flags, VkDeviceSize _size_bytes = 0u) {
                    if (_size_bytes > 0u)
                        bufferMapping(_buffer_id).range = _size_bytes;

                    std::vector<VkfwQueueFamily> __qfamilies((size_t)_qfamilies_id.size());
                    for (uint32_t i = 0u; i < _qfamilies_id.size(); ++i)
                        __qfamilies[i] = cxt_core->queueFamily(_qfamilies_id[i]).handle;
                    buffer(_buffer_id).create_info.setWholeStruct(__qfamilies, _sharing_mode, _usage_flags, bufferMapping(_buffer_id).range);
                }

            private:
                template <typename T>
                static T& at_(std::vector<T> &_objs, uint32_t _index) {
                    SVKFW_ASSERT(_index < _objs.size(), std::out_of_range, "VKFW::ContextMemory", "object index is out of its memory chunk");
                    return _objs[_index];
                }
                template <typename T>
                static const T& at_(const std::vector<T> &_objs, uint32_t _index) {
                    SVKFW_ASSERT(_index < _objs.size(), std::out_of_range, "VKFW::ContextMemory", "object index is out of its memory chunk");
                    return _objs[_index];
                }
            } cxt_memory; // ContextMemory END

            struct ContextBarrier {
                Pool<VkSemaphore, SemaphoreTag> semaphores;
                Pool<VkFence,     FenceTag>         fences;
                Pool<VkEvent,     EventTag>         events;

                Struct::StructWrap<VkSemaphoreCreateInfo> semaphore_wrap;
                Struct::StructWrap<VkFenceCreateInfo>         fence_wrap;
                Struct::StructWrap<VkEventCreateInfo>         event_wrap;

                ContextCore *cxt_core = nullptr;

                ContextBarrier(ContextCore *_cxt_core) { cxt_core = _cxt_core; }
               ~ContextBarrier() { destroyContext(); }

                void createContext() {}

                void destroyContext() {
                    VkDevice                     __device = cxt_core->device.handle;
                    const VkAllocationCallbacks *__alloc_cb = cxt_core->alloc_cb.get();

                    semaphores.forEach([&](SemaphoreId, VkSemaphore &_semaphore) { vkDestroySemaphore(__device, _semaphore, __alloc_cb); });
                    fences    .forEach([&](FenceId,     VkFence     &_fence    ) { vkDestroyFence    (__device, _fence,     __alloc_cb); });
                    events    .forEach([&](EventId,     VkEvent     &_event    ) { vkDestroyEvent    (__device, _event,     __alloc_cb); });
                    semaphores.clear();
                    fences    .clear();
                    events    .clear();
                }

            // Manage Objects

                SemaphoreId addSemaphore() {
                    VkSemaphore __semaphore = VK_NULL_HANDLE;
                    if (vkCreateSemaphore(cxt_core->device.handle, &semaphore_wrap.get(), cxt_core->alloc_cb.get(), &__semaphore) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::ContextBarrier :: addSemaphore", "could not create semaphore"));
                    return semaphores.add(__semaphore);
                }
                FenceId addFence(bool _is_signaled) {
                    VkFence __fence = VK_NULL_HANDLE;
                    fence_wrap.setWholeStruct(_is_signaled);
                    if (vkCreateFence(cxt_core->device.handle, &fence_wrap.get(), cxt_core->alloc_cb.get(), &__fence) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::ContextBarrier :: addFence", "could not create fence"));
                    return fences.add(__fence);
                }
                EventId addEvent(bool _is_signaled) {
                    VkEvent __vk_event = VK_NULL_HANDLE;
                    event_wrap.setWholeStruct(_is_signaled);
                    if (vkCreateEvent(cxt_core->device.handle, &event_wrap.get(), cxt_core->alloc_cb.get(), &__vk_event) != VK_SUCCESS)
                        throw std::runtime_error(SVKFW_WRAPERR("VKFW::ContextBarrier :: addEvent", "could not create event"));
                    return events.add(__vk_event);
                }

                // Note: destroying a none or already destroyed Id does nothing
                void destroySemaphore(SemaphoreId _semaphore_id) {
                    if (!semaphores.contains(_semaphore_id)) return;
                    vkDestroySemaphore(cxt_core->device.handle, semaphores.get(_semaphore_id), cxt_core->alloc_cb.get());
                    semaphores.remove(_semaphore_id);
                }
                void destroyFence(FenceId _fence_id) {
                    if (!fences.contains(_fence_id)) return;
                    vkDestroyFence(cxt_core->device.handle, fences.get(_fence_id), cxt_core->alloc_cb.get());
                    fences.remove(_fence_id);
                }
                void destroyEvent(EventId _event_id) {
                    if (!events.contains(_event_id)) return;
                    vkDestroyEvent(cxt_core->device.handle, events.get(_event_id), cxt_core->alloc_cb.get());
                    events.remove(_event_id);
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
        // Note: in all getters a none Id gives VK_NULL_HANDLE (UINT32_MAX for a queue family), a stale Id throws

            VkInstance               getObjInstance()                         const { return cxt_ptr->cxt_core.instance.handle; }
            VkDebugUtilsMessengerEXT getObjDebugUtilsMessengerEXT()           const { return cxt_ptr->cxt_core.cxt_debug.debug_messenger.handle;}
            VkPhysicalDevice         getObjPhysicalDevice()                   const { return cxt_ptr->cxt_core.phys_device.handle; }
            VkDevice                 getObjDevice()                           const { return cxt_ptr->cxt_core.device.handle; }
            VkfwQueueFamily          getObjQueueFamily(QueueFamilyId _obj_id) const { return _obj_id.isNone() ? UINT32_MAX     : cxt_ptr->cxt_core.queueFamily(_obj_id).handle; }
            VkQueue                  getObjQueue      (QueueId       _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_core.queue      (_obj_id); }
            VkRenderPass             getObjRenderPass()                       const { return cxt_ptr->cxt_core.render_pass.handle; }
            VkAllocationCallbacks*   getSctAllocationCallbacks()                    { return cxt_ptr->cxt_core.alloc_cb.get(); }

            ContextEntity<Object::QueueFamily> shortcutObjQueueFamily(QueueFamilyId _obj_id) { return { &cxt_ptr->cxt_core.queueFamily(_obj_id) }; }
            ContextEntity<Object::Queue      > shortcutObjQueue      (QueueFamilyId _obj_id) { return { &cxt_ptr->cxt_core.queues     (_obj_id) }; }

            ContextEntity<Struct::StructWrap<VkAttachmentDescription>> shortcutSctRenderPassAttachment        (ContextIndex _sct_ci) { return cxt_ptr->cxt_core.render_pass.create_info.shortcutAttachment        (_sct_ci); }
            ContextEntity<Struct::StructWrap<   VkSubpassDescription>> shortcutSctRenderPassSubpassDescription(ContextIndex _sct_ci) { return cxt_ptr->cxt_core.render_pass.create_info.shortcutSubpassDescription(_sct_ci); }
            ContextEntity<Struct::StructWrap<    VkSubpassDependency>> shortcutSctRenderPassSubpassDependency (ContextIndex _sct_ci) { return cxt_ptr->cxt_core.render_pass.create_info.shortcutSubpassDependency (_sct_ci); }

        // Getters (Swapchain)
        // Note: swapchain images are indexed by the image index from 'vkAcquireNextImageKHR'

            VkSwapchainKHR getObjSwapchainKHR() const { return cxt_ptr->cxt_swapchain.swapchain.handle; }
            VkSurfaceKHR   getObjSurfaceKHR()   const { return cxt_ptr->cxt_swapchain.surface  .handle; }
            VkImageView    getObjSwapchainImageView  (ContextIndex _obj_ci) const { return shouldIgnoreCI(_obj_ci) ? VK_NULL_HANDLE : cxt_ptr->cxt_swapchain.image_views [_obj_ci]; }
            VkFramebuffer  getObjSwapchainFramebuffer(ContextIndex _obj_ci) const { return shouldIgnoreCI(_obj_ci) ? VK_NULL_HANDLE : cxt_ptr->cxt_swapchain.framebuffers[_obj_ci]; }

        // Getters (Descriptor)

            VkDescriptorSetLayout getObjDescriptorSetLayout(DescriptorSetLayoutId _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_descriptor.layouts.get(_obj_id).handle; }
            VkDescriptorPool      getObjDescriptorPool     (DescriptorPoolId      _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_descriptor.pools  .get(_obj_id).handle; }
            VkDescriptorSet       getObjDescriptorSet      (DescriptorSetId       _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_descriptor.set(_obj_id); }

            ContextEntity<Object::DescriptorSetLayout> shortcutObjDescriptorSetLayout(DescriptorSetLayoutId _obj_id) { return { &cxt_ptr->cxt_descriptor.layouts.get(_obj_id) }; }
            ContextEntity<Object::DescriptorPool     > shortcutObjDescriptorPool     (DescriptorPoolId      _obj_id) { return { &cxt_ptr->cxt_descriptor.pools  .get(_obj_id) }; }
            ContextEntity<Object::DescriptorSet      > shortcutObjDescriptorSet      (DescriptorSetsId      _obj_id) { return { &cxt_ptr->cxt_descriptor.sets   .get(_obj_id) }; }

            ContextEntity<Struct::StructWrap<VkDescriptorSetLayoutBinding>> shortcutSctDescriptorSetLayoutBinding(DescriptorLayoutBindingId _sct_id) {
                return { cxt_ptr->cxt_descriptor.layouts.get(_sct_id.parent).create_info.shortcutBinding(_sct_id.index).entity_ptr };
            }

        // Getters (Pipeline)

            VkPipeline       getObjPipeline      (PipelineId       _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_pipeline.pipelines.get(_obj_id).handle; }
            VkPipelineLayout getObjPipelineLayout(PipelineLayoutId _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_pipeline.layouts  .get(_obj_id).handle; }
            VkPipelineCache  getObjPipelineCache (PipelineCacheId  _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_pipeline.caches   .get(_obj_id).handle; }
            VkShaderModule   getObjShaderModule  (ShaderModuleId   _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_pipeline.shaders  .get(_obj_id).handle; }

            ContextEntity<Object::Pipeline      > shortcutObjPipeline      (PipelineId       _obj_id) { return { &cxt_ptr->cxt_pipeline.pipelines.get(_obj_id) }; }
            ContextEntity<Object::PipelineLayout> shortcutObjPipelineLayout(PipelineLayoutId _obj_id) { return { &cxt_ptr->cxt_pipeline.layouts  .get(_obj_id) }; }
            ContextEntity<Object::PipelineCache > shortcutObjPipelineCache (PipelineCacheId  _obj_id) { return { &cxt_ptr->cxt_pipeline.caches   .get(_obj_id) }; }
            ContextEntity<Object::ShaderModule  > shortcutObjShaderModule  (ShaderModuleId   _obj_id) { return { &cxt_ptr->cxt_pipeline.shaders  .get(_obj_id) }; }

            ContextEntity<Struct::StructWrap<VkPipelineShaderStageCreateInfo>> shortcutSctShaderStage(ShaderStageId _sct_id) {
                return { cxt_ptr->cxt_pipeline.pipelines.get(_sct_id.parent).shortcutShaderStage(_sct_id.index) };
            }

        // Getters (Command)

            VkCommandPool   getObjCommandPool  (CommandPoolId   _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_command.cmd_pools.get(_obj_id).handle; }
            VkCommandBuffer getObjCommandBuffer(CommandBufferId _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_command.commandBuffer(_obj_id); }

            ContextEntity<Object::CommandPool  > shortcutObjCommandPool  (CommandPoolId    _obj_id) { return { &cxt_ptr->cxt_command.cmd_pools  .get(_obj_id) }; }
            ContextEntity<Object::CommandBuffer> shortcutObjCommandBuffer(CommandBuffersId _obj_id) { return { &cxt_ptr->cxt_command.cmd_buffers.get(_obj_id) }; }

        // Getters (Device Memory)
        
            // Note: a none Id gives VK_NULL_HANDLE, an Id of a destroyed chunk throws
            VkDeviceMemory getObjDeviceMemory(MemoryId     _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_memory.chunk     (_obj_id).memory.handle; }
            VkBuffer       getObjBuffer      (BufferId     _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_memory.buffer    (_obj_id).handle; }
            VkImage        getObjImage       (ImageId      _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_memory.image     (_obj_id).handle; }
            VkBufferView   getObjBufferView  (BufferViewId _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_memory.bufferView(_obj_id).handle; }
            VkImageView    getObjImageView   (ImageViewId  _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_memory.imageView (_obj_id).handle; }

            VKFW::VulkanContext::ContextMemory::MemoryMapping getSctMemoryMapping(BufferId _obj_id) { return _obj_id.isNone() ? VKFW::VulkanContext::ContextMemory::MemoryMapping{ VK_WHOLE_SIZE, VK_WHOLE_SIZE } : cxt_ptr->cxt_memory.bufferMapping(_obj_id); }

            ContextEntity<VulkanContext::ContextMemory::MemoryChunk> shortcutSctMemoryChunk(MemoryId _obj_id) { return { &cxt_ptr->cxt_memory.chunk(_obj_id) }; }
            ContextEntity<Object::DeviceMemory> shortcutObjDeviceMemory(MemoryId     _obj_id) { return { &cxt_ptr->cxt_memory.chunk     (_obj_id).memory }; }
            ContextEntity<Object::Buffer      > shortcutObjBuffer      (BufferId     _obj_id) { return { &cxt_ptr->cxt_memory.buffer    (_obj_id) }; }
            ContextEntity<Object::Image       > shortcutObjImage       (ImageId      _obj_id) { return { &cxt_ptr->cxt_memory.image     (_obj_id) }; }
            ContextEntity<Object::BufferView  > shortcutObjBufferView  (BufferViewId _obj_id) { return { &cxt_ptr->cxt_memory.bufferView(_obj_id) }; }
            ContextEntity<Object::ImageView   > shortcutObjImageView   (ImageViewId  _obj_id) { return { &cxt_ptr->cxt_memory.imageView (_obj_id) }; }

        // Getters (Barrier)

            // Note: a none Id gives VK_NULL_HANDLE, an Id of a destroyed object throws
            VkSemaphore getObjSemaphore(SemaphoreId _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_barrier.semaphores.get(_obj_id); }
            VkFence     getObjFence    (FenceId     _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_barrier.fences    .get(_obj_id); }
            VkEvent     getObjEvent    (EventId     _obj_id) const { return _obj_id.isNone() ? VK_NULL_HANDLE : cxt_ptr->cxt_barrier.events    .get(_obj_id); }

        // Multiple object getters

        // Getters (Core)

            std::vector<VkfwQueueFamily> getObjsQueueFamily(Span<QueueFamilyId> _objs_id) const {
                std::vector<VkfwQueueFamily> __res(_objs_id.size(), UINT32_MAX);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_core.queueFamily(_objs_id[i]).handle;
                return __res;
            }

            std::vector<VkQueue> getObjsQueue(Span<QueueId> _objs_id) const {
                std::vector<VkQueue> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_core.queue(_objs_id[i]);
                return __res;
            }

        // Getters (Swapchain)

            std::vector<VkImageView> getObjsSwapchainImageView(Span<ContextIndex> _objs_ci) const {
                std::vector<VkImageView> __res(_objs_ci.size(), VK_NULL_HANDLE);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_swapchain.image_views[_objs_ci[ci]];
                return __res;
            }

            std::vector<VkFramebuffer> getObjsSwapchainFramebuffer(Span<ContextIndex> _objs_ci) const {
                std::vector<VkFramebuffer> __res(_objs_ci.size(), VK_NULL_HANDLE);
                for (uint32_t ci = 0u; ci < _objs_ci.size(); ++ci) __res[ci] = cxt_ptr->cxt_swapchain.framebuffers[_objs_ci[ci]];
                return __res;
            }

        // Getters (Descriptor)

            std::vector<VkDescriptorSetLayout> getObjsDescriptorSetLayout(Span<DescriptorSetLayoutId> _objs_id) const {
                std::vector<VkDescriptorSetLayout> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_descriptor.layouts.get(_objs_id[i]).handle;
                return __res;
            }

            std::vector<VkDescriptorPool> getObjsDescriptorPool(Span<DescriptorPoolId> _objs_id) const {
                std::vector<VkDescriptorPool> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_descriptor.pools.get(_objs_id[i]).handle;
                return __res;
            }

            std::vector<VkDescriptorSet> getObjsDescriptorSet(Span<DescriptorSetId> _objs_id) const {
                std::vector<VkDescriptorSet> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_descriptor.set(_objs_id[i]);
                return __res;
            }

        // Getters (Pipeline)

            std::vector<VkPipeline> getObjsPipeline(Span<PipelineId> _objs_id) const {
                std::vector<VkPipeline> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_pipeline.pipelines.get(_objs_id[i]).handle;
                return __res;
            }

            std::vector<VkPipelineCache> getObjsPipelineCache(Span<PipelineCacheId> _objs_id) const {
                std::vector<VkPipelineCache> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_pipeline.caches.get(_objs_id[i]).handle;
                return __res;
            }

            std::vector<VkPipelineLayout> getObjsPipelineLayout(Span<PipelineLayoutId> _objs_id) const {
                std::vector<VkPipelineLayout> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_pipeline.layouts.get(_objs_id[i]).handle;
                return __res;
            }

        // Getters (Command)

            std::vector<VkCommandPool> getObjsCommandPool(Span<CommandPoolId> _objs_id) const {
                std::vector<VkCommandPool> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_command.cmd_pools.get(_objs_id[i]).handle;
                return __res;
            }

            std::vector<VkCommandBuffer> getObjsCommandBuffer(Span<CommandBufferId> _objs_id) const {
                std::vector<VkCommandBuffer> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_command.commandBuffer(_objs_id[i]);
                return __res;
            }

        // Getters (Device Memory)

            std::vector<VkDeviceMemory> getObjsDeviceMemory(Span<MemoryId> _objs_id) const {
                std::vector<VkDeviceMemory> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_memory.chunk(_objs_id[i]).memory.handle;
                return __res;
            }

            std::vector<VkBuffer> getObjsBuffer(Span<BufferId> _objs_id) const {
                std::vector<VkBuffer> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_memory.buffer(_objs_id[i]).handle;
                return __res;
            }

            std::vector<VkImage> getObjsImage(Span<ImageId> _objs_id) const {
                std::vector<VkImage> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_memory.image(_objs_id[i]).handle;
                return __res;
            }

            std::vector<VkBufferView> getObjsBufferView(Span<BufferViewId> _objs_id) const {
                std::vector<VkBufferView> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_memory.bufferView(_objs_id[i]).handle;
                return __res;
            }

            std::vector<VkImageView> getObjsImageView(Span<ImageViewId> _objs_id) const {
                std::vector<VkImageView> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_memory.imageView(_objs_id[i]).handle;
                return __res;
            }

        // Getters (Barrier)

            std::vector<VkSemaphore> getObjsSemaphore(Span<SemaphoreId> _objs_id) const {
                std::vector<VkSemaphore> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_barrier.semaphores.get(_objs_id[i]);
                return __res;
            }

            std::vector<VkFence> getObjsFence(Span<FenceId> _objs_id) const {
                std::vector<VkFence> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_barrier.fences.get(_objs_id[i]);
                return __res;
            }

            std::vector<VkEvent> getObjsEvent(Span<EventId> _objs_id) const {
                std::vector<VkEvent> __res(_objs_id.size(), VK_NULL_HANDLE);
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) __res[i] = cxt_ptr->cxt_barrier.events.get(_objs_id[i]);
                return __res;
            }

        // Multiple object getters into an inline array (no memory allocation for up to VKFW_INLINE_ARRAY_SIZE objects)

            void fillObjsDescriptorSet(Span<DescriptorSetId> _objs_id, InlineArray<VkDescriptorSet> &_res) const {
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) _res[i] = cxt_ptr->cxt_descriptor.set(_objs_id[i]);
            }

            void fillObjsCommandBuffer(Span<CommandBufferId> _objs_id, InlineArray<VkCommandBuffer> &_res) const {
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) _res[i] = cxt_ptr->cxt_command.commandBuffer(_objs_id[i]);
            }

            void fillObjsBuffer(Span<BufferId> _objs_id, InlineArray<VkBuffer> &_res) const {
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) _res[i] = cxt_ptr->cxt_memory.buffer(_objs_id[i]).handle;
            }

            void fillObjsSemaphore(Span<SemaphoreId> _objs_id, InlineArray<VkSemaphore> &_res) const {
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) _res[i] = cxt_ptr->cxt_barrier.semaphores.get(_objs_id[i]);
            }

            void fillObjsFence(Span<FenceId> _objs_id, InlineArray<VkFence> &_res) const {
                for (uint32_t i = 0u; i < _objs_id.size(); ++i) _res[i] = cxt_ptr->cxt_barrier.fences.get(_objs_id[i]);
            }
        }; // ObjGetterHandler END


//  == === ==== >   VKFW Barrier Handler

        struct BarrierHandler {
            ObjGetterHandler cxt_handler;

            BarrierHandler(VKFW::VulkanContext *_cxt_ptr = &cxt_vkfw) : cxt_handler{_cxt_ptr} {}

        // Functions

            void WaitForFences(Span<FenceId> _fences_id, VkBool32 _wait_all = VK_TRUE, uint64_t _wait_time = UINT64_MAX) {
                InlineArray<VkFence> __fences(_fences_id.size());
                cxt_handler.fillObjsFence(_fences_id, __fences);
                Func::WaitForFences(cxt_handler.getObjDevice(), __fences, _wait_all, _wait_time);
            }

            void ResetFences(Span<FenceId> _fences_id) {
                InlineArray<VkFence> __fences(_fences_id.size());
                cxt_handler.fillObjsFence(_fences_id, __fences);
                Func::ResetFences(cxt_handler.getObjDevice(), __fences);
            }
        }; // BarrierHandler END


//  == === ==== >   VKFW Device Memory Map Handler

        struct MemoryMapHandler {
            std::pair<void *, MemoryId> mem_handle{nullptr, MemoryId()};
            VkDeviceSize mapped_offset = 0u; // offset of the mapped range in the device memory (offsets in 'buf_maps'/'img_maps' are relative to it)
            std::vector<std::pair<BufferId, VulkanContext::ContextMemory::MemoryMapping>> buf_maps;
            std::vector<std::pair<ImageId,  VulkanContext::ContextMemory::MemoryMapping>> img_maps;

            ObjGetterHandler cxt_handler;
            mutable VkDeviceSize non_coherent_atom_size = 0u; // loaded on demand

            MemoryMapHandler(VKFW::VulkanContext *_cxt_ptr = &cxt_vkfw) : cxt_handler{_cxt_ptr} {}
           ~MemoryMapHandler() {
                if (cxt_handler.getObjDevice() != VK_NULL_HANDLE) UnmapMemory();
                clear();
            }

            // Note: persistently mapped chunks (see 'MemoryChunk::mapped_ptr') are not mapped again, only the pointer is offset
            void MapMemory(MemoryId _memory_id, VkDeviceSize _offset = 0u, VkDeviceSize _size = VK_WHOLE_SIZE) {
                UnmapMemory(); // memory object can't be mapped twice
                void *__persistent_ptr = cxt_handler.cxt_ptr->cxt_memory.getMappedChunk(_memory_id);
                void *__mapped_ptr = __persistent_ptr != nullptr ? (uint8_t*)__persistent_ptr + _offset
                                                                 : Func::MapMemory(cxt_handler.getObjDevice(), cxt_handler.getObjDeviceMemory(_memory_id), _offset, _size);
                mem_handle = { __mapped_ptr, _memory_id };
                mapped_offset = _offset;
            }

            // Note: if the chunk was removed, its memory was already unmapped and freed
            void UnmapMemory() {
                if (cxt_handler.cxt_ptr->cxt_memory.memory_chunks.contains(mem_handle.second) && cxt_handler.cxt_ptr->cxt_memory.getMappedChunk(mem_handle.second) == nullptr)
                    Func::UnmapMemory(cxt_handler.getObjDevice(), cxt_handler.getObjDeviceMemory(mem_handle.second));
                mem_handle = {nullptr, MemoryId()};
                mapped_offset = 0u;
            }

            void clear() {
                mem_handle = {nullptr, MemoryId()};
                mapped_offset = 0u;
                buf_maps.clear();
                img_maps.clear();
            }

        // Memory Functions

            // Converts ranges relative to the mapped memory into absolute VkMappedMemoryRange aligned to 'nonCoherentAtomSize'
            std::vector<VkMappedMemoryRange> getMappedMemoryRanges(Span<VulkanContext::ContextMemory::MemoryMapping> _mem_ranges) const {
                std::vector<VkMappedMemoryRange> __mem_ranges(_mem_ranges.size());
                fillMappedMemoryRanges(_mem_ranges, __mem_ranges.data());
                return __mem_ranges;
            }

            void fillMappedMemoryRanges(Span<VulkanContext::ContextMemory::MemoryMapping> _mem_ranges, VkMappedMemoryRange *_res) const {
                if (non_coherent_atom_size == 0u) {
                    VkPhysicalDeviceProperties __properties{};
                    vkGetPhysicalDeviceProperties(cxt_handler.getObjPhysicalDevice(), &__properties);
                    non_coherent_atom_size = std::max(__properties.limits.nonCoherentAtomSize, VkDeviceSize(1u));
                }
                const VkDeviceSize __alloc_size = cxt_handler.cxt_ptr->cxt_memory.chunk(mem_handle.second).memory.allocate_info.peek().allocationSize;

                VkMappedMemoryRange *__mem_ranges = _res;
                for (uint32_t i = 0u; i < _mem_ranges.size(); ++i) {
                    __mem_ranges[i] = VkMappedMemoryRange{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
                    VkDeviceSize __offset = mapped_offset + _mem_ranges[i].offset;
                    VkDeviceSize __aligned_offset = __offset - __offset % non_coherent_atom_size;

                    __mem_ranges[i].memory = cxt_handler.getObjDeviceMemory(mem_handle.second);
                    __mem_ranges[i].offset = __aligned_offset;
                    __mem_ranges[i].size   = VK_WHOLE_SIZE;
                    if (_mem_ranges[i].range != VK_WHOLE_SIZE) {
                        VkDeviceSize __end = __offset + _mem_ranges[i].range;
                        __end += (non_coherent_atom_size - __end % non_coherent_atom_size) % non_coherent_atom_size;
                        if (__end < __alloc_size)
                            __mem_ranges[i].size = __end - __aligned_offset;
                    }
                }
            }

            // Note: offsets are relative to the start of the mapped memory
            void FlushMappedMemoryRanges(Span<VulkanContext::ContextMemory::MemoryMapping> _mem_ranges) const {
                InlineArray<VkMappedMemoryRange> __vk_ranges(_mem_ranges.size());
                fillMappedMemoryRanges(_mem_ranges, __vk_ranges.data());
                Func::FlushMappedMemoryRanges(cxt_handler.getObjDevice(), __vk_ranges);
            }

            // Note: offsets are relative to the start of the mapped memory
            void InvalidateMappedMemoryRanges(Span<VulkanContext::ContextMemory::MemoryMapping> _mem_ranges) const {
                InlineArray<VkMappedMemoryRange> __vk_ranges(_mem_ranges.size());
                fillMappedMemoryRanges(_mem_ranges, __vk_ranges.data());
                Func::InvalidateMappedMemoryRanges(cxt_handler.getObjDevice(), __vk_ranges);
            }

        // Scenarios

            void mapMemoryFromBuffer(BufferId _buffer_id, VkDeviceSize _offset = 0u, VkDeviceSize _size = VK_WHOLE_SIZE) {
                auto __buffer_mapping = cxt_handler.cxt_ptr->cxt_memory.bufferMapping(_buffer_id);
                SVKFW_ASSERT(_offset         <  __buffer_mapping.range, std::invalid_argument, "VKFW::MemoryMapHandle :: mapMemoryFromBuffer",          "'offset' is out of buffer range: " + std::to_string(_offset        ) + " >= " + std::to_string(__buffer_mapping.range));
                if (_size == VK_WHOLE_SIZE)
                    _size = __buffer_mapping.range - _offset;
                SVKFW_ASSERT(_offset + _size <= __buffer_mapping.range, std::invalid_argument, "VKFW::MemoryMapHandle :: mapMemoryFromBuffer", "'size' + 'offset' is out of buffer range: " + std::to_string(_offset + _size) + " > " + std::to_string(__buffer_mapping.range));

                __buffer_mapping.offset += _offset;
                __buffer_mapping.range   =   _size;

                MapMemory(_buffer_id.parent, __buffer_mapping.offset, __buffer_mapping.range);
            }

            void mapMemoryFromImage(ImageId _image_id, VkDeviceSize _offset = 0u, VkDeviceSize _size = VK_WHOLE_SIZE) {
                auto __image_mapping = cxt_handler.cxt_ptr->cxt_memory.imageMapping(_image_id);
                SVKFW_ASSERT(_offset         <  __image_mapping.range, std::invalid_argument, "VKFW::MemoryMapHandle :: mapMemoryFromImage",          "'offset' is out of image range: " + std::to_string(_offset        ) + " >= " + std::to_string(__image_mapping.range));
                if (_size == VK_WHOLE_SIZE)
                    _size = __image_mapping.range - _offset;
                SVKFW_ASSERT(_offset + _size <= __image_mapping.range, std::invalid_argument, "VKFW::MemoryMapHandle :: mapMemoryFromImage", "'size' + 'offset' is out of image range: " + std::to_string(_offset + _size) + " > " + std::to_string(__image_mapping.range));

                __image_mapping.offset += _offset;
                __image_mapping.range   =   _size;

                MapMemory(_image_id.parent, __image_mapping.offset, __image_mapping.range);
            }

            // Note: all resources must belong to the same memory chunk
            void mapMemoryFromResources(Span<BufferId> _bufs_id, Span<ImageId> _imgs_id) {
                SVKFW_ASSERT(!_bufs_id.empty() || !_imgs_id.empty(), std::invalid_argument, "VKFW::MemoryMapHandler :: mapMemoryFromResources",
                                "Both buffer and image arrays are empty");

                VkDeviceSize __mapping_begin = UINT64_MAX, __mapping_end = 0u;
                MemoryId __memory_id;
                if (!_bufs_id.empty()) __memory_id = _bufs_id[0].parent;
                if (!_imgs_id.empty()) __memory_id = _imgs_id[0].parent;

                buf_maps.resize(_bufs_id.size());
                for (uint32_t i = 0u; i < _bufs_id.size(); ++i) {
                    SVKFW_ASSERT(_bufs_id[i].parent == __memory_id, std::invalid_argument, "VKFW::MemoryMapHandler :: mapMemoryFromResources",
                                    "buffer memory chunk mismatch: " + std::to_string(_bufs_id[i].parent.index) + " and " + std::to_string(__memory_id.index));
                    buf_maps[i].first  = _bufs_id[i];
                    buf_maps[i].second = cxt_handler.cxt_ptr->cxt_memory.bufferMapping(_bufs_id[i]);
                    __mapping_begin = std::min(__mapping_begin, buf_maps[i].second.offset);
                    __mapping_end   = std::max(__mapping_end  , buf_maps[i].second.offset + buf_maps[i].second.range);
                }

                img_maps.resize(_imgs_id.size());
                for (uint32_t i = 0u; i < _imgs_id.size(); ++i) {
                    SVKFW_ASSERT(_imgs_id[i].parent == __memory_id, std::invalid_argument, "VKFW::MemoryMapHandler :: mapMemoryFromResources",
                                    "image memory chunk mismatch: " + std::to_string(_imgs_id[i].parent.index) + " and " + std::to_string(__memory_id.index) + (_bufs_id.empty() ? " (image)" : " (buffer)"));
                    img_maps[i].first  = _imgs_id[i];
                    img_maps[i].second = cxt_handler.cxt_ptr->cxt_memory.imageMapping(_imgs_id[i]);
                    __mapping_begin = std::min(__mapping_begin, img_maps[i].second.offset);
                    __mapping_end   = std::max(__mapping_end  , img_maps[i].second.offset + img_maps[i].second.range);
                }

                for (auto& buf_map : buf_maps)
                    buf_map.second.offset -= __mapping_begin;
                for (auto& img_map : img_maps)
                    img_map.second.offset -= __mapping_begin;

                MapMemory(__memory_id, __mapping_begin, __mapping_end - __mapping_begin);
            }

            // Returns false if the buffer isn't mapped
            bool copyToBufferMap(BufferId _buf_id, const void* _data_ptr, int64_t _mem_offset = 0, VkDeviceSize _mem_size = VK_WHOLE_SIZE) const {
                return copyToMap_(buf_maps, _buf_id, _data_ptr, _mem_offset, _mem_size, "VKFW::MemoryMapHandler :: copyToBufferMap");
            }

            // Returns false if the image isn't mapped
            bool copyToImageMap(ImageId _img_id, const void* _data_ptr, int64_t _mem_offset = 0, VkDeviceSize _mem_size = VK_WHOLE_SIZE) const {
                return copyToMap_(img_maps, _img_id, _data_ptr, _mem_offset, _mem_size, "VKFW::MemoryMapHandler :: copyToImageMap");
            }

            // Returns false if the buffer isn't mapped
            bool copyFromBufferMap(BufferId _buf_id, void* _data_ptr, int64_t _mem_offset = 0, VkDeviceSize _mem_size = VK_WHOLE_SIZE) const {
                return copyFromMap_(buf_maps, _buf_id, _data_ptr, _mem_offset, _mem_size, "VKFW::MemoryMapHandler :: copyFromBufferMap");
            }

            // Returns false if the image isn't mapped
            bool copyFromImageMap(ImageId _img_id, void* _data_ptr, int64_t _mem_offset = 0, VkDeviceSize _mem_size = VK_WHOLE_SIZE) const {
                return copyFromMap_(img_maps, _img_id, _data_ptr, _mem_offset, _mem_size, "VKFW::MemoryMapHandler :: copyFromImageMap");
            }

        private:
            bool isMemoryCoherent_() const {
                return cxt_handler.cxt_ptr->cxt_memory.chunk(mem_handle.second).required_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            }

            template <typename IdT>
            bool copyToMap_(const std::vector<std::pair<IdT, VulkanContext::ContextMemory::MemoryMapping>> &_maps, IdT _id, const void* _data_ptr, int64_t _mem_offset, VkDeviceSize _mem_size, const char *_where) const {
                for (const auto &map : _maps) {
                    if (map.first == _id) {
                        if (_mem_size == VK_WHOLE_SIZE) _mem_size = map.second.range - _mem_offset;
                        memcpy((uint8_t*)mem_handle.first + map.second.offset + _mem_offset, _data_ptr, _mem_size);
                        if (!isMemoryCoherent_())
                            FlushMappedMemoryRanges({ {map.second.offset + _mem_offset, _mem_size} });
                        return true;
                    }
                }
                fprintf(svkfwwarn, "%s: (%u, %u) in %s\n", SVKFW_WRAPWARN("VKFW::MemoryMapHandler", "resource is not mapped"), _id.parent.index, _id.index, _where);
                return false;
            }

            template <typename IdT>
            bool copyFromMap_(const std::vector<std::pair<IdT, VulkanContext::ContextMemory::MemoryMapping>> &_maps, IdT _id, void* _data_ptr, int64_t _mem_offset, VkDeviceSize _mem_size, const char *_where) const {
                for (const auto &map : _maps) {
                    if (map.first == _id) {
                        if (_mem_size == VK_WHOLE_SIZE) _mem_size = map.second.range - _mem_offset;
                        if (!isMemoryCoherent_())
                            InvalidateMappedMemoryRanges({ {map.second.offset + _mem_offset, _mem_size} });
                        memcpy(_data_ptr, (uint8_t*)mem_handle.first + map.second.offset + _mem_offset, _mem_size);
                        return true;
                    }
                }
                fprintf(svkfwwarn, "%s: (%u, %u) in %s\n", SVKFW_WRAPWARN("VKFW::MemoryMapHandler", "resource is not mapped"), _id.parent.index, _id.index, _where);
                return false;
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

            void setCmdBuffer(CommandBufferId _cmd_buffer_id) {
                cmd_buffer = cxt_handler.getObjCommandBuffer(_cmd_buffer_id);
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
                begin_render_pass_wrap.callFunction(cmd_buffer, _contents);
            }

            void CmdEndRenderPass() {
                vkCmdEndRenderPass(cmd_buffer);
            }

            void CmdBindDescriptorSets(PipelineLayoutId _pip_layout_id, VkPipelineBindPoint _pip_bind_pt, Span<DescriptorSetId> _descr_sets_id, Span<uint32_t> _dynamic_descr = {}) {
                InlineArray<VkDescriptorSet> __obj_descr_sets(_descr_sets_id.size());
                cxt_handler.fillObjsDescriptorSet(_descr_sets_id, __obj_descr_sets);
                vkCmdBindDescriptorSets(cmd_buffer, _pip_bind_pt, cxt_handler.getObjPipelineLayout(_pip_layout_id), 0u, __obj_descr_sets.size(), __obj_descr_sets.data(), _dynamic_descr.size(), _dynamic_descr.data());
            }

            void CmdBindIndexBuffer(BufferId _index_buffer_id, VkDeviceSize _offset = 0u, VkIndexType _index_type = VK_INDEX_TYPE_UINT32) {
                vkCmdBindIndexBuffer(cmd_buffer, cxt_handler.getObjBuffer(_index_buffer_id), _offset, _index_type);
            }

            void CmdBindPipeline(PipelineId _pipeline_id) {
                VkPipelineBindPoint __pipeline_type;
                switch (cxt_handler.cxt_ptr->cxt_pipeline.pipelines.get(_pipeline_id).pipeline_type) {
                    case VKFW_PIPELINE_TYPE_GRAPHICS   : __pipeline_type = VK_PIPELINE_BIND_POINT_GRAPHICS       ; break;
                    case VKFW_PIPELINE_TYPE_COMPUTE    : __pipeline_type = VK_PIPELINE_BIND_POINT_COMPUTE        ; break;
                    case VKFW_PIPELINE_TYPE_RAY_TRACING: __pipeline_type = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR; break;
                    default: throw std::runtime_error(SVKFW_WRAPERR("VKFW::CommandHandler :: CmdBindPipeline", "pipeline type is not set"));
                }
                vkCmdBindPipeline(cmd_buffer, __pipeline_type, cxt_handler.getObjPipeline(_pipeline_id));
            }

            // Note: missing offsets are set to 0
            void CmdBindVertexBuffers(Span<BufferId> _buffers_id, Span<VkDeviceSize> _offsets = {}) {
                InlineArray<VkBuffer>     __obj_buffers(_buffers_id.size());
                InlineArray<VkDeviceSize> __offsets    (_buffers_id.size());
                cxt_handler.fillObjsBuffer(_buffers_id, __obj_buffers);
                for (uint32_t i = 0u; i < __offsets.size(); ++i)
                    __offsets[i] = i < _offsets.size() ? _offsets[i] : 0u;

                vkCmdBindVertexBuffers(cmd_buffer, 0u, __obj_buffers.size(), __obj_buffers.data(), __offsets.data());
            }

            void CmdClearAttachments() {
                VkClearAttachment attach{};
                attach.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                attach.clearValue.color = {1.f, 0.f, 1.f, 1.f};
                attach.colorAttachment = 0u;
                VkClearRect rect{};
                rect.rect.offset = {0, 0};
                rect.rect.extent = cxt_handler.cxt_ptr->cxt_swapchain.swapchain.create_info.peek().imageExtent;
                rect.baseArrayLayer = 0;
                rect.layerCount = 1;

                vkCmdClearAttachments(cmd_buffer, 1, &attach, 1, &rect);
            }

            // void CmdClearColorImage() {
            //     vkCmdClearColorImage()
            // }

            void CmdCopyBuffer(BufferId _src_buffer_id, BufferId _dst_buffer_id, Span<VkBufferCopy> _copy_regions) {
                VkBuffer __src_buffer = cxt_handler.getObjBuffer(_src_buffer_id);
                VkBuffer __dst_buffer = cxt_handler.getObjBuffer(_dst_buffer_id);

                vkCmdCopyBuffer(cmd_buffer, __src_buffer, __dst_buffer, (uint32_t) _copy_regions.size(), _copy_regions.data());
            }

            void CmdCopyImage(ImageId _src_image_id, ImageId _dst_image_id, VkImageLayout _layout1, VkImageLayout _layout2, Span<VkImageCopy> _copy_regions) {
                VkImage __src_image = cxt_handler.getObjImage(_src_image_id);
                VkImage __dst_image = cxt_handler.getObjImage(_dst_image_id);

                vkCmdCopyImage(cmd_buffer, __src_image, _layout1, __dst_image, _layout2, (uint32_t) _copy_regions.size(), _copy_regions.data());
            }

            void CmdCopyBufferToImage(BufferId _src_buffer_id, ImageId _dst_image_id, VkImageLayout _layout, Span<VkBufferImageCopy> _copy_regions) {
                VkBuffer __src_buffer = cxt_handler.getObjBuffer(_src_buffer_id);
                VkImage  __dst_image  = cxt_handler.getObjImage ( _dst_image_id);

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

            void CmdSetScissor(Span<VkRect2D> _scissors) {
                vkCmdSetScissor(cmd_buffer, 0, _scissors.size(), _scissors.data());
            }

            void CmdSetViewport(Span<VkViewport> _viewports) {
                vkCmdSetViewport(cmd_buffer, 0, _viewports.size(), _viewports.data());
            }

        // Scenarios

            void setBeginRenderPassRenderPass() {
                begin_render_pass_wrap.wrap.edit().renderPass = cxt_handler.getObjRenderPass();
            }

            void setBeginRenderPassSwapchainRenderArea() {
                begin_render_pass_wrap.wrap.render_area_wrap.setOffset(vec2i(0));
                begin_render_pass_wrap.wrap.render_area_wrap.setExtent(cxt_handler.cxt_ptr->cxt_swapchain.swapchain.create_info.peek().imageExtent);
            }

            void setBeginRenderPassSwapchainFramebuffer(ContextIndex _ci_framebuffer) {
                begin_render_pass_wrap.wrap.edit().framebuffer = cxt_handler.getObjSwapchainFramebuffer(_ci_framebuffer);
            }

            void cmdSetViewportScissorFromSwapchainExtent() {
                VkViewport __swapchain_viewport{};
                VkRect2D   __swapchain_scissor {};

                __swapchain_scissor.extent  = cxt_handler.cxt_ptr->cxt_swapchain.swapchain.create_info.peek().imageExtent;
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

            std::vector<QueueId> wait_queues_for_swapchain; // queues to wait before recreating swapchain (graphics and present)

            ObjGetterHandler cxt_handler;

            FuncHandler(VKFW::VulkanContext *_cxt_ptr = &cxt_vkfw) : cxt_handler{_cxt_ptr} {}
           ~FuncHandler() {}


            void setQueuesToWait(Span<QueueId> _queues_id) {
                wait_queues_for_swapchain.assign(_queues_id.begin(), _queues_id.end());
            }

        // Functions

            void QueuePresentKHR(QueueId _queue_pres_id, uint64_t _recreate_wait_queues_timeout = UINT64_MAX) {
                if (queue_present_wrap.callFunction(cxt_handler.getObjQueue(_queue_pres_id)) == UINT32_MAX) {
                    waitForRequiredQueues(_recreate_wait_queues_timeout);
                    cxt_handler.cxt_ptr->cxt_swapchain.recreate();
                }
            }

            inline void QueueSubmit(QueueId _queue_id, Span<CommandBufferId> _cmdbufs_id, Span<uint32_t> _active_submits, FenceId _fence_id = FenceId()) {
                InlineArray<VkCommandBuffer> __obj_cmdbuf(_cmdbufs_id.size());
                cxt_handler.fillObjsCommandBuffer(_cmdbufs_id, __obj_cmdbuf);
                QueueSubmit(cxt_handler.getObjQueue(_queue_id), __obj_cmdbuf, _active_submits, _fence_id);
            }
            void QueueSubmit(VkQueue _obj_queue, Span<VkCommandBuffer> _obj_cmdbuf, Span<uint32_t> _active_submits, FenceId _fence_id = FenceId()) {
                // Note: 'assign' reuses the vector capacity, so there are no allocations after the first frames
                for (uint32_t i : _active_submits)
                    queue_submit_wrap.wraps[i].command_buffers.assign(_obj_cmdbuf.begin(), _obj_cmdbuf.end());

                queue_submit_wrap.setActiveSubmits(_active_submits);
                queue_submit_wrap.callFunction(_obj_queue, cxt_handler.getObjFence(_fence_id));
            }

            uint32_t AcquireNextImageKHR(SemaphoreId _semaphore_id, FenceId _fence_id = FenceId(), uint64_t _timeout = UINT64_MAX) {
                VkFence __fence = cxt_handler.getObjFence(_fence_id);
                uint32_t __image_index = UINT32_MAX;

                VkResult __result = vkAcquireNextImageKHR(cxt_handler.getObjDevice(), cxt_handler.getObjSwapchainKHR(),
                                                          _timeout, cxt_handler.getObjSemaphore(_semaphore_id), __fence, &__image_index);

                // Note: with VK_SUBOPTIMAL_KHR the image is acquired and the semaphore/fence will be signaled,
                //       so the frame is rendered as usual; the swapchain is recreated after presentation.
                if (__result == VK_ERROR_OUT_OF_DATE_KHR) {
                    waitForRequiredQueues(_timeout);
                    cxt_handler.cxt_ptr->cxt_swapchain.recreate();
                    __image_index = UINT32_MAX;
                }
                else if (__result != VK_SUCCESS && __result != VK_SUBOPTIMAL_KHR)
                    throw std::runtime_error(SVKFW_WRAPERR("VKFW::SwapchainHandler :: AcquireNextImageKHR", "Failed to acquire next swapchain image"));

                return __image_index;
            }

            void UpdateDescriptorSets() {
                update_descr_sets.callFunction(cxt_handler.getObjDevice());
            }
            
            uint32_t GetSwapchainImagesKHR() {
                return cxt_handler.cxt_ptr->cxt_swapchain.getSwapchainImageCount();
            }

        // Scenarios

            void waitForRequiredQueues(uint64_t _timeout = UINT64_MAX) {
                auto __obj_queues = cxt_handler.getObjsQueue(wait_queues_for_swapchain);
                for (auto queue : __obj_queues)  vkQueueWaitIdle(queue);
            }

            void setWholeQueuePresentStruct(Span<SemaphoreId> _wait_semaphores_id,
                                            Span<uint32_t> _image_indices, bool _use_results = false) {
                InlineArray<VkSemaphore> __obj_semaphores(_wait_semaphores_id.size());
                cxt_handler.fillObjsSemaphore(_wait_semaphores_id, __obj_semaphores);
                queue_present_wrap.wrap.setWholeStruct(__obj_semaphores, cxt_handler.getObjSwapchainKHR(), _image_indices, _use_results);
            }

            void writeBufferMemory(CommandHandler &_cmd_handler, QueueId _queue_transfer_id, BufferId _buffer_id, BufferId _staging_buffer_id, const void *_copy_data, size_t _copy_size, size_t _dst_offset = 0u) {
                if ((cxt_handler.cxt_ptr->cxt_memory.buffer(_buffer_id).create_info.peek().usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0u)
                    fprintf(svkfwwarn, "%s (%u, %u) %s\n", SVKFW_WRAPWARN("VKFW::CommandHandler :: writeBufferMemory", "Buffer with Id"), _buffer_id.parent.index, _buffer_id.index,
                                                                            "is not set as transfer operation destination point");

                if ((cxt_handler.cxt_ptr->cxt_memory.buffer(_staging_buffer_id).create_info.peek().usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0u)
                    fprintf(svkfwwarn, "%s (%u, %u) %s\n", SVKFW_WRAPWARN("VKFW::CommandHandler :: writeBufferMemory", "Staging buffer with Id"), _staging_buffer_id.parent.index, _staging_buffer_id.index,
                                                                            "is not set as transfer operation source point");

                if ((cxt_handler.cxt_ptr->cxt_memory.chunk(_staging_buffer_id.parent).required_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0u)
                    fprintf(svkfwwarn, "%s (%u, %u) %s\n", SVKFW_WRAPWARN("VKFW::CommandHandler :: writeBufferMemory", "Staging buffer with Id"), _staging_buffer_id.parent.index, _staging_buffer_id.index,
                                                                            "cannot be mapped to CPU");

                bool   __call_flush = (cxt_handler.cxt_ptr->cxt_memory.chunk(_staging_buffer_id.parent).required_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0u;
                size_t __staging_buffer_map_size = std::min(_copy_size, cxt_handler.cxt_ptr->cxt_memory.bufferMapping(_staging_buffer_id).range);
                BarrierHandler      __barrier_h{cxt_handler.cxt_ptr};
                MemoryMapHandler __buffer_map_h{cxt_handler.cxt_ptr};
                __buffer_map_h.mapMemoryFromBuffer(_staging_buffer_id, 0u, __staging_buffer_map_size);

                FenceId       __fence_id = cxt_handler.cxt_ptr->cxt_barrier.addFence(false);
                ContextIndex __submit_ci = queue_submit_wrap.wraps.size();

                queue_submit_wrap.wraps.emplace_back();
                queue_submit_wrap.wraps[__submit_ci].command_buffers = { _cmd_handler.cmd_buffer };
                queue_submit_wrap.wraps[__submit_ci].wait_stages = {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};

                VkMappedMemoryRange __mem_range{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
                __mem_range.memory = cxt_handler.getObjDeviceMemory(_staging_buffer_id.parent);
                VkBufferCopy __copy_region{};
                __copy_region.dstOffset = _dst_offset;

                while (_copy_size > 0u) {
                    size_t __curr_copy_size = std::min(__staging_buffer_map_size, _copy_size);
                    __copy_region.size = __curr_copy_size;
                    _copy_size -= __curr_copy_size;

                    _cmd_handler.BeginCommandBuffer();

                    memcpy(__buffer_map_h.mem_handle.first, (uint8_t*)_copy_data + (__copy_region.dstOffset - _dst_offset), __curr_copy_size);
                    if (__call_flush) __buffer_map_h.FlushMappedMemoryRanges({ { 0u, __curr_copy_size } });
                    _cmd_handler.CmdCopyBuffer(_staging_buffer_id, _buffer_id, { __copy_region });

                    _cmd_handler.EndCommandBuffer();

                    QueueSubmit(cxt_handler.getObjQueue(_queue_transfer_id), {_cmd_handler.cmd_buffer}, {__submit_ci}, __fence_id);
                    __barrier_h.WaitForFences({__fence_id});
                    __barrier_h.  ResetFences({__fence_id});

                    _cmd_handler.ResetCommandBuffer();
                    __copy_region.dstOffset += __curr_copy_size;
                }
                cxt_handler.cxt_ptr->cxt_barrier.destroyFence(__fence_id);
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
            //     FenceId           __fence_ci = __barrier_h.cxt_handler.cxt_ptr->cxt_barrier.addFence(false);
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