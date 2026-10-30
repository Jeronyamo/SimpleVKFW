#ifndef SVKFW_FILETYPES_H
#define SVKFW_FILETYPES_H

#include <map>
#include <vector>

#include "common/terminal.h"
#include "math/linalg.h"
#include "platform_dependent/pd_filetypes.h"


namespace Simple {
    namespace File {
        enum Filetype : uint32_t {
            NONE,
            BIN,
            VMD,
            OGG,
            WAV,

            UNKNOWN = UINT32_MAX
        }; // Filetype END


// Interfaces

        struct FiletypeContentItf { virtual ~FiletypeContentItf(); };

        // Tries to read all
        struct FiletypeReaderWriterItf {
            virtual ~FiletypeReaderWriterItf();

            virtual bool  read(const std::string &_fpath) = 0;
            virtual bool write(const std::string &_fpath) = 0;
            // Used when file reader has to recognize a file without extension
            virtual bool tryHeaders(File::ReaderItf &_ftype_file) = 0;
        }; // FiletypeReaderWriter END

// Utilities

        bool pathHasExtension(const std::string &_fpath);

        std::string pathGetExtension(const std::string &_fpath);

        std::string pathRemoveExtension(const std::string &_fpath);


// Implementations

        struct ContentVMD : FiletypeContentItf {
            struct BoneKeyframe {
                vec3f position;
                uint32_t frame_id;
                Affine::Rotation::Quaternion rotation;
                char   bone_name[16]; // Actually 15 + '\0'
                uint8_t int_info[64]; // interpolation info
                // Interpolation info (in bytes) in range [0,127]:
                //  0-15: MovX.0x,MovY.0x,      0,     0, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
                // 16-30:         MovY.0x,MovZ.0x,Rot.0x, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
                // 31-45:               0,MovZ.0x,Rot.0x, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
                // 46-60:               0,      0,Rot.0x, MovX.0y,MovY.0y,MovZ.0y,Rot.0y, MovX.1x,MovY.1x,MovZ.1x,Rot.1x, MovX.1y,MovY.1y,MovZ.1y,Rot.1y
                // 61-63:               0,      0,     0
            };

            struct MorphKeyframe {
                uint32_t frame_id;
                float weight;
                char morph_name[16]; // Actually 15 + '\0'
            };

            struct CameraKeyframe {
                vec3f position;
                uint32_t frame_id;
                vec3f rotation;
                float distance;
                uint32_t fov;
                uint32_t perspect; // 1 byte, uint32_t for padding
                uint8_t int_info[24]{};
                // Interpolation info (in bytes) in range [0,127]:
                //  0- 3:  moveX.0x, moveX.1x, moveX.0y, moveX.1y
                //  4- 7:  moveY.0x, moveY.1x, moveY.0y, moveY.1y
                //  8-11:  moveZ.0x, moveZ.1x, moveZ.0y, moveZ.1y
                // 12-15:    Rot.0x,   Rot.1x,   Rot.0y,   Rot.1y
                // 16-19:   Dist.0x,  Dist.1x,  Dist.0y,  Dist.1y
                // 20-23:    FOV.0x,   FOV.1x,   FOV.0y,   FOV.1y
            };

            struct AttrAdjust {
                vec4f pos_dist;
                vec4f rot_fov; // xyz - yaw/pitch/roll, in degrees

                AttrAdjust(bool _is_offset_scale = true);
                AttrAdjust(const vec3f &_pos, const vec3f &_rot,
                        float _cam_dist = 0.f, float _cam_fov = 0.f);

                void defScale();
                void defOffset();
            };

            struct  BoneAttrClampMinmax {
                vec3f pos_min{ 1e6f};
                vec3f rot_min{ 1e6f}; // yaw/pitch/roll, in degrees
                vec3f pos_max{-1e6f};
                vec3f rot_max{-1e6f}; // yaw/pitch/roll, in degrees

                BoneAttrClampMinmax(const vec3f &_rot_min = {1e6f}, const vec3f &_rot_max = {-1e6f},
                                    const vec3f &_pos_min = {1e6f}, const vec3f &_pos_max = {-1e6f});
            };
            struct MorphAttrClampMinmax {
                float val_min = 1e6f,
                      val_max =-1e6f;

                MorphAttrClampMinmax(float _val_min = 1e6f, float _val_max = -1e6f);
            };

