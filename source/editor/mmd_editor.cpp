#include "editor/mmd_editor.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <stdexcept>

#include "platform_dependent/pd_encoding.h"
#include "scene/scene_pmx.h"
#include "sound/player.h"


namespace Simple {
    namespace Editor {
        namespace {
            const float DEG = 3.14159265358979f / 180.f;
            const size_t VMD_BONE_NAME = 15u, VMD_MORPH_NAME = 15u;
            typedef File::ContentVMD VMD;

            std::string fileName(const std::string &_path) {
                const size_t __sep = _path.find_last_of("/\\");
                return __sep == std::string::npos ? _path : _path.substr(__sep + 1u);
            }

            std::string lowerExtension(const std::string &_path) {
                const size_t __dot = _path.find_last_of('.');
                std::string __ext = __dot == std::string::npos ? std::string() : _path.substr(__dot);
                for (size_t i = 0u; i < __ext.size(); ++i) __ext[i] = (char)std::tolower((unsigned char)__ext[i]);
                return __ext;
            }

            void copyPath(char *_dst, size_t _size, const std::string &_src) { snprintf(_dst, _size, "%s", _src.c_str()); }

            std::string trimmed(const char *_str) {
                std::string __s(_str);
                const size_t __b = __s.find_first_not_of(" \t\r\n\"");
                if (__b == std::string::npos) return std::string();
                const size_t __e = __s.find_last_not_of(" \t\r\n\"");
                return __s.substr(__b, __e - __b + 1u);
            }

            std::string rawName(const char *_field, size_t _max) { return std::string(_field, strnlen(_field, _max)); }

            void setRawName(char *_field, size_t _field_size, const std::string &_raw) {
                std::memset(_field, 0, _field_size);
                std::memcpy(_field, _raw.data(), std::min(_raw.size(), _field_size - 1u));
            }

            // MMD's linear curve (the AnimationCurve default)
            VMD::InterpolationInfo linearInterpolation() {
                VMD::InterpolationInfo __info;
                const vec4u8 __linear = { (uint8_t)20, (uint8_t)20, (uint8_t)107, (uint8_t)107 };
                __info.mov_x = __info.mov_y = __info.mov_z = __info.rot = __info.dist = __info.fov = __linear;
                return __info;
            }

            vec4u8 *channelOf(VMD::InterpolationInfo &_info, int _channel) {
                vec4u8 *__channels[6] = { &_info.mov_x, &_info.mov_y, &_info.mov_z, &_info.rot, &_info.dist, &_info.fov };
                return __channels[std::min(std::max(_channel, 0), 5)];
            }

            // Euler degrees (x - pitch, y - yaw, z - roll) <-> rotation
            SG::Quat eulerToQuat(const vec3 &_deg) {
                return Affine::Rotation::ypr2quaternion(_deg.y * DEG, _deg.x * DEG, _deg.z * DEG).normalize();
            }
            vec3 quatToEuler(const SG::Quat &_q) {
                const Affine::Rotation::EulerAngles __e = Affine::Rotation::quaternion2ypr(_q.normalize());
                return { __e.pitch / DEG, __e.yaw / DEG, __e.roll / DEG };
            }

            void initVMD(VMD &_vmd, const std::string &_model_name) {
                _vmd = VMD();
                std::memset(_vmd.header, 0, sizeof(_vmd.header));
                std::memcpy(_vmd.header, "Vocaloid Motion Data 0002", 25u);
                setRawName(_vmd.model, sizeof(_vmd.model), Enc::utf8ToShiftJIS(_model_name, sizeof(_vmd.model) - 1u));
                _vmd.is_old_format = false;
            }

            const char *const CHANNEL_NAMES[6] = { "X", "Y", "Z", "Rotation", "Distance", "FOV" };
        } // anonymous


//  == === ==== >  MMDAudioTrack

        MMDAudioTrack::MMDAudioTrack() : playing(false), position(0u), volume(1.f) {}

        MMDAudioTrack::~MMDAudioTrack() {}

        bool MMDAudioTrack::decode(const std::string &_path, std::vector<int16_t> &_pcm, uint32_t &_sample_rate, std::string &_error) {
            _pcm.clear();
            const std::string __ext = lowerExtension(_path);
            if (__ext == ".wav") {
                File::ReaderWriterWAV __wav;
                if (!__wav.read(_path)) { _error = "can't read the WAV file"; return false; }
                std::vector<float> __samples;
                if (!__wav.file_content.getSamples(__samples) || __wav.file_content.channels == 0u) { _error = "unsupported WAV format"; return false; }
                const uint32_t __ch = __wav.file_content.channels;
                const size_t __frames = __samples.size() / __ch;
                _pcm.resize(__frames * 2u);
                for (size_t i = 0u; i < __frames; ++i)
                    for (uint32_t c = 0u; c < 2u; ++c) {
                        const float __v = __samples[i * __ch + std::min(c, __ch - 1u)];
                        _pcm[2u * i + c] = (int16_t)std::lround(std::min(std::max(__v, -1.f), 1.f) * 32767.f);
                    }
                _sample_rate = __wav.file_content.sample_rate;
                return true;
            }
            if (__ext == ".ogg" || __ext == ".opus") {
                Audio::Sample::SmpAudioFile __ogg(_path);
                const uint32_t __ch = __ogg.opus_head.channels;
                if (__ch == 0u) { _error = "no Opus audio in the file"; return false; }
                const size_t __frames = __ogg.pcm_out.size() / __ch;
                _pcm.resize(__frames * 2u);
                for (size_t i = 0u; i < __frames; ++i)
                    for (uint32_t c = 0u; c < 2u; ++c) _pcm[2u * i + c] = __ogg.pcm_out[i * __ch + std::min(c, __ch - 1u)];
                _sample_rate = 48000u; // Opus is decoded at 48 kHz
                return true;
            }
            _error = "only .ogg (Opus) and .wav are supported";
            return false;
        }

        double MMDAudioTrack::length() const { return sample_rate ? double(pcm.size() / 2u) / sample_rate : 0.; }

        double MMDAudioTrack::time() const { return sample_rate ? double(position.load()) / 4294967296. / sample_rate : 0.; }

        void MMDAudioTrack::seek(double _seconds) {
            position.store(uint64_t(std::max(_seconds, 0.) * sample_rate * 4294967296.));
        }

        float MMDAudioTrack::sample(float) {
            const vec2i16 __s = sample2(0.);
            return (float(__s.x) + float(__s.y)) * 0.5f;
        }

        vec2i16 MMDAudioTrack::sample2(double) {
            if (!playing.load() || pcm.empty()) return vec2i16(int16_t(0));
            uint64_t __pos = position.load();
            const uint64_t __frame = __pos >> 32;
            const size_t __frames = pcm.size() / 2u;
            if (__frame + 1u >= __frames) return vec2i16(int16_t(0));
            const float __t = float(__pos & 0xFFFFFFFFull) / 4294967296.f, __v = volume.load();
            const int16_t *a = &pcm[2u * __frame], *b = a + 2;
            const float l = (a[0] + (b[0] - a[0]) * __t) * __v, r = (a[1] + (b[1] - a[1]) * __t) * __v;
            // Note: a seek from the main thread wins over this step
            const uint64_t __step = (uint64_t(sample_rate) << 32) / std::max(output_rate, 1u);
            position.compare_exchange_strong(__pos, __pos + __step);
            return vec2i16((int16_t)std::max(std::min(l, 32767.f), -32768.f), (int16_t)std::max(std::min(r, 32767.f), -32768.f));
        }


//  == === ==== >  MMDMotion

