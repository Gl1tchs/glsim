#pragma once

#include "scene/registry.h"

namespace gl {

class PhysicsSystem {
public:
	static void init();
	static void shutdown();

	static void update(Registry& registry);
};

} //namespace gl
