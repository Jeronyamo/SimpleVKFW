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
                uint8_t  version;
                uint8_t  channels;
                uint16_t preskip;
                uint32_t sample_rate;
                uint16_t gain;
                uint8_t  mapping_family;
                uint8_t  stream_count;
                uint8_t  coupled_count;
                std::vector<uint8_t> mapping_table;

                std::string infoString() const {
                    std::string __res_info;

                    __res_info += "OpusHead packet:\n";
                    __res_info += " - Version:        " + std::to_string(version       ) + '\n';
                    __res_info += " - Channels:       " + std::to_string(channels      ) + '\n';
                    __res_info += " - Pre-skip:       " + std::to_string(preskip       ) + '\n';
                    __res_info += " - Sample rate:    " + std::to_string(sample_rate   ) + '\n';
                    __res_info += " - Gain:           " + std::to_string(gain          ) + '\n';
                    __res_info += " - Mapping family: " + std::to_string(mapping_family) + '\n';
                    __res_info += " - Stream count:   " + std::to_string(stream_count  ) + '\n';
                    __res_info += " - Coupled count:  " + std::to_string(coupled_count ) + '\n';
                    if (!mapping_table.empty()) {
                        __res_info += " - Mapping table: ";
                        for (uint8_t map_val : mapping_table)
                            __res_info += ' ' + std::to_string(map_val) + ',';
                        __res_info.back() = '\n';
                    }
                    return __res_info;
                }
            } opus_head;

            struct OpusTags {
                std::string vendor;
                std::map<std::string, std::string> tags;

                std::string infoString() const {
                    std::string __res_info;

                    __res_info += "OpusTags Packet:\n";
                    __res_info += " - Vendor: '" + vendor + "'\n";
                    __res_info += " - Tags:\n";
                    for (auto key_val : tags)
                        __res_info += "        '" + key_val.first + "' = '" + key_val.second + "'\n";

                    return __res_info;
                }
            } opus_tags;


            DecoderOpus() {}
           ~DecoderOpus() { closeDecoder(); }


            bool closeDecoder() {
                if (decoder != nullptr) {
                    opus_decoder_destroy(decoder);
                    decoder = nullptr;
                }
                return false;
            }

            bool readOpusHead(const std::vector<unsigned char> &_opus_head) {
                Bytes::BytestreamReader __opus_reader{_opus_head};
                bool __res_success = true;

                // Read 'OpusHead'
                char __opushead_str[9]{};
                __res_success &= __opus_reader.readArray(__opushead_str, 8, Bytes::Endianness.is_big_endian);

                if (std::strcmp(__opushead_str, "OpusHead")) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Codec::DecoderOpus :: readOpusHead", "Packet doesn't start with 'OpusHead'\n"));
                    return false;
                }

                __res_success &= __opus_reader.readValue(opus_head.version       , Bytes::Endianness.is_big_endian);
                __res_success &= __opus_reader.readValue(opus_head.channels      , Bytes::Endianness.is_big_endian);
                __res_success &= __opus_reader.readValue(opus_head.preskip       , Bytes::Endianness.is_big_endian);
                __res_success &= __opus_reader.readValue(opus_head.sample_rate   , Bytes::Endianness.is_big_endian);
                __res_success &= __opus_reader.readValue(opus_head.gain          , Bytes::Endianness.is_big_endian);
                __res_success &= __opus_reader.readValue(opus_head.mapping_family, Bytes::Endianness.is_big_endian);

                if (__res_success && opus_head.mapping_family) {
                    __res_success &= __opus_reader.readValue(opus_head. stream_count, Bytes::Endianness.is_big_endian);
                    __res_success &= __opus_reader.readValue(opus_head.coupled_count, Bytes::Endianness.is_big_endian);
                    opus_head.mapping_table.insert(opus_head.mapping_table.end(), __opus_reader.getData(),
                                                                                  __opus_reader.getData() + opus_head.channels);
                }

                std::string __opushead_info = opus_head.infoString();
                printf("%s\n", __opushead_info.c_str());

                SVKFW_ASSERT(__res_success, std::runtime_error, "Codec::DecoderOpus :: readOpusHead", "Failed to read");

                int __opus_error = 0;
                decoder = opus_decoder_create(opus_sample_rate, opus_head.channels, &__opus_error);
                SVKFW_ASSERT(!__opus_error, std::runtime_error, "Codec::DecoderOpus :: readOpusHead",
                                "Could not create Opus decoder (Error: " + std::string(opus_strerror(__opus_error)) + ")");
                pcm_out.resize(max_frame_size * opus_head.channels);
                return __res_success;
            }

            bool readOpusTags(const std::vector<unsigned char> &_opus_tags) {
                Bytes::BytestreamReader __opus_reader{_opus_tags};
                bool __res_success = true;

                // Read 'OpusTags'
                char __opustags_str[9]{};
                __res_success &= __opus_reader.readArray(__opustags_str, 8, Bytes::Endianness.is_big_endian);

                if (std::strcmp(__opustags_str, "OpusTags")) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Codec::DecoderOpus :: readOpusTags", "Packet doesn't start with 'OpusTags'\n"));
                    return false;
                }

                uint32_t __vendor_str_len = 0u;
                __res_success &= __opus_reader.readValue(__vendor_str_len, Bytes::Endianness.is_big_endian);

                opus_tags.vendor.resize(__vendor_str_len);
                memcpy(&opus_tags.vendor[0], __opus_reader.getData(), __vendor_str_len);
                __res_success &= __opus_reader.moveOffset(__vendor_str_len);

                uint32_t __n_tags = 0u;
                __res_success &= __opus_reader.readValue(__n_tags, Bytes::Endianness.is_big_endian);
                for (uint32_t i = 0u; i < __n_tags; ++i) {
                    uint32_t __tag_str_len = 0u;
                    __res_success &= __opus_reader.readValue(__tag_str_len, Bytes::Endianness.is_big_endian);

                    std::string __tag_str(size_t(__tag_str_len), '\0');
                    memcpy(&__tag_str[0], __opus_reader.getData(), __tag_str_len);
                    __tag_str_len = __tag_str.find('=');
                    if (__tag_str_len > __tag_str.size()) {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("Codec::DecoderOpus :: readOpusTags", "Could not separate tag line: '%s'\n"), __tag_str.c_str());
                        continue;
                    }
                    opus_tags.tags[__tag_str.substr(0u, __tag_str_len)] = __tag_str.substr(__tag_str_len+1);
                }

                std::string __opustags_info = opus_tags.infoString();
                printf("%s\n", __opustags_info.c_str());

                return __res_success;
            }

            bool decodePacket(const std::vector<unsigned char> &_opus_packet) {
                char __opus_str[9]{};

                if (_opus_packet.size() >= 8) {
                    memcpy(__opus_str, _opus_packet.data(), 8);
                    if (!std::strcmp(__opus_str, "OpusHead"))
                        return readOpusHead(_opus_packet);
                    if (!std::strcmp(__opus_str, "OpusTags"))
                        return readOpusTags(_opus_packet);
                }
                int samples_decoded_per_channel = opus_decode(decoder, _opus_packet.data(), _opus_packet.size(),
                                                                        pcm_out.data(), max_frame_size, 0);
                curr_frame_size = samples_decoded_per_channel * opus_head.channels;
                if (samples_decoded_per_channel < 0) {
                    curr_frame_size = 0u;
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Codec::DecoderOpus :: decodePacket", "Error decoding audio packet - %s\n"), opus_strerror(samples_decoded_per_channel));
                }
                return samples_decoded_per_channel > 0;
            }
        }; // DecoderOpus END
    }; // Codec END
}; // Simple END

#endif