#ifndef SVKFW_VKFW_GRAPH_H
#define SVKFW_VKFW_GRAPH_H

#include <functional>
#include <string>

#include "vkfw.h"


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
            std::vector<uint32_t>              swapchain_barriers; // barriers whose image is set in 'execute'
            uint32_t final_first = 0u, final_count = 0u;
            bool compiled = false;

            explicit RenderGraph(VulkanContext &_cxt);
           ~RenderGraph();

        // Declaration

            ResourceId createTexture(const std::string &_name, const TextureDesc &_desc);
            // The swapchain image of the frame (set by 'execute'); presented at the end of the frame
            ResourceId importSwapchain();
            // An image the graph doesn't own; it is in '_initial_layout' at the start of the frame and is left in '_final_layout'
            ResourceId importImage(const std::string &_name, VkImage _image, VkImageView _view, VkFormat _format, VkExtent2D _extent,
                                   VkImageLayout _initial_layout, VkImageLayout _final_layout, bool _output = true);
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

        // Compilation

            // Note: call after the swapchain is created and after every recreation; waits for the device (objects may be recreated).
            //       Render passes are kept between compilations (their attachment formats must stay the same)
            void compile();

            // Records all live passes; 'rec' must be between 'begin' and 'end'
            void execute(CommandRecorder &_rec, uint32_t _image_index);

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

            void setupRendering_();

            // Note: one subpass; initial and final layouts are the attachment layouts, transitions are done by the barriers outside
            void createRenderPasses_();
            VkAttachmentDescription attachmentDescription_(const Access &_access, VkImageLayout _layout) const;

            void createFramebuffers_();
            void destroyFramebuffers_();

            RenderGraph(const RenderGraph &);
            RenderGraph& operator=(const RenderGraph &);
        }; // RenderGraph END


        }; // Graph END
    }; // VKFW END
}; // Simple END

#endif // SVKFW_VKFW_GRAPH_H
