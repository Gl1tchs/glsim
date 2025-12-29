#pragma once

#include "core/registry.h"
#include "glgpu/backend.h"
#include "glgpu/types.h"
#include "graphics/aabb.h"

namespace gl {

struct FrameContext {
	CommandBuffer cmd;
	float dt;
	Frustum frustum;
	Mat4 viewproj;
};

struct RenderPassResources {
	// G-Buffers
	Image g_position; // R16G16B16A16_SFLOAT
	Image g_normal; // R16G16B16A16_SFLOAT
	Image g_albedo; // R8G8B8A8_UNORM
	Image g_depth; // D32_SFLOAT
	// Scene data
	BufferDeviceAddress scene_buffer_addr;
};

class IRenderPass {
public:
	virtual ~IRenderPass() = default;

	virtual void init(std::shared_ptr<RenderBackend> backend, RenderPassResources& res) = 0;
	virtual void execute(const FrameContext& ctx, Registry& registry, RenderPassResources& res) = 0;

	virtual void on_resize(const Vec2u& size, RenderPassResources& res) {};
};

} //namespace gl
