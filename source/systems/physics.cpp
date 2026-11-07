#include "systems/physics.h"
#include "core/engine.h"

#include <cmath>
#include <algorithm>

namespace Simple {
    namespace Physics {

        namespace {
            const float EPS = 1e-6f;

            inline vec3  rot   (const Quat &_q, const vec3 &_v) { return _q.rotateByUnit(_v); }
            inline vec3  invRot(const Quat &_q, const vec3 &_v) { return _q.conjugate().rotateByUnit(_v); }

            using Affine::Rotation::unitOrIdentity;
            using Affine::Rotation::rotationVector;
            using Affine::Rotation::eulerXYZ;
            using Affine::Rotation::fromEulerXYZ;
            using Interpolation::nlerp;

            // q += 0.5 * [w, 0] * q (a world space rotation vector, first order)
            inline void addRotation(Quat &_q, const vec3 &_w) {
                const Quat __dq = Quat(0.f, _w.x, _w.y, _w.z) * _q;
                _q = unitOrIdentity(Quat(_q.s + 0.5f * __dq.s, _q.i + 0.5f * __dq.i, _q.j + 0.5f * __dq.j, _q.k + 0.5f * __dq.k));
            }

            float boundingRadius(const Collider &_c) {
                switch (_c.shape) {
                    case SHAPE_SPHERE:  return _c.size.x;
                    case SHAPE_BOX:     return _c.size.norm();
                    case SHAPE_CAPSULE: return _c.size.x + 0.5f * _c.size.y;
                    default:            return 0.f;
                }
            }
        } // anonymous namespace END


//  == === ==== >  Colliders, bodies

        Collider Collider::sphere(float _radius)  { Collider c; c.shape = SHAPE_SPHERE;  c.size = {_radius, _radius, _radius}; return c; }
        Collider Collider::box(const vec3 &_half) { Collider c; c.shape = SHAPE_BOX;     c.size = _half; return c; }
        Collider Collider::capsule(float _radius, float _length) { Collider c; c.shape = SHAPE_CAPSULE; c.size = {_radius, _length, _radius}; return c; }
        Collider Collider::plane()                { Collider c; c.shape = SHAPE_PLANE;   c.size = {0.f, 0.f, 0.f}; return c; }

        Body Body::at(const vec3 &_position, const Quat &_rotation, float _mass) {
            Body b;
            b.position = _position;
            b.rotation = _rotation;
            b.mass     = _mass;
            return b;
        }


//  == === ==== >  Contacts between shapes

        namespace {
            // Closest points of the segments p1-q1 and p2-q2 (Ericson, Real-Time Collision Detection 5.1.9)
            void closestSegments(const vec3 &_p1, const vec3 &_q1, const vec3 &_p2, const vec3 &_q2, vec3 &_c1, vec3 &_c2) {
                const vec3 d1 = _q1 - _p1, d2 = _q2 - _p2, r = _p1 - _p2;
                const float a = Math::dot(d1, d1), e = Math::dot(d2, d2), f = Math::dot(d2, r);
                float s = 0.f, t = 0.f;
                if (a <= EPS && e <= EPS) { _c1 = _p1; _c2 = _p2; return; }
                if (a <= EPS) t = Math::clampCL(f / e, 0.f, 1.f);
                else {
                    const float c = Math::dot(d1, r);
                    if (e <= EPS) s = Math::clampCL(-c / a, 0.f, 1.f);
                    else {
                        const float b = Math::dot(d1, d2), denom = a * e - b * b;
                        s = denom > EPS ? Math::clampCL((b * f - c * e) / denom, 0.f, 1.f) : 0.f;
                        t = (b * s + f) / e;
                        if (t < 0.f)      { t = 0.f; s = Math::clampCL(-c / a, 0.f, 1.f); }
                        else if (t > 1.f) { t = 1.f; s = Math::clampCL((b - c) / a, 0.f, 1.f); }
                    }
                }
                _c1 = _p1 + d1 * s;
                _c2 = _p2 + d2 * t;
            }

            vec3 closestOnSegment(const vec3 &_p, const vec3 &_a, const vec3 &_b) {
                const vec3 ab = _b - _a;
                const float l2 = Math::dot(ab, ab);
                return l2 <= EPS ? _a : _a + ab * Math::clampCL(Math::dot(_p - _a, ab) / l2, 0.f, 1.f);
            }

            // A sphere or a capsule: a segment with a radius
            struct Swept { vec3 p0, p1; float r; };

            Swept swept(const Collider &_c, const vec3 &_p, const Quat &_q) {
                Swept s;
                s.r = _c.size.x;
                if (_c.shape == SHAPE_CAPSULE) {
                    const vec3 __h = rot(_q, vec3{0.f, 0.5f * _c.size.y, 0.f});
                    s.p0 = _p - __h; s.p1 = _p + __h;
                } else s.p0 = s.p1 = _p;
                return s;
            }

            void addSphereContact(const vec3 &_ca, float _ra, const vec3 &_cb, float _rb, std::vector<ShapeContact> &_out) {
                const vec3 d = _ca - _cb;
                const float dist = d.norm();
                if (dist >= _ra + _rb) return;
                ShapeContact c;
                c.normal  = dist > EPS ? d * (1.f / dist) : vec3{0.f, 1.f, 0.f};
                c.depth   = _ra + _rb - dist;
                c.point_a = _ca - c.normal * _ra;
                c.point_b = _cb + c.normal * _rb;
                _out.push_back(c);
            }

