#pragma once

#include "core/gpu_context.h"
#include "core/system.h"
#include "glgpu/backend.h"
#include "glgpu/color.h"
#include "glgpu/types.h"
#include "glgpu/vec.h"
#include "graphics/camera.h"
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
	virtual ~RenderingSystem() = default;

	void on_init(Registry& p_registry) override;
	void on_update(Registry& p_registry, float p_dt) override;
	void on_destroy(Registry& p_registry) override;

private:
	std::shared_ptr<RenderBackend> backend;
	std::shared_ptr<Window> window;
	std::unique_ptr<Renderer> renderer;

	// Scene data
	std::unique_ptr<GraphicsPipeline> pipeline; // unlit pipeline

	struct Vertex {
		Vec4f position;
	};

	struct SceneData {
		Mat4 viewproj;
	};

	struct PushConstants {
		Mat4 transform;
		BufferDeviceAddress vertex_buffer_addr;
		BufferDeviceAddress scene_buffer_addr;
	};

	struct MaterialData {
		Color base_color;
	};

	Buffer scene_buffer;
	BufferDeviceAddress scene_buffer_addr;

	Buffer material_buffer;
	UniformSet material_set;

	std::shared_ptr<StaticMesh> cube_mesh;
};

} //namespace gl
