#version 450

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) in  vec3  inColor;
layout(location = 1) in  vec2  inUV;
layout(location = 0) out vec4 outColor;


void main() {
    // Note: the vertex color slightly tints the texture
    outColor = vec4(mix(vec3(1.0), inColor, 0.3), 1.0) * texture(tex_sampler, inUV);
}