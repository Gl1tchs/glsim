#include "graphics/rendering_system.h"

#include "core/assert.h"
#include "core/gpu_context.h"
#include "glgpu/backend.h"
#include <SDL2/SDL_video.h>

#ifndef GL_HEADLESS
#include <SDL2/SDL_syswm.h>
#endif

namespace gl {

RenderingSystem::RenderingSystem(GpuContext& p_ctx) { backend = p_ctx.get_backend(); }

void RenderingSystem::on_init(Registry& p_registry) {
	if (!backend->is_swapchain_supported()) {
		GL_ASSERT(false, "[RenderingSystem::on_init] Swapchain is not supported.");
	}

#ifndef GL_HEADLESS
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		GL_LOG_ERROR("SDL could not initialize! SDL_Error: {}", SDL_GetError());
		return;
	}

	SDL_Window* window = SDL_CreateWindow("glsim", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

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

	GL_ASSERT(
			backend->attach_surface(connection_handle, window_handle) == SurfaceCreateError::NONE);

	graphics_queue = backend->queue_get(QueueType::GRAPHICS);
	present_queue = backend->queue_get(QueueType::PRESENT);

	swapchain = backend->swapchain_create();

	uint32_t w, h;
	SDL_GetWindowSize(window, (int*)&w, (int*)&h);

	backend->swapchain_resize(graphics_queue, swapchain, { w, h }, true /* vsync */);

	// Create Command Pool and Buffer
	cmd_pool = backend->command_pool_create(graphics_queue);
	cmd = backend->command_pool_allocate(cmd_pool);

	// Synchronization Primitives
	image_available_sem = backend->semaphore_create();
	render_finished_sem = backend->semaphore_create();
	frame_fence = backend->fence_create();
#endif
}

void RenderingSystem::on_destroy(Registry& p_registry) {
#ifndef GL_HEADLESS
	// Wait for GPU to finish all operations before destroying resources
	backend->device_wait();

	backend->fence_free(frame_fence);
	backend->semaphore_free(image_available_sem);
	backend->semaphore_free(render_finished_sem);

	// Command buffer is freed when pool is freed, but usually explicit free is good practice
	backend->command_pool_free(cmd_pool);
	backend->swapchain_free(swapchain);

	SDL_DestroyWindow(window);
	SDL_Quit();
#endif

	backend.reset();
}

void RenderingSystem::on_update(Registry& p_registry, float p_dt) {
#ifndef GL_HEADLESS
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) {
			// quit = true;
		}
		if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
			backend->device_wait();
			backend->swapchain_resize(graphics_queue, swapchain,
					{ (uint32_t)e.window.data1, (uint32_t)e.window.data2 }, true);
		}
	}

	// Wait for the previous frame to finish processing on the CPU side
	backend->fence_wait(frame_fence);
	backend->fence_reset(frame_fence);

	// Acquire the next image from the swapchain
	// This tells the GPU: "Give me an image index I can draw into."
	// It signals 'image_available_sem' when the image is actually ready to be written to.
	uint32_t image_index = 0;
	auto acquire_result =
			backend->swapchain_acquire_image(swapchain, image_available_sem, &image_index);

	if (!acquire_result) {
		// If acquire failed (e.g. window resized), handle it or skip frame
		return;
	}

	Image swapchain_image = *acquire_result;

	// Record Commands
	backend->command_reset(cmd);
	backend->command_begin(cmd);

	// Transition Image Layout for Clearing
	// Images coming from the swapchain are usually in an UNDEFINED state.
	// command_clear_color requires the image to be in ImageLayout::GENERAL.
	backend->command_transition_image(
			cmd, swapchain_image, ImageLayout::UNDEFINED, ImageLayout::GENERAL);

	// Clear the Screen
	// Calculate a color based on time
	static float s_time = 0.0f;
	s_time += p_dt;

	Color clear_color = { (float)std::abs(sin(s_time)), (float)std::abs(cos(s_time)), 0.2f, 1.0f };

	backend->command_clear_color(cmd, swapchain_image, clear_color);

	// Transition Image Layout for Presentation
	// The presentation engine requires the image to be in PRESENT_SRC layout.
	backend->command_transition_image(
			cmd, swapchain_image, ImageLayout::GENERAL, ImageLayout::PRESENT_SRC);

	backend->command_end(cmd);

	// Submit Command Buffer
	// We wait for 'image_available_sem' (image is ready to write)
	// We signal 'render_finished_sem' (rendering is done)
	// We signal 'frame_fence' so CPU knows when this batch is done
	backend->queue_submit(
			graphics_queue, cmd, frame_fence, image_available_sem, render_finished_sem);

	// Present the image to the screen
	// Waits for 'render_finished_sem'
	backend->queue_present(present_queue, swapchain, render_finished_sem);
#endif
}

} //namespace gl
