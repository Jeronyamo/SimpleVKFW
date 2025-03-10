from gen.lang.common.svkfw_common import Entity


#  ============  Generator  ============  #

class SVKFWCodeGenConfig:
    def __init__(self):
        self.fname: str = ""
        self.fpath: str = ""
        self.indent: int = 0
        self.indent_len: int = 4


class SVKFWCodeGenerator:
    def __init__(self, _config: SVKFWCodeGenConfig):
        self.config: SVKFWCodeGenConfig = _config

    def generate(self, _enitities: list[Entity] = []) -> None:
        """
            Generates strings of entities from ("enitities" argument if not empty or)
            the file_tree with current config parameters and saves them to a file.
        """
        __poss_slash = '\\' if self.config.fpath.find('\\') >= 0 else '/'
        __poss_slash = __poss_slash if self.config.fpath[-1] not in ('\\', '/') else ""
        __fullpath = self.config.fpath + __poss_slash + self.config.fname

        with open(__fullpath, 'w', newline= Entity.SVKFW_NEWLINE) as gen_file:
            for entity in _enitities:
                __res = entity.genstr(self.config.indent, self.config.indent_len)
                gen_file.write(__res)

    def genstr(self, _entity: Entity) -> str:
        """
            Calls entity's genstr() method with current config parameters.
        """
        return _entity.genstr(self.config.indent, self.config.indent_len)