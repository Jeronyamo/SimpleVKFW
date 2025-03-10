#ifndef SOGLFW_DIFFRENDER2D_H
#define SOGLFW_DIFFRENDER2D_H

#include "image.h"
#include "../utilities.h"
#include "../randvec.h"

#include <vector>
#include <string>
#include <unordered_map>

#define PRIM_BORDER 0.003f
#define WAS_EPS 1e-7f


namespace Simple {
    struct Coords2pix {
        Vec2 start, offsetInv;
        UVec2 size;

        UVec2 getPixelIndex(const Vec2 &coords) {
            Vec2 tmp = (coords - start) * offsetInv;
            return {round(tmp.x), round(tmp.y)};
        }
    } transform;

    struct Adam {
        float b1{0.8f}, b2{0.9f}, lr{0.001f}, eps{1e-8f};
        float b1inv{1.f / (1.f - b1)}, b2inv{1.f / (1.f - b2)};

        void update(float &param, float &m, float &v, float &grad) const {
            m = b1 * m + (1.f - b1) * grad;
            v = b2 * v + (1.f - b2) * grad * grad;
            param -= lr * m * b1inv / (sqrt(v * b2inv) + eps);
        }
    };

    struct DiffTextureBase2D {
        virtual void zero_grad() {}
        virtual void step(const Adam &opt, double dmseCoef) {}
        virtual ColorA texcolor(const Vec2 &texcoord, const Color &ref, const float alpha) { return 0.f; }
    };

    struct DiffSingleColor : DiffTextureBase2D {
        ColorA color;
        Vec3 m{0.f}, v{0.f}, Grad{0.f};
        unsigned col_s = 1u;

        DiffSingleColor(const ColorA &col) : color{col} {}

        void zero_grad() { Grad = 0.f; col_s = 1u; }
        void step(const Adam &opt, double dmseCoef) {
            Grad /= col_s;
            opt.update(color.x, m.x, v.x, Grad.x);
            opt.update(color.y, m.y, v.y, Grad.y);
            opt.update(color.z, m.z, v.z, Grad.z);
            color = color;
        }
        ColorA texcolor(const Vec2 &texcoord, const Color &ref, const float alpha)
            {   if (alpha) ++col_s;
                Grad += alpha * Vec3(color.x - ref.x, color.y - ref.y, color.z - ref.z); return color; }
    };

    struct DiffTexture2D : DiffTextureBase2D {
        Image4 tex;
        Vec3 *m, *v, *Grad;
        unsigned *indices;

        DiffTexture2D(const int w, const int h, const ColorA &col = {0.f, 0.f, 0.f, 1.f}) :
                    tex(w, h), m(new Vec3[w * h]), v(new Vec3[w * h]), Grad(new Vec3[w * h]), indices(new unsigned[w * h]) {
            for (auto *ptr = tex.image; ptr < (tex.image + w * h);)
                *ptr++ = col;
        }
        ~DiffTexture2D() {
            if (m != nullptr) delete[] m;
            if (v != nullptr) delete[] v;
            if (Grad != nullptr) delete[] Grad;
            if (indices != nullptr) delete[] indices;
        }

        unsigned params() { return tex.width * tex.height * 3; }
        void zero_grad() { memset(Grad, 0, tex.width * tex.height * sizeof(Vec3));
                           memset(indices, 0, tex.width * tex.height * sizeof(unsigned)); }
        void step(const Adam &opt, double dmseCoef) {
            for (unsigned i = 0u; i < tex.width * tex.height; ++i) {
                Grad[i] /= indices[i] ? indices[i] : 1u;
                opt.update(tex.image[i].x, m[i].x, v[i].x, Grad[i].x);
                opt.update(tex.image[i].y, m[i].y, v[i].y, Grad[i].y);
                opt.update(tex.image[i].z, m[i].z, v[i].z, Grad[i].z);
                tex.image[i] = tex.image[i];
            }
        }

        ColorA texcolor(const Vec2 &texcoord, const Color &ref, const float alpha) {
            if (texcoord.x < 0.f || texcoord.y > 1.f || texcoord.y < 0.f || texcoord.y > 1.f) return {0.f, 0.f, 0.f, 1.f};    ////////////////////////
            Vec2 uv = texcoord * Vec2(tex.width, tex.height);
            UVec2 ind = uv;
            uv -= ind;

            Color res{0.f};
            Vec4 coefs{uv.x * uv.y, (1.f - uv.x) * uv.y, uv.x * (1.f - uv.y), (1.f - uv.x) * (1.f - uv.y)};
            Vec3 tmpres{res.x - ref.x, res.y - ref.y, res.z - ref.z};
            if (ind.x < tex.width && ind.y < tex.height) {
                Color tmpres = tex[ind];
                Grad[ind.y * tex.width + ind.x] += alpha * Vec3(tmpres.x - ref.x, tmpres.y - ref.y, tmpres.z - ref.z);
                res += coefs.x * tmpres;
                ++indices[ind.y * tex.width + ind.x];
                if (ind.x + 1 < tex.width) {
                    tmpres = tex[UVec2(ind.x + 1, ind.y)];
                    Grad[ind.y * tex.width + ind.x + 1] += alpha * Vec3(tmpres.x - ref.x, tmpres.y - ref.y, tmpres.z - ref.z);
                    res += coefs.y * tmpres;
                    ++indices[ind.y * tex.width + ind.x + 1];
                }
                if (ind.y + 1 < tex.height) {
                    tmpres = tex[UVec2(ind.x, ind.y + 1)];
                    Grad[(ind.y + 1) * tex.width + ind.x] += alpha * Vec3(tmpres.x - ref.x, tmpres.y - ref.y, tmpres.z - ref.z);
                    res += coefs.z * tmpres;
                    ++indices[(ind.y + 1) * tex.width + ind.x];
                }
                if (ind.x + 1 < tex.width && ind.y + 1 < tex.height) {
                    tmpres = tex[UVec2(ind.x + 1, ind.y + 1)];
                    Grad[(ind.y + 1) * tex.width + ind.x + 1] += alpha * Vec3(tmpres.x - ref.x, tmpres.y - ref.y, tmpres.z - ref.z);
                    res += coefs.w * tmpres;
                    ++indices[(ind.y + 1) * tex.width + ind.x + 1];
                }
            }
            return res;
        }
    };

