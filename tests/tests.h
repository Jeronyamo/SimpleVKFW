#ifndef SVKFW_TESTS_H
#define SVKFW_TESTS_H

#include <vector>
#include <string>

#include "common/bits.h"
#include "common/terminal.h"
#include "common/argparser.h"
#include "interface/term_handler.h"


// Note: The idea is shamelessly taken from IvanTea's testing system, but worse. Added subtest level.

namespace Simple {
    namespace Tests {
    // Utility enums

        typedef enum SubtestId : uint32_t {
            SUBTEST_NONE   = 0,
            SUBTEST1       = 1u << 0,
            SUBTEST2       = 1u << 1,
            SUBTEST3       = 1u << 2,
            SUBTEST4       = 1u << 3,
            SUBTEST5       = 1u << 4,
            SUBTEST6       = 1u << 5,
            SUBTEST7       = 1u << 6,
            SUBTEST8       = 1u << 7,
            SUBTEST9       = 1u << 8,
            SUBTEST10      = 1u << 9,
            SUBTEST11      = 1u << 10,
            SUBTEST12      = 1u << 11,
            SUBTEST13      = 1u << 12,
            SUBTEST14      = 1u << 13,
            SUBTEST15      = 1u << 14,
            SUBTEST16      = 1u << 15,
            SUBTEST17      = 1u << 16,
            SUBTEST18      = 1u << 17,
            SUBTEST19      = 1u << 18,
            SUBTEST20      = 1u << 19,
            SUBTEST21      = 1u << 20,
            SUBTEST22      = 1u << 21,
            SUBTEST23      = 1u << 22,
            SUBTEST24      = 1u << 23,
            SUBTEST25      = 1u << 24,
            SUBTEST26      = 1u << 25,
            SUBTEST27      = 1u << 26,
            SUBTEST28      = 1u << 27,
            SUBTEST29      = 1u << 28,
            SUBTEST30      = 1u << 29,
            SUBTEST31      = 1u << 30,
            SUBTEST32      = 1u << 31,
            SUBTEST_ALL    = UINT32_MAX
        } SubtestId; // SubtestId END

        enum CompOp {
            TEST_COMP_NONE,
            TEST_COMP_E,
            TEST_COMP_L,
            TEST_COMP_G,
            TEST_COMP_NE,
            TEST_COMP_LE,
            TEST_COMP_GE,
        }; // CompOp END


// Classes

        struct AssertInfoSct {
            std::string assert_descr; // comp  - assertion description
            std::string    val_descr; // val   - compared value description (optional)
            std::string  thold_descr; // thold - threshold description (optional)
            std::string    res_descr; // res   - (possibly generated) string which contains printable assertion result (compared values and op)
            CompOp assert_op = TEST_COMP_NONE;
            bool assert_result = true;
        }; // AssertInfoSct END

        template <typename T>
        std::pair<bool, std::string> test_assert_core(T _val, CompOp _op, T _thold) {
            if(_op != TEST_COMP_E  && _op != TEST_COMP_NE &&
               _op != TEST_COMP_GE && _op != TEST_COMP_LE &&
               _op != TEST_COMP_G  && _op != TEST_COMP_L)
                throw std::runtime_error(SVKFW_WRAPERR("Tests :: test_assert_core", "Unknown comparison op"));

            bool __res_comp = false;
            std::string __res_descr;

            switch (_op) {
                case TEST_COMP_E :
                    __res_descr = " == ";
                    __res_comp = _val == _thold; break;
                case TEST_COMP_NE:
                    __res_descr = " != ";
                    __res_comp = _val != _thold; break;
                case TEST_COMP_GE:
                    __res_descr = " <= ";
                    __res_comp = _val >= _thold; break;
                case TEST_COMP_LE:
                    __res_descr = " >= ";
                    __res_comp = _val >= _thold; break;
                case TEST_COMP_G :
                    __res_descr = " >  ";
                    __res_comp = _val >  _thold; break;
                case TEST_COMP_L :
                    __res_descr = " <  ";
                    __res_comp = _val <  _thold; break;
            }

            return { __res_comp, std::to_string(_val) + __res_descr + std::to_string(_thold) };
        }


