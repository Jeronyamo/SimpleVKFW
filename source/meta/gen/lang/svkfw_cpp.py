from gen.lang.common.svkfw_common import Entity


#  ============  Comments  ============  #

class COMM:
    ONELINE_STD = "ONELINE_STD"
    ONELINE_M = "ONELINE_M"
    ONELINE_L = "ONELINE_L"
    MULLINE_STD = "MULLINE_STD"
    MULLINE_M = "MULLINE_M"


class Comment(Entity):
    def __init__(self, _comm_type: COMM, _lines: str | list[str],
                _empty_lines: tuple[int, int] = (0, 0),
                _indent: int = 0, _indent_fixed: bool = False):
        """
            comm_type: Type of the comment.

            lines: A list with the commented lines.
            If comm_type is "ONELINE_*" and len(_lines) > 1, creates multiple one-line comments.

            empty_lines: (base)

            indent: (base)

            indent_fixed: (base)
        """
        super().__init__(_indent= _indent, _indent_fixed= _indent_fixed, _empty_lines= _empty_lines)

        if isinstance(_lines, str):
            _lines = [_lines]

        # if _comm_type in (COMM.ONELINE_STD, COMM.ONELINE_M, COMM.ONELINE_L) and len(_lines) != 1:
        #     raise ValueError(f"Error: Comment entity - bad number of comment lines: {len(_lines)}, must be 1")

        self.comm_type = _comm_type
        self.lines = _lines

    def genstr(self, _indent: int = 0, _indent_len: int = 4) -> str:
        super().genstr(_indent, _indent_len)
        __res = []

        match self.comm_type:
            case COMM.ONELINE_STD:
                __res = [ self.indent_str + "// " + line  for line in self.lines ]
            case COMM.ONELINE_M:
                __comm = self.indent_str + "//  ============  {}  ============  \\\\" + Entity.SVKFW_NEWLINE
                __res = [ __comm.format(line)  for line in self.lines ]
            case COMM.ONELINE_L:
                __res: list[str] = []
                for line in self.lines:
                    __res.extend([
                                    self.indent_str + f"//  ============{'=' * len(line)}============  \\\\",
                                    self.indent_str + ' ' * 16 + line,
                                    self.indent_str + f"//  ============{'=' * len(line)}============  \\\\" + Entity.SVKFW_NEWLINE,
                                ])
            case COMM.MULLINE_STD:
                if len(self.lines) == 0:
                    __res = [ self.indent_str + "/**/" ]
                elif len(self.lines) == 1:
                    __res = [ self.indent_str + "/* " + self.lines[0] + " */" ]
                else:
                    __res = [ self.indent_str + "/*" ] +\
                            [ self.indent_str + (' ' * 4) + line  for line in self.lines ] +\
                            [ self.indent_str + "*/" ]
            case COMM.MULLINE_M:
                __max_len = max(( len(line)  for line in self.lines ))
                __res = [ self.indent_str + "/*  " + '=' * __max_len ] +\
                        [ self.indent_str + (' ' * 4) + line  for line in self.lines ] +\
                        [ self.indent_str + '=' * __max_len + "  */" ]
        return self.join_strings(__res)

    def to_dict(self):
        return super().to_dict()
        return {
                    "EntityType" : "Comment",
                    "indent" : self.indent,
                    "indent_fixed" : self.indent_fixed,
                    "empty_lines" : self.empty_lines,
                    "comm_type" : self.comm_type,
                    "lines" : self.lines,
                    "child_entities" : self.child_entities
               }


#  ============  Preprocessor  ============  #

class PRE:
    IF = "IF"
    IFDEF = "IFDEF"
    IFNDEF = "IFNDEF"
    ELSE = "ELSE"
    ELIF = "ELIF"
    DEFINE = "DEFINE"
    UNDEF = "UNDEF"
    INCLUDE = "INCLUDE"
    PRAGMA = "PRAGMA"
    LINE = "LINE"


