#ifndef SVKFW_TRACKER_H
#define SVKFW_TRACKER_H

#include <atomic>

#include "sound/instrument_library.h"


// SVKFW Tracker constants

#define SVKFW_TRACKER_MAX_CHANNELS  32u
#define SVKFW_TRACKER_MAX_ROWS     256u
#define SVKFW_TRACKER_MAX_ECHO     0.5f // Max echo delay, seconds


namespace Simple {
    namespace Audio {

//  == === ==== >    Tracker: songs made of patterns (XM-like)

        namespace Tracker {
    //  == === ====     Song data     ==== === ==  \\

            // One channel in one row: "C-4 01 40 4A6" - note, instrument, volume, effect with parameter
            struct Cell {
                static const uint8_t Note_None  =   0u;
                static const uint8_t Note_Off   = 254u; // "===", starts the release
                static const uint8_t Note_Cut   = 253u; // "^^^", silences at once
                static const uint8_t Note_Max   = 120u; // Notes are 1 + NoteFullId: 1 = C-0, 120 = B-9
                static const uint8_t Instrument_None = 255u;
                static const uint8_t Volume_None     = 255u;
                static const uint8_t Volume_Max      =  64u;

                uint8_t note       = Note_None;
                uint8_t instrument = Instrument_None; // Index in 'Song::instruments'
                uint8_t volume     = Volume_None;     // [0, 64]
                char    effect     = 0;               // '0'-'9', 'A'-'Z', 0 - no effect
                uint8_t param      = 0u;

                bool isEmpty() const;
                bool hasNote() const; // A real note, not off/cut

                // "C-4 01 40 4A6", empty parts are dots: "... .. .. ..."
                std::string toString() const;
                static bool parse(const std::string &_str, Cell &_cell);

                static std::string noteToString(uint8_t _note);
                static bool noteFromString(const std::string &_str, uint8_t &_note);
            }; // Cell END

            struct Pattern {
                uint32_t rows = 64u;
                std::vector<Cell> cells; // rows x channels, row-major

                void resize(uint32_t _rows, uint32_t _old_channels, uint32_t _new_channels);
            }; // Pattern END

            struct ChannelSettings {
                std::string name;
                float volume = 1.f;
                float pan    = 0.f;
                bool muted = false, solo = false;
                bool echo  = false; // Sends the channel to the echo
            }; // ChannelSettings END

            // SNES-like echo: delay line with feedback and a low-pass in the loop
            struct EchoSettings {
                float delay    = 0.25f; // Seconds, up to SVKFW_TRACKER_MAX_ECHO
                float feedback = 0.35f;
                float volume   = 0.3f;
                float lowpass  = 0.4f;  // [0, 1), higher - darker repeats
            }; // EchoSettings END

            struct Song {
                static const uint32_t Format_Version = 1u;

                std::string name = "Untitled";
                Music::TuningSystemType tuning = Music::EQUAL_TEMPERAMENT_12T;
                float ref_freq = 440.f;        // Frequency of A-4
                float bpm = 125.f;
                uint32_t rows_per_beat = 4u;   // Row length = 60 / (bpm * rows_per_beat)
                uint32_t ticks_per_row = 6u;   // Effects update once per tick
                uint32_t restart_order = 0u;   // Where the song loops to
                EchoSettings echo;

                std::vector<Synth::Instrument> instruments; // Copies of library instruments (as in .xm), cells refer to them by index
                std::vector<ChannelSettings> channels;
                std::vector<uint32_t> order;          // Pattern indices
                std::vector<Pattern> patterns;

                std::string fpath;

                Song();

                uint32_t numChannels() const;
                void setNumChannels(uint32_t _num_channels);

                uint32_t addPattern(uint32_t _rows = 64u);
                void setPatternRows(uint32_t _pattern, uint32_t _rows);

                Cell& cell(uint32_t _pattern, uint32_t _row, uint32_t _channel);
                const Cell& cell(uint32_t _pattern, uint32_t _row, uint32_t _channel) const;

                int32_t findInstrument(const std::string &_name) const; // -1 if absent
                // Appends a copy with a unique name, returns its index or Cell::Instrument_None if the song is full
                uint8_t addInstrument(const Synth::Instrument &_instrument);
                // Replaces the instrument with the same name (keeps the index) or appends it
                uint8_t setInstrument(const Synth::Instrument &_instrument);

