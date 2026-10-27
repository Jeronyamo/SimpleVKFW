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
            ResourceId() {}
            explicit ResourceId(uint32_t _index) : index{_index} {}
            bool isNone() const { return index == UINT32_MAX; }
        };
        struct PassId {
            uint32_t index = UINT32_MAX;
            PassId() {}
            explicit PassId(uint32_t _index) : index{_index} {}
            bool isNone() const { return index == UINT32_MAX; }
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

            TextureDesc() {}
            TextureDesc(VkFormat _format, float _scale = 1.f) : format{_format}, scale{_scale} {}
            TextureDesc(VkFormat _format, VkExtent2D _extent) : format{_format}, extent(_extent) {}
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
            inline AccessInfo accessInfo(AccessType _type, VkFormat _format, VkPipelineStageFlags2 _shader_stages, VkAttachmentLoadOp _load_op) {
                AccessInfo __info{};
                switch (_type) {
                    case VKFW_ACCESS_COLOR:
                        __info.stages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                        __info.access = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | (_load_op == VK_ATTACHMENT_LOAD_OP_LOAD ? VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT : 0u);
                        __info.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        __info.usage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                        __info.write  = true; break;
                    case VKFW_ACCESS_DEPTH:
                        __info.stages = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
                        __info.access = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        __info.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        __info.usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                        __info.write  = true; break;
                    case VKFW_ACCESS_SAMPLED:
                        __info.stages = _shader_stages;
                        __info.access = VK_ACCESS_2_SHADER_READ_BIT;
                        __info.layout = VKFW::Detail::hasDepth(_format) || VKFW::Detail::hasStencil(_format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        __info.usage  = VK_IMAGE_USAGE_SAMPLED_BIT;
                        __info.write  = false; break;
                    case VKFW_ACCESS_STORAGE_READ:
                        __info.stages = _shader_stages;
                        __info.access = VK_ACCESS_2_SHADER_READ_BIT;
                        __info.layout = VK_IMAGE_LAYOUT_GENERAL;
                        __info.usage  = VK_IMAGE_USAGE_STORAGE_BIT;
                        __info.write  = false; break;
                    case VKFW_ACCESS_STORAGE_WRITE:
                        __info.stages = _shader_stages;
                        __info.access = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                        __info.layout = VK_IMAGE_LAYOUT_GENERAL;
                        __info.usage  = VK_IMAGE_USAGE_STORAGE_BIT;
                        __info.write  = true; break;
                    case VKFW_ACCESS_TRANSFER_SRC:
                        __info.stages = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                        __info.access = VK_ACCESS_2_TRANSFER_READ_BIT;
                        __info.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                        __info.usage  = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                        __info.write  = false; break;
                    case VKFW_ACCESS_TRANSFER_DST:
                        __info.stages = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                        __info.access = VK_ACCESS_2_TRANSFER_WRITE_BIT;
                        __info.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                        __info.usage  = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                        __info.write  = true; break;
                }
                return __info;
            }
        }; // Detail END

        struct RenderGraph;

        // Note: a light handle for declaring the accesses of one pass; accesses must be declared before 'compile'
        struct PassBuilder {
            RenderGraph *graph = nullptr;
            PassId       id;

            PassBuilder(RenderGraph *_graph, PassId _id) : graph{_graph}, id(_id) {}
            operator PassId() const { return id; }

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

            explicit RenderGraph(VulkanContext &_cxt) : cxt(_cxt) {}
           ~RenderGraph() { destroy(); }

        // Declaration

            ResourceId createTexture(const std::string &_name, const TextureDesc &_desc) {
                Resource __res;
                __res.name = _name;
                __res.kind = VKFW_RESOURCE_TRANSIENT;
                __res.desc = _desc;
                return addResource_(__res);
            }
            // The swapchain image of the frame (set by 'execute'); presented at the end of the frame
            ResourceId importSwapchain() {
                Resource __res;
                __res.name   = "swapchain";
                __res.kind   = VKFW_RESOURCE_SWAPCHAIN;
                __res.output = true;
                return addResource_(__res);
            }
            // An image the graph doesn't own; it is in '_initial_layout' at the start of the frame and is left in '_final_layout'
            ResourceId importImage(const std::string &_name, VkImage _image, VkImageView _view, VkFormat _format, VkExtent2D _extent,
                                   VkImageLayout _initial_layout, VkImageLayout _final_layout, bool _output = true) {
                Resource __res;
                __res.name           = _name;
                __res.kind           = VKFW_RESOURCE_IMPORTED;
                __res.desc           = TextureDesc(_format, _extent);
                __res.initial_layout = _initial_layout;
                __res.final_layout   = _final_layout;
                __res.output         = _output;
                __res.image          = _image;
                __res.view           = _view;
                __res.extent         = _extent;
                return addResource_(__res);
            }
            // Passes writing an output are never culled
            void setOutput(ResourceId _res, bool _output = true) { resource_(_res).output = _output; }

            PassBuilder addGraphicsPass(const std::string &_name) { return addPass_(_name, VKFW_PASS_GRAPHICS); }
            PassBuilder addComputePass (const std::string &_name) { return addPass_(_name, VKFW_PASS_COMPUTE ); }
            PassBuilder addTransferPass(const std::string &_name) { return addPass_(_name, VKFW_PASS_TRANSFER); }
            PassBuilder pass(PassId _pass) { pass_(_pass); return PassBuilder(this, _pass); }

        // Access (after 'compile')

            VkImageView view(ResourceId _res) const {
                const Resource &__res = resources.at(_res.index);
                SVKFW_ASSERT(__res.kind != VKFW_RESOURCE_SWAPCHAIN, std::invalid_argument, "VKFW::Graph::RenderGraph :: view", "the swapchain view changes every frame");
                return __res.view;
            }
            VkImage image(ResourceId _res) const {
                const Resource &__res = resources.at(_res.index);
                SVKFW_ASSERT(__res.kind != VKFW_RESOURCE_SWAPCHAIN, std::invalid_argument, "VKFW::Graph::RenderGraph :: image", "the swapchain image changes every frame");
                return __res.image;
            }
            // Note: only without dynamic rendering; pipelines of the pass are created with it (see 'ContextPipeline::createPipeline')
            VkRenderPass renderPass(PassId _pass) const {
                SVKFW_ASSERT(compiled, std::logic_error, "VKFW::Graph::RenderGraph :: renderPass", "the graph is not compiled");
                return passes.at(_pass.index).render_pass;
            }
            bool isLive(PassId _pass) const { return passes.at(_pass.index).live; }

        // Compilation

            // Note: call after the swapchain is created and after every recreation; waits for the device (objects may be recreated).
            //       Render passes are kept between compilations (their attachment formats must stay the same)
            void compile() {
                const VkDevice __device = cxt.cxt_core.device.handle;
                SVKFW_ASSERT(__device != VK_NULL_HANDLE, std::logic_error, "VKFW::Graph::RenderGraph :: compile", "the device is not created");

                cull_();
                vkDeviceWaitIdle(__device);
                destroyFramebuffers_();
                createTextures_();

                // Note: the first run finds the state at the end of the frame, the second one starts from it,
                //       so the first barriers of a frame also wait for the previous frame using the same images
                std::vector<State> __states(resources.size());
                track_(__states, false);
                for (uint32_t r = 0u; r < resources.size(); ++r) {
                    State __end = __states[r];
                    __states[r] = State();
                    if (resources[r].kind == VKFW_RESOURCE_SWAPCHAIN) {
                        // Note: the wait on the 'image available' semaphore must include COLOR_ATTACHMENT_OUTPUT
                        __states[r].write_stages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                        continue;
                    }
                    __states[r].layout       = resources[r].kind == VKFW_RESOURCE_IMPORTED ? resources[r].initial_layout : VK_IMAGE_LAYOUT_UNDEFINED;
                    __states[r].write_stages = __end.write_stages | __end.read_stages;
                    __states[r].write_access = __end.write_access;
                }
                track_(__states, true);

                setupRendering_();
                if (!cxt.cxt_core.profile.dynamic_rendering) {
                    createRenderPasses_();
                    createFramebuffers_();
                }
                compiled = true;
            }

            // Records all live passes; 'rec' must be between 'begin' and 'end'
            void execute(CommandRecorder &_rec, uint32_t _image_index) {
                SVKFW_ASSERT(compiled, std::logic_error, "VKFW::Graph::RenderGraph :: execute", "the graph is not compiled");
                const bool __dynamic_rendering = cxt.cxt_core.profile.dynamic_rendering;
                if (!swapchain_barriers.empty()) {
                    VkImage __image = cxt.cxt_swapchain.image(_image_index);
                    for (uint32_t i = 0u; i < swapchain_barriers.size(); ++i)
                        barriers[swapchain_barriers[i]].image = __image;
                }

                for (uint32_t p = 0u; p < passes.size(); ++p) {
                    Pass &__pass = passes[p];
                    if (!__pass.live) continue;
                    if (__pass.barrier_count)
                        _rec.barrier(Span<VkImageMemoryBarrier2>(&barriers[__pass.barrier_first], __pass.barrier_count));

                    if (__pass.type == VKFW_PASS_GRAPHICS) {
                        if (__dynamic_rendering) {
                            if (__pass.swapchain_color != UINT32_MAX)
                                __pass.colors[__pass.swapchain_color].imageView = cxt.cxt_swapchain.imageView(_image_index);
                            _rec.beginRendering(__pass.extent, __pass.colors, __pass.has_depth ? &__pass.depth : nullptr,
                                                                              __pass.has_stencil ? &__pass.depth : nullptr);
                        }
                        else {
                            VkRenderPassBeginInfo __info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
                            __info.renderPass        = __pass.render_pass;
                            __info.framebuffer       = __pass.framebuffers[__pass.framebuffers.size() > 1u ? _image_index : 0u];
                            __info.renderArea.extent = __pass.extent;
                            __info.clearValueCount   = __pass.clear_values.size();
                            __info.pClearValues      = __pass.clear_values.data();
                            _rec.beginRenderPass(__info);
                        }
                        _rec.setViewportScissor(__pass.extent);
                    }

                    if (__pass.callback) __pass.callback(_rec);

                    if (__pass.type == VKFW_PASS_GRAPHICS) {
                        if (__dynamic_rendering) _rec.endRendering();
                        else                     _rec.endRenderPass();
                    }
                }

                if (final_count)
                    _rec.barrier(Span<VkImageMemoryBarrier2>(&barriers[final_first], final_count));
            }

            // Note: the device must be idle; the declaration is kept, so the graph can be compiled again
            void destroy() {
                if (cxt.cxt_core.device.handle == VK_NULL_HANDLE) return;
                destroyFramebuffers_();
                for (uint32_t p = 0u; p < passes.size(); ++p) {
                    Func::DestroyRenderPass(cxt.cxt_core.device.handle, passes[p].render_pass, cxt.cxt_core.alloc_cb.get());
                    passes[p].render_pass = VK_NULL_HANDLE;
                }
                for (uint32_t r = 0u; r < resources.size(); ++r)
                    if (resources[r].kind == VKFW_RESOURCE_TRANSIENT) destroyTexture_(resources[r]);
                compiled = false;
            }

        // Internal (used by PassBuilder)

            Resource &resource_(ResourceId _res) {
                SVKFW_ASSERT(_res.index < resources.size(), std::out_of_range, "VKFW::Graph::RenderGraph", "unknown resource");
                return resources[_res.index];
            }
            Pass &pass_(PassId _pass) {
                SVKFW_ASSERT(_pass.index < passes.size(), std::out_of_range, "VKFW::Graph::RenderGraph", "unknown pass");
                return passes[_pass.index];
            }
            void addAccess_(PassId _pass, ResourceId _res, AccessType _type, VkAttachmentLoadOp _load_op, VkAttachmentStoreOp _store_op,
                            const VkClearValue &_clear_value, VkPipelineStageFlags2 _stages) {
                Pass &__pass = pass_(_pass);
                resource_(_res);
                SVKFW_ASSERT(!(_type == VKFW_ACCESS_COLOR || _type == VKFW_ACCESS_DEPTH) || __pass.type == VKFW_PASS_GRAPHICS,
                                std::invalid_argument, "VKFW::Graph::PassBuilder", "attachments are used only by graphics passes");
                if (_stages == 0u)
                    _stages = __pass.type == VKFW_PASS_GRAPHICS ? VkPipelineStageFlags2(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT) :
                              __pass.type == VKFW_PASS_COMPUTE  ? VkPipelineStageFlags2(VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT ) :
                                                                  VkPipelineStageFlags2(VK_PIPELINE_STAGE_2_TRANSFER_BIT       );
                Access __access;
                __access.res         = _res;
                __access.type        = _type;
                __access.load_op     = _load_op;
                __access.store_op    = _store_op;
                __access.clear_value = _clear_value;
                __access.stages      = _stages;
                __pass.accesses.push_back(__access);
                compiled = false;
            }

        private:
            ResourceId addResource_(const Resource &_res) {
                resources.push_back(_res);
                compiled = false;
                return ResourceId(resources.size() - 1u);
            }
            PassBuilder addPass_(const std::string &_name, PassType _type) {
                Pass __pass;
                __pass.name = _name;
                __pass.type = _type;
                passes.push_back(__pass);
                compiled = false;
                return PassBuilder(this, PassId(passes.size() - 1u));
            }

            VkFormat format_(const Resource &_res) const {
                return _res.kind == VKFW_RESOURCE_SWAPCHAIN ? cxt.cxt_swapchain.format() : _res.desc.format;
            }
            VkExtent2D extent_(const Resource &_res) const {
                return _res.kind == VKFW_RESOURCE_SWAPCHAIN ? cxt.cxt_swapchain.extent() : _res.extent;
            }
            static bool discards_(const Access &_access) {
                return (_access.type == VKFW_ACCESS_COLOR || _access.type == VKFW_ACCESS_DEPTH) && _access.load_op != VK_ATTACHMENT_LOAD_OP_LOAD;
            }

            // A pass is live if it is kept or writes a resource needed later; a write that discards the contents ends the need
            void cull_() {
                std::vector<char> __needed(resources.size(), 0);
                for (uint32_t r = 0u; r < resources.size(); ++r) {
                    __needed[r] = resources[r].output;
                    resources[r].used = false;
                }

                for (uint32_t p = (uint32_t)passes.size(); p-- > 0u; ) {
                    Pass &__pass = passes[p];
                    __pass.live = __pass.keep;
                    for (uint32_t a = 0u; a < __pass.accesses.size(); ++a) {
                        const Access &__access = __pass.accesses[a];
                        if (Detail::accessInfo(__access.type, VK_FORMAT_UNDEFINED, __access.stages, __access.load_op).write && __needed[__access.res.index])
                            __pass.live = true;
                    }
                    if (!__pass.live) continue;

                    for (uint32_t a = 0u; a < __pass.accesses.size(); ++a)
                        if (discards_(__pass.accesses[a])) __needed[__pass.accesses[a].res.index] = 0;
                    for (uint32_t a = 0u; a < __pass.accesses.size(); ++a) {
                        if (!discards_(__pass.accesses[a])) __needed[__pass.accesses[a].res.index] = 1;
                        resources[__pass.accesses[a].res.index].used = true;
                    }
                }
            }

            void createTextures_() {
                const VkDevice               __device   = cxt.cxt_core.device.handle;
                const VkAllocationCallbacks *__alloc_cb = cxt.cxt_core.alloc_cb.get();
                const VkExtent2D             __sc_extent = cxt.cxt_swapchain.extent();

                std::vector<VkImageUsageFlags> __usage(resources.size(), 0u);
                for (uint32_t p = 0u; p < passes.size(); ++p) {
                    if (!passes[p].live) continue;
                    for (uint32_t a = 0u; a < passes[p].accesses.size(); ++a) {
                        const Access &__access = passes[p].accesses[a];
                        __usage[__access.res.index] |= Detail::accessInfo(__access.type, VK_FORMAT_UNDEFINED, __access.stages, __access.load_op).usage;
                    }
                }

                for (uint32_t r = 0u; r < resources.size(); ++r) {
                    Resource &__res = resources[r];
                    if (__res.kind != VKFW_RESOURCE_TRANSIENT) continue;

                    VkExtent2D __extent = __res.desc.extent;
                    if (__extent.width == 0u || __extent.height == 0u) {
                        __extent.width  = std::max(1u, (uint32_t)(__sc_extent.width  * __res.desc.scale));
                        __extent.height = std::max(1u, (uint32_t)(__sc_extent.height * __res.desc.scale));
                    }
                    const VkImageUsageFlags __res_usage = __usage[r] | __res.desc.extra_usage;
                    if (__res.image != VK_NULL_HANDLE && (!__res.used || __res.usage != __res_usage ||
                                                          __res.extent.width != __extent.width || __res.extent.height != __extent.height))
                        destroyTexture_(__res);
                    if (!__res.used || __res.image != VK_NULL_HANDLE) continue;

                    VkImageCreateInfo __image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
                    __image_info.imageType     = VK_IMAGE_TYPE_2D;
                    __image_info.format        = __res.desc.format;
                    __image_info.extent        = { __extent.width, __extent.height, 1u };
                    __image_info.mipLevels     = __res.desc.mip_levels;
                    __image_info.arrayLayers   = __res.desc.array_layers;
                    __image_info.samples       = __res.desc.samples;
                    __image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
                    __image_info.usage         = __res_usage;
                    __image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
                    __image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    __res.image = Func::CreateImage(__device, &__image_info, __alloc_cb);

                    VkMemoryRequirements __mem_req = Func::GetImageMemoryRequirements(__device, __res.image);
                    VkMemoryAllocateInfo __alloc_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
                    __alloc_info.allocationSize  = __mem_req.size;
                    __alloc_info.memoryTypeIndex = Util::checkPhysicalDeviceMemoryProperties(cxt.cxt_core.phys_device.handle, __mem_req.memoryTypeBits,
                                                                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                    SVKFW_ASSERT(__alloc_info.memoryTypeIndex != UINT32_MAX, std::runtime_error, "VKFW::Graph::RenderGraph :: compile",
                                    "no device-local memory for the texture '" + __res.name + "'");
                    __res.memory = Func::AllocateMemory(__device, &__alloc_info, __alloc_cb);
                    Func::BindImageMemory(__device, __res.image, __res.memory, 0u);

                    // Note: a depth/stencil texture that is only sampled gets a depth-only view (a sampled view can't have both aspects)
                    VkImageAspectFlags __aspects = VKFW::Detail::formatAspects(__res.desc.format);
                    if (__aspects == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) && !(__res_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
                        __aspects = VK_IMAGE_ASPECT_DEPTH_BIT;
                    VkImageViewCreateInfo __view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
                    __view_info.image            = __res.image;
                    __view_info.viewType         = __res.desc.array_layers > 1u ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
                    __view_info.format           = __res.desc.format;
                    __view_info.subresourceRange = { __aspects, 0u, __res.desc.mip_levels, 0u, __res.desc.array_layers };
                    __res.view = Func::CreateImageView(__device, &__view_info, __alloc_cb);

                    __res.extent = __extent;
                    __res.usage  = __res_usage;
                }
            }

            void destroyTexture_(Resource &_res) {
                const VkDevice               __device   = cxt.cxt_core.device.handle;
                const VkAllocationCallbacks *__alloc_cb = cxt.cxt_core.alloc_cb.get();
                Func::DestroyImageView(__device, _res.view,   __alloc_cb);
                Func::DestroyImage    (__device, _res.image,  __alloc_cb);
                Func::FreeMemory      (__device, _res.memory, __alloc_cb);
                _res.view   = VK_NULL_HANDLE;
                _res.image  = VK_NULL_HANDLE;
                _res.memory = VK_NULL_HANDLE;
                _res.extent = VkExtent2D{0u, 0u};
                _res.usage  = 0u;
            }

            // One access of resource '_r': adds the barrier it needs (if '_emit') and updates the state
            void access_(uint32_t _r, State &_state, VkPipelineStageFlags2 _stages, VkAccessFlags2 _access, VkImageLayout _layout, bool _write, bool _emit) {
                const bool __transition = _layout != _state.layout;
                bool __need = false;
                VkPipelineStageFlags2 __src_stages = 0u;
                VkAccessFlags2        __src_access = 0u;
                if (__transition || _write) {
                    // Note: write after write/read; reads need only an execution dependency
                    __src_stages = _state.write_stages | _state.read_stages;
                    __src_access = _state.write_access;
                    __need = __transition || __src_stages != 0u;
                }
                else if (_state.write_stages != 0u && ((_stages & ~_state.visible_stages) != 0u || (_access & ~_state.visible_access) != 0u)) {
                    // Note: read after write, not made visible to these stages yet
                    __src_stages = _state.write_stages;
                    __src_access = _state.write_access;
                    __need = true;
                }

                if (__need && _emit) {
                    const Resource &__res = resources[_r];
                    if (__res.kind == VKFW_RESOURCE_SWAPCHAIN) swapchain_barriers.push_back(barriers.size());
                    barriers.push_back(imageBarrier(__res.image, _state.layout, _layout, __src_stages, __src_access, _stages, _access,
                                                    VKFW::Detail::formatAspects(format_(__res))));
                }

                if (__transition || _write) {
                    // Note: a layout transition is a write too, later accesses are ordered after it
                    _state.write_stages   = _stages;
                    _state.write_access   = _write ? (_access & Detail::WRITE_ACCESS) : VkAccessFlags2(0u);
                    _state.read_stages    = _write ? VkPipelineStageFlags2(0u) : _stages;
                    _state.visible_stages = _stages;
                    _state.visible_access = _access;
                }
                else {
                    _state.read_stages |= _stages;
                    if (__need) {
                        _state.visible_stages |= _stages;
                        _state.visible_access |= _access;
                    }
                }
                _state.layout = _layout;
            }

            void track_(std::vector<State> &_states, bool _emit) {
                if (_emit) {
                    barriers.clear();
                    swapchain_barriers.clear();
                }
                for (uint32_t p = 0u; p < passes.size(); ++p) {
                    Pass &__pass = passes[p];
                    if (!__pass.live) continue;
                    if (_emit) __pass.barrier_first = barriers.size();
                    for (uint32_t a = 0u; a < __pass.accesses.size(); ++a) {
                        const Access &__access = __pass.accesses[a];
                        const uint32_t r = __access.res.index;
                        Detail::AccessInfo __info = Detail::accessInfo(__access.type, format_(resources[r]), __access.stages, __access.load_op);
                        access_(r, _states[r], __info.stages, __info.access, __info.layout, __info.write, _emit);
                    }
                    if (_emit) __pass.barrier_count = barriers.size() - __pass.barrier_first;
                }

                // Final transitions: the swapchain image to PRESENT_SRC, imported images to their final layout
                if (_emit) final_first = barriers.size();
                for (uint32_t r = 0u; r < resources.size(); ++r) {
                    const Resource &__res = resources[r];
                    State &__state = _states[r];
                    if (__res.kind == VKFW_RESOURCE_SWAPCHAIN) {
                        // Note: the destination stages are the last ones that used the image (the present waits on the submit semaphore)
                        VkPipelineStageFlags2 __stages = __state.write_stages | __state.read_stages;
                        access_(r, __state, __stages ? __stages : VkPipelineStageFlags2(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT),
                                0u, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, false, _emit);
                    }
                    else if (__res.kind == VKFW_RESOURCE_IMPORTED && __res.final_layout != VK_IMAGE_LAYOUT_UNDEFINED && __res.final_layout != __state.layout)
                        access_(r, __state, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0u, __res.final_layout, false, _emit);
                }
                if (_emit) final_count = barriers.size() - final_first;
            }

            void setupRendering_() {
                for (uint32_t p = 0u; p < passes.size(); ++p) {
                    Pass &__pass = passes[p];
                    __pass.colors.clear();
                    __pass.clear_values.clear();
                    __pass.has_depth = __pass.has_stencil = false;
                    __pass.swapchain_color = UINT32_MAX;
                    __pass.extent = VkExtent2D{0u, 0u};
                    if (!__pass.live || __pass.type != VKFW_PASS_GRAPHICS) continue;

                    for (uint32_t a = 0u; a < __pass.accesses.size(); ++a) {
                        const Access &__access = __pass.accesses[a];
                        if (__access.type != VKFW_ACCESS_COLOR && __access.type != VKFW_ACCESS_DEPTH) continue;
                        const Resource &__res = resources[__access.res.index];
                        const VkExtent2D __extent = extent_(__res);
                        if (__pass.extent.width == 0u) __pass.extent = __extent;
                        SVKFW_ASSERT(__pass.extent.width == __extent.width && __pass.extent.height == __extent.height, std::runtime_error,
                                        "VKFW::Graph::RenderGraph :: compile", "attachments of the pass '" + __pass.name + "' have different sizes");

                        Detail::AccessInfo __info = Detail::accessInfo(__access.type, format_(__res), __access.stages, __access.load_op);
                        VkRenderingAttachmentInfo __attachment = renderingAttachment(__res.view, __info.layout, __access.load_op, __access.store_op, __access.clear_value);
                        if (__access.type == VKFW_ACCESS_COLOR) {
                            if (__res.kind == VKFW_RESOURCE_SWAPCHAIN) __pass.swapchain_color = __pass.colors.size();
                            __pass.colors.push_back(__attachment);
                        }
                        else {
                            SVKFW_ASSERT(!__pass.has_depth && !__pass.has_stencil, std::runtime_error, "VKFW::Graph::RenderGraph :: compile",
                                            "the pass '" + __pass.name + "' has more than one depth attachment");
                            __pass.depth       = __attachment;
                            __pass.has_depth   = VKFW::Detail::hasDepth  (format_(__res));
                            __pass.has_stencil = VKFW::Detail::hasStencil(format_(__res));
                        }
                    }
                    SVKFW_ASSERT(__pass.extent.width != 0u, std::runtime_error, "VKFW::Graph::RenderGraph :: compile",
                                    "the graphics pass '" + __pass.name + "' has no attachments");

                    // Note: render pass attachments are the colors, then the depth
                    for (uint32_t c = 0u; c < __pass.colors.size(); ++c)
                        __pass.clear_values.push_back(__pass.colors[c].clearValue);
                    if (__pass.has_depth || __pass.has_stencil)
                        __pass.clear_values.push_back(__pass.depth.clearValue);
                }
            }

            // Note: one subpass; initial and final layouts are the attachment layouts, transitions are done by the barriers outside
            void createRenderPasses_() {
                for (uint32_t p = 0u; p < passes.size(); ++p) {
                    Pass &__pass = passes[p];
                    if (!__pass.live || __pass.type != VKFW_PASS_GRAPHICS || __pass.render_pass != VK_NULL_HANDLE) continue;

                    std::vector<VkAttachmentDescription> __attachments;
                    std::vector<VkAttachmentReference>   __color_refs;
                    VkAttachmentReference                __depth_ref{VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED};
                    for (uint32_t a = 0u; a < __pass.accesses.size(); ++a) {
                        const Access &__access = __pass.accesses[a];
                        if (__access.type != VKFW_ACCESS_COLOR) continue;
                        __color_refs.push_back(VkAttachmentReference{(uint32_t)__attachments.size(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
                        __attachments.push_back(attachmentDescription_(__access, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
                    }
                    for (uint32_t a = 0u; a < __pass.accesses.size(); ++a) {
                        const Access &__access = __pass.accesses[a];
                        if (__access.type != VKFW_ACCESS_DEPTH) continue;
                        __depth_ref = VkAttachmentReference{(uint32_t)__attachments.size(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
                        __attachments.push_back(attachmentDescription_(__access, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
                    }

                    VkSubpassDescription __subpass{};
                    __subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    __subpass.colorAttachmentCount    = __color_refs.size();
                    __subpass.pColorAttachments       = __color_refs.data();
                    __subpass.pDepthStencilAttachment = __depth_ref.attachment == VK_ATTACHMENT_UNUSED ? nullptr : &__depth_ref;

                    VkRenderPassCreateInfo __info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
                    __info.attachmentCount = __attachments.size();
                    __info.pAttachments    = __attachments.data();
                    __info.subpassCount    = 1u;
                    __info.pSubpasses      = &__subpass;
                    __pass.render_pass = Func::CreateRenderPass(cxt.cxt_core.device.handle, &__info, cxt.cxt_core.alloc_cb.get());
                }
            }
            VkAttachmentDescription attachmentDescription_(const Access &_access, VkImageLayout _layout) const {
                const Resource &__res    = resources[_access.res.index];
                const VkFormat  __format = format_(__res);
                VkAttachmentDescription __desc{};
                __desc.format         = __format;
                __desc.samples        = __res.kind == VKFW_RESOURCE_SWAPCHAIN ? VK_SAMPLE_COUNT_1_BIT : __res.desc.samples;
                __desc.loadOp         = _access.load_op;
                __desc.storeOp        = _access.store_op;
                __desc.stencilLoadOp  = VKFW::Detail::hasStencil(__format) ? _access.load_op  : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                __desc.stencilStoreOp = VKFW::Detail::hasStencil(__format) ? _access.store_op : VK_ATTACHMENT_STORE_OP_DONT_CARE;
                __desc.initialLayout  = _layout;
                __desc.finalLayout    = _layout;
                return __desc;
            }

            void createFramebuffers_() {
                for (uint32_t p = 0u; p < passes.size(); ++p) {
                    Pass &__pass = passes[p];
                    if (!__pass.live || __pass.type != VKFW_PASS_GRAPHICS) continue;

                    std::vector<VkImageView> __views;
                    for (uint32_t c = 0u; c < __pass.colors.size(); ++c) __views.push_back(__pass.colors[c].imageView);
                    if (__pass.has_depth || __pass.has_stencil)            __views.push_back(__pass.depth.imageView);

                    const uint32_t __count = __pass.swapchain_color != UINT32_MAX ? cxt.cxt_swapchain.getSwapchainImageCount() : 1u;
                    for (uint32_t i = 0u; i < __count; ++i) {
                        if (__pass.swapchain_color != UINT32_MAX) __views[__pass.swapchain_color] = cxt.cxt_swapchain.imageView(i);
                        VkFramebufferCreateInfo __info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
                        __info.renderPass      = __pass.render_pass;
                        __info.attachmentCount = __views.size();
                        __info.pAttachments    = __views.data();
                        __info.width           = __pass.extent.width;
                        __info.height          = __pass.extent.height;
                        __info.layers          = 1u;
                        __pass.framebuffers.push_back(Func::CreateFramebuffer(cxt.cxt_core.device.handle, &__info, cxt.cxt_core.alloc_cb.get()));
                    }
                }
            }
            void destroyFramebuffers_() {
                for (uint32_t p = 0u; p < passes.size(); ++p) {
                    for (uint32_t i = 0u; i < passes[p].framebuffers.size(); ++i)
                        Func::DestroyFramebuffer(cxt.cxt_core.device.handle, passes[p].framebuffers[i], cxt.cxt_core.alloc_cb.get());
                    passes[p].framebuffers.clear();
                }
            }

            RenderGraph(const RenderGraph &);
            RenderGraph& operator=(const RenderGraph &);
        }; // RenderGraph END


        inline PassBuilder& PassBuilder::color(ResourceId _res, VkAttachmentLoadOp _load_op, const VkClearValue &_clear_value, VkAttachmentStoreOp _store_op) {
            graph->addAccess_(id, _res, VKFW_ACCESS_COLOR, _load_op, _store_op, _clear_value, 0u); return *this;
        }
        inline PassBuilder& PassBuilder::depth(ResourceId _res, VkAttachmentLoadOp _load_op, const VkClearValue &_clear_value, VkAttachmentStoreOp _store_op) {
            graph->addAccess_(id, _res, VKFW_ACCESS_DEPTH, _load_op, _store_op, _clear_value, 0u); return *this;
        }
        inline PassBuilder& PassBuilder::sampled(ResourceId _res, VkPipelineStageFlags2 _stages) {
            graph->addAccess_(id, _res, VKFW_ACCESS_SAMPLED, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VkClearValue(), _stages); return *this;
        }
        inline PassBuilder& PassBuilder::storageRead(ResourceId _res, VkPipelineStageFlags2 _stages) {
            graph->addAccess_(id, _res, VKFW_ACCESS_STORAGE_READ, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VkClearValue(), _stages); return *this;
        }
        inline PassBuilder& PassBuilder::storageWrite(ResourceId _res, VkPipelineStageFlags2 _stages) {
            graph->addAccess_(id, _res, VKFW_ACCESS_STORAGE_WRITE, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VkClearValue(), _stages); return *this;
        }
        inline PassBuilder& PassBuilder::transferSrc(ResourceId _res) {
            graph->addAccess_(id, _res, VKFW_ACCESS_TRANSFER_SRC, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VkClearValue(), VK_PIPELINE_STAGE_2_TRANSFER_BIT); return *this;
        }
        inline PassBuilder& PassBuilder::transferDst(ResourceId _res) {
            graph->addAccess_(id, _res, VKFW_ACCESS_TRANSFER_DST, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VkClearValue(), VK_PIPELINE_STAGE_2_TRANSFER_BIT); return *this;
        }
        inline PassBuilder& PassBuilder::keep() {
            graph->pass_(id).keep = true; graph->compiled = false; return *this;
        }
        inline PassBuilder& PassBuilder::execute(const std::function<void(CommandRecorder&)> &_callback) {
            graph->pass_(id).callback = _callback; return *this;
        }

        }; // Graph END
    }; // VKFW END
}; // Simple END

#endif // SVKFW_VKFW_GRAPH_H
