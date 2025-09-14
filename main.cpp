#include "interface/vkfw.h"
#include "main/event_handler.h"
#include "math/linalg.h"
#include "main/camera.h"
#include "main/input_handler.h"

#include "main/image.h"

#include <iostream>


int main(int argc, char **argv) {
    using Simple::VKFW::ContextIndex;

// Config
    // return Simple::test_csound();
    const uint32_t FRAMES_IN_FLIGHT = 2u;

// Scene info

    // Geometry
    struct Vertex {
        Simple::vec2 pos;
        Simple::vec3 color;
    };
    const std::vector<Vertex> vertices = { {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                           {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                           {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
                                           {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}
    };
    const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

    // Camera
    struct UniformBufferObjectCamera {
        Simple::mat4 view, proj;
    } ubo_camera;

    struct UniformBufferObjectModel { // TODO: for instancing
        std::vector<Simple::mat4> model;
    };

    const Simple::vec2u fb_size = {1024u, 1024u};
    const float fov = 33.f, p_near = 0.1f, p_far = 1000.f;
    const Simple::vec3 cam_pos = {-2.f,2.f,-2.f}, look_at = {0.f,0.f,0.f}, up_vect = {0.f,1.f,0.f};

    Simple::WindowInput::CameraControlsCentered main_view_centered_proj{Simple::ProjPerspective{fb_size.cast<float>(), fov, p_near, p_far},
                                                                        Simple::ViewCentered{cam_pos, look_at, up_vect}};

    ubo_camera.view = main_view_centered_proj.getView();
    ubo_camera.proj = main_view_centered_proj.getProj();


// Input handlers

    Simple::WindowInput::FrameInputHandlerGLFW main_window_handler;

    main_window_handler.setAllKeysHandler(&main_view_centered_proj);
    main_window_handler.setScrollHandler(&main_view_centered_proj);
    main_window_handler.setCursorHandler(&main_view_centered_proj);
    main_window_handler.setFBufferSizeHandler(&main_view_centered_proj);

    // Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // for vk only, use context for OpenGL
    Simple::Window main_window{fb_size.x, fb_size.y, "Test"};

    Simple::WindowInput::DefaultHandler main_window_exit{main_window.window};

    // Callbacks
    main_window_handler.setKeysHandler(&main_window_exit);
    main_window_handler.setGLFWCallbacks(main_window.window, Simple::WindowInput::INPUT_TYPE_GLFW_SCROLL |
                                                             Simple::WindowInput::INPUT_TYPE_GLFW_CURSOR |
                                                             Simple::WindowInput::INPUT_TYPE_GLFW_FBUFFER_SIZE |
                                                             Simple::WindowInput::INPUT_TYPE_GLFW_KEY);

// VKFW

    Simple::VKFW::initContext();

    // Instance
    Simple::VKFW::Vulkan_global_context_factory.b_instance.setApplicationInfo("Testapp", { 0u, 0u, 1u }, "SVKFW", { 0u, 0u, 1u });
    Simple::VKFW::Vulkan_global_context_factory.b_instance.addGLFWExtensions();
    Simple::VKFW::Vulkan_global_context_factory.b_instance.addDebugUtilsExtension();
    Simple::VKFW::Vulkan_global_context_factory.b_instance.addValLayers({{ "VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor" }});
    Simple::VKFW::Vulkan_global_context_factory.createInstance();

    // Debug messenger
    Simple::VKFW::Vulkan_global_context_factory.createMessenger(); // CreateInfo struct is too big to be set via single call

    // Surface
    Simple::VKFW::Vulkan_global_context_factory.createSurface(main_window);

    // Physical device
    Simple::VKFW::Vulkan_global_context_factory.pickPhysicalDevice(Simple::VKFW::PhysicalDevice::Suitability::defaultGraphicsTest);

    // Queue family
    // 'ci' - context index
    ContextIndex ci_qfamily = Simple::VKFW::Vulkan_global_context_factory.pickQueueFamily(Simple::VKFW::QueueFamily::Suitability::testSurfAndGraphics);
    ContextIndex ci_qfamily_transfer = Simple::VKFW::Vulkan_global_context_factory.pickQueueFamily(Simple::VKFW::QueueFamily::Suitability::rankTransfer);
    if (ci_qfamily == ci_qfamily_transfer)
        throw std::runtime_error(SVKFW_WRAPERR("main", "could not find a better suited transfer qfamily than graphics"));

    // Logical device
    const uint32_t QUEUES_COUNT = 2u;
    const uint32_t QUEUES_COUNT_TRANSFER = 1u;
    Simple::VKFW::Vulkan_global_context_factory.bDeviceAddQueueInfo(ci_qfamily, QUEUES_COUNT);
    Simple::VKFW::Vulkan_global_context_factory.bDeviceAddQueueInfo(ci_qfamily_transfer, QUEUES_COUNT_TRANSFER);
    {
        VkPhysicalDeviceFeatures device_features{};
        device_features.logicOp = VK_TRUE;
        Simple::VKFW::Vulkan_global_context_factory.b_device.setRequiredFeatures(device_features);
    }
    Simple::VKFW::Vulkan_global_context_factory.b_device.addExtensions({{ VK_KHR_SWAPCHAIN_EXTENSION_NAME }});
    Simple::VKFW::Vulkan_global_context_factory.bDeviceIncludeValLayersLegacy();
    Simple::VKFW::Vulkan_global_context_factory.createDevice();

    // Queue
    ContextIndex ci_queue_graphics = Simple::VKFW::Vulkan_global_context_factory.addFamilyQueues(ci_qfamily, QUEUES_COUNT);
    ContextIndex ci_queue_pres = ci_queue_graphics + 1;
    ContextIndex ci_queue_transfer = Simple::VKFW::Vulkan_global_context_factory.addFamilyQueues(ci_qfamily_transfer, 1);

    // Swapchain
    Simple::VKFW::Vulkan_global_context_factory.cSwapchainSetCapabilities();
    Simple::VKFW::Vulkan_global_context_factory.c_swapchain.b_swapchain.checkSetImageCount(2);
    Simple::VKFW::Vulkan_global_context_factory.c_swapchain.b_swapchain.checkSetImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    Simple::VKFW::Vulkan_global_context_factory.c_swapchain.b_swapchain.chooseCurrentExtent(main_window);
    Simple::VKFW::Vulkan_global_context_factory.c_swapchain.b_swapchain.setCurrentTransform();
    Simple::VKFW::Vulkan_global_context_factory.cSwapchainPickFormat(Simple::VKFW::SwapchainKHR::Suitability::defaultFormatTest);
    Simple::VKFW::Vulkan_global_context_factory.cSwapchainSetPresentationMode(VK_PRESENT_MODE_FIFO_KHR);
    Simple::VKFW::Vulkan_global_context_factory.createSwapchain();

    // Swapchain images
    Simple::VKFW::Vulkan_global_context_factory.c_swapchain.setImageViewSwapchainFormat();
    Simple::VKFW::Vulkan_global_context_factory.c_swapchain.b_image_view.setViewType(VK_IMAGE_VIEW_TYPE_2D);
    Simple::VKFW::Vulkan_global_context_factory.c_swapchain.b_image_view.setRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    Simple::VKFW::Vulkan_global_context_factory.c_swapchain.b_image_view.setComponents();
    Simple::VKFW::Vulkan_global_context_factory.createSwapchainImageViews();

    // Render pass
    {
        // Attachment description
        Simple::VKFW::AttachmentDescription::StructWrap attachment;
        attachment.setFormatAndSize(
            Simple::VKFW::Vulkan_global_context_factory.cSwapchainGetImageInfo().format,
            VK_SAMPLE_COUNT_1_BIT);
        attachment.setColorOps  (VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
        attachment.setStencilOps(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE);
        attachment.setLayouts(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        // Subpass description
        Simple::VKFW::SubpassDescription::StructBuilder subpass_builder;
        subpass_builder.setBindingPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
        // Note: 'pColorBlendState->attachmentCount' must be == 'pSubpasses[0].colorAttachmentCount'
        subpass_builder.setColorAttachments({ { 0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } });

        // Subpass dependency
        Simple::VKFW::SubpassDependency::StructWrap sp_dependency{};
        sp_dependency.setSubpasses(VK_SUBPASS_EXTERNAL, 0u);
        sp_dependency.setSrcMasks(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_NONE);
        sp_dependency.setDstMasks(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

        // Render pass
        Simple::VKFW::Vulkan_global_context_factory.b_render_pass.setAttachments({ attachment.description }); // Note: this vector defines a number and order of VkClearValue-s in BeginRenderPass
        Simple::VKFW::Vulkan_global_context_factory.b_render_pass.setSubpasses({ subpass_builder.getVkStructDCopy() });
        Simple::VKFW::Vulkan_global_context_factory.b_render_pass.setDependencies({ sp_dependency.subpass_dependency });
        Simple::VKFW::Vulkan_global_context_factory.createRenderPass();
    }

    // Swapchain framebuffers
    Simple::VKFW::Vulkan_global_context_factory.c_swapchain.b_framebuffer.setLayers(1u);
    Simple::VKFW::Vulkan_global_context_factory.c_swapchain.setFramebufferSwapchainBufferSize();
    Simple::VKFW::Vulkan_global_context_factory.createSwapchainFramebuffers();

    // Buffers (uniform)
    ContextIndex ci_buffer_uniform[FRAMES_IN_FLIGHT];
    void* ubo_mappings[FRAMES_IN_FLIGHT];
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i) {
        ci_buffer_uniform[i] = Simple::VKFW::Vulkan_global_context_factory.createBuffer({}, VK_SHARING_MODE_EXCLUSIVE, sizeof(UniformBufferObjectCamera), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        Simple::VKFW::Vulkan_global_context_factory.createDeviceMemory(ci_buffer_uniform[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        ubo_mappings[i] = Simple::VKFW::Vulkan_global_context_func.mapMemory(ci_buffer_uniform[i], false);
    }

// Descriptors
    // Descriptor Layout
    // Note: only layout needs to be passed to pipeline, rest of Descriptors stuff can be created later
    Simple::VKFW::Vulkan_global_context_factory.c_descriptor.b_layout.setBindingsSize(1);
    Simple::VKFW::Vulkan_global_context_factory.c_descriptor.b_layout.setBinding(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    ContextIndex ci_ubo_desc_layout = Simple::VKFW::Vulkan_global_context_factory.createDescriptorSetLayout();

    // Descriptor Pool
    ContextIndex ci_ubo_desc_pool = Simple::VKFW::Vulkan_global_context_factory.createDescriptorPool(FRAMES_IN_FLIGHT,
                                        { VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = FRAMES_IN_FLIGHT }});

    // Descriptor Sets
    ContextIndex ci_ubo_desc_set[FRAMES_IN_FLIGHT]{};

    ci_ubo_desc_set[0] = Simple::VKFW::Vulkan_global_context_factory.createDescriptorSets(ci_ubo_desc_pool, { ci_ubo_desc_layout, ci_ubo_desc_layout });
    for (uint32_t i = 1u; i < FRAMES_IN_FLIGHT; ++i)
        ci_ubo_desc_set[i] = ci_ubo_desc_set[0] + i;

    // Update descriptor info
    ContextIndex ci_ubo_sct_write_set[FRAMES_IN_FLIGHT]{};

    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i) {
        ci_ubo_sct_write_set[i] = Simple::VKFW::Vulkan_global_context_func.b_update_descriptor_sets.addSetWrite();
        Simple::VKFW::Vulkan_global_context_func.bUpdateDescrSetsWriteSetInfo(ci_ubo_sct_write_set[i], ci_ubo_desc_set[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
        Simple::VKFW::Vulkan_global_context_func.bUpdateDescrSetsWriteAddBufferInfo(ci_ubo_sct_write_set[i], ci_buffer_uniform[i]);
    }
    Simple::VKFW::Vulkan_global_context_func.updateDescriptorSets();


// Graphics pipeline

    // Layout
    Simple::VKFW::Vulkan_global_context_factory.createPipelineGraphicsLayout({}, {ci_ubo_desc_layout});

    // Shaders
    ContextIndex ci_vert_shader = Simple::VKFW::Vulkan_global_context_factory.createShaderModuleFromFile("./shaders/simple_shader_vert.spv");
    ContextIndex ci_frag_shader = Simple::VKFW::Vulkan_global_context_factory.createShaderModuleFromFile("./shaders/simple_shader_frag.spv");

    // Shader stage
    Simple::VKFW::Vulkan_global_context_factory.cPipelineGraphicsAddShaderStage(ci_vert_shader, VK_SHADER_STAGE_VERTEX_BIT,   "main");
    Simple::VKFW::Vulkan_global_context_factory.cPipelineGraphicsAddShaderStage(ci_frag_shader, VK_SHADER_STAGE_FRAGMENT_BIT, "main");

    // Dynamic state
    Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_dynamic_state.setDynamicStates({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });

    // Vertex input state
    Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_vertex_input_state.setBindings({{ 0u, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX }});
    Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_vertex_input_state.setAttributes({ { 0, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex,   pos) },
                                                                                                                                    { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) } });

    // Input assembly state
    Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_input_assembly_state.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    // Viewport state
    // viewport_state_info.setFromSwapchainExtent(swapchain_extent); // set dynamically
    Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_viewport_state.setScissorsDynamic(1u);
    Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_viewport_state.setViewportsDynamic(1u);

    // Rasterization state
    Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_rasterization_state.defStruct();
    Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_rasterization_state.setPolygonMode(VK_POLYGON_MODE_FILL);
    Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_rasterization_state.setCullMode(VK_CULL_MODE_NONE);
    Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_rasterization_state.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
    Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_rasterization_state.setLineWidth(1.f);

    // Multisample state
    Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_multisample_state.defStruct();

    // Color blend state
    Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_color_blend_state.setLogicOp(VK_LOGIC_OP_COPY);
    { // TODO:
        Simple::VKFW::Pipeline::ColorBlendAttachmentState::StructWrap attachment_state{};
        attachment_state.confBlendOver();
        Simple::VKFW::Vulkan_global_context_factory.c_graphics_pipeline.b_color_blend_state.setAttachments({ attachment_state });
    }

    // Graphics pipeline
    Simple::VKFW::Vulkan_global_context_factory.createPipelineGraphics(0u, false);

    // Command pool
    ContextIndex ci_pool_drawing = Simple::VKFW::Vulkan_global_context_factory.createCommandPool(ci_qfamily, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    ContextIndex ci_pool_staging = Simple::VKFW::Vulkan_global_context_factory.createCommandPool(ci_qfamily_transfer, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);// | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

    // Buffers
    // Note: when binding memory to buffer, (memory - offset) >= required memory for buffer (.size from vkGetBufferMemoryRequirements)
    //       Basically, you bind *buffer* to device memory
    VkDeviceSize staging_buffer_size = std::max(vertices.size() * sizeof(Vertex), indices.size() * sizeof(uint32_t));
    ContextIndex ci_buffer_staging = Simple::VKFW::Vulkan_global_context_factory.createBuffer({ci_qfamily_transfer}, VK_SHARING_MODE_EXCLUSIVE, staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    ContextIndex ci_buffer_vertex  = Simple::VKFW::Vulkan_global_context_factory.createBuffer({ci_qfamily, ci_qfamily_transfer}, VK_SHARING_MODE_CONCURRENT, vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    ContextIndex ci_buffer_index   = Simple::VKFW::Vulkan_global_context_factory.createBuffer({ci_qfamily, ci_qfamily_transfer}, VK_SHARING_MODE_CONCURRENT, indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    // (createDeviceMemory) returns 'ci_device_memory', but it isn't actually needed, memory management does all the work
    Simple::VKFW::Vulkan_global_context_factory.createDeviceMemory(ci_buffer_staging, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    Simple::VKFW::Vulkan_global_context_factory.createDeviceMemory(ci_buffer_vertex, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    Simple::VKFW::Vulkan_global_context_factory.createDeviceMemory(ci_buffer_index, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Command buffer
    ContextIndex ci_cmd_buffer_drawing[FRAMES_IN_FLIGHT]{};

    ci_cmd_buffer_drawing[0] = Simple::VKFW::Vulkan_global_context_factory.createCommandBuffers(ci_pool_drawing, VK_COMMAND_BUFFER_LEVEL_PRIMARY, FRAMES_IN_FLIGHT);
    for (uint32_t i = 1u; i < FRAMES_IN_FLIGHT; ++i)
        ci_cmd_buffer_drawing[i] = ci_cmd_buffer_drawing[0] + i;

    ContextIndex ci_cmd_buffer_staging = Simple::VKFW::Vulkan_global_context_factory.createCommandBuffers(ci_pool_staging, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);


    // Sync objects
    ContextIndex ci_sem_image_available[FRAMES_IN_FLIGHT]{},
                 ci_sem_render_finished[FRAMES_IN_FLIGHT]{};

    // Two cycles, so that the arrays are contigious (or just make a function that creates an array of semaphores)
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i)
        ci_sem_image_available[i] = Simple::VKFW::Vulkan_global_context_factory.createSemaphore();
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i)
        ci_sem_render_finished[i] = Simple::VKFW::Vulkan_global_context_factory.createSemaphore();

    ContextIndex ci_fen_staging = 0u;
    ContextIndex ci_fen_in_flight[FRAMES_IN_FLIGHT]{};
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i)
        ci_fen_in_flight[i] = Simple::VKFW::Vulkan_global_context_factory.createFence(nullptr, VK_FENCE_CREATE_SIGNALED_BIT);
    ci_fen_staging = Simple::VKFW::Vulkan_global_context_factory.createFence();


    // Copy static vertex data
    Simple::VKFW::Vulkan_global_context_factory.cBufferMemoryFill(ci_buffer_staging, vertices.data(), vertices.size() * sizeof(Vertex));
    Simple::VKFW::Vulkan_global_context_func.beginCommandBuffer(ci_cmd_buffer_staging, 0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    Simple::VKFW::Vulkan_global_context_func.cmdCopyBuffer(ci_cmd_buffer_staging, ci_buffer_staging, ci_buffer_vertex);
    Simple::VKFW::Vulkan_global_context_func.endCommandBuffer(ci_cmd_buffer_staging);
    Simple::VKFW::Vulkan_global_context_func.queueSubmit(ci_queue_transfer, {}, {ci_cmd_buffer_staging}, {}, {}, ci_fen_staging);

    Simple::VKFW::Vulkan_global_context_func.waitForFences({ci_fen_staging});
    Simple::VKFW::Vulkan_global_context_func.resetFences({ci_fen_staging});

    // Copy static index data
    Simple::VKFW::Vulkan_global_context_factory.cBufferMemoryFill(ci_buffer_staging, indices.data(), indices.size() * sizeof(uint32_t));
    Simple::VKFW::Vulkan_global_context_func.beginCommandBuffer(ci_cmd_buffer_staging, 0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    Simple::VKFW::Vulkan_global_context_func.cmdCopyBuffer(ci_cmd_buffer_staging, ci_buffer_staging, ci_buffer_index);
    Simple::VKFW::Vulkan_global_context_func.endCommandBuffer(ci_cmd_buffer_staging);
    Simple::VKFW::Vulkan_global_context_func.queueSubmit(ci_queue_transfer, {}, {ci_cmd_buffer_staging}, {}, {}, ci_fen_staging);

    Simple::VKFW::Vulkan_global_context_func.waitForFences({ci_fen_staging});


// Main loop

    uint32_t current_frame = 0u, image_index = UINT32_MAX;
    // VkResult pres_res = VK_SUCCESS;
    // VkPresentInfoKHR pres_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    // pres_info.swapchainCount = 1;
    // pres_info.waitSemaphoreCount = 1;
    // pres_info.pImageIndices = &image_index;
    // pres_info.pResults = &pres_res;

    // Begin render pass create
    Simple::VKFW::Vulkan_global_context_func.b_begin_render_pass.setClearColors({ Simple::VKFW::ClearValue::build(0.f, 0.f, 0.f, 1.f) });

    while (!main_window.shouldClose()) {
        main_window_handler.frameTick();
        glfwPollEvents();
        main_view_centered_proj.manualFrameUpdate(main_window_handler.frame_time);

        Simple::VKFW::Vulkan_global_context_func.waitForFences({ci_fen_in_flight[current_frame]});

        // Note: image_index always matches vulkan (and swapchain) context's ci_framebuffer and ci_image_view
        image_index = Simple::VKFW::Vulkan_global_context_func.acquireNextImageKHR(ci_sem_image_available[current_frame]);
        if (image_index == UINT32_MAX) {
            Simple::VKFW::Vulkan_global_context_factory.cSwapchainRecreate(main_window);
            continue;
        }

        // Update uniform buffers
        ubo_camera.view = main_view_centered_proj.getView();
        ubo_camera.proj = main_view_centered_proj.getProj();
        memcpy(ubo_mappings[current_frame], &ubo_camera, sizeof(ubo_camera));
        // printf("View: [%f, %f, %f, %f], [%f, %f, %f, %f], [%f, %f, %f, %f], [%f, %f, %f, %f]\n", ubo_camera.view.M[0][0], ubo_camera.view.M[0][1], ubo_camera.view.M[0][2], ubo_camera.view.M[0][3],
        //                                                                                          ubo_camera.view.M[1][0], ubo_camera.view.M[1][1], ubo_camera.view.M[1][2], ubo_camera.view.M[1][3],
        //                                                                                          ubo_camera.view.M[2][0], ubo_camera.view.M[2][1], ubo_camera.view.M[2][2], ubo_camera.view.M[2][3],
        //                                                                                          ubo_camera.view.M[3][0], ubo_camera.view.M[3][1], ubo_camera.view.M[3][2], ubo_camera.view.M[3][3]);
        // printf("Proj: [%f, %f, %f, %f], [%f, %f, %f, %f], [%f, %f, %f, %f], [%f, %f, %f, %f]\n", ubo_camera.proj.M[0][0], ubo_camera.proj.M[0][1], ubo_camera.proj.M[0][2], ubo_camera.proj.M[0][3],
        //                                                                                          ubo_camera.proj.M[1][0], ubo_camera.proj.M[1][1], ubo_camera.proj.M[1][2], ubo_camera.proj.M[1][3],
        //                                                                                          ubo_camera.proj.M[2][0], ubo_camera.proj.M[2][1], ubo_camera.proj.M[2][2], ubo_camera.proj.M[2][3],
        //                                                                                          ubo_camera.proj.M[3][0], ubo_camera.proj.M[3][1], ubo_camera.proj.M[3][2], ubo_camera.proj.M[3][3]);

        Simple::VKFW::Vulkan_global_context_func.resetFences({ci_fen_in_flight[current_frame]});
        Simple::VKFW::Vulkan_global_context_func.resetCommandBuffer(ci_cmd_buffer_drawing[current_frame]);

        // Record command buffer
        {
            // Begin recording command buffer
            Simple::VKFW::Vulkan_global_context_func.beginCommandBuffer(ci_cmd_buffer_drawing[current_frame]);

            // Begin render pass
            Simple::VKFW::Vulkan_global_context_func.bCmdBeginRenderPassSetRenderAreaFull();
            Simple::VKFW::Vulkan_global_context_func.cmdBeginRenderPass(ci_cmd_buffer_drawing[current_frame], image_index, VK_SUBPASS_CONTENTS_INLINE);

            // Bind pipeline
            Simple::VKFW::Vulkan_global_context_func.cmdBindPipeline(ci_cmd_buffer_drawing[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS);

            // Bind vertex buffer
            Simple::VKFW::Vulkan_global_context_func.cmdBindVertexBuffers(ci_cmd_buffer_drawing[current_frame], 0, {ci_buffer_vertex});

            // Bind index buffer
            Simple::VKFW::Vulkan_global_context_func.cmdBindIndexBuffer(ci_cmd_buffer_drawing[current_frame], ci_buffer_index, VK_INDEX_TYPE_UINT32);

            // Bind descriptor set (uniform buffer)
            Simple::VKFW::Vulkan_global_context_func.cmdBindDescriptorSets(ci_cmd_buffer_drawing[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, 0, { ci_ubo_desc_set[current_frame] });

            // Set viewport
            Simple::VKFW::Vulkan_global_context_func.cmdSetViewport(ci_cmd_buffer_drawing[current_frame]);

            // Set scissors
            Simple::VKFW::Vulkan_global_context_func.cmdSetScissor(ci_cmd_buffer_drawing[current_frame]);

            // Draw
            // Simple::VKFW::Vulkan_global_context_func.cmdDraw(ci_cmd_buffer_drawing[current_frame], vertices.size(), 1);

            // Draw indexed
            Simple::VKFW::Vulkan_global_context_func.cmdDrawIndexed(ci_cmd_buffer_drawing[current_frame], indices.size(), 1);

            // End render pass
            Simple::VKFW::Vulkan_global_context_func.cmdEndRenderPass(ci_cmd_buffer_drawing[current_frame]);

            // End recording command buffer
            Simple::VKFW::Vulkan_global_context_func.endCommandBuffer(ci_cmd_buffer_drawing[current_frame]);
        }

        // Submit
        Simple::VKFW::Vulkan_global_context_func.queueSubmit(ci_queue_graphics, {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
                                                            {ci_cmd_buffer_drawing [current_frame]}, {ci_sem_image_available[current_frame]},
                                                            {ci_sem_render_finished[current_frame]},  ci_fen_in_flight[current_frame]);

        // Present
        image_index = Simple::VKFW::Vulkan_global_context_func.queuePresentKHR(ci_queue_pres, { ci_sem_render_finished[current_frame] }, { image_index });
        // image_index = Simple::VKFW::Vulkan_global_context_func.queuePresentKHR(ci_queue_pres, pres_info, ci_sem_render_finished[current_frame]);
        if (image_index == UINT32_MAX) {
            Simple::VKFW::Vulkan_global_context_factory.cSwapchainRecreate(main_window);
        }

        current_frame = (current_frame + 1) % FRAMES_IN_FLIGHT;
    } // main loop END

    for (uint32_t curr_frame = 0u; curr_frame < FRAMES_IN_FLIGHT; ++curr_frame) {
        Simple::VKFW::Vulkan_global_context_func.waitForFences({ci_fen_in_flight[curr_frame]});
        Simple::VKFW::Vulkan_global_context_func.resetFences({ci_fen_in_flight[curr_frame]});
        Simple::VKFW::Vulkan_global_context_func.resetCommandBuffer(ci_cmd_buffer_drawing[curr_frame]);
    }

    Simple::VKFW::destroyVulkanContext(Simple::VKFW::Vulkan_global_context);

    printf("Success\n");
    return 0;
}