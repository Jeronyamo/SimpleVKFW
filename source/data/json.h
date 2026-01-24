#ifndef SVKFW_JSON_H
#define SVKFW_JSON_H

#include "common/terminal.h"

#include <vector>


namespace Simple {
    namespace JSON {
        namespace Util {
            std::pair<uint32_t, uint32_t> parseWords(const std::vector<std::string> &_words, const char *_json_str) {
                uint32_t __res_index = UINT32_MAX, __res_offset = UINT32_MAX;

                for (uint32_t i = 0u; i < _words.size(); ++i) {
                    bool __matches = false;
                    for (uint32_t j = 0u; j < _words[i].size(); ++j) {
                        if (_json_str[j] == '\0' || _json_str[j] != _words[i][j])
                            break;
                        __matches = j+1 == _words[i].size();
                    }
                    if (__matches) { __res_index = i; __res_offset = _words[i].size(); }
                }

                return { __res_index, __res_offset };
            }
            uint32_t parseStringType(const char *_json_str) {
                bool __finish = _json_str == nullptr || _json_str[0] != '"';
                uint32_t __res_offset = UINT32_MAX;

                for (uint32_t i = 1u; !__finish; ++i) {
                    switch (_json_str[i]) {
                        case  '"': __res_offset =  i+1;
                        case '\0': __finish = true;
                        case '\\': ++i;
                    }
                }
                return __res_offset;
            }
            uint32_t skipWhitespace(const char *_json_str) {
                uint32_t __res_offset = 0u;
                if (_json_str != nullptr)
                    while (std::isspace(_json_str[__res_offset]) && _json_str[__res_offset] != '\0')
                        __res_offset += 1;
                return __res_offset;
            }
        }


        enum JSONType {
            JSON_TYPE_NULL = 1,
            JSON_TYPE_BOOL = 2,
            JSON_TYPE_NUM  = 4,
            JSON_TYPE_STR  = 8,
            JSON_TYPE_ARR  = 16,
            JSON_TYPE_OBJ  = 32,
        }; // JSONType END


        struct JSONTypeItf {
            virtual JSONType getType() const = 0;
            virtual bool      isLeaf() const = 0;

            virtual uint32_t readString(const char *_json_str) = 0;
            virtual std::vector<std::string> writeString(uint32_t _indent = 0u) const = 0;
        }; // JSONTypeItf END

        JSONTypeItf* parseJSONType(const char *_json_str, uint32_t &_res_offset);


        struct JSONTypeNull : JSONTypeItf {
            virtual JSONType getType() const override { return JSON_TYPE_NULL; }
            virtual     bool  isLeaf() const override { return true; }
            virtual uint32_t readString(const char *_json_str) override {
                std::pair<uint32_t,uint32_t> __index_offset = Util::parseWords({"null"}, _json_str);
                return __index_offset.second;
            }
            virtual std::vector<std::string> writeString(uint32_t _indent = 0u) const override {
                return {"null"};
            }
        }; // JSONTypeNull END


        struct JSONTypeBool : JSONTypeItf {
            bool value;

            virtual JSONType getType() const override { return JSON_TYPE_BOOL; }
            virtual     bool  isLeaf() const override { return true; }
            virtual uint32_t readString(const char *_json_str) override {
                std::pair<uint32_t,uint32_t> __index_offset = Util::parseWords({"false","true"}, _json_str);
                value = __index_offset.first ? true : false;
                return __index_offset.second;
            }
            virtual std::vector<std::string> writeString(uint32_t _indent = 0u) const override {
                return {value ? "true" : "false"};
            }
        }; // JSONTypeBool END


        struct JSONTypeNum : JSONTypeItf {
            double value;

            virtual JSONType getType() const override { return JSON_TYPE_NUM; }
            virtual     bool  isLeaf() const override { return true; }
            virtual uint32_t readString(const char *_json_str) override {
                std::size_t __res_offset = UINT32_MAX;
                try {
                    value = std::stod(_json_str, &__res_offset);
                }
                catch (std::exception) { __res_offset = UINT32_MAX; }
                return (uint32_t) __res_offset;
            }
            virtual std::vector<std::string> writeString(uint32_t _indent = 0u) const override {
                long long __decimal_part = (long long)value;
                return { std::abs(__decimal_part - value) < 1e-6 ? std::to_string(__decimal_part) : std::to_string(value) };
            }
        }; // JSONTypeNum END


