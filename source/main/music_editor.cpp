#include "main/music_editor.h"

#include <cctype>
#include <cfloat>
#include <cstring>


namespace Simple {
    namespace Editor {
        using Audio::Tracker::Cell;
        typedef std::lock_guard<std::mutex> SongLock;

        static const char* const Tracker_Action_Names[TrackerAction::COUNT - TrackerAction::NOTE_OFF] = {
            "note_off", "note_cut", "octave_up", "octave_down", "cursor_up", "cursor_down", "cursor_left", "cursor_right",
            "page_up", "page_down", "row_first", "row_last", "channel_next", "channel_prev", "clear", "insert_row", "delete_row",
            "play_pattern", "play_song", "type"
        };

        static const char* const Wave_Items[Audio::Synth::WAVE_COUNT] = { "Sine", "Pulse", "Triangle", "Saw", "Noise", "Noise (periodic)", "Wavetable" };
        static const char* const Curve_Items[Audio::Synth::CURVE_COUNT] = { "Linear", "Exponential" };

        static const char* const Effects_Help =
            "0xy  Arpeggio: note, +x, +y semitones\n"
            "1xx  Slide up, xx/16 semitones per tick\n"
            "2xx  Slide down\n"
            "3xx  Portamento to the note\n"
            "4xy  Vibrato: speed x, depth y/8 semitones\n"
            "8xx  Pan, 80 - center\n"
            "Axy  Volume slide: x up, y down\n"
            "Bxx  Jump to order xx\n"
            "Dxx  Pattern break to row xx\n"
            "Fxx  Ticks per row (< 20) or BPM\n"
            "ECx  Note cut at tick x\n"
            "EDx  Note delay to tick x";


        TrackerKeys::TrackerKeys() {
            for (uint32_t i = TrackerAction::NOTE_FIRST; i <= TrackerAction::NOTE_LAST; ++i)
                addAction("note_" + std::to_string(i - TrackerAction::NOTE_FIRST));
            for (uint32_t i = TrackerAction::NOTE_OFF; i < TrackerAction::COUNT; ++i)
                addAction(Tracker_Action_Names[i - TrackerAction::NOTE_OFF]);
            enabled = false;
        }

        void TrackerKeys::onAction(WindowInput::ActionId _action, int _glfw_action) {
            // Note: drained by the editor every frame, the limit only guards against a stopped editor
            if (events.size() < 256u)
                events.push_back(KeyEvent{_action, _glfw_action});
        }


        MusicStudio::MusicStudio(const std::string &_library_path, const std::string &_song_path) {
            for (uint32_t i = 0u; i <= TrackerAction::NOTE_LAST; ++i) {
                key_notes[i] = -1;
                key_channels[i] = 0u;
            }
            keys.loadInputMap("resources/input_maps/tracker.json");

            snprintf(library_path, sizeof(library_path), "%s", _library_path.c_str());
            snprintf(song_path,    sizeof(song_path),    "%s", _song_path.c_str());
            if (!_library_path.empty() && !library.load(_library_path))
                status = "Could not load the library '" + _library_path + "'";
            if (!_song_path.empty())
                loadSong_(_song_path);
            player.setSong(&song);
        }

        MusicStudio::~MusicStudio() { stopAudio(); }

        bool MusicStudio::startAudio() {
            if (audio_running) return true;
            try {
                audio.streamSetMode(RTA::STREAM_MODE_OUT, RTA::DEVICE_MODE_DEFAULT);
                audio.deviceUpdate();
                audio.streamSetOptions(0, 0, "SimpleVKFW Music", 0);
                audio.deviceOutputSetParameters(2);
                audio.callbackSet(RTA::rtacbDefault2);
                audio.streamOpen(48000u, 512u, RTAUDIO_SINT16, false);

                const uint32_t __sample_rate = audio.stream_config.sample_rate;
                player.setSampleRate(__sample_rate);
                preview.setSampleRate(__sample_rate);
                audio.stream_config.audio_sampler.setSources({&player, &preview}, {1.f, 1.f});
                audio.stream_config.audio_sampler.setSampleRate(__sample_rate);
                audio.streamStart();
                audio_running = true;
                audio_error.clear();
            }
            catch (const std::exception &_e) {
                audio_error = _e.what();
                fprintf(svkfwwarn, SVKFW_WRAPWARN("Editor::MusicStudio :: startAudio", "%s\n"), _e.what());
            }
            return audio_running;
        }

        void MusicStudio::stopAudio() {
            if (!audio_running) return;
            if (audio.streamIsRunning()) audio.streamStop();
            audio_running = false;
        }


        Audio::Synth::Instrument* MusicStudio::editedInstrument_() {
            if (edit_target == EDIT_SONG)
                return (song_instrument >= 0 && size_t(song_instrument) < song.instruments.size()) ? &song.instruments[song_instrument] : nullptr;
            return (lib_selected >= 0 && uint32_t(lib_selected) < library.size()) ? &library.instruments[lib_selected] : nullptr;
        }

        void MusicStudio::commitInstrument_(const Audio::Synth::InstrumentParams &_params) {
            Audio::Synth::Instrument *__instrument = editedInstrument_();
            if (__instrument == nullptr) return;
            if (edit_target == EDIT_SONG) {
                // Note: the audio thread copies song instruments on note on
                SongLock __lock(player.mutex);
                __instrument->params = _params;
            }
            else {
                __instrument->params = _params;
                library.is_modified = true;
            }
        }

