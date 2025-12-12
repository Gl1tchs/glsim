#pragma once

#include "core/gpu_context.h"
#include "glgpu/vec.h"

#include <SDL2/SDL_video.h>

namespace gl {

class Window {
public:
	Window(GpuContext& p_ctx, const Vec2u& p_size, const char* p_title);
	~Window();

	bool should_close() const;

	void poll_events() const;

	/**
	 * @returns `Image` handle if succeed, `GL_NULL_HANDLE` otherwise
	 */
	Image get_target(Semaphore p_wait_sem);

	void present(Semaphore p_signal_sem);

	void on_resize(const Vec2u& p_size);

	Vec2u get_size() const;

	DataFormat get_swapchain_format() const;

private:
	std::shared_ptr<RenderBackend> backend;

	SDL_Window* window;
	Swapchain swapchain;

	CommandQueue graphics_queue;
	CommandQueue present_queue;

	bool window_should_close = false;
};

} //namespace gl
