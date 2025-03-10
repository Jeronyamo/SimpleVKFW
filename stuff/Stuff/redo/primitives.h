#ifndef SOGLFW_PRIMITIVES_H
#define SOGLFW_PRIMITIVES_H

#include "texture.h"
#include "utilities.h"


namespace Simple {
    struct SceneObject {
        virtual void _polyf() {}
    };

    struct Primitive2D : SceneObject {
        TextureBase2D *tex;
        virtual ColorA intersect(const Vec2 &pt) = 0;

        Primitive2D(TexBaseMakerInterface _tex) : tex{_tex.ptr} {}

        ~Primitive2D() { if (tex != nullptr) delete tex; }
    };

    struct Primitive3D : SceneObject {
        virtual ColorA intersect(const Vec3 &pt, const Vec3 &dir) = 0;
    };

    struct Background : Primitive2D {
        Background(TexBaseMakerInterface _tex) : Primitive2D(_tex) {}

        ColorA intersect(const Vec2 &pt) { return tex->texcolor(pt); }
    };

    struct SDF : Primitive2D {
        Vec2 v1, v2, sizecoef;
        ImageCh sdf;

        void SDFpass(float *beg, float *tmpRow, float *z, int *v, unsigned len, unsigned step, float sdf_inf) {
            int k = 0;

            v[0] = 0;
            z[0] = -sdf_inf;
            z[1] =  sdf_inf;

            for (int q = 1; q < len; q++) {
                float s  = ((beg[q * step] + sqr(q)) - (beg[v[k] * step] + sqr(v[k]))) / (2 * (q - v[k]));
                while (s <= z[k]) {
                    --k;
                    s  = ((beg[q * step] + sqr(q)) - (beg[v[k] * step] + sqr(v[k]))) / (2 * (q - v[k]));
                }

                ++k;
                v[k] = q;
                z[k] = s;
                z[k + 1] = sdf_inf;
            }

            k = 0;
            for (int q = 0; q < len; q++) {
                while (k < len && z[k+1] < q) k++;
                tmpRow[q] = sqr(q-v[k]) + beg[(v[k] < len ? v[k] : (len - 1)) * step];
            }

            for (int q = 0; q < len; q++) {
                beg[q * step] = tmpRow[q];
            }
        }

        void genDF(ImageCh &mask, float sdf_inf) {
            float *sdf1 = new float[mask.width * mask.height];
            float *sdfptr = sdf1, *imptr = mask.image;

            for (int i = 0; i < mask.height; ++i)
                for (int j = 0; j < mask.width; ++j)
                    *sdfptr++ = *imptr++ ? 0.f : sdf_inf;

            unsigned length = mask.width > mask.height ? mask.width : mask.height;
            float *tmpRow = new float[length];
            float *z = new float[length + 1];
            int *v = new int[length];

            for (unsigned i = 0; i < mask.height; ++i)
                SDFpass(&sdf1[i * mask.width], tmpRow, z, v, mask.width, 1, sdf_inf);

            for (unsigned j = 0; j < mask.width; ++j)
                SDFpass(&sdf1[j], tmpRow, z, v, mask.height, mask.width, sdf_inf);

            delete[] tmpRow;
            delete[] z;
            delete[] v;

            memcpy(mask.image, sdf1, mask.width * mask.height * sizeof(float));
            delete[] sdf1;
        }

        void genSDF(ImageCh &mask, float sdf_inf) {
            float *sdf1 = new float[mask.width * mask.height];
            float *sdfInv = new float[mask.width * mask.height];

            for (int i = 0; i < mask.height; ++i)
                for (int j = 0; j < mask.width; ++j) {
                    unsigned index = i * mask.width + j;
                    sdf1[index] = mask.image[index] ? 0.f : sdf_inf;
                    sdfInv[index] = sdf_inf - sdf1[index];
                }

            unsigned length = mask.width > mask.height ? mask.width : mask.height;
            float *tmpRow = new float[length];
            float *z = new float[length + 1];
            int *v = new int[length];

            for (unsigned i = 0; i < mask.height; ++i)
                SDFpass(&sdf1[i * mask.width], tmpRow, z, v, mask.width, 1, sdf_inf);

            for (unsigned j = 0; j < mask.width; ++j)
                SDFpass(&sdf1[j], tmpRow, z, v, mask.height, mask.width, sdf_inf);

            delete[] tmpRow;
            delete[] z;
            delete[] v;

            memcpy(mask.image, sdf1, mask.width * mask.height * sizeof(float));
            delete[] sdf1;
        }

        SDF(const Vec2 &_v1, const Vec2 &_v2, const char *filename, TexBaseMakerInterface texptr, float sdfinf = 1e3f, float sdfthold = 0.5f, bool bitmask = true) :
                v1{_v1}, v2{_v2}, sizecoef{(v2 - v1).inverse()}, sdf(filename), Primitive2D{texptr} {
            if (bitmask) {
                auto *ptr = sdf.image;
                for (unsigned i = 0u; i < sdf.width * sdf.height; ++i)
                    *ptr++ = *ptr >= sdfthold;
                genSDF(sdf, sdfinf);
            }
        }

        ColorA intersect(const Vec2 &pt) {
            Vec2 tmp = (pt - v1) * sizecoef;
            if (0.f <= tmp.x && tmp.x <= 1.f && 0.f <= tmp.y && tmp.y <= 1.f) {
                float sdfcol = sdf[tmp];
                if (sdfcol <= 0.f) return tex->texcolor(tmp);
            }
        }
    };