    struct DiffTexBaseMakerInterface {
        DiffTextureBase2D *ptr = nullptr;

        DiffTexBaseMakerInterface(DiffTextureBase2D *_ptr) : ptr{_ptr} {}
        DiffTexBaseMakerInterface(DiffTexBaseMakerInterface &_texbase) : ptr{_texbase.ptr} {}
        DiffTexBaseMakerInterface(const int w, const int h) : ptr{new DiffTexture2D{w, h}} {}
        DiffTexBaseMakerInterface(const ColorA &_col) : ptr{new DiffSingleColor{_col}} {}
    };


    struct DiffPrimitive2D {
        DiffTextureBase2D *tex;

        DiffPrimitive2D(DiffTexBaseMakerInterface _tex) : tex{_tex.ptr} {}
        ~DiffPrimitive2D() { if (tex != nullptr) delete tex; }

        virtual void zero_grad() { tex->zero_grad(); }
        virtual void step(const Adam &opt, double dmseCoef, const Image4 &res, const Image4 &ref) {}
        virtual ColorA intersect(const Vec2 &pt, const Image4 &ref, const unsigned flatten, const float alpha) = 0;
    };

    struct DiffCircle2D : DiffPrimitive2D {
        Vec2 center;
        float radius, invrad;
        Vec2 dC{0.f}, Cm{0.f}, Cv{0.f};
        float dR{0.f}, Rm{0.f}, Rv{0.f};
        unsigned edge_s = 0u;
        bool do_update = true;

        DiffCircle2D(const Vec2 &_center, float _rad, DiffTexBaseMakerInterface texptr, bool _upd = true) :
                center{_center}, radius{_rad}, invrad{0.5f / _rad - 0.01f}, DiffPrimitive2D{texptr}, do_update(_upd) {}

        virtual void zero_grad() {
            tex->zero_grad();
            dC = 0.f;
            dR = 0.f;
            edge_s = 0u;
        }
        virtual void step(const Adam &opt, double dmseCoef, const Image4 &res, const Image4 &ref) {
            tex->step(opt, dmseCoef);

            float fullLen = 6.28318f * radius;
            unsigned samplesN = 1000u * fullLen;
            edge_s = samplesN;

            for (unsigned i = 0u; i < samplesN; ++i) {
                float t = 6.28318f * vecRandfEngine(vecRandEngine);
                Vec2 norm{cos(t), sin(t)};
                Vec2 pt{center + radius * norm};

                UVec2 uv1{transform.getPixelIndex(pt - 0.003f * norm)},
                      uv2{transform.getPixelIndex(pt + 0.003f * norm)};
                ColorA delta = res[uv1], delta2 = res[uv2];

                UVec2 _ind{transform.getPixelIndex(pt)};
                ColorA I = res[uv1], Ref = ref[uv1];
                float dMSE = dot(Vec3(delta.x - delta2.x, delta.y - delta2.y, delta.z - delta2.z),
                                 Vec3(I.x - Ref.x, I.y - Ref.y, I.z - Ref.z));
                dR += dMSE;
                dC += norm * dMSE;
            }
            std::cout << "Circle:\nRadius: " << radius << std::endl;
            std::cout << "Center: " << center << "; " << radius << std::endl;
            dC /= edge_s ? edge_s : 1u;
            dR /= edge_s ? edge_s : 1u;
            opt.update(center.x, Cm.x, Cv.x, dC.x);
            opt.update(center.y, Cm.y, Cv.y, dC.y);
            opt.update(radius, Rm, Rv, dR);
            if (radius < 0.01f) radius = 0.01f; // artificial, but necessary
        }

        ColorA intersect(const Vec2 &pt, const Image4 &ref, const unsigned flatten, const float alpha) {
            Vec2 l = (pt - center);
            float n = l.norm();

            if (n <= radius) {
                //ColorA rescol = tex->texcolor(0.5f + l / (2 * radius + 0.0002f), ref.image[flatten], alpha);
                return tex->texcolor(0.5f + l / (2 * radius + 0.0002f), ref.image[flatten], alpha);
            }
            return {0.f};
        }
    };

    struct DiffTriangle2D : DiffPrimitive2D {
        Vec2 a, b, c;
        Vec2 dA{0.f}, dB{0.f}, dC{0.f};
        Vec2 dAm{0.f}, dBm{0.f}, dCm{0.f};
        Vec2 dAv{0.f}, dBv{0.f}, dCv{0.f};
        Vec2 texCoord1, texCoord2, texCoord3;
        UVec3 vert_s{1u};


        DiffTriangle2D(const Vec2 &_a, const Vec2 &_b, const Vec2 &_c, DiffTexBaseMakerInterface texptr,
                   const Vec2 &_t1 = {0.f}, const Vec2 &_t2 = {0.f}, const Vec2 &_t3 = {0.f}) :
                a{_a}, b{_b}, c{_c}, DiffPrimitive2D{texptr}, texCoord1{_t1}, texCoord2{_t2}, texCoord3{_t3} {}


