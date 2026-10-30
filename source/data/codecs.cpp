#include "data/codecs.h"

#include <algorithm>

namespace Simple {
    namespace Codec {

        std::string DecoderOpus::OpusHead::infoString() const {
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

        std::string DecoderOpus::OpusTags::infoString() const {
            std::string __res_info;

            __res_info += "OpusTags Packet:\n";
            __res_info += " - Vendor: '" + vendor + "'\n";
            __res_info += " - Tags:\n";
            for (auto key_val : tags)
                __res_info += "        '" + key_val.first + "' = '" + key_val.second + "'\n";

            return __res_info;
        }

        DecoderOpus::DecoderOpus() {}

        DecoderOpus::~DecoderOpus() { closeDecoder(); }

        bool DecoderOpus::closeDecoder() {
            if (decoder != nullptr) {
                opus_decoder_destroy(decoder);
                decoder = nullptr;
            }
            return false;
        }

        bool DecoderOpus::readOpusHead(const std::vector<unsigned char> &_opus_head) {
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
                opus_head.mapping_table.resize(opus_head.channels);
                __res_success &= __opus_reader.readArray(opus_head.mapping_table.data(), opus_head.channels, false);
            }

            std::string __opushead_info = opus_head.infoString();
            printf("%s\n", __opushead_info.c_str());

            SVKFW_ASSERT(__res_success, std::runtime_error, "Codec::DecoderOpus :: readOpusHead", "Failed to read");

            int __opus_error = 0;
            closeDecoder(); // OpusHead may appear again (chained streams)
            decoder = opus_decoder_create(opus_sample_rate, opus_head.channels, &__opus_error);
            SVKFW_ASSERT(!__opus_error, std::runtime_error, "Codec::DecoderOpus :: readOpusHead",
                            "Could not create Opus decoder (Error: " + std::string(opus_strerror(__opus_error)) + ")");
            pcm_out.resize(max_frame_size * opus_head.channels);
            return __res_success;
        }

