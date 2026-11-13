#ifndef SVKFW_RANDVEC_H
#define SVKFW_RANDVEC_H

#include "vectors.h"

#include <random>

// TODO: check all this stuff

//#define VEC_RAND_MAX 2147483646u
//#define VEC_RANDI_MAX RAND_MAX
//#define VEC_RANDF_MAX 1.f


namespace Simple {
    extern std::minstd_rand vecRandNG;
    extern std::uniform_real_distribution <float> vecRandNGf;
    extern std::uniform_int_distribution <int> vecRandNGi;

    vec2u rand2();
    vec3u rand3();
    vec4u rand4();

    vec2i rand2i();
    vec3i rand3i();
    vec4i rand4i();

    vec2  rand2f();
    vec3  rand3f();
    vec4  rand4f();

    vec2 RandVec2(float _max = 1.f);
    vec3 RandVec3(float _max = 1.f);
    vec4 RandVec4(float _max = 1.f);

    vec2 RandVec2(vec2 _max);
    vec3 RandVec3(vec3 _max);
    vec4 RandVec4(vec4 _max);

    vec2 RandVec2(float _max, float _min);
    vec3 RandVec3(float _max, float _min);
    vec4 RandVec4(float _max, float _min);

    vec2 RandVec2(vec2 _max, vec2 _min);
    vec3 RandVec3(vec3 _max, vec3 _min);
    vec4 RandVec4(vec4 _max, vec4 _min);


    namespace Hash {
        uint32_t pcgScramblerU32(uint32_t _val_in);
        uint32_t murmurScramblerU32(uint32_t _val_in);

        uint32_t murmur3U32(const uint8_t* _val_in, uint32_t _val_len, uint32_t _seed);
        uint32_t squirrel5U32(uint32_t _val_in, uint32_t _seed = 0u);

        uint32_t getVecHash2(vec2u _ind2d, uint32_t _noise_seed);
        uint32_t getVecHash3(vec3u _ind3d, uint32_t _noise_seed);
        uint32_t getVecHash4(vec4u _ind4d, uint32_t _noise_seed);
    }; // Hash END


    namespace Noise {
        struct NoisePerlin {
            uint32_t noise_seed;

            NoisePerlin(uint32_t _noise_seed = 42u);

            float sample(vec4f _coords) const;
        }; // NoisePerlin END

        struct NoiseSimplex {
            static constexpr float Simplex_F2 = 0.366025403f;
            static constexpr float Simplex_F3 = 1.f / 3;
            static constexpr float Simplex_F4 = 0.309017f;
            static constexpr float Simplex_G2 = 0.211324865f;
            static constexpr float Simplex_G3 = 1.f / 6;
            static constexpr float Simplex_G4 = 0.138197f;
            // Output scales: about [-1, 1] with the kernel radius^2 0.5 (3D, 4D measured: max 0.99)
            static constexpr float Simplex_M2 = 32.f;
            static constexpr float Simplex_M3 = 76.f;
            static constexpr float Simplex_M4 = 76.f;

            uint32_t noise_seed;

            NoiseSimplex(uint32_t _noise_seed = 42u);

            float sample(vec3f _coords) const;
            float sample(vec4f _coords) const;
        }; // NoiseSimplex END
    }; // Noise END
}; // Simple END

#endif