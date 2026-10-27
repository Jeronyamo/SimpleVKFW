#include "main/colormodel.h"

namespace Simple {
    namespace Color {

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
        const mat3 &D50::convMat<RGB, XYZ>() { return D50::RGB2XYZ; }

        template <>
        const mat3 &D50::convMat<XYZ, RGB>() { return D50::XYZ2RGB; }

        template <>
        const mat3 &D50::convMat<sRGB, XYZ>() { return D50::sRGB2XYZ; }

        template <>
        const mat3 &D50::convMat<XYZ, sRGB>() { return D50::XYZ2sRGB; }

        template <>
        const mat3 &D50::convMat<WideRGB, XYZ>() { return D50::WideRGB2XYZ; }

        template <>
        const mat3 &D50::convMat<XYZ, WideRGB>() { return D50::XYZ2WideRGB; }

        const mat3 D65::sRGB2XYZ { 0.4124564f, 0.3575761f, 0.1804375f,
                                           0.2126729f, 0.7151522f, 0.0721750f,
                                           0.0193339f, 0.1191920f, 0.9503041f };

        const mat3 D65::XYZ2sRGB { 3.2404542f,-1.5371385f,-0.4985314f,
                                          -0.9692660f, 1.8760108f, 0.0415560f,
                                           0.0556434f,-0.2040259f, 1.0572252f };

        template <>
        const mat3 &D65::convMat<sRGB, XYZ>() { return D65::sRGB2XYZ; }

        template <>
        const mat3 &D65::convMat<XYZ, sRGB>() { return D65::XYZ2sRGB; }

        const mat3 E::RGB2XYZ { 0.4887180f, 0.3106803f, 0.2006017f,
                                        0.1762044f, 0.8129847f, 0.0108109f,
                                        0.f,        0.0102048f, 0.9897952f };

        const mat3 E::XYZ2RGB { 2.3706743f,-0.9000405f,-0.4706338f,
                                       -0.5138850f, 1.4253036f, 0.0885814f,
                                        0.0052982f,-0.0146949f, 1.0093968f };

        template <>
        const mat3 &E::convMat<RGB, XYZ>() { return E::RGB2XYZ; }

        template <>
        const mat3 &E::convMat<XYZ, RGB>() { return E::XYZ2RGB; }

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

            // Note: dominant channel for 60-degree sectors 0..5 is R,G,G,B,B,R
            res[((angle+1) >> 1)%3] =  _col_in.y * _col_in.z;
            res[     (7-angle)%3] = (_col_in.y * _col_in.z) * (1 - std::abs((_col_in.x * 3 - std::trunc(_col_in.x * 3)) * 2 - 1));
            res += _col_in.z * (1.f - _col_in.y);

            // printf("Cast HSV -> RGB: [%f, %f, %f] -> [%f, %f, %f]\n", _col_in.x * 360, _col_in.y * 100, _col_in.z * 100, res.x * 255, res.y * 255, res.z * 255);
            return res;
        }
    }; // Color END
}; // Simple END
