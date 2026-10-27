#include "interface/vkfw_graph.h"

namespace Simple {
    namespace VKFW {
        namespace Graph {

            ResourceId::ResourceId() {}

            ResourceId::ResourceId(uint32_t _index) : index{_index} {}

            bool ResourceId::isNone() const { return index == UINT32_MAX; }

            PassId::PassId() {}

            PassId::PassId(uint32_t _index) : index{_index} {}

            bool PassId::isNone() const { return index == UINT32_MAX; }

            TextureDesc::TextureDesc() {}

            TextureDesc::TextureDesc(VkFormat _format, float _scale) : format{_format}, scale{_scale} {}

            TextureDesc::TextureDesc(VkFormat _format, VkExtent2D _extent) : format{_format}, extent(_extent) {}

            namespace Detail {

                AccessInfo accessInfo(AccessType _type, VkFormat _format, VkPipelineStageFlags2 _shader_stages, VkAttachmentLoadOp _load_op) {
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


            PassBuilder::PassBuilder(RenderGraph *_graph, PassId _id) : graph{_graph}, id(_id) {}

            PassBuilder::operator PassId() const { return id; }

            RenderGraph::RenderGraph(VulkanContext &_cxt) : cxt(_cxt) {}

            RenderGraph::~RenderGraph() { destroy(); }

            ResourceId RenderGraph::createTexture(const std::string &_name, const TextureDesc &_desc) {
                Resource __res;
                __res.name = _name;
                __res.kind = VKFW_RESOURCE_TRANSIENT;
                __res.desc = _desc;
                return addResource_(__res);
            }

            ResourceId RenderGraph::importSwapchain() {
                Resource __res;
                __res.name   = "swapchain";
                __res.kind   = VKFW_RESOURCE_SWAPCHAIN;
                __res.output = true;
                return addResource_(__res);
            }

            ResourceId RenderGraph::importImage(const std::string &_name, VkImage _image, VkImageView _view, VkFormat _format, VkExtent2D _extent,
                                   VkImageLayout _initial_layout, VkImageLayout _final_layout, bool _output) {
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

            void RenderGraph::setOutput(ResourceId _res, bool _output) { resource_(_res).output = _output; }

            PassBuilder RenderGraph::addGraphicsPass(const std::string &_name) { return addPass_(_name, VKFW_PASS_GRAPHICS); }

            PassBuilder RenderGraph::addComputePass (const std::string &_name) { return addPass_(_name, VKFW_PASS_COMPUTE ); }

            PassBuilder RenderGraph::addTransferPass(const std::string &_name) { return addPass_(_name, VKFW_PASS_TRANSFER); }

            PassBuilder RenderGraph::pass(PassId _pass) { pass_(_pass); return PassBuilder(this, _pass); }

            VkImageView RenderGraph::view(ResourceId _res) const {
                const Resource &__res = resources.at(_res.index);
                SVKFW_ASSERT(__res.kind != VKFW_RESOURCE_SWAPCHAIN, std::invalid_argument, "VKFW::Graph::RenderGraph :: view", "the swapchain view changes every frame");
                return __res.view;
            }

            VkImage RenderGraph::image(ResourceId _res) const {
                const Resource &__res = resources.at(_res.index);
                SVKFW_ASSERT(__res.kind != VKFW_RESOURCE_SWAPCHAIN, std::invalid_argument, "VKFW::Graph::RenderGraph :: image", "the swapchain image changes every frame");
                return __res.image;
            }

            VkRenderPass RenderGraph::renderPass(PassId _pass) const {
                SVKFW_ASSERT(compiled, std::logic_error, "VKFW::Graph::RenderGraph :: renderPass", "the graph is not compiled");
                return passes.at(_pass.index).render_pass;
            }

            bool RenderGraph::isLive(PassId _pass) const { return passes.at(_pass.index).live; }

            void RenderGraph::compile() {
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

            void RenderGraph::execute(CommandRecorder &_rec, uint32_t _image_index) {
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

            void RenderGraph::destroy() {
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

            RenderGraph::Resource &RenderGraph::resource_(ResourceId _res) {
                SVKFW_ASSERT(_res.index < resources.size(), std::out_of_range, "VKFW::Graph::RenderGraph", "unknown resource");
                return resources[_res.index];
            }

            RenderGraph::Pass &RenderGraph::pass_(PassId _pass) {
                SVKFW_ASSERT(_pass.index < passes.size(), std::out_of_range, "VKFW::Graph::RenderGraph", "unknown pass");
                return passes[_pass.index];
            }

            void RenderGraph::addAccess_(PassId _pass, ResourceId _res, AccessType _type, VkAttachmentLoadOp _load_op, VkAttachmentStoreOp _store_op,
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

            ResourceId RenderGraph::addResource_(const Resource &_res) {
                resources.push_back(_res);
                compiled = false;
                return ResourceId(resources.size() - 1u);
            }

            PassBuilder RenderGraph::addPass_(const std::string &_name, PassType _type) {
                Pass __pass;
                __pass.name = _name;
                __pass.type = _type;
                passes.push_back(__pass);
                compiled = false;
                return PassBuilder(this, PassId(passes.size() - 1u));
            }

            VkFormat RenderGraph::format_(const Resource &_res) const {
                return _res.kind == VKFW_RESOURCE_SWAPCHAIN ? cxt.cxt_swapchain.format() : _res.desc.format;
            }

            VkExtent2D RenderGraph::extent_(const Resource &_res) const {
                return _res.kind == VKFW_RESOURCE_SWAPCHAIN ? cxt.cxt_swapchain.extent() : _res.extent;
            }

            bool RenderGraph::discards_(const Access &_access) {
                return (_access.type == VKFW_ACCESS_COLOR || _access.type == VKFW_ACCESS_DEPTH) && _access.load_op != VK_ATTACHMENT_LOAD_OP_LOAD;
            }

            void RenderGraph::cull_() {
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

            void RenderGraph::createTextures_() {
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

            void RenderGraph::destroyTexture_(Resource &_res) {
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

            void RenderGraph::access_(uint32_t _r, State &_state, VkPipelineStageFlags2 _stages, VkAccessFlags2 _access, VkImageLayout _layout, bool _write, bool _emit) {
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

            void RenderGraph::track_(std::vector<State> &_states, bool _emit) {
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

            void RenderGraph::setupRendering_() {
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

            void RenderGraph::createRenderPasses_() {
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

            VkAttachmentDescription RenderGraph::attachmentDescription_(const Access &_access, VkImageLayout _layout) const {
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

            void RenderGraph::createFramebuffers_() {
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

            void RenderGraph::destroyFramebuffers_() {
                for (uint32_t p = 0u; p < passes.size(); ++p) {
                    for (uint32_t i = 0u; i < passes[p].framebuffers.size(); ++i)
                        Func::DestroyFramebuffer(cxt.cxt_core.device.handle, passes[p].framebuffers[i], cxt.cxt_core.alloc_cb.get());
                    passes[p].framebuffers.clear();
                }
            }

            PassBuilder& PassBuilder::color(ResourceId _res, VkAttachmentLoadOp _load_op, const VkClearValue &_clear_value, VkAttachmentStoreOp _store_op) {
                graph->addAccess_(id, _res, VKFW_ACCESS_COLOR, _load_op, _store_op, _clear_value, 0u); return *this;
            }

            PassBuilder& PassBuilder::depth(ResourceId _res, VkAttachmentLoadOp _load_op, const VkClearValue &_clear_value, VkAttachmentStoreOp _store_op) {
                graph->addAccess_(id, _res, VKFW_ACCESS_DEPTH, _load_op, _store_op, _clear_value, 0u); return *this;
            }

            PassBuilder& PassBuilder::sampled(ResourceId _res, VkPipelineStageFlags2 _stages) {
                graph->addAccess_(id, _res, VKFW_ACCESS_SAMPLED, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VkClearValue(), _stages); return *this;
            }

            PassBuilder& PassBuilder::storageRead(ResourceId _res, VkPipelineStageFlags2 _stages) {
                graph->addAccess_(id, _res, VKFW_ACCESS_STORAGE_READ, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VkClearValue(), _stages); return *this;
            }

            PassBuilder& PassBuilder::storageWrite(ResourceId _res, VkPipelineStageFlags2 _stages) {
                graph->addAccess_(id, _res, VKFW_ACCESS_STORAGE_WRITE, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VkClearValue(), _stages); return *this;
            }

            PassBuilder& PassBuilder::transferSrc(ResourceId _res) {
                graph->addAccess_(id, _res, VKFW_ACCESS_TRANSFER_SRC, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VkClearValue(), VK_PIPELINE_STAGE_2_TRANSFER_BIT); return *this;
            }

            PassBuilder& PassBuilder::transferDst(ResourceId _res) {
                graph->addAccess_(id, _res, VKFW_ACCESS_TRANSFER_DST, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VkClearValue(), VK_PIPELINE_STAGE_2_TRANSFER_BIT); return *this;
            }

            PassBuilder& PassBuilder::keep() {
                graph->pass_(id).keep = true; graph->compiled = false; return *this;
            }

            PassBuilder& PassBuilder::execute(const std::function<void(CommandRecorder&)> &_callback) {
                graph->pass_(id).callback = _callback; return *this;
            }

            TextureInfo::TextureInfo() {}

            TextureInfo::TextureInfo(VkFormat _format, VkExtent2D _extent, uint32_t _mip_levels, Span<QueueFamilyId> _queue_families)
                : format{_format}, extent{_extent.width, _extent.height, 1u}, mip_levels{_mip_levels},
                  queue_families(_queue_families.data(), _queue_families.data() + _queue_families.size()) {}

            uint32_t mipLevelCount(VkExtent3D _extent) {
                uint32_t __size = std::max(_extent.width, std::max(_extent.height, _extent.depth)), __levels = 1u;
                while (__size > 1u) { __size >>= 1u; ++__levels; }
                return __levels;
            }

            Texture::Texture() {}

            bool Texture::isNone() const { return cxt == nullptr || image_id.isNone(); }

            VkImage Texture::image() const { return cxt->get(image_id); }

            VkImageView Texture::view(uint32_t _index) const {
                SVKFW_ASSERT(_index < views.size(), std::out_of_range, "VKFW::Graph::Texture :: view", "view index is out of range");
                return cxt->get(views[_index]);
            }

            const VkImageCreateInfo &Texture::info() const { return cxt->cxt_memory.image(image_id).create_info.peek(); }

            VkExtent3D Texture::extent(uint32_t _mip_level) const {
                const VkExtent3D &__extent = info().extent;
                return { std::max(__extent.width  >> _mip_level, 1u),
                         std::max(__extent.height >> _mip_level, 1u),
                         std::max(__extent.depth  >> _mip_level, 1u) };
            }

            uint32_t Texture::addView(uint32_t _base_mip, uint32_t _mip_count, uint32_t _base_layer, uint32_t _layer_count,
                                      VkImageViewType _type, VkImageAspectFlags _aspect, VkFormat _format, const VkComponentMapping &_components) {
                const VkImageCreateInfo &__image = info();
                SVKFW_ASSERT(_base_mip < __image.mipLevels && _base_layer < __image.arrayLayers, std::out_of_range,
                                "VKFW::Graph::Texture :: addView", "the first mip level or layer is out of the image");
                if (_mip_count   == VK_REMAINING_MIP_LEVELS  ) _mip_count   = __image.mipLevels   - _base_mip;
                if (_layer_count == VK_REMAINING_ARRAY_LAYERS) _layer_count = __image.arrayLayers - _base_layer;

                VulkanContext::ContextMemory &__memory = cxt->cxt_memory;
                const ImageViewId __view_id = __memory.addChunkImageView(image_id.parent);
                // Note: fills the type and the aspect from the image, then the range is narrowed
                __memory.setWholeImageViewCreateInfoStruct(__view_id, image_id, _aspect, _type);
                Struct::StructWrap<VkImageViewCreateInfo> &__view = __memory.imageView(__view_id).create_info;
                if (_type == VK_IMAGE_VIEW_TYPE_MAX_ENUM && _layer_count == 1u) {
                    // One layer of an array: a non-array view
                    if (__view.peek().viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY || __view.peek().viewType == VK_IMAGE_VIEW_TYPE_CUBE ||
                        __view.peek().viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) __view.edit().viewType = VK_IMAGE_VIEW_TYPE_2D;
                    if (__view.peek().viewType == VK_IMAGE_VIEW_TYPE_1D_ARRAY) __view.edit().viewType = VK_IMAGE_VIEW_TYPE_1D;
                }
                if (_format != VK_FORMAT_UNDEFINED) __view.edit().format = _format;
                __view.setComponents(_components.r, _components.g, _components.b, _components.a);
                __view.setRange(__view.peek().subresourceRange.aspectMask, _base_mip, _mip_count, _base_layer, _layer_count);

                views.push_back(__view_id);
                if (__memory.chunk(image_id.parent).memory.handle != VK_NULL_HANDLE)
                    __memory.createChunkImageView(image_id, __view_id);
                return views.size() - 1u;
            }

            void Texture::createViews() {
                VulkanContext::ContextMemory &__memory = cxt->cxt_memory;
                SVKFW_ASSERT(__memory.chunk(image_id.parent).memory.handle != VK_NULL_HANDLE, std::logic_error,
                                "VKFW::Graph::Texture :: createViews", "the memory chunk of the texture is not allocated yet");
                for (uint32_t i = 0u; i < views.size(); ++i)
                    if (__memory.imageView(views[i]).handle == VK_NULL_HANDLE)
                        __memory.createChunkImageView(image_id, views[i]);
            }

            void Texture::destroy() {
                if (isNone()) return;
                VulkanContext::ContextMemory &__memory = cxt->cxt_memory;
                if (own_memory) __memory.removeMemoryChunk(image_id.parent);
                else {
                    // Note: the memory stays with the chunk; the Ids of the views and the image stay in it (destroyed objects)
                    VulkanContext::ContextMemory::MemoryChunk &__chunk = __memory.chunk(image_id.parent);
                    for (uint32_t i = 0u; i < views.size(); ++i) __chunk.destroyImageView(views[i].index);
                    __chunk.destroyImage(image_id.index);
                }
                views.clear();
                image_id = ImageId();
            }

            Texture createTexture(VulkanContext &_cxt, const TextureInfo &_info, MemoryId _memory_id) {
                Texture __texture;
                __texture.cxt        = &_cxt;
                __texture.own_memory = _memory_id.isNone();

                VulkanContext::ContextMemory &__memory = _cxt.cxt_memory;
                const MemoryId __memory_id = __texture.own_memory ? __memory.addMemoryChunk(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) : _memory_id;
                SVKFW_ASSERT(__memory.chunk(__memory_id).memory.handle == VK_NULL_HANDLE, std::logic_error, "VKFW::Graph :: createTexture",
                                "the memory chunk is already allocated, a new image can't be placed in it");

                const uint32_t __mip_levels = _info.mip_levels == 0u ? mipLevelCount(_info.extent) : _info.mip_levels;
                __texture.image_id = __memory.addChunkImage(__memory_id);
                __memory.setWholeImageCreateInfoStruct(__texture.image_id, _info.queue_families, _info.sharing_mode, _info.usage, _info.format,
                                                       _info.extent, __mip_levels, _info.array_layers, _info.samples, _info.tiling, _info.flags);
                __memory.createChunkImage(__texture.image_id);
                if (__texture.own_memory) __memory.allocateChunkMemory(__memory_id);

                __texture.addView(0u, VK_REMAINING_MIP_LEVELS, 0u, VK_REMAINING_ARRAY_LAYERS, _info.view_type, _info.aspect, VK_FORMAT_UNDEFINED, _info.components);
                return __texture;
            }

            void generateMips(CommandRecorder &_rec, const Texture &_texture, uint32_t _mips_have, uint32_t _mips_req,
                              VkImageLayout _layout, VkImageLayout _final_layout) {
                const VkImageCreateInfo &__image  = _texture.info();
                const VkImage            __handle = _texture.image();
                if (_mips_req == 0u) _mips_req = __image.mipLevels;
                SVKFW_ASSERT(_mips_have > 0u, std::invalid_argument, "VKFW::Graph :: generateMips", "mip level 0 must hold the data ('_mips_have' > 0)");
                SVKFW_ASSERT(_mips_req <= __image.mipLevels, std::out_of_range, "VKFW::Graph :: generateMips",
                                "the image has only " + std::to_string(__image.mipLevels) + " mip levels, requested " + std::to_string(_mips_req));

                const VkImageAspectFlags    __aspect     = VKFW::Detail::formatAspects(__image.format);
                const VkPipelineStageFlags2 __all        = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                const VkAccessFlags2        __any_access = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
                // Levels ['_first', '_first' + '_count') of all layers
                struct Range {
                    static VkImageMemoryBarrier2 barrier(VkImageMemoryBarrier2 _barrier, uint32_t _first, uint32_t _count) {
                        _barrier.subresourceRange.baseMipLevel = _first;
                        _barrier.subresourceRange.levelCount   = _count;
                        return _barrier;
                    }
                };

                if (_mips_have >= _mips_req) {
                    if (_layout != _final_layout)
                        _rec.barrier(Range::barrier(imageBarrier(__handle, _layout, _final_layout, __all, VK_ACCESS_2_MEMORY_WRITE_BIT, __all, __any_access, __aspect),
                                                    0u, _mips_req));
                    return;
                }

                const VkFormatFeatureFlags __need = VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
                const VkFormatProperties   __props = Func::GetPhysicalDeviceFormatProperties(_rec.cxt.cxt_core.phys_device.handle, __image.format);
                const VkFormatFeatureFlags __have  = __image.tiling == VK_IMAGE_TILING_LINEAR ? __props.linearTilingFeatures : __props.optimalTilingFeatures;
                SVKFW_ASSERT((__have & __need) == __need, std::runtime_error, "VKFW::Graph :: generateMips",
                                "the image format doesn't support linear filtered blits, format " + std::to_string(__image.format));
                SVKFW_ASSERT((__image.usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) && (__image.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT), std::invalid_argument,
                                "VKFW::Graph :: generateMips", "the image needs TRANSFER_SRC and TRANSFER_DST usage");

                // The last filled level becomes the first source, the new levels become destinations (old contents discarded)
                {
                    VkImageMemoryBarrier2 __barriers[2] = {
                        Range::barrier(imageBarrier(__handle, _layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, __all, VK_ACCESS_2_MEMORY_WRITE_BIT,
                                                    VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, __aspect), _mips_have - 1u, 1u),
                        Range::barrier(imageBarrier(__handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, __all, VK_ACCESS_2_MEMORY_WRITE_BIT,
                                                    VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, __aspect), _mips_have, _mips_req - _mips_have)
                    };
                    _rec.barrier(__barriers);
                }

                // Each level from the previous one; a written level becomes the source of the next blit
                for (uint32_t __level = _mips_have; __level < _mips_req; ++__level) {
                    const VkExtent3D __src = _texture.extent(__level - 1u), __dst = _texture.extent(__level);
                    VkImageBlit __blit{};
                    __blit.srcSubresource = { __aspect, __level - 1u, 0u, __image.arrayLayers };
                    __blit.dstSubresource = { __aspect, __level,      0u, __image.arrayLayers };
                    __blit.srcOffsets[1]  = { (int32_t)__src.width, (int32_t)__src.height, (int32_t)__src.depth };
                    __blit.dstOffsets[1]  = { (int32_t)__dst.width, (int32_t)__dst.height, (int32_t)__dst.depth };
                    _rec.blitImage(_texture.image_id, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _texture.image_id, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   __blit, VK_FILTER_LINEAR);

                    if (__level + 1u < _mips_req)
                        _rec.barrier(Range::barrier(imageBarrier(__handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                                 VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                                                 VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, __aspect), __level, 1u));
                }

                // Final layout: the untouched levels, the sources and the last level
                std::vector<VkImageMemoryBarrier2> __final;
                if (_mips_have > 1u && _layout != _final_layout)
                    __final.push_back(Range::barrier(imageBarrier(__handle, _layout, _final_layout, __all, VK_ACCESS_2_MEMORY_WRITE_BIT,
                                                                  __all, __any_access, __aspect), 0u, _mips_have - 1u));
                __final.push_back(Range::barrier(imageBarrier(__handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _final_layout,
                                                              VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
                                                              __all, __any_access, __aspect), _mips_have - 1u, _mips_req - _mips_have));
                __final.push_back(Range::barrier(imageBarrier(__handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _final_layout,
                                                              VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                                              __all, __any_access, __aspect), _mips_req - 1u, 1u));
                _rec.barrier(__final);
            }

            ResourceId RenderGraph::importTexture(const std::string &_name, const Texture &_texture, VkImageLayout _layout) {
                const VkExtent3D __extent = _texture.extent();
                return importImage(_name, _texture.image(), _texture.view(), _texture.info().format, VkExtent2D{__extent.width, __extent.height},
                                   _layout, _layout, false);
            }
        }; // Graph END
    }; // VKFW END
}; // Simple END
