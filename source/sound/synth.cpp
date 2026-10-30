#include "sound/synth.h"

#include <fstream>


namespace Simple {
    namespace Audio {
        namespace Synth {
            // Exponential segments reach -60 dB in their nominal time
            static const float Exp_Rate = 6.9077553f; // ln(1000)
            static const float Two_Pi   = 6.2831853f;

            static const char* const Waveform_Names[WAVE_COUNT] = { "sine", "pulse", "triangle", "saw", "noise", "noise_periodic", "table" };
            static const char* const Curve_Names[CURVE_COUNT]   = { "linear", "exp" };

            const char* waveformName(Waveform _wave) { return _wave < WAVE_COUNT ? Waveform_Names[_wave] : "unknown"; }

            bool waveformFromName(const std::string &_name, Waveform &_wave) {
                for (uint32_t i = 0u; i < WAVE_COUNT; ++i)
                    if (_name == Waveform_Names[i]) {
                        _wave = Waveform(i);
                        return true;
                    }
                return false;
            }

            const char* curveName(EnvelopeCurve _curve) { return _curve < CURVE_COUNT ? Curve_Names[_curve] : "unknown"; }

            bool curveFromName(const std::string &_name, EnvelopeCurve &_curve) {
                for (uint32_t i = 0u; i < CURVE_COUNT; ++i)
                    if (_name == Curve_Names[i]) {
                        _curve = EnvelopeCurve(i);
                        return true;
                    }
                return false;
            }


            void Oscillator::setTable(const std::vector<uint32_t> &_values, uint8_t _table_bits) {
                table_bits = std::min(std::max(_table_bits, uint8_t(1u)), uint8_t(8u));
                table_len  = uint8_t(std::min(_values.size(), size_t(SVKFW_SYNTH_MAX_TABLE)));
                const float __max_val = float((1u << table_bits) - 1u);
                for (uint32_t i = 0u; i < table_len; ++i)
                    table[i] = 2.f * std::min(float(_values[i]), __max_val) / __max_val - 1.f;
            }

            std::vector<uint32_t> Oscillator::getTable() const {
                const float __max_val = float((1u << table_bits) - 1u);
                std::vector<uint32_t> __res(table_len);
                for (uint32_t i = 0u; i < table_len; ++i)
                    __res[i] = uint32_t(std::max(std::floor((table[i] + 1.f) * 0.5f * __max_val + 0.5f), 0.f));
                return __res;
            }


            void EnvelopeState::noteOn() { stage = STAGE_ATTACK; }

            void EnvelopeState::noteOff() { if (stage != STAGE_IDLE) stage = STAGE_RELEASE; }

            void EnvelopeState::reset() {
                stage = STAGE_IDLE;
                level = 0.f;
            }

            bool EnvelopeState::isActive() const { return stage != STAGE_IDLE; }

            float EnvelopeState::next(const EnvelopeADSR &_env, float _dt) {
                const float __sustain = std::min(std::max(_env.sustain, 0.f), 1.f);
                switch (stage) {
                    case STAGE_IDLE: return 0.f;

                    case STAGE_ATTACK:
                        level = _env.attack <= _dt ? 1.f : level + _dt / _env.attack;
                        if (level >= 1.f) {
                            level = 1.f;
                            stage = STAGE_DECAY;
                        }
                        break;

                    case STAGE_DECAY:
                        if (_env.decay <= _dt)
                            level = __sustain;
                        else if (_env.curve == CURVE_LINEAR)
                            level -= (1.f - __sustain) * _dt / _env.decay;
                        else
                            level = __sustain + (level - __sustain) * std::exp(-Exp_Rate * _dt / _env.decay);
                        if (level - __sustain <= SVKFW_SYNTH_MIN_LEVEL) {
                            level = __sustain;
                            stage = STAGE_SUSTAIN;
                        }
                        break;

                    case STAGE_SUSTAIN:
                        level = __sustain;
                        break;

                    case STAGE_RELEASE:
                        // Note: linear release takes 'release' seconds from full level, so quieter notes end sooner
                        if (_env.release <= _dt)
                            level = 0.f;
                        else if (_env.curve == CURVE_LINEAR)
                            level -= _dt / _env.release;
                        else
                            level *= std::exp(-Exp_Rate * _dt / _env.release);
                        break;
                }
                // Silent sustain ends the note, so one-shot sounds (drums) free their voice without a note off
                if (level <= SVKFW_SYNTH_MIN_LEVEL && (stage == STAGE_RELEASE || stage == STAGE_SUSTAIN))
                    reset();
                return level;
            }


