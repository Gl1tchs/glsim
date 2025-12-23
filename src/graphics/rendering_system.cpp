#include "graphics/rendering_system.h"

#include "core/components.h"
#include "core/event_system.h"
#include "core/gpu_context.h"
#include "core/transform.h"
#include "glgpu/color.h"
#include "glgpu/types.h"
#include "graphics/camera.h"
#include "graphics/graphics_pipeline.h"
#include "graphics/primitives.h"
#include "graphics/renderer.h"

namespace gl {

RenderingSystem::RenderingSystem(GpuContext& p_ctx, std::shared_ptr<Window> p_window) :
		backend(p_ctx.get_backend()),
		window(p_window),
		renderer(std::make_unique<Renderer>(p_ctx)) {
	// Initialize rendering infrastructure
	_init_pipelines(p_ctx);
	_init_primitives();

	// Allocate GPU memory for scene and material data
	_init_scene_buffer();
	_init_material_buffer();
}

RenderingSystem::~RenderingSystem() {
	backend->device_wait();

	// Clean up resources
	backend->uniform_set_free(material_set);
	backend->buffer_free(material_buffer);
	backend->buffer_free(scene_buffer);
}

void RenderingSystem::on_init(Registry& p_registry) {
	event::subscribe<WindowResizeEvent>(
			[&](const WindowResizeEvent& e) { window->on_resize(e.size); });
}

void RenderingSystem::on_destroy(Registry& p_registry) {}

void RenderingSystem::on_update(Registry& p_registry, float p_dt) {
	// Wait for previous frame to be submitted
	renderer->wait_for_frame();

	Semaphore wait_sem = renderer->get_wait_sem();
	Semaphore signal_sem = renderer->get_signal_sem();

	Image target_image = window->get_target(wait_sem);
	if (!target_image) {
		return; // Swapchain is likely out of date or minimized
	}

	// CPU-Side state updates
	_update_scene_uniforms(p_registry, target_image);
	_update_material_uniforms();

	// GPU command recording
	CommandBuffer cmd = renderer->begin_frame(target_image);

	FrameContext frame_ctx = {
		.cmd = cmd,
		.target_image = target_image,
		.dt = p_dt,
	};

	{
		// Transition to attachment layout
		RenderingAttachment attachment = _create_color_attachment(target_image);

		backend->command_begin_rendering(
				cmd, backend->image_get_size(target_image), { attachment });

		// Execute render passes
		_execute_geometry_pass(frame_ctx, p_registry);

		backend->command_end_rendering(cmd);

		// Transition to present layout
		backend->command_transition_image(
				cmd, target_image, ImageLayout::COLOR_ATTACHMENT_OPTIMAL, ImageLayout::PRESENT_SRC);
	}

	renderer->end_frame();

	window->present(signal_sem);
}

void RenderingSystem::_execute_geometry_pass(const FrameContext& ctx, Registry& p_registry) {
	// Bind pipeline global state
	backend->command_bind_graphics_pipeline(ctx.cmd, pipeline->pipeline);
	backend->command_bind_uniform_sets(ctx.cmd, pipeline->shader, 0, { material_set });

	for (Entity entity : p_registry.view<Transform, MeshComponent>()) {
		auto [transform, mc] = p_registry.get_many<Transform, MeshComponent>(entity);

		std::shared_ptr<StaticMesh> mesh = _resolve_mesh(mc->type);
		if (!mesh) {
			continue;
		}

		// Push constants
		PushConstants pc = {};
		pc.transform = transform->to_mat4();
		pc.vertex_buffer_addr = mesh->vertex_buffer_address;
		pc.scene_buffer_addr = scene_buffer_addr;

		backend->command_push_constants(ctx.cmd, pipeline->shader, 0, sizeof(PushConstants), &pc);

		// Draw call
		backend->command_bind_index_buffer(ctx.cmd, mesh->index_buffer, 0, IndexType::UINT32);
		backend->command_draw_indexed(ctx.cmd, mesh->index_count);
	}
}

void RenderingSystem::_init_pipelines(GpuContext& p_ctx) {
	const GraphicsPipelineCreateInfo create_info = {
		.color_attachments = { window->get_swapchain_format() },
		.enable_depth_testing = false,
		// NOTE: memory data is being referenced
		.vertex_shader = "pipelines/unlit/unlit.vert.spv",
		.fragment_shader = "pipelines/unlit/unlit.frag.spv",
	};
	pipeline = std::make_unique<GraphicsPipeline>(p_ctx, create_info);
}

void RenderingSystem::_init_primitives() {
	primitives.cube = create_cube_mesh(backend);
	primitives.plane = create_plane_mesh(backend);
	primitives.sphere = create_sphere_mesh(backend);
}

void RenderingSystem::_init_scene_buffer() {
	scene_buffer = backend->buffer_create(sizeof(SceneData),
			BUFFER_USAGE_STORAGE_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT |
					BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			MemoryAllocationType::CPU);
	scene_buffer_addr = backend->buffer_get_device_address(scene_buffer);
}

void RenderingSystem::_init_material_buffer() {
	material_buffer = backend->buffer_create(sizeof(MaterialData),
			BUFFER_USAGE_UNIFORM_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT |
					BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			MemoryAllocationType::CPU);

	// Initialize material descriptor set
	ShaderUniform uniform;
	uniform.type = ShaderUniformType::UNIFORM_BUFFER;
	uniform.binding = 0;
	uniform.data.push_back(material_buffer);

	material_set = backend->uniform_set_create({ uniform }, pipeline->shader, 0);

	// Initialize default data
	_update_material_uniforms();
}

void RenderingSystem::_update_scene_uniforms(Registry& p_registry, Image p_target_image) {
	const Vec3u size = backend->image_get_size(p_target_image);

	float aspect_ratio = 1.0f;
	if (size.x > 0 && size.y > 0) {
		aspect_ratio = size.y / (float)size.x;
	}

	Mat4 viewproj = Mat4(1.0f);
	for (Entity entity : p_registry.view<Transform, CameraComponent>()) {
		auto [transform, cc] = p_registry.get_many<Transform, CameraComponent>(entity);

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

	SceneData* data = (SceneData*)backend->buffer_map(scene_buffer);
	if (data) {
		data->viewproj = viewproj;
		backend->buffer_unmap(scene_buffer);
	}
}

void RenderingSystem::_update_material_uniforms() {
	MaterialData* data = (MaterialData*)backend->buffer_map(material_buffer);
	if (data) {
		data->base_color = COLOR_MAGENTA;
		backend->buffer_unmap(material_buffer);
	}
}

RenderingAttachment RenderingSystem::_create_color_attachment(Image p_target) {
	RenderingAttachment attachment = {};
	attachment.image = p_target;
	attachment.layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	attachment.clear_color = COLOR_GRAY;
	attachment.load_op = AttachmentLoadOp::CLEAR;
	attachment.store_op = AttachmentStoreOp::STORE;
	return attachment;
}

std::shared_ptr<StaticMesh> RenderingSystem::_resolve_mesh(PrimitiveType p_type) {
	switch (p_type) {
		case PrimitiveType::CUBE:
			return primitives.cube;
		case PrimitiveType::PLANE:
			return primitives.plane;
		case PrimitiveType::SPHERE:
			return primitives.sphere;
		default:
			return nullptr;
	}
}

} // namespace gl
