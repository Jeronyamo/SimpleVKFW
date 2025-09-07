#ifndef SVKFW_FILETYPES_H
#define SVKFW_FILETYPES_H

#include <map>
#include <vector>
// #include <string>
// #include <type_traits>

#include "terminal.h"
#include "linalg.h"

#include "platform_dependent/pd_filetypes.h"


namespace Simple {
// Utilities

    enum Filetype : uint32_t {
        NONE,
        BIN,
        VMD,

        UNKNOWN = UINT32_MAX
    }; // Filetype END


// Interfaces

    struct FiletypeContentItf { virtual ~FiletypeContentItf() {} };

    // Tries to read all
    struct FiletypeReaderWriterItf {
        virtual ~FiletypeReaderWriterItf() {}

        virtual bool  read(const std::string &_fpath) = 0;
        virtual void write(const std::string &_fpath) = 0;
        // Used when file reader has to recognize a file without extension
        virtual bool tryHeaders(FileBinReader &_ftype_file) = 0;
    }; // FiletypeReaderWriter END


// Implementations

    struct ContentVMD : FiletypeContentItf {
        char header[30];
        char model[20];
        bool is_old_format;

        static const std::map<std::string, std::string> bone_name_mappings;
        static const std::map<std::string, std::string> morph_name_mappings;
        static const std::map<std::string, std::string> bone_name_mappings_fromraw;
        static const std::map<std::string, std::string> morph_name_mappings_fromraw;

        struct BoneKeyframe {
            vec3f position;
            uint32_t frame_id;
            Affine::Rotation::Quaternion rotation;
            char bone_name[16]; // Actually 15 + '\0'
            char int_info[64]; // interpolation info
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
            char int_info[24]{};
        };

        std::vector<BoneKeyframe> keyframes_bone;
        std::vector<MorphKeyframe> keyframes_morph;
        std::vector<CameraKeyframe> keyframes_camera;
    }; // ContentVMD END


    struct ReaderWriterVMD : FiletypeReaderWriterItf {
        ContentVMD file_content{};

        ReaderWriterVMD(const std::string &_fpath = "") { if (!_fpath.empty()) read(_fpath); }

        bool read(const std::string &_fpath) override {
            // Open file
            FileBinReader __ftype_file{_fpath};

            // Header
            const uint32_t header_len = 30;
            __ftype_file.read(file_content.header, header_len);
            std::string __res_str = std::string(file_content.header, file_content.header + (header_len - 5)); // 5 chars are ignored

            if (__res_str == "Vocaloid Motion Data file") {
                file_content.is_old_format = true;
            }
            else if (__res_str != "Vocaloid Motion Data 0002") {
                fprintf(svkfwwarn, SVKFW_WRAPWARN("ReaderWriterVMD :: read", "not a VMD file (header does not match): '%s'"), __res_str.c_str());
                return false;
            }

            printf(SVKFW_WRAPINFO("VMD Reader : Header", "'%s'\n"), __res_str.c_str());
            printf(SVKFW_WRAPINFO("VMD Reader : Format", " %s\n" ), file_content.is_old_format ? "old" : "new");

            // Model
            const uint32_t __model_name_len = file_content.is_old_format ? 10 : 20;
            __ftype_file.read(file_content.model, __model_name_len);

            printf(SVKFW_WRAPINFO("VMD Reader : Model name", "'%s'\n"), file_content.model);

            // Bone/Motion keyframes
            const uint32_t __bone_name_len = 15;
            const uint32_t __bone_interp_info_len = 64;
            uint32_t __keyframes_count = __ftype_file.read_binary<uint32_t>();
            file_content.keyframes_bone.resize(__keyframes_count);
            printf("\n" SVKFW_WRAPINFO("VMD Reader : Bone keyframes total", "%d\n"), __keyframes_count);
            std::string curr_name = "None";

            for (uint32_t i = 0u; i < __keyframes_count; ++i) {
                __ftype_file.read(file_content.keyframes_bone[i].bone_name, __bone_name_len);
                __ftype_file.read_binary(file_content.keyframes_bone[i].frame_id);
                __ftype_file.read_binary(file_content.keyframes_bone[i].position);
                __ftype_file.read_binary(file_content.keyframes_bone[i].rotation);
                // Different order, swap elements
                file_content.keyframes_bone[i].rotation = { file_content.keyframes_bone[i].rotation.k,
                                                            file_content.keyframes_bone[i].rotation.s,
                                                            file_content.keyframes_bone[i].rotation.i,
                                                            file_content.keyframes_bone[i].rotation.j };
                __ftype_file.read(file_content.keyframes_bone[i].int_info, __bone_interp_info_len);

                if (ContentVMD::bone_name_mappings_fromraw.find(file_content.keyframes_bone[i].bone_name) !=
                    ContentVMD::bone_name_mappings_fromraw.end()) {
                    std::string tmp_name = ContentVMD::bone_name_mappings_fromraw.at(file_content.keyframes_bone[i].bone_name);

                    if (tmp_name != curr_name) {
                        curr_name = tmp_name;
                        printf(SVKFW_WRAPINFO("VMD Reader : Bone", "%s\n"), curr_name.c_str());
                    }
                }
                else {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("ReaderWriterVMD :: read", "Unknown bone: "));
                    for (char ch : std::string(file_content.keyframes_bone[i].bone_name))
                        std::cout << std::oct << '\\' << uint32_t(uint8_t(ch));
                    printf("\n");
                }
            }