            void sweptSwept(const Swept &_a, const Swept &_b, std::vector<ShapeContact> &_out) {
                const vec3 da = _a.p1 - _a.p0, db = _b.p1 - _b.p0;
                const float la = da.norm(), lb = db.norm();
                if (la > EPS && lb > EPS && std::fabs(Math::dot(da, db)) > 0.95f * la * lb) {
                    // Nearly parallel capsules: the end points against the other segment (up to 4 contacts, a capsule can lie
                    // on another one without rocking)
                    const size_t __first = _out.size();
                    addSphereContact(_a.p0, _a.r, closestOnSegment(_a.p0, _b.p0, _b.p1), _b.r, _out);
                    addSphereContact(_a.p1, _a.r, closestOnSegment(_a.p1, _b.p0, _b.p1), _b.r, _out);
                    addSphereContact(closestOnSegment(_b.p0, _a.p0, _a.p1), _a.r, _b.p0, _b.r, _out);
                    addSphereContact(closestOnSegment(_b.p1, _a.p0, _a.p1), _a.r, _b.p1, _b.r, _out);
                    // Drop duplicates (the same points from both sides)
                    for (size_t i = _out.size(); i-- > __first + 1u; )
                        for (size_t j = __first; j < i; ++j)
                            if ((_out[i].point_a - _out[j].point_a).norm() < 0.1f * _a.r) { _out.erase(_out.begin() + i); break; }
                    if (_out.size() > __first) return;
                }
                vec3 c1, c2;
                closestSegments(_a.p0, _a.p1, _b.p0, _b.p1, c1, c2);
                addSphereContact(c1, _a.r, c2, _b.r, _out);
            }

            // A point with a radius (box local space) against a box with half extents '_h'
            bool pointBox(const vec3 &_s, float _r, const vec3 &_h, vec3 &_n, vec3 &_pb, float &_depth) {
                const vec3 c = { Math::clampCL(_s.x, -_h.x, _h.x), Math::clampCL(_s.y, -_h.y, _h.y), Math::clampCL(_s.z, -_h.z, _h.z) };
                const vec3 d = _s - c;
                const float dist = d.norm();
                if (dist > EPS) {
                    if (dist >= _r) return false;
                    _n = d * (1.f / dist);
                    _depth = _r - dist;
                    _pb = c;
                    return true;
                }
                // Inside: the nearest face
                uint32_t __axis = 0u;
                float __best = 1e30f;
                for (uint32_t i = 0u; i < 3u; ++i) {
                    const float __gap = _h[i] - std::fabs(_s[i]);
                    if (__gap < __best) { __best = __gap; __axis = i; }
                }
                const float __sign = _s[__axis] < 0.f ? -1.f : 1.f;
                _n = Mat::Eye3[__axis] * __sign;
                _depth = _r + __best;
                _pb = _s;
                _pb[__axis] = __sign * _h[__axis];
                return true;
            }

            void sweptBox(const Swept &_a, const Collider &_cb, const vec3 &_pb, const Quat &_qb, std::vector<ShapeContact> &_out) {
                const vec3 h = _cb.size;
                const vec3 e0 = invRot(_qb, _a.p0 - _pb), e1 = invRot(_qb, _a.p1 - _pb);
                // Candidates: the end points and the point of the segment nearest to the box (a few alternating projections)
                vec3 __cand[3];
                uint32_t __count = 0u;
                __cand[__count++] = e0;
                const vec3 __d = e1 - e0;
                const float __l2 = Math::dot(__d, __d);
                if (__l2 > EPS) {
                    __cand[__count++] = e1;
                    float t = 0.5f;
                    for (uint32_t it = 0u; it < 4u; ++it) {
                        const vec3 s = e0 + __d * t;
                        const vec3 c = { Math::clampCL(s.x, -h.x, h.x), Math::clampCL(s.y, -h.y, h.y), Math::clampCL(s.z, -h.z, h.z) };
                        t = Math::clampCL(Math::dot(c - e0, __d) / __l2, 0.f, 1.f);
                    }
                    if (t > 0.02f && t < 0.98f) __cand[__count++] = e0 + __d * t;
                }
                for (uint32_t i = 0u; i < __count; ++i) {
                    vec3 n, pb;
                    float depth;
                    if (!pointBox(__cand[i], _a.r, h, n, pb, depth)) continue;
                    ShapeContact c;
                    c.normal  = rot(_qb, n);
                    c.depth   = depth;
                    c.point_b = _pb + rot(_qb, pb);
                    c.point_a = _pb + rot(_qb, __cand[i]) - c.normal * _a.r;
                    _out.push_back(c);
                }
            }

            void boxVertices(const vec3 &_p, const Quat &_q, const vec3 &_h, vec3 _out[8]) {
                for (uint32_t i = 0u; i < 8u; ++i)
                    _out[i] = _p + rot(_q, vec3{ (i & 1u) ? _h.x : -_h.x, (i & 2u) ? _h.y : -_h.y, (i & 4u) ? _h.z : -_h.z });
            }

