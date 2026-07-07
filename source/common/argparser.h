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

            TermOrNonterm(Nonterminal     _nonterm) : term{}     , nonterm{  _nonterm} {}
            TermOrNonterm(const        char *_term) : term{_term}, nonterm{UINT32_MAX} {}
            TermOrNonterm(const std::string &_term) : term{_term}, nonterm{UINT32_MAX} {}
            TermOrNonterm(const TermOrNonterm &_term_nonterm)
                            : term{_term_nonterm.term}, nonterm{_term_nonterm.nonterm} {}

            bool isNonterm() const { return nonterm != UINT32_MAX; }
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

            ProdRule(const std::vector<TermOrNonterm> &_terms_nonterms) {
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
            ProdRule(const std::initializer_list<TermOrNonterm> &_terms_nonterms)
                            : ProdRule{std::vector<TermOrNonterm>{_terms_nonterms}} {}
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
            void replaceRootNonterminal(Nonterminal _new_nt) { replaceNonterminal(nt_index.back().first, _new_nt); }
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

            void getExitNonterms(std::set<Nonterminal> &_exit_nt_set) const {
                for (const auto &elem : nt_index)
                    if (elem.first != nt_index.back().first)
                        _exit_nt_set.emplace(elem.first);
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
            RuleBlock(const RuleBlock &_block) : prod_rules{_block.prod_rules}, nonterm_exits{_block.nonterm_exits},
                                                 nonterm_offset{_block.nonterm_offset} {}

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

            void getExitNonterminals(std::set<Nonterminal> &_exit_nt_set) const {
                for (const auto &rule : prod_rules)
                    rule.getExitNonterms(_exit_nt_set);
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

        // // Note: probably need one 'default' copy of rule blocks and as many 'specified' instances as needed
        // // UINT: rule_digit  (1) -> rule_nonterm_or_empty (2) -> rule_digit (1)
        // //  INT: rule_signed (1) -> UINT (2,3)

        // // Predefined rule blocks

        namespace Blocks {
            RuleBlock b_end   { {""} };
            RuleBlock b_space { {" "}, {"\t"}, {"\n"} };
            RuleBlock b_digit { {"0"}, {"1"}, {"2"}, {"3"}, {"4"}, {"5"}, {"6"}, {"7"}, {"8"}, {"9"} };
            RuleBlock b_signed{ {"+"}, {"-"}, {""} };

            // Returns a RuleBlock with one rule: "<Nt1><Nt2><Nt3>...<NtX>" where X is 'nt_count' (plus second, empty rule if 'empty_allowed' is true).
            RuleBlock bNt(uint32_t _nt_count, bool _empty_allowed = false) {
                std::vector<TermOrNonterm> __res((size_t)_nt_count, NONTERM_START);
                for (uint32_t i = 0u; i < _nt_count; ++i)
                    __res[i].nonterm = i+1;
                return _empty_allowed ? RuleBlock{ProdRule{__res}, ProdRule{""}} : RuleBlock{ProdRule{__res}};
            }

            RuleBlock bLetterUpper(bool _empty_allowed = false) {
                std::vector<TermOrNonterm> __res((size_t)26, NONTERM_ERR);
                for (uint32_t i = 0u; i < 26; ++i)
                    __res[i].term = 'A' + i;
                return _empty_allowed ? RuleBlock{ProdRule{__res}, ProdRule{""}} : RuleBlock{ProdRule{__res}};
            }

            RuleBlock bLetterLower(bool _empty_allowed = false) {
                std::vector<TermOrNonterm> __res((size_t)26, NONTERM_ERR);
                for (uint32_t i = 0u; i < 26; ++i)
                    __res[i].term = 'a' + i;
                return _empty_allowed ? RuleBlock{ProdRule{__res}, ProdRule{""}} : RuleBlock{ProdRule{__res}};
            }

            RuleBlock bLetter(bool _empty_allowed = false) {
                std::vector<TermOrNonterm> __res((size_t)52, NONTERM_ERR);
                for (uint32_t i = 0u; i < 26; ++i)
                    __res[i].term = 'A' + i;
                for (uint32_t i = 0u; i < 26; ++i)
                    __res[i+26].term = 'a' + i;
                return _empty_allowed ? RuleBlock{ProdRule{__res}, ProdRule{""}} : RuleBlock{ProdRule{__res}};
            }
        }; // Blocks END

        namespace Elements {
            RuleElement e_uint{{ {0, {1,2}}, {2, {3,2}} }, { Blocks::bNt(2), Blocks::b_digit, Blocks::bNt(2, true), Blocks::b_digit }};
            RuleElement e_int {{ {0, {1,2}}, {2, {3,4}}, {4, {5,4}} }, { Blocks::bNt(2), Blocks::b_signed, Blocks::bNt(2), Blocks::b_digit, Blocks::bNt(2, true), Blocks::b_digit }};
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
    inline int      argumentCaster<int     >(const std::string &_casted_string) { return std::stoi(_casted_string); }
    template <>
    inline uint8_t  argumentCaster<uint8_t >(const std::string &_casted_string) { return (uint8_t ) std::stoul(_casted_string); }
    template <>
    inline uint16_t argumentCaster<uint16_t>(const std::string &_casted_string) { return (uint16_t) std::stoul(_casted_string); }
    template <>
    inline uint32_t argumentCaster<uint32_t>(const std::string &_casted_string) { return (uint32_t) std::stoul(_casted_string); }
    template <>
    inline float    argumentCaster<float   >(const std::string &_casted_string) { return std::stof(_casted_string); }
    template <>
    inline double   argumentCaster<double  >(const std::string &_casted_string) { return std::stod(_casted_string); }


    struct ArgParser {
        struct ArgDescription {
            std::string arg_name, arg_alias, arg_description;
            std::vector<uint32_t> arg_counts;

            static void ValidateName(const std::string &_arg_name) {
                SVKFW_ASSERT(   !_arg_name.empty(), std::invalid_argument, "ArgParser :: ArgDescription :: ValidateName",
                                "Name is empty");
                SVKFW_ASSERT(isalpha(_arg_name[0]), std::invalid_argument, "ArgParser :: ArgDescription :: ValidateName",
                                "Argument name '" + _arg_name + "' doesn't start with a letter");

                for (uint32_t i = 1u; i < _arg_name.size(); ++i) {
                    SVKFW_ASSERT(isalnum(_arg_name[i]) || _arg_name[i] == '_', std::invalid_argument, "ArgParser :: ArgDescription :: ValidateName",
                                    "Argument name '" + _arg_name + "' - symbol '" + _arg_name[i] + "' isn't allowed");
                }
            }
            static void ValidateAlias(const std::string &_arg_alias) {
                if (_arg_alias.empty()) return;

                SVKFW_ASSERT(isalpha(_arg_alias[0]), std::invalid_argument, "ArgParser :: ArgDescription :: ValidateAlias",
                                "Argument alias '" + _arg_alias + "' doesn't start with a letter");

                for (uint32_t i = 1u; i < _arg_alias.size(); ++i) {
                    SVKFW_ASSERT(isalnum(_arg_alias[i]) || _arg_alias[i] == '_', std::invalid_argument, "ArgParser :: ArgDescription :: ValidateAlias",
                                    "Argument alias '" + _arg_alias + "' - symbol '" + _arg_alias[i] + "' isn't allowed");
                }
            }
            static void ValidateCounts(const std::string &_arg_name_or_alias, const std::vector<uint32_t> &_arg_counts) {
                for (uint32_t i = 0u; i < _arg_counts.size(); ++i) {
                    SVKFW_ASSERT(_arg_counts[i] != 0 || i+1 == _arg_counts.size(), std::invalid_argument, "ArgParser :: ArgDescription :: ValidateCounts",
                                    "Parameter (" + _arg_name_or_alias + " :: " + std::to_string(i) + "): unspecified length of a vector is not at the end (" + std::to_string(_arg_counts.size()) + ")");
                }
            }


            ArgDescription() {}
            ArgDescription(const std::string &_arg_name, const std::string &_arg_alias, const std::vector<uint32_t> &_arg_counts)
                            : arg_name{_arg_name}, arg_alias{_arg_alias}, arg_counts{arg_counts}, arg_description{} {
                ValidateName(arg_name);
                ValidateAlias(arg_alias);
                ValidateCounts(arg_name, arg_counts);
            }
            ArgDescription(const std::string &_arg_name, const std::vector<uint32_t> &_arg_counts)
                            : ArgDescription{_arg_name, "", _arg_counts} {}
            ArgDescription(const ArgDescription &_arg) = default;


            bool checkArgName (char *_arg) const { return ("--" + arg_name) == _arg; }
            bool checkArgAlias(char *_arg) const { return !arg_alias.empty() && ('-' + arg_alias) == _arg; }

            void setDescription(const std::string &_arg_name, const std::string &_arg_alias, const std::vector<uint32_t> &_arg_counts, const std::string &_arg_descr = "") {
                ValidateName(_arg_name);
                ValidateAlias(_arg_alias);
                ValidateCounts(_arg_name, _arg_counts);

                arg_name = _arg_name;
                arg_alias = _arg_alias;
                arg_counts = _arg_counts;
                arg_description = _arg_descr;
            }

            std::string getArgNamePrint() const { return "--" + arg_name + (arg_alias.empty() ? std::string("") : (" (-" + arg_alias + ")")); }

            std::vector<std::string> getFullDescription() const {
                std::vector<std::string> __res;

                __res.push_back(getArgNamePrint());
                for (uint32_t i = 0u; i < arg_counts.size(); ++i)
                    __res.back() += " - " + std::to_string(arg_counts[i]);

                if (!arg_description.empty())
                    __res.push_back("  Description: " + arg_description);
                return __res;
            }
        }; // ArgDescription END

        struct ArgNamespace { // nested set of arguments
            std::string namespace_name;
            std::vector<ArgNamespace> nested_args;
            std::vector<ArgDescription> arguments;

            ArgNamespace(const std::string &_namespace_name) : namespace_name{_namespace_name} {}
            ArgNamespace(const ArgNamespace &_namespace) = default;
            ArgNamespace(ArgNamespace &&_namespace) : namespace_name{_namespace.namespace_name},
                                                      nested_args{_namespace.nested_args}, arguments{_namespace.arguments} {
                _namespace.namespace_name.clear();
                _namespace.nested_args.clear();
                _namespace.arguments.clear();
            }
           ~ArgNamespace() {}

            void addArgument(const std::string &_arg_name, const std::string &_arg_alias, const std::vector<uint32_t> &_arg_counts) {
                ArgParser::ArgDescription __new_arg;
                __new_arg.setDescription(_arg_name, _arg_alias, _arg_counts);

                for (uint32_t i = 0u; i < arguments.size(); ++i) {
                    if (arguments[i].arg_name  == _arg_name)
                        throw std::invalid_argument(SVKFW_WRAPERR("ArgParser :: ArgNamespace :: addArgument", "Argument name '"  + _arg_name  + "' is already used"));
                    if (arguments[i].arg_alias == _arg_alias)
                        throw std::invalid_argument(SVKFW_WRAPERR("ArgParser :: ArgNamespace :: addArgument", "Argument alias '" + _arg_alias + "' is already used"));
                }
                arguments.push_back(__new_arg);
            }
            void addArgument(const std::string &_arg_name, const std::vector<uint32_t> &_arg_counts) {
                addArgument(_arg_name, "", _arg_counts);
            }
            void createNestedNamespace(const std::string &_namespace_name) {
                nested_args.emplace_back(_namespace_name);
            }
            void addNestedNamespace(const ArgNamespace &_nested_namespace) {
                nested_args.push_back(_nested_namespace);
            }
            void addNestedNamespace(ArgNamespace &&_nested_namespace) {
                nested_args.push_back(std::move(_nested_namespace));
            }

            ArgNamespace& getNestedNamespace(const std::string &_namespace_name) {
                for (int i = nested_args.size()-1; i >= 0; --i) {
                    if (nested_args[i].namespace_name == _namespace_name)
                        return nested_args[i];
                }
                throw std::invalid_argument(SVKFW_WRAPERR("ArgParser :: ArgNamespace :: getNestedNamespace",
                        "No nested namespace with name '" + _namespace_name + "' in namespace '" + namespace_name + "'"));
                return nested_args[0];
            }

            std::vector<std::string> getDescription() const {
                std::vector<std::string> __res;
                // TODO:
                return __res;
            }

            static std::string ParseSingleArgument(std::string _res) {
                if (!_res.empty() && _res.back() == ',') _res.erase(_res.end()-1);
                if (!_res.empty() && _res[0]     == ',') _res.erase(_res.begin());
                return _res;
            }
            static std::vector<std::vector<std::string>> ParseArgument(const ArgDescription &_parsed_arg, int _sub_argc, char **_sub_argv) {
                std::vector<std::vector<std::string>> __res(_parsed_arg.arg_counts.size(), std::vector<std::string>{});

                uint32_t __arg_index = 0u;
                std::string __arg_tmp_storage;
                for (uint32_t i = 0u; i < _parsed_arg.arg_counts.size(); ++i) {
                    for (uint32_t j = 0u; j < _parsed_arg.arg_counts[i]; ++j) {
                        __arg_tmp_storage.clear();
                        while (__arg_tmp_storage.empty()) {
                            SVKFW_ASSERT(__arg_index < _sub_argc, std::runtime_error, "ArgParser::ArgNamespace :: ParseArgument",
                                            "argument count mismatch: argument " + _parsed_arg.getArgNamePrint() + ", state (" + std::to_string(i) + "/" + std::to_string(_parsed_arg.arg_counts.size()) + ", " + std::to_string(j) + ":" + std::to_string(_parsed_arg.arg_counts[i]) + "); argc " + std::to_string(_sub_argc));
                            __arg_tmp_storage = ParseSingleArgument(_sub_argv[__arg_index++]);
                        }
                        __res[i].push_back(__arg_tmp_storage);
                    }
                    if (_parsed_arg.arg_counts[i] == 0u && i+1 == _parsed_arg.arg_counts.size()) {
                        for (__arg_index; __arg_index < _sub_argc; ++__arg_index) {
                            __arg_tmp_storage = ParseSingleArgument(_sub_argv[__arg_index]);
                            if (!__arg_tmp_storage.empty())
                                __res[i].push_back(__arg_tmp_storage);
                        }
                    }
                }
                return __res;
            }
        }; // ArgNamespace END


        Util::EnumCollector    enum_collector;
        // std::vector<ArgNamespace> nested_args;
        std::vector<ArgDescription> arguments;
        std::vector<std::pair<uint32_t, std::vector<std::vector<std::string>>>> argument_values;
        ArgHandler arg_handler;

        ArgParser() {}
        ArgParser(int _prog_argc, char** _prog_argv) : arg_handler{_prog_argc, _prog_argv} {}

        void setArgumentSource(int _argc, char** _argv) { arg_handler.initArgs(_argc, _argv); }

        const ArgDescription* getArgDescriptionByName(const std::string &_arg_name) const {
            for (uint32_t i = 0u; i < arguments.size(); ++i)
                if (arguments[i].arg_name == _arg_name) return arguments.data() + i;
            return nullptr;
        }

        uint32_t getArgDescriptionIdByName(const std::string &_arg_name) const {
            for (uint32_t i = 0u; i < arguments.size(); ++i)
                if (arguments[i].arg_name == _arg_name) return i;
            return UINT32_MAX;
        }

        std::vector<uint32_t> getArgParseIdsByName(const std::string &_arg_name) const {
            std::vector<uint32_t> __res;

            uint32_t __arg_id = getArgDescriptionIdByName(_arg_name);
            if (__arg_id == UINT32_MAX) return __res;

            for (uint32_t i = 0u; i < argument_values.size(); ++i)
                if (argument_values[i].first == __arg_id) __res.push_back(i);
            return __res;
        }

        bool hasArgumentParsed(const std::string &_arg_name) const {
            uint32_t __arg_id = getArgDescriptionIdByName(_arg_name);
            if (__arg_id == UINT32_MAX) return false;

            for (uint32_t i = 0u; i < argument_values.size(); ++i)
                if (argument_values[i].first == __arg_id) return true;
            return false;
        }


        void addArgument(const std::string &_arg_name, const std::string &_arg_alias, const std::vector<uint32_t> &_arg_counts) {
            ArgDescription __new_arg;
            __new_arg.setDescription(_arg_name, _arg_alias, _arg_counts);

            for (uint32_t i = 0u; i < arguments.size(); ++i) {
                if (arguments[i].arg_name  == _arg_name)
                    throw std::invalid_argument(SVKFW_WRAPERR("ArgParser :: addArgument", "Argument name '"  + _arg_name  + "' is already used"));
                if (arguments[i].arg_alias == _arg_alias)
                    throw std::invalid_argument(SVKFW_WRAPERR("ArgParser :: addArgument", "Argument alias '" + _arg_alias + "' is already used"));
            }
            arguments.push_back(__new_arg);
        }

        void addArgument(const std::string &_arg_name, const std::vector<uint32_t> &_arg_counts) {
            addArgument(_arg_name, "", _arg_counts);
        }

        void parseAllArguments() {
            std::vector<std::pair<int,int>> __arg_indices;
            for (int i = 1; i < arg_handler.argc; ++i) {
                if (arg_handler.argv[i][0] == '-') {
                    bool __check_name  = arg_handler.argv[i][1] == '-' && isalpha(arg_handler.argv[i][2]);
                    bool __check_alias = isalpha(arg_handler.argv[i][1]);

                    if (__check_name || __check_alias) {
                        for (uint32_t j = 0u; j < arguments.size(); ++j) {
                            if (__check_name ? arguments[j].checkArgName (arg_handler.argv[i])
                                             : arguments[j].checkArgAlias(arg_handler.argv[i]))
                                __arg_indices.push_back({i,j});
                        }
                    }
                }
            }
            __arg_indices.push_back({arg_handler.argc, -1});

            for (uint32_t i = 0u; i < __arg_indices.size()-1; ++i) {
                argument_values.push_back({ __arg_indices[i].second,
                                            ArgNamespace::ParseArgument(arguments[__arg_indices[i].second],
                                                      __arg_indices[i+1].first - (__arg_indices[i].first+1),
                                                     (__arg_indices[i  ].first+1) < arg_handler.argc ? arg_handler.argv + (__arg_indices[i].first+1) : nullptr) });
            }
        }

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
                __res = argumentCaster<T>(argument_values[__arg_first_parse[0]].second[_arg_value_id][0],
                                          argument_values[__arg_first_parse[0]].second[_arg_value_id][1]);
            }
            return __res;
        }

        template <typename T>
        Vec3Base<T> castArgument3(const std::string &_arg_name, uint32_t _arg_value_id) const {
            Vec3Base<T> __res;

            uint32_t __arg_descr = getArgDescriptionIdByName(_arg_name);
            if (__arg_descr != UINT32_MAX && arguments[__arg_descr].arg_counts.size() > _arg_value_id) {
                SVKFW_ASSERT(arguments[__arg_descr].arg_counts[_arg_value_id] == 2u, std::runtime_error, "Simple::ArgParser :: castArgument(vector3)",
                                "vector3/argument value mismatch");
                std::vector<uint32_t> __arg_first_parse = getArgParseIdsByName(_arg_name);

                SVKFW_ASSERT(!__arg_first_parse.empty(), std::runtime_error, "Simple::ArgParser :: castArgument(vector3)",
                                "argument " + arguments[__arg_descr].getArgNamePrint() + "wasn't passed to the program");
                __res = argumentCaster<T>(argument_values[__arg_first_parse[0]].second[_arg_value_id][0],
                                          argument_values[__arg_first_parse[0]].second[_arg_value_id][1],
                                          argument_values[__arg_first_parse[0]].second[_arg_value_id][2]);
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
                __res = argumentCaster<T>(argument_values[__arg_first_parse[0]].second[_arg_value_id][0],
                                          argument_values[__arg_first_parse[0]].second[_arg_value_id][1],
                                          argument_values[__arg_first_parse[0]].second[_arg_value_id][2],
                                          argument_values[__arg_first_parse[0]].second[_arg_value_id][3]);
            }
            return __res;
        }
    }; // ArgParser END
}; // Simple END


#endif