        inline float PointLineDistance2D(const Vec2 &point, const Vec2 &A, const Vec2 &B) {
            return abs((B.x - A.x) * (A.y - point.y) - (A.x - point.x) * (B.y - A.y)) /
                  (B - A).norm();
        }

        bool PointTriangleTest2D(Vec2 point, Vec2 &_uv) {
            float d = 1.f / ((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));
            Vec3 uv{0.f};
            uv.x = ((b.y - c.y) * (point.x - c.x) + (c.x - b.x) * (point.y - c.y)) * d;
            uv.y = ((c.y - a.y) * (point.x - c.x) + (a.x - c.x) * (point.y - c.y)) * d;
            uv.z = 1.f - uv.x - uv.y;

            _uv = point;
            if (texCoord1.x != texCoord2.x || texCoord2.x != texCoord3.x || texCoord1.y != texCoord2.y || texCoord2.y != texCoord3.y)
                _uv = uv.x * texCoord1 + uv.y * texCoord2 + uv.z * texCoord3;

            return uv.x >= 0.f && uv.x <= 1.f && uv.y >= 0.f && uv.y <= 1.f && uv.z >= 0.f && uv.z <= 1.f;
        }

        Vec2 normal(unsigned edge) {
            Vec2 v1{edge < 2 ? a : b}, v2{edge ? c : b}, ref{*((Vec2*)this + (2 - edge))};
            Vec2 norm = (v1 - v2)["yx"];
            norm.x = -norm.x;
            return dot(norm, ref) < 0 ? norm : -norm;
        }

        float PointEdgesTest2D(Vec2 point) {
            Vec3 edges_dist{ PointLineDistance2D(point, a, b),
                             PointLineDistance2D(point, a, c),
                             PointLineDistance2D(point, b, c) };

            unsigned ind = edges_dist.x > edges_dist.y;
            return edges_dist[edges_dist[ind] < edges_dist.z ? ind : 2u];
        }

        void zero_grad() {
            tex->zero_grad();
            dA = 0.f;
            dB = 0.f;
            dC = 0.f;
            vert_s = 0u;
        }

        void edgeSampling(int index, float t, const Image4 &res, const Image4 &ref) {
            const Vec2 v1{index ? ((index & 1u) ? b : c) : a}, v2{index ? ((index & 1u) ? c : a) : b};
            Vec2 &dV1{index ? ((index & 1u) ? dB : dC) : dA}, &dV2{index ? ((index & 1u) ? dC : dA) : dB};
            Vec2 pt = v1 + t * (v2 - v1);
            Vec2 norm = (v1 - v2)["yx"].normalize();
            norm.x = -norm.x;
            norm = dot(norm, *((Vec2*)this + (2 - index)) - pt) <= 0 ? norm : -norm;

            UVec2 uv1{transform.getPixelIndex(pt - 0.003f * norm)},
                  uv2{transform.getPixelIndex(pt + 0.003f * norm)};
            uv1.x = uv1.x >= transform.size.x ? transform.size.x - 1 : uv1.x;
            uv1.y = uv1.y >= transform.size.y ? transform.size.y - 1 : uv1.y;
            uv2.x = uv2.x >= transform.size.x ? transform.size.x - 1 : uv2.x;
            uv2.y = uv2.y >= transform.size.y ? transform.size.y - 1 : uv2.y;
            //std::cout << "STR1" << std::endl;
            ColorA delta = res[uv1], delta2 = res[uv2];

            UVec2 _ind{transform.getPixelIndex(pt)};
            ColorA I = res[uv1], Ref = ref[uv1];
            //std::cout << "STR2" << std::endl;
            float dMSE = dot(Vec3(delta.x - delta2.x, delta.y - delta2.y, delta.z - delta2.z),
                                Vec3(I.x - Ref.x, I.y - Ref.y, I.z - Ref.z));

            dV1 += (1 - t) * norm * dMSE / (v2 - v1).norm();
            dV2 += t * norm * dMSE / (v2 - v1).norm();
        }

        void step(const Adam &opt, double dmseCoef, const Image4 &res, const Image4 &ref) {
            tex->step(opt, dmseCoef);

            Vec3 edgelens{(b - a).norm(), (c - a).norm(), (c - b).norm()};
            float fullLen = (edgelens.x + edgelens.y + edgelens.z);
            unsigned samplesN = 1000u * fullLen;
            for (unsigned i = 0u; i < samplesN; ++i) {
                float t = vecRandfEngine(vecRandEngine) * fullLen;
                if (t >= (edgelens.x + edgelens.y)) {
                    this->edgeSampling(2, (t - (edgelens.x + edgelens.y)) / edgelens.z, res, ref);
                    vert_s["yz"] += 1;
                }
                else if (t >= edgelens.x) {
                    this->edgeSampling(1, (t - edgelens.x) / edgelens.y, res, ref);
                    vert_s["xz"] += 1;
                }
                else {
                    this->edgeSampling(0, t / edgelens.x, res, ref);
                    vert_s["xy"] += 1;
                }
            }

            dA /= vert_s.x ? vert_s.x : 1u;
            dB /= vert_s.y ? vert_s.y : 1u;
            dC /= vert_s.z ? vert_s.z : 1u;
            dA *= 0.1f;
            dB *= 0.1f;
            dC *= 0.1f;
            opt.update(a.x, dAm.x, dAv.x, dA.x);
            opt.update(a.y, dAm.y, dAv.y, dA.y);
            opt.update(b.x, dBm.x, dBv.x, dB.x);
            opt.update(b.y, dBm.y, dBv.y, dB.y);
            opt.update(c.x, dCm.x, dCv.x, dC.x);
            opt.update(c.y, dCm.y, dCv.y, dC.y);
            a.x = a.x <= 0.0001f ? 0.0001f : (a.x >= 0.9999f ? 0.9999f : a.x);
            a.y = a.y <= 0.0001f ? 0.0001f : (a.y >= 0.9999f ? 0.9999f : a.y);
            b.x = b.x <= 0.0001f ? 0.0001f : (b.x >= 0.9999f ? 0.9999f : b.x);
            b.y = b.y <= 0.0001f ? 0.0001f : (b.y >= 0.9999f ? 0.9999f : b.y);
            c.x = c.x <= 0.0001f ? 0.0001f : (c.x >= 0.9999f ? 0.9999f : c.x);
            c.y = c.y <= 0.0001f ? 0.0001f : (c.y >= 0.9999f ? 0.9999f : c.y);
            std::cout << "Triangle pos: " << a << ", " << b << ", " << c << std::endl;
        }

