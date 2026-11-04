#ifndef SVKFW_SCENE_RENDERER_H
#define SVKFW_SCENE_RENDERER_H

#include <map>
#include <string>
#include <vector>

#include "interface/vkfw.h"
#include "interface/vkfw_graph.h"
#include "scene/scene_graph.h"


namespace Simple {
    namespace SceneGraph {
        namespace Graph = VKFW::Graph;
        using VKFW::VulkanContext;
        using VKFW::CommandRecorder;
        using VKFW::QueueId;
        using VKFW::QueueFamilyId;
        using VKFW::MemoryId;
        using VKFW::SamplerId;
        using VKFW::DescriptorSetLayoutId;
        using VKFW::DescriptorPoolId;
        using VKFW::DescriptorSetsId;
        using VKFW::PipelineLayoutId;
        using VKFW::ShaderModuleId;
        using VKFW::PipelineId;

//  == === ==== ================================== ==== === ==  \\
                    Scene renderer
//  == === ==== ================================== ==== === ==  \\

        // Note: draws a 'Scene' in a graphics pass of the render graph (forward Blinn-Phong, up to 'MAX_LIGHTS' lights).
        //       GPU data: all meshes in one vertex and one index buffer, all textures in one memory chunk (with mip levels),
        //       one descriptor set per material (parameters + base color, normal and emissive textures), one uniform buffer per
        //       frame in flight (camera and lights); the node matrix goes in push constants.
        //       Skinned meshes: the bones, weights and skin mode in a second vertex buffer, the joints of all skins (matrix for
        //       LBS, dual quaternion for DQS) in one storage buffer per frame in flight; a draw finds the joints of its skin by
        //       'gl_InstanceIndex' (the first instance).
        //       Draw order: opaque and masked meshes first, then blended ones back to front by their node (the meshes of one
        //       node keep their order, like MMD draws the materials of a model in the file order).
        //
        //       SceneRenderer renderer(vk);
        //       renderer.upload(scene, info);                         // after the device and the queues are created
        //       renderer.createPipelines(graph, scene_pass);          // after 'graph.compile()'
        //       graph.pass(scene_pass).execute([&](CommandRecorder &rec) { renderer.record(rec); });
        //       ...
        //       scene.updateTransforms();                             // if nodes moved
        //       scene.updateSkins();                                  // if poses changed
        //       renderer.update(frame_in_flight, view, proj);         // each frame, after the fence of the frame is waited
        //       ...
        //       renderer.destroy();                                   // the device must be idle
        struct SceneRenderer {
            static const uint32_t MAX_LIGHTS = 16u; // must match 'shaders/scene.frag'

            struct UploadInfo {
                QueueId       queue;               // a graphics queue: the copies and the mip level blits go there
                QueueFamilyId family;              // the family of 'queue'; the resources are EXCLUSIVE to it
                uint32_t      frames_in_flight = 2u;
                std::string   vertex_shader   = "shaders/scene_vert.spv";
                std::string   skinned_vertex_shader = "shaders/scene_skin_vert.spv";
                std::string   fragment_shader = "shaders/scene_frag.spv";
                float         max_anisotropy  = 16.f;
                VkDeviceSize  staging_size    = 32u * 1024u * 1024u; // grown to fit the largest texture
            };

            // Note: matches 'Material' in the shaders (std140)
            struct MaterialGPU {
                vec4 base_color;
                vec4 specular_shininess;    // rgb, exponent
                vec4 emissive_normal_scale; // rgb, normal map strength
                vec4 alpha;                 // cutoff (-1: opaque), has normal map, two-sided, unused
            };
            struct LightGPU {
                vec4 position_range;        // world position, range (0: unlimited)
                vec4 direction_type;        // world direction (where the light goes), type (LightType)
                vec4 color_intensity;
                vec4 cone;                  // cos(inner), cos(outer), unused
            };
            struct FrameGPU {
                mat4     view, proj;
                vec4     camera_position;
                vec4     ambient;
                uint32_t light_count[4];
                LightGPU lights[MAX_LIGHTS];
            };
            // Note: matches 'Joint' in 'shaders/scene.vert' (std430): the skinning matrix (LBS) and dual quaternion (DQS, xyzw)
            struct JointGPU {
                mat4 matrix;
                vec4 real, dual;
            };
            struct PushConstants {
                mat4 model;
                mat4 normal; // inverse transpose of 'model' (upper 3x3 used)
            };