        struct TestItf {
            struct SubtestSct {
                SubtestSct(TestItf *_test_p, const std::string &_subtest_descr) { _test_p->subtests.push_back({this, _subtest_descr}); }
                virtual ~SubtestSct() {}

                virtual void subtest() { printf("SubtestBase\n"); }
            }; // SubtestSct END

            std::vector<std::pair<SubtestSct*, std::string>> subtests;


            TestItf();
            virtual ~TestItf() {}


            // By default, runs all subtests of a test.
            // Note: max number of subtests is 32
            void run(uint32_t _subtest_flags = SUBTEST_ALL);

            virtual std::string        name() { return        "NameBase"; }
            virtual std::string description() { return "DescriptionBase"; }

            void prepareSubtestGlobalInfo(uint32_t _subtest_id);
            void submitSubtestGlobalInfo(bool _subtest_crashed);
        }; // TestItf END


        namespace TestSystem {
            std::vector<TestItf*> tests;
            uint32_t test_id = 0u,      subtest_id = 0u,      subtest_crash = 0u, // Starts with 1; 0 - uninitialized
                     test_count = 0u,   subtest_fail = 0u,    assert_error  = 0u,
                     test_success = 0u, subtest_success = 0u, assert_success = 0u;
            std::vector<AssertInfoSct> subtest_asserts;

            TermCtrl::TerminalHandler terminal_h{ TermCtrl::TextStyle{},
                                                  TermCtrl::TextStyle{{TermCtrl::TFlag::FG_RGB8 , TermCtrl::TFlag::BOLD}, TermCtrl::TColor::BrightGreen},
                                                  TermCtrl::TextStyle{{TermCtrl::TFlag::FG_RGB8 , TermCtrl::TFlag::BOLD}, TermCtrl::TColor::BrightRed},
                                                  TermCtrl::TextStyle{{TermCtrl::TFlag::FG_RGB8 , TermCtrl::TFlag::BOLD}, TermCtrl::TColor::Red},
                                                  TermCtrl::TextStyle{{TermCtrl::TFlag::FG_RGB8}, TermCtrl::TColor::BrightCyan},
                                                  TermCtrl::TextStyle{{TermCtrl::TFlag::   BOLD}}, };

            enum TStyleID { TSTYLE_DEFAULT, TSTYLE_SUCCESS, TSTYLE_ERROR, TSTYLE_CRASH, TSTYLE_INFO, TSTYLE_BOLD };

            void print_header(const std::vector<uint32_t> &_test_indices, const std::vector<uint32_t> &_subtest_flags) {
                printf("\n");
                terminal_h.print_wcent(TSTYLE_BOLD, "SimpleVKFW Test System", '=');
                terminal_h.print_w();

                terminal_h.print_wcent("", '-');
                terminal_h.print_wcent({terminal_h.encloseWithStyle(TSTYLE_INFO, "Running tests"), TermCtrl::StylizedString{": " + std::to_string(_test_indices.size())}});
                terminal_h.print_wcent("", '-');
                terminal_h.print_w();

                uint32_t __test_index = 0u;
                for (uint32_t i = 0u; i < _test_indices.size(); ++i) {
                    terminal_h.print_w({{"   " + std::to_string(_test_indices[i]) + ". "}, terminal_h.encloseWithStyle(TSTYLE_INFO, tests[_test_indices[i]-1]->name())});
                    for (uint32_t j = 0u; j < tests[_test_indices[i]-1]->subtests.size(); ++j) {
                        if (_subtest_flags[i] & (1 << j))
                            terminal_h.print_w("      " + std::to_string(_test_indices[i]) + '.' + std::to_string(j+1) + ": " + tests[_test_indices[i]-1]->subtests[j].second);
                    }
                    terminal_h.print_w();
                }
                terminal_h.print_w();
                terminal_h.print_wcent("", '=');
            }

