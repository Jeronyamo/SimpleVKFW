#define COLORS_FTOU 255u
#define COLORS_UTOF 0.003921568859f     // 1. / 255

#include "vectors.h"


namespace Simple {
    using std::modf;
    using std::min;

    inline float clamp(float val) { return val < 0.f ? 0.f : (val > 1.f ? 1.f : val); }
    inline int clamp(int val) { return val < 0 ? 0 : (val > 255 ? 255 : val); }
    inline unsigned clamp(unsigned val) { return val > 255u ? 255u : val; }
    template <typename T> inline Vec3Base<T> clamp(const Vec3Base<T> &vec) { return { clamp(vec.x), clamp(vec.y), clamp(vec.z) }; }
    template <typename T> inline Vec4Base<T> clamp(const Vec4Base<T> &vec) { return { clamp(vec.x), clamp(vec.y), clamp(vec.z), clamp(vec.w) }; }


    //  ============  Types Declaration  ============  \\

    template <typename T> struct RGBBase;
    struct RGB;
    struct RGB256;
    template <typename T> struct RGBABase;
    struct RGBA;
    struct RGBA256;

    template <typename T> struct HSVBase;
    template <typename T> struct HSVABase;
    struct HSV;
    struct HSVA;

    template <typename T> struct XYZBase;
    template <typename T> struct XYZABase;
    struct XYZ;
    struct XYZA;

    template <typename T> struct LABBase;
    template <typename T> struct LABABase;
    struct LAB;
    struct LABA;

    template <typename T> struct LMSBase;
    template <typename T> struct LMSABase;
    struct LMS;
    struct LMSA;

    template <typename T> struct ITPBase;
    template <typename T> struct ITPABase;
    struct ITP;
    struct ITPA;

    template <typename T> struct WideRGBBase;
    template <typename T> struct WideRGBABase;
    struct WideRGB;
    struct WideRGBA;


    //  ============  RGB color model  ============  \\

    template <typename T>
    struct RGBBase {
        T r = 0, g = 0, b = 0;

        RGBBase() {}
        template <class R, class G, class B> RGBBase(R _r, G _g, B _b) : r{_r}, g{_g}, b{_b} {}
        template <typename Y> RGBBase(const Vec3Base<Y> &vec) : r{vec.x}, g{vec.y}, b{vec.z} {}
        Vec3Base<T> toVec() { return {r, g, b}; }

        RGBBase<T> operator+(const RGBBase<T> &rgb) { return {clamp(r + rgb.r), clamp(g + rgb.g), clamp(b + rgb.b)}; }
        RGBBase<T> operator-(const RGBBase<T> &rgb) { return {clamp(r - rgb.r), clamp(g - rgb.g), clamp(b - rgb.b)}; }
        RGBBase<T> operator*(const RGBBase<T> &rgb) { return {clamp(r * rgb.r), clamp(g * rgb.g), clamp(b * rgb.b)}; }
        RGBBase<T> operator/(const RGBBase<T> &rgb)
            { return { notCloseToZero(rgb.r) ? clamp(r / rgb.r) : 255, notCloseToZero(rgb.g) ? clamp(g / rgb.g) : 255,
                                                                       notCloseToZero(rgb.b) ? clamp(b / rgb.b) : 255}; }

        void operator+=(const RGBBase<T> &rgb) { *this = {clamp(r + rgb.r), clamp(g + rgb.g), clamp(b + rgb.b)}; }
        void operator-=(const RGBBase<T> &rgb) { *this = {clamp(r - rgb.r), clamp(g - rgb.g), clamp(b - rgb.b)}; }
        void operator*=(const RGBBase<T> &rgb) { *this = {clamp(r * rgb.r), clamp(g * rgb.g), clamp(b * rgb.b)}; }
        void operator/=(const RGBBase<T> &rgb)
            { *this = { notCloseToZero(rgb.r) ? clamp(r / rgb.r) : 255, notCloseToZero(rgb.g) ? clamp(g / rgb.g) : 255,
                                                                        notCloseToZero(rgb.b) ? clamp(b / rgb.b) : 255}; }
    };

    struct RGB : RGBBase<float> {
        template <class R, class G, class B> RGB(R _r, G _g, B _b) : RGBBase{clamp(float(_r)), clamp(float(_g)), clamp(float(_b))} {}
        RGB(const RGB &_col);
        RGB(const RGBA &_col);
        RGB(const RGB256 &_col);
        RGB(const RGBA256 &_col);

        float &operator[](const unsigned index) { return *((float*)this + index); }
        const float &operator[](const unsigned index) const { return *((float*)this + index); }
    };

