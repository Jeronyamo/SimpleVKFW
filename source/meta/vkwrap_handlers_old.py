import gen.lang.svkfw_cpp as scpp
import gen.svkfw_generator as sgen
import vkfw_parts


#  ============  VKFW  Vk object interface  ============  #

class VKFWHandlerMember(scpp.Entity):
    """
        Parameter in BaseHandler/BaseVecHandler.
    """
    def __init__(self, _v_type: str, _v_name: str, _v_def_val: str = "VK_NULL_HANDLE", _v_plural_name: str|None = None):
        """
            v_type: Variable type.
            v_name: Variable name.
            v_def_val: Variable default value.
            v_plural_name: Only for Vk object handle. If defined, creates BaseVecHandler/VecHandler.
        """
        super().__init__()
        self.v_type = _v_type
        self.v_name = _v_name
        self.v_def_val = _v_def_val
        self.v_plural_name = _v_plural_name

    def assign_def_str(self):
        return (self.v_name + " = " + self.v_def_val + ';')  if len(self.v_def_val) else ""

    def assign_objcopy_str(self):
        return self.v_name + " = _handler." + self.v_name + ';'

    def member_str(self):
        return self.v_type + ' ' + self.v_name + (" = " + self.v_def_val if len(self.v_def_val) else "") + ';'

    def constr_init(self):
        return self.v_name + "{_" + self.v_name + '}'

    def copy_constr_init(self):
        return self.v_name + "{_handler." + self.v_name + '}'

    def base_func_param(self):
        return self.v_type + " _" + self.v_name + " = " + self.v_def_val

    def func_param(self):
        return self.v_type + " _" + self.v_name

    def ne_to_def(self):
        return self.v_name + " != " + self.v_def_val

    def vkobj_type_no_prefix(self):
        if not self.v_type.startswith("Vk"):
            raise RuntimeError("Error: Handler main member type is not a Vk handle")
        return self.v_type[2:]

    def vec_member_str(self):
        if self.v_plural_name is not None:
            return "std::vector<" + self.v_type + "> " + self.v_plural_name + ';'
        return self.member_str()

    def vec_func_param(self):
        if self.v_plural_name is not None:
            return "const std::vector<" + self.v_type + "> &_" + self.v_plural_name
        return self.func_param()

    def constr_vec_init(self):
        if self.v_plural_name is not None:
            return self.v_plural_name + "{_" + self.v_plural_name + '}'
        return self.constr_init()

    def copy_constr_vec_init(self):
        if self.v_plural_name is not None:
            return self.v_plural_name + "{_handler." + self.v_plural_name + '}'
        return self.copy_constr_init()

    def vec_ne_to_def(self):
        if self.v_plural_name is not None:
            return '!' + self.v_plural_name + ".empty()"
        return self.ne_to_def()
    
    def vec_param_name(self):
        return self.v_plural_name if self.v_plural_name is not None else self.v_name





