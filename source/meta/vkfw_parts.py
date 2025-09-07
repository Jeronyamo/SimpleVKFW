##  ============  vkwrap_handlers.h - templates  ============  ##

OBJECT_HANDLER_DERIVED =\
"""
struct Handler : ObjectHandlerBase {{
{ind}{obj_type} handler = {obj_def_val};

{ind}Handler({obj_type} _handler = {obj_def_val}) : handler{{_handler}} {{}}
{ind}Handler(const ObjectInfoType &_obj_info) {{
{ind}{ind}set(_obj_info);
{ind}}}
{ind}Handler(const Handler &_handler)
{ind}{ind}{ind}{ind}{ind}: handler{{_handler.handler}} {{}}
{ind}Handler(Handler &&_handler)
{ind}{ind}{ind}{ind}{ind}: handler{{_handler.handler}} {{
{ind}{ind}_handler.clear();
{ind}}}
{ind}~Handler() {{
{ind}{ind}clear();
{ind}}}

{ind}      ObjectCommonType* ptr(uint32_t _index = 0u)       override {{ return (ObjectCommonType*) &handler; }}
{ind}const ObjectCommonType* ptr(uint32_t _index = 0u) const override {{ return (ObjectCommonType*) &handler; }}
{ind}const ObjectCommonType  get(uint32_t _index = 0u) const override {{ return  handler; }}
{ind}      {obj_type}* handlerPtr(uint32_t _index = 0u)       {{ return &handler; }}
{ind}const {obj_type}* handlerPtr(uint32_t _index = 0u) const {{ return &handler; }}
{ind}      uint32_t count() const {{ return 1; }}

{ind}void clear() override {{ handler = {obj_def_val}; }}
{ind}VkObjectType getType() const override {{ return {obj_type_enum}; }}
{ind}void set(const ObjectInfoType &_obj_info) override {{
{ind}{ind}if (_obj_info.obj_type == {obj_type_enum} || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
{ind}{ind}{ind}handler = ({obj_type}) _obj_info.obj_handle;
{ind}{ind}else
{ind}{ind}{ind}fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW :: {obj_type_no_prefix} :: Handler", "attempting to set a handler of different type\\n"));
{ind}}}
{ind}void set({obj_type} _handler) {{
{ind}{ind}handler = _handler;
{ind}}}
{ind}uint32_t destroy(VkInstance _instance, VkDevice _device,
{ind}                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {{
{destroy_func_body}
{ind}}}

{ind}Handler& operator=(const Handler &_handler) {{
{ind}{ind}handler = _handler.handler;
{ind}{ind}return *this;
{ind}}}

{ind}Handler& operator=(Handler &&_handler) {{
{ind}{ind}handler = _handler.handler;
{ind}{ind}_handler.clear();
{ind}{ind}return *this;
{ind}}}
}}; // Handler END
"""

