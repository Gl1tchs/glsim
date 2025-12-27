#include "graphics/rendering_system.h"

#include "core/components.h"
#include "core/event_system.h"
#include "core/gpu_context.h"
#include "core/registry.h"
#include "core/transform.h"
#include "glgpu/color.h"
#include "glgpu/types.h"
#include "graphics/aabb.h"
#include "graphics/camera.h"
#include "graphics/graphics_pipeline.h"
#include "graphics/material.h"
#include "graphics/primitives.h"
#include "graphics/renderer.h"

namespace gl {

struct SceneData {
	Mat4 viewproj;
};

struct PushConstants {
	Mat4 transform;
	BufferDeviceAddress vertex_buffer_addr;
	BufferDeviceAddress scene_buffer_addr;
};

struct MaterialData {
	Color base_color;
};

RenderingSystem::RenderingSystem(GpuContext& ctx, std::shared_ptr<Window> window) :
		_backend(ctx.get_backend()),
		_window(window),
		_renderer(std::make_unique<Renderer>(_backend)) {
	// Initialize rendering infrastructure
	_init_pipelines();
	_init_primitives();

	// Allocate GPU memory for scene and material data
	_init_scene_buffer();
	_init_default_material();
}

RenderingSystem::~RenderingSystem() {
	_backend->device_wait();

	// Clean up resources
	_backend->buffer_free(_scene_buffer);
}

void RenderingSystem::on_init(Registry& registry) {
	event::subscribe<WindowResizeEvent>(
			[&](const WindowResizeEvent& e) { _window->on_resize(e.size); });
}

void RenderingSystem::on_destroy(Registry& registry) {}

void RenderingSystem::on_update(Registry& registry, float dt) {
	// Wait for previous frame to be submitted
	_renderer->wait_for_frame();

	Semaphore wait_sem = _renderer->get_wait_sem();
	Semaphore signal_sem = _renderer->get_signal_sem();

	Image target_image = _window->get_target(wait_sem);
	if (!target_image) {
		return; // Swapchain is likely out of date or minimized
	}

	// Prepare camera and frustum
	Mat4 viewproj = _get_camera_viewproj(registry, target_image);
	Frustum frustum = Frustum::from_view_proj(viewproj);

	// CPU-Side state updates
	_update_scene_uniforms(viewproj);
	_update_material_uniforms(registry);

	// GPU command recording
	CommandBuffer cmd = _renderer->begin_frame(target_image);

	FrameContext frame_ctx = {
		.cmd = cmd,
		.target_image = target_image,
		.dt = dt,
		.frustum = frustum,
	};

	{
		// Transition to attachment layout
		RenderingAttachment attachment = _create_color_attachment(target_image);

		_backend->command_begin_rendering(
				cmd, _backend->image_get_size(target_image).value(), { attachment });

		// Execute render passes
		_execute_geometry_pass(frame_ctx, registry);

		_backend->command_end_rendering(cmd);

		// Transition to present layout
		_backend->command_transition_image(
				cmd, target_image, ImageLayout::COLOR_ATTACHMENT_OPTIMAL, ImageLayout::PRESENT_SRC);
	}

	_renderer->end_frame();

	_window->present(signal_sem);
}

void RenderingSystem::_execute_geometry_pass(const FrameContext& ctx, Registry& registry) {
	// Bind pipeline global state
	_backend->command_bind_graphics_pipeline(ctx.cmd, _pipeline->pipeline);

	for (Entity entity : registry.view<Transform, MeshComponent>()) {
		auto [transform, mc] = registry.get_many<Transform, MeshComponent>(entity);

		std::shared_ptr<StaticMesh> mesh = _resolve_mesh(mc->type);
		if (!mesh) {
			continue;
		}

		const Mat4 transform_mat = transform->to_mat4();

		// If objects is not inside of the view frustum, discard it.
		const AABB aabb = mesh->aabb.transform(transform_mat);
		if (!aabb.is_inside_frustum(ctx.frustum)) {
			continue;
		}

		std::shared_ptr<Material> material = default_material;

		// If entitty has a material component use it
		// otherwise use the default one
		if (MaterialComponent* mc = registry.get<MaterialComponent>(entity)) {
			auto& manager = registry.get_asset_manager();
			if (auto m = manager.get<Material>(mc->material_handle)) {
				material = m;
			}
		}

		_backend->command_bind_uniform_sets(
				ctx.cmd, _pipeline->shader, 0, { material->uniform_set });

		// Push constants
		PushConstants pc = {};
		pc.transform = transform_mat;
		pc.vertex_buffer_addr = mesh->vertex_buffer_address;
		pc.scene_buffer_addr = _scene_buffer_addr;

		_backend->command_push_constants(ctx.cmd, _pipeline->shader, 0, sizeof(PushConstants), &pc);

		// Draw call
		_backend->command_bind_index_buffer(ctx.cmd, mesh->index_buffer, 0, IndexType::UINT32);
		_backend->command_draw_indexed(ctx.cmd, mesh->index_count);
	}
}

void RenderingSystem::_init_pipelines() {
	const GraphicsPipelineCreateInfo create_info = {
		.color_attachments = { _window->get_swapchain_format() },
		.enable_depth_testing = false,
		// NOTE: memory data is being referenced
		.vertex_shader = "pipelines/unlit/unlit.vert.spv",
		.fragment_shader = "pipelines/unlit/unlit.frag.spv",
	};
	_pipeline = GraphicsPipeline::create(_backend, create_info);
}

void RenderingSystem::_init_primitives() {
	_primitives.cube = create_cube_mesh(_backend);
	_primitives.plane = create_plane_mesh(_backend);
	_primitives.sphere = create_sphere_mesh(_backend);
}

void RenderingSystem::_init_scene_buffer() {
	_scene_buffer =
			_backend->buffer_create(sizeof(SceneData),
							BUFFER_USAGE_STORAGE_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT |
									BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
							MemoryAllocationType::CPU)
					.value();
	_scene_buffer_addr = _backend->buffer_get_device_address(_scene_buffer).value();
}

void RenderingSystem::_init_default_material() {
	white_texture = Texture::create(_backend, COLOR_WHITE);

	default_material = std::make_shared<Material>();
	default_material->pipeline = _pipeline;
	default_material->base_color = COLOR_WHITE;
	default_material->diffuse_texture = white_texture;
	default_material->upload(_backend);
}

Mat4 RenderingSystem::_get_camera_viewproj(Registry& registry, Image target_image) {
	const Vec3u size = _backend->image_get_size(target_image).value();

	float aspect_ratio = 1.0f;
	if (size.x > 0 && size.y > 0) {
		aspect_ratio = size.y / (float)size.x;
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

void RenderingSystem::_update_scene_uniforms(const Mat4& viewproj) {
	SceneData* data = (SceneData*)_backend->buffer_map(_scene_buffer).value();
	if (data) {
		data->viewproj = viewproj;
		_backend->buffer_unmap(_scene_buffer);
	}
}

void RenderingSystem::_update_material_uniforms(Registry& registry) {
	for (Entity entity : registry.view<MaterialComponent>()) {
		MaterialComponent* mc = registry.get<MaterialComponent>(entity);

		auto& manager = registry.get_asset_manager();

		auto material = manager.get<Material>(mc->material_handle);
		if (!material) {
			continue;
		}

		// Let material construct uniform buffers if needed
		if (material->is_dirty() || !material->is_complete()) {
			material->pipeline = _pipeline;
			// Use default texture if none provided
			if (!material->diffuse_texture) {
				material->diffuse_texture = white_texture;
			}
			material->upload(_backend);
		}
	}
}

RenderingAttachment RenderingSystem::_create_color_attachment(Image target) {
	RenderingAttachment attachment = {};
	attachment.image = target;
	attachment.layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	attachment.clear_color = COLOR_GRAY;
	attachment.load_op = AttachmentLoadOp::CLEAR;
	attachment.store_op = AttachmentStoreOp::STORE;
	return attachment;
}

std::shared_ptr<StaticMesh> RenderingSystem::_resolve_mesh(PrimitiveType type) {
	switch (type) {
		case PrimitiveType::CUBE:
			return _primitives.cube;
		case PrimitiveType::PLANE:
			return _primitives.plane;
		case PrimitiveType::SPHERE:
			return _primitives.sphere;
		default:
			return nullptr;
	}
}

} // namespace gl
