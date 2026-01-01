#include "graphics/passes/lighting_pass.h"

#include "glgpu/types.h"
#include "graphics/render_graph.h"

namespace gl {

LightingPass::LightingPass(std::shared_ptr<RenderBackend> backend) : _backend(backend) {
	// Create Sampler
	_sampler = _backend->sampler_create({ .min_filter = ImageFiltering::NEAREST,
												.mag_filter = ImageFiltering::NEAREST })
					   .value();

	// Create Pipeline
	const GraphicsPipelineCreateInfo create_info = {
		.color_attachments = { DataFormat::R16G16B16A16_SFLOAT },
		.enable_depth_testing = false,
		.vertex_shader = "display/fullscreen.vert.spv",
		.fragment_shader = "lighting/deferred_lighting.frag.spv",
	};
	_pipeline = GraphicsPipeline::create(_backend, create_info);
}

LightingPass::~LightingPass() {
	_backend->uniform_set_free(_lighting_set);
	_backend->sampler_free(_sampler);
}

void LightingPass::setup(RenderGraph& graph) {
	// Inputs
	_g_albedo = graph.declare_image("GBuffer_Albedo");
	_g_normal = graph.declare_image("GBuffer_Normal");
	_g_ssao = graph.declare_image("SSAO_Blur");

	// Output
	_scene_color_hdr = graph.declare_image("Scene_Color_HDR", { DataFormat::R16G16B16A16_SFLOAT });

	// Usage
	graph.set_sampled(_g_albedo);
	graph.set_sampled(_g_normal);
	graph.set_sampled(_g_ssao);
	graph.set_render_target(_scene_color_hdr);
}

void LightingPass::execute(CommandBuffer cmd, RenderGraph& graph, const RenderQueue& queue) {
	// Create if not exists
	if (!_lighting_set) {
		_update_descriptor_set(graph);
	}

	Image target = graph.get_image(_scene_color_hdr);

	RenderingAttachment attachment = {};
	attachment.image = target;
	attachment.load_op = AttachmentLoadOp::CLEAR;
	attachment.store_op = AttachmentStoreOp::STORE;
	attachment.clear_color = queue.clear_color;

	Vec2u target_size = _backend->image_get_size(target).value();

	_backend->command_begin_rendering(cmd, target_size, { attachment });
	{
		_backend->command_bind_graphics_pipeline(cmd, _pipeline->pipeline);
		_backend->command_bind_uniform_sets(cmd, _pipeline->shader, 0, { _lighting_set });
		_backend->command_draw(cmd, 6);
	}
	_backend->command_end_rendering(cmd);
}

void LightingPass::on_resize(RenderGraph& graph, const Vec2u& size) {
	_update_descriptor_set(graph);
}

void LightingPass::_update_descriptor_set(RenderGraph& graph) {
	if (_lighting_set)
		_backend->uniform_set_free(_lighting_set);

	std::vector<ShaderUniform> uniforms(3);

	// Binding 0: Albedo
	uniforms[0] = { ShaderUniformType::SAMPLER_WITH_TEXTURE, 0,
		{ _sampler, graph.get_image(_g_albedo) } };

	// Binding 1: Normal
	uniforms[1] = { ShaderUniformType::SAMPLER_WITH_TEXTURE, 1,
		{ _sampler, graph.get_image(_g_normal) } };

	// Binding 2: SSAO
	uniforms[2] = { ShaderUniformType::SAMPLER_WITH_TEXTURE, 2,
		{ _sampler, graph.get_image(_g_ssao) } };

	_lighting_set = _backend->uniform_set_create(uniforms, _pipeline->shader, 0).value();
}

} // namespace gl
