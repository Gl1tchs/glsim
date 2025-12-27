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

	// AssetType method overrides

	static std::shared_ptr<Texture> create(std::shared_ptr<RenderBackend> p_backend,
			const Color& p_color, const Vec2u& p_size = { 1, 1 },
			TextureSamplerOptions p_sampler = {});

	static std::shared_ptr<Texture> create(std::shared_ptr<RenderBackend> p_backend,
			DataFormat p_format, const Vec2u& p_size, const void* p_data = nullptr,
			TextureSamplerOptions p_sampler = {});

	static std::shared_ptr<Texture> load_from_file(std::shared_ptr<RenderBackend> p_backend,
			const fs::path& p_asset_path, const TextureSamplerOptions& p_sampler = {});

	ShaderUniform get_uniform(uint32_t p_binding) const;

	DataFormat get_format() const;

	const Vec2u get_size() const;

	const Image get_image() const;

	const Sampler get_sampler() const;

private:
	std::shared_ptr<RenderBackend> backend;

	DataFormat format;
	Image image;
	Sampler sampler;
	Vec2u size;

	TextureSamplerOptions sampler_options;
};

} //namespace gl