        bool MMDMotion::load(const std::string &_path, std::string &_error) {
            File::ReaderWriterVMD __reader;
            if (!__reader.read(_path)) { _error = "can't read '" + _path + "' as VMD"; return false; }
            clear();
            vmd    = __reader.file_content;
            path   = _path;
            name   = fileName(_path);
            loaded = true;
            rebuild();
            return true;
        }

        bool MMDMotion::save(const std::string &_path, std::string &_error) {
            File::ReaderWriterVMD __writer;
            __writer.file_content = vmd;
            if (!__writer.write(_path)) { _error = "can't write '" + _path + "'"; return false; }
            path = _path;
            name = fileName(_path);
            modified = false;
            return true;
        }

        void MMDMotion::clear() {
            path.clear(); name.clear();
            vmd = File::ContentVMD();
            loaded = modified = false;
            clip = SG::AnimationClip();
            camera = SG::CameraClip();
            bone_keys.clear(); morph_keys.clear();
            undo_stack.clear(); redo_stack.clear();
        }

        void MMDMotion::rebuild() {
            clip   = SG::convertVMD(vmd, name);
            camera = SG::convertVMDCamera(vmd, name);
            bone_keys.clear();
            morph_keys.clear();
            for (uint32_t i = 0u; i < (uint32_t)vmd.keyframes_bone.size(); ++i)
                bone_keys[rawName(vmd.keyframes_bone[i].bone_name, VMD_BONE_NAME)].push_back(i);
            for (uint32_t i = 0u; i < (uint32_t)vmd.keyframes_morph.size(); ++i)
                morph_keys[rawName(vmd.keyframes_morph[i].morph_name, VMD_MORPH_NAME)].push_back(i);
            struct ByFrameBone {
                const VMD *vmd;
                bool operator()(uint32_t a, uint32_t b) const { return vmd->keyframes_bone[a].frame_id < vmd->keyframes_bone[b].frame_id; }
            } __by_bone = { &vmd };
            struct ByFrameMorph {
                const VMD *vmd;
                bool operator()(uint32_t a, uint32_t b) const { return vmd->keyframes_morph[a].frame_id < vmd->keyframes_morph[b].frame_id; }
            } __by_morph = { &vmd };
            for (std::map<std::string, std::vector<uint32_t>>::iterator it = bone_keys.begin(); it != bone_keys.end(); ++it)
                std::stable_sort(it->second.begin(), it->second.end(), __by_bone);
            for (std::map<std::string, std::vector<uint32_t>>::iterator it = morph_keys.begin(); it != morph_keys.end(); ++it)
                std::stable_sort(it->second.begin(), it->second.end(), __by_morph);
            struct ByFrameCamera {
                bool operator()(const VMD::CameraKeyframe &a, const VMD::CameraKeyframe &b) const { return a.frame_id < b.frame_id; }
            };
            std::stable_sort(vmd.keyframes_camera.begin(), vmd.keyframes_camera.end(), ByFrameCamera());
        }

        void MMDMotion::beginEdit() {
            undo_stack.push_back(vmd);
            if (undo_stack.size() > 64u) undo_stack.erase(undo_stack.begin());
            redo_stack.clear();
            modified = true;
        }

        bool MMDMotion::undo() {
            if (undo_stack.empty()) return false;
            redo_stack.push_back(vmd);
            vmd = undo_stack.back();
            undo_stack.pop_back();
            modified = true;
            rebuild();
            return true;
        }

        bool MMDMotion::redo() {
            if (redo_stack.empty()) return false;
            undo_stack.push_back(vmd);
            vmd = redo_stack.back();
            redo_stack.pop_back();
            modified = true;
            rebuild();
            return true;
        }

        float MMDMotion::length() const { return std::max(clip.length, camera.length); }


//  == === ==== >  MMDModel

        mat4 MMDModel::offsetMatrix() const {
            return SG::scaling({scale, scale, scale}) & SG::rotation(eulerToQuat(rotation)) & SG::translation(position);
        }


//  == === ==== >  MMDStudio: scene

        MMDStudio::MMDStudio(SG::Scene &_scene) : scene(_scene), base_scene(_scene) {
            copyPath(model_path, sizeof(model_path), "");
            scanAssets_();
        }

        MMDStudio::~MMDStudio() {
            if (sampler) {
                std::lock_guard<std::mutex> __lock(sampler->sources_mutex);
                for (uint32_t i = 0u; i < SVKFW_AS_MAX_SOURCES; ++i)
                    if (sampler->sources[i].sample_ptr == &audio) sampler->sources[i].sample_ptr = &Audio::Sample::audio_placeholder;
            }
        }

        void MMDStudio::attachAudio(Audio::AudioSampler *_sampler, uint32_t _output_rate) {
            if (_sampler == nullptr || sampler != nullptr) return;
            audio.output_rate = std::max(_output_rate, 1u);
            if (_sampler->addSource(&audio, 1.f) < SVKFW_AS_MAX_SOURCES) sampler = _sampler;
        }

        void MMDStudio::scanAssets_() {
            const char *__models[]  = { ".pmx", ".gltf", ".glb", ".fbx", ".obj" };
            const char *__motions[] = { ".vmd" };
            const char *__audio[]   = { ".ogg", ".opus", ".wav" };
            asset_models  = File::listFiles(asset_dir, std::vector<std::string>(__models,  __models  + 5));
            asset_motions = File::listFiles(asset_dir, std::vector<std::string>(__motions, __motions + 1));
            asset_audio   = File::listFiles(asset_dir, std::vector<std::string>(__audio,   __audio   + 3));
        }

        void MMDStudio::applyOffsets_(MMDModel &_model) {
            if (_model.offset_node == SG::SCENE_NONE) return;
            scene.setLocal(_model.offset_node, _model.offsetMatrix());
            scene.setVisible(_model.offset_node, _model.visible);
            pose_changed = true;
        }

        void MMDStudio::bindMotion_(MMDModel &_model, bool _warn) {
            _model.binding = SG::AnimationBinding();
            if (_model.skin == SG::SCENE_NONE || !_model.motion.loaded) return;
            const SG::Skeleton &__skel = scene.skeletons[scene.skins[_model.skin].skeleton];
            _model.binding = SG::bindAnimation(_model.motion.clip, __skel, _warn);
        }

        bool MMDStudio::loadModel(const std::string &_path) {
            MMDModel __model;
            __model.path = _path;
            __model.name = fileName(_path);
            // Note: loaded into its own scene first, so a failed load leaves the scene as it was
            SG::Scene __loaded;
            try {
                SG::loadModel(__loaded, _path);
            } catch (const std::exception &_e) {
                status = std::string("Can't load the model: ") + _e.what();
                return false;
            }
            const uint32_t __skins0 = (uint32_t)scene.skins.size();
            __model.offset_node = scene.addNode("mmd:" + __model.name);
            __model.root = scene.append(__loaded, __model.offset_node);
            __model.skin = __loaded.skins.empty() ? SG::SCENE_NONE : __skins0;
            if (__model.skin != SG::SCENE_NONE) {
                const SG::Skeleton &__skel = scene.skeletons[scene.skins[__model.skin].skeleton];
                for (size_t b = 0u; b < __skel.bones.size(); ++b)  __model.bone_raw.push_back(Enc::utf8ToShiftJIS(__skel.bones[b].name, VMD_BONE_NAME));
                for (size_t m = 0u; m < __skel.morphs.size(); ++m) __model.morph_raw.push_back(Enc::utf8ToShiftJIS(__skel.morphs[m].name, VMD_MORPH_NAME));
            }
            models.push_back(__model);
            applyOffsets_(models.back());
            scene.updateTransforms();
            modelAdded_(models.back());
            selected_model = (int32_t)models.size() - 1;
            target_camera = false;
            select_(TRACK_NONE, SG::SCENE_NONE);
            sceneChanged_();
            pose_changed = tracks_dirty = true;
            status = "Loaded " + __model.name;
            return true;
        }

