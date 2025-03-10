#ifndef SOGLFW_COLOR_H
#define SOGLFW_COLOR_H

#include "../math/vectors.h"

#define COLORS_FTOU 255u
#define COLORS_UTOF 0.003921568859f     // 1.f / 255


namespace Simple {

//  ============  Utilities  ============  \\

    namespace UnHood {
        float clampCol(float val) { return val < 0.f ? 0.f : (val > 1.f ? 1.f : val); }
        uchar clampCol(int   val) { return val < 0   ? 0   : (val > 255 ? 255 : val); }
        uchar clampCol(uint  val) { return val > 255 ? 255 :  val; }
        uchar clampCol(uchar val) { return val; }


        template <typename T>
        struct ColComplementType {
            typedef T type;
        };
        template<> struct ColComplementType<float> {
            typedef uchar type;
        };
        template<> struct ColComplementType<uchar> {
            typedef float type;
        };

        template <typename T>
        struct ColMax {
            static constexpr T max = std::is_floating_point<T>::value ? 1.f : 255u;
        };

        template <typename T1, typename T2>
        struct ColTransform {
            static constexpr T2 coef = std::is_floating_point<T2>::value ? COLORS_UTOF : COLORS_FTOU;
        };
        template <typename T>
        struct ColTransform<T, T> {
            static constexpr T coef = 1;
        };

        template <typename T>
        struct ColTransformTo {
            static constexpr T coef = std::is_floating_point<T>::value ? COLORS_UTOF : COLORS_FTOU;
        };

        template <typename T>
        struct ColTransformFrom {
            static constexpr typename UnHood::ColComplementType<T>::type coef = std::is_floating_point<T>::value ? COLORS_FTOU : COLORS_UTOF;
        };
    };

    namespace ColModel {
        enum Type { RGB, WideRGB, sRGB, XYZ, HSV, LAB, LMS, ICtCp };
    };


//  ============  Color classes declaration  ============  \\

    namespace Colors {
        template <typename T> struct ColorBase;
        template <typename T> struct ColorABase;

        template <typename T>
        struct ColChannels {
            static const int channels = 1;
        };
        template <typename T>
        struct ColChannels<ColorBase<T>> {
            static const int channels = 3;
        };
        template <typename T>
        struct ColChannels<ColorABase<T>> {
            static const int channels = 4;
        };


//  ============  Color model transform matrices  ============  \\

        float ColorOETFpq(float _val) {
            //_val *= 0.0001f;
            _val = pow(_val, 1305.f / 8192);
            return pow(((0.8359375f + 18.8515625f * _val) / (1 + 18.6875f * _val)), 78.84375f);
        }

        float ColorOETFhlg(float _val) {
            return _val <= 1.f / 12 ? sqrt(3 * _val) :
                0.17883277f * log(12 * _val - 0.28466892f) + 0.55991073f;
        }

        float (*ColorOETF[2])(float) = {&ColorOETFpq, &ColorOETFhlg};

        const Mat3 LMS2ITP[2] {
            { 0.5f,         0.5f,         0.f,
              1.613769531f,-3.323486328f, 1.709716797f,
              4.378173828f,-4.245605469f,-0.1325683594f },
            { 0.5f,          0.5f,         0.f,
              0.8850097656f,-1.822509766f, 0.9375f,
              2.319335938f, -2.249023438f,-0.0703125f}
        };

        //E
        const Mat3 RGB2XYZ_E { 0.4887180f, 0.3106803f, 0.2006017f,
                               0.1762044f, 0.8129847f, 0.0108109f,
                               0.f,        0.0102048f, 0.9897952f };
        //E
        const Mat3 XYZ2RGB_E { 2.3706743f,-0.9000405f,-0.4706338f,
                              -0.5138850f, 1.4253036f, 0.0885814f,
                               0.0052982f,-0.0146949f, 1.0093968f };

        //D65
        const Mat3 sRGB2XYZ_D65 { 0.4124564f, 0.3575761f, 0.1804375f,
                                  0.2126729f, 0.7151522f, 0.0721750f,
                                  0.0193339f, 0.1191920f, 0.9503041f };
        //D65
        const Mat3 XYZ2sRGB_D65 { 3.2404542f,-1.5371385f,-0.4985314f,
                                 -0.9692660f, 1.8760108f, 0.0415560f,
                                  0.0556434f,-0.2040259f, 1.0572252f };

        //D50
        const Mat3 RGB2XYZ { 0.4868870f, 0.3062984f, 0.1710347f,
                             0.1746583f, 0.8247541f, 0.0005877f,
                            -0.0012563f, 0.0169832f, 0.8094831f };
        //D50
        const Mat3 XYZ2RGB { 2.3638081f,-0.8676030f,-0.4988161f,
                            -0.5005940f, 1.3962369f, 0.1047562f,
                             0.0141712f,-0.0306400f, 1.2323842f };

        //D50
        const Mat3 sRGB2XYZ { 0.4360747f, 0.3850649f, 0.1430804f,
                              0.2225045f, 0.7168786f, 0.0606169f,
                              0.0139322f, 0.0971045f, 0.7141733f };
        //D50
        const Mat3 XYZ2sRGB { 3.1338561f,-1.6168667f,-0.4906146f,
                             -0.9787684f, 1.9161415f, 0.0334540f,
                              0.0719453f,-0.2289914f, 1.4052427f };

