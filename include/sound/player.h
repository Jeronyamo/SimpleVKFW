#ifndef SVKFW_PLAYER_H
#define SVKFW_PLAYER_H

#include "data/codecs.h"
#include "sound/sampler.h"


namespace Simple {
    namespace Audio {
        namespace Sample {
            struct SmpAudioFile : SmpItf {
                std::vector<int16_t> pcm_out;
                std::vector<uint32_t> page_offsets;
                Codec::DecoderOpus::OpusHead opus_head;
                Codec::DecoderOpus::OpusTags opus_tags;

                SmpAudioFile(const std::string &_ogg_fpath = "");

                float sample (float _t) override;

                // Returns stereo frame starting at sample index '_id' (mono is duplicated), '_id' must be valid
                vec2i16 getFrame_(uint32_t _id) const;
                // Checks if a whole frame starting at sample index '_id' is available
                bool hasFrame_(uint32_t _id) const;

                vec2i16 sample2(double _t) override;

                vec2i16 sample2linear(double _t);

                void decodeOGG(const std::string &_ogg_fpath);
            };
        };
    }; // Audio END

}; // Simple END

#endif