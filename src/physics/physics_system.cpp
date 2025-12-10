#include "physics/physics_system.h"

#include "core/log.h"

namespace gl {

PhysicsSystem::PhysicsSystem(GpuContext& p_ctx) { backend = p_ctx.get_backend(); }

void PhysicsSystem::on_init(Registry& p_registry) { GL_LOG_TRACE("PhysicsSystem::on_init"); }

void PhysicsSystem::on_destroy(Registry& p_registry) { GL_LOG_TRACE("PhysicsSystem::on_update"); }

void PhysicsSystem::on_update(Registry& p_registry, float p_dt) {
	GL_LOG_TRACE("PhysicsSystem::on_update");

	// Simulate stage
	{
		// 60Hz
		constexpr float TIME_STEP = 1.0f / 60.0f;
		// One collision check in every 60 frames.
		constexpr int COLLISION_STEPS = 1;

		// Update physics engine
	}
}

} //namespace gl
