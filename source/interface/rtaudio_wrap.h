#ifndef SVKFW_RTAUDIO_WRAP_H
#define SVKFW_RTAUDIO_WRAP_H

#include "RtAudio.h"


namespace Simple {
    namespace RTA {
    // Device types available, based on number of channels
        enum DeviceType {
            DT_BIT_ONLY_IN  = 1 << 0,
            DT_BIT_ONLY_OUT = 1 << 1,
            DT_BIT_DUPLEX   = 1 << 2,
            DT_BIT_BOTH_IO  = 1 << 3, // TODO: Is it possible that the same device has input and output channels and no duplex?
        }; // DeviceType END

        enum DeviceMode {
            DEVICE_MODE_UNSPEC  , // Initial mode, replaced with DEFAULT in/out mode if this device is used to open a stream, ignored otherwise
            DEVICE_MODE_DEFAULT , // Checks for default input/output device
            DEVICE_MODE_SELECTED, // Uses selected input/output device while it's available, else selects default in/out devices
            DEVICE_MODE_DUPLEX  , // Uses device with duplex channels while it's available, else falls back to DEFAULT in/out modes
        }; // DeviceMode END

        enum StreamMode {
            STREAM_MODE_UNSPEC, // Not setting stream mode is an error
            STREAM_MODE_OUT   , // Output stream
            STREAM_MODE_IN    , // Input stream
            STREAM_MODE_DUPLEX, // Input+output stream, no checks if it uses actual duplex channels of the same device. TODO: what checks to add?
        }; // StreamMode END

        struct RTAStreamConfig {
            StreamMode stream_mode = STREAM_MODE_UNSPEC;
            uint32_t sample_rate;
            uint32_t buffer_frames; // automatically set on stream opening
            RtAudioFormat audio_format;

            uint32_t getBufferElemBytes() const {
                switch (audio_format) {
                    case RTAUDIO_SINT8:   return 1;
                    case RTAUDIO_SINT16:  return 2;
                    case RTAUDIO_SINT24:  return 3;
                    case RTAUDIO_SINT32:  return 4;
                    case RTAUDIO_FLOAT32: return 4;
                    case RTAUDIO_FLOAT64: return 8;
                }
                return 0;
            }
        }; // RTAStreamConfig END

        struct AudioConfig {
            static const uint32_t MAX_VOLUME = 32u;
            uint8_t volume = 10; // final volume aquired from (getVolume)

            float getVolume() const { return volume/255.f*MAX_VOLUME; }
        }; // AudioConfig END

        struct AudioBuffer {
            float *buf = nullptr;
            uint32_t buf_size; // in elements
        } Global_audio_buf; // AudioBuffer END


        struct AudioHandler {
            RtAudio rta_handler;
            RtAudioCallback rta_cback = nullptr;
            RTAStreamConfig stream_config;
            AudioConfig audio_config;

            RtAudio::StreamOptions stream_options;
            struct DeviceHandler {
                RtAudio::StreamParameters stream_parameters; // Note: device id is only stored as 'stream_parameters.deviceId'
                DeviceMode device_mode = DEVICE_MODE_UNSPEC;
            } device_i, device_o; // devices: in, out; duplex mode: 'device_i'+'device_o' parameters are used


            AudioHandler() { sizeof(AudioHandler); }
            ~AudioHandler() {
                if (rta_handler.isStreamRunning())
                    rta_handler.abortStream();
                if (rta_handler.isStreamOpen())
                    rta_handler.closeStream();
            }


            void callbackSet(RtAudioCallback _rta_cback) { rta_cback = _rta_cback; }

        // Stream methods

