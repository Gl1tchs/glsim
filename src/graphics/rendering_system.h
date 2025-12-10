#pragma once

#include "core/gpu_context.h"
#include "core/system.h"
#include "glgpu/backend.h"
#include "glgpu/types.h"
#include "graphics/window.h"

#ifndef GL_HEADLESS
#include <SDL2/SDL.h>
#endif

namespace gl {

class RenderingSystem : public System {
public:
	RenderingSystem(GpuContext& p_ctx, std::shared_ptr<Window> p_target);
	virtual ~RenderingSystem() = default;

	void on_init(Registry& p_registry) override;
	void on_update(Registry& p_registry, float p_dt) override;
	void on_destroy(Registry& p_registry) override;

private:
	std::shared_ptr<RenderBackend> backend;
	std::shared_ptr<Window> window;

	CommandQueue graphics_queue;

	CommandPool cmd_pool;
	CommandBuffer cmd;

	Semaphore image_available_sem;
	Semaphore render_finished_sem;
	Fence frame_fence;
};

} //namespace gl
