#ifndef SVKFW_COLOR_H
#define SVKFW_COLOR_H

// #define SVKFW_COL_FTOU 255u
// #define SVKFW_COL_UTOF 0.003921568859f     // 1 / 255

#include <type_traits>
#include <stdexcept>

#include "math/vectors.h"


namespace Simple {
    namespace Color {
    //  ============  Color classes declaration  ============  \\

        typedef           float col1f;
        typedef             int col1i;
        typedef   unsigned char col1u;
        typedef Vec2Base<col1f> col2f;
        typedef Vec2Base<col1i> col2i;
        typedef Vec2Base<col1u> col2u;
        typedef Vec3Base<col1f> col3f;
        typedef Vec3Base<col1i> col3i;
        typedef Vec3Base<col1u> col3u;
        typedef Vec4Base<col1f> col4f;
        typedef Vec4Base<col1i> col4i;
        typedef Vec4Base<col1u> col4u;


    //  ============  Utilities  ============  \\

        namespace Util {
        //  ============  Color channels of a type  ============  \\

            // Construct color type from channel type and the number of channels
            template <typename T, int Ch> struct ColorConstructorSct {};
            template <> struct ColorConstructorSct<col1f, 1> { using type = col1f; };
            template <> struct ColorConstructorSct<col1i, 1> { using type = col1i; };
            template <> struct ColorConstructorSct<col1u, 1> { using type = col1u; };
            template <> struct ColorConstructorSct<col1f, 2> { using type = col2f; };
            template <> struct ColorConstructorSct<col1i, 2> { using type = col2i; };
            template <> struct ColorConstructorSct<col1u, 2> { using type = col2u; };
            template <> struct ColorConstructorSct<col1f, 3> { using type = col3f; };
            template <> struct ColorConstructorSct<col1i, 3> { using type = col3i; };
            template <> struct ColorConstructorSct<col1u, 3> { using type = col3u; };
            template <> struct ColorConstructorSct<col1f, 4> { using type = col4f; };
            template <> struct ColorConstructorSct<col1i, 4> { using type = col4i; };
            template <> struct ColorConstructorSct<col1u, 4> { using type = col4u; };

            template <typename T, int Ch>
            using ColorConstructor = typename ColorConstructorSct<T, Ch>::type;

            // Get number of channels from a color type
            template <typename T> struct ColorChannels{}; // THIS prevents from using channel types other than float,uchar,int
            template <> struct ColorChannels<col1f> { static constexpr uint32_t channels = 1u; };
            template <> struct ColorChannels<col1i> { static constexpr uint32_t channels = 1u; };
            template <> struct ColorChannels<col1u> { static constexpr uint32_t channels = 1u; };

            template <typename T>
            struct ColorChannels<Vec2Base<T>> { static constexpr uint32_t channels = 2u; };
            template <typename T>
            struct ColorChannels<Vec3Base<T>> { static constexpr uint32_t channels = 3u; };
            template <typename T>
            struct ColorChannels<Vec4Base<T>> { static constexpr uint32_t channels = 4u; };

            // Get channel type from a color type
            template <typename T>
            struct ColorChanTypeSct              { using type = T; };
            template <typename T>
            struct ColorChanTypeSct<Vec2Base<T>> { using type = T; };
            template <typename T>
            struct ColorChanTypeSct<Vec3Base<T>> { using type = T; };
            template <typename T>
            struct ColorChanTypeSct<Vec4Base<T>> { using type = T; };

            template <typename T>
            using  ColorChanType = typename ColorChanTypeSct<T>::type;


            template <typename T> struct ColorChanOpsTypeSct {}; // THIS prevents from using channel types other than float,uchar,int
            template <> struct ColorChanOpsTypeSct<col1f> { using type = col1f; };
            template <> struct ColorChanOpsTypeSct<col1i> { using type = col1i; };
            template <> struct ColorChanOpsTypeSct<col1u> { using type = col1i; };

