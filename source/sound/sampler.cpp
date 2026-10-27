#include "sound/sampler.h"

namespace Simple {
    namespace Audio {
        namespace Music {

            NoteReference::NoteReference() {}

            NoteReference::NoteReference(NoteIndex _ref_note_index, OctaveIndex _ref_octave, NoteFreq _ref_note_freq)
                            : ref_note_freq{_ref_note_freq}, ref_note{_ref_note_index, _ref_octave} {}

            TuningSystemItf::TuningSystemItf() {}

            TuningSystemItf::~TuningSystemItf() {}

            double  TuningSystemItf::intervalCoef(NoteInterval _interval) {
                bool __going_down = _interval < 0;
                if (__going_down) _interval = -_interval;

                NoteFull __interval_info = getNoteFull(_interval);
                double __res = std::pow(octaveCoef(), __interval_info.octave) * subOctaveCoef(__interval_info.note);
                return __going_down ? 1/__res : __res;
            }

            NoteFull TuningSystemItf::getNoteFull(NoteFullId _note_ind) {
                // Note: signed division is required ('numNotes()' is unsigned, mixing would convert negative ids to huge values)
                const NoteFullId __num_notes = NoteFullId(numNotes());
                NoteFull __res;
                __res.note   = _note_ind % __num_notes;
                __res.octave = _note_ind / __num_notes;

                if (__res.note < 0) {
                    __res.note   += __num_notes;
                    __res.octave -= 1;
                }

                return __res;
            }

            NoteFullId TuningSystemItf::getNoteFullId(NoteFull _full_note) {
                return _full_note.octave * numNotes() + _full_note.note;
            }

            TSJustIntonation::TSJustIntonation() {}

            TSJustIntonation::~TSJustIntonation() {}

            TuningSystemType TSJustIntonation::getType() { return JUST_INTONATION; }

            uint32_t TSJustIntonation::numNotes() { return 12u; }

            double TSJustIntonation::octaveCoef() { return Octave; }