        ColorA intersect(const Vec2 &pt, const Image4 &ref, const unsigned flatten, const float alpha) {
            Vec2 uv{0.f};
            if (PointTriangleTest2D(pt, uv)) {
                // INSIDE
                return tex->texcolor(uv, ref.image[flatten], alpha);
            }
            return {0.f};
        }
    };


    struct DiffSampler2D {
        Vec2 pixelPt, pixelSize;
        unsigned count;

        DiffSampler2D(const Vec2 &_pixel, const Vec2 &_pixSize, const unsigned _count) :
                pixelPt{_pixel}, pixelSize{_pixSize}, count{_count} {}
        DiffSampler2D(const DiffSampler2D &_s) : pixelPt{_s.pixelPt}, pixelSize{_s.pixelSize}, count{_s.count} {}

        DiffSampler2D& operator++() { --count; return *this; }
        DiffSampler2D  operator++(int) { DiffSampler2D _ret = *this; --count; return _ret; }
        bool operator==(const DiffSampler2D &_rc) const { return !count; }
        bool operator!=(const DiffSampler2D &_rc) const { return  count; }
        Vec2 operator*() const { return pixelPt + pixelSize * rand2f(); }
        DiffSampler2D begin() { return *this; }
        DiffSampler2D end() { return *this; }
    };





const bool edge_sampling = 0, basic = 0, wa_sampling = 1;





std::unordered_map<std::string, unsigned> param_count;
std::vector <std::string> par_keys{"1", "2", "3", "4", "5", "6", "res_dot", //"res_diff", "res_f",
                                    "kdk_sum", "res_wfieldxy", "res_wfieldz", "res_dbg", "v", "first", "second"//, "res_dfdp"
                                  };
    struct DiffTriangleWAS2D : DiffPrimitive2D {
        Vec2 a, b, c;
        Vec2 dA{0.f}, dB{0.f}, dC{0.f};
        Vec2 dAm{0.f}, dBm{0.f}, dCm{0.f};
        Vec2 dAv{0.f}, dBv{0.f}, dCv{0.f};
        Vec2 texCoord1, texCoord2, texCoord3;
        UVec3 vert_s{1u};


        DiffTriangleWAS2D(const Vec2 &_a, const Vec2 &_b, const Vec2 &_c, DiffTexBaseMakerInterface texptr,
                   const Vec2 &_t1 = {0.f}, const Vec2 &_t2 = {0.f}, const Vec2 &_t3 = {0.f}) :
                a{_a}, b{_b}, c{_c}, DiffPrimitive2D{texptr}, texCoord1{_t1}, texCoord2{_t2}, texCoord3{_t3} {}


        inline float PointLineDistance2D(const Vec2 &point, const Vec2 &A, const Vec2 &B) {
            return abs((B.x - A.x) * (A.y - point.y) - (A.x - point.x) * (B.y - A.y)) /
                  (B - A).norm();
        }

        bool PointTriangleTest2D(Vec2 point, Vec2 &_uv) {
            float d = 1.f / ((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));
            Vec3 uv{0.f};
            uv.x = ((b.y - c.y) * (point.x - c.x) + (c.x - b.x) * (point.y - c.y)) * d;
            uv.y = ((c.y - a.y) * (point.x - c.x) + (a.x - c.x) * (point.y - c.y)) * d;
            uv.z = 1.f - uv.x - uv.y;

            _uv = {uv.x, uv.y};
            //_uv = {clampCol(_uv.x), clampCol(_uv.y)};

            return uv.x >= 0.f && uv.x <= 1.f && uv.y >= 0.f && uv.y <= 1.f && uv.z >= 0.f && uv.z <= 1.f;
        }

        bool PointTriangleTestTexCoords2D(Vec2 point, Vec2 &_uv) {
            float d = 1.f / ((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));
            Vec3 uv{0.f};
            uv.x = ((b.y - c.y) * (point.x - c.x) + (c.x - b.x) * (point.y - c.y)) * d;
            uv.y = ((c.y - a.y) * (point.x - c.x) + (a.x - c.x) * (point.y - c.y)) * d;
            uv.z = 1.f - uv.x - uv.y;

            _uv = uv.x * texCoord1 + uv.y * texCoord2 + uv.z * texCoord3;
            //_uv = {clampCol(_uv.x), clampCol(_uv.y)};

            return uv.x >= 0.f && uv.x <= 1.f && uv.y >= 0.f && uv.y <= 1.f && uv.z >= 0.f && uv.z <= 1.f;
        }