OBJECT_VEC_HANDLER_DERIVED =\
"""
struct VecHandler : ObjectHandlerBase {{
{ind}std::vector<{obj_type}> handlers;

{ind}VecHandler(std::vector<{obj_type}> _handlers = {{}}) : handlers{{_handlers}} {{}}
{ind}VecHandler(const ObjectInfoType &_obj_info) {{
{ind}{ind}set(_obj_info);
{ind}}}
{ind}VecHandler(const Handler &_handler)
{ind}{ind}{ind}{ind}{ind}: handlers{{_handler.handler}} {{}}
{ind}VecHandler(Handler &&_handler)
{ind}{ind}{ind}{ind}{ind}: handlers{{_handler.handler}} {{
{ind}{ind}_handler.clear();
{ind}}}
{ind}VecHandler(const VecHandler &_handlers)
{ind}{ind}{ind}{ind}{ind}: handlers{{_handlers.handlers}} {{}}
{ind}VecHandler(VecHandler &&_handlers)
{ind}{ind}{ind}{ind}{ind}: handlers{{_handlers.handlers}} {{
{ind}{ind}_handlers.clear();
{ind}}}
{ind}~VecHandler() {{
{ind}{ind}clear();
{ind}}}

{ind}      ObjectCommonType* ptr(uint32_t _index = 0u)       override {{ return (ObjectCommonType*) &handlers[_index]; }}
{ind}const ObjectCommonType* ptr(uint32_t _index = 0u) const override {{ return (ObjectCommonType*) &handlers[_index]; }}
{ind}const ObjectCommonType  get(uint32_t _index = 0u) const override {{ return  handlers[_index]; }}
{ind}      {obj_type}* handlerPtr(uint32_t _index = 0u)       {{ return &handlers[_index]; }}
{ind}const {obj_type}* handlerPtr(uint32_t _index = 0u) const {{ return &handlers[_index]; }}
{ind}const {obj_type}  handlerGet(uint32_t _index = 0u) const {{ return  handlers[_index]; }}
{ind}      uint32_t count() const override {{ return handlers.size(); }}

{ind}void clear() override {{
{ind}{ind}handlers.clear();
{ind}}}
{ind}VkObjectType getType() const override {{ return {obj_type_enum}; }}
{ind}void set(const ObjectInfoType &_obj_info) override {{
{ind}{ind}if (_obj_info.obj_type == {obj_type_enum} || _obj_info.obj_type == VK_OBJECT_TYPE_UNKNOWN)
{ind}{ind}{ind}handlers = {{({obj_type}) _obj_info.obj_handle}};
{ind}{ind}else
{ind}{ind}{ind}fprintf(svkfwwarn, SVKFW_WRAPWARN("VKFW :: {obj_type_no_prefix} :: VecHandler", "attempting to set a handler of different type\\n"));
{ind}}}
{ind}void set(std::vector<{obj_type}> _handlers) {{
{ind}{ind}handlers = _handlers;
{ind}}}
{ind}uint32_t destroy(VkInstance _instance, VkDevice _device,
{ind}                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {{
{destroy_vec_func_body}
{ind}}}

{ind}VecHandler& operator=(const VecHandler &_handler) {{
{ind}{ind}handlers = _handler.handlers;
{ind}{ind}return *this;
{ind}}}

{ind}VecHandler& operator=(VecHandler &&_handler) {{
{ind}{ind}handlers = _handler.handlers;
{ind}{ind}_handler.clear();
{ind}{ind}return *this;
{ind}}}
}}; // VecHandler END
"""

OBJECT_DESTROY =\
"""{ind}{ind}vkDestroy{obj_type_no_prefix}({add_param_list}handler, _destroy_callback);
{ind}{ind}return 0u;"""

OBJECT_DESTROY_VEC =\
"""{ind}{ind}for (uint32_t i = 0u; i < handlers.size(); ++i)
{ind}{ind}{ind}vkDestroy{obj_type_no_prefix}({add_param_list}handlers[i], _destroy_callback);
{ind}{ind}return 0u;"""

OBJECT_DESTROY_PLACEHOLDER =\
"""{ind}{ind}return 1u;"""

OBJECT_DESTROY_FUNC =\
"""
{ind}uint32_t destroy(ObjectInfoType _handler, VkInstance _instance, VkDevice _device,
{ind}                 const VkAllocationCallbacks* _destroy_callback = nullptr) override {{
{ind}{ind}vkDestroy{obj_type_no_prefix}({add_param_list}(Vk{obj_type_no_prefix})_handler.obj_handle, _destroy_callback);
{ind}{ind}return 0u;
{ind}}}
"""

OBJECT_HANDLER_EX_DERIVED =\
"""
struct BuilderBase : ObjectHandlerExBase {{
{ind}BuilderBase() {{}}
{ind}~BuilderBase() {{}}

{ind}VkObjectType getType() const override {{ return {obj_type_enum}; }}
{object_destroy_func}}}; // BuilderBase END
"""



## OLD ##

##  ============  VKFWBaseHandle  ============  ##

DESTROY_VK_OBJECT =\
"""

// Destroy Vk object:

{indent}void destroyVkObject() {{
{indent}{indent}vkDestroy{v_type_no_prefix}({reversed_v_list});
{indent}}}
"""


