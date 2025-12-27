#pragma once

#include "glgpu/backend.h"
#include "glgpu/types.h"

namespace gl {

struct FrameData {
	CommandPool command_pool = GL_NULL_HANDLE;
	CommandBuffer cmd = GL_NULL_HANDLE;

	Semaphore wait_sem = GL_NULL_HANDLE, signal_sem = GL_NULL_HANDLE;
	Fence render_fence = GL_NULL_HANDLE;

	void init(std::shared_ptr<RenderBackend> p_backend, CommandQueue p_queue);
	void destroy(std::shared_ptr<RenderBackend> p_backend);
};

/**
 * Class responsible for creating render resources and handling synchronization.
 *
 */
class Renderer {
public:
	Renderer(std::shared_ptr<RenderBackend> p_backend);
	~Renderer();

	/**
	 * Starts a frame to render, prepares render resources handles
	 * synchronization.
	 *
	 * @param p_target Target image to draw
	 * @param p_to_present Should image layout be transitioned to PRESENT_SRC
	 */
	CommandBuffer begin_frame(Image p_target, bool p_to_present = false);

	void end_frame();

	/**
	 * Waits for `render_fence` / render operations to finish.
	 */
	void wait_for_frame();

	Semaphore get_wait_sem();
	Semaphore get_signal_sem();

private:
	FrameData& _get_current_frame();

private:
	std::shared_ptr<RenderBackend> _backend;
	CommandQueue _graphics_queue;

	static constexpr uint8_t SWAPCHAIN_BUFFER_SIZE = 3;

	FrameData _frames[SWAPCHAIN_BUFFER_SIZE];
	uint32_t _frame_number = 0;

	// Render state
	Image _target_image = GL_NULL_HANDLE;
	bool _to_present = false;
};

} //namespace gl
