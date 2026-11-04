#version 450

// Scene renderer (see 'include/scene/scene_renderer.h'): set 0 = frame, set 1 = material, push constants = node matrices.
// Note: the matrices are written row-vector style on the CPU, GLSL reads them transposed, so 'M * v' here is 'v & M' there
// Note: compiled twice: as is (rigid meshes) and with -DSKINNED (scene_skin_vert.spv): the skinning matrices of the skin start
//       at 'gl_InstanceIndex' (the first instance of the draw)

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
layout(set = 0, binding = 1, std430) readonly buffer Joints {
    mat4 joints[];
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
#endif

layout(location = 0) out vec3 out_world_pos;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_uv;
layout(location = 3) out vec4 out_tangent;

#ifdef SKINNED
// Linear blend skinning; the bones are assumed without non-uniform scale, so normals use the same matrix
mat4 skinMatrix() {
    uint base = uint(gl_InstanceIndex);
    return in_weights.x * joints[base + in_bones.x] + in_weights.y * joints[base + in_bones.y]
         + in_weights.z * joints[base + in_bones.z] + in_weights.w * joints[base + in_bones.w];
}
#else
mat4 skinMatrix() { return mat4(1.0); }
#endif

void main() {
    mat4 skin    = skinMatrix();
    vec3 normal  = mat3(skin) * in_normal;
    vec3 tangent = mat3(skin) * in_tangent.xyz;
    vec4 world   = node.model * (skin * vec4(in_position, 1.0));
    out_world_pos = world.xyz;
    out_normal    = mat3(node.normal) * normal;
    out_tangent   = vec4(mat3(node.model) * tangent, in_tangent.w);
    out_uv        = in_uv;
    gl_Position   = frame.proj * frame.view * world;
}