        Vec2 normal(unsigned edge) {
            Vec2 v1{edge < 2 ? a : b}, v2{edge ? c : b}, ref{*((Vec2*)this + (2 - edge))};
            Vec2 norm = (v1 - v2)["yx"];
            norm.x = -norm.x;
            return dot(norm, ref) < 0 ? norm : -norm;
        }

        float PointEdgesTest2D(Vec2 point) {
            Vec3 edges_dist{ PointLineDistance2D(point, a, b),
                             PointLineDistance2D(point, a, c),
                             PointLineDistance2D(point, b, c) };

            unsigned ind = edges_dist.x > edges_dist.y;
            return edges_dist[edges_dist[ind] < edges_dist.z ? ind : 2u];
        }

        void zero_grad() {
            tex->zero_grad();
            dA = 0.f;
            dB = 0.f;
            dC = 0.f;
            vert_s = 0u;
            for (auto key : par_keys)
                param_count[key] = 0u;
        }

        void step(const Adam &opt, double dmseCoef, const Image4 &res, const Image4 &ref) {
            //tex->step(opt, dmseCoef);
            dA /= vert_s.x ? vert_s.x : 1u;
            dB /= vert_s.y ? vert_s.y : 1u;
            dC /= vert_s.z ? vert_s.z : 1u;
            //dA = -dA;
            //dB = -dB;
            //dC = -dC;
            
            opt.update(a.x, dAm.x, dAv.x, dA.x);
            opt.update(a.y, dAm.y, dAv.y, dA.y);
            opt.update(b.x, dBm.x, dBv.x, dB.x);
            opt.update(b.y, dBm.y, dBv.y, dB.y);
            opt.update(c.x, dCm.x, dCv.x, dC.x);
            opt.update(c.y, dCm.y, dCv.y, dC.y);
            a.x = a.x <= 0.0001f ? 0.0001f : (a.x >= 0.9999f ? 0.9999f : a.x);
            a.y = a.y <= 0.0001f ? 0.0001f : (a.y >= 0.9999f ? 0.9999f : a.y);
            b.x = b.x <= 0.0001f ? 0.0001f : (b.x >= 0.9999f ? 0.9999f : b.x);
            b.y = b.y <= 0.0001f ? 0.0001f : (b.y >= 0.9999f ? 0.9999f : b.y);
            c.x = c.x <= 0.0001f ? 0.0001f : (c.x >= 0.9999f ? 0.9999f : c.x);
            c.y = c.y <= 0.0001f ? 0.0001f : (c.y >= 0.9999f ? 0.9999f : c.y);
            std::cout << "WAS Triangle pos: " <<  a << "; " <<  b << "; " <<  c << std::endl;
            std::cout << "WAS Triangle  dS: " << dA << "; " << dB << "; " << dC << std::endl;
            //for (auto key : par_keys) std::cout << key << ": " << param_count[key] << std::endl;
        }



        Vec2 WASdp(unsigned param, const Vec2 &uv) {
            switch (param) {
                case (0): { return {uv.x, 0}; }
                case (1): { return {0, uv.x}; }
                case (2): { return {uv.y, 0}; }
                case (3): { return {0, uv.y}; }
                case (4): { return {1.f - uv.x - uv.y, 0}; }
                case (5): { return {0, 1.f - uv.x - uv.y}; }
                default: {
                    throw std::runtime_error("Error: wrong dp parameter index");
                }
            }
        }

        float & WASdS(unsigned param) {
            switch (param) {
                case (0): { return dA.x; }
                case (1): { return dA.y; }
                case (2): { return dB.x; }
                case (3): { return dB.y; }
                case (4): { return dC.x; }
                case (5): { return dC.y; }
                default: {
                    throw std::runtime_error("Error: wrong dS parameter index");
                }
            }
        }

        Vec3 WASkdk(const Vec2 &p, const Vec2 &_p) {
            Vec2 p_{p - _p};
            float rho = sqrt(p_.x * p_.x + p_.y * p_.y), k = this->PointEdgesTest2D(_p);
            rho = rho > WAS_EPS ? rho : WAS_EPS;
            return {
                1.f / (rho + k),
                p_ / (rho * sqr(rho + k))
            };
        }

// for reference, loops should be merged
        Vec2 WASv(const std::vector<Vec2> &uv_i, const std::vector<Vec3> &kdk_i,
                  Vec3 kdk_sum, const unsigned param, const unsigned Nsamples) {
            Vec2 v{0.f};
            for (unsigned i = 0u; i < Nsamples; ++i) {
                v += kdk_i[i].x * this->WASdp(param, uv_i[i]);
            }
            Vec2 Res{kdk_sum.x > WAS_EPS ? v / kdk_sum.x : v / WAS_EPS};
            return Res;
        }

// for reference, loops should be merged
        float WASdivv(const std::vector<Vec2> &uv_i, const std::vector<Vec3> &kdk_i,
                      Vec3 kdk_sum, const unsigned param, const unsigned Nsamples) {
            float first = 0.f, second = 0.f;

            for (unsigned i = 0u; i < Nsamples; ++i) {
                const Vec2 dp = this->WASdp(param, uv_i[i]);
                first += dot(dp, Vec3(kdk_i[i])["yz"]);
                second += kdk_i[i].x * dot(dp, kdk_sum["yz"]);
            }
            kdk_sum.x = kdk_sum.x > WAS_EPS ? 1.f / kdk_sum.x : 1.f / WAS_EPS;
            return first * kdk_sum.x - second * kdk_sum.x * kdk_sum.x;
        }