            void print_test_header() {
                terminal_h.print_w();
                terminal_h.print_w({terminal_h.encloseWithStyle(TSTYLE_INFO, "   Test " + std::to_string(TestSystem::test_id)), {": " + tests[TestSystem::test_id-1]->name()}});

                std::vector<std::string> __split_str;
                splitString(tests[TestSystem::test_id-1]->description(), terminal_h.line.width - 3, __split_str);
                for (const std::string &_descr_str : __split_str)
                    terminal_h.print_w("   " + _descr_str);
                terminal_h.print_w();
                terminal_h.print_wcent("", '-');
            }

            void print_test_result() {
                terminal_h.print_w();
                terminal_h.print_wcent("", '-');
                terminal_h.print_w();
                terminal_h.print_w({{"      Subtests "}, {terminal_h.encloseWithStyle(TestSystem::subtest_success ? TSTYLE_SUCCESS : TSTYLE_DEFAULT, "Passed ")}, {": " + std::to_string(TestSystem::subtest_success)}});
                terminal_h.print_w({{"      Subtests "}, {terminal_h.encloseWithStyle(TestSystem::subtest_fail    ? TSTYLE_ERROR   : TSTYLE_DEFAULT, "Failed ")}, {": " + std::to_string(TestSystem::subtest_fail   )}});
                terminal_h.print_w({{"      Subtests "}, {terminal_h.encloseWithStyle(TestSystem::subtest_crash   ? TSTYLE_CRASH   : TSTYLE_DEFAULT, "Crashed")}, {": " + std::to_string(TestSystem::subtest_crash  )}});
                terminal_h.print_w();
                terminal_h.print_wcent("", '-');
            }

            void print_subtest_header() {
                terminal_h.print_w();
                terminal_h.print_w({terminal_h.encloseWithStyle(TSTYLE_INFO, "      Subtest " + std::to_string(TestSystem::subtest_id+1)), {": " + tests[TestSystem::test_id-1]->subtests[TestSystem::subtest_id].second}});
            }

            void print_subtest_result(bool _subtest_crashed) {
                if (!_subtest_crashed) {
                    if (TestSystem::assert_success > 0)
                        terminal_h.print_w({terminal_h.encloseWithStyle(TSTYLE_SUCCESS, "         Passed"), {": " + std::to_string(TestSystem::assert_success)}});
                    if (TestSystem::assert_error   > 0)
                        terminal_h.print_w({terminal_h.encloseWithStyle(TSTYLE_ERROR,   "         Failed"), {": " + std::to_string(TestSystem::assert_error  )}});
                }
                TestSystem::subtest_asserts.clear();
            }

            void print_end() {
                terminal_h.print_wcent(TSTYLE_BOLD, "SimpleVKFW Test System End", '=');
                printf("\n");
            }


        // Run tests

            void prepareTestGlobalInfo(uint32_t _test_id);
            void submitTestGlobalInfo();

            void runTest(uint32_t _test_id, uint32_t _subtest_flags) {
                prepareTestGlobalInfo(_test_id);
                tests[_test_id - 1]->run(_subtest_flags);
                submitTestGlobalInfo();
            }

            void run(std::vector<uint32_t> _test_indices = {}, std::vector<uint32_t> _subtest_flags = {}) {
                if (_test_indices.empty()) {
                    _test_indices.resize(tests.size());
                    for (uint32_t i = 0u; i < tests.size(); ++i)
                        _test_indices[i] = i;
                }
                _subtest_flags.resize(_test_indices.size(), SUBTEST_ALL);

                test_count = (uint32_t) _test_indices.size();
                print_header(_test_indices, _subtest_flags);
                for (uint32_t i = 0u; i < _test_indices.size(); ++i) {
                    runTest(_test_indices[i], _subtest_flags[i]);
                }
                print_end();
            }
        }; // TestSystem END


    // Test methods definition

        TestItf::TestItf() {
            TestSystem::tests.push_back(this);
        }

