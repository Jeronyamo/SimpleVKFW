#ifndef SVKFW_SAMPLER_H
#define SVKFW_SAMPLER_H

#include <cmath>
#include <vector>
#include <functional>

#include "common/terminal.h"


namespace Simple {
    namespace Audio {

//  == === ==== >    Music: converting notes to frequencies

        namespace Music {
    //  == === ====     Types & enums     ==== === ==  \\

            // CDEFGAB, _S - sharp, _F - flat
            enum Note : uint8_t {
                C  =  0,
                CS =  1, DF =  1,
                D  =  2,
                DS =  3, EF =  3,
                E  =  4,
                ES =  5, F  =  5,
                FS =  6, GF =  6,
                G  =  7,
                GS =  8, AF =  8,
                A  =  9,
                AS = 10, BF = 10,
                B  = 11,
                BS =  0
            };

            enum IntervalCommon : int16_t {
                UNISON      =  0,
                SECOND_MIN  =  1,
                SECOND_MAJ  =  2,
                THIRD_MIN   =  3,
                THIRD_MAJ   =  4,
                FOURTH_PRF  =  5,
                TRITONE     =  6,
                FIFTH_PRF   =  7,
                SIXTH_MIN   =  8,
                SIXTH_MAJ   =  9,
                SEVENTH_MIN = 10,
                SEVENTH_MAJ = 11,
                OCTAVE      = 12
            };

            typedef   float NoteFreq;     // Stores note frequency, Hz
            typedef     int NoteFullId;   // Stores absolute ID of a note encoded as: (NoteFull::octave * TuningSystem.numNotes()) + NoteFull::note
            typedef int16_t NoteInterval; // Accepts 'IntervalCommon' values or any other int
            typedef int16_t   NoteIndex;
            typedef int16_t OctaveIndex;

            struct NoteFull {
                NoteIndex note;
                OctaveIndex octave;
            };


        // Reference note description (used in tuning)

            const struct NoteReference {
                NoteFull   ref_note{ Note::A, 4};
                NoteFreq   ref_note_freq = 440.f;
                NoteFullId ref_note_ind = INT32_MAX;

                NoteReference() {}
                NoteReference(NoteIndex _ref_note_index, OctaveIndex _ref_octave, NoteFreq _ref_note_freq)
                                : ref_note_freq{_ref_note_freq}, ref_note{_ref_note_index, _ref_octave} {}
                NoteReference(const NoteReference &_note_ref) = default;
            } Ref_Note_A440;


    //  == === ====     Tuning systems     ==== === ==  \\

            enum TuningSystemType {
                EQUAL_TEMPERAMENT_12T,
                EQUAL_TEMPERAMENT,
                JUST_INTONATION
            };

            struct TuningSystemItf {
                TuningSystemItf() {}
                virtual ~TuningSystemItf() {}

                virtual TuningSystemType getType() = 0;
                virtual uint32_t numNotes() = 0;
                virtual double octaveCoef() = 0;
                virtual double subOctaveCoef(NoteInterval _interval) = 0;
                virtual double  intervalCoef(NoteInterval _interval) {
                    bool __going_down = _interval < 0;
                    if (__going_down) _interval = -_interval;

                    NoteFull __interval_info = getNoteFull(_interval);
                    double __res = std::pow(octaveCoef(), __interval_info.octave) * subOctaveCoef(__interval_info.note);
                    return __going_down ? 1/__res : __res;
                }
                NoteFull getNoteFull(NoteFullId _note_ind) {
                    NoteFull __res;
                    __res.note   = _note_ind % numNotes();
                    __res.octave = _note_ind / numNotes();

                    if (_note_ind < 0 && __res.note) {
                        __res.note = numNotes() - __res.note;
                        __res.octave -= 1;
                    }

                    return __res;
                }
                NoteFullId getNoteFullId(NoteFull _full_note) {
                    return _full_note.octave * numNotes() + _full_note.note;
                }
            }; // TuningSystemItf END


            struct TSJustIntonation : TuningSystemItf {
                static constexpr double       Unison =  1.0;
                static constexpr double   Second_min = 16.0/15;
                static constexpr double   Second_maj =  9.0/ 8;
                static constexpr double    Third_min =  6.0/ 5;
                static constexpr double    Third_maj =  5.0/ 4;
                static constexpr double   Fourth_prf =  4.0/ 3;
                static constexpr double      Tritone = 45.0/32;
                static constexpr double    Fifth_prf =  3.0/ 2;
                static constexpr double    Sixth_min =  8.0/ 5;
                static constexpr double    Sixth_maj =  5.0/ 3;
                static constexpr double  Seventh_min = 16.0/ 9;
                static constexpr double  Seventh_clr =  7.0/ 4;
                static constexpr double  Seventh_maj = 15.0/ 8;
                static constexpr double       Octave =  2.0;