        void MusicStudio::loadSong_(const std::string &_fpath) {
            Audio::Tracker::Song __song;
            if (!__song.load(_fpath)) {
                status = "Could not load the song '" + _fpath + "'";
                return;
            }
            {
                SongLock __lock(player.mutex);
                player.playing = false;
                song = __song;
                player.refreshTuning_();
            }
            player.stopNow();
            current_order = current_pattern = 0u;
            cursor = ImGuiCustom::TextGridCursor{};
            song_instrument = 0;
            status = "Loaded '" + _fpath + "'";
        }

        uint32_t MusicStudio::numRows_() const {
            return current_pattern < song.patterns.size() ? song.patterns[current_pattern].rows : 1u;
        }


        bool MusicStudio::begin() { return true; }

        void MusicStudio::end() {}

        void MusicStudio::render() {
            handleKeys_();
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("Music")) {
                    ImGui::MenuItem("Instrument library", nullptr, &show_library);
                    ImGui::MenuItem("Instrument editor",  nullptr, &show_instrument);
                    ImGui::MenuItem("Song",               nullptr, &show_song);
                    ImGui::MenuItem("Pattern editor",     nullptr, &show_pattern);
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
            pattern_focused = instrument_focused = false;
            if (show_library)    windowLibrary();
            if (show_instrument) windowInstrument();
            if (show_song)       windowSong();
            if (show_pattern)    windowPattern();
            // Note: takes effect for the next frame's events
            keys.setEnabled(pattern_focused || instrument_focused);
        }

        void MusicStudio::renderFull() {
            if (begin()) render();
            end();
        }


    //  == === ====     Instrument library     ==== === ==  \\

