#ifndef SVKFW_ARGPARSER_H
#define SVKFW_ARGPARSER_H

#include <vector>
#include <string>
#include <set>
#include <map>
#include <mutex>

#include "common/memory.h"
#include "common/terminal.h"


namespace Simple {
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

        TermOrNonterm(Nonterminal     _nonterm) : term{}     , nonterm{  _nonterm} {}
        TermOrNonterm(const        char *_term) : term{_term}, nonterm{UINT32_MAX} {}
        TermOrNonterm(const std::string &_term) : term{_term}, nonterm{UINT32_MAX} {}
        TermOrNonterm(const TermOrNonterm &_term_nonterm)
                        : term{_term_nonterm.term}, nonterm{_term_nonterm.nonterm} {}

        bool isNonterm() const { return nonterm != UINT32_MAX; }
    }; // TermOrNonterm END

    namespace ParserReg {
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

            ProdRule(const std::initializer_list<TermOrNonterm> &_terms_nonterms) {
                std::string __rule_str;

                for (const auto &right : _terms_nonterms) {
                    if (right.isNonterm())
                        nt_index.emplace_back(right.nonterm, (uint32_t) __rule_str.size());
                    __rule_str += right.term;
                }

                nt_index.emplace_back(NONTERM_START, __rule_str.size() + 1);
                rule_str = new char[__rule_str.size()+1];
                memcpy(rule_str, __rule_str.data(), __rule_str.size());
                rule_str[__rule_str.size()] = '\0';
            }
            ProdRule(const ProdRule &_prod_rule) : rule_str{new char[_prod_rule.nt_index.back().second]},
                                                   nt_index{_prod_rule.nt_index} {
                memcpy(rule_str, _prod_rule.rule_str, _prod_rule.nt_index.back().second);
            }
            ~ProdRule() {
                safeDeleteArr(rule_str);
            }

            uint32_t getRuleLen() const { return nt_index.back().second; }
            void  offsetNonterm(int _nt_offset) { for (auto &el : nt_index) el.first += _nt_offset; }
            void replaceNonterminal(Nonterminal _old_nt, Nonterminal _new_nt) {
                for (auto &el : nt_index)
                    if (el.first == _old_nt) el.first = _new_nt;
            }
            void replaceNonterminals(const std::map<Nonterminal, Nonterminal> &_nt_mapping) {
                for (auto &el : nt_index)
                    if (_nt_mapping.find(el.first) != _nt_mapping.end())
                        el.first = _nt_mapping.at(el.first);
            }
            std::string printRule(RuleState _state) const {
                std::string __res;
                uint32_t nt_id = 0u;
                for (uint32_t id = 0u; id < getRuleLen(); ++id) {
                    if (_state.substr_id == id && _state.nonterm == NONTERM_START)
                        __res += '`';
                    while (nt_id < nt_index.size() && nt_index[nt_id].second == id) {
                        __res += '<';
                        __res += std::to_string(nt_index[nt_id].first);
                        __res += '>';
                        if (_state.substr_id == id && _state.nonterm == nt_index[nt_id].first)
                            __res += '`';
                        nt_id += 1;
                    }
                    __res += rule_str[id];
                }
                if (_state.substr_id >= getRuleLen())
                    __res += '`';
                if (_state.substr_id > getRuleLen())
                    __res += "ERR";

                return __res;
            }

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
            NextSymbolType checkNext(RuleState _state) const {
                NextSymbolType __res = NEXT_TERMINAL;
                if (_state.substr_id >= getRuleLen())
                    __res = NEXT_ERR;

                bool __curr_state_exists = _state.nonterm == NONTERM_START;
                for (uint32_t i = 0u; i < nt_index.size(); ++i) {
                    if (nt_index[i].second >  _state.substr_id || __res == NEXT_ERR) break;
                    if (nt_index[i].second == _state.substr_id) {
                        if (__curr_state_exists)
                            __res = NEXT_NONTERMINAL;
                        if (nt_index[i].first == _state.nonterm)
                            __curr_state_exists = true;
                    }
                }

                if (__res == NEXT_TERMINAL && _state.substr_id+1 == getRuleLen())
                    __res = NEXT_END;

                return __res;
            }

            bool parseRule(RuleState &_state, char _ch) const {
                bool __parse_success = _state.substr_id+1 < getRuleLen() &&
                                        rule_str[_state.substr_id] == _ch;
                _state.substr_id += uint32_t(__parse_success); // TODO: should stop incrementing on error?
                _state.nonterm = __parse_success ? NONTERM_START : NONTERM_ERR;
                return __parse_success;
            }

            void getNontermNext(RuleState &_state) const {
                bool __reached_last_nt = _state.nonterm == NONTERM_START;
                // printf("Getting Nonterminal - substr id = %d\n", _state.substr_id);
                for (const auto &nt : nt_index) {
                    // printf("Nonterminal %d at %d\n", nt.first, nt.second);
                    if (nt.second >  _state.substr_id) break;
                    if (nt.second == _state.substr_id) {
                        if (__reached_last_nt) {
                            _state.nonterm = nt.first;
                            break;
                        }
                        if (nt.first == _state.nonterm)
                            __reached_last_nt = true;
                    }
                }
            }
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

            RuleBlock(const std::initializer_list<ProdRule> &_prod_rules) : prod_rules{_prod_rules} {
                std::set<uint32_t> __all_values;
                for (const ProdRule &rule : prod_rules) {
                    for (const auto &nt : rule.nt_index) {
                        if (nt.first != NONTERM_START)
                            __all_values.emplace(nt.first);
                        nonterm_exits = std::max(nonterm_exits, nt.first);
                    }
                }

                for (uint32_t i = nonterm_exits; i > 0u; --i) { // fixes holes in nonterminal enumeration
                    if (__all_values.find(i) == __all_values.end()) {
                        nonterm_exits -= 1;
                        for (ProdRule &rule : prod_rules)
                            for (auto &nt : rule.nt_index)
                                if (nt.first >= i) nt.first -= 1;
                    }
                }
            }

            // This parse overload returns all rule states at the beginning of parsing.
            // 'substr' - substring (string with offset)
            std::vector<RuleBlockState> parseBlockInit(const char *_substr, uint32_t _substr_size, uint32_t _rule_id_thold = 0) const {
                std::vector<RuleBlockState> __next_states;

                for (uint32_t i = _rule_id_thold; i < prod_rules.size(); ++i) {
                    RuleBlockState __block_state{{0u, NONTERM_START}, i, NEXT_ERR};
                    __block_state.nextsym = prod_rules[i].checkNext(__block_state.rule_state);

                    if (__block_state.nextsym == NEXT_TERMINAL ?
                            prod_rules[i].parseRule(__block_state.rule_state, _substr[0]) :
                            __block_state.nextsym != NEXT_ERR) { // Note: first terminal is parsed twice, here and below
                        __block_state.rule_state.substr_id -= (__block_state.nextsym == NEXT_TERMINAL);
                        __next_states.push_back(__block_state);
                    }
                }

                for (auto &state : __next_states) {
                    while (state.nextsym == NEXT_TERMINAL) {
                        state.nextsym = NEXT_ERR;
                        // Note: 'substr_size' doesn't count '\0' at the end of string, so "<= substr_size" doesn't lead to UB
                        if (state.rule_state.substr_id <= _substr_size && prod_rules[state.rule_id].parseRule(state.rule_state, _substr[state.rule_state.substr_id])) {
                            state.nextsym = prod_rules[state.rule_id].checkNext(state.rule_state);
                        }
                    }
                }

                for (uint32_t i = __next_states.size(); i > 1u; --i) {
                    if (__next_states[i-1].nextsym == NEXT_ERR)
                        __next_states.erase(__next_states.begin() + (i-1));
                }

                return __next_states;
            }

            // Note: 'term_offset' is needed to count parsed terminals. Used to offset the beginning of parsed string.
            //       Offset from parsed child nonterminal is acquired from child nonterminal's end offset.
            RuleBlockState parseBlock(const char *_substr, uint32_t _substr_size, RuleBlockState _state, uint32_t *_term_offset) const {
                while (_state.nextsym == NEXT_TERMINAL) {
                    _state.nextsym = NEXT_ERR;
                    if (_state.rule_state.substr_id <= _substr_size && prod_rules[_state.rule_id].parseRule(_state.rule_state, _substr[_state.rule_state.substr_id])) {
                        if (_term_offset != nullptr) (*_term_offset) += 1u;
                        _state.nextsym = prod_rules[_state.rule_id].checkNext(_state.rule_state);
                    }
                }
                return _state;
            }

            void replaceNonterminal(Nonterminal _old_nt, Nonterminal _new_nt) {
                for (uint32_t i = 0u; i < prod_rules.size(); ++i)
                    prod_rules[i].replaceNonterminal(_old_nt, _new_nt);
            }
            void replaceNonterminals(const std::map<Nonterminal, Nonterminal> &_nt_mapping) {
                for (uint32_t i = 0u; i < prod_rules.size(); ++i)
                    prod_rules[i].replaceNonterminals(_nt_mapping);
            }
            void  offsetNonterminal(int _nt_offset) {
                nonterm_offset += _nt_offset;
                for (uint32_t i = 0u; i < prod_rules.size(); ++i)
                    prod_rules[i].offsetNonterm(_nt_offset);
            }

            inline NextSymbolType checkNext(RuleBlockState _curr_state) const {
                return _curr_state.nextsym != NEXT_ERR ? prod_rules[_curr_state.rule_id].checkNext(_curr_state.rule_state) : NEXT_ERR;
            }
            inline void getNontermNext(RuleBlockState &_curr_state) const {
                prod_rules[_curr_state.rule_id].getNontermNext(_curr_state.rule_state);
            }
            std::string printRule(const RuleBlockState &_curr_state) const {
                return " -> " + prod_rules[_curr_state.rule_id].printRule(_curr_state.rule_state) + " (" + std::to_string(_curr_state.rule_id) + ')';
            }
        }; // RuleBlock END

        // Note: every RuleElement has only one "incoming" nonterminal: RootNonterm.
        // For constructors: to save filling exits for later, set them to 'rule_blocks.size'/'rule_block_elem.size' + offset (offset >= 0),
        //                   then set them via 'replaceExit(offset, nonterminal)'.
        struct RuleElement {
            std::vector<RuleBlock> rule_blocks;
            uint32_t nonterm_exits = 0, exit_offset = 0;

            RuleElement(const RuleBlock &_single_block) : rule_blocks{{_single_block}}, nonterm_exits{_single_block.nonterm_exits}, exit_offset{1} {}
            RuleElement(const std::map<Nonterminal, std::vector<Nonterminal>> &_rule_connections,
                        const std::vector<RuleBlock> &_rule_blocks) : rule_blocks{_rule_blocks}, exit_offset{(uint32_t)_rule_blocks.size()} {
                for (const auto &conn : _rule_connections) {
                    if (rule_blocks[conn.first].nonterm_exits != conn.second.size())
                        throw std::runtime_error(SVKFW_WRAPERR("ParserReg :: RuleElement constructor (2)",
                                                               "rule block exit count mismatch: block " + std::to_string(conn.first) + " expects " +
                                                               std::to_string(rule_blocks[conn.first].nonterm_exits) + " exits to be set, got " +
                                                               std::to_string(conn.second.size())  ));
                    std::map<Nonterminal, Nonterminal> __nt_mapping{{NONTERM_START, conn.first}};
                    for (uint32_t i = 0u; i < conn.second.size(); ++i) {
                        __nt_mapping[i+1] = conn.second[i];
                        if (conn.second[i] >= exit_offset)
                            nonterm_exits = std::max(nonterm_exits, conn.second[i] - exit_offset + 1u);
                    }
                    rule_blocks[conn.first].replaceNonterminals(__nt_mapping);
                }
                if (_rule_connections.empty())
                    nonterm_exits = -1u;
            }
            // Note: all 'elem_connections' keys must be in [0, rule_block_elem.size)
            // TODO: TEST
            RuleElement(std::vector<RuleElement> _rule_block_elem,
                        const std::map<uint32_t, std::vector<uint32_t>> &_elem_connections) {
                std::vector<uint32_t> __elem_nt_offset(_rule_block_elem.size()); // converts elem offset to rule_block, or nonterm offset
                for (uint32_t i = 0u; i < _rule_block_elem.size(); ++i) {
                    __elem_nt_offset[i] = exit_offset;
                    exit_offset += _rule_block_elem[i].rule_blocks.size();
                }

                for (const auto &conn : _elem_connections) {
                    if (_rule_block_elem[conn.first].nonterm_exits != conn.second.size())
                        throw std::runtime_error(SVKFW_WRAPERR("ParserReg :: RuleElement constructor (3)",
                                                               "rule element exit count mismatch: element " + std::to_string(conn.first) + " expects " +
                                                               std::to_string(_rule_block_elem[conn.first].nonterm_exits) + " exits to be set, got " +
                                                               std::to_string(conn.second.size())  ));
                    std::map<Nonterminal, Nonterminal> __nt_mapping{{NONTERM_START, __elem_nt_offset[conn.first]}};
                    for (uint32_t i = 0u; i < conn.second.size(); ++i) {
                        Nonterminal __exit_nonterm = conn.second[i] + (conn.second[i] >= __elem_nt_offset.size() ? exit_offset : __elem_nt_offset[conn.second[i]]);
                        __nt_mapping[i+1] = __exit_nonterm;
                        if (__exit_nonterm >= exit_offset)
                            nonterm_exits = std::max(nonterm_exits, __exit_nonterm - exit_offset + 1u);
                    }
                    _rule_block_elem[conn.first].replaceNonterminals(__nt_mapping);
                }
            }
            RuleElement(const RuleElement &_elem) : rule_blocks{_elem.rule_blocks}, nonterm_exits{_elem.nonterm_exits}, exit_offset{_elem.exit_offset} {}

            void replaceNonterminal(Nonterminal _old_nt, Nonterminal _new_nt) {
                for (uint32_t i = 0u; i < rule_blocks.size(); ++i)
                    rule_blocks[i].replaceNonterminal(_old_nt, _new_nt);
            }
            void replaceNonterminals(const std::map<Nonterminal, Nonterminal> &_nt_mapping) {
                for (uint32_t i = 0u; i < rule_blocks.size(); ++i)
                    rule_blocks[i].replaceNonterminals(_nt_mapping);
            }

            void replaceExit(uint32_t _exit_id, Nonterminal _new_nt) {
                for (uint32_t i = 0u; i < rule_blocks.size(); ++i)
                    rule_blocks[i].replaceNonterminal(exit_offset + _exit_id, _new_nt);
            }
            void replaceExits(const std::map<Nonterminal, Nonterminal> &_exit_mapping) {
                std::map<Nonterminal, Nonterminal> __nt_mapping;
                for (auto &nt_map : _exit_mapping)
                    __nt_mapping[nt_map.first + exit_offset] = nt_map.second;

                for (uint32_t i = 0u; i < rule_blocks.size(); ++i)
                    rule_blocks[i].replaceNonterminals(__nt_mapping);
            }

            void offsetNonterminals(int _nt_offset) {
                if (int(exit_offset) + _nt_offset < rule_blocks.size())
                    throw std::invalid_argument(SVKFW_WRAPERR("ParserReg :: RuleElement :: offsetNonterminals",
                                                              "bad offset: " + std::to_string(_nt_offset) + " -> root nonterminal would be " +
                                                              std::to_string(int(exit_offset) - int(rule_blocks.size()) - _nt_offset)));
                exit_offset += _nt_offset;
                for (auto &rule_block : rule_blocks)
                    rule_block.offsetNonterminal(_nt_offset);
            }

            Nonterminal getRootNonterm() const { return exit_offset - rule_blocks.size(); };
            bool ignored() const { return nonterm_exits & ~uint32_t(INT32_MAX); }
            void ignore(bool _grammar_ignore = true) {
                nonterm_exits = _grammar_ignore ? nonterm_exits | ~uint32_t(INT32_MAX) :
                                                  nonterm_exits &  uint32_t(INT32_MAX);
            }
        }; // RuleElement END


        // Rules -> RuleBlocks -> RuleElements -> RuleGrammar. This class gathers RuleBlocks/RuleElements and manages nonterminal values to avoid collision.
        struct RuleGrammar {
            std::vector<RuleBlock>  rule_blocks;
            std::set<Nonterminal> elem_nonterms; // nonterminals-RootNonterms of RuleElements.

            RuleGrammar() {}
            RuleGrammar(const std::map<uint32_t, std::vector<uint32_t>> &_elem_connections,
                        const std::vector<RuleElement> &_rule_block_elem) { setRules(_elem_connections, _rule_block_elem); }

            void setRules(const std::map<uint32_t, std::vector<uint32_t>> &_elem_connections,
                          std::vector<RuleElement> _rule_block_elem) {
                uint32_t __exit_offset = 0u;
                std::vector<uint32_t> __elem_nt_offset(_rule_block_elem.size()); // converts elem offset to rule_block, or nonterm offset
                for (uint32_t i = 0u; i < _rule_block_elem.size(); ++i) {
                    __elem_nt_offset[i] = __exit_offset;
                    __exit_offset += _rule_block_elem[i].rule_blocks.size();
                }

                for (const auto &conn : _elem_connections) {
                    if (_rule_block_elem[conn.first].nonterm_exits != conn.second.size())
                        throw std::runtime_error(SVKFW_WRAPERR("ParserReg :: RuleGrammar constructor",
                                                               "rule element exit count mismatch: element " + std::to_string(conn.first) + " expects " +
                                                               std::to_string(_rule_block_elem[conn.first].nonterm_exits) + " exits to be set, got " +
                                                               std::to_string(conn.second.size())  ));

                    for (uint32_t i = 0u; i < conn.second.size(); ++i) {
                        if (conn.second[i] >= __elem_nt_offset.size())
                            throw std::runtime_error(SVKFW_WRAPERR("ParserReg :: RuleGrammar constructor",
                                                                   "rule element " + std::to_string(conn.first) + " has unhandled exits: " +
                                                                   std::to_string(conn.second[i]) + " >= " + std::to_string(__elem_nt_offset.size())  ));

                        Nonterminal __exit_nonterm = conn.second[i] + __elem_nt_offset[conn.second[i]];
                        _rule_block_elem[conn.first].offsetNonterminals(__elem_nt_offset[conn.first]);
                        _rule_block_elem[conn.first].replaceExit(i, __exit_nonterm);
                    }
                }

                for (uint32_t i = 0u; i < _rule_block_elem.size(); ++i) {
                    elem_nonterms.emplace(__elem_nt_offset[i]);
                    rule_blocks.insert(rule_blocks.end(), _rule_block_elem[i].rule_blocks.begin(), _rule_block_elem[i].rule_blocks.end());
                }
            }

            inline std::vector<RuleBlockState> parseGrammarInit(const char *_parse_str, const ParserState &_state) const {
                return rule_blocks[_state.parsed_nonterm].parseBlockInit(_parse_str           + _state.str_index_beg,
                                                                         _state.str_index_end - _state.str_index_beg,
                                                                         _state.block_state.rule_id);
            }

            void parseGrammar(const char *_parse_str, ParserState &_state, uint32_t *_term_offset) const {
                _state.block_state = rule_blocks[_state.parsed_nonterm].parseBlock(_parse_str           + _state.str_index_beg,
                                                                                   _state.str_index_end - _state.str_index_beg,
                                                                                   _state.block_state, _term_offset);
            }

            inline NextSymbolType checkNext(ParserState _state) const {
                NextSymbolType __res_next = rule_blocks[_state.parsed_nonterm].checkNext(_state.block_state);
                // printf("Next state type: %s (last nonterm %d)\n", Next_sym_to_str.at(__res_next).c_str(), _state.block_state.rule_state.nonterm);
                return __res_next;
            }
            inline void getNontermNext(ParserState &_state) const {
                rule_blocks[_state.parsed_nonterm].getNontermNext(_state.block_state);
                // printf("Nonterm to parse: %d -> %d\n", _state.parsed_nonterm, _state.block_state.rule_state.nonterm);
            }
            std::string printRule(const ParserState &_state) const {
                return std::to_string(_state.parsed_nonterm) + rule_blocks[_state.parsed_nonterm].printRule(_state.block_state);
            }
        }; // RuleGrammar END


        // // Note: probably need one 'default' copy of rule blocks and as many 'specified' instances as needed
        // // UINT: rule_digit  (1) -> rule_nonterm_or_empty (2) -> rule_digit (1)
        // //  INT: rule_signed (1) -> UINT (2,3)

        // // Predefined rule blocks

        RuleBlock rule_end     { {""} };
        RuleBlock rule_nonterm { { "", 1u} }; // Note: An element from this rule can be used to synchronize nonterminals from different elements (by only changing its exit nonterm).
        RuleBlock rule_nt_empty{ { "", 1u}, {""} };
        RuleBlock rule_space   { {" ", 1u}, {"\t", 1u}, {"\n", 1u} };
        RuleBlock rule_digit   { {"0", 1u}, { "1", 1u}, { "2", 1u}, {"3", 1u}, {"4", 1u}, {"5", 1u}, {"6", 1u}, {"7", 1u}, {"8", 1u}, {"9", 1u} };
        RuleBlock rule_signed  { {"+", 1u}, { "-", 1u}, {  "", 1u} };


        struct Parser {
            std::vector<ParserState> parsed_nonterms;
            const RuleGrammar *grammar = nullptr;
            const char *parsed_str = nullptr;
            uint32_t parsed_str_size = 0u;


            bool parse(const std::string &_str_to_parse, const RuleGrammar &_grammar) {
                grammar = &_grammar;
                parsed_str      = _str_to_parse.data();
                parsed_str_size = _str_to_parse.size();
                parsed_nonterms = {ParserState{}};

                parseRec();

                bool __parse_success = _grammar.rule_blocks.empty() || (parsed_nonterms.size() &&
                                        parsed_nonterms[0].str_index_end == _str_to_parse.size() &&
                                        parsed_nonterms[0].block_state.nextsym == NEXT_END);
                if (__parse_success)
                    for (uint32_t i = parsed_nonterms.size()-1; i > 0u; --i) // Leave only RuleElements info; never delete NONTERM_START's state
                        if (grammar->elem_nonterms.find(parsed_nonterms[i].parsed_nonterm) ==
                            grammar->elem_nonterms.end())
                                parsed_nonterms.erase(parsed_nonterms.begin() + i);

                grammar    = nullptr;
                parsed_str = nullptr;
                parsed_str_size = 0u;
                return __parse_success;
            }

            void parseRec() {
                if (parsed_nonterms.empty()) return;

                const uint32_t curr_st = parsed_nonterms.size() - 1u;
                uint32_t __offset = 0u, __child_ind = 0u;
                // printf("ParsedNT [%d], starts on %d/%d(%d)\n", parsed_nonterms[curr_st].parsed_nonterm, parsed_nonterms[curr_st].str_index_beg,
                //                                              parsed_nonterms[curr_st].str_index_end, parsed_str_size);
                std::vector<RuleBlockState> __next_states = grammar->parseGrammarInit(parsed_str, parsed_nonterms[curr_st]);
                if (__next_states.empty())
                    parsed_nonterms[curr_st].block_state.nextsym = NEXT_ERR;

                // printf("Next states count: %ld\n", __next_states.size());
                for (const auto &st : __next_states) {
                    ParserState __curr_state = parsed_nonterms[curr_st];
                    __curr_state.block_state = st;
                    // printf("%s\n", grammar->printRule(__curr_state).c_str());
                }
                // if (!__next_states.empty())
                //     printf("\n");

                for (uint32_t i = 0u; i < __next_states.size(); ++i) {
                    parsed_nonterms[curr_st].block_state = __next_states[i];
                    parsed_nonterms[curr_st].str_index_end = parsed_nonterms[curr_st].str_index_beg +
                                                             parsed_nonterms[curr_st].block_state.rule_state.substr_id;
                    // printf("[%d] BEG NState %d - rule id %d, state %s\n", parsed_nonterms[curr_st].parsed_nonterm, i, parsed_nonterms[curr_st].block_state.rule_id,
                    //                                                       Next_sym_to_str.at(parsed_nonterms[curr_st].block_state.nextsym).c_str());
                    while (parsed_nonterms[curr_st].block_state.nextsym == NEXT_NONTERMINAL) {
                        // Parse child nonterminal
                        __child_ind = parsed_nonterms.size();
                        grammar->getNontermNext(parsed_nonterms[curr_st]);
                        parsed_nonterms.push_back(ParserState{RuleBlockState{}, parsed_nonterms[curr_st].block_state.rule_state.nonterm,
                                                                                parsed_nonterms[curr_st].str_index_end,
                                                                                parsed_nonterms[curr_st].str_index_end, (uint32_t)parsed_nonterms.size()});
                        parseRec();
                        parsed_nonterms[curr_st].str_index_end = parsed_nonterms[__child_ind].str_index_end;

                        // Error parsing child nonterminal
                        if (parsed_nonterms[__child_ind].block_state.nextsym != NEXT_END)
                            parsed_nonterms[curr_st].block_state.nextsym = NEXT_ERR;

                        // Parse 0+ terminals before next child nonterminal/end
                        // printf("[%d] MID-1 NState %d - state %s\n", parsed_nonterms[curr_st].parsed_nonterm, i, Next_sym_to_str.at(parsed_nonterms[curr_st].block_state.nextsym).c_str());
                        parsed_nonterms[curr_st].block_state.nextsym = grammar->checkNext(parsed_nonterms[curr_st]);
                        if (parsed_nonterms[curr_st].block_state.nextsym == NEXT_TERMINAL)
                            grammar->parseGrammar(parsed_str, parsed_nonterms[curr_st], &parsed_nonterms[curr_st].str_index_end);
                        // printf("[%d] MID-2 NState %d - rule id %d, state %s\n", parsed_nonterms[curr_st].parsed_nonterm, i, parsed_nonterms[curr_st].block_state.rule_id,
                        //                                                       Next_sym_to_str.at(parsed_nonterms[curr_st].block_state.nextsym).c_str());
                    }
                    // printf("[%d] END NState %d - rule id %d, state %s\n", parsed_nonterms[curr_st].parsed_nonterm, i, parsed_nonterms[curr_st].block_state.rule_id,
                    //                                                       Next_sym_to_str.at(parsed_nonterms[curr_st].block_state.nextsym).c_str());
                    if (parsed_nonterms[curr_st].block_state.nextsym == NEXT_END)
                        break;
                    parsed_nonterms.resize(curr_st + 1);
                }
            }

            bool fixRec(uint32_t _state_id) {
                return false;
                if (_state_id >= parsed_nonterms.size()) return false;
                ParserState &__curr_state = parsed_nonterms[_state_id];

                // TODO: try other states on parsing error to handle ambiguity.
            }

            std::vector<std::string> getElements(const std::string &_str_to_parse, Nonterminal _rule_elem) {
                std::vector<std::string> __res;
                for (uint32_t i = 0u; i < parsed_nonterms.size(); ++i)
                    if (parsed_nonterms[i].parsed_nonterm == _rule_elem)
                        __res.emplace_back(_str_to_parse.begin() + parsed_nonterms[i].str_index_beg,
                                           _str_to_parse.begin() + parsed_nonterms[i].str_index_end);

                return __res;
            }
        }; // Parser END
    }; // ParserReg END
}; // Simple END


#endif