            // Clips the incident face (the face of the incident box most opposite to '_n') against the side planes of the
            // reference face (outward normal '_n' = axis '_axis' of the reference box), keeps the points behind the reference
            // face. Returns the number of contacts. '_flip': the reference box is A (the contact normal is -_n)
            uint32_t faceContacts(const vec3 &_ref_p, const Quat &_ref_q, const vec3 &_ref_h, uint32_t _axis, const vec3 &_n,
                                  const vec3 &_inc_p, const Quat &_inc_q, const vec3 &_inc_h, bool _flip, std::vector<ShapeContact> &_out) {
                // The incident face
                uint32_t __inc_axis = 0u;
                float __inc_sign = 1.f, __best = 1e30f;
                for (uint32_t i = 0u; i < 3u; ++i) {
                    const float d = Math::dot(rot(_inc_q, Mat::Eye3[i]), _n);
                    if ( d < __best) { __best =  d; __inc_axis = i; __inc_sign =  1.f; }
                    if (-d < __best) { __best = -d; __inc_axis = i; __inc_sign = -1.f; }
                }
                const uint32_t u = (__inc_axis + 1u) % 3u, v = (__inc_axis + 2u) % 3u;
                std::vector<vec3> __poly, __tmp;
                const float __su[4] = { -1.f, 1.f, 1.f, -1.f }, __sv[4] = { -1.f, -1.f, 1.f, 1.f };
                for (uint32_t c = 0u; c < 4u; ++c) {
                    vec3 l;
                    l[__inc_axis] = __inc_sign * _inc_h[__inc_axis];
                    l[u] = __su[c] * _inc_h[u];
                    l[v] = __sv[c] * _inc_h[v];
                    __poly.push_back(invRot(_ref_q, _inc_p + rot(_inc_q, l) - _ref_p)); // reference box space
                }
                // Clip against the 4 side planes: |x_j| <= h_j for the other two axes
                for (uint32_t j = 0u; j < 3u; ++j) {
                    if (j == _axis) continue;
                    for (int side = -1; side <= 1; side += 2) {
                        __tmp.clear();
                        const float lim = _ref_h[j];
                        for (size_t k = 0u; k < __poly.size(); ++k) {
                            const vec3 &a = __poly[k], &b = __poly[(k + 1u) % __poly.size()];
                            const float da = side * a[j] - lim, db = side * b[j] - lim; // <= 0: inside
                            if (da <= 0.f) __tmp.push_back(a);
                            if ((da <= 0.f) != (db <= 0.f)) __tmp.push_back(a + (b - a) * (da / (da - db)));
                        }
                        __poly.swap(__tmp);
                        if (__poly.empty()) return 0u;
                    }
                }
                const vec3 __n_local = invRot(_ref_q, _n);
                const float __face = _ref_h[_axis];
                uint32_t __added = 0u;
                for (const vec3 &l : __poly) {
                    const float __depth = __face - Math::dot(l, __n_local);
                    if (__depth <= 0.f) continue;
                    const vec3 __p = _ref_p + rot(_ref_q, l);
                    ShapeContact c;
                    c.depth = __depth;
                    if (_flip) { c.normal = _n * -1.f; c.point_b = __p; c.point_a = __p + _n * __depth; }
                    else       { c.normal = _n;        c.point_a = __p; c.point_b = __p + _n * __depth; }
                    _out.push_back(c);
                    ++__added;
                }
                return __added;
            }

            void boxBox(const Collider &_ca, const vec3 &_pa, const Quat &_qa, const Collider &_cb, const vec3 &_pb, const Quat &_qb,
                        std::vector<ShapeContact> &_out) {
                const vec3 ha = _ca.size, hb = _cb.size;
                vec3 A[3], B[3];
                for (uint32_t i = 0u; i < 3u; ++i) { A[i] = rot(_qa, Mat::Eye3[i]); B[i] = rot(_qb, Mat::Eye3[i]); }
                const vec3 T = _pa - _pb;
                float __best = 1e30f;
                vec3  __best_axis;
                int   __best_kind = -1; // 0..2: face of A, 3..5: face of B, 6: edge pair
                for (uint32_t k = 0u; k < 15u; ++k) {
                    vec3 L;
                    if (k < 3u) L = A[k];
                    else if (k < 6u) L = B[k - 3u];
                    else {
                        L = Math::cross(A[(k - 6u) / 3u], B[(k - 6u) % 3u]);
                        const float __l = L.norm();
                        if (__l < 1e-4f) continue;
                        L = L * (1.f / __l);
                    }
                    float ra = 0.f, rb = 0.f;
                    for (uint32_t i = 0u; i < 3u; ++i) { ra += ha[i] * std::fabs(Math::dot(A[i], L)); rb += hb[i] * std::fabs(Math::dot(B[i], L)); }
                    const float __overlap = ra + rb - std::fabs(Math::dot(T, L));
                    if (__overlap < 0.f) return; // a separating axis
                    // Faces are preferred (stable contacts) unless an edge axis is clearly better
                    const bool __better = k < 6u ? __overlap < __best : __overlap < __best * 0.95f - 1e-3f;
                    if (__better) {
                        __best = __overlap;
                        __best_axis = Math::dot(T, L) < 0.f ? L * -1.f : L; // from B to A
                        __best_kind = k < 3u ? (int)k : (k < 6u ? (int)k : 6);
                    }
                }
                if (__best_kind >= 3 && __best_kind < 6) {
                    // The face of B along the normal, A's face clipped
                    if (faceContacts(_pb, _qb, hb, (uint32_t)__best_kind - 3u, __best_axis, _pa, _qa, ha, false, _out) > 0u) return;
                } else if (__best_kind >= 0 && __best_kind < 3) {
                    // The face of A towards B (normal -n), B's face clipped
                    if (faceContacts(_pa, _qa, ha, (uint32_t)__best_kind, __best_axis * -1.f, _pb, _qb, hb, true, _out) > 0u) return;
                }
                vec3 va[8], vb[8];
                boxVertices(_pa, _qa, ha, va);
                boxVertices(_pb, _qb, hb, vb);
                // Edges (or no vertex inside the face): one contact between the deepest vertices
                uint32_t __ia = 0u, __ib = 0u;
                for (uint32_t i = 1u; i < 8u; ++i) {
                    if (Math::dot(va[i], __best_axis) < Math::dot(va[__ia], __best_axis)) __ia = i;
                    if (Math::dot(vb[i], __best_axis) > Math::dot(vb[__ib], __best_axis)) __ib = i;
                }
                ShapeContact c;
                c.normal = __best_axis;
                c.depth  = __best;
                const vec3 __mid = (va[__ia] + vb[__ib]) * 0.5f;
                c.point_a = __mid - __best_axis * (0.5f * __best);
                c.point_b = __mid + __best_axis * (0.5f * __best);
                _out.push_back(c);
            }

            void planeOther(const Collider &_ca, const vec3 &_pa, const Quat &_qa, const vec3 &_pb, const Quat &_qb, std::vector<ShapeContact> &_out) {
                const vec3 n = rot(_qb, vec3{0.f, 1.f, 0.f});
                if (_ca.shape == SHAPE_BOX) {
                    vec3 v[8];
                    boxVertices(_pa, _qa, _ca.size, v);
                    for (uint32_t i = 0u; i < 8u; ++i) {
                        const float s = Math::dot(v[i] - _pb, n);
                        if (s >= 0.f) continue;
                        ShapeContact c;
                        c.normal = n; c.depth = -s; c.point_a = v[i]; c.point_b = v[i] - n * s;
                        _out.push_back(c);
                    }
                    return;
                }
                const Swept sw = swept(_ca, _pa, _qa);
                const vec3 ends[2] = { sw.p0, sw.p1 };
                const uint32_t __count = _ca.shape == SHAPE_CAPSULE ? 2u : 1u;
                for (uint32_t i = 0u; i < __count; ++i) {
                    const float s = Math::dot(ends[i] - _pb, n);
                    if (s >= sw.r) continue;
                    ShapeContact c;
                    c.normal = n; c.depth = sw.r - s; c.point_a = ends[i] - n * sw.r; c.point_b = ends[i] - n * s;
                    _out.push_back(c);
                }
            }
        } // anonymous namespace END