            struct InterpolationInfo {
                vec4u8 mov_x{(uint8_t)20, (uint8_t)107, (uint8_t)20, (uint8_t)107};
                vec4u8 mov_y{(uint8_t)20, (uint8_t)107, (uint8_t)20, (uint8_t)107};
                vec4u8 mov_z{(uint8_t)20, (uint8_t)107, (uint8_t)20, (uint8_t)107};
                vec4u8   rot{(uint8_t)20, (uint8_t)107, (uint8_t)20, (uint8_t)107};
                vec4u8  dist{(uint8_t)20, (uint8_t)107, (uint8_t)20, (uint8_t)107};
                vec4u8   fov{(uint8_t)20, (uint8_t)107, (uint8_t)20, (uint8_t)107};
            };

        // Data

            char header[30];
            char model[20];
            bool is_old_format;

            static const std::map<std::string, std::string> bone_name_mappings;
            static const std::map<std::string, std::string> morph_name_mappings;
            static const std::map<std::string, std::string> bone_name_mappings_fromraw;
            static const std::map<std::string, std::string> morph_name_mappings_fromraw;

            std::vector<BoneKeyframe> keyframes_bone;
            std::vector<MorphKeyframe> keyframes_morph;
            std::vector<CameraKeyframe> keyframes_camera;

        // Methods

            static bool       BoneMapped(const std::string & _bone_name);
            static bool      MorphMapped(const std::string &_morph_name);
            static bool  EqToBoneMapping(const std::string & _bone_name, const std::string &_comp_name);
            static bool EqToMorphMapping(const std::string &_morph_name, const std::string &_comp_name);
            static bool IsLeftBone  (const std::string & _bone_name);
            static bool IsRightBone (const std::string & _bone_name);
            static bool IsLeftMorph (const std::string &_morph_name);
            static bool IsRightMorph(const std::string &_morph_name);
            static std::string GetBoneNameSafe(const std::string &_raw_bone_name);
            static std::string GetMorphNameSafe(const std::string &_raw_morph_name);

            static std::string GetOppositeBone(const std::string &  _bone_name);
            static std::string GetOppositeMorph(const std::string & _morph_name);
            static bool GetOppositeNameInplace(char *_bone_morph_name);
            // Reflect through a plane X=0
            static void ReflectBoneKeyframe(BoneKeyframe &_bone_kf);
            static void ReflectMorphKeyframe(MorphKeyframe &_morph_kf);
            // Reflect through a plane X=0
            static void ReflectCameraKeyframe(CameraKeyframe &_camera_kf);

            void sortBoneKeyframes();

            void sortMorphKeyframes();

            // Reflect through a plane X=0
            void reflectMovement();

            void multKeyframes(const std::map<std::string, AttrAdjust> &_mult_bones,
                               const std::map<std::string,      float> &_mult_morphs,
                               AttrAdjust _cam_mult = {});
            void offsetKeyframes(const std::map<std::string, AttrAdjust> &_offset_bones,
                                 const std::map<std::string,      float> &_offset_morphs,
                                 AttrAdjust _cam_offset = {});

            void clampFrames(const std::vector<std::pair<std::string,  BoneAttrClampMinmax>> &_clamp_bones,
                             const std::vector<std::pair<std::string, MorphAttrClampMinmax>> &_clamp_morphs);

            // One biquad section state (per scalar channel)
            struct BiquadState {
                float z1 = 0.f, z2 = 0.f; // transposed direct-form II states
            };
            struct BiquadCoeffs {
                float b0, b1, b2, a1, a2;
            };

            static BiquadCoeffs makeLowpassBiquad(float fs, float fc, float Q);

            // Process one scalar sample through one biquad section (transposed DF-II)
            static float biquadStep(const BiquadCoeffs& c, BiquadState& s, float x);

            void butterworthLowpass4(std::vector<vec3f> &data, float _freq_sample_hz, float _freq_cutout_hz, bool _is_forward);

            float findU(float _t, float _x1, float _x2);
            float interpolateForMMD(float _t, vec2f _p1, vec2f _p2);

            void reinterpolateBone(const std::string &_raw_bone_name);

            void reinterpolateBones();

            // pos, tangent, fov, dist;
            typedef vec4f CRDataT;
            struct CRDataP {
                vec3f pos, tangent;
                float fov, dist;