    struct RGB256 : RGBBase<unsigned char> {
        template <class R, class G, class B> RGB256(R _r, G _g, B _b) : RGBBase{clamp(int(_r)), clamp(int(_g)), clamp(int(_b))} {}
        RGB256(const RGB &_col);
        RGB256(const RGBA &_col);
        RGB256(const RGB256 &_col);
        RGB256(const RGBA256 &_col);
    };


    //  ============  RGBA color model  ============  \\

    template <typename T>
    struct RGBABase {
        T r = 0, g = 0, b = 0, a = 0;

        RGBABase() {}
        template <class R, class G, class B, class A> RGBABase(R _r, G _g, B _b, A _a) : r{_r}, g{_g}, b{_b}, a{_a} {}
        template <typename Y> RGBABase(const Vec4Base<Y> &vec) : r{vec.x}, g{vec.y}, b{vec.z}, a{vec.w} {}
        Vec4Base<T> toVec() { return {r, g, b, a}; }
    };

    struct RGBA : RGBABase<float> {
        template <class R, class G, class B, class A = float > RGBA(R _r, G _g, B _b, A _a = 1.f) :
            RGBABase{clamp(float(_r)), clamp(float(_g)), clamp(float(_b)), clamp(float(_a))} {}
        RGBA(const RGBA &_col);
        RGBA(const RGBA256 &_col);
        template <typename T> RGBA(const Vec4Base<T> &vec);
        template <typename T = float > RGBA(const RGB &_col, const T _a = 1.f);
        template <typename T = float > RGBA(const RGB256 &_col, const T _a = 1.f);
    };

    struct RGBA256 : RGBABase<unsigned char> {
        template <class R, class G, class B, class A = unsigned > RGBA256(R _r, G _g, B _b, A _a = 255u) :
            RGBABase{clamp(int(_r)), clamp(int(_g)), clamp(int(_b)), clamp(int(_a))} {}
        RGBA256(const RGBA &_col);
        RGBA256(const RGBA256 &_col);
        template <typename T> RGBA256(const Vec4Base<T> &vec);
        template <typename T = int > RGBA256(const RGB &_col, const T _a = 255);
        template <typename T = int > RGBA256(const RGB256 &_col, const T _a = 255);

        RGB rgb() { return { r, g, b }; }
    };

    RGB::RGB(const RGB &_col) : RGBBase{_col.r, _col.g, _col.b} {}
    RGB::RGB(const RGBA &_col) : RGBBase{_col.r, _col.g, _col.b} {}
    RGB::RGB(const RGB256 &_col) : RGBBase{_col.r * COLORS_UTOF, _col.g * COLORS_UTOF, _col.b * COLORS_UTOF} {}
    RGB::RGB(const RGBA256 &_col) : RGBBase{_col.r * COLORS_UTOF, _col.g * COLORS_UTOF, _col.b * COLORS_UTOF} {}

    RGB256::RGB256(const RGB &_col) : RGBBase{(unsigned char) (_col.r * COLORS_FTOU), (unsigned char) (_col.g * COLORS_FTOU), (unsigned char) (_col.b * COLORS_FTOU)} {}
    RGB256::RGB256(const RGBA &_col) : RGBBase{(unsigned char) (_col.r * COLORS_FTOU), (unsigned char) (_col.g * COLORS_FTOU), (unsigned char) (_col.b * COLORS_FTOU)} {}
    RGB256::RGB256(const RGB256 &_col) : RGBBase{_col.r, _col.g, _col.b} {}
    RGB256::RGB256(const RGBA256 &_col) : RGBBase{_col.r, _col.g, _col.b} {}

    RGBA::RGBA(const RGBA &_col) : RGBABase{_col.r, _col.g, _col.b, _col.a} {}
    RGBA::RGBA(const RGBA256 &_col) : RGBABase{_col.r * COLORS_UTOF, _col.g * COLORS_UTOF, _col.b * COLORS_UTOF, _col.a * COLORS_UTOF} {}
    template <typename T> RGBA::RGBA(const Vec4Base<T> &vec) : RGBABase(clamp(Vec4(vec))) {}

    template <typename T> RGBA::RGBA(const RGB &_col, const T _a) :
        RGBABase{_col.r, _col.g, _col.b, clamp(float(_a))} {}
    template <typename T> RGBA::RGBA(const RGB256 &_col, const T _a) :
        RGBABase{_col.r * COLORS_UTOF, _col.g * COLORS_UTOF, _col.b * COLORS_UTOF, clamp(float(_a))} {}

