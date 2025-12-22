#pragma once

#include "glgpu/backend.h"
#include "glgpu/types.h"
#include "glgpu/vec.h"

namespace gl {

struct MeshVertex {
	Vec3f position;
	float uv_x;
	Vec3f normal;
	float uv_y;
};

struct StaticMesh {
	Buffer vertex_buffer;
	Buffer index_buffer;
	BufferDeviceAddress vertex_buffer_address;
	uint32_t index_count;

	// AABB aabb;

	~StaticMesh();

	static std::shared_ptr<StaticMesh> create(std::shared_ptr<RenderBackend> p_backend,
			const std::span<MeshVertex>& p_vertices, const std::span<uint32_t>& p_indices);

private:
	std::shared_ptr<RenderBackend> backend;
};

} //namespace gl
