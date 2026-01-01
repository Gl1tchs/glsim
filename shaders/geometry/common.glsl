#ifndef COMMON_GLSL
#define COMMON_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier : require

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

struct InstanceData {
    mat4 transform;
    uint material_id;
};

layout(buffer_reference, std430) readonly buffer InstanceBuffer {
    InstanceData instances[];
};

struct MaterialData {
    vec4 base_color;
    uint diffuse_tex_id;
    uint _padding[3];
};

layout(buffer_reference, std430) readonly buffer MaterialBuffer {
    MaterialData materials[];
};

layout(push_constant, std430) uniform constants {
    VertexBuffer vertex_buffer;
    SceneBuffer scene_buffer;
    InstanceBuffer instance_buffer;
    MaterialBuffer material_buffer;
    uint base_instance_offset;
    uint _padding;
};

// Shader variables starting with h_ will automatically
// defined as "headless" in backend. Thus will have by default
// binding count of 1000
layout(set = 0, binding = 0) uniform sampler2D h_global_textures[];

#endif // COMMON_GLSL
