#include "graphics/rendering_system.h"

#include "core/components.h"
#include "core/event_system.h"
#include "core/gpu_context.h"
#include "core/registry.h"
#include "core/transform.h"
#include "glgpu/types.h"
#include "graphics/aabb.h"
#include "graphics/camera.h"
#include "graphics/geometry_pass.h"
#include "graphics/render_pass.h"
#include "graphics/renderer.h"
#include "graphics/ssao_pass.h"

namespace gl {

struct SceneData {
	Mat4 viewproj;
};

RenderingSystem::RenderingSystem(GpuContext& ctx, std::shared_ptr<Window> window) :
		_backend(ctx.get_backend()),
		_window(window),
		_renderer(std::make_unique<Renderer>(_backend)) {
	_init_g_buffers(window->get_size());

	// Allocate GPU memory for scene and material data
	_init_scene_resources();

	// Add render passes
	_add_render_pass<GeometryPass>();
	_add_render_pass<SSAOPass>();
}

RenderingSystem::~RenderingSystem() {
	_backend->device_wait();

	_backend->image_free(_g_position);
	_backend->image_free(_g_normal);
	_backend->image_free(_g_albedo);
	_backend->image_free(_g_depth);

	_backend->buffer_free(_scene_buffer);
}

void RenderingSystem::on_init(Registry& registry) {
	event::subscribe<WindowResizeEvent>([&](const WindowResizeEvent& e) {
		_window->on_resize(e.size);
		_init_g_buffers(e.size);

		RenderPassResources res = {
			.g_position = _g_position,
			.g_normal = _g_normal,
			.g_albedo = _g_albedo,
			.g_depth = _g_depth,
			.scene_buffer_addr = _scene_buffer_addr,
		};

		for (auto& pass : _render_passes) {
			pass->on_resize(e.size, res);
		}
	});

	// Initialize render passes
	RenderPassResources res = {
		.g_position = _g_position,
		.g_normal = _g_normal,
		.g_albedo = _g_albedo,
		.g_depth = _g_depth,
		.scene_buffer_addr = _scene_buffer_addr,
	};

	for (auto& pass : _render_passes) {
		pass->init(_backend, res);
	}
}

void RenderingSystem::on_destroy(Registry& registry) {}

void RenderingSystem::on_update(Registry& registry, float dt) {
	// Wait for previous frame to be submitted
	_renderer->wait_for_frame();

	Semaphore wait_sem = _renderer->get_wait_sem();
	Semaphore signal_sem = _renderer->get_signal_sem();

	Image swapchain_image = _window->get_target(wait_sem);
	if (!swapchain_image) {
		return; // Swapchain is likely out of date or minimized
	}

	// Prepare camera and frustum
	Mat4 viewproj = _get_camera_viewproj(registry, swapchain_image);
	Frustum frustum = Frustum::from_view_proj(viewproj);

	// CPU-Side state updates
	_update_scene_resources(viewproj);

	// GPU command recording
	CommandBuffer cmd = _renderer->begin_frame(swapchain_image);

	// Execute render passes
	FrameContext ctx = {
		.cmd = cmd,
		.dt = dt,
		.frustum = frustum,
		.viewproj = viewproj,
	};

	RenderPassResources res = {
		.g_position = _g_position,
		.g_normal = _g_normal,
		.g_albedo = _g_albedo,
		.g_depth = _g_depth,
		.scene_buffer_addr = _scene_buffer_addr,
	};

	for (auto& pass : _render_passes) {
		pass->execute(ctx, registry, res);
	}

	// Copy albedo image to swapchain
	_backend->command_transition_image(
			cmd, swapchain_image, ImageLayout::UNDEFINED, ImageLayout::TRANSFER_DST_OPTIMAL);
	_backend->command_transition_image(
			cmd, _g_albedo, ImageLayout::UNDEFINED, ImageLayout::TRANSFER_SRC_OPTIMAL);

	Vec2u target_size = _backend->image_get_size(swapchain_image).value();
	_backend->command_copy_image_to_image(
			cmd, _g_albedo, swapchain_image, target_size, target_size);

	_backend->command_transition_image(
			cmd, swapchain_image, ImageLayout::TRANSFER_DST_OPTIMAL, ImageLayout::PRESENT_SRC);

	_renderer->end_frame();

	_window->present(signal_sem);
}

void RenderingSystem::_init_g_buffers(const Vec2u& size) {
	// Free already existing buffers
	if (_g_position)
		_backend->image_free(_g_position);
	if (_g_normal)
		_backend->image_free(_g_normal);
	if (_g_albedo)
		_backend->image_free(_g_albedo);
	if (_g_depth)
		_backend->image_free(_g_depth);

	ImageCreateInfo img_info = {
		.size = size,
		.usage = IMAGE_USAGE_COLOR_ATTACHMENT_BIT | IMAGE_USAGE_TRANSFER_SRC_BIT |
				IMAGE_USAGE_TRANSFER_DST_BIT | IMAGE_USAGE_STORAGE_BIT | IMAGE_USAGE_SAMPLED_BIT,
		.mipmapped = false,
		.samples = 1,
	};

	// Init G-Buffers
	img_info.format = DataFormat::R16G16B16A16_SFLOAT;
	_g_position = _backend->image_create(img_info).value();

	img_info.format = DataFormat::R16G16B16A16_SFLOAT;
	_g_normal = _backend->image_create(img_info).value();

	img_info.format = DataFormat::R8G8B8A8_UNORM;
	_g_albedo = _backend->image_create(img_info).value();

	img_info.format = DataFormat::D32_SFLOAT;
	img_info.usage = IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | IMAGE_USAGE_STORAGE_BIT;
	_g_depth = _backend->image_create(img_info).value();
}

Mat4 RenderingSystem::_get_camera_viewproj(Registry& registry, Image target_image) {
	const Vec3u size = _backend->image_get_size(target_image).value();

	float aspect_ratio = 1.0f;
	if (size.x > 0 && size.y > 0) {
		aspect_ratio = (float)size.x / size.y;
	}

	Mat4 viewproj = Mat4(1.0f);
	for (Entity entity : registry.view<Transform, CameraComponent>()) {
		auto [transform, cc] = registry.get_many<Transform, CameraComponent>(entity);

		if (!cc->enabled) {
			continue;
		}

		switch (cc->projection) {
			case CameraProjection::ORTHOGRAPHIC:
				cc->ortho.aspect_ratio = aspect_ratio;
				viewproj =
						cc->ortho.get_projection_matrix() * cc->ortho.get_view_matrix(*transform);
				break;
			case CameraProjection::PERSPECTIVE:
				cc->persp.aspect_ratio = aspect_ratio;
				viewproj =
						cc->persp.get_projection_matrix() * cc->persp.get_view_matrix(*transform);
				break;
		}

		break;
	}

	return viewproj;
}

void RenderingSystem::_init_scene_resources() {
	// Init scene buffer
	_scene_buffer =
			_backend->buffer_create(sizeof(SceneData),
							BUFFER_USAGE_STORAGE_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT |
									BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
							MemoryAllocationType::CPU)
					.value();
	_scene_buffer_addr = _backend->buffer_get_device_address(_scene_buffer).value();
}

void RenderingSystem::_update_scene_resources(const Mat4& viewproj) {
	SceneData* data = (SceneData*)_backend->buffer_map(_scene_buffer).value();
	if (data) {
		data->viewproj = viewproj;
		_backend->buffer_unmap(_scene_buffer);
	}
}

} // namespace gl
