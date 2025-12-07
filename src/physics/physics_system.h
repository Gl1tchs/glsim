#pragma once

#include "scene/registry.h"

namespace gl {

class PhysicsSystem {
public:
	static void update(Registry& registry, float dt);
};

} //namespace gl
