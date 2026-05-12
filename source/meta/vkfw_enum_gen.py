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
    enums: list[tuple[str, dict[int, str]]] = []

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
                    enum_dict_inv: dict[int, str] = {} # aliases ignored because C++ code ignores them
                    for enm, val in enum_strs:
                        try:
                            val_int = int(val)
                            enum_dict_inv[val_int] = enm
                        except:
                            pass
                    enums.append((enum_name, enum_dict_inv))
                    enum_strs = []
                    continue
                if line.find('//') >= 0 or line.find('#') >= 0: continue

                line = line.strip()[:-1].split(" = ")
                assert len(line) == 2
                enum_strs.append(tuple(line))

    file_lines: list[str] = []
    with open(vkfwpath) as vkfwenum_f:
        found_start: bool = False
        for line in vkfwenum_f:
            if found_start and line.find("//") >= 0:
                found_start = False
            if not found_start:
                file_lines.append(line.rstrip())

            if line.find("// Objects") >= 0:
                found_start = True
                for enumn, enumd in enums:
                    file_lines.append("")
                    file_lines.append("    " * 3 + "const EnumWrap " + enumn + "{ {")
                    for enumkey, enumval in enumd.items():
                        file_lines.append("    " * 4 + "{{{enum_int:11}, \"{enum_str}\"}},".format(enum_int= enumkey, enum_str= enumval))
                    file_lines.append("    " * 3 + "} };")

    with open(vkfwpath, "w") as vkfwenum_f:
        vkfwenum_f.writelines( line+'\n'  for line in file_lines )

if __name__ == "__main__":
    invertVulkanCoreEnums("/VulkanSDK/1.4.328.1/Include/vulkan/vulkan_core.h", "source/interface/vkfw_enum.h")