        //D50
        const Mat3 WideRGB2XYZ { 0.7161046f, 0.1009296f, 0.1471858f,
                                 0.2581874f, 0.7249378f, 0.0168748f,
                                 0.f,        0.0517813f, 0.7734287f };
        //D50
        const Mat3 XYZ2WideRGB { 1.4628067f,-0.1840623f,-0.2743606f,
                                -0.5217933f, 1.4472381f, 0.0677227f,
                                 0.0349342f,-0.0968930f, 1.2884099f };

        //CIECAM02 chromatic adaptation transform matrix
        const Mat3 XYZ2LMS { 0.7328f, 0.4296f,-0.1624f,
                            -0.7036f, 1.6975f, 0.0061f,
                             0.0030f, 0.0136f, 0.9834f };

        const Mat3 LMS2XYZ {XYZ2LMS.inverse()};


//  ============  Color base classes  ============  \\

        template <typename T>
        struct ColorBase {
            union {
                struct {T r, g, b;};
                struct {T h, s, v;};
                struct {T x, y, z;};
                struct {T i, Ct, Cp;};
                struct {T lw, mw, sw;};
                struct {T L_, a_, b_;};
            };

            ColorBase() : r{0}, g{0}, b{0} {}
            ColorBase(T _val) :
                    r{UnHood::clampCol(_val  )}, g{UnHood::clampCol(_val  )}, b{UnHood::clampCol(_val  )} {}
            ColorBase(T _v1, T _v2, T _v3) :
                    r{UnHood::clampCol(_v1   )}, g{UnHood::clampCol(_v2   )}, b{UnHood::clampCol(_v3   )} {}
            ColorBase(const ColorBase<T> &_val) :
                    r{UnHood::clampCol(_val.r)}, g{UnHood::clampCol(_val.g)}, b{UnHood::clampCol(_val.b)} {}
            ColorBase(const ColorBase <typename UnHood::ColComplementType<T>::type> &_val) :
                    r{UnHood::clampCol(_val.r) * UnHood::ColTransformTo<T>::coef},
                    g{UnHood::clampCol(_val.g) * UnHood::ColTransformTo<T>::coef},
                    b{UnHood::clampCol(_val.b) * UnHood::ColTransformTo<T>::coef} {}
            ColorBase(const ColorABase<T> &_val) :
                    r{UnHood::clampCol(_val.r)}, g{UnHood::clampCol(_val.g)}, b{UnHood::clampCol(_val.b)} {}
            ColorBase(const ColorABase<typename UnHood::ColComplementType<T>::type> &_val) :
                    r{UnHood::clampCol(_val.r) * UnHood::ColTransformTo<T>::coef},
                    g{UnHood::clampCol(_val.g) * UnHood::ColTransformTo<T>::coef},
                    b{UnHood::clampCol(_val.b) * UnHood::ColTransformTo<T>::coef} {}
            template <typename Y>
            ColorBase(const Vec3Base<Y> _vec) : r{UnHood::clampCol(T(_vec.x))}, g{UnHood::clampCol(T(_vec.y))}, b{UnHood::clampCol(T(_vec.z))} {}


            template <typename Y> ColorBase<T> operator=(const Vec3Base<Y> &_vec)
                { r = UnHood::clampCol(T(_vec.x)); g = UnHood::clampCol(T(_vec.y)); b = UnHood::clampCol(T(_vec.z)); return *this; }

            operator Vec3Base<T>() const { return {r, g, b}; }
            Vec3Base<T>   toVec3() const { return {r, g, b}; }

            T       &operator[](int ind)       { return *((T*)this + ind); }
            const T &operator[](int ind) const { return *((T*)this + ind); }


            ColorBase<T> RGBtoHSV() const {
                ColorBase<T> col{*this};

                unsigned index = r < g ? 0u : 1u;
                col.s = b < (*this)[index] ? b : (*this)[index]; // C Min
                index = b < (*this)[1u - index] ? 1u - index : 2u;
                col.v = (*this)[index];                          // V == C Max
                float delta = col.v - col.s;
                if (col.v > 1e-4f) col.s = UnHood::ColMax<T>::max * (1.f - float(col.s) / col.v); // S
                if (delta > 1e-4f) {
                    unsigned ind1 = (index + 1u) % 3u, ind2 = (index + 2u) % 3u;
                    index = index || g >= b ? index : 4u;
                    col.h = UnHood::ColMax<T>::max * 0.1666666f * (((*this)[ind1] - (*this)[ind2]) / delta + (index << 1u)); // H
                }
                return col;
            }
            void rgb2hsv() {
                ColorBase<T> col{*this};

                unsigned index = col.r < col.g ? 0u : 1u;
                s = col.b < col[index] ? col.b: col[index]; // C Min
                index = col.b < col[1u - index] ? 1u - index : 2u;
                v = col[index];                             // V == C Max
                float delta = v - s;
                if (v > 1e-4f) s = UnHood::ColMax<T>::max * (1.f - float(s) / v); // S
                if (delta > 1e-4f) {
                    unsigned ind1 = (index + 1u) % 3u, ind2 = (index + 2u) % 3u;
                    index = index || col.g >= col.b ? index : 4u;
                    h = UnHood::ColMax<T>::max * 0.1666666f * ((col[ind1] - col[ind2]) / delta + (index << 1u)); // H
                }
            }