            // PolyBLEP residual for a unit step at phase 0, '_dt' is the phase increment per sample
            static inline float polyBlep(double _t, double _dt) {
                if (_t < _dt) {
                    _t /= _dt;
                    return float(_t + _t - _t * _t - 1.);
                }
                if (_t > 1. - _dt) {
                    _t = (_t - 1.) / _dt;
                    return float(_t * _t + _t + _t + 1.);
                }
                return 0.f;
            }

            void Voice::setSampleRate(uint32_t _sample_rate) { inv_sr = 1.f / float(std::max(_sample_rate, 1u)); }

            void Voice::noteOn(const InstrumentParams &_params, float _freq, float _velocity) {
                params = _params;
                params.num_oscillators = std::min(params.num_oscillators, SVKFW_SYNTH_MAX_OSCILLATORS);
                base_freq = _freq;
                velocity = _velocity;
                pitch_offset = 0.f;
                time = 0.f;
                for (uint32_t i = 0u; i < params.num_oscillators; ++i) {
                    osc_phase[i]   = params.oscillators[i].phase - std::floor(params.oscillators[i].phase);
                    osc_ratio[i]   = std::exp2(params.oscillators[i].detune / 12.f);
                    noise_lfsr[i]  = 1u;
                    noise_value[i] = 1.f;
                }
                amp.noteOn();
            }

            void Voice::noteOff() { amp.noteOff(); }

            void Voice::cut() { amp.reset(); }

            bool Voice::isActive() const { return amp.isActive(); }

            float Voice::render() {
                if (!amp.isActive()) return 0.f;
                const float __env = amp.next(params.amp_env, inv_sr);

                float __semitones = float(params.octave * 12) + pitch_offset;
                const PitchEnvelope &__pitch_env = params.pitch_env;
                if (__pitch_env.time > 0.f && time < __pitch_env.time)
                    __semitones += __pitch_env.semitones * (1.f - time / __pitch_env.time);
                const Vibrato &__vibrato = params.vibrato;
                if (__vibrato.depth != 0.f && time >= __vibrato.delay)
                    __semitones += __vibrato.depth * std::sin(Two_Pi * __vibrato.rate * (time - __vibrato.delay));
                const float __freq = __semitones != 0.f ? base_freq * std::exp2(__semitones / 12.f) : base_freq;

                float __res = 0.f;
                for (uint32_t i = 0u; i < params.num_oscillators; ++i) {
                    const Oscillator &__osc = params.oscillators[i];
                    // Note: increments of 0.5 and more alias anyway and break PolyBLEP, so they are clamped
                    const double __dt = std::min(double(__freq * osc_ratio[i] * inv_sr), 0.49);
                    double &__phase = osc_phase[i];
                    float __y = 0.f;

                    switch (__osc.wave) {
                        case WAVE_SINE:
                            __y = std::sin(Two_Pi * float(__phase));
                            break;

                        case WAVE_PULSE: {
                            float __duty = __osc.duty;
                            if (__osc.pwm_depth != 0.f)
                                __duty += __osc.pwm_depth * std::sin(Two_Pi * __osc.pwm_rate * time);
                            __duty = std::min(std::max(__duty, 0.01f), 0.99f);
                            __y = __phase < __duty ? 1.f : -1.f;
                            if (__osc.antialias) {
                                double __fall = __phase - __duty;
                                if (__fall < 0.) __fall += 1.;
                                __y += polyBlep(__phase, __dt) - polyBlep(__fall, __dt);
                            }
                            break;
                        }

                        case WAVE_TRIANGLE:
                            __y = 4.f * std::fabs(float(__phase) - 0.5f) - 1.f;
                            break;

                        case WAVE_SAW:
                            __y = 2.f * float(__phase) - 1.f;
                            if (__osc.antialias)
                                __y -= polyBlep(__phase, __dt);
                            break;

                        case WAVE_NOISE:
                        case WAVE_NOISE_PERIODIC: {
                            // Note: noise has its own clock (32x note frequency, at most once per sample), so higher notes sound brighter
                            __phase += std::min(__dt * 32., 1.);
                            if (__phase >= 1.) {
                                __phase -= 1.;
                                const uint32_t __tap = __osc.wave == WAVE_NOISE ? 1u : 6u;
                                const uint32_t __fb = (noise_lfsr[i] ^ (noise_lfsr[i] >> __tap)) & 1u;
                                noise_lfsr[i] = (noise_lfsr[i] >> 1) | (__fb << 14);
                                noise_value[i] = (noise_lfsr[i] & 1u) ? 1.f : -1.f;
                            }
                            __y = noise_value[i];
                            break;
                        }

                        case WAVE_TABLE:
                            if (__osc.table_len > 0u)
                                __y = __osc.table[std::min(uint32_t(__phase * __osc.table_len), uint32_t(__osc.table_len - 1u))];
                            break;

                        default: break;
                    }

                    if (__osc.bits > 0u) {
                        const float __steps = float(1u << (std::min(__osc.bits, uint8_t(16u)) - 1u));
                        __y = std::floor(std::min(std::max(__y, -1.f), 1.f) * __steps + 0.5f) / __steps;
                    }
                    __res += __osc.volume * __y;

                    if (__osc.wave != WAVE_NOISE && __osc.wave != WAVE_NOISE_PERIODIC) {
                        __phase += __dt;
                        if (__phase >= 1.) __phase -= std::floor(__phase);
                    }
                }
                time += inv_sr;
                return __res * __env * params.volume * velocity;
            }