            // Face/Morph keyframes
            const uint32_t __morph_name_len = 15;
            __ftype_file.read_binary(__keyframes_count);
            file_content.keyframes_morph.resize(__keyframes_count);
            printf("\n" SVKFW_WRAPINFO("VMD Reader : Morph keyframes total", "%d\n"), __keyframes_count);

            for (uint32_t i = 0u; i < __keyframes_count; ++i) {
                __ftype_file.read(file_content.keyframes_morph[i].morph_name, __morph_name_len);
                __ftype_file.read_binary(file_content.keyframes_morph[i].frame_id);
                __ftype_file.read_binary(file_content.keyframes_morph[i].weight  );

                if (ContentVMD::morph_name_mappings_fromraw.find(file_content.keyframes_morph[i].morph_name) !=
                    ContentVMD::morph_name_mappings_fromraw.end()) {
                    std::string tmp_name = ContentVMD::morph_name_mappings_fromraw.at(file_content.keyframes_morph[i].morph_name);

                    if (tmp_name != curr_name) {
                        curr_name = tmp_name;
                        printf(SVKFW_WRAPINFO("VMD Reader : Morph", "%s\n"), curr_name.c_str());
                    }
                }
                else {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("ReaderWriterVMD :: read", "Unknown morph: "));
                    for (char ch : std::string(file_content.keyframes_morph[i].morph_name))
                        std::cout << std::oct << '\\' << uint32_t(uint8_t(ch));
                    printf("\n");
                }
            }

            // Camera keyframes
            const uint32_t __camera_interp_info_len = 24;
            __ftype_file.read_binary(__keyframes_count);
            file_content.keyframes_camera.resize(__keyframes_count);
            printf("\n" SVKFW_WRAPINFO("VMD Reader : Camera keyframes total", "%d\n"), __keyframes_count);