        struct JSONTypeStr : JSONTypeItf {
            std::string value;

            virtual JSONType getType() const override { return JSON_TYPE_STR; }
            virtual     bool  isLeaf() const override { return true; }
            virtual uint32_t readString(const char *_json_str) override {
                uint32_t __res_offset = Util::parseStringType(_json_str);
                if (__res_offset != UINT32_MAX) {
                    value = std::string(_json_str+1, _json_str+__res_offset-1);
                }
                return __res_offset;
            }
            virtual std::vector<std::string> writeString(uint32_t _indent = 0u) const override {
                return {'"' + value + '"'};
            }
        }; // JSONTypeStr END


        struct JSONTypeArr : JSONTypeItf {
            std::vector<JSONTypeItf*> values;

            virtual JSONType getType() const override { return JSON_TYPE_ARR; }
            virtual     bool  isLeaf() const override { return false; }
            virtual uint32_t readString(const char *_json_str) override {
                uint32_t __res_offset = UINT32_MAX;

                bool __finish = _json_str == nullptr || _json_str[0] != '[';
                uint32_t __tmp_offset = 0u, __sum_offset = 1u;
                while (!__finish) {
                    JSONTypeItf* __curr_obj = parseJSONType(_json_str+__sum_offset, __tmp_offset);
                    if (__curr_obj == nullptr) {
                        SVKFW_WRAPWARN("JSONTypeArr :: readString", "couldn't read JSON object in array\n");
                        break;
                    }
                    __sum_offset += __tmp_offset;
                    values.push_back(__curr_obj);

                    // Finish state
                    if (_json_str[__sum_offset] == ']') {
                        __finish = _json_str[__sum_offset] == ']';
                        __sum_offset += 1;
                    }
                    else if (_json_str[__sum_offset] == ',') {
                        __sum_offset += 1;
                        // Processing trailing comma
                        __tmp_offset = Util::skipWhitespace(_json_str+__sum_offset);
                        __sum_offset += __tmp_offset;

                        // Finish state
                        if (_json_str[__sum_offset] == ']') {
                            __finish = _json_str[__sum_offset] == ']';
                            __sum_offset += 1;
                        }
                    }
                    else {
                        SVKFW_WRAPWARN("JSONTypeArr :: readString", "unexpected symbol: " + std::to_string(_json_str[__sum_offset]));
                        break;
                    }
                }
                if (__finish) __res_offset = __sum_offset;
                return __res_offset;
            }
            virtual std::vector<std::string> writeString(uint32_t _indent = 0u) const override {
                const std::string __indent_str = std::string(size_t(_indent), ' ');
                std::vector<std::string> __res, __tmp_res;
                if (values.empty()) _indent = 0u;

                __res.push_back("[");
                for (uint32_t i = 0u; i < values.size(); ++i) {
                    __tmp_res = values[i]->writeString(_indent);
                    for (auto& ln : __tmp_res)
                        __res.push_back(__indent_str + ln);
                    if (i+1 < values.size())
                        __res.back() += ",";
                }
                __res.push_back("]");

                if (_indent == 0) {
                    for (uint32_t i = 1u; i < __res.size(); ++i)
                        __res[0] += ' ' + __res[i];
                    __res.resize(1);
                }

                return __res;
            }
        }; // JSONTypeArr END


        struct JSONTypeObj : JSONTypeItf {
            std::vector<std::pair<JSONTypeItf*,JSONTypeItf*>> values;

            virtual JSONType getType() const override { return JSON_TYPE_OBJ; }
            virtual     bool  isLeaf() const override { return false; }
            virtual uint32_t readString(const char *_json_str) override {
                uint32_t __res_offset = UINT32_MAX;

                bool __finish = _json_str == nullptr || _json_str[0] != '{';
                uint32_t __tmp_offset = 0u, __sum_offset = 1u;
                while (!__finish) {
                    // Reading string
                    JSONTypeItf* __curr_obj = parseJSONType(_json_str+__sum_offset, __tmp_offset);
                    if (__curr_obj == nullptr) {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("JSONTypeObj :: readString", "couldn't read first JSON object (string) in object\n"));
                        break;
                    }
                    __sum_offset += __tmp_offset;
                    values.push_back({__curr_obj, nullptr});

                    // Reading ':'
                    if (_json_str[__sum_offset] != ':') {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("JSONTypeObj :: readString", "unexpected symbol instead of ':' - '%c'\n"), _json_str[__sum_offset]);
                        break;
                    }
                    __sum_offset += 1;