        void MusicStudio::windowLibrary() {
            ImGui::SetNextWindowPos(ImVec2(10.f, 40.f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(560.f, 330.f), ImGuiCond_FirstUseEver);
            if (!ImGui::Begin("Instrument library", &show_library)) { ImGui::End(); return; }

            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText("##library_path", library_path, sizeof(library_path));
            if (ImGui::Button("Load")) {
                if (library.load(library_path)) {
                    lib_selected = 0;
                    status = "Loaded the library";
                }
                else status = "Could not load the library";
            }
            ImGui::SameLine();
            if (ImGui::Button(library.is_modified ? "Save*" : "Save"))
                status = library.save(library_path) ? "Saved the library" : "Could not save the library";

            ImGui::SeparatorText("Instruments");
            if (ImGui::BeginListBox("##library", ImVec2(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 10.f))) {
                for (uint32_t i = 0u; i < library.size(); ++i) {
                    ImGui::PushID(int(i));
                    const bool __selected = edit_target == EDIT_LIBRARY && int32_t(i) == lib_selected;
                    if (ImGui::Selectable(library.instruments[i].name.c_str(), __selected)) {
                        lib_selected = int32_t(i);
                        edit_target = EDIT_LIBRARY;
                    }
                    ImGui::PopID();
                }
                ImGui::EndListBox();
            }

            if (ImGui::Button("New"))       { lib_selected = int32_t(library.addDefault());     edit_target = EDIT_LIBRARY; }
            ImGui::SameLine();
            const bool __valid = lib_selected >= 0 && uint32_t(lib_selected) < library.size();
            ImGui::BeginDisabled(!__valid);
            if (ImGui::Button("Duplicate")) { lib_selected = int32_t(library.duplicate(uint32_t(lib_selected))); edit_target = EDIT_LIBRARY; }
            ImGui::SameLine();
            if (ImGui::Button("Delete")) ImGui::OpenPopup("Delete instrument?");
            if (ImGui::BeginPopupModal("Delete instrument?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Delete '%s' from the library?", __valid ? library.instruments[lib_selected].name.c_str() : "");
                ImGui::TextDisabled("Songs keep their own copies.");
                if (ImGui::Button("Delete")) {
                    library.remove(uint32_t(lib_selected));
                    lib_selected = std::min(lib_selected, int32_t(library.size()) - 1);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }

            ImGui::SeparatorText("Song");
            if (ImGui::Button("Copy to song") && __valid) {
                SongLock __lock(player.mutex);
                const uint8_t __id = song.setInstrument(library.instruments[lib_selected]);
                if (__id != Cell::Instrument_None) song_instrument = __id;
                status = "Copied '" + library.instruments[lib_selected].name + "' to the song";
            }
            ImGui::SetItemTooltip("Replaces the song instrument of the same name, or adds it");
            ImGui::EndDisabled();
            ImGui::SameLine();
            const bool __song_valid = song_instrument >= 0 && size_t(song_instrument) < song.instruments.size();
            ImGui::BeginDisabled(!__song_valid);
            if (ImGui::Button("Copy from song") && __song_valid) {
                const Audio::Synth::Instrument &__instrument = song.instruments[song_instrument];
                const int32_t __id = library.find(__instrument.name);
                if (__id >= 0) {
                    library.instruments[__id] = __instrument;
                    library.is_modified = true;
                    lib_selected = __id;
                }
                else lib_selected = int32_t(library.add(__instrument));
                status = "Copied '" + __instrument.name + "' to the library";
            }
            ImGui::SetItemTooltip("Saves the selected song instrument into the library");
            ImGui::EndDisabled();

            if (!status.empty()) ImGui::TextDisabled("%s", status.c_str());
            if (!audio_running)
                ImGui::TextColored(ImVec4(1.f, 0.5f, 0.3f, 1.f), "No audio: %s", audio_error.empty() ? "not started" : audio_error.c_str());
            ImGui::End();
        }


    //  == === ====     Instrument editor     ==== === ==  \\

        void MusicStudio::windowInstrument() {
            using namespace Audio::Synth;
            ImGui::SetNextWindowPos(ImVec2(10.f, 380.f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(560.f, 510.f), ImGuiCond_FirstUseEver);
            if (!ImGui::Begin("Instrument editor", &show_instrument)) { ImGui::End(); return; }
            instrument_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

            Instrument *__instrument = editedInstrument_();
            if (__instrument == nullptr) {
                ImGui::TextDisabled("Select an instrument in the library or the song");
                ImGui::End();
                return;
            }

            ImGui::TextDisabled(edit_target == EDIT_SONG ? "Song instrument %02X" : "Library instrument", song_instrument);
            // Renaming: library names stay unique, song names too (they are matched when copying)
            std::snprintf(name_buf, sizeof(name_buf), "%s", __instrument->name.c_str());
            if (ImGui::InputText("Name", name_buf, sizeof(name_buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (edit_target == EDIT_LIBRARY) library.rename(uint32_t(lib_selected), name_buf);
                else if (name_buf[0] != '\0' && song.findInstrument(name_buf) < 0) {
                    SongLock __lock(player.mutex);
                    __instrument->name = name_buf;
                }
            }

            InstrumentParams __params = __instrument->params;
            bool __changed = false;

            __changed |= ImGui::SliderFloat("Volume", &__params.volume, 0.f, 1.f);
            __changed |= ImGui::SliderFloat("Pan", &__params.pan, -1.f, 1.f);
            __changed |= ImGui::InputInt("Octave", &__params.octave);
            __params.octave = std::min(std::max(__params.octave, -4), 4);

            ImGui::SeparatorText("Oscillators");
            int32_t __remove = -1;
            for (uint32_t i = 0u; i < __params.num_oscillators; ++i) {
                Oscillator &__osc = __params.oscillators[i];
                ImGui::PushID(int(i));
                int __wave = __osc.wave;
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * 9.f);
                if (ImGui::Combo("##wave", &__wave, Wave_Items, WAVE_COUNT)) {
                    __osc.wave = Waveform(__wave);
                    if (__osc.wave == WAVE_TABLE && __osc.table_len == 0u)
                        __osc.setTable(std::vector<uint32_t>(32u, 8u), 4u);
                    __changed = true;
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Remove")) __remove = int32_t(i);
                __changed |= ImGuiCustom::Knob("Vol##osc", &__osc.volume, 0.f, 1.f, "%.2f");
                ImGui::SameLine();
                __changed |= ImGuiCustom::Knob("Detune", &__osc.detune, -24.f, 24.f, "%.2f st");
                ImGui::SameLine();
                __changed |= ImGuiCustom::Knob("Phase", &__osc.phase, 0.f, 1.f, "%.2f");
                if (__osc.wave == WAVE_PULSE) {
                    ImGui::SameLine();
                    __changed |= ImGuiCustom::Knob("Duty", &__osc.duty, 0.01f, 0.99f, "%.3f");
                    ImGui::SameLine();
                    __changed |= ImGuiCustom::Knob("PWM", &__osc.pwm_depth, 0.f, 0.49f, "%.2f");
                    ImGui::SameLine();
                    __changed |= ImGuiCustom::Knob("PWM Hz", &__osc.pwm_rate, 0.f, 20.f, "%.2f Hz");
                }
                int __bits = __osc.bits;
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * 6.f);
                if (ImGui::SliderInt("Bits", &__bits, 0, 16, __bits == 0 ? "off" : "%d")) {
                    __osc.bits = uint8_t(__bits);
                    __changed = true;
                }
                if (__osc.wave == WAVE_PULSE || __osc.wave == WAVE_SAW) {
                    ImGui::SameLine();
                    __changed |= ImGui::Checkbox("Antialias", &__osc.antialias);
                }
                if (__osc.wave == WAVE_TABLE) {
                    int __len = __osc.table_len, __tbits = __osc.table_bits;
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 6.f);
                    if (ImGui::SliderInt("Length", &__len, 1, int(SVKFW_SYNTH_MAX_TABLE))) {
                        std::vector<uint32_t> __values = __osc.getTable();
                        __values.resize(size_t(__len), __values.empty() ? 0u : __values.back());
                        __osc.setTable(__values, __osc.table_bits);
                        __changed = true;
                    }
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 6.f);
                    if (ImGui::SliderInt("Table bits", &__tbits, 1, 8)) {
                        const std::vector<uint32_t> __values = __osc.getTable();
                        std::vector<uint32_t> __scaled(__values.size());
                        const float __scale = float((1u << __tbits) - 1u) / float((1u << __osc.table_bits) - 1u);
                        for (size_t v = 0u; v < __values.size(); ++v) __scaled[v] = uint32_t(__values[v] * __scale + 0.5f);
                        __osc.setTable(__scaled, uint8_t(__tbits));
                        __changed = true;
                    }
                    __changed |= ImGuiCustom::WavetableEditor("##table", __osc.table, __osc.table_len, 1 << __osc.table_bits,
                                                              ImVec2(0.f, ImGui::GetTextLineHeight() * 4.f));
                }
                ImGui::Separator();
                ImGui::PopID();
            }
            if (__remove >= 0) {
                for (uint32_t i = uint32_t(__remove); i + 1u < __params.num_oscillators; ++i)
                    __params.oscillators[i] = __params.oscillators[i + 1u];
                --__params.num_oscillators;
                __changed = true;
            }
            ImGui::BeginDisabled(__params.num_oscillators >= SVKFW_SYNTH_MAX_OSCILLATORS);
            if (ImGui::Button("Add oscillator")) {
                __params.oscillators[__params.num_oscillators++] = Oscillator{};
                __changed = true;
            }
            ImGui::EndDisabled();

            ImGui::SeparatorText("Amplitude envelope");
            EnvelopeADSR &__env = __params.amp_env;
            __changed |= ImGuiCustom::EnvelopeADSR("##adsr", &__env.attack, &__env.decay, &__env.sustain, &__env.release, 2.f,
                                                   __env.curve == CURVE_EXP, ImVec2(0.f, ImGui::GetTextLineHeight() * 5.f));
            __changed |= ImGuiCustom::Knob("Attack",  &__env.attack,  0.f, 2.f, "%.3f s"); ImGui::SameLine();
            __changed |= ImGuiCustom::Knob("Decay",   &__env.decay,   0.f, 2.f, "%.3f s"); ImGui::SameLine();
            __changed |= ImGuiCustom::Knob("Sustain", &__env.sustain, 0.f, 1.f, "%.2f");   ImGui::SameLine();
            __changed |= ImGuiCustom::Knob("Release", &__env.release, 0.f, 2.f, "%.3f s"); ImGui::SameLine();
            int __curve = __env.curve;
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7.f);
            if (ImGui::Combo("##curve", &__curve, Curve_Items, CURVE_COUNT)) {
                __env.curve = EnvelopeCurve(__curve);
                __changed = true;
            }

            ImGui::SeparatorText("Modulation");
            __changed |= ImGuiCustom::Knob("Vib depth", &__params.vibrato.depth, 0.f, 2.f, "%.2f st"); ImGui::SameLine();
            __changed |= ImGuiCustom::Knob("Vib rate",  &__params.vibrato.rate,  0.f, 16.f, "%.2f Hz"); ImGui::SameLine();
            __changed |= ImGuiCustom::Knob("Vib delay", &__params.vibrato.delay, 0.f, 2.f, "%.2f s");  ImGui::SameLine();
            __changed |= ImGuiCustom::Knob("Pitch env", &__params.pitch_env.semitones, -48.f, 48.f, "%.1f st"); ImGui::SameLine();
            __changed |= ImGuiCustom::Knob("Pitch time", &__params.pitch_env.time, 0.f, 1.f, "%.3f s");

            if (__changed) commitInstrument_(__params);

            // Preview of A-4 held 0.3 s, rendered again only when the parameters change
            if (!wave_preview_valid || std::memcmp(&wave_preview_params, &__params, sizeof(__params)) != 0) {
                wave_preview = renderNote(__params, 440.f, 0.3f, 48000u, 3.f);
                wave_preview_params = __params;
                wave_preview_valid = true;
            }
            ImGui::SeparatorText("Preview (A-4)");
            const int __cycle = std::min(int(wave_preview.size()), 48000 / 440 * 3); // About 3 periods after the attack
            const int __offset = std::min(int(wave_preview.size()) - __cycle, int(std::min(__params.amp_env.attack, 0.1f) * 48000.f));
            const float __width = ImGui::GetContentRegionAvail().x;
            if (!wave_preview.empty()) {
                ImGuiCustom::Waveform("##cycle", wave_preview.data() + std::max(__offset, 0), __cycle, -1.f, 1.f, ImVec2(__width * 0.35f, ImGui::GetTextLineHeight() * 3.f));
                ImGui::SameLine();
                ImGuiCustom::Waveform("##note", wave_preview.data(), int(wave_preview.size()), -1.f, 1.f, ImVec2(0.f, ImGui::GetTextLineHeight() * 3.f));
            }

            ImGui::SeparatorText("Keyboard");
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 6.f);
            ImGui::InputInt("Edit octave", &octave);
            octave = std::min(std::max(octave, 0), 8);
            int __note_on = -1, __note_off = -1;
            NotePreview *__preview = &preview;
            if (ImGuiCustom::PianoKeyboard("##piano", std::max(octave - 1, 0) * 12, 3, [__preview](int _note) { return __preview->isNotePlaying(_note); },
                                           &__note_on, &__note_off, ImVec2(0.f, ImGui::GetTextLineHeight() * 4.f))) {
                if (__note_off >= 0) preview.noteOff(__note_off);
                if (__note_on  >= 0) preview.noteOn(__params, __note_on);
            }
            ImGui::TextDisabled("Keys: Z S X ... (lower octave), Q 2 W ... (upper) while this window is focused");
            ImGui::End();
        }


