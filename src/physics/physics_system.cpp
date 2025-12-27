#include "physics/physics_system.h"

#include "core/transform.h"
#include "physics/rigidbody.h"

namespace gl {

PhysicsSystem::PhysicsSystem(GpuContext& ctx) { _backend = ctx.get_backend(); }

void PhysicsSystem::on_init(Registry& registry) {}

void PhysicsSystem::on_destroy(Registry& registry) {}

void PhysicsSystem::on_update(Registry& registry, float dt) {
	// 60Hz
	constexpr float TIME_STEP = 1.0f / 60.0f;
	// One collision check in every 60 frames.
	constexpr int COLLISION_STEPS = 1;

	// Update physics engine
	_integration_phase(registry, TIME_STEP);
}

void PhysicsSystem::_integration_phase(Registry& registry, float ts) {
	for (Entity entity : registry.view<Transform, Rigidbody>()) {
		auto [transform, rb] = registry.get_many<Transform, Rigidbody>(entity);

		if (rb->is_static) {
			continue;
		}

		// TODO: proper precision
		Vec3f linear_acc = rb->force_acc / (rb->mass == 0.0f ? 0.0001f : rb->mass);
		if (rb->use_gravity) {
			linear_acc = linear_acc + Vec3f(0, -9.81f, 0);
		}

		// Update velocity
		rb->velocity += linear_acc * ts;
		rb->velocity *= std::pow(1.0f - rb->linear_damping, ts);

		// Update transform
		transform->position += rb->velocity * ts;

		// Clear accumulators
		rb->force_acc = Vec3f::zero();
	}
}

} //namespace gl