            vec2f panGains(float _pan) {
                const float __angle = (std::min(std::max(_pan, -1.f), 1.f) + 1.f) * 0.25f * float(M_PI);
                return { std::cos(__angle), std::sin(__angle) };
            }


            const uint32_t NotePreview::Max_Voices;

            NotePreview::NotePreview() {}

            NotePreview::~NotePreview() {}

            void NotePreview::setSampleRate(uint32_t _sample_rate) {
                std::lock_guard<std::mutex> __lock(voices_mutex);
                sample_rate = std::max(_sample_rate, 1u);
                for (uint32_t i = 0u; i < Max_Voices; ++i)
                    voices[i].voice.setSampleRate(sample_rate);
            }

            void NotePreview::noteOn(const InstrumentParams &_params, Music::NoteFullId _note, float _velocity) {
                std::lock_guard<std::mutex> __lock(voices_mutex);
                // Same key retriggers its voice, else a free voice, else the oldest one is stolen
                uint32_t __id = Max_Voices;
                for (uint32_t i = 0u; i < Max_Voices && __id == Max_Voices; ++i)
                    if (voices[i].key == _note) __id = i;
                for (uint32_t i = 0u; i < Max_Voices && __id == Max_Voices; ++i)
                    if (!voices[i].voice.isActive()) __id = i;
                if (__id == Max_Voices) {
                    __id = 0u;
                    for (uint32_t i = 1u; i < Max_Voices; ++i)
                        if (voices[i].age < voices[__id].age) __id = i;
                }
                PreviewVoice &__pv = voices[__id];
                __pv.key = _note;
                __pv.age = ++age_counter;
                __pv.voice.setSampleRate(sample_rate);
                __pv.voice.noteOn(_params, note_handler.getNoteFreq(_note), _velocity);
            }

            void NotePreview::noteOff(Music::NoteFullId _note) {
                std::lock_guard<std::mutex> __lock(voices_mutex);
                for (uint32_t i = 0u; i < Max_Voices; ++i)
                    if (voices[i].key == _note) {
                        voices[i].voice.noteOff();
                        voices[i].key = -1;
                    }
            }

