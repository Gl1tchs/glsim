#include "graphics/window.h"

#include "core/assert.h"
#include "core/event_system.h"
#include "core/input.h"
#include "core/log.h"
#include "glgpu/types.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_video.h>

namespace gl {

Window::Window(GpuContext& p_ctx, const Vec2u& p_size, const char* p_title) :
		backend(p_ctx.get_backend()) {
	if (!backend->is_swapchain_supported()) {
		GL_ASSERT(false, "[Window::Window()] Swapchain is not supported.");
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		GL_LOG_ERROR("SDL could not initialize! SDL_Error: {}", SDL_GetError());
		return;
	}

	SDL_Window* window = SDL_CreateWindow(p_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			p_size.x, p_size.y, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (window == nullptr) {
		GL_LOG_ERROR("Window could not be created! SDL_Error: {}", SDL_GetError());
		SDL_Quit();
		return;
	}

	void* connection_handle = nullptr;
	void* window_handle = nullptr;

	SDL_SysWMinfo wm_info;
	SDL_VERSION(&wm_info.version);
	if (SDL_GetWindowWMInfo(window, &wm_info)) {
#if defined(__linux)
		if (wm_info.subsystem == SDL_SYSWM_X11) {
			connection_handle = wm_info.info.x11.display;
			window_handle = (void*)wm_info.info.x11.window;
		} else {
			GL_ASSERT(false, "Only X11 and windows is supported.");
		}
#elif defined(_WIN32)
		if (wm_info.subsystem == SDL_SYSWM_WINDOWS) {
			window_handle = (void*)wm_info.info.win.window;
			connection_handle = (void*)wm_info.info.win.hinstance; // Usually
		} else {
			GL_ASSERT(false, "Unknown window manager.");
		}
#else
#error "Unsupported OS"
#endif
	}

	// NOTE: this will recreate the surface and very error prone.
	GL_ASSERT(p_ctx.get_backend()->attach_surface(connection_handle, window_handle) ==
			SurfaceCreateError::NONE);

	graphics_queue = backend->queue_get(QueueType::GRAPHICS);
	present_queue = backend->queue_get(QueueType::PRESENT);

	// Create swapchain
	swapchain = backend->swapchain_create();
	backend->swapchain_resize(graphics_queue, swapchain, p_size, true /* vsync */);

	event::subscribe<WindowCloseEvent>([&](WindowCloseEvent e) { window_should_close = true; });

	// Initialize input system
	Input::init();
}

Window::~Window() {
	backend->swapchain_free(swapchain);

	SDL_DestroyWindow(window);
	SDL_Quit();
}

bool Window::should_close() const { return window_should_close; }

void Window::poll_events() const {
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) {
			event::notify<WindowCloseEvent>(WindowCloseEvent{});
		}

		if (e.type == SDL_WINDOWEVENT) {
			if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
				event::notify<WindowResizeEvent>(WindowResizeEvent{
						.size = { (uint32_t)e.window.data1, (uint32_t)e.window.data2 },
				});
			} else if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
				event::notify<WindowCloseEvent>(WindowCloseEvent{});
			} else if (e.window.event == SDL_WINDOWEVENT_MINIMIZED) {
				event::notify<WindowMinimizeEvent>(WindowMinimizeEvent{});
			}
		}

		if (e.type == SDL_KEYDOWN) {
			// TODO: Filter out repeats if desired (e.key.repeat != 0)
			event::notify<KeyPressEvent>(KeyPressEvent{
					.key_code = static_cast<KeyCode>(e.key.keysym.sym),
			});
		}

		if (e.type == SDL_KEYUP) {
			event::notify<KeyReleaseEvent>(KeyReleaseEvent{
					.key_code = static_cast<KeyCode>(e.key.keysym.sym),
			});
		}

		if (e.type == SDL_TEXTINPUT) {
			event::notify<KeyTypeEvent>(KeyTypeEvent{
					.text = e.text.text,
			});
		}

		if (e.type == SDL_MOUSEMOTION) {
			event::notify<MouseMoveEvent>(MouseMoveEvent{
					.position = { (float)e.motion.x, (float)e.motion.y },
			});
		}

		if (e.type == SDL_MOUSEBUTTONDOWN) {
			event::notify<MousePressEvent>(MousePressEvent{
					.button_code = static_cast<MouseButton>(e.button.button),
			});
		}

		if (e.type == SDL_MOUSEBUTTONUP) {
			event::notify<MouseReleaseEvent>(MouseReleaseEvent{
					.button_code = static_cast<MouseButton>(e.button.button),
			});
		}

		if (e.type == SDL_MOUSEWHEEL) {
			event::notify<MouseScrollEvent>(MouseScrollEvent{
					.offset = { (float)e.wheel.x, (float)e.wheel.y },
			});
		}
	}
}

Image Window::get_target(Semaphore p_image_avail_semaphore) {
	// Acquire the next image from the swapchain
	// This tells the GPU: "Give me an image index I can draw into."
	// It signals 'image_available_sem' when the image is actually ready to be written to.
	uint32_t image_index = 0;
	auto acquire_result =
			backend->swapchain_acquire_image(swapchain, p_image_avail_semaphore, &image_index);

	if (!acquire_result) {
		// TODO: If acquire failed (e.g. window resized), handle it or skip frame
		return GL_NULL_HANDLE;
	}

	return *acquire_result;
}

void Window::present(Semaphore p_render_finished_sem) {
	backend->queue_present(present_queue, swapchain, p_render_finished_sem);
}

void Window::on_resize(const Vec2u& p_size) {
	backend->device_wait();
	backend->swapchain_resize(graphics_queue, swapchain, p_size, true);
}

Vec2u Window::get_size() const {
	Vec2u size;
	SDL_GetWindowSize(window, (int*)&size.x, (int*)&size.y);
	return size;
}

} //namespace gl
