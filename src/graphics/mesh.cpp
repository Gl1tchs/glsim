#include "graphics/mesh.h"

namespace gl {

StaticMesh::~StaticMesh() {
	backend->device_wait();
	backend->buffer_free(vertex_buffer);
	backend->buffer_free(index_buffer);
}

std::shared_ptr<StaticMesh> StaticMesh::create(std::shared_ptr<RenderBackend> p_backend,
		const std::span<MeshVertex>& p_vertices, const std::span<uint32_t>& p_indices) {
	if (p_vertices.empty() || p_indices.empty()) {
		return nullptr;
	}

	std::shared_ptr<StaticMesh> smesh = std::make_shared<StaticMesh>();

	const size_t vertex_size = p_vertices.size() * sizeof(MeshVertex);
	const size_t index_size = p_indices.size() * sizeof(uint32_t);

	const size_t data_size = vertex_size + index_size;

	Buffer staging_buffer = p_backend->buffer_create(
			data_size, BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryAllocationType::CPU);

	uint8_t* mapped_data = p_backend->buffer_map(staging_buffer);
	{
		// Copy vertex data
		memcpy(mapped_data, p_vertices.data(), vertex_size);

		// Copy index data
		memcpy(mapped_data + vertex_size, p_indices.data(), index_size);
	}
	p_backend->buffer_unmap(staging_buffer);

	// Create vertex buffer
	smesh->vertex_buffer = p_backend->buffer_create(p_vertices.size() * sizeof(MeshVertex),
			BUFFER_USAGE_STORAGE_BUFFER_BIT | BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
					BUFFER_USAGE_TRANSFER_DST_BIT,
			MemoryAllocationType::GPU);

	// Create index buffer
	smesh->index_buffer = p_backend->buffer_create(p_indices.size() * sizeof(uint32_t),
			BUFFER_USAGE_INDEX_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
			MemoryAllocationType::GPU);

	p_backend->command_immediate_submit([&](CommandBuffer p_cmd) {
		BufferCopyRegion region;

		// Copy vertex buffer
		region.src_offset = 0;
		region.size = vertex_size;
		region.dst_offset = 0;

		p_backend->command_copy_buffer(p_cmd, staging_buffer, smesh->vertex_buffer, { region });

		// Copy index buffer
		region.src_offset = vertex_size;
		region.size = index_size;
		region.dst_offset = 0;

		p_backend->command_copy_buffer(p_cmd, staging_buffer, smesh->index_buffer, { region });
	});

	p_backend->buffer_free(staging_buffer);

	smesh->backend = p_backend;
	smesh->vertex_buffer_address = p_backend->buffer_get_device_address(smesh->vertex_buffer);
	smesh->index_count = p_indices.size();

	return smesh;
}

} //namespace gl
