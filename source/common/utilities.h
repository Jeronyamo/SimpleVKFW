#ifndef SVKFW_UTILITIES_H
#define SVKFW_UTILITIES_H

#include <algorithm>

// this file contains various [template] classes/functions
// used under the hood


namespace Simple {
    namespace Util {
        //  Types and parameters

        // struct ParamFlag {};
        // template <bool flag> struct BoolFlag {};
        // BoolFlag<false> False;
        // BoolFlag<true> True;


        //  Image Utilities

        // template <int C1, int C2> struct MinConstexpr { int minv = C1 < C2 ? C1 : C2; };
        // template <int C1, int C2> struct MaxConstexpr { int maxv = C1 > C2 ? C1 : C2; };


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