#include "graphics/passes/geometry_pass.h"

#include "core/log.h"
#include "glgpu/color.h"
#include "glgpu/types.h"
#include "graphics/rendering_system.h"

namespace gl {

struct PushConstants {
	BufferDeviceAddress vertex_buffer_addr;
	BufferDeviceAddress scene_buffer_addr;
	BufferDeviceAddress instance_buffer_addr;
	BufferDeviceAddress material_buffer_addr;
	uint32_t base_instance_offset;
};

struct SceneData {
	Mat4 viewproj;
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

GeometryPass::GeometryPass(std::shared_ptr<RenderBackend> backend) : _backend(backend) {
	// Init pipeline
	const GraphicsPipelineCreateInfo create_info = {
        .color_attachments = { 
            DataFormat::R16G16B16A16_SFLOAT,    // g_position
            DataFormat::R16G16B16A16_SFLOAT,    // g_normal
            DataFormat::R8G8B8A8_UNORM,         // g_albedo
        },
        .depth_attachment = DataFormat::D32_SFLOAT, // g_depth
        .enable_depth_testing = true,
        .vertex_shader = "src/geometry/opaque_geometry.vert.spv",
        .fragment_shader = "src/geometry/opaque_geometry.frag.spv",
    };
	_pipeline = GraphicsPipeline::create(_backend, create_info);

	// Init scene buffer
	_scene_buffer =
			_backend->buffer_create(sizeof(SceneData),
							BUFFER_USAGE_STORAGE_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT |
									BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
							MemoryAllocationType::CPU)
					.value();
	_scene_buffer_addr = _backend->buffer_get_device_address(_scene_buffer).value();

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
}

GeometryPass::~GeometryPass() {
	_backend->buffer_free(_scene_buffer);
	_backend->buffer_free(_instance_buffer);
	_backend->buffer_free(_material_buffer);
	_backend->uniform_set_free(_bindless_textures);
}

void GeometryPass::setup(RenderGraph& graph) {
	_g_position = graph.declare_image("GBuffer_Position", { DataFormat::R16G16B16A16_SFLOAT });
	_g_normal = graph.declare_image("GBuffer_Normal", { DataFormat::R16G16B16A16_SFLOAT });
	_g_albedo = graph.declare_image("GBuffer_Albedo", { DataFormat::R8G8B8A8_UNORM });
	_g_depth = graph.declare_image("GBuffer_Depth", { DataFormat::D32_SFLOAT });

	// Define usage
	graph.set_render_target(_g_position);
	graph.set_render_target(_g_normal);
	graph.set_render_target(_g_albedo);
	graph.set_render_target(_g_depth);
}

void GeometryPass::execute(CommandBuffer cmd, RenderGraph& graph, const RenderQueue& queue) {
	// Upload materials & bindless descriptors
	{
		MaterialData* gpu_mats = (MaterialData*)_backend->buffer_map(_material_buffer).value();

		for (size_t i = 0; i < queue.materials.size(); ++i) {
			const auto& q_mat = queue.materials[i];

			// Write Buffer Data
			gpu_mats[i].base_color = q_mat.base_color;
			gpu_mats[i].diffuse_tex_id = (uint32_t)i + 1; // Slot index (0 is white)

			// Update Descriptor
			// TODO: In production, optimize this to only update changed slots or use global
			// bindless
			if (q_mat.albedo_map) {
				_backend->uniform_set_update_texture(_bindless_textures, 0, (uint32_t)i + 1,
						q_mat.albedo_map->get_image(), q_mat.albedo_map->get_sampler());
			}
		}
		_backend->buffer_unmap(_material_buffer);
	}

	// Upload Scene Data
	{
		SceneData* scene = (SceneData*)_backend->buffer_map(_scene_buffer).value();
		scene->viewproj = queue.viewproj;
		_backend->buffer_unmap(_scene_buffer);
	}

	struct DrawCmd {
		std::shared_ptr<StaticMesh> mesh;
		uint32_t instance_count;
		uint32_t first_instance;
	};
	std::vector<DrawCmd> draw_cmds;

	// Flatten Queue Batches into Linear GPU Buffer
	{
		InstanceData* gpu_instances = (InstanceData*)_backend->buffer_map(_instance_buffer).value();
		size_t current_offset = 0;

		for (const auto& batch : queue.opaque_batches) {
			const size_t count = batch.instances.size();

			// Safety check
			if (current_offset + count > MAX_INSTANCES) {
				GL_LOG_WARNING("GeometryPass: Max instances exceeded, dropping batch.");
				continue;
			}

			// Transform QueueInstance -> GPU InstanceData
			for (size_t i = 0; i < count; ++i) {
				gpu_instances[current_offset + i].transform = batch.instances[i].transform;
				gpu_instances[current_offset + i].material_id = batch.instances[i].material_index;
			}

			draw_cmds.push_back({ batch.mesh, (uint32_t)count, (uint32_t)current_offset });
			current_offset += count;
		}
		_backend->buffer_unmap(_instance_buffer);
	}

	// Retrieve G-Buffer resources
	Image g_position = graph.get_image(_g_position);
	Image g_normal = graph.get_image(_g_normal);
	Image g_albedo = graph.get_image(_g_albedo);
	Image g_depth = graph.get_image(_g_depth);

	if (!g_position || !g_normal || !g_albedo || !g_depth) {
		GL_LOG_ERROR("[GeometryPass::execute] Could not retrieve render resources.");
		return;
	}

	// Define Attachments
	RenderingAttachment pos_att = {};
	pos_att.image = g_position;
	pos_att.load_op = AttachmentLoadOp::CLEAR;
	pos_att.store_op = AttachmentStoreOp::STORE;
	pos_att.clear_color = COLOR_TRANSPARENT;

	RenderingAttachment norm_att = {};
	norm_att.image = g_normal;
	norm_att.load_op = AttachmentLoadOp::CLEAR;
	norm_att.store_op = AttachmentStoreOp::STORE;
	norm_att.clear_color = COLOR_TRANSPARENT;

	RenderingAttachment color_att = {};
	color_att.image = g_albedo;
	color_att.load_op = AttachmentLoadOp::CLEAR;
	color_att.store_op = AttachmentStoreOp::STORE;
	color_att.clear_color = COLOR_TRANSPARENT;

	Vec2u target_size = _backend->image_get_size(g_albedo).value();

	_backend->command_begin_rendering(cmd, target_size, { pos_att, norm_att, color_att }, g_depth);
	{
		_backend->command_bind_graphics_pipeline(cmd, _pipeline->pipeline);
		_backend->command_bind_uniform_sets(cmd, _pipeline->shader, 0, { _bindless_textures });

		// Draw Loop
		for (const auto& dc : draw_cmds) {
			PushConstants pc = {};
			pc.vertex_buffer_addr = dc.mesh->vertex_buffer_address;
			pc.scene_buffer_addr = _scene_buffer_addr;
			pc.instance_buffer_addr = _instance_buffer_addr;
			pc.material_buffer_addr = _material_buffer_addr;
			pc.base_instance_offset = dc.first_instance;

			_backend->command_push_constants(cmd, _pipeline->shader, 0, sizeof(PushConstants), &pc);

			_backend->command_bind_index_buffer(cmd, dc.mesh->index_buffer, 0, IndexType::UINT32);
			_backend->command_draw_indexed(cmd, dc.mesh->index_count, dc.instance_count);
		}
	}
	_backend->command_end_rendering(cmd);
}

} //namespace gl
