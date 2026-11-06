#include "scene/scene_markers.h"
#include "scene/scene_graph.h"

#include <cmath>


namespace Simple {
    namespace SceneGraph {
        namespace Graph = VKFW::Graph;

        void LookAtMarker::create(VKFW::VulkanContext &_cxt, const Graph::RenderGraph &_graph, Graph::PassId _pass,
                                  const std::string &_vertex_shader, const std::string &_fragment_shader) {
            cxt = &_cxt;
            // Two unit quads crossing along the Y axis
            const Vertex __vertices[] = { {{-0.5f, -0.5f,  0.0f}, {1.0f, 0.0f, 0.0f}}, {{ 0.5f, -0.5f,  0.0f}, {0.0f, 1.0f, 0.0f}},
                                          {{ 0.5f,  0.5f,  0.0f}, {0.0f, 0.0f, 1.0f}}, {{-0.5f,  0.5f,  0.0f}, {1.0f, 1.0f, 1.0f}},
                                          {{ 0.0f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}}, {{ 0.0f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}},
                                          {{ 0.0f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}}, {{ 0.0f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}} };
            const uint32_t __indices[] = { 0, 1, 2, 2, 3, 0,
                                           4, 5, 6, 6, 7, 4 };
            const VkMemoryPropertyFlags __host = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            vertex_buffer = Graph::createBuffer(_cxt, sizeof(__vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, __host);
            index_buffer  = Graph::createBuffer(_cxt, sizeof(__indices),  VK_BUFFER_USAGE_INDEX_BUFFER_BIT,  __host);
            vertex_buffer.write(__vertices, sizeof(__vertices));
            index_buffer.write(__indices, sizeof(__indices));

            const VkPushConstantRange __push = { VK_SHADER_STAGE_VERTEX_BIT, 0u, (uint32_t)sizeof(PushConstants) };
            layout          = Graph::createPipelineLayout(_cxt, {}, VKFW::Span<VkPushConstantRange>(&__push, 1u));
            vertex_shader   = Graph::loadShader(_cxt, _vertex_shader);
            fragment_shader = Graph::loadShader(_cxt, _fragment_shader);

            Graph::GraphicsPipelineDesc __desc;
            __desc.shader(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader).shader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader);
            __desc.layout = layout;
            __desc.vertexBinding(0u, sizeof(Vertex));
            __desc.vertexAttribute(0u, 0u, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos));
            __desc.vertexAttribute(1u, 0u, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));

            // Hidden parts: behind the depth of the scene, blended, nothing written but the color
            __desc.depth_compare = VK_COMPARE_OP_GREATER;
            __desc.depth_write   = false;
            __desc.blend         = Graph::VKFW_BLEND_ALPHA;
            hidden_pipeline = Graph::createGraphicsPipeline(_cxt, _graph, _pass, __desc);

            // Visible parts: opaque with depth, the stencil gets the reference of each quad
            __desc.depth_compare = VK_COMPARE_OP_LESS;
            __desc.depth_write   = true;
            __desc.blend         = Graph::VKFW_BLEND_NONE;
            __desc.stencil(VKFW::Util::stencilOpState(VK_COMPARE_OP_ALWAYS, VK_STENCIL_OP_REPLACE));
            __desc.dynamic_states.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
            visible_pipeline = Graph::createGraphicsPipeline(_cxt, _graph, _pass, __desc);
        }

        void LookAtMarker::record(VKFW::CommandRecorder &_rec, const vec3 &_point, const mat4 &_view, const mat4 &_proj) const {
            if (!cxt) return;
            PushConstants __pc;
            __pc.mvp = scaling({size, size, size}) & translation(_point) & _view & _proj;

            _rec.bindVertexBuffers(vertex_buffer.buffer_id);
            _rec.bindIndexBuffer(index_buffer.buffer_id);

            _rec.bindPipeline(hidden_pipeline);
            __pc.tint = {1.f, 1.f, 1.f, hidden_alpha};
            _rec.pushConstants(layout, VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(__pc), &__pc);
            _rec.drawIndexed(12u, 1u, 0, 0u);

            _rec.bindPipeline(visible_pipeline);
            __pc.tint = {1.f, 1.f, 1.f, 1.f};
            _rec.pushConstants(layout, VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(__pc), &__pc);
            _rec.setStencilReference(VK_STENCIL_FACE_FRONT_AND_BACK, 1u);
            _rec.drawIndexed(6u, 1u, 0, 0u);
            _rec.setStencilReference(VK_STENCIL_FACE_FRONT_AND_BACK, 2u);
            _rec.drawIndexed(6u, 1u, 0, 6u);
        }

        void LookAtMarker::destroy() {
            if (!cxt) return;
            if (!visible_pipeline.isNone()) { cxt->cxt_pipeline.removePipeline(visible_pipeline); visible_pipeline = VKFW::PipelineId(); }
            if (!hidden_pipeline.isNone())  { cxt->cxt_pipeline.removePipeline(hidden_pipeline);  hidden_pipeline  = VKFW::PipelineId(); }
            if (!vertex_shader.isNone())    { cxt->cxt_pipeline.removeShader(vertex_shader);     vertex_shader    = VKFW::ShaderModuleId(); }
            if (!fragment_shader.isNone())  { cxt->cxt_pipeline.removeShader(fragment_shader);   fragment_shader  = VKFW::ShaderModuleId(); }
            if (!layout.isNone())           { cxt->cxt_pipeline.removeLayout(layout);            layout           = VKFW::PipelineLayoutId(); }
            vertex_buffer.destroy();
            index_buffer.destroy();
            cxt = nullptr;
        }
    }; // SceneGraph END
}; // Simple END
