import gen.lang.svkfw_cpp as scpp
import gen.svkfw_generator as sgen
import vkfw_parts


#  ============  VKFW Vk object interface  ============  #

class VulkanName:
    def __init__(self, _var: str = ""):
        self.var = _var

    def dflt(self) -> str:
        return self.var
    
    def hasPrefix(self, _pref: str) -> bool:
        return self.var.startswith(_pref)

    def withPrefix(self, _pref: str) -> str:
        return _pref + self.var

    def minusPrefix(self, _pref: str) -> str:
        if self.hasPrefix(_pref):
            return self.var[len(_pref):]
        return self.var

    def enumElem(self, _enum_name: str) -> str:
        return VulkanName.toEnumStyle(_enum_name + self.var)

# Static
    def camelCase_To_snake_case(_var: str) -> str:
        __res = ""
        for ch in _var:
            __res += ('_' + ch.lower()) if ('A' <= ch <= 'Z') else ch

        if __res[0] == '_':
            __res = __res[1:]
        return __res

    def snake_case_To_camelCase(_var: str, _first_letter_upper: bool = True) -> str:
        __res = ""
        for tok_part in _var.split('_'):
            __res += tok_part.capitalize()

        if not _first_letter_upper:
            __res[0] = __res[0].lower()

        return __res

    def toEnumStyle(_var: str) -> str:
        __res = ""
        for i in range(len(_var)):
            __newsym  = _var[i].upper()
            __lastsym = _var[i-1]  if i else 'A'

            if ('A' <= _var[i] <= 'Z') and not ('A' <= __lastsym <= 'Z'):
                __newsym = '_' + __newsym
            
            __res += __newsym

        if __res[0] == '_':
            __res = __res[1:]
        return __res

    def fromEnumStyle(_var: str, _ext_suffixes: list[str] = [], _first_letter_upper: bool = True) -> str:
        __res = VulkanName.snake_case_To_camelCase(_var, _first_letter_upper)
        for suf in _ext_suffixes:
            if _var.endswith('_' + suf):
                __res = __res[:-len(suf)] + suf
                break
        return __res



class VKFWHandlers(scpp.Entity):
    """
        A template for VKFW BaseHandler, Handler, BaseVecHandler, VecHandler.
    """
    def __init__(self, _object_type: str, _object_default_val: str = "VK_NULL_HANDLE", _destroyable: bool = True, _destroy_param_str: str = "",
                _has_vec_handler: bool = False, _indent: int = 0, _indent_fixed: bool = False, _empty_lines: tuple[int, int] = (0, 0)):
        super().__init__(_indent= _indent, _indent_fixed= _indent_fixed,_empty_lines= _empty_lines)
        self.object_type = _object_type
        self.object_default_val = _object_default_val
        self.destroyable = _destroyable
        self.destroy_param_str = _destroy_param_str
        self.has_vec_handler = _has_vec_handler

    def genstr(self, _indent: int = 0, _indent_len: int = 4):
        super().genstr(_indent, _indent_len)

        __indent_str = ' ' * _indent_len

        __obj_type = self.object_type
        __obj_type_no_prefix = VulkanName(__obj_type).minusPrefix("Vk")
        __obj_type_enum = VulkanName(__obj_type_no_prefix).enumElem("VkObjectType")
        __obj_def_val = self.object_default_val
        __add_param_list = self.destroy_param_str

        __destroy_func_body = vkfw_parts.OBJECT_DESTROY_PLACEHOLDER.format(ind= __indent_str)
        __destroy_vec_func_body = vkfw_parts.OBJECT_DESTROY_PLACEHOLDER.format(ind= __indent_str)
        if self.destroyable:
            __destroy_func_body = vkfw_parts.OBJECT_DESTROY.format(obj_type_no_prefix= __obj_type_no_prefix,
                                                                   add_param_list= __add_param_list, ind= __indent_str)
            __destroy_vec_func_body = vkfw_parts.OBJECT_DESTROY_VEC.format(obj_type_no_prefix= __obj_type_no_prefix,
                                                                           add_param_list= __add_param_list, ind= __indent_str)

        __handler = vkfw_parts.OBJECT_HANDLER_DERIVED.format(obj_type_no_prefix= __obj_type_no_prefix,
                                                             obj_type_enum= __obj_type_enum, obj_type= __obj_type,
                                                             obj_def_val= __obj_def_val, ind= __indent_str,
                                                             destroy_func_body= __destroy_func_body)
        __handler = __handler.replace("\r\n", "\n").split("\n")


        __vec_handler = []
        if self.has_vec_handler:
            __vec_handler = vkfw_parts.OBJECT_VEC_HANDLER_DERIVED.format(obj_type_no_prefix= __obj_type_no_prefix,
                                                                         obj_type_enum= __obj_type_enum, obj_type= __obj_type,
                                                                         obj_def_val= __obj_def_val, ind= __indent_str,
                                                                         destroy_vec_func_body= __destroy_vec_func_body)

            __vec_handler = __vec_handler.replace("\r\n", "\n").split("\n")


        __handler_ex = []
        __object_destroy_func = ""
        if self.destroyable:
            __object_destroy_func = vkfw_parts.OBJECT_DESTROY_FUNC.format(obj_type_no_prefix= __obj_type_no_prefix,
                                                                          add_param_list= __add_param_list, ind= __indent_str)
        __handler_ex = vkfw_parts.OBJECT_HANDLER_EX_DERIVED.format(obj_type_enum= __obj_type_enum,
                                                                   object_destroy_func= __object_destroy_func, ind= __indent_str)
        __handler_ex = __handler_ex.replace("\r\n", "\n").split("\n")

        __functions = []
        if False:
            __destroy_glob_func = vkfw_parts.OBJECT_DESTROY_GLOB.format(obj_type_no_prefix= __obj_type_no_prefix,
                                                                        add_param_list= __add_param_list, ind= __indent_str)

            __destroy_glob_func = __destroy_glob_func.replace("\r\n", "\n").split("\n")
            __functions = [""]
            __functions.extend(__destroy_glob_func)

        return self.join_strings([ (__indent_str * _indent + line if len(line) else "") for line in __handler + __vec_handler + __handler_ex + __functions ])