        void collideShapes(const Collider &_ca, const vec3 &_pa, const Quat &_qa,
                           const Collider &_cb, const vec3 &_pb, const Quat &_qb, std::vector<ShapeContact> &_out) {
            // Order the pair (sphere/capsule < box < plane); a swapped pair gets its contacts flipped
            const auto rank = [](ShapeType _s) { return _s == SHAPE_PLANE ? 3 : (_s == SHAPE_BOX ? 2 : 1); };
            if (rank(_ca.shape) > rank(_cb.shape)) {
                const size_t __first = _out.size();
                collideShapes(_cb, _pb, _qb, _ca, _pa, _qa, _out);
                for (size_t i = __first; i < _out.size(); ++i) {
                    std::swap(_out[i].point_a, _out[i].point_b);
                    _out[i].normal = _out[i].normal * -1.f;
                }
                return;
            }
            if (_cb.shape == SHAPE_PLANE) {
                if (_ca.shape != SHAPE_PLANE) planeOther(_ca, _pa, _qa, _pb, _qb, _out);
                return;
            }
            if (_cb.shape == SHAPE_BOX) {
                if (_ca.shape == SHAPE_BOX) boxBox(_ca, _pa, _qa, _cb, _pb, _qb, _out);
                else sweptBox(swept(_ca, _pa, _qa), _cb, _pb, _qb, _out);
                return;
            }
            sweptSwept(swept(_ca, _pa, _qa), swept(_cb, _pb, _qb), _out);
        }


//  == === ==== >  World

        namespace {
            // Note: the solver works on a dense copy of the body states (the quaternion operations are inline in 'linalg.h')
        } // anonymous namespace END

        namespace {
            typedef World::SolverState SS;

            // World inverse inertia R * diag(ii) * R^T; the solver keeps it for a substep (the rotation changes little in one)
            inline void updateInvInertia(SS &_s) {
                const vec3 c0 = rot(_s.q, vec3{1.f, 0.f, 0.f}), c1 = rot(_s.q, vec3{0.f, 1.f, 0.f}), c2 = rot(_s.q, vec3{0.f, 0.f, 1.f});
                const float R[3][3] = { { c0.x, c1.x, c2.x }, { c0.y, c1.y, c2.y }, { c0.z, c1.z, c2.z } };
                const float d[3] = { _s.ii.x, _s.ii.y, _s.ii.z };
                for (int r = 0; r < 3; ++r)
                    for (int c = 0; c < 3; ++c)
                        _s.iw[r * 3 + c] = R[r][0] * d[0] * R[c][0] + R[r][1] * d[1] * R[c][1] + R[r][2] * d[2] * R[c][2];
            }
            inline vec3 invI(const SS &_s, const vec3 &_v) {
                const float *m = _s.iw;
                return { m[0] * _v.x + m[1] * _v.y + m[2] * _v.z, m[3] * _v.x + m[4] * _v.y + m[5] * _v.z, m[6] * _v.x + m[7] * _v.y + m[8] * _v.z };
            }

            inline float genInvMass(const SS &_s, const vec3 &_r, const vec3 &_n) {
                if (_s.im <= 0.f) return 0.f;
                const vec3 rn = Math::cross(_r, _n);
                return _s.im + Math::dot(rn, invI(_s, rn));
            }

            inline void applyPositional(SS &_s, const vec3 &_p, const vec3 &_r, float _sign) {
                if (_s.im <= 0.f) return;
                _s.x += _p * (_s.im * _sign);
                addRotation(_s.q, invI(_s, Math::cross(_r, _p)) * _sign);
            }

            // XPBD positional constraint: moves the point '_r1' of body 1 by -C along '_dir' (shared with body 2 by the
            // inverse masses); returns the Lagrange multiplier change
            float positional(SS &_b1, const vec3 &_r1, SS &_b2, const vec3 &_r2, const vec3 &_dir, float _c, float _alpha) {
                const float w = genInvMass(_b1, _r1, _dir) + genInvMass(_b2, _r2, _dir);
                if (w + _alpha <= EPS) return 0.f;
                const float dl = -_c / (w + _alpha);
                const vec3 p = _dir * dl;
                applyPositional(_b1, p, _r1,  1.f);
                applyPositional(_b2, p, _r2, -1.f);
                return dl;
            }

            // XPBD angular constraint: rotates body 1 by -C around '_axis' (unit)
            float angular(SS &_b1, SS &_b2, const vec3 &_axis, float _c, float _alpha) {
                const float w = (_b1.im > 0.f ? Math::dot(_axis, invI(_b1, _axis)) : 0.f) + (_b2.im > 0.f ? Math::dot(_axis, invI(_b2, _axis)) : 0.f);
                if (w + _alpha <= EPS) return 0.f;
                const float dl = -_c / (w + _alpha);
                const vec3 p = _axis * dl;
                if (_b1.im > 0.f) addRotation(_b1.q, invI(_b1, p));
                if (_b2.im > 0.f) addRotation(_b2.q, invI(_b2, p) * -1.f);
                return dl;
            }

            // Per axis: min > max: free; else clamped
            inline float limitAxis(float _v, float _min, float _max) { return _min > _max ? _v : Math::clampCL(_v, _min, _max); }
        } // anonymous namespace END

        World::World(ComponentStorage &_ecs) : ecs(_ecs) {}