                CRDataP();
                CRDataP(const CameraKeyframe &_cam_kframe);
                CRDataP(const vec3f &_pos, const vec3f &_tang, float _fov, float _dist);

                ContentVMD::CRDataP operator+(const ContentVMD::CRDataP&_op);
                ContentVMD::CRDataP operator-(const ContentVMD::CRDataP&_op);
                ContentVMD::CRDataP operator*(const ContentVMD::CRDataP&_op);
                ContentVMD::CRDataP operator/(const ContentVMD::CRDataP&_op);
                ContentVMD::CRDataP operator+(const ContentVMD::CRDataT&_op);
                ContentVMD::CRDataP operator-(const ContentVMD::CRDataT&_op);
                ContentVMD::CRDataP operator*(const ContentVMD::CRDataT&_op);
                ContentVMD::CRDataP operator/(const ContentVMD::CRDataT&_op);
            };

            static vec4f DataPtoT(CRDataP _data_new, CRDataP _data_old);

            void reinterpolateCamera(bool _bake_path = false);

            void mixCamera(const ContentVMD &_other_camera, const std::vector<uint32_t> &_switch_frame_ids);


            static InterpolationInfo GetInterpolation(BoneKeyframe &_bone_kframe);
            static InterpolationInfo GetInterpolation(CameraKeyframe &_cam_kframe);
            // Interpolation example - mov_x: x = 0.x, y = 0.y, z = 1.x, w = 1.y 
            static void SetInterpolation(BoneKeyframe &_bone_kframe, const InterpolationInfo &_int_info);
            // mov_x: x = 0.x, y = 0.y, z = 1.x, w = 1.y 
            static void SetInterpolation(CameraKeyframe &_cam_kframe, const InterpolationInfo &_int_info);

            static void SetNameFromMapping(BoneKeyframe &_bone_kframe, std::string _mapping_str);
            static void   ModifyData(const std::string &_vmd_path, bool _fix_blink = true, bool _make_reflected = true, bool _reinterpolate = true);
            static void ModifyCamera(const std::string &_vmd_path, bool _reinterpolate = true);
            static void CreateMixed(const std::string &_motion_vmd_path, const std::string &_camera_vmd_path,
                                    const std::vector<uint32_t> &_split_frame_ids, const AttrAdjust &_motion1_offset,
                                    const AttrAdjust &_camera1_scale, const AttrAdjust &_camera1_offset,
                                    const AttrAdjust &_camera2_scale, const AttrAdjust &_camera2_offset, bool _fix_blink = true);
        }; // ContentVMD END

        // ContentVMD::CRDataP operator+(const ContentVMD::CRDataT&_op1, const ContentVMD::CRDataP&_op2)
        //     { return { _op1.x + _op2.pos, _op1.y + _op2.tangent, _op1.z + _op2.fov, _op1.w + _op2.dist }; }
        // ContentVMD::CRDataP operator-(const ContentVMD::CRDataT&_op1, const ContentVMD::CRDataP&_op2)
        //     { return { _op1.x - _op2.pos, _op1.y - _op2.tangent, _op1.z - _op2.fov, _op1.w - _op2.dist }; }
        // ContentVMD::CRDataP operator*(const ContentVMD::CRDataT&_op1, const ContentVMD::CRDataP&_op2)
        //     { return { _op1.x * _op2.pos, _op1.y * _op2.tangent, _op1.z * _op2.fov, _op1.w * _op2.dist }; }
        // ContentVMD::CRDataP operator/(const ContentVMD::CRDataT&_op1, const ContentVMD::CRDataP&_op2)
        //     { return { _op1.x / _op2.pos, _op1.y / _op2.tangent, _op1.z / _op2.fov, _op1.w / _op2.dist }; }


        struct ReaderWriterVMD : FiletypeReaderWriterItf {
            ContentVMD file_content{};

            ReaderWriterVMD(const std::string &_fpath = "");

            virtual bool read(const std::string &_fpath) override;

            virtual bool write(const std::string &_fpath) override;

            bool tryHeaders(ReaderItf &_ftype_file) override;
        }; // ReaderWriterVMD END

        // VMD: init mappings

         // bone_name_mappings END

         // morph_name_mappings END

        // VMD: init reversed mappings

         // bone_name_mappings END

