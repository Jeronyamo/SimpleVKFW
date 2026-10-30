#ifndef SVKFW_CODECS_H
#define SVKFW_CODECS_H

#include "common/bytes.h"
#include "data/filetypes.h"
#include "opus/include/opus.h"

#include <map>


namespace Simple {
//  ============  Codecs  ============  \\

    namespace Codec {
        struct DecoderOpus {
            OpusDecoder* decoder = nullptr;
            int opus_sample_rate = 48000;
            int max_frame_size   = 5760;
            std::vector<int16_t> pcm_out;
            uint32_t curr_frame_size = 0u;

            struct OpusHead {
                uint8_t  version        = 0u;
                uint8_t  channels       = 0u;
                uint16_t preskip        = 0u;
                uint32_t sample_rate    = 0u;
                uint16_t gain           = 0u;
                uint8_t  mapping_family = 0u;
                uint8_t  stream_count   = 0u;
                uint8_t  coupled_count  = 0u;
                std::vector<uint8_t> mapping_table;

                std::string infoString() const;
            } opus_head;

            struct OpusTags {
                std::string vendor;
                std::map<std::string, std::string> tags;

                std::string infoString() const;
            } opus_tags;


            DecoderOpus();
           ~DecoderOpus();


            bool closeDecoder();

            bool readOpusHead(const std::vector<unsigned char> &_opus_head);

            bool readOpusTags(const std::vector<unsigned char> &_opus_tags);

            bool decodePacket(const std::vector<unsigned char> &_opus_packet);
        }; // DecoderOpus END

        // Encodes 48 kHz mono/stereo samples into an Ogg Opus stream (RFC 7845)
        struct EncoderOpus {
            OpusEncoder* encoder = nullptr;
            int opus_sample_rate = 48000;
            int channels         = 0;
            int frame_size       = 960;    // 20 ms
            int bitrate          = 128000; // Bits per second
            int complexity       = 10;     // [0, 10]
            int preskip          = 0;      // Encoder lookahead, written to OpusHead
            std::string vendor   = "SimpleVKFW";
            std::map<std::string, std::string> tags; // OpusTags comments, e.g. TITLE, ARTIST

            EncoderOpus();
           ~EncoderOpus();

            bool createEncoder(int _channels, int _application = OPUS_APPLICATION_AUDIO);

            bool closeEncoder();

            std::vector<unsigned char> makeOpusHead() const;

            std::vector<unsigned char> makeOpusTags() const;

            // Appends a whole logical stream (headers, audio, end of stream) to '_ogg'.
            // '_samples' are interleaved, '_num_frames' frames of 'channels' samples.
            bool encodeOGG(const float *_samples, size_t _num_frames, File::ContentOGG &_ogg, uint32_t _serial = 0x53564B46u);
        }; // EncoderOpus END
    }; // Codec END
}; // Simple END

#endif