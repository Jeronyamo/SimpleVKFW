#include <vector>
#include <iostream>
#include <thread>
#include <future>
#include <chrono>

#include "tests.h"

// What to test
#include "math/vectors.h"
#include "main/image.h"
#include "sound/player.h"
#include "sound/tracker.h"
#include "interface/rtaudio_wrap.h"
#include "data/codecs.h"
#include "core/etcs_base.h"
#include "core/event_handler.h"
#include "systems/physics.h"
#include "systems/physics_scene.h"
#include "core/engine.h"



namespace Simple {

#undef  SVKFW_TEST_GROUP
#define SVKFW_TEST_GROUP "VectorMatrix"


    SVKFW_ADD_TEST(VecMatOps, "Vector/Matrix Operators") {
        vec2f  tmp2{1.f, 2.f};
        vec2f check{1.f, 2.f};

        Test::testAssert(tmp2.x == 1.f && tmp2.y == 2.f,
                            "vec2 constructor error: " + std::to_string(tmp2.x) + "!=1.0 or "
                                                       + std::to_string(tmp2.y) + "!=2.0");

        Test::testAssert((tmp2 == check).all(),
                            "vec2 eq operator error: " + std::to_string(check.x) + "!=1.0 or "
                                                       + std::to_string(check.y) + "!=2.0");


        tmp2 = { 3, 4 };

        Test::testAssert(tmp2.x == 3.f && tmp2.y == 4.f,
                            "vec2 assignment error: " + std::to_string(tmp2.x) + "!=3.0 or "
                                                      + std::to_string(tmp2.y) + "!=4.0");


        tmp2["yx"] = { 6, 5 };

        Test::testAssert(tmp2.x == 5.f && tmp2.y == 6.f,
                            "vec2 subvector assignment error (1): " + std::to_string(tmp2.x) + "!=5.0 or "
                                                                    + std::to_string(tmp2.y) + "!=6.0");


        tmp2.subvec<'y','x','y'>() = { 6, 7, 8 };

        Test::testAssert(tmp2.x == 7.f && tmp2.y == 8.f,
                            "vec2 subvector assignment error (2): " + std::to_string(tmp2.x) + "!=7.0 or "
                                                                    + std::to_string(tmp2.y) + "!=8.0");
    }


#undef  SVKFW_TEST_GROUP
#define SVKFW_TEST_GROUP "Image"


    SVKFW_ADD_TEST(ImageIO, "Image Save/Load") {
        Img::Image4f image4f("tests/resources/test_bunny.png");
        Img::Image3f image3f("tests/resources/test_bunny.png");
        Img::Image3u image3u("tests/resources/test_bunny.png");

        printf("Images loaded\n");
        Img::Image3f img3f = image3u.imgSwitchType();

        printf("Image u: w: %d, h: %d, ch: %d\n", image3u.width, image3u.height, image3u.channels);
        printf("Image f: w: %d, h: %d, ch: %d\n", image4f.width, image4f.height, image4f.channels);
        printf("Image3f: w: %d, h: %d, ch: %d\n", image3f.width, image3f.height, image3f.channels);
        printf("Img3f:   w: %d, h: %d, ch: %d\n",   img3f.width,   img3f.height,   img3f.channels);

        image3u.imgSave<Img::PNG>("tests/out/test_bunny_3u.png");
        image4f.imgSave<Img::PNG>("tests/out/test_bunny_4f.png");
        image3f.imgSave<Img::PNG>("tests/out/test_bunny_3f.png");
        img3f  .imgSave<Img::PNG>("tests/out/test_bunny_img3f.png");
    }


    SVKFW_ADD_TEST(ImageTransforms, "Basic image transforms") {
        Img::Image4f image4f("tests/resources/test_bunny.png");

        image4f.imgMirror(true, true);
        image4f.imgSave<Img::PNG>("tests/out/test_bunny_mirror_xy.png");
        image4f.imgMirror(true, true);
        image4f = image4f.imgResample(image4f.width*0.566, image4f.height*0.666);
        image4f.imgSave<Img::PNG>("tests/out/test_bunny_resampled.png");
        image4f.imgTranspose();
        image4f.imgSave<Img::PNG>("tests/out/test_bunny_transpose.png");
        image4f.imgSave<Img::PNG>("tests/out/test_bunny_transpose.png");
        Img::Image1f r   = image4f.imgGetChannels(0);
        Img::Image2f bg  = image4f.imgGetChannels(2, 1);
        Img::Image3f bgr =   Img::CombineChannels(bg, r);
        bgr.imgSave<Img::PNG>("tests/out/test_bunny_BGR.png");
    }


    void genDiffImage(const Img::Image3f &image1, const Img::Image3f &image2, const std::string &_img_path_res) {
        if (image1.width != image2.width || image1.height != image2.height)
            throw std::runtime_error(SVKFW_WRAPERR("Test2 :: genDiffImage", "image sizes don't match"));

        Img::Image3f __res{image1.width, image1.height};
        #pragma omp parallel for
        for (uint32_t i = 0u; i < __res.img.size(); ++i) {
            float __diff = image1.img[i].x - image2.img[i].x;
            vec3f __other_color = __diff < 0.f ? vec3f{1.f,0.f,0.f} : vec3f{0.f,0.f,1.f};
            __res.img[i] = __other_color * std::abs(__diff) + vec3f{1.f} * (1.f - std::abs(__diff));
            __res.img[i].x = std::pow(__res.img[i].x, 4.f);
            __res.img[i].y = std::pow(__res.img[i].y, 4.f);
            __res.img[i].z = std::pow(__res.img[i].z, 4.f);
        }

        __res.imgSave<Img::PNG>(_img_path_res);
    }

    SVKFW_ADD_TEST(ImageDiff, "Save/Load img differences") {
        Img::Image3f image_ref("tests/resources/ref_1.png");
        Img::Image3f image_res("tests/resources/ep7_res_1.png");

        float psnr = Img::Metric::PSNR(image_ref, image_res);
        printf("PSNR (ref_1, ep7_res_1) = %f\n", psnr);

        genDiffImage(image_res, image_ref, "tests/out/diff_ep7_res_1.png");
    }


    SVKFW_ADD_TEST(ImageMetrics, "Image Metrics") {
        Img::Image4f image4f("tests/resources/test_bunny.png");

        float res_mse  = Img::Metric:: MSE(image4f, image4f);
        float res_psnr = Img::Metric::PSNR(image4f, image4f);
        float res_ssim = Img::Metric::SSIM(image4f, image4f);
        printf("Image metrics for identical images:  MSE = %f\n", res_mse );
        printf("Image metrics for identical images: PSNR = %f\n", res_psnr);
        printf("Image metrics for identical images: SSIM = %f\n", res_ssim);

        Test::testAssert(res_mse , Test::TEST_COMP_E,   0.f, "MSE  for identical images", "MSE ", "Expected");
        Test::testAssert(res_psnr, Test::TEST_COMP_E, 100.f, "PSNR for identical images", "PSNR", "Expected");
        Test::testAssert(res_ssim, Test::TEST_COMP_E,   1.f, "SSIM for identical images", "SSIM", "Expected");
    }


#undef  SVKFW_TEST_GROUP
#define SVKFW_TEST_GROUP "Audio"


