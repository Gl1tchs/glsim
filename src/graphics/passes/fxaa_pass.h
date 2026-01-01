#pragma once

#include "graphics/render_graph.h"
#include "graphics/render_pass.h"

namespace gl {

class FXAAPass : public IRenderPass {
public:
	FXAAPass(std::shared_ptr<RenderBackend> backend);
	virtual ~FXAAPass();

	void setup(RenderGraph& graph) override;
	void execute(CommandBuffer cmd, RenderGraph& graph, const RenderQueue& queue) override;

	void on_resize(RenderGraph& graph, const Vec2u& size) override;

private:
	void _update_descriptor_set(RenderGraph& graph);

private:
	std::shared_ptr<RenderBackend> _backend;

	VImageHandle _scene_color_hdr;
	VImageHandle _final_color_ldr;

	Pipeline _pipeline;
	Shader _shader;
	Sampler _sampler;
	UniformSet _fxaa_set = GL_NULL_HANDLE;
};

} //namespace gl
