#include "physics/physics_system.h"

#include "physics/rigidbody.h"

namespace gl {

void PhysicsSystem::update(Registry& registry, float dt) {
	for (auto entity : registry.view<Position, Velocity>()) {
		auto [pos, vel] = registry.get_many<Position, Velocity>(entity);

		pos->x += vel->x * dt;
		pos->y += vel->y * dt;
		pos->z += vel->z * dt;
	}
}

} //namespace gl
