#ifndef SVKFW_VKFW_GRAPH_H
#define SVKFW_VKFW_GRAPH_H

#include <functional>
#include <string>

#include "vkfw.h"
#include "main/image.h"


namespace Simple {
    namespace VKFW {
        namespace Graph {

//  == === ==== ================================== ==== === ==  \\
                    Render Graph
//  == === ==== ================================== ==== === ==  \\

        // Note: the frame is a list of passes; each pass declares what it reads and writes, the graph places
        //       barriers and layout transitions, owns the transient textures and begins/ends rendering
        //       (dynamic rendering or a VkRenderPass per pass, by 'profile.dynamic_rendering').
        //
        //       RenderGraph graph(vk);
        //       ResourceId backbuffer = graph.importSwapchain();
        //       PassId     scene      = graph.addGraphicsPass("scene").color(backbuffer, VK_ATTACHMENT_LOAD_OP_CLEAR, clear_color);
        //       graph.compile();                       // after the swapchain is created and after every recreation
        //       graph.pass(scene).execute([&](CommandRecorder &rec) { ... });
        //       ...
        //       graph.execute(rec, image_index);       // in the frame, between rec.begin() and rec.end()

        struct ResourceId {
            uint32_t index = UINT32_MAX;
            ResourceId();
            explicit ResourceId(uint32_t _index);
            bool isNone() const;
        };
        struct PassId {
            uint32_t index = UINT32_MAX;
            PassId();
            explicit PassId(uint32_t _index);
            bool isNone() const;
        };

        // Note: the size is 'extent' if it is set, otherwise the swapchain extent times 'scale';
        //       the usage is derived from the declared accesses ('extra_usage' is added to it)
        struct TextureDesc {
            VkFormat              format       = VK_FORMAT_UNDEFINED;
            float                 scale        = 1.f;
            VkExtent2D            extent       = {0u, 0u};
            VkSampleCountFlagBits samples      = VK_SAMPLE_COUNT_1_BIT;
            uint32_t              mip_levels   = 1u;
            uint32_t              array_layers = 1u;
            VkImageUsageFlags     extra_usage  = 0u;

            TextureDesc();
            TextureDesc(VkFormat _format, float _scale = 1.f);
            TextureDesc(VkFormat _format, VkExtent2D _extent);
        }; // TextureDesc END

        enum PassType {
            VKFW_PASS_GRAPHICS = 0,
            VKFW_PASS_COMPUTE,
            VKFW_PASS_TRANSFER
        };

        enum AccessType {
            VKFW_ACCESS_COLOR = 0,
            VKFW_ACCESS_DEPTH,
            VKFW_ACCESS_SAMPLED,
            VKFW_ACCESS_STORAGE_READ,
            VKFW_ACCESS_STORAGE_WRITE,
            VKFW_ACCESS_TRANSFER_SRC,
            VKFW_ACCESS_TRANSFER_DST
        };

        namespace Detail {
            // Note: only flags that exist without synchronization2, so the barriers can be converted
            const VkAccessFlags2 WRITE_ACCESS = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                                VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;

            struct AccessInfo {
                VkPipelineStageFlags2 stages;
                VkAccessFlags2        access;
                VkImageLayout         layout;
                VkImageUsageFlags     usage;
                bool                  write;
            };
            // '_shader_stages' is used by the shader accesses (sampled and storage)
            AccessInfo accessInfo(AccessType _type, VkFormat _format, VkPipelineStageFlags2 _shader_stages, VkAttachmentLoadOp _load_op);
        }; // Detail END

//  == === ==== ================================== ==== === ==  \\
                    Textures
//  == === ==== ================================== ==== === ==  \\

        // Note: everything needed for the image and its whole view; the defaults make a 2D sampled texture
        //       that can be uploaded with an 'UploadBatch' and get its mip levels from 'generateMips'
        struct TextureInfo {
            VkFormat              format       = VK_FORMAT_R8G8B8A8_SRGB;
            VkExtent3D            extent       = {1u, 1u, 1u}; // 'depth' > 1 makes a 3D texture
            uint32_t              mip_levels   = 1u;           // 0: the full chain down to 1x1 (see 'mipLevelCount')
            uint32_t              array_layers = 1u;
            VkSampleCountFlagBits samples      = VK_SAMPLE_COUNT_1_BIT;
            VkImageUsageFlags     usage        = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            VkImageCreateFlags    flags        = 0u;           // e.g. VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT for cube maps (6 layers each)
            VkImageTiling         tiling       = VK_IMAGE_TILING_OPTIMAL;
            // Families that use the texture; with several of them and VK_SHARING_MODE_EXCLUSIVE the uploads need
            // 'UploadBatch::setOwner' (see 'ContextMemory::setWholeImageCreateInfoStruct')
            std::vector<QueueFamilyId> queue_families;
            VkSharingMode         sharing_mode = VK_SHARING_MODE_EXCLUSIVE;