            struct MeshRange {
                uint32_t first_index = 0u, index_count = 0u;
                int32_t  vertex_offset = 0;
                uint32_t skin_offset = SCENE_NONE; // first vertex in 'skin_buffer', SCENE_NONE: not skinned
            };
            struct Draw {
                uint32_t node, mesh;
            };

            VulkanContext &cxt;
            const Scene   *scene = nullptr;

            // Options (read by 'createPipelines')
            // Note: with the engine camera (left-handed world, see 'scene_graph.h') front faces end up clockwise on the screen
            VkFrontFace front_face = VK_FRONT_FACE_CLOCKWISE;
            bool        backface_culling = true; // for materials that aren't two-sided

            // GPU data
            Graph::Buffer               vertex_buffer, index_buffer, material_buffer;
            std::vector<Graph::Buffer>  frame_buffers;  // per frame in flight, HOST_VISIBLE
            Graph::Buffer               skin_buffer;    // 'SkinVertex' of the skinned meshes
            std::vector<Graph::Buffer>  joint_buffers;  // per frame in flight, HOST_VISIBLE: the skinning matrices of all skins
            std::vector<uint32_t>       joint_offsets;  // per skin: its first joint in the joint buffers
            std::vector<JointGPU>       joint_data;     // 'update' scratch
            VkDeviceSize                material_stride = 0u;
            MemoryId                    texture_memory;
            std::vector<Graph::Texture> textures;       // per scene texture, then the defaults: white, flat normal
            uint32_t                    white_texture = 0u, flat_normal_texture = 0u;
            std::vector<MeshRange>      mesh_ranges;
            SamplerId                   sampler;

            DescriptorSetLayoutId       frame_layout, material_layout;
            DescriptorPoolId            descriptor_pool;
            DescriptorSetsId            frame_sets, material_sets; // the last material set is the default material
            PipelineLayoutId            pipeline_layout;
            ShaderModuleId              vertex_shader, skinned_vertex_shader, fragment_shader;
            std::map<uint32_t, PipelineId> pipelines;            // see 'pipelineKey_'

            // Per frame ('update')
            uint32_t          frame = 0u;
            std::vector<Draw> opaque_draws, blend_draws;
            uint32_t          draw_count = 0u; // drawn in the last 'record'

            explicit SceneRenderer(VulkanContext &_cxt);
           ~SceneRenderer();

            // Copies the meshes, materials and textures of '_scene' to the device and creates the descriptors; waits for the copies.
            // The scene must stay alive: 'update' reads its transforms, poses, lights and visibility every frame.
            // Note: meshes, materials, textures and skins added later need another 'upload' (after 'destroy')
            void upload(const Scene &_scene, const UploadInfo &_info);

            // Creates the pipelines for drawing in '_pass' (the graph must be compiled; with VkRenderPass the pipelines are tied to the
            // render pass of the pass, which the graph keeps between compilations)
            void createPipelines(const Graph::RenderGraph &_graph, Graph::PassId _pass);

            // Writes the camera and the lights of frame '_frame' and builds the draw lists (call after the fence of the frame is waited).
            // '_view' and '_proj' are the camera matrices (row-vector style, like 'Camera::getView/getProj')
            void update(uint32_t _frame, const mat4 &_view, const mat4 &_proj);

            // Records the draws of the frame of the last 'update' (inside the rendering of the pass of 'createPipelines')
            void record(CommandRecorder &_rec);

            // Note: the device must not use the resources
            void destroy();

        private:
            static uint32_t pipelineKey_(bool _blend, bool _cull, bool _depth_write, bool _skinned);
            PipelineId pipeline_(const Material *_material, bool _skinned) const;
            // The skin used to draw the mesh of '_draw', SCENE_NONE if it is drawn rigidly
            uint32_t skin_(const Draw &_draw) const;
            const Material *material_(uint32_t _mesh) const;
            void draw_(CommandRecorder &_rec, const Draw &_draw, PipelineId &_bound_pipeline, uint32_t &_bound_material, bool &_offset_bound);

            SceneRenderer(const SceneRenderer &);
            SceneRenderer& operator=(const SceneRenderer &);
        }; // SceneRenderer END

    }; // SceneGraph END
}; // Simple END

#endif // SVKFW_SCENE_RENDERER_H