                TSJustIntonation() {}
               ~TSJustIntonation() {}

                virtual TuningSystemType getType() override { return JUST_INTONATION; }
                virtual uint32_t numNotes() override { return 12u; }
                virtual double octaveCoef() override { return Octave; }
                virtual double subOctaveCoef(NoteInterval _interval) override {
                    switch (_interval) {
                        case IntervalCommon::UNISON      : return      Unison;
                        case IntervalCommon::SECOND_MIN  : return  Second_min;
                        case IntervalCommon::SECOND_MAJ  : return  Second_maj;
                        case IntervalCommon::THIRD_MIN   : return   Third_min;
                        case IntervalCommon::THIRD_MAJ   : return   Third_maj;
                        case IntervalCommon::FOURTH_PRF  : return  Fourth_prf;
                        case IntervalCommon::TRITONE     : return     Tritone;
                        case IntervalCommon::FIFTH_PRF   : return   Fifth_prf;
                        case IntervalCommon::SIXTH_MIN   : return   Sixth_min;
                        case IntervalCommon::SIXTH_MAJ   : return   Sixth_maj;
                        case IntervalCommon::SEVENTH_MIN : return Seventh_min;
                        case IntervalCommon::SEVENTH_MAJ : return Seventh_maj;
                        case IntervalCommon::OCTAVE      : return      Octave;
                    }
                    return 1.0;
                }
            }; // TSJustIntonation END

            struct TSEqualTemperament12 : TuningSystemItf {
                static const double       Unison;
                static const double   Second_min;
                static const double   Second_maj;
                static const double    Third_min;
                static const double    Third_maj;
                static const double   Fourth_prf;
                static const double      Tritone;
                static const double    Fifth_prf;
                static const double    Sixth_min;
                static const double    Sixth_maj;
                static const double  Seventh_min;
                static const double  Seventh_maj;
                static const double       Octave;

                TSEqualTemperament12() {}
               ~TSEqualTemperament12() {}

                virtual TuningSystemType getType() override { return EQUAL_TEMPERAMENT_12T; }
                virtual uint32_t numNotes() override { return 12u; }
                virtual double octaveCoef() override { return Octave; }
                virtual double subOctaveCoef(NoteInterval _interval) override {
                    switch (_interval) {
                        case IntervalCommon::UNISON      : return      Unison;
                        case IntervalCommon::SECOND_MIN  : return  Second_min;
                        case IntervalCommon::SECOND_MAJ  : return  Second_maj;
                        case IntervalCommon::THIRD_MIN   : return   Third_min;
                        case IntervalCommon::THIRD_MAJ   : return   Third_maj;
                        case IntervalCommon::FOURTH_PRF  : return  Fourth_prf;
                        case IntervalCommon::TRITONE     : return     Tritone;
                        case IntervalCommon::FIFTH_PRF   : return   Fifth_prf;
                        case IntervalCommon::SIXTH_MIN   : return   Sixth_min;
                        case IntervalCommon::SIXTH_MAJ   : return   Sixth_maj;
                        case IntervalCommon::SEVENTH_MIN : return Seventh_min;
                        case IntervalCommon::SEVENTH_MAJ : return Seventh_maj;
                        case IntervalCommon::OCTAVE      : return      Octave;
                    }
                    return 1.0;
                }
            }; // TSEqualTemperament12 END
            const double  TSEqualTemperament12::     Unison = 1.0;
            const double  TSEqualTemperament12:: Second_min = std::pow(   2, 1./12);
            const double  TSEqualTemperament12:: Second_maj = std::pow(   2, 1./ 6);
            const double  TSEqualTemperament12::  Third_min = std::pow(   2, 1./ 4);
            const double  TSEqualTemperament12::  Third_maj = std::pow(   2, 1./ 3);
            const double  TSEqualTemperament12:: Fourth_prf = std::pow(  32, 1./12);
            const double  TSEqualTemperament12::    Tritone = std::sqrt(2);
            const double  TSEqualTemperament12::  Fifth_prf = std::pow( 128, 1./12);
            const double  TSEqualTemperament12::  Sixth_min = std::pow(   4, 1./ 3);
            const double  TSEqualTemperament12::  Sixth_maj = std::pow(   8, 1./ 4);
            const double  TSEqualTemperament12::Seventh_min = std::pow(  32, 1./ 6);
            const double  TSEqualTemperament12::Seventh_maj = std::pow(2048, 1./12);
            const double  TSEqualTemperament12::     Octave = 2.0;

