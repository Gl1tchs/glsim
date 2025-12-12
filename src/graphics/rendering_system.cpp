#include "graphics/rendering_system.h"

#include "core/event_system.h"
#include "core/gpu_context.h"
#include "glgpu/types.h"
#include "graphics/renderer.h"

namespace gl {

RenderingSystem::RenderingSystem(GpuContext& p_ctx, std::shared_ptr<Window> p_window) :
		backend(p_ctx.get_backend()),
		window(p_window),
		renderer(std::make_unique<Renderer>(p_ctx)) {}

void RenderingSystem::on_init(Registry& p_registry) {
	event::subscribe<WindowResizeEvent>(
			[&](const WindowResizeEvent& e) { window->on_resize(e.size); });
}

void RenderingSystem::on_destroy(Registry& p_registry) {}

void RenderingSystem::on_update(Registry& p_registry, float p_dt) {
	renderer->wait_for_frame();

	Semaphore signal_sem = renderer->get_signal_sem();
	Semaphore wait_sem = renderer->get_wait_sem();

	// Retrieve image from swapchain
	Image target_image = window->get_target(wait_sem);
	if (!target_image) {
		return;
	}

	// Begin frame
	CommandBuffer cmd = renderer->begin_frame(target_image);
	{
		backend->command_transition_image(
				cmd, target_image, ImageLayout::UNDEFINED, ImageLayout::GENERAL);

		// Clear the Screen
		// Calculate a color based on time
		static float s_time = 0.0f;
		s_time += p_dt;

		Color clear_color = { (float)std::abs(sin(s_time)), (float)std::abs(cos(s_time)), 0.2f,
			1.0f };

		backend->command_clear_color(cmd, target_image, clear_color);

		// Transition Image Layout for Presentation
		// The presentation engine requires the image to be in PRESENT_SRC layout.
		backend->command_transition_image(
				cmd, target_image, ImageLayout::GENERAL, ImageLayout::PRESENT_SRC);
	}
	renderer->end_frame();

	// Present the image to the screen
	// Waits for 'render_finished_semaphore'
	window->present(signal_sem);
}

} //namespace gl
