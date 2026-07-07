#include "main/event_handler.h"
#include "math/linalg.h"
#include "main/camera.h"
#include "main/input_handler.h"
#include "main/image.h"
#include "main/imgui_events.h"
#include "interface/vkfw.h"
#include "interface/imgui_wrap.h"

#include <iostream>


int main(int argc, char **argv) {
    Simple::ArgHandler arg_handler{argc, argv};
    using Simple::VKFW::ContextIndex;
    using Simple::VKFW::ContextIndex2;

// Config
    // return Simple::test_csound();
    const uint32_t FRAMES_IN_FLIGHT = 2u;
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
    main_window_handler.setAllMButtonsHandler(&main_view_centered_proj);
    main_window_handler.setFBufferSizeHandler(&main_view_centered_proj);

    // Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // for vk only, use context for OpenGL
    Simple::Window main_window{fb_size.x, fb_size.y, "Test"};

    Simple::WindowInput::DefaultHandler main_window_exit{main_window.window};

    // ImGUI widgets
    main_window_handler.imgui_handler.widgets.push_back(new Simple::Event::ImGuiWidgetLoadFile{});
    main_window_handler.imgui_handler.widgets.push_back(new Simple::Event::ImGuiTestWindow{});

    // Callbacks
    main_window_handler.setKeysHandler(&main_window_exit);
    main_window_handler.setGLFWCallbacks(main_window.window);


////  VKFW

    Simple::VKFW::ObjGetterHandler vkfw_getter;
    Simple::VKFW::FuncHandler func_handler;

    // Instance
    Simple::VKFW::cxt_vkfw.cxt_core.setDefaultApplicationInfo();
    Simple::VKFW::cxt_vkfw.cxt_core.activateDebug();
    Simple::VKFW::cxt_vkfw.cxt_core.instance.create_info.addLayers({{"VK_LAYER_LUNARG_monitor"}});
    Simple::VKFW::cxt_vkfw.cxt_core.instance.create_info.addExtGLFW();
    Simple::VKFW::cxt_vkfw.cxt_core.instance.create_info.addExtensions({{ VK_KHR_SURFACE_EXTENSION_NAME }});
    Simple::VKFW::cxt_vkfw.cxt_core.createInstance();

    {
        Simple::vec4u api_version;
        api_version["xyz"] = Simple::VKFW::VulkanAPIVersion::split(Simple::VKFW::VulkanAPIVersion::available(), &api_version.w);
        printf(SVKFW_WRAPINFO("main", "Available VulkanAPI version: %d.%d.%d.%d\n"), api_version.x, api_version.y, api_version.z, api_version.w); fflush(stdout);
    }

    // Debug messenger
    Simple::VKFW::cxt_vkfw.cxt_core.createDebugUtilsMessenger();

    // Physical device
    Simple::VKFW::cxt_vkfw.cxt_core.loadPhysicalDeviceInfo();
    Simple::VKFW::cxt_vkfw.cxt_core.findPhysicalDevice(); // Graphics + discrete GPU test by default

    // Queue family
    ContextIndex ci_qfam_graphics = Simple::VKFW::cxt_vkfw.cxt_core.addQueueFamily();
    ContextIndex ci_qfam_transfer = Simple::VKFW::cxt_vkfw.cxt_core.addQueueFamily();
    auto ce_qfam_graphics = vkfw_getter.shortcutObjQueueFamily(ci_qfam_graphics);
    auto ce_qfam_transfer = vkfw_getter.shortcutObjQueueFamily(ci_qfam_transfer);

    Simple::VKFW::cxt_vkfw.cxt_core.loadQueueFamiliesInfo(); // TODO: optimize

    ce_qfam_graphics->select_info.selectWithTest(Simple::VKFW::Util::SelectQueueFamily::testSurfAndGraphics);
    ce_qfam_transfer->select_info.selectWithRank(Simple::VKFW::Util::SelectQueueFamily::rankTransfer);

    ce_qfam_graphics->createObject(); // actually it is selected, not created
    ce_qfam_transfer->createObject(); // actually it is selected, not created

    SVKFW_ASSERT(vkfw_getter.getObjQueueFamily(ci_qfam_graphics) != vkfw_getter.getObjQueueFamily(ci_qfam_transfer),
                    std::runtime_error, "main()", "Transfer and Graphics queue families are the same");

    // Logical device
    const uint32_t QUEUES_COUNT_GRAPHICS = 2u;
    const uint32_t QUEUES_COUNT_TRANSFER = 1u;
    Simple::VKFW::cxt_vkfw.cxt_core.setQueueFamilyQueueCount(ci_qfam_graphics, QUEUES_COUNT_GRAPHICS);
    Simple::VKFW::cxt_vkfw.cxt_core.setQueueFamilyQueueCount(ci_qfam_transfer, QUEUES_COUNT_TRANSFER);
    Simple::VKFW::cxt_vkfw.cxt_core.setDeviceQueueInfo();

    Simple::VKFW::cxt_vkfw.cxt_core.device.create_info.req_features.logicOp = VK_TRUE;
    Simple::VKFW::cxt_vkfw.cxt_core.device.create_info.addExtensions({{ VK_KHR_SWAPCHAIN_EXTENSION_NAME }});
    Simple::VKFW::cxt_vkfw.cxt_core.createDevice();

    // Queue
    Simple::VKFW::cxt_vkfw.cxt_core.acquireQueues();
    ContextIndex2 ci_queue_graphics{ci_qfam_graphics, 0u};
    ContextIndex2 ci_queue_present {ci_qfam_graphics, 1u};
    ContextIndex2 ci_queue_transfer{ci_qfam_transfer, 0u};

    Simple::VKFW::cxt_vkfw.initNoncoreContexts();

    // Surface
    Simple::VKFW::cxt_vkfw.cxt_swapchain.setWindow(main_window);
    Simple::VKFW::cxt_vkfw.cxt_swapchain.createSurface();


// Render pass:
    // Attachment description
    ContextIndex ci_attachment = Simple::VKFW::cxt_vkfw.cxt_core.addSctRenderPassAttachment();
    auto         ce_attachment = vkfw_getter.shortcutSctRenderPassAttachment(ci_attachment);

    Simple::VKFW::cxt_vkfw.cxt_swapchain.pickSwapchainImageFormat();
    Simple::VKFW::cxt_vkfw.cxt_swapchain.setRenderPassAttachmentImageFormat (ci_attachment);
    ce_attachment->wrap.samples = VK_SAMPLE_COUNT_1_BIT;
    ce_attachment->setColorOps  (VK_ATTACHMENT_LOAD_OP_CLEAR,     VK_ATTACHMENT_STORE_OP_STORE);
    ce_attachment->setStencilOps(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE);
    ce_attachment->setLayouts   (VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Subpass description
    ContextIndex ci_subpass_descr = Simple::VKFW::cxt_vkfw.cxt_core.addSctRenderPassSubpassDescription();
    auto         ce_subpass_descr = vkfw_getter.shortcutSctRenderPassSubpassDescription(ci_subpass_descr);

    ce_subpass_descr->addColorAttachment(0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    ce_subpass_descr->wrap.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // Note: 'pColorBlendState->attachmentCount' must be == 'pSubpasses[0].colorAttachmentCount'

    // Subpass dependency
    ContextIndex ci_subpass_dep = Simple::VKFW::cxt_vkfw.cxt_core.addSctRenderPassSubpassDependency();
    auto         ce_subpass_dep = vkfw_getter.shortcutSctRenderPassSubpassDependency(ci_subpass_descr);
    ce_subpass_dep->setSubpasses(VK_SUBPASS_EXTERNAL, 0u);
    ce_subpass_dep->setSrcMasks(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_NONE);
    ce_subpass_dep->setDstMasks(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

    Simple::VKFW::cxt_vkfw.cxt_core.createRenderPass();


// Swapchain:
    // Swapchain
    Simple::VKFW::cxt_vkfw.cxt_swapchain.setSwapchainSurfaceCapabilities();
    Simple::VKFW::cxt_vkfw.cxt_swapchain.swapchain.create_info.checkSetImageCount(2);
    Simple::VKFW::cxt_vkfw.cxt_swapchain.swapchain.create_info.checkSetImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    Simple::VKFW::cxt_vkfw.cxt_swapchain.swapchain.create_info.wrap.imageArrayLayers = 1;
    Simple::VKFW::cxt_vkfw.cxt_swapchain.swapchain.create_info.wrap.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    Simple::VKFW::cxt_vkfw.cxt_swapchain.swapchain.create_info.wrap.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    Simple::VKFW::cxt_vkfw.cxt_swapchain.setSwapchainExtentFromWindow();
    // Simple::VKFW::cxt_vkfw.cxt_swapchain.pickSwapchainImageFormat();

    // Swapchain image views
    Simple::VKFW::cxt_vkfw.cxt_swapchain.image_view_wrap.setComponents(); // set color components (identity by default)
    Simple::VKFW::cxt_vkfw.cxt_swapchain.image_view_wrap.setRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    Simple::VKFW::cxt_vkfw.cxt_swapchain.image_view_wrap.wrap.viewType = VK_IMAGE_VIEW_TYPE_2D;

    // Swapchain framebuffers
    Simple::VKFW::cxt_vkfw.cxt_swapchain.framebuffer_wrap.wrap.layers = 1u;

    Simple::VKFW::cxt_vkfw.cxt_swapchain.createSwapchainWithDependencies(); // creates swapchain, its images and framebuffers


// Memory-related objects (buffers and images):
    // Staging/Uniform buffers: host-coherent memory
    ContextIndex ci_memory_staging = Simple::VKFW::cxt_vkfw.cxt_memory.addMemoryChunk(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    ContextIndex ci_memory_uniform = Simple::VKFW::cxt_vkfw.cxt_memory.addMemoryChunk(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto         ce_memory_staging = vkfw_getter.shortcutSctMemoryChunk(ci_memory_staging);
    auto         ce_memory_uniform = vkfw_getter.shortcutSctMemoryChunk(ci_memory_uniform);

    ContextIndex2 ci_buffer_uniform[FRAMES_IN_FLIGHT], ci_buffer_staging;
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i) {
        ci_buffer_uniform[i] = Simple::VKFW::cxt_vkfw.cxt_memory.addChunkBuffer(ci_memory_uniform, sizeof(UniformBufferObjectCamera));
        Simple::VKFW::cxt_vkfw.cxt_memory.setWholeBufferCreateInfoStruct(ci_buffer_uniform[i], { ci_qfam_graphics },
                                                                         VK_SHARING_MODE_EXCLUSIVE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        Simple::VKFW::cxt_vkfw.cxt_memory.createChunkBuffer(ci_buffer_uniform[i]);
    }

    ci_buffer_staging = Simple::VKFW::cxt_vkfw.cxt_memory.addChunkBuffer(ci_memory_staging, STAGING_BUFFER_SIZE);
    Simple::VKFW::cxt_vkfw.cxt_memory.setWholeBufferCreateInfoStruct(ci_buffer_staging, { ci_qfam_transfer },
                                                                     VK_SHARING_MODE_EXCLUSIVE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    Simple::VKFW::cxt_vkfw.cxt_memory.createChunkBuffer(ci_buffer_staging);

    Simple::VKFW::cxt_vkfw.cxt_memory.allocateChunkMemory(ci_memory_staging);
    Simple::VKFW::cxt_vkfw.cxt_memory.allocateChunkMemory(ci_memory_uniform);

    // Vertex/Index buffers: device-local memory
    ContextIndex ci_memory_device_optim = Simple::VKFW::cxt_vkfw.cxt_memory.addMemoryChunk(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto         ce_memory_device_optim = vkfw_getter.shortcutSctMemoryChunk(ci_memory_device_optim);

    ContextIndex2 ci_buffer_vertex, ci_buffer_index;
    ci_buffer_vertex = Simple::VKFW::cxt_vkfw.cxt_memory.addChunkBuffer(ci_memory_device_optim, vertices.size() * sizeof(Vertex));
    Simple::VKFW::cxt_vkfw.cxt_memory.setWholeBufferCreateInfoStruct(ci_buffer_vertex, { ci_qfam_graphics },
                                                                     VK_SHARING_MODE_EXCLUSIVE, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    Simple::VKFW::cxt_vkfw.cxt_memory.createChunkBuffer(ci_buffer_vertex);

    ci_buffer_index = Simple::VKFW::cxt_vkfw.cxt_memory.addChunkBuffer(ci_memory_device_optim, indices.size() * sizeof(uint32_t));
    Simple::VKFW::cxt_vkfw.cxt_memory.setWholeBufferCreateInfoStruct(ci_buffer_index, { ci_qfam_graphics },
                                                                     VK_SHARING_MODE_EXCLUSIVE, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    Simple::VKFW::cxt_vkfw.cxt_memory.createChunkBuffer(ci_buffer_index);

    Simple::VKFW::cxt_vkfw.cxt_memory.allocateChunkMemory(ci_memory_device_optim);


// Descriptors:
    // Descriptor Layout
    ContextIndex  ci_descr_layout = Simple::VKFW::cxt_vkfw.cxt_descriptor.addLayout();
    ContextIndex2 ci_descr_layout_binding = Simple::VKFW::cxt_vkfw.cxt_descriptor.addSctLayoutBinding(ci_descr_layout);
    auto          ce_descr_layout_binding = vkfw_getter.shortcutSctDescriptorSetLayoutBinding(ci_descr_layout_binding);

    ce_descr_layout_binding->setWholeStruct(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);

    Simple::VKFW::cxt_vkfw.cxt_descriptor.createLayout(ci_descr_layout);

    // Descriptor Pool
    ContextIndex ci_descr_pool_ubo = Simple::VKFW::cxt_vkfw.cxt_descriptor.addPool();
    auto         ce_descr_pool_ubo = vkfw_getter.shortcutObjDescriptorPool(ci_descr_pool_ubo);

    ce_descr_pool_ubo->create_info.wrap.maxSets = FRAMES_IN_FLIGHT;
    ce_descr_pool_ubo->create_info.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, FRAMES_IN_FLIGHT);

    Simple::VKFW::cxt_vkfw.cxt_descriptor.createPool(ci_descr_pool_ubo);

    // TODO: sync descriptor sets and pools (1 pool -> multiple sets)
    // Descriptor Sets
    ContextIndex ci_descr_pool_sets_ubo = Simple::VKFW::cxt_vkfw.cxt_descriptor.addSet();
    std::vector<ContextIndex> cis_ubo_set_layouts((size_t)FRAMES_IN_FLIGHT, ci_descr_layout); // we need 'FRAMES_IN_FLIGHT' sets, each with 'ci_descr_layout' layout
    Simple::VKFW::cxt_vkfw.cxt_descriptor.setAllForSet(ci_descr_pool_sets_ubo, ci_descr_pool_ubo, cis_ubo_set_layouts);

    ContextIndex2 ci_descr_sets_ubo[FRAMES_IN_FLIGHT]{};
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i)
        ci_descr_sets_ubo[i] = { ci_descr_pool_sets_ubo, i };

    Simple::VKFW::cxt_vkfw.cxt_descriptor.createSets(ci_descr_pool_sets_ubo);

    // Descriptor Sets Update

    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i) {
        auto ce_write_set = func_handler.update_descr_sets.addWriteSet();
        auto buffer_mapping = vkfw_getter.getSctMemoryMapping(ci_buffer_uniform[i]);
        // printf("UBO (%d, %d) info: offset %ld, range %ld\n", ci_buffer_uniform[i].first, ci_buffer_uniform[i].second, buffer_mapping.offset, buffer_mapping.range);
        ce_write_set->addBufferInfo(vkfw_getter.getObjBuffer(ci_buffer_uniform[i]), 0u, buffer_mapping.range);
        ce_write_set->setScalars(vkfw_getter.getObjDescriptorSet(ci_descr_sets_ubo[i]), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0u);
    }
    func_handler.update_descr_sets.updateWrap();
    func_handler.UpdateDescriptorSets();


// Graphics pipeline:
    ContextIndex ci_graphics_pip = Simple::VKFW::cxt_vkfw.cxt_pipeline.addPipeline();
    auto ce_graphics_pip = vkfw_getter.shortcutObjPipeline(ci_graphics_pip);
    ce_graphics_pip->setPipelineType(Simple::VKFW::PipelineType::VKFW_PIPELINE_TYPE_GRAPHICS);

    // Layout
    ContextIndex ci_pip_layout = Simple::VKFW::cxt_vkfw.cxt_pipeline.addLayout();
    Simple::VKFW::cxt_vkfw.cxt_pipeline.setLayoutDescriptorSetLayouts(ci_pip_layout, { ci_descr_layout });
    Simple::VKFW::cxt_vkfw.cxt_pipeline.createLayout(ci_pip_layout);

    // Shaders
    ContextIndex ci_shader_vert = Simple::VKFW::cxt_vkfw.cxt_pipeline.addShader();
    ContextIndex ci_shader_frag = Simple::VKFW::cxt_vkfw.cxt_pipeline.addShader();
    vkfw_getter.shortcutObjShaderModule(ci_shader_vert)->create_info.fromFile("./shaders/simple_shader_vert.spv");
    vkfw_getter.shortcutObjShaderModule(ci_shader_frag)->create_info.fromFile("./shaders/simple_shader_frag.spv");
    Simple::VKFW::cxt_vkfw.cxt_pipeline.createShader(ci_shader_vert);
    Simple::VKFW::cxt_vkfw.cxt_pipeline.createShader(ci_shader_frag);

    // Shader stage
    ContextIndex2 ci_sstage_vert = Simple::VKFW::cxt_vkfw.cxt_pipeline.addShaderStage(ci_graphics_pip);
    ContextIndex2 ci_sstage_frag = Simple::VKFW::cxt_vkfw.cxt_pipeline.addShaderStage(ci_graphics_pip);

    auto ce_sstage_vert = vkfw_getter.shortcutSctShaderStage(ci_sstage_vert);
    ce_sstage_vert->setShaderInfo(vkfw_getter.getObjShaderModule(ci_shader_vert), VK_SHADER_STAGE_VERTEX_BIT,   "main");
    auto ce_sstage_frag = vkfw_getter.shortcutSctShaderStage(ci_sstage_frag);
    ce_sstage_frag->setShaderInfo(vkfw_getter.getObjShaderModule(ci_shader_frag), VK_SHADER_STAGE_FRAGMENT_BIT, "main");

    // Dynamic state
    ce_graphics_pip->graphics_create_info_wrap.dynamic_state_wrap.dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    // Vertex input state
    ce_graphics_pip->graphics_create_info_wrap.vertex_input_state_wrap.bindings = {{ 0u, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX }};
    ce_graphics_pip->graphics_create_info_wrap.vertex_input_state_wrap.attributes = { { 0, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex,   pos) },
                                                                                      { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) } };

    // Input assembly state
    ce_graphics_pip->graphics_create_info_wrap.input_assembly_state_wrap.setWholeStruct(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);

    // Viewport state
    // viewport_state_info.setFromSwapchainExtent(swapchain_extent); // set dynamically
    ce_graphics_pip->graphics_create_info_wrap.viewport_state_wrap. setScissorsDynamic(1u);
    ce_graphics_pip->graphics_create_info_wrap.viewport_state_wrap.setViewportsDynamic(1u);

    // Rasterization state
    ce_graphics_pip->graphics_create_info_wrap.rasterization_state_wrap.setDepthBias(false);
    ce_graphics_pip->graphics_create_info_wrap.rasterization_state_wrap.wrap.polygonMode = VK_POLYGON_MODE_FILL;
    ce_graphics_pip->graphics_create_info_wrap.rasterization_state_wrap.wrap.cullMode = VK_CULL_MODE_NONE;
    ce_graphics_pip->graphics_create_info_wrap.rasterization_state_wrap.wrap.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    ce_graphics_pip->graphics_create_info_wrap.rasterization_state_wrap.wrap.lineWidth = 1.f;

    // Multisample state
    ce_graphics_pip->graphics_create_info_wrap.multisample_state_wrap.wrap.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ce_graphics_pip->graphics_create_info_wrap.multisample_state_wrap.wrap.minSampleShading = 1.f;

    // Color blend state
    ce_graphics_pip->graphics_create_info_wrap.color_blend_state_wrap.setLogicOp(VK_LOGIC_OP_COPY);
    ce_graphics_pip->graphics_create_info_wrap.color_blend_state_wrap.attachments_wrap.emplace_back(); // has default configuration: "Porter-Duff blend over"

    // Graphics pipeline
    vkfw_getter.shortcutObjPipeline(ci_graphics_pip)->graphics_create_info_wrap.setPipelineStates(0xfffu);
    Simple::VKFW::cxt_vkfw.cxt_pipeline.createPipeline(ci_graphics_pip, 0u, ci_pip_layout);

    // Command pool
    Simple::VKFW::cxt_vkfw.cxt_command.addCmdPool();
    ContextIndex ci_pool_drawing = Simple::VKFW::cxt_vkfw.cxt_command.addCmdPool();
    ContextIndex ci_pool_staging = Simple::VKFW::cxt_vkfw.cxt_command.addCmdPool();
    Simple::VKFW::cxt_vkfw.cxt_command.createCmdPool(ci_pool_drawing, ci_qfam_graphics, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    Simple::VKFW::cxt_vkfw.cxt_command.createCmdPool(ci_pool_staging, ci_qfam_transfer, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    // Command buffers
    ContextIndex ci_cmd_buffers_staging = Simple::VKFW::cxt_vkfw.cxt_command.addCmdBuffer();
    ContextIndex ci_cmd_buffers_drawing = Simple::VKFW::cxt_vkfw.cxt_command.addCmdBuffer();

    Simple::VKFW::cxt_vkfw.cxt_command.createCmdBuffersPrimary(ci_cmd_buffers_staging, ci_pool_staging, 1);
    Simple::VKFW::cxt_vkfw.cxt_command.createCmdBuffersPrimary(ci_cmd_buffers_drawing, ci_pool_drawing, FRAMES_IN_FLIGHT);

    ContextIndex2 ci_cmdbuf_staging = { ci_cmd_buffers_staging, 0 };
    ContextIndex2 ci_cmdbuf_drawing[FRAMES_IN_FLIGHT];
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i)
        ci_cmdbuf_drawing[i] = { ci_cmd_buffers_drawing, i };


// Sync objects:
    // Semaphores
    ContextIndex ci_sem_image_available[FRAMES_IN_FLIGHT]{},
                 ci_sem_render_finished[FRAMES_IN_FLIGHT]{};

    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i)
        ci_sem_image_available[i] = Simple::VKFW::cxt_vkfw.cxt_barrier.addSemaphore();
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i)
        ci_sem_render_finished[i] = Simple::VKFW::cxt_vkfw.cxt_barrier.addSemaphore();

    // Fences
    ContextIndex ci_fen_in_flight[FRAMES_IN_FLIGHT]{};
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i)
        ci_fen_in_flight[i] = Simple::VKFW::cxt_vkfw.cxt_barrier.addFence(true);


// Other preparations:
    func_handler.setFencesToWait({ci_fen_in_flight, ci_fen_in_flight + FRAMES_IN_FLIGHT});

    // Copy static vertex/index data
    Simple::VKFW::CommandHandler cmd_handler;

    cmd_handler.setCmdBuffer(ci_cmdbuf_staging);

    func_handler.writeBufferMemory(cmd_handler, ci_queue_transfer, ci_buffer_vertex, ci_buffer_staging, vertices.data(), vertices.size() * sizeof(Vertex  ));
    func_handler.writeBufferMemory(cmd_handler, ci_queue_transfer, ci_buffer_index , ci_buffer_staging,  indices.data(),  indices.size() * sizeof(uint32_t));

    cmd_handler.setCmdBuffer(ci_cmdbuf_drawing[0]);

    // ImGUI init
    main_window_handler.imgui_handler.initializeContext(vkfw_getter, ci_qfam_graphics, ci_queue_graphics, FRAMES_IN_FLIGHT);
    main_window_handler.imgui_handler.enableForSVKFWWindow(main_window, false);


// Main loop:
    // Preparations

    // Note: 'image_index' matches SwapchainContext's 'ci_framebuffer' and 'ci_image_view'
    uint32_t current_frame = 0u, image_index = UINT32_MAX;
    Simple::VKFW::BarrierHandler barrier_handler;


    Simple::VKFW::MemoryMapHandler ubo_memory_handler{};
    ubo_memory_handler.mapMemoryFromResources({ci_buffer_uniform[0], ci_buffer_uniform[1]}, {});

    // Begin render pass static info
    cmd_handler.begin_render_pass_wrap.wrap.addClearValue4f({0.f, 0.f, 0.f, 1.f});

    // Queue submit static info
    func_handler.queue_submit_wrap.wraps.resize(FRAMES_IN_FLIGHT);
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i) {
        func_handler.queue_submit_wrap.wraps[i].wait_stages   = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        func_handler.queue_submit_wrap.wraps[i].wait_semaphores = { vkfw_getter.getObjSemaphore(ci_sem_image_available[i]) };
    }

    // Loop
    while (!main_window.shouldClose()) {
        main_window_handler.frameTick();
        glfwPollEvents();
        main_view_centered_proj.manualFrameUpdate(main_window_handler.frame_time);

        barrier_handler.WaitForFences({ ci_fen_in_flight[current_frame] });
        barrier_handler.ResetFences({ci_fen_in_flight[current_frame]});

        image_index = func_handler.AcquireNextImageKHR(ci_sem_image_available[current_frame]); // swapchain recreated in this function if needed
        if (image_index == UINT32_MAX) continue;

        // Update uniform buffers
        ubo_camera.view = main_view_centered_proj.getView();
        ubo_camera.proj = main_view_centered_proj.getProj();
        ubo_memory_handler.copyToBufferMap(ci_buffer_uniform[0], &ubo_camera, 0, sizeof(ubo_camera));
        ubo_memory_handler.copyToBufferMap(ci_buffer_uniform[1], &ubo_camera, 0, sizeof(ubo_camera));


        // Set command buffer
        cmd_handler.setCmdBuffer(ci_cmdbuf_drawing[current_frame]);
        cmd_handler.ResetCommandBuffer();

        // Begin recording command buffer
        cmd_handler.BeginCommandBuffer();

        // Begin render pass
        cmd_handler.setBeginRenderPassSwapchainRenderArea();
        cmd_handler.setBeginRenderPassSwapchainFramebuffer(image_index);
        cmd_handler.CmdBeginRenderPass(VK_SUBPASS_CONTENTS_INLINE);

        cmd_handler.CmdBindPipeline(ci_graphics_pip);
        cmd_handler.CmdBindVertexBuffers({ci_buffer_vertex});
        cmd_handler.CmdBindIndexBuffer(ci_buffer_index);

        // Bind descriptor set (uniform buffer)
        cmd_handler.CmdBindDescriptorSets(ci_pip_layout, VK_PIPELINE_BIND_POINT_GRAPHICS, { ci_descr_sets_ubo[current_frame] });

        // Set viewport, scissor
        cmd_handler.cmdSetViewportScissorFromSwapchainExtent();

        // cmd_handler.CmdClearAttachments();
        // cmd_handler.CmdDraw(vertices.size(), 1);
        // Draw (indexed)
        cmd_handler.CmdDrawIndexed(indices.size(), 1);

        // Draw GUI
        main_window_handler.imgui_handler.newFrame();
        main_window_handler.imgui_handler.render();
        main_window_handler.imgui_handler.fillCommandBuffer(cmd_handler.cmd_buffer);

        // End render pass
        cmd_handler.CmdEndRenderPass();

        // End recording command buffer
        cmd_handler.EndCommandBuffer();


        // Submit
        func_handler.queue_submit_wrap.wraps[current_frame].signal_semaphores = { vkfw_getter.getObjSemaphore(ci_sem_render_finished[image_index]) };
        func_handler.QueueSubmit(ci_queue_graphics, {ci_cmdbuf_drawing[current_frame]}, {current_frame}, ci_fen_in_flight[current_frame]);

        // Present
        func_handler.setWholeQueuePresentStruct({ ci_sem_render_finished[image_index] }, { image_index });
        func_handler.QueuePresentKHR(ci_queue_present);

        current_frame = (current_frame + 1) % FRAMES_IN_FLIGHT;
    } // main loop END

    vkDeviceWaitIdle(vkfw_getter.getObjDevice());
    for (uint32_t curr_frame = 0u; curr_frame < FRAMES_IN_FLIGHT; ++curr_frame) {
        barrier_handler.WaitForFences({ci_fen_in_flight[curr_frame]});
        barrier_handler.ResetFences({ci_fen_in_flight[curr_frame]});
        cmd_handler.setCmdBuffer({ci_cmdbuf_drawing[curr_frame]});
        cmd_handler.ResetCommandBuffer();
    }

    // ImGUI widgets destruction
    delete main_window_handler.imgui_handler.widgets[1];
    delete main_window_handler.imgui_handler.widgets[0];
    main_window_handler.imgui_handler.destroyContext();

    // Destruction
    Simple::VKFW::cxt_vkfw.destroyContext();
    main_window.destroy();
    Simple::glfw_init.~GLFWInitClass();

    printf("Success\n");
    return 0;
}