#ifndef SVKFW_VKFW_H
#define SVKFW_VKFW_H

#include "vkfw_wrap.h"


namespace Simple {
    namespace VKFW {

//  == === ==== ====================== ==== === ==  \\
                    Vulkan Context
//  == === ==== ====================== ==== === ==  \\

        // Note: layer 2 only reports the swapchain state, the caller decides when to recreate it
        enum SwapchainStatus {
            VKFW_SWAPCHAIN_OK = 0,
            VKFW_SWAPCHAIN_SUBOPTIMAL,   // image is acquired/presented, but the swapchain should be recreated
            VKFW_SWAPCHAIN_OUT_OF_DATE,  // nothing was acquired/presented, the swapchain must be recreated
            VKFW_SWAPCHAIN_NOT_READY     // acquisition timed out, try again later
        }; // SwapchainStatus END

        struct AcquireResult {
            SwapchainStatus status;
            uint32_t   image_index; // UINT32_MAX if no image was acquired
        }; // AcquireResult END

        #define VKFW_MAX_SUBMIT_SEMAPHORES      8u
        #define VKFW_MAX_SUBMIT_COMMAND_BUFFERS 8u

        // Note: one VkSubmitInfo described by Ids; stored inline, so it can be filled every frame without allocations
        struct SubmitDesc {
            SemaphoreId          wait_semaphores  [VKFW_MAX_SUBMIT_SEMAPHORES];
            VkPipelineStageFlags wait_stages      [VKFW_MAX_SUBMIT_SEMAPHORES];
            CommandBufferId      command_buffers  [VKFW_MAX_SUBMIT_COMMAND_BUFFERS];
            SemaphoreId          signal_semaphores[VKFW_MAX_SUBMIT_SEMAPHORES];
            uint32_t wait_count = 0u, command_count = 0u, signal_count = 0u;

            SubmitDesc& wait(SemaphoreId _semaphore_id, VkPipelineStageFlags _stage) {
                SVKFW_ASSERT(wait_count < VKFW_MAX_SUBMIT_SEMAPHORES, std::length_error, "VKFW::SubmitDesc :: wait", "too many wait semaphores");
                wait_semaphores[wait_count] = _semaphore_id;
                wait_stages    [wait_count] = _stage;
                ++wait_count;
                return *this;
            }
            SubmitDesc& commands(CommandBufferId _cmd_buffer_id) {
                SVKFW_ASSERT(command_count < VKFW_MAX_SUBMIT_COMMAND_BUFFERS, std::length_error, "VKFW::SubmitDesc :: commands", "too many command buffers");
                command_buffers[command_count++] = _cmd_buffer_id;
                return *this;
            }
            SubmitDesc& signal(SemaphoreId _semaphore_id) {
                SVKFW_ASSERT(signal_count < VKFW_MAX_SUBMIT_SEMAPHORES, std::length_error, "VKFW::SubmitDesc :: signal", "too many signal semaphores");
                signal_semaphores[signal_count++] = _semaphore_id;
                return *this;
            }
            void clear() { wait_count = command_count = signal_count = 0u; }
        }; // SubmitDesc END

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

