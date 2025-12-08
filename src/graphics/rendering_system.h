#pragma once

#include "scene/system.h"

namespace gl {

class RenderingSystem : public System {
public:
	virtual ~RenderingSystem() = default;

	void on_init(Registry& p_registry) override;
	void on_update(Registry& p_registry, float p_dt) override;
	void on_destroy(Registry& p_registry) override;
};

} //namespace gl