#include "main/event_handler.h"
#include "math/linalg.h"
#include "main/camera.h"
#include "main/input_handler.h"
#include "main/image.h"
#include "main/imgui_events.h"
#include "main/music_editor.h"
#include "main/mmd_editor.h"
#include "main/mmd_physics.h"
#include "interface/vkfw.h"
#include "interface/vkfw_graph.h"
#include "interface/imgui_wrap.h"
#include "scene/scene_graph.h"
#include "scene/scene_culling.h"
#include "scene/scene_renderer.h"
#include "scene/scene_markers.h"

#include <cmath>
#include <iostream>


int main(int argc, char **argv) {
    Simple::ArgHandler arg_handler{argc, argv};
    using Simple::VKFW::Graph::FrameCapture;
    namespace Graph = Simple::VKFW::Graph;
    namespace SG = Simple::SceneGraph;

// Config
    const uint32_t FRAMES_IN_FLIGHT = 4u;

// Scene info

    // Window icon (RGBA8, loaded with stb_image)
    Simple::Img::Image4u icon_image("resources/textures/checker.png");

    // Camera
    const Simple::vec2u fb_size = {1600u,  900u};
    const float fov = 33.f, p_near = 0.1f, p_far = 1000.f;
    const Simple::vec3 cam_pos = {-2.f,2.f,-2.f}, look_at = {0.f,0.f,0.f}, up_vect = {0.f,1.f,0.f};

    Simple::WindowInput::CameraControlsCentered main_view_centered_proj{Simple::ProjPerspective{fb_size.cast<float>(), fov, p_near, p_far},
                                                                        Simple::ViewCentered{cam_pos, look_at, up_vect}};

    // Scene graph: models are added here (e.g. 'SG::loadModel', see 'scene/scene_pmx.h'), lights added by hand
    SG::Scene scene;
    scene.ambient = {0.35f, 0.35f, 0.4f};
    // Key light from the front-left above, a warm point light behind, a pink spot from above
    scene.addLight(SG::Light::directional({1.f, 0.97f, 0.92f}, 0.9f), scene.addNode("sun", SG::SCENE_NONE, SG::lookAlong({0.5f, -0.8f, 0.6f})));
    scene.addLight(SG::Light::point({1.f, 0.7f, 0.4f}, 400.f, 80.f), scene.addNode("back_light", SG::SCENE_NONE, SG::translation({0.f, 25.f, 20.f})));
    scene.addLight(SG::Light::spot({1.f, 0.3f, 0.8f}, 3000.f, 0.25f, 0.4f, 200.f),
                   scene.addNode("stage_spot", SG::SCENE_NONE, SG::lookAlong({0.f, -1.f, 0.4f}) & SG::translation({0.f, 60.f, -40.f})));
    scene.updateTransforms();


// Input handlers

    // Note: layers from the top: ImGui (built in), screenshots (targets are set with the render graph), window controls, camera.
    //       The keys of each layer come from its input map in 'resources/input_maps'
    Simple::WindowInput::FrameInputHandlerGLFW main_window_handler;
    main_view_centered_proj.loadInputMap("resources/input_maps/camera.json");

    // Window
    Simple::GLFWInitClass glfw_init;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // for vk only, use context for OpenGL
    Simple::Window main_window{fb_size.x, fb_size.y, "Test"};
    main_window.setIcon(icon_image);

    Simple::WindowInput::DefaultHandler main_window_controls{main_window.window};
    main_window_controls.loadInputMap("resources/input_maps/window.json");
    Simple::WindowInput::ScreenshotHandler screenshots;
    screenshots.loadInputMap("resources/input_maps/screenshots.json");
    main_window_handler.pushLayer(&screenshots);
    main_window_handler.pushLayer(&main_window_controls);
    main_window_handler.pushLayer(&main_view_centered_proj);

    // ImGUI widgets
    main_window_handler.imgui_handler.setFont("resources/fonts/RobotoMono-VariableFont_wght.ttf", 24.f);
    // Japanese glyphs (MMD bone and morph names) from a system font, if there is one
    main_window_handler.imgui_handler.mergeFont({ "resources/fonts/japanese.ttf", "C:/Windows/Fonts/meiryo.ttc", "C:/Windows/Fonts/YuGothM.ttc",
                                                  "C:/Windows/Fonts/msgothic.ttc", "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
                                                  "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc", "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc" }, 24.f);
    // Simple::Event::ImGuiTestWindow *test_window = new Simple::Event::ImGuiTestWindow{};
    // main_window_handler.imgui_handler.widgets.push_back(test_window);

    // Music: instrument library, song and tracker windows; its keys go right after ImGui, above the camera
    Simple::Editor::MusicStudio music_studio;
    music_studio.startAudio();
    main_window_handler.insertLayer(0u, &music_studio.keys);
    main_window_handler.imgui_handler.widgets.push_back(&music_studio);

    // Camera settings: "Controls" menu of the main menu bar (movement speed, distance, fovy bounds of the scroll, cursor freezing)
    Simple::WindowInput::CameraSettingsWidget camera_settings{&main_view_centered_proj, &main_view_centered_proj.getClassProj(),
                                                              &main_view_centered_proj.getClassView(), &main_window_handler.imgui_handler};
    main_window_handler.imgui_handler.widgets.push_back(&camera_settings);

    // MMD: models, motions, camera and audio with a keyframe timeline (menu "MMD"); the audio track plays through the music stream
    Simple::Editor::MMDStudio mmd_studio(scene);
    if (music_studio.audio_running)
        mmd_studio.attachAudio(&music_studio.audio.stream_config.audio_sampler, music_studio.audio.stream_config.sample_rate);
    main_window_handler.imgui_handler.widgets.push_back(&mmd_studio);
    // MMD physics (rigid bodies and joints of the models; "MMD" -> "Physics" for its settings)
    Simple::Editor::MMDPhysicsWidget mmd_physics;
    mmd_studio.simulation = &mmd_physics;
    main_window_handler.imgui_handler.widgets.push_back(&mmd_physics);

    // Callbacks
    main_window_handler.setGLFWCallbacks(main_window.window);


////  VKFW

    // Instance, physical device, device with the queues, surface and swapchain (see 'Graph::DeviceSetup')
    Simple::VKFW::VulkanContext vk;
    // Device features: 'features' are required ('createDevice' throws with the names of the unsupported ones),
    //                  'optional_features' are enabled if the device supports them
    // Note: Vulkan 1.0 features are in 'head().features'; the features of the profile are enabled by 'createDevice'
    vk.cxt_core.device.create_info.features.get<VkPhysicalDeviceVulkan12Features>().timelineSemaphore = VK_TRUE;
    // Note: without it 'createSampler' turns anisotropic filtering off
    vk.cxt_core.device.create_info.optional_features.head().features.samplerAnisotropy = VK_TRUE;

    Graph::DeviceSetup device_setup;
    // Note: dynamic rendering + synchronization2 + extended dynamic state need Vulkan 1.3; with all set to false rendering goes
    //       through VkRenderPass with the original barriers (Vulkan 1.1)
    device_setup.profile.dynamic_rendering      = true;
    device_setup.profile.synchronization2       = true;
    device_setup.profile.extended_dynamic_state = true;
    device_setup.image_count = FRAMES_IN_FLIGHT;
    // Note: TRANSFER_SRC lets the frame be read back (see 'RenderGraph::readback')
    device_setup.image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    const Graph::DeviceQueues queues = Graph::setupDevice(vk, main_window, device_setup);


// Render graph:
    // Note: the scene pass draws into the swapchain image with a depth/stencil buffer, the UI pass draws ImGui over it
    //       (without depth). The graph places the barriers and begins/ends rendering (dynamic rendering or a render pass
    //       of each pass, by the profile); the depth/stencil texture is recreated with the swapchain size by 'compile'
    const VkClearValue clear_color = {{{0.f, 0.f, 0.f, 1.f}}};
    VkClearValue clear_depth_stencil;
    clear_depth_stencil.depthStencil = {1.f, 0u};

    // Depth/stencil format: D24_UNORM_S8_UINT if the device has it (otherwise D32_SFLOAT_S8_UINT), readable by transfers
    const VkFormat depth_stencil_format = Simple::VKFW::Util::pickDepthStencilFormat(vk.cxt_core.phys_device.handle, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);
    Graph::TextureDesc depth_stencil_desc(depth_stencil_format);
    depth_stencil_desc.extra_usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // for 'RenderGraph::readback'

    Graph::RenderGraph graph(vk);
    Graph::ResourceId  backbuffer    = graph.importSwapchain();
    Graph::ResourceId  depth_stencil = graph.createTexture("depth_stencil", depth_stencil_desc);
    // Note: STORE keeps the depth and stencil after the pass, so they can be read back
    Graph::PassId      scene_pass = graph.addGraphicsPass("scene").color(backbuffer, VK_ATTACHMENT_LOAD_OP_CLEAR, clear_color)
                                                                  .depth(depth_stencil, VK_ATTACHMENT_LOAD_OP_CLEAR, clear_depth_stencil);
    Graph::PassId      ui_pass    = graph.addGraphicsPass("ui").color(backbuffer, VK_ATTACHMENT_LOAD_OP_LOAD);

    graph.compile(); // after the swapchain creation and after every recreation

    // Screenshots (PNG in 'screenshots/'): the scene without ImGui, the whole frame, depth, stencil
    // Note: the copy is made in the next frame and the file is written when its fence is waited (see the main loop)
    screenshots.setTarget(Simple::WindowInput::Actions::SCREENSHOT_SCENE,   FrameCapture::VKFW_CAPTURE_COLOR,   scene_pass, backbuffer);
    screenshots.setTarget(Simple::WindowInput::Actions::SCREENSHOT_FRAME,   FrameCapture::VKFW_CAPTURE_COLOR,   ui_pass,    backbuffer);
    screenshots.setTarget(Simple::WindowInput::Actions::SCREENSHOT_DEPTH,   FrameCapture::VKFW_CAPTURE_DEPTH,   scene_pass, depth_stencil);
    screenshots.setTarget(Simple::WindowInput::Actions::SCREENSHOT_STENCIL, FrameCapture::VKFW_CAPTURE_STENCIL, scene_pass, depth_stencil);


// Scene objects:
    // Scene renderer: copies the scene to the device and makes its pipelines for the scene pass; draws what the culling sees
    SG::SceneRenderer scene_renderer(vk);
    SG::SceneCulling  scene_culling;
    scene_renderer.culling = &scene_culling;
    SG::SceneRenderer::UploadInfo scene_upload;
    scene_upload.queue  = queues.graphics;
    scene_upload.family = queues.graphics_family;
    scene_upload.frames_in_flight = FRAMES_IN_FLIGHT;
    scene_culling.build(scene);
    scene_renderer.upload(scene, scene_upload);
    scene_renderer.createPipelines(graph, scene_pass);

    // The point the camera looks at, shown while a mouse button moves the camera
    SG::LookAtMarker look_at_marker;
    look_at_marker.create(vk, graph, scene_pass);
    camera_settings.marker_size = &look_at_marker.size; // "Controls" -> "Camera"
    camera_settings.marker_size_default = look_at_marker.size;

    // Frames in flight: command buffers, semaphores and fences; acquire, submit, present, swapchain recreation
    Graph::FrameLoop frames(vk, graph, queues, FRAMES_IN_FLIGHT);

    // ImGUI init
    main_window_handler.imgui_handler.initializeContext(vk, queues.graphics_family, queues.graphics, FRAMES_IN_FLIGHT, graph.renderPass(ui_pass));
    main_window_handler.imgui_handler.enableForSVKFWWindow(main_window, false);


// Main loop:
    // Camera of the frame (the VMD camera of the MMD studio replaces the engine camera if it is on)
    Simple::mat4 view, proj;
    bool show_marker = false;

    // Commands of the scene pass (recorded inside its rendering, viewport and scissor are set by the graph)
    graph.pass(scene_pass).execute([&](Simple::VKFW::CommandRecorder &rec) {
        scene_renderer.record(rec);
        if (show_marker)
            look_at_marker.record(rec, main_view_centered_proj.getClassView().look_at, view, proj);
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

        if (!frames.begin()) continue; // the swapchain was out of date (recreated) or not ready
        screenshots.save(frames.frame); // the copy of this frame in flight (if any) is ready after its fence

        // MMD playback (poses, the VMD camera); a loaded or removed model needs a new upload of the scene (the device is idle)
        mmd_studio.update(main_window_handler.frame_time);
        if (mmd_studio.takeSceneChanged()) {
            vkDeviceWaitIdle(vk.cxt_core.device.handle);
            scene_renderer.destroy();
            scene_culling.build(scene);
            scene_renderer.upload(scene, scene_upload);
            scene_renderer.createPipelines(graph, scene_pass);
        }

        // Camera and scene data of this frame in flight (the device doesn't use them after the fence wait)
        view = main_view_centered_proj.getView();
        proj = main_view_centered_proj.getProj();
        const bool vmd_camera = mmd_studio.applyCamera(main_view_centered_proj.getClassProj(), view, proj);
        show_marker = !vmd_camera && (main_view_centered_proj.cursorRotates() || main_view_centered_proj.cursorPans() ||
                                      main_view_centered_proj.cursorDollies());
        scene.updateSkins(); // the poses changed since the last frame (animations: see 'scene/scene_animation.h')
        scene_renderer.update(frames.frame, view, proj);

        // Record: ImGui, a requested screenshot (copied by the graph), the passes; then submit and present
        main_window_handler.imgui_handler.newFrame();
        main_window_handler.imgui_handler.render();
        screenshots.record(graph, frames.frame);
        // Note: 'RenderGraph::readbackAfter' (as the screenshots use it) copies a resource after a pass into an 'ImageReadback',
        //       readable after the fence of this frame
        frames.end();
    } // main loop END

    frames.destroy(); // waits for the device
    screenshots.destroy(); // writes the last one, the device is idle

    // ImGUI widgets destruction
    main_window_handler.imgui_handler.destroyContext();

    // Destruction
    look_at_marker.destroy();
    scene_renderer.destroy();
    graph.destroy();
    vk.destroyContext();
    main_window.destroy();
    glfw_init.terminate();

    printf(SVKFW_WRAPINFO("main", "Success\n"));
    return 0;
}
