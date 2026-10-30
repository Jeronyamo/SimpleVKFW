#ifndef SVKFW_SYNTH_H
#define SVKFW_SYNTH_H

#include <string>
#include <vector>
#include <mutex>

#include "sound/sampler.h"


// SVKFW Synth constants

#define SVKFW_SYNTH_MAX_OSCILLATORS 4u
#define SVKFW_SYNTH_MAX_TABLE      64u  // Max wavetable length (GB: 32 x 4 bit, PC Engine: 32 x 5 bit)
#define SVKFW_SYNTH_MIN_LEVEL    1e-4f  // Envelope level treated as silence (-80 dB)


namespace Simple {
    namespace Audio {

//  == === ==== >    Synth: instruments made of oscillators and envelopes

        namespace Synth {
    //  == === ====     Instrument description     ==== === ==  \\

            enum Waveform : uint8_t {
                WAVE_SINE,
                WAVE_PULSE,          // Square with variable duty cycle (NES/GB/DS PSG)
                WAVE_TRIANGLE,
                WAVE_SAW,
                WAVE_NOISE,          // White noise, clocked at 32x note frequency
                WAVE_NOISE_PERIODIC, // NES "short mode" LFSR: metallic, pitched noise
                WAVE_TABLE,          // Wavetable, no interpolation between points
                WAVE_COUNT
            };

            const char* waveformName(Waveform _wave);
            bool waveformFromName(const std::string &_name, Waveform &_wave);

            enum EnvelopeCurve : uint8_t {
                CURVE_LINEAR,
                CURVE_EXP, // Attack stays linear (as on SNES), decay and release fall exponentially
                CURVE_COUNT
            };

            const char* curveName(EnvelopeCurve _curve);
            bool curveFromName(const std::string &_name, EnvelopeCurve &_curve);

            struct Oscillator {
                Waveform wave = WAVE_PULSE;
                uint8_t bits = 0u;           // Output quantization in bits, 0 - off (e.g. 4 for NES triangle)
                uint8_t table_bits = 4u;     // Wavetable values are integers in [0, 2^table_bits - 1]
                uint8_t table_len = 0u;      // Number of used values in 'table'
                bool antialias = true;       // PolyBLEP for pulse and saw
                float volume = 1.f;
                float duty   = 0.5f;         // Pulse duty cycle in (0, 1)
                float detune = 0.f;          // Semitones (fractional for cents)
                float phase  = 0.f;          // Start phase in [0, 1)
                float pwm_depth = 0.f;       // Duty modulation amplitude
                float pwm_rate  = 0.f;       // Duty modulation frequency, Hz
                float table[SVKFW_SYNTH_MAX_TABLE]{}; // Normalized to [-1, 1]

                void setTable(const std::vector<uint32_t> &_values, uint8_t _table_bits);
                std::vector<uint32_t> getTable() const;
            }; // Oscillator END

            struct EnvelopeADSR {
                float attack  = 0.005f; // Seconds
                float decay   = 0.1f;   // Seconds
                float sustain = 0.7f;   // Level in [0, 1]
                float release = 0.1f;   // Seconds
                EnvelopeCurve curve = CURVE_EXP;
            }; // EnvelopeADSR END

            struct Vibrato {
                float depth = 0.f; // Semitones
                float rate  = 5.f; // Hz
                float delay = 0.f; // Seconds after note on
            }; // Vibrato END

            // Pitch starts 'semitones' away from the note and slides to it in 'time' seconds (drums, "pew" sounds)
            struct PitchEnvelope {
                float semitones = 0.f;
                float time      = 0.f;
            }; // PitchEnvelope END

            // Note: plain data without heap members, so voices copy it on note on and edits never race with playback
            struct InstrumentParams {
                float volume = 0.8f;
                float pan    = 0.f;  // [-1, 1]
                int32_t octave = 0;  // Added to every played note
                uint32_t num_oscillators = 1u;
                Oscillator oscillators[SVKFW_SYNTH_MAX_OSCILLATORS];
                EnvelopeADSR  amp_env;
                Vibrato       vibrato;
                PitchEnvelope pitch_env;
            }; // InstrumentParams END

            struct Instrument {
                std::string name;
                InstrumentParams params;
            }; // Instrument END


    //  == === ====     Playback state     ==== === ==  \\

            struct EnvelopeState {
                enum Stage : uint8_t { STAGE_IDLE, STAGE_ATTACK, STAGE_DECAY, STAGE_SUSTAIN, STAGE_RELEASE };

                Stage stage = STAGE_IDLE;
                float level = 0.f;

                // Note: attack starts from the current level, so retriggering doesn't click
                void noteOn();
                void noteOff();
                void reset();
                bool isActive() const;
                float next(const EnvelopeADSR &_env, float _dt);
            }; // EnvelopeState END


            // One playing note of an instrument
            struct Voice {
                InstrumentParams params;
                EnvelopeState amp;
                double   osc_phase[SVKFW_SYNTH_MAX_OSCILLATORS]{};
                float    osc_ratio[SVKFW_SYNTH_MAX_OSCILLATORS]{}; // 2^(detune/12)
                float    noise_value[SVKFW_SYNTH_MAX_OSCILLATORS]{};
                uint32_t noise_lfsr[SVKFW_SYNTH_MAX_OSCILLATORS]{};
                float base_freq = 440.f;   // Hz, the note itself
                float pitch_offset = 0.f;  // Semitones, set by the sequencer (slides, vibrato, arpeggio)
                float velocity = 1.f;
                float time = 0.f;          // Seconds since note on
                float inv_sr = 1.f / 48000.f;

                void setSampleRate(uint32_t _sample_rate);

                void noteOn(const InstrumentParams &_params, float _freq, float _velocity = 1.f);
                void noteOff();
                void cut();
                bool isActive() const;

                // Mono sample in about [-1, 1], instrument pan is applied by the caller
                float render();
            }; // Voice END

            // Constant-power pan law, _pan in [-1, 1]
            vec2f panGains(float _pan);


            // Plays instruments from the UI (keyboard preview). Sampled from the audio thread through 'AudioSampler'.
            struct NotePreview : Sample::SmpItf {
                static const uint32_t Max_Voices = 8u;

                struct PreviewVoice {
                    Voice voice;
                    int32_t key = -1;    // Note id that started the voice, -1 when free
                    uint32_t age = 0u;
                } voices[Max_Voices];

                Music::NoteHandler note_handler;
                uint32_t sample_rate = 48000u, age_counter = 0u;
                float master_volume = 0.5f;
                std::mutex voices_mutex;

                NotePreview();
                virtual ~NotePreview() override;

                void setSampleRate(uint32_t _sample_rate);

                // '_note' is a full note id: octave * 12 + note
                void noteOn (const InstrumentParams &_params, Music::NoteFullId _note, float _velocity = 1.f);
                void noteOff(Music::NoteFullId _note);
                void allNotesOff();
                bool isNotePlaying(Music::NoteFullId _note);

                vec2f renderFrame();

                float   sample (float  _t) override;
                vec2i16 sample2(double _t) override;
            }; // NotePreview END


    //  == === ====     Utilities     ==== === ==  \\

            // Renders a single note: '_hold' seconds pressed, then until the release ends (or '_max_len' seconds)
            std::vector<float> renderNote(const InstrumentParams &_params, float _freq, float _hold, uint32_t _sample_rate,
                                          float _max_len = 10.f);
        }; // Synth END
    }; // Audio END
}; // Simple END

#endif
