#include "graphics/passes/geometry_pass.h"

#include "glgpu/color.h"
#include "glgpu/types.h"
#include "graphics/primitives.h"
#include "graphics/rendering_system.h"

namespace gl {

struct PushConstants {
	BufferDeviceAddress vertex_buffer_addr;
	BufferDeviceAddress scene_buffer_addr;
	BufferDeviceAddress instance_buffer_addr;
	BufferDeviceAddress material_buffer_addr;
	uint32_t base_instance_offset;
};

struct MaterialData {
	Color base_color;
	uint32_t diffuse_tex_id;
	uint32_t _padding[3];
};

struct InstanceData {
	Mat4 transform;
	uint32_t material_id;
	uint32_t padding[3];
};

GeometryPass::~GeometryPass() {
	_backend->buffer_free(_instance_buffer);
	_backend->buffer_free(_material_buffer);

	_backend->uniform_set_free(_bindless_textures);
}

void GeometryPass::init(std::shared_ptr<RenderBackend> backend, RenderPassResources& res) {
	_backend = backend;

	// Init pipeline
	const GraphicsPipelineCreateInfo create_info = {
		.color_attachments = { 
            // See RenderPassResources 
            DataFormat::R16G16B16A16_SFLOAT,    // g_position
            DataFormat::R16G16B16A16_SFLOAT,    // g_normal
            DataFormat::R8G8B8A8_UNORM,         // g_albedo
        },
        .depth_attachment = DataFormat::D32_SFLOAT, // g_depth
		.enable_depth_testing = true,
		// NOTE: memory data is being referenced
		.vertex_shader = "src/geometry/opaque_geometry.vert.spv",
		.fragment_shader = "src/geometry/opaque_geometry.frag.spv",
	};
	_pipeline = GraphicsPipeline::create(_backend, create_info);

	// Init primitives
	_primitives.cube = create_cube_mesh(_backend);
	_primitives.plane = create_plane_mesh(_backend);
	_primitives.sphere = create_sphere_mesh(_backend);

	// Init instance buffer
	_instance_buffer = _backend->buffer_create(sizeof(InstanceData) * MAX_INSTANCES,
									   BUFFER_USAGE_STORAGE_BUFFER_BIT |
											   BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
									   MemoryAllocationType::CPU)
							   .value();
	_instance_buffer_addr = _backend->buffer_get_device_address(_instance_buffer).value();

	// Init material buffer
	_material_buffer = _backend->buffer_create(sizeof(MaterialData) * MAX_INSTANCES,
									   BUFFER_USAGE_STORAGE_BUFFER_BIT |
											   BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
									   MemoryAllocationType::CPU)
							   .value();
	_material_buffer_addr = _backend->buffer_get_device_address(_material_buffer).value();

	// Allocate texture uniforms
	_bindless_textures =
			_backend->uniform_set_create_bindless(_pipeline->shader, 0, 0, MAX_INSTANCES).value();

	// Default texture
	_white_texture = Texture::create(_backend, COLOR_WHITE);
	_backend->uniform_set_update_texture(
			_bindless_textures, 0, 0, _white_texture->get_image(), _white_texture->get_sampler());
}

void GeometryPass::execute(const FrameContext& ctx, Registry& registry, RenderPassResources& res) {
	// Transition images
	_backend->command_transition_image(
			ctx.cmd, res.g_position, ImageLayout::UNDEFINED, ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	_backend->command_transition_image(
			ctx.cmd, res.g_normal, ImageLayout::UNDEFINED, ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	_backend->command_transition_image(
			ctx.cmd, res.g_albedo, ImageLayout::UNDEFINED, ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	_backend->command_transition_image(ctx.cmd, res.g_depth, ImageLayout::UNDEFINED,
			ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	// Upload materials/textures to the gpu
	_update_material_buffers(registry);

	// Define Attachments
	// Location 0: Position
	RenderingAttachment pos_att = {};
	pos_att.image = res.g_position;
	pos_att.load_op = AttachmentLoadOp::CLEAR;
	pos_att.store_op = AttachmentStoreOp::STORE;
	pos_att.clear_color = COLOR_TRANSPARENT;

	// Location 1: Normal
	RenderingAttachment norm_att = {};
	norm_att.image = res.g_normal;
	norm_att.load_op = AttachmentLoadOp::CLEAR;
	norm_att.store_op = AttachmentStoreOp::STORE;
	norm_att.clear_color = COLOR_TRANSPARENT;

	// Location 2: Albedo
	RenderingAttachment color_att = {};
	color_att.image = res.g_albedo;
	color_att.load_op = AttachmentLoadOp::CLEAR;
	color_att.store_op = AttachmentStoreOp::STORE;
	color_att.clear_color = COLOR_TRANSPARENT;

	Vec2u target_size = _backend->image_get_size(res.g_albedo).value();
	_backend->command_begin_rendering(
			ctx.cmd, target_size, { pos_att, norm_att, color_att }, res.g_depth);
	{
		_backend->command_bind_graphics_pipeline(ctx.cmd, _pipeline->pipeline);

		// Bind bindless texture uniform set
		_backend->command_bind_uniform_sets(ctx.cmd, _pipeline->shader, 0, { _bindless_textures });

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
			pc.scene_buffer_addr = res.scene_buffer_addr;
			pc.instance_buffer_addr = _instance_buffer_addr;
			pc.material_buffer_addr = _material_buffer_addr;
			pc.base_instance_offset = cmd.first_instance_index;

			_backend->command_push_constants(
					ctx.cmd, _pipeline->shader, 0, sizeof(PushConstants), &pc);

			_backend->command_bind_index_buffer(
					ctx.cmd, cmd.mesh->index_buffer, 0, IndexType::UINT32);
			_backend->command_draw_indexed(ctx.cmd, cmd.mesh->index_count, cmd.instance_count);
		}
	}
	_backend->command_end_rendering(ctx.cmd);
}

void GeometryPass::_update_material_buffers(Registry& registry) {
	size_t offset = 0;

	_entity_material_map.clear(); // Reset map for this frame

	MaterialData* mapped_data = (MaterialData*)_backend->buffer_map(_material_buffer).value();

	for (Entity entity : registry.view<MaterialComponent>()) {
		MaterialComponent* mc = registry.get<MaterialComponent>(entity);

		MaterialData* data = mapped_data + offset;
		data->base_color = mc->base_color;

		// TODO: cache the texture, it might use the same instance

		// Upload the texture
		uint32_t tex_id = 0; // 0 is default white texture
		if (auto texture = registry.get_asset_manager().get<Texture>(mc->diffuse_tex_id)) {
			tex_id = offset + 1;
			_backend->uniform_set_update_texture(
					_bindless_textures, 0, tex_id, texture->get_image(), texture->get_sampler());
		}

		data->diffuse_tex_id = tex_id;

		// Save the index
		_entity_material_map.insert_or_assign(entity, offset);

		offset++;
	}
	_backend->buffer_unmap(_material_buffer);
}

std::shared_ptr<StaticMesh> GeometryPass::_resolve_mesh(PrimitiveType type) {
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

} //namespace gl
