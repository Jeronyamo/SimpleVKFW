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
}; // Simple END

#endif