            ColorBase<T> HSVtoRGB() const {
                ColorBase<T> col{v};

                float to1 = UnHood::ColTransform<T, float>::coef;
                float Vmin = (UnHood::ColMax<T>::max - s) * to1 * v, H_i;
                float a = (v - Vmin) * std::modf(h * to1 * 6.f, &H_i);
                float Vdec = v - a, Vinc = Vmin + a;
                unsigned Hi = unsigned(H_i);
                Hi = Hi <= 5u ? Hi : 5u;

                col[(2u + (Hi >> 1)) % 3u] = Vmin;
                if (Hi & 1u) col[Hi >> 1] = Vdec;
                else col[(1u + (Hi >> 1)) % 3u] = Vinc;

                return col;
            }
            void hsv2rgb() {
                ColorBase<T> col{*this};

                float to1 = UnHood::ColTransform<T, float>::coef;
                float Vmin = (UnHood::ColMax<T>::max - col.s) * to1 * col.v, H_i;
                float a = (col.v - Vmin) * std::modf(col.h * to1 * 6.f, &H_i);
                float Vdec = col.v - a, Vinc = Vmin + a;
                unsigned Hi = unsigned(H_i);
                Hi = Hi <= 5u ? Hi : 5u;

                (*this)[(2u + (Hi >> 1)) % 3u] = Vmin;
                if (Hi & 1u) (*this)[Hi >> 1] = Vdec;
                else (*this)[(1u + (Hi >> 1)) % 3u] = Vinc;
            }


            ColorBase<T> LABtoXYZ() const {
                ColorBase<float> ref{*this};
                const float e = 216.f / 24389, k = 27.f / 24389;
                Vec3 f;
                f.y = (ref.L_ + 16) / 116;
                f.x = ref.a_ / 500 + f.y;
                f.z = f.y - ref.b_ / 200;
                Vec3 f3(f * f * f);
                //D50
                ref.L_ *= k;
                ref.y = (ref.L_ * k > e ? f3.y : ref.L_ * k);
                ref.x = (f3.x > e ? f3.x : (116 * f.x - 16) * k) * 0.96422f;
                ref.z = (f3.z > e ? f3.z : (116 * f.z - 16) * k) * 0.82521f;
                return ref;
            }
            void lab2xyz() {
                ColorBase<float> ref{*this};
                const float e = 216.f / 24389, k = 27.f / 24389;
                Vec3 f;
                f.y = (ref.L_ + 16) / 116;
                f.x =  ref.a_ / 500 + f.y;
                f.z =  f.y - ref.b_ / 200;
                Vec3 f3(f * f * f);
                //D50
                this->x = (f3.x > e ? f3.x : (116 * f.x - 16) * k) * 0.96422f * UnHood::ColMax<T>::max;
                this->y = (ref.L_ * k > e ? f3.y : ref.L_ * k) * UnHood::ColMax<T>::max;
                this->z = (f3.z > e ? f3.z : (116 * f.z - 16) * k) * 0.82521f * UnHood::ColMax<T>::max;
            }

            ColorBase<T> XYZtoLAB() const {
                ColorBase<float> ref{*this};
                const float e = 216.f / 24389, k = 24389.f / 27;
                //D50
                ref.x *= 1.f / 0.96422f;
                ref.z *= 1.f / 0.82521f;

                Vec3 f{ref.x > e ? cbrt(ref.x) : (k * ref.x + 16) / 116,
                       ref.y > e ? cbrt(ref.y) : (k * ref.y + 16) / 116,
                       ref.z > e ? cbrt(ref.z) : (k * ref.z + 16) / 116};

                ref.L_ = 116 * f.y - 16;
                ref.a_ = 500 * (f.x - f.y);
                ref.b_ = 200 * (f.y - f.z);
                return ref;
            }
            void xyz2lab() {
                ColorBase<float> ref{*this};
                const float e = 216.f / 24389, k = 24389.f / 27;
                //D50
                ref.x *= 1.f / 0.96422f;
                ref.z *= 1.f / 0.82521f;

                Vec3 f{ref.x > e ? cbrt(ref.x) : (k * ref.x + 16) / 116,
                       ref.y > e ? cbrt(ref.y) : (k * ref.y + 16) / 116,
                       ref.z > e ? cbrt(ref.z) : (k * ref.z + 16) / 116};

                this->L_ = (116 * f.y - 16)  * UnHood::ColMax<T>::max;
                this->a_ = 500 * (f.x - f.y) * UnHood::ColMax<T>::max;
                this->b_ = 200 * (f.y - f.z) * UnHood::ColMax<T>::max;
            }


            ColorBase<T> LMStoITP(bool use_hlg = true) const {
                Vec3 tmp{lw, mw, sw};
                tmp = LMS2ITP[use_hlg] & Vec3(ColorOETF[use_hlg](tmp.x),
                                              ColorOETF[use_hlg](tmp.y),
                                              ColorOETF[use_hlg](tmp.z));
                tmp.y += 0.5f;
                tmp.z += 0.5f;
                return tmp * UnHood::ColMax<T>::max;
            }
            void lms2itp(bool use_hlg = true) {
                Vec3 tmp{lw, mw, sw};
                tmp = LMS2ITP[use_hlg] & Vec3(ColorOETF[use_hlg](tmp.x),
                                              ColorOETF[use_hlg](tmp.y),
                                              ColorOETF[use_hlg](tmp.z));
                i = tmp.x * UnHood::ColMax<T>::max;
                Ct = (tmp.y + 0.5f) * UnHood::ColMax<T>::max;
                Cp = (tmp.z + 0.5f) * UnHood::ColMax<T>::max;
            }
    //TODO: ITP 2 LMS
        };

