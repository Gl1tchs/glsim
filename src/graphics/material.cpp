#include "graphics/material.h"
#include "core/crypto.h"

namespace gl {

Material::~Material() {
	if (!_backend) {
		return;
	}

	if (uniform_buffer) {
		_backend->buffer_free(uniform_buffer);
	}

	if (uniform_set) {
		_backend->uniform_set_free(uniform_set);
	}
}

bool Material::is_dirty() const { return hash != std::hash<Material>{}(*this); }

bool Material::is_complete() const { return pipeline != GL_NULL_HANDLE; }

void Material::upload(std::shared_ptr<RenderBackend> backend) {
	_backend = backend;

	if (!uniform_buffer) {
		uniform_buffer =
				backend->buffer_create(sizeof(Color),
							   BUFFER_USAGE_UNIFORM_BUFFER_BIT | BUFFER_USAGE_TRANSFER_SRC_BIT,
							   MemoryAllocationType::CPU)
						.value();
	}

	// Copy buffer data
	void* gpu_ptr = backend->buffer_map(uniform_buffer).value();
	std::memcpy(gpu_ptr, &base_color, sizeof(Color));
	backend->buffer_unmap(uniform_buffer);

	// Setup uniforms
	std::vector<ShaderUniform> uniforms(2);
	uniforms[0] = {
		.type = ShaderUniformType::UNIFORM_BUFFER,
		.binding = 0,
		.data = { uniform_buffer },
	};
	uniforms[1] = diffuse_texture->get_uniform(1);

	if (uniform_set) {
		backend->uniform_set_free(uniform_set);
	}

	// Create uniform set
	uniform_set = backend->uniform_set_create(uniforms, pipeline->shader, 0).value();

	// Save the hash
	hash = std::hash<Material>{}(*this);

	return;
}

} //namespace gl