        std::vector<Vec3> WASvDivv(const Vec2 &p, const unsigned Nsamples, const unsigned Nparams, float _rad) {
            std::vector<Vec3> v_n(Nparams);
            std::vector<Vec2> p_i;  // Nsamples
            std::vector<Vec2> uv_i; // Nsamples

            for (auto _par : DiffSampler2D({0.f}, {0.05f, 2 * M_PI}, Nsamples)) {
                    p_i.push_back({p + _par.x * Vec2(sin(_par.y), cos(_par.y))});
                    if ((p - p_i.back()).norm() > _rad + 1e-4f) {
                        std::cout << p << "; " << p_i.back() << "; " << (p - p_i.back()).norm() << " > " << _rad << std::endl;
                        throw std::runtime_error("Error: bad sample");
                    }
                    uv_i.push_back(0);
                    this->PointTriangleTest2D(p_i.back(), uv_i.back());
            }

            for (unsigned n = 0u; n < Nparams; ++n) {
                std::vector<Vec3>kdk_i;
                Vec3 kdk_sum{0.f};
                for (unsigned i = 0u; i < Nsamples; ++i) {
                    kdk_i.push_back(this->WASkdk(p, p_i[i]));
                    kdk_sum += kdk_i.back();
                }

                Vec2 v{0.f};
                float first = 0.f, second = 0.f;
                for (unsigned i = 0u; i < Nsamples; ++i) {
                    Vec2 dp{this->WASdp(n, uv_i[i])};
                    first += dot(kdk_i[i]["yz"], dp);
                    second += kdk_i[i].x * dot(dp, kdk_sum["yz"]);
                    v += kdk_i[i].x * dp;
                }
                kdk_sum.x = kdk_sum.x > WAS_EPS ? 1.f / kdk_sum.x : 1.f / WAS_EPS;
                v_n[n] = {v * kdk_sum.x, first * kdk_sum.x - second * kdk_sum.x * kdk_sum.x};
                if (n < 4u) {
                    //param_count["v"] += !notCloseToZero(v.x) && !notCloseToZero(v.y);
                    //param_count["first"] += !notCloseToZero(first);
                    //param_count["second"] += !notCloseToZero(second);
                    //param_count["res_wfieldxy"] += !notCloseToZero(v_n[n].x) && !notCloseToZero(v_n[n].y);
                    //param_count["res_wfieldz"] += !notCloseToZero(v_n[n].z);
                    //param_count["res_dbg"] += 1u;
                    //param_count["kdk_sum"] += !notCloseToZero(kdk_sum.y) || !notCloseToZero(kdk_sum.z);
                }
            }
            return v_n;
        }

