#ifndef SVKFW_UTILITIES_H
#define SVKFW_UTILITIES_H

#include <algorithm>

// this file contains various [template] classes/functions
// used under the hood


namespace Simple {
    namespace Util {
        // types and parameters
        const struct BoolCl { bool value; } FALSE = { false }, TRUE = { true };

        struct ParamFlag {};
        template <bool flag> struct BoolFlag {};
        BoolFlag<false> False;
        BoolFlag<true> True;


        template <typename A, bool _val> struct safe_make_signed {};
        template <typename A> struct safe_make_signed<A, true> {
            using type = std::make_signed<A>;
        };
        template <typename A> struct safe_make_signed<A, false> {
            using type = A;
        };


        //  ============  Image Utilities  ============  \\

        // if unique_ptr/shared_ptr memory is provided by malloc, set this as the deleter
        struct FreeDeleter { void operator()(void *ptr) const { free(ptr); } };

        template <int C1, int C2> struct MinConstexpr { int minv = C1 < C2 ? C1 : C2; };
        template <int C1, int C2> struct MaxConstexpr { int maxv = C1 > C2 ? C1 : C2; };


        //  Vectors
        template <class C>
        using remove_ref = typename std::remove_reference<C>::type;
        template <class C>
        using remove_ref_c = typename std::remove_const<typename std::remove_reference<C>::type>::type;
        template <class C, class Y>
        struct is_same_base {
            static constexpr bool value = std::is_same<remove_ref_c<C>, remove_ref_c<Y>>::value;
        };
        template <typename C, typename Y>
        using comm_type = typename std::common_type<C, Y>::type;
        template <typename C, typename Y>
        using comm_vecsub_type = typename safe_make_signed<comm_type<C,Y>,
                                            std::is_unsigned<std::common_type<C, Y>>::value>::type;
    }; // Util END
}; // Simple END

#endif