#pragma once

#include "core/components.h"
#include "graphics/graphics_pipeline.h"
#include "graphics/mesh.h"
#include "graphics/render_pass.h"
#include "graphics/texture.h"

namespace gl {

class GeometryPass : public IRenderPass {
public:
	virtual ~GeometryPass();

	void init(std::shared_ptr<RenderBackend> backend) override;
	void execute(
			const FrameContext& ctx, Registry& registry, RenderPassResources& resources) override;

private:
	void _update_material_buffers(Registry& registry);

	std::shared_ptr<StaticMesh> _resolve_mesh(PrimitiveType type);

private:
	std::shared_ptr<RenderBackend> _backend;
	std::shared_ptr<GraphicsPipeline> _pipeline; // unlit pipeline

	Buffer _instance_buffer;
	BufferDeviceAddress _instance_buffer_addr;

	Buffer _material_buffer;
	BufferDeviceAddress _material_buffer_addr;

	UniformSet _bindless_textures;

	std::unordered_map<Entity, size_t> _entity_material_map;

	std::shared_ptr<Texture> _white_texture;

	struct {
		std::shared_ptr<StaticMesh> cube;
		std::shared_ptr<StaticMesh> plane;
		std::shared_ptr<StaticMesh> sphere;
	} _primitives;
};

} //namespace gl
