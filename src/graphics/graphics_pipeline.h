#pragma once

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

	~GraphicsPipeline();

	static std::shared_ptr<GraphicsPipeline> create(
			std::shared_ptr<RenderBackend> backend, const GraphicsPipelineCreateInfo& info);

private:
	std::shared_ptr<RenderBackend> _backend;
};

} //namespace gl
