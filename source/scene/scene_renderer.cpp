#include "scene/scene_renderer.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <stdexcept>


namespace Simple {
    namespace SceneGraph {
        using namespace VKFW;

        SceneRenderer::SceneRenderer(VulkanContext &_cxt) : cxt(_cxt) {}

        SceneRenderer::~SceneRenderer() {}

        uint32_t SceneRenderer::pipelineKey_(bool _blend, bool _cull, bool _depth_write) {
            return (_blend ? 1u : 0u) | (_cull ? 2u : 0u) | (_depth_write ? 4u : 0u);
        }

        const Material *SceneRenderer::material_(uint32_t _mesh) const {
            uint32_t __m = scene->meshes[_mesh].material;
            return __m == SCENE_NONE ? nullptr : &scene->materials[__m];
        }

        PipelineId SceneRenderer::pipeline_(const Material *_material) const {
            const bool __blend = _material && _material->alpha_mode == SCENE_ALPHA_BLEND;
            const bool __cull  = backface_culling && !(_material && _material->two_sided);
            const bool __depth_write = !__blend || _material->blend_depth_write;
            return pipelines.at(pipelineKey_(__blend, __cull, __depth_write));
        }


//  == === ==== >  Upload

        void SceneRenderer::upload(const Scene &_scene, const UploadInfo &_info) {
            SVKFW_ASSERT(mesh_ranges.empty() && textures.empty(), std::logic_error, "SceneGraph::SceneRenderer :: upload",
                            "already uploaded, call 'destroy' first");
            scene = &_scene;
            using Graph::createBuffer;

        // Geometry: one vertex buffer and one index buffer
            size_t __vertex_count = 0u, __index_count = 0u;
            mesh_ranges.resize(_scene.meshes.size());
            for (size_t i = 0u; i < _scene.meshes.size(); ++i) {
                mesh_ranges[i].vertex_offset = (int32_t)__vertex_count;
                mesh_ranges[i].first_index   = (uint32_t)__index_count;
                mesh_ranges[i].index_count   = (uint32_t)_scene.meshes[i].indices.size();
                __vertex_count += _scene.meshes[i].vertices.size();
                __index_count  += _scene.meshes[i].indices.size();
            }
            const VkBufferUsageFlags __dst = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            const QueueFamilyId __families[] = { _info.family };
            vertex_buffer = createBuffer(cxt, std::max<size_t>(__vertex_count, 1u) * sizeof(Vertex), __dst | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, __families);
            index_buffer  = createBuffer(cxt, std::max<size_t>(__index_count, 1u) * sizeof(uint32_t), __dst | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, __families);

        // Materials: one uniform block each (aligned for the descriptor offsets), the last one is the default material
            const VkDeviceSize __ubo_align = cxt.cxt_core.phys_device.properties().head().properties.limits.minUniformBufferOffsetAlignment;
            material_stride = Graph::alignUp(sizeof(MaterialGPU), __ubo_align);
            const uint32_t __material_count = (uint32_t)_scene.materials.size() + 1u;
            std::vector<uint8_t> __material_data((size_t)(material_stride * __material_count), 0u);
            for (uint32_t i = 0u; i < __material_count; ++i) {
                const Material __mat = i < _scene.materials.size() ? _scene.materials[i] : Material();
                MaterialGPU __gpu;
                __gpu.base_color            = __mat.base_color;
                __gpu.specular_shininess    = { __mat.specular.x, __mat.specular.y, __mat.specular.z, __mat.shininess };
                __gpu.emissive_normal_scale = { __mat.emissive.x, __mat.emissive.y, __mat.emissive.z, __mat.normal_scale };
                // Blended texels that are (almost) fully transparent are discarded, so they don't write depth
                float __cutoff = __mat.alpha_mode == SCENE_ALPHA_MASK ? __mat.alpha_cutoff : __mat.alpha_mode == SCENE_ALPHA_BLEND ? 1.f / 255.f : -1.f;
                __gpu.alpha = { __cutoff, __mat.normal_texture != SCENE_NONE ? 1.f : 0.f, __mat.two_sided ? 1.f : 0.f, 0.f };
                std::memcpy(&__material_data[(size_t)(material_stride * i)], &__gpu, sizeof(__gpu));
            }
            material_buffer = createBuffer(cxt, __material_data.size(), __dst | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, __families);

        // Frame uniform buffers
            for (uint32_t i = 0u; i < _info.frames_in_flight; ++i)
                frame_buffers.push_back(createBuffer(cxt, sizeof(FrameGPU), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, __families));

        // Textures: one memory chunk, all mip levels; then the defaults
            texture_memory = cxt.cxt_memory.addMemoryChunk(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VkDeviceSize __largest = 4u;
            for (size_t i = 0u; i < _scene.textures.size(); ++i) {
                const Texture &__tex = _scene.textures[i];
                VkExtent2D __extent = { (uint32_t)std::max(__tex.image.width, 1), (uint32_t)std::max(__tex.image.height, 1) };
                textures.push_back(Graph::createTexture(cxt, Graph::TextureInfo(__tex.srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM,
                                                                                 __extent, 0u, __families), texture_memory));
                __largest = std::max<VkDeviceSize>(__largest, (VkDeviceSize)__extent.width * __extent.height * 4u);
            }
            white_texture = (uint32_t)textures.size();
            textures.push_back(Graph::createTexture(cxt, Graph::TextureInfo(VK_FORMAT_R8G8B8A8_SRGB,  VkExtent2D{1u, 1u}, 1u, __families), texture_memory));
            flat_normal_texture = (uint32_t)textures.size();
            textures.push_back(Graph::createTexture(cxt, Graph::TextureInfo(VK_FORMAT_R8G8B8A8_UNORM, VkExtent2D{1u, 1u}, 1u, __families), texture_memory));
            cxt.cxt_memory.allocateChunkMemory(texture_memory);
            for (size_t i = 0u; i < textures.size(); ++i) textures[i].createViews();

        // Copies (one batch on the graphics queue, so no ownership transfer), then the mip levels
            Graph::Buffer __staging = createBuffer(cxt, std::max(_info.staging_size, __largest), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, __families);
            CommandPoolId    __pool = cxt.cxt_command.addCmdPool();
            CommandBuffersId __cmds = cxt.cxt_command.addCmdBuffer();
            cxt.cxt_command.createCmdPool(__pool, _info.family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
            cxt.cxt_command.createCmdBuffersPrimary(__cmds, __pool, 1u);
            const CommandBufferId __cmd(__cmds, 0u);
            {
                UploadBatch __upload(cxt, _info.queue, __cmd, __staging.buffer_id);
                for (size_t i = 0u; i < _scene.meshes.size(); ++i) {
                    const Mesh &__mesh = _scene.meshes[i];
                    if (!__mesh.vertices.empty())
                        __upload.copyToBuffer(vertex_buffer.buffer_id, __mesh.vertices.data(), __mesh.vertices.size() * sizeof(Vertex),
                                              (VkDeviceSize)mesh_ranges[i].vertex_offset * sizeof(Vertex));
                    if (!__mesh.indices.empty())
                        __upload.copyToBuffer(index_buffer.buffer_id, __mesh.indices.data(), __mesh.indices.size() * sizeof(uint32_t),
                                              (VkDeviceSize)mesh_ranges[i].first_index * sizeof(uint32_t));
                }
                __upload.copyToBuffer(material_buffer.buffer_id, __material_data.data(), __material_data.size());
                for (size_t i = 0u; i < _scene.textures.size(); ++i) {
                    const Img::Image4u &__img = _scene.textures[i].image;
                    if (__img.img.empty()) continue;
                    __upload.copyToImage(textures[i].image_id, __img.img.data(), __img.img.size() * 4u);
                }
                const uint8_t __white[4] = {255u, 255u, 255u, 255u}, __flat[4] = {128u, 128u, 255u, 255u};
                __upload.copyToImage(textures[white_texture].image_id, __white, 4u);
                __upload.copyToImage(textures[flat_normal_texture].image_id, __flat, 4u);
                __upload.submit();
                __upload.wait();
            }
            {
                CommandRecorder __rec(cxt, __cmd);
                __rec.reset();
                __rec.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
                for (size_t i = 0u; i < _scene.textures.size(); ++i)
                    Graph::generateMips(__rec, textures[i]);
                __rec.end();
                FenceId __fence = cxt.cxt_barrier.addFence(false);
                SubmitDesc __submit;
                __submit.commands(__cmd);
                cxt.submit(_info.queue, __submit, __fence);
                const FenceId __fences[] = { __fence };
                cxt.cxt_barrier.wait(__fences);
                cxt.cxt_barrier.destroyFence(__fence);
            }
            cxt.cxt_command.removeCmdBuffers(__cmds);
            cxt.cxt_command.removeCmdPool(__pool);
            __staging.destroy();

        // Sampler
            sampler = cxt.cxt_descriptor.addSampler();
            cxt.cxt_descriptor.sampler(sampler).create_info.setWholeStruct(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, _info.max_anisotropy);
            cxt.cxt_descriptor.createSampler(sampler);

        // Descriptors: set 0 = frame (camera, lights), set 1 = material (parameters, base color, normal, emissive)
            frame_layout = cxt.cxt_descriptor.addLayout();
            cxt.cxt_descriptor.layoutBinding(cxt.cxt_descriptor.addSctLayoutBinding(frame_layout))
                .setWholeStruct(0u, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1u, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
            cxt.cxt_descriptor.createLayout(frame_layout);

            material_layout = cxt.cxt_descriptor.addLayout();
            cxt.cxt_descriptor.layoutBinding(cxt.cxt_descriptor.addSctLayoutBinding(material_layout))
                .setWholeStruct(0u, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1u, VK_SHADER_STAGE_FRAGMENT_BIT);
            for (uint32_t b = 1u; b <= 3u; ++b)
                cxt.cxt_descriptor.layoutBinding(cxt.cxt_descriptor.addSctLayoutBinding(material_layout))
                    .setWholeStruct(b, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1u, VK_SHADER_STAGE_FRAGMENT_BIT);
            cxt.cxt_descriptor.createLayout(material_layout);

            descriptor_pool = cxt.cxt_descriptor.addPool();
            {
                Object::DescriptorPool &__pool_obj = cxt.cxt_descriptor.pool(descriptor_pool);
                __pool_obj.create_info.edit().maxSets = _info.frames_in_flight + __material_count;
                __pool_obj.create_info.edit().flags   = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // 'destroy' frees the sets
                __pool_obj.create_info.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _info.frames_in_flight + __material_count);
                __pool_obj.create_info.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3u * __material_count);
            }
            cxt.cxt_descriptor.createPool(descriptor_pool);

            frame_sets = cxt.cxt_descriptor.addSet();
            cxt.cxt_descriptor.setAllForSet(frame_sets, descriptor_pool, std::vector<DescriptorSetLayoutId>(_info.frames_in_flight, frame_layout));
            cxt.cxt_descriptor.createSets(frame_sets);
            material_sets = cxt.cxt_descriptor.addSet();
            cxt.cxt_descriptor.setAllForSet(material_sets, descriptor_pool, std::vector<DescriptorSetLayoutId>(__material_count, material_layout));
            cxt.cxt_descriptor.createSets(material_sets);

            std::vector<Struct::StructWrap<VkWriteDescriptorSet>> __writes;
            for (uint32_t i = 0u; i < _info.frames_in_flight; ++i) {
                __writes.push_back(Struct::StructWrap<VkWriteDescriptorSet>());
                __writes.back().addBufferInfo(frame_buffers[i].handle(), 0u, sizeof(FrameGPU));
                __writes.back().setScalars(cxt.get(DescriptorSetId(frame_sets, i)), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0u);
            }
            const VkSampler __sampler = cxt.get(sampler);
            for (uint32_t i = 0u; i < __material_count; ++i) {
                const Material __mat = i < _scene.materials.size() ? _scene.materials[i] : Material();
                const VkDescriptorSet __set = cxt.get(DescriptorSetId(material_sets, i));
                __writes.push_back(Struct::StructWrap<VkWriteDescriptorSet>());
                __writes.back().addBufferInfo(material_buffer.handle(), material_stride * i, sizeof(MaterialGPU));
                __writes.back().setScalars(__set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0u);
                const uint32_t __tex[3] = {
                    __mat.base_color_texture == SCENE_NONE ? white_texture       : __mat.base_color_texture,
                    __mat.normal_texture     == SCENE_NONE ? flat_normal_texture : __mat.normal_texture,
                    __mat.emissive_texture   == SCENE_NONE ? white_texture       : __mat.emissive_texture };
                for (uint32_t b = 0u; b < 3u; ++b) {
                    __writes.push_back(Struct::StructWrap<VkWriteDescriptorSet>());
                    __writes.back().addImageInfo(__sampler, textures[__tex[b]].view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                    __writes.back().setScalars(__set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, b + 1u);
                }
            }
            cxt.cxt_descriptor.updateSets(__writes);

        // Pipeline layout and shaders (the pipelines need the pass, see 'createPipelines')
            pipeline_layout = cxt.cxt_pipeline.addLayout();
            const DescriptorSetLayoutId __layouts[] = { frame_layout, material_layout };
            cxt.cxt_pipeline.setLayoutDescriptorSetLayouts(pipeline_layout, __layouts);
            cxt.cxt_pipeline.addLayoutPushConstant(pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(PushConstants));
            cxt.cxt_pipeline.createLayout(pipeline_layout);
            vertex_shader   = Graph::loadShader(cxt, _info.vertex_shader);
            fragment_shader = Graph::loadShader(cxt, _info.fragment_shader);
        }

        void SceneRenderer::createPipelines(const Graph::RenderGraph &_graph, Graph::PassId _pass) {
            SVKFW_ASSERT(!pipeline_layout.isNone(), std::logic_error, "SceneGraph::SceneRenderer :: createPipelines", "call 'upload' first");
            for (std::map<uint32_t, PipelineId>::iterator it = pipelines.begin(); it != pipelines.end(); ++it)
                cxt.cxt_pipeline.removePipeline(it->second);
            pipelines.clear();

            Graph::GraphicsPipelineDesc __desc;
            __desc.layout = pipeline_layout;
            __desc.shader(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader).shader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader);
            __desc.vertexBinding(0u, sizeof(Vertex))
                  .vertexAttribute(0u, 0u, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(Vertex, position))
                  .vertexAttribute(1u, 0u, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(Vertex, normal))
                  .vertexAttribute(2u, 0u, VK_FORMAT_R32G32_SFLOAT,       offsetof(Vertex, uv))
                  .vertexAttribute(3u, 0u, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, tangent));
            __desc.front_face    = front_face;
            __desc.depth_compare = VK_COMPARE_OP_LESS_OR_EQUAL;
            // All variants: opaque / blended, culled / two-sided, with / without depth writes (blended only)
            for (uint32_t __key = 0u; __key < 8u; ++__key) {
                const bool __blend = (__key & 1u) != 0u, __cull = (__key & 2u) != 0u, __depth_write = (__key & 4u) != 0u;
                if (!__blend && !__depth_write) continue; // opaque always writes depth
                __desc.blend       = __blend ? Graph::VKFW_BLEND_ALPHA : Graph::VKFW_BLEND_NONE;
                __desc.cull_mode   = __cull ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
                __desc.depth_write = __depth_write;
                pipelines[__key] = Graph::createGraphicsPipeline(cxt, _graph, _pass, __desc);
            }
        }


//  == === ==== >  Frame

