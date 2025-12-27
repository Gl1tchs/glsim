#pragma once

#include "core/crypto.h"
#include "glgpu/color.h"
#include "glgpu/types.h"
#include "graphics/graphics_pipeline.h"
#include "graphics/texture.h"

namespace gl {

struct Material {
	Color base_color = COLOR_WHITE;
	std::shared_ptr<Texture> diffuse_texture = nullptr;

	std::shared_ptr<GraphicsPipeline> pipeline = nullptr;

	UniformSet uniform_set = GL_NULL_HANDLE;
	Buffer uniform_buffer = GL_NULL_HANDLE;

	size_t hash;

	~Material();

	bool is_dirty() const;

	// Is material constructable / uploadable
	bool is_complete() const;

	void upload(std::shared_ptr<RenderBackend> backend);

private:
	std::shared_ptr<RenderBackend> _backend;
};

} //namespace gl

namespace std {
template <> struct hash<gl::Material> {
	size_t operator()(const gl::Material& material) {
		size_t hash = 0;
		gl::hash_combine(hash, std::hash<gl::Color>{}(material.base_color));
		gl::hash_combine(hash, material.diffuse_texture); // hash the address
		return hash;
	}
};
} //namespace std
