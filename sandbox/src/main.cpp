#include "asset/asset_registry.h"
#include "core/components.h"
#include "core/event_system.h"
#include "core/gpu_context.h"
#include "core/input.h"
#include "core/timer.h"
#include "core/transform.h"
#include "core/world.h"
#include "glgpu/color.h"
#include "graphics/rendering_system.h"
#include "graphics/window.h"
#include "physics/physics_system.h"
#include "physics/rigidbody.h"

using namespace gl;

int main(int argc, char* argv[]) {
	GpuContext gpu;

	World world;

	auto window = std::make_shared<Window>(gpu, Vec2u{ 800, 600 }, "Glsim Sandbox");

	world.add_system(std::make_shared<RenderingSystem>(gpu, window));
	world.add_system(std::make_shared<PhysicsSystem>(gpu));

	Entity camera = world.spawn();
	{
		auto cam_transform = world.assign<Transform>(camera);
		cam_transform->position.z = 15;

		auto cc = world.assign<CameraComponent>(camera);
	}

	Entity player = world.spawn();

	auto player_rb = world.assign<Rigidbody>(player);
	player_rb->use_gravity = false;

	auto player_transform = world.assign<Transform>(player);
	player_transform->scale = Vec3f(0.75f);

	world.assign<MeshComponent>(player)->type = PrimitiveType::CUBE;

	auto player_mat = world.assign<MaterialComponent>(player);
	player_mat->base_color = COLOR_WHITE;
	player_mat->diffuse_tex_id = INVALID_ASSET_HANDLE;

	for (int i = 0; i < 20; i++) {
		Entity sphere = world.spawn();

		auto transform = world.assign<Transform>(sphere);
		transform->position =
				Vec3f((float)(i % 5) * 2.0f - 4.0f, ((float)i / 5.0f) * 2.0f - 4.0f, 0.0f);
		transform->scale = Vec3f(0.5f);

		auto mc = world.assign<MaterialComponent>(sphere);
		mc->base_color = COLOR_WHITE;

		auto mesh = world.assign<MeshComponent>(sphere);
		mesh->type = PrimitiveType::SPHERE;

		auto rb = world.assign<Rigidbody>(sphere);
		rb->use_gravity = false;
	}

	Timer timer;
	while (!window->should_close()) {
		timer.tick();

		window->poll_events();

		float force = 5.0f;
		if (Input::is_key_pressed(KeyCode::D)) {
			player_rb->add_force(Vec3f::right() * force);
		}
		if (Input::is_key_pressed(KeyCode::A)) {
			player_rb->add_force(-Vec3f::right() * force);
		}
		if (Input::is_key_pressed(KeyCode::W)) {
			player_rb->add_force(Vec3f::up() * force);
		}
		if (Input::is_key_pressed(KeyCode::S)) {
			player_rb->add_force(-Vec3f::up() * force);
		}

		float dt = timer.get_delta_time();

		if (Input::is_key_pressed(KeyCode::E)) {
			player_transform->rotate(30 * dt, Vec3f::up());
		}
		if (Input::is_key_pressed(KeyCode::Q)) {
			player_transform->rotate(-30 * dt, Vec3f::up());
		}

		if (Input::is_key_pressed(KeyCode::SPACE)) {
			player_rb->velocity = Vec3f::zero();
		}

		world.update(dt);
	}

	return 0;
}
