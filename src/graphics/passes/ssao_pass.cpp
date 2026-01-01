#include "graphics/passes/ssao_pass.h"

#include "core/random.h"
#include "glgpu/types.h"
#include "graphics/render_graph.h"
#include "graphics/shader_library.h"

namespace gl {

struct SSAOData {
	Mat4 viewproj;
	float radius;
	float bias;
	float _padding[2];
	Vec3f samples[64];
};

SSAOPass::SSAOPass(std::shared_ptr<RenderBackend> backend) : _backend(backend) {
	_nearest_sampler = _backend->sampler_create({
														.min_filter = ImageFiltering::NEAREST,
														.mag_filter = ImageFiltering::NEAREST,
												})
							   .value();

	_create_pipelines();

	// Init SSAO Kernel Buffer
	_ssao_params_buffer =
			_backend->buffer_create(sizeof(SSAOData),
							BUFFER_USAGE_UNIFORM_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
							MemoryAllocationType::CPU)
					.value();

	// Populate static kernel data
	SSAOData* data = (SSAOData*)_backend->buffer_map(_ssao_params_buffer).value();
	if (data) {
		data->radius = 0.6f;
		data->bias = 0.001f;
		for (int i = 0; i < 64; ++i) {
			Vec3f sample(
					random_float(-1.0f, 1.0f), random_float(-1.0f, 1.0f), random_float(0.0f, 1.0f));
			sample = sample.normalize();
			float scale = (float)i / 64.0f;
			scale = 0.1f + (scale * scale) * (0.9f);
			data->samples[i] = sample * scale;
		}
		_backend->buffer_unmap(_ssao_params_buffer);
	}
}

SSAOPass::~SSAOPass() {
	_backend->uniform_set_free(_blur_set);
	_backend->pipeline_free(_blur_pipeline);
	_backend->shader_free(_blur_shader);

	_backend->buffer_free(_ssao_params_buffer);
	_backend->sampler_free(_nearest_sampler);
	_backend->uniform_set_free(_ssao_set);
	_backend->pipeline_free(_ssao_pipeline);
	_backend->shader_free(_ssao_shader);
}

void SSAOPass::setup(RenderGraph& graph) {
	// Acquire Inputs
	_g_position = graph.declare_image("GBuffer_Position");
	_g_normal = graph.declare_image("GBuffer_Normal");

	//  Create Outputs
	_ssao_raw = graph.declare_image("SSAO_Raw", { DataFormat::R8_UNORM });
	_ssao_blur = graph.declare_image("SSAO_Blur", { DataFormat::R8_UNORM });

	// Define Usage
	graph.set_sampled(_g_position);
	graph.set_sampled(_g_normal);

	graph.set_storage_write(_ssao_raw);
	graph.set_storage_write(_ssao_blur);
}

void SSAOPass::execute(CommandBuffer cmd, RenderGraph& graph, const RenderQueue& queue) {
	// Update SSAO data
	{
		SSAOData* data = (SSAOData*)_backend->buffer_map(_ssao_params_buffer).value();
		if (data) {
			// Use the camera data from the RenderQueue
			data->viewproj = queue.viewproj;
			_backend->buffer_unmap(_ssao_params_buffer);
		}
	}

	// Ensure descriptors are valid (create if first run)
	if (!_ssao_set) {
		_update_descriptor_sets(graph);
	}

	Image raw_img = graph.get_image(_ssao_raw);
	Vec2u size = _backend->image_get_size(raw_img).value();
	Vec3u groups = { (size.x + 15) / 16, (size.y + 15) / 16, 1 };

	// Dispatch SSAO Calculation
	_backend->command_bind_compute_pipeline(cmd, _ssao_pipeline);
	_backend->command_bind_uniform_sets(cmd, _ssao_shader, 0, { _ssao_set }, PipelineType::COMPUTE);
	_backend->command_dispatch(cmd, groups.x, groups.y, groups.z);

	// Barrier between SSAO_blur and SSAO_calc
	graph.transition_image(cmd, _ssao_raw, ImageLayout::GENERAL);

	// Dispatch Blur
	_backend->command_bind_compute_pipeline(cmd, _blur_pipeline);
	_backend->command_bind_uniform_sets(cmd, _blur_shader, 0, { _blur_set }, PipelineType::COMPUTE);
	_backend->command_dispatch(cmd, groups.x, groups.y, groups.z);
}

void SSAOPass::on_resize(RenderGraph& graph, const Vec2u& size) {
	// Physical images changed, we must recreate descriptors pointing to them
	_update_descriptor_sets(graph);
}

void SSAOPass::_update_descriptor_sets(RenderGraph& graph) {
	if (_ssao_set)
		_backend->uniform_set_free(_ssao_set);
	if (_blur_set)
		_backend->uniform_set_free(_blur_set);

	Image phys_pos = graph.get_image(_g_position);
	Image phys_norm = graph.get_image(_g_normal);
	Image phys_raw = graph.get_image(_ssao_raw);
	Image phys_blur = graph.get_image(_ssao_blur);

	// Set 1: Calc
	{
		std::vector<ShaderUniform> uniforms(4);
		uniforms[0] = { ShaderUniformType::UNIFORM_BUFFER, 0, { _ssao_params_buffer } };
		uniforms[1] = { ShaderUniformType::SAMPLER_WITH_TEXTURE, 1,
			{ _nearest_sampler, phys_pos } };
		uniforms[2] = { ShaderUniformType::SAMPLER_WITH_TEXTURE, 2,
			{ _nearest_sampler, phys_norm } };
		uniforms[3] = { ShaderUniformType::IMAGE, 3, { phys_raw } };

		_ssao_set = _backend->uniform_set_create(uniforms, _ssao_shader, 0).value();
	}

	// Set 2: Blur
	{
		std::vector<ShaderUniform> uniforms(2);
		uniforms[0] = { ShaderUniformType::SAMPLER_WITH_TEXTURE, 0,
			{ _nearest_sampler, phys_pos } };
		uniforms[1] = { ShaderUniformType::IMAGE, 1, { phys_blur } };

		_blur_set = _backend->uniform_set_create(uniforms, _blur_shader, 0).value();
	}
}

void SSAOPass::_create_pipelines() {
	auto ssao_code = shader_library::get_spirv_data("ssao/ssao_calc.comp.spv");
	_ssao_shader =
			_backend->shader_create_from_bytecode({ { ssao_code, SHADER_STAGE_COMPUTE_BIT } })
					.value();
	_ssao_pipeline = _backend->compute_pipeline_create(_ssao_shader).value();

	auto blur_code = shader_library::get_spirv_data("ssao/ssao_blur.comp.spv");
	_blur_shader =
			_backend->shader_create_from_bytecode({ { blur_code, SHADER_STAGE_COMPUTE_BIT } })
					.value();
	_blur_pipeline = _backend->compute_pipeline_create(_blur_shader).value();
}

} // namespace gl
