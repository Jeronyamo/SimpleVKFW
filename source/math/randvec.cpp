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
}; // Simple END
