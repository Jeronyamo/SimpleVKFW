#ifndef SVKFW_ARGPARSER_H
#define SVKFW_ARGPARSER_H

#include <vector>
#include <string>
#include <set>
#include <map>
#include <mutex>

#include "common/memory.h"
#include "common/terminal.h"
#include "common/utilities.h"
#include "math/vectors.h"


namespace Simple {
    namespace Parser {
        enum NextSymbolType { NEXT_START, NEXT_ERR, NEXT_END, NEXT_NONTERMINAL, NEXT_TERMINAL };
        const std::map<NextSymbolType, std::string> Next_sym_to_str{ {NEXT_START, "NEXT_START"}, {NEXT_ERR, "NEXT_ERR"},
                                                                     {NEXT_END, "NEXT_END"}, {NEXT_NONTERMINAL, "NEXT_NONTERMINAL"},
                                                                     {NEXT_TERMINAL, "NEXT_TERMINAL"} };

        typedef uint32_t Nonterminal;
        constexpr Nonterminal NONTERM_START = 0u;         // the first nonterminal
        constexpr Nonterminal NONTERM_ERR   = UINT32_MAX; // error nonterminal

        struct TermOrNonterm {
            std::string    term;
            Nonterminal nonterm;

            TermOrNonterm(Nonterminal     _nonterm);
            TermOrNonterm(const        char *_term);
            TermOrNonterm(const std::string &_term);
            TermOrNonterm(const TermOrNonterm &_term_nonterm);

            bool isNonterm() const;
        }; // TermOrNonterm END

        struct RuleState {
            uint32_t  substr_id;
            Nonterminal nonterm;
        }; // RuleState END

        struct RuleBlockState {
            RuleState rule_state;
            uint32_t rule_id;
            NextSymbolType nextsym;
        }; // RuleBlockState END

        struct ParserState {
            RuleBlockState block_state;
            Nonterminal parsed_nonterm;
            uint32_t str_index_beg;
            uint32_t str_index_end;
            uint32_t parent_index;
        }; // ParserState END

        // Example: A -> aAbBcCd, A,B,C - nonterminals, a,b,c,d - (possibly empty) terminal strings
        // Only right side of a rule, they are gathered in RuleBlocks
        struct ProdRule {
            char* rule_str = nullptr;
            std::vector<std::pair<Nonterminal, uint32_t>> nt_index;

            ProdRule(const std::vector<TermOrNonterm> &_terms_nonterms);
            ProdRule(const std::initializer_list<TermOrNonterm> &_terms_nonterms);
            ProdRule(const ProdRule &_prod_rule);
            ~ProdRule();

            uint32_t getRuleLen() const;
            void  offsetNonterm(int _nt_offset);
            void replaceNonterminal(Nonterminal _old_nt, Nonterminal _new_nt);
            void replaceNonterminals(const std::map<Nonterminal, Nonterminal> &_nt_mapping);
            void replaceRootNonterminal(Nonterminal _new_nt);
            std::string printRule(RuleState _state) const;

            // Algorithm examples:
            //
            // 'A -> aBCd' (B,C - nonterminals):
            //     checkNext(0, NONTERM_START)      -> NEXT_TERMINAL,
            //     parse('a', 0)                    -> true,
            //
            //     checkNext(1, NONTERM_START)      -> NEXT_NONTERMINAL,
            //     getNontermNext(1, NONTERM_START) -> B,
            //     <parse B>                        -> true,
            //
            //     checkNext(1, B)                  -> NEXT_NONTERMINAL,
            //     getNontermNext(1, B)             -> C,
            //     <parse C>                        -> true,
            //
            //     checkNext(1, C)                  -> NEXT_TERMINAL,
            //     parse('d', 1)                    -> true,
            //
            //     checkNext(2, NONTERM_START)      -> NEXT_END,
            //
            //
            // 'A -> BC' (B,C - nonterminals):
            //     checkNext(0, NONTERM_START)      -> NEXT_NONTERMINAL,
            //     getNontermNext(0, NONTERM_START) -> B,
            //     <parse B>                        -> true,
            //
            //     checkNext(0, B)                  -> NEXT_NONTERMINAL,
            //     getNontermNext(0, B)             -> C,
            //     <parse C>                        -> true,
            //
            //     checkNext(0, C)                  -> NEXT_END,
            //
            //
            // 'A ->' (empty rule):
            //     checkNext(0, NONTERM_START)      -> NEXT_END,
            //
            NextSymbolType checkNext(RuleState _state) const;