        EntityID World::createBody(const Body &_body, const Collider &_collider, TagID _motion) {
            const EntityID e = ecs.create(tagMask(_motion));
            Body &b = ecs.add<Body>(e, _body);
            ecs.add<Collider>(e, _collider);
            b.rotation = unitOrIdentity(b.rotation);
            b.prev_position = b.frame_position = b.position;
            b.prev_rotation = b.frame_rotation = b.rotation;
            updateMass(e);
            return e;
        }

        void World::attach(Core::Engine &_engine) { events = &_engine.events; }

        void World::detach(Core::Engine &_engine) { if (events == &_engine.events) events = nullptr; }

        void World::update(Core::Phase _phase, const Core::FrameTime &_time) {
            if (_phase == Core::PHASE_PHYSICS) update(_time.sim_dt);
        }

        EntityID World::createJoint(const Joint &_joint) {
            const EntityID e = ecs.create();
            ecs.add<Joint>(e, _joint);
            return e;
        }

        void World::updateMass(EntityID _e) {
            Body *b = ecs.get<Body>(_e);
            const Collider *c = ecs.get<Collider>(_e);
            if (!b || !c) return;
            if (!ecs.hasTag(_e, TagID::TAG_PHYSICS_DYNAMIC)) { b->inv_mass = 0.f; b->inv_inertia = {0.f, 0.f, 0.f}; return; }
            if (b->mass <= 0.f) b->mass = 1.f;
            const float m = b->mass;
            vec3 I;
            switch (c->shape) {
                case SHAPE_SPHERE: { const float r = c->size.x; I = vec3{0.4f * m * r * r}; } break;
                case SHAPE_BOX: {
                    const vec3 h = c->size;
                    I = { m / 3.f * (h.y * h.y + h.z * h.z), m / 3.f * (h.x * h.x + h.z * h.z), m / 3.f * (h.x * h.x + h.y * h.y) };
                } break;
                case SHAPE_CAPSULE: {
                    // Bullet: the box around the capsule
                    const vec3 h = { c->size.x, c->size.x + 0.5f * c->size.y, c->size.x };
                    I = { m / 3.f * (h.y * h.y + h.z * h.z), m / 3.f * (h.x * h.x + h.z * h.z), m / 3.f * (h.x * h.x + h.y * h.y) };
                } break;
                default: I = vec3{m};
            }
            b->inertia     = I;
            b->inv_mass    = 1.f / m;
            b->inv_inertia = { I.x > EPS ? 1.f / I.x : 0.f, I.y > EPS ? 1.f / I.y : 0.f, I.z > EPS ? 1.f / I.z : 0.f };
        }

        void World::setMotion(EntityID _e, TagID _motion) {
            ecs.delTags(_e, tagMask(TagID::TAG_PHYSICS_STATIC, TagID::TAG_PHYSICS_KINEMATIC, TagID::TAG_PHYSICS_DYNAMIC));
            ecs.addTag(_e, _motion);
            updateMass(_e);
        }

        void World::teleport(EntityID _e, const vec3 &_p, const Quat &_q, bool _reset_velocity) {
            Body *b = ecs.get<Body>(_e);
            if (!b) return;
            b->position = b->prev_position = b->frame_position = b->target_position = _p;
            b->rotation = b->prev_rotation = b->frame_rotation = b->target_rotation = unitOrIdentity(_q);
            b->has_target = false;
            if (_reset_velocity) { b->velocity = {0.f, 0.f, 0.f}; b->angular_velocity = {0.f, 0.f, 0.f}; }
        }

        void World::setKinematicTarget(EntityID _e, const vec3 &_p, const Quat &_q) {
            Body *b = ecs.get<Body>(_e);
            if (!b) return;
            b->target_position = _p;
            b->target_rotation = unitOrIdentity(_q);
            b->has_target = true;
        }

        void World::removeBrokenJoints() {
            std::vector<EntityID> __broken;
            ecs.each<Joint>([&](EntityID _e, Joint &_j) {
                if (!ecs.has<Body>(_j.body_a) || !ecs.has<Body>(_j.body_b)) __broken.push_back(_e);
            });
            for (const EntityID &e : __broken) ecs.destroy(e);
        }

        void World::gather_() {
            bodies_.clear();
            states_.clear();
            joints_.clear();
            jointed_.clear();
            std::fill(slot_.begin(), slot_.end(), UINT32_MAX);
            const TagFilter __active = TagFilter().none(TagID::TAG_PHYSICS_DISABLED);
            ecs.each<Body, Collider>([&](EntityID _e, Body &_b, Collider &_c) {
                const TagMask __tags = ecs.tags(_e);
                SolverBody s;
                s.entity    = _e;
                s.body      = &_b;
                s.collider  = &_c;
                s.dynamic   = (__tags & tagMask(TagID::TAG_PHYSICS_DYNAMIC)) != 0u;
                s.kinematic = (__tags & tagMask(TagID::TAG_PHYSICS_KINEMATIC)) != 0u;
                s.report    = (__tags & tagMask(TagID::TAG_PHYSICS_REPORT_CONTACTS)) != 0u;
                s.radius    = boundingRadius(_c);
                SolverState st;
                st.x = st.px = _b.position;
                st.q = st.pq = _b.rotation;
                st.v = _b.velocity;
                st.w = _b.angular_velocity;
                st.im = s.dynamic ? _b.inv_mass : 0.f;
                st.ii = s.dynamic ? _b.inv_inertia : vec3{0.f, 0.f, 0.f};
                st.fx = _b.frame_position;   st.tx = _b.target_position;
                st.fq = _b.frame_rotation; st.tq = _b.target_rotation;
                st.has_target = s.kinematic && _b.has_target;
                updateInvInertia(st);
                if (slot_.size() <= _e.index) slot_.resize(_e.index + 1u, UINT32_MAX);
                slot_[_e.index] = (uint32_t)bodies_.size();
                bodies_.push_back(s);
                states_.push_back(st);
            }, __active);
            ecs.each<Joint>([&](EntityID _e, Joint &_j) {
                if (!ecs.isValid(_j.body_a) || !ecs.isValid(_j.body_b)) return;
                const uint32_t a = _j.body_a.index < slot_.size() ? slot_[_j.body_a.index] : UINT32_MAX;
                const uint32_t b = _j.body_b.index < slot_.size() ? slot_[_j.body_b.index] : UINT32_MAX;
                if (a == UINT32_MAX || b == UINT32_MAX || a == b) return;
                if (!bodies_[a].dynamic && !bodies_[b].dynamic) return;
                SolverJoint j;
                j.joint = &_j; j.a = a; j.b = b;
                joints_.push_back(j);
                if (!_j.collide_connected)
                    jointed_.insert(std::make_pair(std::min(a, b) * 1ull, std::max(a, b) * 1ull));
            }, __active);
            stats.bodies = (uint32_t)bodies_.size();
            stats.dynamic_bodies = 0u;
            for (const SolverBody &s : bodies_) stats.dynamic_bodies += s.dynamic ? 1u : 0u;
            stats.joints = (uint32_t)joints_.size();
        }

