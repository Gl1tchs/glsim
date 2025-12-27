#pragma once

#include "core/gpu_context.h"
#include "core/system.h"

namespace gl {

class PhysicsSystem : public System {
public:
	PhysicsSystem(GpuContext& ctx);
	virtual ~PhysicsSystem() = default;

	void on_init(Registry& registry) override;
	void on_update(Registry& registry, float dt) override;
	void on_destroy(Registry& registry) override;

private:
	void _integration_phase(Registry& registry, float ts);

private:
	std::shared_ptr<RenderBackend> _backend;
};

} //namespace gl
