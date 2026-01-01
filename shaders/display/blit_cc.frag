/**
 * @file blit_cc.frag
 *
 * Blits final color image to the swapchain.
 */

#version 450

layout(location = 0) in vec2 v_uv;

layout(location = 0) out vec4 o_color;

layout(set = 0, binding = 0) uniform sampler2D u_final_color_ldr;

void main() {
    o_color = texture(u_final_color_ldr, v_uv);
}