    RGBA256::RGBA256(const RGBA &_col) : RGBABase{(unsigned char) (_col.r * COLORS_FTOU), (unsigned char) (_col.g * COLORS_FTOU), (unsigned char) (_col.b * COLORS_FTOU), (unsigned char) (_col.a * COLORS_FTOU)} {}
    RGBA256::RGBA256(const RGBA256 &_col) : RGBABase{_col.r, _col.g, _col.b, _col.a} {}
    template <typename T> RGBA256::RGBA256(const Vec4Base<T> &vec) : RGBABase(clamp(IVec4(vec))) {}

    template <typename T> RGBA256::RGBA256(const RGB &_col, const T _a) :
        RGBABase{_col.r * COLORS_FTOU, _col.g * COLORS_FTOU, _col.b * COLORS_FTOU, clamp(int(_a))} {}
    template <typename T> RGBA256::RGBA256(const RGB256 &_col, const T _a) :
        RGBABase{_col.r, _col.g, _col.b, clamp(int(_a))} {}


    //  ============  RGB/RGBA color blends  ============  \\

    template <typename T> inline RGBBase<T> MIN(const RGBBase<T> &c1, const RGBBase<T> &c2)
        { return {c1.r <= c2.r ? c1.r : c2.r, c1.g <= c2.g ? c1.g : c2.g, c1.b <= c2.b ? c1.b : c2.b}; }
    template <typename T> inline RGBBase<T> MAX(const RGBBase<T> &c1, const RGBBase<T> &c2)
        { return {c1.r >= c2.r ? c1.r : c2.r, c1.g >= c2.g ? c1.g : c2.g, c1.b >= c2.b ? c1.b : c2.b}; }
    template <typename T> inline RGBBase<T> AVG(const RGBBase<T> &c1, const RGBBase<T> &c2)
        { return {(c1.r + c2.r) / 2, (c1.g + c2.g) / 2, (c1.b + c2.b) / 2}; }
    template <typename T> inline RGBBase<T> DIF(const RGBBase<T> &c1, const RGBBase<T> &c2)
        { return {c1.r >= c2.r ? c1.r - c2.r : c2.r - c1.r,
                  c1.g >= c2.g ? c1.g - c2.g : c2.g - c1.g,
                  c1.b >= c2.b ? c1.b - c2.b : c2.b - c1.b}; }

    template <typename T> inline RGBABase<T> MIN(const RGBABase<T> &c1, const RGBBase<T> &c2)
        { return {c1.r <= c2.r ? c1.r : c2.r, c1.g <= c2.g ? c1.g : c2.g, c1.b <= c2.b ? c1.b : c2.b, c1.a}; }
    template <typename T> inline RGBABase<T> MAX(const RGBABase<T> &c1, const RGBBase<T> &c2)
        { return {c1.r >= c2.r ? c1.r : c2.r, c1.g >= c2.g ? c1.g : c2.g, c1.b >= c2.b ? c1.b : c2.b, 1.f}; }
    template <typename T> inline RGBABase<T> AVG(const RGBABase<T> &c1, const RGBBase<T> &c2)
        { return {(c1.r + c2.r) / 2, (c1.g + c2.g) / 2, (c1.b + c2.b) / 2, (c1.a + 1.f) / 2}; }
    template <typename T> inline RGBABase<T> DIF(const RGBABase<T> &c1, const RGBBase<T> &c2)
        { return {c1.r >= c2.r ? c1.r - c2.r : c2.r - c1.r,
                  c1.g >= c2.g ? c1.g - c2.g : c2.g - c1.g,
                  c1.b >= c2.b ? c1.b - c2.b : c2.b - c1.b,
                  1.f - c1.a}; }

    template <typename T> inline RGBABase<T> MIN(const RGBBase<T> &c1, const RGBABase<T> &c2)
        { return {c1.r <= c2.r ? c1.r : c2.r, c1.g <= c2.g ? c1.g : c2.g, c1.b <= c2.b ? c1.b : c2.b, c2.a}; }
    template <typename T> inline RGBABase<T> MAX(const RGBBase<T> &c1, const RGBABase<T> &c2)
        { return {c1.r >= c2.r ? c1.r : c2.r, c1.g >= c2.g ? c1.g : c2.g, c1.b >= c2.b ? c1.b : c2.b, 1.f}; }
    template <typename T> inline RGBABase<T> AVG(const RGBBase<T> &c1, const RGBABase<T> &c2)
        { return {(c1.r + c2.r) / 2, (c1.g + c2.g) / 2, (c1.b + c2.b) / 2, (1.f + c2.a) / 2}; }
    template <typename T> inline RGBABase<T> DIF(const RGBBase<T> &c1, const RGBABase<T> &c2)
        { return {c1.r >= c2.r ? c1.r - c2.r : c2.r - c1.r,
                  c1.g >= c2.g ? c1.g - c2.g : c2.g - c1.g,
                  c1.b >= c2.b ? c1.b - c2.b : c2.b - c1.b,
                  1.f - c2.a}; }

