#pragma once

#include "core/gpu_context.h"
#include "core/system.h"
#include "glgpu/backend.h"
#include "glgpu/matrix.h"
#include "glgpu/types.h"
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
	void _init_g_buffers(const Vec2u& size);

	Mat4 _get_camera_viewproj(Registry& registry, Image target_image);

	void _init_scene_resources();

	void _update_scene_resources(const Mat4& viewproj);

private:
	template <typename T>
		requires std::is_base_of_v<IRenderPass, T>
	void _add_render_pass() {
		_render_passes.push_back(std::make_unique<T>());
	}

private:
	std::shared_ptr<RenderBackend> _backend;
	std::shared_ptr<Window> _window;
	std::unique_ptr<Renderer> _renderer;

	Buffer _scene_buffer;
	BufferDeviceAddress _scene_buffer_addr;

	Image _g_position = GL_NULL_HANDLE;
	Image _g_normal = GL_NULL_HANDLE;
	Image _g_albedo = GL_NULL_HANDLE;
	Image _g_depth = GL_NULL_HANDLE;

	std::vector<std::unique_ptr<IRenderPass>> _render_passes;
};

} //namespace gl