        void TestItf::run(uint32_t _subtest_flags) {
            for (uint32_t i = 0u; i < subtests.size(); ++i) {
                bool __subtest_crashed = false;

                if (_subtest_flags & Bits::Mask32::Bit(i)) {
                    TestSystem::subtest_id = i;

                    prepareSubtestGlobalInfo(i);
                    try {
                        subtests[i].first->subtest();
                    }
                    catch (std::exception) {
                        __subtest_crashed = true;
                    }
                }
                submitSubtestGlobalInfo(__subtest_crashed);
            }
        }

        void TestItf::prepareSubtestGlobalInfo(uint32_t _subtest_id) {
            TestSystem::subtest_id = _subtest_id;

            TestSystem::print_subtest_header();
        }

        void TestItf::submitSubtestGlobalInfo(bool _subtest_crashed) {
            TestSystem::subtest_crash += _subtest_crashed;

            if (!_subtest_crashed) {
                if (TestSystem::assert_error > 0)
                    TestSystem::subtest_fail += 1;
                else
                    TestSystem::subtest_success += 1;
            }

            TestSystem::print_subtest_result(_subtest_crashed);
            TestSystem::subtest_asserts.clear();
            TestSystem::assert_error   = 0u;
            TestSystem::assert_success = 0u;
        }


    // TestSystem functions definition

        void TestSystem::prepareTestGlobalInfo(uint32_t _test_id) {
            TestSystem::test_id = _test_id;

            TestSystem::print_test_header();
            TestSystem::subtest_fail    = 0u;
            TestSystem::subtest_crash   = 0u;
            TestSystem::subtest_success = 0u;
        }

        void TestSystem::submitTestGlobalInfo() {
            TestSystem::print_test_result();
        }


    // Assert functions definition

        template <typename T>
        void testAssert(T _val, CompOp _op, T _thold,
                        const std::string &_assert_descr,
                        const std::string &_val_descr = "",
                        const std::string &_thold_descr = "") {
            std::pair<bool, std::string> __assert_res = test_assert_core(_val, _op, _thold);

            AssertInfoSct __res{};
            __res.assert_descr  = _assert_descr;
            __res.val_descr     =    _val_descr;
            __res.thold_descr   =  _thold_descr;
            __res.res_descr     = __assert_res.second;
            __res.assert_op     =  _op;
            __res.assert_result = __assert_res.first;

            if (__assert_res.first)
                TestSystem::assert_success += 1;
            else
                TestSystem::assert_error   += 1;
            TestSystem::subtest_asserts.push_back(__res);
        }

        void testAssert(bool _assert_res, const std::string &_assert_descr) {
            AssertInfoSct __res{};
            __res.assert_descr  = _assert_descr;
            __res.assert_result = _assert_res;

            if (_assert_res)
                TestSystem::assert_success += 1;
            else
                TestSystem::assert_error   += 1;
            TestSystem::subtest_asserts.push_back(__res);
        }
    }; // Tests END
}; // Simple END


// Test struct definition
#define SVKFW_TEST_BEG(TEST_NAME, TEST_DESCR_STR)\
struct TEST_NAME : Simple::Tests::TestItf {\
    std::string name() override { return #TEST_NAME; }\
    std::string description() override { return TEST_DESCR_STR; }\
    TEST_NAME() : Simple::Tests::TestItf{} {}

#define SVKFW_TEST_END(TEST_NAME)  } test_##TEST_NAME

// Subtest method definition
#define SVKFW_SUBTEST_BEG(SUBTEST_NAME, SUBTEST_DESCR_STR)\
struct SUBTEST_NAME : Simple::Tests::TestItf::SubtestSct {\
    SUBTEST_NAME(Simple::Tests::TestItf *_test_ptr) : Simple::Tests::TestItf::SubtestSct{_test_ptr, SUBTEST_DESCR_STR} {}\
    void subtest() override

#define SVKFW_SUBTEST_END(SUBTEST_NAME) } st_##SUBTEST_NAME{this}

#endif