        void SceneRenderer::update(uint32_t _frame, const mat4 &_view, const mat4 &_proj) {
            SVKFW_ASSERT(scene && _frame < frame_buffers.size(), std::out_of_range, "SceneGraph::SceneRenderer :: update",
                            "no scene or no frame " + std::to_string(_frame));
            SVKFW_ASSERT(scene->world.size() == scene->nodes.size(), std::logic_error, "SceneGraph::SceneRenderer :: update",
                            "call 'Scene::updateTransforms' after changing the nodes");
            frame = _frame;
            const Scene &__scene = *scene;

        // Camera and lights
            FrameGPU __gpu;
            std::memset(&__gpu, 0, sizeof(__gpu));
            __gpu.view = _view;
            __gpu.proj = _proj;
            const mat4 __inv_view = _view.inverse<float>();
            __gpu.camera_position = { __inv_view.M[3][0], __inv_view.M[3][1], __inv_view.M[3][2], 1.f };
            __gpu.ambient = { __scene.ambient.x, __scene.ambient.y, __scene.ambient.z, 0.f };
            uint32_t __count = 0u;
            for (size_t i = 0u; i < __scene.lights.size() && __count < MAX_LIGHTS; ++i) {
                const Light &__light = __scene.lights[i];
                if (__light.node == SCENE_NONE || !__scene.world_visible[__light.node] || __light.intensity <= 0.f) continue;
                const mat4 &__world = __scene.world[__light.node];
                const vec3 __pos = transformPoint(__world, {0.f, 0.f, 0.f});
                const vec3 __dir = transformDir(__world, {0.f, 0.f, 1.f}).normalize();
                LightGPU &__l = __gpu.lights[__count++];
                __l.position_range  = { __pos.x, __pos.y, __pos.z, __light.range };
                __l.direction_type  = { __dir.x, __dir.y, __dir.z, (float)__light.type };
                __l.color_intensity = { __light.color.x, __light.color.y, __light.color.z, __light.intensity };
                __l.cone            = { std::cos(__light.inner_cone), std::cos(__light.outer_cone), 0.f, 0.f };
            }
            __gpu.light_count[0] = __count;
            frame_buffers[_frame].write(&__gpu, sizeof(__gpu));

        // Draw lists
            opaque_draws.clear();
            blend_draws.clear();
            struct Sorted { float depth; uint32_t order; Draw draw; };
            std::vector<Sorted> __blended;
            for (uint32_t n = 0u; n < (uint32_t)__scene.nodes.size(); ++n) {
                const Node &__node = __scene.nodes[n];
                if (__node.meshes.empty() || !__scene.world_visible[n]) continue;
                // Note: blended meshes are sorted by the center of their node, so the meshes of one model keep the file order
                AABB __bounds;
                for (size_t m = 0u; m < __node.meshes.size(); ++m) __bounds.add(__scene.meshes[__node.meshes[m]].bounds);
                const vec3 __center = transformPoint(_view, transformPoint(__scene.world[n], __bounds.center()));
                for (size_t m = 0u; m < __node.meshes.size(); ++m) {
                    const uint32_t __mesh = __node.meshes[m];
                    const Material *__mat = material_(__mesh);
                    if ((__mat && !__mat->visible) || mesh_ranges[__mesh].index_count == 0u) continue;
                    Draw __draw = { n, __mesh };
                    if (__mat && __mat->alpha_mode == SCENE_ALPHA_BLEND) {
                        Sorted __s = { __center.z, (uint32_t)__blended.size(), __draw };
                        __blended.push_back(__s);
                    } else opaque_draws.push_back(__draw);
                }
            }
            // Note: the camera looks along -Z in the view space ('View::ViewMat'), so the farthest has the smallest z
            struct Order {
                bool operator()(const Sorted &_a, const Sorted &_b) const {
                    return _a.depth != _b.depth ? _a.depth < _b.depth : _a.order < _b.order;
                }
            };
            std::sort(__blended.begin(), __blended.end(), Order());
            for (size_t i = 0u; i < __blended.size(); ++i) blend_draws.push_back(__blended[i].draw);
        }

