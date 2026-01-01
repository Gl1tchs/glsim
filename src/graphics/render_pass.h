#pragma once

#include "glgpu/matrix.h"
#include "glgpu/types.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"

namespace gl {

struct QueueInstance {
	Mat4 transform;
	uint32_t material_index;
};

struct QueueBatch {
	std::shared_ptr<StaticMesh> mesh;
	std::vector<QueueInstance> instances;
};

struct QueueMaterial {
	Color base_color = COLOR_WHITE;
	std::shared_ptr<Texture> albedo_map = nullptr;
};

struct RenderQueue {
	// Scene Globals
	Mat4 viewproj;
	Vec3f camera_pos;

	// Settings
	Color clear_color;

	// Drawing Lists
	std::vector<QueueBatch> opaque_batches;

	// Resources to upload
	std::vector<QueueMaterial> materials;
};

class RenderGraph;

class IRenderPass {
public:
	virtual ~IRenderPass() = default;

	virtual void setup(RenderGraph& graph) = 0;
	virtual void execute(CommandBuffer cmd, RenderGraph& graph, const RenderQueue& queue) = 0;

	virtual void on_resize(RenderGraph& graph, const Vec2u& size) {}
};

} //namespace gl
