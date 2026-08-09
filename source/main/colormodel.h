#ifndef SVKFW_COLORMODEL_H
#define SVKFW_COLORMODEL_H

#include "math/vectors.h"
#include "main/color.h"


namespace Simple {
//  ====================================  \\
                Color models
//  ====================================  \\

//  ============  Utilities and reference white namespaces  ============  \\

    namespace Color {
// Supported color models
        enum ColorM { RGB, sRGB, WideRGB, XYZ, HSV/*, ITP, LMS, LAB*/ };



// D50
        struct D50 {
            static const mat3 RGB2XYZ;
            static const mat3 XYZ2RGB;

            static const mat3 sRGB2XYZ;
            static const mat3 XYZ2sRGB;

            static const mat3 WideRGB2XYZ;
            static const mat3 XYZ2WideRGB;

            template <ColorM _From, ColorM _To>
            static inline const mat3 &convMat() {
                if (_From != _To)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Color :: D50 :: convMat", "White conversion not implemented\n"));
                return Mat::Eye3;
            }
        };

        const mat3 D50::RGB2XYZ { 0.4868870f, 0.3062984f, 0.1710347f,
                                  0.1746583f, 0.8247541f, 0.0005877f,
                                 -0.0012563f, 0.0169832f, 0.8094831f };
        const mat3 D50::XYZ2RGB { 2.3638081f,-0.8676030f,-0.4988161f,
                                 -0.5005940f, 1.3962369f, 0.1047562f,
                                  0.0141712f,-0.0306400f, 1.2323842f };

        const mat3 D50::sRGB2XYZ { 0.4360747f, 0.3850649f, 0.1430804f,
                                   0.2225045f, 0.7168786f, 0.0606169f,
                                   0.0139322f, 0.0971045f, 0.7141733f };
        const mat3 D50::XYZ2sRGB { 3.1338561f,-1.6168667f,-0.4906146f,
                                  -0.9787684f, 1.9161415f, 0.0334540f,
                                   0.0719453f,-0.2289914f, 1.4052427f };

        const mat3 D50::WideRGB2XYZ { 0.7161046f, 0.1009296f, 0.1471858f,
                                      0.2581874f, 0.7249378f, 0.0168748f,
                                      0.f,        0.0517813f, 0.7734287f };
        const mat3 D50::XYZ2WideRGB { 1.4628067f,-0.1840623f,-0.2743606f,
                                     -0.5217933f, 1.4472381f, 0.0677227f,
                                      0.0349342f,-0.0968930f, 1.2884099f };

        template <>
        inline const mat3 &D50::convMat<RGB, XYZ>() { return D50::RGB2XYZ; }
        template <>
        inline const mat3 &D50::convMat<XYZ, RGB>() { return D50::XYZ2RGB; }

        template <>
        inline const mat3 &D50::convMat<sRGB, XYZ>() { return D50::sRGB2XYZ; }
        template <>
        inline const mat3 &D50::convMat<XYZ, sRGB>() { return D50::XYZ2sRGB; }

        template <>
        inline const mat3 &D50::convMat<WideRGB, XYZ>() { return D50::WideRGB2XYZ; }
        template <>
        inline const mat3 &D50::convMat<XYZ, WideRGB>() { return D50::XYZ2WideRGB; }


// D65
        struct D65 {
            static const mat3 sRGB2XYZ;
            static const mat3 XYZ2sRGB;

            template <ColorM _From, ColorM _To>
            static inline const mat3 &convMat() {
                if (_From != _To)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Color :: D65 :: convMat", "White conversion not implemented\n"));
                return Mat::Eye3;
            }
        };
        const mat3 D65::sRGB2XYZ { 0.4124564f, 0.3575761f, 0.1804375f,
                                   0.2126729f, 0.7151522f, 0.0721750f,
                                   0.0193339f, 0.1191920f, 0.9503041f };
        const mat3 D65::XYZ2sRGB { 3.2404542f,-1.5371385f,-0.4985314f,
                                  -0.9692660f, 1.8760108f, 0.0415560f,
                                   0.0556434f,-0.2040259f, 1.0572252f };

        template <>
        inline const mat3 &D65::convMat<sRGB, XYZ>() { return D65::sRGB2XYZ; }
        template <>
        inline const mat3 &D65::convMat<XYZ, sRGB>() { return D65::XYZ2sRGB; }


// E
        struct E {
            static const mat3 RGB2XYZ; 
            static const mat3 XYZ2RGB;

            template <ColorM _From, ColorM _To>
            static inline const mat3 &convMat() {
                if (_From != _To)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Color :: E :: convMat", "White conversion not implemented\n"));
                return Mat::Eye3;
            }
        };
        const mat3 E::RGB2XYZ { 0.4887180f, 0.3106803f, 0.2006017f,
                                0.1762044f, 0.8129847f, 0.0108109f,
                                0.f,        0.0102048f, 0.9897952f };
        const mat3 E::XYZ2RGB { 2.3706743f,-0.9000405f,-0.4706338f,
                               -0.5138850f, 1.4253036f, 0.0885814f,
                                0.0052982f,-0.0146949f, 1.0093968f };

        template <>
        inline const mat3 &E::convMat<RGB, XYZ>() { return E::RGB2XYZ; }
        template <>
        inline const mat3 &E::convMat<XYZ, RGB>() { return E::XYZ2RGB; }


//  ============  Color white transforms  ============  \\