    template <typename T> inline RGBABase<T> MIN(const RGBABase<T> &c1, const RGBABase<T> &c2)
        { return {c1.r <= c2.r ? c1.r : c2.r, c1.g <= c2.g ? c1.g : c2.g, c1.b <= c2.b ? c1.b : c2.b, c1.a <= c2.a ? c1.a : c2.a}; }
    template <typename T> inline RGBABase<T> MAX(const RGBABase<T> &c1, const RGBABase<T> &c2)
        { return {c1.r >= c2.r ? c1.r : c2.r, c1.g >= c2.g ? c1.g : c2.g, c1.b >= c2.b ? c1.b : c2.b, c1.a >= c2.a ? c1.a : c2.a}; }
    template <typename T> inline RGBABase<T> AVG(const RGBABase<T> &c1, const RGBABase<T> &c2)
        { return {(c1.r + c2.r) / 2, (c1.g + c2.g) / 2, (c1.b + c2.b) / 2, (c1.a + c2.a) / 2}; }
    template <typename T> inline RGBABase<T> DIF(const RGBABase<T> &c1, const RGBABase<T> &c2)
        { return {c1.r >= c2.r ? c1.r - c2.r : c2.r - c1.r,
                  c1.g >= c2.g ? c1.g - c2.g : c2.g - c1.g,
                  c1.b >= c2.b ? c1.b - c2.b : c2.b - c1.b,
                  c1.a >= c2.a ? c1.a - c2.a : c2.a - c1.a}; }

    
    inline RGBA PorterDuffBlendOver(const RGBA &c1, const RGBA &c2)
        { return {min(c1.r + (1.f - c1.a) * c2.r, 1.f), min(c1.g + (1.f - c1.a) * c2.g, 1.f),
                  min(c1.b + (1.f - c1.a) * c2.b, 1.f), min(c1.a + (1.f - c1.a) * c2.a, 1.f)}; }
    inline RGBA PorterDuffBlendIn  (const RGBA &c1, const RGBA &c2)
        { return {c2.a * c1.r, c2.a * c1.g, c2.a * c1.b, c2.a * c1.a}; }
    inline RGBA PorterDuffBlendOut (const RGBA &c1, const RGBA &c2)
        { return {(1.f - c2.a) * c1.r, (1.f - c2.a) * c1.g, (1.f - c2.a) * c1.b, (1.f - c2.a) * c1.a}; }
    inline RGBA PorterDuffBlendAtop(const RGBA &c1, const RGBA &c2)
        { return {min(c2.a * c1.r + (1.f - c1.a) * c2.r, 1.f), min(c2.a * c1.g + (1.f - c1.a) * c2.g, 1.f),
                  min(c2.a * c1.b + (1.f - c1.a) * c2.b, 1.f), c2.a}; }
    inline RGBA PorterDuffBlendXor (const RGBA &c1, const RGBA &c2)
        { return {min((1.f - c2.a) * c1.r + (1.f - c1.a) * c2.r, 1.f), min((1.f - c2.a) * c1.g + (1.f - c1.a) * c2.g, 1.f),
                  min((1.f - c2.a) * c1.b + (1.f - c1.a) * c2.b, 1.f), min((1.f - c2.a) * c1.a + (1.f - c1.a) * c2.a, 1.f)}; }

    inline RGBA PorterDuffBlendOver(const RGBA &c1, const RGB &c2)
        { return {min(c1.r + (1.f - c1.a) * c2.r, 1.f), min(c1.g + (1.f - c1.a) * c2.g, 1.f),
                  min(c1.b + (1.f - c1.a) * c2.b, 1.f), 1.f}; }
    inline RGBA PorterDuffBlendIn  (const RGBA &c1, const RGB &c2)
        { return c1; }
    inline RGBA PorterDuffBlendOut (const RGBA &c1, const RGB &c2)
        { return {0.f, 0.f, 0.f, 0.f}; }
    inline RGBA PorterDuffBlendAtop(const RGBA &c1, const RGB &c2)
        { return {min(c1.r + (1.f - c1.a) * c2.r, 1.f), min(c1.g + (1.f - c1.a) * c2.g, 1.f),
                  min(c1.b + (1.f - c1.a) * c2.b, 1.f), 1.f}; }
    inline RGBA PorterDuffBlendXor (const RGBA &c1, const RGB &c2)
        { return {(1.f - c1.a) * c2.r, (1.f - c1.a) * c2.g, (1.f - c1.a) * c2.b, 1.f - c1.a}; }

