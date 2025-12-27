#pragma once

#include "glgpu/backend.h"

namespace gl {

class GpuContext {
public:
	GpuContext();
	~GpuContext() = default;

	std::shared_ptr<RenderBackend> get_backend();

private:
	std::shared_ptr<RenderBackend> _backend;
};

} //namespace gl