BASE_HANDLER =\
"""struct BaseHandler {{
// Data:

{indent}{data}


// Constructors:

{indent}BaseHandler() {{}}
{indent}BaseHandler({constr_params}){il_offset}: {constr_init} {{}}
{indent}BaseHandler(const BaseHandler &_handler){il_offset}: {copy_constr_init} {{}}
{indent}BaseHandler(BaseHandler &&_handler){il_offset}: {copy_constr_init} {{
{indent}{indent}_handler.clearHandler();
{indent}}}
{indent}virtual ~BaseHandler() {{
{indent}{indent}clearHandler();
{indent}}}


// Assignment:

{indent}BaseHandler& operator=(const BaseHandler &_handler) {{
{assign_objcopy}
{indent}{indent}return *this;
{indent}}}

{indent}BaseHandler& operator=(BaseHandler &&_handler) {{
{assign_objcopy}
{indent}{indent}_handler.clearHandler();
{indent}{indent}return *this;
{indent}}}


// Pointer access:

{indent}const {v_type}* ptr() const {{ return &{v_name}; }}
{destroy_vk_obj}

// Clear state:

{indent}void clearHandler() {{
{indent}{indent}{assign_def}
{indent}}}
}}; // BaseHandler END"""



HANDLER =\
"""

struct Handler : BaseHandler {{
// Constructors:

{indent}Handler() {{}}
{indent}Handler({constr_params}){il_offset}: BaseHandler{{{constr_param_names}}} {{}}
{indent}Handler(const BaseHandler &_handler) : BaseHandler{{_handler}} {{}}
{indent}Handler(BaseHandler &&_handler) : BaseHandler{{_handler}} {{
{indent}{indent}_handler.clearHandler();
{indent}}}
{indent}Handler(const Handler &_handler) : BaseHandler{{_handler}} {{}}
{indent}Handler(Handler &&_handler) : BaseHandler{{_handler}} {{
{indent}{indent}_handler.clearHandler();
{indent}}}

{indent}~Handler() {{
{indent}{indent}if ({nonzero_check})
{indent}{indent}{indent}destroyVkObject();
{indent}}}


// Assignment:

{indent}Handler& operator=(const Handler &_handler) {{
{assign_objcopy}
{indent}{indent}return *this;
{indent}}}

{indent}Handler& operator=(Handler &&_handler) {{
{assign_objcopy}
{indent}{indent}_handler.clearHandler();
{indent}{indent}return *this;
{indent}}}
}}; // Handler END"""


##  ============  BaseVecHandler  ============  ##

DESTROY_ALL_VK_OBJECTS =\
"""

// Destroy all Vk objects in vector:

{indent}void destroyAllVkObjects() {{
{indent}{indent}for (auto {v_name} : {v_plural_name})
{indent}{indent}{indent}vkDestroy{v_type_no_prefix}({reversed_v_list});
{indent}}}
"""