            for (uint32_t i = 0u; i < __keyframes_count; ++i) {
                __ftype_file.read_binary(file_content.keyframes_camera[i].frame_id);
                __ftype_file.read_binary(file_content.keyframes_camera[i].distance);
                __ftype_file.read_binary(file_content.keyframes_camera[i].position);
                __ftype_file.read_binary(file_content.keyframes_camera[i].rotation);
                __ftype_file.read(file_content.keyframes_camera[i].int_info, __camera_interp_info_len);
                __ftype_file.read_binary(file_content.keyframes_camera[i].fov     );
                file_content.keyframes_camera[i].perspect = __ftype_file.read_binary<uint8_t>(); // reading 1 byte is super effective, duh
            }
            // printf(SVKFW_WRAPINFO("VMD Reader : Result", "%s\n"), __ftype_file.file_r.good() ? SVKFW_ENCLOSE(SGR_COL_FG_GREEN, "GOOD") : SVKFW_ENCLOSE(SGR_COL_FG_YELLOW, "BAD"));
            return true;
        }

        void write(const std::string &_fpath) override {
            // Open file
            FileBinWriter __ftype_file{_fpath};

            // Header
            const uint32_t __header_len = 30;
            __ftype_file.write(file_content.header, __header_len);

            // Model
            const uint32_t __model_name_len = file_content.is_old_format ? 10 : 20;
            __ftype_file.write(file_content.model, __model_name_len);

            // Bone/Motion keyframes
            const uint32_t __bone_interp_info_len = 64;
            const uint32_t __bone_name_len = 15;
            uint32_t __keyframes_count = file_content.keyframes_bone.size();
            __ftype_file.write_binary(__keyframes_count);

            for (uint32_t i = 0u; i < __keyframes_count; ++i) {
                __ftype_file.write(file_content.keyframes_bone[i].bone_name, __bone_name_len);
                __ftype_file.write_binary(file_content.keyframes_bone[i].frame_id);
                __ftype_file.write_binary(file_content.keyframes_bone[i].position);
                vec4f __quaternion_data_ordered = { file_content.keyframes_bone[i].rotation.i,
                                                    file_content.keyframes_bone[i].rotation.j,
                                                    file_content.keyframes_bone[i].rotation.k,
                                                    file_content.keyframes_bone[i].rotation.s };
                __ftype_file.write_binary(__quaternion_data_ordered);
                __ftype_file.write(file_content.keyframes_bone[i].int_info, __bone_interp_info_len);
            }

            // Face/Morph keyframes
            const uint32_t __morph_name_len = 15;
            __keyframes_count = file_content.keyframes_morph.size();
            __ftype_file.write_binary(__keyframes_count);

            for (uint32_t i = 0u; i < __keyframes_count; ++i) {
                __ftype_file.write(file_content.keyframes_morph[i].morph_name, __morph_name_len);
                __ftype_file.write_binary(file_content.keyframes_morph[i].frame_id);
                __ftype_file.write_binary(file_content.keyframes_morph[i].weight);
            }

            // Camera keyframes
            const uint32_t __camera_interp_info_len = 24;
            __keyframes_count = file_content.keyframes_camera.size();
            __ftype_file.write_binary(__keyframes_count);

            for (uint32_t i = 0u; i < __keyframes_count; ++i) {
                __ftype_file.write_binary(file_content.keyframes_camera[i].frame_id);
                __ftype_file.write_binary(file_content.keyframes_camera[i].distance);
                __ftype_file.write_binary(file_content.keyframes_camera[i].position);
                __ftype_file.write_binary(file_content.keyframes_camera[i].rotation);
                __ftype_file.write(file_content.keyframes_camera[i].int_info, __camera_interp_info_len);
                __ftype_file.write_binary(file_content.keyframes_camera[i].fov     );
                __ftype_file.write_binary((uint8_t)file_content.keyframes_camera[i].perspect);
            }
        }

        bool tryHeaders(FileBinReader &_ftype_file) override {
            const uint32_t header_len = 30;
            char __ch_buffer[32]{};

            _ftype_file.read(__ch_buffer, header_len);
            std::string __res_str = std::string(__ch_buffer, __ch_buffer + header_len - 5);
            return __res_str == "Vocaloid Motion Data file" || __res_str == "Vocaloid Motion Data 0002";
        }

        void hand_fix(float _val = 40.7f) {
            uint32_t __hand_count_l = 0u, __hand_count_r = 0u;

            for (auto &bone_frame : file_content.keyframes_bone) {
                if (ContentVMD::bone_name_mappings_fromraw.at(bone_frame.bone_name) == "hand_r") {
                    auto __angles = Affine::Rotation::quaternion2ypr(bone_frame.rotation);
                    __angles.roll = std::min(__angles.roll,  _val * float(M_PI/180));
                    bone_frame.rotation = Affine::Rotation::ypr2quaternion(__angles.yaw, __angles.pitch, __angles.roll);
                    ++__hand_count_r;
                    // __angles = Affine::Rotation::quaternion2ypr(bone_frame.rotation);
                    // printf("[%d] RHand: %f, %f, %f --- %f, %f, %f, %f\n", bone_frame.frame_id, __angles.pitch * (180/M_PI), __angles.yaw * (180/M_PI), __angles.roll * (180/M_PI));
                }
                if (ContentVMD::bone_name_mappings_fromraw.at(bone_frame.bone_name) == "hand_l") {
                    auto __angles = Affine::Rotation::quaternion2ypr(bone_frame.rotation);
                    __angles.roll = std::max(__angles.roll, -_val * float(M_PI/180));
                    bone_frame.rotation = Affine::Rotation::ypr2quaternion(__angles.yaw, __angles.pitch, __angles.roll);
                    ++__hand_count_l;
                }
            }
            printf("Hand keyframe fixes: %d left hand, %d right hand\n", __hand_count_l, __hand_count_r);
        }
    }; // ReaderWriterVMD END

    // VMD: init mappings

    const std::map<std::string, std::string> ContentVMD::bone_name_mappings
    {
        // Center
        {"root", "\221\123\202\304\202\314\220\145",},
        {"center", "\203\132\203\223\203\136\201\133",},
        {"groove", "\203\117\203\213\201\133\203\165",},
        {"back_hips", "\215\230",},
        // Body
        {"hips", "\221\314\202\314\217\144\220\123",},
        {"spine", "\217\343\224\274\220\147",},
        {"spine2", "\217\343\224\274\220\147\62",},
        {"spine3", "\217\343\224\274\220\147\63",},
        {"neck", "\216\361",},
        {"head", "\223\252",},
        {"waist", "\211\272\224\274\220\147",},
        // Right Arm
        {"shoulder_p_r", "\211\105\214\250\120",},
        {"shoulder_r", "\211\105\214\250",},
        {"shoulder_c_r", "\211\105\214\250\103",},
        {"uparm_r", "\211\105\230\162",},
        {"uparm_twist_r", "\211\105\230\162\235\200",},
        {"lowarm_r", "\211\105\202\320\202\266",},
        {"lowarm_twist_r", "\211\105\216\350\235\200",},
        {"hand_r", "\211\105\216\350\216\361",},
        {"hand_dummy_r", "\211\105\203\137\203\176\201\133",},
        {"ik_hand_r", "\211\105\230\162\202\150\202\152",},
        // Left Arm
        {"shoulder_p_l", "\215\266\214\250\120",},
        {"shoulder_l", "\215\266\214\250",},
        {"shoulder_c_l", "\215\266\214\250\103",},
        {"uparm_l", "\215\266\230\162",},
        {"uparm_twist_l", "\215\266\230\162\235\200",},
        {"lowarm_l", "\215\266\202\320\202\266",},
        {"lowarm_twist_l", "\215\266\216\350\235\200",},
        {"hand_l", "\215\266\216\350\216\361",},
        {"hand_dummy_l", "\215\266\203\137\203\176\201\133",},
        {"ik_hand_l", "\215\266\230\162\202\150\202\152",},
        // Right Hand
        {"f_thumb01_r", "\211\105\220\145\216\167\202\117",},
        {"f_thumb02_r", "\211\105\220\145\216\167\202\120",},
        {"f_thumb03_r", "\211\105\220\145\216\167\202\121",},
        {"f_index01_r", "\211\105\220\154\216\167\202\120",},
        {"f_index02_r", "\211\105\220\154\216\167\202\121",},
        {"f_index03_r", "\211\105\220\154\216\167\202\122",},
        {"f_middle01_r", "\211\105\222\206\216\167\202\120",},
        {"f_middle02_r", "\211\105\222\206\216\167\202\121",},
        {"f_middle03_r", "\211\105\222\206\216\167\202\122",},
        {"f_ring01_r", "\211\105\226\362\216\167\202\120",},
        {"f_ring02_r", "\211\105\226\362\216\167\202\121",},
        {"f_ring03_r", "\211\105\226\362\216\167\202\122",},
        {"f_little01_r", "\211\105\217\254\216\167\202\120",},
        {"f_little02_r", "\211\105\217\254\216\167\202\121",},
        {"f_little03_r", "\211\105\217\254\216\167\202\122",},
        // Left Hand
        {"f_thumb01_l", "\215\266\220\145\216\167\202\117",},
        {"f_thumb02_l", "\215\266\220\145\216\167\202\120",},
        {"f_thumb03_l", "\215\266\220\145\216\167\202\121",},
        {"f_index01_l", "\215\266\220\154\216\167\202\120",},
        {"f_index02_l", "\215\266\220\154\216\167\202\121",},
        {"f_index03_l", "\215\266\220\154\216\167\202\122",},
        {"f_middle01_l", "\215\266\222\206\216\167\202\120",},
        {"f_middle02_l", "\215\266\222\206\216\167\202\121",},
        {"f_middle03_l", "\215\266\222\206\216\167\202\122",},
        {"f_ring01_l", "\215\266\226\362\216\167\202\120",},
        {"f_ring02_l", "\215\266\226\362\216\167\202\121",},
        {"f_ring03_l", "\215\266\226\362\216\167\202\122",},
        {"f_little01_l", "\215\266\217\254\216\167\202\120",},
        {"f_little02_l", "\215\266\217\254\216\167\202\121",},
        {"f_little03_l", "\215\266\217\254\216\167\202\122",},
        // Right Leg
        {"back_hips_c_r", "\215\230\203\114\203\203\203\223\203\132\203\213\211\105",},
        {"upleg_r", "\211\105\221\253",},
        {"lowleg_r", "\211\105\202\320\202\264",},
        {"foot_r", "\211\105\221\253\216\361",},
        {"toe_r", "\211\105\202\302\202\334\220\346",},
        {"ik_f_root_r", "\211\105\221\253\111\113\220\145",},
        {"ik_foot_r", "\211\105\221\253\202\150\202\152",},
        {"ik_toe_r", "\211\105\202\302\202\334\220\346\202\150\202\152",},
        {"upleg_d_r", "\211\105\221\253\104",},
        {"lowleg_d_r", "\211\105\202\320\202\264\104",},
        {"foot_d_r", "\211\105\221\253\216\361\104",},
        {"heel_ex_r", "\211\105\346\371\105\130",},
        {"toe_ex_r", "\211\105\221\253\220\346\105\130",},
        // Left Leg
        {"back_hips_c_l", "\215\230\203\114\203\203\203\223\203\132\203\213\215\266",},
        {"upleg_l", "\215\266\221\253",},
        {"lowleg_l", "\215\266\202\320\202\264",},
        {"foot_l", "\215\266\221\253\216\361",},
        {"toe_l", "\215\266\202\302\202\334\220\346",},
        {"ik_f_root_l", "\215\266\221\253\111\113\220\145",},
        {"ik_foot_l", "\215\266\221\253\202\150\202\152",},
        {"ik_toe_l", "\215\266\202\302\202\334\220\346\202\150\202\152",},
        {"upleg_d_l", "\215\266\221\253\104",},
        {"lowleg_d_l", "\215\266\202\320\202\264\104",},
        {"foot_d_l", "\215\266\221\253\216\361\104",},
        {"heel_ex_l", "\215\266\346\371\105\130",},
        {"toe_ex_l", "\215\266\221\253\220\346\105\130",},
        // Eyes
        {"eyes", "\227\274\226\332",},
        {"eye_r", "\211\105\226\332",},
        {"eye_l", "\215\266\226\332",},
        // Misc
        {"mouth_base", "\214\373\203\170\201\133\203\130",},
        {"eyebrow_base", "\224\373\203\170\201\133\203\130",},
    }; // bone_name_mappings END

    const std::map<std::string, std::string> ContentVMD::morph_name_mappings
    {
        // Mouth
        {"mouth_a", "\202\240",},
        {"mouth_i", "\202\242",},
        {"mouth_u", "\202\244",},
        {"mouth_e", "\202\246",},
        {"mouth_o", "\202\250",},
        {"mouth_wa", "\202\355",},
        {"mouth_n", "\202\361",},
        {"mouth_a2", "\202\240\202\121",},
        {"mouth_wa2", "\203\217",},
        {"mouth_smile", "\202\311\202\342\202\350",},
        {"mouth_smile2", "\202\311\202\342\202\350\202\121",},
        {"mouth_smile3", "\202\311\202\301\202\261\202\350",},
        {"mouth_tongueout", "\202\330\202\353\202\301",},
        {"mouth_he", "\201\310",},
        {"mouth_triangle", "\201\243",},
        {"mouth_cat", "\203\326",},
        {"mouth_cat2", "\203\326\201\240",},
        {"mouth_bothup", "\214\373\212\160\217\343\202\260",},
        {"mouth_bothdown", "\214\373\212\160\211\272\202\260",},
        {"mouth_wider", "\214\373\211\241\215\114\202\260",},
        // Eyes
        {"blink", "\202\334\202\316\202\275\202\253",},
        {"eyesmile", "\217\316\202\242",},
        {"eyesmile2", "\217\316\202\242\226\332",},
        {"wink_r", "\203\105\203\102\203\223\203\116\211\105",},
        {"wink_l", "\203\105\203\102\203\223\203\116",},
        {"wink_r2", "\263\250\335\270\202\121\211\105",},
        {"wink_l2", "\203\105\203\102\203\223\203\116\202\121",},
        {"surprise", "\202\321\202\301\202\255\202\350",},
        {"stare", "\202\266\202\306\226\332",},
        {"eye_angry", "\267\330\257",},
        {"nagomi", "\202\310\202\262\202\335",},
        {"hau", "\202\315\202\243",},
        {"hatyu", "\202\315\202\277\202\343\226\332",},
        {"star", "\220\257\226\332",},
        {"heart", "\202\315\202\237\202\306",},
        {"iris_shrink", "\223\265\217\254",},
        {"face_red", "\217\306\202\352",},
        // Eyebrow
        {"serious", "\220\136\226\312\226\332",},
        {"confuse", "\215\242\202\351",},
        {"angry", "\223\173\202\350",},
        {"eyebrowsmile", "\202\311\202\261\202\350",},
        {"eyebrowup", "\217\343",},
        {"eyebrowdown", "\211\272",},
    }; // morph_name_mappings END

    // VMD: init reversed mappings

    const std::map<std::string, std::string> ContentVMD::bone_name_mappings_fromraw
    {
        // Center
        {"\221\123\202\304\202\314\220\145","root",      },
        {"\203\132\203\223\203\136\201\133","center",    },
        {"\203\117\203\213\201\133\203\165","groove",    },
        {"\215\230",                        "back_hips", },
        // Body
        {"\221\314\202\314\217\144\220\123","hips",   },
        {"\217\343\224\274\220\147",        "spine",  },
        {"\217\343\224\274\220\147\62",     "spine2", },
        {"\217\343\224\274\220\147\63",     "spine3", },
        {"\216\361",                        "neck",   },
        {"\223\252",                        "head",   },
        {"\211\272\224\274\220\147",        "waist",  },
        // Right Arm
        {"\211\105\214\250\120",            "shoulder_p_r",   },
        {"\211\105\214\250",                "shoulder_r",     },
        {"\211\105\214\250\103",            "shoulder_c_r",   },
        {"\211\105\230\162",                "uparm_r",        },
        {"\211\105\230\162\235\200",        "uparm_twist_r",  },
        {"\211\105\202\320\202\266",        "lowarm_r",       },
        {"\211\105\216\350\235\200",        "lowarm_twist_r", },
        {"\211\105\216\350\216\361",        "hand_r",         },
        {"\211\105\203\137\203\176\201\133","hand_dummy_r",   },
        {"\211\105\230\162\202\150\202\152","ik_hand_r",      },
        // Left Arm
        {"\215\266\214\250\120",            "shoulder_p_l",   },
        {"\215\266\214\250",                "shoulder_l",     },
        {"\215\266\214\250\103",            "shoulder_c_l",   },
        {"\215\266\230\162",                "uparm_l",        },
        {"\215\266\230\162\235\200",        "uparm_twist_l",  },
        {"\215\266\202\320\202\266",        "lowarm_l",       },
        {"\215\266\216\350\235\200",        "lowarm_twist_l", },
        {"\215\266\216\350\216\361",        "hand_l",         },
        {"\215\266\203\137\203\176\201\133","hand_dummy_l",   },
        {"\215\266\230\162\202\150\202\152","ik_hand_l",      },
        // Right Hand
        {"\211\105\220\145\216\167\202\117","f_thumb01_r",  },
        {"\211\105\220\145\216\167\202\120","f_thumb02_r",  },
        {"\211\105\220\145\216\167\202\121","f_thumb03_r",  },
        {"\211\105\220\154\216\167\202\120","f_index01_r",  },
        {"\211\105\220\154\216\167\202\121","f_index02_r",  },
        {"\211\105\220\154\216\167\202\122","f_index03_r",  },
        {"\211\105\222\206\216\167\202\120","f_middle01_r", },
        {"\211\105\222\206\216\167\202\121","f_middle02_r", },
        {"\211\105\222\206\216\167\202\122","f_middle03_r", },
        {"\211\105\226\362\216\167\202\120","f_ring01_r",   },
        {"\211\105\226\362\216\167\202\121","f_ring02_r",   },
        {"\211\105\226\362\216\167\202\122","f_ring03_r",   },
        {"\211\105\217\254\216\167\202\120","f_little01_r", },
        {"\211\105\217\254\216\167\202\121","f_little02_r", },
        {"\211\105\217\254\216\167\202\122","f_little03_r", },
        // Left Hand
        {"\215\266\220\145\216\167\202\117","f_thumb01_l",  },
        {"\215\266\220\145\216\167\202\120","f_thumb02_l",  },
        {"\215\266\220\145\216\167\202\121","f_thumb03_l",  },
        {"\215\266\220\154\216\167\202\120","f_index01_l",  },
        {"\215\266\220\154\216\167\202\121","f_index02_l",  },
        {"\215\266\220\154\216\167\202\122","f_index03_l",  },
        {"\215\266\222\206\216\167\202\120","f_middle01_l", },
        {"\215\266\222\206\216\167\202\121","f_middle02_l", },
        {"\215\266\222\206\216\167\202\122","f_middle03_l", },
        {"\215\266\226\362\216\167\202\120","f_ring01_l",   },
        {"\215\266\226\362\216\167\202\121","f_ring02_l",   },
        {"\215\266\226\362\216\167\202\122","f_ring03_l",   },
        {"\215\266\217\254\216\167\202\120","f_little01_l", },
        {"\215\266\217\254\216\167\202\121","f_little02_l", },
        {"\215\266\217\254\216\167\202\122","f_little03_l", },
        // Right Leg
        {"\215\230\203\114\203\203\203\223\203\132\203\213\211\105","back_hips_c_r", },
        {"\211\105\221\253",                                        "upleg_r",       },
        {"\211\105\202\320\202\264",                                "lowleg_r",      },
        {"\211\105\221\253\216\361",                                "foot_r",        },
        {"\211\105\202\302\202\334\220\346",                        "toe_r",         },
        {"\211\105\221\253\111\113\220\145",                        "ik_f_root_r",   },
        {"\211\105\221\253\202\150\202\152",                        "ik_foot_r",     },
        {"\211\105\202\302\202\334\220\346\202\150\202\152",        "ik_toe_r",      },
        {"\211\105\221\253\104",                                    "upleg_d_r",     },
        {"\211\105\202\320\202\264\104",                            "lowleg_d_r",    },
        {"\211\105\221\253\216\361\104",                            "foot_d_r",      },
        {"\211\105\346\371\105\130",                                "heel_ex_r",     },
        {"\211\105\221\253\220\346\105\130",                        "toe_ex_r",      },
        // Left Leg
        {"\215\230\203\114\203\203\203\223\203\132\203\213\215\266","back_hips_c_l", },
        {"\215\266\221\253",                                        "upleg_l",       },
        {"\215\266\202\320\202\264",                                "lowleg_l",      },
        {"\215\266\221\253\216\361",                                "foot_l",        },
        {"\215\266\202\302\202\334\220\346",                        "toe_l",         },
        {"\215\266\221\253\111\113\220\145",                        "ik_f_root_l",   },
        {"\215\266\221\253\202\150\202\152",                        "ik_foot_l",     },
        {"\215\266\202\302\202\334\220\346\202\150\202\152",        "ik_toe_l",      },
        {"\215\266\221\253\104",                                    "upleg_d_l",     },
        {"\215\266\202\320\202\264\104",                            "lowleg_d_l",    },
        {"\215\266\221\253\216\361\104",                            "foot_d_l",      },
        {"\215\266\346\371\105\130",                                "heel_ex_l",     },
        {"\215\266\221\253\220\346\105\130",                        "toe_ex_l",      },
        // Eyes
        {"\227\274\226\332","eyes",  },
        {"\211\105\226\332","eye_r", },
        {"\215\266\226\332","eye_l", },
        // Misc
        {"\214\373\203\170\201\133\203\130","mouth_base",   },
        {"\224\373\203\170\201\133\203\130","eyebrow_base", },
    }; // bone_name_mappings END

    const std::map<std::string, std::string> ContentVMD::morph_name_mappings_fromraw
    {
        // Mouth
        {"\202\240",                         "mouth_a",         },
        {"\202\242",                         "mouth_i",         },
        {"\202\244",                         "mouth_u",         },
        {"\202\246",                         "mouth_e",         },
        {"\202\250",                         "mouth_o",         },
        {"\202\355",                         "mouth_wa",        },
        {"\202\361",                         "mouth_n",         },
        {"\202\240\202\121",                 "mouth_a2",        },
        {"\203\217",                         "mouth_wa2",       },
        {"\202\311\202\342\202\350",         "mouth_smile",     },
        {"\202\311\202\342\202\350\202\121", "mouth_smile2",    },
        {"\202\311\202\301\202\261\202\350", "mouth_smile3",    },
        {"\202\330\202\353\202\301",         "mouth_tongueout", },
        {"\201\310",                         "mouth_he",        },
        {"\201\243",                         "mouth_triangle",  },
        {"\203\326",                         "mouth_cat",       },
        {"\203\326\201\240",                 "mouth_cat2",      },
        {"\214\373\212\160\217\343\202\260", "mouth_bothup",    },
        {"\214\373\212\160\211\272\202\260", "mouth_bothdown",  },
        {"\214\373\211\241\215\114\202\260", "mouth_wider",     },
        // Eyes
        {"\202\334\202\316\202\275\202\253",        "blink",       },
        {"\217\316\202\242",                        "eyesmile",    },
        {"\217\316\202\242\226\332",                "eyesmile2",   },
        {"\203\105\203\102\203\223\203\116\211\105","wink_r",      },
        {"\203\105\203\102\203\223\203\116",        "wink_l",      },
        {"\263\250\335\270\202\121\211\105",        "wink_r2",     },
        {"\203\105\203\102\203\223\203\116\202\121","wink_l2",     },
        {"\202\321\202\301\202\255\202\350",        "surprise",    },
        {"\202\266\202\306\226\332",                "stare",       },
        {"\267\330\257",                            "eye_angry",   },
        {"\202\310\202\262\202\335",                "nagomi",      },
        {"\202\315\202\243",                        "hau",         },
        {"\202\315\202\277\202\343\226\332",        "hatyu",       },
        {"\220\257\226\332",                        "star",        },
        {"\202\315\202\237\202\306",                "heart",       },
        {"\223\265\217\254",                        "iris_shrink", },
        {"\217\306\202\352",                        "face_red",    },
        // Eyebrow
        {"\220\136\226\312\226\332","serious",      },
        {"\215\242\202\351",        "confuse",      },
        {"\223\173\202\350",        "angry",        },
        {"\202\311\202\261\202\350","eyebrowsmile", },
        {"\217\343",                "eyebrowup",    },
        {"\211\272",                "eyebrowdown",  },
    }; // morph_name_mappings END
}; // Simple END

#endif