    inline RGBA PorterDuffBlendOver(const RGB &c1, const RGBA &c2)
        { return c1; }
    inline RGBA PorterDuffBlendIn  (const RGB &c1, const RGBA &c2)
        { return {c2.a * c1.r, c2.a * c1.g, c2.a * c1.b, c2.a}; }
    inline RGBA PorterDuffBlendOut (const RGB &c1, const RGBA &c2)
        { return {(1.f - c2.a) * c1.r, (1.f - c2.a) * c1.g, (1.f - c2.a) * c1.b, 1.f - c2.a}; }
    inline RGBA PorterDuffBlendAtop(const RGB &c1, const RGBA &c2)
        { return {c2.a * c1.r, c2.a * c1.g, c2.a * c1.b, c2.a}; }
    inline RGBA PorterDuffBlendXor (const RGB &c1, const RGBA &c2)
        { return {(1.f - c2.a) * c1.r, (1.f - c2.a) * c1.g, (1.f - c2.a) * c1.b, 1.f - c2.a}; }


    //  ============  HSV color model  ============  \\

    template <typename T>
    struct HSVBase {
        T h, s, v;

        HSVBase() {}
        template <class H, class S, class V> HSVBase(H _h, S _s, V _v) : h{_h}, s{_s}, v{_v} {}
        template <typename Y> HSVBase(const Vec3Base<Y> &vec) : h{vec.x}, s{vec.y}, v{vec.z} {}
        Vec3Base<T> toVec() { return {h, s, v}; }
    };

    struct HSV : HSVBase<float> {
        template <class H, class S, class V> HSV(H _h, S _s, V _v) : HSVBase{clamp(float(_h)), clamp(float(_s)), clamp(float(_v))} {}
        HSV(const HSV &_col) : HSVBase{_col.h, _col.s, _col.v} {}
        HSV(const HSVA &_col);
        template <typename T> HSV(const Vec3Base<T> &vec) : HSVBase(clamp(Vec3(vec))) {}
    };


    //  ============  HSVA color model  ============  \\

    template <typename T>
    struct HSVABase {
        T h, s, v, a;

        HSVABase() {}
        template <class H, class S, class V, class A> HSVABase(H _h, S _s, V _v, A _a) : h{_h}, s{_s}, v{_v}, a{_a} {}
        template <typename Y> HSVABase(const Vec4Base<Y> &vec) : h{vec.x}, s{vec.y}, v{vec.z}, a{vec.w} {}
        Vec4Base<T> toVec() { return {h, s, v, a}; }
    };

    struct HSVA : HSVABase<float> {
        template <class H, class S, class V, class A = float > HSVA(H _h, S _s, V _v, A _a = 1.f) :
            HSVABase{clamp(float(_h)), clamp(float(_s)), clamp(float(_v)), clamp(float(_a))} {}
        template <class A = float > HSVA(const HSV &_col, A _a = 1.f) : HSVABase{_col.h, _col.s, _col.v, clamp(_a)} {}
        HSVA(const HSVA &_col) : HSVABase{_col.h, _col.s, _col.v, _col.a} {}
        template <typename T> HSVA(const Vec4Base<T> &vec) : HSVABase(clamp(Vec4(vec))) {}
    };

    HSV::HSV(const HSVA &_col) : HSVBase{_col.h, _col.s, _col.v} {}


    //  ============  XYZ color model  ============  \\

    template <typename T>
    struct XYZBase {
        T x, y, z;
    
        XYZBase() {}
        template <class X, class Y, class Z> XYZBase(X _x, Y _y, Z _z) : x{_x}, y{_y}, z{_z} {}
        template <typename Y> XYZBase(const Vec3Base<Y> &vec) : x{vec.x}, y{vec.y}, z{vec.z} {}
        Vec3Base<T> toVec() { return {x, y, z}; }
    };

    struct XYZ : XYZBase<float> {
        template <class X, class Y, class Z> XYZ(X _x, Y _y, Z _z) : XYZBase{clamp(float(_x)), clamp(float(_y)), clamp(float(_z))} {}
        XYZ(const XYZ &_col) : XYZBase{_col.x, _col.y, _col.z} {}
        XYZ(const XYZA &_col);
        template <typename T> XYZ(const Vec3Base<T> &vec) : XYZBase(clamp(Vec3(vec))) {}
    };


