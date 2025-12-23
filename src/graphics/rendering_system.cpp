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
	// Create graphics pipeline
	const GraphicsPipelineCreateInfo create_info = {
		.color_attachments = { window->get_swapchain_format() },
		.enable_depth_testing = false,
		.vertex_shader = "pipelines/unlit/unlit.vert.spv",
		.fragment_shader = "pipelines/unlit/unlit.frag.spv",
	};
	pipeline = std::make_unique<GraphicsPipeline>(p_ctx, create_info);

	// Create primitives
	primitives.cube = create_cube_mesh(backend);
	primitives.plane = create_plane_mesh(backend);
	primitives.sphere = create_sphere_mesh(backend);

	{
		scene_buffer = backend->buffer_create(sizeof(SceneData),
				BUFFER_USAGE_STORAGE_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT |
						BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
				MemoryAllocationType::CPU);
		scene_buffer_addr = backend->buffer_get_device_address(scene_buffer);

		SceneData* data = (SceneData*)backend->buffer_map(scene_buffer);
		{
			camera_transform.position = { 0, 0, 3 };
			data->viewproj =
					camera.get_projection_matrix() * camera.get_view_matrix(camera_transform);
		}
		backend->buffer_unmap(scene_buffer);
	}

	{
		material_buffer = backend->buffer_create(sizeof(MaterialData),
				BUFFER_USAGE_UNIFORM_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT |
						BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
				MemoryAllocationType::CPU);

		MaterialData* data = (MaterialData*)backend->buffer_map(material_buffer);
		{
			data->base_color = COLOR_RED;
		}
		backend->buffer_unmap(material_buffer);

		ShaderUniform uniform;
		uniform.type = ShaderUniformType::UNIFORM_BUFFER;
		uniform.binding = 0;
		uniform.data.push_back(material_buffer);

		material_set = backend->uniform_set_create({ uniform }, pipeline->shader, 0);
	}
}

RenderingSystem::~RenderingSystem() {
	backend->device_wait();

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
		// Prepare scene and material resources
		_prepare_resources(p_registry, target_image);

		RenderingAttachment attachment = {};
		attachment.image = target_image;
		attachment.layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
		attachment.clear_color = COLOR_GRAY;
		attachment.load_op = AttachmentLoadOp::CLEAR;
		attachment.store_op = AttachmentStoreOp::STORE;

		backend->command_begin_rendering(
				cmd, backend->image_get_size(target_image), { attachment });

		// TODO: move this to _geometry_pass when other materials added

		// Bind cube pipeline (this is the only one existing at the moment)
		backend->command_bind_graphics_pipeline(cmd, pipeline->pipeline);

		// Bind scene buffer
		backend->command_bind_uniform_sets(cmd, pipeline->shader, 0, { material_set });

		_geometry_pass(cmd, p_registry, p_dt);

		backend->command_end_rendering(cmd);

		// Transition Image Layout for Presentation
		// The presentation engine requires the image to be in PRESENT_SRC layout.
		backend->command_transition_image(
				cmd, target_image, ImageLayout::COLOR_ATTACHMENT_OPTIMAL, ImageLayout::PRESENT_SRC);
	}
	renderer->end_frame();

	// Present the image to the screen
	// Waits for 'render_finished_semaphore'
	window->present(signal_sem);
}

void RenderingSystem::_prepare_resources(Registry& p_registry, Image p_target_image) {
	{
		MaterialData* data = (MaterialData*)backend->buffer_map(material_buffer);
		{
			data->base_color = COLOR_MAGENTA;
		}
		backend->buffer_unmap(material_buffer);
	}

	{
		const Vec3u size = backend->image_get_size(p_target_image);

		// Update aspect ratio
		camera.aspect_ratio = size.y / (float)size.x;

		SceneData* data = (SceneData*)backend->buffer_map(scene_buffer);
		{
			data->viewproj =
					camera.get_projection_matrix() * camera.get_view_matrix(camera_transform);
		}
		backend->buffer_unmap(scene_buffer);
	}
}

void RenderingSystem::_geometry_pass(CommandBuffer p_cmd, Registry& p_registry, float p_dt) {
	for (Entity entity : p_registry.view<Transform, MeshComponent>()) {
		auto [transform, mc] = p_registry.get_many<Transform, MeshComponent>(entity);

		// TODO: remove this
		transform->rotate(20 * p_dt, VEC3_UP);

		std::shared_ptr<StaticMesh> mesh_in_use = nullptr;
		switch (mc->type) {
			case PrimitiveType::CUBE:
				mesh_in_use = primitives.cube;
				break;
			case PrimitiveType::PLANE:
				mesh_in_use = primitives.plane;
				break;
			case PrimitiveType::SPHERE:
				mesh_in_use = primitives.sphere;
				break;
			default:
				continue;
		}

		if (!mesh_in_use) {
			continue;
		}

		// Bind push constants
		PushConstants pc = {};
		pc.transform = transform->to_mat4();
		pc.vertex_buffer_addr = mesh_in_use->vertex_buffer_address;
		pc.scene_buffer_addr = scene_buffer_addr;

		backend->command_push_constants(p_cmd, pipeline->shader, 0, sizeof(PushConstants), &pc);

		// Draw
		backend->command_bind_index_buffer(p_cmd, mesh_in_use->index_buffer, 0, IndexType::UINT32);
		backend->command_draw_indexed(p_cmd, mesh_in_use->index_count);
	}
}

} //namespace gl
