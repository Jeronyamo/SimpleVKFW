#include "interface/vkfw.h"

namespace Simple {
    namespace VKFW {

        uint32_t DeviceProfile::minApiVersion() const {
            return (dynamic_rendering || synchronization2) ? VK_API_VERSION_1_3 : VK_API_VERSION_1_1;
        }

        namespace Detail {

            VkPipelineStageFlags toStageFlags1(VkPipelineStageFlags2 _stages, VkPipelineStageFlags _none_stage) {
                SVKFW_ASSERT((_stages >> 32u) == 0u, std::invalid_argument, "VKFW::Detail :: toStageFlags1", "the stage needs synchronization2");
                return _stages == 0u ? _none_stage : VkPipelineStageFlags(_stages);
            }

            VkAccessFlags toAccessFlags1(VkAccessFlags2 _access) {
                SVKFW_ASSERT((_access >> 32u) == 0u, std::invalid_argument, "VKFW::Detail :: toAccessFlags1", "the access needs synchronization2");
                return VkAccessFlags(_access);
            }

            bool hasDepth(VkFormat _format) {
                switch (_format) {
                    case VK_FORMAT_D16_UNORM: case VK_FORMAT_X8_D24_UNORM_PACK32: case VK_FORMAT_D32_SFLOAT:
                    case VK_FORMAT_D16_UNORM_S8_UINT: case VK_FORMAT_D24_UNORM_S8_UINT: case VK_FORMAT_D32_SFLOAT_S8_UINT: return true;
                    default: return false;
                }
            }

            bool hasStencil(VkFormat _format) {
                switch (_format) {
                    case VK_FORMAT_S8_UINT: case VK_FORMAT_D16_UNORM_S8_UINT: case VK_FORMAT_D24_UNORM_S8_UINT: case VK_FORMAT_D32_SFLOAT_S8_UINT: return true;
                    default: return false;
                }
            }

