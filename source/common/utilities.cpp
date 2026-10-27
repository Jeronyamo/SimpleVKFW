#include "common/utilities.h"

namespace Simple {
    namespace Util {

        void EnumCollector::addEnum(const std::string &_enum_name, std::vector<std::string> _enum_values) {
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

        StringVec::StringVec() {}

        StringVec::StringVec(const std::vector<std::string> &_l) : list{_l} { updateEntriesPtr(); }

        StringVec::StringVec(const StringVec &_sv) : list{_sv.list} { updateEntriesPtr(); }

        StringVec::StringVec(StringVec &&_sv) : list{std::move(_sv.list)}, _ptr{_sv._ptr} { _sv._ptr = nullptr; }

        StringVec::StringVec(const char **_arr, uint32_t _count) {
            list.insert(list.end(), _arr, _arr + _count);
            updateEntriesPtr();
        }

        StringVec::~StringVec() { clear(); }

        const StringVec &StringVec::operator=(const StringVec &_sv) {
            setEntries(_sv);
            return *this;
        }

        void StringVec::clear() {
            list.clear();
            safeDeleteArr(_ptr);
        }

        uint32_t StringVec::count() const { return list.size(); }

        void StringVec::setEntries(const StringVec &_sv) {
            list = _sv.list;
            updateEntriesPtr();
        }

        void StringVec::addEntries(const std::vector<std::string> &_l) {
            list.insert(list.end(), _l.begin(), _l.end());
            updateEntriesPtr();
        }

        const char **StringVec::getEntries() const {
            return _ptr;
        }

        void StringVec::updateEntriesPtr() {
            safeDeleteArr(_ptr);

            if (list.size()) {
                _ptr = new const char *[list.size()];
                for (uint32_t i = 0u; i < list.size(); ++i)
                    _ptr[i] = list[i].c_str();
            }
        }

        std::string stringStrip(const std::string &_in_str) {
            long __offset1 = 0, __offset2 = long(_in_str.size()) - 1;
            for (; __offset1 <= __offset2 && std::isspace((unsigned char)_in_str[__offset1]); ++__offset1);
            for (; __offset2 >= __offset1 && std::isspace((unsigned char)_in_str[__offset2]); --__offset2);
            return _in_str.substr(__offset1, __offset2 - __offset1 + 1); // second argument is length
        }
    }; // Util END
}; // Simple END