    struct AsyncExit {
        static char getAnswer() {    
            char __res = ' ';
            while (__res != 'q' && __res != 'Q') { std::cin >> __res; }
            return __res;
        }

        std::future<char> __res = std::async(getAnswer);

        bool gotAnswer() { return __res.wait_for(std::chrono::seconds(1)) == std::future_status::ready; }
    }; // AsyncExit END

    SVKFW_ADD_TEST(AudioDevice, "Audio devices info") {
        Simple::RTA::AudioHandler audio_device;

        for (uint32_t dev_id : audio_device.rta_handler.getDeviceIds()) {
            std::cout << audio_device.deviceGetInfoStr_(dev_id) << "\n";
        }
    }


    SVKFW_ADD_TEST(AudioDeviceUpdate, "Audio devices update") {
        RTA::AudioHandler audio_device;

        audio_device.streamSetMode(RTA::STREAM_MODE_DUPLEX,
                                   RTA::DEVICE_MODE_DEFAULT,
                                   RTA::DEVICE_MODE_DEFAULT);

        // while (true) {
            audio_device.deviceInputUpdate();
            audio_device.deviceOutputUpdate();
        // }
    }


    SVKFW_ADD_TEST(AudioStreamOpen, "Audio stream open") {
        RTA::AudioHandler audio_o;
        audio_o.streamSetMode(RTA::STREAM_MODE_DUPLEX,
                              RTA::DEVICE_MODE_DEFAULT,
                              RTA::DEVICE_MODE_DEFAULT);
        audio_o.deviceInputUpdate();
        audio_o.deviceOutputUpdate();
        audio_o.streamSetOptions(RTAUDIO_NONINTERLEAVED, 0, "Test Audio Output/Input", 0);
        audio_o.deviceOutputSetParameters(2);
        audio_o.deviceInputSetParameters(2);
        audio_o.streamOpen(0, 256, RTAUDIO_FLOAT32);
        Test::testAssert(audio_o.streamIsOpen(), "Audio stream is not open");
    }


    SVKFW_ADD_TEST(AudioStreamDuplex, "Audio run duplex stream") {
        RTA::AudioHandler audio_o;
        const uint32_t __channels = 2;
        const uint32_t __buf_frames = 544;
        const uint32_t __buf_size = __buf_frames * __channels * sizeof(int);

        audio_o.streamSetMode(RTA::STREAM_MODE_DUPLEX,
                              RTA::DEVICE_MODE_DEFAULT,
                              RTA::DEVICE_MODE_DEFAULT);
        audio_o.deviceUpdate();
        audio_o.streamSetOptions(0, 0, "Test Audio Output/Input", 0);
        audio_o.deviceOutputSetParameters(__channels);
        audio_o.deviceInputSetParameters(__channels);
        audio_o.callbackSet(RTA::rtacbInOut);

        audio_o.streamOpen(0, __buf_frames, RTAUDIO_SINT16, false);
        Test::testAssert(audio_o.streamIsOpen(), "Audio stream is not open");

        audio_o.streamStart();

        AsyncExit __exit_state;
        while (!__exit_state.gotAnswer()) { audio_o.deviceUpdate(); }
    }


    SVKFW_ADD_TEST(AudioSeparateStreams, "Run separate in/out streams") {
#ifdef RTA_ASIO
        test_system.terminal_h.print_w("AudioSeparateStreams: ASIO doesn't support multiple audio streams, aborting");
        return;
#endif
        RTA::AudioHandler audio_i, audio_o;
        const uint32_t __channels = 1;
        const uint32_t __buf_frames = 512;
        const uint32_t __buf_size = __buf_frames * __channels * sizeof(int);
        // TODO:
        // RTA::Global_audio_buf.buf_size = __buf_frames * __channels;
        // RTA::Global_audio_buf.buf = new float[__buf_frames * __channels]{};

        audio_i.streamSetMode(RTA::STREAM_MODE_IN,
                              RTA::DEVICE_MODE_DEFAULT,
                              RTA::DEVICE_MODE_DEFAULT);
        audio_o.streamSetMode(RTA::STREAM_MODE_OUT,
                              RTA::DEVICE_MODE_DEFAULT,
                              RTA::DEVICE_MODE_DEFAULT);
        audio_i.deviceUpdate();
        audio_o.deviceUpdate();
        audio_i.streamSetOptions(0, 0, "Test Audio Input", 0);
        audio_o.streamSetOptions(0, 0, "Test Audio Output", 0);
        audio_o.deviceOutputSetParameters(__channels);
        audio_i.deviceInputSetParameters(__channels);
        // TODO:
        // audio_i.callbackSet(RTA::rtacb_record);   // Will use OGG Opus to save records
        // audio_o.callbackSet(RTA::rtacb_playback);

        audio_i.streamOpen(48000, __buf_frames, RTAUDIO_FLOAT32, false);
        audio_o.streamOpen(48000, __buf_frames, RTAUDIO_FLOAT32, false);
        Test::testAssert(audio_i.streamIsOpen(), "Audio stream i is not open");
        Test::testAssert(audio_o.streamIsOpen(), "Audio stream o is not open");

        audio_o.streamStart();
        audio_i.streamStart();

        return; // TODO: this test requires OGG file save/load + Opus codec.

        AsyncExit __exit_state;
        while (!__exit_state.gotAnswer()) { audio_i.deviceUpdate(); audio_o.deviceUpdate(); }
    }


    // '_on_open' gets the stream sample rate before the stream starts
    void audioPlayer(std::vector<Audio::Sample::SmpItf*> _sources, const std::vector<float> &_weights,
                     const std::function<void(uint32_t)> &_on_open = nullptr) {
        RTA::AudioHandler audio_o;
        const uint32_t __channels   =   2;
        const uint32_t __buf_frames = 512;

        audio_o.streamSetMode(RTA::STREAM_MODE_OUT,
                              RTA::DEVICE_MODE_DEFAULT);
        audio_o.deviceUpdate();
        audio_o.streamSetOptions(0, 0, "Test Audio Output", 0);
        audio_o.deviceOutputSetParameters(__channels);
        audio_o.callbackSet(RTA::rtacbDefault2);

        audio_o.streamOpen(0, __buf_frames, RTAUDIO_SINT16, false);
        Test::testAssert(audio_o.streamIsOpen(), "Audio stream is not open");

        // Set audio to play
        audio_o.stream_config.audio_sampler.setSources(_sources, _weights);
        audio_o.stream_config.audio_sampler.setSampleRate(audio_o.stream_config.sample_rate);
        if (_on_open) _on_open(audio_o.stream_config.sample_rate);

        // Start stream
        audio_o.streamStart();

        AsyncExit __exit_state;
        while (!__exit_state.gotAnswer()) { audio_o.deviceUpdate(); }
        audio_o.streamStop();
    }

    SVKFW_ADD_TEST(AudioFuncSampler, "Audio function sampler") {
        static Audio::Sample::SmpSineWave __sine_wave1;
        static Audio::Sample::SmpSineWave __sine_wave2;

        __sine_wave1.setFreq(440.f);
        __sine_wave2.setFreq(440.f * (2.f/3));

        static auto __wave_sampler = [&](float _t) { return 0.45f * __sine_wave1.sample(_t) + 0.45f * __sine_wave2.sample(_t); };
        static Audio::Sample::SmpFunction __sampler{__wave_sampler};

        audioPlayer({&__sampler}, {1.f});
    }