            // The whole view: VK_IMAGE_VIEW_TYPE_MAX_ENUM and 0 pick the type and aspect from the image
            VkImageViewType       view_type    = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
            VkImageAspectFlags    aspect       = 0u;
            VkComponentMapping    components   = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                                   VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };

            TextureInfo();
            TextureInfo(VkFormat _format, VkExtent2D _extent, uint32_t _mip_levels = 1u, Span<QueueFamilyId> _queue_families = {});
        }; // TextureInfo END

        // Number of mip levels down to 1x1(x1)
        uint32_t mipLevelCount(VkExtent3D _extent);

        // Note: an image of ContextMemory with its views; a copyable handle (it doesn't destroy anything by itself).
        //       views[0] is the whole view described by 'TextureInfo', 'addView' adds views of parts of the image
        //       (one mip level, one layer, another format of the same class, a swizzle...)
        struct Texture {
            VulkanContext           *cxt = nullptr;
            ImageId                  image_id;
            std::vector<ImageViewId> views;
            bool                     own_memory = false; // the texture has its own memory chunk

            Texture();

            bool isNone() const;
            VkImage     image() const;
            VkImageView view(uint32_t _index = 0u) const;
            const VkImageCreateInfo &info() const;
            VkExtent3D  extent(uint32_t _mip_level = 0u) const;

            // Adds a view and creates it if the memory is bound already (otherwise 'createViews' does it); returns its index in 'views'.
            // '_type' == VK_IMAGE_VIEW_TYPE_MAX_ENUM: from the image, '_aspect' == 0: from the format, '_format' == UNDEFINED: the image format
            uint32_t addView(uint32_t _base_mip = 0u, uint32_t _mip_count = VK_REMAINING_MIP_LEVELS,
                             uint32_t _base_layer = 0u, uint32_t _layer_count = VK_REMAINING_ARRAY_LAYERS,
                             VkImageViewType _type = VK_IMAGE_VIEW_TYPE_MAX_ENUM, VkImageAspectFlags _aspect = 0u,
                             VkFormat _format = VK_FORMAT_UNDEFINED, const VkComponentMapping &_components = VkComponentMapping());

            // Creates the views that aren't created yet; for textures in a shared chunk, after 'ContextMemory::allocateChunkMemory'
            void createViews();

            // Destroys the views and the image (with its own memory chunk if it has one); the device must not use them
            void destroy();
        }; // Texture END

        // Creates the image and its whole view.
        //   '_memory_id' is none: the texture gets its own DEVICE_LOCAL memory chunk and is ready to use;
        //   otherwise the image is added to that chunk (it must not be allocated yet) and the views are created
        //   by 'Texture::createViews' after 'ContextMemory::allocateChunkMemory' (many textures, one allocation)
        Texture createTexture(VulkanContext &_cxt, const TextureInfo &_info, MemoryId _memory_id = MemoryId());

