#include "graphics/rendering_system.h"

#include "core/components.h"
#include "core/event_system.h"
#include "core/gpu_context.h"
#include "core/registry.h"
#include "core/transform.h"
#include "glgpu/types.h"
#include "graphics/aabb.h"
#include "graphics/camera.h"
#include "graphics/primitives.h"
#include "graphics/render_pass.h"
#include "graphics/renderer.h"

#include "graphics/passes/geometry_pass.h"
#include "graphics/passes/lighting_pass.h"
#include "graphics/passes/ssao_pass.h"

namespace gl {

RenderingSystem::RenderingSystem(GpuContext& ctx, std::shared_ptr<Window> window) :
		_backend(ctx.get_backend()),
		_window(window),
		_renderer(std::make_unique<Renderer>(_backend)),
		_render_graph(_backend) {
	// Create primitives
	_primitives.cube = create_cube_mesh(_backend);
	_primitives.plane = create_plane_mesh(_backend);
	_primitives.sphere = create_sphere_mesh(_backend);

	_white_texture = Texture::create(_backend, COLOR_WHITE);

	// Add render passes
	_render_graph.add_pass<GeometryPass>(_backend);
	_render_graph.add_pass<SSAOPass>(_backend);
	_render_graph.add_pass<LightingPass>(_backend, _window->get_swapchain_format());
}

RenderingSystem::~RenderingSystem() { _backend->device_wait(); }

void RenderingSystem::on_init(Registry& registry) {
	event::subscribe<WindowResizeEvent>(
			[&](const WindowResizeEvent& e) { _window->on_resize(e.size); });
}

void RenderingSystem::on_destroy(Registry& registry) {}

void RenderingSystem::on_update(Registry& registry, float dt) {
	// Minimized
	if (_window->get_size().x == 0 || _window->get_size().y == 0) {
		return;
	}

	_renderer->wait_for_frame();

	Semaphore wait_sem = _renderer->get_wait_sem();
	Semaphore signal_sem = _renderer->get_signal_sem();

	Image backbuffer = _window->get_target(wait_sem);
	if (!backbuffer) {
		return;
	}

	//  Resolve Camera
	const Vec2u size = _window->get_size();
	const auto [cam_view, cam_proj, cam_pos, cam_frustum] =
			_get_main_camera_data(registry, (float)size.x / size.y);

	// Construct render queue
	RenderQueue queue = _extract_render_queue(registry, cam_frustum, cam_proj * cam_view);
	queue.camera_pos = cam_pos;

	// Compile and execute render graph
	VHandle backbuffer_handle = _render_graph.import_image("Backbuffer", backbuffer);

	RenderContext ctx = {};
	ctx.backbuffer_size = _window->get_size();
	ctx.backbuffer_format = _swapchain_format;

	_render_graph.compile(ctx);

	CommandBuffer cmd = _renderer->begin_frame(backbuffer);
	{
		_render_graph.execute(cmd, queue);

		// Transition backbuffer for presentation
		_render_graph.transition_image(cmd, backbuffer_handle, ImageLayout::PRESENT_SRC);
	}
	_renderer->end_frame();

	_window->present(signal_sem);
}

RenderingSystem::CameraData RenderingSystem::_get_main_camera_data(
		Registry& registry, float aspect_ratio) {
	CameraData cam_data = {};
	for (Entity entity : registry.view<Transform, CameraComponent>()) {
		auto [transform, cc] = registry.get_many<Transform, CameraComponent>(entity);

		if (!cc->enabled) {
			continue;
		}

		cam_data.pos = transform->position;

		switch (cc->projection) {
			case CameraProjection::ORTHOGRAPHIC:
				cc->ortho.aspect_ratio = aspect_ratio;
				cam_data.proj = cc->ortho.get_projection_matrix();
				cam_data.view = cc->ortho.get_view_matrix(*transform);
				break;
			case CameraProjection::PERSPECTIVE:
				cc->persp.aspect_ratio = aspect_ratio;
				cam_data.proj = cc->persp.get_projection_matrix();
				cam_data.view = cc->persp.get_view_matrix(*transform);
				break;
		}

		break;
	}

	cam_data.frustum = Frustum::from_view_proj(cam_data.proj * cam_data.view);

	return cam_data;
}

RenderQueue RenderingSystem::_extract_render_queue(
		Registry& registry, const Frustum& frustum, Mat4 viewproj) {
	RenderQueue queue;
	queue.viewproj = viewproj;

	// Temporary map to batch by Mesh
	std::unordered_map<std::shared_ptr<StaticMesh>, std::vector<QueueInstance>> batch_map;

	for (auto entity : registry.view<Transform, MeshComponent>()) {
		auto [transform, mesh_comp] = registry.get_many<Transform, MeshComponent>(entity);

		// Resolve Mesh
		std::shared_ptr<StaticMesh> mesh = _resolve_mesh(mesh_comp->type);
		if (!mesh) {
			continue;
		}

		// Culling
		const Mat4 model = transform->to_mat4();
		if (!mesh->aabb.transform(model).is_inside_frustum(frustum)) {
			continue;
		}

		// Material Packing
		// We add the material to the linear list and get its index
		const uint32_t mat_index = (uint32_t)queue.materials.size();

		{
			QueueMaterial q_mat = {};

			// Resolve material component if any
			if (MaterialComponent* mat_comp = registry.get<MaterialComponent>(entity)) {
				q_mat.base_color = mat_comp->base_color;

				// Resolve texture asset
				if (auto mat_texture = registry.get_asset_manager().get<Texture>(
							mat_comp->diffuse_tex_id)) {
					q_mat.albedo_map = mat_texture;
				}
			}

			if (!q_mat.albedo_map) {
				q_mat.albedo_map = _white_texture;
			}

			queue.materials.push_back(q_mat);
		}

		// Add Instance
		batch_map[mesh].push_back({ model, mat_index });
	}

	// Flatten Map to Vector
	for (auto& [mesh, instances] : batch_map) {
		queue.opaque_batches.push_back({ mesh, std::move(instances) });
	}

	return queue;
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
