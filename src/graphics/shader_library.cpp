#include "graphics/shader_library.h"

#include "shader_bundle.gen.h"

namespace gl::shader_library {

std::vector<uint32_t> get_spirv_data(const std::string& path) {
	BundleFileData shader_data = {};
	bool shader_found = false;

	for (int i = 0; i < BUNDLE_FILE_COUNT; i++) {
		BundleFileData data = BUNDLE_FILES[i];
		if (path == data.path) {
			shader_data = data;
			shader_found = true;
			break;
		}
	}

	if (!shader_found) {
		return {};
	}

	const uint32_t* bundle_data = (uint32_t*)&BUNDLE_DATA[shader_data.start_idx];

	size_t word_count = shader_data.size / sizeof(uint32_t);

	return std::vector<uint32_t>(bundle_data, bundle_data + word_count);
}

} //namespace gl::shader_library
