#ifndef SVKFW_RTAUDIO_WRAP_H
#define SVKFW_RTAUDIO_WRAP_H

#include "RtAudio.h"
#include "sound/player.h"


namespace Simple {
    namespace RTA {
    // Device types available, based on number of channels
        enum DeviceType {
            DT_BIT_ONLY_IN  = 1 << 0,
            DT_BIT_ONLY_OUT = 1 << 1,
            DT_BIT_DUPLEX   = 1 << 2,
            DT_BIT_BOTH_IO  = 1 << 3,
        }; // DeviceType END

        enum DeviceMode {
            DEVICE_MODE_UNSPEC  , // Initial mode; changes to DEFAULT in/out mode if the device is used to open a stream, ignored otherwise
            DEVICE_MODE_DEFAULT , // Checks for default input/output device
            DEVICE_MODE_SELECTED, // Uses selected input/output device while it's available, else selects default in/out devices
            DEVICE_MODE_DUPLEX  , // Uses device with duplex channels while it's available, else falls back to DEFAULT in/out modes
        }; // DeviceMode END

        enum StreamMode {
            STREAM_MODE_UNSPEC, // Leaving stream mode unspecified is an error
            STREAM_MODE_OUT   , // Output stream
            STREAM_MODE_IN    , // Input stream
            STREAM_MODE_DUPLEX, // Input+output stream, no checks if it uses actual duplex channels of the same device.
        }; // StreamMode END


