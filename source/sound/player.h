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

                SmpAudioFile(const std::string &_ogg_fpath = "") { if (!_ogg_fpath.empty()) decodeOGG(_ogg_fpath); }

                float sample (float _t) override {
                    uint32_t __t_step = uint32_t(_t * 48000) * opus_head.channels;
                    return __t_step < pcm_out.size() ? pcm_out[__t_step] : 0.f;
                }

                vec2i16 sample2(float _t) override {
                    uint32_t __t_step = uint32_t(_t * 48000) * opus_head.channels;
                    if (__t_step >= pcm_out.size()) return 0.f;

                    if (opus_head.channels > 1)
                        return { pcm_out[__t_step], pcm_out[__t_step+1] };
                    return {pcm_out[__t_step]};
                }

                void decodeOGG(const std::string &_ogg_fpath) {
                    File::ReaderWriterOGG ogg_reader{_ogg_fpath};
                    Codec::DecoderOpus  opus_decoder;

                    std::vector<unsigned char> __opus_packet;

                    bool __packet_ended = false, __packet_decoded = false;
                    uint32_t __segment_offset = 0u;

                    for (const auto& page : ogg_reader.file_content.pages) {
                        __segment_offset = 0u;

                        for (uint32_t i = 0u; i < page.total_segments; ++i) {
                            if (__packet_ended) {
                                __packet_decoded = opus_decoder.decodePacket(__opus_packet);
                                page_offsets.push_back(pcm_out.size());
                                pcm_out.insert(pcm_out.end(), opus_decoder.pcm_out.begin(), opus_decoder.pcm_out.begin() + opus_decoder.curr_frame_size);

                                __opus_packet.clear();
                                __packet_ended = false;
                            }

                            __opus_packet.insert(__opus_packet.end(),
                                                page.segment_data.begin() + __segment_offset,
                                                page.segment_data.begin() + __segment_offset + page.segment_table[i]);

                            __segment_offset += page.segment_table[i];
                            __packet_ended    = page.segment_table[i] < 255u;
                        }
                    }
                    if (__packet_ended) {
                        __packet_decoded = opus_decoder.decodePacket(__opus_packet);
                        page_offsets.push_back(pcm_out.size());
                        pcm_out.insert(pcm_out.end(), opus_decoder.pcm_out.begin(), opus_decoder.pcm_out.begin() + opus_decoder.curr_frame_size);
                    }
                    opus_head = opus_decoder.opus_head;
                    opus_tags = opus_decoder.opus_tags;

                    setDuration( (pcm_out.size() / opus_head.channels) / float(opus_decoder.opus_sample_rate) );

                    printf(SVKFW_WRAPINFO("Audio::Sample::SmpAudioFile :: decodeOgg", "OGG file successfully decoded.\n - Uncompressed size: %.1f MB\n - Duration: %02d:%02d:%02d\n"),
                                            pcm_out.size() * sizeof(int16_t) / float(1024 * 1024), uint32_t(std::round(duration)) / 3600, uint32_t(std::round(duration))%3600/60, uint32_t(std::round(duration))%60 );
                }
            };
        };
    }; // Audio END

}; // Simple END

#endif