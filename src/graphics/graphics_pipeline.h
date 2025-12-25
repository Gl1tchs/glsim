#pragma once

#include "core/gpu_context.h"
#include "glgpu/backend.h"
#include "glgpu/types.h"

namespace gl {

struct GraphicsPipelineCreateInfo {
	std::vector<DataFormat> color_attachments;
	DataFormat depth_attachment = DataFormat::UNDEFINED;

	RenderPrimitive primitive = RenderPrimitive::TRIANGLE_LIST;

	bool enable_blend = false;
	bool enable_depth_testing = true;
	bool enable_depth_write = true;

	bool wireframe = false;

	std::string vertex_shader;
	std::string fragment_shader;
};

struct GraphicsPipeline {
	Pipeline pipeline;
	Shader shader;

	GraphicsPipeline(GpuContext& p_ctx, const GraphicsPipelineCreateInfo& p_info);
	~GraphicsPipeline();

private:
	std::shared_ptr<RenderBackend> backend;
};

} //namespace gl