    SVKFW_ADD_TEST(AudioMixerSampler, "Audio - mixer sampler") {
        static Audio::Sample::SmpSineWave __sine_wave1;
        static Audio::Sample::SmpSineWave __sine_wave2;

        __sine_wave1.setFreq(440.f);
        __sine_wave2.setFreq(440.f * (2.f/3));
        __sine_wave1.setDuration(2.f);
        __sine_wave2.setDuration(3.f);

        audioPlayer({&__sine_wave1, &__sine_wave2}, {0.45f, 0.45f});
    }


    SVKFW_ADD_TEST(AudioSawSampler, "Audio - saw sampler") {
        static Audio::Sample::SmpSaw __saw_sampler;
        __saw_sampler.setFreq(440.f);

        audioPlayer({&__saw_sampler}, {0.9f});
    }


    SVKFW_ADD_TEST(AudioOGGFileSampler, "Audio - OGG file playback") {
        Simple::Audio::Sample::SmpAudioFile __ogg_sampler{"tests/resources/KMT - Daedalus (extended).ogg"};

        audioPlayer({&__ogg_sampler}, {1.f});
    }



#undef  SVKFW_TEST_GROUP
#define SVKFW_TEST_GROUP "Synth"


    SVKFW_ADD_TEST(SynthLibraryRoundTrip, "Synth - instrument library save/load") {
        Audio::Synth::InstrumentLibrary __library;
        Test::testAssert(__library.load("resources/music/instruments.json"), "Could not load the instrument library");
        Test::testAssert(__library.size() > 0u, "Instrument library is empty");
        Test::testAssert(__library.save("tests/resources/instruments_roundtrip.json"), "Could not save the instrument library");

        Audio::Synth::InstrumentLibrary __reloaded;
        Test::testAssert(__reloaded.load("tests/resources/instruments_roundtrip.json"), "Could not reload the instrument library");
        Test::testAssert(__reloaded.size() == __library.size(), "Instrument count changed after save/load");
        for (uint32_t i = 0u; i < __library.size(); ++i) {
            const Audio::Synth::InstrumentParams &__a = __library.instruments[i].params, &__b = __reloaded.instruments[i].params;
            Test::testAssert(__library.instruments[i].name == __reloaded.instruments[i].name, "Instrument name changed after save/load");
            Test::testAssert(__a.num_oscillators == __b.num_oscillators && __a.oscillators[0].wave == __b.oscillators[0].wave &&
                             __a.amp_env.decay == __b.amp_env.decay, "Instrument parameters changed after save/load");
        }
        std::remove("tests/resources/instruments_roundtrip.json");
    }


    SVKFW_ADD_TEST(SynthInstruments, "Synth - play every library instrument (C-E-G-C)") {
        static Audio::Synth::InstrumentLibrary __library;
        Test::testAssert(__library.load("resources/music/instruments.json"), "Could not load the instrument library");

        const uint32_t __sample_rate = 48000u;
        Audio::Music::NoteHandler __notes;
        static std::vector<float> __track;
        __track.clear();
        for (uint32_t i = 0u; i < __library.size(); ++i) {
            printf("%2u: %s\n", i, __library.instruments[i].name.c_str());
            for (Audio::Music::NoteFullId __note : {48, 52, 55, 60}) {
                const std::vector<float> __rendered = Audio::Synth::renderNote(__library.instruments[i].params, __notes.getNoteFreq(__note), 0.2f, __sample_rate, 2.f);
                __track.insert(__track.end(), __rendered.begin(), __rendered.end());
                __track.resize(__track.size() + __sample_rate / 20u, 0.f);
            }
        }

        // Note: 'audioPlayer' opens the stream at the device rate, so the track is resampled by nearest neighbour
        static auto __track_sampler = [](float _t) {
            const size_t __id = size_t(_t * 48000.f);
            return __id < __track.size() ? __track[__id] : 0.f;
        };
        static Audio::Sample::SmpFunction __sampler{__track_sampler};
        __sampler.setDuration(float(__track.size()) / __sample_rate);
        audioPlayer({&__sampler}, {1.f});
    }



    SVKFW_ADD_TEST(TrackerSongRoundTrip, "Tracker - song save/load and offline render") {
        Audio::Tracker::Song __song;
        Test::testAssert(__song.load("resources/music/songs/demo_chip.json"), "Could not load the demo song");
        Test::testAssert(__song.instruments.size() > 0u, "The demo song has no instruments");

        Test::testAssert(__song.save("tests/resources/song_roundtrip.json"), "Could not save the song");
        Audio::Tracker::Song __reloaded;
        Test::testAssert(__reloaded.load("tests/resources/song_roundtrip.json"), "Could not reload the song");
        Test::testAssert(__reloaded.numChannels() == __song.numChannels() && __reloaded.patterns.size() == __song.patterns.size() &&
                         __reloaded.order == __song.order && __reloaded.instruments.size() == __song.instruments.size(), "Song structure changed after save/load");
        for (size_t p = 0u; p < __song.patterns.size(); ++p)
            for (uint32_t r = 0u; r < __song.patterns[p].rows; ++r)
                for (uint32_t ch = 0u; ch < __song.numChannels(); ++ch)
                    Test::testAssert(__song.cell(uint32_t(p), r, ch).toString() == __reloaded.cell(uint32_t(p), r, ch).toString(), "Cell changed after save/load");
        std::remove("tests/resources/song_roundtrip.json");

        const std::vector<vec2f> __frames = Audio::Tracker::SongPlayer::renderSong(__song, 48000u, 60.f);
        const float __expected = 3.f * 64.f * __song.rowSeconds(); // 3 orders of 64 rows
        Test::testAssert(__frames.size() >= size_t(__expected * 48000.f) && __frames.size() < size_t((__expected + 5.f) * 48000.f),
                         "Rendered song length is off");
    }


    SVKFW_ADD_TEST(TrackerDemoSong, "Tracker - play the demo song") {
        static Audio::Tracker::Song __song;
        static Audio::Tracker::SongPlayer __player;
        Test::testAssert(__song.load("resources/music/songs/demo_chip.json"), "Could not load the demo song");

        __player.setSong(&__song);
        audioPlayer({&__player}, {1.f}, [](uint32_t _sample_rate) {
            __player.setSampleRate(_sample_rate);
            __player.play();
        });
        __player.stopNow();
    }