        // Fills mip levels ['_mips_have', '_mips_req') of all layers: each level is a linear blit of the previous one.
        //   Before: levels [0, '_mips_have') hold the data and are in '_layout'; the contents of the other levels are discarded.
        //   After:  levels [0, '_mips_req') are in '_final_layout' (levels from '_mips_req' on are not touched).
        // '_mips_req' == 0: all levels of the image. With '_mips_have' >= '_mips_req' only the layout changes.
        // Note: needs a graphics queue, TRANSFER_SRC and TRANSFER_DST usage and a format with linear filtered blits (throws otherwise).
        //       The barriers wait for all earlier commands and make the result visible to all later ones (meant for loading, not per frame)
        void generateMips(CommandRecorder &_rec, const Texture &_texture, uint32_t _mips_have = 1u, uint32_t _mips_req = 0u,
                          VkImageLayout _layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VkImageLayout _final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


//  == === ==== ================================== ==== === ==  \\
                    Buffers
//  == === ==== ================================== ==== === ==  \\

        // Note: a buffer of ContextMemory; a copyable handle like 'Texture' (it doesn't destroy anything by itself).
        //       HOST_VISIBLE buffers stay mapped ('mapped', 'write'); DEVICE_LOCAL ones are filled with an 'UploadBatch'
        struct Buffer {
            VulkanContext *cxt = nullptr;
            BufferId       buffer_id;
            VkDeviceSize   size = 0u;
            bool           own_memory = false; // the buffer has its own memory chunk

            Buffer();

            bool isNone() const;
            VkBuffer handle() const;
            // nullptr if the memory isn't HOST_VISIBLE (or not allocated yet)
            void *mapped() const;
            // Copies into a HOST_VISIBLE buffer and flushes it if needed (the device must not use the range)
            void write(const void *_data, VkDeviceSize _size, VkDeviceSize _offset = 0u) const;
            // For descriptor writes
            VkDescriptorBufferInfo info(VkDeviceSize _offset = 0u, VkDeviceSize _range = VK_WHOLE_SIZE) const;

            // Destroys the buffer (with its own memory chunk if it has one); the device must not use it
            void destroy();
        }; // Buffer END

        // Creates a buffer of '_size' bytes.
        //   '_memory_id' is none: the buffer gets its own memory chunk with '_memory_flags' and is ready to use;
        //   otherwise the buffer is added to that chunk (it must not be allocated yet, '_memory_flags' is ignored) and can be used
        //   after 'ContextMemory::allocateChunkMemory' (many buffers, one allocation).
        // '_queue_families': the families that use it (several with VK_SHARING_MODE_CONCURRENT, see 'TextureInfo::queue_families')
        Buffer createBuffer(VulkanContext &_cxt, VkDeviceSize _size, VkBufferUsageFlags _usage,
                            VkMemoryPropertyFlags _memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            Span<QueueFamilyId> _queue_families = {}, VkSharingMode _sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
                            MemoryId _memory_id = MemoryId());

        // Rounds '_size' up to a multiple of '_alignment' (a power of two), e.g. for 'minUniformBufferOffsetAlignment'
        VkDeviceSize alignUp(VkDeviceSize _size, VkDeviceSize _alignment);


        // Note: a HOST_VISIBLE buffer that receives a copy of one image (see 'RenderGraph::readback'), e.g. for screenshots.
        //       The buffer is created and grown by 'RenderGraph::readback'; the data is ready after the fence of the submit with the
        //       copy is signaled, then 'readColor', 'readDepth', 'readStencil' or 'data' read it (the last recorded copy).
        //       A readback that is being written by the device must not be recorded again or destroyed (wait for its fence first).
        //       Layout in the buffer: texels are tightly packed rows, top row first; each aspect is a separate block (see 'offset')
        struct ImageReadback {
            VulkanContext     *cxt = nullptr;
            MemoryId           memory_id;
            BufferId           buffer_id;
            VkDeviceSize       capacity = 0u;

            // The last recorded copy
            VkFormat           format  = VK_FORMAT_UNDEFINED;
            VkExtent2D         extent  = {0u, 0u};
            VkImageAspectFlags aspects = 0u;
            VkDeviceSize       offsets[3] = {0u, 0u, 0u}; // of the color, depth and stencil blocks

            ImageReadback();

            bool isNone() const;
            bool has(VkImageAspectFlagBits _aspect) const;
            VkDeviceSize offset(VkImageAspectFlagBits _aspect) const;
            // The texels of one aspect (the mapped buffer; invalidated first if the memory isn't HOST_COHERENT)
            const void *data(VkImageAspectFlagBits _aspect = VK_IMAGE_ASPECT_COLOR_BIT) const;

            // Color images with 8-bit RGBA/BGRA formats (UNORM or SRGB; the values are copied as stored, sRGB is not decoded)
            void readColor(Img::Image4u &_image) const;
            // Depth in [0, 1] (D16, X8_D24, D24S8, D32, D32S8)
            void readDepth(Img::Image1f &_image) const;
            void readStencil(Img::Image1u &_image) const;

            // Note: the device must not use the buffer
            void destroy();

            // Makes the buffer at least '_size' bytes (a new memory chunk if it is smaller)
            void reserve_(VulkanContext &_cxt, VkDeviceSize _size);
        }; // ImageReadback END


        struct RenderGraph;

        // Note: a light handle for declaring the accesses of one pass; accesses must be declared before 'compile'
        struct PassBuilder {
            RenderGraph *graph = nullptr;
            PassId       id;

            PassBuilder(RenderGraph *_graph, PassId _id);
            operator PassId() const;

            // Attachments (graphics passes); without LOAD the previous contents are discarded
            PassBuilder& color(ResourceId _res, VkAttachmentLoadOp _load_op = VK_ATTACHMENT_LOAD_OP_LOAD, const VkClearValue &_clear_value = VkClearValue(),
                               VkAttachmentStoreOp _store_op = VK_ATTACHMENT_STORE_OP_STORE);
            PassBuilder& depth(ResourceId _res, VkAttachmentLoadOp _load_op = VK_ATTACHMENT_LOAD_OP_LOAD, const VkClearValue &_clear_value = VkClearValue(),
                               VkAttachmentStoreOp _store_op = VK_ATTACHMENT_STORE_OP_STORE);
            // Shader accesses; '_stages' == 0: the fragment shader in graphics passes, the compute shader in compute passes
            PassBuilder& sampled     (ResourceId _res, VkPipelineStageFlags2 _stages = 0u);
            PassBuilder& storageRead (ResourceId _res, VkPipelineStageFlags2 _stages = 0u);
            PassBuilder& storageWrite(ResourceId _res, VkPipelineStageFlags2 _stages = 0u);
            // Transfer accesses
            PassBuilder& transferSrc(ResourceId _res);
            PassBuilder& transferDst(ResourceId _res);
            // The pass is never culled (e.g. it writes buffers, which the graph doesn't track)
            PassBuilder& keep();
            // The commands of the pass; for graphics passes they are recorded inside the rendering, with viewport and scissor set
            PassBuilder& execute(const std::function<void(CommandRecorder&)> &_callback);
        }; // PassBuilder END

        struct RenderGraph {
            enum ResourceKind { VKFW_RESOURCE_TRANSIENT = 0, VKFW_RESOURCE_SWAPCHAIN, VKFW_RESOURCE_IMPORTED };

            struct Resource {
                std::string   name;
                ResourceKind  kind = VKFW_RESOURCE_TRANSIENT;
                TextureDesc   desc;
                VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED; // imported images
                VkImageLayout   final_layout = VK_IMAGE_LAYOUT_UNDEFINED; // imported images (UNDEFINED: left as the last pass needs it)
                bool          output = false;

                // Compiled
                VkImage           image  = VK_NULL_HANDLE;
                VkImageView       view   = VK_NULL_HANDLE;
                VkDeviceMemory    memory = VK_NULL_HANDLE; // transient textures only
                VkExtent2D        extent = {0u, 0u};
                VkImageUsageFlags usage  = 0u;
                bool              used   = false;          // accessed by a live pass
            };

            struct Access {
                ResourceId            res;
                AccessType            type;
                VkAttachmentLoadOp    load_op;
                VkAttachmentStoreOp   store_op;
                VkClearValue          clear_value;
                VkPipelineStageFlags2 stages;
            };

            struct Pass {
                std::string         name;
                PassType            type = VKFW_PASS_GRAPHICS;
                std::vector<Access> accesses;
                std::function<void(CommandRecorder&)> callback;
                bool                keep = false;

                // Compiled
                bool       live = false;
                uint32_t   barrier_first = 0u, barrier_count = 0u;
                VkExtent2D extent = {0u, 0u};
                std::vector<VkRenderingAttachmentInfo> colors;
                VkRenderingAttachmentInfo depth;
                bool       has_depth = false, has_stencil = false;
                uint32_t   swapchain_color = UINT32_MAX; // index in 'colors' of the swapchain image

                // Render pass backend
                VkRenderPass               render_pass = VK_NULL_HANDLE;
                std::vector<VkFramebuffer> framebuffers; // one per swapchain image if the pass renders into it, otherwise one
                std::vector<VkClearValue>  clear_values;
            };

            // Note: per resource state of the barrier computation
            struct State {
                VkImageLayout         layout = VK_IMAGE_LAYOUT_UNDEFINED;
                VkPipelineStageFlags2 write_stages = 0u, read_stages = 0u, visible_stages = 0u;
                VkAccessFlags2        write_access = 0u, visible_access = 0u;
            };

            VulkanContext         &cxt;
            std::vector<Resource>  resources;
            std::vector<Pass>      passes;

            std::vector<VkImageMemoryBarrier2> barriers;           // barriers of all passes, then the final ones
            std::vector<State>                 end_states;         // the state of each resource at the end of a frame
            std::vector<State>                 pass_states;        // the state of each resource after each pass (pass-major)
            std::vector<uint32_t>              swapchain_barriers; // barriers whose image is set in 'execute'
            uint32_t final_first = 0u, final_count = 0u;
            bool compiled = false;

            // Note: copies requested by 'readbackAfter', recorded by the next 'execute'
            struct ReadbackRequest {
                PassId              pass;
                ResourceId          res;
                ImageReadback      *readback;
                VkImageAspectFlags  aspects;
            };
            std::vector<ReadbackRequest> readback_requests;

            explicit RenderGraph(VulkanContext &_cxt);
           ~RenderGraph();

        // Declaration

            ResourceId createTexture(const std::string &_name, const TextureDesc &_desc);
            // The swapchain image of the frame (set by 'execute'); presented at the end of the frame
            ResourceId importSwapchain();
            // An image the graph doesn't own; it is in '_initial_layout' at the start of the frame and is left in '_final_layout'
            ResourceId importImage(const std::string &_name, VkImage _image, VkImageView _view, VkFormat _format, VkExtent2D _extent,
                                   VkImageLayout _initial_layout, VkImageLayout _final_layout, bool _output = true);
            // A texture the graph doesn't own (its whole view); e.g. sampled by passes and left in '_layout'
            ResourceId importTexture(const std::string &_name, const Texture &_texture,
                                     VkImageLayout _layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            // Passes writing an output are never culled
            void setOutput(ResourceId _res, bool _output = true);

            PassBuilder addGraphicsPass(const std::string &_name);
            PassBuilder addComputePass (const std::string &_name);
            PassBuilder addTransferPass(const std::string &_name);
            PassBuilder pass(PassId _pass);

        // Access (after 'compile')

            VkImageView view(ResourceId _res) const;
            VkImage image(ResourceId _res) const;
            // Note: only without dynamic rendering; pipelines of the pass are created with it (see 'ContextPipeline::createPipeline')
            VkRenderPass renderPass(PassId _pass) const;
            bool isLive(PassId _pass) const;
            // Attachment formats of a graphics pass (known before 'compile', the swapchain format needs the swapchain)
            std::vector<VkFormat> colorFormats(PassId _pass) const;
            VkFormat depthFormat(PassId _pass) const; // VK_FORMAT_UNDEFINED if the pass has no depth attachment

        // Compilation

            // Note: call after the swapchain is created and after every recreation; waits for the device (objects may be recreated).
            //       Render passes are kept between compilations (their attachment formats must stay the same)
            void compile();

            // Records all live passes; 'rec' must be between 'begin' and 'end'
            void execute(CommandRecorder &_rec, uint32_t _image_index);

            // Records a copy of the resource into '_readback' (mip level 0, layer 0); call after 'execute' with the same recorder and
            // image index. The image goes from its end-of-frame layout to TRANSFER_SRC and back, so later frames see it unchanged.
            // '_aspects' == 0: all aspects of the format (depth and stencil of a depth/stencil texture).
            // Note: the image needs VK_IMAGE_USAGE_TRANSFER_SRC_BIT: for graph textures put it in 'TextureDesc::extra_usage',
            //       for the swapchain in the swapchain image usage. The contents are those left by the frame: the last pass writing
            //       an attachment must store it (VK_ATTACHMENT_STORE_OP_STORE). Textures not used by a live pass don't exist
            void readback(CommandRecorder &_rec, ResourceId _res, uint32_t _image_index, ImageReadback &_readback, VkImageAspectFlags _aspects = 0u);

            // Like 'readback', but the copy is recorded by the next 'execute' right after the pass '_pass', so it holds the resource
            // as that pass left it (e.g. the swapchain image before the UI pass draws over it). Call before 'execute' (after 'compile');
            // '_readback' must stay alive until then. 'compile' drops the requests that are not recorded yet
            void readbackAfter(PassId _pass, ResourceId _res, ImageReadback &_readback, VkImageAspectFlags _aspects = 0u);

            // Note: the device must be idle; the declaration is kept, so the graph can be compiled again
            void destroy();

        // Internal (used by PassBuilder)

            Resource &resource_(ResourceId _res);
            Pass &pass_(PassId _pass);
            void addAccess_(PassId _pass, ResourceId _res, AccessType _type, VkAttachmentLoadOp _load_op, VkAttachmentStoreOp _store_op,
                            const VkClearValue &_clear_value, VkPipelineStageFlags2 _stages);

        private:
            ResourceId addResource_(const Resource &_res);
            PassBuilder addPass_(const std::string &_name, PassType _type);

            VkFormat format_(const Resource &_res) const;
            VkExtent2D extent_(const Resource &_res) const;
            static bool discards_(const Access &_access);

            // A pass is live if it is kept or writes a resource needed later; a write that discards the contents ends the need
            void cull_();

            void createTextures_();

            void destroyTexture_(Resource &_res);

            // One access of resource '_r': adds the barrier it needs (if '_emit') and updates the state
            void access_(uint32_t _r, State &_state, VkPipelineStageFlags2 _stages, VkAccessFlags2 _access, VkImageLayout _layout, bool _write, bool _emit);

            void track_(std::vector<State> &_states, bool _emit);

            // The copy of 'readback' and 'readbackAfter'; '_state' is the state of the resource at the point of the copy
            void recordReadback_(CommandRecorder &_rec, ResourceId _res, uint32_t _image_index, ImageReadback &_readback,
                                 VkImageAspectFlags _aspects, const State &_state);

            void setupRendering_();

            // Note: one subpass; initial and final layouts are the attachment layouts, transitions are done by the barriers outside
            void createRenderPasses_();
            VkAttachmentDescription attachmentDescription_(const Access &_access, VkImageLayout _layout) const;

            void createFramebuffers_();
            void destroyFramebuffers_();

            RenderGraph(const RenderGraph &);
            RenderGraph& operator=(const RenderGraph &);
        }; // RenderGraph END


//  == === ==== ================================== ==== === ==  \\
                    Pipelines for passes
//  == === ==== ================================== ==== === ==  \\

        // Loads a SPIR-V file and creates the shader module (throws if the file can't be read)
        ShaderModuleId loadShader(VulkanContext &_cxt, const std::string &_path);

        enum BlendMode {
            VKFW_BLEND_NONE = 0,       // the color is written as is
            VKFW_BLEND_ALPHA,          // src * a + dst * (1 - a)
            VKFW_BLEND_PREMULTIPLIED,  // src + dst * (1 - a)
            VKFW_BLEND_ADDITIVE        // src * a + dst
        };

        // Note: the usual state of a graphics pipeline drawn in a pass of the render graph. The attachment formats (or the render
        //       pass without dynamic rendering) come from the pass, viewport and scissor are dynamic (the graph sets them).
        //       The same blend mode is used for all color attachments. For anything else edit the pipeline create info
        //       ('vk.cxt_pipeline.pipeline(id).graphics_create_info_wrap') and call 'createPipeline' again
        struct GraphicsPipelineDesc {
            struct Stage {
                VkShaderStageFlagBits stage;
                ShaderModuleId        module;
                std::string           entry;
            };
            std::vector<Stage> stages;
            PipelineLayoutId   layout;

            std::vector<VkVertexInputBindingDescription>   bindings;
            std::vector<VkVertexInputAttributeDescription> attributes;
            VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            VkPolygonMode   polygon_mode = VK_POLYGON_MODE_FILL;
            VkCullModeFlags cull_mode    = VK_CULL_MODE_NONE;
            VkFrontFace     front_face   = VK_FRONT_FACE_COUNTER_CLOCKWISE;

            // Used only if the pass has a depth attachment
            bool        depth_test    = true;
            bool        depth_write   = true;
            VkCompareOp depth_compare = VK_COMPARE_OP_LESS;

            BlendMode   blend = VKFW_BLEND_NONE;
            // Used only if the pass has a stencil aspect; the reference is usually dynamic (VK_DYNAMIC_STATE_STENCIL_REFERENCE)
            bool             stencil_test = false;
            VkStencilOpState stencil_front = {}, stencil_back = {};

            // Added to VK_DYNAMIC_STATE_VIEWPORT and VK_DYNAMIC_STATE_SCISSOR
            std::vector<VkDynamicState> dynamic_states;

            GraphicsPipelineDesc();
            GraphicsPipelineDesc &shader(VkShaderStageFlagBits _stage, ShaderModuleId _module, const std::string &_entry = "main");
            // One interleaved vertex buffer at binding '_binding'
            GraphicsPipelineDesc &vertexBinding(uint32_t _binding, uint32_t _stride, VkVertexInputRate _rate = VK_VERTEX_INPUT_RATE_VERTEX);
            GraphicsPipelineDesc &vertexAttribute(uint32_t _location, uint32_t _binding, VkFormat _format, uint32_t _offset);
            // The same stencil state for both faces (see 'Util::stencilOpState')
            GraphicsPipelineDesc &stencil(const VkStencilOpState &_state);
        }; // GraphicsPipelineDesc END

        // Creates a graphics pipeline for drawing in the pass '_pass' (the graph must be compiled: the pass gives the attachment
        // formats or its render pass). Pipelines don't depend on the swapchain extent, so they survive 'compile' after a resize
        PipelineId createGraphicsPipeline(VulkanContext &_cxt, const RenderGraph &_graph, PassId _pass, const GraphicsPipelineDesc &_desc);

        // A pipeline layout with descriptor set layouts and push constant ranges
        PipelineLayoutId createPipelineLayout(VulkanContext &_cxt, Span<DescriptorSetLayoutId> _set_layouts,
                                              Span<VkPushConstantRange> _push_constants = {});


        // Note: screenshots. 'request' asks for one, 'record' (before 'RenderGraph::execute') requests its copy with 'readbackAfter',
        //       'save' (after the fence of that submit is waited) writes a PNG. One capture at a time: requests made while one is
        //       pending are ignored. Depth is saved normalized (the nearest visible depth is white, the farthest is dark gray, the
        //       cleared depth is black); stencil values are scaled so the largest one is white. Color alpha is saved as opaque
        struct FrameCapture {
            enum Kind { VKFW_CAPTURE_COLOR = 0, VKFW_CAPTURE_DEPTH, VKFW_CAPTURE_STENCIL };

            ImageReadback readback;
            bool          requested = false;
            Kind          kind = VKFW_CAPTURE_COLOR;
            PassId        pass;
            ResourceId    res;
            std::string   file_name;
            uint32_t      recorded_frame = UINT32_MAX; // the frame of the recorded copy, UINT32_MAX if there is none

            // The copy holds '_res' as the pass '_after' left it; '_file_name' is written as is (e.g. "screenshots/scene.png").
            // Returns false if a capture is already requested or recorded
            bool request(Kind _kind, PassId _after, ResourceId _res, const std::string &_file_name);
            bool pending() const;

            // Call before 'RenderGraph::execute' of the frame; '_frame' identifies its fence (e.g. the frame in flight index)
            void record(RenderGraph &_graph, uint32_t _frame);
            // Call after the fence of '_frame' is waited; does nothing for other frames. Returns true if the file is written
            bool save(uint32_t _frame);

            // Note: the device must not use the buffer
            void destroy();
        }; // FrameCapture END


//  == === ==== ================================== ==== === ==  \\
                    Device setup
//  == === ==== ================================== ==== === ==  \\

        // Note: the parameters of 'setupDevice'. Device features are set before the call in 'vk.cxt_core.device.create_info.features'
        //       (required) and 'optional_features' (enabled if supported); the features of 'profile' are added by 'createDevice'
        struct DeviceSetup {
            // Instance
            bool        debug = true;                     // validation layers and the debug messenger
            std::vector<std::string> optional_layers = { "VK_LAYER_LUNARG_monitor" }; // added if available (not shipped with newer SDKs)
            std::vector<std::string> instance_extensions;         // added to the GLFW ones and VK_KHR_surface
            // Physical device: the best ranked one with a rank > 0 (default: discrete > integrated > virtual > CPU, with VK_KHR_swapchain)
            Util::PhysDeviceRankFunc device_rank = nullptr;
            // Note: dynamic rendering + synchronization2 + extended dynamic state need Vulkan 1.3; with all set to false rendering goes
            //       through VkRenderPass with the original barriers (Vulkan 1.1) and the stencil ops are fixed in the pipeline
            DeviceProfile profile;
            // Queues: the graphics family (also presents) gives the graphics queue and, if it has more queues, a separate present queue;
            //         the transfer queue is in a transfer-only (or the least loaded) family if there is one, otherwise it is the graphics queue
            bool        separate_present_queue = true;
            bool        separate_transfer_family = true;
            std::vector<std::string> device_extensions;          // required (VK_KHR_swapchain is always added)
            std::vector<std::string> optional_device_extensions = { VK_EXT_MEMORY_BUDGET_EXTENSION_NAME }; // enabled if supported
            // Swapchain
            uint32_t          image_count  = 3u;          // clamped to the surface limits
            VkImageUsageFlags image_usage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // the supported ones are set
            VkPresentModeKHR  present_mode = VK_PRESENT_MODE_FIFO_KHR;
            VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

            DeviceSetup();
        }; // DeviceSetup END

        // Note: the queues made by 'setupDevice'; when there is no separate family / queue the ids are the same
        struct DeviceQueues {
            QueueFamilyId graphics_family, transfer_family;
            QueueId       graphics, present, transfer;

            bool sharedTransfer() const; // the transfer queue is in the graphics family
        }; // DeviceQueues END

        // The rank of 'DeviceSetup::device_rank' by default
        int defaultDeviceRank(const Util::PhysDeviceFullInfo &_info);

        // Creates the instance, the debug messenger, picks the physical device, creates the device with the queues, the non-core
        // contexts, the surface of '_window' and the swapchain with its image views
        DeviceQueues setupDevice(VulkanContext &_cxt, const Window &_window, const DeviceSetup &_setup = DeviceSetup());


//  == === ==== ================================== ==== === ==  \\
                    Frames in flight
//  == === ==== ================================== ==== === ==  \\

        // Records commands with '_record' into a new command buffer, submits them to '_queue' (of '_family') and waits for them.
        // Meant for loading (e.g. 'generateMips' after an upload), not for every frame: it creates and destroys a command pool
        void submitAndWait(VulkanContext &_cxt, QueueId _queue, QueueFamilyId _family, const std::function<void(CommandRecorder&)> &_record);

        // Note: the frame loop of a window: per frame in flight a command buffer, an 'image available' semaphore and a fence,
        //       per swapchain image a 'render finished' semaphore (there may be more images than frames, and their count may change
        //       after a recreation). 'begin' waits for the fence of the frame and acquires an image, 'end' records the graph,
        //       submits and presents; both recreate the swapchain and compile the graph when it is out of date.
        //
        //       FrameLoop frames(vk, graph, queues, FRAMES_IN_FLIGHT);
        //       while (!window.shouldClose()) {
        //           ...                                  // input
        //           if (!frames.begin()) continue;       // the swapchain was recreated or isn't ready: no frame this time
        //           ...                                  // updates of the resources of 'frames.frame' (the device doesn't use them now)
        //           frames.end();                        // 'graph.execute', submit, present
        //       }
        //       frames.destroy();                        // waits for the device
        struct FrameLoop {
            VulkanContext   &cxt;
            RenderGraph     &graph;
            QueueId          graphics, present;
            uint32_t         frames_in_flight;
            uint32_t         frame       = 0u;      // the frame in flight (index of its resources)
            uint32_t         image_index = 0u;      // the swapchain image of the frame (after 'begin')
            CommandPoolId    cmd_pool;
            CommandBuffersId cmd_buffers;
            std::vector<SemaphoreId> image_available; // per frame in flight
            std::vector<SemaphoreId> render_finished; // per swapchain image
            std::vector<FenceId>     in_flight;       // per frame in flight
            // Called after the swapchain is recreated and the graph is compiled (e.g. for resources with the window size)
            std::function<void()> on_recreate;

            FrameLoop(VulkanContext &_cxt, RenderGraph &_graph, const DeviceQueues &_queues, uint32_t _frames_in_flight);
           ~FrameLoop();

            CommandBufferId commandBuffer(uint32_t _frame) const;
            // The command buffer of the current frame (it is begun by 'begin')
            CommandRecorder recorder() const;

            // Waits for the frame's fence, acquires a swapchain image and begins its command buffer; returns false if there is no
            // frame now (the swapchain was out of date and is recreated, or the image isn't ready)
            bool begin();
            // Records the graph, ends the command buffer, submits it and presents the image; the next frame becomes current
            void end();
            // Recreates the swapchain and compiles the graph (waits for the device)
            void recreate();
            // Waits for the device and frees the command buffers (the semaphores and fences go with the context)
            void destroy();

        private:
            bool destroyed_ = false;
            FrameLoop(const FrameLoop &);
            FrameLoop& operator=(const FrameLoop &);
        }; // FrameLoop END


        }; // Graph END
    }; // VKFW END
}; // Simple END

#endif // SVKFW_VKFW_GRAPH_H
