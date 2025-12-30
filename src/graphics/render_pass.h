#pragma once

#include "glgpu/matrix.h"
#include "glgpu/types.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"

namespace gl {

struct VHandle {
	uint32_t id = 0xFFFFFFFF;

	constexpr bool is_valid() const { return id != 0xFFFFFFFF; }

	constexpr bool operator==(const VHandle& other) const { return id == other.id; }
};

typedef VHandle VImageHandle;

class RenderGraph;

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

	// Drawing Lists
	std::vector<QueueBatch> opaque_batches;

	// Resources to upload
	std::vector<QueueMaterial> materials;
};

class IRenderPass {
public:
	virtual ~IRenderPass() = default;

	virtual void setup(RenderGraph& graph) = 0;
	virtual void execute(CommandBuffer cmd, RenderGraph& graph, const RenderQueue& queue) = 0;

	virtual void on_resize(RenderGraph& graph, const Vec2u& size) {}
};

} //namespace gl

namespace std {
template <> struct hash<gl::VHandle> {
	size_t operator()(const gl::VHandle& handle) const { return handle.id; }
};
} //namespace std
