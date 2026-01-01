#include "graphics/passes/fxaa_pass.h"

#include "glgpu/types.h"
#include "graphics/render_graph.h"
#include "graphics/shader_library.h"

namespace gl {

struct PushConstants {
	Vec2f inverse_screen_size;
	float fixed_threshold;
	float relative_threshold;
	float subpixel_quality;
};

FXAAPass::FXAAPass(std::shared_ptr<RenderBackend> backend) : _backend(backend) {
	// Create Sampler
	_sampler = _backend->sampler_create({ .min_filter = ImageFiltering::NEAREST,
												.mag_filter = ImageFiltering::NEAREST })
					   .value();

	// Create Pipeline
	_shader = _backend->shader_create_from_bytecode(
							  {
									  {
											  shader_library::get_spirv_data("fxaa/fxaa.comp.spv"),
											  SHADER_STAGE_COMPUTE_BIT,
									  },
							  })
					  .value();
	_pipeline = _backend->compute_pipeline_create(_shader).value();
}

FXAAPass::~FXAAPass() {
	_backend->uniform_set_free(_fxaa_set);
	_backend->sampler_free(_sampler);

	_backend->pipeline_free(_pipeline);
	_backend->shader_free(_shader);
}

void FXAAPass::setup(RenderGraph& graph) {
	_scene_color_hdr = graph.declare_image("Scene_Color_HDR");
	_final_color_ldr = graph.declare_image("Final_Color_LDR", { DataFormat::R16G16B16A16_SFLOAT });

	graph.set_sampled(_scene_color_hdr);
	graph.set_storage_write(_final_color_ldr);
}

void FXAAPass::execute(CommandBuffer cmd, RenderGraph& graph, const RenderQueue& queue) {
	// Create if not exists
	if (!_fxaa_set) {
		_update_descriptor_set(graph);
	}

	Vec2u size = _backend->image_get_size(graph.get_image(_final_color_ldr)).value();
	Vec3u groups = { (size.x + 15) / 16, (size.y + 15) / 16, 1 };

	PushConstants pc = {};
	pc.inverse_screen_size = Vec2f(1.0f / size.x, 1.0f / size.y);
	pc.fixed_threshold = 0.0833f;
	pc.relative_threshold = 0.166f;
	pc.subpixel_quality = 0.75f;

	_backend->command_bind_compute_pipeline(cmd, _pipeline);
	_backend->command_bind_uniform_sets(cmd, _shader, 0, { _fxaa_set }, PipelineType::COMPUTE);
	_backend->command_push_constants(cmd, _shader, 0, sizeof(PushConstants), &pc);
	_backend->command_dispatch(cmd, groups.x, groups.y, groups.z);
}

void FXAAPass::on_resize(RenderGraph& graph, const Vec2u& size) { _update_descriptor_set(graph); }

void FXAAPass::_update_descriptor_set(RenderGraph& graph) {
	if (_fxaa_set)
		_backend->uniform_set_free(_fxaa_set);

	std::vector<ShaderUniform> uniforms(2);
	uniforms[0] = { ShaderUniformType::SAMPLER_WITH_TEXTURE, 0,
		{ _sampler, graph.get_image(_scene_color_hdr) } };
	uniforms[1] = { ShaderUniformType::IMAGE, 1, { graph.get_image(_final_color_ldr) } };

	_fxaa_set = _backend->uniform_set_create(uniforms, _shader, 0).value();
}

} // namespace gl
