#version 450

// Scene renderer (see 'include/scene/scene_renderer.h'): set 0 = frame, set 1 = material, push constants = node matrices.
// Note: the matrices are written row-vector style on the CPU, GLSL reads them transposed, so 'M * v' here is 'v & M' there

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

layout(push_constant) uniform Node {
    mat4 model;
    mat4 normal;
} node;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_tangent;

layout(location = 0) out vec3 out_world_pos;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_uv;
layout(location = 3) out vec4 out_tangent;

void main() {
    vec4 world = node.model * vec4(in_position, 1.0);
    out_world_pos = world.xyz;
    out_normal    = mat3(node.normal) * in_normal;
    out_tangent   = vec4(mat3(node.model) * in_tangent.xyz, in_tangent.w);
    out_uv        = in_uv;
    gl_Position   = frame.proj * frame.view * world;
}