            void streamSetMode(StreamMode _stream_mode, DeviceMode _o_device_mode = DEVICE_MODE_UNSPEC, DeviceMode _i_device_mode = DEVICE_MODE_UNSPEC) {
                stream_config.stream_mode = _stream_mode;

                if ((uint32_t(stream_config.stream_mode) & 1) && _o_device_mode == DEVICE_MODE_UNSPEC)
                    _o_device_mode = DEVICE_MODE_DEFAULT;
                if ((uint32_t(stream_config.stream_mode) & 2) && _i_device_mode == DEVICE_MODE_UNSPEC)
                    _i_device_mode = DEVICE_MODE_DEFAULT;

                device_i.device_mode = _i_device_mode;
                device_o.device_mode = _o_device_mode;
            }

            void streamSetOptions(RtAudioStreamFlags _s_flags, uint32_t _s_nbuffers, const std::string &_s_name = "SimpleVKFW_AudioHandler", int _s_priority = 0) {
                stream_options.flags           = _s_flags;
                stream_options.numberOfBuffers = _s_nbuffers;
                stream_options.streamName      = _s_name;
                stream_options.priority        = _s_priority;
            }

            void streamOpen(uint32_t _sample_rate, uint32_t _buf_frames, RtAudioFormat _audio_format, bool _use_options = true) {
                RtAudio::StreamParameters* __o_params = (uint32_t(stream_config.stream_mode) & 1) ? &device_o.stream_parameters : nullptr;
                RtAudio::StreamParameters* __i_params = (uint32_t(stream_config.stream_mode) & 2) ? &device_i.stream_parameters : nullptr;

                if (_sample_rate == 0) {
                    if (__o_params != nullptr) {
                        uint32_t __preferred_srate = rta_handler.getDeviceInfo(__o_params->deviceId).preferredSampleRate;
                        _sample_rate = __preferred_srate;
                    }
                    if (__o_params != nullptr) {
                        uint32_t __preferred_srate = rta_handler.getDeviceInfo(__o_params->deviceId).preferredSampleRate;
                        _sample_rate = std::max(_sample_rate, __preferred_srate);
                    }
                }
                stream_config.sample_rate = _sample_rate;
                stream_config.buffer_frames = _buf_frames;
                stream_config.audio_format = _audio_format;
                RtAudioErrorType  __res = rta_handler.openStream(__o_params, __i_params, _audio_format, _sample_rate, 
                                                                 &stream_config.buffer_frames, rta_cback, this, _use_options ? &stream_options : nullptr);

                printf("Stream open with parameters:\nBuffer frames: %d\n", stream_config.buffer_frames);
                if (__res != RTAUDIO_NO_ERROR) {
                    throw std::runtime_error(SVKFW_WRAPERR("RTA :: AudioHandler :: streamOpen", "Error code " + std::to_string(__res) + ";\n" + rta_handler.getErrorText()));
                }
            }

            void streamStart() { rta_handler.startStream(); }
            void streamStop()  { rta_handler. stopStream(); }
            void streamAbort() { rta_handler.abortStream(); }
            bool streamIsOpen()     { return rta_handler.isStreamOpen();     }
            bool streamIsRunning()  { return rta_handler.isStreamRunning();  }
            long streamGetLatency() { return rta_handler.getStreamLatency(); }
            void streamShowWarnings(bool _do_show = true) { return rta_handler.showWarnings(_do_show); }


        // Device/Input device/Output device methods

            DeviceType deviceGetAvailableTypes() {
                uint32_t __res = 0;

                RtAudio::DeviceInfo __device_info;
                for (uint32_t dev_id : rta_handler.getDeviceIds()) {
                    __device_info = rta_handler.getDeviceInfo(dev_id);
                    if (__device_info.inputChannels >  0 && __device_info.outputChannels == 0)
                        __res |= DT_BIT_ONLY_IN;
                    if (__device_info.inputChannels == 0 && __device_info.outputChannels >  0)
                        __res |= DT_BIT_ONLY_OUT;
                    if (__device_info.inputChannels >  0 && __device_info.outputChannels >  0)
                        __res |= DT_BIT_BOTH_IO;
                    if (__device_info.duplexChannels > 0)
                        __res |= DT_BIT_DUPLEX;
                }

                return (DeviceType) __res;
            }