        void MMDStudio::modelAdded_(const MMDModel &_model) {
            if (!events) return;
            Event::SceneModelEvent __added;
            __added.root = _model.root;
            __added.skin = _model.skin;
            events->dispatch(Event::Event::make(Event::EV_TYPE_SCENE_MODEL_ADDED, __added));
        }

        void MMDStudio::sceneChanged_() {
            if (events) events->write(Event::EV_TYPE_SCENE_CHANGED);
        }

        void MMDStudio::rebuildScene_() {
            if (events) events->dispatch(Event::Event(Event::EV_TYPE_SCENE_CLEARED));
            scene = base_scene;
            scene.markAllDirty();
            for (size_t i = 0u; i < models.size(); ++i) {
                MMDModel &__model = models[i];
                SG::Scene __loaded;
                try { SG::loadModel(__loaded, __model.path); }
                catch (const std::exception &_e) { status = std::string("Can't reload a model: ") + _e.what(); }
                const uint32_t __skins0 = (uint32_t)scene.skins.size();
                __model.offset_node = scene.addNode("mmd:" + __model.name);
                __model.root = scene.append(__loaded, __model.offset_node);
                __model.skin = __loaded.skins.empty() ? SG::SCENE_NONE : __skins0;
                applyOffsets_(__model);
                bindMotion_(__model, false);
            }
            scene.updateTransforms();
            for (size_t i = 0u; i < models.size(); ++i) modelAdded_(models[i]);
            sceneChanged_();
            pose_changed = tracks_dirty = true;
        }

        void MMDStudio::removeModel(uint32_t _model) {
            if (_model >= models.size()) return;
            models.erase(models.begin() + _model);
            if (selected_model >= (int32_t)models.size()) selected_model = (int32_t)models.size() - 1;
            select_(TRACK_NONE, SG::SCENE_NONE);
            rebuildScene_();
        }

        bool MMDStudio::loadMotion(uint32_t _model, const std::string &_path) {
            if (_model >= models.size()) return false;
            MMDMotion __motion;
            std::string __error;
            if (!__motion.load(_path, __error)) { status = __error; return false; }
            if (__motion.vmd.keyframes_bone.empty() && __motion.vmd.keyframes_morph.empty() && !__motion.vmd.keyframes_camera.empty()) {
                status = "'" + __motion.name + "' is a camera motion, load it as the camera";
                return false;
            }
            models[_model].motion = __motion;
            bindMotion_(models[_model], true);
            const SG::AnimationBinding &__b = models[_model].binding;
            char __buf[256];
            snprintf(__buf, sizeof(__buf), "Loaded %s: %u bone, %u morph tracks (%u bones, %u morphs missing in the model)",
                     __motion.name.c_str(), (unsigned)__motion.clip.bones.size(), (unsigned)__motion.clip.morphs.size(),
                     __b.missing_bones, __b.missing_morphs);
            status = __buf;
            pose_changed = tracks_dirty = true;
            return true;
        }

        bool MMDStudio::loadCamera(const std::string &_path) {
            MMDMotion __motion;
            std::string __error;
            if (!__motion.load(_path, __error)) { status = __error; return false; }
            if (__motion.vmd.keyframes_camera.empty()) { status = "'" + __motion.name + "' has no camera keys"; return false; }
            camera_motion = __motion;
            use_camera = true;
            status = "Loaded camera " + __motion.name;
            tracks_dirty = true;
            return true;
        }

        bool MMDStudio::loadAudio(const std::string &_path) {
            std::vector<int16_t> __pcm;
            uint32_t __rate = 48000u;
            std::string __error;
            if (!MMDAudioTrack::decode(_path, __pcm, __rate, __error)) { status = "Can't load the audio: " + __error; return false; }
            {
                // Note: the audio thread reads 'pcm' while it holds the mutex of the sampler
                std::unique_lock<std::mutex> __lock;
                if (sampler) __lock = std::unique_lock<std::mutex>(sampler->sources_mutex);
                audio.playing.store(false);
                audio.pcm.swap(__pcm);
                audio.sample_rate = __rate;
                audio.seek(0.);
            }
            audio_path = _path;
            status = "Loaded audio " + fileName(_path) + (sampler ? "" : " (no audio output: the timeline plays without it)");
            return true;
        }

        float MMDStudio::length() const {
            float __len = 0.f;
            for (size_t i = 0u; i < models.size(); ++i) __len = std::max(__len, models[i].motion.length());
            if (camera_motion.loaded) __len = std::max(__len, camera_motion.length());
            if (!audio.pcm.empty()) __len = std::max(__len, float(audio.length() * fps) + audio_offset);
            return __len;
        }

        void MMDStudio::seek(float _frame) {
            frame = std::max(_frame, 0.f);
            play_start = frame;
            play_clock = 0.;
            const double __t = (frame - audio_offset) / fps;
            audio.seek(__t);
            audio.playing.store(playing && __t >= 0. && __t < audio.length());
            edit_active = false;
            pose_changed = true;
        }

        void MMDStudio::play() {
            if (playing) return;
            if (frame >= length() && length() > 0.f) frame = 0.f;
            playing = true;
            seek(frame);
        }

        void MMDStudio::stop() {
            playing = false;
            audio.playing.store(false);
            play_start = frame;
            play_clock = 0.;
            pose_changed = true;
        }

        bool MMDStudio::applyCamera(const ProjPerspective &_base, mat4 &_view, mat4 &_proj) const {
            const bool __editing = edit_active && sel_type == TRACK_CAMERA;
            if (!use_camera || (!__editing && (!camera_motion.loaded || camera_motion.camera.empty()))) return false;
            const SG::CameraPose __pose = __editing ? edit_camera : camera_motion.camera.evaluate(frame);
            vec3 __eye, __up;
            __pose.lookAt(__eye, __up);
            _view = View::ViewMat(__eye, __pose.center, __up);
            // Note: MMD's fov is the vertical angle of the whole view, so the aspect comes from the current framebuffer
            //       ('ProjPerspective' keeps the pixel scale of its first size instead)
            const float __aspect = _base.fb_now.y > 0.f ? _base.fb_now.x / _base.fb_now.y : 16.f / 9.f;
            _proj = Projection::Perspective(std::min(std::max(__pose.fov, 1.f), 175.f), __aspect, _base.p_near, _base.p_far);
            return true;
        }


//  == === ==== >  MMDStudio: playback

