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

Window::Window(GpuContext& ctx, const Vec2u& size, const char* title) :
		_backend(ctx.get_backend()) {
	if (!_backend->is_swapchain_supported()) {
		GL_ASSERT(false, "[Window::Window()] Swapchain is not supported.");
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		GL_LOG_ERROR("SDL could not initialize! SDL_Error: {}", SDL_GetError());
		return;
	}

	SDL_Window* window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			size.x, size.y, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

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
	GL_ASSERT(ctx.get_backend()->attach_surface(connection_handle, window_handle).is_ok());

	_graphics_queue = _backend->queue_get(QueueType::GRAPHICS).value();
	_present_queue = _backend->queue_get(QueueType::PRESENT).value();

	// Create swapchain
	_swapchain = _backend->swapchain_create().value();
	_backend->swapchain_resize(_graphics_queue, _swapchain, size, true /* vsync */);

	event::subscribe<WindowCloseEvent>([&](WindowCloseEvent e) { _window_should_close = true; });

	// Initialize input system
	Input::init();
}

Window::~Window() {
	_backend->swapchain_free(_swapchain);

	SDL_DestroyWindow(_window);
	SDL_Quit();
}

bool Window::should_close() const { return _window_should_close; }

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

Image Window::get_target(Semaphore wait_sem) {
	// Acquire the next image from the swapchain
	// This tells the GPU: "Give me an image index I can draw into."
	// It signals 'image_available_sem' when the image is actually ready to be written to.
	uint32_t image_index = 0;
	const auto acquire_result =
			_backend->swapchain_acquire_image(_swapchain, wait_sem, &image_index);

	if (acquire_result.is_error()) {
		switch (acquire_result.error()) {
			case Error::SWAPCHAIN_OUT_OF_DATE:
				on_resize(get_size());
				return GL_NULL_HANDLE;
			case Error::SWAPCHAIN_SUBOPTIMAL:
				GL_LOG_FATAL("[Window::get_target] Failed to acquire swapchain image.");
				return GL_NULL_HANDLE;
			default:
				break;
		}
	}

	return *acquire_result;
}

void Window::present(Semaphore signal_sem) {
	if (!_backend->queue_present(_present_queue, _swapchain, signal_sem)) {
		on_resize(get_size());
	}
}

void Window::on_resize(const Vec2u& size) {
	_backend->device_wait();
	_backend->swapchain_resize(_graphics_queue, _swapchain, size, true);
}

Vec2u Window::get_size() const {
	Vec2u size;
	SDL_GetWindowSize(_window, (int*)&size.x, (int*)&size.y);
	return size;
}

DataFormat Window::get_swapchain_format() const {
	return _backend->swapchain_get_format(_swapchain).value();
}

} // namespace gl