            VkImageAspectFlags formatAspects(VkFormat _format) {
                VkImageAspectFlags __aspects = (hasDepth  (_format) ? VkImageAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT  ) : 0u) |
                                               (hasStencil(_format) ? VkImageAspectFlags(VK_IMAGE_ASPECT_STENCIL_BIT) : 0u);
                return __aspects ? __aspects : VkImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT);
            }
        }; // Detail END


        VkImageMemoryBarrier2 imageBarrier(VkImage _image, VkImageLayout _old_layout, VkImageLayout _new_layout,
                                                  VkPipelineStageFlags2 _src_stages, VkAccessFlags2 _src_access,
                                                  VkPipelineStageFlags2 _dst_stages, VkAccessFlags2 _dst_access,
                                                  VkImageAspectFlags _aspect) {
            VkImageMemoryBarrier2 __barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
            __barrier.srcStageMask        = _src_stages;
            __barrier.srcAccessMask       = _src_access;
            __barrier.dstStageMask        = _dst_stages;
            __barrier.dstAccessMask       = _dst_access;
            __barrier.oldLayout           = _old_layout;
            __barrier.newLayout           = _new_layout;
            __barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            __barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            __barrier.image               = _image;
            __barrier.subresourceRange    = { _aspect, 0u, VK_REMAINING_MIP_LEVELS, 0u, VK_REMAINING_ARRAY_LAYERS };
            return __barrier;
        }

        VkRenderingAttachmentInfo renderingAttachment(VkImageView _view, VkImageLayout _layout,
                                                             VkAttachmentLoadOp _load_op, VkAttachmentStoreOp _store_op,
                                                             const VkClearValue &_clear_value) {
            VkRenderingAttachmentInfo __info{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
            __info.imageView   = _view;
            __info.imageLayout = _layout;
            __info.loadOp      = _load_op;
            __info.storeOp     = _store_op;
            __info.clearValue  = _clear_value;
            return __info;
        }

        SubmitDesc& SubmitDesc::wait(SemaphoreId _semaphore_id, VkPipelineStageFlags2 _stage) {
            SVKFW_ASSERT(wait_count < VKFW_MAX_SUBMIT_SEMAPHORES, std::length_error, "VKFW::SubmitDesc :: wait", "too many wait semaphores");
            wait_semaphores[wait_count] = _semaphore_id;
            wait_stages    [wait_count] = _stage;
            ++wait_count;
            return *this;
        }

        SubmitDesc& SubmitDesc::commands(CommandBufferId _cmd_buffer_id) {
            SVKFW_ASSERT(command_count < VKFW_MAX_SUBMIT_COMMAND_BUFFERS, std::length_error, "VKFW::SubmitDesc :: commands", "too many command buffers");
            command_buffers[command_count++] = _cmd_buffer_id;
            return *this;
        }

        SubmitDesc& SubmitDesc::signal(SemaphoreId _semaphore_id, VkPipelineStageFlags2 _stage) {
            SVKFW_ASSERT(signal_count < VKFW_MAX_SUBMIT_SEMAPHORES, std::length_error, "VKFW::SubmitDesc :: signal", "too many signal semaphores");
            signal_semaphores[signal_count] = _semaphore_id;
            signal_stages    [signal_count] = _stage;
            ++signal_count;
            return *this;
        }

        void SubmitDesc::clear() { wait_count = command_count = signal_count = 0u; }

        VulkanContext::ContextDebug::ContextDebug(VulkanContext* _cxt_ptr) : cxt_ptr{_cxt_ptr} {}

        VulkanContext::ContextDebug::~ContextDebug() {}

        VulkanContext::ContextDebug& VulkanContext::ContextDebug::operator=(VkResult _vk_func_result) { /* TODO: */ return *this; }

        VulkanContext::ContextCore::ContextCore(VulkanContext* _cxt_ptr) : cxt_debug{_cxt_ptr} {
            SVKFW_ASSERT(volkInitialize() == VK_SUCCESS, std::runtime_error, "VKFW::VulkanContext :: ContextCore constructor",
                            "volk initialization failed");
            // Note: members are constructed before volkInitialize(), so the version was computed without 'vkEnumerateInstanceVersion'
            instance.create_info.app_info_wrap.setAvailableVersionAPI();
        }

        VulkanContext::ContextCore::~ContextCore() { destroyContext(); }

        void VulkanContext::ContextCore::destroyContext() {
            destroyRenderPass();
            destroyDevice();
            destroyPhysicalDevice();
            destroyDebugUtilsMessenger();
            destroyInstance();
        }

        QueueFamilyId VulkanContext::ContextCore::addQueueFamily() { return queue_families.add(); }

        Object::QueueFamily &VulkanContext::ContextCore::queueFamily(QueueFamilyId _id)       { return queue_families.get(_id).family; }

        const Object::QueueFamily &VulkanContext::ContextCore::queueFamily(QueueFamilyId _id) const { return queue_families.get(_id).family; }

        Object::Queue       &VulkanContext::ContextCore::queues     (QueueFamilyId _id)       { return queue_families.get(_id).queues; }

        VkQueue VulkanContext::ContextCore::queue(QueueId _id) const {
            const std::vector<VkQueue> &__handles = queue_families.get(_id.parent).queues.handles;
            SVKFW_ASSERT(_id.index < __handles.size(), std::out_of_range, "VKFW::ContextCore :: queue", "queue index is out of its family");
            return __handles[_id.index];
        }

        void VulkanContext::ContextCore::createInstance() {
            instance.createObject(alloc_cb.get());
        }

        void VulkanContext::ContextCore::createDebugUtilsMessenger() {
            cxt_debug.debug_messenger.createObject(instance.handle, alloc_cb.get());
        }

        void VulkanContext::ContextCore::loadPhysicalDeviceInfo() {
            // Note: an application info without a version means Vulkan 1.0
            const uint32_t __api_version = instance.create_info.app_info_wrap.peek().apiVersion;
            phys_device.select_info.loadInfo(instance.handle, __api_version == 0u ? VK_API_VERSION_1_0 : __api_version);
        }

        void VulkanContext::ContextCore::findPhysicalDevice() {
            phys_device.createObject();
        }

        void VulkanContext::ContextCore::loadQueueFamiliesInfo() {
            queue_families.forEach([this](QueueFamilyId, QueueFamilyEntry &_qfam) {
                _qfam.family.select_info.loadInfo(instance.handle, phys_device.handle);
            });
        }

        void VulkanContext::ContextCore::findQueueFamilies() {
            queue_families.forEach([](QueueFamilyId, QueueFamilyEntry &_qfam) { _qfam.family.createObject(); });
        }

        uint32_t VulkanContext::ContextCore::apiVersion() const {
            return std::min(instance.create_info.app_info_wrap.peek().apiVersion, phys_device.properties().head().properties.apiVersion);
        }

        void VulkanContext::ContextCore::checkProfileSupport() const {
            SVKFW_ASSERT(apiVersion() >= profile.minApiVersion(), std::runtime_error, "VKFW::ContextCore :: checkProfileSupport",
                            "the device profile needs Vulkan " + std::to_string(VK_API_VERSION_MAJOR(profile.minApiVersion())) + "." +
                                                                 std::to_string(VK_API_VERSION_MINOR(profile.minApiVersion())));
        }

        void VulkanContext::ContextCore::createDevice() {
            checkProfileSupport();
            Struct::StructWrap<VkDeviceCreateInfo> &__info = device.create_info;
            // Note: the features of the profile are mandatory in Vulkan 1.3, so they are only enabled
            if (profile.dynamic_rendering) __info.features.get<VkPhysicalDeviceVulkan13Features>().dynamicRendering = VK_TRUE;
            if (profile.synchronization2 ) __info.features.get<VkPhysicalDeviceVulkan13Features>().synchronization2 = VK_TRUE;

            // Note: features of versions above the used API version can't be enabled (their structures are not chained)
            const uint32_t __api_version = apiVersion();
            DeviceFeatures __supported = phys_device.features();
            if (__api_version < VK_API_VERSION_1_3)
                __supported.get<VkPhysicalDeviceVulkan13Features>() = VkPhysicalDeviceVulkan13Features();
            if (__api_version < VK_API_VERSION_1_2) {
                __supported.get<VkPhysicalDeviceVulkan11Features>() = VkPhysicalDeviceVulkan11Features();
                __supported.get<VkPhysicalDeviceVulkan12Features>() = VkPhysicalDeviceVulkan12Features();
            }
            __info.enableOptionalFeatures(__supported);
            const std::vector<std::string> __missing = __info.unsupportedFeatures(__supported);
            std::string __list;
            for (uint32_t i = 0u; i < __missing.size(); ++i) __list += (i ? ", " : "") + __missing[i];
            SVKFW_ASSERT(__missing.empty(), std::runtime_error, "VKFW::ContextCore :: createDevice", "the device doesn't support the required features: " + __list);
            __info.api_version = __api_version;

            device.createObject(phys_device.handle, alloc_cb.get());
        }

        void VulkanContext::ContextCore::acquireQueues() {
            queue_families.forEach([this](QueueFamilyId, QueueFamilyEntry &_qfam) { _qfam.queues.createObject(device.handle); });
        }

        void VulkanContext::ContextCore::createRenderPass() {
            SVKFW_ASSERT(!profile.dynamic_rendering, std::logic_error, "VKFW::ContextCore :: createRenderPass", "the render pass is not used with dynamic rendering");
            render_pass.createObject(device.handle, alloc_cb.get());
        }

        void VulkanContext::ContextCore::destroyInstance() {
            instance.destroyObject(alloc_cb.get());
        }

        void VulkanContext::ContextCore::destroyDebugUtilsMessenger() {
            cxt_debug.debug_messenger.destroyObject(instance.handle, alloc_cb.get());
        }

        void VulkanContext::ContextCore::destroyPhysicalDevice() {
            phys_device.destroyObject();
        }

        void VulkanContext::ContextCore::destroyDevice() {
            device.destroyObject(alloc_cb.get());
        }

        void VulkanContext::ContextCore::destroyRenderPass() {
            render_pass.destroyObject(device.handle, alloc_cb.get());
        }

        ContextIndex VulkanContext::ContextCore::addSctRenderPassAttachment        () { return render_pass.create_info.addAttachment        (); }

        ContextIndex VulkanContext::ContextCore::addSctRenderPassSubpassDescription() { return render_pass.create_info.addSubpassDescription(); }

        ContextIndex VulkanContext::ContextCore::addSctRenderPassSubpassDependency () { return render_pass.create_info.addSubpassDependency (); }

        void VulkanContext::ContextCore::setDefaultApplicationInfo() {
            instance.create_info.app_info_wrap.setWholeStruct();
        }

        void VulkanContext::ContextCore::activateDebug() {
            instance.create_info.addExtDebugUtils();
            instance.create_info.addValidationLayer();
            if (VulkanAPIVersion::join(instance.create_info.app_info_wrap.api_version["xyz"],
                                       instance.create_info.app_info_wrap.api_version.w) <= VK_API_VERSION_1_0)
                device.create_info.addValidationLayer(); // physical device isn't found at this point
        }

        void VulkanContext::ContextCore::setQueueFamilyQueueCount(QueueFamilyId _queue_family_id, uint32_t _queue_count, const std::vector<float> &_priorities) {
            QueueFamilyEntry &__qfam = queue_families.get(_queue_family_id);
            __qfam.queues.create_info.queue_family = __qfam.family.handle;
            __qfam.queues.create_info.setQueueCount(_queue_count, _priorities);
        }

        void VulkanContext::ContextCore::setDeviceQueueInfo() {
            device.create_info.queue_info.clear();
            queue_families.forEach([this](QueueFamilyId, QueueFamilyEntry &_qfam) {
                device.create_info.queue_info.push_back(_qfam.queues.create_info.get());
            });
        }

        VulkanContext::ContextSwapchain::ContextSwapchain(ContextCore *_cxt_core) { cxt_core = _cxt_core; }

        VulkanContext::ContextSwapchain::~ContextSwapchain() { destroyContext(); }

        void VulkanContext::ContextSwapchain::destroyContext() {
            destroySwapchainWithDependencies();
            destroySurface();
        }

        void VulkanContext::ContextSwapchain::createSurface() {
            surface.createObject(cxt_core->instance.handle, window, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextSwapchain::createSwapchain() {
            swapchain.create_info.edit().surface = surface.handle;
            swapchain.createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
            image_view_wrap.edit().format = swapchain.create_info.peek().imageFormat;
            framebuffer_wrap.setBufferSize(swapchain.create_info.peek().imageExtent);
        }

        void VulkanContext::ContextSwapchain::createImageViews() {
            images = Func::GetSwapchainImagesKHR(cxt_core->device.handle, swapchain.handle);

            image_views.resize(images.size());
            for (ContextIndex ci = 0u; ci < images.size(); ++ci) {
                image_view_wrap.edit().image = images[ci];
                image_views[ci] = Func::CreateImageView(cxt_core->device.handle, &image_view_wrap.get(), cxt_core->alloc_cb.get());
            }
        }

        void VulkanContext::ContextSwapchain::createFramebuffers() {
            // Note: not needed if the render passes are elsewhere (e.g. in 'Graph::RenderGraph')
            if (cxt_core->profile.dynamic_rendering || cxt_core->render_pass.handle == VK_NULL_HANDLE) return;
            framebuffers.resize(image_views.size());
            framebuffer_wrap.edit().renderPass = cxt_core->render_pass.handle;

            for (ContextIndex ci = 0u; ci < framebuffers.size(); ++ci) {
                framebuffer_wrap.attachments = {image_views[ci]};
                framebuffers[ci] = Func::CreateFramebuffer(cxt_core->device.handle, &framebuffer_wrap.get(), cxt_core->alloc_cb.get());
            }
        }

        void VulkanContext::ContextSwapchain::destroySurface() {
            surface.destroyObject(cxt_core->instance.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextSwapchain::destroySwapchain() {
            swapchain.destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextSwapchain::destroyImageViews() {
            for (auto    img_view :  image_views)
                vkDestroyImageView(cxt_core->device.handle, img_view, cxt_core->alloc_cb.get());
            image_views.clear();
            images.clear(); // Note: the images belong to the swapchain
        }

        void VulkanContext::ContextSwapchain::destroyFramebuffers() {
            for (auto framebuffer : framebuffers)
                vkDestroyFramebuffer(cxt_core->device.handle, framebuffer, cxt_core->alloc_cb.get());
            framebuffers.clear();
        }

        void VulkanContext::ContextSwapchain::setWindow(const Window &_window) {
            window = _window.window;
        }

        void  VulkanContext::ContextSwapchain::createSwapchainWithDependencies() {
            createSwapchain();
            createImageViews();
            createFramebuffers();
        }

        void VulkanContext::ContextSwapchain::destroySwapchainWithDependencies() {
            destroyFramebuffers();
            destroyImageViews();
            destroySwapchain();
        }

        uint32_t VulkanContext::ContextSwapchain::getSwapchainImageCount() const {
            return image_views.size();
        }

        VkFramebuffer VulkanContext::ContextSwapchain::framebuffer(uint32_t _image_index) const {
            SVKFW_ASSERT(_image_index < framebuffers.size(), std::out_of_range, "VKFW::ContextSwapchain :: framebuffer", "image index is out of the swapchain");
            return framebuffers[_image_index];
        }

        VkImage VulkanContext::ContextSwapchain::image(uint32_t _image_index) const {
            SVKFW_ASSERT(_image_index < images.size(), std::out_of_range, "VKFW::ContextSwapchain :: image", "image index is out of the swapchain");
            return images[_image_index];
        }

        VkImageView VulkanContext::ContextSwapchain::imageView(uint32_t _image_index) const {
            SVKFW_ASSERT(_image_index < image_views.size(), std::out_of_range, "VKFW::ContextSwapchain :: imageView", "image index is out of the swapchain");
            return image_views[_image_index];
        }

        VkExtent2D VulkanContext::ContextSwapchain::extent() const {
            return swapchain.create_info.peek().imageExtent;
        }

        VkFormat VulkanContext::ContextSwapchain::format() const {
            return swapchain.create_info.peek().imageFormat;
        }

        AcquireResult VulkanContext::ContextSwapchain::acquire(VkSemaphore _semaphore, VkFence _fence, uint64_t _timeout) {
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

        SwapchainStatus VulkanContext::ContextSwapchain::present(VkQueue _queue, Span<VkSemaphore> _wait_semaphores, uint32_t _image_index) {
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

        void VulkanContext::ContextSwapchain::pickSwapchainImageFormat(const Util::SurfaceFormatTestFunc _test_func) {
            swapchain.create_info.pickFormat(cxt_core->phys_device.handle, surface.handle, _test_func);
        }

        void VulkanContext::ContextSwapchain::setSwapchainExtentFromWindow() {
            swapchain.create_info.chooseCurrentExtent(window);
        }

        void VulkanContext::ContextSwapchain::setSwapchainSurfaceCapabilities() {
            swapchain.create_info.setSurfaceCapabilities(cxt_core->phys_device.handle, surface.handle);
        }

        void VulkanContext::ContextSwapchain::setRenderPassAttachmentImageFormat(ContextIndex _attachment_ci) {
            cxt_core->render_pass.create_info.attachments_wrap[_attachment_ci].edit().format = swapchain.create_info.peek().imageFormat;
        }

        void VulkanContext::ContextSwapchain::recreate() {
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

        VulkanContext::ContextDescriptor::ContextDescriptor(ContextCore *_cxt_core) { cxt_core = _cxt_core; }

        VulkanContext::ContextDescriptor::~ContextDescriptor() { destroyContext(); }

        void  VulkanContext::ContextDescriptor::createContext() {}

        void VulkanContext::ContextDescriptor::destroyContext() {
            // Note: sets are freed when their pools are destroyed
            VkDevice                     __device   = cxt_core->device.handle;
            const VkAllocationCallbacks *__alloc_cb = cxt_core->alloc_cb.get();
            pools  .forEach([&](DescriptorPoolId,      Object::DescriptorPool      &_pool  ) { _pool  .destroyObject(__device, __alloc_cb); });
            layouts.forEach([&](DescriptorSetLayoutId, Object::DescriptorSetLayout &_layout) { _layout.destroyObject(__device, __alloc_cb); });
            samplers.forEach([&](SamplerId,            Object::Sampler             &_sampler) { _sampler.destroyObject(__device, __alloc_cb); });
            samplers.clear();
            sets   .clear();
            pools  .clear();
            layouts.clear();
        }

        VkDescriptorSet VulkanContext::ContextDescriptor::set(DescriptorSetId _id) const {
            const std::vector<VkDescriptorSet> &__handles = sets.get(_id.parent).handles;
            SVKFW_ASSERT(_id.index < __handles.size(), std::out_of_range, "VKFW::ContextDescriptor :: set", "set index is out of its group");
            return __handles[_id.index];
        }

        Object::DescriptorSetLayout &VulkanContext::ContextDescriptor::layout  (DescriptorSetLayoutId _id) { return layouts.get(_id); }

        Object::DescriptorPool      &VulkanContext::ContextDescriptor::pool    (DescriptorPoolId      _id) { return pools  .get(_id); }

        Object::DescriptorSet       &VulkanContext::ContextDescriptor::setGroup(DescriptorSetsId      _id) { return sets   .get(_id); }

        Object::Sampler             &VulkanContext::ContextDescriptor::sampler (SamplerId             _id) { return samplers.get(_id); }

        Struct::StructWrap<VkDescriptorSetLayoutBinding> &VulkanContext::ContextDescriptor::layoutBinding(DescriptorLayoutBindingId _id) {
            Object::DescriptorSetLayout &__layout = layouts.get(_id.parent);
            SVKFW_ASSERT(_id.index < __layout.create_info.bindings_wrap.size(), std::out_of_range, "VKFW::ContextDescriptor :: layoutBinding", "binding index is out of its layout");
            return __layout.create_info.bindings_wrap[_id.index];
        }

        void VulkanContext::ContextDescriptor::updateSets(Span<VkWriteDescriptorSet> _writes, Span<VkCopyDescriptorSet> _copies) {
            vkUpdateDescriptorSets(cxt_core->device.handle, _writes.size(), _writes.data(), _copies.size(), _copies.data());
        }

        void VulkanContext::ContextDescriptor::updateSets(std::vector<Struct::StructWrap<VkWriteDescriptorSet>> &_writes) {
            InlineArray<VkWriteDescriptorSet> __writes(_writes.size());
            for (uint32_t i = 0u; i < __writes.size(); ++i)
                __writes[i] = _writes[i].get();
            updateSets(__writes);
        }

        DescriptorSetLayoutId VulkanContext::ContextDescriptor::addLayout() { return layouts.add(); }

        DescriptorPoolId      VulkanContext::ContextDescriptor::addPool  () { return   pools.add(); }

        DescriptorSetsId      VulkanContext::ContextDescriptor::addSet   () { return    sets.add(); }

        SamplerId             VulkanContext::ContextDescriptor::addSampler() { return samplers.add(); }

        void VulkanContext::ContextDescriptor::createLayout(DescriptorSetLayoutId _layout_id) {
            layouts.get(_layout_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextDescriptor::createPool(DescriptorPoolId _pool_id) {
            pools.get(_pool_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextDescriptor::createSets(DescriptorSetsId _sets_id) {
            sets.get(_sets_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextDescriptor::createSampler(SamplerId _sampler_id) {
            Object::Sampler &__sampler = samplers.get(_sampler_id);
            if (__sampler.create_info.peek().anisotropyEnable) {
                if (!cxt_core->device.create_info.features.head().features.samplerAnisotropy) {
                    fprintf(svkfwwarn, "%s\n", SVKFW_WRAPWARN("VKFW::ContextDescriptor :: createSampler", "the 'samplerAnisotropy' feature isn't enabled, anisotropic filtering is turned off"));
                    __sampler.create_info.setAnisotropy(0.f);
                }
                else {
                    const float __max = cxt_core->phys_device.properties().head().properties.limits.maxSamplerAnisotropy;
                    if (__sampler.create_info.peek().maxAnisotropy > __max) __sampler.create_info.setAnisotropy(__max);
                }
            }
            __sampler.createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextDescriptor::destroyLayout(DescriptorSetLayoutId _layout_id) {
            layouts.get(_layout_id).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextDescriptor::destroyPool(DescriptorPoolId _pool_id) {
            pools.get(_pool_id).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextDescriptor::destroySets(DescriptorSetsId _sets_id) {
            sets.get(_sets_id).destroyObject(cxt_core->device.handle);
        }

        void VulkanContext::ContextDescriptor::destroySampler(SamplerId _sampler_id) {
            samplers.get(_sampler_id).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextDescriptor::removeLayout(DescriptorSetLayoutId _layout_id) { if (layouts.contains(_layout_id)) { destroyLayout(_layout_id); layouts.remove(_layout_id); } }

        void VulkanContext::ContextDescriptor::removePool  (DescriptorPoolId      _pool_id  ) { if (  pools.contains(_pool_id  )) { destroyPool  (_pool_id  );   pools.remove(_pool_id  ); } }

        void VulkanContext::ContextDescriptor::removeSets  (DescriptorSetsId      _sets_id  ) { if (   sets.contains(_sets_id  )) { destroySets  (_sets_id  );    sets.remove(_sets_id  ); } }

        void VulkanContext::ContextDescriptor::removeSampler(SamplerId            _sampler_id) { if (samplers.contains(_sampler_id)) { destroySampler(_sampler_id); samplers.remove(_sampler_id); } }

        DescriptorLayoutBindingId VulkanContext::ContextDescriptor::addSctLayoutBinding(DescriptorSetLayoutId _layout_id) {
            return DescriptorLayoutBindingId(_layout_id, layouts.get(_layout_id).create_info.addBinding());
        }

        void VulkanContext::ContextDescriptor::setAllForSet(DescriptorSetsId _sets_id, DescriptorPoolId _pool_id, Span<DescriptorSetLayoutId> _layouts_id) {
            std::vector<VkDescriptorSetLayout> __layouts(_layouts_id.size());
            for (uint32_t i = 0u; i < _layouts_id.size(); ++i)
                __layouts[i] = layouts.get(_layouts_id[i]).handle;

            sets.get(_sets_id).alloc_info.setWholeStruct(pools.get(_pool_id).handle, __layouts);
        }

        void VulkanContext::ContextDescriptor::setPoolForSet(DescriptorSetsId _sets_id, DescriptorPoolId _pool_id) {
            sets.get(_sets_id).alloc_info.edit().descriptorPool = pools.get(_pool_id).handle;
        }

        void VulkanContext::ContextDescriptor::setLayoutsForSet(DescriptorSetsId _sets_id, Span<DescriptorSetLayoutId> _layouts_id) {
            std::vector<VkDescriptorSetLayout> __layouts(_layouts_id.size());
            for (uint32_t i = 0u; i < _layouts_id.size(); ++i)
                __layouts[i] = layouts.get(_layouts_id[i]).handle;

            sets.get(_sets_id).alloc_info.set_layouts = __layouts;
        }

        VulkanContext::ContextPipeline::ContextPipeline(ContextCore *_cxt_core) { cxt_core = _cxt_core; }

        VulkanContext::ContextPipeline::~ContextPipeline() { destroyContext(); }

        void  VulkanContext::ContextPipeline::createContext() {}

        void VulkanContext::ContextPipeline::destroyContext() {
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

        Object::Pipeline       &VulkanContext::ContextPipeline::pipeline(PipelineId       _id) { return pipelines.get(_id); }

        Object::PipelineLayout &VulkanContext::ContextPipeline::layout  (PipelineLayoutId _id) { return layouts  .get(_id); }

        Object::PipelineCache  &VulkanContext::ContextPipeline::cache   (PipelineCacheId  _id) { return caches   .get(_id); }

        Object::ShaderModule   &VulkanContext::ContextPipeline::shader  (ShaderModuleId   _id) { return shaders  .get(_id); }

        Struct::StructWrap<VkPipelineShaderStageCreateInfo> &VulkanContext::ContextPipeline::shaderStage(ShaderStageId _id) {
            return *pipelines.get(_id.parent).shortcutShaderStage(_id.index);
        }

        PipelineId       VulkanContext::ContextPipeline::addPipeline() { return pipelines.add(); }

        ShaderModuleId   VulkanContext::ContextPipeline::addShader  () { return   shaders.add(); }

        PipelineLayoutId VulkanContext::ContextPipeline::addLayout  () { return   layouts.add(); }

        PipelineCacheId  VulkanContext::ContextPipeline::addCache   () { return    caches.add(); }

        void VulkanContext::ContextPipeline::createPipeline(PipelineId _pipeline_id, uint32_t _subpass, PipelineLayoutId _layout_id, PipelineCacheId _cache_id,
                            VkDeferredOperationKHR _deferred_op) {
            createPipeline(_pipeline_id, cxt_core->render_pass.handle, _subpass, _layout_id, _cache_id, _deferred_op);
        }

        void VulkanContext::ContextPipeline::createPipeline(PipelineId _pipeline_id, VkRenderPass _render_pass, uint32_t _subpass, PipelineLayoutId _layout_id,
                            PipelineCacheId _cache_id, VkDeferredOperationKHR _deferred_op) {
            if (cxt_core->profile.dynamic_rendering) _render_pass = VK_NULL_HANDLE;
            Object::Pipeline &__pipeline = pipelines.get(_pipeline_id);
            VkPipelineLayout  __layout   = layouts  .get(_layout_id).handle;
            switch (__pipeline.pipeline_type) {
                case VKFW_PIPELINE_TYPE_GRAPHICS:
                    // Note: with dynamic rendering the attachment formats are set through 'graphics_create_info_wrap.rendering_wrap'
                    __pipeline.  graphics_create_info_wrap.use_dynamic_rendering = cxt_core->profile.dynamic_rendering;
                    __pipeline.  graphics_create_info_wrap.edit().renderPass = _render_pass;
                    __pipeline.  graphics_create_info_wrap.edit().subpass    = _subpass;

                    __pipeline.  graphics_create_info_wrap.edit().layout = __layout; break;
                case VKFW_PIPELINE_TYPE_COMPUTE:
                    __pipeline.   compute_create_info_wrap.edit().layout = __layout; break;
                case VKFW_PIPELINE_TYPE_RAY_TRACING:
                    __pipeline.raytracing_create_info_wrap.edit().layout = __layout; break;
                default: break;
            }
            VkPipelineCache __pipeline_cache = _cache_id.isNone() ? VK_NULL_HANDLE : caches.get(_cache_id).handle;
            __pipeline.createObject(cxt_core->device.handle, _render_pass, _subpass,
                                    _deferred_op, __pipeline_cache, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextPipeline::createCache (PipelineCacheId _cache_id) {
            caches .get(_cache_id ).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextPipeline::createLayout(PipelineLayoutId _layout_id) {
            layouts.get(_layout_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextPipeline::createShader(ShaderModuleId _shader_id) {
            shaders.get(_shader_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextPipeline::destroyPipeline(PipelineId _pipeline_id) {
            pipelines.get(_pipeline_id).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextPipeline::destroyCache   (PipelineCacheId _cache_id) {
            caches   .get(_cache_id   ).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextPipeline::destroyLayout  (PipelineLayoutId _layout_id) {
            layouts  .get(_layout_id  ).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextPipeline::destroyShader  (ShaderModuleId _shader_id) {
            shaders  .get(_shader_id  ).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextPipeline::removePipeline(PipelineId       _pipeline_id) { if (pipelines.contains(_pipeline_id)) { destroyPipeline(_pipeline_id); pipelines.remove(_pipeline_id); } }

        void VulkanContext::ContextPipeline::removeCache   (PipelineCacheId  _cache_id   ) { if (caches   .contains(_cache_id   )) { destroyCache   (_cache_id   ); caches   .remove(_cache_id   ); } }

        void VulkanContext::ContextPipeline::removeLayout  (PipelineLayoutId _layout_id  ) { if (layouts  .contains(_layout_id  )) { destroyLayout  (_layout_id  ); layouts  .remove(_layout_id  ); } }

        void VulkanContext::ContextPipeline::removeShader  (ShaderModuleId   _shader_id  ) { if (shaders  .contains(_shader_id  )) { destroyShader  (_shader_id  ); shaders  .remove(_shader_id  ); } }

        void VulkanContext::ContextPipeline::setLayoutDescriptorSetLayouts(PipelineLayoutId _layout_id, Span<DescriptorSetLayoutId> _descr_set_layouts_id) {
            Object::PipelineLayout &__layout = layouts.get(_layout_id);
            __layout.create_info.layouts.resize(_descr_set_layouts_id.size(), VK_NULL_HANDLE);
            for (uint32_t i = 0u; i < _descr_set_layouts_id.size(); ++i)
                __layout.create_info.layouts[i] = cxt_core->cxt_debug.cxt_ptr->cxt_descriptor.layouts.get(_descr_set_layouts_id[i]).handle;
        }

        void VulkanContext::ContextPipeline::addLayoutPushConstant(PipelineLayoutId _layout_id, VkShaderStageFlags _stage_flags, uint32_t _offset, uint32_t _size) {
            layouts.get(_layout_id).create_info.constants.push_back(VkPushConstantRange{_stage_flags, _offset, _size});
        }

        ShaderStageId VulkanContext::ContextPipeline::addShaderStage(PipelineId _pipeline_id) {
            return ShaderStageId(_pipeline_id, pipelines.get(_pipeline_id).addShaderStage());
        }

        VulkanContext::ContextCommand::ContextCommand(ContextCore *_cxt_core) { cxt_core = _cxt_core; }

        VulkanContext::ContextCommand::~ContextCommand() { destroyContext(); }

        void  VulkanContext::ContextCommand::createContext() {}

        void VulkanContext::ContextCommand::destroyContext() {
            VkDevice                     __device   = cxt_core->device.handle;
            const VkAllocationCallbacks *__alloc_cb = cxt_core->alloc_cb.get();
            cmd_buffers.forEach([&](CommandBuffersId, Object::CommandBuffer &_cmd_buff) { _cmd_buff.destroyObject(__device); });
            cmd_pools  .forEach([&](CommandPoolId,    Object::CommandPool   &_cmd_pool) { _cmd_pool.destroyObject(__device, __alloc_cb); });
            cmd_buffers.clear();
            cmd_pools  .clear();
        }

        VkCommandBuffer VulkanContext::ContextCommand::commandBuffer(CommandBufferId _id) const {
            const std::vector<VkCommandBuffer> &__handles = cmd_buffers.get(_id.parent).handles;
            SVKFW_ASSERT(_id.index < __handles.size(), std::out_of_range, "VKFW::ContextCommand :: commandBuffer", "command buffer index is out of its group");
            return __handles[_id.index];
        }

        Object::CommandPool   &VulkanContext::ContextCommand::cmdPool       (CommandPoolId    _id) { return cmd_pools  .get(_id); }

        Object::CommandBuffer &VulkanContext::ContextCommand::cmdBufferGroup(CommandBuffersId _id) { return cmd_buffers.get(_id); }

        CommandPoolId    VulkanContext::ContextCommand::addCmdPool  () { return   cmd_pools.add(); }

        CommandBuffersId VulkanContext::ContextCommand::addCmdBuffer() { return cmd_buffers.add(); }

        void VulkanContext::ContextCommand::createCmdPool(CommandPoolId _cmd_pool_id) {
            cmd_pools.get(_cmd_pool_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextCommand::createCmdBuffers(CommandBuffersId _cmd_buffers_id) {
            cmd_buffers.get(_cmd_buffers_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextCommand::removeCmdBuffers(CommandBuffersId _cmd_buffers_id) {
            if (!cmd_buffers.contains(_cmd_buffers_id)) return;
            cmd_buffers.get(_cmd_buffers_id).destroyObject(cxt_core->device.handle);
            cmd_buffers.remove(_cmd_buffers_id);
        }

        void VulkanContext::ContextCommand::removeCmdPool(CommandPoolId _cmd_pool_id) {
            if (!cmd_pools.contains(_cmd_pool_id)) return;
            cmd_pools.get(_cmd_pool_id).destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
            cmd_pools.remove(_cmd_pool_id);
        }

        void VulkanContext::ContextCommand::createCmdPool(CommandPoolId _cmd_pool_id, QueueFamilyId _queue_family_id, VkCommandPoolCreateFlags _flags) {
            cmd_pools.get(_cmd_pool_id).createObject(cxt_core->device.handle, cxt_core->queueFamily(_queue_family_id).handle, _flags, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextCommand::createCmdBuffersPrimary(CommandBuffersId _cmd_buffers_id, CommandPoolId _cmd_pool_id, uint32_t _cmd_buffers_count) {
            cmd_buffers.get(_cmd_buffers_id).createPrimary(cxt_core->device.handle, cmd_pools.get(_cmd_pool_id).handle, _cmd_buffers_count, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextCommand::createCmdBuffersSecondary(CommandBuffersId _cmd_buffers_id, CommandPoolId _cmd_pool_id, uint32_t _cmd_buffers_count) {
            cmd_buffers.get(_cmd_buffers_id).createSecondary(cxt_core->device.handle, cmd_pools.get(_cmd_pool_id).handle, _cmd_buffers_count, cxt_core->alloc_cb.get());
        }

        VulkanContext::ContextMemory::MemoryChunk::MemoryChunk() {}

        VulkanContext::ContextMemory::MemoryChunk::MemoryChunk(ContextCore *_cxt_core) { cxt_core = _cxt_core; }

        void VulkanContext::ContextMemory::MemoryChunk::destroyChunk() {
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

        VkMemoryRequirements VulkanContext::ContextMemory::MemoryChunk::computeLayout(VkDeviceSize _granularity) {
            VkMemoryRequirements __total{ 0u, 1u, UINT32_MAX };
            int __prev_linear = -1;
            for (uint32_t ci = 0u; ci < buffers.size(); ++ci)
                place_(__total, Func::GetBufferMemoryRequirements(cxt_core->device.handle, buffers[ci].handle), true,
                       __prev_linear, _granularity, buffer_maps[ci], "buffer " + std::to_string(ci));
            for (uint32_t ci = 0u; ci < images.size(); ++ci) {
                place_(__total, Func::GetImageMemoryRequirements(cxt_core->device.handle, images[ci].handle),
                       images[ci].create_info.peek().tiling == VK_IMAGE_TILING_LINEAR, __prev_linear, _granularity, image_maps[ci], "image " + std::to_string(ci));
                // Note: the user size has no meaning for images, the range is what the device needs
                image_maps[ci].range = std::max(image_maps[ci].range, Func::GetImageMemoryRequirements(cxt_core->device.handle, images[ci].handle).size);
            }
            return __total;
        }

        void VulkanContext::ContextMemory::MemoryChunk::bindAndMap() {
            // Note: mapping a memory object once and keeping it mapped is allowed and cheaper than mapping it every frame
            if (required_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                mapped_ptr = Func::MapMemory(cxt_core->device.handle, memory.handle, 0u, VK_WHOLE_SIZE);

            for (uint32_t ci = 0u; ci < buffers.size(); ++ci)
                Func::BindBufferMemory(cxt_core->device.handle, buffers[ci].handle, memory.handle, buffer_maps[ci].offset);
            for (uint32_t ci = 0u; ci <  images.size(); ++ci)
                Func::BindImageMemory (cxt_core->device.handle,  images[ci].handle, memory.handle,  image_maps[ci].offset);
        }

        uint32_t VulkanContext::ContextMemory::MemoryChunk::addBuffer(uint32_t _buf_size) { buffers.emplace_back(); buffer_maps.push_back(MemoryMapping{0u, _buf_size, VK_QUEUE_FAMILY_IGNORED}); return buffers.size() - 1; }

        uint32_t VulkanContext::ContextMemory::MemoryChunk::addImage (uint32_t _img_size) {  images.emplace_back();  image_maps.push_back(MemoryMapping{0u, _img_size, VK_QUEUE_FAMILY_IGNORED}); return  images.size() - 1; }

        uint32_t VulkanContext::ContextMemory::MemoryChunk::addBufferView() { buffer_views.emplace_back(); return buffer_views.size() - 1; }

        uint32_t VulkanContext::ContextMemory::MemoryChunk::addImageView () {  image_views.emplace_back(); return  image_views.size() - 1; }

        void VulkanContext::ContextMemory::MemoryChunk::createBuffer(uint32_t _buffer_index) {
            buffers[_buffer_index].createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextMemory::MemoryChunk::createImage (uint32_t _image_index) {
            images [ _image_index].createObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextMemory::MemoryChunk::createBufferView(uint32_t _buffer_view_index, uint32_t _buffer_index) {
            buffer_views[_buffer_view_index].createObject(cxt_core->device.handle, buffers[_buffer_index].handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextMemory::MemoryChunk::createImageView (uint32_t  _image_view_index, uint32_t  _image_index) {
            image_views [ _image_view_index].createObject(cxt_core->device.handle,  images[ _image_index].handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextMemory::MemoryChunk::destroyBuffer(uint32_t _buffer_index) {
            buffers[_buffer_index].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextMemory::MemoryChunk::destroyImage (uint32_t _image_index) {
            images [ _image_index].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextMemory::MemoryChunk::destroyBufferView(uint32_t _buffer_view_index) {
            buffer_views[_buffer_view_index].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextMemory::MemoryChunk::destroyImageView (uint32_t  _image_view_index) {
            image_views [ _image_view_index].destroyObject(cxt_core->device.handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextMemory::MemoryChunk::place_(VkMemoryRequirements &_total, const VkMemoryRequirements &_req, bool _linear, int &_prev_linear,
                           VkDeviceSize _granularity, MemoryMapping &_map, const std::string &_what) {
            SVKFW_ASSERT(_total.memoryTypeBits & _req.memoryTypeBits, std::runtime_error, "VKFW::ContextMemory :: allocateChunkMemory",
                            "no memory type left that fits all objects of the chunk, at " + _what);
            _total.memoryTypeBits &= _req.memoryTypeBits;

            VkDeviceSize __alignment = std::max(_req.alignment, VkDeviceSize(1u));
            if (_prev_linear != -1 && _prev_linear != int(_linear))
                __alignment = std::max(__alignment, _granularity);
            _total.size += (__alignment - _total.size % __alignment) % __alignment;
            _total.alignment = std::max(_total.alignment, __alignment);
            _map.offset = _total.size;

            // Note: the device may require more memory than requested by user ('range'), so the max is used
            _total.size += std::max(_map.range, _req.size);
            _prev_linear = int(_linear);
        }

        VulkanContext::ContextMemory::ContextMemory(ContextCore *_cxt_core) { cxt_core = _cxt_core; for (uint32_t i = 0u; i < VK_MAX_MEMORY_HEAPS; ++i) heap_usage[i] = 0u; }

        VulkanContext::ContextMemory::~ContextMemory() { destroyContext(); }

        void VulkanContext::ContextMemory::initContext() {
            memory_properties.loadForPhysicalDevice(cxt_core->phys_device.handle);

            VkPhysicalDeviceProperties __properties{};
            vkGetPhysicalDeviceProperties(cxt_core->phys_device.handle, &__properties);
            non_coherent_atom_size   = std::max(__properties.limits.nonCoherentAtomSize,    VkDeviceSize(1u));
            buffer_image_granularity = std::max(__properties.limits.bufferImageGranularity, VkDeviceSize(1u));

            const std::vector<std::string> &__extensions = cxt_core->device.create_info.extensions.list;
            memory_budget_available = std::find(__extensions.begin(), __extensions.end(), std::string(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME)) != __extensions.end();
        }

        void VulkanContext::ContextMemory::destroyContext() {
            memory_chunks.forEach([this](MemoryId, MemoryChunk &_chunk) { releaseChunk_(_chunk); });
            memory_chunks.clear();
        }

        VulkanContext::ContextMemory::HeapBudget VulkanContext::ContextMemory::heapBudget(uint32_t _heap_index) const {
            const VkPhysicalDeviceMemoryProperties &__props = memory_properties.peek().memoryProperties;
            SVKFW_ASSERT(_heap_index < __props.memoryHeapCount, std::out_of_range, "VKFW::ContextMemory :: heapBudget", "heap index is out of range");
            HeapBudget __budget = { __props.memoryHeaps[_heap_index].size, heap_usage[_heap_index] };
            if (memory_budget_available) {
                StructChain<VkPhysicalDeviceMemoryProperties2, VkPhysicalDeviceMemoryBudgetPropertiesEXT> __chain;
                vkGetPhysicalDeviceMemoryProperties2(cxt_core->phys_device.handle, &__chain.head());
                __budget.budget = __chain.get<VkPhysicalDeviceMemoryBudgetPropertiesEXT>().heapBudget[_heap_index];
                __budget.usage  = __chain.get<VkPhysicalDeviceMemoryBudgetPropertiesEXT>().heapUsage [_heap_index];
            }
            return __budget;
        }

        VulkanContext::ContextMemory::MemoryChunk &VulkanContext::ContextMemory::chunk(MemoryId _id)       { return memory_chunks.get(_id); }

        const VulkanContext::ContextMemory::MemoryChunk &VulkanContext::ContextMemory::chunk(MemoryId _id) const { return memory_chunks.get(_id); }

        Object::Buffer     &VulkanContext::ContextMemory::buffer    (BufferId     _id)       { return at_(chunk(_id.parent).buffers,      _id.index); }

        const Object::Buffer     &VulkanContext::ContextMemory::buffer    (BufferId     _id) const { return at_(chunk(_id.parent).buffers,      _id.index); }

        Object::Image      &VulkanContext::ContextMemory::image     (ImageId      _id)       { return at_(chunk(_id.parent).images,       _id.index); }

        const Object::Image      &VulkanContext::ContextMemory::image     (ImageId      _id) const { return at_(chunk(_id.parent).images,       _id.index); }

        Object::BufferView &VulkanContext::ContextMemory::bufferView(BufferViewId _id)       { return at_(chunk(_id.parent).buffer_views, _id.index); }

        const Object::BufferView &VulkanContext::ContextMemory::bufferView(BufferViewId _id) const { return at_(chunk(_id.parent).buffer_views, _id.index); }

        Object::ImageView  &VulkanContext::ContextMemory::imageView (ImageViewId  _id)       { return at_(chunk(_id.parent).image_views,  _id.index); }

        const Object::ImageView  &VulkanContext::ContextMemory::imageView (ImageViewId  _id) const { return at_(chunk(_id.parent).image_views,  _id.index); }

        VulkanContext::ContextMemory::MemoryMapping &VulkanContext::ContextMemory::bufferMapping(BufferId _id)       { return at_(chunk(_id.parent).buffer_maps, _id.index); }

        const VulkanContext::ContextMemory::MemoryMapping &VulkanContext::ContextMemory::bufferMapping(BufferId _id) const { return at_(chunk(_id.parent).buffer_maps, _id.index); }

        VulkanContext::ContextMemory::MemoryMapping & VulkanContext::ContextMemory::imageMapping(ImageId  _id)       { return at_(chunk(_id.parent). image_maps, _id.index); }

        const VulkanContext::ContextMemory::MemoryMapping & VulkanContext::ContextMemory::imageMapping(ImageId  _id) const { return at_(chunk(_id.parent). image_maps, _id.index); }

        MemoryId VulkanContext::ContextMemory::addMemoryChunk(VkMemoryPropertyFlags _required_flags) {
            MemoryChunk __chunk(cxt_core);
            __chunk.required_flags = _required_flags;
            return memory_chunks.add(__chunk);
        }

        void VulkanContext::ContextMemory::removeMemoryChunk(MemoryId _memory_id) {
            if (!memory_chunks.contains(_memory_id)) return;
            releaseChunk_(memory_chunks.get(_memory_id));
            memory_chunks.remove(_memory_id);
        }

        void VulkanContext::ContextMemory::allocateChunkMemory(MemoryId _memory_id) {
            MemoryChunk &__chunk = chunk(_memory_id);
            SVKFW_ASSERT(__chunk.memory.handle == VK_NULL_HANDLE, std::logic_error, "VKFW::ContextMemory :: allocateChunkMemory", "the chunk is already allocated");
            const VkMemoryRequirements __req = __chunk.computeLayout(buffer_image_granularity);
            SVKFW_ASSERT(__req.size > 0u, std::logic_error, "VKFW::ContextMemory :: allocateChunkMemory", "the chunk has no buffers or images");

            const VkPhysicalDeviceMemoryProperties &__props = memory_properties.peek().memoryProperties;
            bool     __any_type = false;
            VkResult __vk_res   = VK_ERROR_OUT_OF_DEVICE_MEMORY;
            for (uint32_t __fits_pass = 0u; __fits_pass < 2u && __chunk.memory.handle == VK_NULL_HANDLE; ++__fits_pass) {
                for (uint32_t i = 0u; i < __props.memoryTypeCount; ++i) {
                    if ((__req.memoryTypeBits & (1u << i)) == 0u) continue; // can't hold the objects
                    if ((__props.memoryTypes[i].propertyFlags & __chunk.required_flags) != __chunk.required_flags) continue;
                    __any_type = true;

                    const uint32_t   __heap   = __props.memoryTypes[i].heapIndex;
                    const HeapBudget __budget = heapBudget(__heap);
                    const bool       __fits   = __budget.usage <= __budget.budget && __req.size <= __budget.budget - __budget.usage;
                    if (__fits != (__fits_pass == 0u)) continue;

                    __chunk.memory.allocate_info.edit().memoryTypeIndex = i;
                    __chunk.memory.allocate_info.edit().allocationSize  = __req.size;
                    VkDeviceMemory __memory = VK_NULL_HANDLE;
                    __vk_res = vkAllocateMemory(cxt_core->device.handle, &__chunk.memory.allocate_info.get(), cxt_core->alloc_cb.get(), &__memory);
                    if (__vk_res == VK_SUCCESS) {
                        __chunk.memory.handle = __memory;
                        __chunk.memory_type   = i;
                        __chunk.heap_index    = __heap;
                        heap_usage[__heap]   += __req.size;
                        break;
                    }
                    SVKFW_ASSERT(__vk_res == VK_ERROR_OUT_OF_DEVICE_MEMORY || __vk_res == VK_ERROR_OUT_OF_HOST_MEMORY, std::runtime_error,
                                    "VKFW::ContextMemory :: allocateChunkMemory", "allocation failed, code " + std::to_string(__vk_res));
                }
            }
            SVKFW_ASSERT(__any_type, std::runtime_error, "VKFW::ContextMemory :: allocateChunkMemory",
                            "no memory type has the required properties and can hold the buffers/images of the chunk");
            SVKFW_ASSERT(__chunk.memory.handle != VK_NULL_HANDLE, std::runtime_error, "VKFW::ContextMemory :: allocateChunkMemory",
                            "out of memory: could not allocate " + std::to_string(__req.size) + " bytes, code " + std::to_string(__vk_res));
            __chunk.bindAndMap();
        }

        BufferId     VulkanContext::ContextMemory::addChunkBuffer    (MemoryId _memory_id, uint32_t _buf_size) { return BufferId    (_memory_id, chunk(_memory_id).addBuffer(_buf_size)); }

        ImageId      VulkanContext::ContextMemory::addChunkImage     (MemoryId _memory_id, uint32_t _img_size) { return ImageId     (_memory_id, chunk(_memory_id).addImage (_img_size)); }

        BufferViewId VulkanContext::ContextMemory::addChunkBufferView(MemoryId _memory_id) { return BufferViewId(_memory_id, chunk(_memory_id).addBufferView()); }

        ImageViewId  VulkanContext::ContextMemory::addChunkImageView (MemoryId _memory_id) { return ImageViewId (_memory_id, chunk(_memory_id).addImageView ()); }

        void VulkanContext::ContextMemory::createChunkBuffer(BufferId _buffer_id) { buffer(_buffer_id).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get()); }

        void VulkanContext::ContextMemory::createChunkImage (ImageId  _image_id ) { image (_image_id ).createObject(cxt_core->device.handle, cxt_core->alloc_cb.get()); }

        void VulkanContext::ContextMemory::createChunkBufferView(BufferId _buffer_id, BufferViewId _buf_view_id) {
            bufferView(_buf_view_id).createObject(cxt_core->device.handle, buffer(_buffer_id).handle, cxt_core->alloc_cb.get());
        }

        void VulkanContext::ContextMemory::createChunkImageView (ImageId  _image_id , ImageViewId  _img_view_id) {
            imageView (_img_view_id).createObject(cxt_core->device.handle, image (_image_id ).handle, cxt_core->alloc_cb.get());
        }

        void* VulkanContext::ContextMemory::getMappedChunk (MemoryId _memory_id) const { return chunk(_memory_id).mapped_ptr; }

        void* VulkanContext::ContextMemory::getMappedBuffer(BufferId _buffer_id) const {
            void *__chunk_ptr = getMappedChunk(_buffer_id.parent);
            return __chunk_ptr == nullptr ? nullptr : (uint8_t*)__chunk_ptr + bufferMapping(_buffer_id).offset;
        }

        void* VulkanContext::ContextMemory::getMappedImage (ImageId  _image_id ) const {
            void *__chunk_ptr = getMappedChunk(_image_id.parent);
            return __chunk_ptr == nullptr ? nullptr : (uint8_t*)__chunk_ptr + imageMapping(_image_id).offset;
        }

        bool VulkanContext::ContextMemory::isChunkCoherent(MemoryId _memory_id) const {
            return (chunk(_memory_id).required_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0u;
        }

        void VulkanContext::ContextMemory::flushBuffer(BufferId _buffer_id, VkDeviceSize _offset, VkDeviceSize _size) const {
            if (isChunkCoherent(_buffer_id.parent)) return;
            VkMappedMemoryRange __range = mappedRange_(_buffer_id, _offset, _size);
            SVKFW_ASSERT(vkFlushMappedMemoryRanges(cxt_core->device.handle, 1u, &__range) == VK_SUCCESS, std::runtime_error,
                            "VKFW::ContextMemory :: flushBuffer", "could not flush mapped memory");
        }

        void VulkanContext::ContextMemory::invalidateBuffer(BufferId _buffer_id, VkDeviceSize _offset, VkDeviceSize _size) const {
            if (isChunkCoherent(_buffer_id.parent)) return;
            VkMappedMemoryRange __range = mappedRange_(_buffer_id, _offset, _size);
            SVKFW_ASSERT(vkInvalidateMappedMemoryRanges(cxt_core->device.handle, 1u, &__range) == VK_SUCCESS, std::runtime_error,
                            "VKFW::ContextMemory :: invalidateBuffer", "could not invalidate mapped memory");
        }

        void VulkanContext::ContextMemory::writeBuffer(BufferId _buffer_id, const void *_data, VkDeviceSize _size, VkDeviceSize _offset) const {
            uint8_t *__ptr = (uint8_t*)checkMappedRange_(_buffer_id, _offset, _size, "VKFW::ContextMemory :: writeBuffer");
            memcpy(__ptr + _offset, _data, _size);
            flushBuffer(_buffer_id, _offset, _size);
        }

        void VulkanContext::ContextMemory::readBuffer(BufferId _buffer_id, void *_data, VkDeviceSize _size, VkDeviceSize _offset) const {
            const uint8_t *__ptr = (const uint8_t*)checkMappedRange_(_buffer_id, _offset, _size, "VKFW::ContextMemory :: readBuffer");
            invalidateBuffer(_buffer_id, _offset, _size);
            memcpy(_data, __ptr + _offset, _size);
        }

        void VulkanContext::ContextMemory::setWholeBufferCreateInfoStruct(BufferId _buffer_id, Span<QueueFamilyId> _qfamilies_id,
                                            VkSharingMode _sharing_mode, VkBufferUsageFlags _usage_flags, VkDeviceSize _size_bytes) {
            if (_size_bytes > 0u)
                bufferMapping(_buffer_id).range = _size_bytes;

            std::vector<VkfwQueueFamily> __qfamilies((size_t)_qfamilies_id.size());
            for (uint32_t i = 0u; i < _qfamilies_id.size(); ++i)
                __qfamilies[i] = cxt_core->queueFamily(_qfamilies_id[i]).handle;
            buffer(_buffer_id).create_info.setWholeStruct(__qfamilies, _sharing_mode, _usage_flags, bufferMapping(_buffer_id).range);
        }

        void VulkanContext::ContextMemory::setWholeImageCreateInfoStruct(ImageId _image_id, Span<QueueFamilyId> _qfamilies_id, VkSharingMode _sharing_mode,
                                           VkImageUsageFlags _usage_flags, VkFormat _format, VkExtent3D _extent,
                                           uint32_t _mip_levels, uint32_t _array_layers, VkSampleCountFlagBits _samples,
                                           VkImageTiling _tiling, VkImageCreateFlags _flags) {
            std::vector<uint32_t> __qfamilies;
            for (uint32_t i = 0u; i < _qfamilies_id.size(); ++i) {
                const uint32_t __family = cxt_core->queueFamily(_qfamilies_id[i]).handle;
                if (std::find(__qfamilies.begin(), __qfamilies.end(), __family) == __qfamilies.end()) __qfamilies.push_back(__family);
            }
            if (__qfamilies.size() < 2u) {
                _sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
                __qfamilies.clear();
            }
            image(_image_id).create_info.setWholeStruct(__qfamilies, _sharing_mode, _usage_flags, _format, _extent,
                                                       _mip_levels, _array_layers, _samples, _tiling, _flags);
        }

        void VulkanContext::ContextMemory::setWholeImageViewCreateInfoStruct(ImageViewId _img_view_id, ImageId _image_id, VkImageAspectFlags _aspect,
                                               VkImageViewType _type) {
            const VkImageCreateInfo &__image = image(_image_id).create_info.peek();
            if (_aspect == 0u) {
                _aspect = Detail::formatAspects(__image.format);
                if (_aspect == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) && !(__image.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
                    _aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            if (_type == VK_IMAGE_VIEW_TYPE_MAX_ENUM) {
                const bool __cube = (__image.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) && __image.arrayLayers % 6u == 0u;
                _type = __image.imageType == VK_IMAGE_TYPE_3D ? VK_IMAGE_VIEW_TYPE_3D :
                        __image.imageType == VK_IMAGE_TYPE_1D ? (__image.arrayLayers > 1u ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D) :
                        __cube                                ? (__image.arrayLayers > 6u ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE) :
                                                                (__image.arrayLayers > 1u ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D);
            }
            Struct::StructWrap<VkImageViewCreateInfo> &__view = imageView(_img_view_id).create_info;
            __view.edit().viewType = _type;
            __view.edit().format   = __image.format;
            __view.setComponents();
            __view.setRange(_aspect, 0u, __image.mipLevels, 0u, __image.arrayLayers);
        }

        void VulkanContext::ContextMemory::releaseChunk_(MemoryChunk &_chunk) {
            if (_chunk.memory.handle != VK_NULL_HANDLE && _chunk.heap_index < VK_MAX_MEMORY_HEAPS)
                heap_usage[_chunk.heap_index] -= _chunk.memory.allocate_info.peek().allocationSize;
            _chunk.destroyChunk();
            _chunk.memory_type = _chunk.heap_index = UINT32_MAX;
        }

        void* VulkanContext::ContextMemory::checkMappedRange_(BufferId _buffer_id, VkDeviceSize _offset, VkDeviceSize _size, const char *_where) const {
            void *__ptr = getMappedBuffer(_buffer_id);
            SVKFW_ASSERT(__ptr != nullptr, std::invalid_argument, "VKFW::ContextMemory", "buffer memory is not HOST_VISIBLE or not allocated, in " + std::string(_where));
            SVKFW_ASSERT(_offset + _size <= bufferMapping(_buffer_id).range, std::out_of_range, "VKFW::ContextMemory",
                            "range is out of the buffer: " + std::to_string(_offset + _size) + " > " + std::to_string(bufferMapping(_buffer_id).range) + ", in " + _where);
            return __ptr;
        }

        VkMappedMemoryRange VulkanContext::ContextMemory::mappedRange_(BufferId _buffer_id, VkDeviceSize _offset, VkDeviceSize _size) const {
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

        VulkanContext::ContextBarrier::ContextBarrier(ContextCore *_cxt_core) { cxt_core = _cxt_core; }

        VulkanContext::ContextBarrier::~ContextBarrier() { destroyContext(); }

        void VulkanContext::ContextBarrier::createContext() {}

        void VulkanContext::ContextBarrier::destroyContext() {
            VkDevice                     __device = cxt_core->device.handle;
            const VkAllocationCallbacks *__alloc_cb = cxt_core->alloc_cb.get();

            semaphores.forEach([&](SemaphoreId, VkSemaphore &_semaphore) { vkDestroySemaphore(__device, _semaphore, __alloc_cb); });
            fences    .forEach([&](FenceId,     VkFence     &_fence    ) { vkDestroyFence    (__device, _fence,     __alloc_cb); });
            events    .forEach([&](EventId,     VkEvent     &_event    ) { vkDestroyEvent    (__device, _event,     __alloc_cb); });
            semaphores.clear();
            fences    .clear();
            events    .clear();
        }

        SemaphoreId VulkanContext::ContextBarrier::addSemaphore() {
            VkSemaphore __semaphore = VK_NULL_HANDLE;
            if (vkCreateSemaphore(cxt_core->device.handle, &semaphore_wrap.get(), cxt_core->alloc_cb.get(), &__semaphore) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("VKFW::ContextBarrier :: addSemaphore", "could not create semaphore"));
            return semaphores.add(__semaphore);
        }

        FenceId VulkanContext::ContextBarrier::addFence(bool _is_signaled) {
            VkFence __fence = VK_NULL_HANDLE;
            fence_wrap.setWholeStruct(_is_signaled);
            if (vkCreateFence(cxt_core->device.handle, &fence_wrap.get(), cxt_core->alloc_cb.get(), &__fence) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("VKFW::ContextBarrier :: addFence", "could not create fence"));
            return fences.add(__fence);
        }

        EventId VulkanContext::ContextBarrier::addEvent(bool _is_signaled) {
            VkEvent __vk_event = VK_NULL_HANDLE;
            event_wrap.setWholeStruct(_is_signaled);
            if (vkCreateEvent(cxt_core->device.handle, &event_wrap.get(), cxt_core->alloc_cb.get(), &__vk_event) != VK_SUCCESS)
                throw std::runtime_error(SVKFW_WRAPERR("VKFW::ContextBarrier :: addEvent", "could not create event"));
            return events.add(__vk_event);
        }

        VkResult VulkanContext::ContextBarrier::wait(Span<FenceId> _fences_id, bool _wait_all, uint64_t _timeout) const {
            InlineArray<VkFence> __fences(_fences_id.size());
            for (uint32_t i = 0u; i < __fences.size(); ++i) __fences[i] = fences.get(_fences_id[i]);
            VkResult __res = vkWaitForFences(cxt_core->device.handle, __fences.size(), __fences.data(), _wait_all ? VK_TRUE : VK_FALSE, _timeout);
            SVKFW_ASSERT(__res == VK_SUCCESS || __res == VK_TIMEOUT, std::runtime_error, "VKFW::ContextBarrier :: wait", "could not wait for fences, code " + std::to_string(__res));
            return __res;
        }

        void VulkanContext::ContextBarrier::reset(Span<FenceId> _fences_id) const {
            InlineArray<VkFence> __fences(_fences_id.size());
            for (uint32_t i = 0u; i < __fences.size(); ++i) __fences[i] = fences.get(_fences_id[i]);
            SVKFW_ASSERT(vkResetFences(cxt_core->device.handle, __fences.size(), __fences.data()) == VK_SUCCESS, std::runtime_error,
                            "VKFW::ContextBarrier :: reset", "could not reset fences");
        }

        void VulkanContext::ContextBarrier::destroySemaphore(SemaphoreId _semaphore_id) {
            if (!semaphores.contains(_semaphore_id)) return;
            vkDestroySemaphore(cxt_core->device.handle, semaphores.get(_semaphore_id), cxt_core->alloc_cb.get());
            semaphores.remove(_semaphore_id);
        }

        void VulkanContext::ContextBarrier::destroyFence(FenceId _fence_id) {
            if (!fences.contains(_fence_id)) return;
            vkDestroyFence(cxt_core->device.handle, fences.get(_fence_id), cxt_core->alloc_cb.get());
            fences.remove(_fence_id);
        }

        void VulkanContext::ContextBarrier::destroyEvent(EventId _event_id) {
            if (!events.contains(_event_id)) return;
            vkDestroyEvent(cxt_core->device.handle, events.get(_event_id), cxt_core->alloc_cb.get());
            events.remove(_event_id);
        }

        VulkanContext::VulkanContext() : cxt_core{this},
                          cxt_swapchain{&cxt_core}, cxt_descriptor{&cxt_core}, cxt_pipeline{&cxt_core},
                          cxt_command  {&cxt_core}, cxt_memory    {&cxt_core}, cxt_barrier {&cxt_core} {}

        VulkanContext::~VulkanContext() { /* destroyContext();  */ }

        void VulkanContext::destroyContext() {
            cxt_barrier.destroyContext();
            cxt_memory.destroyContext();
            cxt_command.destroyContext();
            cxt_pipeline.destroyContext();
            cxt_swapchain.destroyContext();
            cxt_descriptor.destroyContext();
            cxt_core.destroyContext();
        }

        VkfwQueueFamily       VulkanContext::get(QueueFamilyId         _id) const { return _id.isNone() ? UINT32_MAX     : cxt_core.queueFamily(_id).handle; }

        VkQueue               VulkanContext::get(QueueId               _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_core.queue(_id); }

        VkDescriptorSetLayout VulkanContext::get(DescriptorSetLayoutId _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_descriptor.layouts.get(_id).handle; }

        VkDescriptorPool      VulkanContext::get(DescriptorPoolId      _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_descriptor.pools  .get(_id).handle; }

        VkDescriptorSet       VulkanContext::get(DescriptorSetId       _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_descriptor.set(_id); }

        VkSampler             VulkanContext::get(SamplerId             _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_descriptor.samplers.get(_id).handle; }

        VkPipeline            VulkanContext::get(PipelineId            _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_pipeline.pipelines.get(_id).handle; }

        VkPipelineLayout      VulkanContext::get(PipelineLayoutId      _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_pipeline.layouts  .get(_id).handle; }

        VkPipelineCache       VulkanContext::get(PipelineCacheId       _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_pipeline.caches   .get(_id).handle; }

        VkShaderModule        VulkanContext::get(ShaderModuleId        _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_pipeline.shaders  .get(_id).handle; }

        VkCommandPool         VulkanContext::get(CommandPoolId         _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_command.cmd_pools.get(_id).handle; }

        VkCommandBuffer       VulkanContext::get(CommandBufferId       _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_command.commandBuffer(_id); }

        VkDeviceMemory        VulkanContext::get(MemoryId              _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_memory.chunk(_id).memory.handle; }

        VkBuffer              VulkanContext::get(BufferId              _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_memory.buffer    (_id).handle; }

        VkImage               VulkanContext::get(ImageId               _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_memory.image     (_id).handle; }

        VkBufferView          VulkanContext::get(BufferViewId          _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_memory.bufferView(_id).handle; }

        VkImageView           VulkanContext::get(ImageViewId           _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_memory.imageView (_id).handle; }

        VkSemaphore           VulkanContext::get(SemaphoreId           _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_barrier.semaphores.get(_id); }

        VkFence               VulkanContext::get(FenceId               _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_barrier.fences    .get(_id); }

        VkEvent               VulkanContext::get(EventId               _id) const { return _id.isNone() ? VK_NULL_HANDLE : cxt_barrier.events    .get(_id); }

        void VulkanContext::submit(QueueId _queue_id, Span<SubmitDesc> _submits, FenceId _fence_id) const {
            if (cxt_core.profile.synchronization2) submit2_(_queue_id, _submits, _fence_id);
            else                                   submit1_(_queue_id, _submits, _fence_id);
        }

        AcquireResult VulkanContext::acquireNextImage(SemaphoreId _semaphore_id, FenceId _fence_id, uint64_t _timeout) {
            return cxt_swapchain.acquire(get(_semaphore_id), get(_fence_id), _timeout);
        }

        SwapchainStatus VulkanContext::present(QueueId _queue_id, Span<SemaphoreId> _wait_semaphores_id, uint32_t _image_index) {
            InlineArray<VkSemaphore> __semaphores(_wait_semaphores_id.size());
            fill(_wait_semaphores_id, __semaphores.data());
            return cxt_swapchain.present(get(_queue_id), __semaphores, _image_index);
        }

        void VulkanContext::recreateSwapchain() {
            vkDeviceWaitIdle(cxt_core.device.handle);
            cxt_swapchain.recreate();
        }

        void VulkanContext::initNoncoreContexts() {
            cxt_memory.initContext();
        }

        void VulkanContext::handleResult(VkResult _result) {
            // TODO: handle VkResult
        }

        void VulkanContext::submit2_(QueueId _queue_id, Span<SubmitDesc> _submits, FenceId _fence_id) const {
            struct Infos {
                VkSemaphoreSubmitInfo     wait  [VKFW_MAX_SUBMIT_SEMAPHORES];
                VkCommandBufferSubmitInfo cmd   [VKFW_MAX_SUBMIT_COMMAND_BUFFERS];
                VkSemaphoreSubmitInfo     signal[VKFW_MAX_SUBMIT_SEMAPHORES];
            };
            InlineArray<Infos,         4u> __infos  (_submits.size());
            InlineArray<VkSubmitInfo2, 4u> __submits(_submits.size());

            for (uint32_t i = 0u; i < _submits.size(); ++i) {
                const SubmitDesc &__desc = _submits[i];
                for (uint32_t j = 0u; j < __desc.wait_count; ++j) {
                    __infos[i].wait[j] = VkSemaphoreSubmitInfo{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO};
                    __infos[i].wait[j].semaphore = get(__desc.wait_semaphores[j]);
                    __infos[i].wait[j].stageMask = __desc.wait_stages[j];
                }
                for (uint32_t j = 0u; j < __desc.command_count; ++j) {
                    __infos[i].cmd[j] = VkCommandBufferSubmitInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO};
                    __infos[i].cmd[j].commandBuffer = get(__desc.command_buffers[j]);
                }
                for (uint32_t j = 0u; j < __desc.signal_count; ++j) {
                    __infos[i].signal[j] = VkSemaphoreSubmitInfo{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO};
                    __infos[i].signal[j].semaphore = get(__desc.signal_semaphores[j]);
                    __infos[i].signal[j].stageMask = __desc.signal_stages[j];
                }

                __submits[i] = VkSubmitInfo2{VK_STRUCTURE_TYPE_SUBMIT_INFO_2};
                __submits[i].waitSemaphoreInfoCount   = __desc.wait_count;
                __submits[i].pWaitSemaphoreInfos      = __infos[i].wait;
                __submits[i].commandBufferInfoCount   = __desc.command_count;
                __submits[i].pCommandBufferInfos      = __infos[i].cmd;
                __submits[i].signalSemaphoreInfoCount = __desc.signal_count;
                __submits[i].pSignalSemaphoreInfos    = __infos[i].signal;
            }

            Func::QueueSubmit2(get(_queue_id), Span<VkSubmitInfo2>(__submits.data(), __submits.size()), get(_fence_id));
        }

        void VulkanContext::submit1_(QueueId _queue_id, Span<SubmitDesc> _submits, FenceId _fence_id) const {
            struct Handles {
                VkSemaphore          wait  [VKFW_MAX_SUBMIT_SEMAPHORES];
                VkPipelineStageFlags stages[VKFW_MAX_SUBMIT_SEMAPHORES];
                VkCommandBuffer      cmd   [VKFW_MAX_SUBMIT_COMMAND_BUFFERS];
                VkSemaphore          signal[VKFW_MAX_SUBMIT_SEMAPHORES];
            };
            InlineArray<Handles,      4u> __handles(_submits.size());
            InlineArray<VkSubmitInfo, 4u> __infos  (_submits.size());

            for (uint32_t i = 0u; i < _submits.size(); ++i) {
                const SubmitDesc &__desc = _submits[i];
                fill(Span<SemaphoreId    >(__desc.wait_semaphores,   __desc.wait_count   ), __handles[i].wait  );
                fill(Span<CommandBufferId>(__desc.command_buffers,   __desc.command_count), __handles[i].cmd   );
                fill(Span<SemaphoreId    >(__desc.signal_semaphores, __desc.signal_count ), __handles[i].signal);
                for (uint32_t j = 0u; j < __desc.wait_count; ++j)
                    __handles[i].stages[j] = Detail::toStageFlags1(__desc.wait_stages[j], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

                __infos[i] = VkSubmitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
                __infos[i].waitSemaphoreCount   = __desc.wait_count;
                __infos[i].pWaitSemaphores      = __handles[i].wait;
                __infos[i].pWaitDstStageMask    = __handles[i].stages;
                __infos[i].commandBufferCount   = __desc.command_count;
                __infos[i].pCommandBuffers      = __handles[i].cmd;
                __infos[i].signalSemaphoreCount = __desc.signal_count;
                __infos[i].pSignalSemaphores    = __handles[i].signal;
            }

            Func::QueueSubmit(get(_queue_id), Span<VkSubmitInfo>(__infos.data(), __infos.size()), get(_fence_id));
        }

        CommandRecorder::CommandRecorder(const VulkanContext &_cxt, CommandBufferId _cmd_buffer_id) : cxt(_cxt), cmd{_cxt.get(_cmd_buffer_id)} {}

        CommandRecorder::CommandRecorder(const VulkanContext &_cxt, VkCommandBuffer _cmd_buffer   ) : cxt(_cxt), cmd{_cmd_buffer} {}

        VkCommandBuffer CommandRecorder::handle() const { return cmd; }

        void CommandRecorder::begin(VkCommandBufferUsageFlags _flags, const VkCommandBufferInheritanceInfo *_inheritance) {
            VkCommandBufferBeginInfo __info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            __info.flags            = _flags;
            __info.pInheritanceInfo = _inheritance;
            Func::BeginCommandBuffer(cmd, &__info);
        }

        void CommandRecorder::end() {
            SVKFW_ASSERT(vkEndCommandBuffer(cmd) == VK_SUCCESS, std::runtime_error, "VKFW::CommandRecorder :: end", "could not end command buffer");
        }

        void CommandRecorder::reset(bool _release_resources) {
            vkResetCommandBuffer(cmd, _release_resources ? VkCommandBufferResetFlags(VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) : VkCommandBufferResetFlags(0u));
        }

        void CommandRecorder::beginRenderPass(VkFramebuffer _framebuffer, VkExtent2D _extent, Span<VkClearValue> _clear_values,
                             VkSubpassContents _contents) {
            VkRenderPassBeginInfo __info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            __info.renderPass        = cxt.cxt_core.render_pass.handle;
            __info.framebuffer       = _framebuffer;
            __info.renderArea.extent = _extent;
            __info.clearValueCount   = _clear_values.size();
            __info.pClearValues      = _clear_values.data();
            vkCmdBeginRenderPass(cmd, &__info, _contents);
        }

        void CommandRecorder::beginRenderPass(const VkRenderPassBeginInfo &_info, VkSubpassContents _contents) {
            vkCmdBeginRenderPass(cmd, &_info, _contents);
        }

        void CommandRecorder::nextSubpass(VkSubpassContents _contents) {
            vkCmdNextSubpass(cmd, _contents);
        }

        void CommandRecorder::endRenderPass() {
            vkCmdEndRenderPass(cmd);
        }

        void CommandRecorder::beginRendering(const VkRenderingInfo &_info) {
            vkCmdBeginRendering(cmd, &_info);
        }

        void CommandRecorder::beginRendering(VkExtent2D _extent, Span<VkRenderingAttachmentInfo> _color_attachments,
                            const VkRenderingAttachmentInfo *_depth_attachment, const VkRenderingAttachmentInfo *_stencil_attachment,
                            uint32_t _layer_count, VkRenderingFlags _flags) {
            VkRenderingInfo __info{VK_STRUCTURE_TYPE_RENDERING_INFO};
            __info.flags                = _flags;
            __info.renderArea.extent    = _extent;
            __info.layerCount           = _layer_count;
            __info.colorAttachmentCount = _color_attachments.size();
            __info.pColorAttachments    = _color_attachments.data();
            __info.pDepthAttachment     = _depth_attachment;
            __info.pStencilAttachment   = _stencil_attachment;
            vkCmdBeginRendering(cmd, &__info);
        }

        void CommandRecorder::endRendering() {
            vkCmdEndRendering(cmd);
        }

        void CommandRecorder::beginSwapchainRendering(uint32_t _image_index, const VkClearValue &_clear_value) {
            const VulkanContext::ContextSwapchain &__swapchain = cxt.cxt_swapchain;
            if (!cxt.cxt_core.profile.dynamic_rendering) {
                beginRenderPass(__swapchain.framebuffer(_image_index), __swapchain.extent(), _clear_value);
                return;
            }
            barrier(imageBarrier(__swapchain.image(_image_index), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                 VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_NONE,
                                 VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT));
            VkRenderingAttachmentInfo __color = renderingAttachment(__swapchain.imageView(_image_index), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                    VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, _clear_value);
            beginRendering(__swapchain.extent(), __color);
        }

        void CommandRecorder::endSwapchainRendering(uint32_t _image_index) {
            if (!cxt.cxt_core.profile.dynamic_rendering) {
                endRenderPass();
                return;
            }
            endRendering();
            barrier(imageBarrier(cxt.cxt_swapchain.image(_image_index), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                 VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                 VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_NONE));
        }

        void CommandRecorder::bindPipeline(PipelineId _pipeline_id) {
            VkPipelineBindPoint __bind_point;
            switch (cxt.cxt_pipeline.pipelines.get(_pipeline_id).pipeline_type) {
                case VKFW_PIPELINE_TYPE_GRAPHICS   : __bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS       ; break;
                case VKFW_PIPELINE_TYPE_COMPUTE    : __bind_point = VK_PIPELINE_BIND_POINT_COMPUTE        ; break;
                case VKFW_PIPELINE_TYPE_RAY_TRACING: __bind_point = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR; break;
                default: throw std::runtime_error(SVKFW_WRAPERR("VKFW::CommandRecorder :: bindPipeline", "pipeline type is not set"));
            }
            vkCmdBindPipeline(cmd, __bind_point, cxt.get(_pipeline_id));
        }

        void CommandRecorder::bindDescriptorSets(PipelineLayoutId _layout_id, Span<DescriptorSetId> _sets_id, Span<uint32_t> _dynamic_offsets,
                                VkPipelineBindPoint _bind_point, uint32_t _first_set) {
            InlineArray<VkDescriptorSet> __sets(_sets_id.size());
            cxt.fill(_sets_id, __sets.data());
            vkCmdBindDescriptorSets(cmd, _bind_point, cxt.get(_layout_id), _first_set, __sets.size(), __sets.data(), _dynamic_offsets.size(), _dynamic_offsets.data());
        }

        void CommandRecorder::bindVertexBuffers(Span<BufferId> _buffers_id, Span<VkDeviceSize> _offsets, uint32_t _first_binding) {
            InlineArray<VkBuffer>     __buffers(_buffers_id.size());
            InlineArray<VkDeviceSize> __offsets(_buffers_id.size());
            cxt.fill(_buffers_id, __buffers.data());
            for (uint32_t i = 0u; i < __offsets.size(); ++i)
                __offsets[i] = i < _offsets.size() ? _offsets[i] : 0u;
            vkCmdBindVertexBuffers(cmd, _first_binding, __buffers.size(), __buffers.data(), __offsets.data());
        }

        void CommandRecorder::bindIndexBuffer(BufferId _buffer_id, VkDeviceSize _offset, VkIndexType _index_type) {
            vkCmdBindIndexBuffer(cmd, cxt.get(_buffer_id), _offset, _index_type);
        }

        void CommandRecorder::pushConstants(PipelineLayoutId _layout_id, VkShaderStageFlags _stages, uint32_t _offset, uint32_t _size, const void *_data) {
            vkCmdPushConstants(cmd, cxt.get(_layout_id), _stages, _offset, _size, _data);
        }

        void CommandRecorder::setViewports(Span<VkViewport> _viewports, uint32_t _first) { vkCmdSetViewport(cmd, _first, _viewports.size(), _viewports.data()); }

        void CommandRecorder::setScissors (Span<VkRect2D>   _scissors,  uint32_t _first) { vkCmdSetScissor (cmd, _first, _scissors .size(), _scissors .data()); }

        void CommandRecorder::setLineWidth(float _line_width) { vkCmdSetLineWidth(cmd, _line_width); }

        void CommandRecorder::setViewportScissor(VkExtent2D _extent) {
            VkViewport __viewport{};
            __viewport.width    = (float)_extent.width;
            __viewport.height   = (float)_extent.height;
            __viewport.maxDepth = 1.f;
            VkRect2D __scissor{};
            __scissor.extent = _extent;
            vkCmdSetViewport(cmd, 0u, 1u, &__viewport);
            vkCmdSetScissor (cmd, 0u, 1u, &__scissor );
        }

        void CommandRecorder::draw(uint32_t _vert_count, uint32_t _inst_count, uint32_t _first_vert, uint32_t _first_inst) {
            vkCmdDraw(cmd, _vert_count, _inst_count, _first_vert, _first_inst);
        }

        void CommandRecorder::drawIndexed(uint32_t _ind_count, uint32_t _inst_count, int32_t _vert_offset, uint32_t _first_ind, uint32_t _first_inst) {
            vkCmdDrawIndexed(cmd, _ind_count, _inst_count, _first_ind, _vert_offset, _first_inst);
        }

        void CommandRecorder::dispatch(uint32_t _x, uint32_t _y, uint32_t _z) {
            vkCmdDispatch(cmd, _x, _y, _z);
        }

        void CommandRecorder::clearAttachments(Span<VkClearAttachment> _attachments, Span<VkClearRect> _rects) {
            vkCmdClearAttachments(cmd, _attachments.size(), _attachments.data(), _rects.size(), _rects.data());
        }

        void CommandRecorder::copyBuffer(BufferId _src_id, BufferId _dst_id, Span<VkBufferCopy> _regions) {
            vkCmdCopyBuffer(cmd, cxt.get(_src_id), cxt.get(_dst_id), _regions.size(), _regions.data());
        }

        void CommandRecorder::copyImage(ImageId _src_id, VkImageLayout _src_layout, ImageId _dst_id, VkImageLayout _dst_layout, Span<VkImageCopy> _regions) {
            vkCmdCopyImage(cmd, cxt.get(_src_id), _src_layout, cxt.get(_dst_id), _dst_layout, _regions.size(), _regions.data());
        }

        void CommandRecorder::copyBufferToImage(BufferId _src_id, ImageId _dst_id, VkImageLayout _dst_layout, Span<VkBufferImageCopy> _regions) {
            vkCmdCopyBufferToImage(cmd, cxt.get(_src_id), cxt.get(_dst_id), _dst_layout, _regions.size(), _regions.data());
        }

        void CommandRecorder::blitImage(ImageId _src_id, VkImageLayout _src_layout, ImageId _dst_id, VkImageLayout _dst_layout, Span<VkImageBlit> _regions,
                                        VkFilter _filter) {
            vkCmdBlitImage(cmd, cxt.get(_src_id), _src_layout, cxt.get(_dst_id), _dst_layout, _regions.size(), _regions.data(), _filter);
        }

        void CommandRecorder::barrier(Span<VkImageMemoryBarrier2> _image_barriers, Span<VkBufferMemoryBarrier2> _buffer_barriers,
                     Span<VkMemoryBarrier2> _memory_barriers, VkDependencyFlags _flags) {
            if (cxt.cxt_core.profile.synchronization2) {
                VkDependencyInfo __info{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
                __info.dependencyFlags          = _flags;
                __info.memoryBarrierCount       = _memory_barriers.size();
                __info.pMemoryBarriers          = _memory_barriers.data();
                __info.bufferMemoryBarrierCount = _buffer_barriers.size();
                __info.pBufferMemoryBarriers    = _buffer_barriers.data();
                __info.imageMemoryBarrierCount  = _image_barriers.size();
                __info.pImageMemoryBarriers     = _image_barriers.data();
                vkCmdPipelineBarrier2(cmd, &__info);
                return;
            }
            for (uint32_t i = 0u; i < _memory_barriers.size(); ++i) {
                const VkMemoryBarrier2 &__b = _memory_barriers[i];
                VkMemoryBarrier __barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
                __barrier.srcAccessMask = Detail::toAccessFlags1(__b.srcAccessMask);
                __barrier.dstAccessMask = Detail::toAccessFlags1(__b.dstAccessMask);
                pipelineBarrier(Detail::toStageFlags1(__b.srcStageMask, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                Detail::toStageFlags1(__b.dstStageMask, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT), __barrier, {}, {}, _flags);
            }
            for (uint32_t i = 0u; i < _buffer_barriers.size(); ++i) {
                const VkBufferMemoryBarrier2 &__b = _buffer_barriers[i];
                VkBufferMemoryBarrier __barrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
                __barrier.srcAccessMask       = Detail::toAccessFlags1(__b.srcAccessMask);
                __barrier.dstAccessMask       = Detail::toAccessFlags1(__b.dstAccessMask);
                __barrier.srcQueueFamilyIndex = __b.srcQueueFamilyIndex;
                __barrier.dstQueueFamilyIndex = __b.dstQueueFamilyIndex;
                __barrier.buffer              = __b.buffer;
                __barrier.offset              = __b.offset;
                __barrier.size                = __b.size;
                pipelineBarrier(Detail::toStageFlags1(__b.srcStageMask, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                Detail::toStageFlags1(__b.dstStageMask, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT), {}, __barrier, {}, _flags);
            }
            for (uint32_t i = 0u; i < _image_barriers.size(); ++i) {
                const VkImageMemoryBarrier2 &__b = _image_barriers[i];
                VkImageMemoryBarrier __barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
                __barrier.srcAccessMask       = Detail::toAccessFlags1(__b.srcAccessMask);
                __barrier.dstAccessMask       = Detail::toAccessFlags1(__b.dstAccessMask);
                __barrier.oldLayout           = __b.oldLayout;
                __barrier.newLayout           = __b.newLayout;
                __barrier.srcQueueFamilyIndex = __b.srcQueueFamilyIndex;
                __barrier.dstQueueFamilyIndex = __b.dstQueueFamilyIndex;
                __barrier.image               = __b.image;
                __barrier.subresourceRange    = __b.subresourceRange;
                pipelineBarrier(Detail::toStageFlags1(__b.srcStageMask, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                                Detail::toStageFlags1(__b.dstStageMask, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT), {}, {}, __barrier, _flags);
            }
        }

        void CommandRecorder::pipelineBarrier(VkPipelineStageFlags _src_stages, VkPipelineStageFlags _dst_stages,
                             Span<VkMemoryBarrier> _memory_barriers, Span<VkBufferMemoryBarrier> _buffer_barriers,
                             Span<VkImageMemoryBarrier> _image_barriers, VkDependencyFlags _flags) {
            vkCmdPipelineBarrier(cmd, _src_stages, _dst_stages, _flags, _memory_barriers.size(), _memory_barriers.data(),
                                 _buffer_barriers.size(), _buffer_barriers.data(), _image_barriers.size(), _image_barriers.data());
        }

        UploadBatch::UploadBatch(VulkanContext &_cxt, QueueId _queue_id, CommandBufferId _cmd_buffer_id, BufferId _staging_buffer_id)
            : cxt(_cxt), queue_id{_queue_id}, cmd_buffer_id{_cmd_buffer_id}, staging_buffer_id{_staging_buffer_id} {
            staging_ptr = (uint8_t*)cxt.cxt_memory.getMappedBuffer(staging_buffer_id);
            SVKFW_ASSERT(staging_ptr != nullptr, std::invalid_argument, "VKFW::UploadBatch", "staging buffer memory is not HOST_VISIBLE or not allocated");
            if ((cxt.cxt_memory.buffer(staging_buffer_id).create_info.peek().usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0u)
                fprintf(svkfwwarn, "%s (%u, %u)\n", SVKFW_WRAPWARN("VKFW::UploadBatch", "staging buffer is not set as transfer source:"), staging_buffer_id.parent.index, staging_buffer_id.index);

            staging_size = cxt.cxt_memory.bufferMapping(staging_buffer_id).range;
            fence_id = cxt.cxt_barrier.addFence(false);
        }

        UploadBatch::~UploadBatch() {
             // Note: the fence, the semaphore and the staging buffer must not be in use when the batch is gone
             if (cxt.cxt_barrier.fences.contains(fence_id)) {
                 try { wait(); } catch (...) {}
                 cxt.cxt_barrier.destroyFence(fence_id);
             }
             if (cxt.cxt_barrier.semaphores.contains(owner_semaphore_id))
                 cxt.cxt_barrier.destroySemaphore(owner_semaphore_id);
             if (cxt.cxt_barrier.semaphores.contains(return_semaphore_id))
                 cxt.cxt_barrier.destroySemaphore(return_semaphore_id);
             if (cxt.cxt_command.cmd_buffers.contains(owner_cmd_buffers_id))
                 cxt.cxt_command.removeCmdBuffers(owner_cmd_buffers_id);
         }

        void UploadBatch::setOwner(QueueId _owner_queue_id, CommandPoolId _owner_cmd_pool_id) {
            SVKFW_ASSERT(!is_submitted && regions.empty() && image_regions.empty() && released_buffers.empty() && owner_queue_id.isNone(),
                            std::logic_error, "VKFW::UploadBatch :: setOwner", "the owner is set once, before the copies");
            owner_queue_id = _owner_queue_id;
            if (!transfersOwnership_()) return;

            owner_cmd_buffers_id = cxt.cxt_command.addCmdBuffer();
            cxt.cxt_command.createCmdBuffersPrimary(owner_cmd_buffers_id, _owner_cmd_pool_id, 2u);
            return_semaphore_id = cxt.cxt_barrier.addSemaphore();
            owner_semaphore_id  = cxt.cxt_barrier.addSemaphore();
        }

        void UploadBatch::copyToBuffer(BufferId _dst_buffer_id, const void *_data, VkDeviceSize _size, VkDeviceSize _dst_offset) {
            if ((cxt.cxt_memory.buffer(_dst_buffer_id).create_info.peek().usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0u)
                fprintf(svkfwwarn, "%s (%u, %u)\n", SVKFW_WRAPWARN("VKFW::UploadBatch :: copyToBuffer", "buffer is not set as transfer destination:"), _dst_buffer_id.parent.index, _dst_buffer_id.index);
            SVKFW_ASSERT(_dst_offset + _size <= cxt.cxt_memory.bufferMapping(_dst_buffer_id).range, std::out_of_range, "VKFW::UploadBatch :: copyToBuffer",
                            "range is out of the buffer: " + std::to_string(_dst_offset + _size) + " > " + std::to_string(cxt.cxt_memory.bufferMapping(_dst_buffer_id).range));

            wait(); // the staging buffer may still be read by the previous submit
            const uint8_t *__src = (const uint8_t*)_data;
            while (_size > 0u) {
                // Note: the buffer is still being written, so it is released in a later submit
                if (staging_used == staging_size) { submit_(_dst_buffer_id); wait(); }

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

        void UploadBatch::copyToImage(ImageId _dst_image_id, const void *_data, VkDeviceSize _size,
                         VkImageLayout _final_layout) {
            const VkImageCreateInfo &__image = cxt.cxt_memory.image(_dst_image_id).create_info.peek();
            if ((__image.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0u)
                fprintf(svkfwwarn, "%s (%u, %u)\n", SVKFW_WRAPWARN("VKFW::UploadBatch :: copyToImage", "image is not set as transfer destination:"), _dst_image_id.parent.index, _dst_image_id.index);
            SVKFW_ASSERT(_size <= staging_size, std::length_error, "VKFW::UploadBatch :: copyToImage",
                            "image data doesn't fit the staging buffer: " + std::to_string(_size) + " > " + std::to_string(staging_size));
            for (uint32_t i = 0u; i < image_regions.size(); ++i)
                SVKFW_ASSERT(image_regions[i].dst_image_id.parent.index != _dst_image_id.parent.index || image_regions[i].dst_image_id.index != _dst_image_id.index,
                                std::invalid_argument, "VKFW::UploadBatch :: copyToImage", "the image is already copied in this batch");

            wait(); // the staging buffer may still be read by the previous submit
            // Note: 'bufferOffset' must be a multiple of the texel block size (1-16 bytes, 3/6/12 for RGB formats) and of 4: 48 fits all
            const VkDeviceSize __alignment = 48u;
            VkDeviceSize __offset = staging_used + (__alignment - staging_used % __alignment) % __alignment;
            if (__offset + _size > staging_size) { submit_(BufferId()); wait(); __offset = 0u; }
            memcpy(staging_ptr + __offset, _data, _size);
            staging_used = __offset + _size;

            VkImageAspectFlags __aspect = Detail::formatAspects(__image.format);
            if (__aspect & VK_IMAGE_ASPECT_DEPTH_BIT) __aspect = VK_IMAGE_ASPECT_DEPTH_BIT;

            ImageRegion __region;
            __region.dst_image_id = _dst_image_id;
            __region.final_layout = _final_layout;
            __region.aspect       = __aspect;
            __region.copy = VkBufferImageCopy{};
            __region.copy.bufferOffset     = __offset;
            __region.copy.imageSubresource = { __aspect, 0u, 0u, __image.arrayLayers };
            __region.copy.imageExtent      = __image.extent;
            image_regions.push_back(__region);
        }

        FenceId UploadBatch::submit() { return submit_(BufferId()); }

        void UploadBatch::wait() {
            if (!is_submitted) return;
            cxt.cxt_barrier.wait(fence_id);
            cxt.cxt_barrier.reset(fence_id);
            is_submitted = false;
            staging_used = 0u;
        }

        bool UploadBatch::isSubmitted() const { return is_submitted; }

        bool UploadBatch::transfersOwnership_() const {
            return !owner_queue_id.isNone() && cxt.get(owner_queue_id.parent) != cxt.get(queue_id.parent);
        }

        VkBufferMemoryBarrier2 UploadBatch::bufferBarrier_(VkBuffer _buffer, uint32_t _src_family, uint32_t _dst_family,
                                                     VkPipelineStageFlags2 _src_stages, VkAccessFlags2 _src_access,
                                                     VkPipelineStageFlags2 _dst_stages, VkAccessFlags2 _dst_access) {
            VkBufferMemoryBarrier2 __barrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2};
            __barrier.srcStageMask        = _src_stages;
            __barrier.srcAccessMask       = _src_access;
            __barrier.dstStageMask        = _dst_stages;
            __barrier.dstAccessMask       = _dst_access;
            __barrier.srcQueueFamilyIndex = _src_family;
            __barrier.dstQueueFamilyIndex = _dst_family;
            __barrier.buffer              = _buffer;
            __barrier.offset              = 0u;
            __barrier.size                = VK_WHOLE_SIZE;
            return __barrier;
        }

        FenceId UploadBatch::submit_(BufferId _keep_buffer_id) {
            if (regions.empty() && image_regions.empty()) return FenceId();
            cxt.cxt_memory.flushBuffer(staging_buffer_id, 0u, staging_used);

            const bool     __transfer   = transfersOwnership_();
            const uint32_t __src_family = cxt.get(queue_id.parent);
            const uint32_t __dst_family = __transfer ? cxt.get(owner_queue_id.parent) : uint32_t(VK_QUEUE_FAMILY_IGNORED);
            const VkAccessFlags2 __any_access = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;

            // Buffers owned by another family: given back by the owner before the copies (step 1)
            std::vector<VkBufferMemoryBarrier2> __return_release, __return_acquire;
            for (uint32_t i = 0u; i < regions.size(); ++i) {
                const BufferId __id = regions[i].dst_buffer_id;
                VulkanContext::ContextMemory::MemoryMapping &__mapping = cxt.cxt_memory.bufferMapping(__id);
                if (cxt.cxt_memory.buffer(__id).create_info.peek().sharingMode == VK_SHARING_MODE_CONCURRENT) continue;
                if (__mapping.owner_family == VK_QUEUE_FAMILY_IGNORED || __mapping.owner_family == __src_family) continue;
                SVKFW_ASSERT(__transfer && __mapping.owner_family == __dst_family, std::logic_error, "VKFW::UploadBatch :: submit",
                                "the buffer (" + std::to_string(__id.parent.index) + ", " + std::to_string(__id.index) + ") is owned by another queue family, "
                                "set it as the owner of the batch ('setOwner')");
                const VkBuffer __buffer = cxt.get(__id);
                // Note: work submitted earlier on the owner queue may use the buffer in any stage
                __return_release.push_back(bufferBarrier_(__buffer, __dst_family, __src_family, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, __any_access,
                                                          VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE));
                __return_acquire.push_back(bufferBarrier_(__buffer, __dst_family, __src_family, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
                                                          VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT));
                __mapping.owner_family = __src_family;
            }

            // Buffers to hand over to the owner (EXCLUSIVE ones, once each); '_keep_buffer_id' stays for the next submit
            std::vector<BufferId> __release_buffers;
            if (__transfer) {
                for (uint32_t i = 0u; i < regions.size(); ++i) {
                    const BufferId __id = regions[i].dst_buffer_id;
                    if (cxt.cxt_memory.buffer(__id).create_info.peek().sharingMode == VK_SHARING_MODE_CONCURRENT) continue;
                    if (!containsBuffer_(released_buffers, __id)) released_buffers.push_back(__id);
                }
                for (uint32_t i = 0u; i < released_buffers.size(); ++i)
                    if (!sameBuffer_(released_buffers[i], _keep_buffer_id)) __release_buffers.push_back(released_buffers[i]);
                released_buffers.clear();
                if (!_keep_buffer_id.isNone() && cxt.cxt_memory.buffer(_keep_buffer_id).create_info.peek().sharingMode != VK_SHARING_MODE_CONCURRENT)
                    released_buffers.push_back(_keep_buffer_id);
            }

            // Step 1: the owner gives the buffers back
            const bool __return = !__return_release.empty();
            if (__return) {
                CommandRecorder __return_rec(cxt, CommandBufferId(owner_cmd_buffers_id, 0u));
                __return_rec.reset();
                __return_rec.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
                __return_rec.barrier({}, __return_release);
                __return_rec.end();

                SubmitDesc __return_submit;
                __return_submit.commands(CommandBufferId(owner_cmd_buffers_id, 0u)).signal(return_semaphore_id, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
                cxt.submit(owner_queue_id, __return_submit);
            }

            // Step 2: copies on the queue of the batch
            CommandRecorder __rec(cxt, cmd_buffer_id);
            __rec.reset();
            __rec.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            if (__return) __rec.barrier({}, __return_acquire);
            for (uint32_t i = 0u; i < regions.size(); ++i)
                __rec.copyBuffer(staging_buffer_id, regions[i].dst_buffer_id, regions[i].copy);

            // Released by the queue of the batch and acquired by the owner queue: same barriers, the stages and accesses
            // of the other side are ignored (NONE)
            std::vector<VkImageMemoryBarrier2>  __image_release,  __image_acquire;
            std::vector<VkBufferMemoryBarrier2> __buffer_release, __buffer_acquire;

            if (!image_regions.empty()) {
                std::vector<VkImageMemoryBarrier2> __barriers(image_regions.size());
                for (uint32_t i = 0u; i < image_regions.size(); ++i)
                    __barriers[i] = imageBarrier(cxt.get(image_regions[i].dst_image_id), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                 VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                                 image_regions[i].aspect);
                __rec.barrier(__barriers);
                for (uint32_t i = 0u; i < image_regions.size(); ++i)
                    __rec.copyBufferToImage(staging_buffer_id, image_regions[i].dst_image_id, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image_regions[i].copy);

                __barriers.clear();
                for (uint32_t i = 0u; i < image_regions.size(); ++i) {
                    const ImageRegion &__region = image_regions[i];
                    const VkImage      __image  = cxt.get(__region.dst_image_id);
                    const bool __release = __transfer && cxt.cxt_memory.image(__region.dst_image_id).create_info.peek().sharingMode != VK_SHARING_MODE_CONCURRENT;
                    if (!__release) {
                        // Note: later users wait for the fence (or a semaphore), so the destination is any stage
                        __barriers.push_back(imageBarrier(__image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, __region.final_layout,
                                                          VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                                          VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_NONE, __region.aspect));
                        continue;
                    }
                    // Note: the layout transition is done once, between the release and the acquire
                    VkImageMemoryBarrier2 __barrier = imageBarrier(__image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, __region.final_layout,
                                                                   VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                                                   VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, __region.aspect);
                    __barrier.srcQueueFamilyIndex = __src_family;
                    __barrier.dstQueueFamilyIndex = __dst_family;
                    __image_release.push_back(__barrier);
                    __barrier.srcStageMask  = VK_PIPELINE_STAGE_2_NONE;
                    __barrier.srcAccessMask = VK_ACCESS_2_NONE;
                    __barrier.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                    __barrier.dstAccessMask = __any_access;
                    __image_acquire.push_back(__barrier);
                    cxt.cxt_memory.imageMapping(__region.dst_image_id).owner_family = __dst_family;
                }
                if (!__barriers.empty()) __rec.barrier(__barriers);
            }

            for (uint32_t i = 0u; i < __release_buffers.size(); ++i) {
                const VkBuffer __buffer = cxt.get(__release_buffers[i]);
                __buffer_release.push_back(bufferBarrier_(__buffer, __src_family, __dst_family, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                                          VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE));
                __buffer_acquire.push_back(bufferBarrier_(__buffer, __src_family, __dst_family, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
                                                          VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, __any_access));
                cxt.cxt_memory.bufferMapping(__release_buffers[i]).owner_family = __dst_family;
            }
            const bool __acquire = !__image_release.empty() || !__buffer_release.empty();
            if (__acquire) __rec.barrier(__image_release, __buffer_release);
            __rec.end();

            SubmitDesc __submit;
            __submit.commands(cmd_buffer_id);
            if (__return) __submit.wait(return_semaphore_id, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
            if (!__acquire) {
                cxt.submit(queue_id, __submit, fence_id);
            }
            else {
                // Step 3: the owner acquires the resources
                __submit.signal(owner_semaphore_id, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
                cxt.submit(queue_id, __submit);

                CommandRecorder __owner_rec(cxt, CommandBufferId(owner_cmd_buffers_id, 1u));
                __owner_rec.reset();
                __owner_rec.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
                __owner_rec.barrier(__image_acquire, __buffer_acquire);
                __owner_rec.end();

                SubmitDesc __owner_submit;
                __owner_submit.wait(owner_semaphore_id, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT).commands(CommandBufferId(owner_cmd_buffers_id, 1u));
                cxt.submit(owner_queue_id, __owner_submit, fence_id);
            }

            regions.clear();
            image_regions.clear();
            is_submitted = true;
            return fence_id;
        }

        bool UploadBatch::sameBuffer_(BufferId _a, BufferId _b) {
            return _a.parent.index == _b.parent.index && _a.parent.generation == _b.parent.generation && _a.index == _b.index;
        }

        bool UploadBatch::containsBuffer_(const std::vector<BufferId> &_buffers, BufferId _id) {
            for (uint32_t i = 0u; i < _buffers.size(); ++i) if (sameBuffer_(_buffers[i], _id)) return true;
            return false;
        }
    }; // VKFW END
}; // Simple END
