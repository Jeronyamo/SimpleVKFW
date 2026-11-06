#version 450

// Markers (see 'include/scene/scene_markers.h'): vertex colors, the whole transform and a tint in push constants.
// Note: the matrix is written row-vector style on the CPU, GLSL reads it transposed, so 'M * v' here is 'v & M' there

layout(push_constant) uniform Marker {
    mat4 mvp;
    vec4 tint; // multiplies the vertex color (alpha: opacity)
} marker;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec4 out_color;

void main() {
    out_color   = vec4(in_color, 1.0) * marker.tint;
    gl_Position = marker.mvp * vec4(in_position, 1.0);
}
