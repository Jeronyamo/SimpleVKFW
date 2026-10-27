#ifndef SVKFW_SAMPLER_H
#define SVKFW_SAMPLER_H

#include <cmath>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>

#include "common/terminal.h"
#include "math/vectors.h"


// SVKFW Audio Source constants

#define SVKFW_AS_MIN_WEIGHT     1e-4f
#define SVKFW_AS_MAX_SOURCES    127u
#define SVKFW_AS_MAX_DURATION   1e6f


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

                NoteReference();
                NoteReference(NoteIndex _ref_note_index, OctaveIndex _ref_octave, NoteFreq _ref_note_freq);
                NoteReference(const NoteReference &_note_ref) = default;
            } Ref_Note_A440;


    //  == === ====     Tuning systems     ==== === ==  \\

            enum TuningSystemType {
                EQUAL_TEMPERAMENT_12T,
                EQUAL_TEMPERAMENT,
                JUST_INTONATION
            };

            struct TuningSystemItf {
                TuningSystemItf();
                virtual ~TuningSystemItf();

                virtual TuningSystemType getType() = 0;
                virtual uint32_t numNotes() = 0;
                virtual double octaveCoef() = 0;
                virtual double subOctaveCoef(NoteInterval _interval) = 0;
                virtual double  intervalCoef(NoteInterval _interval);
                NoteFull getNoteFull(NoteFullId _note_ind);
                NoteFullId getNoteFullId(NoteFull _full_note);
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

                TSJustIntonation();
               ~TSJustIntonation();

                virtual TuningSystemType getType() override;
                virtual uint32_t numNotes() override;
                virtual double octaveCoef() override;
                virtual double subOctaveCoef(NoteInterval _interval) override;
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

                TSEqualTemperament12();
               ~TSEqualTemperament12();

                virtual TuningSystemType getType() override;
                virtual uint32_t numNotes() override;
                virtual double octaveCoef() override;
                virtual double subOctaveCoef(NoteInterval _interval) override;
            }; // TSEqualTemperament12 END


            struct TSEqualTemperament : TuningSystemItf {
                uint32_t num_notes;
                TSEqualTemperament(uint32_t _num_notes = 12);

                virtual uint32_t numNotes() override;
                virtual double octaveCoef() override;
                virtual TuningSystemType getType() override;
                virtual double subOctaveCoef(NoteInterval _interval) override;
                virtual double  intervalCoef(NoteInterval _interval) override;
            }; // TSEqualTemperament END

            TuningSystemItf* getTuningSystemHandler(TuningSystemType _ts_type);


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

                Scale(NoteFullId _tonic, ScaleType _scale_type);

                static std::vector<uint8_t> GetSteps(NoteFullId _tonic, ScaleType _scale_type);
            }; // Scale END


            struct NoteHandler {
                NoteReference   reference_note;
                std::shared_ptr<TuningSystemItf> tuning_system; // Note: owned (previously leaked), shared between copies

                NoteHandler(TuningSystemType _ts_type = EQUAL_TEMPERAMENT_12T, NoteReference _ref_note = Ref_Note_A440);

                NoteFreq getNoteFreq(NoteFullId _note_ind);
                NoteFreq getNoteFreq(NoteFull _full_note);
                NoteFullId getNoteFullId(NoteFull _full_note);
                NoteFullId getNoteFullId(NoteFreq _note_freq);
                NoteFull getNoteFull(NoteFullId _note_ind);
                NoteFull getNoteFull(NoteFreq _note_freq);
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
                float duration = SVKFW_AS_MAX_DURATION; // Sample duration in seconds. Default: SVKFW_AS_MAX_DURATION for 'infinite' samples.

                virtual ~SmpItf();

                // t = (t_step / sample_rate) * frequency
                virtual float   sample (float  _t) = 0;
                virtual vec2i16 sample2(double _t) = 0;
                void setDuration(float _dur);
            }; // SmpItf END

            struct SmpPlaceholder : SmpItf {
                SmpPlaceholder();
               ~SmpPlaceholder();
                float   sample (float  _t) override;
                vec2i16 sample2(double _t) override;
            };
            extern SmpPlaceholder audio_placeholder;

            struct SmpSineWave : SmpItf {
                float freq = 440.f;

                SmpSineWave();
               ~SmpSineWave();
                float   sample (float  _t) override;
                vec2i16 sample2(double _t) override;
                void setFreq(float _freq);
            };

            struct SmpSaw : SmpItf {
                float freq = 440.f;

                SmpSaw();
               ~SmpSaw();
                // Saw wave in [-1, 1)
                float   sample (float  _t) override;
                vec2i16 sample2(double _t) override;
                void setFreq(float _freq);
            };

            struct SmpTestWave : SmpItf {
                float freq = 440.f;

                SmpTestWave();
               ~SmpTestWave();
                // Note: log(0) = -inf, so t = 0 is skipped
                float   sample (float  _t) override;
                vec2i16 sample2(double _t) override;
                void setFreq(float _freq);
            };

            // Set (lambda) function to sample from.
            struct SmpFunction : SmpItf {
                std::function<float(float)> sampler_func;

                SmpFunction(const std::function<float(float)> &_sampler_func);
               ~SmpFunction();
                float   sample (float  _t) override;
                vec2i16 sample2(double _t) override;
            };
        }; // Sample END


        struct AudioSampler {
            struct AudioSampleConfig {
                Sample::SmpItf* sample_ptr;
                uint32_t   t;
                float weight;
            } sources[SVKFW_AS_MAX_SOURCES]{};

            uint32_t sample_rate = 1u;
            float inv_max_weight = 1.f, inv_sr = 1.f;
            uint32_t pad1_;
            std::mutex sources_mutex; // Note: sources are changed from the main thread while being sampled in the audio thread

            AudioSampler();
            AudioSampler(const std::vector<Sample::SmpItf*> &_sources, const std::vector<float> &_weights = {});
           ~AudioSampler();

            void clearState();

            void setSampleRate(uint32_t _sample_rate);

            float sample();

            vec2i16 sample2();

            uint32_t addSource(Sample::SmpItf* _source, float _weight);

            void setSources(const std::vector<Sample::SmpItf*> &_sources, const std::vector<float> &_weights = {});
        }; // AudioSampler END
    }; // Audio END
}; // Simple END

#endif