    //  ============  XYZA color model  ============  \\

    template <typename T>
    struct XYZABase {
        T x, y, z, a;
    
        XYZABase() {}
        template <class X, class Y, class Z, class A> XYZABase(X _x, Y _y, Z _z, A _a) : x{_x}, y{_y}, z{_z}, a{_a} {}
        template <typename Y> XYZABase(const Vec4Base<Y> &vec) : x{vec.x}, y{vec.y}, z{vec.z}, a{vec.w} {}
        Vec4Base<T> toVec() { return {x, y, z, a}; }
    };

    struct XYZA : XYZABase<float> {
        template <class X, class Y, class Z, class A = float > XYZA(X _x, Y _y, Z _z, A _a = 1.f) :
            XYZABase{clamp(float(_x)), clamp(float(_y)), clamp(float(_z)), clamp(float(_a))} {}
        template <class A = float > XYZA(const XYZ &_col, A _a = 1.f) : XYZABase{_col.x, _col.y, _col.z, clamp(_a)} {}
        XYZA(const XYZA &_col) : XYZABase{_col.x, _col.y, _col.z, _col.a} {}
        template <typename T> XYZA(const Vec4Base<T> &vec) : XYZABase(clamp(Vec4(vec))) {}
    };

    XYZ::XYZ(const XYZA &_col) : XYZBase{_col.x, _col.y, _col.z} {}

    //  ============  CIELAB (L*a*b*) color model  ============  \\

    template <typename T>
    struct LABBase {
        T l, a, b;
    
        LABBase() {}
        template <class L, class A, class B> LABBase(L _l, A _a, B _b) : l{_l}, a{_a}, b{_b} {}
        template <typename Y> LABBase(const Vec3Base<Y> &vec) : l{vec.x}, a{vec.y}, b{vec.z} {}
        Vec3Base<T> toVec() { return {l, a, b}; }
    };

    struct LAB : LABBase<float> {
        template <class L, class A, class B> LAB(L _l, A _a, B _b) : LABBase{clamp(float(_l)), clamp(float(_a)), clamp(float(_b))} {}
        LAB(const LAB &_col) : LABBase{_col.l, _col.a, _col.b} {}
        LAB(const LABA &_col);
        template <typename T> LAB(const Vec3Base<T> &vec) : LABBase(clamp(Vec3(vec))) {}
    };


    //  ============  CIELAB (L*a*b*) [+ Alpha] color model  ============  \\

    template <typename T>
    struct LABABase {
        T l, a, b, al;

        LABABase() {}
        template <class X, class Y, class Z, class A> LABABase(X _l, Y _a, Z _b, A _al) : l{_l}, a{_a}, b{_b}, al{_al} {}
        template <typename Y> LABABase(const Vec4Base<Y> &vec) : l{vec.x}, a{vec.y}, b{vec.z}, al{vec.w} {}
        Vec4Base<T> toVec() { return {l, a, b, al}; }
    };

    struct LABA : LABABase<float> {
        template <class L, class A, class B, class Al = float > LABA(L _l, A _a, B _b, Al _al = 1.f) :
            LABABase{clamp(float(_l)), clamp(float(_a)), clamp(float(_b)), clamp(float(_al))} {}
        template <class A = float > LABA(const LAB &_col, A _al = 1.f) : LABABase{_col.l, _col.a, _col.b, clamp(float(_al))} {}
        LABA(const LABA &_col) : LABABase{_col.l, _col.a, _col.b, _col.al} {}
        template <typename T> LABA(const Vec4Base<T> &vec) : LABABase(clamp(Vec4(vec))) {}
    };

    LAB::LAB(const LABA &_col) : LABBase{_col.l, _col.a, _col.b} {}


    //  ============  LMS color model  ============  \\

    template <typename T>
    struct LMSBase {
        T l, m, s;

        LMSBase() {}
        template <class L, class M, class S> LMSBase(L _l, M _m, S _s) : l{_l}, m{_m}, s{_s} {}
        template <typename Y> LMSBase(const Vec3Base<Y> &vec) : l{vec.x}, m{vec.y}, s{vec.z} {}
        Vec3Base<T> toVec() { return {l, m, s}; }
    };