        template <class _From, class _To>
        struct White {
            template <class T>
            static Vec3Base<T> Convert(const Vec3Base<T> &_c) {
                if (!std::is_same<_From, _To>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Color :: ConvertWhite", "Color3 white conversion - not implemented"));
                return _c;
            }
            template <class T>
            static Vec4Base<T> Convert(const Vec4Base<T> &_c) {
                if (!std::is_same<_From, _To>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Color :: ConvertWhite", "Color4 white conversion - not implemented"));
                return _c;
            }
        }; // White END

// Implementations
// D50
        template <>
        struct White<D50, D65> {
            template <class T>
            static Vec3Base<T> inline Convert(const Vec3Base<T> &_c) {
                return { 0.9857398f * _c.x, _c.y, 1.3194581f * _c.z };
            }
            template <class T>
            static Vec4Base<T> inline Convert(const Vec4Base<T> &_c) {
                return { 0.9857398f * _c.x, _c.y, 1.3194581f * _c.z, _c.w };
            }
        };

        template <>
        struct White<D50, E> {
            template <class T>
            static Vec3Base<T> inline Convert(const Vec3Base<T> &_c) {
                return { 1.0371077f * _c.x, _c.y, 1.2118128f * _c.z };
            }
            template <class T>
            static Vec4Base<T> inline Convert(const Vec4Base<T> &_c) {
                return { 1.0371077f * _c.x, _c.y, 1.2118128f * _c.z, _c.w };
            }
        };

// D65
        template <>
        struct White<D65, D50> {
            template <class T>
            static Vec3Base<T> inline Convert(const Vec3Base<T> &_c) {
                return { 1.0144665f * _c.x, _c.y, 0.7578869f * _c.z };
            }
            template <class T>
            static Vec4Base<T> inline Convert(const Vec4Base<T> &_c) {
                return { 1.0144665f * _c.x, _c.y, 0.7578869f * _c.z, _c.w };
            }
        };

        template <>
        struct White<D65, E> {
            template <class T>
            static Vec3Base<T> inline Convert(const Vec3Base<T> &_c) {
                return { 1.0521111f * _c.x, _c.y, 0.918417f * _c.z };
            }
            template <class T>
            static Vec4Base<T> inline Convert(const Vec4Base<T> &_c) {
                return { 1.0521111f * _c.x, _c.y, 0.918417f * _c.z, _c.w };
            }
        };

// E
        template <>
        struct White<E, D50> {
            template <class T>
            static Vec3Base<T> inline Convert(const Vec3Base<T> &_c) {
                return { 0.96422f * _c.x, _c.y, 0.82521f * _c.z };
            }
            template <class T>
            static Vec4Base<T> inline Convert(const Vec4Base<T> &_c) {
                return { 0.96422f * _c.x, _c.y, 0.82521f * _c.z, _c.w };
            }
        };

        template <>
        struct White<E, D65> {
            template <class T>
            static Vec3Base<T> inline Convert(const Vec3Base<T> &_c) {
                return { 0.82521f * _c.x, _c.y, 1.08883f * _c.z };
            }
            template <class T>
            static Vec4Base<T> inline Convert(const Vec4Base<T> &_c) {
                return { 0.82521f * _c.x, _c.y, 1.08883f * _c.z, _c.w };
            }
        };


//  ============  Color model transforms  ============  \\

        template <ColorM _from, ColorM _to>
        col3f convert3f(const col3f &_col_in) {
            if (_from != _to)
                fprintf(svkfwwarn, SVKFW_WRAPWARN("Color :: convert", "Color model conversion <%d, %d> is not implemented\n"), _from, _to);

            return _col_in;
        }
        template <ColorM _from, ColorM _to, class T>
        inline Vec3Base<T> convert(const Vec3Base<T> &_col_in) {
            return (convert3f<_from, _to>((_col_in * Util::ConvertCoef<T, col1f>::coef).template cast<col1f>())
                        * Util::ConvertCoef<col1f, T>::coef).template cast<T>();
        }
        template <ColorM _from, ColorM _to, class T>
        inline Vec4Base<T> convert(const Vec4Base<T> &_col_in) {
            return { convert<_from, _to>(_col_in["xyz"]), _col_in.w };
        }

        template <>
        col3f convert3f<Color::RGB, Color::HSV>(const col3f &_col_in) {
            col3f    res{};
            uint32_t amax = Math::amaxOf(_col_in);
            res.z = _col_in[amax]; // V
            res.x = res.z - _col_in[Math::aminOf(_col_in)];

            if (res.z > SVKFW_EPS4)
                res.y = res.x / res.z; // S

            if (res.x > SVKFW_EPS4) {
                res.x =  (_col_in[(amax+1)%3] - _col_in[(amax+2)%3]) / res.x + (amax << 1u);
                if (res.x <  0.f) res.x += 6;
                if (res.x >= 6.f) res.x -= 6;
                res.x /= 6;
            }
            // printf("Cast RGB -> HSV: [%f, %f, %f] -> [%f, %f, %f]\n", _col_in.x * 255, _col_in.y * 255, _col_in.z * 255, res.x * 360, res.y * 100, res.z * 100);
            return res;
        }

        template <>
        col3f convert3f<Color::HSV, Color::RGB>(const col3f &_col_in) {
            col3f res{};
            uint32_t angle = _col_in.x * 6;

            res[(angle+1 >> 2)%3] =  _col_in.y * _col_in.z;
            res[     (7-angle)%3] = (_col_in.y * _col_in.z) * (1 - std::abs((_col_in.x * 3 - std::trunc(_col_in.x * 3)) * 2 - 1));
            res += _col_in.z * (1.f - _col_in.y);

            // printf("Cast HSV -> RGB: [%f, %f, %f] -> [%f, %f, %f]\n", _col_in.x * 360, _col_in.y * 100, _col_in.z * 100, res.x * 255, res.y * 255, res.z * 255);
            return res;
        }
    };
};

#endif