#include "interface/rtaudio_wrap.h"

namespace Simple {
    namespace RTA {

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

        uint32_t AudioStreamConfig::getBufferElemBytes() const { return bufferElemBytes(audio_format); }

        uint32_t AudioStreamConfig::getMaxVolume()       const { return       maxVolume(audio_format); }

        float    AudioStreamConfig::getVolume()          const { return audio_settings.volume/256.f*getMaxVolume(); }

        void AudioStreamConfig::updateSampleRate(uint32_t _sample_rate) {
            sample_rate = _sample_rate;
            audio_sampler.sample_rate = _sample_rate;
        }

        AudioHandler::AudioHandler() : rta_handler{preferredApi_()}, rta_cback{nullptr} {}

        AudioHandler::~AudioHandler() {
             if (rta_handler.isStreamRunning())
                 rta_handler.abortStream();
             if (rta_handler.isStreamOpen())
                 rta_handler.closeStream();
         }

        RtAudio::Api AudioHandler::preferredApi_() {
            std::vector<RtAudio::Api> __apis;
            RtAudio::getCompiledApi(__apis);
            for (RtAudio::Api __api : __apis) {
                if (__api != RtAudio::LINUX_PULSE)
                    continue;
                RtAudio __probe{__api, [](RtAudioErrorType, const std::string &) {}};
                if (__probe.getDeviceCount() > 0)
                    return __api;
            }
            return RtAudio::UNSPECIFIED;
        }

        void AudioHandler::callbackSet(RtAudioCallback _rta_cback) { rta_cback = _rta_cback; }

        void AudioHandler::streamSetMode(StreamMode _stream_mode, DeviceMode _o_device_mode, DeviceMode _i_device_mode) {
            stream_config.stream_mode = _stream_mode;

            if ((uint32_t(stream_config.stream_mode) & 1) && _o_device_mode == DEVICE_MODE_UNSPEC)
                _o_device_mode = DEVICE_MODE_DEFAULT;
            if ((uint32_t(stream_config.stream_mode) & 2) && _i_device_mode == DEVICE_MODE_UNSPEC)
                _i_device_mode = DEVICE_MODE_DEFAULT;

            stream_config.device_i_mode = _i_device_mode;
            stream_config.device_o_mode = _o_device_mode;
        }

        void AudioHandler::streamSetOptions(RtAudioStreamFlags _s_flags, uint32_t _s_nbuffers, const std::string &_s_name, int _s_priority) {
            stream_config.stream_options.flags           = _s_flags;
            stream_config.stream_options.numberOfBuffers = _s_nbuffers;
            stream_config.stream_options.streamName      = _s_name;
            stream_config.stream_options.priority        = _s_priority;
        }

        void AudioHandler::streamOpen(uint32_t _sample_rate, uint32_t _buf_frames, RtAudioFormat _audio_format, bool _use_options) {
            RtAudio::StreamParameters* __o_params = (uint32_t(stream_config.stream_mode) & 1) ?
                                                             &stream_config.device_o : nullptr;
            RtAudio::StreamParameters* __i_params = (uint32_t(stream_config.stream_mode) & 2) ?
                                                             &stream_config.device_i : nullptr;

            if (_sample_rate == 0) {
                if (__o_params != nullptr) {
                    uint32_t __preferred_srate = rta_handler.getDeviceInfo(__o_params->deviceId).preferredSampleRate;
                    _sample_rate = __preferred_srate;
                }
                if (__i_params != nullptr) {
                    uint32_t __preferred_srate = rta_handler.getDeviceInfo(__i_params->deviceId).preferredSampleRate;
                    _sample_rate = std::max(_sample_rate, __preferred_srate);
                }
            }
            stream_config.updateSampleRate(_sample_rate);
            stream_config.buffer_frames = _buf_frames;
            stream_config.audio_format = _audio_format;
            RtAudioErrorType  __res = rta_handler.openStream(__o_params, __i_params, _audio_format, _sample_rate, 
                                                             &stream_config.buffer_frames, rta_cback, &stream_config,
                                                             _use_options ? &stream_config.stream_options : nullptr);

            // printf("Stream open with parameters:\nBuffer frames: %d\nMax volume: %d\n\n", stream_config.buffer_frames, audio_settings.max_volume);
            if (__res != RTAUDIO_NO_ERROR) {
                throw std::runtime_error(SVKFW_WRAPERR("RTA :: AudioHandler :: streamOpen", "Error code " + std::to_string(__res) + ";\n" + rta_handler.getErrorText()));
            }
        }

