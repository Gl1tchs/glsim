#pragma once

#include "core/registry.h"

namespace gl {

class System {
public:
	virtual ~System() = default;

	virtual void on_init(Registry& p_registry) {};
	virtual void on_update(Registry& p_registry, float p_dt) {};
	virtual void on_destroy(Registry& p_registry) {};
};

} //namespace gl