        template <typename T>
        struct ColorABase : ColorBase<T> {
            T a;

            ColorABase() : ColorBase<T>(), a{0} {}
            ColorABase(T _val) : ColorBase<T>(_val), a{UnHood::clampCol(_val)} {}
            ColorABase(T _v1, T _v2, T _v3, T _v4) : ColorBase<T>(_v1, _v2, _v3), a{UnHood::clampCol(_v4)} {}
            template <typename Y1, typename Y2, typename Y3, typename Y4> ColorABase(Y1 _v1, Y2 _v2, Y3 _v3, Y4 _v4) :
                    ColorBase<T>(_v1, _v2, _v3), a{UnHood::clampCol(T(_v4))} {}
            ColorABase(const ColorBase<T> &_val, T _a = UnHood::ColMax<T>::max) :
                    ColorBase<T>(_val), a{UnHood::clampCol(_a)} {}
            ColorABase(const ColorBase<typename UnHood::ColComplementType<T>::type> &_val, T _a = UnHood::ColMax<T>::max) :
                    ColorBase<T>(_val), a{UnHood::clampCol(_a)} {}
            ColorABase(const ColorABase<T> &_val) :
                    ColorBase<T>(_val), a{UnHood::clampCol(_val.a)} {}
            ColorABase(const ColorABase<typename UnHood::ColComplementType<T>::type> &_val) :
                    ColorBase<T>(_val), a{UnHood::clampCol(_val.a) * UnHood::ColTransformTo<T>::coef} {}

            template <typename Y>
            ColorABase(const Vec4Base<Y> _vec) : ColorBase<T>(_vec.x, _vec.y, _vec.z), a{UnHood::clampCol(T(_vec.w))} {}
            template <typename Y>
            ColorABase<T> operator=(const Vec4Base<Y> &_vec)
                { this->r = UnHood::clampCol(T(_vec.x)); this->g = UnHood::clampCol(T(_vec.y));
                  this->b = UnHood::clampCol(T(_vec.z)); this->a = UnHood::clampCol(T(_vec.w)); return *this; }

            operator Vec4Base<T>() const { return {this->r, this->g, this->b, a}; }
            Vec4Base<T>   toVec4() const { return {this->r, this->g, this->b, a}; }
    };


        //  ============  Arithmetic operations 3-3, 3-1, 1-3  ============  \\

        template <typename T> ColorBase<T> operator+(const ColorBase<T> &op1, const ColorBase<T> &op2)
            { return {op1.r + op2.r, op1.g + op2.g, op1.b + op2.b}; }
        template <typename T> ColorBase<T> operator-(const ColorBase<T> &op1, const ColorBase<T> &op2)
            { return {op1.r - op2.r, op1.g - op2.g, op1.b - op2.b}; }
        template <typename T> ColorBase<T> operator*(const ColorBase<T> &op1, const ColorBase<T> &op2)
            { return {op1.r * op2.r, op1.g * op2.g, op1.b * op2.b}; }
        template <typename T> ColorBase<T> operator/(const ColorBase<T> &op1, const ColorBase<T> &op2)
            { return {Func::notCloseToZero(op2.r) ? op1.r / op2.r : UnHood::ColMax<T>::max,
                      Func::notCloseToZero(op2.g) ? op1.g / op2.g : UnHood::ColMax<T>::max,
                      Func::notCloseToZero(op2.b) ? op1.b / op2.b : UnHood::ColMax<T>::max}; }

        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorBase<T>>::type operator+(const ColorBase<T> &op1, Y op2)
            { return {op1.r + op2, op1.g + op2, op1.b + op2}; }
        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorBase<T>>::type operator-(const ColorBase<T> &op1, Y op2)
            { return {op1.r - op2, op1.g - op2, op1.b - op2}; }
        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorBase<T>>::type operator*(const ColorBase<T> &op1, Y op2)
            { return {op1.r * op2, op1.g * op2, op1.b * op2}; }
        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorBase<T>>::type operator/(const ColorBase<T> &op1, Y op2)
            { return Func::notCloseToZero(op2) ? ColorBase<T>(op1.r / op2, op1.g / op2, op1.b / op2) : ColorBase<T>(UnHood::ColMax<T>::max); }

        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorBase<T>>::type operator+(Y op1, const ColorBase<T> &op2)
            { return {op1 + op2.r, op1 + op2.g, op1 + op2.b}; }
        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorBase<T>>::type operator-(Y op1, const ColorBase<T> &op2)
            { return {op1 - op2.r, op1 - op2.g, op1 - op2.b}; }
        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorBase<T>>::type operator*(Y op1, const ColorBase<T> &op2)
            { return {op1 * op2.r, op1 * op2.g, op1 * op2.b}; }
        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorBase<T>>::type operator/(Y op1, const ColorBase<T> &op2)
            { return {Func::notCloseToZero(op2.r) ? op1 / op2.r : UnHood::ColMax<T>::max,
                      Func::notCloseToZero(op2.g) ? op1 / op2.g : UnHood::ColMax<T>::max,
                      Func::notCloseToZero(op2.b) ? op1 / op2.b : UnHood::ColMax<T>::max}; }