        ColorA intersect(const Vec2 &pt, const Image4 &ref, const unsigned flatten, const float alpha) {
            Vec2 _uv{0.f};
            if (PointTriangleTestTexCoords2D(pt, _uv)) {
                if (basic) {
                    BVec2 _da, _db, _dc;
                    Vec2 __uv;
                    {
                        a.x += 1.f / 256.f;
                        _da.x = PointTriangleTest2D(pt, __uv);
                        a.x -= 1.f / 256.f;
                    }
                    {
                        a.y += 1.f / 256.f;
                        _da.y = PointTriangleTest2D(pt, __uv);
                        a.y -= 1.f / 256.f;
                    }
                    {
                        b.x += 1.f / 256.f;
                        _db.x = PointTriangleTest2D(pt, __uv);
                        b.x -= 1.f / 256.f;
                    }
                    {
                        b.y += 1.f / 256.f;
                        _db.y = PointTriangleTest2D(pt, __uv);
                        b.y -= 1.f / 256.f;
                    }
                    {
                        c.x += 1.f / 256.f;
                        _dc.x = PointTriangleTest2D(pt, __uv);
                        c.x -= 1.f / 256.f;
                    }
                    {
                        c.y += 1.f / 256.f;
                        _dc.y = PointTriangleTest2D(pt, __uv);
                        c.y -= 1.f / 256.f;
                    }
                    vert_s += 1;
                    dA += 1.f - _da;
                    dB += 1.f - _db;
                    dC += 1.f - _dc;
                    return {1.f - _dc.x, 1.f - _dc.y, 0.f, 1.f};
                }
                else if (edge_sampling) {
                    
                }
                else if (wa_sampling) {
                    const unsigned Nsamples = 8u, Nparams = 6u;
                    float sampleR = 1.f;
                    ColorA _ref{ref.image[flatten]};
                    ColorA _res{tex->texcolor(_uv, ref.image[flatten], alpha)};
                    const Vec3 _diff{_res.x - _ref.x, _res.y - _ref.y, _res.z - _ref.z};
                    const Vec3 f{1.f};
                    //const Vec3 f{_res.x, _res.y, _res.z};
                    Vec2 dfdp[3];

                    {
                        float eps = 0.01f;
                        Vec2 _p{pt + Vec2(eps, 0.f)}, tmpuv{0.f};
                        float flag = PointTriangleTest2D(_p, tmpuv);
                        ColorA Xplus {flag};
                        //ColorA Xplus {tex->texcolor(tmpuv, ref.image[flatten], 0.f) * 1};
                        _p = pt - Vec2(eps, 0.f);
                        flag = PointTriangleTest2D(_p, tmpuv);
                        ColorA Xminus{flag};
                        //ColorA Xminus{tex->texcolor(tmpuv, ref.image[flatten], 0.f) * 1};
                        _p = pt + Vec2(0.f, eps);
                        flag = PointTriangleTest2D(_p, tmpuv);
                        ColorA Yplus {flag};
                        //ColorA Yplus {tex->texcolor(tmpuv, ref.image[flatten], 0.f) * 1};
                        _p = pt - Vec2(0.f, eps);
                        flag = PointTriangleTest2D(_p, tmpuv);
                        ColorA Yminus{flag};
                        //ColorA Yminus{tex->texcolor(tmpuv, ref.image[flatten], 0.f) * 1};

                        dfdp[0] = { Xplus.r - Xminus.r, Yplus.r - Yminus.r };
                        dfdp[1] = { Xplus.g - Xminus.g, Yplus.g - Yminus.g };
                        dfdp[2] = { Xplus.b - Xminus.b, Yplus.b - Yminus.b };

                        Vec2 ba = b - a, ca = c - a;
                        Vec2 len{ba.norm(), ca.norm()};
                        sampleR *=  len.x * len.y * sqrt(1.f + sqr(dot(ba, ca) / (len.x * len.y)));
                        sampleR /= (len.x + len.y + (c - b).norm());
                    }

                    std::vector<Vec3> wfield = this->WASvDivv(pt, Nsamples, Nparams, sampleR);
                    float dervs[Nparams];
                    for (unsigned i = 0u; i < 3; ++i)
                        for (unsigned j = 0u; j < Nparams; ++j) {
                            float Res = _diff[i] * (dot(dfdp[i], wfield[j]["xy"]) + f[i] * wfield[j].z);
                            if (!notCloseToZero(Res) && j < 4u) {
                                param_count[par_keys[j]] += 1u;
                                //param_count["res_dot"] += !notCloseToZero(dot(dfdp[i], wfield[j]["xy"]));
                                //param_count["res_dfdp"] += !notCloseToZero(dfdp[i].x) && !notCloseToZero(dfdp[i].y);
                                //param_count["res_diff"] += !notCloseToZero(_diff[i]);
                                //param_count["res_f"] += !notCloseToZero(f[i]);
                            }
                            this->WASdS(j) += Res;
                            dervs[i] = Res;
                        }
                    vert_s += 1;
                    return {dervs[4], dervs[5], -dervs[4], 1.f};
                    //return _res;
                }
            } // if in triangle
            else if (basic) {
                    BVec2 _da, _db, _dc;
                    Vec2 __uv;
                    {
                        a.x += 1.f / 256.f;
                        _da.x = PointTriangleTest2D(pt, __uv);
                        a.x -= 1.f / 256.f;
                    }
                    {
                        a.y += 1.f / 256.f;
                        _da.y = PointTriangleTest2D(pt, __uv);
                        a.y -= 1.f / 256.f;
                    }
                    {
                        b.x += 1.f / 256.f;
                        _db.x = PointTriangleTest2D(pt, __uv);
                        b.x -= 1.f / 256.f;
                    }
                    {
                        b.y += 1.f / 256.f;
                        _db.y = PointTriangleTest2D(pt, __uv);
                        b.y -= 1.f / 256.f;
                    }
                    {
                        c.x += 1.f / 256.f;
                        _dc.x = PointTriangleTest2D(pt, __uv);
                        c.x -= 1.f / 256.f;
                    }
                    {
                        c.y += 1.f / 256.f;
                        _dc.y = PointTriangleTest2D(pt, __uv);
                        c.y -= 1.f / 256.f;
                    }
                    vert_s.x += _da.x || _da.y;
                    vert_s.y += _db.x || _db.y;
                    vert_s.z += _dc.x || _dc.y;
                    dA -= _da;
                    dB -= _db;
                    dC -= _dc;
                    return {0.f, -_dc.y, _dc.x, 1.f};
            }
            return {0.f};
        }
    };




    struct DiffSDFImage : DiffPrimitive2D {
        DiffSDFImage(DiffTexBaseMakerInterface texptr) : DiffPrimitive2D(texptr) {}

        void zero_grad() {
            tex->zero_grad();
        }
        void step(const Adam &opt, double dmseCoef, const Image4 &res, const Image4 &ref) {
            tex->step(opt, dmseCoef);
        }
        ColorA intersect(const Vec2 &pt, const Image4 &ref, const unsigned flatten, const float alpha) {
            return tex->texcolor(pt, ref.image[flatten], alpha);
        }
    };



    struct DiffRayCaster2D {
        Vec2 retval;
        UVec2 curr, size;
        Vec2 start, offset;

        DiffRayCaster2D(const UVec2 &_size, const Vec2 &_start, const Vec2 &_stop) :
            retval{_start}, curr{0u}, size{_size}, start{_start}, offset{(_stop - _start) / _size} {}
        DiffRayCaster2D(const Vec2 &_start, const Vec2 &_offset, const UVec2 &_size) :
            retval{_start}, curr{0u}, size{_size}, start{_start}, offset{_offset} {}
        DiffRayCaster2D(const DiffRayCaster2D &_rc) : retval{_rc.retval}, curr{_rc.curr}, size{_rc.size}, start{_rc.start}, offset{_rc.offset} {}

        DiffRayCaster2D& operator++() {
            curr.x = ++curr.x == size.x ? 0u : curr.x;
            curr.y += !curr.x;
            retval = curr.x ? Vec2(retval.x + offset.x, retval.y) : Vec2(start.x, retval.y + offset.y);
            return *this;
        }
        DiffRayCaster2D  operator++(int) {DiffRayCaster2D _ret = *this; ++(*this); return _ret;}
        bool operator==(const DiffRayCaster2D &_rc) const { return curr.y == _rc.size.y; }
        bool operator!=(const DiffRayCaster2D &_rc) const { return curr.y != _rc.size.y; }
        Vec2 operator*() const { return retval; }
        DiffRayCaster2D begin() { return *this; }
        DiffRayCaster2D end() { return *this; }
    };