            template <typename T>
            using  ColorChanOpsType = typename ColorChanOpsTypeSct<ColorChanType<T>>::type;


            template <typename T>
            struct ChanMax {}; // THIS prevents from using channel types other than float,uchar,int
            template <> struct ChanMax<col1u> { static constexpr   int val = 255; };
            template <> struct ChanMax<col1i> { static constexpr   int val = 255; };
            template <> struct ChanMax<col1f> { static constexpr float val = 1.f; };

            template <typename T>
            struct OtherChanTypeSct {}; // THIS prevents from using channel types other than float,uchar,int
            template <> struct OtherChanTypeSct<col1f> { using type = col1u; };
            template <> struct OtherChanTypeSct<col1i> { using type = col1f; };
            template <> struct OtherChanTypeSct<col1u> { using type = col1f; };
            template <typename T>
            struct OtherChanTypeSct<Vec2Base<T>> { using type = typename OtherChanTypeSct<T>::type; };
            template <typename T>
            struct OtherChanTypeSct<Vec3Base<T>> { using type = typename OtherChanTypeSct<T>::type; };
            template <typename T>
            struct OtherChanTypeSct<Vec4Base<T>> { using type = typename OtherChanTypeSct<T>::type; };

            template <typename T>
            using OtherChanType = typename OtherChanTypeSct<T>::type;


            template <typename T>
            struct OtherColorTypeSct              { using type =          OtherChanTypeSct<T> ; };
            template <typename T>
            struct OtherColorTypeSct<Vec2Base<T>> { using type = Vec2Base<OtherChanTypeSct<T>>; };
            template <typename T>
            struct OtherColorTypeSct<Vec3Base<T>> { using type = Vec3Base<OtherChanTypeSct<T>>; };
            template <typename T>
            struct OtherColorTypeSct<Vec4Base<T>> { using type = Vec4Base<OtherChanTypeSct<T>>; };

            template <typename T>
            using OtherColorType = typename OtherColorTypeSct<T>::type;


            template <bool FromFlt, bool ToFlt>
            struct ConvertCoefSct { static constexpr int coef = 1; };
            template <>
            struct ConvertCoefSct<true, false> { static constexpr   int coef = 255; };
            template <>
            struct ConvertCoefSct<false, true> { static constexpr float coef = 1.f / 255; };

            // color conversion coefficient (uchar-based <-> float-based)
            // takes two <ops>/<color> types, finds <base ops>/<base> types
            // (int, uchar, float) and checks using is_floating_point
            template <typename T1, typename T2>
            using ConvertCoef = ConvertCoefSct<std::is_floating_point<ColorChanType<T1>>::value,
                                               std::is_floating_point<ColorChanType<T2>>::value>;
        }; // Util END


        // Utility functions

        namespace Func {
        // Combine channels: 2 arguments
            template <typename T, typename T1, typename T2>
            inline Vec2Base<T> combineChannels(T1 _c1, T2 _c2) { return { T(_c1 * Util::ConvertCoef<T1, T>::coef),
                                                                          T(_c2 * Util::ConvertCoef<T2, T>::coef) }; }
            template <typename T, typename T1, typename T2>
            inline Vec3Base<T> combineChannels(const Vec2Base<T1> &_c1, T2 _c2) { return { Vec2Base<T>(_c1 * Util::ConvertCoef<T1, T>::coef),
                                                                                                    T (_c2 * Util::ConvertCoef<T2, T>::coef) }; }
            template <typename T, typename T1, typename T2>
            inline Vec3Base<T> combineChannels(T1 _c1, const Vec2Base<T2> &_c2) { return {          T (_c1 * Util::ConvertCoef<T1, T>::coef),
                                                                                           Vec2Base<T>(_c2 * Util::ConvertCoef<T2, T>::coef) }; }
            template <typename T, typename T1, typename T2>
            inline Vec4Base<T> combineChannels(const Vec3Base<T1> &_c1, T2 _c2) { return { Vec3Base<T>(_c1 * Util::ConvertCoef<T1, T>::coef),
                                                                                                    T (_c2 * Util::ConvertCoef<T2, T>::coef) }; }
            template <typename T, typename T1, typename T2>
            inline Vec4Base<T> combineChannels(T1 _c1, const Vec3Base<T2> &_c2) { return {          T (_c1 * Util::ConvertCoef<T1, T>::coef),
                                                                                           Vec3Base<T>(_c2 * Util::ConvertCoef<T2, T>::coef) }; }
            template <typename T, typename T1, typename T2>
            inline Vec4Base<T> combineChannels(const Vec2Base<T1> &_c1, const Vec2Base<T2> &_c2) { return { Vec2Base<T>(_c1 * Util::ConvertCoef<T1, T>::coef),
                                                                                                            Vec2Base<T>(_c2 * Util::ConvertCoef<T2, T>::coef) }; }

