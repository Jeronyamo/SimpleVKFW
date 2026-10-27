#ifndef SVKFW_VKFW_H
#define SVKFW_VKFW_H

#include <algorithm>

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

        // Note: which way of rendering and synchronization the contexts use; set it before 'ContextCore::createDevice'.
        //       The default needs Vulkan 1.3. Turning both options off needs only Vulkan 1.1 (e.g. for older mobile devices):
        //       rendering goes through VkRenderPass/VkFramebuffer, barriers and submits use the original commands.
        struct DeviceProfile {
            bool dynamic_rendering = true; // false: VkRenderPass + VkFramebuffer ('ContextCore::render_pass', swapchain framebuffers)
            bool synchronization2  = true; // false: vkCmdPipelineBarrier / vkQueueSubmit (64-bit stage/access flags are not available)
            // Depth/stencil test enables, compare ops and stencil ops as dynamic state (core in Vulkan 1.3; see 'CommandRecorder::setStencilOp')
            bool extended_dynamic_state = true;

            uint32_t minApiVersion() const;
        }; // DeviceProfile END

        namespace Detail {
            // Note: synchronization2 flags without the 64-bit ones are the original flags; 'NONE' becomes '_none_stage'
            VkPipelineStageFlags toStageFlags1(VkPipelineStageFlags2 _stages, VkPipelineStageFlags _none_stage);
            VkAccessFlags toAccessFlags1(VkAccessFlags2 _access);

            bool hasDepth(VkFormat _format);
            bool hasStencil(VkFormat _format);
            // All aspects of the format (COLOR for color formats)
            VkImageAspectFlags formatAspects(VkFormat _format);
            // Bytes per texel of one aspect when it is copied to a buffer (depth: 2 for D16, 4 for the others; stencil: 1);
            // for color formats only the common uncompressed ones are known (throws for the others)
            uint32_t texelSize(VkFormat _format, VkImageAspectFlagBits _aspect = VK_IMAGE_ASPECT_COLOR_BIT);
        }; // Detail END

        // Barrier for one image (all mips and layers of '_aspect'); used with 'CommandRecorder::barrier'
        VkImageMemoryBarrier2 imageBarrier(VkImage _image, VkImageLayout _old_layout, VkImageLayout _new_layout,
                                                  VkPipelineStageFlags2 _src_stages, VkAccessFlags2 _src_access,
                                                  VkPipelineStageFlags2 _dst_stages, VkAccessFlags2 _dst_access,
                                                  VkImageAspectFlags _aspect = VK_IMAGE_ASPECT_COLOR_BIT);

        // Attachment for 'CommandRecorder::beginRendering'
        VkRenderingAttachmentInfo renderingAttachment(VkImageView _view, VkImageLayout _layout,
                                                             VkAttachmentLoadOp _load_op, VkAttachmentStoreOp _store_op,
                                                             const VkClearValue &_clear_value = VkClearValue());

        #define VKFW_MAX_SUBMIT_SEMAPHORES      8u
        #define VKFW_MAX_SUBMIT_COMMAND_BUFFERS 8u

        // Note: one VkSubmitInfo(2) described by Ids; stored inline, so it can be filled every frame without allocations.
        //       Stages are synchronization2 flags; without synchronization2 they must fit the original 32-bit flags
        struct SubmitDesc {
            SemaphoreId          wait_semaphores  [VKFW_MAX_SUBMIT_SEMAPHORES];
            VkPipelineStageFlags2 wait_stages     [VKFW_MAX_SUBMIT_SEMAPHORES];
            CommandBufferId      command_buffers  [VKFW_MAX_SUBMIT_COMMAND_BUFFERS];
            SemaphoreId          signal_semaphores[VKFW_MAX_SUBMIT_SEMAPHORES];
            VkPipelineStageFlags2 signal_stages   [VKFW_MAX_SUBMIT_SEMAPHORES]; // Note: used only with synchronization2
            uint32_t wait_count = 0u, command_count = 0u, signal_count = 0u;

            SubmitDesc& wait(SemaphoreId _semaphore_id, VkPipelineStageFlags2 _stage);
            SubmitDesc& commands(CommandBufferId _cmd_buffer_id);
            SubmitDesc& signal(SemaphoreId _semaphore_id, VkPipelineStageFlags2 _stage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
            void clear();
        }; // SubmitDesc END

        struct VulkanContext {
            struct ContextDebug {
                VulkanContext *cxt_ptr = nullptr;
                Object::DebugUtilsMessengerEXT debug_messenger;

                ContextDebug(VulkanContext* _cxt_ptr);
               ~ContextDebug();

               ContextDebug& operator=(VkResult _vk_func_result);
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
                Object::RenderPass render_pass; // Note: used only without 'profile.dynamic_rendering'

                // Note: device features are set in 'device.create_info.features' (required) and 'device.create_info.optional_features'
                //       (enabled if supported); the features and properties of the selected device: 'phys_device.features()/properties()'
                DeviceProfile profile;

                Struct::StructWrap<VkAllocationCallbacks> alloc_cb;
                ContextDebug cxt_debug;

                ContextCore(VulkanContext* _cxt_ptr);
               ~ContextCore();

                void destroyContext();

            // Manage Objects
                // Automatically adds a queue vector handler of the added queue family
                QueueFamilyId addQueueFamily();

            // Access by Id (throws if the Id is none, stale or out of its family)

                      Object::QueueFamily &queueFamily(QueueFamilyId _id);
                const Object::QueueFamily &queueFamily(QueueFamilyId _id) const;
                      Object::Queue       &queues     (QueueFamilyId _id);
                VkQueue queue(QueueId _id) const;

            // Physical device queries for structures that 'phys_device.features()/properties()' don't hold, e.g. of extensions
            // (pass a StructChain headed by VkPhysicalDeviceFeatures2 / VkPhysicalDeviceProperties2)

                template <typename... Ts>
                void queryFeatures(StructChain<VkPhysicalDeviceFeatures2, Ts...> &_chain) const {
                    vkGetPhysicalDeviceFeatures2(phys_device.handle, &_chain.head());
                }
                template <typename... Ts>
                void queryProperties(StructChain<VkPhysicalDeviceProperties2, Ts...> &_chain) const {
                    vkGetPhysicalDeviceProperties2(phys_device.handle, &_chain.head());
                }

            // Create Objects

                void createInstance();
                void createDebugUtilsMessenger();
                void loadPhysicalDeviceInfo();
                void findPhysicalDevice();
                // TODO: USE ONE QUEUE_FAMILY WRAP! DATA IS DUPLICATED
                void loadQueueFamiliesInfo();
                void findQueueFamilies();
                // Note: the API version used with the device (instance and device versions, whichever is lower); valid after 'findPhysicalDevice'
                uint32_t apiVersion() const;
                // Throws if the selected physical device can't be used with 'profile'
                void checkProfileSupport() const;
                // Enables the features of 'profile' and the supported optional features, chains only the structures of the device
                // version; throws with the names of the required features the device doesn't support
                void createDevice();
                void acquireQueues();
                void createRenderPass();

                void destroyInstance();
                void destroyDebugUtilsMessenger();
                void destroyPhysicalDevice();
                void destroyDevice();
                void destroyRenderPass();

            // Scenarios

                ContextIndex addSctRenderPassAttachment        ();
                ContextIndex addSctRenderPassSubpassDescription();
                ContextIndex addSctRenderPassSubpassDependency ();

                void setDefaultApplicationInfo();
                void activateDebug();
                void setQueueFamilyQueueCount(QueueFamilyId _queue_family_id, uint32_t _queue_count, const std::vector<float> &_priorities = {1.0f});
                void setDeviceQueueInfo();
            } cxt_core; // ContextCore END

            struct ContextSwapchain {
                Object::SwapchainKHR swapchain;
                Object::  SurfaceKHR   surface;
                std::vector<VkImage>       images;
                std::vector<VkImageView> image_views;
                std::vector<VkFramebuffer> framebuffers; // Note: created only without dynamic rendering, if 'ContextCore::render_pass' is created
                GLFWwindow *window = nullptr;

                Struct::StructWrap<  VkImageViewCreateInfo>  image_view_wrap;
                Struct::StructWrap<VkFramebufferCreateInfo> framebuffer_wrap;

                ContextCore *cxt_core = nullptr;

                ContextSwapchain(ContextCore *_cxt_core);
               ~ContextSwapchain();

                void destroyContext();

            // Manage Objects

                void createSurface();
                void createSwapchain();
                void createImageViews();
                void createFramebuffers();

                void destroySurface();
                void destroySwapchain();
                void destroyImageViews();
                void destroyFramebuffers();

            // Scenarios

                void setWindow(const Window &_window);
                void  createSwapchainWithDependencies();
                void destroySwapchainWithDependencies();
                uint32_t getSwapchainImageCount() const;
                // Note: swapchain images are indexed by the image index from 'acquire'
                VkFramebuffer framebuffer(uint32_t _image_index) const;
                VkImage image(uint32_t _image_index) const;
                VkImageView imageView(uint32_t _image_index) const;
                VkExtent2D extent() const;
                VkFormat format() const;

            // Functions

                AcquireResult acquire(VkSemaphore _semaphore, VkFence _fence = VK_NULL_HANDLE, uint64_t _timeout = UINT64_MAX);
                SwapchainStatus present(VkQueue _queue, Span<VkSemaphore> _wait_semaphores, uint32_t _image_index);

                void pickSwapchainImageFormat(const Util::SurfaceFormatTestFunc _test_func = Util::SelectSurfaceFormat::DefaultFormatTest);
                void setSwapchainExtentFromWindow();
                void setSwapchainSurfaceCapabilities();
                void setRenderPassAttachmentImageFormat(ContextIndex _attachment_ci);
                // Note: the caller must make sure the swapchain images are not in use (see 'VulkanContext::recreateSwapchain')
                //       Blocks while the window is minimized (zero extent)
                void recreate();
            } cxt_swapchain; // ContextSwapchain END

            struct ContextDescriptor {
                Pool<Object::DescriptorSetLayout, DescriptorSetLayoutTag> layouts;
                Pool<Object::DescriptorPool,      DescriptorPoolTag>        pools;
                Pool<Object::DescriptorSet,       DescriptorSetsTag>         sets;
                // Note: '1' Pool AND 'N' Layouts --> 'N' Sets
                // Note: samplers live here because they are used only through descriptors (or as immutable samplers of a layout)
                Pool<Object::Sampler,             SamplerTag>            samplers;

                ContextCore *cxt_core = nullptr;

                ContextDescriptor(ContextCore *_cxt_core);
               ~ContextDescriptor();

                // Note: Only Core and Swapchain contexts' logic allows creation of all objects at once
                void  createContext();
                void destroyContext();

            // Access by Id (throws if the Id is none, stale or out of its group)

                VkDescriptorSet set(DescriptorSetId _id) const;

                // Note: references are invalidated when an object of the same kind is added
                Object::DescriptorSetLayout &layout  (DescriptorSetLayoutId _id);
                Object::DescriptorPool      &pool    (DescriptorPoolId      _id);
                Object::DescriptorSet       &setGroup(DescriptorSetsId      _id);
                Struct::StructWrap<VkDescriptorSetLayoutBinding> &layoutBinding(DescriptorLayoutBindingId _id);
                Object::Sampler             &sampler (SamplerId             _id);

            // Functions

                void updateSets(Span<VkWriteDescriptorSet> _writes, Span<VkCopyDescriptorSet> _copies = {});
                void updateSets(std::vector<Struct::StructWrap<VkWriteDescriptorSet>> &_writes);

            // Manage Objects

                DescriptorSetLayoutId addLayout();
                DescriptorPoolId      addPool  ();
                DescriptorSetsId      addSet   ();
                SamplerId             addSampler();

                void createLayout(DescriptorSetLayoutId _layout_id);
                void createPool(DescriptorPoolId _pool_id);
                void createSets(DescriptorSetsId _sets_id);
                // Note: anisotropy is turned off if the 'samplerAnisotropy' feature isn't enabled, and clamped to 'maxSamplerAnisotropy'
                void createSampler(SamplerId _sampler_id);

                // Note: 'destroy*' keeps the Id (the object can be created again), 'remove*' also frees the Id
                void destroyLayout(DescriptorSetLayoutId _layout_id);
                void destroyPool(DescriptorPoolId _pool_id);
                void destroySets(DescriptorSetsId _sets_id);
                void destroySampler(SamplerId _sampler_id);

                void removeLayout(DescriptorSetLayoutId _layout_id);
                void removePool  (DescriptorPoolId      _pool_id  );
                void removeSets  (DescriptorSetsId      _sets_id  );
                void removeSampler(SamplerId            _sampler_id);

            // Scenarios

                DescriptorLayoutBindingId addSctLayoutBinding(DescriptorSetLayoutId _layout_id);
                void setAllForSet(DescriptorSetsId _sets_id, DescriptorPoolId _pool_id, Span<DescriptorSetLayoutId> _layouts_id);
                void setPoolForSet(DescriptorSetsId _sets_id, DescriptorPoolId _pool_id);
                void setLayoutsForSet(DescriptorSetsId _sets_id, Span<DescriptorSetLayoutId> _layouts_id);
            } cxt_descriptor; // ContextDescriptors END

            struct ContextPipeline {
                Pool<Object::Pipeline,       PipelineTag>       pipelines;
                Pool<Object::PipelineLayout, PipelineLayoutTag>   layouts;
                Pool<Object::PipelineCache,  PipelineCacheTag>     caches;
                Pool<Object::ShaderModule,   ShaderModuleTag>     shaders;

                ContextCore *cxt_core = nullptr;

                ContextPipeline(ContextCore *_cxt_core);
               ~ContextPipeline();

                // Note: Only Core and Swapchain contexts' logic allows creation of all objects at once
                void  createContext();
                void destroyContext();

            // Access by Id (throws if the Id is none or stale)
            // Note: references are invalidated when an object of the same kind is added

                Object::Pipeline       &pipeline(PipelineId       _id);
                Object::PipelineLayout &layout  (PipelineLayoutId _id);
                Object::PipelineCache  &cache   (PipelineCacheId  _id);
                Object::ShaderModule   &shader  (ShaderModuleId   _id);
                Struct::StructWrap<VkPipelineShaderStageCreateInfo> &shaderStage(ShaderStageId _id);

            // Manage Objects

                PipelineId       addPipeline();
                ShaderModuleId   addShader  ();
                PipelineLayoutId addLayout  ();
                PipelineCacheId  addCache   ();

                void createPipeline(PipelineId _pipeline_id, uint32_t _subpass, PipelineLayoutId _layout_id, PipelineCacheId _cache_id = PipelineCacheId(),
                                    VkDeferredOperationKHR _deferred_op = VK_NULL_HANDLE);
                // Note: '_render_pass' is used only without dynamic rendering (e.g. 'Graph::RenderGraph::renderPass')
                void createPipeline(PipelineId _pipeline_id, VkRenderPass _render_pass, uint32_t _subpass, PipelineLayoutId _layout_id,
                                    PipelineCacheId _cache_id = PipelineCacheId(), VkDeferredOperationKHR _deferred_op = VK_NULL_HANDLE);
                void createCache (PipelineCacheId _cache_id);
                void createLayout(PipelineLayoutId _layout_id);
                void createShader(ShaderModuleId _shader_id);

                // Note: 'destroy*' keeps the Id (the object can be created again), 'remove*' also frees the Id
                void destroyPipeline(PipelineId _pipeline_id);
                void destroyCache   (PipelineCacheId _cache_id);
                void destroyLayout  (PipelineLayoutId _layout_id);
                void destroyShader  (ShaderModuleId _shader_id);

                void removePipeline(PipelineId       _pipeline_id);
                void removeCache   (PipelineCacheId  _cache_id   );
                void removeLayout  (PipelineLayoutId _layout_id  );
                void removeShader  (ShaderModuleId   _shader_id  );

            // Scenarios

                void setLayoutDescriptorSetLayouts(PipelineLayoutId _layout_id, Span<DescriptorSetLayoutId> _descr_set_layouts_id);
                void addLayoutPushConstant(PipelineLayoutId _layout_id, VkShaderStageFlags _stage_flags, uint32_t _offset, uint32_t _size);
                ShaderStageId addShaderStage(PipelineId _pipeline_id);
            } cxt_pipeline; // ContextPipeline END

            struct ContextCommand {
                Pool<Object::CommandPool,   CommandPoolTag>      cmd_pools;
                Pool<Object::CommandBuffer, CommandBuffersTag> cmd_buffers;

                ContextCore *cxt_core = nullptr;

                ContextCommand(ContextCore *_cxt_core);
               ~ContextCommand();

                // Note: Only Core and Swapchain contexts' logic allows creation of all objects at once
                void  createContext();
                void destroyContext();

            // Access by Id (throws if the Id is none, stale or out of its group)

                VkCommandBuffer commandBuffer(CommandBufferId _id) const;

                Object::CommandPool   &cmdPool       (CommandPoolId    _id);
                Object::CommandBuffer &cmdBufferGroup(CommandBuffersId _id);

            // Manage Objects

                CommandPoolId    addCmdPool  ();
                CommandBuffersId addCmdBuffer();

                void createCmdPool(CommandPoolId _cmd_pool_id);
                void createCmdBuffers(CommandBuffersId _cmd_buffers_id);

                // Note: removing a pool frees its command buffers on the Vulkan side, remove their groups first
                void removeCmdBuffers(CommandBuffersId _cmd_buffers_id);
                void removeCmdPool(CommandPoolId _cmd_pool_id);

            // Scenarios

                void createCmdPool(CommandPoolId _cmd_pool_id, QueueFamilyId _queue_family_id, VkCommandPoolCreateFlags _flags = 0u);
                void createCmdBuffersPrimary(CommandBuffersId _cmd_buffers_id, CommandPoolId _cmd_pool_id, uint32_t _cmd_buffers_count);
                void createCmdBuffersSecondary(CommandBuffersId _cmd_buffers_id, CommandPoolId _cmd_pool_id, uint32_t _cmd_buffers_count);
            } cxt_command; // ContextCommand END

            struct ContextMemory {
                struct MemoryMapping {
                    VkDeviceSize offset; // offset in the allocated device memory
                    VkDeviceSize  range; // required device memory size in bytes
                    // Note: the queue family that owns an EXCLUSIVE resource after the last ownership transfer by an 'UploadBatch'
                    //       (VK_QUEUE_FAMILY_IGNORED: not transferred yet); set it by hand if the resource changes owner elsewhere
                    uint32_t owner_family;
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
                    uint32_t memory_type = UINT32_MAX; // set by 'ContextMemory::allocateChunkMemory'
                    uint32_t heap_index  = UINT32_MAX;

                    ContextCore *cxt_core = nullptr;

                    MemoryChunk();
                    MemoryChunk(ContextCore *_cxt_core);

                    void destroyChunk();

                    // Computes the offsets of all buffers and images; returns the size and the memory types that fit all of them
                    // Note: a linear resource (buffer, linear image) and an optimal image must not share a 'bufferImageGranularity' page,
                    //       so a resource of the other kind starts on a new page
                    VkMemoryRequirements computeLayout(VkDeviceSize _granularity);

                    // Binds all buffers and images to the allocated memory and maps HOST_VISIBLE memory
                    void bindAndMap();

                // Manage Objects

                    // Note: return the object's index inside the chunk ('ContextMemory::addChunk*' wraps it into a typed Id)
                    uint32_t addBuffer(uint32_t _buf_size = 0u);
                    uint32_t addImage (uint32_t _img_size = 0u);
                    uint32_t addBufferView();
                    uint32_t addImageView ();

                    void createBuffer(uint32_t _buffer_index);
                    void createImage (uint32_t _image_index);
                    void createBufferView(uint32_t _buffer_view_index, uint32_t _buffer_index);
                    void createImageView (uint32_t  _image_view_index, uint32_t  _image_index);

                    void destroyBuffer(uint32_t _buffer_index);
                    void destroyImage (uint32_t _image_index);
                    void destroyBufferView(uint32_t _buffer_view_index);
                    void destroyImageView (uint32_t  _image_view_index);

                private:
                    static void place_(VkMemoryRequirements &_total, const VkMemoryRequirements &_req, bool _linear, int &_prev_linear,
                                       VkDeviceSize _granularity, MemoryMapping &_map, const std::string &_what);
                }; // MemoryChunk END
                Pool<MemoryChunk, MemoryTag> memory_chunks;

                // Note: memory heaps may be views of the same physical memory (integrated GPUs, resizable BAR, the host-visible part of VRAM),
                //       so heap sizes don't add up and the free space of a heap can't be derived from its size and our allocations.
                //       With VK_EXT_memory_budget the driver reports the budget and usage of each heap (all processes accounted);
                //       without it the heap size and our own allocations are only an estimate. Either way the budget is a hint:
                //       'allocateChunkMemory' prefers memory types whose heap fits the budget and falls back to the others if an allocation fails
                struct HeapBudget {
                    VkDeviceSize budget; // how much this process may use without problems (heap size without VK_EXT_memory_budget)
                    VkDeviceSize usage;  // how much this process uses (only our chunks without VK_EXT_memory_budget)
                };

                Struct::StructWrap<VkPhysicalDeviceMemoryProperties2> memory_properties; // Note: read-only, as reported by the device
                VkDeviceSize heap_usage[VK_MAX_MEMORY_HEAPS];  // bytes allocated by the chunks, per heap
                VkDeviceSize non_coherent_atom_size     = 1u;  // loaded in 'initContext'
                VkDeviceSize buffer_image_granularity   = 1u;  // loaded in 'initContext'
                bool         memory_budget_available = false;  // the device is created with VK_EXT_memory_budget

                ContextCore *cxt_core = nullptr;

                ContextMemory(ContextCore *_cxt_core);
               ~ContextMemory();

                // Note: after 'ContextCore::createDevice'
                void initContext();
                void destroyContext();

                HeapBudget heapBudget(uint32_t _heap_index) const;

            // Access by Id (throws if the Id is none, stale or out of its chunk)

                      MemoryChunk &chunk(MemoryId _id);
                const MemoryChunk &chunk(MemoryId _id) const;

                      Object::Buffer     &buffer    (BufferId     _id);
                const Object::Buffer     &buffer    (BufferId     _id) const;
                      Object::Image      &image     (ImageId      _id);
                const Object::Image      &image     (ImageId      _id) const;
                      Object::BufferView &bufferView(BufferViewId _id);
                const Object::BufferView &bufferView(BufferViewId _id) const;
                      Object::ImageView  &imageView (ImageViewId  _id);
                const Object::ImageView  &imageView (ImageViewId  _id) const;

                      MemoryMapping &bufferMapping(BufferId _id);
                const MemoryMapping &bufferMapping(BufferId _id) const;
                      MemoryMapping & imageMapping(ImageId  _id);
                const MemoryMapping & imageMapping(ImageId  _id) const;

            // Manage objects

                MemoryId addMemoryChunk(VkMemoryPropertyFlags _required_flags);
                // Destroys the chunk with all its buffers, images and views; their Ids become invalid
                void removeMemoryChunk(MemoryId _memory_id);
                // Allocates one memory object for all buffers and images of the chunk (they must be created) and binds them
                // Note: memory types are tried in the device order (it lists the preferred types first): first the types whose heap
                //       fits the budget, then the others; a type is skipped if its allocation fails with an out-of-memory error
                void allocateChunkMemory(MemoryId _memory_id);

                BufferId     addChunkBuffer    (MemoryId _memory_id, uint32_t _buf_size = 0u);
                ImageId      addChunkImage     (MemoryId _memory_id, uint32_t _img_size = 0u);
                BufferViewId addChunkBufferView(MemoryId _memory_id);
                ImageViewId  addChunkImageView (MemoryId _memory_id);

                void createChunkBuffer(BufferId _buffer_id);
                void createChunkImage (ImageId  _image_id );
                // Note: the view and the viewed resource may belong to different chunks
                void createChunkBufferView(BufferId _buffer_id, BufferViewId _buf_view_id);
                void createChunkImageView (ImageId  _image_id , ImageViewId  _img_view_id);

                // Returns nullptr if the chunk isn't HOST_VISIBLE or isn't allocated yet
                void* getMappedChunk (MemoryId _memory_id) const;
                void* getMappedBuffer(BufferId _buffer_id) const;
                void* getMappedImage (ImageId  _image_id ) const;
                bool isChunkCoherent(MemoryId _memory_id) const;

            // Functions (persistently mapped memory)
            // Note: offsets are relative to the buffer; flush/invalidate do nothing for HOST_COHERENT memory

                void flushBuffer(BufferId _buffer_id, VkDeviceSize _offset = 0u, VkDeviceSize _size = VK_WHOLE_SIZE) const;
                void invalidateBuffer(BufferId _buffer_id, VkDeviceSize _offset = 0u, VkDeviceSize _size = VK_WHOLE_SIZE) const;
                // Copies CPU data into a HOST_VISIBLE buffer and flushes it if needed (the GPU must not be using this range)
                void writeBuffer(BufferId _buffer_id, const void *_data, VkDeviceSize _size, VkDeviceSize _offset = 0u) const;
                // Invalidates the range if needed and copies it from a HOST_VISIBLE buffer into CPU memory
                void readBuffer(BufferId _buffer_id, void *_data, VkDeviceSize _size, VkDeviceSize _offset = 0u) const;

            // Scenarios

                void setWholeBufferCreateInfoStruct(BufferId _buffer_id, Span<QueueFamilyId> _qfamilies_id,
                                                    VkSharingMode _sharing_mode, VkBufferUsageFlags _usage_flags, VkDeviceSize _size_bytes = 0u);

                // Note: '_extent.depth' > 1 makes a 3D image. An image used on queues of several families (e.g. uploaded with an
                //       'UploadBatch' on the transfer queue and sampled on the graphics queue) needs all of them with VK_SHARING_MODE_CONCURRENT,
                //       otherwise its contents need a queue family ownership transfer; repeated families are merged, and with one family
                //       left the sharing mode becomes EXCLUSIVE
                void setWholeImageCreateInfoStruct(ImageId _image_id, Span<QueueFamilyId> _qfamilies_id, VkSharingMode _sharing_mode,
                                                   VkImageUsageFlags _usage_flags, VkFormat _format, VkExtent3D _extent,
                                                   uint32_t _mip_levels = 1u, uint32_t _array_layers = 1u, VkSampleCountFlagBits _samples = VK_SAMPLE_COUNT_1_BIT,
                                                   VkImageTiling _tiling = VK_IMAGE_TILING_OPTIMAL, VkImageCreateFlags _flags = 0u);

                // Fills the view create info from the image: its format, all mip levels and layers, identity swizzle.
                // '_aspect' == 0: all aspects of the format, but only DEPTH for a depth/stencil image that isn't an attachment
                // (a sampled view has one aspect); '_type' == VK_IMAGE_VIEW_TYPE_MAX_ENUM: 3D, CUBE(_ARRAY), 2D_ARRAY or 2D from the image
                void setWholeImageViewCreateInfoStruct(ImageViewId _img_view_id, ImageId _image_id, VkImageAspectFlags _aspect = 0u,
                                                       VkImageViewType _type = VK_IMAGE_VIEW_TYPE_MAX_ENUM);

            private:
                void releaseChunk_(MemoryChunk &_chunk);
                void* checkMappedRange_(BufferId _buffer_id, VkDeviceSize _offset, VkDeviceSize _size, const char *_where) const;
                // Note: VkMappedMemoryRange offset and size must be multiples of 'nonCoherentAtomSize' (or the size must reach the allocation end)
                VkMappedMemoryRange mappedRange_(BufferId _buffer_id, VkDeviceSize _offset, VkDeviceSize _size) const;

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

                ContextBarrier(ContextCore *_cxt_core);
               ~ContextBarrier();

                void createContext();
                void destroyContext();

            // Manage Objects

                SemaphoreId addSemaphore();
                FenceId addFence(bool _is_signaled);
                EventId addEvent(bool _is_signaled);

            // Functions

                // Returns VK_SUCCESS or VK_TIMEOUT, throws on errors
                VkResult wait(Span<FenceId> _fences_id, bool _wait_all = true, uint64_t _timeout = UINT64_MAX) const;
                void reset(Span<FenceId> _fences_id) const;
                // Note: destroying a none or already destroyed Id does nothing
                void destroySemaphore(SemaphoreId _semaphore_id);
                void destroyFence(FenceId _fence_id);
                void destroyEvent(EventId _event_id);
            } cxt_barrier; // ContextBarrier END


            VulkanContext();
           ~VulkanContext();

            void destroyContext();

        // Handles by Id
        // Note: a none Id gives VK_NULL_HANDLE (UINT32_MAX for a queue family), a stale Id throws

            VkfwQueueFamily       get(QueueFamilyId         _id) const;
            VkQueue               get(QueueId               _id) const;

            VkDescriptorSetLayout get(DescriptorSetLayoutId _id) const;
            VkDescriptorPool      get(DescriptorPoolId      _id) const;
            VkDescriptorSet       get(DescriptorSetId       _id) const;
            VkSampler             get(SamplerId             _id) const;

            VkPipeline            get(PipelineId            _id) const;
            VkPipelineLayout      get(PipelineLayoutId      _id) const;
            VkPipelineCache       get(PipelineCacheId       _id) const;
            VkShaderModule        get(ShaderModuleId        _id) const;

            VkCommandPool         get(CommandPoolId         _id) const;
            VkCommandBuffer       get(CommandBufferId       _id) const;

            VkDeviceMemory        get(MemoryId              _id) const;
            VkBuffer              get(BufferId              _id) const;
            VkImage               get(ImageId               _id) const;
            VkBufferView          get(BufferViewId          _id) const;
            VkImageView           get(ImageViewId           _id) const;

            VkSemaphore           get(SemaphoreId           _id) const;
            VkFence               get(FenceId               _id) const;
            VkEvent               get(EventId               _id) const;

            // Writes the handles of '_ids' into '_res' (must hold at least '_ids.size()' elements)
            template <typename IdT, typename HandleT>
            void fill(Span<IdT> _ids, HandleT *_res) const {
                for (uint32_t i = 0u; i < _ids.size(); ++i) _res[i] = get(_ids[i]);
            }

        // Functions

            // Note: uses vkQueueSubmit2 with 'profile.synchronization2', vkQueueSubmit otherwise
            void submit(QueueId _queue_id, Span<SubmitDesc> _submits, FenceId _fence_id = FenceId()) const;
            AcquireResult acquireNextImage(SemaphoreId _semaphore_id, FenceId _fence_id = FenceId(), uint64_t _timeout = UINT64_MAX);
            SwapchainStatus present(QueueId _queue_id, Span<SemaphoreId> _wait_semaphores_id, uint32_t _image_index);
            // Note: waits until the device is idle, so it is meant for rare events (resize, OUT_OF_DATE), not for every frame
            void recreateSwapchain();

        // Scenarios

            void initNoncoreContexts();
            void handleResult(VkResult _result);

        private:
            void submit2_(QueueId _queue_id, Span<SubmitDesc> _submits, FenceId _fence_id) const;
            void submit1_(QueueId _queue_id, Span<SubmitDesc> _submits, FenceId _fence_id) const;

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

            CommandRecorder(const VulkanContext &_cxt, CommandBufferId _cmd_buffer_id);
            CommandRecorder(const VulkanContext &_cxt, VkCommandBuffer _cmd_buffer   );

            VkCommandBuffer handle() const;

        // Command buffer

            void begin(VkCommandBufferUsageFlags _flags = 0u, const VkCommandBufferInheritanceInfo *_inheritance = nullptr);
            void end();
            // Note: the pool must be created with VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
            void reset(bool _release_resources = false);

        // Render pass

            // Note: uses the render pass of the core context, render area starts at (0, 0)
            void beginRenderPass(VkFramebuffer _framebuffer, VkExtent2D _extent, Span<VkClearValue> _clear_values = {},
                                 VkSubpassContents _contents = VK_SUBPASS_CONTENTS_INLINE);
            void beginRenderPass(const VkRenderPassBeginInfo &_info, VkSubpassContents _contents = VK_SUBPASS_CONTENTS_INLINE);
            void nextSubpass(VkSubpassContents _contents = VK_SUBPASS_CONTENTS_INLINE);
            void endRenderPass();

        // Dynamic rendering (needs 'profile.dynamic_rendering')

            void beginRendering(const VkRenderingInfo &_info);
            // Note: render area starts at (0, 0)
            void beginRendering(VkExtent2D _extent, Span<VkRenderingAttachmentInfo> _color_attachments,
                                const VkRenderingAttachmentInfo *_depth_attachment = nullptr, const VkRenderingAttachmentInfo *_stencil_attachment = nullptr,
                                uint32_t _layer_count = 1u, VkRenderingFlags _flags = 0u);
            void endRendering();

        // Swapchain image (works with both profiles)

            // Starts rendering into a swapchain image, clearing it with '_clear_value':
            //   - dynamic rendering: transitions the image to COLOR_ATTACHMENT_OPTIMAL and begins rendering;
            //   - render pass: begins the render pass of the core context with the image framebuffer
            //     (layouts and the clear are defined by the render pass)
            // Note: the wait on the 'image available' semaphore must include COLOR_ATTACHMENT_OUTPUT
            void beginSwapchainRendering(uint32_t _image_index, const VkClearValue &_clear_value);
            // Ends rendering into a swapchain image and makes it ready for presentation
            void endSwapchainRendering(uint32_t _image_index);

        // Bind

            void bindPipeline(PipelineId _pipeline_id);
            void bindDescriptorSets(PipelineLayoutId _layout_id, Span<DescriptorSetId> _sets_id, Span<uint32_t> _dynamic_offsets = {},
                                    VkPipelineBindPoint _bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS, uint32_t _first_set = 0u);
            // Note: missing offsets are set to 0
            void bindVertexBuffers(Span<BufferId> _buffers_id, Span<VkDeviceSize> _offsets = {}, uint32_t _first_binding = 0u);
            void bindIndexBuffer(BufferId _buffer_id, VkDeviceSize _offset = 0u, VkIndexType _index_type = VK_INDEX_TYPE_UINT32);
            void pushConstants(PipelineLayoutId _layout_id, VkShaderStageFlags _stages, uint32_t _offset, uint32_t _size, const void *_data);

        // Dynamic state

            void setViewports(Span<VkViewport> _viewports, uint32_t _first = 0u);
            void setScissors (Span<VkRect2D>   _scissors,  uint32_t _first = 0u);
            void setLineWidth(float _line_width);
            // Sets viewport 0 (depth 0..1) and scissor 0 to cover '_extent'
            void setViewportScissor(VkExtent2D _extent);

            // Stencil (the pipeline must have the matching VK_DYNAMIC_STATE_STENCIL_* state)
            void setStencilReference  (VkStencilFaceFlags _faces, uint32_t _reference);
            void setStencilCompareMask(VkStencilFaceFlags _faces, uint32_t _mask);
            void setStencilWriteMask  (VkStencilFaceFlags _faces, uint32_t _mask);
            // Note: these need 'profile.extended_dynamic_state' (VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE, VK_DYNAMIC_STATE_STENCIL_OP,
            //       VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE, VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE, VK_DYNAMIC_STATE_DEPTH_COMPARE_OP)
            void setStencilTestEnable(bool _enable);
            void setStencilOp(VkStencilFaceFlags _faces, VkCompareOp _compare_op, VkStencilOp _pass_op,
                              VkStencilOp _fail_op = VK_STENCIL_OP_KEEP, VkStencilOp _depth_fail_op = VK_STENCIL_OP_KEEP);
            // The whole state of the faces: ops, compare and write masks, reference (each part needs its dynamic state)
            void setStencil(VkStencilFaceFlags _faces, const VkStencilOpState &_state);
            void setDepthTestEnable (bool _enable);
            void setDepthWriteEnable(bool _enable);
            void setDepthCompareOp  (VkCompareOp _compare_op);

        // Draw

            void draw(uint32_t _vert_count, uint32_t _inst_count = 1u, uint32_t _first_vert = 0u, uint32_t _first_inst = 0u);
            // Note: each index is 'index + vert_offset'
            void drawIndexed(uint32_t _ind_count, uint32_t _inst_count = 1u, int32_t _vert_offset = 0, uint32_t _first_ind = 0u, uint32_t _first_inst = 0u);
            void dispatch(uint32_t _x, uint32_t _y = 1u, uint32_t _z = 1u);
            void clearAttachments(Span<VkClearAttachment> _attachments, Span<VkClearRect> _rects);

        // Transfer

            void copyBuffer(BufferId _src_id, BufferId _dst_id, Span<VkBufferCopy> _regions);
            void copyImage(ImageId _src_id, VkImageLayout _src_layout, ImageId _dst_id, VkImageLayout _dst_layout, Span<VkImageCopy> _regions);
            void copyBufferToImage(BufferId _src_id, ImageId _dst_id, VkImageLayout _dst_layout, Span<VkBufferImageCopy> _regions);
            void copyImageToBuffer(ImageId _src_id, VkImageLayout _src_layout, BufferId _dst_id, Span<VkBufferImageCopy> _regions);
            // Note: for images outside 'cxt_memory' (swapchain images, render graph textures)
            void copyImageToBuffer(VkImage _src,    VkImageLayout _src_layout, BufferId _dst_id, Span<VkBufferImageCopy> _regions);
            // Note: needs a graphics queue; the formats must support blits (and linear filtering for VK_FILTER_LINEAR)
            void blitImage(ImageId _src_id, VkImageLayout _src_layout, ImageId _dst_id, VkImageLayout _dst_layout, Span<VkImageBlit> _regions,
                           VkFilter _filter = VK_FILTER_LINEAR);

        // Synchronization

            // Note: vkCmdPipelineBarrier2 with 'profile.synchronization2'; otherwise the barriers are converted to vkCmdPipelineBarrier
            //       (one call per barrier, so the stages of each barrier are kept; 64-bit flags throw)
            void barrier(Span<VkImageMemoryBarrier2> _image_barriers, Span<VkBufferMemoryBarrier2> _buffer_barriers = {},
                         Span<VkMemoryBarrier2> _memory_barriers = {}, VkDependencyFlags _flags = 0u);
            void pipelineBarrier(VkPipelineStageFlags _src_stages, VkPipelineStageFlags _dst_stages,
                                 Span<VkMemoryBarrier> _memory_barriers, Span<VkBufferMemoryBarrier> _buffer_barriers = {},
                                 Span<VkImageMemoryBarrier> _image_barriers = {}, VkDependencyFlags _flags = 0u);
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
            UploadBatch(VulkanContext &_cxt, QueueId _queue_id, CommandBufferId _cmd_buffer_id, BufferId _staging_buffer_id);
           ~UploadBatch();

            // Hands the uploaded buffers and images over to the queue family of '_owner_queue_id' (queue family ownership transfer).
            // Needed for VK_SHARING_MODE_EXCLUSIVE resources used by another family than the queue of the batch (e.g. uploaded on the
            // transfer queue, used on the graphics queue); skipped for CONCURRENT resources and when the families are the same.
            // In each submit:
            //   1. owner queue: buffers the owner got from an earlier upload are released back to the batch queue (their contents are kept);
            //   2. batch queue: they are acquired, the copies are done, all copied resources are released to the owner;
            //   3. owner queue: the resources are acquired; the fence of the batch is signaled here.
            // Note: the batch allocates two command buffers from '_owner_cmd_pool_id' (of the owner family, created with
            //       VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT). Work on the owner queue is ordered around the transfer by the
            //       submission order: work submitted before 'submit' finishes before the copies, work submitted after it sees the new data.
            //       Images are not given back before a copy: 'copyToImage' discards their contents anyway
            void setOwner(QueueId _owner_queue_id, CommandPoolId _owner_cmd_pool_id);

            // Copies the data into the staging buffer now, the device buffer is written after 'submit'
            void copyToBuffer(BufferId _dst_buffer_id, const void *_data, VkDeviceSize _size, VkDeviceSize _dst_offset = 0u);

            // Copies tightly packed texels into mip level 0 of all layers of the image (the copy is done after 'submit');
            // in the submit the image goes from UNDEFINED (old contents are discarded) to TRANSFER_DST and then to '_final_layout'
            // Note: an EXCLUSIVE image used by another queue family needs 'setOwner' (or the image is CONCURRENT, see
            //       'ContextMemory::setWholeImageCreateInfoStruct'). Other mip levels are not filled; the data of one image must fit the staging buffer; depth/stencil images get the depth aspect
            void copyToImage(ImageId _dst_image_id, const void *_data, VkDeviceSize _size,
                             VkImageLayout _final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            // Records and submits all collected copies; returns the fence signaled when they are done (none if there was nothing to submit)
            FenceId submit();
            // Waits for the submitted copies; after it the staging buffer is free again
            void wait();
            bool isSubmitted() const;

        private:
            bool transfersOwnership_() const;

            static VkBufferMemoryBarrier2 bufferBarrier_(VkBuffer _buffer, uint32_t _src_family, uint32_t _dst_family,
                                                         VkPipelineStageFlags2 _src_stages, VkAccessFlags2 _src_access,
                                                         VkPipelineStageFlags2 _dst_stages, VkAccessFlags2 _dst_access);

            // '_keep_buffer_id' is still being written: its release waits for a later submit
            FenceId submit_(BufferId _keep_buffer_id);

            static bool sameBuffer_(BufferId _a, BufferId _b);
            static bool containsBuffer_(const std::vector<BufferId> &_buffers, BufferId _id);

            struct Region {
                BufferId     dst_buffer_id;
                VkBufferCopy copy;
            };
            struct ImageRegion {
                ImageId            dst_image_id;
                VkBufferImageCopy  copy;
                VkImageLayout      final_layout;
                VkImageAspectFlags aspect;
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
            std::vector<Region>      regions;
            std::vector<ImageRegion> image_regions;
            std::vector<BufferId>    released_buffers; // copied, but not released to the owner yet

            QueueId          owner_queue_id;
            CommandBuffersId owner_cmd_buffers_id; // [0]: gives buffers back (step 1), [1]: acquires the uploads (step 3)
            SemaphoreId      return_semaphore_id;  // step 1 -> step 2
            SemaphoreId      owner_semaphore_id;   // step 2 -> step 3

            UploadBatch(const UploadBatch &);
            UploadBatch& operator=(const UploadBatch &);
        }; // UploadBatch END
    }; // VKFW END
}; // Simple END

#endif