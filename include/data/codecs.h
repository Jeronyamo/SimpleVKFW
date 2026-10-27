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
    }; // Codec END
}; // Simple END

#endif