#include "core/world.h"

#include "core/system.h"

namespace gl {

World::~World() { cleanup(); }

void World::cleanup() {
	for (auto& system : _systems) {
		system->on_destroy(*this);
	}
}

void World::update(float dt) {
	for (auto& system : _systems) {
		system->on_update(*this, dt);
	}
}

void World::add_system(std::shared_ptr<System> system) {
	system->on_init(*this);
	_systems.push_back(system);
}

} //namespace gl