    //  == === ====     Song     ==== === ==  \\

        void MusicStudio::windowSong() {
            ImGui::SetNextWindowPos(ImVec2(1180.f, 40.f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(410.f, 850.f), ImGuiCond_FirstUseEver);
            if (!ImGui::Begin("Song", &show_song)) { ImGui::End(); return; }

            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText("##song_path", song_path, sizeof(song_path));
            if (ImGui::Button("Load")) loadSong_(song_path);
            ImGui::SameLine();
            if (ImGui::Button("Save")) status = song.save(song_path) ? "Saved the song" : "Could not save the song";
            ImGui::SameLine();
            if (ImGui::Button("New")) {
                player.stopNow();
                {
                    SongLock __lock(player.mutex);
                    song = Audio::Tracker::Song{};
                    player.refreshTuning_();
                }
                current_order = current_pattern = 0u;
                cursor = ImGuiCustom::TextGridCursor{};
            }
            ImGui::SameLine();
            if (ImGui::Button("Export WAV")) {
                std::string __wav = song_path;
                const size_t __dot = __wav.find_last_of('.');
                __wav = (__dot == std::string::npos ? __wav : __wav.substr(0u, __dot)) + ".wav";
                std::vector<vec2f> __frames;
                {
                    // Note: renders a copy, so playback and editing go on
                    Audio::Tracker::Song __copy;
                    { SongLock __lock(player.mutex); __copy = song; }
                    __frames = Audio::Tracker::SongPlayer::renderSong(__copy, 48000u);
                }
                status = Audio::Synth::writeWAV(__wav, __frames, 48000u) ? "Exported '" + __wav + "'" : "Could not export '" + __wav + "'";
            }

            // Transport
            ImGui::SeparatorText("Playback");
            const bool __playing = player.playing;
            if (ImGui::Button(__playing ? "Stop" : "Play song")) {
                if (__playing) player.stop();
                else player.play(current_order);
            }
            ImGui::SameLine();
            if (ImGui::Button("Play pattern")) player.playPattern(current_pattern);
            ImGui::SameLine();
            ImGui::Checkbox("Loop", &player.loop);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 6.f);
            ImGui::SliderFloat("Master", &player.master_volume, 0.f, 1.f, "%.2f");
            ImGui::Text("Order %02X  Row %02X", unsigned(player.play_order), unsigned(player.play_row));

            {
                SongLock __lock(player.mutex);
                char __name[128];
                std::snprintf(__name, sizeof(__name), "%s", song.name.c_str());
                if (ImGui::InputText("Title", __name, sizeof(__name))) song.name = __name;

                ImGui::SetNextItemWidth(ImGui::GetFontSize() * 6.f);
                ImGui::DragFloat("BPM", &song.bpm, 0.5f, 20.f, 400.f, "%.1f");
                ImGui::SameLine();
                int __rpb = int(song.rows_per_beat), __tpr = int(song.ticks_per_row);
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5.f);
                if (ImGui::InputInt("Rows/beat", &__rpb)) song.rows_per_beat = uint32_t(std::min(std::max(__rpb, 1), 16));
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5.f);
                if (ImGui::InputInt("Ticks/row", &__tpr)) song.ticks_per_row = uint32_t(std::min(std::max(__tpr, 1), 31));

