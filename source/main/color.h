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

        typedef             float Color1f;
        typedef               int Color1i;
        typedef     unsigned char Color1u;
        typedef Vec2Base<Color1f> Color2f;
        typedef Vec2Base<Color1i> Color2i;
        typedef Vec2Base<Color1u> Color2u;
        typedef Vec3Base<Color1f> Color3f;
        typedef Vec3Base<Color1i> Color3i;
        typedef Vec3Base<Color1u> Color3u;
        typedef Vec4Base<Color1f> Color4f;
        typedef Vec4Base<Color1i> Color4i;
        typedef Vec4Base<Color1u> Color4u;


    //  ============  Utilities  ============  \\

        namespace Util {
        //  ============  Color channels of a type  ============  \\

            template <typename T> struct Channels{}; // THIS prevents from using channel types other than float,uchar,int
            template <> struct Channels<Color1f> { static constexpr uint32_t value = 1u; };
            template <> struct Channels<Color1i> { static constexpr uint32_t value = 1u; };
            template <> struct Channels<Color1u> { static constexpr uint32_t value = 1u; };

            template <typename T>
            struct Channels<Vec2Base<T>> { static constexpr uint32_t value = 2u; };
            template <typename T>
            struct Channels<Vec3Base<T>> { static constexpr uint32_t value = 3u; };
            template <typename T>
            struct Channels<Vec4Base<T>> { static constexpr uint32_t value = 4u; };


            template <typename T>
            struct ChannelTypeSct              { using type = T; };
            template <typename T>
            struct ChannelTypeSct<Vec2Base<T>> { using type = T; };
            template <typename T>
            struct ChannelTypeSct<Vec3Base<T>> { using type = T; };
            template <typename T>
            struct ChannelTypeSct<Vec4Base<T>> { using type = T; };
            template <typename T>
            using  ChannelType = typename ChannelTypeSct<T>::type;

            template <typename T> struct ChannelOpsTypeSct {}; // THIS prevents from using channel types other than float,uchar,int
            template <> struct ChannelOpsTypeSct<Color1f> { using type = Color1f; };
            template <> struct ChannelOpsTypeSct<Color1i> { using type = Color1i; };
            template <> struct ChannelOpsTypeSct<Color1u> { using type = Color1i; };
            template <typename T>
            using  ChannelOpsType = typename ChannelOpsTypeSct<ChannelType<T>>::type;


            template <typename T>
            struct Max {}; // THIS prevents from using channel types other than float,uchar,int
            template <> struct Max<Color1u> { static constexpr   int val = 255; };
            template <> struct Max<Color1i> { static constexpr   int val = 255; };
            template <> struct Max<Color1f> { static constexpr float val = 1.f; };
            // const float Max<Color1f>::val = MaxFloat;

            template <typename T>
            struct OtherChanTypeSct {}; // THIS prevents from using channel types other than float,uchar,int
            template <> struct OtherChanTypeSct<Color1f> { using type = Color1u; };
            template <> struct OtherChanTypeSct<Color1i> { using type = Color1f; };
            template <> struct OtherChanTypeSct<Color1u> { using type = Color1f; };
            template <typename T>
            struct OtherChanTypeSct<Vec2Base<T>> { using type = typename OtherChanTypeSct<T>::type; };
            template <typename T>
            struct OtherChanTypeSct<Vec3Base<T>> { using type = typename OtherChanTypeSct<T>::type; };
            template <typename T>
            struct OtherChanTypeSct<Vec4Base<T>> { using type = typename OtherChanTypeSct<T>::type; };
            template <typename T>
            using OtherChanType = typename OtherChanTypeSct<T>::type;

            template <typename T>
            struct OtherTypeSct              { using type =          OtherChanTypeSct<T> ; };
            template <typename T>
            struct OtherTypeSct<Vec2Base<T>> { using type = Vec2Base<OtherChanTypeSct<T>>; };
            template <typename T>
            struct OtherTypeSct<Vec3Base<T>> { using type = Vec3Base<OtherChanTypeSct<T>>; };
            template <typename T>
            struct OtherTypeSct<Vec4Base<T>> { using type = Vec4Base<OtherChanTypeSct<T>>; };
            template <typename T>
            using OtherType = typename OtherTypeSct<T>::type;


            template <bool FromFlt, bool ToFlt>
            struct ConvertSct { static constexpr int coef = 1; };
            template <>
            struct ConvertSct<true, false> { static constexpr   int coef = 255; };
            template <>
            struct ConvertSct<false, true> { static constexpr float coef = 1.f / 255; };
            // const float ConvertSct<false, true>::coef;
            // coefficient for color conversion (uchar-based <-> float-based)
            // takes two <ops>/<color> types, finds <base ops>/<base> types
            // (int, uchar, float) and checks using is_floating_point
            template <typename T1, typename T2>
            using Convert = ConvertSct<std::is_floating_point<ChannelType<T1>>::value,
                                       std::is_floating_point<ChannelType<T2>>::value>;


            template <typename T, int Ch>
            struct ColorTypeSct{}; // THIS restricts the number of channels to [0,4]
            template <typename T>
            struct ColorTypeSct<T, 0> { using type = T; };
            template <typename T>
            struct ColorTypeSct<T, 1> { using type = T; };
            template <typename T>
            struct ColorTypeSct<T, 2> { using type = Vec2Base<T>; };
            template <typename T>
            struct ColorTypeSct<T, 3> { using type = Vec3Base<T>; };
            template <typename T>
            struct ColorTypeSct<T, 4> { using type = Vec4Base<T>; };
            template <typename T, int Ch>
            using ColorType = typename ColorTypeSct<T, Ch>::type;

            // TODO: move to compression methods (when they'll be somewhere)

            // Color1f clamp(Color1f _v) {
            //     // if (_v <= 0.f || std::isnan(_v)) return 0.f; // from <cmath> - removed for debugging
            //     if (_v <= 0.f) return 0.f;
            //     if (_v >= 1.f) return 1.f;
            //     return _v;
            // }
            // inline Color1u clamp(Color1i _v) {
            //     // if (_v <=   0) return   0;
            //     // if (_v >= 255) return 255;
            //     return _v;
            // }
            // inline Color1u clamp(Color1u _v) {
            //     return _v;
            // }
        }; // Util END


        // Utility functions

        namespace Func {
            template <typename T>
            inline Vec4Base<T> addAlpha(const Vec4Base<T> &_c) { return _c; }
            template <typename T>
            inline Vec4Base<T> addAlpha(const Vec3Base<T> &_c, T _a = Util::Max<T>::val) { return { _c, _a }; }

            template <typename T>
            inline Vec3Base<T> cutAlpha(const Vec3Base<T> &_c) { return _c; }
            template <typename T>
            inline Vec3Base<T> cutAlpha(const Vec4Base<T> &_c) { return _c["xyz"]; }

            template <typename T1, typename T2 = Util::OtherType<T1>>
            inline T2 convert(const T1 &_v) { return _v * Util::Convert<T1, T2>::coef; }
            template <typename T1, typename T2>
            inline T2 convert(const typename std::enable_if<Util::Channels<T1>::value == 2 &&
                                                            Util::Channels<T2>::value == 2, T1>::type &_v) { return { (Util::ChannelType<T2>) (_v.x * Util::Convert<T1, T2>::coef),
                                                                                                                      (Util::ChannelType<T2>) (_v.y * Util::Convert<T1, T2>::coef) }; }
            template <typename T1, typename T2>
            inline T2 convert(const typename std::enable_if<Util::Channels<T1>::value == 3 &&
                                                            Util::Channels<T2>::value == 3, T1>::type &_v) { return { (Util::ChannelType<T2>) (_v.x * Util::Convert<T1, T2>::coef),
                                                                                                                      (Util::ChannelType<T2>) (_v.y * Util::Convert<T1, T2>::coef),
                                                                                                                      (Util::ChannelType<T2>) (_v.z * Util::Convert<T1, T2>::coef)}; }
            template <typename T1, typename T2>
            inline T2 convert(const typename std::enable_if<Util::Channels<T1>::value == 4 &&
                                                            Util::Channels<T2>::value == 4, T1>::type &_v) { return { (Util::ChannelType<T2>) (_v.x * Util::Convert<T1, T2>::coef),
                                                                                                                      (Util::ChannelType<T2>) (_v.y * Util::Convert<T1, T2>::coef),
                                                                                                                      (Util::ChannelType<T2>) (_v.z * Util::Convert<T1, T2>::coef),
                                                                                                                      (Util::ChannelType<T2>) (_v.w * Util::Convert<T1, T2>::coef)}; }
            template <typename T1, typename T2>
            inline T2 convert(const typename std::enable_if<Util::Channels<T1>::value == 3 &&
                                                            Util::Channels<T2>::value == 4, T1>::type &_v) { return addAlpha(_v) * Util::Convert<T1, T2>::coef; }
            template <typename T1, typename T2>
            inline T2 convert(const typename std::enable_if<Util::Channels<T1>::value == 4 &&
                                                            Util::Channels<T2>::value == 3, T1>::type &_v) { return cutAlpha(_v) * Util::Convert<T1, T2>::coef; }
        }; // Func END



