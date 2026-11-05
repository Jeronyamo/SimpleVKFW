#version 450

// Scene renderer (see 'include/scene/scene_renderer.h'): set 0 = frame, set 1 = material, push constants = node matrices.
// Note: the matrices are written row-vector style on the CPU, GLSL reads them transposed, so 'M * v' here is 'v & M' there
// Note: compiled twice: as is (rigid meshes) and with -DSKINNED (scene_skin_vert.spv): 'gl_InstanceIndex' (the first instance
//       of the draw) is the skin; its first joint and first morph weight are in 'skin_data'. Each vertex adds its morph offsets,
//       then it is skinned linearly (LBS) or with dual quaternions (DQS)

struct Light {
    vec4 position_range;
    vec4 direction_type;
    vec4 color_intensity;
    vec4 cone;
};

layout(set = 0, binding = 0) uniform Frame {
    mat4  view;
    mat4  proj;
    vec4  camera_position;
    vec4  ambient;
    uvec4 light_count;
    Light lights[16];
} frame;

#ifdef SKINNED
struct Joint {
    mat4 matrix; // LBS
    vec4 real;   // DQS: the rotation (xyzw)
    vec4 dual;   //      the translation part
};
layout(set = 0, binding = 1, std430) readonly buffer Joints {
    Joint joints[];
};
// Per skin: (first joint, first morph weight, 0, 0); then the morph weights (float bits)
layout(set = 0, binding = 2, std430) readonly buffer SkinData {
    uint skin_data[];
};
struct MorphDelta {
    vec3 offset; // position, or UV (xy) if the top bit of 'morph' is set
    uint morph;  // the morph in the skeleton
};
layout(set = 0, binding = 3, std430) readonly buffer MorphDeltas {
    MorphDelta morph_deltas[];
};
#endif

layout(push_constant) uniform Node {
    mat4 model;
    mat4 normal;
} node;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_tangent;
#ifdef SKINNED
layout(location = 4) in uvec4 in_bones;
layout(location = 5) in vec4  in_weights;
layout(location = 6) in uint  in_mode;  // 0: LBS, 1: DQS
layout(location = 7) in uvec2 in_morphs; // first offset in 'morph_deltas', count
#endif

layout(location = 0) out vec3 out_world_pos;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_uv;
layout(location = 3) out vec4 out_tangent;

#ifdef SKINNED
vec3 rotate(vec4 q, vec3 v) {
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

// The morph offsets of the vertex times the weights of their morphs (MMD keeps the normals)
void morph(inout vec3 position, inout vec2 uv) {
    uint weights = skin_data[4u * uint(gl_InstanceIndex) + 1u];
    for (uint i = 0u; i < in_morphs.y; ++i) {
        MorphDelta d = morph_deltas[in_morphs.x + i];
        float w = uintBitsToFloat(skin_data[weights + (d.morph & 0x7FFFFFFFu)]);
        if ((d.morph & 0x80000000u) != 0u) uv += w * d.offset.xy;
        else position += w * d.offset;
    }
}

// The skinned position, normal and tangent (in the skin space)
void skin(inout vec3 position, inout vec3 normal, inout vec3 tangent) {
    uint base = skin_data[4u * uint(gl_InstanceIndex)];
    uvec4 b = in_bones + uvec4(base);
    if (in_mode == 0u) {
        // Linear blend skinning; the bones are assumed without non-uniform scale, so normals use the same matrix
        mat4 m = in_weights.x * joints[b.x].matrix + in_weights.y * joints[b.y].matrix
               + in_weights.z * joints[b.z].matrix + in_weights.w * joints[b.w].matrix;
        position = (m * vec4(position, 1.0)).xyz;
        normal   = mat3(m) * normal;
        tangent  = mat3(m) * tangent;
        return;
    }
    // Dual quaternion skinning: blended in the hemisphere of the first bone, then normalized
    vec4 r0 = joints[b.x].real;
    vec4 real = in_weights.x * r0, dual = in_weights.x * joints[b.x].dual;
    float s1 = dot(r0, joints[b.y].real) < 0.0 ? -1.0 : 1.0;
    float s2 = dot(r0, joints[b.z].real) < 0.0 ? -1.0 : 1.0;
    float s3 = dot(r0, joints[b.w].real) < 0.0 ? -1.0 : 1.0;
    real += in_weights.y * s1 * joints[b.y].real + in_weights.z * s2 * joints[b.z].real + in_weights.w * s3 * joints[b.w].real;
    dual += in_weights.y * s1 * joints[b.y].dual + in_weights.z * s2 * joints[b.z].dual + in_weights.w * s3 * joints[b.w].dual;
    float len = length(real);
    real /= len;
    dual /= len;
    vec3 translation = 2.0 * (real.w * dual.xyz - dual.w * real.xyz + cross(real.xyz, dual.xyz));
    position = rotate(real, position) + translation;
    normal   = rotate(real, normal);
    tangent  = rotate(real, tangent);
}
#else
void morph(inout vec3 position, inout vec2 uv) {}
void skin(inout vec3 position, inout vec3 normal, inout vec3 tangent) {}
#endif

void main() {
    vec3 position = in_position, normal = in_normal, tangent = in_tangent.xyz;
    vec2 uv = in_uv;
    morph(position, uv);
    skin(position, normal, tangent);
    vec4 world = node.model * vec4(position, 1.0);
    out_world_pos = world.xyz;
    out_normal    = mat3(node.normal) * normal;
    out_tangent   = vec4(mat3(node.model) * tangent, in_tangent.w);
    out_uv        = uv;
    gl_Position   = frame.proj * frame.view * world;
}
