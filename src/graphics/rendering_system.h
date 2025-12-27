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
	RenderingSystem(GpuContext& ctx, std::shared_ptr<Window> target);
	virtual ~RenderingSystem();

	void on_init(Registry& registry) override;
	void on_update(Registry& registry, float dt) override;
	void on_destroy(Registry& registry) override;

private:
	// Render Passes

	struct FrameContext {
		CommandBuffer cmd;
		Image target_image;
		float dt;
		Frustum frustum;
	};

	void _execute_geometry_pass(const FrameContext& ctx, Registry& registry);

private:
	// Initialization helpers

	void _init_pipelines();

	void _init_primitives();

	void _init_scene_buffer();

	void _init_material_buffer();

	// Update helpers

	Mat4 _get_camera_viewproj(Registry& registry, Image target_image);

	void _update_scene_uniforms(const Mat4& viewproj);

	void _update_material_uniforms();

	RenderingAttachment _create_color_attachment(Image target);

	std::shared_ptr<StaticMesh> _resolve_mesh(PrimitiveType type);

private:
	std::shared_ptr<RenderBackend> _backend;
	std::shared_ptr<Window> _window;
	std::unique_ptr<Renderer> _renderer;

	// Scene data
	std::shared_ptr<GraphicsPipeline> _pipeline; // unlit pipeline

	Buffer _scene_buffer;
	BufferDeviceAddress _scene_buffer_addr;

	Buffer _material_buffer;
	UniformSet _material_set;

	struct {
		std::shared_ptr<StaticMesh> cube;
		std::shared_ptr<StaticMesh> plane;
		std::shared_ptr<StaticMesh> sphere;
	} _primitives;
};

} //namespace gl
