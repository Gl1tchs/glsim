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
#include "graphics/primitives.h"
#include "graphics/renderer.h"

namespace gl {

struct SceneData {
	Mat4 viewproj;
};

struct PushConstants {
	BufferDeviceAddress vertex_buffer_addr;
	BufferDeviceAddress scene_buffer_addr;
	BufferDeviceAddress instance_buffer_addr;
	BufferDeviceAddress material_buffer_addr;
	uint32_t base_instance_offset;
};

struct MaterialData {
	Color base_color;
};

struct InstanceData {
	Mat4 transform;
	uint32_t material_id;
	uint32_t padding[3];
};

RenderingSystem::RenderingSystem(GpuContext& ctx, std::shared_ptr<Window> window) :
		_backend(ctx.get_backend()),
		_window(window),
		_renderer(std::make_unique<Renderer>(_backend)) {
	// Initialize rendering infrastructure
	_init_pipelines();
	_init_primitives();

	// Allocate GPU memory for scene and material data
	_init_buffers();
	_init_default_material();
}

RenderingSystem::~RenderingSystem() {
	_backend->device_wait();

	// Clean up resources
	_backend->buffer_free(_scene_buffer);
	_backend->buffer_free(_instance_buffer);
	_backend->buffer_free(_material_buffer);
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

	_update_material_buffer(registry);

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
	_backend->command_bind_graphics_pipeline(ctx.cmd, _pipeline->pipeline);

	std::unordered_map<std::shared_ptr<StaticMesh>, std::vector<InstanceData>> batches;

	for (Entity entity : registry.view<Transform, MeshComponent>()) {
		auto [transform, mc] = registry.get_many<Transform, MeshComponent>(entity);
		auto mesh = _resolve_mesh(mc->type);

		if (!mesh) {
			continue;
		}

		Mat4 model = transform->to_mat4();

		// Culling
		if (!mesh->aabb.transform(model).is_inside_frustum(ctx.frustum)) {
			continue;
		}

		// Resolve Material ID
		uint32_t mat_id = 0; // Default material index
		if (_entity_material_map.contains(entity)) {
			mat_id = _entity_material_map[entity];
		}

		// Push full struct
		InstanceData instance = {};
		instance.transform = model;
		instance.material_id = mat_id;

		batches[mesh].push_back(instance);
	}

	// Structure for command recording
	struct DrawCommand {
		std::shared_ptr<StaticMesh> mesh;
		uint32_t instance_count;
		uint32_t first_instance_index;
	};
	std::vector<DrawCommand> commands;

	size_t current_offset = 0;
	InstanceData* mapped_data = (InstanceData*)_backend->buffer_map(_instance_buffer).value();

	for (auto& [mesh, instances] : batches) {
		const uint32_t count = static_cast<uint32_t>(instances.size());

		memcpy(mapped_data + current_offset, instances.data(), count * sizeof(InstanceData));

		commands.push_back({ mesh, count, (uint32_t)current_offset });
		current_offset += count;
	}
	_backend->buffer_unmap(_instance_buffer);

	// Draw
	for (const auto& cmd : commands) {
		PushConstants pc = {};
		pc.vertex_buffer_addr = cmd.mesh->vertex_buffer_address;
		pc.scene_buffer_addr = _scene_buffer_addr;
		pc.instance_buffer_addr = _instance_buffer_addr;
		pc.material_buffer_addr = _material_buffer_addr;
		pc.base_instance_offset = cmd.first_instance_index;

		_backend->command_push_constants(ctx.cmd, _pipeline->shader, 0, sizeof(PushConstants), &pc);

		_backend->command_bind_index_buffer(ctx.cmd, cmd.mesh->index_buffer, 0, IndexType::UINT32);
		_backend->command_draw_indexed(ctx.cmd, cmd.mesh->index_count, cmd.instance_count);
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

void RenderingSystem::_init_buffers() {
	_scene_buffer =
			_backend->buffer_create(sizeof(SceneData),
							BUFFER_USAGE_STORAGE_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT |
									BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
							MemoryAllocationType::CPU)
					.value();
	_scene_buffer_addr = _backend->buffer_get_device_address(_scene_buffer).value();

	_instance_buffer = _backend->buffer_create(sizeof(InstanceData) * MAX_INSTANCES,
									   BUFFER_USAGE_STORAGE_BUFFER_BIT |
											   BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
									   MemoryAllocationType::CPU)
							   .value();
	_instance_buffer_addr = _backend->buffer_get_device_address(_instance_buffer).value();

	_material_buffer = _backend->buffer_create(sizeof(MaterialData) * MAX_INSTANCES,
									   BUFFER_USAGE_STORAGE_BUFFER_BIT |
											   BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
									   MemoryAllocationType::CPU)
							   .value();
	_material_buffer_addr = _backend->buffer_get_device_address(_material_buffer).value();
}

void RenderingSystem::_init_default_material() {
	_white_texture = Texture::create(_backend, COLOR_WHITE);
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

void RenderingSystem::_update_material_buffer(Registry& registry) {
	size_t offset = 0;

	_entity_material_map.clear(); // Reset map for this frame

	MaterialData* mapped_data = (MaterialData*)_backend->buffer_map(_material_buffer).value();

	for (Entity entity : registry.view<MaterialComponent>()) {
		MaterialComponent* mc = registry.get<MaterialComponent>(entity);

		MaterialData* data = mapped_data + offset;
		data->base_color = mc->base_color;

		// Save the index
		_entity_material_map.insert_or_assign(entity, offset);

		offset++;
	}
	_backend->buffer_unmap(_material_buffer);
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
