#pragma once

#include "core/system.h"

namespace gl {

class PhysicsSystem : public System {
public:
	virtual ~PhysicsSystem() = default;

	void on_init(Registry& p_registry) override;
	void on_update(Registry& p_registry, float p_dt) override;
	void on_destroy(Registry& p_registry) override;
};

} //namespace gl
