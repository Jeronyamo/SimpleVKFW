#include "main/event_handler.h"
#include "math/linalg.h"
#include "main/camera.h"
#include "main/input_handler.h"
#include "main/image.h"
#include "main/imgui_events.h"
#include "interface/vkfw.h"
#include "interface/vkfw_graph.h"
#include "interface/imgui_wrap.h"

#include <iostream>


int main(int argc, char **argv) {
    Simple::ArgHandler arg_handler{argc, argv};
    using Simple::VKFW::ContextIndex;
    using Simple::VKFW::SemaphoreId;
    using Simple::VKFW::FenceId;
    using Simple::VKFW::MemoryId;
    using Simple::VKFW::BufferId;
    using Simple::VKFW::QueueFamilyId;
    using Simple::VKFW::QueueId;
    using Simple::VKFW::DescriptorSetLayoutId;
    using Simple::VKFW::DescriptorLayoutBindingId;
    using Simple::VKFW::DescriptorPoolId;
    using Simple::VKFW::DescriptorSetsId;
    using Simple::VKFW::DescriptorSetId;
    using Simple::VKFW::PipelineId;
    using Simple::VKFW::PipelineLayoutId;
    using Simple::VKFW::ShaderModuleId;
    using Simple::VKFW::ShaderStageId;
    using Simple::VKFW::CommandPoolId;
    using Simple::VKFW::CommandBuffersId;
    using Simple::VKFW::CommandBufferId;

// Config
    const uint32_t FRAMES_IN_FLIGHT = 4u;
    const uint32_t STAGING_BUFFER_SIZE = 16 * 1024*1024; //  16 MB for now

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

    const Simple::vec2u fb_size = {1600u,  900u};
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
    main_window_handler.setAllMButtonsHandler(&main_view_centered_proj);
    main_window_handler.setFBufferSizeHandler(&main_view_centered_proj);

    // Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // for vk only, use context for OpenGL
    Simple::Window main_window{fb_size.x, fb_size.y, "Test"};

    Simple::WindowInput::DefaultHandler main_window_exit{main_window.window};

    // ImGUI widgets
    main_window_handler.imgui_handler.setFont("resources/fonts/RobotoMono-VariableFont_wght.ttf", 24.f);
    main_window_handler.imgui_handler.widgets.push_back(new Simple::Event::ImGuiWidgetLoadFile{});
    main_window_handler.imgui_handler.widgets.push_back(new Simple::Event::ImGuiTestWindow{});

    // Callbacks
    main_window_handler.setKeysHandler(&main_window_exit);
    main_window_handler.setGLFWCallbacks(main_window.window);


////  VKFW

    Simple::VKFW::VulkanContext vk;

    // Instance
    vk.cxt_core.setDefaultApplicationInfo();
    vk.cxt_core.activateDebug();
    if (Simple::VKFW::Util::allInstanceLayersAvailable({{"VK_LAYER_LUNARG_monitor"}})) // the layer isn't shipped with newer SDKs
        vk.cxt_core.instance.create_info.addLayers({{"VK_LAYER_LUNARG_monitor"}});
    vk.cxt_core.instance.create_info.addExtGLFW();
    vk.cxt_core.instance.create_info.addExtensions({{ VK_KHR_SURFACE_EXTENSION_NAME }});
    vk.cxt_core.createInstance();

    {
        Simple::vec4u api_version;
        api_version["xyz"] = Simple::VKFW::VulkanAPIVersion::split(Simple::VKFW::VulkanAPIVersion::available(), &api_version.w);
        printf(SVKFW_WRAPINFO("main", "Available VulkanAPI version: %d.%d.%d.%d\n"), api_version.x, api_version.y, api_version.z, api_version.w); fflush(stdout);
    }

    // Debug messenger
    vk.cxt_core.createDebugUtilsMessenger();

    // Physical device
    vk.cxt_core.loadPhysicalDeviceInfo();
    vk.cxt_core.findPhysicalDevice(); // Graphics + discrete GPU test by default

    // Device profile: dynamic rendering + synchronization2 need Vulkan 1.3;
    //                 with both set to false rendering goes through VkRenderPass with the original barriers (Vulkan 1.1)
    vk.cxt_core.profile.dynamic_rendering = true;
    vk.cxt_core.profile.synchronization2  = true;
    vk.cxt_core.checkProfileSupport();

    // Queue family
    QueueFamilyId ci_qfam_graphics = vk.cxt_core.addQueueFamily();
    QueueFamilyId ci_qfam_transfer = vk.cxt_core.addQueueFamily();
    Simple::VKFW::Object::QueueFamily &qfam_graphics = vk.cxt_core.queueFamily(ci_qfam_graphics);
    Simple::VKFW::Object::QueueFamily &qfam_transfer = vk.cxt_core.queueFamily(ci_qfam_transfer);

    vk.cxt_core.loadQueueFamiliesInfo(); // TODO: optimize

    qfam_graphics.select_info.selectWithTest(Simple::VKFW::Util::SelectQueueFamily::testSurfAndGraphics);
    qfam_transfer.select_info.selectWithRank(Simple::VKFW::Util::SelectQueueFamily::rankTransfer);

    qfam_graphics.createObject(); // actually it is selected, not created
    qfam_transfer.createObject(); // actually it is selected, not created

    SVKFW_ASSERT(vk.get(ci_qfam_graphics) != vk.get(ci_qfam_transfer),
                    std::runtime_error, "main()", "Transfer and Graphics queue families are the same");

    // Logical device
    const uint32_t QUEUES_COUNT_GRAPHICS = 2u;
    const uint32_t QUEUES_COUNT_TRANSFER = 1u;
    vk.cxt_core.setQueueFamilyQueueCount(ci_qfam_graphics, QUEUES_COUNT_GRAPHICS);
    vk.cxt_core.setQueueFamilyQueueCount(ci_qfam_transfer, QUEUES_COUNT_TRANSFER);
    vk.cxt_core.setDeviceQueueInfo();

    // Device features: Vulkan 1.0 features are set through 'req_features', newer ones through 'device_features'
    // Note: the features of the profile are enabled by 'createDevice'; structures of versions the device doesn't support are ignored
    vk.cxt_core.device_features.get<VkPhysicalDeviceVulkan12Features>().timelineSemaphore = VK_TRUE;

    vk.cxt_core.device.create_info.req_features.logicOp = VK_TRUE;
    vk.cxt_core.device.create_info.addExtensions({{ VK_KHR_SWAPCHAIN_EXTENSION_NAME }});
    vk.cxt_core.createDevice();

    // Queue
    vk.cxt_core.acquireQueues();
    QueueId ci_queue_graphics{ci_qfam_graphics, 0u};
    QueueId ci_queue_present {ci_qfam_graphics, 1u};
    QueueId ci_queue_transfer{ci_qfam_transfer, 0u};

    vk.initNoncoreContexts();

    // Surface
    vk.cxt_swapchain.setWindow(main_window);
    vk.cxt_swapchain.createSurface();


// Swapchain:
    // Note: the render passes (without dynamic rendering) are created by the render graph
    vk.cxt_swapchain.pickSwapchainImageFormat();

    // Swapchain
    vk.cxt_swapchain.setSwapchainSurfaceCapabilities();
    vk.cxt_swapchain.swapchain.create_info.checkSetImageCount(FRAMES_IN_FLIGHT);
    vk.cxt_swapchain.swapchain.create_info.checkSetImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vk.cxt_swapchain.swapchain.create_info.edit().imageArrayLayers = 1;
    vk.cxt_swapchain.swapchain.create_info.edit().compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vk.cxt_swapchain.swapchain.create_info.edit().presentMode = VK_PRESENT_MODE_FIFO_KHR;
    vk.cxt_swapchain.setSwapchainExtentFromWindow();

    // Swapchain image views
    vk.cxt_swapchain.image_view_wrap.setComponents(); // set color components (identity by default)
    vk.cxt_swapchain.image_view_wrap.setRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    vk.cxt_swapchain.image_view_wrap.edit().viewType = VK_IMAGE_VIEW_TYPE_2D;

    vk.cxt_swapchain.createSwapchainWithDependencies(); // creates swapchain and its images


// Render graph:
    // Note: one pass draws the scene and ImGui into the swapchain image; the graph places the barriers and
    //       begins/ends rendering (dynamic rendering or a render pass of the pass, by the profile)
    const VkClearValue clear_color = {{{0.f, 0.f, 0.f, 1.f}}};

    Simple::VKFW::Graph::RenderGraph graph(vk);
    Simple::VKFW::Graph::ResourceId  backbuffer = graph.importSwapchain();
    Simple::VKFW::Graph::PassId      scene_pass = graph.addGraphicsPass("scene").color(backbuffer, VK_ATTACHMENT_LOAD_OP_CLEAR, clear_color);

    graph.compile(); // after the swapchain creation and after every recreation


// Memory-related objects (buffers and images):
    // Staging/Uniform buffers: host-coherent memory
    MemoryId     ci_memory_staging = vk.cxt_memory.addMemoryChunk(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    MemoryId     ci_memory_uniform = vk.cxt_memory.addMemoryChunk(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    BufferId ci_buffer_uniform[FRAMES_IN_FLIGHT], ci_buffer_staging;
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i) {
        ci_buffer_uniform[i] = vk.cxt_memory.addChunkBuffer(ci_memory_uniform, sizeof(UniformBufferObjectCamera));
        vk.cxt_memory.setWholeBufferCreateInfoStruct(ci_buffer_uniform[i], { ci_qfam_graphics },
                                                                         VK_SHARING_MODE_EXCLUSIVE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        vk.cxt_memory.createChunkBuffer(ci_buffer_uniform[i]);
    }

    ci_buffer_staging = vk.cxt_memory.addChunkBuffer(ci_memory_staging, STAGING_BUFFER_SIZE);
    vk.cxt_memory.setWholeBufferCreateInfoStruct(ci_buffer_staging, { ci_qfam_transfer },
                                                                     VK_SHARING_MODE_EXCLUSIVE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vk.cxt_memory.createChunkBuffer(ci_buffer_staging);

    vk.cxt_memory.allocateChunkMemory(ci_memory_staging);
    vk.cxt_memory.allocateChunkMemory(ci_memory_uniform);

    // Vertex/Index buffers: device-local memory
    MemoryId     ci_memory_device_optim = vk.cxt_memory.addMemoryChunk(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    BufferId ci_buffer_vertex, ci_buffer_index;
    ci_buffer_vertex = vk.cxt_memory.addChunkBuffer(ci_memory_device_optim, vertices.size() * sizeof(Vertex));
    vk.cxt_memory.setWholeBufferCreateInfoStruct(ci_buffer_vertex, { ci_qfam_graphics },
                                                                     VK_SHARING_MODE_EXCLUSIVE, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vk.cxt_memory.createChunkBuffer(ci_buffer_vertex);

    ci_buffer_index = vk.cxt_memory.addChunkBuffer(ci_memory_device_optim, indices.size() * sizeof(uint32_t));
    vk.cxt_memory.setWholeBufferCreateInfoStruct(ci_buffer_index, { ci_qfam_graphics },
                                                                     VK_SHARING_MODE_EXCLUSIVE, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    vk.cxt_memory.createChunkBuffer(ci_buffer_index);

    vk.cxt_memory.allocateChunkMemory(ci_memory_device_optim);


// Descriptors:
    // Descriptor Layout
    DescriptorSetLayoutId     ci_descr_layout = vk.cxt_descriptor.addLayout();
    DescriptorLayoutBindingId ci_descr_layout_binding = vk.cxt_descriptor.addSctLayoutBinding(ci_descr_layout);

    vk.cxt_descriptor.layoutBinding(ci_descr_layout_binding).setWholeStruct(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);

    vk.cxt_descriptor.createLayout(ci_descr_layout);

    // Descriptor Pool
    DescriptorPoolId ci_descr_pool_ubo = vk.cxt_descriptor.addPool();
    Simple::VKFW::Object::DescriptorPool &descr_pool_ubo = vk.cxt_descriptor.pool(ci_descr_pool_ubo);

    descr_pool_ubo.create_info.edit().maxSets = FRAMES_IN_FLIGHT;
    descr_pool_ubo.create_info.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, FRAMES_IN_FLIGHT);

    vk.cxt_descriptor.createPool(ci_descr_pool_ubo);

    // TODO: sync descriptor sets and pools (1 pool -> multiple sets)
    // Descriptor Sets
    DescriptorSetsId ci_descr_pool_sets_ubo = vk.cxt_descriptor.addSet();
    std::vector<DescriptorSetLayoutId> cis_ubo_set_layouts((size_t)FRAMES_IN_FLIGHT, ci_descr_layout); // we need 'FRAMES_IN_FLIGHT' sets, each with 'ci_descr_layout' layout
    vk.cxt_descriptor.setAllForSet(ci_descr_pool_sets_ubo, ci_descr_pool_ubo, cis_ubo_set_layouts);

    DescriptorSetId ci_descr_sets_ubo[FRAMES_IN_FLIGHT]{};
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i)
        ci_descr_sets_ubo[i] = DescriptorSetId(ci_descr_pool_sets_ubo, i);

    vk.cxt_descriptor.createSets(ci_descr_pool_sets_ubo);

    // Descriptor Sets Update

    std::vector<Simple::VKFW::Struct::StructWrap<VkWriteDescriptorSet>> ubo_writes(FRAMES_IN_FLIGHT);
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i) {
        ubo_writes[i].addBufferInfo(vk.get(ci_buffer_uniform[i]), 0u, vk.cxt_memory.bufferMapping(ci_buffer_uniform[i]).range);
        ubo_writes[i].setScalars(vk.get(ci_descr_sets_ubo[i]), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0u);
    }
    vk.cxt_descriptor.updateSets(ubo_writes);


// Graphics pipeline:
    PipelineId ci_graphics_pip = vk.cxt_pipeline.addPipeline();
    // Note: the reference stays valid until another pipeline is added
    Simple::VKFW::Object::Pipeline &graphics_pip = vk.cxt_pipeline.pipeline(ci_graphics_pip);
    graphics_pip.setPipelineType(Simple::VKFW::PipelineType::VKFW_PIPELINE_TYPE_GRAPHICS);

    // Layout
    PipelineLayoutId ci_pip_layout = vk.cxt_pipeline.addLayout();
    vk.cxt_pipeline.setLayoutDescriptorSetLayouts(ci_pip_layout, { ci_descr_layout });
    vk.cxt_pipeline.createLayout(ci_pip_layout);

    // Shaders
    ShaderModuleId ci_shader_vert = vk.cxt_pipeline.addShader();
    ShaderModuleId ci_shader_frag = vk.cxt_pipeline.addShader();
    vk.cxt_pipeline.shader(ci_shader_vert).create_info.fromFile("./shaders/simple_shader_vert.spv");
    vk.cxt_pipeline.shader(ci_shader_frag).create_info.fromFile("./shaders/simple_shader_frag.spv");
    vk.cxt_pipeline.createShader(ci_shader_vert);
    vk.cxt_pipeline.createShader(ci_shader_frag);

    // Shader stage
    ShaderStageId ci_sstage_vert = vk.cxt_pipeline.addShaderStage(ci_graphics_pip);
    ShaderStageId ci_sstage_frag = vk.cxt_pipeline.addShaderStage(ci_graphics_pip);

    vk.cxt_pipeline.shaderStage(ci_sstage_vert).setShaderInfo(vk.get(ci_shader_vert), VK_SHADER_STAGE_VERTEX_BIT,   "main");
    vk.cxt_pipeline.shaderStage(ci_sstage_frag).setShaderInfo(vk.get(ci_shader_frag), VK_SHADER_STAGE_FRAGMENT_BIT, "main");

    // Dynamic state
    graphics_pip.graphics_create_info_wrap.dynamic_state_wrap.dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    // Vertex input state
    graphics_pip.graphics_create_info_wrap.vertex_input_state_wrap.bindings = {{ 0u, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX }};
    graphics_pip.graphics_create_info_wrap.vertex_input_state_wrap.attributes = { { 0, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex,   pos) },
                                                                                      { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) } };

    // Input assembly state
    graphics_pip.graphics_create_info_wrap.input_assembly_state_wrap.setWholeStruct(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);

    // Viewport state
    // viewport_state_info.setFromSwapchainExtent(swapchain_extent); // set dynamically
    graphics_pip.graphics_create_info_wrap.viewport_state_wrap. setScissorsDynamic(1u);
    graphics_pip.graphics_create_info_wrap.viewport_state_wrap.setViewportsDynamic(1u);

    // Rasterization state
    graphics_pip.graphics_create_info_wrap.rasterization_state_wrap.setDepthBias(false);
    graphics_pip.graphics_create_info_wrap.rasterization_state_wrap.edit().polygonMode = VK_POLYGON_MODE_FILL;
    graphics_pip.graphics_create_info_wrap.rasterization_state_wrap.edit().cullMode = VK_CULL_MODE_NONE;
    graphics_pip.graphics_create_info_wrap.rasterization_state_wrap.edit().frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    graphics_pip.graphics_create_info_wrap.rasterization_state_wrap.edit().lineWidth = 1.f;

    // Multisample state
    graphics_pip.graphics_create_info_wrap.multisample_state_wrap.edit().rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    graphics_pip.graphics_create_info_wrap.multisample_state_wrap.edit().minSampleShading = 1.f;

    // Color blend state
    graphics_pip.graphics_create_info_wrap.color_blend_state_wrap.setLogicOp(VK_LOGIC_OP_COPY);
    graphics_pip.graphics_create_info_wrap.color_blend_state_wrap.attachments_wrap.emplace_back(); // has default configuration: "Porter-Duff blend over"

    // Graphics pipeline
    graphics_pip.graphics_create_info_wrap.setPipelineStates(0xfffu);
    // Note: attachment formats for dynamic rendering (ignored with the render pass)
    graphics_pip.graphics_create_info_wrap.rendering_wrap.setFormats(vk.cxt_swapchain.format());
    vk.cxt_pipeline.createPipeline(ci_graphics_pip, graph.renderPass(scene_pass), 0u, ci_pip_layout);

    // Command pool
    vk.cxt_command.addCmdPool();
    CommandPoolId ci_pool_drawing = vk.cxt_command.addCmdPool();
    CommandPoolId ci_pool_staging = vk.cxt_command.addCmdPool();
    vk.cxt_command.createCmdPool(ci_pool_drawing, ci_qfam_graphics, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vk.cxt_command.createCmdPool(ci_pool_staging, ci_qfam_transfer, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    // Command buffers
    CommandBuffersId ci_cmd_buffers_staging = vk.cxt_command.addCmdBuffer();
    CommandBuffersId ci_cmd_buffers_drawing = vk.cxt_command.addCmdBuffer();

    vk.cxt_command.createCmdBuffersPrimary(ci_cmd_buffers_staging, ci_pool_staging, 1);
    vk.cxt_command.createCmdBuffersPrimary(ci_cmd_buffers_drawing, ci_pool_drawing, FRAMES_IN_FLIGHT);

    CommandBufferId ci_cmdbuf_staging(ci_cmd_buffers_staging, 0u);
    CommandBufferId ci_cmdbuf_drawing[FRAMES_IN_FLIGHT];
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i)
        ci_cmdbuf_drawing[i] = CommandBufferId(ci_cmd_buffers_drawing, i);


// Sync objects:
    // Semaphores
    // Note: 'image_available' semaphores are used per frame in flight, 'render_finished' - per swapchain image
    //       (swapchain may contain more images than 'FRAMES_IN_FLIGHT', and the image count may change after recreation)
    SemaphoreId ci_sem_image_available[FRAMES_IN_FLIGHT]{};
    std::vector<SemaphoreId> ci_sem_render_finished;

    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i)
        ci_sem_image_available[i] = vk.cxt_barrier.addSemaphore();
    while (ci_sem_render_finished.size() < vk.cxt_swapchain.getSwapchainImageCount())
        ci_sem_render_finished.push_back(vk.cxt_barrier.addSemaphore());

    // Fences
    FenceId ci_fen_in_flight[FRAMES_IN_FLIGHT]{};
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i)
        ci_fen_in_flight[i] = vk.cxt_barrier.addFence(true);


// Other preparations:
    // Copy static vertex/index data (both copies go in one submit)
    {
        Simple::VKFW::UploadBatch upload(vk, ci_queue_transfer, ci_cmdbuf_staging, ci_buffer_staging);
        upload.copyToBuffer(ci_buffer_vertex, vertices.data(), vertices.size() * sizeof(Vertex  ));
        upload.copyToBuffer(ci_buffer_index ,  indices.data(),  indices.size() * sizeof(uint32_t));
        upload.submit();
        upload.wait();
    }

    // ImGUI init
    main_window_handler.imgui_handler.initializeContext(vk, ci_qfam_graphics, ci_queue_graphics, FRAMES_IN_FLIGHT, graph.renderPass(scene_pass));
    main_window_handler.imgui_handler.enableForSVKFWWindow(main_window, false);


// Main loop:
    // Preparations

    // Note: 'acquired.image_index' matches the swapchain images and image views
    uint32_t current_frame = 0u;

    // Commands of the scene pass (recorded inside its rendering, viewport and scissor are set by the graph)
    graph.pass(scene_pass).execute([&](Simple::VKFW::CommandRecorder &rec) {
        rec.bindPipeline(ci_graphics_pip);
        rec.bindVertexBuffers(ci_buffer_vertex);
        rec.bindIndexBuffer(ci_buffer_index);

        // Bind descriptor set (uniform buffer)
        rec.bindDescriptorSets(ci_pip_layout, ci_descr_sets_ubo[current_frame]);

        // Draw
        // rec.draw(vertices.size());
        // Draw (indexed)
        rec.drawIndexed(indices.size());

        // Draw GUI
        main_window_handler.imgui_handler.fillCommandBuffer(rec.handle());
    });

    // Loop
    while (!main_window.shouldClose()) {
        main_window_handler.frameTick();
        glfwPollEvents();
        main_view_centered_proj.manualFrameUpdate(main_window_handler.frame_time);

        vk.cxt_barrier.wait(ci_fen_in_flight[current_frame]);

        Simple::VKFW::AcquireResult acquired = vk.acquireNextImage(ci_sem_image_available[current_frame]);
        // Note: the fence is reset only after successful acquisition, otherwise it would never be signaled again
        if (acquired.status == Simple::VKFW::VKFW_SWAPCHAIN_OUT_OF_DATE) { vk.recreateSwapchain(); graph.compile(); continue; }
        if (acquired.status == Simple::VKFW::VKFW_SWAPCHAIN_NOT_READY) continue;
        vk.cxt_barrier.reset(ci_fen_in_flight[current_frame]);

        // Swapchain image count may grow after recreation
        while (ci_sem_render_finished.size() < vk.cxt_swapchain.getSwapchainImageCount())
            ci_sem_render_finished.push_back(vk.cxt_barrier.addSemaphore());

        // Update uniform buffers (per frame in flight: the buffer of this frame is not used by the GPU after the fence wait)
        ubo_camera.view = main_view_centered_proj.getView();
        ubo_camera.proj = main_view_centered_proj.getProj();
        vk.cxt_memory.writeBuffer(ci_buffer_uniform[current_frame], &ubo_camera, sizeof(ubo_camera));


        // Record command buffer
        Simple::VKFW::CommandRecorder rec(vk, ci_cmdbuf_drawing[current_frame]);
        rec.reset();
        rec.begin();

        main_window_handler.imgui_handler.newFrame();
        main_window_handler.imgui_handler.render();
        graph.execute(rec, acquired.image_index);
        rec.end();


        // Submit
        Simple::VKFW::SubmitDesc submit;
        submit.wait(ci_sem_image_available[current_frame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
              .commands(ci_cmdbuf_drawing[current_frame])
              .signal(ci_sem_render_finished[acquired.image_index]);
        vk.submit(ci_queue_graphics, submit, ci_fen_in_flight[current_frame]);

        // Present
        Simple::VKFW::SwapchainStatus presented = vk.present(ci_queue_present, ci_sem_render_finished[acquired.image_index], acquired.image_index);
        if (presented != Simple::VKFW::VKFW_SWAPCHAIN_OK) {
            vk.recreateSwapchain();
            graph.compile();
        }

        current_frame = (current_frame + 1) % FRAMES_IN_FLIGHT;
    } // main loop END

    vkDeviceWaitIdle(vk.cxt_core.device.handle);
    for (uint32_t curr_frame = 0u; curr_frame < FRAMES_IN_FLIGHT; ++curr_frame)
        Simple::VKFW::CommandRecorder(vk, ci_cmdbuf_drawing[curr_frame]).reset();

    // ImGUI widgets destruction
    delete main_window_handler.imgui_handler.widgets[1];
    delete main_window_handler.imgui_handler.widgets[0];
    main_window_handler.imgui_handler.destroyContext();

    // Destruction
    graph.destroy();
    vk.destroyContext();
    main_window.destroy();
    Simple::glfw_init.terminate();

    printf(SVKFW_WRAPINFO("main", "Success\n"));
    return 0;
}