    SVKFW_ADD_TEST(AudioWAVRoundTrip, "Audio - WAV write/read in every PCM format") {
        const uint16_t __bits[] = { 8u, 16u, 24u, 32u };
        for (uint16_t __channels = 1u; __channels <= 3u; ++__channels)
            for (uint16_t __bits_per_sample : __bits) {
                const uint32_t __num_frames = 1000u;
                std::vector<float> __samples(__num_frames * __channels);
                for (size_t i = 0u; i < __samples.size(); ++i)
                    __samples[i] = 0.9f * std::sin(0.01f * float(i)) + (i == 7u ? 2.f : 0.f); // One clipped sample

                File::ReaderWriterWAV __wav;
                __wav.file_content.channels        = __channels;
                __wav.file_content.bits_per_sample = __bits_per_sample;
                __wav.file_content.sample_rate     = 44100u;
                Test::testAssert(__wav.file_content.setSamples(__samples.data(), __num_frames), "Could not encode WAV samples");
                Test::testAssert(__wav.write("tests/resources/roundtrip.wav"), "Could not write the WAV file");

                File::ReaderWriterWAV __reloaded{"tests/resources/roundtrip.wav"};
                const File::ContentWAV &__content = __reloaded.file_content;
                Test::testAssert(__content.format_tag == File::WAV_FORMAT_PCM && __content.channels == __channels &&
                                 __content.bits_per_sample == __bits_per_sample && __content.sample_rate == 44100u &&
                                 __content.numFrames() == __num_frames, "WAV format changed after write/read");

                std::vector<float> __decoded;
                Test::testAssert(__content.getSamples(__decoded) && __decoded.size() == __samples.size(), "Could not decode WAV samples");
                const float __tolerance = 2.f / float(1u << (__bits_per_sample - 1u)) + 1e-6f;
                for (size_t i = 0u; i < __samples.size(); ++i)
                    Test::testAssert(std::fabs(__decoded[i] - std::min(__samples[i], 1.f)) <= __tolerance, "WAV sample changed after write/read");
            }
        std::remove("tests/resources/roundtrip.wav");
    }


    SVKFW_ADD_TEST(AudioOpusRoundTrip, "Audio - Ogg Opus encode/write/read/decode") {
        const uint32_t __num_frames = 48000u + 123u; // Not a multiple of the 20 ms frame
        std::vector<float> __samples(__num_frames * 2u);
        for (uint32_t i = 0u; i < __num_frames; ++i) {
            __samples[2u * i     ] = 0.5f * std::sin(2.f * 3.14159265f * 440.f * float(i) / 48000.f);
            __samples[2u * i + 1u] = 0.5f * std::sin(2.f * 3.14159265f * 660.f * float(i) / 48000.f);
        }

        Codec::EncoderOpus __encoder;
        __encoder.tags["TITLE"] = "Round trip";
        File::ReaderWriterOGG __ogg;
        Test::testAssert(__encoder.createEncoder(2), "Could not create the Opus encoder");
        Test::testAssert(__encoder.encodeOGG(__samples.data(), __num_frames, __ogg.file_content), "Could not encode Opus");
        Test::testAssert(__ogg.write("tests/resources/roundtrip.ogg"), "Could not write the Ogg file");

        File::ReaderWriterOGG __reloaded{"tests/resources/roundtrip.ogg"};
        const std::vector<File::ContentOGG::PageOGG> &__pages = __reloaded.file_content.pages;
        Test::testAssert(__pages.size() == __ogg.file_content.pages.size() && __pages.size() > 3u, "Ogg page count changed after write/read");
        Test::testAssert((__pages.front().header_flags & 0x02u) && (__pages.back().header_flags & 0x04u), "Ogg stream has no begin/end flags");
        Test::testAssert(__pages.back().granule_position == uint64_t(__encoder.preskip) + __num_frames, "Wrong last granule position");
        for (const File::ContentOGG::PageOGG &__page : __pages)
            Test::testAssert(__page.checksum == File::ContentOGG::Checksum(__page), "Wrong Ogg page checksum");

        // Same packet assembly as Sample::SmpAudioFile
        Codec::DecoderOpus __decoder;
        std::vector<int16_t> __decoded;
        std::vector<unsigned char> __packet;
        for (const File::ContentOGG::PageOGG &__page : __pages) {
            uint32_t __offset = 0u;
            for (uint32_t i = 0u; i < __page.total_segments; ++i) {
                __packet.insert(__packet.end(), __page.segment_data.begin() + __offset, __page.segment_data.begin() + __offset + __page.segment_table[i]);
                __offset += __page.segment_table[i];
                if (__page.segment_table[i] == 255u) continue;
                if (__decoder.decodePacket(__packet) && __decoder.curr_frame_size)
                    __decoded.insert(__decoded.end(), __decoder.pcm_out.begin(), __decoder.pcm_out.begin() + __decoder.curr_frame_size);
                __packet.clear();
            }
        }
        Test::testAssert(__decoder.opus_head.channels == 2u && __decoder.opus_head.preskip == __encoder.preskip, "Wrong OpusHead after write/read");
        Test::testAssert(__decoder.opus_tags.tags["TITLE"] == "Round trip", "Wrong OpusTags after write/read");
        Test::testAssert(__decoded.size() >= (size_t(__encoder.preskip) + __num_frames) * 2u, "Decoded Opus stream is too short");

        // Lossy: compare the signal energy after the pre-skip
        double __energy_in = 0., __energy_out = 0., __error = 0.;
        for (uint32_t i = 0u; i < __num_frames * 2u; ++i) {
            const double __decoded_sample = __decoded[size_t(__encoder.preskip) * 2u + i] / 32768.;
            __energy_in  += double(__samples[i]) * __samples[i];
            __energy_out += __decoded_sample * __decoded_sample;
            __error      += (__decoded_sample - __samples[i]) * (__decoded_sample - __samples[i]);
        }
        Test::testAssert(std::fabs(__energy_out / __energy_in - 1.) < 0.1, "Decoded Opus signal energy differs too much");
        Test::testAssert(__error / __energy_in < 0.05, "Decoded Opus signal differs too much");
        std::remove("tests/resources/roundtrip.ogg");
    }



#undef  SVKFW_TEST_GROUP
#define SVKFW_TEST_GROUP "Parser"


    void parseNext(const Parser::ProdRule &_prod_rule, Parser::RuleState _state, Parser::NextSymbolType _expected_state) {
        static uint32_t err_count = 1u;
        Parser::NextSymbolType __next_info = _prod_rule.checkNext(_state);
        // printf("Next state: %d(%d)\n", (int)__next_info, (int)_expected_state);
        if (__next_info != _expected_state) throw std::runtime_error(SVKFW_WRAPERR("Test4 :: parseNext", "Error " + std::to_string(err_count)));
        err_count += 1;
    }

    void parseTerm(const Parser::ProdRule &_prod_rule, Parser::RuleState &_state, char _expected_ch) {
        static uint32_t err_count = 1u;
        bool __success = _prod_rule.parseRule(_state, _expected_ch);
        // printf("Next terminal symbol: %c(%c)\n", (_state.substr_id < _prod_rule.nt_index.back().second ? _prod_rule.rule_str[_state.substr_id-1] : ' '), _expected_ch); // "substr_id - 1" because parseRule increments it after parsing
        if (!__success) throw std::runtime_error(SVKFW_WRAPERR("Test4 :: parseTerm", "Error " + std::to_string(err_count++)));
        err_count += 1;
    }

    void parseNonterm(const Parser::ProdRule &_prod_rule, Parser::RuleState &_state, Parser::Nonterminal _expected_nt) {
        static uint32_t err_count = 1u;
        _prod_rule.getNontermNext(_state);
        // printf("Next nonterm: %d(%d)\n", _state.nonterm, _expected_nt);
        if (_state.nonterm != _expected_nt) throw std::runtime_error(SVKFW_WRAPERR("Test4 :: parseNonterm", "Error " + std::to_string(err_count++)));
        err_count += 1;
    }

