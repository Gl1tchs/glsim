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

	virtual ~StaticMesh();

	static std::shared_ptr<StaticMesh> create(std::shared_ptr<RenderBackend> p_backend,
			const std::span<const MeshVertex> p_vertices, std::span<const uint32_t> p_indices);

	void upload(std::span<const MeshVertex> p_vertices, std::span<const uint32_t> p_indices);

private:
	std::shared_ptr<RenderBackend> backend;
};

} //namespace gl