            bool parseRule(RuleState &_state, char _ch) const;

            void getNontermNext(RuleState &_state) const;

            void getExitNonterms(std::set<Nonterminal> &_exit_nt_set) const;
        }; // ProdRule END

        // TODO: bring back nonterm IDs to support "aAAb" rules, where A - nonterminal

        // A number of rules with same nonterminal on the left side. Describes a simple element of parser, like a digit, a letter, etc.
        // Note: When initialized (i.e. from a config), left_nonterminal is zero and right-side nonterminals enumerated from 0 to N.
        //       Then, when these blocks are gathered, offset is added to all nonterminals. Different offsets give different instances of the same element.
        //       On the other hand, same instance can be used everywhere where we aren't interested in this particular parse element.
        struct RuleBlock {
            std::vector<ProdRule> prod_rules;
            uint32_t nonterm_exits = 0; // Shows how many nonterminals with positive ids (external nonterminals) are in this block. All must be connected to actual nonterminals.
            int nonterm_offset = 0; // not needed, 4byte padding

            RuleBlock(const std::initializer_list<ProdRule> &_prod_rules);
            RuleBlock(const RuleBlock &_block);

            // This parse overload returns all rule states at the beginning of parsing.
            // 'substr' - substring (string with offset)
            std::vector<RuleBlockState> parseBlockInit(const char *_substr, uint32_t _substr_size, uint32_t _rule_id_thold = 0) const;

            // Note: 'term_offset' is needed to count parsed terminals. Used to offset the beginning of parsed string.
            //       Offset from parsed child nonterminal is acquired from child nonterminal's end offset.
            RuleBlockState parseBlock(const char *_substr, uint32_t _substr_size, RuleBlockState _state, uint32_t *_term_offset) const;

            void replaceNonterminal(Nonterminal _old_nt, Nonterminal _new_nt);
            void replaceNonterminals(const std::map<Nonterminal, Nonterminal> &_nt_mapping);
            void  offsetNonterminal(int _nt_offset);

            NextSymbolType checkNext(RuleBlockState _curr_state) const;
            void getNontermNext(RuleBlockState &_curr_state) const;
            std::string printRule(const RuleBlockState &_curr_state) const;

            void getExitNonterminals(std::set<Nonterminal> &_exit_nt_set) const;
        }; // RuleBlock END

        // Note: every RuleElement has only one "incoming" nonterminal: RootNonterm.
        // For constructors: to save filling exits for later, set them to 'rule_blocks.size'/'rule_block_elem.size' + offset (offset >= 0),
        //                   then set them via 'replaceExit(offset, nonterminal)'.
        struct RuleElement {
            std::vector<RuleBlock> rule_blocks;
            uint32_t nonterm_exits = 0, exit_offset = 0;

            RuleElement(const RuleBlock &_single_block);
            RuleElement(const std::map<Nonterminal, std::vector<Nonterminal>> &_rule_connections,
                        const std::vector<RuleBlock> &_rule_blocks);
            // Note: all 'elem_connections' keys must be in [0, rule_block_elem.size)
            // TODO: TEST
            RuleElement(std::vector<RuleElement> _rule_block_elem,
                        const std::map<uint32_t, std::vector<uint32_t>> &_elem_connections);
            RuleElement(const RuleElement &_elem);

            void replaceNonterminal(Nonterminal _old_nt, Nonterminal _new_nt);
            void replaceNonterminals(const std::map<Nonterminal, Nonterminal> &_nt_mapping);

            void replaceExit(uint32_t _exit_id, Nonterminal _new_nt);
            void replaceExits(const std::map<Nonterminal, Nonterminal> &_exit_mapping);

            void offsetNonterminals(int _nt_offset);