        void MMDStudio::update(double _dt) {
            const float __dt = (float)std::max(_dt, 0.);
            if (playing) {
                play_clock += __dt;
                frame = play_start + float(play_clock * fps);
                // The audio: started when the play head reaches it, the clock follows it when they drift apart
                if (!audio.pcm.empty() && sampler) {
                    const double __t = (frame - audio_offset) / fps;
                    if (!audio.playing.load() && __t >= 0. && __t < audio.length()) { audio.seek(__t); audio.playing.store(true); }
                    else if (audio.playing.load() && std::fabs(audio.time() - __t) > 0.05) {
                        frame = float(audio.time() * fps) + audio_offset;
                        play_start = frame;
                        play_clock = 0.;
                    }
                }
                const float __len = length();
                if (frame >= __len) {
                    if (loop && __len > 0.f) seek(0.f);
                    else { frame = __len; stop(); }
                }
            }

            // Animations (from the rest pose, so tracks that ended or edits that were dropped don't stay)
            for (size_t i = 0u; i < models.size(); ++i) {
                MMDModel &__model = models[i];
                if (__model.skin == SG::SCENE_NONE) continue;
                const SG::Skeleton &__skel = scene.skeletons[scene.skins[__model.skin].skeleton];
                SG::Pose &__pose = scene.pose(__model.skin);
                if (__pose.local.size() != __skel.bones.size() || __pose.morph_weights.size() != __skel.morphs.size()) __pose.reset(__skel);
                for (size_t b = 0u; b < __skel.bones.size(); ++b) __pose.local[b] = __skel.bones[b].rest;
                std::fill(__pose.morph_weights.begin(), __pose.morph_weights.end(), 0.f);
                if (__model.motion.loaded) SG::applyAnimation(__model.motion.clip, __model.binding, __skel, frame, __pose);
            }
            // The keyframe window shows the animated values until they are edited, then previews the edit
            refreshEdit_();
            applyEdit_();

            if (pose_changed && events) events->dispatch(Event::Event(Event::EV_TYPE_SCENE_RESET));
            sim_dt = playing ? __dt : 0.f;
            pose_changed = false;
            scene.updateTransforms();
        }


//  == === ==== >  MMDStudio: tracks and keys

        MMDModel *MMDStudio::selectedModel_() {
            return !target_camera && selected_model >= 0 && selected_model < (int32_t)models.size() ? &models[selected_model] : nullptr;
        }

        MMDMotion *MMDStudio::targetMotion_() {
            if (target_camera) return &camera_motion;
            MMDModel *__model = selectedModel_();
            return __model ? &__model->motion : nullptr;
        }

        void MMDStudio::select_(TrackType _type, uint32_t _index) {
            if (_type != sel_type || _index != sel_index) {
                edit_active = false;
                tracks_dirty = true;
            }
            sel_type = _type;
            sel_index = _index;
        }

        const MMDStudio::Track *MMDStudio::selectedTrack_() const {
            return selected_track >= 0 && selected_track < (int32_t)tracks.size() ? &tracks[selected_track] : nullptr;
        }

        void MMDStudio::buildTracks_() {
            tracks.clear();
            rows.clear();
            selected_track = -1;
            const ImU32 __bone_color = IM_COL32(230, 160, 60, 255), __morph_color = IM_COL32(110, 200, 120, 255),
                        __camera_color = IM_COL32(110, 160, 240, 255);
            if (target_camera) {
                Track __track;
                __track.type = TRACK_CAMERA;
                __track.index = 0u;
                ImGuiCustom::TimelineRow __row;
                __row.name = "Camera";
                __row.color = __camera_color;
                for (size_t k = 0u; k < camera_motion.vmd.keyframes_camera.size(); ++k)
                    __row.frames.push_back(float(camera_motion.vmd.keyframes_camera[k].frame_id));
                tracks.push_back(__track);
                rows.push_back(__row);
                if (sel_type != TRACK_CAMERA) { sel_type = TRACK_CAMERA; sel_index = 0u; edit_active = false; }
                selected_track = 0;
            }
            MMDModel *__model = selectedModel_();
            if (__model && __model->skin != SG::SCENE_NONE) {
                const SG::Skeleton &__skel = scene.skeletons[scene.skins[__model->skin].skeleton];
                const MMDMotion &__motion = __model->motion;
                for (int __kind = 0; __kind < 2; ++__kind) {
                    const bool __bones = __kind == 0;
                    const std::vector<std::string> &__raws = __bones ? __model->bone_raw : __model->morph_raw;
                    const std::map<std::string, std::vector<uint32_t>> &__keys = __bones ? __motion.bone_keys : __motion.morph_keys;
                    const TrackType __type = __bones ? TRACK_BONE : TRACK_MORPH;
                    Track __header;
                    ImGuiCustom::TimelineRow __header_row;
                    __header_row.header = true;
                    __header_row.name = __bones ? "Bones" : "Morphs";
                    tracks.push_back(__header);
                    rows.push_back(__header_row);
                    for (uint32_t i = 0u; i < (uint32_t)__raws.size(); ++i) {
                        std::map<std::string, std::vector<uint32_t>>::const_iterator __found = __keys.find(__raws[i]);
                        const bool __selected = sel_type == __type && sel_index == i;
                        if (!__selected && __found == __keys.end() && !(__bones ? all_bones : all_morphs)) continue;
                        Track __track;
                        __track.type  = __type;
                        __track.index = i;
                        __track.raw   = __raws[i];
                        ImGuiCustom::TimelineRow __row;
                        __row.name  = __bones ? __skel.bones[i].name.c_str() : __skel.morphs[i].name.c_str();
                        __row.color = __bones ? __bone_color : __morph_color;
                        if (__found != __keys.end())
                            for (size_t k = 0u; k < __found->second.size(); ++k)
                                __row.frames.push_back(float(__bones ? __motion.vmd.keyframes_bone[__found->second[k]].frame_id
                                                                     : __motion.vmd.keyframes_morph[__found->second[k]].frame_id));
                        if (__selected) selected_track = (int32_t)tracks.size();
                        tracks.push_back(__track);
                        rows.push_back(__row);
                    }
                }
            }
            timeline.selected_row = selected_track;
            if (selected_track < 0) timeline.selected_key = -1;
            tracks_dirty = false;
        }

        int32_t MMDStudio::keyAt_(const Track &_track, uint32_t _frame) const {
            MMDStudio *__self = const_cast<MMDStudio*>(this);
            const MMDMotion *__motion = __self->targetMotion_();
            if (!__motion || !__motion->loaded) return -1;
            if (_track.type == TRACK_CAMERA) {
                for (size_t k = 0u; k < __motion->vmd.keyframes_camera.size(); ++k)
                    if (__motion->vmd.keyframes_camera[k].frame_id == _frame) return (int32_t)k;
                return -1;
            }
            const bool __bone = _track.type == TRACK_BONE;
            const std::map<std::string, std::vector<uint32_t>> &__keys = __bone ? __motion->bone_keys : __motion->morph_keys;
            std::map<std::string, std::vector<uint32_t>>::const_iterator __found = __keys.find(_track.raw);
            if (__found == __keys.end()) return -1;
            for (size_t k = 0u; k < __found->second.size(); ++k) {
                const uint32_t i = __found->second[k];
                const uint32_t __f = __bone ? __motion->vmd.keyframes_bone[i].frame_id : __motion->vmd.keyframes_morph[i].frame_id;
                if (__f == _frame) return (int32_t)i;
            }
            return -1;
        }

        void MMDStudio::motionChanged_(MMDMotion &_motion) {
            _motion.rebuild();
            for (size_t i = 0u; i < models.size(); ++i)
                if (&models[i].motion == &_motion) bindMotion_(models[i], false);
            tracks_dirty = pose_changed = true;
            edit_active = false;
        }

        void MMDStudio::refreshEdit_() {
            const Track *__track = selectedTrack_();
            if (!__track || edit_active) return;
            if (__track->type == TRACK_CAMERA) {
                edit_camera = camera_motion.loaded && !camera_motion.camera.empty() ? camera_motion.camera.evaluate(frame) : SG::CameraPose();
                return;
            }
            MMDModel *__model = selectedModel_();
            if (!__model || __model->skin == SG::SCENE_NONE) return;
            const SG::Skeleton &__skel = scene.skeletons[scene.skins[__model->skin].skeleton];
            const SG::Pose &__pose = scene.skins[__model->skin].pose;
            if (__track->type == TRACK_BONE && __track->index < __pose.local.size()) {
                const SG::Bone &__bone = __skel.bones[__track->index];
                edit_translation = __pose.local[__track->index].translation - __bone.rest.translation;
                edit_rotation    = quatToEuler(__bone.rest.rotation.inverse() * __pose.local[__track->index].rotation);
            } else if (__track->type == TRACK_MORPH && __track->index < __pose.morph_weights.size())
                edit_weight = __pose.morph_weights[__track->index];
        }