            void deviceInputSetParameters(uint32_t _nchannels, uint32_t _first_channel = 0) {
                device_i.stream_parameters.firstChannel = _first_channel;
                device_i.stream_parameters.nChannels    = _nchannels;
            }
            void deviceOutputSetParameters(uint32_t _nchannels, uint32_t _first_channel = 0) {
                device_o.stream_parameters.firstChannel = _first_channel;
                device_o.stream_parameters.nChannels    = _nchannels;
            }

            bool deviceInputIsFormatSupported(RtAudioFormat _audio_format) {
                return rta_handler.getDeviceInfo(device_i.stream_parameters.deviceId).nativeFormats & _audio_format;
            }
            bool deviceOutputIsFormatSupported(RtAudioFormat _audio_format) {
                return rta_handler.getDeviceInfo(device_o.stream_parameters.deviceId).nativeFormats & _audio_format;
            }

            void deviceUpdate_(bool _is_input_device) {
                DeviceHandler &__device_io = _is_input_device ? device_i : device_o;
                const uint32_t __old_device_id = __device_io.stream_parameters.deviceId;

                switch (__device_io.device_mode) {
                    case DEVICE_MODE_SELECTED: {
                        RtAudio::DeviceInfo __device_info = rta_handler.getDeviceInfo(__device_io.stream_parameters.deviceId);
                        if (__device_info.ID == __device_io.stream_parameters.deviceId)
                            break;
                    }
                    case DEVICE_MODE_DEFAULT: {
                        __device_io.stream_parameters.deviceId = _is_input_device ? rta_handler.getDefaultInputDevice() :
                                                                                    rta_handler.getDefaultOutputDevice();
                        break;
                    }
                    case DEVICE_MODE_DUPLEX: { // can be used if other device is not duplex for some reason
                        RtAudio::DeviceInfo __device_info = rta_handler.getDeviceInfo(__device_io.stream_parameters.deviceId);
                        if (__device_info.ID != __device_io.stream_parameters.deviceId) {
                            uint32_t __inout_dev_id = 0; // use device that has both in and out channels, if such exists
                            for (uint32_t dev_id : rta_handler.getDeviceIds()) {
                                __device_info = rta_handler.getDeviceInfo(dev_id);
                                if (__device_info.duplexChannels > 0) {
                                    __device_io.stream_parameters.deviceId = dev_id;
                                    break;
                                }
                                if (__device_info.inputChannels > 0 && __device_info.outputChannels > 0)
                                    __inout_dev_id = dev_id;
                            }

                            __device_io.stream_parameters.deviceId = __inout_dev_id;
                            if (__inout_dev_id == 0) {
                                __device_io.stream_parameters.deviceId = _is_input_device ? rta_handler.getDefaultInputDevice() :
                                                                                            rta_handler.getDefaultOutputDevice();
                                __device_io.device_mode = DEVICE_MODE_DEFAULT;
                                // TODO: maybe fall back to some other method (2 lines above). Or not (below).
                                // throw std::runtime_error(SVKFW_WRAPERR("RTA :: AudioHandler :: deviceUpdate", "no duplex devices available (i)"));
                            }
                        }
                        break;
                    }
                    case DEVICE_MODE_UNSPEC: {
                        break;
                    }
                    default: {
                        throw std::runtime_error(SVKFW_WRAPERR("RTA :: AudioHandler :: deviceUpdate_", "unsupported device mode: " + std::to_string(__device_io.device_mode)));
                    }
                }

                if (__old_device_id != __device_io.stream_parameters.deviceId) {
                    std::string __info_string = deviceGetInfoStr_(__device_io.stream_parameters.deviceId);
                    printf("%s %s\n", _is_input_device ? "New Input" : "New Output", __info_string.c_str());
                }
            }
            inline void deviceInputUpdate()  { deviceUpdate_( true); }
            inline void deviceOutputUpdate() { deviceUpdate_(false); }
            void deviceDuplexUpdate() {
                if (device_i.device_mode == DEVICE_MODE_DUPLEX &&
                    device_o.device_mode == DEVICE_MODE_DUPLEX) {
                    RtAudio::DeviceInfo __device_info = rta_handler.getDeviceInfo(device_o.stream_parameters.deviceId);
                    bool __duplex_o_exists = device_o.stream_parameters.deviceId > 0 && __device_info.ID == device_o.stream_parameters.deviceId && __device_info.duplexChannels > 0;
                    bool __duplex_i_exists = __duplex_o_exists;
                    bool __same_i_o = device_o.stream_parameters.deviceId == device_i.stream_parameters.deviceId;

                    if (!__same_i_o) {
                        __device_info = rta_handler.getDeviceInfo(device_i.stream_parameters.deviceId);
                        __duplex_i_exists = device_i.stream_parameters.deviceId > 0 && __device_info.ID == device_i.stream_parameters.deviceId && __device_info.duplexChannels > 0;
                    }

                    if (__duplex_o_exists != __duplex_i_exists) { // one duplex device
                        device_i.stream_parameters.deviceId = __duplex_o_exists ? device_o.stream_parameters.deviceId :
                                                                                  device_i.stream_parameters.deviceId;
                        device_o.stream_parameters.deviceId = __duplex_i_exists ? device_i.stream_parameters.deviceId :
                                                                                  device_o.stream_parameters.deviceId;
                    }
                    else if (!(__duplex_o_exists || __duplex_i_exists)) { // no duplex devices
                        uint32_t __inout_dev_id = 0; // use device that has both in and out channels, if such exists
                        for (uint32_t dev_id : rta_handler.getDeviceIds()) {
                            __device_info = rta_handler.getDeviceInfo(dev_id);
                            if (__device_info.duplexChannels > 0) {
                                device_i.stream_parameters.deviceId = dev_id;
                                device_o.stream_parameters.deviceId = dev_id;
                                break;
                            }
                            if (__device_info.inputChannels > 0 && __device_info.outputChannels > 0)
                                __inout_dev_id = dev_id;
                        }

                        device_i.stream_parameters.deviceId = __inout_dev_id;
                        device_o.stream_parameters.deviceId = __inout_dev_id;
                        if (__inout_dev_id == 0) {
                            device_i.stream_parameters.deviceId = rta_handler.getDefaultInputDevice();
                            device_o.stream_parameters.deviceId = rta_handler.getDefaultOutputDevice();
                            device_i.device_mode = DEVICE_MODE_DEFAULT;
                            device_o.device_mode = DEVICE_MODE_DEFAULT;
                            // TODO: maybe fall back to some other method (4 lines above). Or not (below).
                            // throw std::runtime_error(SVKFW_WRAPERR("RTA :: AudioHandler :: deviceUpdate", "no duplex devices available (i)"));
                        }
                    }
                    else if (!__same_i_o) { // two different duplex devices
                        // TODO: sync by setting same duplex device. Might do additional work here choosing between device_i/device_o as duplex device.
                        device_i.stream_parameters.deviceId = device_o.stream_parameters.deviceId;
                    }
                    // else {} - same duplex device, no work needed
                }
            }
            inline void deviceUpdateAll() {
                if (device_i.device_mode == DEVICE_MODE_DUPLEX &&
                    device_o.device_mode == DEVICE_MODE_DUPLEX) {
                    deviceDuplexUpdate();
                }
                else {
                    deviceOutputUpdate();
                    deviceInputUpdate();
                }
            }


