#include "graphics/texture.h"

#include "core/log.h"
#include "glgpu/types.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace gl {

Texture::~Texture() {
	backend->image_free(image);
	backend->sampler_free(sampler);
}

std::shared_ptr<Texture> Texture::create(std::shared_ptr<RenderBackend> p_backend,
		const Color& p_color, const Vec2u& p_size, TextureSamplerOptions p_sampler) {
	uint32_t color_data = p_color.as_uint();
	return Texture::create(p_backend, DataFormat::R8G8B8A8_UNORM, p_size, &color_data, p_sampler);
}

std::shared_ptr<Texture> Texture::create(std::shared_ptr<RenderBackend> p_backend,
		DataFormat p_format, const Vec2u& p_size, const void* p_data,
		TextureSamplerOptions p_sampler) {
	ImageCreateInfo image_info = {};
	image_info.format = p_format;
	image_info.size = p_size;
	image_info.data = p_data;
	image_info.mipmapped = true;

	Image image = p_backend->image_create(image_info);

	SamplerCreateInfo sampler_info = {};
	sampler_info.min_filter = p_sampler.min_filter;
	sampler_info.mag_filter = p_sampler.mag_filter;
	sampler_info.wrap_u = p_sampler.wrap_u;
	sampler_info.wrap_v = p_sampler.wrap_v;
	sampler_info.wrap_w = p_sampler.wrap_w;
	sampler_info.mip_levels = p_backend->image_get_mip_levels(image);

	Sampler sampler = p_backend->sampler_create(sampler_info);

	std::shared_ptr<Texture> tx = std::make_shared<Texture>();
	tx->backend = p_backend;
	tx->format = p_format;
	tx->size = p_size;
	tx->image = image;
	tx->sampler = sampler;
	tx->sampler_options = p_sampler;

	return tx;
}

std::shared_ptr<Texture> Texture::load_from_file(std::shared_ptr<RenderBackend> p_backend,
		const fs::path& p_asset_path, const TextureSamplerOptions& p_sampler) {
	if (!fs::exists(p_asset_path)) {
		GL_LOG_ERROR(
				"[Texture::load_from_file] Unable to load texture from file, file do not exist.");
		return nullptr;
	}

	int w, h;
	stbi_uc* data = stbi_load(p_asset_path.string().c_str(), &w, &h, nullptr, STBI_rgb_alpha);

	ImageCreateInfo image_info = {};
	image_info.format = DataFormat::R8G8B8A8_UNORM;
	image_info.size = Vec2u(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
	image_info.data = data;
	image_info.mipmapped = true;

	Image image = p_backend->image_create(image_info);

	SamplerCreateInfo sampler_info = {};
	sampler_info.min_filter = p_sampler.min_filter;
	sampler_info.mag_filter = p_sampler.mag_filter;
	sampler_info.wrap_u = p_sampler.wrap_u;
	sampler_info.wrap_v = p_sampler.wrap_v;
	sampler_info.wrap_w = p_sampler.wrap_w;
	sampler_info.mip_levels = p_backend->image_get_mip_levels(image);

	Sampler sampler = p_backend->sampler_create(sampler_info);

	std::shared_ptr<Texture> tx = std::make_shared<Texture>();
	tx->backend = p_backend;
	tx->format = DataFormat::R8G8B8A8_UNORM;
	tx->size = { static_cast<uint32_t>(w), static_cast<uint32_t>(h) };
	tx->image = image;
	tx->sampler = sampler;
	tx->sampler_options = p_sampler;

	stbi_image_free(data);

	return tx;
}

ShaderUniform Texture::get_uniform(uint32_t p_binding) const {
	ShaderUniform uniform;
	uniform.type = ShaderUniformType::SAMPLER_WITH_TEXTURE;
	uniform.binding = p_binding;
	uniform.data.push_back(sampler);
	uniform.data.push_back(image);

	return uniform;
}

DataFormat Texture::get_format() const { return format; }

const Vec2u Texture::get_size() const { return size; }

const Image Texture::get_image() const { return image; }

const Sampler Texture::get_sampler() const { return sampler; }

} //namespace gl
