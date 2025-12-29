#version 450

#include "pipelines/unlit/unlit_common.glsl"

layout(location = 0) out vec3 v_position;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;
layout(location = 3) out vec4 v_mat_color;
layout(location = 4) out uint v_diffuse_tex_id;

void main() {
    MeshVertex vertex = vertex_buffer.vertices[gl_VertexIndex];
    InstanceData instance = instance_buffer.instances[gl_InstanceIndex + base_instance_offset];

    SceneBuffer scene_data = scene_buffer;

    vec4 frag_pos = instance.transform * vec4(vertex.position, 1.0f);

    gl_Position = scene_data.view_projection * frag_pos;

    v_position = frag_pos.xyz;
    v_normal = vertex.normal;
    v_uv = vec2(vertex.uv_x, vertex.uv_y);

    MaterialData mat = material_buffer.materials[instance.material_id];
    v_mat_color = mat.base_color;
    v_diffuse_tex_id = mat.diffuse_tex_id;
}