    SVKFW_ADD_TEST(ParseSingleRule, "Parse single rule") {
        Parser::Nonterminal nonterm1 = 1u;
        Parser::Nonterminal nonterm2 = 2u;
        Parser::Nonterminal nonterm3 = 3u;
        Parser::ProdRule prod_rule1{{ nonterm1 }};
        Parser::ProdRule prod_rule2{{ "u" }};
        Parser::ProdRule prod_rule3{{ nonterm1, "u" }};
        Parser::ProdRule prod_rule4{{ "ab", nonterm1, "u", nonterm2, nonterm3 }};

        // State
        Parser::RuleState __state{ 0u, Parser::NONTERM_START };

        // Parse rule 1
        // printf("\nParse rule 1\n");
        __state = Parser::RuleState{ 0u, Parser::NONTERM_START };
        parseNext   (prod_rule1, __state, Parser::NEXT_NONTERMINAL);
        parseNonterm(prod_rule1, __state, nonterm1);
        parseNext   (prod_rule1, __state, Parser::NEXT_END);

        // Parse rule 2
        // printf("\nParse rule 2\n");
        __state = Parser::RuleState{ 0u, Parser::NONTERM_START };
        parseNext   (prod_rule2, __state, Parser::NEXT_TERMINAL);
        parseTerm   (prod_rule2, __state, 'u');
        // printf("State: %d-%d\n", __state.nonterm, __state.substr_id);
        parseNext   (prod_rule2, __state, Parser::NEXT_END);

        // Parse rule 3
        // printf("\nParse rule 3\n");
        __state = Parser::RuleState{ 0u, Parser::NONTERM_START };
        parseNext   (prod_rule3, __state, Parser::NEXT_NONTERMINAL);
        parseNonterm(prod_rule3, __state, nonterm1);
        parseNext   (prod_rule3, __state, Parser::NEXT_TERMINAL);
        parseTerm   (prod_rule3, __state, 'u');
        parseNext   (prod_rule3, __state, Parser::NEXT_END);

        // Parse rule 4
        // printf("\nParse rule 4\n");
        __state = Parser::RuleState{ 0u, Parser::NONTERM_START };
        parseNext   (prod_rule4, __state, Parser::NEXT_TERMINAL);
        parseTerm   (prod_rule4, __state, 'a');
        parseNext   (prod_rule4, __state, Parser::NEXT_TERMINAL);
        parseTerm   (prod_rule4, __state, 'b');
        parseNext   (prod_rule4, __state, Parser::NEXT_NONTERMINAL);
        parseNonterm(prod_rule4, __state, nonterm1);
        parseNext   (prod_rule4, __state, Parser::NEXT_TERMINAL);
        parseTerm   (prod_rule4, __state, 'u');
        parseNext   (prod_rule4, __state, Parser::NEXT_NONTERMINAL);
        parseNonterm(prod_rule4, __state, nonterm2);
        parseNext   (prod_rule4, __state, Parser::NEXT_NONTERMINAL);
        parseNonterm(prod_rule4, __state, nonterm3);
        parseNext   (prod_rule4, __state, Parser::NEXT_END);

        // printf("Success\n");
    }


    SVKFW_ADD_TEST(ParseInt, "Parse integer") {
        const char *parsed_str1 = "+98345";
        const char *parsed_str2 = "197645921446";
        const char *parsed_str3 = "-12";
        const char *parsed_str4 = "12.34";
        const char *parsed_str5 = "";

        // Parse strings
        bool __res_parse = false;
        std::vector<std::string> __res_strings;
        Parser::RuleGrammar __int_grammar{ {}, { Parser::Elements::e_int } };
        Parser::Parser __int_parser{};

        __res_parse   = __int_parser.parse(parsed_str1, __int_grammar);
        __res_strings = __int_parser.getElements(parsed_str1, Parser::NONTERM_START);
        Test::testAssert(__res_parse, "Integer 1: '" + std::string(parsed_str1) + "' parse error");
        Test::testAssert(__res_strings.size() == 1, "Integer 1: parsed rule element info size mismatch - " + std::to_string(__res_strings.size()) + " != 1");
        if (__res_strings.size() == 1)
            Test::testAssert(__res_strings[0] == parsed_str1, "Integer 1: retrieved integer error: '" + __res_strings[0] + "' != '" + std::string(parsed_str1) + "'");

        __res_parse   = __int_parser.parse(parsed_str2, __int_grammar);
        __res_strings = __int_parser.getElements(parsed_str2, Parser::NONTERM_START);
        Test::testAssert(__res_parse, "Integer 2: '" + std::string(parsed_str2) + "' parse error");
        Test::testAssert(__res_strings.size() == 1, "Integer 2: parsed rule element info size mismatch - " + std::to_string(__res_strings.size()) + " != 1");
        if (__res_strings.size() == 1)
            Test::testAssert(__res_strings[0] == parsed_str2, "Integer 2: retrieved integer error: '" + __res_strings[0] + "' != '" + std::string(parsed_str2) + "'");

        __res_parse   = __int_parser.parse(parsed_str3, __int_grammar);
        __res_strings = __int_parser.getElements(parsed_str3, Parser::NONTERM_START);
        Test::testAssert(__res_parse, "Integer 3: '" + std::string(parsed_str3) + "' parse error");
        Test::testAssert(__res_strings.size() == 1, "Integer 3: parsed rule element info size mismatch - " + std::to_string(__res_strings.size()) + " != 1");
        if (__res_strings.size() == 1)
            Test::testAssert(__res_strings[0] == parsed_str3, "Integer 3: retrieved integer error: '" + __res_strings[0] + "' != '" + std::string(parsed_str3) + "'");

        __res_parse   = __int_parser.parse(parsed_str4, __int_grammar);
        Test::testAssert(!__res_parse, "Integer 4: '" + std::string(parsed_str4) + "' parse error");
        __res_parse   = __int_parser.parse(parsed_str5, __int_grammar);
        Test::testAssert(!__res_parse, "Integer 5: '" + std::string(parsed_str5) + "' parse error");
    }


    SVKFW_ADD_TEST(ParseJSON, "JSON Parsing") {
        // Example from Wikipedia/JSON
        std::string json_example = "{\n  \"first_name\" : \"John\",\n  \"last_name\" : \"Smith\",\n  \"is_alive\" : true,\n  \"age\" : 27,\n  \"address\" : {\n    \"street_address\" : \"21 2nd Street\",\n    \"city\" : \"New York\",\n    \"state\" : \"NY\",\n    \"postal_code\" : \"10021-3100\"\n  },\n  \"phone_numbers\" : [\n    {\n      \"type\" : \"home\",\n      \"number\" : \"212 555-1234\"\n    },\n    {\n      \"type\" : \"office\",\n      \"number\" : \"646 555-4567\"\n    }\n  ],\n  \"children\" : [\n    \"Catherine\",\n    \"Thomas\",\n    \"Trevor\"\n  ],\n  \"spouse\" : null\n}\n";

        Simple::File::ContentJSON json_parsed;
        json_parsed.readFromString(json_example.c_str());
        std::string res_json = json_parsed.writeToString(2);

        Test::testAssert(json_example == res_json, "writeToString result doesn't match reference");
    }



#undef  SVKFW_TEST_GROUP
#define SVKFW_TEST_GROUP "Terminal"