        void MMDStudio::applyEdit_() {
            const Track *__track = selectedTrack_();
            if (!__track || !edit_active || __track->type == TRACK_CAMERA) return;
            MMDModel *__model = selectedModel_();
            if (!__model || __model->skin == SG::SCENE_NONE) return;
            const SG::Skeleton &__skel = scene.skeletons[scene.skins[__model->skin].skeleton];
            SG::Pose &__pose = scene.pose(__model->skin);
            if (__track->type == TRACK_BONE && __track->index < __pose.local.size()) {
                const SG::Bone &__bone = __skel.bones[__track->index];
                __pose.local[__track->index].translation = __bone.rest.translation + edit_translation;
                __pose.local[__track->index].rotation    = __bone.rest.rotation * eulerToQuat(edit_rotation);
            } else if (__track->type == TRACK_MORPH && __track->index < __pose.morph_weights.size())
                __pose.morph_weights[__track->index] = edit_weight;
        }

        void MMDStudio::registerKey_() {
            const Track *__track = selectedTrack_();
            if (!__track || __track->type == TRACK_NONE) return;
            MMDMotion *__motion = targetMotion_();
            if (!__motion) return;
            if (!__motion->loaded) {
                // A new motion for the model (or a new camera motion), saved with "Save as"
                MMDModel *__model = selectedModel_();
                __motion->clear();
                initVMD(__motion->vmd, target_camera ? std::string("\xE3\x82\xAB\xE3\x83\xA1\xE3\x83\xA9\xE3\x83\xBB\xE7\x85\xA7\xE6\x98\x8E") // カメラ・照明
                                                     : (__model ? __model->name : std::string("model")));
                __motion->loaded = true;
                __motion->name   = target_camera ? "camera.vmd" : (__model ? __model->name + ".vmd" : "motion.vmd");
            }
            const uint32_t __frame = (uint32_t)std::lround(std::max(frame, 0.f));
            const int32_t  __key = keyAt_(*__track, __frame);
            __motion->beginEdit();
            VMD &__vmd = __motion->vmd;
            if (__track->type == TRACK_BONE) {
                if (__key < 0) {
                    VMD::BoneKeyframe __kf{};
                    setRawName(__kf.bone_name, sizeof(__kf.bone_name), __track->raw);
                    __kf.frame_id = __frame;
                    VMD::SetInterpolation(__kf, linearInterpolation());
                    __vmd.keyframes_bone.push_back(__kf);
                }
                VMD::BoneKeyframe &__kf = __key < 0 ? __vmd.keyframes_bone.back() : __vmd.keyframes_bone[__key];
                __kf.position = { edit_translation.x, edit_translation.y, edit_translation.z };
                __kf.rotation = eulerToQuat(edit_rotation);
            } else if (__track->type == TRACK_MORPH) {
                if (__key < 0) {
                    VMD::MorphKeyframe __kf{};
                    setRawName(__kf.morph_name, sizeof(__kf.morph_name), __track->raw);
                    __kf.frame_id = __frame;
                    __vmd.keyframes_morph.push_back(__kf);
                }
                VMD::MorphKeyframe &__kf = __key < 0 ? __vmd.keyframes_morph.back() : __vmd.keyframes_morph[__key];
                __kf.weight = edit_weight;
            } else {
                if (__key < 0) {
                    VMD::CameraKeyframe __kf{};
                    __kf.frame_id = __frame;
                    VMD::SetInterpolation(__kf, linearInterpolation());
                    __vmd.keyframes_camera.push_back(__kf);
                }
                VMD::CameraKeyframe &__kf = __key < 0 ? __vmd.keyframes_camera.back() : __vmd.keyframes_camera[__key];
                __kf.position = { edit_camera.center.x, edit_camera.center.y, edit_camera.center.z };
                __kf.rotation = { edit_camera.rotation.x, edit_camera.rotation.y, edit_camera.rotation.z };
                __kf.distance = -edit_camera.distance; // the VMD stores it negated
                __kf.fov      = (uint32_t)std::lround(std::min(std::max(edit_camera.fov, 1.f), 175.f));
                __kf.perspect = edit_camera.perspective ? 0u : 1u;
            }
            motionChanged_(*__motion);
            if (target_camera) use_camera = true;
        }

        void MMDStudio::deleteKey_() {
            const Track *__track = selectedTrack_();
            MMDMotion *__motion = targetMotion_();
            if (!__track || !__motion) return;
            const int32_t __key = keyAt_(*__track, (uint32_t)std::lround(std::max(frame, 0.f)));
            if (__key < 0) return;
            __motion->beginEdit();
            if (__track->type == TRACK_BONE)        __motion->vmd.keyframes_bone.erase(__motion->vmd.keyframes_bone.begin() + __key);
            else if (__track->type == TRACK_MORPH)  __motion->vmd.keyframes_morph.erase(__motion->vmd.keyframes_morph.begin() + __key);
            else                                    __motion->vmd.keyframes_camera.erase(__motion->vmd.keyframes_camera.begin() + __key);
            motionChanged_(*__motion);
        }

        void MMDStudio::moveKey_(const Track &_track, uint32_t _from, uint32_t _to) {
            MMDMotion *__motion = targetMotion_();
            if (!__motion || _from == _to) return;
            const int32_t __src = keyAt_(_track, _from), __dst = keyAt_(_track, _to);
            if (__src < 0) return;
            __motion->beginEdit();
            VMD &__vmd = __motion->vmd;
            if (_track.type == TRACK_BONE) {
                __vmd.keyframes_bone[__src].frame_id = _to;
                if (__dst >= 0) __vmd.keyframes_bone.erase(__vmd.keyframes_bone.begin() + __dst);
            } else if (_track.type == TRACK_MORPH) {
                __vmd.keyframes_morph[__src].frame_id = _to;
                if (__dst >= 0) __vmd.keyframes_morph.erase(__vmd.keyframes_morph.begin() + __dst);
            } else {
                __vmd.keyframes_camera[__src].frame_id = _to;
                if (__dst >= 0) __vmd.keyframes_camera.erase(__vmd.keyframes_camera.begin() + __dst);
            }
            motionChanged_(*__motion);
            seek(float(_to));
        }


//  == === ==== >  MMDStudio: UI

        bool MMDStudio::begin() { return true; }

        void MMDStudio::end() {}

        void MMDStudio::renderFull() {
            if (begin()) render();
            end();
        }

        void MMDStudio::confirm_(const std::string &_text, const std::function<void()> &_action) {
            confirm_text = _text;
            confirm_action = _action;
        }

