#include "graphics/mesh.h"

namespace gl {

StaticMesh::~StaticMesh() {
	backend->buffer_free(vertex_buffer);
	backend->buffer_free(index_buffer);
}

std::shared_ptr<StaticMesh> StaticMesh::create(std::shared_ptr<RenderBackend> p_backend,
		std::span<const MeshVertex> p_vertices, std::span<const uint32_t> p_indices) {
	if (p_vertices.empty() || p_indices.empty()) {
		return nullptr;
	}

	std::shared_ptr<StaticMesh> smesh = std::make_shared<StaticMesh>();
	smesh->backend = p_backend;

	smesh->upload(p_vertices, p_indices);

	return smesh;
}

static AABB _get_aabb_from_vertices(std::span<const MeshVertex> p_vertices) {
	Vec3f min = Vec3f(std::numeric_limits<float>::max());
	Vec3f max = Vec3f(std::numeric_limits<float>::lowest());

	for (const auto& v : p_vertices) {
		min = math::min(min, v.position);
		max = math::max(max, v.position);
	}

	return { min, max };
}

void StaticMesh::upload(
		std::span<const MeshVertex> p_vertices, std::span<const uint32_t> p_indices) {
	const size_t vertex_size = p_vertices.size() * sizeof(MeshVertex);
	const size_t index_size = p_indices.size() * sizeof(uint32_t);
	const size_t data_size = vertex_size + index_size;

	Buffer staging_buffer = backend->buffer_create(
			data_size, BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryAllocationType::CPU);

	uint8_t* mapped_data = backend->buffer_map(staging_buffer);
	{
		// Copy vertex data
		memcpy(mapped_data, p_vertices.data(), vertex_size);

		// Copy index data
		memcpy(mapped_data + vertex_size, p_indices.data(), index_size);
	}
	backend->buffer_unmap(staging_buffer);

	// Create vertex buffer
	vertex_buffer = backend->buffer_create(p_vertices.size() * sizeof(MeshVertex),
			BUFFER_USAGE_STORAGE_BUFFER_BIT | BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
					BUFFER_USAGE_TRANSFER_DST_BIT,
			MemoryAllocationType::GPU);

	// Create index buffer
	index_buffer = backend->buffer_create(p_indices.size() * sizeof(uint32_t),
			BUFFER_USAGE_INDEX_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
			MemoryAllocationType::GPU);

	backend->command_immediate_submit([&](CommandBuffer p_cmd) {
		BufferCopyRegion region;

		// Copy vertex buffer
		region.src_offset = 0;
		region.size = vertex_size;
		region.dst_offset = 0;

		backend->command_copy_buffer(p_cmd, staging_buffer, vertex_buffer, { region });

		// Copy index buffer
		region.src_offset = vertex_size;
		region.size = index_size;
		region.dst_offset = 0;

		backend->command_copy_buffer(p_cmd, staging_buffer, index_buffer, { region });
	});

	backend->buffer_free(staging_buffer);

	vertex_buffer_address = backend->buffer_get_device_address(vertex_buffer);
	index_count = p_indices.size();
	aabb = _get_aabb_from_vertices(p_vertices);
}

} //namespace gl
