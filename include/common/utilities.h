#ifndef SVKFW_UTILITIES_H
#define SVKFW_UTILITIES_H

#include <type_traits>
#include <map>

#include "common/terminal.h"
#include "common/memory.h"

// this file contains various [template] classes/functions used under the hood


namespace Simple {
    namespace Util {
        //  Types and parameters

        struct EnumCollector {
            std::map<std::string, std::vector<std::string>> enum_collection;

            void addEnum(const std::string &_enum_name, std::vector<std::string> _enum_values);
        }; // EnumCollector END

    // Used to simplify work with all the "const char **" struct members in Vulkan
        struct StringVec {
            std::vector<std::string> list;

            StringVec();
            StringVec(const std::vector<std::string> &_l);
            StringVec(const StringVec &_sv);
            StringVec(StringVec &&_sv);
            StringVec(const char **_arr, uint32_t _count);
           ~StringVec();

            const StringVec &operator=(const StringVec &_sv);

            void clear();

            uint32_t count() const;


            void setEntries(const StringVec &_sv);

            void addEntries(const std::vector<std::string> &_l);

            const char **getEntries() const;

        private:
            const char **_ptr = nullptr;

            void updateEntriesPtr();
        }; // StringVec END


        //  Functions

        std::string stringStrip(const std::string &_in_str);



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