    struct LMS : LMSBase<float> {
        template <class L, class A, class B> LMS(L _l, A _m, B _s) : LMSBase{clamp(float(_l)), clamp(float(_m)), clamp(float(_s))} {}
        LMS(const LMS &_col) : LMSBase{_col.l, _col.m, _col.s} {}
        LMS(const LMSA &_col);
        template <typename T> LMS(const Vec3Base<T> &vec) : LMSBase(clamp(Vec3(vec))) {}
    };


    //  ============  LMSA color model  ============  \\

    template <typename T>
    struct LMSABase {
        T l, m, s, a;

        LMSABase() {}
        template <class X, class Y, class Z, class A> LMSABase(X _l, Y _m, Z _s, A _a) : l{_l}, m{_m}, s{_s}, a{_a} {}
        template <typename Y> LMSABase(const Vec4Base<Y> &vec) : l{vec.x}, m{vec.y}, s{vec.z}, a{vec.w} {}
        Vec4Base<T> toVec() { return {l, m, s, a}; }
    };

    struct LMSA : LMSABase<float> {
        template <class L, class M, class S, class A = float > LMSA(L _l, M _m, S _s, A _a = 1.f) :
            LMSABase{clamp(float(_l)), clamp(float(_m)), clamp(float(_s)), clamp(float(_a))} {}
        template <class A = float > LMSA(const LMS &_col, A _a = 1.f) : LMSABase{_col.l, _col.m, _col.s, clamp(float(_a))} {}
        LMSA(const LMSA &_col) : LMSABase{_col.l, _col.m, _col.s, _col.a} {}
        template <typename T> LMSA(const Vec4Base<T> &vec) : LMSABase(clamp(Vec4(vec))) {}
    };

    LMS::LMS(const LMSA &_col) : LMSBase{_col.l, _col.m, _col.s} {}


    //  ============  ICtCp color model  ============  \\

    template <typename C>
    struct ITPBase {
        C i, t, p;

        ITPBase() {}
        template <class I, class T, class P> ITPBase(I _i, T _t, P _p) : i{_i}, t{_t}, p{_p} {}
        template <typename Y> ITPBase(const Vec3Base<Y> &vec) : i{vec.x}, t{vec.y}, p{vec.z} {}
        Vec3Base<C> toVec() { return {i, t, p}; }
    };

    struct ITP : ITPBase<float> {
        template <class I, class T, class P> ITP(I _i, T _t, P _p) : ITPBase{clamp(float(_i)), clamp(float(_t)), clamp(float(_p))} {}
        ITP(const ITP &_col) : ITPBase{_col.i, _col.t, _col.p} {}
        ITP(const ITPA &_col);
        template <typename T> ITP(const Vec3Base<T> &vec) : ITPBase(clamp(Vec3(vec))) {}
    };


    //  ============  ICtCpA color model  ============  \\

    template <typename C>
    struct ITPABase {
        C i, t, p, a;

        ITPABase() {}
        template <class I, class T, class P, class A> ITPABase(I _i, T _t, P _p, A _a) : i{_i}, t{_t}, p{_p}, a{_a} {}
        template <typename Y> ITPABase(const Vec4Base<Y> &vec) : i{vec.x}, t{vec.y}, p{vec.z}, a{vec.w} {}
        Vec4Base<C> toVec() { return {i, t, p, a}; }
    };

    struct ITPA : ITPABase<float> {
        template <class I, class T, class P, class A = float > ITPA(I _i, T _t, P _p, A _a = 1.f) :
            ITPABase{clamp(float(_i)), clamp(float(_t)), clamp(float(_p)), clamp(float(_a))} {}
        template <class A = float > ITPA(const ITP &_col, A _a = 1.f) : ITPABase{_col.i, _col.t, _col.p, clamp(float(_a))} {}
        ITPA(const ITPA &_col) : ITPABase{_col.i, _col.t, _col.p, _col.a} {}
        template <typename T> ITPA(const Vec4Base<T> &vec) : ITPABase(clamp(Vec4(vec))) {}
    };

    ITP::ITP(const ITPA &_col) : ITPBase{_col.i, _col.t, _col.p} {}


    //  ============  Wide gamut RGB color model  ============  \\

    template <typename T>
    struct WideRGBBase {
        T r, g, b;

        WideRGBBase() {}
        template <class R, class G, class B> WideRGBBase(R _r, G _g, B _b) : r{_r}, g{_g}, b{_b} {}
        template <typename Y> WideRGBBase(const Vec3Base<Y> &vec) : r{vec.x}, g{vec.y}, b{vec.z} {}
        Vec3Base<T> toVec() { return {r, g, b}; }
    };