        void World::broadPhase_(float _h) {
            pairs_.clear();
            // Sweep and prune along X over bounding spheres (grown by the motion of the step); planes are paired with all
            struct Item { float lo, hi; uint32_t body; };
            std::vector<Item> __items;
            std::vector<uint32_t> __planes;
            __items.reserve(bodies_.size());
            for (uint32_t i = 0u; i < (uint32_t)bodies_.size(); ++i) {
                const SolverBody &s = bodies_[i];
                const SolverState &st = states_[i];
                if (s.collider->shape == SHAPE_PLANE) { __planes.push_back(i); continue; }
                const float r = s.radius + st.v.norm() * _h + settings.contact_margin + (st.has_target ? (st.tx - st.x).norm() : 0.f);
                Item it;
                it.lo = st.x.x - r; it.hi = st.x.x + r; it.body = i;
                __items.push_back(it);
            }
            std::sort(__items.begin(), __items.end(), [](const Item &_a, const Item &_b) { return _a.lo < _b.lo; });
            const auto accept = [&](uint32_t _a, uint32_t _b) {
                const SolverBody &a = bodies_[_a], &b = bodies_[_b];
                if (!a.dynamic && !b.dynamic) return false;
                if (!(a.collider->group & b.collider->mask) || !(b.collider->group & a.collider->mask)) return false;
                return jointed_.find(std::make_pair(std::min(_a, _b) * 1ull, std::max(_a, _b) * 1ull)) == jointed_.end();
            };
            for (size_t i = 0u; i < __items.size(); ++i) {
                for (size_t j = i + 1u; j < __items.size() && __items[j].lo <= __items[i].hi; ++j) {
                    // Full bounding sphere test
                    const float ra = 0.5f * (__items[i].hi - __items[i].lo), rb = 0.5f * (__items[j].hi - __items[j].lo);
                    const vec3 d = states_[__items[i].body].x - states_[__items[j].body].x;
                    if (Math::dot(d, d) > (ra + rb) * (ra + rb)) continue;
                    if (accept(__items[i].body, __items[j].body)) pairs_.push_back(std::make_pair(__items[i].body, __items[j].body));
                }
            }
            for (uint32_t p : __planes)
                for (const Item &it : __items)
                    if (accept(it.body, p)) pairs_.push_back(std::make_pair(it.body, p));
            stats.pairs = (uint32_t)pairs_.size();
        }

        void World::collide_() {
            contacts_.clear();
            std::vector<ShapeContact> &__found = found_;
            for (const std::pair<uint32_t, uint32_t> &pair : pairs_) {
                const SolverBody &A = bodies_[pair.first], &B = bodies_[pair.second];
                const SolverState &a = states_[pair.first], &b = states_[pair.second];
                // Cheap bounding sphere test with the current positions
                const float __reach = A.radius + B.radius + settings.contact_margin;
                if (A.collider->shape != SHAPE_PLANE && B.collider->shape != SHAPE_PLANE) {
                    const vec3 d = a.x - b.x;
                    if (Math::dot(d, d) > __reach * __reach) continue;
                }
                __found.clear();
                collideShapes(*A.collider, a.x, a.q, *B.collider, b.x, b.q, __found);
                for (const ShapeContact &f : __found) {
                    Contact c;
                    c.a = pair.first; c.b = pair.second;
                    c.normal = f.normal;
                    // Body-local points: the solver follows them while the bodies move
                    c.ra = invRot(a.q, f.point_a - a.x);
                    c.rb = invRot(b.q, f.point_b - b.x);
                    c.depth = f.depth;
                    c.lambda_n = 0.f;
                    c.friction    = A.collider->friction * B.collider->friction;
                    c.restitution = A.collider->restitution * B.collider->restitution;
                    const vec3 __ra = f.point_a - a.x, __rb = f.point_b - b.x;
                    const vec3 __v = (a.v + Math::cross(a.w, __ra)) - (b.v + Math::cross(b.w, __rb));
                    c.vn = Math::dot(c.normal, __v);
                    contacts_.push_back(c);
                }
            }
            stats.contacts = (uint32_t)contacts_.size();
        }

