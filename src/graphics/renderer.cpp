#include "graphics/renderer.h"

#include "core/gpu_context.h"
#include "core/log.h"
#include "glgpu/types.h"

namespace gl {

void FrameData::init(std::shared_ptr<RenderBackend> p_backend, CommandQueue p_queue) {
	command_pool = p_backend->command_pool_create(p_queue);
	cmd = p_backend->command_pool_allocate(command_pool);

	wait_sem = p_backend->semaphore_create();
	signal_sem = p_backend->semaphore_create();

	render_fence = p_backend->fence_create();
}

void FrameData::destroy(std::shared_ptr<RenderBackend> p_backend) {
	p_backend->command_pool_free(command_pool);

	p_backend->semaphore_free(wait_sem);
	p_backend->semaphore_free(signal_sem);

	p_backend->fence_free(render_fence);
}

Renderer::Renderer(GpuContext& p_ctx) :
		backend(p_ctx.get_backend()), graphics_queue(backend->queue_get(QueueType::GRAPHICS)) {
	for (auto& frame_data : frames) {
		frame_data.init(backend, graphics_queue);
	}
}

Renderer::~Renderer() {
	backend->device_wait();
	for (auto& frame_data : frames) {
		frame_data.destroy(backend);
	}
}

CommandBuffer Renderer::begin_frame(Image p_target, bool p_to_present) {
	if (!p_target) {
		GL_LOG_ERROR("[Renderer::begin_frame] Invalid target image.");
		return GL_NULL_HANDLE;
	}

	FrameData& frame = _get_current_frame();

	backend->fence_reset(frame.render_fence);

	// Standard Command setup
	backend->command_reset(frame.cmd);
	backend->command_begin(frame.cmd);

	// Image transition
	backend->command_transition_image(
			frame.cmd, p_target, ImageLayout::UNDEFINED, ImageLayout::COLOR_ATTACHMENT_OPTIMAL);

	// Dynamic State
	const Vec3u extent = backend->image_get_size(p_target);
	backend->command_set_viewport(frame.cmd, extent);
	backend->command_set_scissor(frame.cmd, extent);

	// Set render state
	target_image = p_target;
	to_present = p_to_present;

	return frame.cmd;
}

void Renderer::end_frame() {
	if (!target_image) {
		GL_LOG_WARNING("[Renderer::end_frame] Called without Renderer::begin_frame.");
		return;
	}

	FrameData& frame = _get_current_frame();

	// Transition image to present layout if needed
	if (to_present) {
		backend->command_transition_image(frame.cmd, target_image,
				ImageLayout::COLOR_ATTACHMENT_OPTIMAL, ImageLayout::PRESENT_SRC);
	}

	backend->command_end(frame.cmd);

	// Render
	backend->queue_submit(
			graphics_queue, frame.cmd, frame.render_fence, frame.wait_sem, frame.signal_sem);

	// Reset render state
	target_image = GL_NULL_HANDLE;
	to_present = false;

	frame_number++;
}

void Renderer::wait_for_frame() { backend->fence_wait(_get_current_frame().render_fence); }

Semaphore Renderer::get_wait_sem() { return _get_current_frame().wait_sem; }
Semaphore Renderer::get_signal_sem() { return _get_current_frame().signal_sem; }

FrameData& Renderer::_get_current_frame() { return frames[frame_number % SWAPCHAIN_BUFFER_SIZE]; }

} // namespace gl
