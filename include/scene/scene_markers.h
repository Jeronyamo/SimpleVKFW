#ifndef SVKFW_SCENE_MARKERS_H
#define SVKFW_SCENE_MARKERS_H

#include "interface/vkfw.h"
#include "interface/vkfw_graph.h"
#include "math/linalg.h"


namespace Simple {
    namespace SceneGraph {

//  == === ==== ================================== ==== === ==  \\
                    Markers
//  == === ==== ================================== ==== === ==  \\

        // Note: the point the camera looks at, as two vertex-colored quads crossing along the Y axis, 'size' world units wide
        //       (it gets smaller with the distance like the scene). Drawn in two parts: where the scene hides it, faintly ('hidden_alpha',
        //       no depth write); where it is visible, opaque with depth (the quads sort each other) and the stencil reference
        //       1 / 2 for the first / second quad (visible in the stencil screenshot).
        //
        //       LookAtMarker marker;
        //       marker.create(vk, graph, scene_pass);                 // after 'graph.compile()'
        //       graph.pass(scene_pass).execute([&](CommandRecorder &rec) {
        //           ...
        //           if (show) marker.record(rec, look_at, view, proj);
        //       });
        //       ...
        //       marker.destroy();                                     // the device must be idle
        struct LookAtMarker {
            struct Vertex {
                vec3 pos;
                vec3 color;
            };
            // Note: matches 'Marker' in 'shaders/marker.vert'
            struct PushConstants {
                mat4 mvp;
                vec4 tint;
            };

            VKFW::VulkanContext *cxt = nullptr;
            float size = 2.f;            // world units (the quads are 'size' x 'size')
            float hidden_alpha = 0.25f;  // the parts behind the scene

            VKFW::Graph::Buffer     vertex_buffer, index_buffer; // HOST_VISIBLE (a few vertices)
            VKFW::PipelineLayoutId  layout;
            VKFW::ShaderModuleId    vertex_shader, fragment_shader;
            VKFW::PipelineId        visible_pipeline, hidden_pipeline;

            // Creates the buffers, the shaders and the pipelines for drawing in '_pass' (the graph must be compiled)
            void create(VKFW::VulkanContext &_cxt, const VKFW::Graph::RenderGraph &_graph, VKFW::Graph::PassId _pass,
                        const std::string &_vertex_shader = "shaders/marker_vert.spv", const std::string &_fragment_shader = "shaders/marker_frag.spv");
            // Records the marker at '_point' (inside the rendering of the pass of 'create'); '_view' and '_proj' are the camera
            // matrices (row-vector style)
            void record(VKFW::CommandRecorder &_rec, const vec3 &_point, const mat4 &_view, const mat4 &_proj) const;
            // Note: the device must not use the resources
            void destroy();
        }; // LookAtMarker END
    }; // SceneGraph END
}; // Simple END

#endif // SVKFW_SCENE_MARKERS_H
