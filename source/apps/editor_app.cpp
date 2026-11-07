#include "apps/editor_app.h"

#include "math/linalg.h"
#include "main/camera.h"
#include "main/input_handler.h"
#include "main/image.h"
#include "editor/imgui_events.h"
#include "editor/music_editor.h"
#include "editor/mmd_editor.h"
#include "editor/mmd_physics.h"
#include "interface/vkfw.h"
#include "interface/vkfw_graph.h"
#include "interface/imgui_wrap.h"
#include "scene/scene_graph.h"
#include "scene/scene_culling.h"
#include "scene/scene_renderer.h"
#include "scene/scene_markers.h"
#include "systems/physics.h"
#include "systems/physics_scene.h"

#include <cmath>
#include <iostream>
#include <memory>


namespace Simple {
    namespace Apps {
        namespace Graph = VKFW::Graph;
        namespace SG = SceneGraph;
        using VKFW::Graph::FrameCapture;

        namespace {
        // Config
            const uint32_t FRAMES_IN_FLIGHT = 4u;

        // Camera
            const vec2u fb_size = {1600u,  900u};
            const float fov = 33.f, p_near = 0.1f, p_far = 1000.f;
            const vec3  cam_pos = {-2.f,2.f,-2.f}, look_at = {0.f,0.f,0.f}, up_vect = {0.f,1.f,0.f};

            // Note: the window has no OpenGL context (Vulkan only); set before the window is made
            struct NoClientApiHint {
                NoClientApiHint() { glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); }
            };
        } // anonymous namespace END


//  == === ==== ================================== ==== === ==  \\
                    Editor state
//  == === ==== ================================== ==== === ==  \\

        // Note: the members are made in this order and destroyed in the reverse one ('destroy' first frees the Vulkan objects)
        struct EditorApp::State {
            Core::Engine &engine;

        // Scene info
            // Window icon (RGBA8, loaded with stb_image)
            Img::Image4u icon_image;

            WindowInput::CameraControlsCentered main_view_centered_proj{ProjPerspective{fb_size.cast<float>(), fov, p_near, p_far},
                                                                        ViewCentered{cam_pos, look_at, up_vect}};

            // Scene graph: models are added here (e.g. 'SG::loadModel', see 'scene/scene_pmx.h'), lights added by hand
            SG::Scene scene;

        // Input handlers
            // Note: layers from the top: ImGui (built in), screenshots (targets are set with the render graph), window controls, camera.
            //       The keys of each layer come from its input map in 'resources/input_maps'
            WindowInput::FrameInputHandlerGLFW main_window_handler;

        // Window
            GLFWInitClass   glfw_init;
            NoClientApiHint no_client_api;
            Window          main_window{fb_size.x, fb_size.y, "Test"};

            WindowInput::DefaultHandler    main_window_controls{main_window.window};
            WindowInput::ScreenshotHandler screenshots;

        // Widgets
            // Music: instrument library, song and tracker windows; its keys go right after ImGui, above the camera
            Editor::MusicStudio music_studio;
            // Camera settings: "Controls" menu of the main menu bar (movement speed, distance, fovy bounds of the scroll, cursor freezing)
            WindowInput::CameraSettingsWidget camera_settings{&main_view_centered_proj, &main_view_centered_proj.getClassProj(),
                                                              &main_view_centered_proj.getClassView(), &main_window_handler.imgui_handler};
            // MMD: models, motions, camera and audio with a keyframe timeline (menu "MMD"); the audio track plays through the music stream
            Editor::MMDStudio mmd_studio{scene};

        // Systems: the physics world in the engine's components; the bodies of the scene models (rigid bodies and joints of
        //          PMX models, colliders of other models), told about the models by the scene events of the MMD studio
            Physics::World          physics_world{engine.ecs};
            Physics::SceneBodies    scene_bodies{scene, physics_world};
            Editor::MMDPhysicsWidget physics_widget{&physics_world, &scene_bodies}; // "MMD" -> "Physics"