            Nonterminal getRootNonterm() const;;
            bool ignored() const;
            void ignore(bool _grammar_ignore = true);
        }; // RuleElement END


        // Rules -> RuleBlocks -> RuleElements -> RuleGrammar. This class gathers RuleBlocks/RuleElements and manages nonterminal values to avoid collision.
        struct RuleGrammar {
            std::vector<RuleBlock>  rule_blocks;
            std::set<Nonterminal> elem_nonterms; // nonterminals-RootNonterms of RuleElements.

            RuleGrammar();
            RuleGrammar(const std::map<uint32_t, std::vector<uint32_t>> &_elem_connections,
                        const std::vector<RuleElement> &_rule_block_elem);

            void setRules(const std::map<uint32_t, std::vector<uint32_t>> &_elem_connections,
                          std::vector<RuleElement> _rule_block_elem);

            std::vector<RuleBlockState> parseGrammarInit(const char *_parse_str, const ParserState &_state) const;

            void parseGrammar(const char *_parse_str, ParserState &_state, uint32_t *_term_offset) const;

            NextSymbolType checkNext(ParserState _state) const;
            void getNontermNext(ParserState &_state) const;
            std::string printRule(const ParserState &_state) const;
        }; // RuleGrammar END


        struct Parser {
            std::vector<ParserState> parsed_nonterms;
            const RuleGrammar *grammar = nullptr;
            const char *parsed_str = nullptr;
            uint32_t parsed_str_size = 0u;


            bool parse(const std::string &_str_to_parse, const RuleGrammar &_grammar);

            void parseRec();

            bool fixRec(uint32_t _state_id);

            std::vector<std::string> getElements(const std::string &_str_to_parse, Nonterminal _rule_elem);
        }; // Parser END

        // // Note: probably need one 'default' copy of rule blocks and as many 'specified' instances as needed
        // // UINT: rule_digit  (1) -> rule_nonterm_or_empty (2) -> rule_digit (1)
        // //  INT: rule_signed (1) -> UINT (2,3)

        // // Predefined rule blocks

        namespace Blocks {
            extern RuleBlock b_end;
            extern RuleBlock b_space;
            extern RuleBlock b_digit;
            extern RuleBlock b_signed;

            // Returns a RuleBlock with one rule: "<Nt1><Nt2><Nt3>...<NtX>" where X is 'nt_count' (plus second, empty rule if 'empty_allowed' is true).
            RuleBlock bNt(uint32_t _nt_count, bool _empty_allowed = false);

            RuleBlock bLetterUpper(bool _empty_allowed = false);

            RuleBlock bLetterLower(bool _empty_allowed = false);

            RuleBlock bLetter(bool _empty_allowed = false);
        }; // Blocks END

        namespace Elements {
            extern RuleElement e_uint;
            extern RuleElement e_int;
        }; // Elements END
    }; // Parser END


//  ================  ArgParser  ================  \\

    template <typename T>
    inline T argumentCaster(const std::string &_casted_string);
    template <typename T>
    inline Vec2Base<T> argumentCaster(const std::string &_str_x, const std::string &_str_y) {
        return Vec2Base<T>{ argumentCaster<T>(_str_x), argumentCaster<T>(_str_y) };
    }
    template <typename T>
    inline Vec3Base<T> argumentCaster(const std::string &_str_x, const std::string &_str_y, const std::string &_str_z) {
        return Vec3Base<T>{ argumentCaster<T>(_str_x), argumentCaster<T>(_str_y), argumentCaster<T>(_str_z) };
    }
    template <typename T>
    inline Vec4Base<T> argumentCaster(const std::string &_str_x, const std::string &_str_y, const std::string &_str_z, const std::string &_str_w) {
        return Vec4Base<T>{ argumentCaster<T>(_str_x), argumentCaster<T>(_str_y), argumentCaster<T>(_str_z), argumentCaster<T>(_str_w) };
    }

    template <>
    int      argumentCaster<int     >(const std::string &_casted_string);
    template <>
    uint8_t  argumentCaster<uint8_t >(const std::string &_casted_string);
    template <>
    uint16_t argumentCaster<uint16_t>(const std::string &_casted_string);
    template <>
    uint32_t argumentCaster<uint32_t>(const std::string &_casted_string);
    template <>
    float    argumentCaster<float   >(const std::string &_casted_string);
    template <>
    double   argumentCaster<double  >(const std::string &_casted_string);


