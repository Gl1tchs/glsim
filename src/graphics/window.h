#pragma once

#include "core/gpu_context.h"
#include "glgpu/vector.h"

#include <SDL2/SDL_video.h>

namespace gl {

class Window {
public:
	Window(GpuContext& ctx, const Vec2u& size, const char* title);
	~Window();

	bool should_close() const;

	void poll_events() const;

	/**
	 * @returns `Image` handle if succeed, `GL_NULL_HANDLE` otherwise
	 */
	Image get_target(Semaphore wait_sem);

	void present(Semaphore signal_sem);

	void on_resize(const Vec2u& size);

	Vec2u get_size() const;

	DataFormat get_swapchain_format() const;

private:
	std::shared_ptr<RenderBackend> _backend;

	SDL_Window* _window;
	Swapchain _swapchain;

	CommandQueue _graphics_queue;
	CommandQueue _present_queue;

	bool _window_should_close = false;
};

} //namespace gl
