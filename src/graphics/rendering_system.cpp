#include "graphics/rendering_system.h"

#include "core/components.h"
#include "core/event_system.h"
#include "core/gpu_context.h"
#include "core/transform.h"
#include "glgpu/color.h"
#include "glgpu/types.h"
#include "graphics/camera.h"
#include "graphics/graphics_pipeline.h"
#include "graphics/renderer.h"

namespace gl {

RenderingSystem::RenderingSystem(GpuContext& p_ctx, std::shared_ptr<Window> p_window) :
		backend(p_ctx.get_backend()),
		window(p_window),
		renderer(std::make_unique<Renderer>(p_ctx)) {
	const GraphicsPipelineCreateInfo create_info = {
		.color_attachments = { window->get_swapchain_format() },
		.enable_depth_testing = false,
		.vertex_shader = "pipelines/unlit/unlit.vert.spv",
		.fragment_shader = "pipelines/unlit/unlit.frag.spv",
	};
	pipeline = std::make_unique<GraphicsPipeline>(p_ctx, create_info);

	{
		std::vector<MeshVertex> vertices = {
			// Position           uv_x   Normal              uv_y
			// Front face (Z = 1)
			{ { -0.5f, -0.5f, 0.5f }, 0.0f, { 0.0f, 0.0f, 1.0f }, 0.0f }, // 0
			{ { 0.5f, -0.5f, 0.5f }, 1.0f, { 0.0f, 0.0f, 1.0f }, 0.0f }, // 1
			{ { 0.5f, 0.5f, 0.5f }, 1.0f, { 0.0f, 0.0f, 1.0f }, 1.0f }, // 2
			{ { -0.5f, 0.5f, 0.5f }, 0.0f, { 0.0f, 0.0f, 1.0f }, 1.0f }, // 3

			// Back face (Z = -1)
			{ { -0.5f, -0.5f, -0.5f }, 1.0f, { 0.0f, 0.0f, -1.0f }, 0.0f }, // 4
			{ { -0.5f, 0.5f, -0.5f }, 1.0f, { 0.0f, 0.0f, -1.0f }, 1.0f }, // 5
			{ { 0.5f, 0.5f, -0.5f }, 0.0f, { 0.0f, 0.0f, -1.0f }, 1.0f }, // 6
			{ { 0.5f, -0.5f, -0.5f }, 0.0f, { 0.0f, 0.0f, -1.0f }, 0.0f }, // 7

			// Top face (Y = 1)
			{ { -0.5f, 0.5f, 0.5f }, 0.0f, { 0.0f, 1.0f, 0.0f }, 0.0f }, // 8
			{ { 0.5f, 0.5f, 0.5f }, 1.0f, { 0.0f, 1.0f, 0.0f }, 0.0f }, // 9
			{ { 0.5f, 0.5f, -0.5f }, 1.0f, { 0.0f, 1.0f, 0.0f }, 1.0f }, // 10
			{ { -0.5f, 0.5f, -0.5f }, 0.0f, { 0.0f, 1.0f, 0.0f }, 1.0f }, // 11

			// Bottom face (Y = -1)
			{ { -0.5f, -0.5f, 0.5f }, 0.0f, { 0.0f, -1.0f, 0.0f }, 1.0f }, // 12
			{ { -0.5f, -0.5f, -0.5f }, 0.0f, { 0.0f, -1.0f, 0.0f }, 0.0f }, // 13
			{ { 0.5f, -0.5f, -0.5f }, 1.0f, { 0.0f, -1.0f, 0.0f }, 0.0f }, // 14
			{ { 0.5f, -0.5f, 0.5f }, 1.0f, { 0.0f, -1.0f, 0.0f }, 1.0f }, // 15

			// Right face (X = 1)
			{ { 0.5f, -0.5f, 0.5f }, 0.0f, { 1.0f, 0.0f, 0.0f }, 0.0f }, // 16
			{ { 0.5f, -0.5f, -0.5f }, 1.0f, { 1.0f, 0.0f, 0.0f }, 0.0f }, // 17
			{ { 0.5f, 0.5f, -0.5f }, 1.0f, { 1.0f, 0.0f, 0.0f }, 1.0f }, // 18
			{ { 0.5f, 0.5f, 0.5f }, 0.0f, { 1.0f, 0.0f, 0.0f }, 1.0f }, // 19

			// Left face (X = -1)
			{ { -0.5f, -0.5f, 0.5f }, 1.0f, { -1.0f, 0.0f, 0.0f }, 0.0f }, // 20
			{ { -0.5f, 0.5f, 0.5f }, 1.0f, { -1.0f, 0.0f, 0.0f }, 1.0f }, // 21
			{ { -0.5f, 0.5f, -0.5f }, 0.0f, { -1.0f, 0.0f, 0.0f }, 1.0f }, // 22
			{ { -0.5f, -0.5f, -0.5f }, 0.0f, { -1.0f, 0.0f, 0.0f }, 0.0f } // 23
		};

		std::vector<uint32_t> indices = {
			0, 1, 2, 0, 2, 3, // Front
			4, 5, 6, 4, 6, 7, // Back
			8, 9, 10, 8, 10, 11, // Top
			12, 13, 14, 12, 14, 15, // Bottom
			16, 17, 18, 16, 18, 19, // Right
			20, 21, 22, 20, 22, 23 // Left
		};

		cube_mesh = StaticMesh::create(backend, vertices, indices);
	}

	{
		scene_buffer = backend->buffer_create(sizeof(SceneData),
				BUFFER_USAGE_STORAGE_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT |
						BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
				MemoryAllocationType::CPU);
		scene_buffer_addr = backend->buffer_get_device_address(scene_buffer);

		SceneData* data = (SceneData*)backend->buffer_map(scene_buffer);
		{
			PerspectiveCamera camera;

			Transform cam_transform;
			cam_transform.position = { 0, 0, -3 };

			data->viewproj = camera.get_projection_matrix() * camera.get_view_matrix(cam_transform);
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

void RenderingSystem::on_init(Registry& p_registry) {
	event::subscribe<WindowResizeEvent>(
			[&](const WindowResizeEvent& e) { window->on_resize(e.size); });
}

void RenderingSystem::on_destroy(Registry& p_registry) {
	backend->device_wait();
	backend->uniform_set_free(material_set);
	backend->buffer_free(material_buffer);
	backend->buffer_free(scene_buffer);

	cube_mesh.reset();
}

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
		// Calculate a color based on time
		static float s_time = 0.0f;
		s_time += p_dt;

		const Color color(std::abs(sin(s_time)), std::abs(cos(s_time)), 0.2f, 1.0f);

		{
			MaterialData* data = (MaterialData*)backend->buffer_map(material_buffer);
			{
				data->base_color = color;
			}
			backend->buffer_unmap(material_buffer);
		}

		RenderingAttachment attachment = {};
		attachment.image = target_image;
		attachment.layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
		attachment.clear_color = COLOR_GRAY;
		attachment.load_op = AttachmentLoadOp::CLEAR;
		attachment.store_op = AttachmentStoreOp::STORE;

		backend->command_begin_rendering(
				cmd, backend->image_get_size(target_image), { attachment });
		{
			backend->command_bind_graphics_pipeline(cmd, pipeline->pipeline);

			backend->command_bind_uniform_sets(cmd, pipeline->shader, 0, { material_set });

			for (Entity entity : p_registry.view<Transform, MeshComponent>()) {
				auto [transform, mc] = p_registry.get_many<Transform, MeshComponent>(entity);

				transform->rotate(20 * p_dt, VEC3_UP);

				switch (mc->type) {
					case MeshType::CUBE:
						PushConstants pc = {};
						pc.transform = transform->to_mat4();
						pc.vertex_buffer_addr = cube_mesh->vertex_buffer_address;
						pc.scene_buffer_addr = scene_buffer_addr;

						backend->command_push_constants(
								cmd, pipeline->shader, 0, sizeof(PushConstants), &pc);

						backend->command_bind_index_buffer(
								cmd, cube_mesh->index_buffer, 0, IndexType::UINT32);
						backend->command_draw_indexed(cmd, 36);
				}
			}
		}
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

} //namespace gl
