#pragma once

#include "core/registry.h"

namespace gl {

class System {
public:
	virtual ~System() = default;

	virtual void on_init(Registry& registry) {};
	virtual void on_update(Registry& registry, float dt) {};
	virtual void on_destroy(Registry& registry) {};
};

} //namespace gl