        void SceneRenderer::draw_(CommandRecorder &_rec, const Draw &_draw, PipelineId &_bound_pipeline, uint32_t &_bound_material) {
            const Material *__mat = material_(_draw.mesh);
            const PipelineId __pipeline = pipeline_(__mat);
            if (_bound_pipeline.isNone() || cxt.get(__pipeline) != cxt.get(_bound_pipeline)) {
                _rec.bindPipeline(__pipeline);
                _bound_pipeline = __pipeline;
            }
            uint32_t __material = scene->meshes[_draw.mesh].material;
            if (__material == SCENE_NONE) __material = (uint32_t)scene->materials.size(); // the default material
            if (__material != _bound_material) {
                const DescriptorSetId __set[] = { DescriptorSetId(material_sets, __material) };
                _rec.bindDescriptorSets(pipeline_layout, __set, {}, VK_PIPELINE_BIND_POINT_GRAPHICS, 1u);
                _bound_material = __material;
            }
            PushConstants __push;
            __push.model  = scene->world[_draw.node];
            __push.normal = __push.model.inverse<float>().T();
            _rec.pushConstants(pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(__push), &__push);
            const MeshRange &__range = mesh_ranges[_draw.mesh];
            _rec.drawIndexed(__range.index_count, 1u, __range.vertex_offset, __range.first_index);
            ++draw_count;
        }

