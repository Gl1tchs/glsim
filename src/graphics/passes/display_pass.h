#pragma once

#include "graphics/graphics_pipeline.h"
#include "graphics/render_graph.h"
#include "graphics/render_pass.h"

namespace gl {

class DisplayPass : public IRenderPass {
public:
	DisplayPass(std::shared_ptr<RenderBackend> backend, DataFormat backbuffer_format);
	virtual ~DisplayPass();

	void setup(RenderGraph& graph) override;
	void execute(CommandBuffer cmd, RenderGraph& graph, const RenderQueue& queue) override;

	void on_resize(RenderGraph& graph, const Vec2u& size) override;

private:
	void _update_descriptor_set(RenderGraph& graph);

private:
	std::shared_ptr<RenderBackend> _backend;

	VImageHandle _final_color_ldr;
	VImageHandle _backbuffer;

	std::shared_ptr<GraphicsPipeline> _pipeline;
	Sampler _sampler;
	UniformSet _display_set = GL_NULL_HANDLE;
};

} //namespace gl