            // Used by getInputDeviceInfoStr/getOutputDeviceInfoStr
            std::string deviceGetInfoStr_(uint32_t _device_id) {
                std::string __res = "";

                if (_device_id > 0) {
                    RtAudio::DeviceInfo __device_info = rta_handler.getDeviceInfo(_device_id);

                    if (_device_id == __device_info.ID) {
                        __res += "Audio Device Info:\n";
                        __res += "- Name: " + __device_info.name + '\n';
                        __res += "- ID: " + std::to_string(__device_info.ID) + '\n';
                        __res += "- Channels (i): " + std::to_string(__device_info. inputChannels) + '\n';
                        __res += "- Channels (o): " + std::to_string(__device_info.outputChannels) + '\n';
                        __res += "- Channels (d): " + std::to_string(__device_info.duplexChannels) + '\n';
                        __res += "- Is default (i): " + std::to_string(__device_info.isDefaultInput)  + '\n';
                        __res += "- Is default (o): " + std::to_string(__device_info.isDefaultOutput) + '\n';
                        __res += "- Sample rate (curr): " + std::to_string(__device_info.  currentSampleRate) + '\n';
                        __res += "- Sample rate (pref): " + std::to_string(__device_info.preferredSampleRate) + '\n';
                        __res += "- Native formats (bitflag): " + std::to_string(__device_info.nativeFormats) + '\n';
                        __res += "- Native formats: ";

                        if (__device_info.nativeFormats & RTAUDIO_SINT8)
                            __res += __device_info.nativeFormats >= RTAUDIO_SINT16  ? "Int8, "    : "Int8";
                        if (__device_info.nativeFormats & RTAUDIO_SINT16)
                            __res += __device_info.nativeFormats >= RTAUDIO_SINT24  ? "Int16, "   : "Int16";
                        if (__device_info.nativeFormats & RTAUDIO_SINT24)
                            __res += __device_info.nativeFormats >= RTAUDIO_SINT32  ? "Int24, "   : "Int24";
                        if (__device_info.nativeFormats & RTAUDIO_SINT32)
                            __res += __device_info.nativeFormats >= RTAUDIO_FLOAT32 ? "Int32, "   : "Int32";
                        if (__device_info.nativeFormats & RTAUDIO_FLOAT32)
                            __res += __device_info.nativeFormats >= RTAUDIO_FLOAT64 ? "Float32, " : "Float32";
                        if (__device_info.nativeFormats & RTAUDIO_FLOAT64)
                            __res += "Float64";
                        __res += '\n';
                    }
                }
                return __res;
            }
            inline std::string  deviceInputGetInfoStr() { return deviceGetInfoStr_(device_i.stream_parameters.deviceId); }
            inline std::string deviceOutputGetInfoStr() { return deviceGetInfoStr_(device_o.stream_parameters.deviceId); }
        }; // AudioHandler END


