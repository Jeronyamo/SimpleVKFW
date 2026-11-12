#include "math/randvec.h"

namespace Simple {

    std::minstd_rand vecRandNG;

    std::uniform_real_distribution <float> vecRandNGf(0.f, 1.f);

    std::uniform_int_distribution <int> vecRandNGi(-RAND_MAX, RAND_MAX);

    // Note: minstd_rand returns uint_fast32_t (64-bit on Linux), clang rejects narrowing it in {}
    vec2u rand2() { return {uint32_t(vecRandNG()), uint32_t(vecRandNG())}; }

    vec3u rand3() { return {uint32_t(vecRandNG()), uint32_t(vecRandNG()), uint32_t(vecRandNG())}; }

    vec4u rand4() { return {uint32_t(vecRandNG()), uint32_t(vecRandNG()), uint32_t(vecRandNG()), uint32_t(vecRandNG())}; }

    vec2i rand2i() { return {vecRandNGi(vecRandNG), vecRandNGi(vecRandNG)}; }

    vec3i rand3i() { return {vecRandNGi(vecRandNG), vecRandNGi(vecRandNG), vecRandNGi(vecRandNG)}; }

    vec4i rand4i() { return {vecRandNGi(vecRandNG), vecRandNGi(vecRandNG), vecRandNGi(vecRandNG), vecRandNGi(vecRandNG)}; }

    vec2  rand2f() { return {vecRandNGf(vecRandNG), vecRandNGf(vecRandNG)}; }

    vec3  rand3f() { return {vecRandNGf(vecRandNG), vecRandNGf(vecRandNG), vecRandNGf(vecRandNG)}; }

    vec4  rand4f() { return {vecRandNGf(vecRandNG), vecRandNGf(vecRandNG), vecRandNGf(vecRandNG), vecRandNGf(vecRandNG)}; }

    vec2 RandVec2(float _max) { return rand2f() * _max; }

    vec3 RandVec3(float _max) { return rand3f() * _max; }

    vec4 RandVec4(float _max) { return rand4f() * _max; }

    vec2 RandVec2(vec2 _max) { return rand2f() * _max; }

    vec3 RandVec3(vec3 _max) { return rand3f() * _max; }

    vec4 RandVec4(vec4 _max) { return rand4f() * _max; }

    vec2 RandVec2(float _max, float _min) { return rand2f() * (_max - _min) + _min; }

    vec3 RandVec3(float _max, float _min) { return rand3f() * (_max - _min) + _min; }

    vec4 RandVec4(float _max, float _min) { return rand4f() * (_max - _min) + _min; }

    vec2 RandVec2(vec2 _max, vec2 _min) { return rand2f() * (_max - _min) + _min; }

    vec3 RandVec3(vec3 _max, vec3 _min) { return rand3f() * (_max - _min) + _min; }

    vec4 RandVec4(vec4 _max, vec4 _min) { return rand4f() * (_max - _min) + _min; }

    namespace Hash {
        uint32_t pcgScramblerU32(uint32_t _val_in) {
            _val_in = _val_in * 747796405u + 2891336453u;
            _val_in = ((_val_in >> ((_val_in >> 28u) + 4u)) ^ _val_in) * 277803737u;
            return (_val_in >> 22u) ^ _val_in;
        }
        uint32_t murmurScramblerU32(uint32_t _val_in) {
            _val_in *= 3432918353u;
            _val_in = (_val_in << 15u) | (_val_in >> 17u);
            _val_in *= 461845907u;
            return _val_in;
        }

        uint32_t murmur3U32(const uint8_t* _val_in, uint32_t _val_len, uint32_t _seed) {
            uint32_t h = _seed;
            uint32_t k;

            for (size_t i = _val_len >> 2u; i; i--) {
                memcpy(&k, _val_in, sizeof(uint32_t));
                _val_in += sizeof(uint32_t);
                h ^= murmurScramblerU32(k);
                h = (h << 13u) | (h >> 19u);
                h = h * 5u + 3864292196u;
            }

            k = 0u;
            for (uint32_t i = _val_len & 3u; i > 0u; --i) {
                k <<= 8u;
                k |= _val_in[i - 1];
            }

            h ^= murmurScramblerU32(k);
            h ^= _val_len;
            h ^= h >> 16u;
            h *= 2246822507u;
            h ^= h >> 13u;
            h *= 3266489909u;
            h ^= h >> 16u;
            return h;
        }