    struct SDFTriangle2D : Primitive2D {
        Vec2 a, b, c;

        SDFTriangle2D(const Vec2 &_a, const Vec2 &_b, const Vec2 &_c, TexBaseMakerInterface texptr) :
                a{_a}, b{_b}, c{_c}, Primitive2D{texptr} {}

        float getSDF(const Vec2 &pt) {
            Vec2 e0 =  b - a, e1 =  c - b, e2 =  a - c;
            Vec2 v0 = pt - a, v1 = pt - b, v2 = pt - c;
            Vec2 pq0 = v0 - e0 * clampCol(dot(v0,e0) / dot(e0,e0));
            Vec2 pq1 = v1 - e1 * clampCol(dot(v1,e1) / dot(e1,e1));
            Vec2 pq2 = v2 - e2 * clampCol(dot(v2,e2) / dot(e2,e2));
            float s = sign(e0.x * e2.y - e0.y * e2.x);
            Vec2 d = min(min(Vec2(dot(pq0, pq0), s * (v0.x * e0.y - v0.y * e0.x)),
                             Vec2(dot(pq1, pq1), s * (v1.x * e1.y - v1.y * e1.x))),
                             Vec2(dot(pq2, pq2), s * (v2.x * e2.y - v2.y * e2.x)));
            return -sqrt(d.x) * sign(d.y);
        }

        ColorA intersect(const Vec2 &pt) {
            if (getSDF(pt) <= 0.f) return tex->texcolor(pt);
            return {0.f};
        }

    };

    struct Circle2D : Primitive2D {
        Vec2 center;
        float radius, invrad;

        Circle2D(const Vec2 &_center, float _rad, TexBaseMakerInterface texptr) : center{_center}, radius{_rad}, invrad{0.5f / _rad}, Primitive2D{texptr} {}

        ColorA intersect(const Vec2 &pt) {
            Vec2 l = (pt - center);
            float n = l.norm() - radius;

            //if (0.f < n && n <= 0.025f) return {0.f, 0.f, 0.f, 1.f};
            if (n <= 0.f) return tex->texcolor(0.5f + l * invrad);
            return {0.f};
        }
    };

    struct Triangle2D : Primitive2D {
        Vec2 a, b, c;
        Vec2 texCoord1, texCoord2, texCoord3;

        Triangle2D(const Vec2 &_a, const Vec2 &_b, const Vec2 &_c, TexBaseMakerInterface texptr,
                   const Vec2 &_t1 = {0.f}, const Vec2 &_t2 = {0.f}, const Vec2 &_t3 = {0.f}) :
                a{_a}, b{_b}, c{_c}, Primitive2D{texptr}, texCoord1{_t1}, texCoord2{_t2}, texCoord3{_t3} {}

        inline float PointLineDistance2D(const Vec2 &point, const Vec2 &A, const Vec2 &B) {
            return abs((B.x - A.x) * (A.y - point.y) - (A.x - point.x) * (B.y - A.y)) /
                  sqrt((B.x - A.x) * (B.x - A.x) + (B.y - A.y) * (B.y - A.y));
        }

        bool PointTriangleTest2D(Vec2 point, Vec2 &_uv) {
            float  d = 1.f / ((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));
            Vec3 uv{0.f};
            uv.x = ((b.y - c.y) * (point.x - c.x) + (c.x - b.x) * (point.y - c.y)) * d;
            uv.y = ((c.y - a.y) * (point.x - c.x) + (a.x - c.x) * (point.y - c.y)) * d;
            uv.z = 1.f - uv.x - uv.y;

            _uv = point;
            if (texCoord1.x != texCoord2.x || texCoord2.x != texCoord3.x || texCoord1.y != texCoord2.y || texCoord2.y != texCoord3.y)
                _uv = uv.x * texCoord1 + uv.y * texCoord2 + uv.z * texCoord3;

            return uv.x >= 0.f && uv.x <= 1.f && uv.y >= 0.f && uv.y <= 1.f && uv.z >= 0.f && uv.z <= 1.f;
        }

        bool PointEdgesTest2D(Vec2 point) {
            Vec3 edges_dist{ PointLineDistance2D(point, a, b),
                             PointLineDistance2D(point, a, c),
                             PointLineDistance2D(point, b, c) };

            return edges_dist.x <= 0.003f || edges_dist.y <= 0.003f || edges_dist.z <= 0.003f;
        }

        ColorA intersect(const Vec2 &pt) {
            Vec2 uv{0.f};
            if (PointTriangleTest2D(pt, uv)) {
                //if (PointEdgesTest2D(pt)) return {0.f, 0.f, 0.f, 1.f};
                //else 
                return tex->texcolor(uv);
            }
            return {0.f};
        }
    };

    struct Triangle3D : Primitive3D {
        Vec3 a, b, c;
        UVec3 indices{0u};
        Vec3 normals[3]{{0.f}, {0.f}, {0.f}};
        Vec2 texCoords[3]{{0.f}, {0.f}, {0.f}};

        ColorA intersect(const Vec3 &pt, const Vec3 &dir) {}
    };

    struct TriangleCol3D : Primitive3D {
        Vec3 a, b, c;
        UVec3 inds;
        ColorA color;

        ColorA intersect(const Vec3 &pt, const Vec3 &dir) {}
    };

    struct Sphere3D : Primitive3D {
        ColorA intersect(const Vec3 &pt, const Vec3 &dir) {}

    };
};


#endif