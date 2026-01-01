#pragma once

#include "graphics/render_graph.h"
#include "graphics/render_pass.h"

namespace gl {

/**
 * Compute Pass responsible for rendering SSAO Buffer
 */
class SSAOPass : public IRenderPass {
public:
	SSAOPass(std::shared_ptr<RenderBackend> backend);
	virtual ~SSAOPass();

	void setup(RenderGraph& graph) override;

	void execute(CommandBuffer cmd, RenderGraph& graph, const RenderQueue& queue) override;

	void on_resize(RenderGraph& graph, const Vec2u& size) override;

private:
	void _create_pipelines();
	void _update_descriptor_sets(RenderGraph& graph);

private:
	std::shared_ptr<RenderBackend> _backend;

	VImageHandle _g_position;
	VImageHandle _g_normal;
	VImageHandle _ssao_raw;
	VImageHandle _ssao_blur;

	// Calculation pipeline
	Pipeline _ssao_pipeline;
	Shader _ssao_shader;
	UniformSet _ssao_set = GL_NULL_HANDLE;

	Buffer _ssao_params_buffer;

	// Blur pipeline
	Pipeline _blur_pipeline;
	Shader _blur_shader;
	UniformSet _blur_set = GL_NULL_HANDLE;

	Sampler _nearest_sampler;
};

} //namespace gl
