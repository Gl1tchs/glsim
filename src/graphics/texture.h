#pragma once

#include "glgpu/backend.h"
#include "glgpu/color.h"
#include "glgpu/types.h"

namespace gl {

struct TextureSamplerOptions {
	ImageFiltering mag_filter = ImageFiltering::LINEAR;
	ImageFiltering min_filter = ImageFiltering::LINEAR;
	ImageWrappingMode wrap_u = ImageWrappingMode::CLAMP_TO_EDGE;
	ImageWrappingMode wrap_v = ImageWrappingMode::CLAMP_TO_EDGE;
	ImageWrappingMode wrap_w = ImageWrappingMode::CLAMP_TO_EDGE;
};

/**
 * High level abstraction over Image handle.  Provides functionality to load
 * image files as well as constructing from raw data.
 */
class Texture {
public:
	~Texture();

	static std::shared_ptr<Texture> create(std::shared_ptr<RenderBackend> backend,
			const Color& color, const Vec2u& size = { 1, 1 }, TextureSamplerOptions sampler = {});

	static std::shared_ptr<Texture> create(std::shared_ptr<RenderBackend> backend,
			DataFormat format, const Vec2u& size, const void* data = nullptr,
			TextureSamplerOptions sampler = {});

	static std::shared_ptr<Texture> load_from_file(std::shared_ptr<RenderBackend> backend,
			const fs::path& asset_path, const TextureSamplerOptions& sampler = {});

	ShaderUniform get_uniform(uint32_t binding) const;

	DataFormat get_format() const;

	const Vec2u get_size() const;

	const Image get_image() const;

	const Sampler get_sampler() const;

private:
	std::shared_ptr<RenderBackend> _backend;

	DataFormat _format;
	Image _image;
	Sampler _sampler;
	Vec2u _size;

	TextureSamplerOptions _sampler_options;
};

} //namespace gl
