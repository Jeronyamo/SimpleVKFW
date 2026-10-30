#ifndef SVKFW_MUSIC_EDITOR_H
#define SVKFW_MUSIC_EDITOR_H

#include "interface/imgui_wrap.h"
#include "interface/rtaudio_wrap.h"
#include "main/input_handler.h"
#include "sound/tracker.h"


namespace Simple {
    namespace Editor {
        // Actions of the tracker keyboard layer (resources/input_maps/tracker.json)
        namespace TrackerAction {
            enum : WindowInput::ActionId {
                NOTE_FIRST = 0u,                 // "note_0" .. "note_28": semitones above C of the edit octave
                NOTE_LAST  = NOTE_FIRST + 28u,
                NOTE_OFF,
                NOTE_CUT,
                OCTAVE_UP,
                OCTAVE_DOWN,
                CURSOR_UP,
                CURSOR_DOWN,
                CURSOR_LEFT,
                CURSOR_RIGHT,
                PAGE_UP,
                PAGE_DOWN,
                ROW_FIRST,
                ROW_LAST,
                CHANNEL_NEXT,
                CHANNEL_PREV,
                CLEAR,
                INSERT_ROW,
                DELETE_ROW,
                PLAY_PATTERN, // Plays the pattern or stops
                PLAY_SONG,    // Plays the song from the current order or stops
                TYPE,         // Keys typed into the hex columns, consumed so the camera doesn't move
                COUNT
            };
        }; // TrackerAction END

        // Keyboard layer of the music editor: enabled while the pattern or instrument window has the focus,
        // queues bound key presses/repeats/releases for the editor
        struct TrackerKeys : WindowInput::InputHandlerItf {
            struct KeyEvent {
                WindowInput::ActionId action;
                int glfw_action; // GLFW_PRESS, GLFW_REPEAT, GLFW_RELEASE
            };
            std::vector<KeyEvent> events;

            TrackerKeys();
            void onAction(WindowInput::ActionId _action, int _glfw_action) override;
        }; // TrackerKeys END


        // Instrument library, song editor and tracker in ImGui windows, with its own audio output stream.
        // Note: add 'keys' as an input layer right after ImGui: 'insertLayer(0, &studio.keys)'
        struct MusicStudio : ImGUI::WidgetItf {
            enum EditTarget { EDIT_LIBRARY, EDIT_SONG };

        // Data
            Audio::Synth::InstrumentLibrary library;
            Audio::Tracker::Song song;
            Audio::Tracker::SongPlayer player;
            Audio::Synth::NotePreview preview;
            RTA::AudioHandler audio;
            TrackerKeys keys;
            bool audio_running = false;
            std::string audio_error;

        // UI state
            char library_path[256]{}, song_path[256]{}, name_buf[64]{};
            EditTarget edit_target = EDIT_SONG;
            int32_t lib_selected = 0, song_instrument = 0;
            uint32_t current_order = 0u, current_pattern = 0u;
            ImGuiCustom::TextGridCursor cursor;
            int32_t octave = 4, edit_step = 1;
            uint32_t typed_digits = 0u;      // Digits typed into the current hex field
            int32_t scroll_to = -1;
            bool follow = true, pattern_focused = false, instrument_focused = false;
            bool show_library = true, show_instrument = true, show_song = true, show_pattern = true;
            int32_t key_notes[TrackerAction::NOTE_LAST + 1u]; // Note started by each note key, -1 if none
            uint32_t key_channels[TrackerAction::NOTE_LAST + 1u];
            std::vector<float> wave_preview;
            Audio::Synth::InstrumentParams wave_preview_params;
            bool wave_preview_valid = false;
            int32_t export_opus_kbps = 160;
            std::string status;

            MusicStudio(const std::string &_library_path = "resources/music/instruments.json",
                        const std::string &_song_path    = "resources/music/songs/demo_chip.json");
            virtual ~MusicStudio() override;

            // Opens the default output device at 48 kHz, returns false (and sets 'audio_error') on failure
            bool startAudio();
            void stopAudio();

            virtual bool      begin() override;
            virtual void        end() override;
            virtual void     render() override;
            virtual void renderFull() override;

        // Windows
            void windowLibrary();
            void windowInstrument();
            void windowSong();
            void windowPattern();

        // Helpers
            Audio::Synth::Instrument* editedInstrument_();
            void commitInstrument_(const Audio::Synth::InstrumentParams &_params);
            void loadSong_(const std::string &_fpath);
            // Renders the song at 48 kHz next to 'song_path' as .wav (16 bit PCM) or .ogg (Opus)
            bool exportSong_(bool _opus);
            void handleKeys_();
            void handlePatternKey_(WindowInput::ActionId _action, int _glfw_action);
            void typeHex_(char _ch);
            void moveCursor_(int32_t _rows, int32_t _subs);
            void advance_();
            uint32_t numRows_() const;
        }; // MusicStudio END
    }; // Editor END
}; // Simple END

#endif