            // Combining 2 arguments of the same (channel) type
            template <typename T>
            inline Vec2Base<T> combineChannels(T _c1, T _c2) { return { _c1, _c2 }; }
            template <typename T>
            inline Vec3Base<T> combineChannels(const Vec2Base<T> &_c1, T _c2) { return { _c1, _c2 }; }
            template <typename T>
            inline Vec3Base<T> combineChannels(T _c1, const Vec2Base<T> &_c2) { return { _c1, _c2 }; }
            template <typename T>
            inline Vec4Base<T> combineChannels(const Vec3Base<T> &_c1, T _c2) { return { _c1, _c2 }; }
            template <typename T>
            inline Vec4Base<T> combineChannels(T _c1, const Vec3Base<T> &_c2) { return { _c1, _c2 }; }
            template <typename T>
            inline Vec4Base<T> combineChannels(const Vec2Base<T> &_c1, const Vec2Base<T> &_c2) { return { _c1, _c2 }; }

        // Combine channels: 3 arguments
            template <typename T, typename T1, typename T2, typename T3>
            inline Vec3Base<T> combineChannels(T1 _c1, T2 _c2, T3 _c3) { return { T(_c1 * Util::ConvertCoef<T1, T>::coef),
                                                                                  T(_c2 * Util::ConvertCoef<T2, T>::coef),
                                                                                  T(_c3 * Util::ConvertCoef<T3, T>::coef) }; }
            template <typename T, typename T1, typename T2, typename T3>
            inline Vec3Base<T> combineChannels(const Vec2Base<T1> &_c1, T2 _c2, T3 _c3) { return { Vec2Base<T>(_c1 * Util::ConvertCoef<T1, T>::coef),
                                                                                                            T (_c2 * Util::ConvertCoef<T2, T>::coef),
                                                                                                            T (_c3 * Util::ConvertCoef<T3, T>::coef) }; }
            template <typename T, typename T1, typename T2, typename T3>
            inline Vec3Base<T> combineChannels(T1 _c1, const Vec2Base<T2> &_c2, T3 _c3) { return {          T (_c1 * Util::ConvertCoef<T1, T>::coef),
                                                                                                   Vec2Base<T>(_c2 * Util::ConvertCoef<T2, T>::coef),
                                                                                                            T (_c3 * Util::ConvertCoef<T3, T>::coef) }; }
            template <typename T, typename T1, typename T2, typename T3>
            inline Vec3Base<T> combineChannels(T1 _c1, T2 _c2, const Vec2Base<T3> &_c3) { return {          T (_c1 * Util::ConvertCoef<T1, T>::coef),
                                                                                                            T (_c2 * Util::ConvertCoef<T2, T>::coef),
                                                                                                   Vec2Base<T>(_c3 * Util::ConvertCoef<T3, T>::coef) }; }

