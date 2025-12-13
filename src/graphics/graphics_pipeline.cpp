#include "graphics/graphics_pipeline.h"

#include "glgpu/types.h"

#include "shader_bundle.gen.h"

namespace gl {

// Get bundled spirv data
static std::vector<uint32_t> _get_spirv_data(const std::string& p_path) {
	BundleFileData shader_data = {};
	bool shader_found = false;

	for (int i = 0; i < BUNDLE_FILE_COUNT; i++) {
		BundleFileData data = BUNDLE_FILES[i];
		if (p_path == data.path) {
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

GraphicsPipeline::GraphicsPipeline(GpuContext& p_ctx, const GraphicsPipelineCreateInfo& p_info) :
		backend(p_ctx.get_backend()) {
	const std::vector<SpirvEntry> shader_entries = {
		{
				// Vertex shader
				.byte_code = _get_spirv_data(p_info.vertex_shader),
				.stage = SHADER_STAGE_VERTEX,
		},
		{
				// Fragment shader
				.byte_code = _get_spirv_data(p_info.fragment_shader),
				.stage = SHADER_STAGE_FRAGMENT,
		},
	};

	shader = backend->shader_create_from_bytecode(shader_entries);

	const PipelineVertexInputState vertex_input_state = {};
	const PipelineRasterizationState rasterization_state = {};
	const PipelineMultisampleState multisample_state = {};
	const PipelineDepthStencilState depth_stencil_state = {
		.enable_depth_test = p_info.enable_depth_testing,
		.enable_depth_write = p_info.enable_depth_write,
		.depth_compare_operator = CompareOperator::LESS,
		.enable_depth_range = true,
	};
	const PipelineColorBlendState color_blend_state = p_info.enable_blend
			? PipelineColorBlendState::create_blend(p_info.color_attachments.size())
			: PipelineColorBlendState::create_disabled(p_info.color_attachments.size());
	const PipelineRenderingState rendering_state = {
		.color_attachments = p_info.color_attachments,
		.depth_attachment = p_info.depth_attachment,
	};

	pipeline = backend->render_pipeline_create(shader, p_info.primitive, vertex_input_state,
			rasterization_state, multisample_state, depth_stencil_state, color_blend_state, 0,
			rendering_state);
}

GraphicsPipeline::~GraphicsPipeline() {
	backend->shader_free(shader);
	backend->pipeline_free(pipeline);
}

} //namespace gl
