#include "physics/physics_system.h"

#include "core/transform.h"
#include "physics/rigidbody.h"

namespace gl {

PhysicsSystem::PhysicsSystem(GpuContext& p_ctx) { backend = p_ctx.get_backend(); }

void PhysicsSystem::on_init(Registry& p_registry) {}

void PhysicsSystem::on_destroy(Registry& p_registry) {}

void PhysicsSystem::on_update(Registry& p_registry, float p_dt) {
	// 60Hz
	constexpr float TIME_STEP = 1.0f / 60.0f;
	// One collision check in every 60 frames.
	constexpr int COLLISION_STEPS = 1;

	// Update physics engine
	_integration_phase(p_registry, TIME_STEP);
}

void PhysicsSystem::_integration_phase(Registry& p_registry, float p_ts) {
	for (Entity entity : p_registry.view<Transform, Rigidbody>()) {
		auto [transform, rb] = p_registry.get_many<Transform, Rigidbody>(entity);

		if (rb->is_static) {
			continue;
		}

		// TODO: proper precision
		Vec3f linear_acc = rb->force_acc / (rb->mass == 0.0f ? 0.0001f : rb->mass);
		if (rb->use_gravity) {
			linear_acc = linear_acc + Vec3f(0, -9.81f, 0);
		}

		// Update velocity
		rb->velocity += linear_acc * p_ts;
		rb->velocity *= std::pow(1.0f - rb->linear_damping, p_ts);

		// Update transform
		transform->position += rb->velocity * p_ts;

		// Clear accumulators
		rb->force_acc = Vec3f::zero();
	}
}

} //namespace gl