            // Combining 3 arguments of the same (channel) type
            template <typename T>
            inline Vec3Base<T> combineChannels(T _c1, T _c2, T _c3) { return { _c1, _c2, _c3 }; }
            template <typename T>
            inline Vec3Base<T> combineChannels(const Vec2Base<T> &_c1, T _c2, T _c3) { return { _c1, _c2, _c3 }; }
            template <typename T>
            inline Vec3Base<T> combineChannels(T _c1, const Vec2Base<T> &_c2, T _c3) { return { _c1, _c2, _c3 }; }
            template <typename T>
            inline Vec3Base<T> combineChannels(T _c1, T _c2, const Vec2Base<T> &_c3) { return { _c1, _c2, _c3 }; }

        // Combine channels: 4 arguments
            template <typename T, typename T1, typename T2, typename T3, typename T4>
            inline Vec4Base<T> combineChannels(T1 _c1, T2 _c2, T3 _c3, T4 _c4) { return { T(_c1 * Util::ConvertCoef<T1, T>::coef),
                                                                                          T(_c2 * Util::ConvertCoef<T2, T>::coef),
                                                                                          T(_c3 * Util::ConvertCoef<T3, T>::coef),
                                                                                          T(_c4 * Util::ConvertCoef<T4, T>::coef) }; }

            // Combining 4 arguments of the same (channel) type
            template <typename T>
            inline Vec4Base<T> combineChannels(T _c1, T _c2, T _c3, T _c4) { return { _c1,_c2,_c3,_c4 }; }


        // Convert types
            template <typename T, typename T1>
            inline T convert(const T1 &_v) { return T(_v * Util::ConvertCoef<T1, T>::coef); }
            template <typename T, typename T1>
            inline Vec2Base<T> convert(const Vec2Base<T1> &_v) { return { T(_v.x * Util::ConvertCoef<T1, T>::coef),
                                                                          T(_v.y * Util::ConvertCoef<T1, T>::coef) }; }
            template <typename T, typename T1>
            inline Vec3Base<T> convert(const Vec3Base<T1> &_v) { return { T(_v.x * Util::ConvertCoef<T1, T>::coef),
                                                                          T(_v.y * Util::ConvertCoef<T1, T>::coef),
                                                                          T(_v.z * Util::ConvertCoef<T1, T>::coef) }; }
            template <typename T, typename T1>
            inline Vec4Base<T> convert(const Vec4Base<T1> &_v) { return { T(_v.x * Util::ConvertCoef<T1, T>::coef),
                                                                          T(_v.y * Util::ConvertCoef<T1, T>::coef),
                                                                          T(_v.z * Util::ConvertCoef<T1, T>::coef),
                                                                          T(_v.w * Util::ConvertCoef<T1, T>::coef) }; }

        // Access channels
            template <typename T>
            inline T getChannel(T _col, int _ch1) { return _col; }
            template <typename T>
            inline T getChannel(const Vec2Base<T> _col, int _ch1) { return _col[_ch1]; }
            template <typename T>
            inline T getChannel(const Vec3Base<T> _col, int _ch1) { return _col[_ch1]; }
            template <typename T>
            inline T getChannel(const Vec4Base<T> _col, int _ch1) { return _col[_ch1]; }

        // Clamp color values to LDR limits
            template <typename T>
            inline T clampLDR(const T &_col) {
                return Math::clampCL(_col, {0}, {Util::ChanMax<Util::ColorChanType<T>>::val});
            }
        }; // Func END



//  ========================================  \\
                Color operations
//  ========================================  \\

//  ============  Porter-Duff blends 4-4, 4-3, 3-4  ============  \\

