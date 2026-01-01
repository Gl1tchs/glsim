#pragma once

#include "graphics/graphics_pipeline.h"
#include "graphics/render_graph.h"
#include "graphics/render_pass.h"

namespace gl {

/**
 * Forward+ Pass responsible for rendering G-Buffers
 */
class GeometryPass : public IRenderPass {
public:
	GeometryPass(std::shared_ptr<RenderBackend> backend);
	virtual ~GeometryPass();

	void setup(RenderGraph& graph) override;
	void execute(CommandBuffer cmd, RenderGraph& graph, const RenderQueue& queue) override;

private:
	std::shared_ptr<RenderBackend> _backend;
	std::shared_ptr<GraphicsPipeline> _pipeline;

	// Graph Handles
	VImageHandle _g_position;
	VImageHandle _g_normal;
	VImageHandle _g_albedo;
	VImageHandle _g_depth;

	// GPU Resources
	Buffer _scene_buffer;
	BufferDeviceAddress _scene_buffer_addr;

	Buffer _instance_buffer;
	BufferDeviceAddress _instance_buffer_addr;

	Buffer _material_buffer;
	BufferDeviceAddress _material_buffer_addr;

	UniformSet _bindless_textures;
};

} //namespace gl
