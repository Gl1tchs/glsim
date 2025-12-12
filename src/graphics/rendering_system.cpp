#include "graphics/rendering_system.h"

#include "core/event_system.h"
#include "core/gpu_context.h"

namespace gl {

RenderingSystem::RenderingSystem(GpuContext& p_ctx, std::shared_ptr<Window> p_window) :
		backend(p_ctx.get_backend()), window(p_window) {}

void RenderingSystem::on_init(Registry& p_registry) {
	graphics_queue = backend->queue_get(QueueType::GRAPHICS);

	// Create Command Pool and Buffer
	cmd_pool = backend->command_pool_create(graphics_queue);
	cmd = backend->command_pool_allocate(cmd_pool);

	// Synchronization Primitives
	image_available_sem = backend->semaphore_create();
	render_finished_sem = backend->semaphore_create();
	frame_fence = backend->fence_create();

	event::subscribe<WindowResizeEvent>(
			[&](const WindowResizeEvent& e) { window->on_resize(e.size); });
}

void RenderingSystem::on_destroy(Registry& p_registry) {
	// Wait for GPU to finish all operations before destroying resources
	backend->device_wait();

	backend->fence_free(frame_fence);
	backend->semaphore_free(image_available_sem);
	backend->semaphore_free(render_finished_sem);

	// Command buffer is freed when pool is freed, but usually explicit free is good practice
	backend->command_pool_free(cmd_pool);
}

void RenderingSystem::on_update(Registry& p_registry, float p_dt) {
	// Wait for the previous frame to finish processing on the CPU side
	backend->fence_wait(frame_fence);
	backend->fence_reset(frame_fence);

	Image target_image = window->get_target(image_available_sem);
	if (!target_image) {
		return;
	}

	// Record Commands
	backend->command_reset(cmd);
	backend->command_begin(cmd);

	// Transition Image Layout for Clearing
	// Images coming from the swapchain are usually in an UNDEFINED state.
	// command_clear_color requires the image to be in ImageLayout::GENERAL.
	backend->command_transition_image(
			cmd, target_image, ImageLayout::UNDEFINED, ImageLayout::GENERAL);

	// Clear the Screen
	// Calculate a color based on time
	static float s_time = 0.0f;
	s_time += p_dt;

	Color clear_color = { (float)std::abs(sin(s_time)), (float)std::abs(cos(s_time)), 0.2f, 1.0f };

	backend->command_clear_color(cmd, target_image, clear_color);

	// Transition Image Layout for Presentation
	// The presentation engine requires the image to be in PRESENT_SRC layout.
	backend->command_transition_image(
			cmd, target_image, ImageLayout::GENERAL, ImageLayout::PRESENT_SRC);

	backend->command_end(cmd);

	// Submit Command Buffer
	// We wait for 'image_available_sem' (image is ready to write)
	// We signal 'render_finished_sem' (rendering is done)
	// We signal 'frame_fence' so CPU knows when this batch is done
	backend->queue_submit(
			graphics_queue, cmd, frame_fence, image_available_sem, render_finished_sem);

	// Present the image to the screen
	// Waits for 'render_finished_sem'
	window->present(render_finished_sem);
}

} //namespace gl