                float rowSeconds() const;

                bool load(const std::string &_fpath);
                bool save(const std::string &_fpath = "");
            }; // Song END


    //  == === ====     Playback     ==== === ==  \\

            struct ChannelState {
                Synth::Voice voice, tail;  // 'tail' keeps the release of the previous note
                int32_t instrument = -1;   // Song instrument index
                float volume = 1.f;        // From the volume column and Axy
                float pan = 0.f;           // Set by 8xx, added to the channel and instrument pan
                float base_freq = 0.f;
                float slide = 0.f;         // Semitones, 1xx/2xx/3xx, reset by a new note
                float porta_target = 0.f;  // Semitones relative to 'base_freq' for 3xx
                float vibrato_pos = 0.f;   // Cycles
                vec2f peak{};              // For VU meters, decays in the player

                Cell cell;                 // Current row cell (effect state)
                Cell delayed;              // EDx: cell waiting for its tick
                uint8_t delay_tick = 0u;
                uint8_t mem_slide_up = 0u, mem_slide_down = 0u, mem_porta = 0u, mem_vibrato = 0u, mem_volslide = 0u;
            }; // ChannelState END

            // Plays a song with its own instruments. Sampled from the audio thread through 'AudioSampler'.
            // Note: lock 'mutex' while editing the song during playback.
            struct SongPlayer : Sample::SmpItf {
                Song *song = nullptr;
                Music::NoteHandler note_handler;
                std::mutex mutex;

                ChannelState channels[SVKFW_TRACKER_MAX_CHANNELS];
                uint32_t sample_rate = 48000u;
                float master_volume = 0.5f;
                bool loop = true;          // Loop the song (or the pattern in pattern mode)
                bool pattern_mode = false; // Plays only 'pattern_mode_id'
                uint32_t pattern_mode_id = 0u;

                // Position, readable from the UI thread
                std::atomic<bool> playing;
                std::atomic<uint32_t> play_order, play_row;

                // Internal timing
                uint32_t tick = 0u, ticks_per_row = 6u;
                float bpm = 125.f;
                double samples_to_tick = 0.;
                int32_t jump_order = -1, jump_row = -1; // Bxx/Dxx, applied after the row
                int32_t queued_order = -1;              // 'jumpToOrder' at the pattern end
                std::vector<vec2f> echo_buffer;
                uint32_t echo_pos = 0u;
                vec2f echo_lp{};

                SongPlayer();
                virtual ~SongPlayer() override;

                void setSong(Song *_song);
                void refreshTuning_(); // Call with 'mutex' locked after the song tuning changes

                void setSampleRate(uint32_t _sample_rate);

                void play(uint32_t _order = 0u, uint32_t _row = 0u);
                void playPattern(uint32_t _pattern, uint32_t _row = 0u);
                void stop();      // Releases playing notes
                void stopNow();   // Silences at once
                void jumpToOrder(uint32_t _order, bool _at_pattern_end = true);
                bool isSilent();  // Not playing and all voices ended

                // Plays one cell on a channel (note entry in the editor), doesn't need 'play'
                void previewCell(uint32_t _channel, const Cell &_cell);

                vec2f renderFrame();
                void render(std::vector<vec2f> &_frames, uint32_t _num_frames);

                // Renders the whole song once (no loop) for export, '_max_seconds' limits endless Bxx loops
                static std::vector<vec2f> renderSong(Song &_song, uint32_t _sample_rate, float _max_seconds = 600.f);

                float   sample (float  _t) override;
                vec2i16 sample2(double _t) override;

            // Internals, called with 'mutex' locked

                void resetChannels_();
                void processTick_();
                void nextRow_();
                void triggerCell_(uint32_t _ch, const Cell &_cell);
                void applyEffectTick_(uint32_t _ch);
                const Synth::InstrumentParams* instrumentParams_(int32_t _song_instrument) const;
                uint32_t currentPattern_() const;
            }; // SongPlayer END
        }; // Tracker END
    }; // Audio END
}; // Simple END

#endif
