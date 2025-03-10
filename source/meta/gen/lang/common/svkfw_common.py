#  ============  Base class  ============  #

class Entity:
    ENTITY_DICT = {}
    SVKFW_NEWLINE_LF = '\n'
    SVKFW_NEWLINE_CRLF = "\r\n"
    SVKFW_NEWLINE = SVKFW_NEWLINE_LF


    def __init__(self,
                _indent: int = 0,
                _indent_fixed: bool = False,
                _empty_lines: tuple[int, int] = (0, 0),
                _child_entities: list["Entity"] = []):
        """
            indent: Number of indentations. One indentation is SVKFWCodeGenConfig().indent_len spaces.

            indent_fixed: if True, ignores "indent" parameter passed in genstr(). Useful for comment and preprocessor entities.

            empty_lines: pair (N, M). Puts empty lines -- N before and M after -- the entity's lines.

            child_entities: Entities used to format strings in genstr().
        """
        self.indent: int = _indent
        self.indent_fixed: bool = _indent_fixed
        self.empty_lines: tuple[int] = _empty_lines
        self.child_entities: list[Entity] = _child_entities

    def __init_subclass__(cls, /, **kwargs):
        """
            Saves subclass objects to Entity.ENTITY_DICT, so "from_dict"
            doesn't have to use "eval()" to build entities.
        """
        super().__init_subclass__(**kwargs)
        Entity.ENTITY_DICT.update({ str(cls)[str(cls).rfind('.') + 1 : -2] : cls })

    def genstr(self, _indent: int = 0, _indent_len: int = 4) -> str:
        """
            Returns (formatted) text.

            indent: If indent_fixed == False, this parameter sets current indent.

            indent_len: indent size in spaces. Indent string is ((' ' * indent_len) * indent).
        """
        if not self.indent_fixed:
            self.indent = _indent

        self.indent_str: str = (' ' * _indent_len) * self.indent
        return ""

    def join_strings(self, _lines: list[str]) -> str:
        """
            Join text strings using entity's configuration. Adds empty lines set by empty_lines argument.
        """
        _lines = ["" for _ in range(self.empty_lines[0])] + _lines + ["" for _ in range(self.empty_lines[1])]
        return Entity.SVKFW_NEWLINE.join(_lines)

    # def set_child_entities(self, _child_entities: list) -> "Entity":
    #     """
    #         Sets child Entities used in formatting genstr() lines.
    #     """
    #     self.child_entities: list[Entity] = _child_entities
    #     return self

    def to_dict(self) -> dict:
        """
            Save all __init__ parameters as a dict. If this entity type can have child entities, it has
            "child_entities" parameter in __init__, and they are also recursively saved.
        """
        # Skip "self" and underscores, convert varnames to class members
        __varnames_list: tuple[str] = self.__init__.__code__.co_varnames
        __varnames_list:  list[str] = [ varname[1:] if varname[0] == '_' else varname  for varname in __varnames_list[1:] ]

        # Manage "EntityType"
        __res_dict = { "EntityType" : str(self.__class__)[str(self.__class__).rfind('.') + 1 : -2] }

        # Manage other parameters except "child_entities"
        for _param in __varnames_list:
            if _param != "child_entities":
                __res_dict[_param] = self.__dict__[_param]

                if isinstance(__res_dict[_param], Entity):
                    __res_dict[_param] = [ __res_dict[_param].to_dict() ]
                elif isinstance(__res_dict[_param], (list, tuple)):
                    num_entities = 0
                    for ent in __res_dict[_param]:
                        num_entities += isinstance(ent, Entity)
                    if num_entities == 0:
                        continue
                    elif num_entities != len(__res_dict[_param]):
                        raise RuntimeError("Error: Entity.to_dict - {} parameter {} does not match the format".format(__res_dict["EntityType"], _param))
                    __res_dict[_param] = [ entity.to_dict()  for entity in __res_dict[_param] ]


        # Manage "child_entities"
        __child_entities = [ entity.to_dict()  for entity in self.child_entities ]

        if len(__child_entities):
            __res_dict["child_entities"] = __child_entities

        return __res_dict



    def from_dict(_entity_dict: dict[str, str|list[dict[str]]]) -> "Entity":
        """
            Static method, which takes entity dict from "to_dict()" and creates an object of this type.
        """
        __entity_type = _entity_dict.pop("EntityType")
        __child_entities = _entity_dict.pop("child_entities", None)
        __entity_dict_filtered = {}

        for key in _entity_dict:
            val = _entity_dict[key]
            if isinstance(val, dict) and val.get("EntityType", None) is not None:
                val = Entity.from_dict(val)
            elif isinstance(val, (list, tuple)):
                val = type(val)((Entity.from_dict(ent) if (isinstance(ent, dict) and ent.get("EntityType", None) is not None) else ent)  for ent in val)
            __entity_dict_filtered['_' + key] = val

        if __child_entities is not None:
            for i in range(len(__child_entities)):
                __child_entities[i] = Entity.from_dict(__child_entities[i])
            __entity_dict_filtered["_child_entities"] = __child_entities

        return Entity.ENTITY_DICT[__entity_type](**__entity_dict_filtered)

    def to_json(_filepath: str, _entities_list: list["Entity"], OpenTextMode: str = "w", _json_indent: int = 4):
        from json import dump

        for i in range(len(_entities_list)):
            _entities_list[i] = _entities_list[i].to_dict()

        with open(_filepath, OpenTextMode) as json_file:
            dump(_entities_list, json_file, ensure_ascii= False, indent= _json_indent)

    def from_json(_filepath: str) -> list["Entity"]:
        from json import load

        with open(_filepath) as json_file:
            __entities_list = load(json_file)

        if isinstance(__entities_list, dict):
            __entities_list = [__entities_list]
        for i in range(len(__entities_list)):
            __entities_list[i] = Entity.from_dict(__entities_list[i])

        return __entities_list


# class EntityList(Entity):
#     def __init__(self, _child_entities: list[Entity] = []):
#         self.child_entities: list[Entity] = _child_entities

#     def genstr(self, _indent: int = 0, _indent_len: int = 4):
#         return self.join_strings([ entity.genstr(_indent, _indent_len)  for entity in self.child_entities ])