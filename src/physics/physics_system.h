#pragma once

#include "core/gpu_context.h"
#include "core/system.h"

namespace gl {

class PhysicsSystem : public System {
public:
	PhysicsSystem(GpuContext& p_ctx);
	virtual ~PhysicsSystem() = default;

	void on_init(Registry& p_registry) override;
	void on_update(Registry& p_registry, float p_dt) override;
	void on_destroy(Registry& p_registry) override;

private:
	std::shared_ptr<RenderBackend> backend;
};

} //namespace gl