        template <typename T>
        Vec4Base<T> PorterDuffBlendOver(const Vec4Base<T> &_c1, const Vec4Base<T> &_c2)
            { return (Util::ChanMax<T>::val - _c1.w) * Util::ConvertCoef<T, float>::coef * _c2 + _c1; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendIn  (const Vec4Base<T> &_c1, const Vec4Base<T> &_c2)
            { return                          _c2.w  * Util::ConvertCoef<T, float>::coef * _c1; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendOut (const Vec4Base<T> &_c1, const Vec4Base<T> &_c2)
            { return (Util::ChanMax<T>::val - _c2.w) * Util::ConvertCoef<T, float>::coef * _c1; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendAtop(const Vec4Base<T> &_c1, const Vec4Base<T> &_c2)
            { return                          _c2.w  * Util::ConvertCoef<T, float>::coef * _c1 +
                     (Util::ChanMax<T>::val - _c1.w) * Util::ConvertCoef<T, float>::coef * _c2; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendXor (const Vec4Base<T> &_c1, const Vec4Base<T> &_c2)
            { return (Util::ChanMax<T>::val - _c2.w) * Util::ConvertCoef<T, float>::coef * _c1 +
                     (Util::ChanMax<T>::val - _c1.w) * Util::ConvertCoef<T, float>::coef * _c2; }

        template <typename T> inline
        Vec4Base<T> PorterDuffBlendOver(const Vec4Base<T> &_c1, const Vec3Base<T> &_c2) { return PorterDuffBlendOver(_c1, Vec4Base<T>(_c2, Util::ChanMax<T>::val)); }
        template <typename T> inline
        Vec4Base<T> PorterDuffBlendOver(const Vec3Base<T> &_c1, const Vec4Base<T> &_c2) { return PorterDuffBlendOver(Vec4Base<T>(_c1, Util::ChanMax<T>::val), _c2); }
        template <typename T> inline
        Vec4Base<T> PorterDuffBlendIn  (const Vec4Base<T> &_c1, const Vec3Base<T> &_c2) { return PorterDuffBlendIn  (_c1, Vec4Base<T>(_c2, Util::ChanMax<T>::val)); }
        template <typename T> inline
        Vec4Base<T> PorterDuffBlendIn  (const Vec3Base<T> &_c1, const Vec4Base<T> &_c2) { return PorterDuffBlendIn  (Vec4Base<T>(_c1, Util::ChanMax<T>::val), _c2); }
        template <typename T> inline
        Vec4Base<T> PorterDuffBlendOut (const Vec4Base<T> &_c1, const Vec3Base<T> &_c2) { return PorterDuffBlendOut (_c1, Vec4Base<T>(_c2, Util::ChanMax<T>::val)); }
        template <typename T> inline
        Vec4Base<T> PorterDuffBlendOut (const Vec3Base<T> &_c1, const Vec4Base<T> &_c2) { return PorterDuffBlendOut (Vec4Base<T>(_c1, Util::ChanMax<T>::val), _c2); }
        template <typename T> inline
        Vec4Base<T> PorterDuffBlendAtop(const Vec4Base<T> &_c1, const Vec3Base<T> &_c2) { return PorterDuffBlendAtop(_c1, Vec4Base<T>(_c2, Util::ChanMax<T>::val)); }
        template <typename T> inline
        Vec4Base<T> PorterDuffBlendAtop(const Vec3Base<T> &_c1, const Vec4Base<T> &_c2) { return PorterDuffBlendAtop(Vec4Base<T>(_c1, Util::ChanMax<T>::val), _c2); }
        template <typename T> inline
        Vec4Base<T> PorterDuffBlendXor (const Vec4Base<T> &_c1, const Vec3Base<T> &_c2) { return PorterDuffBlendXor (_c1, Vec4Base<T>(_c2, Util::ChanMax<T>::val)); }
        template <typename T> inline
        Vec4Base<T> PorterDuffBlendXor (const Vec3Base<T> &_c1, const Vec4Base<T> &_c2) { return PorterDuffBlendXor (Vec4Base<T>(_c1, Util::ChanMax<T>::val), _c2); }
    }; // Color END
}; // Simple END

#endif