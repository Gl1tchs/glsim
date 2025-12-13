#version 450

#include "unlit/unlit_common.glsl"

layout(location = 0) out vec3 v_position;

void main() {
    MeshVertex v = u_push_constants.vertex_buffer.vertices[gl_VertexIndex];
    SceneBuffer scene_data = u_push_constants.scene_buffer;

    vec4 frag_pos = u_push_constants.transform * vec4(v.position, 1.0f);

    gl_Position = scene_data.view_projection * frag_pos;

    v_position = frag_pos.xyz;
}