                      Object::QueueFamily &queueFamily(QueueFamilyId _id)       { return queue_families.get(_id).family; }
                const Object::QueueFamily &queueFamily(QueueFamilyId _id) const { return queue_families.get(_id).family; }
                      Object::Queue       &queues     (QueueFamilyId _id)       { return queue_families.get(_id).queues; }
                VkQueue queue(QueueId _id) const {
                    const std::vector<VkQueue> &__handles = queue_families.get(_id.parent).queues.handles;
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

                uint32_t getSwapchainImageCount() const {
                    return image_views.size();
                }

                // Note: swapchain images are indexed by the image index from 'acquire'
                VkFramebuffer framebuffer(uint32_t _image_index) const {
                    SVKFW_ASSERT(_image_index < framebuffers.size(), std::out_of_range, "VKFW::ContextSwapchain :: framebuffer", "image index is out of the swapchain");
                    return framebuffers[_image_index];
                }
                VkImageView imageView(uint32_t _image_index) const {
                    SVKFW_ASSERT(_image_index < image_views.size(), std::out_of_range, "VKFW::ContextSwapchain :: imageView", "image index is out of the swapchain");
                    return image_views[_image_index];
                }
                VkExtent2D extent() const {
                    return swapchain.create_info.peek().imageExtent;
                }

            // Functions

                AcquireResult acquire(VkSemaphore _semaphore, VkFence _fence = VK_NULL_HANDLE, uint64_t _timeout = UINT64_MAX) {
                    AcquireResult __res = { VKFW_SWAPCHAIN_OK, UINT32_MAX };
                    VkResult __vk_res = vkAcquireNextImageKHR(cxt_core->device.handle, swapchain.handle, _timeout, _semaphore, _fence, &__res.image_index);
                    switch (__vk_res) {
                        case VK_SUCCESS: break;
                        // Note: the image is acquired and the semaphore/fence will be signaled, so the frame can be rendered as usual
                        case VK_SUBOPTIMAL_KHR:        __res.status = VKFW_SWAPCHAIN_SUBOPTIMAL; break;
                        case VK_ERROR_OUT_OF_DATE_KHR: __res.status = VKFW_SWAPCHAIN_OUT_OF_DATE; __res.image_index = UINT32_MAX; break;
                        case VK_TIMEOUT:
                        case VK_NOT_READY:             __res.status = VKFW_SWAPCHAIN_NOT_READY;   __res.image_index = UINT32_MAX; break;
                        default: throw std::runtime_error(SVKFW_WRAPERR("VKFW::ContextSwapchain :: acquire", "failed to acquire next swapchain image, code ") + std::to_string(__vk_res));
                    }
                    return __res;
                }

                SwapchainStatus present(VkQueue _queue, Span<VkSemaphore> _wait_semaphores, uint32_t _image_index) {
                    VkPresentInfoKHR __info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
                    __info.waitSemaphoreCount = _wait_semaphores.size();
                    __info.pWaitSemaphores    = _wait_semaphores.data();
                    __info.swapchainCount     = 1u;
                    __info.pSwapchains        = &swapchain.handle;
                    __info.pImageIndices      = &_image_index;

                    VkResult __vk_res = vkQueuePresentKHR(_queue, &__info);
                    switch (__vk_res) {
                        case VK_SUCCESS:               return VKFW_SWAPCHAIN_OK;
                        case VK_SUBOPTIMAL_KHR:        return VKFW_SWAPCHAIN_SUBOPTIMAL;
                        case VK_ERROR_OUT_OF_DATE_KHR: return VKFW_SWAPCHAIN_OUT_OF_DATE;
                        default: throw std::runtime_error(SVKFW_WRAPERR("VKFW::ContextSwapchain :: present", "failed to present, code ") + std::to_string(__vk_res));
                    }
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

                // Note: the caller must make sure the swapchain images are not in use (see 'VulkanContext::recreateSwapchain')
                //       Blocks while the window is minimized (zero extent)
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

                VkDescriptorSet set(DescriptorSetId _id) const {
                    const std::vector<VkDescriptorSet> &__handles = sets.get(_id.parent).handles;
                    SVKFW_ASSERT(_id.index < __handles.size(), std::out_of_range, "VKFW::ContextDescriptor :: set", "set index is out of its group");
                    return __handles[_id.index];
                }

                // Note: references are invalidated when an object of the same kind is added
                Object::DescriptorSetLayout &layout  (DescriptorSetLayoutId _id) { return layouts.get(_id); }
                Object::DescriptorPool      &pool    (DescriptorPoolId      _id) { return pools  .get(_id); }
                Object::DescriptorSet       &setGroup(DescriptorSetsId      _id) { return sets   .get(_id); }
                Struct::StructWrap<VkDescriptorSetLayoutBinding> &layoutBinding(DescriptorLayoutBindingId _id) {
                    Object::DescriptorSetLayout &__layout = layouts.get(_id.parent);
                    SVKFW_ASSERT(_id.index < __layout.create_info.bindings_wrap.size(), std::out_of_range, "VKFW::ContextDescriptor :: layoutBinding", "binding index is out of its layout");
                    return __layout.create_info.bindings_wrap[_id.index];
                }

            // Functions

                void updateSets(Span<VkWriteDescriptorSet> _writes, Span<VkCopyDescriptorSet> _copies = {}) {
                    vkUpdateDescriptorSets(cxt_core->device.handle, _writes.size(), _writes.data(), _copies.size(), _copies.data());
                }
                void updateSets(std::vector<Struct::StructWrap<VkWriteDescriptorSet>> &_writes) {
                    InlineArray<VkWriteDescriptorSet> __writes(_writes.size());
                    for (uint32_t i = 0u; i < __writes.size(); ++i)
                        __writes[i] = _writes[i].get();
                    updateSets(__writes);
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

            // Access by Id (throws if the Id is none or stale)
            // Note: references are invalidated when an object of the same kind is added

                Object::Pipeline       &pipeline(PipelineId       _id) { return pipelines.get(_id); }
                Object::PipelineLayout &layout  (PipelineLayoutId _id) { return layouts  .get(_id); }
                Object::PipelineCache  &cache   (PipelineCacheId  _id) { return caches   .get(_id); }
                Object::ShaderModule   &shader  (ShaderModuleId   _id) { return shaders  .get(_id); }
                Struct::StructWrap<VkPipelineShaderStageCreateInfo> &shaderStage(ShaderStageId _id) {
                    return *pipelines.get(_id.parent).shortcutShaderStage(_id.index);
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

                VkCommandBuffer commandBuffer(CommandBufferId _id) const {
                    const std::vector<VkCommandBuffer> &__handles = cmd_buffers.get(_id.parent).handles;
                    SVKFW_ASSERT(_id.index < __handles.size(), std::out_of_range, "VKFW::ContextCommand :: commandBuffer", "command buffer index is out of its group");
                    return __handles[_id.index];
                }

                Object::CommandPool   &cmdPool       (CommandPoolId    _id) { return cmd_pools  .get(_id); }
                Object::CommandBuffer &cmdBufferGroup(CommandBuffersId _id) { return cmd_buffers.get(_id); }

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
                VkDeviceSize non_coherent_atom_size = 1u; // loaded in 'initContext'

                ContextCore *cxt_core = nullptr;

                ContextMemory(ContextCore *_cxt_core) { cxt_core = _cxt_core; }
               ~ContextMemory() { destroyContext(); }

                void initContext() {
                    memory_properties.loadForPhysicalDevice(cxt_core->phys_device.handle);

                    VkPhysicalDeviceProperties __properties{};
                    vkGetPhysicalDeviceProperties(cxt_core->phys_device.handle, &__properties);
                    non_coherent_atom_size = std::max(__properties.limits.nonCoherentAtomSize, VkDeviceSize(1u));
                }

                void destroyContext() {
                    memory_chunks.forEach([](MemoryId, MemoryChunk &_chunk) { _chunk.destroyChunk(); });
                    memory_chunks.clear();
                }

            // Access by Id (throws if the Id is none, stale or out of its chunk)

                      MemoryChunk &chunk(MemoryId _id)       { return memory_chunks.get(_id); }
                const MemoryChunk &chunk(MemoryId _id) const { return memory_chunks.get(_id); }

                      Object::Buffer     &buffer    (BufferId     _id)       { return at_(chunk(_id.parent).buffers,      _id.index); }
                const Object::Buffer     &buffer    (BufferId     _id) const { return at_(chunk(_id.parent).buffers,      _id.index); }
                      Object::Image      &image     (ImageId      _id)       { return at_(chunk(_id.parent).images,       _id.index); }
                const Object::Image      &image     (ImageId      _id) const { return at_(chunk(_id.parent).images,       _id.index); }
                      Object::BufferView &bufferView(BufferViewId _id)       { return at_(chunk(_id.parent).buffer_views, _id.index); }
                const Object::BufferView &bufferView(BufferViewId _id) const { return at_(chunk(_id.parent).buffer_views, _id.index); }
                      Object::ImageView  &imageView (ImageViewId  _id)       { return at_(chunk(_id.parent).image_views,  _id.index); }
                const Object::ImageView  &imageView (ImageViewId  _id) const { return at_(chunk(_id.parent).image_views,  _id.index); }

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

                bool isChunkCoherent(MemoryId _memory_id) const {
                    return (chunk(_memory_id).required_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0u;
                }

            // Functions (persistently mapped memory)
            // Note: offsets are relative to the buffer; flush/invalidate do nothing for HOST_COHERENT memory

                void flushBuffer(BufferId _buffer_id, VkDeviceSize _offset = 0u, VkDeviceSize _size = VK_WHOLE_SIZE) const {
                    if (isChunkCoherent(_buffer_id.parent)) return;
                    VkMappedMemoryRange __range = mappedRange_(_buffer_id, _offset, _size);
                    SVKFW_ASSERT(vkFlushMappedMemoryRanges(cxt_core->device.handle, 1u, &__range) == VK_SUCCESS, std::runtime_error,
                                    "VKFW::ContextMemory :: flushBuffer", "could not flush mapped memory");
                }
                void invalidateBuffer(BufferId _buffer_id, VkDeviceSize _offset = 0u, VkDeviceSize _size = VK_WHOLE_SIZE) const {
                    if (isChunkCoherent(_buffer_id.parent)) return;
                    VkMappedMemoryRange __range = mappedRange_(_buffer_id, _offset, _size);
                    SVKFW_ASSERT(vkInvalidateMappedMemoryRanges(cxt_core->device.handle, 1u, &__range) == VK_SUCCESS, std::runtime_error,
                                    "VKFW::ContextMemory :: invalidateBuffer", "could not invalidate mapped memory");
                }

                // Copies CPU data into a HOST_VISIBLE buffer and flushes it if needed (the GPU must not be using this range)
                void writeBuffer(BufferId _buffer_id, const void *_data, VkDeviceSize _size, VkDeviceSize _offset = 0u) const {
                    uint8_t *__ptr = (uint8_t*)checkMappedRange_(_buffer_id, _offset, _size, "VKFW::ContextMemory :: writeBuffer");
                    memcpy(__ptr + _offset, _data, _size);
                    flushBuffer(_buffer_id, _offset, _size);
                }
                // Invalidates the range if needed and copies it from a HOST_VISIBLE buffer into CPU memory
                void readBuffer(BufferId _buffer_id, void *_data, VkDeviceSize _size, VkDeviceSize _offset = 0u) const {
                    const uint8_t *__ptr = (const uint8_t*)checkMappedRange_(_buffer_id, _offset, _size, "VKFW::ContextMemory :: readBuffer");
                    invalidateBuffer(_buffer_id, _offset, _size);
                    memcpy(_data, __ptr + _offset, _size);
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
                void* checkMappedRange_(BufferId _buffer_id, VkDeviceSize _offset, VkDeviceSize _size, const char *_where) const {
                    void *__ptr = getMappedBuffer(_buffer_id);
                    SVKFW_ASSERT(__ptr != nullptr, std::invalid_argument, "VKFW::ContextMemory", "buffer memory is not HOST_VISIBLE or not allocated, in " + std::string(_where));
                    SVKFW_ASSERT(_offset + _size <= bufferMapping(_buffer_id).range, std::out_of_range, "VKFW::ContextMemory",
                                    "range is out of the buffer: " + std::to_string(_offset + _size) + " > " + std::to_string(bufferMapping(_buffer_id).range) + ", in " + _where);
                    return __ptr;
                }

                // Note: VkMappedMemoryRange offset and size must be multiples of 'nonCoherentAtomSize' (or the size must reach the allocation end)
                VkMappedMemoryRange mappedRange_(BufferId _buffer_id, VkDeviceSize _offset, VkDeviceSize _size) const {
                    const MemoryChunk &__chunk = chunk(_buffer_id.parent);
                    const MemoryMapping &__mapping = bufferMapping(_buffer_id);
                    const VkDeviceSize __alloc_size = __chunk.memory.allocate_info.peek().allocationSize;

                    VkMappedMemoryRange __range{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
                    VkDeviceSize __begin = __mapping.offset + _offset;
                    VkDeviceSize __end   = _size == VK_WHOLE_SIZE ? __mapping.offset + __mapping.range : __begin + _size;
                    __end += (non_coherent_atom_size - __end % non_coherent_atom_size) % non_coherent_atom_size;

                    __range.memory = __chunk.memory.handle;
                    __range.offset = __begin - __begin % non_coherent_atom_size;
                    __range.size   = __end < __alloc_size ? __end - __range.offset : VK_WHOLE_SIZE;
                    return __range;
                }

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

            // Functions

                // Returns VK_SUCCESS or VK_TIMEOUT, throws on errors
                VkResult wait(Span<FenceId> _fences_id, bool _wait_all = true, uint64_t _timeout = UINT64_MAX) const {
                    InlineArray<VkFence> __fences(_fences_id.size());
                    for (uint32_t i = 0u; i < __fences.size(); ++i) __fences[i] = fences.get(_fences_id[i]);
                    VkResult __res = vkWaitForFences(cxt_core->device.handle, __fences.size(), __fences.data(), _wait_all ? VK_TRUE : VK_FALSE, _timeout);
                    SVKFW_ASSERT(__res == VK_SUCCESS || __res == VK_TIMEOUT, std::runtime_error, "VKFW::ContextBarrier :: wait", "could not wait for fences, code " + std::to_string(__res));
                    return __res;
                }
                void reset(Span<FenceId> _fences_id) const {
                    InlineArray<VkFence> __fences(_fences_id.size());
                    for (uint32_t i = 0u; i < __fences.size(); ++i) __fences[i] = fences.get(_fences_id[i]);
                    SVKFW_ASSERT(vkResetFences(cxt_core->device.handle, __fences.size(), __fences.data()) == VK_SUCCESS, std::runtime_error,
                                    "VKFW::ContextBarrier :: reset", "could not reset fences");
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

        // Handles by Id
        // Note: a none Id gives VK_NULL_HANDLE (UINT32_MAX for a queue family), a stale Id throws

            VkfwQueueFamily       get(QueueFamilyId         _id) const { return _id.isNone() ? UINT32_MAX     : cxt_core.queueFamily(_id).handle; }
            VkQueue               get(QueueId               _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_core.queue(_id); }

            VkDescriptorSetLayout get(DescriptorSetLayoutId _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_descriptor.layouts.get(_id).handle; }
            VkDescriptorPool      get(DescriptorPoolId      _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_descriptor.pools  .get(_id).handle; }
            VkDescriptorSet       get(DescriptorSetId       _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_descriptor.set(_id); }

            VkPipeline            get(PipelineId            _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_pipeline.pipelines.get(_id).handle; }
            VkPipelineLayout      get(PipelineLayoutId      _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_pipeline.layouts  .get(_id).handle; }
            VkPipelineCache       get(PipelineCacheId       _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_pipeline.caches   .get(_id).handle; }
            VkShaderModule        get(ShaderModuleId        _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_pipeline.shaders  .get(_id).handle; }

            VkCommandPool         get(CommandPoolId         _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_command.cmd_pools.get(_id).handle; }
            VkCommandBuffer       get(CommandBufferId       _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_command.commandBuffer(_id); }

            VkDeviceMemory        get(MemoryId              _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_memory.chunk(_id).memory.handle; }
            VkBuffer              get(BufferId              _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_memory.buffer    (_id).handle; }
            VkImage               get(ImageId               _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_memory.image     (_id).handle; }
            VkBufferView          get(BufferViewId          _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_memory.bufferView(_id).handle; }
            VkImageView           get(ImageViewId           _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_memory.imageView (_id).handle; }

            VkSemaphore           get(SemaphoreId           _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_barrier.semaphores.get(_id); }
            VkFence               get(FenceId               _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_barrier.fences    .get(_id); }
            VkEvent               get(EventId               _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_barrier.events    .get(_id); }

            // Writes the handles of '_ids' into '_res' (must hold at least '_ids.size()' elements)
            template <typename IdT, typename HandleT>
            void fill(Span<IdT> _ids, HandleT *_res) const {
                for (uint32_t i = 0u; i < _ids.size(); ++i) _res[i] = get(_ids[i]);
            }

        // Functions

            void submit(QueueId _queue_id, Span<SubmitDesc> _submits, FenceId _fence_id = FenceId()) const {
                struct Handles {
                    VkSemaphore     wait  [VKFW_MAX_SUBMIT_SEMAPHORES];
                    VkCommandBuffer cmd   [VKFW_MAX_SUBMIT_COMMAND_BUFFERS];
                    VkSemaphore     signal[VKFW_MAX_SUBMIT_SEMAPHORES];
                };
                InlineArray<Handles,      4u> __handles(_submits.size());
                InlineArray<VkSubmitInfo, 4u> __infos  (_submits.size());

                for (uint32_t i = 0u; i < _submits.size(); ++i) {
                    const SubmitDesc &__desc = _submits[i];
                    fill(Span<SemaphoreId    >(__desc.wait_semaphores,   __desc.wait_count   ), __handles[i].wait  );
                    fill(Span<CommandBufferId>(__desc.command_buffers,   __desc.command_count), __handles[i].cmd   );
                    fill(Span<SemaphoreId    >(__desc.signal_semaphores, __desc.signal_count ), __handles[i].signal);

                    __infos[i] = VkSubmitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
                    __infos[i].waitSemaphoreCount   = __desc.wait_count;
                    __infos[i].pWaitSemaphores      = __handles[i].wait;
                    __infos[i].pWaitDstStageMask    = __desc.wait_stages;
                    __infos[i].commandBufferCount   = __desc.command_count;
                    __infos[i].pCommandBuffers      = __handles[i].cmd;
                    __infos[i].signalSemaphoreCount = __desc.signal_count;
                    __infos[i].pSignalSemaphores    = __handles[i].signal;
                }

                VkResult __res = vkQueueSubmit(get(_queue_id), __infos.size(), __infos.data(), get(_fence_id));
                SVKFW_ASSERT(__res == VK_SUCCESS, std::runtime_error, "VKFW::VulkanContext :: submit", "could not submit to queue, code " + std::to_string(__res));
            }

            AcquireResult acquireNextImage(SemaphoreId _semaphore_id, FenceId _fence_id = FenceId(), uint64_t _timeout = UINT64_MAX) {
                return cxt_swapchain.acquire(get(_semaphore_id), get(_fence_id), _timeout);
            }

            SwapchainStatus present(QueueId _queue_id, Span<SemaphoreId> _wait_semaphores_id, uint32_t _image_index) {
                InlineArray<VkSemaphore> __semaphores(_wait_semaphores_id.size());
                fill(_wait_semaphores_id, __semaphores.data());
                return cxt_swapchain.present(get(_queue_id), __semaphores, _image_index);
            }

            // Note: waits until the device is idle, so it is meant for rare events (resize, OUT_OF_DATE), not for every frame
            void recreateSwapchain() {
                vkDeviceWaitIdle(cxt_core.device.handle);
                cxt_swapchain.recreate();
            }

        // Scenarios

            void initNoncoreContexts() {
                cxt_memory.initContext();
            }

            void handleResult(VkResult _result) {
                // TODO: handle VkResult
            }

        private:
            // Note: contexts keep pointers to each other, so a copy would point into the original
            VulkanContext(const VulkanContext &);
            VulkanContext& operator=(const VulkanContext &);
        }; // VulkanContext END



//  == === ==== ================================== ==== === ==  \\
                    Vulkan Command Recorder
//  == === ==== ================================== ==== === ==  \\

        // Note: a light object for the stack: one command buffer plus the context to resolve Ids;
        //       every command converts Ids into handles on the stack, so recording doesn't allocate
        struct CommandRecorder {
            const VulkanContext &cxt;
            VkCommandBuffer      cmd = VK_NULL_HANDLE;

            CommandRecorder(const VulkanContext &_cxt, CommandBufferId _cmd_buffer_id) : cxt(_cxt), cmd{_cxt.get(_cmd_buffer_id)} {}
            CommandRecorder(const VulkanContext &_cxt, VkCommandBuffer _cmd_buffer   ) : cxt(_cxt), cmd{_cmd_buffer} {}

            VkCommandBuffer handle() const { return cmd; }

        // Command buffer

            void begin(VkCommandBufferUsageFlags _flags = 0u, const VkCommandBufferInheritanceInfo *_inheritance = nullptr) {
                VkCommandBufferBeginInfo __info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
                __info.flags            = _flags;
                __info.pInheritanceInfo = _inheritance;
                Func::BeginCommandBuffer(cmd, &__info);
            }
            void end() {
                SVKFW_ASSERT(vkEndCommandBuffer(cmd) == VK_SUCCESS, std::runtime_error, "VKFW::CommandRecorder :: end", "could not end command buffer");
            }
            // Note: the pool must be created with VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
            void reset(bool _release_resources = false) {
                vkResetCommandBuffer(cmd, _release_resources ? VkCommandBufferResetFlags(VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) : VkCommandBufferResetFlags(0u));
            }

        // Render pass

            // Note: uses the render pass of the core context, render area starts at (0, 0)
            void beginRenderPass(VkFramebuffer _framebuffer, VkExtent2D _extent, Span<VkClearValue> _clear_values = {},
                                 VkSubpassContents _contents = VK_SUBPASS_CONTENTS_INLINE) {
                VkRenderPassBeginInfo __info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
                __info.renderPass        = cxt.cxt_core.render_pass.handle;
                __info.framebuffer       = _framebuffer;
                __info.renderArea.extent = _extent;
                __info.clearValueCount   = _clear_values.size();
                __info.pClearValues      = _clear_values.data();
                vkCmdBeginRenderPass(cmd, &__info, _contents);
            }
            void beginRenderPass(const VkRenderPassBeginInfo &_info, VkSubpassContents _contents = VK_SUBPASS_CONTENTS_INLINE) {
                vkCmdBeginRenderPass(cmd, &_info, _contents);
            }
            void nextSubpass(VkSubpassContents _contents = VK_SUBPASS_CONTENTS_INLINE) {
                vkCmdNextSubpass(cmd, _contents);
            }
            void endRenderPass() {
                vkCmdEndRenderPass(cmd);
            }

        // Bind

            void bindPipeline(PipelineId _pipeline_id) {
                VkPipelineBindPoint __bind_point;
                switch (cxt.cxt_pipeline.pipelines.get(_pipeline_id).pipeline_type) {
                    case VKFW_PIPELINE_TYPE_GRAPHICS   : __bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS       ; break;
                    case VKFW_PIPELINE_TYPE_COMPUTE    : __bind_point = VK_PIPELINE_BIND_POINT_COMPUTE        ; break;
                    case VKFW_PIPELINE_TYPE_RAY_TRACING: __bind_point = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR; break;
                    default: throw std::runtime_error(SVKFW_WRAPERR("VKFW::CommandRecorder :: bindPipeline", "pipeline type is not set"));
                }
                vkCmdBindPipeline(cmd, __bind_point, cxt.get(_pipeline_id));
            }

            void bindDescriptorSets(PipelineLayoutId _layout_id, Span<DescriptorSetId> _sets_id, Span<uint32_t> _dynamic_offsets = {},
                                    VkPipelineBindPoint _bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS, uint32_t _first_set = 0u) {
                InlineArray<VkDescriptorSet> __sets(_sets_id.size());
                cxt.fill(_sets_id, __sets.data());
                vkCmdBindDescriptorSets(cmd, _bind_point, cxt.get(_layout_id), _first_set, __sets.size(), __sets.data(), _dynamic_offsets.size(), _dynamic_offsets.data());
            }

            // Note: missing offsets are set to 0
            void bindVertexBuffers(Span<BufferId> _buffers_id, Span<VkDeviceSize> _offsets = {}, uint32_t _first_binding = 0u) {
                InlineArray<VkBuffer>     __buffers(_buffers_id.size());
                InlineArray<VkDeviceSize> __offsets(_buffers_id.size());
                cxt.fill(_buffers_id, __buffers.data());
                for (uint32_t i = 0u; i < __offsets.size(); ++i)
                    __offsets[i] = i < _offsets.size() ? _offsets[i] : 0u;
                vkCmdBindVertexBuffers(cmd, _first_binding, __buffers.size(), __buffers.data(), __offsets.data());
            }

            void bindIndexBuffer(BufferId _buffer_id, VkDeviceSize _offset = 0u, VkIndexType _index_type = VK_INDEX_TYPE_UINT32) {
                vkCmdBindIndexBuffer(cmd, cxt.get(_buffer_id), _offset, _index_type);
            }

            void pushConstants(PipelineLayoutId _layout_id, VkShaderStageFlags _stages, uint32_t _offset, uint32_t _size, const void *_data) {
                vkCmdPushConstants(cmd, cxt.get(_layout_id), _stages, _offset, _size, _data);
            }

        // Dynamic state

            void setViewports(Span<VkViewport> _viewports, uint32_t _first = 0u) { vkCmdSetViewport(cmd, _first, _viewports.size(), _viewports.data()); }
            void setScissors (Span<VkRect2D>   _scissors,  uint32_t _first = 0u) { vkCmdSetScissor (cmd, _first, _scissors .size(), _scissors .data()); }
            void setLineWidth(float _line_width) { vkCmdSetLineWidth(cmd, _line_width); }

            // Sets viewport 0 (depth 0..1) and scissor 0 to cover '_extent'
            void setViewportScissor(VkExtent2D _extent) {
                VkViewport __viewport{};
                __viewport.width    = (float)_extent.width;
                __viewport.height   = (float)_extent.height;
                __viewport.maxDepth = 1.f;
                VkRect2D __scissor{};
                __scissor.extent = _extent;
                vkCmdSetViewport(cmd, 0u, 1u, &__viewport);
                vkCmdSetScissor (cmd, 0u, 1u, &__scissor );
            }

        // Draw

            void draw(uint32_t _vert_count, uint32_t _inst_count = 1u, uint32_t _first_vert = 0u, uint32_t _first_inst = 0u) {
                vkCmdDraw(cmd, _vert_count, _inst_count, _first_vert, _first_inst);
            }
            // Note: each index is 'index + vert_offset'
            void drawIndexed(uint32_t _ind_count, uint32_t _inst_count = 1u, int32_t _vert_offset = 0, uint32_t _first_ind = 0u, uint32_t _first_inst = 0u) {
                vkCmdDrawIndexed(cmd, _ind_count, _inst_count, _first_ind, _vert_offset, _first_inst);
            }
            void dispatch(uint32_t _x, uint32_t _y = 1u, uint32_t _z = 1u) {
                vkCmdDispatch(cmd, _x, _y, _z);
            }

            void clearAttachments(Span<VkClearAttachment> _attachments, Span<VkClearRect> _rects) {
                vkCmdClearAttachments(cmd, _attachments.size(), _attachments.data(), _rects.size(), _rects.data());
            }

        // Transfer

            void copyBuffer(BufferId _src_id, BufferId _dst_id, Span<VkBufferCopy> _regions) {
                vkCmdCopyBuffer(cmd, cxt.get(_src_id), cxt.get(_dst_id), _regions.size(), _regions.data());
            }
            void copyImage(ImageId _src_id, VkImageLayout _src_layout, ImageId _dst_id, VkImageLayout _dst_layout, Span<VkImageCopy> _regions) {
                vkCmdCopyImage(cmd, cxt.get(_src_id), _src_layout, cxt.get(_dst_id), _dst_layout, _regions.size(), _regions.data());
            }
            void copyBufferToImage(BufferId _src_id, ImageId _dst_id, VkImageLayout _dst_layout, Span<VkBufferImageCopy> _regions) {
                vkCmdCopyBufferToImage(cmd, cxt.get(_src_id), cxt.get(_dst_id), _dst_layout, _regions.size(), _regions.data());
            }

        // Synchronization

            void pipelineBarrier(VkPipelineStageFlags _src_stages, VkPipelineStageFlags _dst_stages,
                                 Span<VkMemoryBarrier> _memory_barriers, Span<VkBufferMemoryBarrier> _buffer_barriers = {},
                                 Span<VkImageMemoryBarrier> _image_barriers = {}, VkDependencyFlags _flags = 0u) {
                vkCmdPipelineBarrier(cmd, _src_stages, _dst_stages, _flags, _memory_barriers.size(), _memory_barriers.data(),
                                     _buffer_barriers.size(), _buffer_barriers.data(), _image_barriers.size(), _image_barriers.data());
            }
        }; // CommandRecorder END



//  == === ==== ================================== ==== === ==  \\
                    Vulkan Upload Batch
//  == === ==== ================================== ==== === ==  \\

        // Note: collects copies into device buffers through one HOST_VISIBLE staging buffer and submits them at once.
        //       'submit' doesn't block the CPU; the staging buffer is reused only after 'wait'.
        //       If the staging buffer is full, the batch submits and waits by itself, so any amount of data fits.
        //
        //       UploadBatch upload(vk, transfer_queue, staging_cmd, staging_buffer);
        //       upload.copyToBuffer(vertex_buffer, vertices.data(), vertices_size);
        //       upload.copyToBuffer( index_buffer,  indices.data(),  indices_size);
        //       upload.submit();   // one vkQueueSubmit for both copies
        //       ...                // other CPU work
        //       upload.wait();
        struct UploadBatch {
            UploadBatch(VulkanContext &_cxt, QueueId _queue_id, CommandBufferId _cmd_buffer_id, BufferId _staging_buffer_id)
                : cxt(_cxt), queue_id{_queue_id}, cmd_buffer_id{_cmd_buffer_id}, staging_buffer_id{_staging_buffer_id} {
                staging_ptr = (uint8_t*)cxt.cxt_memory.getMappedBuffer(staging_buffer_id);
                SVKFW_ASSERT(staging_ptr != nullptr, std::invalid_argument, "VKFW::UploadBatch", "staging buffer memory is not HOST_VISIBLE or not allocated");
                if ((cxt.cxt_memory.buffer(staging_buffer_id).create_info.peek().usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0u)
                    fprintf(svkfwwarn, "%s (%u, %u)\n", SVKFW_WRAPWARN("VKFW::UploadBatch", "staging buffer is not set as transfer source:"), staging_buffer_id.parent.index, staging_buffer_id.index);

                staging_size = cxt.cxt_memory.bufferMapping(staging_buffer_id).range;
                fence_id = cxt.cxt_barrier.addFence(false);
            }
           ~UploadBatch() {
                // Note: the fence and the staging buffer must not be in use when the batch is gone
                if (cxt.cxt_barrier.fences.contains(fence_id)) {
                    try { wait(); } catch (...) {}
                    cxt.cxt_barrier.destroyFence(fence_id);
                }
            }

            // Copies the data into the staging buffer now, the device buffer is written after 'submit'
            void copyToBuffer(BufferId _dst_buffer_id, const void *_data, VkDeviceSize _size, VkDeviceSize _dst_offset = 0u) {
                if ((cxt.cxt_memory.buffer(_dst_buffer_id).create_info.peek().usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0u)
                    fprintf(svkfwwarn, "%s (%u, %u)\n", SVKFW_WRAPWARN("VKFW::UploadBatch :: copyToBuffer", "buffer is not set as transfer destination:"), _dst_buffer_id.parent.index, _dst_buffer_id.index);
                SVKFW_ASSERT(_dst_offset + _size <= cxt.cxt_memory.bufferMapping(_dst_buffer_id).range, std::out_of_range, "VKFW::UploadBatch :: copyToBuffer",
                                "range is out of the buffer: " + std::to_string(_dst_offset + _size) + " > " + std::to_string(cxt.cxt_memory.bufferMapping(_dst_buffer_id).range));

                wait(); // the staging buffer may still be read by the previous submit
                const uint8_t *__src = (const uint8_t*)_data;
                while (_size > 0u) {
                    if (staging_used == staging_size) { submit(); wait(); }

                    VkDeviceSize __part = std::min(_size, staging_size - staging_used);
                    memcpy(staging_ptr + staging_used, __src, __part);

                    Region __region;
                    __region.dst_buffer_id = _dst_buffer_id;
                    __region.copy.srcOffset = staging_used;
                    __region.copy.dstOffset = _dst_offset;
                    __region.copy.size      = __part;
                    regions.push_back(__region);

                    staging_used += __part;
                    __src        += __part;
                    _dst_offset  += __part;
                    _size        -= __part;
                }
            }

            // Records and submits all collected copies; returns the fence signaled when they are done (none if there was nothing to submit)
            FenceId submit() {
                if (regions.empty()) return FenceId();
                cxt.cxt_memory.flushBuffer(staging_buffer_id, 0u, staging_used);

                CommandRecorder __rec(cxt, cmd_buffer_id);
                __rec.reset();
                __rec.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
                for (uint32_t i = 0u; i < regions.size(); ++i)
                    __rec.copyBuffer(staging_buffer_id, regions[i].dst_buffer_id, regions[i].copy);
                __rec.end();

                SubmitDesc __submit;
                __submit.commands(cmd_buffer_id);
                cxt.submit(queue_id, __submit, fence_id);

                regions.clear();
                is_submitted = true;
                return fence_id;
            }

            // Waits for the submitted copies; after it the staging buffer is free again
            void wait() {
                if (!is_submitted) return;
                cxt.cxt_barrier.wait(fence_id);
                cxt.cxt_barrier.reset(fence_id);
                is_submitted = false;
                staging_used = 0u;
            }

            bool isSubmitted() const { return is_submitted; }

        private:
            struct Region {
                BufferId     dst_buffer_id;
                VkBufferCopy copy;
            };

            VulkanContext  &cxt;
            QueueId         queue_id;
            CommandBufferId cmd_buffer_id;
            BufferId        staging_buffer_id;
            FenceId         fence_id;

            uint8_t     *staging_ptr  = nullptr;
            VkDeviceSize staging_size = 0u;
            VkDeviceSize staging_used = 0u;
            bool         is_submitted = false;
            std::vector<Region> regions;

            UploadBatch(const UploadBatch &);
            UploadBatch& operator=(const UploadBatch &);
        }; // UploadBatch END
    }; // VKFW END
}; // Simple END

#endif