         // morph_name_mappings END


    // OGG

        struct ContentOGG : FiletypeContentItf {
            struct PageOGG {
                char signature[4]; // these 4 bytes must be "OggS"
                uint8_t  version;
                uint8_t  header_flags;
                uint64_t granule_position;
                uint32_t serial_number;
                uint32_t sequence_number;
                uint32_t checksum;
                uint8_t  total_segments;
                uint8_t  segment_table[256];
                std::vector<unsigned char> segment_data; // segment_data.size() == total_segments, i-th segment data acquired by calculating offset (it's size is in segment_table[i])
            }; // PageOGG END
            std::vector<PageOGG> pages;
            bool page_open = false; // Writing: the last page still takes packets

            // CRC-32 of a page (polynomial 0x04C11DB7) as stored in its checksum field
            static uint32_t Checksum(const PageOGG &_page);

            // Writing: laces a packet of the logical stream '_serial' into pages.
            // '_granule_position' is the stream position after the packet ends. A page is closed
            // when it is full, when '_flush' is set or at '_end_of_stream' (the packet is the last one).
            void addPacket(uint32_t _serial, const std::vector<unsigned char> &_packet,
                           uint64_t _granule_position, bool _flush = false, bool _end_of_stream = false);
            // Closes the open page and sets its checksum
            void flushPage();
        }; // ContentOGG END

        struct ReaderWriterOGG : FiletypeReaderWriterItf {
            ContentOGG file_content;

            ReaderWriterOGG(const std::string &_fpath = "");

            virtual bool read(const std::string &_fpath) override;
            virtual bool write(const std::string &_fpath) override;

            bool tryHeaders(ReaderItf &_ftype_file) override;
        }; // ReaderWriterOGG END


    // WAV

        // Format tags of the 'fmt ' chunk
        enum WAVFormatTag : uint16_t {
            WAV_FORMAT_PCM        = 0x0001u,
            WAV_FORMAT_IEEE_FLOAT = 0x0003u,
            WAV_FORMAT_ALAW       = 0x0006u,
            WAV_FORMAT_MULAW      = 0x0007u,
            WAV_FORMAT_IMA_ADPCM  = 0x0011u,
            WAV_FORMAT_EXTENSIBLE = 0xFFFEu, // Real tag is in the first 2 bytes of the sub-format GUID
        }; // WAVFormatTag END

        // Converts samples between floats in [-1, 1] and the bytes of a 'data' chunk.
        // There is one codec per format tag, see ContentWAV::RegisterCodec.
        struct WAVCodecItf {
            virtual ~WAVCodecItf();

            virtual uint16_t formatTag() const = 0;
            virtual bool supportsBits(uint16_t _bits_per_sample) const = 0;
            // Bytes per frame of all channels ('nBlockAlign')
            virtual uint16_t blockAlign(uint16_t _channels, uint16_t _bits_per_sample) const = 0;
            // Samples are interleaved by channel
            virtual bool encode(const float *_samples, size_t _num_samples, uint16_t _bits_per_sample, std::vector<uint8_t> &_out_data) const = 0;
            virtual bool decode(const uint8_t *_data, size_t _num_bytes, uint16_t _bits_per_sample, std::vector<float> &_out_samples) const = 0;
        }; // WAVCodecItf END

        // Integer PCM: 8 bit unsigned, 16/24/32 bit signed. Samples are clamped and truncated.
        struct WAVCodecPCM : WAVCodecItf {
            virtual uint16_t formatTag() const override;
            virtual bool supportsBits(uint16_t _bits_per_sample) const override;
            virtual uint16_t blockAlign(uint16_t _channels, uint16_t _bits_per_sample) const override;
            virtual bool encode(const float *_samples, size_t _num_samples, uint16_t _bits_per_sample, std::vector<uint8_t> &_out_data) const override;
            virtual bool decode(const uint8_t *_data, size_t _num_bytes, uint16_t _bits_per_sample, std::vector<float> &_out_samples) const override;
        }; // WAVCodecPCM END

        struct ContentWAV : FiletypeContentItf {
            uint16_t format_tag      = WAV_FORMAT_PCM; // Never WAV_FORMAT_EXTENSIBLE, the writer chooses the layout
            uint16_t channels        = 2u;
            uint32_t sample_rate     = 48000u;
            uint16_t bits_per_sample = 16u;
            std::vector<uint8_t> data; // 'data' chunk as stored in the file

