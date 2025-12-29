#pragma once

#include "graphics/graphics_pipeline.h"
#include "graphics/render_pass.h"

namespace gl {

class LightingPass : public IRenderPass {
public:
	virtual ~LightingPass();

	void init(std::shared_ptr<RenderBackend> backend, RenderPassResources& res) override;
	void execute(const FrameContext& ctx, Registry& registry, RenderPassResources& res) override;

	void on_resize(const Vec2u& size, RenderPassResources& res) override;

private:
	void _init_uniform_set(RenderPassResources& res);

private:
	std::shared_ptr<RenderBackend> _backend;

	std::shared_ptr<GraphicsPipeline> _pipeline;

	Sampler _sampler;
	UniformSet _lighting_set = GL_NULL_HANDLE;
};

} //namespace gl
