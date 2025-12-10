#pragma once

#include "core/gpu_context.h"
#include "core/system.h"
#include "glgpu/backend.h"
#include "glgpu/types.h"

#ifndef GL_HEADLESS
#include <SDL2/SDL.h>
#endif

namespace gl {

class RenderingSystem : public System {
public:
	RenderingSystem(GpuContext& p_ctx);
	virtual ~RenderingSystem() = default;

	void on_init(Registry& p_registry) override;
	void on_update(Registry& p_registry, float p_dt) override;
	void on_destroy(Registry& p_registry) override;

private:
	std::shared_ptr<RenderBackend> backend;

	CommandQueue graphics_queue = GL_NULL_HANDLE;
	CommandQueue present_queue = GL_NULL_HANDLE;

#ifndef GL_HEADLESS
	SDL_Window* window = nullptr;

	Swapchain swapchain = GL_NULL_HANDLE;

	CommandPool cmd_pool = GL_NULL_HANDLE;
	CommandBuffer cmd = GL_NULL_HANDLE;

	Semaphore image_available_sem = GL_NULL_HANDLE;
	Semaphore render_finished_sem = GL_NULL_HANDLE;
	Fence frame_fence = GL_NULL_HANDLE;
#endif
};

} //namespace gl