            // Codecs by format tag. PCM is registered by default, a registered codec must stay alive.
            static void RegisterCodec(const WAVCodecItf *_codec);
            static const WAVCodecItf* FindCodec(uint16_t _format_tag);

            uint16_t blockAlign() const;
            uint32_t numFrames() const;

            // Encode with the codec of 'format_tag' and 'bits_per_sample', samples are interleaved by 'channels'
            bool setSamples(const float *_samples, size_t _num_frames);
            bool setSamples(const std::vector<vec2f> &_frames); // Sets 'channels' to 2
            bool getSamples(std::vector<float> &_out_samples) const;
        }; // ContentWAV END

        struct ReaderWriterWAV : FiletypeReaderWriterItf {
            ContentWAV file_content;

            ReaderWriterWAV(const std::string &_fpath = "");

            virtual bool read(const std::string &_fpath) override;
            virtual bool write(const std::string &_fpath) override;

            bool tryHeaders(ReaderItf &_ftype_file) override;
        }; // ReaderWriterWAV END


    // JSON


        namespace Util {
            std::pair<uint32_t, uint32_t> parseWords(const std::vector<std::string> &_words, const char *_json_str);
            uint32_t skipWhitespace(const char *_json_str);
        };

        enum JSONType {
            JSON_TYPE_NULL = 1,
            JSON_TYPE_BOOL = 2,
            JSON_TYPE_NUM  = 4,
            JSON_TYPE_STR  = 8,
            JSON_TYPE_ARR  = 16,
            JSON_TYPE_OBJ  = 32,
        }; // JSONType END


        struct ContentJSON : FiletypeContentItf {
            struct JSONNode {
                JSONType node_type = JSON_TYPE_NULL;
                union {
                    bool value_bool;
                    double value_num;
                    std::string value_str;
                    std::vector<JSONNode> value_arr_obj;
                };

                JSONNode();
                JSONNode(const JSONNode &_node);
                ~JSONNode();

                const JSONNode& operator=(const JSONNode& _node);

                void removeValue();

            // Switch Active Value

                void switchValueNull();
                void switchValueBool();
                void switchValueNum ();
                void switchValueStr ();
                void switchValueArr ();
                void switchValueObj ();

            // Set Value

                void setValueNull();
                void setValueBool(bool _value);
                void setValueStr(const std::string &_value);
                void setValueNum(double _value);
                void setValueArr(const std::vector<JSONNode> &_value);
                void setValueObj(const JSONNode &_key, const JSONNode &_value);

            // Read Value From String

                uint32_t readValueNull(const char *_json_str);
                uint32_t readValueBool(const char *_json_str);
                uint32_t readValueNum (const char *_json_str);
                uint32_t readValueStr (const char *_json_str);
                uint32_t readValueArr (const char *_json_str);
                uint32_t readValueObj (const char *_json_str);

                uint32_t readValue(const char *_json_str);

            // Write Values To String
                std::vector<std::string> writeValueNull(uint32_t _indent = 0u);
                std::vector<std::string> writeValueBool(uint32_t _indent = 0u);
                std::vector<std::string> writeValueNum (uint32_t _indent = 0u);
                std::vector<std::string> writeValueStr (uint32_t _indent = 0u);

                std::vector<std::string> writeValueArr (uint32_t _indent = 0u);
                std::vector<std::string> writeValueObj (uint32_t _indent = 0u);

                std::vector<std::string> writeValue(uint32_t _indent = 0u);

            // Object/array helpers

                const JSONNode* find(const std::string &_key) const; // Object member by key, nullptr if absent or not an object
                JSONNode& addMember(const std::string &_key);         // Appends a null member to an object (switches to object first)
                JSONNode& pushBack();                                  // Appends a null element to an array (switches to array first)
                double getNum(const std::string &_key, double _default) const;
                bool  getBool(const std::string &_key, bool _default) const;
                std::string getStr(const std::string &_key, const std::string &_default) const;
            } root_node; // JSONNode END

            ContentJSON();
            virtual ~ContentJSON() override;

            void readFromString(const char *_json_str);
            std::string writeToString(uint32_t _indent = 0u);
        }; // ContentJSON END

    }; // File END
}; // Simple END

#endif