class Preprocessor(Entity):
    def __init__(self, _dir_type: PRE, _args: list[str|tuple[str,str]] = [], _child_entities: list[Entity] = [],
                _empty_lines: tuple[int, int] = (0, 0)):
        super().__init__(_child_entities= _child_entities, _indent= 0,
                         _indent_fixed= True, _empty_lines= _empty_lines)

        _dir_type: str = str(_dir_type)

        if _dir_type == PRE.ELSE and len(_args):
            raise ValueError("Error: non-empty argument list for #else directive")

        if _dir_type not in (PRE.IF, PRE.IFDEF, PRE.IFNDEF, PRE.ELSE, PRE.ELIF) and len(_child_entities):
            raise ValueError(f"Error: non-empty child entities list for non-if/non-else directive ({_dir_type})")

        self.if_directive: bool = _dir_type in (PRE.IF, PRE.IFDEF, PRE.IFNDEF)
        self.dir_type = _dir_type
        self.args = _args

    def genstr(self, _indent: int = 0, _indent_len: int = 4) -> str:
        super().genstr(_indent, _indent_len)

        __directive = '#' + self.dir_type.lower()
        __num_strings = len(self.args) + (__directive == "#else")
        __res = [ self.indent_str + __directive + ("" if len(self.args) == 0 else ' ' + self.args[i])  for i in range(__num_strings) ]

        __res.extend(child.genstr(_indent, _indent_len) for child in self.child_entities)

        if self.if_directive:
            __res.append("#endif")
        return self.join_strings(__res)

    def to_dict(self):
        return super().to_dict()
        return {
                    "EntityType" : "Preprocessor",
                    "empty_lines" : self.empty_lines,
                    "dir_type" : self.dir_type,
                    "args" : self.args,
                    "child_entities" : [ child.to_dict()  for child in self.child_entities ]
               }


#  ============  C++  ============  #

# TODO this section (the hardest part)

class CPP:
    # Make these "Var"s
    NEW = "new"
    SIZEOF = "sizeof"


class OP:
    SI = "++"
    PI = "++"
    SD = "--"
    PD = "--"
    UP = '+'
    UM = '-'
    LNOT = '!'
    BNOT = '~'
    ADD = '+'
    SUB = '-'
    MUL = '*'
    DIV = '/'
    BAND = '&'
    BOR = '|'
    AND = "&&"
    OR = "||"
    TERN = "?:"
    ASGN = '='
    AA = "+="
    SA = "-="
    MA = "*="
    DA = "/="
    BAA = "&="
    BOA = "|="
    BXA = "^="
    BLSA = "<<="
    BRSA = ">>="



class Expr(Entity):
    def __init__(self, __expr: "Expr", __op_history: list[OP|list[OP]] = []):
        """
            expr: String containing full expression.

            op_history: list of all operations which resulted in this expression, starting from the first.
        """
        self._expr = __expr
        self._op_history = __op_history


class Var(Expr):
    def __init__(self, __e_type: str, __e_name: str):
        self._e_type = __e_type
        self._e_name = __e_name

class Function(Entity):
    def __init__(self, _head: str, _body: str, 
                       _indent: int = 0,
                       _indent_fixed: bool = False,
                       _empty_lines: tuple[int, int] = (0, 0)):
        super().__init__(_indent, _indent_fixed, _empty_lines)
        self.head = _head
        self.body = _body

    def genstr(self, _indent: int = 0, _indent_len: int = 4) -> str:
        super().genstr(_indent, _indent_len)

        return self.join_strings([ self.head + " {", self.body, '}' ])


#  ============  Incapsulation - class, struct, union, namespace  ============  #

class INCAP:
    NAMESPACE = "NAMESPACE"
    STRUCT = "STRUCT"
    CLASS = "CLASS"
    UNION = "UNION"

class ACC_SPEC:
    DEFAULT = "DEFAULT"
    PRIVATE = "PRIVATE"
    PROTECTED = "PROTECTED"
    PUBLIC = "PUBLIC"


class BaseClassList(Entity):
    def __init__(self, _name_list: str | list[str|tuple[str, ACC_SPEC]|tuple[str, ACC_SPEC, bool]]):
        """
            _name_list: list of tuples ("ClassName", ACC_SPEC, is_virtual_base)
        """
        super().__init__(_indent_fixed= True)

        if (type(_name_list) is str):
            _name_list = [_name_list]

        for i in range(len(_name_list)):
            __new_elem = ["", ACC_SPEC.DEFAULT, False]
            if (type(_name_list[i]) is str):
                __new_elem[0] = _name_list[i]
            elif len(_name_list[i]) == 1:
                __new_elem[0] = _name_list[i][0]
            elif len(_name_list[i]) == 2:
                __new_elem[0] = _name_list[i][0]
                if type(_name_list[i][1]) is ACC_SPEC:
                    __new_elem[1] = _name_list[i][1]
                elif type(_name_list[i][1]) is bool:
                    __new_elem[2] = _name_list[i][1]
                else: raise RuntimeError("Error: BaseClassList unsupported type")
            else:
                __new_elem = _name_list[i]
            _name_list[i] = __new_elem

        self.name_list = _name_list

    def genstr(self, _indent: int = 0, _indent_len: int = 4) -> str:
        super().genstr(_indent, _indent_len)

        if len(self.name_list) == 0:
            return ""

        __specifiers = { ACC_SPEC.DEFAULT : "", ACC_SPEC.PRIVATE : "private ",
                         ACC_SPEC.PROTECTED : "protected ", ACC_SPEC.PUBLIC : "public " }
        __res = [ " : " + ", ".join([ __specifiers[elem[1]] + ("" if elem[2] == False else "virtual ") + elem[0]  for elem in self.name_list]) ]
        return self.join_strings(__res)

    def to_dict(self):
        return {
                    "EntityType" : "BaseClassList",
                    "name_list"  : self.name_list
               }



