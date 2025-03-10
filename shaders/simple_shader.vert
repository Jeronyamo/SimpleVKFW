#version 450

// All scalar/vector types take 1 location, dvec3/dvec4 (anything bigger than 16 bytes?) take 2 locations
// A struct takes locations based on its members:
// https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)

// Also "component" (not for matrices)
layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} scene_camera;

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 out_color;

void main() {
    out_color = in_color;
    gl_Position = scene_camera.proj * scene_camera.view * vec4(in_position, 0.0, 1.0);
}