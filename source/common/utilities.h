#ifndef SVKFW_UTILITIES_H
#define SVKFW_UTILITIES_H

#include <type_traits>

#include "common/terminal.h"
#include "common/memory.h"

// this file contains various [template] classes/functions used under the hood


namespace Simple {
    namespace Util {
        //  Types and parameters

        struct EnumCollector {
            std::map<std::string, std::vector<std::string>> enum_collection;

            void addEnum(const std::string &_enum_name, std::vector<std::string> _enum_values) {
                for (const auto& en_data : enum_collection) {
                    SVKFW_WASSERT(en_data.first != _enum_name, "Util :: EnumCollector", "Enum '" + _enum_name + "' - already exists");
                    if (en_data.first == _enum_name) return;
                }

                for (char ch : _enum_name) {
                    SVKFW_ASSERT(!isspace(ch), std::invalid_argument, "Util :: EnumCollector",
                                "Enum '" + _enum_name + "' - enum name contains space character");
                }

                for (uint32_t i = 0u; i < _enum_values.size(); ++i) {
                    std::vector<uint32_t> __dups;

                    for (char ch : _enum_values[i]) {
                        SVKFW_ASSERT(!isspace(ch), std::invalid_argument, "Util :: EnumCollector",
                                  "Enum '" + _enum_name + "', value '" + _enum_values[i] + "' - contains space character");
                    }

                    for (uint32_t j = _enum_values.size()-1; j > i; --j)
                        if (_enum_values[i] == _enum_values[j]) __dups.push_back(j);

                    SVKFW_WASSERT(__dups.empty(), "Util :: EnumCollector",
                                  "Enum '" + _enum_name + "', value '" + _enum_values[i] + "' - met " + std::to_string(__dups.size() + 1) + " times");

                    for (uint32_t j : __dups)
                        _enum_values.erase(_enum_values.begin()+j);
                }

                enum_collection[_enum_name] = _enum_values;
            }
        }; // EnumCollector END

    // Used to simplify work with all the "const char **" struct members in Vulkan
        struct StringVec {
            std::vector<std::string> list;

            StringVec() {}
            StringVec(const std::vector<std::string> &_l) : list{_l} { updateEntriesPtr(); }
            StringVec(const StringVec &_sv) : list{_sv.list} { updateEntriesPtr(); }
            StringVec(StringVec &&_sv) : list{std::move(_sv.list)}, _ptr{_sv._ptr} { _sv._ptr = nullptr; }
            StringVec(const char **_arr, uint32_t _count) {
                list.insert(list.end(), _arr, _arr + _count);
                updateEntriesPtr();
            }
           ~StringVec() { clear(); }

            const StringVec &operator=(const StringVec &_sv) {
                setEntries(_sv);
                return *this;
            }

            void clear() {
                list.clear();
                safeDeleteArr(_ptr);
            }

            uint32_t count() const { return list.size(); }


            void setEntries(const StringVec &_sv) {
                list = _sv.list;
                updateEntriesPtr();
            }

            void addEntries(const std::vector<std::string> &_l) {
                list.insert(list.end(), _l.begin(), _l.end());
                updateEntriesPtr();
            }

            const char **getEntries() const {
                return _ptr;
            }

        private:
            const char **_ptr = nullptr;

            void updateEntriesPtr() {
                safeDeleteArr(_ptr);

                if (list.size()) {
                    _ptr = new const char *[list.size()];
                    for (uint32_t i = 0u; i < list.size(); ++i)
                        _ptr[i] = list[i].c_str();
                }
            }
        }; // StringVec END


        //  Functions

        std::string stringStrip(const std::string &_in_str) {
            long __offset1 = 0, __offset2 = _in_str.size()-1;
            for (; __offset1 < _in_str.size() && std::isspace(_in_str[__offset1]); ++__offset1);
            for (; __offset2 >=             0 && std::isspace(_in_str[__offset2]); --__offset2);
            return _in_str.substr(__offset1, __offset2+1);
        }


        //  Template "shortcuts"

        template <typename C, bool _val> struct safe_make_signed_b {};
        template <typename C> struct safe_make_signed_b<C,  true> {
            using type = typename std::make_signed<C>::type;
        };
        template <typename C> struct safe_make_signed_b<C, false> {
            using type = C;
        };
        template <typename C>
        using safe_make_signed = typename safe_make_signed_b<C, std::is_unsigned<C>::value>::type;

        template <class C>
        using remove_ref   = typename std::remove_reference<C>::type;
        template <class C>
        using remove_ref_c = typename std::remove_const<typename std::remove_reference<C>::type>::type;

        template <class C, class Y>
        struct is_same_base {
            static constexpr bool value = std::is_same<remove_ref_c<C>, remove_ref_c<Y>>::value;
        };

        template <typename C, typename Y>
        using comm_type = typename std::common_type<C, Y>::type;
        template <typename C, typename Y>
        using comm_vecsub_type = safe_make_signed<comm_type<C,Y>>;
    }; // Util END
}; // Simple END

#endif