    struct ArgParser {
        struct ArgDescription {
            std::string arg_name, arg_alias, arg_description;
            std::vector<uint32_t> arg_counts;

            static void ValidateName(const std::string &_arg_name);
            static void ValidateAlias(const std::string &_arg_alias);
            static void ValidateCounts(const std::string &_arg_name_or_alias, const std::vector<uint32_t> &_arg_counts);


            ArgDescription();
            ArgDescription(const std::string &_arg_name, const std::string &_arg_alias, const std::vector<uint32_t> &_arg_counts);
            ArgDescription(const std::string &_arg_name, const std::vector<uint32_t> &_arg_counts);
            ArgDescription(const ArgDescription &_arg) = default;


            bool checkArgName (char *_arg) const;
            bool checkArgAlias(char *_arg) const;

            void setDescription(const std::string &_arg_name, const std::string &_arg_alias, const std::vector<uint32_t> &_arg_counts, const std::string &_arg_descr = "");

            std::string getArgNamePrint() const;

            std::vector<std::string> getFullDescription() const;
        }; // ArgDescription END

        struct ArgNamespace { // nested set of arguments
            std::string namespace_name;
            std::vector<ArgNamespace> nested_args;
            std::vector<ArgDescription> arguments;

            ArgNamespace(const std::string &_namespace_name);
            ArgNamespace(const ArgNamespace &_namespace) = default;
            ArgNamespace(ArgNamespace &&_namespace);
           ~ArgNamespace();

            void addArgument(const std::string &_arg_name, const std::string &_arg_alias, const std::vector<uint32_t> &_arg_counts);
            void addArgument(const std::string &_arg_name, const std::vector<uint32_t> &_arg_counts);
            void createNestedNamespace(const std::string &_namespace_name);
            void addNestedNamespace(const ArgNamespace &_nested_namespace);
            void addNestedNamespace(ArgNamespace &&_nested_namespace);

            ArgNamespace& getNestedNamespace(const std::string &_namespace_name);

            std::vector<std::string> getDescription() const;

            static std::string ParseSingleArgument(std::string _res);
            static std::vector<std::vector<std::string>> ParseArgument(const ArgDescription &_parsed_arg, int _sub_argc, char **_sub_argv);
        }; // ArgNamespace END


        Util::EnumCollector    enum_collector;
        // std::vector<ArgNamespace> nested_args;
        std::vector<ArgDescription> arguments;
        std::vector<std::pair<uint32_t, std::vector<std::vector<std::string>>>> argument_values;
        ArgHandler arg_handler;

        ArgParser();
        ArgParser(int _prog_argc, char** _prog_argv);

        void setArgumentSource(int _argc, char** _argv);

        const ArgDescription* getArgDescriptionByName(const std::string &_arg_name) const;

        uint32_t getArgDescriptionIdByName(const std::string &_arg_name) const;

        std::vector<uint32_t> getArgParseIdsByName(const std::string &_arg_name) const;

        bool hasArgumentParsed(const std::string &_arg_name) const;


        void addArgument(const std::string &_arg_name, const std::string &_arg_alias, const std::vector<uint32_t> &_arg_counts);

        void addArgument(const std::string &_arg_name, const std::vector<uint32_t> &_arg_counts);

        void parseAllArguments();

        template <typename T>
        T castArgument(const std::string &_arg_name, uint32_t _arg_value_id = 0u) const {
            T __res = -1;

            uint32_t __arg_descr = getArgDescriptionIdByName(_arg_name);
            if (__arg_descr != UINT32_MAX && arguments[__arg_descr].arg_counts.size() > _arg_value_id) {
                SVKFW_ASSERT(arguments[__arg_descr].arg_counts[_arg_value_id] == 1u, std::runtime_error, "Simple::ArgParser :: castArgument(scalar)",
                                "trying to cast scalar from a vector argument value");
                std::vector<uint32_t> __arg_first_parse = getArgParseIdsByName(_arg_name);

                SVKFW_ASSERT(!__arg_first_parse.empty(), std::runtime_error, "Simple::ArgParser :: castArgument(scalar)",
                                "argument " + arguments[__arg_descr].getArgNamePrint() + "wasn't passed to the program");
                __res = argumentCaster<T>(argument_values[__arg_first_parse[0]].second[_arg_value_id][0]);
            }
            return __res;
        }