        void World::solveJoint_(const SolverJoint &_sj, float _h) {
            const Joint &J = *_sj.joint;
            SS &A = states_[_sj.a], &B = states_[_sj.b];
            const float __h2 = _h * _h;
            const Quat __frame_a = J.frame_a, __frame_b = J.frame_b;
            const bool __angular_springs = J.angular_spring.x > 0.f || J.angular_spring.y > 0.f || J.angular_spring.z > 0.f;
            const bool __linear_springs  = J.linear_spring.x  > 0.f || J.linear_spring.y  > 0.f || J.linear_spring.z  > 0.f;

            // Rotation limits: clamp the Euler angles of B's frame in A's frame, rotate B's frame towards the clamped rotation
            {
                const Quat FA = A.q * __frame_a, FB = B.q * __frame_b;
                const vec3 e = eulerXYZ(FA.conjugate() * FB);
                const vec3 t = { limitAxis(e.x, J.angular_min.x, J.angular_max.x),
                                 limitAxis(e.y, J.angular_min.y, J.angular_max.y),
                                 limitAxis(e.z, J.angular_min.z, J.angular_max.z) };
                if (std::fabs(e.x - t.x) + std::fabs(e.y - t.y) + std::fabs(e.z - t.z) > EPS) {
                    const vec3 dphi = rotationVector(FB * (FA * fromEulerXYZ(t)).conjugate());
                    const float angle = dphi.norm();
                    if (angle > EPS) angular(B, A, dphi * (1.f / angle), angle, 0.f);
                }
                // Angular springs (towards zero angles), per axis of A's frame
                if (__angular_springs) {
                    const Quat FA2 = A.q * __frame_a, FB2 = B.q * __frame_b;
                    const vec3 e2 = eulerXYZ(FA2.conjugate() * FB2);
                    for (uint32_t i = 0u; i < 3u; ++i) {
                        const float k = J.angular_spring[i];
                        if (k <= 0.f || std::fabs(e2[i]) < EPS) continue;
                        angular(B, A, rot(FA2, Mat::Eye3[i]), e2[i], 1.f / (k * __h2));
                    }
                }
            }
            // Translation limits: B's anchor in A's frame
            {
                const Quat FA = A.q * __frame_a;
                const vec3 rA = rot(A.q, J.anchor_a), rB = rot(B.q, J.anchor_b);
                const vec3 d = invRot(FA, (B.x + rB) - (A.x + rA));
                const vec3 t = { limitAxis(d.x, J.linear_min.x, J.linear_max.x),
                                 limitAxis(d.y, J.linear_min.y, J.linear_max.y),
                                 limitAxis(d.z, J.linear_min.z, J.linear_max.z) };
                const vec3 err = rot(FA, d - t);
                const float c = err.norm();
                if (c > EPS) positional(B, rB, A, rA, err * (1.f / c), c, 0.f);
            }
            if (__linear_springs) {
                const Quat FA = A.q * __frame_a;
                const vec3 rA = rot(A.q, J.anchor_a), rB = rot(B.q, J.anchor_b);
                const vec3 d = invRot(FA, (B.x + rB) - (A.x + rA));
                for (uint32_t i = 0u; i < 3u; ++i) {
                    const float k = J.linear_spring[i];
                    if (k <= 0.f || std::fabs(d[i]) < EPS) continue;
                    positional(B, rB, A, rA, rot(FA, Mat::Eye3[i]), d[i], 1.f / (k * __h2));
                }
            }
        }

        void World::solveContacts_(float _h) {
            (void)_h;
            for (Contact &c : contacts_) {
                const SolverBody &SA = bodies_[c.a], &SB_ = bodies_[c.b];
                SS &A = states_[c.a], &B = states_[c.b];
                vec3 ra = rot(A.q, c.ra), rb = rot(B.q, c.rb);
                const float d = Math::dot((B.x + rb) - (A.x + ra), c.normal);
                if (d <= 0.f) continue;
                // Normal: push A along the normal
                c.lambda_n = -positional(A, ra, B, rb, c.normal * -1.f, d, 0.f);
                if (SA.report || SB_.report) {
                    const uint64_t ha = SA.entity.hash(), hb = SB_.entity.hash();
                    touching_.insert(std::make_pair(std::min(ha, hb), std::max(ha, hb)));
                }
                // Static friction: undo the tangential sliding of the contact points during this substep
                ra = rot(A.q, c.ra); rb = rot(B.q, c.rb);
                const vec3 p1 = A.x + ra, p2 = B.x + rb;
                const vec3 p1_prev = A.px + rot(A.pq, c.ra);
                const vec3 p2_prev = B.px + rot(B.pq, c.rb);
                const vec3 dp = (p1 - p1_prev) - (p2 - p2_prev);
                const vec3 dpt = dp - c.normal * Math::dot(dp, c.normal);
                const float lt = dpt.norm();
                if (lt <= EPS) continue;
                const vec3 dir = dpt * (1.f / lt);
                const float w = genInvMass(A, ra, dir) + genInvMass(B, rb, dir);
                if (w <= EPS || lt / w > c.friction * c.lambda_n) continue;
                positional(A, ra, B, rb, dir, lt, 0.f);
            }
        }

        void World::solveVelocities_(float _h) {
            const float __g = settings.gravity.norm();
            for (const Contact &c : contacts_) {
                if (c.lambda_n <= 0.f) continue;
                SS &A = states_[c.a], &B = states_[c.b];
                const vec3 ra = rot(A.q, c.ra), rb = rot(B.q, c.rb);
                const vec3 v = (A.v + Math::cross(A.w, ra)) - (B.v + Math::cross(B.w, rb));
                const float vn = Math::dot(c.normal, v);
                const vec3 vt = v - c.normal * vn;
                const float lvt = vt.norm();
                vec3 dv = {0.f, 0.f, 0.f};
                // Dynamic friction
                if (lvt > EPS) {
                    const float fn = c.lambda_n / (_h * _h);
                    dv -= vt * (std::min(_h * c.friction * fn, lvt) / lvt);
                }
                // Restitution (none for slow contacts: resting bodies don't jitter)
                const float e = std::fabs(c.vn) <= 2.f * __g * _h ? 0.f : c.restitution;
                dv += c.normal * (-vn + std::max(-e * c.vn, 0.f));
                const float ldv = dv.norm();
                if (ldv <= EPS) continue;
                const vec3 dir = dv * (1.f / ldv);
                const float w = genInvMass(A, ra, dir) + genInvMass(B, rb, dir);
                if (w <= EPS) continue;
                const vec3 p = dv * (1.f / w);
                if (A.im > 0.f) { A.v += p * A.im; A.w += invI(A, Math::cross(ra, p)); }
                if (B.im > 0.f) { B.v -= p * B.im; B.w -= invI(B, Math::cross(rb, p)); }
            }
        }

