#pragma once

#include "scene/registry.h"
#include "scene/system.h"

namespace gl {

class World : public Registry {
public:
	virtual ~World();

	void cleanup();

	void update(float p_dt);

	void add_system(std::shared_ptr<System> p_system);

private:
	std::vector<std::shared_ptr<System>> systems;
};

} //namespace gl
