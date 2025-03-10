#include "interface/vkfw.h"
#include "main/event_handler.h"
#include "math/linalg.h"

#include <iostream>


int main() {
    // Common

    Simple::TermCtrl::TextStyle _i{};

    // Init & instance

    if (volkInitialize() != VK_SUCCESS)
        throw std::runtime_error(SVKFW_WRAPERR("main", "could not initialize Volk"));

    Simple::VKFW::Instance::Handler instance;
    {
        Simple::VKFW::Instance::Builder info;
        info.setApplicationInfo("Testapp", { 0u, 0u, 1u }, "SVKFW", { 0u, 0u, 1u });
        info.addGLFWExtensions();
        info.addDebugUtilsExtension();
        info.addValLayers();
        info.addValLayers({{"VK_LAYER_LUNARG_monitor"}});

        instance = info.createObject();
    }

    // Debug messenger
    Simple::VKFW::DebugUtilsMessengerEXT::Handler debug_messenger;
    {
        Simple::VKFW::DebugUtilsMessengerEXT::Builder info{instance};
        debug_messenger = info.createObject();
    }

    // Window & surface
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // for vk only, use context for OpenGL
    Simple::Window main_window{1024, 1024, "Test"};

    // Callbacks
    glfwSetKeyCallback(main_window.window, Simple::GLFWDefCallacks::defKeyCallback);
    // glfwSetWindowSizeCallback(main_window.window, );

    Simple::VKFW::SurfaceKHR::Handler surface;
    {
        Simple::VKFW::SurfaceKHR::Builder info{instance, main_window};
        surface = info.createObject();
    }

    // Physical device
    Simple::VKFW::PhysicalDevice::Handler phys_device;
    {
        Simple::VKFW::PhysicalDevice::Manager device_manager{instance};
        device_manager.loadInfo();
        phys_device = device_manager.testPickOne(Simple::VKFW::PhysicalDevice::Suitability::defaultGraphicsTest);
    }

    // Queue family
    Simple::VKFW::QueueFamily::Handler qfamily_graphics_pres;
    Simple::VKFW::QueueFamily::Manager qfamily_manager{instance, phys_device};
    {
        qfamily_manager.loadInfo();
        qfamily_graphics_pres = qfamily_manager.testPickOne(Simple::VKFW::QueueFamily::Suitability::testSurfAndGraphics);
    }


    // Logical device
    Simple::VKFW::Device::Handler logic_device;
    {
        Simple::VKFW::Device::Builder device_info{phys_device};
        device_info.addQueueInfo(qfamily_manager.getElemInfo(qfamily_graphics_pres.index).properties, qfamily_graphics_pres, 2);

        // Set required features
        VkPhysicalDeviceFeatures device_features{};
        device_features.logicOp = VK_TRUE;
        device_info.setRequiredFeatures(device_features);

        if (Simple::VKFW::VulkanAPIVersion::available() <= VK_API_VERSION_1_0)
            device_info.addValLayers();

        device_info.addExtensions({{ VK_KHR_SWAPCHAIN_EXTENSION_NAME }});

        logic_device = device_info.createObject();
    }

    // Queue
    Simple::VKFW::Queue::Handler graphics_queue, pres_queue;
    {
        Simple::VKFW::Queue::Builder queue_getter{logic_device, qfamily_graphics_pres};
        graphics_queue = queue_getter.createObject(0);
        pres_queue = queue_getter.createObject(1);
    }



    // Swapchain
    VkFormat swapchain_format{};
    VkExtent2D swapchain_extent{}; // can be used to set Viewport state
    Simple::VKFW::SwapchainKHR::Handler swapchain;
    {
        Simple::VKFW::SwapchainKHR::Builder swapchain_info{phys_device, surface, logic_device};
        swapchain_info.loadSurfaceCapabilities(); // not needed if phys_device and surface are passed, otherwise must be called first
        swapchain_info.chooseImageCount(2);
        swapchain_info.setCurrentTransform();
        swapchain_info.chooseCurrentExtent(main_window);
        swapchain_info.choosePresentationMode(Simple::VKFW::SwapchainKHR::Suitability::defaultPresentModeTest);
        swapchain_info.chooseFormat(Simple::VKFW::SwapchainKHR::Suitability::defaultFormatTest);
        swapchain_info.checkAndSetImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        swapchain_info.initHandler(swapchain);
        swapchain_format = swapchain_info.image_info.format;
        swapchain_extent = swapchain_info.image_info.extent;
    }

    // Swapchain images
    Simple::VKFW::Image::VecHandler swapchain_images = Simple::VKFW::Func::GetSwapchainImages(logic_device, swapchain);

    // Image view
    Simple::VKFW::ImageView::VecHandler swapchain_image_views;
    {
        Simple::VKFW::ImageView::Builder image_view_info{logic_device};
        image_view_info.setFormat(swapchain_format);
        image_view_info.setViewType(VK_IMAGE_VIEW_TYPE_2D);
        image_view_info.setRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
        image_view_info.setComponents();
        for (VkImage sc_image : swapchain_images.handlers) {
            image_view_info.setImage(sc_image);
            swapchain_image_views.handlers.push_back(image_view_info.createObject()); // TODO: "add()" method for VecHandlers
        }
    }



// Graphics pipeline

    // Attachment description
    Simple::VKFW::AttachmentDescription::StructWrap attachment;
    attachment.setFormatAndSize(swapchain_format, VK_SAMPLE_COUNT_1_BIT);
    attachment.setColorOps  (VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
    attachment.setStencilOps(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE);
    attachment.setLayouts(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Subpass description
    Simple::VKFW::SubpassDescription::SctHandler subpass;
    {
        Simple::VKFW::SubpassDescription::Builder subpass_builder;
        subpass_builder.setBindingPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
        // Note: pColorBlendState->attachmentCount must be == pSubpasses[0].colorAttachmentCount
        subpass_builder.setColorAttachments({ { 0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } });
        subpass = subpass_builder.makeInfoStructDCopy();
    }

    // Render pass
    Simple::VKFW::RenderPass::Handler render_pass;
    {
        Simple::VKFW::RenderPass::Builder render_pass_info{logic_device};
        render_pass_info.setAttachments({ attachment });
        render_pass_info.setSubpasses({ subpass.subpass_description });

        Simple::VKFW::SubpassDependency::StructWrap sp_dependency{};
        sp_dependency.setSubpasses(VK_SUBPASS_EXTERNAL, 0u);
        sp_dependency.setSrcMasks(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0u);
        sp_dependency.setDstMasks(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
        render_pass_info.setDependencies({ sp_dependency.subpass_dependency });

        render_pass = render_pass_info.createObject();
    }

    // Pipeline layout
    Simple::VKFW::PipelineLayout::Handler pipeline_layout;
    {
        Simple::VKFW::PipelineLayout::Builder pipeline_layout_info{logic_device};
        pipeline_layout = pipeline_layout_info.createObject();
    }


    // Shaders
    Simple::VKFW::ShaderModule::Handler vert_shader, frag_shader;
    {
        Simple::VKFW::ShaderModule::Builder shader_info{logic_device};
        shader_info.fromFile("./shaders/vert.spv");
        vert_shader = shader_info.createObject();
        shader_info.fromFile("./shaders/frag.spv");
        frag_shader = shader_info.createObject();
    }

    // Shader stage
    Simple::VKFW::Pipeline::ShaderStage::InfoHandlerView shader_stage_vert, shader_stage_frag;
    {
        Simple::VKFW::Pipeline::ShaderStage::Builder shader_stage_info;
        shader_stage_info.setShaderInfo(vert_shader, VK_SHADER_STAGE_VERTEX_BIT,   "main");
        shader_stage_vert = shader_stage_info.makeInfoStruct();
        shader_stage_info.setShaderInfo(frag_shader, VK_SHADER_STAGE_FRAGMENT_BIT, "main");
        shader_stage_frag = shader_stage_info.makeInfoStruct();
    }

    // Dynamic state
    Simple::VKFW::Pipeline::DynamicState::InfoHandler dynamic_state;
    {
        Simple::VKFW::Pipeline::DynamicState::Builder dynamic_state_info;
        dynamic_state_info.setDynamicStates({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
        dynamic_state = dynamic_state_info.makeInfoStructDCopy();
    }

    // Vertex input state
    Simple::VKFW::Pipeline::VertexInputState::InfoHandler vertex_input_state;
    {
        Simple::VKFW::Pipeline::VertexInputState::Builder vertex_input_state_info;
        vertex_input_state = vertex_input_state_info.makeInfoStructDCopy();
    }

    // Input assembly state
    Simple::VKFW::Pipeline::InputAssemblyState::StructWrap input_assembly_state;
    input_assembly_state.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    // Viewport state
    Simple::VKFW::Pipeline::ViewportState::InfoHandler viewport_state;
    {
        Simple::VKFW::Pipeline::ViewportState::Builder viewport_state_info;
        // viewport_state_info.setFromSwapchainExtent(swapchain_extent); // set dynamically
        viewport_state_info.setScissorsDynamic(1u);
        viewport_state_info.setViewportsDynamic(1u);
        viewport_state = viewport_state_info.makeInfoStructDCopy();
    }

    // Rasterization state
    Simple::VKFW::Pipeline::RasterizationState::StructWrap rasterization_state;
    rasterization_state.defStruct();
    rasterization_state.setPolygonMode(VK_POLYGON_MODE_FILL);
    rasterization_state.setCullMode(VK_CULL_MODE_BACK_BIT);
    rasterization_state.setFrontFace(VK_FRONT_FACE_CLOCKWISE);

    // Multisample state
    Simple::VKFW::Pipeline::MultisampleState::StructWrap multisample_state;
    multisample_state.defStruct();

    // Color blend state
    Simple::VKFW::Pipeline::ColorBlendState::InfoHandler color_blend_state;
    {
        Simple::VKFW::Pipeline::ColorBlendState::Builder color_blend_state_info;
        Simple::VKFW::Pipeline::ColorBlendAttachmentState::StructWrap attachment_state{};
        attachment_state.defStruct();
        color_blend_state_info.setLogicOp(VK_LOGIC_OP_COPY);
        color_blend_state_info.setAttachments({attachment_state});
        color_blend_state = color_blend_state_info.makeInfoStructDCopy();
    }

    // Graphics pipeline
    VkPipeline graphics_pipeline;
    {
        Simple::VKFW::Pipeline::Graphics::Builder graphics_pipeline_info{pipeline_layout};
        graphics_pipeline_info.setShaderStages({{ shader_stage_vert, shader_stage_frag }});
        graphics_pipeline_info.setVertexInputState(vertex_input_state.ptr());
        graphics_pipeline_info.setInputAssemblyState(input_assembly_state.ptr());
        graphics_pipeline_info.setDynamicState(dynamic_state.ptr());
        graphics_pipeline_info.setViewportState(viewport_state.ptr());
        graphics_pipeline_info.setRasterizationState(rasterization_state.ptr());
        graphics_pipeline_info.setMultisampleState(multisample_state.ptr());
        graphics_pipeline_info.setColorBlendState(color_blend_state.ptr());
        graphics_pipeline_info.setSubpass(0u);

        graphics_pipeline = graphics_pipeline_info.createObject(device, render_pass);
    }


    // Swapchain framebuffers
    Simple::VKFW::Framebuffer::VecHandler swapchain_fbuffers;
    {
        Simple::VKFW::Framebuffer::Builder fbuffers_info{logic_device, render_pass};
        fbuffers_info.setBufferSizeAndLayers(swapchain_extent, 1);
        // TODO: VecHandlerExt - swapchain_fbuffers.device = logic_device.device;

        for (VkImageView img_view : swapchain_image_views.handlers) {
            fbuffers_info.setAttachment(img_view);
            swapchain_fbuffers.handlers.push_back(fbuffers_info.createObject());
        }
    }

    // Command pool
    Simple::VKFW::CommandPool::Handler command_pool;
    {
        Simple::VKFW::CommandPool::Builder command_pool_info{logic_device, qfamily_graphics_pres};
        command_pool = command_pool_info.createObject(nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    }

    // Command buffer
    Simple::VKFW::CommandBuffer::Handler command_buffer;
    {
        Simple::VKFW::CommandBuffer::Builder command_buffer_info{logic_device, command_pool};
        command_buffer_info.setLevelPrimary();
        command_buffer_info.setBufferCount(1);
        command_buffer = command_buffer_info.createObject();
    }

    // Syncronization objects
    Simple::VKFW::Semaphore::Handler sem_image_available = Simple::VKFW::Semaphore::Builder{logic_device}.createObject(),
                                     sem_render_finished = Simple::VKFW::Semaphore::Builder{logic_device}.createObject();
    Simple::VKFW::Fence::Handler fen_in_flight = Simple::VKFW::Fence::Builder{logic_device}.createObject(nullptr, VK_FENCE_CREATE_SIGNALED_BIT);




    // Main loop
    std::chrono::steady_clock clock;
    auto tp = clock.now();
    double means = 0.f;
    uint32_t frames = 0u;
    while (!glfwWindowShouldClose(main_window.window)) {
        glfwPollEvents();

        Simple::VKFW::Func::WaitForFences(logic_device, {fen_in_flight});
        Simple::VKFW::Func::ResetFences(logic_device, {fen_in_flight});

        tp = clock.now();

        uint32_t image_index = Simple::VKFW::Func::AcquireNextImageKHR(logic_device, swapchain, sem_image_available);

        Simple::VKFW::Func::ResetCommandBuffer(command_buffer);

        // Record command buffer
        {
            // Begin recording command buffer
            Simple::VKFW::Func::BeginCommandBuffer(command_buffer);

            // Begin render pass
            {
                Simple::VKFW::Cmd::BeginRenderPass cmd_begin_render_pass{command_buffer, render_pass};
                cmd_begin_render_pass.setSubpassContents(VK_SUBPASS_CONTENTS_INLINE);
                cmd_begin_render_pass.setFramebuffer(swapchain_fbuffers.handlerGet(image_index));
                cmd_begin_render_pass.addBlackColor();
                cmd_begin_render_pass.setRenderArea(swapchain_extent);

                cmd_begin_render_pass.cmdCall();
            }

            // Bind pipeline
            Simple::VKFW::Cmd::BindPipeline::CmdCall(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

            // Set viewport
            {
                Simple::VKFW::Viewport::StructWrap viewport{swapchain_extent};
                Simple::VKFW::Cmd::SetViewport::CmdCall(command_buffer, {viewport.viewport});
            }

            // Set scissors
            {
                Simple::VKFW::Rect2D::StructWrap scissor{swapchain_extent};
                Simple::VKFW::Cmd::SetScissor::CmdCall(command_buffer, {scissor.rectangle});
            }

            // Draw
            Simple::VKFW::Cmd::Draw::CmdCall(command_buffer, 3, 1);

            // End render pass
            Simple::VKFW::Cmd::EndRenderPass::CmdCall(command_buffer);

            // End recording command buffer
            Simple::VKFW::Func::EndCommandBuffer(command_buffer);
        }

        // Submit
        {
            Simple::VKFW::SubmitInfo::InfoHandler submit_info;
            {
                Simple::VKFW::SubmitInfo::Builder submit_info_builder{{command_buffer}, {sem_image_available}, {sem_render_finished}};
                submit_info_builder.setWaitStages({VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});
                submit_info = submit_info_builder.makeInfoStruct();
            }
            Simple::VKFW::Func::QueueSubmit(graphics_queue, {submit_info.submit_info}, {fen_in_flight});
        }

        // Present
        {
            Simple::VKFW::PresentInfoKHR::InfoHandler pres_info;
            {
                Simple::VKFW::PresentInfoKHR::Builder pres_info_builder{{sem_render_finished}, {swapchain}};
                pres_info_builder.setImageIndices({image_index});
                pres_info = pres_info_builder.makeInfoStruct();
            }
            Simple::VKFW::Func::QueuePresentKHR(pres_queue, pres_info);
        }
        means += (clock.now() - tp).count() * 1e-9;
        ++frames;
    }

    printf("Avg FPS without fences: %f\n", 1.f / (means / frames));

    Simple::VKFW::Func::WaitForFences(logic_device, {fen_in_flight});
    Simple::VKFW::Func::ResetFences(logic_device, {fen_in_flight});
    Simple::VKFW::Func::ResetCommandBuffer(command_buffer);

    printf("Success\n");
    return 0;
}