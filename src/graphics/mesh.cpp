#include "graphics/mesh.h"

namespace gl {

StaticMesh::~StaticMesh() {
	_backend->buffer_free(vertex_buffer);
	_backend->buffer_free(index_buffer);
}

std::shared_ptr<StaticMesh> StaticMesh::create(std::shared_ptr<RenderBackend> backend,
		std::span<const MeshVertex> vertices, std::span<const uint32_t> indices) {
	if (vertices.empty() || indices.empty()) {
		return nullptr;
	}

	std::shared_ptr<StaticMesh> smesh = std::make_shared<StaticMesh>();
	smesh->_backend = backend;

	smesh->upload(vertices, indices);

	return smesh;
}

static AABB _get_aabb_from_vertices(std::span<const MeshVertex> vertices) {
	Vec3f min = Vec3f(std::numeric_limits<float>::max());
	Vec3f max = Vec3f(std::numeric_limits<float>::lowest());

	for (const auto& v : vertices) {
		min = math::min(min, v.position);
		max = math::max(max, v.position);
	}

	return { min, max };
}

void StaticMesh::upload(std::span<const MeshVertex> vertices, std::span<const uint32_t> indices) {
	const size_t vertex_size = vertices.size() * sizeof(MeshVertex);
	const size_t index_size = indices.size() * sizeof(uint32_t);
	const size_t data_size = vertex_size + index_size;

	Buffer staging_buffer = _backend->buffer_create(data_size, BUFFER_USAGE_TRANSFER_SRC_BIT,
											MemoryAllocationType::CPU)
									.value();

	uint8_t* mapped_data = _backend->buffer_map(staging_buffer).value();
	{
		// Copy vertex data
		memcpy(mapped_data, vertices.data(), vertex_size);

		// Copy index data
		memcpy(mapped_data + vertex_size, indices.data(), index_size);
	}
	_backend->buffer_unmap(staging_buffer);

	// Create vertex buffer
	vertex_buffer = _backend->buffer_create(vertices.size() * sizeof(MeshVertex),
									BUFFER_USAGE_STORAGE_BUFFER_BIT |
											BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
											BUFFER_USAGE_TRANSFER_DST_BIT,
									MemoryAllocationType::GPU)
							.value();

	// Create index buffer
	index_buffer = _backend->buffer_create(indices.size() * sizeof(uint32_t),
								   BUFFER_USAGE_INDEX_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
								   MemoryAllocationType::GPU)
						   .value();

	_backend->command_immediate_submit([&](CommandBuffer cmd) {
		BufferCopyRegion region;

		// Copy vertex buffer
		region.src_offset = 0;
		region.size = vertex_size;
		region.dst_offset = 0;

		_backend->command_copy_buffer(cmd, staging_buffer, vertex_buffer, { region });

		// Copy index buffer
		region.src_offset = vertex_size;
		region.size = index_size;
		region.dst_offset = 0;

		_backend->command_copy_buffer(cmd, staging_buffer, index_buffer, { region });
	});

	_backend->buffer_free(staging_buffer);

	vertex_buffer_address = _backend->buffer_get_device_address(vertex_buffer).value();
	index_count = indices.size();
	aabb = _get_aabb_from_vertices(vertices);
}

} //namespace gl