                int __tuning = song.tuning == Audio::Music::JUST_INTONATION ? 1 : 0;
                const char *const __tunings[2] = { "12-TET", "Just intonation" };
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * 9.f);
                bool __tuning_changed = ImGui::Combo("Tuning", &__tuning, __tunings, 2);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5.f);
                __tuning_changed |= ImGui::DragFloat("A-4, Hz", &song.ref_freq, 0.1f, 400.f, 480.f, "%.1f");
                if (__tuning_changed) {
                    song.tuning = __tuning ? Audio::Music::JUST_INTONATION : Audio::Music::EQUAL_TEMPERAMENT_12T;
                    player.refreshTuning_();
                }

                if (ImGui::TreeNode("Echo")) {
                    ImGui::SliderFloat("Delay",    &song.echo.delay, 0.01f, SVKFW_TRACKER_MAX_ECHO, "%.3f s");
                    ImGui::SliderFloat("Feedback", &song.echo.feedback, 0.f, 0.95f);
                    ImGui::SliderFloat("Volume##echo", &song.echo.volume, 0.f, 1.f);
                    ImGui::SliderFloat("Low-pass", &song.echo.lowpass, 0.f, 0.95f);
                    ImGui::TreePop();
                }

                // Channels
                ImGui::SeparatorText("Channels");
                int __channels = int(song.numChannels());
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5.f);
                if (ImGui::InputInt("Count", &__channels)) {
                    song.setNumChannels(uint32_t(std::min(std::max(__channels, 1), int(SVKFW_TRACKER_MAX_CHANNELS))));
                    cursor.column = std::min(cursor.column, int(song.numChannels()) - 1);
                }
                if (ImGui::BeginTable("##channels", 6, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX)) {
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 5.f);
                    ImGui::TableSetupColumn("Vol");
                    ImGui::TableSetupColumn("Pan");
                    ImGui::TableSetupColumn("M/S");
                    ImGui::TableSetupColumn("Echo");
                    ImGui::TableSetupColumn("Level");
                    ImGui::TableHeadersRow();
                    for (uint32_t ch = 0u; ch < song.numChannels(); ++ch) {
                        Audio::Tracker::ChannelSettings &__settings = song.channels[ch];
                        ImGui::PushID(int(ch));
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        char __channel_name[32];
                        std::snprintf(__channel_name, sizeof(__channel_name), "%s", __settings.name.c_str());
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        if (ImGui::InputText("##name", __channel_name, sizeof(__channel_name))) __settings.name = __channel_name;
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 3.f);
                        ImGui::DragFloat("##vol", &__settings.volume, 0.01f, 0.f, 1.f, "%.2f");
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 3.f);
                        ImGui::DragFloat("##pan", &__settings.pan, 0.01f, -1.f, 1.f, "%.2f");
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("##mute", &__settings.muted);
                        ImGui::SetItemTooltip("Mute");
                        ImGui::SameLine();
                        ImGui::Checkbox("##solo", &__settings.solo);
                        ImGui::SetItemTooltip("Solo");
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("##echo", &__settings.echo);
                        ImGui::TableNextColumn();
                        const vec2f __peak = player.channels[ch].peak;
                        ImGuiCustom::VuMeter("##vu", __peak.x, __peak.y, ImVec2(ImGui::GetFontSize() * 4.f, ImGui::GetTextLineHeight() * 0.6f));
                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                }

                // Song instruments: the column "Ins" of the cells refers to these indices
                ImGui::SeparatorText("Instruments");
                if (ImGui::BeginListBox("##song_instruments", ImVec2(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 6.f))) {
                    for (size_t i = 0u; i < song.instruments.size(); ++i) {
                        char __label[96];
                        std::snprintf(__label, sizeof(__label), "%02X  %s", unsigned(i), song.instruments[i].name.c_str());
                        if (ImGui::Selectable(__label, edit_target == EDIT_SONG && int32_t(i) == song_instrument)) {
                            song_instrument = int32_t(i);
                            edit_target = EDIT_SONG;
                        }
                    }
                    ImGui::EndListBox();
                }
                ImGui::TextDisabled("Add instruments with \"Copy to song\" in the library");

                // Order list
                ImGui::SeparatorText("Order");
                if (ImGui::BeginListBox("##order", ImVec2(ImGui::GetFontSize() * 6.f, ImGui::GetTextLineHeightWithSpacing() * 8.f))) {
                    for (size_t i = 0u; i < song.order.size(); ++i) {
                        char __label[32];
                        std::snprintf(__label, sizeof(__label), "%02X: %02X", unsigned(i), song.order[i]);
                        const bool __is_playing = player.playing && !player.pattern_mode && player.play_order == i;
                        if (__is_playing) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_PlotHistogram));
                        if (ImGui::Selectable(__label, i == current_order)) {
                            current_order = uint32_t(i);
                            current_pattern = song.order[i];
                        }
                        if (__is_playing) ImGui::PopStyleColor();
                    }
                    ImGui::EndListBox();
                }
                ImGui::SameLine();
                ImGui::BeginGroup();
                current_order = std::min(current_order, uint32_t(song.order.size() - 1u));
                int __pattern_id = int(song.order[current_order]);
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5.f);
                if (ImGui::InputInt("Pattern", &__pattern_id)) {
                    song.order[current_order] = uint32_t(std::min(std::max(__pattern_id, 0), int(song.patterns.size()) - 1));
                    current_pattern = song.order[current_order];
                }
                if (ImGui::Button("Insert")) {
                    song.order.insert(song.order.begin() + current_order + 1u, song.order[current_order]);
                    ++current_order;
                }
                ImGui::SameLine();
                ImGui::BeginDisabled(song.order.size() <= 1u);
                if (ImGui::Button("Remove")) {
                    song.order.erase(song.order.begin() + current_order);
                    current_order = std::min(current_order, uint32_t(song.order.size() - 1u));
                    current_pattern = song.order[current_order];
                }
                ImGui::EndDisabled();
                if (ImGui::Button("New pattern")) {
                    current_pattern = song.addPattern(numRows_());
                    song.order.insert(song.order.begin() + current_order + 1u, current_pattern);
                    ++current_order;
                }
                ImGui::SameLine();
                if (ImGui::Button("Clone pattern")) {
                    song.patterns.push_back(song.patterns[current_pattern]);
                    current_pattern = uint32_t(song.patterns.size() - 1u);
                    song.order.insert(song.order.begin() + current_order + 1u, current_pattern);
                    ++current_order;
                }
                int __restart = int(song.restart_order);
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5.f);
                if (ImGui::InputInt("Restart", &__restart))
                    song.restart_order = uint32_t(std::min(std::max(__restart, 0), int(song.order.size()) - 1));
                ImGui::SetItemTooltip("Order the song loops to");
                ImGui::EndGroup();
            }
            ImGui::End();
        }


    //  == === ====     Pattern editor     ==== === ==  \\

        void MusicStudio::windowPattern() {
            ImGui::SetNextWindowPos(ImVec2(580.f, 40.f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(590.f, 850.f), ImGuiCond_FirstUseEver);
            if (!ImGui::Begin("Pattern editor", &show_pattern)) { ImGui::End(); return; }
            pattern_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

            SongLock __lock(player.mutex);
            current_pattern = std::min(current_pattern, uint32_t(song.patterns.size() - 1u));

            // Follows the song position while playing
            const bool __playing = player.playing;
            const uint32_t __play_pattern = player.pattern_mode ? player.pattern_mode_id :
                                            (player.play_order < song.order.size() ? song.order[player.play_order] : 0u);
            if (__playing && follow) {
                if (!player.pattern_mode) current_order = player.play_order;
                current_pattern = __play_pattern;
                scroll_to = int32_t(player.play_row);
            }

            int __pattern = int(current_pattern);
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5.f);
            if (ImGui::InputInt("Pattern", &__pattern)) current_pattern = uint32_t(std::min(std::max(__pattern, 0), int(song.patterns.size()) - 1));
            ImGui::SameLine();
            int __rows = int(numRows_());
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5.f);
            if (ImGui::InputInt("Rows", &__rows, 16, 16)) {
                song.setPatternRows(current_pattern, uint32_t(std::min(std::max(__rows, 1), int(SVKFW_TRACKER_MAX_ROWS))));
                cursor.row = std::min(cursor.row, int(numRows_()) - 1);
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4.f);
            ImGui::InputInt("Octave", &octave);
            octave = std::min(std::max(octave, 0), 8);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4.f);
            ImGui::InputInt("Step", &edit_step);
            edit_step = std::min(std::max(edit_step, 0), 16);
            ImGui::SameLine();
            ImGui::Checkbox("Follow", &follow);
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::BeginItemTooltip()) {
                ImGui::TextUnformatted(Effects_Help);
                ImGui::Separator();
                ImGui::TextUnformatted("Notes: Z S X D C ... / Q 2 W 3 E ...  Off: 1  Cut: \\  Octave: Ctrl+Up/Down\n"
                                       "Delete - clear, Insert/Backspace - insert/remove row, Space - play pattern, Enter - play song");
                ImGui::EndTooltip();
            }
            const bool __has_instrument = song_instrument >= 0 && size_t(song_instrument) < song.instruments.size();
            ImGui::Text("Instrument: %s", __has_instrument ? song.instruments[song_instrument].name.c_str() : "none (copy one from the library)");

            ImGuiCustom::TextGridStyle __style;
            __style.sub_widths = {3, 2, 2, 3};
            __style.sub_colors = {0u, IM_COL32(110, 200, 240, 255), IM_COL32(130, 220, 120, 255), IM_COL32(240, 170, 90, 255)};
            __style.beat_rows = int(song.rows_per_beat);
            __style.bar_rows  = int(song.rows_per_beat * 4u);

            const Audio::Tracker::Song &__song = song;
            const uint32_t __pattern_id = current_pattern;
            cursor.column = std::min(cursor.column, int(song.numChannels()) - 1);
            const int __play_row = (__playing && __play_pattern == current_pattern) ? int(player.play_row) : -1;
            if (ImGuiCustom::TextGrid("##grid", int(numRows_()), int(song.numChannels()),
                                      [&__song](int _c) { return __song.channels[_c].name.c_str(); }, __style,
                                      [&__song, __pattern_id](int _r, int _c, int _s, char *_buf, int _size) {
                                          const Cell &__cell = __song.cell(__pattern_id, uint32_t(_r), uint32_t(_c));
                                          switch (_s) {
                                              case 0: std::snprintf(_buf, size_t(_size), "%s", Cell::noteToString(__cell.note).c_str()); break;
                                              case 1: if (__cell.instrument == Cell::Instrument_None) std::snprintf(_buf, size_t(_size), "..");
                                                      else std::snprintf(_buf, size_t(_size), "%02X", __cell.instrument);
                                                      break;
                                              case 2: if (__cell.volume == Cell::Volume_None) std::snprintf(_buf, size_t(_size), "..");
                                                      else std::snprintf(_buf, size_t(_size), "%02X", __cell.volume);
                                                      break;
                                              default: if (__cell.effect == 0) std::snprintf(_buf, size_t(_size), "...");
                                                       else std::snprintf(_buf, size_t(_size), "%c%02X", __cell.effect, __cell.param);
                                                       break;
                                          }
                                      },
                                      cursor, __play_row, scroll_to, ImVec2(0.f, 0.f)))
                typed_digits = 0u;
            scroll_to = -1;
            ImGui::End();
        }


    //  == === ====     Keyboard     ==== === ==  \\

        void MusicStudio::moveCursor_(int32_t _rows, int32_t _subs) {
            const int32_t __rows = int32_t(numRows_()), __channels = int32_t(song.numChannels());
            cursor.row = ((cursor.row + _rows) % __rows + __rows) % __rows;
            int32_t __sub = cursor.column * 4 + cursor.sub + _subs;
            __sub = ((__sub % (__channels * 4)) + __channels * 4) % (__channels * 4);
            cursor.column = __sub / 4;
            cursor.sub = __sub % 4;
            typed_digits = 0u;
            scroll_to = cursor.row;
        }

        void MusicStudio::advance_() {
            if (edit_step > 0) moveCursor_(edit_step, 0);
            typed_digits = 0u;
        }

        void MusicStudio::typeHex_(char _ch) {
            Cell &__cell = song.cell(current_pattern, uint32_t(cursor.row), uint32_t(cursor.column));
            const char __upper = char(std::toupper((unsigned char)_ch));
            int32_t __digit = -1;
            if (__upper >= '0' && __upper <= '9') __digit = __upper - '0';
            if (__upper >= 'A' && __upper <= 'F') __digit = __upper - 'A' + 10;

            switch (cursor.sub) {
                case 1: // Instrument: digits shift in from the right
                    if (__digit < 0) return;
                    __cell.instrument = uint8_t(((__cell.instrument == Cell::Instrument_None ? 0u : __cell.instrument) << 4) | uint32_t(__digit));
                    if (__cell.instrument == Cell::Instrument_None) __cell.instrument = 0xFEu;
                    if (++typed_digits >= 2u) advance_();
                    break;
                case 2: // Volume, up to 40
                    if (__digit < 0) return;
                    __cell.volume = uint8_t(std::min(((__cell.volume == Cell::Volume_None ? 0u : __cell.volume) << 4 | uint32_t(__digit)) & 0xFFu, uint32_t(Cell::Volume_Max)));
                    if (++typed_digits >= 2u) advance_();
                    break;
                case 3: // Effect: the letter, then two parameter digits
                    if (typed_digits == 0u) {
                        if (!std::isalnum((unsigned char)__upper)) return;
                        __cell.effect = __upper;
                        typed_digits = 1u;
                    }
                    else {
                        if (__digit < 0) return;
                        __cell.param = uint8_t((__cell.param << 4) | uint32_t(__digit));
                        if (++typed_digits >= 3u) advance_();
                    }
                    break;
                default: break;
            }
        }

        void MusicStudio::handlePatternKey_(WindowInput::ActionId _action, int _glfw_action) {
            using namespace TrackerAction;
            const bool __press = _glfw_action != GLFW_RELEASE;
            const int32_t __rows = int32_t(numRows_());

            if (_action >= NOTE_FIRST && _action <= NOTE_LAST) {
                const uint32_t __key = _action - NOTE_FIRST;
                if (!__press) {
                    // Note: the preview of the entered note stops on the key release
                    if (key_notes[__key] >= 0) {
                        Cell __off;
                        __off.note = Cell::Note_Off;
                        player.previewCell(key_channels[__key], __off);
                        key_notes[__key] = -1;
                    }
                    return;
                }
                if (_glfw_action == GLFW_REPEAT || cursor.sub != 0) return;
                const int32_t __note = octave * 12 + int32_t(__key);
                if (__note < 0 || __note >= int32_t(Cell::Note_Max)) return;
                Cell __cell;
                {
                    SongLock __lock(player.mutex);
                    Cell &__target = song.cell(current_pattern, uint32_t(cursor.row), uint32_t(cursor.column));
                    __target.note = uint8_t(__note + 1);
                    if (song_instrument >= 0 && size_t(song_instrument) < song.instruments.size())
                        __target.instrument = uint8_t(song_instrument);
                    __cell = __target;
                }
                __cell.effect = 0;
                if (!player.playing) {
                    player.previewCell(uint32_t(cursor.column), __cell);
                    key_notes[__key] = __note;
                    key_channels[__key] = uint32_t(cursor.column);
                }
                advance_();
                return;
            }
            if (!__press) return;

            SongLock __lock(player.mutex);
            Cell &__cell = song.cell(current_pattern, uint32_t(cursor.row), uint32_t(cursor.column));
            switch (_action) {
                case NOTE_OFF:
                case NOTE_CUT:
                    if (cursor.sub != 0) break;
                    __cell.note = _action == NOTE_OFF ? Cell::Note_Off : Cell::Note_Cut;
                    __cell.instrument = Cell::Instrument_None;
                    advance_();
                    break;
                case OCTAVE_UP:    octave = std::min(octave + 1, 8); break;
                case OCTAVE_DOWN:  octave = std::max(octave - 1, 0); break;
                case CURSOR_UP:    moveCursor_(-1, 0); break;
                case CURSOR_DOWN:  moveCursor_( 1, 0); break;
                case CURSOR_LEFT:  moveCursor_(0, -1); break;
                case CURSOR_RIGHT: moveCursor_(0,  1); break;
                case PAGE_UP:      moveCursor_(-std::min(cursor.row, int32_t(song.rows_per_beat * 4u)), 0); break;
                case PAGE_DOWN:    moveCursor_( std::min(__rows - 1 - cursor.row, int32_t(song.rows_per_beat * 4u)), 0); break;
                case ROW_FIRST:    moveCursor_(-cursor.row, 0); break;
                case ROW_LAST:     moveCursor_(__rows - 1 - cursor.row, 0); break;
                case CHANNEL_NEXT: moveCursor_(0, 4 - cursor.sub); break;
                case CHANNEL_PREV: moveCursor_(0, -4 - cursor.sub); break;
                case CLEAR:
                    switch (cursor.sub) {
                        case 0:  __cell.note = Cell::Note_None; __cell.instrument = Cell::Instrument_None; break;
                        case 1:  __cell.instrument = Cell::Instrument_None; break;
                        case 2:  __cell.volume = Cell::Volume_None; break;
                        default: __cell.effect = 0; __cell.param = 0u; break;
                    }
                    advance_();
                    break;
                case INSERT_ROW:
                case DELETE_ROW: {
                    const uint32_t __ch = uint32_t(cursor.column);
                    if (_action == INSERT_ROW) {
                        for (int32_t r = __rows - 1; r > cursor.row; --r)
                            song.cell(current_pattern, uint32_t(r), __ch) = song.cell(current_pattern, uint32_t(r - 1), __ch);
                        song.cell(current_pattern, uint32_t(cursor.row), __ch) = Cell{};
                    }
                    else {
                        for (int32_t r = cursor.row; r + 1 < __rows; ++r)
                            song.cell(current_pattern, uint32_t(r), __ch) = song.cell(current_pattern, uint32_t(r + 1), __ch);
                        song.cell(current_pattern, uint32_t(__rows - 1), __ch) = Cell{};
                    }
                    break;
                }
                default: break;
            }
        }

        void MusicStudio::handleKeys_() {
            using namespace TrackerAction;
            const std::vector<TrackerKeys::KeyEvent> __events = keys.events;
            keys.events.clear();

            for (size_t i = 0u; i < __events.size(); ++i) {
                const WindowInput::ActionId __action = __events[i].action;
                const int __glfw_action = __events[i].glfw_action;

                // Transport works in both windows
                if (__glfw_action == GLFW_PRESS && (__action == PLAY_PATTERN || __action == PLAY_SONG)) {
                    if (player.playing) player.stop();
                    else if (__action == PLAY_PATTERN) player.playPattern(current_pattern);
                    else player.play(current_order);
                    continue;
                }

                if (instrument_focused && !pattern_focused) {
                    // Keyjazz with the edited instrument
                    if (__action > NOTE_LAST) continue;
                    const uint32_t __key = __action - NOTE_FIRST;
                    const Audio::Synth::Instrument *__instrument = editedInstrument_();
                    if (__glfw_action == GLFW_PRESS && __instrument != nullptr) {
                        key_notes[__key] = octave * 12 + int32_t(__key);
                        preview.noteOn(__instrument->params, key_notes[__key]);
                    }
                    else if (__glfw_action == GLFW_RELEASE && key_notes[__key] >= 0) {
                        preview.noteOff(key_notes[__key]);
                        key_notes[__key] = -1;
                    }
                    continue;
                }
                handlePatternKey_(__action, __glfw_action);
            }

            // Hex digits and effect letters come as characters (the same keys also play notes in the note column)
            if (pattern_focused && cursor.sub != 0) {
                const ImGuiIO &__io = ImGui::GetIO();
                SongLock __lock(player.mutex);
                for (int i = 0; i < __io.InputQueueCharacters.Size; ++i) {
                    const ImWchar __ch = __io.InputQueueCharacters[i];
                    if (__ch < 128 && __ch > ' ') typeHex_(char(__ch));
                }
            }
        }
    }; // Editor END
}; // Simple END