class VKFWHandlers(scpp.Entity):
    """
        A template for VKFW BaseHandler, Handler, BaseVecHandler, VecHandler.
    """
    def __init__(self, _members: list[VKFWHandlerMember], _destroyable: bool = True, _indent: int = 0, _indent_fixed: bool = False, _empty_lines: tuple[int, int] = (0, 0)):
        """
            members: A list of Handler/VecHandler members. First element must be Vk object handle -
            the main variable of this class. If it is destroyable (has vkCreate*/vkDestroy* functions),
            the order must be same as reversed order of vkDestroy* parameters (without destroy callback,
            it is created automatically).

            destroyable: Whether Vk object has vkDestroy* function or not. If "True", creates
            "const VkAllocationCallbacks* destroy_callback" member, destroyVkObject() method
            and derived "class Handler : BaseHandler", its destructor calls destroyVkObject().
            If "False": "using Handler = BaseHandler".

            indent: (base)
            indent_fixed: (base)
            empty_lines: (base)
        """
        super().__init__(_indent= _indent, _indent_fixed= _indent_fixed,_empty_lines= _empty_lines)
        self.members = _members
        self.destroyable = _destroyable
        self.has_vec_handler = _members[0].v_plural_name is not None

    def genstr(self, _indent: int = 0, _indent_len: int = 4):
        super().genstr(_indent, _indent_len)

        __destroy_callback = VKFWHandlerMember("const VkAllocationCallbacks*", "destroy_callback", "nullptr")
        if self.destroyable:
            self.members.append(__destroy_callback)

        # BaseHandler
        __indent_str = ' ' * _indent_len
        __indent_nl = scpp.Entity.SVKFW_NEWLINE
        __data = (__indent_nl + __indent_str).join(param.member_str()  for param in self.members)
        __constr_params = ", ".join(param.func_param()  for param in self.members) + (" = nullptr" if self.destroyable else "")
        __constr_init = ", ".join(param.constr_init()  for param in self.members)
        __copy_constr_init = ", ".join(param.copy_constr_init()  for param in self.members)
        __assign_objcopy = (__indent_nl + 2*__indent_str).join(param.assign_objcopy_str()  for param in self.members)
        if len(__assign_objcopy) > 0:
            __assign_objcopy = 2*__indent_str + __assign_objcopy
        __assign_def = (__indent_nl + 2*__indent_str).join(param.assign_def_str()  for param in self.members)
        __destroy_vk_obj = ""

        
        __base_constr_params = self.members[0].func_param()
        if len(self.members) > 1:
            __base_constr_params += ", " + ", ".join(param.base_func_param()  for param in self.members[1:])

        # init list offset - TODO
        __constr_il_offset = __indent_nl + 5 * __indent_str if True else ' '

        # Handler-only
        __constr_param_names = ""
        __param_nondef_check = ""
        if self.destroyable:
            __constr_param_names = ", ".join('_' + param.v_name  for param in self.members)
            self.members.pop()

            __param_nondef_check = " && ".join(param.ne_to_def()  for param in self.members)
            __destroy_vk_obj = vkfw_parts.DESTROY_VK_OBJECT\
                                .format(indent= __indent_str, v_type_no_prefix= self.members[0].vkobj_type_no_prefix(),
                                        reversed_v_list= ", ".join([member.v_name  for member in self.members][::-1] + [__destroy_callback.v_name]))

        __base_handler = vkfw_parts.BASE_HANDLER\
                                .format(indent= __indent_str, data= __data, constr_params= __base_constr_params,
                                        constr_init= __constr_init, copy_constr_init= __copy_constr_init,
                                        assign_objcopy= __assign_objcopy, v_type= self.members[0].v_type,
                                        v_name= self.members[0].v_name, destroy_vk_obj= __destroy_vk_obj,
                                        assign_def= __assign_def, il_offset= __constr_il_offset)
        __base_handler = __base_handler.replace("\r\n", "\n").split("\n")


        __handler = ["", "using Handler = BaseHandler;"]
        if self.destroyable:
            __handler = vkfw_parts.HANDLER\
                                .format(indent= __indent_str, constr_params= __constr_params,
                                        il_offset= __constr_il_offset, constr_param_names = __constr_param_names,
                                        assign_objcopy= __assign_objcopy, nonzero_check= __param_nondef_check)
            __handler = __handler.replace("\r\n", "\n").split("\n")

        return self.join_strings([ (__indent_str * _indent + line if len(line) else "") for line in __base_handler + __handler ])