    // Callbacks. Last argument is expected to be a pointer to AudioHandler.

        int rtacb_sin(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
                      double _stream_t, RtAudioStreamStatus _status, void *_config) {
            float *__buffer = (float*) _o_buffer;
            static float __last_val = 0.f;
            // static uint32_t __time_to_print = 5;

            if (_status == RTAUDIO_INPUT_OVERFLOW)
                std::cout << "Stream overflow detected" << std::endl;
            else if (_status == RTAUDIO_OUTPUT_UNDERFLOW)
                std::cout << "Stream underflow detected" << std::endl;
            else if (_status)
                std::cout << "Stream underflow AND overflow detected" << std::endl;

            // Write interleaved audio data.
            #pragma omp parallel for collapse(2)
            for (uint32_t i = 0; i < _n_buf_frames; ++i) {
                for (uint32_t j = 0; j < 2; ++j) {
                    *__buffer++ = ((AudioHandler*)_config)->audio_config.getVolume() * std::sin(__last_val);
                }
            }
            __last_val += 0.05f;

            // if (_stream_t > time_to_print) {
            //     __time_to_print += 5;
            //     __buffer   = (float*) _o_buffer;
            //     for (uint32_t i = 0u; i < _n_buf_frames; ++i)
            //         std::cout << __buffer[i] << ", ";
            //     std::cout << "\n";
            //     for (uint32_t i = _n_buf_frames; i < 2*_n_buf_frames; ++i)
            //         std::cout << __last_val << ", ";
            //     std::cout << "\n" << std::endl;
            // }

            return 0;
        }