            struct TSEqualTemperament : TuningSystemItf {
                uint32_t num_notes;
                TSEqualTemperament(uint32_t _num_notes = 12) : num_notes{_num_notes} {}

                virtual uint32_t numNotes() override { return num_notes; }
                virtual double octaveCoef() override { return       2.0; }
                virtual TuningSystemType getType() override { return EQUAL_TEMPERAMENT; }
                virtual double subOctaveCoef(NoteInterval _interval) override { return std::pow(std::pow(2, _interval), 1./num_notes); }
                virtual double  intervalCoef(NoteInterval _interval) override { return std::pow(std::pow(2, _interval), 1./num_notes); }
            }; // TSEqualTemperament END

            TuningSystemItf* getTuningSystemHandler(TuningSystemType _ts_type) {
                switch (_ts_type) {
                    case EQUAL_TEMPERAMENT_12T: return new TSEqualTemperament12;
                    case EQUAL_TEMPERAMENT: return new TSEqualTemperament;
                    case JUST_INTONATION: return new TSJustIntonation;
                }
                return nullptr;
            }


    //  == === ====     Scales     ==== === ==  \\

            enum ScaleType {
                UNDEFINED_SCALE,
                HEPTATONIC_MAJOR,
                HEPTATONIC_MAJOR_HARMONIC,
                HEPTATONIC_MINOR, // Natural minor scale
                HEPTATONIC_MINOR_MELODIC,
                HEPTATONIC_MINOR_HARMONIC,
                HEPTATONIC_MODE_IONIAN,
                HEPTATONIC_MODE_DORIAN,
                HEPTATONIC_MODE_PHRYGIAN,
                HEPTATONIC_MODE_LYDIAN,
                HEPTATONIC_MODE_MIXOLYDIAN,
                HEPTATONIC_MODE_AEOLIAN = HEPTATONIC_MINOR,
                HEPTATONIC_MODE_LOCRIAN = HEPTATONIC_MODE_MIXOLYDIAN + 1,
                CHROMATIC,
            };


            struct Scale {
                NoteFullId tonic;
                ScaleType scale_type;
                std::vector<uint8_t> steps;

                Scale(NoteFullId _tonic, ScaleType _scale_type)
                                : tonic{_tonic}, scale_type{_scale_type}, steps{GetSteps(_tonic, _scale_type)} {}

                static std::vector<uint8_t> GetSteps(NoteFullId _tonic, ScaleType _scale_type) {
                    // TODO: function 'getNumSteps(ScaleType)' to handle any 'tonic' value
                    SVKFW_ASSERT(_tonic >= 0 && _tonic < 81, std::invalid_argument, "Audio :: Music :: Scale :: GetSteps",
                                                                            "'tonic' must be between 0 and 'numNotes()'");

                    std::vector<uint8_t> __res_steps;
                    switch (_scale_type) {
                        case UNDEFINED_SCALE:            __res_steps = {}; break;
                        case HEPTATONIC_MAJOR:           __res_steps = {0,2,4,5,7,9,11}; break;
                        case HEPTATONIC_MAJOR_HARMONIC:  __res_steps = {0,2,4,5,7,8,11}; break;
                        // case HEPTATONIC_MODE_AEOLIAN:
                        case HEPTATONIC_MINOR:           __res_steps = {0,2,3,5,7,8,10}; break;
                        case HEPTATONIC_MINOR_MELODIC:   __res_steps = {0,2,3,5,7,8,11}; break;
                        case HEPTATONIC_MINOR_HARMONIC:  __res_steps = {0,2,3,5,7,9,11}; break;
                        case HEPTATONIC_MODE_IONIAN:     __res_steps = {0,2,4,5,7,9,11}; break;
                        case HEPTATONIC_MODE_DORIAN:     __res_steps = {0,2,3,5,7,9,10}; break;
                        case HEPTATONIC_MODE_PHRYGIAN:   __res_steps = {0,1,3,5,7,8,10}; break;
                        case HEPTATONIC_MODE_LYDIAN:     __res_steps = {0,2,4,6,7,9,11}; break;
                        case HEPTATONIC_MODE_MIXOLYDIAN: __res_steps = {0,2,4,5,7,9,10}; break;
                        case HEPTATONIC_MODE_LOCRIAN:    __res_steps = {0,1,3,5,6,8,10}; break;
                        case CHROMATIC:       __res_steps = {0,1,2,3,4,5,6,7,8,9,10,11}; break;
                    }
                    for (auto& step : __res_steps)
                        step = (step + _tonic) % __res_steps.size();
                    return __res_steps;
                }
            }; // Scale END