        template <typename T> void operator+=(ColorBase<T> &op1, const ColorBase<T> &op2)
            { op1 = {op1.r + op2.r, op1.g + op2.g, op1.b + op2.b}; }
        template <typename T> void operator-=(ColorBase<T> &op1, const ColorBase<T> &op2)
            { op1 = {op1.r - op2.r, op1.g - op2.g, op1.b - op2.b}; }
        template <typename T> void operator*=(ColorBase<T> &op1, const ColorBase<T> &op2)
            { op1 = {op1.r * op2.r, op1.g * op2.g, op1.b * op2.b}; }
        template <typename T> void operator/=(ColorBase<T> &op1, const ColorBase<T> &op2)
            { op1 = {Func::notCloseToZero(op2.r) ? op1.r / op2.r : UnHood::ColMax<T>::max,
                     Func::notCloseToZero(op2.g) ? op1.g / op2.g : UnHood::ColMax<T>::max,
                     Func::notCloseToZero(op2.b) ? op1.b / op2.b : UnHood::ColMax<T>::max}; }

        template <typename T, typename Y> typename std::enable_if<std::is_fundamental<Y>::value, void>::type
        operator+=(ColorBase<T> &op1, Y op2) { op1 = {op1.r + op2, op1.g + op2, op1.b + op2}; }
        template <typename T, typename Y> typename std::enable_if<std::is_fundamental<Y>::value, void>::type
        operator-=(ColorBase<T> &op1, Y op2) { op1 = {op1.r - op2, op1.g - op2, op1.b - op2}; }
        template <typename T, typename Y> typename std::enable_if<std::is_fundamental<Y>::value, void>::type
        operator*=(ColorBase<T> &op1, Y op2) { op1 = {op1.r * op2, op1.g * op2, op1.b * op2}; }
        template <typename T, typename Y> typename std::enable_if<std::is_fundamental<Y>::value, void>::type
        operator/=(ColorBase<T> &op1, Y op2)
            { op1 = Func::notCloseToZero(op2) ? ColorBase<T>(op1.r / op2, op1.g / op2, op1.b / op2) : ColorBase<T>(UnHood::ColMax<T>::max); }


        //  ============  Arithmetic operations 4-4, 4-1, 1-4  ============  \\

        template <typename T> ColorABase<T> operator+(const ColorABase<T> &op1, const ColorABase<T> &op2)
            { return {op1.r + op2.r, op1.g + op2.g, op1.b + op2.b, op1.a + op2.a}; }
        template <typename T> ColorABase<T> operator-(const ColorABase<T> &op1, const ColorABase<T> &op2)
            { return {op1.r - op2.r, op1.g - op2.g, op1.b - op2.b, op1.a - op2.a}; }
        template <typename T> ColorABase<T> operator*(const ColorABase<T> &op1, const ColorABase<T> &op2)
            { return {op1.r * op2.r, op1.g * op2.g, op1.b * op2.b, op1.a * op2.a}; }
        template <typename T> ColorABase<T> operator/(const ColorABase<T> &op1, const ColorABase<T> &op2)
            { return {Func::notCloseToZero(op2.r) ? op1.r / op2.r : UnHood::ColMax<T>::max, Func::notCloseToZero(op2.g) ? op1.g / op2.g : UnHood::ColMax<T>::max,
                    Func::notCloseToZero(op2.b) ? op1.b / op2.b : UnHood::ColMax<T>::max, Func::notCloseToZero(op2.a) ? op1.a / op2.a : UnHood::ColMax<T>::max}; }

        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorABase<T>>::type operator+(const ColorABase<T> &op1, Y op2)
            { return {op1.r + op2, op1.g + op2, op1.b + op2, op1.a + op2}; }
        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorABase<T>>::type operator-(const ColorABase<T> &op1, Y op2)
            { return {op1.r - op2, op1.g - op2, op1.b - op2, op1.a - op2}; }
        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorABase<T>>::type operator*(const ColorABase<T> &op1, Y op2)
            { return {op1.r * op2, op1.g * op2, op1.b * op2, op1.a * op2}; }
        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorABase<T>>::type operator/(const ColorABase<T> &op1, Y op2)
            { return Func::notCloseToZero(op2) ? ColorABase<T>(op1.r / op2, op1.g / op2, op1.b / op2, op1.a / op2) : ColorABase<T>(UnHood::ColMax<T>::max); }

        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorABase<T>>::type operator+(Y op1, const ColorABase<T> &op2)
            { return {op1 + op2.r, op1 + op2.g, op1 + op2.b, op1 + op2.a}; }
        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorABase<T>>::type operator-(Y op1, const ColorABase<T> &op2)
            { return {op1 - op2.r, op1 - op2.g, op1 - op2.b, op1 - op2.a}; }
        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorABase<T>>::type operator*(Y op1, const ColorABase<T> &op2)
            { return {op1 * op2.r, op1 * op2.g, op1 * op2.b, op1 * op2.a}; }
        template <typename T, typename Y>
        typename std::enable_if<std::is_fundamental<Y>::value, ColorABase<T>>::type operator/(Y op1, const ColorABase<T> &op2)
            { return {Func::notCloseToZero(op2.r) ? op1 / op2.r : UnHood::ColMax<T>::max, Func::notCloseToZero(op2.g) ? op1 / op2.g : UnHood::ColMax<T>::max,
                    Func::notCloseToZero(op2.b) ? op1 / op2.b : UnHood::ColMax<T>::max, Func::notCloseToZero(op2.a) ? op1 / op2.a : UnHood::ColMax<T>::max}; }