class VulkanCoreParserConfig:
    def __init__(self, _enums: list[str] = [], _structs_unions: list[str] = [], _parse_objects: bool = True,
                 _objects_without_vec_handler: list[str] = [], _objects_ignore: list[str] = []):
        self.parse_enums = set(_enums)
        self.parse_structs_unions = set(_structs_unions)
        self.parse_objects = _parse_objects
        self.objects_without_vec_handler = set(objects_without_vec_handler)
        self.objects_ignore = set(_objects_ignore)

        if _parse_objects:
            self.parse_enums.add("VkObjectType")


class VulkanCoreParser:
    def __init__(self, _vulkan_core_h_path: str, _config: VulkanCoreParserConfig):
        self.fpath = _vulkan_core_h_path
        self.config = _config
        self.objects_list: list[str] = []
        self.destroy_info: dict[str, list[list[str, str]]] = {}
        self.enum_info:    dict[str, list[list[str, str]]] = {}


    def parse_destroy_function(self, line: str):
        """
            Parsing vkDestroy*(...) functions to generate correct "ObjectHandler.destroy()" methods
        """
        __vkobj_type, __members = line.split(")(")
        __members = [ [par_type, VulkanName.camelCase_To_snake_case(par_name)]  for param in __members.split(", ")[::-1]  for par_type, par_name in [param.split()]]

        self.destroy_info["Vk" + __vkobj_type] = __members


    def vulkan_core_parse(self) -> None:
        def match_line_beg_end(_line: str, _l_beg: str, _l_end: str) -> bool:
            return _line.startswith(_l_beg) and _line.endswith(_l_end)

        def cut_line_beg_end(_line: str, _l_beg: str, _l_end: str) -> str:
            return _line[ len(_l_beg) : -len(_l_end) ]


        with open(self.fpath) as vkcore_f:
            vk_destroy_beg = "typedef void (VKAPI_PTR *PFN_vkDestroy"
            vk_destroy_end = ", const VkAllocationCallbacks* pAllocator);"

            vk_enum_block_beg_beg = "typedef enum "
            vk_enum_block_beg_end = " {"
            vk_enum_block_end_beg = "} "
            vk_enum_block_end_end = ";"
            vk_enum_block_in: bool = False

            vk_define_handle_beg = "VK_DEFINE_"
            vk_define_handle_end = ")"


            for line in vkcore_f.readlines():
                line = line.strip()

                # Parse | begin parse blocks
                if self.config.parse_objects and match_line_beg_end(line, vk_destroy_beg, vk_destroy_end):
                    self.parse_destroy_function(cut_line_beg_end(line, vk_destroy_beg, vk_destroy_end))
                elif self.config.parse_objects and match_line_beg_end(line, vk_define_handle_beg, vk_define_handle_end):
                    self.objects_list.append(line[line.rfind('(')+1:-1])
                elif match_line_beg_end(line, vk_enum_block_beg_beg, vk_enum_block_beg_end):
                    enum_name = cut_line_beg_end(line, vk_enum_block_beg_beg, vk_enum_block_beg_end)

                    if enum_name in self.config.parse_enums:
                        self.enum_info[enum_name] = []
                        vk_enum_block_in = True
                    continue

                # In | End blocks
                vk_enum_block_in = vk_enum_block_in and not match_line_beg_end(line, vk_enum_block_end_beg, vk_enum_block_end_end)
                if vk_enum_block_in:
                    var, val = line.split(" = ")
                    if val.endswith(','):
                        val = val[:-1]

                    self.enum_info[enum_name].append([var, val])



    def generate_vkfw_handlers(self) -> list[VKFWHandlers]:
        __res_handlers: list[VKFWHandlers] = []

        for vk_obj_type, enum_val in self.enum_info["VkObjectType"]:
            if vk_obj_type not in self.config.objects_ignore:
                if not enum_val.startswith(VulkanName.toEnumStyle("VkObjectType")): # if not an alias
                    __object_type = VulkanName.fromEnumStyle(vk_obj_type)
                    __object_type = "Vk" + VulkanName(__object_type).minusPrefix("VkObjectType")

                    # Fix suffixes
                    if __object_type not in self.objects_list:
                        for i in range(len(__object_type)-1,0,-1):
                            if 'A' <= __object_type[i] <= 'Z':
                                __object_type = __object_type[:i] + __object_type[i:].upper()
                                break

                    __destroyable = self.destroy_info.get(__object_type, None) is not None

                    __destroy_param_str = ""
                    if __destroyable:
                        __destroy_param_str = ", ".join([ '_' + param_name[1]  for param_name in (self.destroy_info[__object_type][1:]) ])
                        if __destroy_param_str != "":
                            __destroy_param_str += ", "

                    __has_vec_handler = __object_type not in self.config.objects_without_vec_handler
                    __res_handler = VKFWHandlers(_object_type= __object_type, _destroyable= __destroyable,
                                                 _destroy_param_str= __destroy_param_str, _has_vec_handler= __has_vec_handler,
                                                 _indent_fixed= False, _empty_lines= (0, 0))
                    __res_handler = scpp.Incap(scpp.INCAP.NAMESPACE, VulkanName(__object_type).minusPrefix("Vk"), _child_entities=[__res_handler], _empty_lines= (2, 0))
                    __res_handlers.append(__res_handler)
                # else: # handle this case ?
        return __res_handlers


