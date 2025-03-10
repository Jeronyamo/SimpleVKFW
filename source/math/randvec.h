#ifndef SVKFW_RANDVEC_H
#define SVKFW_RANDVEC_H

#include "vectors.h"

#include <random>

// TODO: check all this stuff

//#define VEC_RAND_MAX 2147483646u
//#define VEC_RANDI_MAX RAND_MAX
//#define VEC_RANDF_MAX 1.f


namespace Simple {
    std::minstd_rand vecRandNG;
    std::uniform_real_distribution <float> vecRandNGf(0.f, 1.f);
    std::uniform_int_distribution <int> vecRandNGi(-RAND_MAX, RAND_MAX);

    inline vec2u rand2() { return {vecRandNG(), vecRandNG()}; }
    inline vec3u rand3() { return {vecRandNG(), vecRandNG(), vecRandNG()}; }
    inline vec4u rand4() { return {vecRandNG(), vecRandNG(), vecRandNG(), vecRandNG()}; }

    inline vec2i rand2i() { return {vecRandNGi(vecRandNG), vecRandNGi(vecRandNG)}; }
    inline vec3i rand3i() { return {vecRandNGi(vecRandNG), vecRandNGi(vecRandNG), vecRandNGi(vecRandNG)}; }
    inline vec4i rand4i() { return {vecRandNGi(vecRandNG), vecRandNGi(vecRandNG), vecRandNGi(vecRandNG), vecRandNGi(vecRandNG)}; }

    inline vec2  rand2f() { return {vecRandNGf(vecRandNG), vecRandNGf(vecRandNG)}; }
    inline vec3  rand3f() { return {vecRandNGf(vecRandNG), vecRandNGf(vecRandNG), vecRandNGf(vecRandNG)}; }
    inline vec4  rand4f() { return {vecRandNGf(vecRandNG), vecRandNGf(vecRandNG), vecRandNGf(vecRandNG), vecRandNGf(vecRandNG)}; }

    inline vec2 RandVec2(float _max = 1.f) { return rand2f() * _max; }
    inline vec3 RandVec3(float _max = 1.f) { return rand3f() * _max; }
    inline vec4 RandVec4(float _max = 1.f) { return rand4f() * _max; }

    inline vec2 RandVec2(vec2 _max) { return rand2f() * _max; }
    inline vec3 RandVec3(vec3 _max) { return rand3f() * _max; }
    inline vec4 RandVec4(vec4 _max) { return rand4f() * _max; }

    inline vec2 RandVec2(float _max, float _min) { return rand2f() * (_max - _min) + _min; }
    inline vec3 RandVec3(float _max, float _min) { return rand3f() * (_max - _min) + _min; }
    inline vec4 RandVec4(float _max, float _min) { return rand4f() * (_max - _min) + _min; }

    inline vec2 RandVec2(vec2 _max, vec2 _min) { return rand2f() * (_max - _min) + _min; }
    inline vec3 RandVec3(vec3 _max, vec3 _min) { return rand3f() * (_max - _min) + _min; }
    inline vec4 RandVec4(vec4 _max, vec4 _min) { return rand4f() * (_max - _min) + _min; }
}; // Simple END

#endif