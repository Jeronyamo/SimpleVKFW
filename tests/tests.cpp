#include <vector>
#include <iostream>
#include <thread>
#include <future>
#include <chrono>

#include "tests.h"

// What to test
#include "math/vectors.h"
#include "main/image.h"
#include "sound/sound.h"
#include "interface/rtaudio_wrap.h"



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


    void genDiffImage(const std::string &_img_path1, const std::string &_img_path2, const std::string &_img_path_res) {
        Img::Image3f image1(_img_path1);
        Img::Image3f image2(_img_path2);

        if (image1.width != image2.width || image1.height != image2.height)
            throw std::runtime_error(SVKFW_WRAPERR("Test2 :: genDiffImage", "image sizes don't match"));

        Img::Image3f __res{image1.width, image1.height};
        for (uint32_t i = 0u; i < __res.img.size(); ++i) {
            float __diff = image1.img[i].x - image2.img[i].x;
            vec3f __other_color = __diff < 0.f ? vec3f{1.f,0.f,0.f} : vec3f{0.f,0.f,1.f};
            __res.img[i] = __other_color * std::abs(__diff) + vec3f{1.f} * (1.f - std::abs(__diff));
        }

        __res.imgSave<Img::PNG>(_img_path_res);
    }

    SVKFW_ADD_TEST(ImageDiff, "Save/Load img differences") {
        std::string __img_dir = "/Programming/SimpleVKFW/tests/diff/";

        genDiffImage(__img_dir + "armadillo_ref.png", __img_dir + "armadillo_res.png", __img_dir + "arma_diff.png");
        genDiffImage(__img_dir + "bunny_ref.png"    , __img_dir + "bunny_res.png"    , __img_dir + "bunny_diff.png");
        genDiffImage(__img_dir + "dragon_ref.png"   , __img_dir + "dragon_res.png"   , __img_dir + "dragon_diff.png");
        genDiffImage(__img_dir + "happy_ref.png"    , __img_dir + "happy_res.png"    , __img_dir + "happy_diff.png");
        genDiffImage(__img_dir + "nefertiti_ref.png", __img_dir + "nefertiti_res.png", __img_dir + "nefertiti_diff.png");
        genDiffImage(__img_dir + "teapot_ref.png"   , __img_dir + "teapot_res.png"   , __img_dir + "teapot_diff.png");
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
        audio_o.deviceUpdateAll();
        audio_o.streamSetOptions(0, 0, "Test Audio Output/Input", 0);
        audio_o.deviceOutputSetParameters(__channels);
        audio_o.deviceInputSetParameters(__channels);
        audio_o.callbackSet(RTA::rtacb_inout);

        audio_o.streamOpen(0, __buf_frames, RTAUDIO_SINT16, false);
        Test::testAssert(audio_o.streamIsOpen(), "Audio stream is not open");

        audio_o.streamStart();

        AsyncExit __exit_state;
        while (!__exit_state.gotAnswer()) { audio_o.deviceUpdateAll(); }
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
        audio_i.deviceUpdateAll();
        audio_o.deviceUpdateAll();
        audio_i.streamSetOptions(0, 0, "Test Audio Input", 0);
        audio_o.streamSetOptions(0, 0, "Test Audio Output", 0);
        audio_o.deviceOutputSetParameters(__channels);
        audio_i.deviceInputSetParameters(__channels);
        // TODO:
        // audio_i.callbackSet(RTA::rtacb_record);
        // audio_o.callbackSet(RTA::rtacb_playback);

        audio_i.streamOpen(48000, __buf_frames, RTAUDIO_FLOAT32, false);
        audio_o.streamOpen(48000, __buf_frames, RTAUDIO_FLOAT32, false);
        Test::testAssert(audio_i.streamIsOpen(), "Audio stream i is not open");
        Test::testAssert(audio_o.streamIsOpen(), "Audio stream o is not open");

        audio_o.streamStart();
        audio_i.streamStart();

        AsyncExit __exit_state;
        while (!__exit_state.gotAnswer()) { audio_i.deviceUpdateAll(); audio_o.deviceUpdateAll(); }
    }


    void audioPlayer(std::vector<Sound::Sample::SmpItf*> _sources, const std::vector<float> &_weights, bool _set_srate_to_sources) {
        RTA::AudioHandler audio_o;
        const uint32_t __channels   =   2;
        const uint32_t __buf_frames = 544;

        audio_o.streamSetMode(RTA::STREAM_MODE_OUT,
                                RTA::DEVICE_MODE_DEFAULT);
        audio_o.deviceUpdateAll();
        audio_o.streamSetOptions(0, 0, "Test Audio Output", 0);
        audio_o.deviceOutputSetParameters(__channels);
        audio_o.callbackSet(RTA::rtacbDefault);

        audio_o.streamOpen(0, __buf_frames, RTAUDIO_SINT16, false);
        Test::testAssert(audio_o.streamIsOpen(), "Audio stream is not open");

        // Set audio to play
        for (uint32_t i = 0u; i < _sources.size(); ++i) {
            if (_set_srate_to_sources) _sources[i]->setSRate(audio_o.stream_config.sample_rate);
            audio_o.playAudio(_sources[i], _weights[i]);
        }

        // Start stream
        audio_o.streamStart();

        AsyncExit __exit_state;
        while (!__exit_state.gotAnswer()) { audio_o.deviceUpdateAll(); }
        audio_o.streamStop();
    }

    SVKFW_ADD_TEST(AudioFuncSampler, "Audio function sampler") {
        static Sound::Sample::SmpSineWave __sine_wave1{440.f          };
        static Sound::Sample::SmpSineWave __sine_wave2{440.f * (2.f/3)};

        __sine_wave1.t_step = 1.f / 48000; // yet another evidence of SmpFunction's impracticality
        __sine_wave2.t_step = 1.f / 48000;
        static auto __wave_sampler = [&]() { return 0.45f * __sine_wave1.sample() + 0.45f * __sine_wave2.sample(); };
        static Sound::Sample::SmpFunction __sampler{__wave_sampler};

        audioPlayer({&__sampler}, {1.f}, true);
    }


    SVKFW_ADD_TEST(AudioMixerSampler, "Audio - mixer sampler") {
        static Sound::Sample::SmpSineWave __sine_wave1{440.f          };
        static Sound::Sample::SmpSineWave __sine_wave2{440.f * (2.f/3)};

        audioPlayer({&__sine_wave1, &__sine_wave2}, {0.45f, 0.45f}, false);
    }


    SVKFW_ADD_TEST(AudioSawSampler, "Audio - saw sampler") {
        static Sound::Sample::SmpSaw __saw_sampler{440.f};
        audioPlayer({&__saw_sampler}, {0.9f}, false);
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
}; // Simple END


int main(int argc, char **argv) {
    // Arguments: names of tests and test groups as follows:
    //            --tests TestName1 [, TestName2, ...] --test_groups TestGroupName1 [, TestGroupName2, ...]
    // Note: If no tests specified, run all

    printf("out-1\n");
    fflush(stdout);
    Simple::ArgParser tests_argparser{argc, argv};
    printf("out0\n");
    fflush(stdout);
    tests_argparser.addArgument("tests",       "t",  {0u});
    tests_argparser.addArgument("test_groups", "tg", {0u});
    printf("out1\n");
    fflush(stdout);
    tests_argparser.parseAllArguments();

    std::vector<std::string> called_tests, called_test_groups;
    std::vector<uint32_t> parsed_tests       = tests_argparser.getArgParseIdsByName("tests"      );
    std::vector<uint32_t> parsed_test_groups = tests_argparser.getArgParseIdsByName("test_groups");

    if (!parsed_tests.empty())
        called_tests       = tests_argparser.argument_values[parsed_tests[0]      ].second[0];
    if (!parsed_test_groups.empty())
        called_test_groups = tests_argparser.argument_values[parsed_test_groups[0]].second[0];

    Simple::Test::test_system.run(called_tests, called_test_groups);
    return 0;
}