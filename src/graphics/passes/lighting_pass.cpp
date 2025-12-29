#include "graphics/passes/lighting_pass.h"

#include "glgpu/types.h"
#include "graphics/render_pass.h"

namespace gl {

LightingPass::~LightingPass() {
	_backend->uniform_set_free(_lighting_set);
	_backend->sampler_free(_sampler);
}

void LightingPass::init(std::shared_ptr<RenderBackend> backend, RenderPassResources& res) {
	_backend = backend;

	// Init pipeline
	const GraphicsPipelineCreateInfo create_info = {
		.color_attachments = { res.swapchain_format },
		.enable_depth_testing = false,
		.vertex_shader = "src/lighting/fullscreen.vert.spv",
		.fragment_shader = "src/lighting/deferred_lighting.frag.spv",
	};
	_pipeline = GraphicsPipeline::create(_backend, create_info);

	_sampler = _backend->sampler_create({ .min_filter = ImageFiltering::NEAREST,
												.mag_filter = ImageFiltering::NEAREST })
					   .value();

	_init_uniform_set(res);
}

void LightingPass::execute(const FrameContext& ctx, Registry& registry, RenderPassResources& res) {
	// Image transition
	_backend->command_transition_image(ctx.cmd, ctx.swapchain_image, ImageLayout::UNDEFINED,
			ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	_backend->command_transition_image(ctx.cmd, res.g_albedo, ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
			ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	_backend->command_transition_image(
			ctx.cmd, res.g_ssao, ImageLayout::GENERAL, ImageLayout::SHADER_READ_ONLY_OPTIMAL);

	RenderingAttachment attachment = {};
	attachment.image = ctx.swapchain_image;
	attachment.load_op = AttachmentLoadOp::CLEAR;
	attachment.store_op = AttachmentStoreOp::STORE;
	attachment.clear_color = Color(0.52, 0.8, 0.92); // Sky color

	Vec2u target_size = _backend->image_get_size(ctx.swapchain_image).value();
	_backend->command_begin_rendering(ctx.cmd, target_size, { attachment });
	{
		_backend->command_bind_graphics_pipeline(ctx.cmd, _pipeline->pipeline);
		_backend->command_bind_uniform_sets(ctx.cmd, _pipeline->shader, 0, { _lighting_set });
		_backend->command_draw(ctx.cmd, 6);
	}
	_backend->command_end_rendering(ctx.cmd);
}

void LightingPass::on_resize(const Vec2u& size, RenderPassResources& res) {
	if (_lighting_set)
		_backend->uniform_set_free(_lighting_set);

	_init_uniform_set(res);
}

void LightingPass::_init_uniform_set(RenderPassResources& res) {
	std::vector<ShaderUniform> uniforms(3);

	uniforms[0].type = ShaderUniformType::SAMPLER_WITH_TEXTURE;
	uniforms[0].binding = 0;
	uniforms[0].data.push_back(_sampler);
	uniforms[0].data.push_back(res.g_albedo);

	uniforms[1].type = ShaderUniformType::SAMPLER_WITH_TEXTURE;
	uniforms[1].binding = 1;
	uniforms[1].data.push_back(_sampler);
	uniforms[1].data.push_back(res.g_normal);

	uniforms[2].type = ShaderUniformType::SAMPLER_WITH_TEXTURE;
	uniforms[2].binding = 2;
	uniforms[2].data.push_back(_sampler);
	uniforms[2].data.push_back(res.g_ssao);

	_lighting_set = _backend->uniform_set_create(uniforms, _pipeline->shader, 0).value();
}

} //namespace gl