    SVKFW_ADD_TEST(TerminalFormatting, "Terminal style state setters/getters") {
        TermCtrl::TerminalHandler terminal_handler{};
        uint32_t __tstyle_color_green = terminal_handler.addState(TermCtrl::TextStyle{{TermCtrl::TFlag::FG_RGB8}, TermCtrl::TColor::BrightGreen});
        uint32_t __tstyle_color_red   = terminal_handler.addState(TermCtrl::TextStyle{{TermCtrl::TFlag::FG_RGB8}, TermCtrl::TColor::BrightRed});

        Test::testAssert(terminal_handler.style.states[__tstyle_color_red].get<TermCtrl::RGBModeFG>() == TermCtrl::RGBModeFG::RGB8, "Red color state error - doesn't show color change");
    }

#undef  SVKFW_TEST_GROUP
#define SVKFW_TEST_GROUP "ETCS"


    SVKFW_ADD_TEST(ETCSEntities, "Entities, tags, components, generations") {
        struct Health { int hp; };
        struct Name   { std::string name; };
        ComponentStorage ecs;
        EntityID a = ecs.create(tagMask(TagID::TAG_PHYSICS_DYNAMIC));
        EntityID b = ecs.create();
        ecs.add<Health>(a, Health{10});
        ecs.add<Health>(b, Health{20});
        ecs.add<Name>(b, Name{"b"});
        Test::testAssert(ecs.count() == 2u && ecs.get<Health>(a)->hp == 10 && ecs.has<Name>(b) && !ecs.has<Name>(a), "components");
        Test::testAssert(ecs.hasTag(a, TagID::TAG_PHYSICS_DYNAMIC) && !ecs.hasTag(b, TagID::TAG_PHYSICS_DYNAMIC), "tags");

        int __sum = 0, __calls = 0;
        ecs.each<Health>([&](EntityID, Health &_h) { __sum += _h.hp; ++__calls; });
        Test::testAssert(__sum == 30 && __calls == 2, "each<Health>");
        __calls = 0;
        ecs.each<Name, Health>([&](EntityID _e, Name &_n, Health &_h) { Test::testAssert(_e == b && _n.name == "b" && _h.hp == 20, "each<Name, Health> data"); ++__calls; });
        Test::testAssert(__calls == 1, "each<Name, Health> count");
        __calls = 0;
        ecs.each<Health>([&](EntityID, Health &) { ++__calls; }, TagFilter().none(TagID::TAG_PHYSICS_DYNAMIC));
        Test::testAssert(__calls == 1, "each with a tag filter");

        ecs.destroy(a);
        Test::testAssert(!ecs.isValid(a) && ecs.get<Health>(a) == nullptr && ecs.gather().size() == 1u, "destroy");
        EntityID c = ecs.create();
        Test::testAssert(c.index == a.index && c.generation != a.generation && !ecs.has<Health>(c) && !ecs.isValid(a), "slot reuse with a new generation");
        // The swap-remove kept b's data
        Test::testAssert(ecs.get<Health>(b)->hp == 20, "data after removing another entity");
        Test::testAssert(ecs.remove<Health>(b) && !ecs.has<Health>(b) && ecs.has<Name>(b), "remove one component");
        ecs.clear();
        Test::testAssert(ecs.count() == 0u && !ecs.isValid(b), "clear");
    }


#undef  SVKFW_TEST_GROUP
#define SVKFW_TEST_GROUP "Events"


    SVKFW_ADD_TEST(EventQueue, "Event queue, payload, subscriptions") {
        struct Payload { int a; float b; };
        Event::EventHandler events;
        std::vector<int> __got;
        const uint32_t EV_A = Event::EV_TYPE_USER, EV_B = Event::EV_TYPE_USER + 1u;
        Event::SubscriptionID __sa = events.subscribe(EV_A, [&](Event::EventHandler &_h, const Event::Event &_e) {
            __got.push_back(_e.get<Payload>().a);
            _h.write(EV_B); // written during dispatching: handled in the next round
        });
        events.subscribe(EV_B, [&](Event::EventHandler &, const Event::Event &) { __got.push_back(-1); }, 1u); // once
        events.write(Event::Event::make(EV_A, Payload{7, 1.f}));
        events.write(Event::Event::make(EV_A, Payload{8, 2.f}));
        // Writes from another thread
        std::thread __t([&]() { for (int i = 0; i < 100; ++i) events.write(Event::EV_TYPE_USER + 5u); });
        __t.join();
        const uint32_t __n = events.poll();
        Test::testAssert(__got.size() == 3u && __got[0] == 7 && __got[1] == 8 && __got[2] == -1, "dispatch order and one-shot subscription");
        Test::testAssert(__n == 104u, "dispatched count " + std::to_string(__n));
        Test::testAssert(events.unsubscribe(__sa) && !events.unsubscribe(__sa), "unsubscribe");
        events.write(Event::Event::make(EV_A, Payload{9, 0.f}));
        events.poll();
        Test::testAssert(__got.size() == 3u, "no calls after unsubscribe");
        events.stopWriting();
        Test::testAssert(!events.write(EV_A), "stopped writing");
    }


#undef  SVKFW_TEST_GROUP
#define SVKFW_TEST_GROUP "Physics"


    SVKFW_ADD_TEST(PhysicsShapes, "Physics: resting shapes, stacking, Euler angles") {
        using namespace Physics;
        const vec3 __angles{0.3f, -0.7f, 1.1f}, __back = eulerXYZ(fromEulerXYZ(__angles));
        Test::testAssert((__back - __angles).norm() < 1e-4f, "Euler XYZ round trip");

        ComponentStorage ecs;
        World world(ecs);
        world.settings.gravity = {0.f, -9.8f, 0.f};
        world.createBody(Body(), Collider::plane(), TagID::TAG_PHYSICS_STATIC);
        EntityID sphere  = world.createBody(Body::at({0.f, 3.f, 0.f}), Collider::sphere(0.5f), TagID::TAG_PHYSICS_DYNAMIC);
        EntityID capsule = world.createBody(Body::at({-5.f, 2.f, 0.f}, Quat(vec3{0.f, 0.f, 1.f}, 1.2f)), Collider::capsule(0.3f, 2.f), TagID::TAG_PHYSICS_DYNAMIC);
        EntityID boxes[3];
        for (int i = 0; i < 3; ++i)
            boxes[i] = world.createBody(Body::at({5.f + 0.05f * i, 0.5f + 1.01f * i, 0.f}, Quat(vec3{0.f, 1.f, 0.f}, 0.3f * i)),
                                        Collider::box({0.5f, 0.5f, 0.5f}), TagID::TAG_PHYSICS_DYNAMIC);
        for (int f = 0; f < 300; ++f) world.update(1.f / 60.f);
        Test::testAssert(std::fabs(ecs.get<Body>(sphere)->position.y - 0.5f) < 0.01f, "sphere rests on the ground");
        Test::testAssert(std::fabs(ecs.get<Body>(capsule)->position.y - 0.3f) < 0.01f, "capsule lies on the ground");
        for (int i = 0; i < 3; ++i) {
            const vec3 p = ecs.get<Body>(boxes[i])->position;
            Test::testAssert(std::fabs(p.y - (0.5f + i)) < 0.02f && std::fabs(p.x - 5.f) < 0.3f, "box stack, box " + std::to_string(i) +
                             " at " + std::to_string(p.x) + " " + std::to_string(p.y));
        }
    }

