#version 450

// Forward Blinn-Phong for the scene renderer (see 'include/scene/scene_renderer.h')

const uint LIGHT_DIRECTIONAL = 0u;
const uint LIGHT_POINT       = 1u;
const uint LIGHT_SPOT        = 2u;
const uint MAX_LIGHTS        = 16u;

struct Light {
    vec4 position_range;  // world position, range (0: unlimited)
    vec4 direction_type;  // world direction the light goes, type
    vec4 color_intensity;
    vec4 cone;            // cos(inner), cos(outer)
};

layout(set = 0, binding = 0) uniform Frame {
    mat4  view;
    mat4  proj;
    vec4  camera_position;
    vec4  ambient;
    uvec4 light_count;
    Light lights[MAX_LIGHTS];
} frame;

layout(set = 1, binding = 0) uniform Material {
    vec4 base_color;
    vec4 specular_shininess;
    vec4 emissive_normal_scale;
    vec4 alpha;           // cutoff (< 0: opaque), has normal map, two-sided
    vec4 texture_mul;     // texture tint (MMD material morphs): texel * mul + add
    vec4 texture_add;
} material;
layout(set = 1, binding = 1) uniform sampler2D base_color_tex;
layout(set = 1, binding = 2) uniform sampler2D normal_tex;
layout(set = 1, binding = 3) uniform sampler2D emissive_tex;

layout(location = 0) in vec3 in_world_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_tangent;

layout(location = 0) out vec4 out_color;

// Inverse square falloff, faded out to zero at 'range' (no window if range == 0)
float attenuation(float dist, float range) {
    float att = 1.0 / max(dist * dist, 0.01);
    if (range > 0.0) {
        float r = clamp(1.0 - pow(dist / range, 4.0), 0.0, 1.0);
        att *= r * r;
    }
    return att;
}

void main() {
    vec4 base = material.base_color * clamp(texture(base_color_tex, in_uv) * material.texture_mul + material.texture_add, 0.0, 1.0);
    if (material.alpha.x >= 0.0 && base.a < material.alpha.x) discard;

    vec3 n = normalize(in_normal);
    // Back faces of two-sided materials are lit from their own side
    if (!gl_FrontFacing && material.alpha.z > 0.5) n = -n;
    if (material.alpha.y > 0.5 && dot(in_tangent.xyz, in_tangent.xyz) > 0.0) {
        vec3 t = normalize(in_tangent.xyz - n * dot(n, in_tangent.xyz));
        vec3 b = cross(n, t) * in_tangent.w;
        vec3 tn = texture(normal_tex, in_uv).xyz * 2.0 - 1.0;
        tn.xy *= material.emissive_normal_scale.w;
        n = normalize(mat3(t, b, n) * tn);
    }

    vec3 v = normalize(frame.camera_position.xyz - in_world_pos);
    vec3 color = frame.ambient.rgb * base.rgb;
    for (uint i = 0u; i < min(frame.light_count.x, MAX_LIGHTS); ++i) {
        Light light = frame.lights[i];
        uint  type  = uint(light.direction_type.w + 0.5);
        vec3  l;
        float att = 1.0;
        if (type == LIGHT_DIRECTIONAL) {
            l = -light.direction_type.xyz;
        } else {
            vec3  to_light = light.position_range.xyz - in_world_pos;
            float dist = length(to_light);
            l   = to_light / max(dist, 1e-4);
            att = attenuation(dist, light.position_range.w);
            if (type == LIGHT_SPOT) {
                float cos_angle = dot(-l, light.direction_type.xyz);
                att *= smoothstep(light.cone.y, light.cone.x, cos_angle);
            }
        }
        float ndl = max(dot(n, l), 0.0);
        if (ndl <= 0.0 || att <= 0.0) continue;
        vec3 radiance = light.color_intensity.rgb * light.color_intensity.w * att;
        vec3 h = normalize(l + v);
        float spec = pow(max(dot(n, h), 0.0), max(material.specular_shininess.w, 1.0));
        color += radiance * (base.rgb * ndl + material.specular_shininess.rgb * spec);
    }
    color += material.emissive_normal_scale.rgb * texture(emissive_tex, in_uv).rgb;
    out_color = vec4(color, base.a);
}