        template <typename T> void operator+=(ColorABase<T> &op1, const ColorABase<T> &op2)
            { op1 = {op1.r + op2.r, op1.g + op2.g, op1.b + op2.b, op1.a + op2.a}; }
        template <typename T> void operator-=(ColorABase<T> &op1, const ColorABase<T> &op2)
            { op1 = {op1.r - op2.r, op1.g - op2.g, op1.b - op2.b, op1.a - op2.a}; }
        template <typename T> void operator*=(ColorABase<T> &op1, const ColorABase<T> &op2)
            { op1 = {op1.r * op2.r, op1.g * op2.g, op1.b * op2.b, op1.a * op2.a}; }
        template <typename T> void operator/=(ColorABase<T> &op1, const ColorABase<T> &op2)
            { op1 = {Func::notCloseToZero(op2.r) ? op1.r / op2.r : UnHood::ColMax<T>::max, Func::notCloseToZero(op2.g) ? op1.g / op2.g : UnHood::ColMax<T>::max,
                    Func::notCloseToZero(op2.b) ? op1.b / op2.b : UnHood::ColMax<T>::max, Func::notCloseToZero(op2.a) ? op1.a / op2.a : UnHood::ColMax<T>::max}; }

        template <typename T, typename Y> void operator+=(ColorABase<T> &op1, Y op2) { op1 = {op1.r + op2, op1.g + op2, op1.b + op2, op1.a + op2}; }
        template <typename T, typename Y> void operator-=(ColorABase<T> &op1, Y op2) { op1 = {op1.r - op2, op1.g - op2, op1.b - op2, op1.a - op2}; }
        template <typename T, typename Y> void operator*=(ColorABase<T> &op1, Y op2) { op1 = {op1.r * op2, op1.g * op2, op1.b * op2, op1.a * op2}; }
        template <typename T, typename Y> void operator/=(ColorABase<T> &op1, Y op2)
            { op1 = Func::notCloseToZero(op2) ? ColorABase<T>(op1.r / op2, op1.g / op2, op1.b / op2, op1.a / op2) : ColorABase<T>(UnHood::ColMax<T>::max); }


        //  ============  Mixed color operations 4-3, 3-4  ============  \\

        template <typename T> ColorABase<T> operator+(const ColorABase<T> &op1, const ColorBase<T> &op2)
            { return {op1.r + op2.r, op1.g + op2.g, op1.b + op2.b, UnHood::ColMax<T>::max}; }
        template <typename T> ColorABase<T> operator-(const ColorABase<T> &op1, const ColorBase<T> &op2)
            { return {op1.r - op2.r, op1.g - op2.g, op1.b - op2.b,  T(0)}; }
        template <typename T> ColorABase<T> operator*(const ColorABase<T> &op1, const ColorBase<T> &op2)
            { return {op1.r * op2.r, op1.g * op2.g, op1.b * op2.b, op1.a}; }
        template <typename T> ColorABase<T> operator/(const ColorABase<T> &op1, const ColorBase<T> &op2)
            { return {Func::notCloseToZero(op2.r) ? op1.r / op2.r : UnHood::ColMax<T>::max, Func::notCloseToZero(op2.g) ? op1.g / op2.g : UnHood::ColMax<T>::max,
                    Func::notCloseToZero(op2.b) ? op1.b / op2.b : UnHood::ColMax<T>::max, op1.a}; }

        template <typename T> ColorABase<T> operator+(const ColorBase<T> &op1, const ColorABase<T> &op2)
            { return {op1.r + op2.r, op1.g + op2.g, op1.b + op2.b, UnHood::ColMax<T>::max}; }
        template <typename T> ColorABase<T> operator-(const ColorBase<T> &op1, const ColorABase<T> &op2)
            { return {op1.r - op2.r, op1.g - op2.g, op1.b - op2.b, UnHood::ColMax<T>::max - op2.a}; }
        template <typename T> ColorABase<T> operator*(const ColorBase<T> &op1, const ColorABase<T> &op2)
            { return {op1.r * op2.r, op1.g * op2.g, op1.b * op2.b, op2.a}; }
        template <typename T> ColorABase<T> operator/(const ColorBase<T> &op1, const ColorABase<T> &op2)
            { return {Func::notCloseToZero(op2.r) ? op1.r / op2.r : UnHood::ColMax<T>::max, Func::notCloseToZero(op2.g) ?          op1.g / op2.g : UnHood::ColMax<T>::max,
                    Func::notCloseToZero(op2.b) ? op1.b / op2.b : UnHood::ColMax<T>::max, Func::notCloseToZero(op2.a) ? UnHood::ColMax<T>::max / op2.a : UnHood::ColMax<T>::max}; }


        template <typename T> void operator+=(ColorABase<T> &op1, const ColorBase<T> &op2) { op1 = {op1.r + op2.r, op1.g + op2.g, op1.b + op2.b, UnHood::ColMax<T>::max}; }
        template <typename T> void operator-=(ColorABase<T> &op1, const ColorBase<T> &op2) { op1 = {op1.r - op2.r, op1.g - op2.g, op1.b - op2.b, T(0)}; }
        template <typename T> void operator*=(ColorABase<T> &op1, const ColorBase<T> &op2) { op1 = {op1.r * op2.r, op1.g * op2.g, op1.b * op2.b, op1.a}; }
        template <typename T> void operator/=(ColorABase<T> &op1, const ColorBase<T> &op2)
            { op1 = {Func::notCloseToZero(op2.r) ? op1.r / op2.r : UnHood::ColMax<T>::max, Func::notCloseToZero(op2.g) ? op1.g / op2.g : UnHood::ColMax<T>::max,
                    Func::notCloseToZero(op2.b) ? op1.b / op2.b : UnHood::ColMax<T>::max, op1.a}; }

