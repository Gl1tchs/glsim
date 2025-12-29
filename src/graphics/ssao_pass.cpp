#include "graphics/ssao_pass.h"
#include "core/random.h"
#include "glgpu/types.h"
#include "graphics/shader_library.h"

namespace gl {

struct SSAOData {
	Mat4 viewproj;
	float radius;
	float bias;
	float _padding[2];
	Vec3f samples[64];
};

SSAOPass::~SSAOPass() {
	// Destroy blur data
	_backend->uniform_set_free(_blur_set);
	_backend->pipeline_free(_blur_pipeline);
	_backend->shader_free(_blur_shader);

	// Destroy SSAO data
	_backend->buffer_free(_ssao_data);
	_backend->sampler_free(_nearest_sampler);
	_backend->uniform_set_free(_ssao_set);
	_backend->pipeline_free(_ssao_pipeline);
	_backend->shader_free(_ssao_shader);
}

void SSAOPass::init(std::shared_ptr<RenderBackend> backend, RenderPassResources& res) {
	_backend = backend;

	_nearest_sampler = _backend->sampler_create({ .min_filter = ImageFiltering::NEAREST,
														.mag_filter = ImageFiltering::NEAREST })
							   .value();

	// Init pipelines
	{
		const std::vector<SpirvEntry> shader_entries = {
			{
					.byte_code = shader_library::get_spirv_data("post-process/SSAO.comp.spv"),
					.stage = SHADER_STAGE_COMPUTE_BIT,
			},
		};

		_ssao_shader = _backend->shader_create_from_bytecode(shader_entries).value();
		_ssao_pipeline = _backend->compute_pipeline_create(_ssao_shader).value();

		_ssao_data =
				_backend->buffer_create(sizeof(SSAOData),
								BUFFER_USAGE_UNIFORM_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
								MemoryAllocationType::CPU)
						.value();

		// Initialize uniform buffer
		SSAOData* data = (SSAOData*)_backend->buffer_map(_ssao_data).value();
		{
			data->radius = 0.5f;
			data->bias = 0.025f;

			for (int i = 0; i < 64; ++i) {
				// Hemisphere: z in [0, 1], x/y in [-1, 1]
				Vec3f sample(random_float(-1.0f, 1.0f), random_float(-1.0f, 1.0f),
						random_float(0.0f, 1.0f));
				sample = sample.normalize();

				// Scale samples to cluster them near the origin for better local occlusion
				float scale = (float)i / 32.0f;
				scale = 0.1f + (scale * scale) * (1.0f - 0.1f); // Lerp

				data->samples[i] = sample * scale;
			}
		}
		_backend->buffer_unmap(_ssao_data);
	}

	{
		const std::vector<SpirvEntry> shader_entries = {
			{
					.byte_code = shader_library::get_spirv_data("post-process/SSAO_blur.comp.spv"),
					.stage = SHADER_STAGE_COMPUTE_BIT,
			},
		};

		_blur_shader = _backend->shader_create_from_bytecode(shader_entries).value();
		_blur_pipeline = _backend->compute_pipeline_create(_blur_shader).value();
	}

	// Init uniform buffers for SSAO pipelines
	_init_uniforms(res);
}

void SSAOPass::execute(const FrameContext& ctx, Registry& registry, RenderPassResources& res) {
	// Transition images
	_backend->command_transition_image(ctx.cmd, res.g_position,
			ImageLayout::COLOR_ATTACHMENT_OPTIMAL, ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	_backend->command_transition_image(ctx.cmd, res.g_normal, ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
			ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	_backend->command_transition_image(
			ctx.cmd, res.g_albedo, ImageLayout::COLOR_ATTACHMENT_OPTIMAL, ImageLayout::GENERAL);

	// Update SSAO data
	SSAOData* data = (SSAOData*)_backend->buffer_map(_ssao_data).value();
	{
		data->viewproj = ctx.viewproj;
	}
	_backend->buffer_unmap(_ssao_data);

	Vec2u size = _backend->image_get_size(res.g_albedo).value();

	// Dispatch SSAO
	_backend->command_bind_compute_pipeline(ctx.cmd, _ssao_pipeline);
	_backend->command_bind_uniform_sets(
			ctx.cmd, _ssao_shader, 0, { _ssao_set }, PipelineType::COMPUTE);
	_backend->command_dispatch(ctx.cmd, (size.x + 15) / 16, (size.y + 15) / 16, 1);

	// Dispatch blur pass
	_backend->command_bind_compute_pipeline(ctx.cmd, _blur_pipeline);
	_backend->command_bind_uniform_sets(
			ctx.cmd, _blur_shader, 0, { _blur_set }, PipelineType::COMPUTE);
	_backend->command_dispatch(ctx.cmd, (size.x + 15) / 16, (size.y + 15) / 16, 1);

	// Re-transition for synchronization
	_backend->command_transition_image(
			ctx.cmd, res.g_albedo, ImageLayout::GENERAL, ImageLayout::SHADER_READ_ONLY_OPTIMAL);
}

void SSAOPass::on_resize(const Vec2u& size, RenderPassResources& res) {
	if (_ssao_set) {
		_backend->uniform_set_free(_ssao_set);
	}
	if (_blur_set) {
		_backend->uniform_set_free(_blur_set);
	}

	_init_uniforms(res);
}

void SSAOPass::_init_uniforms(RenderPassResources& res) {
	std::vector<ShaderUniform> uniforms(4);

	uniforms[0].type = ShaderUniformType::UNIFORM_BUFFER;
	uniforms[0].binding = 0;
	uniforms[0].data.push_back(_ssao_data);

	uniforms[1].type = ShaderUniformType::SAMPLER_WITH_TEXTURE;
	uniforms[1].binding = 1;
	uniforms[1].data.push_back(_nearest_sampler);
	uniforms[1].data.push_back(res.g_position);

	uniforms[2].type = ShaderUniformType::SAMPLER_WITH_TEXTURE;
	uniforms[2].binding = 2;
	uniforms[2].data.push_back(_nearest_sampler);
	uniforms[2].data.push_back(res.g_normal);

	uniforms[3].type = ShaderUniformType::IMAGE;
	uniforms[3].binding = 3;
	uniforms[3].data.push_back(res.g_albedo);

	_ssao_set = _backend->uniform_set_create(uniforms, _ssao_shader, 0).value();

	// Resize the uniform vector
	uniforms.clear();
	uniforms.resize(2);

	uniforms[0].type = ShaderUniformType::SAMPLER_WITH_TEXTURE;
	uniforms[0].binding = 0;
	uniforms[0].data.push_back(_nearest_sampler);
	uniforms[0].data.push_back(res.g_position);

	uniforms[1].type = ShaderUniformType::IMAGE;
	uniforms[1].binding = 1;
	uniforms[1].data.push_back(res.g_albedo);

	_blur_set = _backend->uniform_set_create(uniforms, _blur_shader, 0).value();
}

} //namespace gl