        // VKFW
            // Instance, physical device, device with the queues, surface and swapchain (see 'Graph::DeviceSetup')
            VKFW::VulkanContext vk;
            Graph::DeviceQueues queues;

        // Render graph
            Graph::RenderGraph graph{vk};
            Graph::ResourceId  backbuffer, depth_stencil;
            Graph::PassId      scene_pass, ui_pass;

        // Scene objects
            SG::SceneRenderer scene_renderer{vk};
            SG::SceneCulling  scene_culling;
            SG::SceneRenderer::UploadInfo scene_upload;
            SG::LookAtMarker  look_at_marker;
            std::unique_ptr<Graph::FrameLoop> frames_;

        // Frame
            // Camera of the frame (the VMD camera of the MMD studio replaces the engine camera if it is on)
            mat4 view, proj;
            bool show_marker = false;
            Event::SubscriptionID scene_changed_sub = 0u;
            bool destroyed = false;

            explicit State(Core::Engine &_engine) : engine(_engine), icon_image("resources/textures/checker.png") {}

            void start();
            bool frame();
            void destroy();
            void uploadScene();
        }; // EditorApp::State END


        void EditorApp::State::start() {
            // Lights: key light from the front-left above, a warm point light behind, a pink spot from above
            scene.ambient = {0.35f, 0.35f, 0.4f};
            scene.addLight(SG::Light::directional({1.f, 0.97f, 0.92f}, 0.9f), scene.addNode("sun", SG::SCENE_NONE, SG::lookAlong({0.5f, -0.8f, 0.6f})));
            scene.addLight(SG::Light::point({1.f, 0.7f, 0.4f}, 400.f, 80.f), scene.addNode("back_light", SG::SCENE_NONE, SG::translation({0.f, 25.f, 20.f})));
            scene.addLight(SG::Light::spot({1.f, 0.3f, 0.8f}, 3000.f, 0.25f, 0.4f, 200.f),
                           scene.addNode("stage_spot", SG::SCENE_NONE, SG::lookAlong({0.f, -1.f, 0.4f}) & SG::translation({0.f, 60.f, -40.f})));
            scene.updateTransforms();
            // Note: the MMD studio keeps the scene without its models for rebuilds: it takes it again with the lights
            mmd_studio.base_scene = scene;

        // Input
            main_view_centered_proj.loadInputMap("resources/input_maps/camera.json");
            main_window.setIcon(icon_image);
            main_window_controls.loadInputMap("resources/input_maps/window.json");
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
            // Event::ImGuiTestWindow *test_window = new Event::ImGuiTestWindow{};
            // main_window_handler.imgui_handler.widgets.push_back(test_window);

            music_studio.startAudio();
            main_window_handler.insertLayer(0u, &music_studio.keys);
            main_window_handler.imgui_handler.widgets.push_back(&music_studio);
            main_window_handler.imgui_handler.widgets.push_back(&camera_settings);

            mmd_studio.events = &engine.events;
            if (music_studio.audio_running)
                mmd_studio.attachAudio(&music_studio.audio.stream_config.audio_sampler, music_studio.audio.stream_config.sample_rate);
            main_window_handler.imgui_handler.widgets.push_back(&mmd_studio);
            main_window_handler.imgui_handler.widgets.push_back(&physics_widget);

            // Callbacks
            main_window_handler.setGLFWCallbacks(main_window.window);

        // Systems (run by the phases of 'frame')
            engine.addSystem(&physics_world);
            engine.addSystem(&scene_bodies);


        ////  VKFW

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
            queues = Graph::setupDevice(vk, main_window, device_setup);


        // Render graph:
            // Note: the scene pass draws into the swapchain image with a depth/stencil buffer, the UI pass draws ImGui over it
            //       (without depth). The graph places the barriers and begins/ends rendering (dynamic rendering or a render pass
            //       of each pass, by the profile); the depth/stencil texture is recreated with the swapchain size by 'compile'
            const VkClearValue clear_color = {{{0.f, 0.f, 0.f, 1.f}}};
            VkClearValue clear_depth_stencil;
            clear_depth_stencil.depthStencil = {1.f, 0u};

            // Depth/stencil format: D24_UNORM_S8_UINT if the device has it (otherwise D32_SFLOAT_S8_UINT), readable by transfers
            const VkFormat depth_stencil_format = VKFW::Util::pickDepthStencilFormat(vk.cxt_core.phys_device.handle, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);
            Graph::TextureDesc depth_stencil_desc(depth_stencil_format);
            depth_stencil_desc.extra_usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // for 'RenderGraph::readback'

            backbuffer    = graph.importSwapchain();
            depth_stencil = graph.createTexture("depth_stencil", depth_stencil_desc);
            // Note: STORE keeps the depth and stencil after the pass, so they can be read back
            scene_pass = graph.addGraphicsPass("scene").color(backbuffer, VK_ATTACHMENT_LOAD_OP_CLEAR, clear_color)
                                                       .depth(depth_stencil, VK_ATTACHMENT_LOAD_OP_CLEAR, clear_depth_stencil);
            ui_pass    = graph.addGraphicsPass("ui").color(backbuffer, VK_ATTACHMENT_LOAD_OP_LOAD);

            graph.compile(); // after the swapchain creation and after every recreation

            // Screenshots (PNG in 'screenshots/'): the scene without ImGui, the whole frame, depth, stencil
            // Note: the copy is made in the next frame and the file is written when its fence is waited (see 'frame')
            screenshots.setTarget(WindowInput::Actions::SCREENSHOT_SCENE,   FrameCapture::VKFW_CAPTURE_COLOR,   scene_pass, backbuffer);
            screenshots.setTarget(WindowInput::Actions::SCREENSHOT_FRAME,   FrameCapture::VKFW_CAPTURE_COLOR,   ui_pass,    backbuffer);
            screenshots.setTarget(WindowInput::Actions::SCREENSHOT_DEPTH,   FrameCapture::VKFW_CAPTURE_DEPTH,   scene_pass, depth_stencil);
            screenshots.setTarget(WindowInput::Actions::SCREENSHOT_STENCIL, FrameCapture::VKFW_CAPTURE_STENCIL, scene_pass, depth_stencil);


        // Scene objects:
            // Scene renderer: copies the scene to the device and makes its pipelines for the scene pass; draws what the culling sees
            scene_renderer.culling = &scene_culling;
            scene_upload.queue  = queues.graphics;
            scene_upload.family = queues.graphics_family;
            scene_upload.frames_in_flight = FRAMES_IN_FLIGHT;
            scene_culling.build(scene);
            scene_renderer.upload(scene, scene_upload);
            scene_renderer.createPipelines(graph, scene_pass);
            // A loaded or removed model (queued by the MMD studio): a new upload of the scene, polled at the start of the frame
            scene_changed_sub = engine.events.subscribe(Event::EV_TYPE_SCENE_CHANGED, [this](Event::EventHandler &, const Event::Event &) {
                uploadScene();
            });

            // The point the camera looks at, shown while a mouse button moves the camera
            look_at_marker.create(vk, graph, scene_pass);
            camera_settings.marker_size = &look_at_marker.size; // "Controls" -> "Camera"
            camera_settings.marker_size_default = look_at_marker.size;

            // Frames in flight: command buffers, semaphores and fences; acquire, submit, present, swapchain recreation
            frames_.reset(new Graph::FrameLoop(vk, graph, queues, FRAMES_IN_FLIGHT));

            // ImGUI init
            main_window_handler.imgui_handler.initializeContext(vk, queues.graphics_family, queues.graphics, FRAMES_IN_FLIGHT, graph.renderPass(ui_pass));
            main_window_handler.imgui_handler.enableForSVKFWWindow(main_window, false);


        // Commands of the passes
            // The scene pass (recorded inside its rendering, viewport and scissor are set by the graph)
            graph.pass(scene_pass).execute([this](VKFW::CommandRecorder &rec) {
                scene_renderer.record(rec);
                if (show_marker)
                    look_at_marker.record(rec, main_view_centered_proj.getClassView().look_at, view, proj);
            });

            // The UI pass: ImGui over the scene
            graph.pass(ui_pass).execute([this](VKFW::CommandRecorder &rec) {
                main_window_handler.imgui_handler.fillCommandBuffer(rec.handle());
            });
        }

