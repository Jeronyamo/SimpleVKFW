import os


vkfw_enum_beg: str =\
"""#ifndef SVKFW_VKFW_ENUM_H
#define SVKFW_VKFW_ENUM_H

#include <map>
#include <string>


namespace Simple {
    namespace VKFW {
        namespace Enum {
        // Class

            struct EnumWrap {
                std::map<uint32_t, std::string> enum_names;

                EnumWrap(const std::map<uint32_t, std::string> &_enum_names) : enum_names{_enum_names} {}

                std::string getName(uint32_t _enum_val) const {
                    auto __enum_iter = enum_names.find(_enum_val);
                    return __enum_iter == enum_names.end() ? "" : __enum_iter->second;
                }

                std::string getBitNames(uint32_t _enum_val) const {
                    std::string __res_str = "";

                    for (uint32_t i = 0u; i < 32u; ++i) {
                        if ((_enum_val & (1<<i)) == 0) continue;

                        std::string __curr_str = getName(1 << i);
                        if (!__curr_str.empty()) {
                            if (!__res_str.empty()) __res_str += " | ";
                            __res_str += __curr_str;
                        }
                    }
                    return __res_str;
                }
            }; // EnumWrap END


        // Objects"""

vkfw_enum_end: str =\
"""        }; // Enum END
    }; // VKFW END
}; // Simple END

#endif"""


def  findSubstrEnd(source: str, substr: str) -> int:
    res_id = source.find(substr)
    return res_id + (len(substr) if (res_id >= 0) else 0)
def rfindSubstrEnd(source: str, substr: str) -> int:
    res_id = source.rfind(substr)
    return res_id + (len(substr) if (res_id >= 0) else 0)

def isUpper(char: str): return ord('A') <= ord(char) <= ord('Z')
def isLower(char: str): return ord('a') <= ord(char) <= ord('z')
def isDigit(char: str): return ord('0') <= ord(char) <= ord('9')


def invertVulkanCoreEnums(vkpath: str, vkfwpath: str) -> None:
    global vkfw_enum_beg, vkfw_enum_end

    enums: list[tuple[str, dict[str, str]]] = []

    with open(vkpath) as vkcore_f:
        in_enum: bool = False
        enum_strs: list[tuple[str, str]] = []
        enum_name = ""

        trigger = "typedef enum"
        for line in vkcore_f:
            if line.startswith(trigger):
                in_enum = True
                enum_name = line[len(trigger) : line.find("{")].strip()
                if enum_name.startswith("Vk"): enum_name = enum_name[2:]

                enum_name_tmp = ""
                for i in range(len(enum_name)):
                    if i > 0 and isUpper(enum_name[i]) and (isLower(enum_name[i-1]) or isDigit(enum_name[i-1])):
                        enum_name_tmp += '_'
                    enum_name_tmp += enum_name[i].lower()
                enum_name = enum_name_tmp
                continue

            if in_enum:
                if line.startswith('}'):
                    in_enum = False
                    enum_dict_inv: dict[str, str] = {} # aliases ignored because C++ code ignores them
                    for enm, val in enum_strs:
                        try:
                            val_int = int(val, base=0)
                            enum_dict_inv[val] = enm
                        except:
                            pass
                    enums.append((enum_name, enum_dict_inv))
                    enum_strs = []
                    continue
                if line.find('//') >= 0 or line.find('#') >= 0: continue

                line = line.strip()[:-1].split(" = ")
                assert len(line) == 2
                enum_strs.append((line[0], line[1]))

    file_lines: list[str] = [ vkfw_enum_beg ]

    for enumn, enumd in enums:
        file_lines.append("")
        file_lines.append("    " * 3 + "const EnumWrap " + enumn + "{ {")
        for enumkey, enumval in enumd.items():
            file_lines.append("    " * 4 + "{{{enum_int:11}, \"{enum_str}\"}},".format(enum_int= ("" if enumkey.startswith('-') else " ") + enumkey, enum_str= enumval))
        file_lines.append("    " * 3 + "} };")

    file_lines.append(vkfw_enum_end)

    with open(vkfwpath, "w") as vkfwenum_f:
        vkfwenum_f.writelines( line+'\n'  for line in file_lines )

if __name__ == "__main__":
    vk_sdk_path = os.environ.get("VULKAN_SDK", None)
    vk_sdk_path = os.environ.get("VK_SDK_PATH", None) if vk_sdk_path is None else vk_sdk_path
    if vk_sdk_path is None:
        print("Error: couldn't find Vulkan SDK path, no enums generated")
        exit(-1)
    vk_sdk_path = os.path.join(vk_sdk_path, "Include", "vulkan", "vulkan_core.h")
    print(f"Found Vulkan core at: '{vk_sdk_path}'")
    invertVulkanCoreEnums(vk_sdk_path, "source/interface/vkfw_enum.h")