        void MMDStudio::render() {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("MMD")) {
                    ImGui::MenuItem("Scene",    nullptr, &show_scene);
                    ImGui::MenuItem("Timeline", nullptr, &show_timeline);
                    ImGui::MenuItem("Keyframe", nullptr, &show_keyframe);
                    ImGui::Separator();
                    if (ImGui::MenuItem(playing ? "Pause" : "Play")) { if (playing) stop(); else play(); }
                    if (ImGui::MenuItem("Rewind")) seek(0.f);
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
            if (tracks_dirty) buildTracks_();
            if (show_scene)    windowScene();
            if (show_timeline) windowTimeline();
            if (show_keyframe) windowKeyframe();

            if (!confirm_text.empty()) ImGui::OpenPopup("Confirm##mmd");
            if (ImGui::BeginPopupModal("Confirm##mmd", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextUnformatted(confirm_text.c_str());
                if (ImGui::Button("OK")) {
                    std::function<void()> __action = confirm_action;
                    confirm_text.clear();
                    confirm_action = std::function<void()>();
                    ImGui::CloseCurrentPopup();
                    if (__action) __action();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    confirm_text.clear();
                    confirm_action = std::function<void()>();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

        namespace {
            // A combo with the found assets (picks into '_buf') and the path field itself
            void assetPicker(const char *_id, const std::vector<std::string> &_assets, char *_buf, size_t _size) {
                ImGui::PushID(_id);
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * 2.f);
                if (ImGui::BeginCombo("##assets", "", ImGuiComboFlags_NoPreview | ImGuiComboFlags_HeightLarge)) {
                    if (_assets.empty()) ImGui::TextDisabled("Nothing found (see the asset folder)");
                    for (size_t i = 0u; i < _assets.size(); ++i)
                        if (ImGui::Selectable(_assets[i].c_str(), _assets[i] == _buf)) copyPath(_buf, _size, _assets[i]);
                    ImGui::EndCombo();
                }
                ImGui::SameLine();
                ImGui::SetNextItemWidth(-ImGui::GetFontSize() * 5.f);
                ImGui::InputText("##path", _buf, _size);
                ImGui::PopID();
            }

            void motionLabel(const MMDMotion &_motion) {
                if (!_motion.loaded) { ImGui::TextDisabled("no motion"); return; }
                ImGui::Text("%s%s  (%u frames)", _motion.name.c_str(), _motion.modified ? " *" : "", (unsigned)_motion.length());
            }
        } // anonymous

        void MMDStudio::windowScene() {
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 26.f, ImGui::GetFontSize() * 34.f), ImGuiCond_FirstUseEver);
            if (!ImGui::Begin("MMD Scene", &show_scene)) { ImGui::End(); return; }

            // Assets
            if (ImGui::CollapsingHeader("Asset folder")) {
                char __dir[256];
                copyPath(__dir, sizeof(__dir), asset_dir);
                ImGui::SetNextItemWidth(-ImGui::GetFontSize() * 5.f);
                if (ImGui::InputText("##asset_dir", __dir, sizeof(__dir))) asset_dir = __dir;
                ImGui::SameLine();
                if (ImGui::Button("Rescan")) scanAssets_();
                ImGui::TextDisabled("%u models, %u motions, %u audio files", (unsigned)asset_models.size(), (unsigned)asset_motions.size(),
                                    (unsigned)asset_audio.size());
            }

            // Models
            if (ImGui::CollapsingHeader("Models", ImGuiTreeNodeFlags_DefaultOpen)) {
                assetPicker("model", asset_models, model_path, sizeof(model_path));
                ImGui::SameLine();
                if (ImGui::Button("Load##model") && model_path[0]) loadModel(trimmed(model_path));
                for (size_t i = 0u; i < models.size(); ++i) {
                    ImGui::PushID((int)i);
                    char __label[512];
                    snprintf(__label, sizeof(__label), "%s%s", models[i].name.c_str(), models[i].motion.modified ? " *" : "");
                    if (ImGui::Selectable(__label, !target_camera && selected_model == (int32_t)i)) {
                        if (selected_model != (int32_t)i || target_camera) select_(TRACK_NONE, SG::SCENE_NONE);
                        selected_model = (int32_t)i;
                        target_camera = false;
                        tracks_dirty = true;
                    }
                    ImGui::PopID();
                }

                MMDModel *__model = selectedModel_();
                if (__model) {
                    ImGui::SeparatorText(__model->name.c_str());
                    bool __offset = false;
                    __offset |= ImGui::Checkbox("Visible", &__model->visible);
                    __offset |= ImGui::DragFloat3("Position", &__model->position.x, 0.05f);
                    __offset |= ImGui::DragFloat3("Rotation", &__model->rotation.x, 0.5f, -360.f, 360.f, "%.1f deg");
                    __offset |= ImGui::DragFloat("Scale", &__model->scale, 0.01f, 0.01f, 100.f);
                    if (__offset) applyOffsets_(*__model);

                    ImGui::Text("Motion:"); ImGui::SameLine(); motionLabel(__model->motion);
                    assetPicker("motion", asset_motions, motion_path, sizeof(motion_path));
                    ImGui::SameLine();
                    if (ImGui::Button("Load##motion") && motion_path[0]) {
                        const uint32_t __index = (uint32_t)selected_model;
                        const std::string __path = trimmed(motion_path);
                        if (__model->motion.modified) confirm_("The motion has unsaved changes. Load another one anyway?",
                                                               [this, __index, __path]() { loadMotion(__index, __path); });
                        else loadMotion(__index, __path);
                    }
                    if (__model->motion.loaded) {
                        if (ImGui::Button("Unload motion")) {
                            MMDMotion *__m = &__model->motion;
                            std::function<void()> __unload = [this, __m]() { __m->clear(); motionChanged_(*__m); };
                            if (__m->modified) confirm_("The motion has unsaved changes. Unload it anyway?", __unload);
                            else __unload();
                        }
                        ImGui::SameLine();
                    }
                    if (ImGui::Button("Remove model")) {
                        const uint32_t __index = (uint32_t)selected_model;
                        confirm_("Remove '" + __model->name + "' from the scene?" + (__model->motion.modified ? "\nIts motion has unsaved changes." : ""),
                                 [this, __index]() { removeModel(__index); });
                    }
                }
            }

            // Camera
            if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("Motion:"); ImGui::SameLine(); motionLabel(camera_motion);
                assetPicker("camera", asset_motions, camera_path, sizeof(camera_path));
                ImGui::SameLine();
                if (ImGui::Button("Load##camera") && camera_path[0]) {
                    const std::string __path = trimmed(camera_path);
                    if (camera_motion.modified) confirm_("The camera motion has unsaved changes. Load another one anyway?",
                                                         [this, __path]() { loadCamera(__path); });
                    else loadCamera(__path);
                }
                ImGui::Checkbox("Use the VMD camera", &use_camera);
                ImGui::SameLine();
                if (ImGui::Button("Edit camera keys")) { target_camera = true; tracks_dirty = true; }
            }

            // Audio
            if (ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (audio.pcm.empty()) ImGui::TextDisabled("no audio");
                else ImGui::Text("%s  (%.1f s)", fileName(audio_path).c_str(), audio.length());
                if (!sampler) ImGui::TextDisabled("No audio output stream: the timeline plays silently");
                assetPicker("audio", asset_audio, audio_path_buf, sizeof(audio_path_buf));
                ImGui::SameLine();
                if (ImGui::Button("Load##audio") && audio_path_buf[0]) loadAudio(trimmed(audio_path_buf));
                float __volume = audio.volume.load();
                if (ImGui::SliderFloat("Volume", &__volume, 0.f, 2.f)) audio.volume.store(__volume);
                if (ImGui::DragFloat("Start frame", &audio_offset, 0.25f, -100000.f, 100000.f, "%.1f")) seek(frame);
            }

            // The motion of the timeline target: saving and processing (the former VMD Editor)
            MMDMotion *__motion = targetMotion_();
            if (ImGui::CollapsingHeader("Motion tools", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (!__motion || !__motion->loaded) ImGui::TextDisabled("Select a model with a motion (or the camera)");
                else {
                    ImGui::Text("%s:", target_camera ? "Camera" : "Model motion"); ImGui::SameLine(); motionLabel(*__motion);
                    // Note: the path field follows the target, so a motion is never saved over the file of another one by accident
                    if (save_path_motion != __motion) { copyPath(save_path, sizeof(save_path), __motion->path); save_path_motion = __motion; }
                    ImGui::SetNextItemWidth(-ImGui::GetFontSize() * 9.f);
                    ImGui::InputText("##save_path", save_path, sizeof(save_path));
                    ImGui::SameLine();
                    std::string __error;
                    if (ImGui::Button("Save")) {
                        const std::string __path = trimmed(save_path);
                        if (__path.empty()) status = "Enter a path to save to";
                        else if (!__motion->save(__path, __error)) status = __error;
                        else { status = "Saved " + __path; scanAssets_(); }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Revert") && !__motion->path.empty()) {
                        const std::string __path = __motion->path;
                        MMDMotion *__m = __motion;
                        confirm_("Reload '" + __m->name + "' from the file and drop the changes?", [this, __m, __path]() {
                            std::string __err;
                            if (__m->load(__path, __err)) motionChanged_(*__m); else status = __err;
                        });
                    }

                    ImGui::TextDisabled("Processing (applied to the loaded motion; Save writes it):");
                    if (!target_camera) {
                        if (ImGui::Button("Fix blink")) {
                            __motion->beginEdit();
                            __motion->vmd.clampFrames({}, {{"blink", VMD::MorphAttrClampMinmax(0.01f, 0.99f)}});
                            motionChanged_(*__motion);
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Keeps the blink morph in [0.01, 0.99]");
                        ImGui::SameLine();
                        if (ImGui::Button("Reinterpolate bones")) {
                            __motion->beginEdit();
                            __motion->vmd.reinterpolateBones();
                            motionChanged_(*__motion);
                        }
                    } else if (ImGui::Button("Reinterpolate camera")) {
                        __motion->beginEdit();
                        __motion->vmd.reinterpolateCamera();
                        motionChanged_(*__motion);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Reflect (X = 0)")) {
                        __motion->beginEdit();
                        __motion->vmd.reflectMovement();
                        motionChanged_(*__motion);
                    }
                }
            }

            if (!status.empty()) { ImGui::Separator(); ImGui::TextWrapped("%s", status.c_str()); }
            ImGui::End();
        }

        void MMDStudio::windowTimeline() {
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 48.f, ImGui::GetFontSize() * 18.f), ImGuiCond_FirstUseEver);
            if (!ImGui::Begin("MMD Timeline", &show_timeline)) { ImGui::End(); return; }
            const float __len = length();

            // Transport
            if (ImGui::Button("|<")) seek(0.f);
            ImGui::SameLine();
            if (ImGui::Button("<")) seek(std::floor(frame) - 1.f);
            ImGui::SameLine();
            if (ImGui::Button(playing ? "Pause" : "Play ")) { if (playing) stop(); else play(); }
            ImGui::SameLine();
            if (ImGui::Button(">")) seek(std::floor(frame) + 1.f);
            ImGui::SameLine();
            if (ImGui::Button(">|")) seek(__len);
            ImGui::SameLine();
            int __frame = (int)std::lround(frame);
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 6.f);
            if (ImGui::InputInt("##frame", &__frame)) seek(float(std::max(__frame, 0)));
            ImGui::SameLine();
            ImGui::Text("/ %d", (int)__len);
            ImGui::SameLine();
            if (ImGui::Button("Go to frame")) timeline.scroll_to = frame;
            ImGui::SameLine();
            ImGui::Checkbox("Loop", &loop);

