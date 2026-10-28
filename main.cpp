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
    using Simple::VKFW::SamplerId;
    using Simple::VKFW::Graph::FrameCapture;

// Config
    const uint32_t FRAMES_IN_FLIGHT = 4u;
    const uint32_t STAGING_BUFFER_SIZE = 16 * 1024*1024; //  16 MB for now

// Scene info

    // Geometry
    struct Vertex {
        Simple::vec3 pos;
        Simple::vec3 color;
        Simple::vec2 uv;
    };
    // Note: two quads crossing along the Y axis (the depth test sorts them); the texture repeats twice along each side
    const std::vector<Vertex> vertices = { {{-0.5f, -0.5f,  0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
                                           {{ 0.5f, -0.5f,  0.0f}, {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f}},
                                           {{ 0.5f,  0.5f,  0.0f}, {0.0f, 0.0f, 1.0f}, {2.0f, 2.0f}},
                                           {{-0.5f,  0.5f,  0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 2.0f}},

                                           {{ 0.0f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                                           {{ 0.0f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {2.0f, 0.0f}},
                                           {{ 0.0f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {2.0f, 2.0f}},
                                           {{ 0.0f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 2.0f}}
    };

    // Texture (RGBA8, loaded with stb_image)
    Simple::Img::Image4u texture_image("resources/textures/checker.png");
    static_assert(sizeof(Simple::Img::Image4u::ColorType) == 4u, "texels must be tightly packed RGBA8");
    const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0,
                                            4, 5, 6, 6, 7, 4 };

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

    // Note: layers from the top: ImGui (built in), screenshots (targets are set with the render graph), window controls, camera.
    //       The keys of each layer come from its input map in 'resources/input_maps'
    Simple::WindowInput::FrameInputHandlerGLFW main_window_handler;
    main_view_centered_proj.loadInputMap("resources/input_maps/camera.json");

    // Window
    Simple::GLFWInitClass glfw_init;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // for vk only, use context for OpenGL
    Simple::Window main_window{fb_size.x, fb_size.y, "Test"};
    main_window.setIcon(texture_image);

    Simple::WindowInput::DefaultHandler main_window_controls{main_window.window};
    main_window_controls.loadInputMap("resources/input_maps/window.json");
    Simple::WindowInput::ScreenshotHandler screenshots;
    screenshots.loadInputMap("resources/input_maps/screenshots.json");
    main_window_handler.pushLayer(&screenshots);
    main_window_handler.pushLayer(&main_window_controls);
    main_window_handler.pushLayer(&main_view_centered_proj);

    // ImGUI widgets
    main_window_handler.imgui_handler.setFont("resources/fonts/RobotoMono-VariableFont_wght.ttf", 24.f);
    main_window_handler.imgui_handler.widgets.push_back(new Simple::Event::ImGuiWidgetLoadFile{});
    main_window_handler.imgui_handler.widgets.push_back(new Simple::Event::ImGuiTestWindow{});

    // Callbacks
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

    // Device profile: dynamic rendering + synchronization2 + extended dynamic state need Vulkan 1.3;
    //                 with all set to false rendering goes through VkRenderPass with the original barriers (Vulkan 1.1)
    //                 and the stencil ops are fixed in the pipeline (only the reference and the masks stay dynamic)
    vk.cxt_core.profile.dynamic_rendering = true;
    vk.cxt_core.profile.synchronization2  = true;
    vk.cxt_core.profile.extended_dynamic_state = true;
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

    // Device features: 'features' are required ('createDevice' throws with the names of the unsupported ones),
    //                  'optional_features' are enabled if the device supports them
    // Note: Vulkan 1.0 features are in 'head().features'; the features of the profile are enabled by 'createDevice'
    vk.cxt_core.device.create_info.features.get<VkPhysicalDeviceVulkan12Features>().timelineSemaphore = VK_TRUE;
    vk.cxt_core.device.create_info.features.head().features.logicOp = VK_TRUE;
    // Note: without it 'createSampler' turns anisotropic filtering off
    vk.cxt_core.device.create_info.optional_features.head().features.samplerAnisotropy = VK_TRUE;
    vk.cxt_core.device.create_info.addExtensions({{ VK_KHR_SWAPCHAIN_EXTENSION_NAME }});
    // Note: lets 'cxt_memory' see the real free space of the memory heaps (see 'ContextMemory::heapBudget')
    if (Simple::VKFW::Util::allDeviceExtAvailable(vk.cxt_core.phys_device.handle, {{ VK_EXT_MEMORY_BUDGET_EXTENSION_NAME }}))
        vk.cxt_core.device.create_info.addExtensions({{ VK_EXT_MEMORY_BUDGET_EXTENSION_NAME }});
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
    // Note: TRANSFER_SRC lets the frame be read back (see 'RenderGraph::readback'); the supported flags are set anyway
    vk.cxt_swapchain.swapchain.create_info.checkSetImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
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
    // Note: the scene pass draws into the swapchain image with a depth/stencil buffer, the UI pass draws ImGui over it
    //       (without depth). The graph places the barriers and begins/ends rendering (dynamic rendering or a render pass
    //       of each pass, by the profile); the depth/stencil texture is recreated with the swapchain size by 'compile'
    const VkClearValue clear_color = {{{0.f, 0.f, 0.f, 1.f}}};
    VkClearValue clear_depth_stencil;
    clear_depth_stencil.depthStencil = {1.f, 0u};

    // Depth/stencil format: D24_UNORM_S8_UINT if the device has it (otherwise D32_SFLOAT_S8_UINT), readable by transfers
    const VkFormat depth_stencil_format = Simple::VKFW::Util::pickDepthStencilFormat(vk.cxt_core.phys_device.handle, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);
    Simple::VKFW::Graph::TextureDesc depth_stencil_desc(depth_stencil_format);
    depth_stencil_desc.extra_usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // for 'RenderGraph::readback'

    Simple::VKFW::Graph::RenderGraph graph(vk);
    Simple::VKFW::Graph::ResourceId  backbuffer    = graph.importSwapchain();
    Simple::VKFW::Graph::ResourceId  depth_stencil = graph.createTexture("depth_stencil", depth_stencil_desc);
    // Note: STORE keeps the depth and stencil after the pass, so they can be read back
    Simple::VKFW::Graph::PassId      scene_pass = graph.addGraphicsPass("scene").color(backbuffer, VK_ATTACHMENT_LOAD_OP_CLEAR, clear_color)
                                                                                .depth(depth_stencil, VK_ATTACHMENT_LOAD_OP_CLEAR, clear_depth_stencil);
    Simple::VKFW::Graph::PassId      ui_pass    = graph.addGraphicsPass("ui").color(backbuffer, VK_ATTACHMENT_LOAD_OP_LOAD);

    graph.compile(); // after the swapchain creation and after every recreation

    // Screenshots (PNG in 'screenshots/'): the scene without ImGui, the whole frame, depth, stencil
    // Note: the copy is made in the next frame and the file is written when its fence is waited (see the main loop)
    screenshots.setTarget(Simple::WindowInput::Actions::SCREENSHOT_SCENE,   FrameCapture::VKFW_CAPTURE_COLOR,   scene_pass, backbuffer);
    screenshots.setTarget(Simple::WindowInput::Actions::SCREENSHOT_FRAME,   FrameCapture::VKFW_CAPTURE_COLOR,   ui_pass,    backbuffer);
    screenshots.setTarget(Simple::WindowInput::Actions::SCREENSHOT_DEPTH,   FrameCapture::VKFW_CAPTURE_DEPTH,   scene_pass, depth_stencil);
    screenshots.setTarget(Simple::WindowInput::Actions::SCREENSHOT_STENCIL, FrameCapture::VKFW_CAPTURE_STENCIL, scene_pass, depth_stencil);


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

    // Texture: image + whole view in its own device-local memory, with the full mip chain
    // Note: EXCLUSIVE to the graphics family, the upload on the transfer queue hands it over (see 'UploadBatch::setOwner')
    Simple::VKFW::Graph::Texture texture = Simple::VKFW::Graph::createTexture(vk,
        Simple::VKFW::Graph::TextureInfo(VK_FORMAT_R8G8B8A8_SRGB, VkExtent2D{(uint32_t)texture_image.width, (uint32_t)texture_image.height},
                                         0u, { ci_qfam_graphics }));

    // Sampler: linear filtering between texels and mip levels, repeated, anisotropic if the device supports it
    SamplerId ci_sampler = vk.cxt_descriptor.addSampler();
    vk.cxt_descriptor.sampler(ci_sampler).create_info.setWholeStruct(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 16.f);
    vk.cxt_descriptor.createSampler(ci_sampler);


// Descriptors:
    // Descriptor Layout
    DescriptorSetLayoutId     ci_descr_layout = vk.cxt_descriptor.addLayout();
    DescriptorLayoutBindingId ci_descr_layout_binding = vk.cxt_descriptor.addSctLayoutBinding(ci_descr_layout);
    DescriptorLayoutBindingId ci_descr_layout_texture = vk.cxt_descriptor.addSctLayoutBinding(ci_descr_layout);

    vk.cxt_descriptor.layoutBinding(ci_descr_layout_binding).setWholeStruct(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    vk.cxt_descriptor.layoutBinding(ci_descr_layout_texture).setWholeStruct(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

    vk.cxt_descriptor.createLayout(ci_descr_layout);

    // Descriptor Pool
    DescriptorPoolId ci_descr_pool_ubo = vk.cxt_descriptor.addPool();
    Simple::VKFW::Object::DescriptorPool &descr_pool_ubo = vk.cxt_descriptor.pool(ci_descr_pool_ubo);

    descr_pool_ubo.create_info.edit().maxSets = FRAMES_IN_FLIGHT;
    descr_pool_ubo.create_info.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, FRAMES_IN_FLIGHT);
    descr_pool_ubo.create_info.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, FRAMES_IN_FLIGHT);

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

    // Note: two writes per set: the uniform buffer (binding 0) and the texture with its sampler (binding 1)
    std::vector<Simple::VKFW::Struct::StructWrap<VkWriteDescriptorSet>> descr_writes(2u * FRAMES_IN_FLIGHT);
    for (uint32_t i = 0u; i < FRAMES_IN_FLIGHT; ++i) {
        descr_writes[2*i].addBufferInfo(vk.get(ci_buffer_uniform[i]), 0u, vk.cxt_memory.bufferMapping(ci_buffer_uniform[i]).range);
        descr_writes[2*i].setScalars(vk.get(ci_descr_sets_ubo[i]), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0u);
        descr_writes[2*i + 1].addImageInfo(vk.get(ci_sampler), texture.view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        descr_writes[2*i + 1].setScalars(vk.get(ci_descr_sets_ubo[i]), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1u);
    }
    vk.cxt_descriptor.updateSets(descr_writes);


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

    // Dynamic state: the stencil reference and masks are always set by the commands, the stencil test and its ops too
    //                with the extended dynamic state (e.g. several stencil passes with different references and ops, one pipeline)
    const bool dynamic_stencil_ops = vk.cxt_core.profile.extended_dynamic_state;
    graphics_pip.graphics_create_info_wrap.dynamic_state_wrap.dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
                                                                                 VK_DYNAMIC_STATE_STENCIL_REFERENCE,
                                                                                 VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK, VK_DYNAMIC_STATE_STENCIL_WRITE_MASK };
    if (dynamic_stencil_ops) {
        graphics_pip.graphics_create_info_wrap.dynamic_state_wrap.dynamic_states.push_back(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE);
        graphics_pip.graphics_create_info_wrap.dynamic_state_wrap.dynamic_states.push_back(VK_DYNAMIC_STATE_STENCIL_OP);
    }

    // Vertex input state
    graphics_pip.graphics_create_info_wrap.vertex_input_state_wrap.bindings = {{ 0u, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX }};
    graphics_pip.graphics_create_info_wrap.vertex_input_state_wrap.attributes = { { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex,   pos) },
                                                                                  { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) },
                                                                                  { 2, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex,    uv) } };

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

    // Depth/stencil state: nearer fragments pass (depth cleared to 1.0); the stencil writes the reference of each draw
    // Note: with 'dynamic_stencil_ops' the stencil part is only a default, the commands set it
    graphics_pip.graphics_create_info_wrap.depth_stencil_state_wrap.setDepth(true, true, VK_COMPARE_OP_LESS);
    graphics_pip.graphics_create_info_wrap.depth_stencil_state_wrap.setStencil(true, Simple::VKFW::Util::stencilOpState(VK_COMPARE_OP_ALWAYS, VK_STENCIL_OP_REPLACE));

    // Color blend state
    graphics_pip.graphics_create_info_wrap.color_blend_state_wrap.setLogicOp(VK_LOGIC_OP_COPY);
    graphics_pip.graphics_create_info_wrap.color_blend_state_wrap.attachments_wrap.emplace_back(); // has default configuration: "Porter-Duff blend over"

    // Graphics pipeline
    graphics_pip.graphics_create_info_wrap.setPipelineStates(0xfffu);
    // Note: attachment formats for dynamic rendering (ignored with the render pass)
    graphics_pip.graphics_create_info_wrap.rendering_wrap.setFormats(vk.cxt_swapchain.format(), depth_stencil_format, depth_stencil_format);
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
    // Copy static vertex/index data and mip level 0 of the texture (all copies go in one submit)
    // Note: the buffers and the texture are EXCLUSIVE to the graphics family, so the batch hands them over from the transfer queue
    {
        Simple::VKFW::UploadBatch upload(vk, ci_queue_transfer, ci_cmdbuf_staging, ci_buffer_staging);
        upload.setOwner(ci_queue_graphics, ci_pool_drawing);
        upload.copyToBuffer(ci_buffer_vertex, vertices.data(), vertices.size() * sizeof(Vertex  ));
        upload.copyToBuffer(ci_buffer_index ,  indices.data(),  indices.size() * sizeof(uint32_t));
        upload.copyToImage(texture.image_id, texture_image.img.data(), texture_image.img.size() * sizeof(Simple::Img::Image4u::ColorType));
        upload.submit();
        upload.wait();
    }

    // Texture mip levels: blits need a graphics queue, so they go in a separate submit after the upload
    {
        FenceId ci_fen_mips = vk.cxt_barrier.addFence(false);
        Simple::VKFW::CommandRecorder rec(vk, ci_cmdbuf_drawing[0]);
        rec.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        Simple::VKFW::Graph::generateMips(rec, texture); // level 0 is uploaded, the others are made from it
        rec.end();

        Simple::VKFW::SubmitDesc submit;
        submit.commands(ci_cmdbuf_drawing[0]);
        vk.submit(ci_queue_graphics, submit, ci_fen_mips);
        vk.cxt_barrier.wait(ci_fen_mips);
        vk.cxt_barrier.destroyFence(ci_fen_mips);
        rec.reset();
    }

    // ImGUI init
    main_window_handler.imgui_handler.initializeContext(vk, ci_qfam_graphics, ci_queue_graphics, FRAMES_IN_FLIGHT, graph.renderPass(ui_pass));
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

        // Stencil: each quad writes its own value where it is visible (1 for the first one, 2 for the second one)
        if (dynamic_stencil_ops) {
            rec.setStencilTestEnable(true);
            rec.setStencilOp(VK_STENCIL_FACE_FRONT_AND_BACK, VK_COMPARE_OP_ALWAYS, VK_STENCIL_OP_REPLACE);
        }
        rec.setStencilCompareMask(VK_STENCIL_FACE_FRONT_AND_BACK, 0xffu);
        rec.setStencilWriteMask  (VK_STENCIL_FACE_FRONT_AND_BACK, 0xffu);

        // Draw (indexed)
        rec.setStencilReference(VK_STENCIL_FACE_FRONT_AND_BACK, 1u);
        rec.drawIndexed(6u, 1u, 0, 0u);
        rec.setStencilReference(VK_STENCIL_FACE_FRONT_AND_BACK, 2u);
        rec.drawIndexed(6u, 1u, 0, 6u);
    });

    // Commands of the UI pass: ImGui over the scene
    graph.pass(ui_pass).execute([&](Simple::VKFW::CommandRecorder &rec) {
        main_window_handler.imgui_handler.fillCommandBuffer(rec.handle());
    });

    // Loop
    while (!main_window.shouldClose()) {
        main_window_handler.frameTick();
        glfwPollEvents();
        main_view_centered_proj.manualFrameUpdate(main_window_handler.frame_time);

        vk.cxt_barrier.wait(ci_fen_in_flight[current_frame]);
        screenshots.save(current_frame); // the copy of this frame in flight (if any) is ready after the fence

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
        screenshots.record(graph, current_frame); // a requested screenshot is copied by 'execute'
        graph.execute(rec, acquired.image_index);
        // Note: a frame can also be read back here, at its end, e.g. 'graph.readback(rec, depth_stencil, acquired.image_index, readback)'
        //       (or 'backbuffer' for the color), and read with 'readback.readDepth(...)' / 'readStencil' / 'readColor' after the fence
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
    screenshots.destroy(); // writes the last one, the device is idle
    for (uint32_t curr_frame = 0u; curr_frame < FRAMES_IN_FLIGHT; ++curr_frame)
        Simple::VKFW::CommandRecorder(vk, ci_cmdbuf_drawing[curr_frame]).reset();

    // ImGUI widgets destruction
    delete main_window_handler.imgui_handler.widgets[1];
    delete main_window_handler.imgui_handler.widgets[0];
    main_window_handler.imgui_handler.destroyContext();

    // Destruction
    texture.destroy();
    graph.destroy();
    vk.destroyContext();
    main_window.destroy();
   glfw_init.terminate();

    printf(SVKFW_WRAPINFO("main", "Success\n"));
    return 0;
}