        void AudioHandler::streamStart() { rta_handler.startStream(); }

        void AudioHandler::streamStop()  { rta_handler. stopStream(); }

        void AudioHandler::streamAbort() { rta_handler.abortStream(); }

        bool AudioHandler::streamIsOpen()     { return rta_handler.isStreamOpen();     }

        bool AudioHandler::streamIsRunning()  { return rta_handler.isStreamRunning();  }

        long AudioHandler::streamGetLatency() { return rta_handler.getStreamLatency(); }

        void AudioHandler::streamShowWarnings(bool _do_show) { rta_handler.showWarnings(_do_show); }

        DeviceType AudioHandler::deviceGetAvailableTypes() {
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

        void AudioHandler::deviceInputSetParameters(uint32_t _nchannels, uint32_t _first_channel) {
            stream_config.device_i.firstChannel = _first_channel;
            stream_config.device_i.nChannels    = _nchannels;
        }

        void AudioHandler::deviceOutputSetParameters(uint32_t _nchannels, uint32_t _first_channel) {
            stream_config.device_o.firstChannel = _first_channel;
            stream_config.device_o.nChannels    = _nchannels;
        }

        bool AudioHandler::deviceInputIsFormatSupported(RtAudioFormat _audio_format) {
            return rta_handler.getDeviceInfo(stream_config.device_i.deviceId).nativeFormats & _audio_format;
        }

        bool AudioHandler::deviceOutputIsFormatSupported(RtAudioFormat _audio_format) {
            return rta_handler.getDeviceInfo(stream_config.device_o.deviceId).nativeFormats & _audio_format;
        }

        void AudioHandler::deviceUpdate_(RtAudio::StreamParameters &_device_io, DeviceMode &_device_io_mode, uint32_t _def_device, const char *_device_type) {
            const uint32_t __old_device_id = _device_io.deviceId;

            switch (_device_io_mode) {
                case DEVICE_MODE_SELECTED: {
                    RtAudio::DeviceInfo __device_info = rta_handler.getDeviceInfo(_device_io.deviceId);
                    if (__device_info.ID == _device_io.deviceId)
                        break;
                }
                case DEVICE_MODE_DEFAULT: {
                    _device_io.deviceId = _def_device;
                    break;
                }
                case DEVICE_MODE_DUPLEX: { // can be used if other device is not duplex for some reason
                    RtAudio::DeviceInfo __device_info = rta_handler.getDeviceInfo(_device_io.deviceId);
                    if (__device_info.ID != _device_io.deviceId) {
                        uint32_t __inout_dev_id = 0; // use device that has both in and out channels, if such exists
                        for (uint32_t dev_id : rta_handler.getDeviceIds()) {
                            __device_info = rta_handler.getDeviceInfo(dev_id);
                            if (__device_info.duplexChannels  ||  __device_info.inputChannels && __device_info.outputChannels) {
                                __inout_dev_id = dev_id;
                                if (__device_info.duplexChannels) break;
                            }
                        }

                        _device_io.deviceId = __inout_dev_id;
                        if (__inout_dev_id == 0) {
                            _device_io.deviceId = _def_device;
                            _device_io_mode = DEVICE_MODE_DEFAULT;
                        }
                    }
                    break;
                }
                case DEVICE_MODE_UNSPEC: {
                    break;
                }
                default: {
                    throw std::runtime_error(SVKFW_WRAPERR("RTA :: AudioHandler :: deviceUpdate_", "unsupported device mode: " + std::to_string(_device_io_mode)));
                }
            }

            if (__old_device_id != _device_io.deviceId) {
                std::string __info_string = deviceGetInfoStr_(_device_io.deviceId);
                printf(SVKFW_WRAPINFO("RTA :: AudioHandler :: deviceUpdate_", "New %s %s\n"), _device_type, __info_string.c_str());
            }
        }

        void AudioHandler::deviceInputUpdate () { deviceUpdate_(stream_config.device_i, stream_config.device_i_mode, rta_handler.getDefaultInputDevice (),  "Input"); }

        void AudioHandler::deviceOutputUpdate() { deviceUpdate_(stream_config.device_o, stream_config.device_o_mode, rta_handler.getDefaultOutputDevice(), "Output"); }

        void AudioHandler::deviceDuplexUpdate() {
            if (stream_config.device_i_mode == DEVICE_MODE_DUPLEX &&
                stream_config.device_o_mode == DEVICE_MODE_DUPLEX) {
                RtAudio::DeviceInfo __device_info = rta_handler.getDeviceInfo(stream_config.device_o.deviceId);
                bool __duplex_o_exists = stream_config.device_o.deviceId > 0 &&
                                         __device_info.ID == stream_config.device_o.deviceId &&
                                         __device_info.duplexChannels > 0;
                bool __duplex_i_exists = __duplex_o_exists;
                bool __same_i_o = stream_config.device_o.deviceId == stream_config.device_i.deviceId;

                if (!__same_i_o) {
                    __device_info = rta_handler.getDeviceInfo(stream_config.device_i.deviceId);
                    __duplex_i_exists = stream_config.device_i.deviceId > 0 &&
                                        __device_info.ID == stream_config.device_i.deviceId &&
                                        __device_info.duplexChannels > 0;
                }

                if (__duplex_o_exists != __duplex_i_exists) { // one duplex device
                    stream_config.device_i.deviceId = __duplex_o_exists ? stream_config.device_o.deviceId :
                                                                                            stream_config.device_i.deviceId;
                    stream_config.device_o.deviceId = __duplex_i_exists ? stream_config.device_i.deviceId :
                                                                                            stream_config.device_o.deviceId;
                }
                else if (!(__duplex_o_exists || __duplex_i_exists)) { // no duplex devices
                    uint32_t __inout_dev_id = 0; // use device that has both in and out channels, if such exists
                    for (uint32_t dev_id : rta_handler.getDeviceIds()) {
                        __device_info = rta_handler.getDeviceInfo(dev_id);
                        if (__device_info.duplexChannels > 0) { // duplex device is preferred
                            __inout_dev_id = dev_id;
                            break;
                        }
                        if (__device_info.inputChannels > 0 && __device_info.outputChannels > 0)
                            __inout_dev_id = dev_id;
                    }


                    stream_config.device_i.deviceId = __inout_dev_id;
                    stream_config.device_o.deviceId = __inout_dev_id;
                    if (__inout_dev_id == 0) {
                        stream_config.device_i.deviceId = rta_handler.getDefaultInputDevice();
                        stream_config.device_o.deviceId = rta_handler.getDefaultOutputDevice();
                        stream_config.device_i_mode = DEVICE_MODE_DEFAULT;
                        stream_config.device_o_mode = DEVICE_MODE_DEFAULT;
                        // TODO: maybe fall back to some other method (4 lines above). Or not (below).
                        // throw std::runtime_error(SVKFW_WRAPERR("RTA :: AudioHandler :: deviceUpdate", "no duplex devices available (i)"));
                    }
                }
                else if (!__same_i_o) { // two different duplex devices
                    // TODO: sync by setting same duplex device. Might do additional work here choosing between device_i/device_o as duplex device.
                    stream_config.device_i.deviceId = stream_config.device_o.deviceId;
                }
                // else {} - same duplex device, no work needed
            }
        }

        void AudioHandler::deviceUpdate() {
            if (stream_config.device_i_mode == DEVICE_MODE_DUPLEX &&
                stream_config.device_o_mode == DEVICE_MODE_DUPLEX) {
                deviceDuplexUpdate();
            }
            else {
                deviceOutputUpdate();
                deviceInputUpdate();
            }
        }

        std::string AudioHandler::deviceGetInfoStr_(uint32_t _device_id) {
            std::string __res = "";

            if (_device_id > 0) {
                RtAudio::DeviceInfo __device_info = rta_handler.getDeviceInfo(_device_id);

                if (_device_id == __device_info.ID) {
                    __res += "Audio Device Info:\n";
                    __res += " - Name: " + __device_info.name + '\n';
                    __res += " - ID: " + std::to_string(__device_info.ID) + '\n';
                    __res += " - Channels (i): " + std::to_string(__device_info. inputChannels) + '\n';
                    __res += " - Channels (o): " + std::to_string(__device_info.outputChannels) + '\n';
                    __res += " - Channels (d): " + std::to_string(__device_info.duplexChannels) + '\n';
                    __res += " - Is default (i): " + std::to_string(__device_info.isDefaultInput)  + '\n';
                    __res += " - Is default (o): " + std::to_string(__device_info.isDefaultOutput) + '\n';
                    __res += " - Sample rate (curr): " + std::to_string(__device_info.  currentSampleRate) + '\n';
                    __res += " - Sample rate (pref): " + std::to_string(__device_info.preferredSampleRate) + '\n';
                    __res += " - Native formats (bitflag): " + std::to_string(__device_info.nativeFormats) + '\n';
                    __res += " - Native formats: ";

                    if (__device_info.nativeFormats & RTAUDIO_SINT8)
                        __res += __device_info.nativeFormats >= RTAUDIO_SINT16  ? "Int8, "  : "Int8" ;
                    if (__device_info.nativeFormats & RTAUDIO_SINT16)
                        __res += __device_info.nativeFormats >= RTAUDIO_SINT24  ? "Int16, " : "Int16";
                    if (__device_info.nativeFormats & RTAUDIO_SINT24)
                        __res += __device_info.nativeFormats >= RTAUDIO_SINT32  ? "Int24, " : "Int24";
                    if (__device_info.nativeFormats & RTAUDIO_SINT32)
                        __res += __device_info.nativeFormats >= RTAUDIO_FLOAT32 ? "Int32, " : "Int32";
                    if (__device_info.nativeFormats & RTAUDIO_FLOAT32)
                        __res += __device_info.nativeFormats >= RTAUDIO_FLOAT64 ? "Flt32, " : "Flt32";
                    if (__device_info.nativeFormats & RTAUDIO_FLOAT64)
                        __res += "Flt64";
                    __res += '\n';
                }
            }
            return __res;
        }

        std::string  AudioHandler::deviceInputGetInfoStr() { return deviceGetInfoStr_(stream_config.device_i.deviceId); }

        std::string AudioHandler::deviceOutputGetInfoStr() { return deviceGetInfoStr_(stream_config.device_o.deviceId); }

        void AudioHandler::playAudio(Audio::Sample::SmpItf *_source, float _weight) {
            stream_config.audio_sampler.addSource(_source, _weight);
        }

        StreamBuffer::StreamBuffer(void *_buffer, uint32_t _buf_frames, uint32_t _channels, AudioStreamConfig *_config)
                        : buffer{_buffer}, audio_format{_config->audio_format}, sample_rate{_config->sample_rate},
                          buffer_frames{_buf_frames}, channels{_channels} {}

        int8_t * StreamBuffer::bufInt8 () { return (int8_t *)buffer; }

        int16_t* StreamBuffer::bufInt16() { return (int16_t*)buffer; }

        int32_t* StreamBuffer::bufInt32() { return (int32_t*)buffer; }

        float  * StreamBuffer::bufFlt32() { return (float  *)buffer; }

        double * StreamBuffer::bufFlt64() { return (double *)buffer; }

        void StreamBuffer::setValForChannel(float _val, uint32_t _frame_id, uint32_t _ch) {
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

        void StreamBuffer::setValForAllChannels(float _val, uint32_t _frame_id) {
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

        void StreamBuffer::setSignalForChannel(Audio::AudioSampler *_sampler, uint32_t _ch) {
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

        void StreamBuffer::setSignalForAllChannels(Audio::AudioSampler *_sampler) {
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

        void StreamBuffer::setSignalFor2Channels(Audio::AudioSampler *_sampler) {
            switch (audio_format) {
                case RTAUDIO_SINT8: {
                    for (uint32_t i = 0; i < buffer_frames; ++i) {
                        vec2f __val = (float(maxVolume<RTAUDIO_SINT8>()) / maxVolume<RTAUDIO_SINT16>()) * _sampler->sample2().cast<float>();
                        bufInt8()[i * channels + 0] = (int8_t) __val.x;
                        bufInt8()[i * channels + 1] = (int8_t) __val.y;
                    }
                    break;
                }
                case RTAUDIO_SINT16: {
                    for (uint32_t i = 0; i < buffer_frames; ++i) {
                        vec2i16 __val = _sampler->sample2();
                        bufInt16()[i * channels + 0] = __val.x;
                        bufInt16()[i * channels + 1] = __val.y;
                    }
                    break;
                }
                case RTAUDIO_SINT32: {
                    for (uint32_t i = 0; i < buffer_frames; ++i) {
                        vec2f __val = (float(maxVolume<RTAUDIO_SINT32>()) / maxVolume<RTAUDIO_SINT16>()) * _sampler->sample2().cast<float>();
                        bufInt32()[i * channels + 0] = (int32_t) __val.x;
                        bufInt32()[i * channels + 1] = (int32_t) __val.y;
                    }
                    break;
                }
                case RTAUDIO_FLOAT32: {
                    for (uint32_t i = 0; i < buffer_frames; ++i) {
                        vec2f __val = (float(maxVolume<RTAUDIO_FLOAT32>()) / maxVolume<RTAUDIO_SINT16>()) * _sampler->sample2().cast<float>();
                        bufFlt32()[i * channels + 0] = __val.x;
                        bufFlt32()[i * channels + 1] = __val.y;
                    }
                    break;
                }
                case RTAUDIO_FLOAT64: {
                    for (uint32_t i = 0; i < buffer_frames; ++i) {
                        vec2d __val = (float(maxVolume<RTAUDIO_FLOAT64>()) / maxVolume<RTAUDIO_SINT16>()) * _sampler->sample2().cast<float>();
                        bufFlt64()[i * channels + 0] = __val.x;
                        bufFlt64()[i * channels + 1] = __val.y;
                    }
                    break;
                }
            }
        }

        void rtacbCallbackWarningCheck(RtAudioStreamStatus _status) {
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
            rtacbCallbackWarningCheck(_status);

            // Write interleaved audio data.
            AudioStreamConfig *__config = (AudioStreamConfig*)_config;

            // Note: not static - buffer pointer, frame count and stream parameters may change between calls
            StreamBuffer __outbuf_wrap{_o_buffer, _n_buf_frames, __config->device_o.nChannels, __config};

            __outbuf_wrap.setSignalForAllChannels(&__config->audio_sampler);
            return 0;
        }

        int rtacbDefault2(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
                          double _stream_t, RtAudioStreamStatus _status, void *_config) {
            rtacbCallbackWarningCheck(_status);

            // Write interleaved audio data.
            AudioStreamConfig *__config = (AudioStreamConfig*)_config;

            // Note: not static - buffer pointer, frame count and stream parameters may change between calls
            StreamBuffer __outbuf_wrap{_o_buffer, _n_buf_frames, __config->device_o.nChannels, __config};

            __outbuf_wrap.setSignalFor2Channels(&__config->audio_sampler);
            return 0;
        }

        int rtacbSawNoRepeat(void *_o_buffer, void *inputBuffer, unsigned int _n_buf_frames,
                      double _stream_t, RtAudioStreamStatus _status, void *_config) {
            AudioStreamConfig *__config = (AudioStreamConfig*)_config;
            // Note: not static - buffer pointer, frame count and stream parameters may change between calls
            StreamBuffer __outbuf_wrap{_o_buffer, _n_buf_frames, __config->device_o.nChannels, __config};
            static float __last_val[3] = { -1.f, 0.000001f, 0.000002f };

            rtacbCallbackWarningCheck(_status);

            // Write interleaved audio data.
            for (uint32_t i = 0; i < _n_buf_frames; ++i) {
                for (uint32_t j = 0; j < __outbuf_wrap.channels; ++j) {
                    __outbuf_wrap.setValForChannel(__config->getMaxVolume() * __last_val[0], i, j);
                }

                __last_val[0] += __last_val[1];
                if (std::abs(__last_val[0]) >= 1.f) {
                    __last_val[1] = -__last_val[1];
                }

                __last_val[1] += Math::sign(__last_val[1]) * __last_val[2];
                if (__last_val[1] >= 0.1f)
                    __last_val[2] = -__last_val[2];
            }

            return 0;
        }

        int rtacbSawPulse(void *_o_buffer, void *inputBuffer, unsigned int _n_buf_frames,
                      double _stream_t, RtAudioStreamStatus _status, void *_config) {
            AudioStreamConfig *__config = (AudioStreamConfig*)_config;
            // Note: not static - buffer pointer, frame count and stream parameters may change between calls
            StreamBuffer __outbuf_wrap{_o_buffer, _n_buf_frames, __config->device_o.nChannels, __config};
            static float __last_val[3] = { -1.f, 0.000001f, 1.00006f };

            rtacbCallbackWarningCheck(_status);

            // Write interleaved audio data.
            for (uint32_t i = 0; i < _n_buf_frames; ++i) {
                for (uint32_t j = 0; j < __outbuf_wrap.channels; ++j) {
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

        int rtacbSaw(void *_o_buffer, void *inputBuffer, unsigned int _n_buf_frames,
                      double _stream_t, RtAudioStreamStatus _status, void *_config) {
            AudioStreamConfig *__config = (AudioStreamConfig*)_config;
            // Note: not static - buffer pointer, frame count and stream parameters may change between calls
            StreamBuffer __outbuf_wrap{_o_buffer, _n_buf_frames, __config->device_o.nChannels, __config};
            static float __last_val[2] = { -1.f, 0.02f };
            static Audio::Sample::SmpSaw __saw_sample;
            static Audio::AudioSampler  __audio_sampler{ {&__saw_sample} };
            __audio_sampler.setSampleRate(__config->sample_rate);

            rtacbCallbackWarningCheck(_status);

            __outbuf_wrap.setSignalForAllChannels(&__audio_sampler);
            return 0;
        }

        int rtacbInOut(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
                        double _stream_t, RtAudioStreamStatus _status, void *_config) {
            rtacbCallbackWarningCheck(_status);

            AudioStreamConfig *__config = (AudioStreamConfig*)_config;

            // If the number of input and output channels is equal, we simply copy the buffer.
            if (__config->device_i.nChannels == __config->device_o.nChannels) {
                uint32_t bytes = __config->device_o.nChannels * __config->buffer_frames *
                                 __config->getBufferElemBytes();
                memcpy(_o_buffer, _i_buffer, bytes);
            }
            return 0;
        }
    }; // RTA END
}; // Simple END