            struct NoteHandler {
                NoteReference   reference_note;
                TuningSystemItf *tuning_system;

                NoteHandler(TuningSystemType _ts_type = EQUAL_TEMPERAMENT_12T, NoteReference _ref_note = Ref_Note_A440)
                                : reference_note{_ref_note}, tuning_system{getTuningSystemHandler(_ts_type)} {
                    reference_note.ref_note_ind = tuning_system->getNoteFullId(reference_note.ref_note);
                }

                NoteFreq getNoteFreq(NoteFullId _note_ind) {
                    return reference_note.ref_note_freq * tuning_system->intervalCoef(_note_ind - reference_note.ref_note_ind);
                }
                NoteFreq getNoteFreq(NoteFull _full_note) {
                    return getNoteFreq(tuning_system->getNoteFullId(_full_note));
                }
                NoteFullId getNoteFullId(NoteFull _full_note) {
                    return tuning_system->getNoteFullId(_full_note);
                }
                NoteFullId getNoteFullId(NoteFreq _note_freq) {
                    NoteInterval __note_int = 0;

                    while (_note_freq * tuning_system->octaveCoef() <= reference_note.ref_note_freq) {
                        __note_int -= tuning_system->numNotes();
                        _note_freq *= tuning_system->octaveCoef();
                    }
                    while (_note_freq / tuning_system->octaveCoef() >  reference_note.ref_note_freq) {
                        __note_int += tuning_system->numNotes();
                        _note_freq /= tuning_system->octaveCoef();
                    }

                    NoteFreq __min_dist = 2*tuning_system->octaveCoef() + 1;
                    NoteInterval __sub_octave_interval = tuning_system->numNotes();
                    float __sub_octave_coef = -1.f;
                    for (int i = 0; i < tuning_system->numNotes(); ++i) {
                        float    __curr_coef = tuning_system->intervalCoef(i - reference_note.ref_note.note);
                        NoteFreq __curr_freq = reference_note.ref_note_freq * __curr_coef;
                        NoteFreq __curr_dist = std::abs(__curr_freq - _note_freq);

                        if (__curr_dist < __min_dist) {
                            __min_dist = __curr_dist;
                            __sub_octave_interval = i - reference_note.ref_note.note;
                            __sub_octave_coef = __curr_coef;
                        }
                    }

                    if (__sub_octave_coef > 0.f) {
                        _note_freq /= __sub_octave_coef;
                        __note_int += __sub_octave_interval;
                        printf(SVKFW_WRAPINFO("Audio :: Music :: NoteHandler :: getNoteFullId",
                                "reference frequency: %f; got frequency %f; error %f"), reference_note.ref_note_freq, _note_freq, std::abs(_note_freq - reference_note.ref_note_freq));
                    }
                    return reference_note.ref_note_ind + __note_int;
                }
                NoteFull getNoteFull(NoteFullId _note_ind) {
                    return tuning_system->getNoteFull(_note_ind);
                }
                NoteFull getNoteFull(NoteFreq _note_freq) {
                    return tuning_system->getNoteFull(getNoteFullId(_note_freq));
                }
            }; // NoteHandler END


            struct NotePoint {
                NoteFull note;
                uint32_t t;
            };

            struct NoteSheet {
                std::vector<std::vector<NotePoint>> sample_sections;

                // TODO: 
            }; // NoteSheet END
        }; // Music END


//  == === ==== >    Sample: "instruments" created with "math"

        namespace Sample {
            struct SmpItf {
                float duration = 0.f; // Sample duration in seconds; Default: 0; 0 means infinite sample; no need to use it in 'sample()' method, it's for 'Audio::AudioSampler'.

                virtual ~SmpItf() {}

                // t = (t_step / sample_rate) * frequency
                virtual float sample(float _t) = 0;
                void setDuration(float _dur) { duration = _dur; }
            }; // SmpItf END