        int rtacb_saw(void *_o_buffer, void *inputBuffer, unsigned int _n_buf_frames,
                      double _stream_t, RtAudioStreamStatus _status, void *_config) {
            float *buffer = (float*) _o_buffer;
            static float __last_val[2] = { 0.f, 0.f };

            if (_status == RTAUDIO_INPUT_OVERFLOW)
                std::cout << "Stream overflow detected" << std::endl;
            else if (_status == RTAUDIO_OUTPUT_UNDERFLOW)
                std::cout << "Stream underflow detected" << std::endl;
            else if (_status)
                std::cout << "Stream underflow AND overflow detected" << std::endl;

            // Write interleaved audio data.
            for (uint32_t i = 0; i < _n_buf_frames; ++i) {
                for (uint32_t j = 0; j < 2; ++j) {
                    *buffer++ = __last_val[j];

                    __last_val[j] += 0.005f * (j + 1.f + j * 0.1f);
                    if (__last_val[j] >= 1.f) __last_val[j] -= 2.f;
                }
            }

            return 0;
        }

        int rtacb_inout(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
                        double _stream_t, RtAudioStreamStatus _status, void *_config) {
            if (_status == RTAUDIO_INPUT_OVERFLOW)
                printf("Stream overflow detected\n");
            // else if (_status == RTAUDIO_OUTPUT_UNDERFLOW)
            //     std::cout << "Stream underflow detected" << std::endl;
            // else if (_status)
            //     std::cout << "Stream underflow AND overflow detected" << std::endl;

            AudioHandler *__config = (AudioHandler*)_config;

            // If the number of input and output channels is equal, we simply copy buffer here.
            if (__config->device_i.stream_parameters.nChannels == __config->device_o.stream_parameters.nChannels) {
                uint32_t bytes = __config->device_o.stream_parameters.nChannels * __config->stream_config.buffer_frames *
                                 __config->stream_config.getBufferElemBytes();
                memcpy(_o_buffer, _i_buffer, bytes);

                // if (__config->stream_config.audio_format == RTAUDIO_FLOAT32) {
                //     float *__i_buffer = (float*) _i_buffer;
                //     float *__o_buffer = (float*) _o_buffer;

                //     // #pragma omp parallel for
                //     for (uint32_t i = 0u; i < __config->device_o.stream_parameters.nChannels * __config->stream_config.buffer_frames; ++i)
                //         __o_buffer[i] = __i_buffer[i] * __config->audio_config.getVolume();
                // }
            }
            return 0;
        }

        int rtacb_record(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
                         double _stream_t, RtAudioStreamStatus _status, void *_config) {
            if (_status == RTAUDIO_INPUT_OVERFLOW)
                printf("Record: Stream overflow detected\n");
            else if (_status == RTAUDIO_OUTPUT_UNDERFLOW)
                std::cout << "Record: Stream underflow detected" << std::endl;
            // else if (_status)
            //     std::cout << "Stream underflow AND overflow detected" << std::endl;

            memcpy(Global_audio_buf.buf, _i_buffer, Global_audio_buf.buf_size * sizeof(float));
            return 0;
        }

        int rtacb_playback(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
                         double _stream_t, RtAudioStreamStatus _status, void *_config) {
            if (_status == RTAUDIO_INPUT_OVERFLOW)
                printf("Playback: Stream overflow detected\n");
            else if (_status == RTAUDIO_OUTPUT_UNDERFLOW)
                std::cout << "Playback: Stream underflow detected" << std::endl;
            // else if (_status)
            //     std::cout << "Stream underflow AND overflow detected" << std::endl;

            memcpy(_o_buffer, Global_audio_buf.buf, Global_audio_buf.buf_size * sizeof(float));
            return 0;
        }
    }; // RTA END
}; // Simple END

#endif