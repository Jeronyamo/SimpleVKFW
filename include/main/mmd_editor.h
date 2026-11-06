#ifndef SVKFW_MMD_EDITOR_H
#define SVKFW_MMD_EDITOR_H

#include <atomic>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "interface/imgui_wrap.h"
#include "data/filetypes.h"
#include "main/camera.h"
#include "scene/scene_graph.h"
#include "scene/scene_animation.h"
#include "sound/sampler.h"


namespace Simple {
    namespace Editor {
        namespace SG = SceneGraph;

//  == === ==== ================================== ==== === ==  \\
                    MMD editor
//  == === ==== ================================== ==== === ==  \\

        // Note: the "MMD" menu of the main menu bar: a scene window (models with their offsets and motions, camera motion,
        //       audio, the VMD processing tools), a timeline (playback and the keyframes of the selected model or camera) and
        //       a keyframe window (edits the keys at the current frame and their interpolation curves, like MMD).
        //       Loaded VMD files stay as 'File::ContentVMD' (all edits and processing change it, then the animation clips are
        //       rebuilt from it), so a modified motion is saved as a VMD again.
        //
        //       MMDStudio mmd(scene);                                    // after the lights are added (kept on rebuilds)
        //       mmd.attachAudio(&audio_handler.stream_config.audio_sampler, sample_rate); // optional, plays the audio track
        //       imgui_handler.widgets.push_back(&mmd);
        //       ...
        //       mmd.update(frame_time);                                  // each frame, before 'scene.updateSkins'
        //       if (mmd.takeSceneChanged()) { ... re-upload the scene renderer, rebuild the culling ... }
        //       mmd.applyCamera(camera.proj_class, view, proj);          // the VMD camera, if one is loaded and on

        // Physics (or any other simulation) driven by the MMD playback
        struct MMDSimulationItf {
            virtual ~MMDSimulationItf();
            // After a model with a skin was added to the scene
            virtual void addModel(SG::Scene &_scene, uint32_t _skin) = 0;
            // After a jump in time (seek, stop, edits, offset changes): back to the animated pose
            virtual void reset(SG::Scene &_scene) = 0;
            // Before the scene is rebuilt (a model was removed); 'addModel' follows for each remaining model
            virtual void clear() = 0;
            // Each frame after the animations are applied ('_dt': seconds of playback, 0 when paused)
            virtual void update(SG::Scene &_scene, float _dt) = 0;
        }; // MMDSimulationItf END


        // Audio track of the timeline: stereo 16-bit PCM played from 'position' while 'playing' (the timeline seeks it).
        // Note: 'sample2' runs in the audio thread (with the sampler's mutex locked), the rest in the main thread
        struct MMDAudioTrack : Audio::Sample::SmpItf {
            std::vector<int16_t>  pcm;              // interleaved stereo
            uint32_t              sample_rate = 48000u;
            uint32_t              output_rate = 48000u;
            std::atomic<bool>     playing;
            std::atomic<uint64_t> position;         // source frames, 32.32 fixed point
            std::atomic<float>    volume;

            MMDAudioTrack();
            virtual ~MMDAudioTrack() override;

            // .ogg (Opus) or .wav; returns false and sets '_error' on failure
            static bool decode(const std::string &_path, std::vector<int16_t> &_pcm, uint32_t &_sample_rate, std::string &_error);

            double length() const;  // seconds
            double time() const;    // seconds
            void   seek(double _seconds);

            virtual float   sample (float  _t) override;
            virtual vec2i16 sample2(double _t) override;
        }; // MMDAudioTrack END


        // A loaded VMD: the file data (edited) and the clips made from it
        struct MMDMotion {
            std::string      path, name;
            File::ContentVMD vmd;
            bool             loaded = false, modified = false;
            SG::AnimationClip clip;
            SG::CameraClip    camera;
            // Keyframe indices of each track in 'vmd' (sorted by frame), by the raw (Shift-JIS) name
            std::map<std::string, std::vector<uint32_t>> bone_keys, morph_keys;
            // Undo / redo: snapshots of 'vmd' before each edit
            std::vector<File::ContentVMD> undo_stack, redo_stack;

            bool load(const std::string &_path, std::string &_error);
            bool save(const std::string &_path, std::string &_error);
            void clear();
            // After 'vmd' changed: the clips and the key indices
            void rebuild();
            // Call before changing 'vmd' (pushes the undo snapshot, marks it modified)
            void beginEdit();
            bool undo();
            bool redo();
            float length() const; // the last key frame
        }; // MMDMotion END


        struct MMDModel {
            std::string path, name;
            uint32_t    offset_node = SG::SCENE_NONE; // the node of the offsets, the model's root is its child
            uint32_t    root = SG::SCENE_NONE;
            uint32_t    skin = SG::SCENE_NONE;
            vec3        position = {0.f, 0.f, 0.f};
            vec3        rotation = {0.f, 0.f, 0.f};   // degrees: pitch (X), yaw (Y), roll (Z)
            float       scale = 1.f;
            bool        visible = true;
            MMDMotion   motion;
            SG::AnimationBinding binding;
            // The VMD names (Shift-JIS, cut to 15 bytes) of the bones and morphs of the skeleton
            std::vector<std::string> bone_raw, morph_raw;