        template <typename T>
        Vec2Base<T> castArgument2(const std::string &_arg_name, uint32_t _arg_value_id) const {
            Vec2Base<T> __res;

            uint32_t __arg_descr = getArgDescriptionIdByName(_arg_name);
            if (__arg_descr != UINT32_MAX && arguments[__arg_descr].arg_counts.size() > _arg_value_id) {
                SVKFW_ASSERT(arguments[__arg_descr].arg_counts[_arg_value_id] == 2u, std::runtime_error, "Simple::ArgParser :: castArgument(vector2)",
                                "vector2/argument value mismatch");
                std::vector<uint32_t> __arg_first_parse = getArgParseIdsByName(_arg_name);

                SVKFW_ASSERT(!__arg_first_parse.empty(), std::runtime_error, "Simple::ArgParser :: castArgument(vector2)",
                                "argument " + arguments[__arg_descr].getArgNamePrint() + "wasn't passed to the program");
                const std::vector<std::string> &__values = argument_values[__arg_first_parse[0]].second[_arg_value_id];
                __res = { argumentCaster<T>(__values[0]), argumentCaster<T>(__values[1]) };
            }
            return __res;
        }

        template <typename T>
        Vec3Base<T> castArgument3(const std::string &_arg_name, uint32_t _arg_value_id) const {
            Vec3Base<T> __res;

            uint32_t __arg_descr = getArgDescriptionIdByName(_arg_name);
            if (__arg_descr != UINT32_MAX && arguments[__arg_descr].arg_counts.size() > _arg_value_id) {
                SVKFW_ASSERT(arguments[__arg_descr].arg_counts[_arg_value_id] == 3u, std::runtime_error, "Simple::ArgParser :: castArgument(vector3)",
                                "vector3/argument value mismatch");
                std::vector<uint32_t> __arg_first_parse = getArgParseIdsByName(_arg_name);

                SVKFW_ASSERT(!__arg_first_parse.empty(), std::runtime_error, "Simple::ArgParser :: castArgument(vector3)",
                                "argument " + arguments[__arg_descr].getArgNamePrint() + "wasn't passed to the program");
                const std::vector<std::string> &__values = argument_values[__arg_first_parse[0]].second[_arg_value_id];
                __res = { argumentCaster<T>(__values[0]), argumentCaster<T>(__values[1]), argumentCaster<T>(__values[2]) };
            }
            return __res;
        }

        template <typename T>
        Vec4Base<T> castArgument4(const std::string &_arg_name, uint32_t _arg_value_id) const {
            Vec4Base<T> __res;

            uint32_t __arg_descr = getArgDescriptionIdByName(_arg_name);
            if (__arg_descr != UINT32_MAX && arguments[__arg_descr].arg_counts.size() > _arg_value_id) {
                SVKFW_ASSERT(arguments[__arg_descr].arg_counts[_arg_value_id] == 4u, std::runtime_error, "Simple::ArgParser :: castArgument(vector4)",
                                "vector4/argument value mismatch");
                std::vector<uint32_t> __arg_first_parse = getArgParseIdsByName(_arg_name);

                SVKFW_ASSERT(!__arg_first_parse.empty(), std::runtime_error, "Simple::ArgParser :: castArgument(vector4)",
                                "argument " + arguments[__arg_descr].getArgNamePrint() + "wasn't passed to the program");
                const std::vector<std::string> &__values = argument_values[__arg_first_parse[0]].second[_arg_value_id];
                __res = { argumentCaster<T>(__values[0]), argumentCaster<T>(__values[1]), argumentCaster<T>(__values[2]), argumentCaster<T>(__values[3]) };

            }
            return __res;
        }
    }; // ArgParser END
}; // Simple END


#endif