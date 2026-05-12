#ifndef SVKFW_VKFW_ENUM_H
#define SVKFW_VKFW_ENUM_H

#include <map>
#include <string>


namespace Simple {
    namespace VKFW {
        namespace Enum {
        // Class

            struct EnumWrap {
                std::map<uint32_t, std::string> enum_names;

                EnumWrap(const std::map<uint32_t, std::string> &_enum_names) : enum_names{_enum_names} {}

                std::string getName(uint32_t _enum_val) const {
                    auto __enum_iter = enum_names.find(_enum_val);
                    return __enum_iter == enum_names.end() ? "" : __enum_iter->second;
                }

                std::string getBitNames(uint32_t _enum_val) const {
                    std::string __res_str = "";

                    for (uint32_t i = 0u; i < 32u; ++i) {
                        if ((_enum_val & (1<<i)) == 0) continue;

                        std::string __curr_str = getName(1 << i);
                        if (!__curr_str.empty()) {
                            if (!__res_str.empty()) __res_str += " | ";
                            __res_str += __curr_str;
                        }
                    }
                    return __res_str;
                }
            }; // EnumWrap END


        // Objects
        }; // Enum END
    }; // VKFW END
}; // Simple END

#endif