    SVKFW_ADD_TEST(PhysicsJoints, "Physics: joints, kinematic bodies, contact events") {
        using namespace Physics;
        ComponentStorage ecs;
        Event::EventHandler events;
        World world(ecs);
        world.events = &events;
        world.settings.gravity = {0.f, -9.8f, 0.f};
        // A pendulum: a locked point joint at the kinematic pivot
        EntityID pivot = world.createBody(Body::at({0.f, 10.f, 0.f}), Collider::sphere(0.1f), TagID::TAG_PHYSICS_KINEMATIC);
        EntityID bob   = world.createBody(Body::at({1.f, 10.f, 0.f}), Collider::box({1.f, 0.1f, 0.1f}), TagID::TAG_PHYSICS_DYNAMIC); // a rod
        Joint j;
        j.body_a = pivot; j.body_b = bob;
        j.anchor_b = {-1.f, 0.f, 0.f};
        j.angular_min = {1.f, 1.f, 1.f}; j.angular_max = {-1.f, -1.f, -1.f}; // free rotation
        world.createJoint(j);
        float __max_err = 0.f, __min_y = 10.f;
        for (int f = 0; f < 240; ++f) {
            world.update(1.f / 60.f);
            const Body &__b = *ecs.get<Body>(bob);
            const vec3 p = __b.position;
            // The joint point of the bob stays at the pivot
            __max_err = std::max(__max_err, (p + __b.rotation.rotateByUnit(j.anchor_b) - vec3{0.f, 10.f, 0.f}).norm());
            __min_y = std::min(__min_y, p.y);
        }
        Test::testAssert(__max_err < 0.02f, "pendulum joint kept, error " + std::to_string(__max_err));
        Test::testAssert(__min_y < 9.1f, "pendulum swings down, lowest y " + std::to_string(__min_y));

        // A kinematic box moving into a resting sphere pushes it; contact events
        world.createBody(Body(), Collider::plane(), TagID::TAG_PHYSICS_STATIC);
        EntityID pusher = world.createBody(Body::at({-3.f, 0.5f, 5.f}), Collider::box({0.5f, 0.5f, 0.5f}), TagID::TAG_PHYSICS_KINEMATIC);
        EntityID ball   = world.createBody(Body::at({0.f, 0.5f, 5.f}), Collider::sphere(0.5f), TagID::TAG_PHYSICS_DYNAMIC);
        ecs.addTag(ball, TagID::TAG_PHYSICS_REPORT_CONTACTS);
        uint32_t __begins = 0u;
        events.subscribe(Event::EV_TYPE_PHYSICS_CONTACT_BEGIN, [&](Event::EventHandler &, const Event::Event &_e) {
            const ContactEvent c = _e.get<ContactEvent>();
            if (c.body_a == pusher || c.body_b == pusher) ++__begins;
        });
        for (int f = 0; f < 60; ++f) {
            world.setKinematicTarget(pusher, {-3.f + 0.05f * (f + 1), 0.5f, 5.f}, Quat());
            world.update(1.f / 60.f);
            events.poll();
        }
        Test::testAssert(ecs.get<Body>(ball)->position.x > 0.5f, "the kinematic box pushed the ball");
        Test::testAssert(__begins >= 1u, "contact begin event");
    }

    SVKFW_ADD_TEST(PhysicsMMD, "Physics: MMD bodies drive bones") {
        using namespace SceneGraph;
        // A root bone with a kinematic body and a 3-link hair chain (dynamic capsules along +X in the bind pose)
        Scene scene;
        Skeleton sk;
        Bone root; root.name = "root"; root.rest.translation = {0.f, 10.f, 0.f};
        sk.addBone(root);
        for (uint32_t i = 1u; i <= 3u; ++i) {
            Bone b; b.name = "hair" + std::to_string(i); b.parent = i - 1u; b.rest.translation = {1.f, 0.f, 0.f};
            sk.addBone(b);
        }
        RigidBodyDesc rb;
        rb.bone = 0u; rb.shape = SCENE_RIGID_SPHERE; rb.size = {0.5f, 0.f, 0.f}; rb.position = {0.f, 10.f, 0.f};
        rb.group = 0u; rb.collision_mask = 0xFFFEu; rb.mode = SCENE_RIGID_FOLLOW_BONE;
        sk.rigid_bodies.push_back(rb);
        for (uint32_t i = 1u; i <= 3u; ++i) {
            RigidBodyDesc h;
            h.bone = i; h.shape = SCENE_RIGID_CAPSULE; h.size = {0.2f, 0.6f, 0.f};
            h.position = {i - 0.5f, 10.f, 0.f}; h.rotation = Quat(vec3{0.f, 0.f, 1.f}, 1.5707963f);
            h.group = 1u; h.collision_mask = 0xFFFCu; h.mode = SCENE_RIGID_PHYSICS; h.linear_damping = h.angular_damping = 0.5f;
            sk.rigid_bodies.push_back(h);
            JointDesc jd;
            jd.body_a = i - 1u; jd.body_b = i; jd.position = {i - 1.f, 10.f, 0.f};
            jd.angular_min = {-1.f, -1.f, -1.f}; jd.angular_max = {1.f, 1.f, 1.f};
            sk.joints.push_back(jd);
        }
        sk.finalize();
        const uint32_t node = scene.addNode("model");
        const uint32_t skin = scene.addSkin(scene.addSkeleton(sk), node);
        scene.nodes[node].skin = skin;

        // Through the engine: the model comes with the scene event, the phases run the systems
        Core::Engine engine;
        Physics::World world(engine.ecs);
        Physics::SceneBodies bodies(scene, world);
        engine.addSystem(&world);
        engine.addSystem(&bodies);
        Event::SceneModelEvent added;
        added.root = node; added.skin = skin;
        engine.events.dispatch(Event::Event::make(Event::EV_TYPE_SCENE_MODEL_ADDED, added));
        Test::testAssert(bodies.modelCount() == 1u && world.ecs.count() == 8u, "the model entity, 4 bodies and 3 joints");
        Core::FrameTime time;
        time.dt = time.sim_dt = 1.f / 60.f;
        for (int f = 0; f < 180; ++f) {
            engine.runPhase(Core::PHASE_ANIMATION, time);
            engine.runPhase(Core::PHASE_PHYSICS, time);
            engine.runPhase(Core::PHASE_SCENE, time);
        }
        const Pose &pose = scene.skins[skin].pose;
        const vec3 tip = { pose.model[3].M[3][0], pose.model[3].M[3][1], pose.model[3].M[3][2] };
        Test::testAssert(tip.y < 8.f, "the hair chain falls under gravity, tip y " + std::to_string(tip.y));
        // The bone chain stays connected: bone 1 at 1 unit from the root
        const vec3 b1 = { pose.model[1].M[3][0], pose.model[1].M[3][1], pose.model[1].M[3][2] };
        Test::testAssert(std::fabs((b1 - vec3{0.f, 10.f, 0.f}).norm() - 1.f) < 0.05f, "hair1 stays at the root, distance " +
                         std::to_string((b1 - vec3{0.f, 10.f, 0.f}).norm()));
        // Moving the model node moves the kinematic body with it
        scene.setLocal(node, translation({5.f, 0.f, 0.f}));
        for (int f = 0; f < 60; ++f) bodies.simulate(1.f / 60.f);
        const vec3 b1m = { pose.model[1].M[3][0], pose.model[1].M[3][1], pose.model[1].M[3][2] };
        Test::testAssert(std::fabs((b1m - vec3{0.f, 10.f, 0.f}).norm() - 1.f) < 0.05f, "bones stay in the skeleton space after moving the model");
    }

