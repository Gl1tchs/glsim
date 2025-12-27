#include "graphics/graphics_pipeline.h"

#include "shader_bundle.gen.h"

namespace gl {

// Get bundled spirv data
static std::vector<uint32_t> _get_spirv_data(const std::string& path) {
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

	return std::vector<uint32_t>(bundle_data, bundle_data + shader_data.size);
}

std::shared_ptr<GraphicsPipeline> GraphicsPipeline::create(
		std::shared_ptr<RenderBackend> backend, const GraphicsPipelineCreateInfo& info) {
	const std::vector<SpirvEntry> shader_entries = {
		{ // Vertex shader
				.byte_code = _get_spirv_data(info.vertex_shader),
				.stage = SHADER_STAGE_VERTEX_BIT },
		{ // Fragment shader
				.byte_code = _get_spirv_data(info.fragment_shader),
				.stage = SHADER_STAGE_FRAGMENT_BIT },
	};

	auto shader_res = backend->shader_create_from_bytecode(shader_entries);
	if (shader_res.is_error()) {
		return nullptr;
	}

	const RenderPipelineCreateInfo create_info = {
        .shader = shader_res.value(),
        .primitive = info.primitive,
        .vertex_input_state = {},
        .rasterization_state = {
            .wireframe = info.wireframe,
        },
        .multisample_state = {},
        .depth_stencil_state = {
            .enable_depth_test = info.enable_depth_testing,
            .enable_depth_write = info.enable_depth_write,
            .depth_compare_operator = CompareOperator::LESS,
            .enable_depth_range = true,
        },
        .color_blend_state = info.enable_blend
			? PipelineColorBlendState::create_blend(info.color_attachments.size())
			: PipelineColorBlendState::create_disabled(info.color_attachments.size()),
        .dynamic_state= 0,
        .render_pass = GL_NULL_HANDLE,
        .rendering_info = {
            .color_attachments = info.color_attachments,
            .depth_attachment = info.depth_attachment,
        },
    };

	auto pipeline_res = backend->render_pipeline_create(create_info);
	if (pipeline_res.is_error()) {
		return nullptr;
	}

	std::shared_ptr<GraphicsPipeline> gp = std::make_shared<GraphicsPipeline>();
	gp->_backend = backend;
	gp->pipeline = pipeline_res.value();
	gp->shader = shader_res.value();

	return gp;
}

GraphicsPipeline::~GraphicsPipeline() {
	_backend->shader_free(shader);
	_backend->pipeline_free(pipeline);
}

} //namespace gl