                    // Reading object
                    __curr_obj = parseJSONType(_json_str+__sum_offset, __tmp_offset);
                    if (__curr_obj == nullptr) {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("JSONTypeObj :: readString", "couldn't read second JSON object in object\n"));
                        break;
                    }
                    __sum_offset += __tmp_offset;
                    values.back().second = __curr_obj;

                    // Finish state
                    if (_json_str[__sum_offset] == '}') {
                        __finish = _json_str[__sum_offset] == '}';
                        __sum_offset += 1;
                    }
                    else if (_json_str[__sum_offset] == ',') {
                        __sum_offset += 1;
                        // Processing trailing comma
                        __tmp_offset = Util::skipWhitespace(_json_str+__sum_offset);
                        __sum_offset += __tmp_offset;

                        // Finish state
                        if (_json_str[__sum_offset] == '}') {
                            __finish = _json_str[__sum_offset] == '}';
                            __sum_offset += 1;
                        }
                    }
                    else {
                        std::string __warn_msg = SVKFW_WRAPWARN("JSONTypeObj :: readString", "unexpected symbol: " + std::to_string(_json_str[__sum_offset]));
                        fprintf(svkfwwarn, __warn_msg.c_str());
                        break;
                    }
                }
                if (__finish) __res_offset = __sum_offset;
                return __res_offset;
            }
            virtual std::vector<std::string> writeString(uint32_t _indent = 0u) const override {
                const std::string __indent_str = std::string(size_t(_indent), ' ');
                std::vector<std::string> __res, __tmp_res;
                if (values.empty()) _indent = 0u;

                __res.push_back("{");
                for (uint32_t i = 0u; i < values.size(); ++i) {
                    __tmp_res = values[i].first->writeString(_indent);
                    for (auto& ln : __tmp_res)
                        __res.push_back(__indent_str + ln);
                    __res.back() += " : ";

                    __tmp_res = values[i].second->writeString(_indent);
                    __res.back() += __tmp_res[0];
                    for (uint32_t j = 1u; j < __tmp_res.size(); ++j)
                        __res.push_back(__indent_str + __tmp_res[j]);
                    if (i+1 < values.size())
                        __res.back() += ",";
                }
                __res.push_back("}");

                if (_indent == 0) {
                    for (uint32_t i = 1u; i < __res.size(); ++i)
                        __res[0] += ' ' + __res[i];
                    __res.resize(1);
                }

                return __res;
            }
        }; // JSONTypeObj END



        JSONTypeItf* parseJSONType(const char *_json_str, uint32_t &_res_offset) {
            JSONTypeItf* __res_ptr = nullptr;
            _res_offset = 0u;
            if (_json_str == nullptr) { return nullptr; }

            uint32_t __tmp_offset = Util::skipWhitespace(_json_str);
            _res_offset += __tmp_offset;

            switch (_json_str[_res_offset]) {
                case 't':
                case 'f': __res_ptr = new JSONTypeBool{}; break;
                case 'n': __res_ptr = new JSONTypeNull{}; break;
                case '"': __res_ptr = new JSONTypeStr {}; break;
                case '[': __res_ptr = new JSONTypeArr {}; break;
                case '{': __res_ptr = new JSONTypeObj {}; break;
                default : __res_ptr = new JSONTypeNum {}; break;
            }
            __tmp_offset = __res_ptr->readString(_json_str+_res_offset);
            if (__tmp_offset == UINT32_MAX) {
                if (__res_ptr != nullptr) delete __res_ptr;
                _res_offset = UINT32_MAX;
                return nullptr;
            }
            _res_offset += __tmp_offset;
            auto lines = __res_ptr->writeString();

            __tmp_offset = Util::skipWhitespace(_json_str+_res_offset);
            _res_offset += __tmp_offset;
            return __res_ptr;
        }
    }; // JSON END
}; // Simple END

#endif