//  ========================================  \\
                Color operations
//  ========================================  \\

//  ============  Porter-Duff blends 4-4, 4-3, 3-4  ============  \\

        template <typename T>
        Vec4Base<T> PorterDuffBlendOver(const Vec4Base<T> &_c1, const Vec4Base<T> &_c2)
            { return (Util::Max<T>::val - _c1.w) * Util::Convert<T, float>::coef * _c2 + _c1; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendIn  (const Vec4Base<T> &_c1, const Vec4Base<T> &_c2)
            { return                      _c2.w  * Util::Convert<T, float>::coef * _c1; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendOut (const Vec4Base<T> &_c1, const Vec4Base<T> &_c2)
            { return (Util::Max<T>::val - _c2.w) * Util::Convert<T, float>::coef * _c1; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendAtop(const Vec4Base<T> &_c1, const Vec4Base<T> &_c2)
            { return                      _c2.w  * Util::Convert<T, float>::coef * _c1 +
                     (Util::Max<T>::val - _c1.w) * Util::Convert<T, float>::coef * _c2; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendXor (const Vec4Base<T> &_c1, const Vec4Base<T> &_c2)
            { return (Util::Max<T>::val - _c2.w) * Util::Convert<T, float>::coef * _c1 +
                     (Util::Max<T>::val - _c1.w) * Util::Convert<T, float>::coef * _c2; }

// Do I even need these?
        template <typename T>
        Vec4Base<T> PorterDuffBlendOver(const Vec4Base<T> &_c1, const Vec3Base<T> &_c2)
            { return {_c1.x + (Util::Max<T>::val - _c1.w) * Util::Convert<T, float>::coef * _c2.x,
                      _c1.y + (Util::Max<T>::val - _c1.w) * Util::Convert<T, float>::coef * _c2.y,
                      _c1.z + (Util::Max<T>::val - _c1.w) * Util::Convert<T, float>::coef * _c2.z,
                      Util::Max<T>::val}; }
        template <typename T> inline
        Vec4Base<T> PorterDuffBlendIn  (const Vec4Base<T> &_c1, const Vec3Base<T> &_c2)
            { return _c1; }
        template <typename T> inline
        Vec4Base<T> PorterDuffBlendOut (const Vec4Base<T> &_c1, const Vec3Base<T> &_c2)
            { return {}; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendAtop(const Vec4Base<T> &_c1, const Vec3Base<T> &_c2)
            { return {_c1.x + (Util::Max<T>::val - _c1.w) * Util::Convert<T, float>::coef * _c2.x,
                      _c1.y + (Util::Max<T>::val - _c1.w) * Util::Convert<T, float>::coef * _c2.y,
                      _c1.z + (Util::Max<T>::val - _c1.w) * Util::Convert<T, float>::coef * _c2.z,
                      Util::Max<T>::val}; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendXor (const Vec4Base<T> &_c1, const Vec3Base<T> &_c2)
            { return {(Util::Max<T>::val - _c1.w) * Util::Convert<T, float>::coef * _c2.x,
                      (Util::Max<T>::val - _c1.w) * Util::Convert<T, float>::coef * _c2.y,
                      (Util::Max<T>::val - _c1.w) * Util::Convert<T, float>::coef * _c2.z,
                       Util::Max<T>::val - _c1.w}; }

        template <typename T> inline
        Vec4Base<T> PorterDuffBlendOver(const Vec3Base<T> &_c1, const Vec4Base<T> &_c2)
            { return _c1; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendIn  (const Vec3Base<T> &_c1, const Vec4Base<T> &_c2)
            { return {_c2.w * Util::Convert<T, float>::coef * _c1.x,
                      _c2.w * Util::Convert<T, float>::coef * _c1.y,
                      _c2.w * Util::Convert<T, float>::coef * _c1.z,
                      _c2.w}; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendOut (const Vec3Base<T> &_c1, const Vec4Base<T> &_c2)
            { return {(Util::Max<T>::val - _c2.w) * Util::Convert<T, float>::coef * _c1.x,
                      (Util::Max<T>::val - _c2.w) * Util::Convert<T, float>::coef * _c1.y,
                      (Util::Max<T>::val - _c2.w) * Util::Convert<T, float>::coef * _c1.z,
                       Util::Max<T>::val - _c2.w}; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendAtop(const Vec3Base<T> &_c1, const Vec4Base<T> &_c2)
            { return {_c2.w * Util::Convert<T, float>::coef * _c1.x,
                      _c2.w * Util::Convert<T, float>::coef * _c1.y,
                      _c2.w * Util::Convert<T, float>::coef * _c1.z,
                      _c2.w}; }
        template <typename T>
        Vec4Base<T> PorterDuffBlendXor (const Vec3Base<T> &_c1, const Vec4Base<T> &_c2)
            { return {(Util::Max<T>::val - _c2.w) * Util::Convert<T, float>::coef * _c1.x,
                      (Util::Max<T>::val - _c2.w) * Util::Convert<T, float>::coef * _c1.y,
                      (Util::Max<T>::val - _c2.w) * Util::Convert<T, float>::coef * _c1.z,
                       Util::Max<T>::val - _c2.w}; }
    }; // Color END
}; // Simple END

#endif