            void NotePreview::allNotesOff() {
                std::lock_guard<std::mutex> __lock(voices_mutex);
                for (uint32_t i = 0u; i < Max_Voices; ++i) {
                    voices[i].voice.noteOff();
                    voices[i].key = -1;
                }
            }

            bool NotePreview::isNotePlaying(Music::NoteFullId _note) {
                std::lock_guard<std::mutex> __lock(voices_mutex);
                for (uint32_t i = 0u; i < Max_Voices; ++i)
                    if (voices[i].key == _note) return true;
                return false;
            }

            vec2f NotePreview::renderFrame() {
                std::lock_guard<std::mutex> __lock(voices_mutex);
                vec2f __res{};
                for (uint32_t i = 0u; i < Max_Voices; ++i) {
                    Voice &__voice = voices[i].voice;
                    if (!__voice.isActive()) continue;
                    __res += __voice.render() * panGains(__voice.params.pan);
                }
                return __res * master_volume;
            }

            float NotePreview::sample(float _t) {
                const vec2f __frame = renderFrame();
                return 0.5f * (__frame.x + __frame.y);
            }

            vec2i16 NotePreview::sample2(double _t) {
                const vec2f __frame = Math::clampCL(renderFrame(), vec2f{-1.f}, vec2f{1.f}) * float(INT16_MAX);
                return __frame.cast<int16_t>();
            }


            std::vector<float> renderNote(const InstrumentParams &_params, float _freq, float _hold, uint32_t _sample_rate, float _max_len) {
                Voice __voice;
                __voice.setSampleRate(_sample_rate);
                __voice.noteOn(_params, _freq);

                const uint32_t __hold_frames = uint32_t(std::max(_hold, 0.f) * _sample_rate);
                const uint32_t __max_frames  = uint32_t(std::max(_max_len, 0.f) * _sample_rate);
                std::vector<float> __res;
                __res.reserve(__hold_frames);
                while (__res.size() < __max_frames && __voice.isActive()) {
                    if (__res.size() == __hold_frames) __voice.noteOff();
                    __res.push_back(__voice.render());
                }
                return __res;
            }

            static void writeLE(std::ofstream &_file, uint32_t _value, uint32_t _bytes) {
                for (uint32_t i = 0u; i < _bytes; ++i)
                    _file.put(char((_value >> (8u * i)) & 0xFFu));
            }

            bool writeWAV(const std::string &_fpath, const std::vector<vec2f> &_frames, uint32_t _sample_rate) {
                std::ofstream __file(_fpath.c_str(), std::ios::binary);
                if (!__file) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Synth :: writeWAV", "could not open '%s'\n"), _fpath.c_str());
                    return false;
                }
                const uint32_t __data_bytes = uint32_t(_frames.size()) * 4u;
                __file.write("RIFF", 4);
                writeLE(__file, 36u + __data_bytes, 4u);
                __file.write("WAVEfmt ", 8);
                writeLE(__file, 16u, 4u);                // fmt chunk size
                writeLE(__file, 1u, 2u);                 // PCM
                writeLE(__file, 2u, 2u);                 // Channels
                writeLE(__file, _sample_rate, 4u);
                writeLE(__file, _sample_rate * 4u, 4u);  // Bytes per second
                writeLE(__file, 4u, 2u);                 // Block align
                writeLE(__file, 16u, 2u);                // Bits per sample
                __file.write("data", 4);
                writeLE(__file, __data_bytes, 4u);
                for (const vec2f &__frame : _frames) {
                    const vec2f __clamped = Math::clampCL(__frame, vec2f{-1.f}, vec2f{1.f}) * float(INT16_MAX);
                    writeLE(__file, uint32_t(int32_t(__clamped.x)) & 0xFFFFu, 2u);
                    writeLE(__file, uint32_t(int32_t(__clamped.y)) & 0xFFFFu, 2u);
                }
                return bool(__file);
            }
        }; // Synth END
    }; // Audio END
}; // Simple END