        template <RtAudioFormat _audio_format>
        inline uint32_t bufferElemBytes() {
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
        uint32_t bufferElemBytes(RtAudioFormat _audio_format);

        template <RtAudioFormat _audio_format>
        inline uint32_t maxVolume() {
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
        uint32_t maxVolume(RtAudioFormat _audio_format);


        // This class contains parameters set by a user
        struct AudioSettings {
            uint8_t volume = 255;
        }; // AudioSettings END

        // This class contains all information available to RTAudio callbacks via 'void* config' argument
        struct AudioStreamConfig {
            StreamMode stream_mode = STREAM_MODE_UNSPEC;
            uint32_t sample_rate;
            uint32_t buffer_frames; // automatically set on stream opening
            RtAudioFormat audio_format;
            AudioSettings audio_settings;
            Audio::AudioSampler audio_sampler;

            RtAudio::StreamOptions stream_options;
            RtAudio::StreamParameters device_i, device_o;
            DeviceMode device_i_mode = DEVICE_MODE_UNSPEC, device_o_mode = DEVICE_MODE_UNSPEC;

            uint32_t getBufferElemBytes() const;
            uint32_t getMaxVolume()       const;
            float    getVolume()          const; // max is 255/256 * (maxVolume)
            void updateSampleRate(uint32_t _sample_rate);
        }; // AudioStreamConfig END


        struct AudioHandler {
            RtAudio rta_handler;
            RtAudioCallback rta_cback;
            AudioStreamConfig stream_config;


            AudioHandler();
           ~AudioHandler();


            // RtAudio picks the first compiled API with devices, which is ALSA on Linux. Its "default" device goes through
            // the sound server anyway and can't open duplex streams there, so the server's own API is preferred when it runs.
            static RtAudio::Api preferredApi_();


            void callbackSet(RtAudioCallback _rta_cback);

        // Stream methods

            void streamSetMode(StreamMode _stream_mode, DeviceMode _o_device_mode = DEVICE_MODE_UNSPEC, DeviceMode _i_device_mode = DEVICE_MODE_UNSPEC);

            void streamSetOptions(RtAudioStreamFlags _s_flags, uint32_t _s_nbuffers, const std::string &_s_name = "SimpleVKFW_AudioHandler", int _s_priority = 0);

            void streamOpen(uint32_t _sample_rate, uint32_t _buf_frames, RtAudioFormat _audio_format, bool _use_options = true);

            void streamStart();
            void streamStop();
            void streamAbort();
            bool streamIsOpen();
            bool streamIsRunning();
            long streamGetLatency();
            void streamShowWarnings(bool _do_show = true);


        // Device/Input device/Output device methods

            DeviceType deviceGetAvailableTypes();

            void deviceInputSetParameters(uint32_t _nchannels, uint32_t _first_channel = 0);
            void deviceOutputSetParameters(uint32_t _nchannels, uint32_t _first_channel = 0);

            bool deviceInputIsFormatSupported(RtAudioFormat _audio_format);
            bool deviceOutputIsFormatSupported(RtAudioFormat _audio_format);

            void deviceUpdate_(RtAudio::StreamParameters &_device_io, DeviceMode &_device_io_mode, uint32_t _def_device, const char *_device_type);
            void deviceInputUpdate ();
            void deviceOutputUpdate();

            void deviceDuplexUpdate();
            void deviceUpdate();


            // Used by getInputDeviceInfoStr/getOutputDeviceInfoStr
            std::string deviceGetInfoStr_(uint32_t _device_id);
            std::string  deviceInputGetInfoStr();
            std::string deviceOutputGetInfoStr();

        // User methods

            void playAudio(Audio::Sample::SmpItf *_source, float _weight = 1.f);
        }; // AudioHandler END


        // Manages "void *buffer" from RtAudio stream callbacks. TODO: manage Int24, Flt64
        struct StreamBuffer {
            void *buffer;
            RtAudioFormat audio_format;
            uint32_t sample_rate;
            uint32_t buffer_frames;
            uint32_t channels;

            StreamBuffer(void *_buffer, uint32_t _buf_frames, uint32_t _channels, AudioStreamConfig *_config);

            int8_t * bufInt8 ();
            int16_t* bufInt16();
            int32_t* bufInt32();
            float  * bufFlt32();
            double * bufFlt64();


            void setValForChannel(float _val, uint32_t _frame_id, uint32_t _ch);

            void setValForAllChannels(float _val, uint32_t _frame_id);

            void setSignalForChannel(Audio::AudioSampler *_sampler, uint32_t _ch);

            void setSignalForAllChannels(Audio::AudioSampler *_sampler);

            void setSignalFor2Channels(Audio::AudioSampler *_sampler);
        }; // StreamBuffer END


    // Callbacks. Last argument is expected to be a pointer to AudioHandler.

        void rtacbCallbackWarningCheck(RtAudioStreamStatus _status);

        int rtacbDefault(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
                         double _stream_t, RtAudioStreamStatus _status, void *_config);

        // Default callback for 2 channels
        int rtacbDefault2(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
                          double _stream_t, RtAudioStreamStatus _status, void *_config);

        int rtacbSawNoRepeat(void *_o_buffer, void *inputBuffer, unsigned int _n_buf_frames,
                      double _stream_t, RtAudioStreamStatus _status, void *_config);

        int rtacbSawPulse(void *_o_buffer, void *inputBuffer, unsigned int _n_buf_frames,
                      double _stream_t, RtAudioStreamStatus _status, void *_config);

        int rtacbSaw(void *_o_buffer, void *inputBuffer, unsigned int _n_buf_frames,
                      double _stream_t, RtAudioStreamStatus _status, void *_config);

        int rtacbInOut(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
                        double _stream_t, RtAudioStreamStatus _status, void *_config);

        // int rtacb_record(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
        //                  double _stream_t, RtAudioStreamStatus _status, void *_config) {
        //     rtacbCallbackWarningCheck(_status);

        //     memcpy(Global_audio_buf.buf, _i_buffer, Global_audio_buf.buf_size * sizeof(float));
        //     return 0;
        // }

        // int rtacb_playback(void *_o_buffer, void *_i_buffer, unsigned int _n_buf_frames,
        //                  double _stream_t, RtAudioStreamStatus _status, void *_config) {
        //     rtacbCallbackWarningCheck(_status);

        //     memcpy(_o_buffer, Global_audio_buf.buf, Global_audio_buf.buf_size * sizeof(float));
        //     return 0;
        // }
    }; // RTA END
}; // Simple END

#endif