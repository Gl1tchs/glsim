#include "core/gpu_context.h"

#include "glgpu/backend.h"

namespace gl {

GpuContext::GpuContext() {
	RenderBackendCreateInfo create_info = {
		.api = RenderAPI::VULKAN,
		.required_features = RENDER_BACKEND_FEATURE_NONE,
	};

	backend = RenderBackend::create(create_info);
}

std::shared_ptr<RenderBackend> GpuContext::get_backend() { return backend; }

} //namespace gl