        uint32_t squirrel5U32(uint32_t _val_in, uint32_t _seed) {
            _val_in *= 0xD2A80A3F;
            _val_in += _seed;
            _val_in ^= (_val_in >> 9u);
            _val_in += 0xA884F197;
            _val_in ^= (_val_in >> 11u);
            _val_in *= 0x6C736F4B;
            _val_in ^= (_val_in >> 15u);
            _val_in += 0xB79F3CAC;
            _val_in ^= (_val_in >> 17u);
            _val_in *= 0x6B97E5D1;
            return _val_in;
        }

        uint32_t getVecHash2(vec2u _ind2d, uint32_t _noise_seed) {
            return squirrel5U32(_ind2d.x +
                    squirrel5U32(_ind2d.y, _noise_seed), _noise_seed);
        }
        uint32_t getVecHash3(vec3u _ind3d, uint32_t _noise_seed) {
            return squirrel5U32(_ind3d.x +
                    squirrel5U32(_ind3d.y +
                     squirrel5U32(_ind3d.z, _noise_seed), _noise_seed), _noise_seed);
        }
        uint32_t getVecHash4(vec4u _ind4d, uint32_t _noise_seed) {
            return squirrel5U32(_ind4d.x +
                    squirrel5U32(_ind4d.y +
                     squirrel5U32(_ind4d.z +
                      squirrel5U32(_ind4d.w, _noise_seed), _noise_seed), _noise_seed), _noise_seed);
        }
    }; // Hash END

    namespace Noise {
        vec2f getDot2f(uint32_t _val_hash, vec2f _d) {
            _val_hash &= 7u;
            _val_hash += 1u;

            vec2f __res{ float(_val_hash / 3), float(_val_hash % 3) };
            __res.x = __res.x == 2.f ? -_d.x : __res.x * _d.x;
            __res.y = __res.y == 2.f ? -_d.y : __res.y * _d.y;
            return __res;
        }
        float getDot3f(uint32_t _val_hash, vec3f _d) {
            uint8_t h = _val_hash & 15u;

            float u = h < 8u ? _d.x : _d.y;
            float v = h < 4u ? _d.y : ((h == 12u || h == 14u) ? _d.x : _d.z);

            float first_term  = (h & 1u) ? -u : u;
            float second_term = (h & 2u) ? -v : v;
            return first_term + second_term;
        }
        float getDot4f(uint32_t _val_hash, vec4f _d) {
            _val_hash &= 31u;
            int8_t __sign1 = (_val_hash & 1u) ? -1 : 1,
                   __sign2 = (_val_hash & 2u) ? -1 : 1;
            _val_hash >>= 2u;
            int8_t __ind1 = ((_val_hash + 5) & 7), __ind2 = ((_val_hash + 2) & 7);
            __ind1 = __ind1 < 3 ? 1 + (__ind1==2) : 0;
            __ind2 = __ind2 < 4 ? 1 + (__ind2& 1) : 3 - (__ind2==5);
            return __sign1 * _d[__ind1] + __sign2 * _d[__ind2];
        }


        NoisePerlin::NoisePerlin(uint32_t _noise_seed) : noise_seed{_noise_seed} {}