        void World::reportContacts_() {
            if (events) {
                for (const auto &pair : touching_)
                    if (touching_prev_.find(pair) == touching_prev_.end()) {
                        ContactEvent ev = {};
                        ev.body_a = EntityID((uint32_t)pair.first, (uint32_t)(pair.first >> 32));
                        ev.body_b = EntityID((uint32_t)pair.second, (uint32_t)(pair.second >> 32));
                        // The deepest contact of the pair for the point and the normal
                        float __best = -1.f;
                        for (const Contact &c : contacts_) {
                            const uint64_t ha = bodies_[c.a].entity.hash(), hb = bodies_[c.b].entity.hash();
                            if (std::make_pair(std::min(ha, hb), std::max(ha, hb)) != pair || c.depth <= __best) continue;
                            __best = c.depth;
                            const vec3 p = states_[c.a].x + rot(states_[c.a].q, c.ra);
                            const vec3 n = ha == pair.first ? c.normal : c.normal * -1.f;
                            ev.point[0] = p.x; ev.point[1] = p.y; ev.point[2] = p.z;
                            ev.normal[0] = n.x; ev.normal[1] = n.y; ev.normal[2] = n.z;
                            ev.depth = c.depth;
                        }
                        events->write(Event::Event::make(Event::EV_TYPE_PHYSICS_CONTACT_BEGIN, ev, ev.body_a));
                    }
                for (const auto &pair : touching_prev_)
                    if (touching_.find(pair) == touching_.end()) {
                        ContactEvent ev = {};
                        ev.body_a = EntityID((uint32_t)pair.first, (uint32_t)(pair.first >> 32));
                        ev.body_b = EntityID((uint32_t)pair.second, (uint32_t)(pair.second >> 32));
                        events->write(Event::Event::make(Event::EV_TYPE_PHYSICS_CONTACT_END, ev, ev.body_a));
                    }
            }
            touching_prev_.swap(touching_);
            touching_.clear();
        }

        uint32_t World::update(float _dt) {
            stats.steps = 0u;
            if (_dt <= 0.f || settings.fixed_step <= 0.f) return 0u;
            accumulator_ += _dt;
            uint32_t __steps = (uint32_t)(accumulator_ / settings.fixed_step);
            if (__steps > settings.max_steps) { __steps = settings.max_steps; accumulator_ = 0.f; }
            else accumulator_ -= __steps * settings.fixed_step;
            if (__steps == 0u) return 0u;
            // Kinematic bodies move from their current pose to their target over all steps of this update
            ecs.each<Body>([](EntityID, Body &_b) { _b.frame_position = _b.position; _b.frame_rotation = _b.rotation; });
            for (uint32_t s = 0u; s < __steps; ++s) {
                stepFraction_ = std::make_pair(s, __steps);
                step(settings.fixed_step);
            }
            stepFraction_ = std::make_pair(0u, 1u);
            stats.steps = __steps;
            return __steps;
        }

        void World::step(float _step) {
            if (stepFraction_.first == 0u && stepFraction_.second == 1u)
                ecs.each<Body>([](EntityID, Body &_b) { _b.frame_position = _b.position; _b.frame_rotation = _b.rotation; });
            gather_();
            broadPhase_(_step);
            const uint32_t __n = std::max(settings.substeps, 1u);
            const float h = _step / (float)__n;
            // Damping per substep (Bullet style: the fraction lost per second)
            std::vector<float> &__ld = damping_;
            __ld.resize(bodies_.size() * 2u);
            for (size_t i = 0u; i < bodies_.size(); ++i) {
                __ld[2u * i]      = std::pow(Math::clampCL(1.f - bodies_[i].body->linear_damping,  0.f, 1.f), h);
                __ld[2u * i + 1u] = std::pow(Math::clampCL(1.f - bodies_[i].body->angular_damping, 0.f, 1.f), h);
            }
            for (uint32_t sub = 0u; sub < __n; ++sub) {
                const float __f = ((float)stepFraction_.first + (float)(sub + 1u) / (float)__n) / (float)stepFraction_.second;
                // Integrate
                for (size_t i = 0u; i < bodies_.size(); ++i) {
                    const SolverBody &s = bodies_[i];
                    SolverState &st = states_[i];
                    st.px = st.x;
                    st.pq = st.q;
                    if (s.dynamic) {
                        const Body &b = *s.body;
                        st.v += (settings.gravity * b.gravity_scale + b.force * st.im) * h;
                        st.w += invI(st, b.torque) * h;
                        st.v *= __ld[2u * i];
                        st.w *= __ld[2u * i + 1u];
                        st.x += st.v * h;
                        addRotation(st.q, st.w * h);
                        updateInvInertia(st);
                    } else if (st.has_target) {
                        st.x = Interpolation::BezierLinear<vec3>::Interpolate(__f, st.fx, st.tx);
                        st.q = nlerp(st.fq, st.tq, __f);
                    }
                }
                collide_();
                // Positions
                for (const SolverJoint &j : joints_) solveJoint_(j, h);
                solveContacts_(h);
                // Velocities from the positions
                const float __inv_h = 1.f / h;
                for (size_t i = 0u; i < bodies_.size(); ++i) {
                    const SolverBody &s = bodies_[i];
                    SolverState &st = states_[i];
                    if (!s.dynamic && !s.kinematic) { st.v = {0.f, 0.f, 0.f}; st.w = {0.f, 0.f, 0.f}; continue; }
                    st.v = (st.x - st.px) * __inv_h;
                    st.w = rotationVector(st.q * st.pq.conjugate()) * __inv_h;
                    if (s.dynamic) {
                        const float lv = st.v.norm(), lw = st.w.norm();
                        if (lv > settings.max_velocity) st.v *= settings.max_velocity / lv;
                        if (lw > settings.max_angular_velocity) st.w *= settings.max_angular_velocity / lw;
                    }
                }
                solveVelocities_(h);
            }
            // Back to the components
            for (size_t i = 0u; i < bodies_.size(); ++i) {
                Body &b = *bodies_[i].body;
                const SolverState &st = states_[i];
                b.position = st.x;           b.rotation = st.q;
                b.prev_position = st.px;     b.prev_rotation = st.pq;
                b.velocity = st.v;           b.angular_velocity = st.w;
                b.force  = {0.f, 0.f, 0.f};
                b.torque = {0.f, 0.f, 0.f};
            }
            reportContacts_();
        }
    }; // Physics END
}; // Simple END
