#ifndef SVKFW_TESTS_H
#define SVKFW_TESTS_H

#include <vector>
#include <string>

#include "common/bits.h"
#include "common/terminal.h"
#include "common/argparser.h"
#include "interface/term_handler.h"

#define SVKFW_TEST_GROUP "None"
#define SVKFW_ADD_TEST(SVKFW_TEST_NAME, SVKFW_TEST_DESCR) struct Test##SVKFW_TEST_NAME : Simple::Test::TestItf {\
                                                              Test##SVKFW_TEST_NAME() : Simple::Test::TestItf{} {}\
                                                              void run() override;\
                                                              std::string        name() override { return #SVKFW_TEST_NAME ; }\
                                                              std::string description() override { return #SVKFW_TEST_DESCR; }\
                                                              std::string       group() override { return  SVKFW_TEST_GROUP; }\
                                                          } test_##SVKFW_TEST_NAME;\
                                                          void Test##SVKFW_TEST_NAME::run()



namespace Simple {
    namespace Test {
    // Utility enums

        enum CompOp {
            TEST_COMP_NE,
            TEST_COMP_E,
            TEST_COMP_GE,
            TEST_COMP_L,
            TEST_COMP_LE,
            TEST_COMP_G,
            TEST_COMP_NONE
        }; // CompOp END


// Classes

        struct AssertInfo {
            std::string assert_descr; // assertion description
            std::string result_descr; // (possibly generated) string which contains printable assertion result (compared values and op)
            std::string  value_descr; // compared value description (optional)
            std::string  thold_descr; // threshold description (optional)
            CompOp assert_op;
            bool assert_success; // assertion expression must return true, which can be interpreted as a success
        }; // AssertInfo END

        template <typename T>
        std::pair<bool, std::string> test_assert_core(T _val, CompOp _op, T _thold) {
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
            TestItf();
            virtual ~TestItf() {}

            virtual void run() = 0;
            virtual std::string        name() = 0;
            virtual std::string description() = 0;
            virtual std::string       group() = 0;
        }; // TestItf END

        struct TestSystem {
            std::vector<  TestItf*> tests;
            std::vector<AssertInfo> test_asserts;
            uint32_t test_id = 0u, test_count = 0u, test_success = 0u;

            TermCtrl::TerminalHandler terminal_h{ TermCtrl::TextStyle{},
                                                  TermCtrl::TextStyle{{TermCtrl::TFlag::FG_RGB8 , TermCtrl::TFlag::BOLD}, TermCtrl::TColor::BrightGreen},
                                                  TermCtrl::TextStyle{{TermCtrl::TFlag::FG_RGB8 , TermCtrl::TFlag::BOLD}, TermCtrl::TColor::BrightRed},
                                                  TermCtrl::TextStyle{{TermCtrl::TFlag::FG_RGB8 , TermCtrl::TFlag::BOLD}, TermCtrl::TColor::Red},
                                                  TermCtrl::TextStyle{{TermCtrl::TFlag::FG_RGB8}, TermCtrl::TColor::BrightCyan},
                                                  TermCtrl::TextStyle{{TermCtrl::TFlag::   BOLD}}, };

            enum TStyleID { TSTYLE_DEFAULT, TSTYLE_SUCCESS, TSTYLE_ERROR, TSTYLE_CRASH, TSTYLE_INFO, TSTYLE_BOLD };

            uint32_t findTestIdByName(const std::string &_test_name) {
                for (uint32_t i = 0u; i < tests.size(); ++i)
                    if (tests[i]->name() == _test_name) return i;
                return UINT32_MAX;
            }

            TestItf* findTestByName(const std::string &_test_name) {
                for (TestItf* test : tests)
                    if (test->name() == _test_name) return test;
                return nullptr;
            }

            std::vector<TestItf*> findTestsByGroup(const std::string &_test_group) {
                std::vector<TestItf*> __res;

                for (TestItf* test : tests)
                    if (test->group() == _test_group) __res.push_back(test);
                return __res;
            }

            vec2u getTestNumber(const std::string &_test_name) {
                vec2u __res{UINT32_MAX};

                std::vector<std::pair<std::string, uint32_t>> __test_groups;
                for (const auto& test : tests) {
                    bool __is_new_test_group = true;
                    for (uint32_t group_id = 0u; group_id < __test_groups.size(); ++group_id) {
                        if (__test_groups[group_id].first == test->group()) {
                            __is_new_test_group = false;
                            __res.x = group_id;
                            break;
                        }
                    }
                    if (__is_new_test_group) {
                        __res.x = __test_groups.size();
                        __test_groups.emplace_back(test->group(), 0u);
                    }
                    if (_test_name == test->name()) {
                        __res.y = __test_groups[__res.x].second;
                        break;
                    }
                    __test_groups[__res.x].second += 1;
                }
                return __res + 1;
            }

            void printHeader(const std::vector<std::string> &_test_names) {
                printf("\n");
                terminal_h.print_wcent(TSTYLE_BOLD, "SimpleVKFW Test System", '=');
                terminal_h.print_w();

                terminal_h.print_wcent("", '-');
                terminal_h.print_wcent({terminal_h.encloseWithStyle(TSTYLE_BOLD, "Running tests"), TermCtrl::StylizedString{": " + std::to_string(_test_names.size())}});
                terminal_h.print_wcent("", '-');
                terminal_h.print_w();

                for (uint32_t i = 0u; i < _test_names.size(); ++i) {
                    vec2u __test_number = getTestNumber(_test_names[i]);
                    terminal_h.print_w({{"   " + std::to_string(__test_number.x) + "." + std::to_string(__test_number.y) + ". "}, terminal_h.encloseWithStyle(TSTYLE_INFO, tests[findTestIdByName(_test_names[i])]->group()), {"."}, terminal_h.encloseWithStyle(TSTYLE_INFO, tests[findTestIdByName(_test_names[i])]->name())});
                    terminal_h.print_w();
                }
                terminal_h.print_w();
                terminal_h.print_wcent("", '=');
            }

            void printTestHeader() {
                terminal_h.print_w();
                terminal_h.print_w({terminal_h.encloseWithStyle(TSTYLE_INFO, "   TEST " + std::to_string(test_id+1)), {": " + tests[TestSystem::test_id]->name()}});

                std::vector<std::string> __split_str;
                splitString(tests[test_id]->description(), terminal_h.line.width - 3, __split_str);
                for (const std::string &_descr_str : __split_str)
                    terminal_h.print_w("   " + _descr_str);
                terminal_h.print_w();
                terminal_h.print_wcent("", '-');
            }

            void printTestResult(bool _test_crashed) {
                terminal_h.print_w();
                terminal_h.print_wcent("", '-');
                terminal_h.print_w();

                uint32_t __asserts_success = 0u, __asserts_fail = 0u;
                for (const auto &a_info : test_asserts) {
                    __asserts_success +=  a_info.assert_success;
                    __asserts_fail    += !a_info.assert_success;
                }

                if (__asserts_success)
                    terminal_h.print_w({terminal_h.encloseWithStyle(TSTYLE_SUCCESS, "         Passed"), {": " + std::to_string(__asserts_success)}});
                if (__asserts_fail)
                    terminal_h.print_w({terminal_h.encloseWithStyle(TSTYLE_ERROR,   "         Failed"), {": " + std::to_string(__asserts_fail   )}});
                if (_test_crashed)
                    terminal_h.print_w({terminal_h.encloseWithStyle(TSTYLE_CRASH,   "     TEST CRASHED")});
                test_asserts.clear();

                terminal_h.print_w();
                terminal_h.print_wcent("", '-');
            }

            void printAssertResult(const AssertInfo &_assert_info) {
                if (_assert_info.assert_op != TEST_COMP_NONE) {
                    printf("Printing placeholder\n");
                }
                else if (!_assert_info.assert_success) {
                    terminal_h.print_w({terminal_h.encloseWithStyle(TSTYLE_ERROR, "Error"), {" in assertion:"}});
                    terminal_h.print_w("   " + _assert_info.assert_descr);
                }

                test_asserts.push_back(_assert_info);
            }

            void printEnd() {
                terminal_h.print_wcent(TSTYLE_BOLD, "SimpleVKFW Test System End", '=');
                printf("\n");
            }

            void printTestList() {
                std::vector<std::string> __run_tests(tests.size(), "");
                for (uint32_t i = 0u; i < tests.size(); ++i)
                    __run_tests[i] = tests[i]->name();

                printHeader(__run_tests);
            }


        // Run tests

            void runTest(uint32_t _test_id) {
                test_id = _test_id;
                printTestHeader();
                test_asserts.clear();

                tests[_test_id]->run();

                printTestResult(false); // Argument not used because Windows can't fork
            }

            void run(const std::vector<std::string> &_test_names, const std::vector<std::string> &_test_groups) {
                std::vector<std::string> __run_tests;

                if (_test_names.empty() && _test_groups.empty()) {
                    __run_tests.resize(tests.size());
                    for (uint32_t i = 0u; i < __run_tests.size(); ++i)
                        __run_tests[i] = tests[i]->name();
                }
                if (!_test_names.empty()) {
                    __run_tests = _test_names;
                }
                if (!_test_groups.empty()) {
                    std::vector<TestItf*> __tmp_tests;
                    uint32_t __curr_size = 0u;

                    for (uint32_t i = 0u; i < _test_groups.size(); ++i) {
                        __tmp_tests = findTestsByGroup(_test_groups[i]);
                        __curr_size = __run_tests.size();

                        __run_tests.resize(__curr_size + __tmp_tests.size());
                        for (uint32_t j = 0u; j < __tmp_tests.size(); ++j)
                            __run_tests[__curr_size + j] = __tmp_tests[j]->name();
                    }
                }

                for (int i = __run_tests.size()-1; i > 0; --i) {
                    bool __is_duplicate = false;

                    for (int j = i-1; j >= 0; --j)
                        if (__run_tests[j] == __run_tests[i]) {
                            __is_duplicate = true;
                            break;
                        }

                    if (__is_duplicate)
                        __run_tests.erase(__run_tests.begin()+i);
                }

                printHeader(__run_tests);
                for (const std::string& test_name : __run_tests) runTest(findTestIdByName(test_name));
                printEnd();
            }
        } test_system; // TestSystem END


    // TestItf constructor definition

        TestItf::TestItf() {
            test_system.tests.push_back(this);
        }


    // Assert functions definition

        template <typename T>
        void testAssert(T _val, CompOp _op, T _thold,
                        const std::string &_assert_descr,
                        const std::string &_val_descr = "",
                        const std::string &_thold_descr = "") {
            std::pair<bool, std::string> __assert_res = test_assert_core(_val, _op, _thold);

            AssertInfo __res{};
            __res.assert_descr   = _assert_descr;
            __res.value_descr    =    _val_descr;
            __res.thold_descr    =  _thold_descr;
            __res.result_descr   = __assert_res.second;
            __res.assert_op      =  _op;
            __res.assert_success = __assert_res.first;

            test_system.printAssertResult(__res);
        }

        void testAssert(bool _assert_res, const std::string &_assert_descr) {
            AssertInfo __res{};
            __res.assert_op  =  TEST_COMP_NONE;
            __res.assert_descr = _assert_descr;
            __res.assert_success = _assert_res;

            test_system.printAssertResult(__res);
        }
    }; // Test END
}; // Simple END


#endif
