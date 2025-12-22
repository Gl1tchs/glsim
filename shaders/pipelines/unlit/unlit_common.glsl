#ifndef UNLIT_COMMON_GLSL
#define UNLIT_COMMON_GLSL

#extension GL_EXT_buffer_reference : require

struct MeshVertex {
  vec3 position;
  float uv_x;
  vec3 normal;
  float uv_y;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer {
  MeshVertex vertices[];
};

layout(buffer_reference, std430) readonly buffer SceneBuffer {
  mat4 view_projection;
};

layout(push_constant, std430) uniform constants {
  mat4 transform;
  VertexBuffer vertex_buffer;
  SceneBuffer scene_buffer;
}
u_push_constants;

layout(set = 0, binding = 0) uniform MaterialData {
  vec4 base_color;
}
u_material_data;

#endif // UNLIT_COMMON_GLSL