            mat4 offsetMatrix() const;
        }; // MMDModel END


        struct MMDStudio : ImGUI::WidgetItf {
            enum TrackType { TRACK_NONE = 0, TRACK_BONE, TRACK_MORPH, TRACK_CAMERA };

            // A row of the timeline
            struct Track {
                TrackType   type = TRACK_NONE;
                uint32_t    index = SG::SCENE_NONE;  // bone / morph of the skeleton
                std::string raw;                     // the VMD name (Shift-JIS, cut to the VMD field)
            };

        // Scene
            SG::Scene              &scene;
            SG::Scene               base_scene;      // the scene without the MMD models (rebuilds start from it)
            std::vector<MMDModel>   models;
            MMDMotion               camera_motion;
            bool                    use_camera = true;
            MMDSimulationItf       *simulation = nullptr;

        // Audio
            MMDAudioTrack           audio;
            std::string             audio_path;
            Audio::AudioSampler    *sampler = nullptr;
            float                   audio_offset = 0.f; // the frame where the audio starts

        // Playback (frames, 30 per second like MMD)
            float                   frame = 0.f;
            bool                    playing = false, loop = false;
            float                   fps = 30.f;
            double                  play_clock = 0.;    // seconds since the start of playback (without audio)
            float                   play_start = 0.f;

        // UI state
            bool show_scene = true, show_timeline = true, show_keyframe = true;
            char model_path[512]{}, motion_path[512]{}, camera_path[512]{}, audio_path_buf[512]{}, save_path[512]{};
            std::vector<std::string> asset_models, asset_motions, asset_audio;
            std::string asset_dir = "resources/assets";
            int32_t  selected_model = -1;     // -1: the camera is the timeline target
            bool     target_camera = false;
            bool     all_bones = false, all_morphs = false;
            std::vector<Track>        tracks;
            std::vector<ImGuiCustom::TimelineRow> rows;
            ImGuiCustom::TimelineState timeline;
            TrackType sel_type = TRACK_NONE;  // the selected track (it is in 'tracks' even without keys)
            uint32_t  sel_index = SG::SCENE_NONE;
            int32_t  selected_track = -1;     // in 'tracks'
            bool     tracks_dirty = true;
            bool     curve_dragging = false;
            char     bone_filter[64]{};
            int32_t  curve_channel = 0;
            const MMDMotion *save_path_motion = nullptr; // the motion 'save_path' was filled for
            std::string status;
            std::string confirm_text;
            std::function<void()> confirm_action;

        // Key edit buffer (the values shown in the keyframe window; previewed until registered or reset)
            bool  edit_active = false;        // the buffer differs from the animation and is shown
            vec3  edit_translation = {0.f, 0.f, 0.f};
            vec3  edit_rotation = {0.f, 0.f, 0.f}; // degrees
            float edit_weight = 0.f;
            SG::CameraPose edit_camera;
            int32_t edit_track = -1;
            float   edit_frame = -1.f;

            bool scene_changed = false;       // the renderer must upload the scene again
            bool pose_changed  = true;        // the poses must be applied again (seek, edit)

            explicit MMDStudio(SG::Scene &_scene);
            virtual ~MMDStudio() override;

            // Plays the audio track through '_sampler' (e.g. the one of the music editor's stream)
            void attachAudio(Audio::AudioSampler *_sampler, uint32_t _output_rate);

            // Playback, animations, simulation; call each frame before 'scene.updateSkins'
            void update(double _dt);
            // True once after the scene got new meshes, materials or skins (the renderer must upload it again)
            bool takeSceneChanged();
            // With a camera motion loaded and on: the view and the projection (the fov of the VMD; near, far and the size from '_base')
            bool applyCamera(const ProjPerspective &_base, mat4 &_view, mat4 &_proj) const;

            bool loadModel(const std::string &_path);
            void removeModel(uint32_t _model);
            bool loadMotion(uint32_t _model, const std::string &_path);
            bool loadCamera(const std::string &_path);
            bool loadAudio(const std::string &_path);
            void seek(float _frame);
            void play();
            void stop();
            float length() const; // frames: the longest motion or the audio

            virtual bool      begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;

        // Windows
            void windowScene();
            void windowTimeline();
            void windowKeyframe();

        // Helpers
            MMDMotion *targetMotion_();
            void scanAssets_();
            void rebuildScene_();
            void applyOffsets_(MMDModel &_model);
            void bindMotion_(MMDModel &_model, bool _warn);
            void motionChanged_(MMDMotion &_motion);
            void buildTracks_();
            void select_(TrackType _type, uint32_t _index);
            const Track *selectedTrack_() const;
            MMDModel *selectedModel_();
            void refreshEdit_();
            void applyEdit_();
            void registerKey_();
            void deleteKey_();
            int32_t keyAt_(const Track &_track, uint32_t _frame) const; // index in the VMD keyframe vector, -1 if none
            void moveKey_(const Track &_track, uint32_t _from, uint32_t _to);
            void curveEditor_();
            void confirm_(const std::string &_text, const std::function<void()> &_action);
        }; // MMDStudio END
    }; // Editor END
}; // Simple END

#endif
