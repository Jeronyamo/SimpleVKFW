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
        // // Example: A -> aAc; A -> bBd; A,B - nonterminals, a,b,c,d - (possibly empty) terminal strings
        // // Only right side of a rule, they are gathered in RuleBlocks
        // struct ProdRule {
        //     char* rule_str = nullptr;
        //     uint32_t nonterm_len_id = -1u; // 2bytes - len | 2bytes - id -> max rule length is 2^16 - 1
        //     Nonterminal nonterm     = -1u;

        //     ProdRule(const std::string &_left_term, Nonterminal _nonterm = -1u, const std::string &_right_term = "") {
        //         const uint32_t __rule_len = _left_term.size() + _right_term.size();

        //         if (__rule_len >= uint16_t(-1))
        //             throw std::invalid_argument(SVKFW_WRAPERR("ParserReg :: ProdRule", "rule string is longer than 65535 symbols"));

        //         rule_str = new char[__rule_len + 1];
        //         nonterm_len_id = (uint16_t(__rule_len) << 16) + uint16_t(_left_term.size());

        //         if (! _left_term.empty())
        //             memcpy(rule_str,  _left_term.data(),  _left_term.size());
        //         if (!_right_term.empty())
        //             memcpy(rule_str + _left_term.size(), _right_term.data(), _right_term.size());
        //         rule_str[__rule_len] = '\0';
        //     }
        //     ProdRule(const ProdRule &_prod_rule) : nonterm_len_id{_prod_rule.nonterm_len_id}, nonterm{_prod_rule.nonterm} {
        //         rule_str = new char[nonterm_len_id + 1];
        //         memcpy(rule_str, _prod_rule.rule_str, nonterm_len_id + 1);
        //     }
        //     ~ProdRule() {
        //         if (rule_str) {
        //             delete[]  rule_str;
        //             rule_str = nullptr;
        //         }
        //     }

        //     void  offsetNonterm(int _nt_offset)     { nonterm += _nt_offset; }
        //     void replaceNonterminal(Nonterminal _nonterm) { nonterm  = _nonterm; }
        //     uint32_t getRuleLen  () const { return nonterm_len_id >>     16; }
        //     uint32_t getNontermId() const { return nonterm_len_id && 0x00ff; }

        //     // Note: when multiple nonterminals will be allowed again, pass last parsed nonterminal.
        //     //
        //     // Algorithm examples:
        //     //
        //     // 'A -> aBCd' (B,C - nonterminals):
        //     //     checkNext(0, NONTERM_START)      -> NEXT_TERMINAL,
        //     //     parse('a', 0)                    -> true,
        //     //
        //     //     checkNext(1, NONTERM_START)      -> NEXT_NONTERMINAL,
        //     //     getNontermNext(1, NONTERM_START) -> B,
        //     //     <parse B>                        -> true,
        //     //
        //     //     checkNext(1, B)                  -> NEXT_NONTERMINAL,
        //     //     getNontermNext(1, B)             -> C,
        //     //     <parse C>                        -> true,
        //     //
        //     //     checkNext(1, C)                  -> NEXT_TERMINAL,
        //     //     parse('d', 1)                    -> true,
        //     //
        //     //     checkNext(2, NONTERM_START)      -> NEXT_END,
        //     //
        //     //
        //     // 'A -> BC' (B,C - nonterminals):
        //     //     checkNext(0, NONTERM_START)      -> NEXT_NONTERMINAL,
        //     //     getNontermNext(0, NONTERM_START) -> B,
        //     //     <parse B>                        -> true,
        //     //
        //     //     checkNext(0, B)                  -> NEXT_NONTERMINAL,
        //     //     getNontermNext(0, B)             -> C,
        //     //     <parse C>                        -> true,
        //     //
        //     //     checkNext(0, C)                  -> NEXT_END,
        //     //
        //     //
        //     // 'A ->' (empty rule):
        //     //     checkNext(0, NONTERM_START)      -> NEXT_END,
        //     //
        //     NextSymbolType checkNext(uint32_t _ind, Nonterminal _last_nonterm) const {
        //         NextSymbolType __res = NEXT_TERMINAL;
        //         if (_ind > getRuleLen())
        //             __res = NEXT_ERR;

        //         if (_ind == getNontermId()) {
        //             __res = NEXT_ERR;
        //             if (_last_nonterm == NONTERM_START)
        //                 __res = NEXT_NONTERMINAL;
        //             if (_last_nonterm == nonterm)
        //                 __res = NEXT_TERMINAL;
        //         }

        //         if (__res == NEXT_TERMINAL && _ind == getRuleLen())
        //             __res = NEXT_END;

        //         return __res;
        //     }

        //     bool parse(char _ch, uint32_t _ind) const {
        //         return _ind < getRuleLen() && rule_str[_ind] == _ch;
        //     }

        //     Nonterminal getNontermNext(uint32_t _ind, Nonterminal _last_nonterm) const {
        //         if (_ind == getNontermId() && _last_nonterm == NONTERM_START)
        //             return nonterm;
        //         return NONTERM_ERR;
        //     }
        //     // Nonterminal's ID takes string index into account: in a rule 'A -> aB' (B - nonterminal) B has ID 1 only if 'ind' == 1
        //     uint32_t getNontermID(uint32_t _ind, Nonterminal _curr_nonterm) const {
        //         uint32_t __res = uint16_t(-1);
        //         if (_curr_nonterm == NONTERM_START)
        //             __res = 0;
        //         if (_ind == getNontermId() && _curr_nonterm == nonterm)
        //             __res = 1;
        //         return __res;
        //     }
        //     Nonterminal getNontermByID(uint32_t _ind, uint32_t _nonterm_id) const {
        //         Nonterminal __res = NONTERM_ERR;
        //         if (_nonterm_id == 0)
        //             __res = NONTERM_START;
        //         if (_ind == getNontermId() && _nonterm_id == 1)
        //             __res = nonterm;
        //         return __res;
        //     }
        // }; // ProdRule END

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


    namespace ParserCFG {
        // struct ProdRule {
        //     char *rule_str = nullptr;
        //     std::vector<std::pair<uint32_t, Nonterminal>> nonterms; // first element is always {rule_str_len, -1}
        // }; // ProdRule END

        // Example: A -> aAbBcCd, A,B,C - nonterminals, a,b,c,d - (possibly empty) terminal strings
        // struct ProdRule {
        //     char* rule_str = nullptr;
        //     uint32_t* nonterm_ids = nullptr;
        //     Nonterminal* nonterms = nullptr;
        //     Nonterminal left_nonterminal = PARSE_ERROR; // context-free
        //     uint32_t nonterm_len = 0u; // nonterm_ids[nonterm_len] gives length of the string

        //     ProdRule(Nonterminal _left_side, const std::vector<TermOrNonterm> &_right_side) : left_nonterminal{_left_side} {
        //         std::string __rule_str;
        //         std::vector<uint32_t> __nonterm_ids;
        //         std::vector<Nonterminal> __nonterms;

        //         for (const auto &right : _right_side) {
        //             if (right.isNonterm()) {
        //                 __nonterm_ids.emplace_back(__rule_str.size());
        //                 __nonterms.emplace_back(right.nonterm);
        //             }
        //             __rule_str += right.term;
        //         }

        //         rule_str = new char[__rule_str.size() + 1]{};
        //         nonterm_ids = new uint32_t[__nonterm_ids.size() + 1];
        //         nonterms = new Nonterminal[__nonterms.size()];

        //         memcpy(rule_str, __rule_str.data(), __rule_str.size());
        //         memcpy(nonterm_ids, __nonterm_ids.data(), __nonterm_ids.size());
        //         memcpy(nonterms, __nonterms.data(), __nonterms.size());

        //         nonterm_len = __nonterm_ids.size();
        //         nonterm_ids[nonterm_len] = (uint32_t) __rule_str.size();
        //     }
        //     ProdRule(const ProdRule &_prod_rule) {
        //         rule_str = new char[_prod_rule.nonterm_ids[nonterm_len] + 1];
        //         nonterm_ids = new uint32_t[_prod_rule.nonterm_len + 1];
        //         nonterms = new Nonterminal[_prod_rule.nonterm_len];

        //         memcpy(rule_str, _prod_rule.rule_str, _prod_rule.nonterm_ids[nonterm_len] + 1);
        //         memcpy(nonterm_ids, _prod_rule.nonterm_ids, _prod_rule.nonterm_len + 1);
        //         memcpy(nonterms, _prod_rule.nonterms, _prod_rule.nonterm_len);
        //     }
        //     ~ProdRule() {
        //         delete[]  rule_str;
        //         rule_str = nullptr;
        //         delete[]  nonterm_ids;
        //         nonterm_ids = nullptr;
        //         delete[]  nonterms;
        //         nonterms = nullptr;
        //     }

        //     void nontermOffset(uint32_t _offset) {
        //         left_nonterminal += _offset;
        //         for (uint32_t i = 0u; i < nonterm_len; ++i)
        //             nonterms[i] += _offset;
        //     }

        //     NextSymbolType checkNext(uint32_t _ind) const {
        //         if (_ind == nonterm_ids[nonterm_len])
        //             return NEXT_END;
        //         if (_ind >  nonterm_ids[nonterm_len])
        //             return NEXT_ERR;

        //         for (uint32_t __nonterm_id = 0u; __nonterm_id < nonterm_len && nonterm_ids[__nonterm_id] <= _ind; ++__nonterm_id)
        //             if (nonterm_ids[__nonterm_id] == _ind)
        //                 return NEXT_NONTERMINAL;

        //         return NEXT_TERMINAL;
        //     }

        //     Nonterminal parse(char _ch, uint32_t _ind) const {
        //         if (_ind == nonterm_ids[nonterm_len])
        //             return PARSE_END;
        //         if (_ind >  nonterm_ids[nonterm_len])
        //             return PARSE_ERROR;

        //         for (uint32_t __nonterm_id = 0u; __nonterm_id < nonterm_len && nonterm_ids[__nonterm_id] <= _ind; ++__nonterm_id)
        //             if (nonterm_ids[__nonterm_id] == _ind)
        //                 return nonterms[__nonterm_id];

        //         if (rule_str[_ind] == _ch)
        //             return PARSE_SUCCESS;

        //         return PARSE_FAILURE;
        //     }
        // }; // ProdRule END


        // struct ParseTreeNode {
        //     Nonterminal nonterm;
        //     uint32_t parent_node;
        //     uint32_t rule_nonterm_id; // rule's right part may have multiple nonterminals (A -> aBbCc; B -> ...; C -> ...)
        //     uint32_t rule_id;
        //     uint32_t str_offset;
        //     uint32_t str_len;
        // }; // ParseTreeNode END

        // struct ParseTree {
        //     std::string parsed_str;
        //     std::vector<ParseTreeNode> nodes;

        //     void deleteNodeChildren(uint32_t _node_id) {
        //         Nonterminal __nonterm_to_delete = nodes[_node_id].nonterm;

        //         for (uint32_t i = 0u; i < nodes.size(); ++i) {
        //             Nonterminal __curr_nonterm = nodes[i].nonterm;
        //             uint32_t __parent_node = nodes[i].parent_node;

        //             while (__curr_nonterm != 0) {
        //                 if (__curr_nonterm == __nonterm_to_delete ||
        //                     __curr_nonterm == -1) {
        //                     nodes[_node_id].nonterm = -1;
        //                     break;
        //                 }

        //                 __curr_nonterm = nodes[__parent_node].nonterm;
        //                 __parent_node  = nodes[__parent_node].parent_node;
        //             }
        //         }
        //     }
        // }; // ParseTree END



    // 'LATE Ain’T Earley: A Faster Parallel Earley Parser'
        // struct ParserLATE {
        // Data

        //     struct EarleyItem;
        //     std::map<Nonterminal, std::vector<ProdRule>> prod_rules;
        //     std::set<EarleyItem>    chart_set;
        //     std::set<EarleyItem> complete_set;
        //     std::map<std::pair<Nonterminal, uint32_t>, std::set<EarleyItem>> requests_map; // !!!!!:  // TODO: put these pairs to vector (or other container), use key: index in this container. Contain previous indices, this should be enough. It can have multiple previous indices -> no point doing it backwards
        //     std::map<std::pair<Nonterminal, uint32_t>, std::set<  uint32_t>>  replies_map;
        //     std::mutex complete_mutex;
        //     std::mutex requests_mutex;

        //     struct EarleyItem {
        //         const ProdRule *rule;
        //         uint32_t i, j, k; // Note: i and k are indices for parsed string, i<=k. j means index of rule's string, which does not equal to k-i, because nonterminals take one symbol in my representation

        //         EarleyItem(const ProdRule *_rule, uint32_t _i, uint32_t _j, uint32_t _k)
        //                         : rule{_rule}, i{_i}, j{_j}, k{_k} {}
        //         EarleyItem(const EarleyItem &_val) : rule{_val.rule}, i{_val.i}, j{_val.j}, k{_val.k} {}

        //         bool operator==(const EarleyItem &_cmp) const {
        //             return rule == _cmp.rule && i == _cmp.i && j == _cmp.j && k == _cmp.k;
        //         }

        //         bool operator<(const EarleyItem &_cmp) const {
        //             bool __res = false;
        //             if      (i <  _cmp.i) __res = true;
        //             else if (i == _cmp.i && k <  _cmp.k) __res = true;
        //             else if (i == _cmp.i && k == _cmp.k && rule <  _cmp.rule) __res = true;
        //             else if (i == _cmp.i && k == _cmp.k && rule == _cmp.rule && j < _cmp.j) __res = true;
        //             return __res;
        //         }

        //         NextSymbolType getState() const { return rule->checkNext(j); }
        //     }; // EarleyItem END

        // // Constructor & methods
        //     // Note: expects at least one rule with 'NONTERM_START' (== 0) on the left side
        //     ParserLATE(const std::vector<ProdRule> &_prod_rules) {
        //         for (const auto &prod_rule : _prod_rules) {
        //             if (prod_rules.find(prod_rule.left_nonterminal) == prod_rules.end())
        //                 prod_rules[prod_rule.left_nonterminal] = {};
        //             prod_rules[prod_rule.left_nonterminal].push_back(prod_rule);
        //         }
        //     }
        //     // ParserLATE(const std::map<Nonterminal, std::vector<ProdRule>> &_prod_rules = {})
        //     //                 : prod_rules{_prod_rules} {}


        //     // If there's <=32 starting rules, 'start_success_ids' has corresponding bits set to 1
        //     // (from least significant to most). Overall, it sets bits for the first 32 rules, no matter how many there are.
        //     bool parse(const std::string &_str_to_parse, uint32_t *_start_success_ids = nullptr, ParseTree *_res_tree = nullptr) {
        //         chart_set.clear();
        //         complete_set.clear();

        //         for (const ProdRule &__rule : prod_rules[NONTERM_START])
        //             chart_set.emplace(&__rule, 0u, 0u, 0u); // add all begin states

        //         while (!chart_set.empty()) {
        //             EarleyItem __curr_state = *(--chart_set.end());
        //             chart_set.erase(--chart_set.end());

        //             switch (__curr_state.getState()) {
        //                 case NEXT_END: {
        //                     completer(_str_to_parse, __curr_state);
        //                     break;
        //                 }
        //                 case NEXT_NONTERMINAL: {
        //                     predictor(_str_to_parse, __curr_state);
        //                     break;
        //                 }
        //                 case NEXT_TERMINAL: {
        //                     scanner(_str_to_parse, __curr_state);
        //                     break;
        //                 }
        //                 case NEXT_ERR: {
        //                     // This is expected (only when grammar is ambiguous?)
        //                     // throw std::invalid_argument(SVKFW_WRAPERR("ParserLATE :: Parser :: parse", "parse error"));
        //                     break;
        //                 }
        //                 default: {
        //                     throw std::runtime_error(SVKFW_WRAPERR("ParserLATE :: Parser :: parse", "how did you get here?"));
        //                     break;
        //                 }
        //             }
        //         }

        //         bool __res = false;
        //         uint32_t __count = 0u;
        //         for (const ProdRule &__rule : prod_rules[NONTERM_START]) {
        //             if (complete_set.find({&__rule, 0u, (uint32_t)__rule.nonterm_ids[__rule.nonterm_len], (uint32_t)_str_to_parse.size()}) != complete_set.end()) { // j must be -1 (or maybe use rule_str length here)
        //                 __res = true;
        //                 if (_start_success_ids != nullptr)
        //                     *_start_success_ids |= (1 << __count);
        //             }
        //             ++__count;
        //         }

        //         if (__res && _res_tree != nullptr)
        //             *_res_tree = collectAllNonterminals(_str_to_parse);

        //         return __res;
        //     }

        //     bool   scanner(const std::string &_str, EarleyItem _state) {
        //         bool __res = _state.rule->parse(_str[_state.k], _state.j) == PARSE_SUCCESS;
        //         if (__res)
        //             chart_set.emplace(_state.rule, _state.i, _state.j+1, _state.k+1);
        //         return __res;
        //     }

        //     bool predictor(const std::string &_str, EarleyItem _state) {
        //         Nonterminal __next_nonterm = _state.rule->parse(_str[_state.k], _state.j);
        //         bool __new_nonterm_on_pos = false;
        //         {
        //             std::lock_guard<std::mutex> lk(requests_mutex);
        //             __new_nonterm_on_pos = requests_map.find({__next_nonterm, _state.k}) == requests_map.end();
        //             if(__new_nonterm_on_pos)
        //                 requests_map[{__next_nonterm, _state.k}] = {};

        //             requests_map[{__next_nonterm, _state.k}].emplace(_state);
        //         }

        //         if (__new_nonterm_on_pos) {
        //             for (const ProdRule &__rule : prod_rules[__next_nonterm]) {
        //                 EarleyItem __new_state{&__rule, _state.k, 0u, _state.k};
        //                 if (!(__new_state.getState() == NEXT_TERMINAL && __rule.parse(_str[_state.k], 0) != PARSE_FAILURE))
        //                     chart_set.emplace(__new_state);
        //             }
        //         }

        //         if (replies_map.find({__next_nonterm, _state.k}) != replies_map.end()) {
        //             for (uint32_t k : replies_map[{__next_nonterm, _state.k}]) {
        //                 chart_set.emplace(_state.rule, _state.i, _state.j+1, k);
        //             }
        //         }
        //         return __new_nonterm_on_pos;
        //     }

        //     bool completer(const std::string &_str, EarleyItem _state) {
        //         bool __add_first = false;
        //         {
        //             std::lock_guard<std::mutex> lk(complete_mutex);
        //             __add_first = complete_set.find(_state) == complete_set.end();
        //             if (__add_first)
        //                 complete_set.emplace(_state);
        //         }

        //         if (__add_first) {
        //             if (replies_map.find({_state.rule->left_nonterminal, _state.i}) == replies_map.end())
        //                 replies_map[{_state.rule->left_nonterminal, _state.i}] = {};
        //             replies_map[{_state.rule->left_nonterminal, _state.i}].emplace(_state.k);

        //             if (requests_map.find({_state.rule->left_nonterminal, _state.i}) != requests_map.end()) {
        //                 for (const EarleyItem &__st : requests_map[{_state.rule->left_nonterminal, _state.i}]) {
        //                     chart_set.emplace(__st.rule, __st.i, __st.j+1, _state.k);
        //                 }
        //             }
        //         }
        //         return __add_first;
        //     }

        //     ParseTree collectAllNonterminals(const std::string &_str_to_parse) {
        //         // TODO: walk over parse tree (that has successfully parsed branch) and find the way from beginning to end (made of parsed nonterminals)
        //         // TODO: test this function
        //         ParseTree __res;
        //         __res.nodes.push_back({ NONTERM_START, -1u, -1u, 0u, (uint32_t)_str_to_parse.size() });

        //         for (uint32_t i = 0u; i < prod_rules[NONTERM_START].size(); ++i) {
        //             if (complete_set.find({&prod_rules[NONTERM_START][i], 0u, (uint32_t)prod_rules[NONTERM_START][i].nonterm_ids[prod_rules[NONTERM_START][i].nonterm_len], (uint32_t)_str_to_parse.size()}) != complete_set.end()) { // j must be -1 (or maybe use rule_str length here)
        //                 __res.nodes[0].rule_id = i;
        //             }
        //         }

        //         if (__res.nodes[0].rule_id == -1)
        //             return __res;

        //         __res.parsed_str = _str_to_parse;
        //         std::vector<uint32_t> __node_stack = {0u};
        //         std::vector<ParseTreeNode> __temp_nodes;
        //         while(!__node_stack.empty()) { // adds children; if don't exist - wrong node/parent, check next rule
        //             uint32_t __node_id = __node_stack.back();
        //             __node_stack.erase(--__node_stack.end());

        //             auto& __node = __res.nodes[__node_id];
        //             const auto& __rules = prod_rules[__node.nonterm];

        //             uint32_t __rule_i = __node.rule_id == -1 ? 0u : __node.rule_id + 1;
        //             __node.rule_id = -1;
        //             for (; __rule_i < __rules.size(); ++__rule_i) {
        //                 __temp_nodes.clear();
        //                 bool _all_exist = true;
        //                 for (uint32_t i = 0u; i < __rules[__rule_i].nonterm_len; ++i) {
        //                     std::pair<Nonterminal, uint32_t> __map_key = { __rules[__rule_i].nonterms[i], __rules[__rule_i].nonterm_ids[i] - i + __node.str_offset };

        //                     if (replies_map.find(__map_key) == replies_map.end() || replies_map[__map_key].size() == 0u) {
        //                         _all_exist = false;
        //                         break;
        //                     }

        //                     if (replies_map[__map_key].size()) {
        //                         throw std::runtime_error(SVKFW_WRAPERR("ParserLATE :: Parser :: collectAllNonterminals", "TODO: HANDLE MULTIPLE REPLIES FOR ONE NONTERMINAL"));
        //                     }
        //                     __temp_nodes.emplace_back(__map_key.first, __node_id, i, __rule_i, __map_key.second, *replies_map[__map_key].begin());
        //                 }

        //                 if (_all_exist) {
        //                     __node.rule_id = __rule_i; // add this rule's id to current node
        //                     __res.nodes.insert(__res.nodes.end(), __temp_nodes.begin(), __temp_nodes.end()); // add nodes for all children
        //                     for (uint32_t __new_node_id = 0u; __new_node_id < __temp_nodes.size(); ++__new_node_id) // add children ids to stack
        //                         __node_stack.push_back(__node_stack.size());
        //                     break;
        //                 }
        //             }

        //             if (__node.rule_id == -1) {
        //                 __node.nonterm  = -1;
        //                 __node_stack.push_back(__node.parent_node);
        //             }
        //         }
        //     }
        // }; // ParserLATE END


        // // A number of rules with one nonterminal on the left side. Describes an element of parser, like a number, a word, etc.
        // // Note: When initialized (i.e. from a config), left_nonterminal is zero and right-side nonterminals enumerated from 0 to N.
        // //       Then, when these blocks are gathered, offset is added to all nonterminals. Different offsets give different instances of the same element.
        // struct RuleBlock {
        //     Nonterminal left_nonterminal;
        //     uint32_t right_nonterminals_count;
        //     std::vector<ProdRule> prod_rules;

        //     RuleBlock(Nonterminal _left_nonterm, const std::vector<std::vector<TermOrNonterm>> &_right_sides) {
        //         uint32_t __total_nonterm_count = 0u, __max_nonterm_id = 0u; // when initialized correctly, 'max_nonterm_id' <= 'total_nonterm_count'. Nonterminals equal to left-side (recursion) don't count.
        //         prod_rules.reserve(_right_sides.size());

        //         for (uint32_t i = 0u; i < _right_sides.size(); ++i) {
        //             prod_rules.emplace_back(_left_nonterm, _right_sides[i]);
        //         }
        //     }
        // }; // RuleBlock END


        // RuleBlock rule_uint{ 0, { {"0", 0u}, {"1", 0u}, {"2", 0u}, {"3", 0u}, {"4", 0u}, {"5", 0u}, {"6", 0u}, {"7", 0u}, {"8", 0u}, {"9", 0u},
        //                           {"0"},     {"1"},     {"2"},     {"3"},     {"4"},     {"5"},     {"6"},     {"7"},     {"8"},     {"9"}  }};
        // RuleBlock rule_int{ 0, { {"-", 1u}, {1u} }};
        // RuleBlock rule_float{ 0, { {1u, ".", 2u} }};
    }; // ParserCFG END
}; // Simple END


#endif