class VKFWVecHandlers(scpp.Entity):
    """
        VKFW BaseVecHandler and VecHandler.
    """
    def __init__(self, _members: list[VKFWHandlerMember], _destroyable: bool = True, _indent: int = 0,
                       _indent_fixed: bool = False, _empty_lines: tuple[int, int] = (0, 0)):
        super().__init__(_indent=_indent, _indent_fixed=_indent_fixed, _empty_lines=_empty_lines)

        self.members = _members
        self.destroyable = _destroyable
        self.has_vec_handler = _members[0].v_plural_name is not None

    def genstr(self, _indent: int = 0, _indent_len: int = 4):
        super().genstr(_indent, _indent_len)

        if not self.has_vec_handler:
            return ""

        __destroy_callback = VKFWHandlerMember("const VkAllocationCallbacks*", "destroy_callback", "nullptr")
        if self.destroyable:
            self.members.append(__destroy_callback)

        # BaseVecHandler
        __indent_str = ' ' * _indent_len
        __indent_nl = scpp.Entity.SVKFW_NEWLINE
        __vec_data = (__indent_nl + __indent_str).join(param.vec_member_str()  for param in self.members)
        __constr_vec_params = ", ".join(param.vec_func_param()   for param in self.members) + (" = nullptr" if self.destroyable else "")
        __constr_vec_init   = ", ".join(param.constr_vec_init()  for param in self.members)

        __assign_objcopy_other = (__indent_nl + 2*__indent_str).join(param.assign_objcopy_str()  for param in self.members[1:])
        # can be moved to:  if len(self.members) > 1:
        if len(__assign_objcopy_other) > 0:
            __assign_objcopy_other = 2*__indent_str + __assign_objcopy_other
        __assign_def_other = (__indent_nl + 2*__indent_str).join(param.assign_def_str()  for param in self.members[1:])
        # can be moved to:  if len(self.members) > 1:
        if len(__assign_def_other):
            __assign_def_other = __indent_nl + 2*__indent_str + __assign_def_other

        # init list offset - TODO
        __constr_il_offset = __indent_nl + 5 * __indent_str if True else ' '

        # Depends on existence of vars other than handle itself
        __copy_constr_init_other = ""
        __other_element_names = ""
        if len(self.members) > 1:
            __copy_constr_init_other = ", " + ", ".join(param.copy_constr_init()  for param in self.members[1:])
            __other_element_names = ", " + ", ".join(param.v_name  for param in self.members[1:])

        # VecHandler-only
        __vec_param_names = ", ".join(param.vec_param_name()  for param in self.members)

        __destroy_all_vk_objects = ""
        __vec_param_nondef_check = ""
        if self.destroyable:
            self.members.pop()
            __vec_param_nondef_check = " && ".join(param.vec_ne_to_def()  for param in self.members)
            # Later check cases when Vulkan has vkDestroy for an array (to call that function once instead of using this for-loop)
            __destroy_all_vk_objects = vkfw_parts.DESTROY_ALL_VK_OBJECTS\
                                    .format(indent= __indent_str, v_name= self.members[0].v_name,
                                                v_type_no_prefix= self.members[0].vkobj_type_no_prefix(),
                                                v_plural_name= self.members[0].v_plural_name,
                                                reversed_v_list= ", ".join([member.v_name
                                                                            for member in self.members][::-1] + [__destroy_callback.v_name]))

        __base_vec_handler = vkfw_parts.BASE_VEC_HANDLER\
                                .format(indent= __indent_str, vec_data= __vec_data, constr_vec_params= __constr_vec_params,
                                        constr_vec_init= __constr_vec_init, other_element_names= __other_element_names,
                                        copy_constr_init_other= __copy_constr_init_other, assign_objcopy_other= __assign_objcopy_other,
                                        v_type= self.members[0].v_type, v_name= self.members[0].v_name,
                                        v_plural_name= self.members[0].v_plural_name, destroy_all_vk_objects= __destroy_all_vk_objects,
                                        assign_def_other= __assign_def_other, il_offset= __constr_il_offset)

        __base_vec_handler = __base_vec_handler.replace("\r\n", "\n").split("\n")


        __vec_handler = ["", "using VecHandler = BaseVecHandler;"]
        if self.destroyable:
            __vec_handler: str = vkfw_parts.VEC_HANDLER\
                                    .format(indent= __indent_str, constr_vec_params= __constr_vec_params,
                                            constr_vec_param_names= __vec_param_names, v_plural_name= self.members[0].v_plural_name,
                                            assign_objcopy_other= __assign_objcopy_other,
                                            vec_nonzero_check= __vec_param_nondef_check, il_offset= __constr_il_offset)

            __vec_handler = __vec_handler.replace("\r\n", "\n").split("\n")

        return self.join_strings([ (__indent_str * _indent + line if len(line) else "") for line in __base_vec_handler + __vec_handler ])



