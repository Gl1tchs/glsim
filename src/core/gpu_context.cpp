#include "core/gpu_context.h"

#include "glgpu/backend.h"

namespace gl {

GpuContext::GpuContext() {
	RenderBackendCreateInfo create_info = {
		.api = RenderAPI::VULKAN,
		.required_features = RENDER_BACKEND_FEATURE_NONE,
	};

	_backend = RenderBackend::create(create_info).value(); // must be valid
}

std::shared_ptr<RenderBackend> GpuContext::get_backend() { return _backend; }

} //namespace gl
