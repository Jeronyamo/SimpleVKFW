#ifndef SVKFW_RTAUDIO_WRAP_H
#define SVKFW_RTAUDIO_WRAP_H

#include "RtAudio.h"


namespace Simple {
    namespace RTA {
        // Device types (based on channels) available
        enum DeviceType {
            DT_BIT_ONLY_IN  = 1 << 0,
            DT_BIT_ONLY_OUT = 1 << 1,
            DT_BIT_DUPLEX   = 1 << 2,
            DT_BIT_BOTH_IO  = 1 << 3, // TODO: Is it possible that the same device has input and output channels and no duplex?
        }; // DeviceType END

        enum DeviceMode {
            DEVICE_MODE_UNSPECIFIED , // Initial mode, replaced with DEFAULT in/out mode if this device is used to open a stream, ignored otherwise
            DEVICE_MODE_DEFAULT     , // Checks for default input/output device
            DEVICE_MODE_SELECTED    , // Uses selected input/output device while it's available, else selects default in/out devices
            DEVICE_MODE_DUPLEX      , // Uses device with duplex channels while it's available, else falls back to DEFAULT in/out modes
        }; // DeviceMode END

        enum StreamMode {
            STREAM_MODE_UNSPECIFIED, // Not setting stream mode is an error
            STREAM_MODE_OUT        , // Output stream
            STREAM_MODE_IN         , // Input stream
            STREAM_MODE_DUPLEX     , // Input+output stream, no checks if it uses actual duplex channels of the same device. TODO: what checks to add?
        }; // StreamMode END


        // Callbacks

        int rtacb_sin(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                      double streamTime, RtAudioStreamStatus status, void *userData) {
            uint32_t i, j;
            float *buffer     = (float*) outputBuffer;
            static float lastValues = 0.f;
            static uint32_t time_to_print = 5;

            if (status == RTAUDIO_INPUT_OVERFLOW)
                std::cout << "Stream overflow detected" << std::endl;
            else if (status == RTAUDIO_OUTPUT_UNDERFLOW)
                std::cout << "Stream underflow detected" << std::endl;
            else if (status)
                std::cout << "Stream underflow AND overflow detected" << std::endl;

            // Write interleaved audio data.
            #pragma omp parallel for collapse(2)
            for (uint32_t i = 0; i < nBufferFrames; ++i) {
                for (uint32_t j = 0; j < 2; ++j) {
                    *buffer++ = std::sin(lastValues);
                }
            }
            lastValues += 0.05f;

            if (streamTime > time_to_print) {
                time_to_print += 5;
                buffer     = (float*) outputBuffer;
                for (uint32_t i = 0u; i < nBufferFrames; ++i)
                    std::cout << buffer[i] << ", ";
                std::cout << "\n";
                for (uint32_t i = nBufferFrames; i < 2*nBufferFrames; ++i)
                    std::cout << buffer[i] << ", ";
                std::cout << "\n" << std::endl;
            }

            return 0;
        }

        int rtacb_saw(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                      double streamTime, RtAudioStreamStatus status, void *userData) {
            uint32_t i, j;
            float *buffer     = (float*) outputBuffer;
            static float lastValues[2] = { 0.f, 0.f };

            if (status == RTAUDIO_INPUT_OVERFLOW)
                std::cout << "Stream overflow detected" << std::endl;
            else if (status == RTAUDIO_OUTPUT_UNDERFLOW)
                std::cout << "Stream underflow detected" << std::endl;
            else if (status)
                std::cout << "Stream underflow AND overflow detected" << std::endl;

            // Write interleaved audio data.
            for (uint32_t i = 0; i < nBufferFrames; ++i) {
                for (uint32_t j = 0; j < 2; ++j) {
                    *buffer++ = lastValues[j];

                    lastValues[j] += 0.005f * (j + 1.f + j * 0.1f);
                    if (lastValues[j] >= 1.f) lastValues[j] -= 2.f;
                }
            }

            return 0;
        }

