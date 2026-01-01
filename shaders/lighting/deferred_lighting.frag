/**
 * @file deferred_lighting.frag
 *
 * Draw light objects on top of albedo pass.
 */

#version 450

layout(location = 0) in vec2 v_uv;

layout(location = 0) out vec4 o_color;

layout(set = 0, binding = 0) uniform sampler2D g_albedo;
layout(set = 0, binding = 1) uniform sampler2D g_normal;
layout(set = 0, binding = 2) uniform sampler2D g_ssao;

void main() {
    vec4 albedo = texture(g_albedo, v_uv);

    // Make clear color shown
    if (albedo.a <= 0.01) {
        discard;
    }

    vec3 normal = texture(g_normal, v_uv).xyz;
    float ssao = texture(g_ssao, v_uv).r; // Our single channel R8 AO

    // Basic Ambient Lighting
    vec3 ambient_color = vec3(0.2); // Constant ambient term
    vec3 ambient = ambient_color * albedo.rgb * ssao;

    // Simple Directional Light (Simulating a Sun)
    vec3 light_dir = normalize(vec3(0.5, 1.0, 0.5));
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = diff * albedo.rgb;

    // Final Color: SSAO only affects ambient light!
    // (In real life, AO doesn't block a direct flashlight, only sky/bounced light)
    o_color = vec4(ambient + diffuse, 1.0);
}