        bool DecoderOpus::readOpusTags(const std::vector<unsigned char> &_opus_tags) {
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
            __res_success &= __opus_reader.readArray(&opus_tags.vendor[0], __vendor_str_len, false);

            uint32_t __n_tags = 0u;
            __res_success &= __opus_reader.readValue(__n_tags, Bytes::Endianness.is_big_endian);
            for (uint32_t i = 0u; __res_success && i < __n_tags; ++i) {
                uint32_t __tag_str_len = 0u;
                __res_success &= __opus_reader.readValue(__tag_str_len, Bytes::Endianness.is_big_endian);

                // Note: readArray checks bounds and moves the stream offset to the next tag
                std::string __tag_str(size_t(__tag_str_len), '\0');
                __res_success = __res_success && __opus_reader.readArray(&__tag_str[0], __tag_str_len, false);
                if (!__res_success) break;

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

        bool DecoderOpus::decodePacket(const std::vector<unsigned char> &_opus_packet) {
            char __opus_str[9]{};

            if (_opus_packet.size() >= 8) {
                memcpy(__opus_str, _opus_packet.data(), 8);
                if (!std::strcmp(__opus_str, "OpusHead"))
                    return readOpusHead(_opus_packet);
                if (!std::strcmp(__opus_str, "OpusTags"))
                    return readOpusTags(_opus_packet);
            }
            if (decoder == nullptr) {
                curr_frame_size = 0u;
                fprintf(svkfwwarn, SVKFW_WRAPWARN("Codec::DecoderOpus :: decodePacket", "Audio packet before 'OpusHead', decoder isn't created\n"));
                return false;
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

        EncoderOpus::EncoderOpus() {}

        EncoderOpus::~EncoderOpus() { closeEncoder(); }

        bool EncoderOpus::createEncoder(int _channels, int _application) {
            closeEncoder();
            if (_channels < 1 || _channels > 2) {
                fprintf(svkfwwarn, SVKFW_WRAPWARN("Codec::EncoderOpus :: createEncoder", "Only mono and stereo are supported, got %d channels\n"), _channels);
                return false;
            }

            int __opus_error = 0;
            encoder = opus_encoder_create(opus_sample_rate, _channels, _application, &__opus_error);
            if (__opus_error != OPUS_OK || encoder == nullptr) {
                fprintf(svkfwwarn, SVKFW_WRAPWARN("Codec::EncoderOpus :: createEncoder", "Could not create Opus encoder (Error: %s)\n"), opus_strerror(__opus_error));
                encoder = nullptr;
                return false;
            }
            channels = _channels;
            opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
            opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(complexity));

            opus_int32 __lookahead = 0;
            opus_encoder_ctl(encoder, OPUS_GET_LOOKAHEAD(&__lookahead));
            preskip = int(__lookahead);
            return true;
        }

        bool EncoderOpus::closeEncoder() {
            if (encoder != nullptr) {
                opus_encoder_destroy(encoder);
                encoder = nullptr;
            }
            return false;
        }

        std::vector<unsigned char> EncoderOpus::makeOpusHead() const {
            std::vector<unsigned char> __res_packet;
            Bytes::BytestreamWriter __opus_writer{__res_packet};
            const bool __swap = Bytes::Endianness.is_big_endian;

            __opus_writer.writeArray("OpusHead", 8, false);
            __opus_writer.writeValue(uint8_t (1u)              , __swap); // Version
            __opus_writer.writeValue(uint8_t (channels)        , __swap);
            __opus_writer.writeValue(uint16_t(preskip)         , __swap);
            __opus_writer.writeValue(uint32_t(opus_sample_rate), __swap); // Input sample rate
            __opus_writer.writeValue(int16_t (0)               , __swap); // Output gain
            __opus_writer.writeValue(uint8_t (0u)              , __swap); // Mapping family 0: mono/stereo
            return __res_packet;
        }

        std::vector<unsigned char> EncoderOpus::makeOpusTags() const {
            std::vector<unsigned char> __res_packet;
            Bytes::BytestreamWriter __opus_writer{__res_packet};
            const bool __swap = Bytes::Endianness.is_big_endian;

            __opus_writer.writeArray("OpusTags", 8, false);
            __opus_writer.writeValue(uint32_t(vendor.size()), __swap);
            __opus_writer.writeArray(vendor.data(), uint32_t(vendor.size()), false);
            __opus_writer.writeValue(uint32_t(tags.size()), __swap);
            for (const auto &key_val : tags) {
                const std::string __tag = key_val.first + '=' + key_val.second;
                __opus_writer.writeValue(uint32_t(__tag.size()), __swap);
                __opus_writer.writeArray(__tag.data(), uint32_t(__tag.size()), false);
            }
            return __res_packet;
        }

        bool EncoderOpus::encodeOGG(const float *_samples, size_t _num_frames, File::ContentOGG &_ogg, uint32_t _serial) {
            if (encoder == nullptr) {
                fprintf(svkfwwarn, SVKFW_WRAPWARN("Codec::EncoderOpus :: encodeOGG", "Encoder isn't created\n"));
                return false;
            }

            // Header pages: OpusHead and OpusTags each end their page, granule position 0
            _ogg.addPacket(_serial, makeOpusHead(), 0u, true);
            _ogg.addPacket(_serial, makeOpusTags(), 0u, true);

            // The decoder drops 'preskip' samples, so the input is padded until the last real sample is out.
            // The last granule position (preskip + length) trims the padding.
            const uint64_t __end_granule = uint64_t(preskip) + _num_frames;
            const size_t   __num_packets = size_t((__end_granule + frame_size - 1u) / frame_size);

            std::vector<float> __frame(size_t(frame_size) * channels);
            std::vector<unsigned char> __packet(4000u);
            for (size_t p = 0u; p < __num_packets; ++p) {
                const size_t __first = p * size_t(frame_size);
                const size_t __count = __first < _num_frames ? std::min(size_t(frame_size), _num_frames - __first) : 0u;
                std::fill(__frame.begin(), __frame.end(), 0.f);
                if (__count) std::copy(_samples + __first * channels, _samples + (__first + __count) * channels, __frame.begin());

                __packet.resize(4000u);
                const opus_int32 __bytes = opus_encode_float(encoder, __frame.data(), frame_size, __packet.data(), opus_int32(__packet.size()));
                if (__bytes < 0) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Codec::EncoderOpus :: encodeOGG", "Error encoding packet %u - %s\n"), unsigned(p), opus_strerror(__bytes));
                    return false;
                }
                __packet.resize(size_t(__bytes));

                const bool __last = p + 1u == __num_packets;
                const uint64_t __granule = __last ? __end_granule : uint64_t(p + 1u) * frame_size;
                _ogg.addPacket(_serial, __packet, __granule, false, __last);
            }
            return true;
        }
    }; // Codec END
}; // Simple END