        int rtacb_inout(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                        double streamTime, RtAudioStreamStatus status, void *data) {
            if (status == RTAUDIO_INPUT_OVERFLOW)
                std::cout << "Stream overflow detected" << std::endl;
            else if (status == RTAUDIO_OUTPUT_UNDERFLOW)
                std::cout << "Stream underflow detected" << std::endl;
            else if (status)
                std::cout << "Stream underflow AND overflow detected" << std::endl;

            // Since the number of input and output channels is equal, we can do
            // a simple buffer copy operation here.
            uint32_t *bytes = (uint32_t*) data;
            memcpy(outputBuffer, inputBuffer, *bytes);
            return 0;
        }


        struct AudioHandler {
            RtAudio rta_handler;
            RtAudioCallback rta_cback = nullptr;
            StreamMode stream_mode = STREAM_MODE_UNSPECIFIED;

            RtAudio::StreamOptions stream_options;
            struct DeviceHandler {
                RtAudio::StreamParameters stream_parameters; // Note: device id is only stored as stream_parameters.deviceId
                DeviceMode device_mode = DEVICE_MODE_UNSPECIFIED;
            } device_i, device_o; // devices: in, out; duplex mode: device_i+device_o parameters are used

            void* cback_data = nullptr;


            AudioHandler() { sizeof(AudioHandler); }
            ~AudioHandler() {
                if (rta_handler.isStreamRunning())
                    rta_handler.abortStream();
                if (rta_handler.isStreamOpen())
                    rta_handler.closeStream();
            }


            void callbackSet(RtAudioCallback _rta_cback, void *_cback_data = nullptr) {
                rta_cback  =  _rta_cback;
                cback_data = _cback_data;
            }


        // Stream methods

            void streamSetMode(StreamMode _stream_mode, DeviceMode _o_device_mode = DEVICE_MODE_UNSPECIFIED, DeviceMode _i_device_mode = DEVICE_MODE_UNSPECIFIED) {
                stream_mode = _stream_mode;

                if ((uint32_t(stream_mode) & 1) && _o_device_mode == DEVICE_MODE_UNSPECIFIED)
                    _o_device_mode = DEVICE_MODE_DEFAULT;
                if ((uint32_t(stream_mode) & 2) && _i_device_mode == DEVICE_MODE_UNSPECIFIED)
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

            void streamOpen(uint32_t _sample_rate, uint32_t _buf_frames, RtAudioFormat _audio_format, bool _use_options = true, void* _cback_data = nullptr) {
                RtAudio::StreamParameters* __o_params = (uint32_t(stream_mode) & 1) ? &device_o.stream_parameters : nullptr;
                RtAudio::StreamParameters* __i_params = (uint32_t(stream_mode) & 2) ? &device_i.stream_parameters : nullptr;

                RtAudioErrorType  __res = rta_handler.openStream(__o_params, __i_params, _audio_format, _sample_rate, 
                                                                 &_buf_frames, rta_cback, _cback_data ? _cback_data : cback_data, &stream_options);

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

            void device_Update(bool _is_input_device) {
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
                    case DEVICE_MODE_UNSPECIFIED: {
                        break;
                    }
                    default: {
                        throw std::runtime_error(SVKFW_WRAPERR("RTA :: AudioHandler :: device_Update", "unsupported device mode: " + std::to_string(__device_io.device_mode)));
                    }
                }

                if (__old_device_id != __device_io.stream_parameters.deviceId) {
                    std::string __info_string = device_GetInfoStr(__device_io.stream_parameters.deviceId);
                    printf("%s %s\n", _is_input_device ? "New Input" : "New Output", __info_string.c_str());
                }
            }
            inline void deviceInputUpdate()  { device_Update(true); }
            inline void deviceOutputUpdate() { device_Update(false); }
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
                        // TODO: sync by setting same duplex device. Might do additional work here choosing between device_i/device_o duplex device.
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
            std::string device_GetInfoStr(uint32_t _device_id) {
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
            inline std::string  deviceInputGetInfoStr() { return device_GetInfoStr(device_i.stream_parameters.deviceId); }
            inline std::string deviceOutputGetInfoStr() { return device_GetInfoStr(device_o.stream_parameters.deviceId); }
        }; // AudioHandler END
    }; // RTA END
}; // Simple END

#endif