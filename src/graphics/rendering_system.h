#pragma once

#include "core/components.h"
#include "core/gpu_context.h"
#include "core/system.h"
#include "glgpu/backend.h"
#include "glgpu/matrix.h"
#include "glgpu/types.h"
#include "graphics/graphics_pipeline.h"
#include "graphics/mesh.h"
#include "graphics/renderer.h"
#include "graphics/window.h"

#ifndef GL_HEADLESS
#include <SDL2/SDL.h>
#endif

namespace gl {

class RenderingSystem : public System {
public:
	RenderingSystem(GpuContext& p_ctx, std::shared_ptr<Window> p_target);
	virtual ~RenderingSystem();

	void on_init(Registry& p_registry) override;
	void on_update(Registry& p_registry, float p_dt) override;
	void on_destroy(Registry& p_registry) override;

private:
	// Render Passes

	struct FrameContext {
		CommandBuffer cmd;
		Image target_image;
		float dt;
		Frustum frustum;
	};

	void _execute_geometry_pass(const FrameContext& p_ctx, Registry& p_registry);

private:
	// Initialization helpers

	void _init_pipelines(GpuContext& p_ctx);

	void _init_primitives();

	void _init_scene_buffer();

	void _init_material_buffer();

	// Update helpers

	Mat4 _get_camera_viewproj(Registry& p_registry, Image p_target_image);

	void _update_scene_uniforms(const Mat4& p_viewproj);

	void _update_material_uniforms();

	RenderingAttachment _create_color_attachment(Image p_target);

	std::shared_ptr<StaticMesh> _resolve_mesh(PrimitiveType p_type);

private:
	std::shared_ptr<RenderBackend> backend;
	std::shared_ptr<Window> window;
	std::unique_ptr<Renderer> renderer;

	// Scene data
	std::unique_ptr<GraphicsPipeline> pipeline; // unlit pipeline

	Buffer scene_buffer;
	BufferDeviceAddress scene_buffer_addr;

	Buffer material_buffer;
	UniformSet material_set;

	struct {
		std::shared_ptr<StaticMesh> cube;
		std::shared_ptr<StaticMesh> plane;
		std::shared_ptr<StaticMesh> sphere;
	} primitives;
};

} //namespace gl
