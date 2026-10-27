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


        template <>
        const mat3 &D50::convMat<RGB, XYZ>();
        template <>
        const mat3 &D50::convMat<XYZ, RGB>();

        template <>
        const mat3 &D50::convMat<sRGB, XYZ>();
        template <>
        const mat3 &D50::convMat<XYZ, sRGB>();

        template <>
        const mat3 &D50::convMat<WideRGB, XYZ>();
        template <>
        const mat3 &D50::convMat<XYZ, WideRGB>();


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


        template <>
        const mat3 &D65::convMat<sRGB, XYZ>();
        template <>
        const mat3 &D65::convMat<XYZ, sRGB>();


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


        template <>
        const mat3 &E::convMat<RGB, XYZ>();
        template <>
        const mat3 &E::convMat<XYZ, RGB>();


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
                return { 0.95047f * _c.x, _c.y, 1.08883f * _c.z };
            }
            template <class T>
            static Vec4Base<T> inline Convert(const Vec4Base<T> &_c) {
                return { 0.95047f * _c.x, _c.y, 1.08883f * _c.z, _c.w };

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
        col3f convert3f<Color::RGB, Color::HSV>(const col3f &_col_in);

        template <>
        col3f convert3f<Color::HSV, Color::RGB>(const col3f &_col_in);
    };
};

#endif