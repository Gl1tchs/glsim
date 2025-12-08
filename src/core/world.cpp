#include "core/world.h"

#include "core/system.h"

namespace gl {

World::~World() { cleanup(); }

void World::cleanup() {
	for (auto& system : systems) {
		system->on_destroy(*this);
	}
}

void World::update(float p_dt) {
	for (auto& system : systems) {
		system->on_update(*this, p_dt);
	}
}

void World::add_system(std::shared_ptr<System> p_system) {
	p_system->on_init(*this);
	systems.push_back(p_system);
}

} //namespace gl