class Constructor(Entity):
    "TODO"
    def __init__(self):
        pass


class Destructor(Entity):
    "TODO"
    def __init__(self):
        pass


class Incap(Entity):
    def __init__(self, _incap_type: INCAP, _incap_name: str,
                _needs_semicolon: bool = True,
                _child_entities: list = [],
                _indent: int = 0,
                _indent_fixed: bool = False,
                _empty_lines: tuple[int, int] = (0, 0)):
        # __child_entities: list[Entity] = []

        # self.base_classes = BaseClassList([])
        # self.destructor = Destructor()
        # self.constructors: list[Constructor] = []
        # for ent in _child_entities:
        #     if type(ent) is BaseClassList:
        #         self.base_classes = ent
            # elif type(ent) is Constructor:
            #     self.constructors.append(ent)
            # elif type(ent) is Destructor:
            #     self.destructor = ent
            # else:
            #     __child_entities.append(ent)

        super().__init__(_child_entities= _child_entities, _indent= _indent,
                         _indent_fixed= _indent_fixed, _empty_lines= _empty_lines)

        _incap_type: str = str(_incap_type)
        self.incap_type = _incap_type
        self.incap_name = _incap_name
        self.needs_semicolon = _needs_semicolon

    def genstr(self, _indent: int = 0, _indent_len: int = 4) -> str:
        super().genstr(_indent, _indent_len)

        __res = [ self.indent_str + self.incap_type.lower() + " " + self.incap_name + " {",
                  *[ child.genstr(_indent + 1, _indent_len) for child in self.child_entities ],
                  (self.indent_str + "};" if self.needs_semicolon else '}') + f" // {self.incap_name} END" ]
        return self.join_strings(__res)

    def to_dict(self) -> dict:
        return super().to_dict()
        return {
                    "EntityType" : "Incap",
                    "indent" : self.indent,
                    "indent_fixed" : self.indent_fixed,
                    "empty_lines" : self.empty_lines,
                    "incap_type" : self.incap_type,
                    "incap_name" : self.incap_name,
                    "needs_semicolon" : self.needs_semicolon,
                    "child_entities" : [ entity.to_dict()  for entity in self.child_entities ]
               }



#  ============  Text-based (templates, simpler)  ============  #

class Text(Entity):
    def __init__(self, _text: str, _indent: int = 0, _indent_fixed: bool = True,
                _child_entities: list[Entity] = [], _empty_lines: tuple[int, int] = (0, 0)):
        """
            Adds text.
        """
        super().__init__(_child_entities= _child_entities, _indent= _indent,
                         _indent_fixed= _indent_fixed, _empty_lines= _empty_lines)
        self.text = _text

    def genstr(self, _indent: int = 0, _indent_len: int = 4) -> str:
        super().genstr(_indent, _indent_len)

        return self.join_strings([self.text])

    def to_dict(self) -> dict:
        return super().to_dict()
        return {
                    "EntityType" : "Text",
                    "indent" : self.indent,
                    "indent_fixed" : self.indent_fixed,
                    "empty_lines" : self.empty_lines,
                    "text" : self.text,
                    "child_entities" : [ entity.to_dict()  for entity in self.child_entities ]
               }


#  ============  Shortcuts  ============  #

class IncapWithBody(Entity):
    def __init__(self, _incap_type: INCAP, _incap_name: str,
                _incap_body: str,
                _needs_semicolon: bool = True,
                _indent: int = 0,
                _indent_fixed: bool = False,
                _empty_lines: tuple[int, int] = (0, 0)):
        super().__init__(_indent= _indent, _indent_fixed= _indent_fixed, _empty_lines= _empty_lines)

        _incap_type: str = str(_incap_type)
        self.incap_type = _incap_type
        self.incap_name = _incap_name
        self.incap_body = _incap_body
        self.needs_semicolon = _needs_semicolon

    def genstr(self, _indent: int = 0, _indent_len: int = 4) -> str:
        super().genstr(_indent, _indent_len)

        __res = [ self.indent_str + self.incap_type.lower() + " " + self.incap_name + " {", self.incap_body,
                 (self.indent_str + "};" if self.needs_semicolon else '}') + f" // {self.incap_name} END" ]
        return self.join_strings(__res)
        


