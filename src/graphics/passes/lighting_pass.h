#pragma once

#include "graphics/graphics_pipeline.h"
#include "graphics/render_pass.h"

namespace gl {

/**
 * Deferred Lighting Pass
 * Renders full screen quad, combining G-Buffers + SSAO -> Swapchain
 */
class LightingPass : public IRenderPass {
public:
	LightingPass(std::shared_ptr<RenderBackend> backend, DataFormat backbuffer_format);
	virtual ~LightingPass();

	void setup(RenderGraph& graph) override;
	void execute(CommandBuffer cmd, RenderGraph& graph, const RenderQueue& queue) override;

	void on_resize(RenderGraph& graph, const Vec2u& size) override;

private:
	void _update_descriptor_set(RenderGraph& graph);

private:
	std::shared_ptr<RenderBackend> _backend;

	VImageHandle _g_albedo;
	VImageHandle _g_normal;
	VImageHandle _g_ssao;
	VImageHandle _backbuffer;

	std::shared_ptr<GraphicsPipeline> _pipeline;
	Sampler _sampler;
	UniformSet _lighting_set = GL_NULL_HANDLE;
};

} //namespace gl