        float NoisePerlin::sample(vec4f _coords) const {
            vec4i __ind4d = _coords.cast<int32_t>();

            float points[8];
            for (int i = 0; i < 16; ++i) {
                vec4i __n_coords = __ind4d + vec4i{(i & 8) != 0, (i & 4) != 0, (i & 2) != 0, (i & 1) != 0};
                float __curr_pt = getDot4f(Hash::getVecHash4(__n_coords.cast<uint32_t>(), noise_seed), _coords - __n_coords);
                points[i>>1] = i & 1 ? points[i>>1] + _coords.w * (__curr_pt - points[i>>1]) : __curr_pt;
            }
            for (int i = 0; i < 8; i+=2)
                points[i>>1] = points[i] + _coords.z * (points[i+1] - points[i]);
            for (int i = 0; i < 4; i+=2)
                points[i>>1] = points[i] + _coords.y * (points[i+1] - points[i]);
            return points[0] + _coords.x * (points[1] - points[0]);
        }

        NoiseSimplex::NoiseSimplex(uint32_t _noise_seed) : noise_seed{_noise_seed} {}

        float NoiseSimplex::sample(vec3f _coords) {
            vec3f __tformed   =  _coords + Math::sum(_coords) * Simplex_F3;
            vec3i __ind3d     = __tformed.cast<int32_t>();
            vec3f __pt_deltas = __tformed - __ind3d;

            vec3i8 __axes_order{(int8_t)Math::amaxOf(__pt_deltas), 0, (int8_t)Math::aminOf(__pt_deltas)}, __p_offs{};
            __axes_order.y = 3 - __axes_order.x - __axes_order.z;

            vec3f __n_coords = __ind3d + Math::sum(__ind3d) * Simplex_G3,
                  __v_dir0   =  _coords - __n_coords;

            const float r2 = 0.5f;
            float __res = 0.f;
            for (int i = 0; i < 4; ++i) {
                if (i) __p_offs[__axes_order[i-1]] += 1;
                vec3f __v_dir = __v_dir0 - __p_offs + Math::sum(__p_offs) * Simplex_G3;
                float t = r2 - __v_dir.norm2();
                if (t > 0.f) {
                    t *= t; t *= t;
                    t *= getDot3f(Hash::getVecHash3(__ind3d.cast<uint32_t>(), noise_seed), __v_dir);
                    __res += t;
                }
            }
            return Simplex_M3 * __res;
        }

        float NoiseSimplex::sample(vec4f _coords) {
            vec4f __tformed   =  _coords + Math::sum(_coords) * Simplex_F4;
            vec4i __ind4d     = __tformed.cast<int32_t>();
            vec4f __pt_deltas = __tformed - __ind4d;

            vec4i8 __axes_order{(int8_t)Math::amaxOf(__pt_deltas), 0, 0, (int8_t)Math::aminOf(__pt_deltas)}, __p_offs{};
            if (__axes_order.x == __axes_order.w)
                __axes_order.x = (__axes_order.x + 1) & 3;

            __axes_order.y = (__axes_order.x + 1) & 3;
            if (__axes_order.y == __axes_order.w)
                __axes_order.y = (__axes_order.x + 3) & 3;

            __axes_order.z = (__axes_order.x + 2) & 3;
            if (__axes_order.z == __axes_order.w)
                __axes_order.z = (__axes_order.x + 3) & 3;

            if (__pt_deltas[__axes_order.y] < __pt_deltas[__axes_order.z])
                std::swap(__axes_order.y, __axes_order.z);


            vec4f __n_coords = __ind4d + Math::sum(__ind4d) * Simplex_G4,
                    __v_dir0   = _coords - __n_coords;

            const float r2 = 0.5f;
            float __res = 0.f;
            for (int i = 0; i < 5; ++i) {
                if (i) __p_offs[__axes_order[i-1]] += 1;
                vec4f __v_dir = __v_dir0 - __p_offs + Math::sum(__p_offs) * Simplex_G3;
                float t = r2 - __v_dir.norm2();
                if (t > 0.f) {
                    t *= t; t *= t;
                    t *= getDot4f(Hash::getVecHash4(__ind4d.cast<uint32_t>(), noise_seed), __v_dir);
                    __res += t;
                }
            }
            return Simplex_M4 * __res;
        }
    }; // Noise END

}; // Simple END