        void EditorApp::State::uploadScene() {
            vkDeviceWaitIdle(vk.cxt_core.device.handle);
            scene_renderer.destroy();
            scene_culling.build(scene);
            scene_renderer.upload(scene, scene_upload);
            scene_renderer.createPipelines(graph, scene_pass);
        }

        bool EditorApp::State::frame() {
            Graph::FrameLoop &frames = *frames_;
            Core::FrameTime time;
            time.frame = engine.frame();

            // Input: window events go through the layers of the input handler
            main_window_handler.frameTick();
            glfwPollEvents();
            main_view_centered_proj.manualFrameUpdate(main_window_handler.frame_time);
            time.dt = (float)main_window_handler.frame_time;
            engine.runPhase(Core::PHASE_INPUT, time);

            if (!frames.begin()) return !main_window.shouldClose(); // the swapchain was out of date (recreated) or not ready
            screenshots.save(frames.frame); // the copy of this frame in flight (if any) is ready after its fence

            // MMD playback (poses, the VMD camera); the simulation time follows the playback (0 while paused)
            mmd_studio.update(main_window_handler.frame_time);
            time.sim_dt = mmd_studio.sim_dt;
            // Simulations: kinematic targets from the animated pose, the physics step, bones and nodes from the bodies
            engine.runPhase(Core::PHASE_ANIMATION, time);
            engine.runPhase(Core::PHASE_PHYSICS,   time);
            engine.runPhase(Core::PHASE_SCENE,     time);

            // Camera and scene data of this frame in flight (the device doesn't use them after the fence wait)
            view = main_view_centered_proj.getView();
            proj = main_view_centered_proj.getProj();
            const bool vmd_camera = mmd_studio.applyCamera(main_view_centered_proj.getClassProj(), view, proj);
            show_marker = !vmd_camera && (main_view_centered_proj.cursorRotates() || main_view_centered_proj.cursorPans() ||
                                          main_view_centered_proj.cursorDollies());
            scene.updateSkins(); // the poses changed since the last frame (animations: see 'scene/scene_animation.h')
            engine.runPhase(Core::PHASE_PREPARE, time);
            scene_renderer.update(frames.frame, view, proj);

            // Record: ImGui, a requested screenshot (copied by the graph), the passes; then submit and present
            main_window_handler.imgui_handler.newFrame();
            main_window_handler.imgui_handler.render();
            screenshots.record(graph, frames.frame);
            engine.runPhase(Core::PHASE_RENDER, time);
            // Note: 'RenderGraph::readbackAfter' (as the screenshots use it) copies a resource after a pass into an 'ImageReadback',
            //       readable after the fence of this frame
            frames.end();
            return !main_window.shouldClose();
        }

        void EditorApp::State::destroy() {
            if (destroyed) return;
            destroyed = true;
            engine.events.unsubscribe(scene_changed_sub);
            engine.removeSystem(&scene_bodies);
            engine.removeSystem(&physics_world);
            mmd_studio.events = nullptr;

            if (frames_) frames_->destroy(); // waits for the device
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
        }


//  == === ==== ================================== ==== === ==  \\
                    Editor app
//  == === ==== ================================== ==== === ==  \\

        EditorApp::EditorApp() {}

        EditorApp::~EditorApp() { stop(); }

        bool EditorApp::start(Core::Engine &_engine) {
            if (state_) return true;
            state_ = new State(_engine);
            state_->start();
            return true;
        }

        bool EditorApp::frame() { return state_ && state_->frame(); }

        void EditorApp::stop() {
            if (!state_) return;
            state_->destroy();
            delete state_;
            state_ = nullptr;
        }
    }; // Apps END
}; // Simple END
