#version 450

#include "pipelines/unlit/unlit_common.glsl"

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec4 v_mat_color;
layout(location = 4) flat in uint v_diffuse_tex_id;

layout(location = 0) out vec4 o_position;
layout(location = 1) out vec4 o_normal;
layout(location = 2) out vec4 o_color;

void main() {
    vec4 diffuse_color = texture(h_global_textures[nonuniformEXT(v_diffuse_tex_id)], v_uv);
    o_color = v_mat_color * diffuse_color;

    o_position = vec4(v_position, 1.0);
    o_normal = vec4(normalize(v_normal), 1.0);
}
