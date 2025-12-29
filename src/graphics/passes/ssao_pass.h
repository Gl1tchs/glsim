#pragma once

#include "graphics/render_pass.h"

namespace gl {

class SSAOPass : public IRenderPass {
public:
	virtual ~SSAOPass();

	void init(std::shared_ptr<RenderBackend> backend, RenderPassResources& res) override;
	void execute(const FrameContext& ctx, Registry& registry, RenderPassResources& res) override;

	void on_resize(const Vec2u& size, RenderPassResources& res) override;

private:
	void _init_uniforms(RenderPassResources& res);

private:
	std::shared_ptr<RenderBackend> _backend;

	Pipeline _ssao_pipeline;
	Shader _ssao_shader;

	UniformSet _ssao_set;
	Buffer _ssao_data;

	Pipeline _blur_pipeline;
	Shader _blur_shader;

	UniformSet _blur_set;
	Sampler _nearest_sampler;
};

} //namespace gl