    struct DiffLayer2D {
        std::vector <DiffPrimitive2D*> objects;

        DiffLayer2D() {}
        DiffLayer2D(const DiffLayer2D &_l) : objects{_l.objects} {}
        DiffLayer2D(const std::initializer_list<DiffPrimitive2D*> &_obj) : objects{_obj} {}
    };

    struct DiffRenderer2D {
        Vec2 ULcorner, step;
        UVec2 size;
        std::vector<DiffLayer2D> layers;

        DiffRenderer2D(const DiffRenderer2D &_rend) : ULcorner{_rend.ULcorner}, step{_rend.step}, size{_rend.size}, layers{_rend.layers} {}
        DiffRenderer2D(Vec2 _ul, Vec2 _step, UVec2 _size, const std::vector<DiffLayer2D> &_l) : ULcorner{_ul}, step{_step}, size{_size}, layers{_l} {}

        Image4 render(const Image4 &ref, const char *out_folder, unsigned Niters = 10u, float _lr = 0.001f) {
            const int Nsamples = 7;
            const float sampleCoef = 1.f / Nsamples;
            const float mseCoef = 1.f / (ref.width * ref.height * 3);
            Image4 res{size.x, size.y};

            std::vector <DiffPrimitive2D*> objects;
            for (auto layer : layers)
                for (auto obj : layer.objects)
                    objects.push_back(obj);

            Adam optim;
            optim.lr = _lr;

            transform.offsetInv = 1.f / step;
            transform.start = ULcorner;
            transform.size = size;

            for (unsigned i = 0u; i < Niters; ++i) {
                for (auto obj : objects) obj->zero_grad();
                Vec3 MSE = 0.f;
                unsigned j = 0u; 
                for (auto ray : DiffRayCaster2D(ULcorner, step, size)) {
                    Vec4 rescol{0.f};

                    for (auto sample : DiffSampler2D(ray, step, Nsamples)) {
                        ColorA tmp = {0.f};
                        for (auto layer : layers) {
                            for (auto obj : layer.objects) {
                                tmp = obj->intersect(sample, ref, j, 1.f - tmp.a);
                                rescol += Vec4(tmp.r, tmp.g, tmp.b, tmp.a);
                                if (tmp.a >= 1.f) break;
                            }
                            if (tmp.a >= 1.f) break;
                        }
                    }
                    rescol *= sampleCoef;
                    res.image[j] = {rescol.x, rescol.y, rescol.z, 1.f};
                    Vec3 diff{ rescol.x - ref.image[j].x, rescol.y - ref.image[j].y, rescol.z - ref.image[j].z };
                    MSE += diff * diff;
                    ++j;
                }

                MSE *= mseCoef;
                for (auto obj : objects) obj->step(optim, 2 * mseCoef, res, ref);
                std::cout << "Step " << i << ", MSE = " << MSE << '\n' << std::endl;
                if (!(i % 10))
                    res.saveAsPNG((std::string(out_folder) + std::string("iter") + std::to_string(i) + std::string(".png")).c_str());
            }
            return res;
        }
    };
};

            //   uv.x = ((e - f) * (x - c) + (c - b) * (y - f)) / ((e - f) * (a - c) + (c - b) * (d - f))
            //   uv.y = ((f - d) * (x - c) + (a - c) * (y - f)) / ((f - d) * (b - c) + (a - c) * (e - f))
            // d uv.x / d a.x = ((f - e) (b (f - y) - c (e - y) + x (e - f)))/(a (e - f) - b (d - f) - c (e - d))^2
            // d uv.x / d b.x = ((e - f) (a (f - y) + c (y - d) + x (d - f)))/(a (f - e) + b (d - f) + c (e - d))^2
            // d uv.x / d c.x = ((e - f) (a (y - e) + b (d - y) + x (e - d)))/(a (f - e) + b (d - f) + c (e - d))^2
            // d uv.x / d a.y = ((b - c) (b (f - y) - c (e - y) + x (e - f)))/(a (e - f) - b (d - f) - c (e - d))^2
            // d uv.x / d b.y = ((b - c) (a (y - f) + c (d - y) + x (f - d)))/(a (f - e) + b (d - f) + c (e - d))^2
            // d uv.x / d c.y = ((b - c) (a (e - y) - b (d - y) + x (d - e)))/(a (e - f) - b (d - f) - c (e - d))^2

            // d uv.y / d a.x = ((d - f) (b (f - y) - c (e - y) + x (e - f)))/(a (e - f) - b (d - f) - c (e - d))^2
            // d uv.y / d b.x = ((f - d) (a (f - y) - c (d - y) + x (d - f)))/(a (e - f) - b (d - f) - c (e - d))^2
            // d uv.y / d c.x = ((d - f) (a (e - y) + b (y - d) + x (d - e)))/(a (f - e) + b (d - f) + c (e - d))^2
            // d uv.y / d a.y = ((a - c) (b (y - f) + c (e - y) + x (f - e)))/(a (e - f) - b (d - f) - c (e - d))^2
            // d uv.y / d b.y = ((a - c) (a (f - y) - c (d - y) + x (d - f)))/(a (e - f) - b (d - f) - c (e - d))^2
            // d uv.y / d c.y = ((c - a) (a (e - y) - b (d - y) + x (d - e)))/(a (e - f) - b (d - f) - c (e - d))^2


#endif