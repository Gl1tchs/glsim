/**
 * @file fullscreen.frag
 *
 * Draws fullscreen quad without using any vertex buffer
 */

#version 450

layout(location = 0) out vec2 v_uv;

void main() {
    v_uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(v_uv * 2.0f - 1.0f, 0.0f, 1.0f);
    v_uv.y = 1.0 - v_uv.y; // invert y
}
