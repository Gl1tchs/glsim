#pragma once

#include "core/components.h"
#include "core/gpu_context.h"
#include "core/system.h"
#include "glgpu/backend.h"
#include "glgpu/matrix.h"
#include "glgpu/types.h"
#include "graphics/render_graph.h"
#include "graphics/render_pass.h"
#include "graphics/renderer.h"
#include "graphics/window.h"

#ifndef GL_HEADLESS
#include <SDL2/SDL.h>
#endif

namespace gl {

constexpr size_t MAX_INSTANCES = 1000;

class RenderingSystem : public System {
public:
	RenderingSystem(GpuContext& ctx, std::shared_ptr<Window> target);
	virtual ~RenderingSystem();

	void on_init(Registry& registry) override;
	void on_update(Registry& registry, float dt) override;
	void on_destroy(Registry& registry) override;

private:
	struct CameraData {
		Mat4 view;
		Mat4 proj;
		Vec3f pos;
		Frustum frustum;
	};

	CameraData _get_main_camera_data(Registry& registry, float aspect_ratio);

	RenderQueue _extract_render_queue(Registry& registry, const Frustum& frustum, Mat4 viewproj);

	std::shared_ptr<StaticMesh> _resolve_mesh(PrimitiveType type);

private:
	std::shared_ptr<RenderBackend> _backend;
	std::shared_ptr<Window> _window;
	std::unique_ptr<Renderer> _renderer;

	RenderGraph _render_graph;
	DataFormat _swapchain_format;

	std::shared_ptr<Texture> _white_texture;

	struct {
		std::shared_ptr<StaticMesh> cube;
		std::shared_ptr<StaticMesh> plane;
		std::shared_ptr<StaticMesh> sphere;
	} _primitives;
};

} //namespace gl