        template <typename T> void operator+=(ColorBase<T> &op1, const ColorABase<T> &op2) { op1 = {op1.r + op2.r, op1.g + op2.g, op1.b + op2.b}; }
        template <typename T> void operator-=(ColorBase<T> &op1, const ColorABase<T> &op2) { op1 = {op1.r - op2.r, op1.g - op2.g, op1.b - op2.b}; }
        template <typename T> void operator*=(ColorBase<T> &op1, const ColorABase<T> &op2) { op1 = {op1.r * op2.r, op1.g * op2.g, op1.b * op2.b}; }
        template <typename T> void operator/=(ColorBase<T> &op1, const ColorABase<T> &op2)
            { op1 = {Func::notCloseToZero(op2.r) ? op1.r / op2.r : UnHood::ColMax<T>::max,
                     Func::notCloseToZero(op2.g) ? op1.g / op2.g : UnHood::ColMax<T>::max,
                     Func::notCloseToZero(op2.b) ? op1.b / op2.b : UnHood::ColMax<T>::max}; }


        //  ============  Porter-Duff blends 4-4, 4-3, 3-4  ============  \\

        inline ColorABase<float> PorterDuffBlendOver(const ColorABase<float> &c1, const ColorABase<float> &c2)
            { return {c1.r + (1.f - c1.a) * c2.r, c1.g + (1.f - c1.a) * c2.g,
                    c1.b + (1.f - c1.a) * c2.b, c1.a + (1.f - c1.a) * c2.a}; }
        inline ColorABase<float> PorterDuffBlendIn  (const ColorABase<float> &c1, const ColorABase<float> &c2)
            { return {c2.a * c1.r, c2.a * c1.g, c2.a * c1.b, c2.a * c1.a}; }
        inline ColorABase<float> PorterDuffBlendOut (const ColorABase<float> &c1, const ColorABase<float> &c2)
            { return {(1.f - c2.a) * c1.r, (1.f - c2.a) * c1.g, (1.f - c2.a) * c1.b, (1.f - c2.a) * c1.a}; }
        inline ColorABase<float> PorterDuffBlendAtop(const ColorABase<float> &c1, const ColorABase<float> &c2)
            { return {c2.a * c1.r + (1.f - c1.a) * c2.r, c2.a * c1.g + (1.f - c1.a) * c2.g,
                      c2.a * c1.b + (1.f - c1.a) * c2.b, c2.a}; }
        inline ColorABase<float> PorterDuffBlendXor (const ColorABase<float> &c1, const ColorABase<float> &c2)
            { return {(1.f - c2.a) * c1.r + (1.f - c1.a) * c2.r, (1.f - c2.a) * c1.g + (1.f - c1.a) * c2.g,
                      (1.f - c2.a) * c1.b + (1.f - c1.a) * c2.b, (1.f - c2.a) * c1.a + (1.f - c1.a) * c2.a}; }

        inline ColorABase<float> PorterDuffBlendOver(const ColorABase<float> &c1, const ColorBase<float> &c2)
            { return {c1.r + (1.f - c1.a) * c2.r, c1.g + (1.f - c1.a) * c2.g,
                      c1.b + (1.f - c1.a) * c2.b, 1.f}; }
        inline ColorABase<float> PorterDuffBlendIn  (const ColorABase<float> &c1, const ColorBase<float> &c2)
            { return c1; }
        inline ColorABase<float> PorterDuffBlendOut (const ColorABase<float> &c1, const ColorBase<float> &c2)
            { return {0.f, 0.f, 0.f, 0.f}; }
        inline ColorABase<float> PorterDuffBlendAtop(const ColorABase<float> &c1, const ColorBase<float> &c2)
            { return {c1.r + (1.f - c1.a) * c2.r, c1.g + (1.f - c1.a) * c2.g,
                    c1.b + (1.f - c1.a) * c2.b, 1.f}; }
        inline ColorABase<float> PorterDuffBlendXor (const ColorABase<float> &c1, const ColorBase<float> &c2)
            { return {(1.f - c1.a) * c2.r, (1.f - c1.a) * c2.g, (1.f - c1.a) * c2.b, 1.f - c1.a}; }

        inline ColorABase<float> PorterDuffBlendOver(const ColorBase<float> &c1, const ColorABase<float> &c2)
            { return c1; }
        inline ColorABase<float> PorterDuffBlendIn  (const ColorBase<float> &c1, const ColorABase<float> &c2)
            { return {c2.a * c1.r, c2.a * c1.g, c2.a * c1.b, c2.a}; }
        inline ColorABase<float> PorterDuffBlendOut (const ColorBase<float> &c1, const ColorABase<float> &c2)
            { return {(1.f - c2.a) * c1.r, (1.f - c2.a) * c1.g, (1.f - c2.a) * c1.b, 1.f - c2.a}; }
        inline ColorABase<float> PorterDuffBlendAtop(const ColorBase<float> &c1, const ColorABase<float> &c2)
            { return {c2.a * c1.r, c2.a * c1.g, c2.a * c1.b, c2.a}; }
        inline ColorABase<float> PorterDuffBlendXor (const ColorBase<float> &c1, const ColorABase<float> &c2)
            { return {(1.f - c2.a) * c1.r, (1.f - c2.a) * c1.g, (1.f - c2.a) * c1.b, 1.f - c2.a}; }