# t = Incap(INCAP.NAMESPACE, "TestNS")
# # Entity.to_json("./incap.json", [t], _json_indent= 2)

# import json

# with open("./incap.json", "w+") as json_f:
#     json.dump(t.to_dict(), json_f, ensure_ascii=False, indent=2)

# with open("./incap.json") as json_f:
#     y: Incap = Entity.from_dict(json.load(json_f))


# Replaced by other entities


# class Head(Text):
#     def __init__(self, _fname: str, _children: list[Entity] = []):
#         super().__init__(_indent_fixed= True)

#         import re

#         if re.match(r"[a-zA-Z0-9_].[a-zA-Z0-9_]", _fname) is None:
#             raise ValueError("Error: generated file name is not allowed - must consist of letters, numbers, underscore")

#         _fname.replace('.', '_')
#         self.ifndef_param = "SVKFW_" + _fname.upper()
#         self.children = _children

#     def genstr(self, _indent: int = 0, _indent_len: int = 4) -> str:
#         super().genstr(_indent, _indent_len)

#         __res = [ "#ifndef " + self.ifndef_param,
#                   "#define " + self.ifndef_param,
#                   "",
#                   *[ child.genstr(_indent, _indent_len) for child in self.child_entities ],
#                   "" ]

#         return self.join_strings(__res)


# replaced by Preprocessor


# class PreIf(Preprocessor):
#     def __init__(self, _if_type: type[PRE], _condition: str, _children: list[Entity] = []):
#         match _if_type:
#             case PRE.IF:
#                 self.if_type = "if"
#             case PRE.IFDEF:
#                 self.if_type = "ifdef"
#             case PRE.IFNDEF:
#                 self.if_type = "ifndef"
#             case _:
#                 raise ValueError("Error: bad preprocessor IF type: " + str(_if_type))
#         self.condition = _condition
#         self.children = _children

#     def genstr(self) -> list[str]:
#         self.format_counts = [ (i+1, 1)  for i in range(len(self.children)) ]
#         return [ f"#{self.if_type} {self.condition}" ] + [ "{}"  for _ in range(len(self.children)) ] + ["#endif"]


# class PreElse(Preprocessor):
#     def __init__(self, _else_type: type[PRE], _condition: str = ""):
#         # else_type might be used for the new directives (in C++23), but for now it's either "else" or "elif"
#         self.condition = _condition
#         self.is_elif: bool = self.condition != ""

#     def genstr(self) -> list[str]:
#         return [ "#elif " + self.condition if self.is_elif else "#else" ]


# class PreDefines(Preprocessor):
#     def __init__(self, _define_name_val_pairs: list[tuple[str, str]]):
#         """
#             define_name_val_pairs: list of pairs of str.

#                 pair ("name", "val") -> "#define name val"

#                 ("name", "") -> "#define name"
#         """
#         self.defines = _define_name_val_pairs

#     def genstr(self) -> list[str]:
#         return [ "#define " + name + (" " + val if len(val) else "")  for name, val in self.defines ]


# class PreIncludes(Preprocessor):
#     def __init__(self, _headers: list[str]):
#         """
#             _headers: example - ["\<cmath\>", "\"vkwrap.h\""]
#         """
#         self.headers = _headers

#     def genstr(self) -> list[str]:
#         return [ "#include " + header  for header in self.headers ]




# replaced by Incap


# class Namespace(Entity):
#     def __init__(self, _namespace_name: str, _needs_semicolon: bool, _children: list[Entity] = []):
#         super().__init__(1, (1, -1))
#         self.namespace_name = _namespace_name
#         self.needs_semicolon = _needs_semicolon
#         self.children = _children

#     def genstr(self, _indent: int = 0, _indent_len: int = 4) -> str:
#         super().genstr(_indent, _indent_len)

#         __res = [ self.indent_str + "namespace " + self.namespace_name + " {",
#                   *[ child.genstr(_indent + 1, _indent_len) for child in self.child_entities ],
#                   self.indent_str + "};" if self.needs_semicolon else '}' ]
#         return self.join_strings(__res)