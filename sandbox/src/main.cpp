#include "core/components.h"
#include "core/event_system.h"
#include "core/gpu_context.h"
#include "core/input.h"
#include "core/transform.h"
#include "core/world.h"
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
		cam_transform->position.z = 5;

		auto cc = world.assign<CameraComponent>(camera);
	}

	auto entity = world.spawn();

	auto mesh = world.assign<MeshComponent>(entity);
	mesh->type = PrimitiveType::SPHERE;

	auto transform = world.assign<Transform>(entity);
	transform->scale = Vec3f(0.25f);

	auto rb = world.assign<Rigidbody>(entity);
	rb->use_gravity = false;

	while (!window->should_close()) {
		window->poll_events();

		if (Input::is_key_pressed(KeyCode::D)) {
			rb->add_force(Vec3f::right());
		}
		if (Input::is_key_pressed(KeyCode::A)) {
			rb->add_force(-Vec3f::right());
		}

		if (Input::is_key_pressed(KeyCode::W)) {
			rb->add_force(Vec3f::up());
		}
		if (Input::is_key_pressed(KeyCode::S)) {
			rb->add_force(-Vec3f::up());
		}

		world.update(1 / 144.0f);
	}

	return 0;
}
