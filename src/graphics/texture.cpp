#include "graphics/texture.h"

#include "core/log.h"
#include "glgpu/types.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace gl {

Texture::~Texture() {
	_backend->image_free(_image);
	_backend->sampler_free(_sampler);
}

std::shared_ptr<Texture> Texture::create(std::shared_ptr<RenderBackend> backend, const Color& color,
		const Vec2u& size, TextureSamplerOptions sampler) {
	uint32_t color_data = color.as_uint();
	return Texture::create(backend, DataFormat::R8G8B8A8_UNORM, size, &color_data, sampler);
}

std::shared_ptr<Texture> Texture::create(std::shared_ptr<RenderBackend> backend, DataFormat format,
		const Vec2u& size, const void* data, TextureSamplerOptions sampler_opt) {
	ImageCreateInfo image_info = {};
	image_info.format = format;
	image_info.size = size;
	image_info.data = data;
	image_info.mipmapped = true;

	Image image = backend->image_create(image_info).value();

	SamplerCreateInfo sampler_info = {};
	sampler_info.min_filter = sampler_opt.min_filter;
	sampler_info.mag_filter = sampler_opt.mag_filter;
	sampler_info.wrap_u = sampler_opt.wrap_u;
	sampler_info.wrap_v = sampler_opt.wrap_v;
	sampler_info.wrap_w = sampler_opt.wrap_w;
	sampler_info.mip_levels = backend->image_get_mip_levels(image).value();

	Sampler sampler = backend->sampler_create(sampler_info).value();

	std::shared_ptr<Texture> tx = std::make_shared<Texture>();
	tx->_backend = backend;
	tx->_format = format;
	tx->_size = size;
	tx->_image = image;
	tx->_sampler = sampler;
	tx->_sampler_options = sampler_opt;

	return tx;
}

std::shared_ptr<Texture> Texture::load_from_file(std::shared_ptr<RenderBackend> backend,
		const fs::path& path, const TextureSamplerOptions& sampler_opt) {
	if (!fs::exists(path)) {
		GL_LOG_ERROR(
				"[Texture::load_from_file] Unable to load texture from file, file do not exist.");
		return nullptr;
	}

	int w, h;
	stbi_uc* data = stbi_load(path.string().c_str(), &w, &h, nullptr, STBI_rgb_alpha);

	ImageCreateInfo image_info = {};
	image_info.format = DataFormat::R8G8B8A8_UNORM;
	image_info.size = Vec2u(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
	image_info.data = data;
	image_info.mipmapped = true;

	Image image = backend->image_create(image_info).value();

	SamplerCreateInfo sampler_info = {};
	sampler_info.min_filter = sampler_opt.min_filter;
	sampler_info.mag_filter = sampler_opt.mag_filter;
	sampler_info.wrap_u = sampler_opt.wrap_u;
	sampler_info.wrap_v = sampler_opt.wrap_v;
	sampler_info.wrap_w = sampler_opt.wrap_w;
	sampler_info.mip_levels = backend->image_get_mip_levels(image).value();

	Sampler sampler = backend->sampler_create(sampler_info).value();

	std::shared_ptr<Texture> tx = std::make_shared<Texture>();
	tx->_backend = backend;
	tx->_format = DataFormat::R8G8B8A8_UNORM;
	tx->_size = { static_cast<uint32_t>(w), static_cast<uint32_t>(h) };
	tx->_image = image;
	tx->_sampler = sampler;
	tx->_sampler_options = sampler_opt;

	stbi_image_free(data);

	return tx;
}

ShaderUniform Texture::get_uniform(uint32_t binding) const {
	ShaderUniform uniform;
	uniform.type = ShaderUniformType::SAMPLER_WITH_TEXTURE;
	uniform.binding = binding;
	uniform.data.push_back(_sampler);
	uniform.data.push_back(_image);

	return uniform;
}

DataFormat Texture::get_format() const { return _format; }

const Vec2u Texture::get_size() const { return _size; }

const Image Texture::get_image() const { return _image; }

const Sampler Texture::get_sampler() const { return _sampler; }

} //namespace gl