            double TSJustIntonation::subOctaveCoef(NoteInterval _interval) {
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

            TSEqualTemperament12::TSEqualTemperament12() {}

            TSEqualTemperament12::~TSEqualTemperament12() {}

            TuningSystemType TSEqualTemperament12::getType() { return EQUAL_TEMPERAMENT_12T; }

            uint32_t TSEqualTemperament12::numNotes() { return 12u; }

            double TSEqualTemperament12::octaveCoef() { return Octave; }

            double TSEqualTemperament12::subOctaveCoef(NoteInterval _interval) {
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

            TSEqualTemperament::TSEqualTemperament(uint32_t _num_notes) : num_notes{_num_notes} {}

            uint32_t TSEqualTemperament::numNotes() { return num_notes; }

            double TSEqualTemperament::octaveCoef() { return       2.0; }

            TuningSystemType TSEqualTemperament::getType() { return EQUAL_TEMPERAMENT; }

            double TSEqualTemperament::subOctaveCoef(NoteInterval _interval) { return std::pow(std::pow(2, _interval), 1./num_notes); }

            double  TSEqualTemperament::intervalCoef(NoteInterval _interval) { return std::pow(std::pow(2, _interval), 1./num_notes); }

            TuningSystemItf* getTuningSystemHandler(TuningSystemType _ts_type) {
                switch (_ts_type) {
                    case EQUAL_TEMPERAMENT_12T: return new TSEqualTemperament12;
                    case EQUAL_TEMPERAMENT: return new TSEqualTemperament;
                    case JUST_INTONATION: return new TSJustIntonation;
                }
                return nullptr;
            }

            Scale::Scale(NoteFullId _tonic, ScaleType _scale_type)
                            : tonic{_tonic}, scale_type{_scale_type}, steps{GetSteps(_tonic, _scale_type)} {}

            std::vector<uint8_t> Scale::GetSteps(NoteFullId _tonic, ScaleType _scale_type) {
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
                    case HEPTATONIC_MINOR_MELODIC:   __res_steps = {0,2,3,5,7,9,11}; break;
                    case HEPTATONIC_MINOR_HARMONIC:  __res_steps = {0,2,3,5,7,8,11}; break;
                    case HEPTATONIC_MODE_IONIAN:     __res_steps = {0,2,4,5,7,9,11}; break;
                    case HEPTATONIC_MODE_DORIAN:     __res_steps = {0,2,3,5,7,9,10}; break;
                    case HEPTATONIC_MODE_PHRYGIAN:   __res_steps = {0,1,3,5,7,8,10}; break;
                    case HEPTATONIC_MODE_LYDIAN:     __res_steps = {0,2,4,6,7,9,11}; break;
                    case HEPTATONIC_MODE_MIXOLYDIAN: __res_steps = {0,2,4,5,7,9,10}; break;
                    case HEPTATONIC_MODE_LOCRIAN:    __res_steps = {0,1,3,5,6,8,10}; break;
                    case CHROMATIC:       __res_steps = {0,1,2,3,4,5,6,7,8,9,10,11}; break;
                }
                // Note: steps are semitones, so the result is taken modulo 12 (not modulo the number of steps)
                for (auto& step : __res_steps)
                    step = (step + _tonic) % 12;
                return __res_steps;
            }

            NoteHandler::NoteHandler(TuningSystemType _ts_type, NoteReference _ref_note)
                            : reference_note{_ref_note}, tuning_system{getTuningSystemHandler(_ts_type)} {
                reference_note.ref_note_ind = tuning_system->getNoteFullId(reference_note.ref_note);
            }

            NoteFreq NoteHandler::getNoteFreq(NoteFullId _note_ind) {
                return reference_note.ref_note_freq * tuning_system->intervalCoef(_note_ind - reference_note.ref_note_ind);
            }

            NoteFreq NoteHandler::getNoteFreq(NoteFull _full_note) {
                return getNoteFreq(tuning_system->getNoteFullId(_full_note));
            }

            NoteFullId NoteHandler::getNoteFullId(NoteFull _full_note) {
                return tuning_system->getNoteFullId(_full_note);
            }

            NoteFullId NoteHandler::getNoteFullId(NoteFreq _note_freq) {
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

            NoteFull NoteHandler::getNoteFull(NoteFullId _note_ind) {
                return tuning_system->getNoteFull(_note_ind);
            }

            NoteFull NoteHandler::getNoteFull(NoteFreq _note_freq) {
                return tuning_system->getNoteFull(getNoteFullId(_note_freq));
            }
        }; // Music END


        namespace Sample {

            SmpItf::~SmpItf() {}

            void SmpItf::setDuration(float _dur) { duration = _dur; }

            SmpPlaceholder::SmpPlaceholder() {}

            SmpPlaceholder::~SmpPlaceholder() {}

            float   SmpPlaceholder::sample (float  _t) { return 0.f; }

            vec2i16 SmpPlaceholder::sample2(double _t) { return 0  ; }

            SmpPlaceholder audio_placeholder;

            SmpSineWave::SmpSineWave() {}

            SmpSineWave::~SmpSineWave() {}

            float   SmpSineWave::sample (float  _t) { return (_t <= duration) * std::sin(_t * freq * (2 * M_PI)); }

            vec2i16 SmpSineWave::sample2(double _t) { return (_t <= duration) * std::sin(_t * freq * (2 * M_PI)) * INT16_MAX; }

            void SmpSineWave::setFreq(float _freq) { freq = _freq; }

            SmpSaw::SmpSaw() {}

            SmpSaw::~SmpSaw() {}

            float   SmpSaw::sample (float  _t) { float  __int_part; return (_t <= duration) * (2.f * std::modf(_t*freq, &__int_part) - 1.f); }

            vec2i16 SmpSaw::sample2(double _t) { double __int_part; return (_t <= duration) * (2.  * std::modf(_t*freq, &__int_part) - 1. ) * INT16_MAX; }

            void SmpSaw::setFreq(float _freq) { freq = _freq; }

            SmpTestWave::SmpTestWave() {}

            SmpTestWave::~SmpTestWave() {}

            float   SmpTestWave::sample (float  _t) { return (_t > 0.f && _t <= duration) ? std::sin(std::log(_t*freq) * _t*freq) : 0.f; }

            vec2i16 SmpTestWave::sample2(double _t) { return (_t > 0.  && _t <= duration) ? std::sin(std::log(_t*freq) * _t*freq) * INT16_MAX : 0.; }

            void SmpTestWave::setFreq(float _freq) { freq = _freq; }

            SmpFunction::SmpFunction(const std::function<float(float)> &_sampler_func) : sampler_func{_sampler_func} {}

            SmpFunction::~SmpFunction() {}

            float   SmpFunction::sample (float  _t) { return (_t <= duration) * sampler_func(_t); }

            vec2i16 SmpFunction::sample2(double _t) { return (_t <= duration) * sampler_func(_t) * INT16_MAX; }
        }; // Sample END


        AudioSampler::AudioSampler() { clearState(); }

        AudioSampler::AudioSampler(const std::vector<Sample::SmpItf*> &_sources, const std::vector<float> &_weights) { setSources(_sources, _weights); }

        AudioSampler::~AudioSampler() { clearState(); }

        void AudioSampler::clearState() {
            std::lock_guard<std::mutex> __lock(sources_mutex);
            for (uint32_t i = 0u; i < SVKFW_AS_MAX_SOURCES; ++i)
                sources[i] = { &Sample::audio_placeholder };
            sample_rate = 1u;
            inv_max_weight = inv_sr = 1.f;
        }

        void AudioSampler::setSampleRate(uint32_t _sample_rate) {
            sample_rate = std::max(_sample_rate, 1u);
            inv_sr = 1.f / sample_rate;
        }

        float AudioSampler::sample() {
            std::lock_guard<std::mutex> __lock(sources_mutex);
            float __res = 0.f;
            float t, __max_weight = SVKFW_AS_MIN_WEIGHT;

            for (uint32_t i = 0u; i < SVKFW_AS_MAX_SOURCES; ++i) {
                t = sources[i].t++ * inv_sr;

                if (t > sources[i].sample_ptr->duration)
                    sources[i] = { &Sample::audio_placeholder };

                __max_weight = std::max(__max_weight, sources[i].weight);
                __res += (sources[i].weight * inv_max_weight) * sources[i].sample_ptr->sample(t);
            }
            inv_max_weight = 1.f / __max_weight;
            return __res;
        }

        vec2i16 AudioSampler::sample2() {
            std::lock_guard<std::mutex> __lock(sources_mutex);
            vec2f __res{}; // Note: accumulated in float, int16 overflows when several sources are mixed
            float __max_weight = SVKFW_AS_MIN_WEIGHT;
            double t;

            for (uint32_t i = 0u; i < SVKFW_AS_MAX_SOURCES; ++i) {
                t = sources[i].t++ * inv_sr;

                if (t > sources[i].sample_ptr->duration)
                    sources[i] = { &Sample::audio_placeholder };

                __max_weight = std::max(__max_weight, sources[i].weight);
                __res += (sources[i].weight * inv_max_weight) * sources[i].sample_ptr->sample2(t).cast<float>();
            }
            inv_max_weight = 1.f / __max_weight;
            return Math::clampCL(__res, vec2f{float(INT16_MIN)}, vec2f{float(INT16_MAX)}).cast<int16_t>();
        }

        uint32_t AudioSampler::addSource(Sample::SmpItf* _source, float _weight) {
            std::lock_guard<std::mutex> __lock(sources_mutex);
            uint32_t __res_i = SVKFW_AS_MAX_SOURCES;
            if (_weight >= SVKFW_AS_MIN_WEIGHT) {
                for (uint32_t i = 0u; i < SVKFW_AS_MAX_SOURCES; ++i)
                    if (sources[i].sample_ptr == &Sample::audio_placeholder) {
                        __res_i = i;
                        sources[i].sample_ptr = _source;
                        sources[i].weight = _weight;
                        sources[i].t = 0u;

                        inv_max_weight = std::min(inv_max_weight, 1.f / _weight);
                        break;
                    }
                SVKFW_WASSERT(__res_i < SVKFW_AS_MAX_SOURCES, "AudioSampler :: addSource", "couldn't add audio sample\n");
            }
            else printf(SVKFW_WRAPINFO("AudioSampler :: addSource", "Source ignored - weight is too small (%f < %f)\n"), _weight, SVKFW_AS_MIN_WEIGHT);
            return __res_i;
        }

        void AudioSampler::setSources(const std::vector<Sample::SmpItf*> &_sources, const std::vector<float> &_weights) {
            SVKFW_ASSERT(_weights.empty() || _weights.size() == _sources.size(), std::invalid_argument,
                            "AudioSampler :: setSources", "Expected 0 weights or " + std::to_string(_sources.size()) +
                                                                          ", got " + std::to_string(_weights.size()));
            SVKFW_ASSERT(_sources.size() <= SVKFW_AS_MAX_SOURCES, std::invalid_argument,
                            "AudioSampler :: setSources", "Expected up to " + std::to_string(SVKFW_AS_MAX_SOURCES) + " audio sources, got " + std::to_string(_sources.size()));
            std::lock_guard<std::mutex> __lock(sources_mutex);

            float __max_weight = SVKFW_AS_MIN_WEIGHT;
            for (uint32_t i = 0u; i < SVKFW_AS_MAX_SOURCES; ++i) {
                sources[i].sample_ptr = i < _sources.size() ? _sources[i] : &Sample::audio_placeholder;
                sources[i].weight     = i < _weights.size() ? std::max(_weights[i], 0.f) : 1.f / std::max(_sources.size(), size_t(1u));
                sources[i].t          = 0u;
                __max_weight = std::max(__max_weight, sources[i].weight);
            }
            inv_max_weight = 1.f / __max_weight;
        }
    }; // Audio END
}; // Simple END