BASE_VEC_HANDLER =\
"""struct BaseVecHandler {{
// Data:

{indent}{vec_data}


// Constructors:

{indent}BaseVecHandler() {{}}
{indent}BaseVecHandler({constr_vec_params}){il_offset}: {constr_vec_init} {{}}
{indent}BaseVecHandler(const BaseHandler &_handler){il_offset}: {v_plural_name}{{_handler.{v_name}}}{copy_constr_init_other} {{}}
{indent}BaseVecHandler(BaseHandler &&_handler){il_offset}: {v_plural_name}{{_handler.{v_name}}}{copy_constr_init_other} {{
{indent}{indent}_handler.clearHandler();
{indent}}}
{indent}BaseVecHandler(const BaseVecHandler &_handler){il_offset}: {v_plural_name}{{_handler.{v_plural_name}}}{copy_constr_init_other} {{}}
{indent}BaseVecHandler(BaseVecHandler &&_handler){il_offset}: {v_plural_name}{{_handler.{v_plural_name}}}{copy_constr_init_other} {{
{indent}{indent}_handler.clearVecHandler();
{indent}}}
{indent}virtual ~BaseVecHandler() {{
{indent}{indent}clearVecHandler();
{indent}}}


// Assignment:

{indent}BaseVecHandler& operator=(const BaseHandler &_handler) {{
{indent}{indent}{v_plural_name} = {{ _handler.{v_name} }};
{assign_objcopy_other}
{indent}{indent}return *this;
{indent}}}

{indent}BaseVecHandler& operator=(BaseHandler &&_handler) {{
{indent}{indent}{v_plural_name} = {{ _handler.{v_name} }};
{assign_objcopy_other}
{indent}{indent}_handler.clearHandler();
{indent}{indent}return *this;
{indent}}}

{indent}BaseVecHandler& operator=(const BaseVecHandler &_handler) {{
{indent}{indent}{v_plural_name} = _handler.{v_plural_name};
{assign_objcopy_other}
{indent}{indent}return *this;
{indent}}}

{indent}BaseVecHandler& operator=(BaseVecHandler &&_handler) {{
{indent}{indent}{v_plural_name} = _handler.{v_plural_name};
{assign_objcopy_other}
{indent}{indent}_handler.clearVecHandler();
{indent}{indent}return *this;
{indent}}}


// Access by index:

{indent}{v_type} operator[](uint32_t _index) const {{ return {v_plural_name}[_index]; }}
{indent}BaseHandler getBaseHandler(uint32_t _index) const {{ return {{ {v_plural_name}[_index]{other_element_names} }}; }}


// Pointer and count:

{indent}const {v_type}* ptr() const {{ return {v_plural_name}.data(); }}
{indent}uint32_t count() const {{ return {v_plural_name}.size(); }}
{destroy_all_vk_objects}

// Clear state:

{indent}void clearVecHandler() {{
{indent}{indent}{v_plural_name}.clear();{assign_def_other}
{indent}}}
}}; // BaseVecHandler END"""

# pointer & count: (for Vulkan struct's array members)
# clearVecHandler: (clear Data to prevent destroying "elements" when they were moved, or copied - then call it manually)




VEC_HANDLER =\
"""

struct VecHandler : BaseVecHandler {{
// Constructors:

{indent}VecHandler() {{}}
{indent}VecHandler({constr_vec_params}){il_offset}: BaseVecHandler{{{constr_vec_param_names}}} {{}}
{indent}VecHandler(const BaseHandler &_handler) : BaseVecHandler{{_handler}} {{}}
{indent}VecHandler(BaseHandler &&_handler) : BaseVecHandler{{_handler}} {{
{indent}{indent}_handler.clearHandler();
{indent}}}
{indent}VecHandler(const BaseVecHandler &_handler) : BaseVecHandler{{_handler}} {{}}
{indent}VecHandler(BaseVecHandler &&_handler) : BaseVecHandler{{_handler}} {{
{indent}{indent}_handler.clearVecHandler();
{indent}}}
{indent}VecHandler(const VecHandler &_handler) : BaseVecHandler{{_handler}} {{}}
{indent}VecHandler(VecHandler &&_handler) : BaseVecHandler{{_handler}} {{
{indent}{indent}_handler.clearVecHandler();
{indent}}}

{indent}~VecHandler() {{
{indent}{indent}if ({vec_nonzero_check})
{indent}{indent}{indent}destroyAllVkObjects();
{indent}}}


// Assignment:

{indent}VecHandler& operator=(const VecHandler &_handler) {{
{indent}{indent}{v_plural_name} = _handler.{v_plural_name};
{indent}{indent}{assign_objcopy_other}
{indent}{indent}return *this;
{indent}}}

{indent}VecHandler& operator=(VecHandler &&_handler) {{
{indent}{indent}{v_plural_name} = _handler.{v_plural_name};
{indent}{indent}{assign_objcopy_other}
{indent}{indent}_handler.clearVecHandler();
{indent}{indent}return *this;
{indent}}}
}}; // VecHandler END"""