    struct WideRGB : WideRGBBase<float> {
        template <class R, class G, class B> WideRGB(R _r, G _g, B _b) : WideRGBBase{clamp(float(_r)), clamp(float(_g)), clamp(float(_b))} {}
        WideRGB(const WideRGB &_col) : WideRGBBase{_col.r, _col.g, _col.b} {}
        WideRGB(const WideRGBA &_col);
        template <typename T> WideRGB(const Vec3Base<T> &vec) : WideRGBBase(clamp(Vec3(vec))) {}
    };


    //  ============  Wide gamut RGBA color model  ============  \\

    template <typename T>
    struct WideRGBABase {
        T r, g, b, a;

        WideRGBABase() {}
        template <class R, class G, class B, class A> WideRGBABase(R _r, G _g, B _b, A _a) : r{_r}, g{_g}, b{_b}, a{_a} {}
        template <typename Y> WideRGBABase(const Vec4Base<Y> &vec) : r{vec.x}, g{vec.y}, b{vec.z}, a{vec.w} {}
        Vec4Base<T> toVec() { return {r, g, b, a}; }
    };

    struct WideRGBA : WideRGBABase<float> {
        template <class R, class G, class B, class A = float > WideRGBA(R _r, G _g, B _b, A _a = 1.f) :
            WideRGBABase{clamp(float(_r)), clamp(float(_g)), clamp(float(_b)), clamp(float(_a))} {}
        template <class A = float > WideRGBA(const WideRGB &_col, A _a = 1.f) : WideRGBABase{_col.r, _col.g, _col.b, clamp(float(_a))} {}
        WideRGBA(const WideRGBA &_col) : WideRGBABase{_col.r, _col.g, _col.b, _col.a} {}
        template <typename T> WideRGBA(const Vec4Base<T> &vec) : WideRGBABase(clamp(Vec4(vec))) {}
    };

    WideRGB::WideRGB(const WideRGBA &_col) : WideRGBBase{_col.r, _col.g, _col.b} {}


    HSV RGB2HSV(const RGB &rgb) {
        HSV hsv{0.f, 0.f, 0.f};

        unsigned index = rgb.r < rgb.g ? 0u : 1u;
        hsv.s = rgb.b < rgb[index] ? rgb.b: rgb[index]; // C Min
        index = rgb.b < rgb[1u -index] ? 1u - index : 2u;
        hsv.v = rgb[index];                             // V == C Max
        float delta = hsv.v - hsv.s;
        if (hsv.v > 1e-4f) hsv.s = 1.f - hsv.s / hsv.v; // S
        if (delta > 1e-4f) {
            unsigned ind1 = (index + 1u) % 3u, ind2 = (index + 2u) % 3u;
            index = index || rgb.g >= rgb.b ? index : 4u;
            hsv.h = 0.1666666f * ((rgb[ind1] - rgb[ind2]) / delta + (index << 1u)); // H
        }
        return hsv;
    }

    RGB HSV2RGB(const HSV &hsv) {
        RGB rgb{hsv.v, hsv.v, hsv.v};

        float Vmin = (1.f - hsv.s) * hsv.v, H_i;
        float a = (hsv.v - Vmin) * modf(hsv.h * 6.f, &H_i);
        float Vdec = hsv.v - a, Vinc = Vmin + a;
        unsigned Hi = unsigned(H_i);
        Hi = Hi < 6u ? Hi : 5u;

        rgb[(2u + (Hi >> 1)) % 3u] = Vmin;
        if (Hi & 1u) rgb[Hi >> 1] = Vdec;
        else rgb[(1u + (Hi >> 1)) % 3u] = Vinc;

        return rgb;
    }
    

    //  ============  Reference white  ============  \\

    const XYZ D50 = { 0.96422f, 1.f, 0.82521f };
    const XYZ D65 = { 0.95047f, 1.f, 1.08883f };
    const XYZ D502D65 = { 0.9857398f, 1.f, 1.3194581f };
    const XYZ D652D50 = { 1.0144665f, 1.f, 0.7578869f };

    //E
    const Mat3 RGB2XYZ_E { 0.4887180f, 0.3106803f, 0.2006017f,
                           0.1762044f, 0.8129847f, 0.0108109f,
                           0.0f,       0.0102048f, 0.9897952f };
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
                             0.0f,       0.0517813f, 0.7734287f };
    //D50
    const Mat3 XYZ2WideRGB { 1.4628067f,-0.1840623f,-0.2743606f,
                            -0.5217933f, 1.4472381f, 0.0677227f,
                             0.0349342f,-0.0968930f, 1.2884099f };
}