    SVKFW_ADD_TEST(PhysicsSceneNodes, "Physics: colliders of nodes from mesh bounds") {
        using namespace SceneGraph;
        // A model without a skeleton: a node with a 2x2x2 box mesh at y = 10, scaled by 0.5
        Scene scene;
        Mesh mesh;
        mesh.bounds.add(vec3{-1.f, -1.f, -1.f});
        mesh.bounds.add(vec3{ 1.f,  1.f,  1.f});
        scene.meshes.push_back(mesh);
        const uint32_t root = scene.addNode("model", SCENE_NONE, scaling({0.5f, 0.5f, 0.5f}) & translation({0.f, 10.f, 0.f}));
        const uint32_t part = scene.addNode("part", root);
        scene.nodes[part].meshes.push_back(0u);
        scene.updateTransforms();

        ComponentStorage ecs;
        Physics::World world(ecs);
        world.settings.gravity = {0.f, -10.f, 0.f};
        Physics::SceneBodies bodies(scene, world);
        Test::testAssert(bodies.addModel(root, SCENE_NONE).isNone(), "no bodies without 'auto_colliders'");
        bodies.auto_colliders = true;
        bodies.auto_fit = Physics::FIT_BOX;
        bodies.auto_motion = TagID::TAG_PHYSICS_DYNAMIC;
        const EntityID model = bodies.addModel(root, SCENE_NONE);
        Test::testAssert(!model.isNone() && ecs.get<Physics::PhysicsModel>(model)->bodies.size() == 1u, "one body for the node with a mesh");
        const EntityID body = ecs.get<Physics::PhysicsModel>(model)->bodies[0];
        const Physics::Collider *col = ecs.get<Physics::Collider>(body);
        Test::testAssert(col && std::fabs(col->size.x - 0.5f) < 1e-4f, "box half extents scaled by the node, " + std::to_string(col ? col->size.x : 0.f));
        world.createBody(Physics::Body(), Physics::Collider::plane(), TagID::TAG_PHYSICS_STATIC);
        for (int f = 0; f < 240; ++f) bodies.simulate(1.f / 60.f);
        const vec3 p = transformPoint(scene.world[part], vec3{0.f, 0.f, 0.f});
        Test::testAssert(std::fabs(p.y - 0.5f) < 0.1f, "the node fell onto the ground, y " + std::to_string(p.y));
        Test::testAssert(std::fabs(scene.world[part].M[0][0] * scene.world[part].M[0][0] + scene.world[part].M[0][1] * scene.world[part].M[0][1] +
                                   scene.world[part].M[0][2] * scene.world[part].M[0][2] - 0.25f) < 1e-3f, "the node keeps its scale");
        bodies.clear();
        Test::testAssert(!ecs.isValid(body) && !ecs.isValid(model) && bodies.modelCount() == 0u, "'clear' removes the model and its bodies");
    }

#undef  SVKFW_TEST_GROUP
#define SVKFW_TEST_GROUP "Engine"

    SVKFW_ADD_TEST(EngineApps, "Engine: apps by events, systems by phases") {
        struct CountingApp : Core::AppItf {
            Core::Engine *engine = nullptr;
            int frames = 0, started = 0, stopped = 0, *runs;
            explicit CountingApp(int *_runs) : runs(_runs) {}
            const char *name() const override { return "counting"; }
            bool start(Core::Engine &_engine) override { engine = &_engine; ++started; ++*runs; return true; }
            bool frame() override {
                // Another app's event while this one runs: ignored
                if (++frames == 2) engine->events.write(Event::EV_TYPE_RUN_GAME);
                return frames < 5;
            }
            void stop() override { ++stopped; }
        };
        struct PhaseSystem : Core::System {
            std::vector<uint32_t> seen;
            const char *name() const override { return "phases"; }
            uint32_t phases() const override { return Core::phaseBit(Core::PHASE_PHYSICS) | Core::phaseBit(Core::PHASE_SCENE); }
            void update(Core::Phase _phase, const Core::FrameTime &) override { seen.push_back((uint32_t)_phase); }
        };
        int runs_editor = 0, runs_game = 0;
        Core::Engine engine;
        engine.registerApp(Event::EV_TYPE_RUN_EDITOR, [&]() -> Core::AppItf* { return new CountingApp(&runs_editor); });
        engine.registerApp(Event::EV_TYPE_RUN_GAME,   [&]() -> Core::AppItf* { return new CountingApp(&runs_game);   });
        Test::testAssert(engine.run() != 0, "nothing to run without an event");
        engine.events.write(Event::EV_TYPE_RUN_EDITOR);
        Test::testAssert(engine.run() == 0 && engine.frame() == 5u, "the app ran until it quit, frames " + std::to_string(engine.frame()));
        Test::testAssert(runs_editor == 1 && runs_game == 0, "the game event was ignored while the editor ran");

        engine.max_frames = 3u;
        engine.events.write(Event::EV_TYPE_RUN_GAME);
        Test::testAssert(engine.run() == 0 && engine.frame() == 3u && runs_game == 1, "'max_frames' stops the app");

        PhaseSystem sys;
        engine.addSystem(&sys);
        Core::FrameTime t;
        for (uint32_t p = 0u; p < Core::PHASE_COUNT; ++p) engine.runPhase((Core::Phase)p, t);
        Test::testAssert(sys.seen.size() == 2u && sys.seen[0] == Core::PHASE_PHYSICS && sys.seen[1] == Core::PHASE_SCENE, "systems run in their phases");
        Test::testAssert(engine.removeSystem(&sys) && !engine.removeSystem(&sys), "removeSystem");
    }
}; // Simple END


int main(int argc, char **argv) {
    // Arguments: names of tests and test groups as follows:
    //            --tests TestName1 [, TestName2, ...] --test_groups TestGroupName1 [, TestGroupName2, ...]
    // Note: If no tests specified, run all

    Simple::ArgParser tests_argparser{argc, argv};
    tests_argparser.addArgument("list",        "l",  {  }); // empty counts list  -> it is a flag, no values expected
    tests_argparser.addArgument("tests",       "t",  {0u}); // counts list with 0 -> expected a vector with >=0 parameters
    tests_argparser.addArgument("test_groups", "tg", {0u}); // counts list with 0 -> expected a vector with >=0 parameters
    tests_argparser.parseAllArguments();

    if (tests_argparser.hasArgumentParsed("list")) {
        Simple::Test::test_system.printTestList();
    }
    else {
        std::vector<std::string> called_tests, called_test_groups;
        std::vector<uint32_t> parsed_tests       = tests_argparser.getArgParseIdsByName("tests"      );
        std::vector<uint32_t> parsed_test_groups = tests_argparser.getArgParseIdsByName("test_groups");

        if (!parsed_tests.empty())
            called_tests       = tests_argparser.argument_values[parsed_tests[0]      ].second[0];
        if (!parsed_test_groups.empty())
            called_test_groups = tests_argparser.argument_values[parsed_test_groups[0]].second[0];

        Simple::Test::test_system.run(called_tests, called_test_groups);
    }
    return 0;
}