        void SceneRenderer::record(CommandRecorder &_rec) {
            draw_count = 0u;
            if (!scene || pipelines.empty()) return;
            const BufferId __vertex[] = { vertex_buffer.buffer_id };
            _rec.bindVertexBuffers(__vertex);
            _rec.bindIndexBuffer(index_buffer.buffer_id);
            const DescriptorSetId __frame_set[] = { DescriptorSetId(frame_sets, frame) };
            _rec.bindDescriptorSets(pipeline_layout, __frame_set);

            PipelineId __bound_pipeline;
            uint32_t   __bound_material = UINT32_MAX;
            for (size_t i = 0u; i < opaque_draws.size(); ++i) draw_(_rec, opaque_draws[i], __bound_pipeline, __bound_material);
            for (size_t i = 0u; i <  blend_draws.size(); ++i) draw_(_rec,  blend_draws[i], __bound_pipeline, __bound_material);
        }

        void SceneRenderer::destroy() {
            for (std::map<uint32_t, PipelineId>::iterator it = pipelines.begin(); it != pipelines.end(); ++it)
                cxt.cxt_pipeline.removePipeline(it->second);
            pipelines.clear();
            if (!vertex_shader.isNone())   { cxt.cxt_pipeline.removeShader(vertex_shader);   vertex_shader   = ShaderModuleId(); }
            if (!fragment_shader.isNone()) { cxt.cxt_pipeline.removeShader(fragment_shader); fragment_shader = ShaderModuleId(); }
            if (!pipeline_layout.isNone()) { cxt.cxt_pipeline.removeLayout(pipeline_layout); pipeline_layout = PipelineLayoutId(); }

            if (!material_sets.isNone())   { cxt.cxt_descriptor.removeSets(material_sets);   material_sets = DescriptorSetsId(); }
            if (!frame_sets.isNone())      { cxt.cxt_descriptor.removeSets(frame_sets);      frame_sets    = DescriptorSetsId(); }
            if (!descriptor_pool.isNone()) { cxt.cxt_descriptor.removePool(descriptor_pool); descriptor_pool = DescriptorPoolId(); }
            if (!material_layout.isNone()) { cxt.cxt_descriptor.removeLayout(material_layout); material_layout = DescriptorSetLayoutId(); }
            if (!frame_layout.isNone())    { cxt.cxt_descriptor.removeLayout(frame_layout);    frame_layout    = DescriptorSetLayoutId(); }
            if (!sampler.isNone())         { cxt.cxt_descriptor.removeSampler(sampler);        sampler         = SamplerId(); }

            textures.clear(); // the views and images go with their chunk
            if (!texture_memory.isNone())  { cxt.cxt_memory.removeMemoryChunk(texture_memory); texture_memory = MemoryId(); }
            for (size_t i = 0u; i < frame_buffers.size(); ++i) frame_buffers[i].destroy();
            frame_buffers.clear();
            vertex_buffer.destroy();
            index_buffer.destroy();
            material_buffer.destroy();
            mesh_ranges.clear();
            opaque_draws.clear();
            blend_draws.clear();
            scene = nullptr;
        }

    }; // SceneGraph END
}; // Simple END