        inline ColorABase<uchar> PorterDuffBlendOver(const ColorABase<uchar> &c1, const ColorABase<uchar> &c2)
            { return c1 + ((255 - c1.a) * COLORS_UTOF) * c2; }
        inline ColorABase<uchar> PorterDuffBlendIn  (const ColorABase<uchar> &c1, const ColorABase<uchar> &c2)
            { return c1 * (c2.a * COLORS_UTOF); }
        inline ColorABase<uchar> PorterDuffBlendOut (const ColorABase<uchar> &c1, const ColorABase<uchar> &c2)
            { return c1 * ((255 - c2.a) * COLORS_UTOF); }
        inline ColorABase<uchar> PorterDuffBlendAtop(const ColorABase<uchar> &c1, const ColorABase<uchar> &c2)
            { const float k = (255 - c1.a) * COLORS_UTOF, k1 = c2.a * COLORS_UTOF;
            return {(uchar)(k1 * c1.r) + (uchar)(k * c2.r), (uchar)(k1 * c1.g) + (uchar)(k * c2.g),
                    (uchar)(k1 * c1.b) + (uchar)(k * c2.b), c2.a}; }
        inline ColorABase<uchar> PorterDuffBlendXor (const ColorABase<uchar> &c1, const ColorABase<uchar> &c2)
            { const float k = (255 - c2.a) * COLORS_UTOF;
            return { (uchar)(k * c1.r) + (uchar)(k * c2.r), (uchar)(k * c1.g) + (uchar)(k * c2.g),
                    (uchar)(k * c1.b) + (uchar)(k * c2.b), (uchar)(k * c1.a) + (uchar)(k * c2.a)}; }

        inline ColorABase<uchar> PorterDuffBlendOver(const ColorABase<uchar> &c1, const ColorBase<uchar> &c2)
            { const float k = (255 - c1.a) * COLORS_UTOF;
            return {c1.r + (uchar)(k * c2.r), c1.g + (uchar)(k * c2.g), c1.b + (uchar)(k * c2.b), 255}; }
        inline ColorABase<uchar> PorterDuffBlendIn  (const ColorABase<uchar> &c1, const ColorBase<uchar> &c2)
            { return c1; }
        inline ColorABase<uchar> PorterDuffBlendOut (const ColorABase<uchar> &c1, const ColorBase<uchar> &c2)
            { return 0; }
        inline ColorABase<uchar> PorterDuffBlendAtop(const ColorABase<uchar> &c1, const ColorBase<uchar> &c2)
            { const float k = (255 - c1.a) * COLORS_UTOF;
            return {c1.r + (uchar)(k * c2.r), c1.g + (uchar)(k * c2.g), c1.b + (uchar)(k * c2.b), 255}; }
        inline ColorABase<uchar> PorterDuffBlendXor (const ColorABase<uchar> &c1, const ColorBase<uchar> &c2)
            { const float k = (255 - c1.a) * COLORS_UTOF;
            return {(uchar)(k * c2.r), (uchar)(k * c2.g), (uchar)(k * c2.b), 255u - c1.a}; }

        inline ColorABase<uchar> PorterDuffBlendOver(const ColorBase<uchar> &c1, const ColorABase<uchar> &c2)
            { return c1; }
        inline ColorABase<uchar> PorterDuffBlendIn  (const ColorBase<uchar> &c1, const ColorABase<uchar> &c2)
            { const float k1 = c2.a * COLORS_UTOF;
            return {(uchar)(k1 * c1.r), (uchar)(k1 * c1.g), (uchar)(k1 * c1.b), c2.a}; }
        inline ColorABase<uchar> PorterDuffBlendOut (const ColorBase<uchar> &c1, const ColorABase<uchar> &c2)
            { const float k = (255 - c2.a) * COLORS_UTOF;
            return {(uchar)(k * c1.r), (uchar)(k * c1.g), (uchar)(k * c1.b), 255u - c2.a}; }
        inline ColorABase<uchar> PorterDuffBlendAtop(const ColorBase<uchar> &c1, const ColorABase<uchar> &c2)
            { const float k1 = c2.a * COLORS_UTOF;
            return {(uchar)(k1 * c1.r), (uchar)(k1 * c1.g), (uchar)(k1 * c1.b), c2.a}; }
        inline ColorABase<uchar> PorterDuffBlendXor (const ColorBase<uchar> &c1, const ColorABase<uchar> &c2)
            { const float k = (255 - c2.a) * COLORS_UTOF;
            return {(uchar)(k * c1.r), (uchar)(k * c1.g), (uchar)(k * c1.b), 255u - c2.a}; }

        
        //  ============  Derived color classes declaration  ============  \\

        typedef ColorBase <float> Color;
        typedef ColorBase <uchar> Color256;
        typedef ColorABase<float> ColorA;
        typedef ColorABase<uchar> ColorA256;


        //  ============  Reference white  ============  \\

        const Color D50 = { 0.96422f, 1.f, 0.82521f };
        const Color D65 = { 0.95047f, 1.f, 1.08883f };
        const Color D502D65 = { 0.9857398f, 1.f, 1.3194581f };
        const Color D652D50 = { 1.0144665f, 1.f, 0.7578869f };
    };
};


#endif