            struct SmpPlaceholder : SmpItf {
                SmpPlaceholder() {}
               ~SmpPlaceholder() {}
                float sample(float _t) override { return 0.f; }
            } audio_placeholder;

            struct SmpSineWave : SmpItf {
                float freq = 440.f;

                SmpSineWave() {}
               ~SmpSineWave() {}
                float sample(float _t) override { return std::sin(_t * freq * (2 * M_PI)); }
                void setFreq(float _freq) { freq = _freq; }
            };

            struct SmpSaw : SmpItf {
                float freq = 440.f;

                SmpSaw() {}
               ~SmpSaw() {}
                float sample(float _t) override { return std::abs(std::max(2*std::modf(_t*freq, &_t)-1.f, 0.5f)); }
                void setFreq(float _freq) { freq = _freq; }
            };

            struct SmpTestWave : SmpItf {
                float freq = 440.f;

                SmpTestWave() {}
               ~SmpTestWave() {}
                float sample(float _t) override { return std::sin(std::log(_t*freq) * _t*freq); }
                void setFreq(float _freq) { freq = _freq; }
            };

            // Set (lambda) function to sample from.
            struct SmpFunction : SmpItf {
                std::function<float(float)> sampler_func;

                SmpFunction(const std::function<float(float)> &_sampler_func) : sampler_func{_sampler_func} {}
               ~SmpFunction() {}
                float sample(float _t) override { return sampler_func(_t); }
            };
        }; // Sample END


        struct AudioSampler {
            struct AudioSampleConfig {
                Sample::SmpItf* sample_ptr;
                uint32_t   t;
                float weight;
            } sources[31]{}; // 31 audio sources: is it realistically enough/too many?

            uint32_t sample_rate = 1u, pad1, pad2, pad3;

            AudioSampler() { for (uint32_t i = 0u; i < 31u; ++i) sources[i] = { &Sample::audio_placeholder, 0u, 1.f }; }
            AudioSampler(const std::vector<Sample::SmpItf*> &_sources, const std::vector<float> &_weights = {}) { setSources(_sources, _weights); }
           ~AudioSampler() {}

            void setSampleRate(uint32_t _sample_rate) {
                sample_rate = _sample_rate;
            }

            float sample() {
                float __res = 0.f;
                uint32_t __finish_t = 0u;

                for (uint32_t i = 0u; i < 31u; ++i) {
                    if (sources[i].sample_ptr == &Sample::audio_placeholder) continue;

                    __finish_t = std::ceil(sources[i].sample_ptr->duration * sample_rate);
                    if (__finish_t && sources[i].t >= __finish_t)
                        sources[i].sample_ptr = &Sample::audio_placeholder;
                    __res += sources[i].weight * sources[i].sample_ptr->sample((sources[i].t++) / float(sample_rate));
                }
                return __res;
            }

            uint32_t addSource(Sample::SmpItf* _source, float _weight) {
                uint32_t __res_i = 31u;
                for (uint32_t i = 0u; i < 31u; ++i)
                    if (sources[i].sample_ptr == nullptr) {
                        __res_i = i;
                        sources[i].sample_ptr = _source;
                        sources[i].weight = _weight;
                        sources[i].t = 0u;
                    }
                SVKFW_WASSERT(__res_i < 31u, "AudioSampler :: addSource", "couldn't add audio sample\n");
                return __res_i;
            }

            void setSources(const std::vector<Sample::SmpItf*> &_sources, const std::vector<float> &_weights = {}) {
                SVKFW_ASSERT(_weights.empty() || _weights.size() == _sources.size(), std::invalid_argument,
                                "AudioSampler :: setSources", "Expected 0 weights or " + std::to_string(_sources.size()) +
                                                                              ", got " + std::to_string(_weights.size()));
                SVKFW_ASSERT(_sources.size() <= 31u, std::invalid_argument,
                                "AudioSampler :: setSources", "Expected up to 31 audio sources added, got " + std::to_string(_sources.size()));
                for (uint32_t i = 0u; i < 31u; ++i) {
                    sources[i].sample_ptr = i < _sources.size() ? _sources[i] : &Sample::audio_placeholder;
                    sources[i].weight     = i < _weights.size() ? _weights[i] : 1.f / std::max(_sources.size(), size_t(1u));
                    sources[i].t          = 0u;
                }
            }
        }; // AudioSampler END
    }; // Audio END
}; // Simple END

#endif