#include "graphics/passes/display_pass.h"

#include "glgpu/types.h"
#include "graphics/render_graph.h"

namespace gl {

DisplayPass::DisplayPass(std::shared_ptr<RenderBackend> backend, DataFormat backbuffer_format) :
		_backend(backend) {
	// Create Sampler
	_sampler = _backend->sampler_create({ .min_filter = ImageFiltering::NEAREST,
												.mag_filter = ImageFiltering::NEAREST })
					   .value();

	// Create Pipeline
	const GraphicsPipelineCreateInfo create_info = {
		.color_attachments = { backbuffer_format },
		.enable_depth_testing = false,
		.vertex_shader = "display/fullscreen.vert.spv",
		.fragment_shader = "display/blit_cc.frag.spv",
	};
	_pipeline = GraphicsPipeline::create(_backend, create_info);
}

DisplayPass::~DisplayPass() {
	_backend->uniform_set_free(_display_set);
	_backend->sampler_free(_sampler);
}

void DisplayPass::setup(RenderGraph& graph) {
	// Inputs
	_final_color_ldr = graph.declare_image("Final_Color_LDR");

	// Output
	// RenderingSystem imports the physical image every frame before compile.
	_backbuffer = graph.declare_image("Backbuffer");

	// Usage
	graph.set_sampled(_final_color_ldr);
	graph.set_render_target(_backbuffer);
}

void DisplayPass::execute(CommandBuffer cmd, RenderGraph& graph, const RenderQueue& queue) {
	// Create if not exists
	if (!_display_set) {
		_update_descriptor_set(graph);
	}

	Image target = graph.get_image(_backbuffer);

	RenderingAttachment attachment = {};
	attachment.image = target;
	attachment.load_op = AttachmentLoadOp::CLEAR;
	attachment.store_op = AttachmentStoreOp::STORE;
	attachment.clear_color = queue.clear_color;

	Vec2u target_size = _backend->image_get_size(target).value();

	_backend->command_begin_rendering(cmd, target_size, { attachment });
	{
		_backend->command_bind_graphics_pipeline(cmd, _pipeline->pipeline);
		_backend->command_bind_uniform_sets(cmd, _pipeline->shader, 0, { _display_set });
		_backend->command_draw(cmd, 6);
	}
	_backend->command_end_rendering(cmd);
}

void DisplayPass::on_resize(RenderGraph& graph, const Vec2u& size) {
	_update_descriptor_set(graph);
}

void DisplayPass::_update_descriptor_set(RenderGraph& graph) {
	if (_display_set)
		_backend->uniform_set_free(_display_set);

	std::vector<ShaderUniform> uniforms(1);
	uniforms[0] = { ShaderUniformType::SAMPLER_WITH_TEXTURE, 0,
		{ _sampler, graph.get_image(_final_color_ldr) } };

	_display_set = _backend->uniform_set_create(uniforms, _pipeline->shader, 0).value();
}

} // namespace gl
