#ifndef SVKFW_RTAUDIO_WRAP_H
#define SVKFW_RTAUDIO_WRAP_H

#include "RtAudio.h"
#include "sound/samples.h"


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


        template <RtAudioFormat _audio_format>
        uint32_t bufferElemBytes() {
            switch (_audio_format) {
                case RTAUDIO_SINT8:   return sizeof( int8_t);
                case RTAUDIO_SINT16:  return sizeof(int16_t);
                case RTAUDIO_SINT24:  return              3u;
                case RTAUDIO_SINT32:  return sizeof(int32_t);
                case RTAUDIO_FLOAT32: return sizeof(  float);
                case RTAUDIO_FLOAT64: return sizeof( double);
            }
            return 0;
        }
        uint32_t bufferElemBytes(RtAudioFormat _audio_format) {
            switch (_audio_format) {
                case RTAUDIO_SINT8:   return sizeof( int8_t);
                case RTAUDIO_SINT16:  return sizeof(int16_t);
                case RTAUDIO_SINT24:  return              3u;
                case RTAUDIO_SINT32:  return sizeof(int32_t);
                case RTAUDIO_FLOAT32: return sizeof(  float);
                case RTAUDIO_FLOAT64: return sizeof( double);
            }
            return 0;
        }

        template <RtAudioFormat _audio_format>
        uint32_t maxVolume() {
            switch (_audio_format) {
                case RTAUDIO_SINT8:   return  INT8_MAX;
                case RTAUDIO_SINT16:  return INT16_MAX;
                case RTAUDIO_SINT24:  return INT32_MAX >> 8;
                case RTAUDIO_SINT32:  return INT32_MAX;
                case RTAUDIO_FLOAT32: return        1u;
                case RTAUDIO_FLOAT64: return        1u;
            }
            return 0;
        }
        uint32_t maxVolume(RtAudioFormat _audio_format) {
            switch (_audio_format) {
                case RTAUDIO_SINT8:   return  INT8_MAX;
                case RTAUDIO_SINT16:  return INT16_MAX;
                case RTAUDIO_SINT24:  return INT32_MAX >> 8;
                case RTAUDIO_SINT32:  return INT32_MAX;
                case RTAUDIO_FLOAT32: return        1u;
                case RTAUDIO_FLOAT64: return        1u;
            }
            return 0;
        }


        // This class contains parameters set by a user
        struct AudioConfig {
            uint8_t volume = 255;
            Sound::Sample::SmpMixer played_sound;
        }; // AudioConfig END

        // This class contains all information available to RTAudio callbacks via 'void* config' argument
        struct RTAStreamConfig {
            StreamMode stream_mode = STREAM_MODE_UNSPEC;
            uint32_t sample_rate;
            uint32_t buffer_frames; // automatically set on stream opening
            RtAudioFormat audio_format;
            AudioConfig audio_config;

            RtAudio::StreamOptions stream_options;
            struct DeviceHandler {
                RtAudio::StreamParameters stream_parameters; // Note: device id is only stored as 'stream_parameters.deviceId'
                DeviceMode device_mode = DEVICE_MODE_UNSPEC;
            } device_i, device_o; // devices: in, out; duplex mode: 'device_i'+'device_o' parameters are used


            uint32_t getBufferElemBytes() const { return bufferElemBytes(audio_format); }
            uint32_t getMaxVolume() const { return maxVolume(audio_format); }
            float getVolume() const { return audio_config.volume/256.f*getMaxVolume(); }
            void updateSampleRate(uint32_t _sample_rate) {
                sample_rate = _sample_rate;
                audio_config.played_sound.setSRate(_sample_rate);
            }
        }; // RTAStreamConfig END


        struct AudioHandler {
            RtAudio rta_handler;
            RtAudioCallback rta_cback;
            RTAStreamConfig stream_config;


            AudioHandler() : rta_cback{nullptr} { sizeof(AudioHandler); }
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

                stream_config.device_i.device_mode = _i_device_mode;
                stream_config.device_o.device_mode = _o_device_mode;
            }

            void streamSetOptions(RtAudioStreamFlags _s_flags, uint32_t _s_nbuffers, const std::string &_s_name = "SimpleVKFW_AudioHandler", int _s_priority = 0) {
                stream_config.stream_options.flags           = _s_flags;
                stream_config.stream_options.numberOfBuffers = _s_nbuffers;
                stream_config.stream_options.streamName      = _s_name;
                stream_config.stream_options.priority        = _s_priority;
            }

            void streamOpen(uint32_t _sample_rate, uint32_t _buf_frames, RtAudioFormat _audio_format, bool _use_options = true) {
                RtAudio::StreamParameters* __o_params = (uint32_t(stream_config.stream_mode) & 1) ?
                                                                 &stream_config.device_o.stream_parameters : nullptr;
                RtAudio::StreamParameters* __i_params = (uint32_t(stream_config.stream_mode) & 2) ?
                                                                 &stream_config.device_i.stream_parameters : nullptr;

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
                stream_config.updateSampleRate(_sample_rate);
                stream_config.buffer_frames = _buf_frames;
                stream_config.audio_format = _audio_format;
                RtAudioErrorType  __res = rta_handler.openStream(__o_params, __i_params, _audio_format, _sample_rate, 
                                                                 &stream_config.buffer_frames, rta_cback, &stream_config,
                                                                 _use_options ? &stream_config.stream_options : nullptr);

                // printf("Stream open with parameters:\nBuffer frames: %d\nMax volume: %d\n\n", stream_config.buffer_frames, audio_config.max_volume);
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
            void streamShowWarnings(bool _do_show = true) { rta_handler.showWarnings(_do_show); }


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
                stream_config.device_i.stream_parameters.firstChannel = _first_channel;
                stream_config.device_i.stream_parameters.nChannels    = _nchannels;
            }
            void deviceOutputSetParameters(uint32_t _nchannels, uint32_t _first_channel = 0) {
                stream_config.device_o.stream_parameters.firstChannel = _first_channel;
                stream_config.device_o.stream_parameters.nChannels    = _nchannels;
            }

            bool deviceInputIsFormatSupported(RtAudioFormat _audio_format) {
                return rta_handler.getDeviceInfo(stream_config.device_i.stream_parameters.deviceId).nativeFormats & _audio_format;
            }
            bool deviceOutputIsFormatSupported(RtAudioFormat _audio_format) {
                return rta_handler.getDeviceInfo(stream_config.device_o.stream_parameters.deviceId).nativeFormats & _audio_format;
            }

            void deviceUpdate_(bool _is_input_device) {
                RTAStreamConfig::DeviceHandler &__device_io = _is_input_device ? stream_config.device_i : stream_config.device_o;
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
                if (stream_config.device_i.device_mode == DEVICE_MODE_DUPLEX &&
                    stream_config.device_o.device_mode == DEVICE_MODE_DUPLEX) {
                    RtAudio::DeviceInfo __device_info = rta_handler.getDeviceInfo(stream_config.device_o.stream_parameters.deviceId);
                    bool __duplex_o_exists = stream_config.device_o.stream_parameters.deviceId > 0 &&
                                             __device_info.ID == stream_config.device_o.stream_parameters.deviceId &&
                                             __device_info.duplexChannels > 0;
                    bool __duplex_i_exists = __duplex_o_exists;
                    bool __same_i_o = stream_config.device_o.stream_parameters.deviceId == stream_config.device_i.stream_parameters.deviceId;

                    if (!__same_i_o) {
                        __device_info = rta_handler.getDeviceInfo(stream_config.device_i.stream_parameters.deviceId);
                        __duplex_i_exists = stream_config.device_i.stream_parameters.deviceId > 0 &&
                                            __device_info.ID == stream_config.device_i.stream_parameters.deviceId &&
                                            __device_info.duplexChannels > 0;
                    }

                    if (__duplex_o_exists != __duplex_i_exists) { // one duplex device
                        stream_config.device_i.stream_parameters.deviceId = __duplex_o_exists ? stream_config.device_o.stream_parameters.deviceId :
                                                                                                stream_config.device_i.stream_parameters.deviceId;
                        stream_config.device_o.stream_parameters.deviceId = __duplex_i_exists ? stream_config.device_i.stream_parameters.deviceId :
                                                                                                stream_config.device_o.stream_parameters.deviceId;
                    }
                    else if (!(__duplex_o_exists || __duplex_i_exists)) { // no duplex devices
                        uint32_t __inout_dev_id = 0; // use device that has both in and out channels, if such exists
                        for (uint32_t dev_id : rta_handler.getDeviceIds()) {
                            __device_info = rta_handler.getDeviceInfo(dev_id);
                            if (__device_info.duplexChannels > 0) {
                                stream_config.device_i.stream_parameters.deviceId = dev_id;
                                stream_config.device_o.stream_parameters.deviceId = dev_id;
                                break;
                            }
                            if (__device_info.inputChannels > 0 && __device_info.outputChannels > 0)
                                __inout_dev_id = dev_id;
                        }

                        stream_config.device_i.stream_parameters.deviceId = __inout_dev_id;
                        stream_config.device_o.stream_parameters.deviceId = __inout_dev_id;
                        if (__inout_dev_id == 0) {
                            stream_config.device_i.stream_parameters.deviceId = rta_handler.getDefaultInputDevice();
                            stream_config.device_o.stream_parameters.deviceId = rta_handler.getDefaultOutputDevice();
                            stream_config.device_i.device_mode = DEVICE_MODE_DEFAULT;
                            stream_config.device_o.device_mode = DEVICE_MODE_DEFAULT;
                            // TODO: maybe fall back to some other method (4 lines above). Or not (below).
                            // throw std::runtime_error(SVKFW_WRAPERR("RTA :: AudioHandler :: deviceUpdate", "no duplex devices available (i)"));
                        }
                    }
                    else if (!__same_i_o) { // two different duplex devices
                        // TODO: sync by setting same duplex device. Might do additional work here choosing between device_i/device_o as duplex device.
                        stream_config.device_i.stream_parameters.deviceId = stream_config.device_o.stream_parameters.deviceId;
                    }
                    // else {} - same duplex device, no work needed
                }
            }
            inline void deviceUpdateAll() {
                if (stream_config.device_i.device_mode == DEVICE_MODE_DUPLEX &&
                    stream_config.device_o.device_mode == DEVICE_MODE_DUPLEX) {
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
            inline std::string  deviceInputGetInfoStr() { return deviceGetInfoStr_(stream_config.device_i.stream_parameters.deviceId); }
            inline std::string deviceOutputGetInfoStr() { return deviceGetInfoStr_(stream_config.device_o.stream_parameters.deviceId); }

        // User methods

            void playAudio(Sound::Sample::SmpItf *_source, float _weight) {
                _source->setSRate(stream_config.sample_rate);
                stream_config.audio_config.played_sound.addSource(_source, _weight);
            }
        }; // AudioHandler END


        // Manages "void *buffer" from RtAudio stream callbacks. TODO: manage Int24, Flt64
        struct StreamBuffer {
            void *buffer;
            RtAudioFormat audio_format;
            uint32_t sample_rate;
            uint32_t buffer_frames;
            uint32_t channels;

            StreamBuffer(void *_buffer, uint32_t _buf_frames, uint32_t _channels, RTAStreamConfig *_config)
                            : buffer{_buffer}, audio_format{_config->audio_format}, sample_rate{_config->sample_rate},
                              buffer_frames{_buf_frames}, channels{_channels} {}

            inline int8_t * bufInt8 () { return (int8_t *)buffer; }
            inline int16_t* bufInt16() { return (int16_t*)buffer; }
            inline int32_t* bufInt32() { return (int32_t*)buffer; }
            inline float  * bufFlt32() { return (float  *)buffer; }
            inline double * bufFlt64() { return (double *)buffer; }


            void setValForChannel(float _val, uint32_t _frame_id, uint32_t _ch) {
                switch (audio_format) {
                    case RTAUDIO_SINT8:
                        bufInt8 ()[_frame_id * channels + _ch] = _val; break;
                    case RTAUDIO_SINT16:
                        bufInt16()[_frame_id * channels + _ch] = _val; break;
                    case RTAUDIO_SINT32:
                        bufInt32()[_frame_id * channels + _ch] = _val; break;
                    case RTAUDIO_FLOAT32:
                        bufFlt32()[_frame_id * channels + _ch] = _val; break;
                    case RTAUDIO_FLOAT64:
                        bufFlt64()[_frame_id * channels + _ch] = _val; break;
                }
            }

            void setValForAllChannels(float _val, uint32_t _frame_id) {
                switch (audio_format) {
                    case RTAUDIO_SINT8: {
                        for (uint32_t j = 0; j < channels; ++j)
                            bufInt8 ()[_frame_id * channels + j] = _val;
                        break;
                    }
                    case RTAUDIO_SINT16: {
                        for (uint32_t j = 0; j < channels; ++j)
                            bufInt16()[_frame_id * channels + j] = _val;
                        break;
                    }
                    case RTAUDIO_SINT32: {
                        for (uint32_t j = 0; j < channels; ++j)
                            bufInt32()[_frame_id * channels + j] = _val;
                        break;
                    }
                    case RTAUDIO_FLOAT32: {
                        for (uint32_t j = 0; j < channels; ++j)
                            bufFlt32()[_frame_id * channels + j] = _val;
                        break;
                    }
                    case RTAUDIO_FLOAT64: {
                        for (uint32_t j = 0; j < channels; ++j)
                            bufFlt64()[_frame_id * channels + j] = _val;
                        break;
                    }
                }
            }

            void setSignalForChannel(Sound::Sample::SmpItf *_sampler, uint32_t _ch) {
                switch (audio_format) {
                    case RTAUDIO_SINT8: {
                        for (uint32_t i = 0; i < buffer_frames; ++i)
                            bufInt8 ()[i * channels + _ch] = maxVolume<RTAUDIO_SINT8>() * _sampler->sample();
                        break;
                    }
                    case RTAUDIO_SINT16: {
                        for (uint32_t i = 0; i < buffer_frames; ++i)
                            bufInt16()[i * channels + _ch] = maxVolume<RTAUDIO_SINT16>() * _sampler->sample();
                        break;
                    }
                    case RTAUDIO_SINT32: {
                        for (uint32_t i = 0; i < buffer_frames; ++i)
                            bufInt32()[i * channels + _ch] = maxVolume<RTAUDIO_SINT32>() * _sampler->sample();
                        break;
                    }
                    case RTAUDIO_FLOAT32: {
                        for (uint32_t i = 0; i < buffer_frames; ++i)
                            bufFlt32()[i * channels + _ch] = maxVolume<RTAUDIO_FLOAT32>() * _sampler->sample();
                        break;
                    }
                    case RTAUDIO_FLOAT64: {
                        for (uint32_t i = 0; i < buffer_frames; ++i)
                            bufFlt64()[i * channels + _ch] = maxVolume<RTAUDIO_FLOAT64>() * _sampler->sample();
                        break;
                    }
                }
            }

            void setSignalForAllChannels(Sound::Sample::SmpItf *_sampler) {
                switch (audio_format) {
                    case RTAUDIO_SINT8: {
                        for (uint32_t i = 0; i < buffer_frames; ++i) {
                            float __val = maxVolume<RTAUDIO_SINT8>() * _sampler->sample();
                            for (uint32_t j = 0; j < channels; ++j)
                                bufInt8 ()[i * channels + j] = __val;
                        }
                        break;
                    }
                    case RTAUDIO_SINT16: {
                        for (uint32_t i = 0; i < buffer_frames; ++i) {
                            float __val = maxVolume<RTAUDIO_SINT16>() * _sampler->sample();
                            // printf("%f\n", __val);
                            for (uint32_t j = 0; j < channels; ++j)
                                bufInt16()[i * channels + j] = __val;
                        }
                        break;
                    }
                    case RTAUDIO_SINT32: {
                        for (uint32_t i = 0; i < buffer_frames; ++i) {
                            float __val = maxVolume<RTAUDIO_SINT32>() * _sampler->sample();
                            for (uint32_t j = 0; j < channels; ++j)
                                bufInt32()[i * channels + j] = __val;
                        }
                        break;
                    }
                    case RTAUDIO_FLOAT32: {
                        for (uint32_t i = 0; i < buffer_frames; ++i) {
                            float __val = maxVolume<RTAUDIO_FLOAT32>() * _sampler->sample();
                            for (uint32_t j = 0; j < channels; ++j)
                                bufFlt32()[i * channels + j] = __val;
                        }
                        break;
                    }
                    case RTAUDIO_FLOAT64: {
                        for (uint32_t i = 0; i < buffer_frames; ++i) {
                            float __val = maxVolume<RTAUDIO_FLOAT64>() * _sampler->sample();
                            for (uint32_t j = 0; j < channels; ++j)
                                bufFlt64()[i * channels + j] = __val;
                        }
                        break;
                    }
                }
            }
        }; // StreamBuffer END


    // Callbacks. Last argument is expected to be a pointer to AudioHandler.

        void rtacbackWarningCheck(RtAudioStreamStatus _status) {
            if (_status) {
                const char *__res = "Stream unknown abnormal state detected\n";
                if (_status == RTAUDIO_INPUT_OVERFLOW)
                    __res = "Stream overflow detected\n";
                else if (_status == RTAUDIO_OUTPUT_UNDERFLOW)
                    __res = "Stream underflow detected\n";
                printf("%s", __res);
            }
        }

        int rtacbDefault(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
                          double _stream_t, RtAudioStreamStatus _status, void *_config) {
            rtacbackWarningCheck(_status);

            // Write interleaved audio data.
            RTAStreamConfig *__config = (RTAStreamConfig*)_config;

            static StreamBuffer __outbuf_wrap{_o_buffer, _n_buf_frames, __config->device_o.stream_parameters.nChannels, __config};
            __outbuf_wrap.buffer = _o_buffer;
            __outbuf_wrap.buffer_frames = _n_buf_frames;
            // static Sound::TestWave __sine_wave1{1.f, __config->getMaxVolume()};

            __outbuf_wrap.setSignalForAllChannels(&__config->audio_config.played_sound); // Note: for interleaved mode. TODO: Support non-interleaved
            return 0;
        }

        int rtacb_saw_once(void *_o_buffer, void *inputBuffer, unsigned int _n_buf_frames,
                      double _stream_t, RtAudioStreamStatus _status, void *_config) {
            RTAStreamConfig *__config = (RTAStreamConfig*)_config;
            static StreamBuffer __outbuf_wrap{_o_buffer, _n_buf_frames, __config->device_o.stream_parameters.nChannels, __config};
            static float __last_val[3] = { -1.f, 0.000001f, 0.000002f };

            rtacbackWarningCheck(_status);

            // Write interleaved audio data.
            for (uint32_t i = 0; i < _n_buf_frames; ++i) {
                for (uint32_t j = 0; j < 2; ++j) {
                    __outbuf_wrap.setValForChannel(__config->getMaxVolume() * __last_val[0], i, j);
                }

                __last_val[0] += __last_val[1];
                if (std::abs(__last_val[0]) >= 1.f) {
                    __last_val[1] = -__last_val[1];
                }

                __last_val[1] += Math::sign(__last_val[1]) * __last_val[2];
                if (__last_val[1] >= 0.1f)
                    __last_val[2] = -__last_val[2];
                // printf("%f, ", __last_val[1]);
            }

            return 0;
        }

        int rtacb_saw_pulse(void *_o_buffer, void *inputBuffer, unsigned int _n_buf_frames,
                      double _stream_t, RtAudioStreamStatus _status, void *_config) {
            RTAStreamConfig *__config = (RTAStreamConfig*)_config;
            static StreamBuffer __outbuf_wrap{_o_buffer, _n_buf_frames, __config->device_o.stream_parameters.nChannels, __config};
            static float __last_val[3] = { -1.f, 0.000001f, 1.00006f };

            rtacbackWarningCheck(_status);

            // Write interleaved audio data.
            for (uint32_t i = 0; i < _n_buf_frames; ++i) {
                for (uint32_t j = 0; j < 2; ++j) {
                    __outbuf_wrap.setValForChannel(__config->getMaxVolume() * __last_val[0], i, j);
                }

                __last_val[0] += __last_val[1];
                if (std::abs(__last_val[0]) >= 1.f) {
                    __last_val[1] = -__last_val[1];
                }

                __last_val[1] *= __last_val[2];
                if (std::abs(__last_val[1]) >=  0.1f && __last_val[2] > 1.f ||
                    std::abs(__last_val[1]) <= 1e-6f && __last_val[2] < 1.f)
                    __last_val[2] = 1.f / __last_val[2];
                // printf("%f, ", __last_val[1]);
            }

            return 0;
        }

        int rtacb_saw(void *_o_buffer, void *inputBuffer, unsigned int _n_buf_frames,
                      double _stream_t, RtAudioStreamStatus _status, void *_config) {
            RTAStreamConfig *__config = (RTAStreamConfig*)_config;
            static StreamBuffer __outbuf_wrap{_o_buffer, _n_buf_frames, __config->device_o.stream_parameters.nChannels, __config};
            static float __last_val[2] = { -1.f, 0.02f };
            float __srate_inv = 1.f / __config->sample_rate;
            static Sound::Sample::SmpSaw __saw_sampler{440.f};

            rtacbackWarningCheck(_status);

            __outbuf_wrap.setSignalForAllChannels(&__saw_sampler);
            return 0;
        }

        int rtacb_inout(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
                        double _stream_t, RtAudioStreamStatus _status, void *_config) {
            rtacbackWarningCheck(_status);

            RTAStreamConfig *__config = (RTAStreamConfig*)_config;

            // If the number of input and output channels is equal, we simply copy buffer here.
            if (__config->device_i.stream_parameters.nChannels == __config->device_o.stream_parameters.nChannels) {
                uint32_t bytes = __config->device_o.stream_parameters.nChannels * __config->buffer_frames *
                                 __config->getBufferElemBytes();
                memcpy(_o_buffer, _i_buffer, bytes);
            }
            return 0;
        }

        // int rtacb_record(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
        //                  double _stream_t, RtAudioStreamStatus _status, void *_config) {
        //     rtacbackWarningCheck(_status);

        //     memcpy(Global_audio_buf.buf, _i_buffer, Global_audio_buf.buf_size * sizeof(float));
        //     return 0;
        // }

        // int rtacb_playback(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
        //                  double _stream_t, RtAudioStreamStatus _status, void *_config) {
        //     rtacbackWarningCheck(_status);

        //     memcpy(_o_buffer, Global_audio_buf.buf, Global_audio_buf.buf_size * sizeof(float));
        //     return 0;
        // }
    }; // RTA END
}; // Simple END

#endif