class VKFWObject(scpp.Entity):
    def __init__(self, _obj_name: str, _members: list[VKFWHandlerMember], _destroyable: bool = True, _indent: int = 0,
                       _indent_fixed: bool = False, _child_entities: list[scpp.Entity] = [], _empty_lines: tuple[int, int] = (0, 0)):
        super().__init__(_indent, _indent_fixed, _empty_lines, _child_entities)

        self.obj_name = _obj_name
        self.destroyable = _destroyable
        self.members: list[VKFWHandlerMember] = _members
        self.has_vec_handle = self.members[0].v_plural_name is not None

        self.object_entity: scpp.Incap =\
            scpp.Incap(scpp.INCAP.NAMESPACE, _obj_name, _indent= _indent, _indent_fixed= _indent_fixed,
                       _empty_lines= _empty_lines, _child_entities=
                [ VKFWHandlers(self.members, self.destroyable) ] +
                ( [ VKFWVecHandlers(self.members, self.destroyable, _empty_lines= (2, 0)) ] if self.has_vec_handle else [] ) +\
                self.child_entities
            )

    def genstr(self, _indent: int = 0, _indent_len: int = 4):
        return self.object_entity.genstr(_indent, _indent_len)


def vulkan_core_parser(_fpath: str, _replaces: dict[str, dict[str, str]]) -> list[VKFWObject]:
    def camelCase_To_snake_case(_var: str):
        __diff = ord('a') - ord('A')
        __res: str = ""

        for letter in _var:
            if ord('A') <= ord(letter) <= ord('Z'):
                if len(__res) and not (ord('A') <= ord(__res[-1]) <= ord('Z')):
                    __res += '_'
                if not (ord('A') <= ord(__res[-1]) <= ord('Z')):
                    __res += chr(ord(letter) + __diff)
                else:
                    __res += letter
            else:
                __res += letter
        return __res


    __res = []
    with open(_fpath) as vkcore_f:
        vk_destroy_beg = "typedef void (VKAPI_PTR *PFN_vkDestroy"
        vk_destroy_end = ", const VkAllocationCallbacks* pAllocator);"

        for line in vkcore_f.readlines():
            line = line.strip() # to get rid of CRLF's '\r'?
            if line.startswith(vk_destroy_beg) and line.endswith(vk_destroy_end):
                """
                    VKFWObjects are primarily created for destroyable Vulkan objects,
                    so vkDestroy* functions are used.
                """

                line = line[len(vk_destroy_beg) : line.find(vk_destroy_end)]
                __param_plural_name = None
                __vkobj_name, __members = line.split(")(")
                __vkobj_name = "VKFW" + __vkobj_name
                __members = [ [par_type, camelCase_To_snake_case(par_name)]  for param in __members.split(", ")[::-1]  for par_type, par_name in [param.split()]]

                # Do replacements

                if (replace_dict := _replaces.get(__vkobj_name, None)) is not None:
                    __vkobj_name = replace_dict.pop("VKFWObjectName", __vkobj_name)
                    __param_plural_name = replace_dict.pop("VKFWPluralName", None)

                    for member_name in replace_dict:
                        for i in range(len(__members)):
                            if __members[i][1] == member_name:
                                __members[i][1] = replace_dict[member_name]
                                break

                __members = [ VKFWHandlerMember(__members[0][0], __members[0][1], _v_plural_name= __param_plural_name) ] +\
                            [ VKFWHandlerMember(param[0], param[1]) for param in __members[1:] ]
                __res.append(VKFWObject(__vkobj_name, __members, _empty_lines= (2, 0)))
    return __res