            // Target and track filters
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 10.f);
            const char *__target = target_camera ? "Camera" : (selectedModel_() ? selectedModel_()->name.c_str() : "(no model)");
            if (ImGui::BeginCombo("##target", __target)) {
                for (size_t i = 0u; i < models.size(); ++i)
                    if (ImGui::Selectable(models[i].name.c_str(), !target_camera && selected_model == (int32_t)i)) {
                        selected_model = (int32_t)i; target_camera = false;
                        select_(TRACK_NONE, SG::SCENE_NONE);
                        tracks_dirty = true;
                    }
                if (ImGui::Selectable("Camera", target_camera)) { target_camera = true; tracks_dirty = true; }
                ImGui::EndCombo();
            }
            if (!target_camera) {
                ImGui::SameLine();
                if (ImGui::Checkbox("All bones", &all_bones)) tracks_dirty = true;
                ImGui::SameLine();
                if (ImGui::Checkbox("All morphs", &all_morphs)) tracks_dirty = true;
            }
            MMDMotion *__motion = targetMotion_();
            ImGui::SameLine();
            ImGui::BeginDisabled(!__motion || __motion->undo_stack.empty());
            if (ImGui::Button("Undo") && __motion && __motion->undo()) motionChanged_(*__motion);
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::BeginDisabled(!__motion || __motion->redo_stack.empty());
            if (ImGui::Button("Redo") && __motion && __motion->redo()) motionChanged_(*__motion);
            ImGui::EndDisabled();