if __name__ == "__main__":
    # Hard-coded data
    objects_without_vec_handler: list[str] = []

    objects_ignore = [
                        "VK_OBJECT_TYPE_UNKNOWN",
                        "VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA",
                        "VK_OBJECT_TYPE_MAX_ENUM"
                     ]

    vulkan_core_fpath = "/VulkanSDK/1.3.275.0/Include/vulkan/vulkan_core.h"
    output_fpath = "./source/interface/vkfw_handlers.h"


    #  Parse vulkan_core.h
    config = VulkanCoreParserConfig(_objects_without_vec_handler= objects_without_vec_handler,
                                    _objects_ignore= objects_ignore)
    parser = VulkanCoreParser(vulkan_core_fpath, config)
    parser.vulkan_core_parse()
    handler_entities = parser.generate_vkfw_handlers()


    #  Generate preprocessor flag
    preproc_flag = "SVKFW_" + output_fpath[output_fpath.rfind('/')+1:].replace('.', '_').upper()

    #  File tree
    vkfw_entity =\
    scpp.Preprocessor(scpp.PRE.IFNDEF, [preproc_flag], _child_entities=
        [
        scpp.Preprocessor(scpp.PRE.DEFINE, [preproc_flag]),

        scpp.Preprocessor(scpp.PRE.INCLUDE, ["\"vkfw_base.h\""], _empty_lines= (1, 0)),

        scpp.Incap(scpp.INCAP.NAMESPACE, "Simple", _empty_lines= (2, 1), _child_entities=
            [
            scpp.Incap(scpp.INCAP.NAMESPACE, "VKFW", _empty_lines= (0, 0), _child_entities=
                [
                scpp.Comment(scpp.COMM.ONELINE_L, "Vulkan framework - handlers", _empty_lines= (1, 0), _indent_fixed= True),
                ] +
                handler_entities
            )
            ]
        )
        ]
    )

    # Generate
    json_path = "./source/meta/vkfw_handlers.json"
    scpp.Entity.to_json(json_path, [vkfw_entity])
    vkfw_entity: scpp.Preprocessor = scpp.Entity.from_json(json_path)[0]

    config = sgen.SVKFWCodeGenConfig()
    config.fname = output_fpath[ output_fpath.rfind('/')+1:]
    config.fpath = output_fpath[:output_fpath.rfind('/')+1 ]

    code_generator = sgen.SVKFWCodeGenerator(config)

    code_generator.generate([vkfw_entity])