if __name__ == "__main__":
    #  Hard-coded replaces of various parts of VKFW objects
    #  dict[str, dict[str, str]]:
    #        key - <VKFW object name used as namespace>
    #        val - <key: VKFWObjectName> - replaces namespace name (does not change the key)
    #              <key: VKFWPluralName> - adds plural name to the first member, which allows to create VecHandler
    #              <key: other> - replaces member names
    replaces =\
    {
        "VKFWDebugUtilsMessengerEXT" :
        {
            "VKFWObjectName" : "VKFWDebugMessengerEXT"
        },
        "VKFWAccelerationStructureNV" :
        {
            "VKFWObjectName" : "VKFWAccelStructNV",
            "acceleration_structure" : "accel_struct"
        },
        "VKFWAccelerationStructureKHR" :
        {
            "VKFWObjectName" : "VKFWAccelStructKHR",
            "acceleration_structure" : "accel_struct"
        },
        "VKFWShaderModule" :
        {
            "VKFWPluralName" : "shader_modules"
        },
        "VKFWFramebuffer" :
        {
            "VKFWPluralName" : "framebuffers"
        },
        "VKFWImage" :
        {
            "VKFWPluralName" : "images"
        },
        "VKFWImageView" :
        {
            "VKFWPluralName" : "image_views"
        },
        "VKFWSwapchainKHR" :
        {
            "VKFWPluralName" : "swapchains"
        },
        "VKFWSemaphore" :
        {
            "VKFWPluralName" : "semaphores"
        },
        "VKFWFence" :
        {
            "VKFWPluralName" : "fences"
        }
    }


    #  Handlers
    vkfw_destroyable_objects = vulkan_core_parser("/VulkanSDK/1.3.275.0/Include/vulkan/vulkan_core.h", replaces)
    vkfw_non_destroyable_objects =\
    [
        VKFWObject("VKFWPhysDevice", [VKFWHandlerMember("VkPhysicalDevice", "phys_device", _v_plural_name= "phys_devices")], _destroyable= False),
        VKFWObject("VKFWQueueFamily", [VKFWHandlerMember("uint32_t", "index", "-1", "indices")], False, _empty_lines= (2, 0)),
        VKFWObject("VKFWQueue", [VKFWHandlerMember("VkQueue", "queue", "VK_NULL_HANDLE", "queues")], False, _empty_lines= (2, 0)),
        VKFWObject("VKFWCommandBuffer", [VKFWHandlerMember("VkCommandBuffer", "command_buffer", "VK_NULL_HANDLE", "command_buffers")], False, _empty_lines= (2, 0))
    ]


    #  File tree

    vkwrap_entity =\
    scpp.Preprocessor(scpp.PRE.IFNDEF, ["SVKFW_VKWRAP_HANDLERS_OLD_H"], _child_entities=
        [
            scpp.Preprocessor(scpp.PRE.DEFINE, ["SVKFW_VKWRAP_HANDLERS_OLD_H"]),

            scpp.Preprocessor(scpp.PRE.INCLUDE, ["\"common/terminal.h\"", "\"glfwrap.h\""], _empty_lines= (1, 0)),
            scpp.Preprocessor(scpp.PRE.INCLUDE, ["<vector>"], _empty_lines= (1, 0)), # , "<string>", "<fstream>"

            scpp.Incap(scpp.INCAP.NAMESPACE, "Simple", _empty_lines= (2, 1), _child_entities=
                [
                    scpp.Comment(scpp.COMM.ONELINE_L, "Vulkan framework - handlers", _empty_lines= (1, 0), _indent_fixed= True),
                    scpp.Comment(scpp.COMM.ONELINE_M, "Destroyable objects", _empty_lines= (1, 0), _indent_fixed= True),
                ] +
                vkfw_destroyable_objects +
                [
                    scpp.Comment(scpp.COMM.ONELINE_L, "Non-destroyable objects", _empty_lines= (2, 0), _indent_fixed= True),
                ] +
                vkfw_non_destroyable_objects
            )
        ]
    )


    # Generate

    scpp.Entity.to_json("./vkwrap_handlers.json", [vkwrap_entity])
    vkwrap_entity: scpp.Preprocessor = scpp.Entity.from_json("./vkwrap_handlers.json")[0]

    config = sgen.SVKFWCodeGenConfig()
    config.fname = "vkwrap_handlers.h"
    config.fpath = "./source/interface"

    code_generator = sgen.SVKFWCodeGenerator(config)

    code_generator.generate([vkwrap_entity])