            // Keyboard (while this window has the focus)
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::GetIO().WantTextInput) {
                const ImGuiIO &__io = ImGui::GetIO();
                if (ImGui::IsKeyPressed(ImGuiKey_Space, false)) { if (playing) stop(); else play(); }
                if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))  seek(std::floor(frame) - (__io.KeyShift ? 10.f : 1.f));
                if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) seek(std::floor(frame) + (__io.KeyShift ? 10.f : 1.f));
                if (__io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false) && __motion && __motion->undo()) motionChanged_(*__motion);
                if (__io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false) && __motion && __motion->redo()) motionChanged_(*__motion);
                if (ImGui::IsKeyPressed(ImGuiKey_Enter, false)) registerKey_();
                if (ImGui::IsKeyPressed(ImGuiKey_Delete, false)) deleteKey_();
            }
            if (tracks_dirty) buildTracks_();

            ImGuiCustom::TimelineEvent __tl_event;
            // The rest of the window (the frame scroll bar stays visible at the bottom)
            const ImVec2 __tl_size(0.f, std::max(ImGui::GetContentRegionAvail().y, ImGui::GetTextLineHeightWithSpacing() * 6.f));
            if (ImGuiCustom::Timeline("##mmd_timeline", rows, frame, __len, timeline, __tl_event, __tl_size)) {
                const Track *__track = __tl_event.row >= 0 && __tl_event.row < (int)tracks.size() ? &tracks[__tl_event.row] : nullptr;
                switch (__tl_event.type) {
                    case ImGuiCustom::TimelineEvent::TL_EVENT_SEEK: seek(__tl_event.frame); break;
                    case ImGuiCustom::TimelineEvent::TL_EVENT_SELECT:
                        if (__track && __track->type != TRACK_NONE) {
                            const int __key = __tl_event.key;
                            select_(__track->type, __track->index);
                            selected_track = __tl_event.row;
                            timeline.selected_key = __key;
                        }
                        if (__tl_event.frame >= 0.f) seek(__tl_event.frame);
                        break;
                    case ImGuiCustom::TimelineEvent::TL_EVENT_MOVE_KEY:
                        if (__track && __tl_event.key >= 0 && __tl_event.key < (int)rows[__tl_event.row].frames.size()) {
                            const Track __copy = *__track;
                            moveKey_(__copy, (uint32_t)rows[__tl_event.row].frames[__tl_event.key], (uint32_t)std::max(__tl_event.frame, 0.f));
                        }
                        break;
                    case ImGuiCustom::TimelineEvent::TL_EVENT_DOUBLE_CLICK: seek(__tl_event.frame); break;
                    default: break;
                }
            }
            ImGui::End();
        }

        void MMDStudio::curveEditor_() {
            const Track *__track = selectedTrack_();
            MMDMotion *__motion = targetMotion_();
            if (!__track || !__motion || __track->type == TRACK_MORPH) return;
            const int32_t __key = keyAt_(*__track, (uint32_t)std::lround(std::max(frame, 0.f)));
            ImGui::SeparatorText("Interpolation");
            if (__key < 0) { ImGui::TextDisabled("Morphs are linear; bones and the camera: register a key at this frame to edit its curves"); return; }
            const bool __camera = __track->type == TRACK_CAMERA;
            const int __channels = __camera ? 6 : 4;
            curve_channel = std::min(curve_channel, __channels - 1);
            for (int c = 0; c < __channels; ++c) {
                if (c > 0) ImGui::SameLine();
                ImGui::RadioButton(CHANNEL_NAMES[c], &curve_channel, c);
            }
            VMD::InterpolationInfo __info = __camera ? VMD::GetInterpolation(__motion->vmd.keyframes_camera[__key])
                                                     : VMD::GetInterpolation(__motion->vmd.keyframes_bone[__key]);
            vec4u8 &__ch = *channelOf(__info, curve_channel);
            int x1 = __ch.x, y1 = __ch.y, x2 = __ch.z, y2 = __ch.w;
            bool __changed = ImGuiCustom::BezierCurve127("##curve", &x1, &y1, &x2, &y2);
            const bool __active = ImGui::IsItemActive();
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Text("(%d, %d)\n(%d, %d)", x1, y1, x2, y2);
            int __preset = -1;
            if (ImGui::Button("Linear"))   __preset = 0;
            if (ImGui::Button("Ease in"))  __preset = 1;
            if (ImGui::Button("Ease out")) __preset = 2;
            if (ImGui::Button("Smooth"))   __preset = 3;
            const bool __all = ImGui::Button("Copy to all channels");
            ImGui::EndGroup();
            const int __presets[4][4] = { {20, 20, 107, 107}, {64, 0, 127, 127}, {0, 0, 64, 127}, {64, 0, 64, 127} };
            if (__preset >= 0) { x1 = __presets[__preset][0]; y1 = __presets[__preset][1]; x2 = __presets[__preset][2]; y2 = __presets[__preset][3]; __changed = true; }
            if (!__changed && !__all) { if (!__active) curve_dragging = false; return; }
            // One undo step per drag
            if (!curve_dragging) __motion->beginEdit();
            curve_dragging = __active;
            __ch = { (uint8_t)x1, (uint8_t)y1, (uint8_t)x2, (uint8_t)y2 };
            if (__all) for (int c = 0; c < __channels; ++c) *channelOf(__info, c) = __ch;
            if (__camera) VMD::SetInterpolation(__motion->vmd.keyframes_camera[__key], __info);
            else          VMD::SetInterpolation(__motion->vmd.keyframes_bone[__key], __info);
            __motion->rebuild();
            for (size_t i = 0u; i < models.size(); ++i) if (&models[i].motion == __motion) bindMotion_(models[i], false);
            pose_changed = true;
        }

        void MMDStudio::windowKeyframe() {
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 24.f, ImGui::GetFontSize() * 26.f), ImGuiCond_FirstUseEver);
            if (!ImGui::Begin("MMD Keyframe", &show_keyframe)) { ImGui::End(); return; }

            // Picks any bone or morph of the model (the timeline lists only the keyed ones)
            MMDModel *__model = selectedModel_();
            if (__model && __model->skin != SG::SCENE_NONE) {
                const SG::Skeleton &__skel = scene.skeletons[scene.skins[__model->skin].skeleton];
                const char *__preview = sel_type == TRACK_BONE  && sel_index < __skel.bones.size()  ? __skel.bones[sel_index].name.c_str()
                                      : sel_type == TRACK_MORPH && sel_index < __skel.morphs.size() ? __skel.morphs[sel_index].name.c_str() : "(select)";
                if (ImGui::BeginCombo("Bone / morph", __preview, ImGuiComboFlags_HeightLarge)) {
                    ImGui::InputText("Filter", bone_filter, sizeof(bone_filter));
                    const std::string __filter = bone_filter;
                    ImGui::SeparatorText("Bones");
                    for (uint32_t i = 0u; i < (uint32_t)__skel.bones.size(); ++i) {
                        if (!__filter.empty() && __skel.bones[i].name.find(__filter) == std::string::npos) continue;
                        ImGui::PushID((int)i);
                        if (ImGui::Selectable(__skel.bones[i].name.c_str(), sel_type == TRACK_BONE && sel_index == i)) select_(TRACK_BONE, i);
                        ImGui::PopID();
                    }
                    ImGui::SeparatorText("Morphs");
                    for (uint32_t i = 0u; i < (uint32_t)__skel.morphs.size(); ++i) {
                        if (!__filter.empty() && __skel.morphs[i].name.find(__filter) == std::string::npos) continue;
                        ImGui::PushID((int)(i + 100000u));
                        if (ImGui::Selectable(__skel.morphs[i].name.c_str(), sel_type == TRACK_MORPH && sel_index == i)) select_(TRACK_MORPH, i);
                        ImGui::PopID();
                    }
                    ImGui::EndCombo();
                }
            }
            if (tracks_dirty) buildTracks_();
            const Track *__track = selectedTrack_();
            if (!__track || __track->type == TRACK_NONE) {
                ImGui::TextDisabled("Select a track in the timeline or a bone / morph above");
                ImGui::End();
                return;
            }
            const uint32_t __frame = (uint32_t)std::lround(std::max(frame, 0.f));
            const bool __has_key = keyAt_(*__track, __frame) >= 0;
            ImGui::Text("Frame %u: %s%s", __frame, __has_key ? "key" : "no key", edit_active ? "  (edited, not registered)" : "");

            bool __edited = false;
            if (__track->type == TRACK_BONE) {
                __edited |= ImGui::DragFloat3("Translation", &edit_translation.x, 0.02f);
                __edited |= ImGui::DragFloat3("Rotation", &edit_rotation.x, 0.25f, -360.f, 360.f, "%.2f deg");
                ImGui::TextDisabled("Rotation: X pitch, Y yaw, Z roll (in the bone's rest space)");
            } else if (__track->type == TRACK_MORPH) {
                __edited |= ImGui::SliderFloat("Weight", &edit_weight, 0.f, 1.f);
            } else {
                __edited |= ImGui::DragFloat3("Center", &edit_camera.center.x, 0.05f);
                vec3 __deg = edit_camera.rotation * (1.f / DEG);
                if (ImGui::DragFloat3("Rotation", &__deg.x, 0.25f, -3600.f, 3600.f, "%.2f deg")) { edit_camera.rotation = __deg * DEG; __edited = true; }
                __edited |= ImGui::DragFloat("Distance", &edit_camera.distance, 0.1f);
                __edited |= ImGui::DragFloat("FOV", &edit_camera.fov, 0.25f, 1.f, 175.f, "%.0f deg");
                __edited |= ImGui::Checkbox("Perspective", &edit_camera.perspective);
            }
            if (__edited) { edit_active = true; pose_changed = true; }

            if (ImGui::Button("Register key (Enter)")) registerKey_();
            ImGui::SameLine();
            ImGui::BeginDisabled(!__has_key);
            if (ImGui::Button("Delete key (Del)")) deleteKey_();
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::BeginDisabled(!edit_active);
            if (ImGui::Button("Reset")) { edit_active = false; pose_changed = true; }
            ImGui::EndDisabled();

            curveEditor_();
            ImGui::End();
        }
    }; // Editor END
}; // Simple END
