#pragma once

#include "glgpu/backend.h"
#include "glgpu/types.h"
#include "graphics/render_pass.h"

namespace gl {

struct RenderContext {
	Vec2u backbuffer_size;
	DataFormat backbuffer_format;
};

struct RenderPassImageDef {
	DataFormat format = DataFormat::UNDEFINED;
	Vec2u size = Vec2u::zero();
};

struct RenderPassImageUsage {
	ImageUsageFlags usage;
	ImageLayout layout;
};

class RenderGraph {
public:
	RenderGraph(std::shared_ptr<RenderBackend> backend);
	~RenderGraph();

	template <typename T, typename... Args>
		requires std::is_base_of_v<IRenderPass, T>
	T& add_pass(Args&&... args) {
		auto pass = std::make_shared<T>(std::forward<Args>(args)...);
		_passes.push_back({ pass });
		return *pass;
	}

	/**
	 * Creates or retrieves virtual image
	 * NOTE: def must be defined if resource does not exist
	 */
	VImageHandle declare_image(const std::string& name, RenderPassImageDef def = {});

	VImageHandle import_image(const std::string& name, Image physical_handle);

	void set_render_target(VImageHandle handle);
	void set_sampled(VImageHandle handle);
	void set_storage_write(VImageHandle handle);

	void compile(const RenderContext& ctx);
	void execute(CommandBuffer cmd, const RenderQueue& queue);

	Image get_image(VImageHandle handle) const;

	bool transition_image(CommandBuffer cmd, VImageHandle handle, ImageLayout new_layout);

private:
	uint32_t _get_or_create_resource_id(const std::string& name);

private:
	struct ResourceNode {
		DataFormat format;
		Vec2u size;
		Image image_handle = GL_NULL_HANDLE;
		ImageLayout current_layout = ImageLayout::UNDEFINED;
		ImageUsageFlags current_usage = 0;

		bool is_external = false;
#ifdef GL_DEBUG_BUILD
		std::string debug_name;
#endif
	};

	struct PassNode {
		std::shared_ptr<IRenderPass> pass;
		std::unordered_map<VImageHandle, RenderPassImageUsage> image_infos;
	};

	std::unordered_map<uint32_t, ResourceNode> _resources;
	std::unordered_map<std::string, uint32_t> _resource_names;

	std::vector<PassNode> _passes;

	PassNode* _current_setup_pass = nullptr;
	RenderContext _last_ctx;

	std::shared_ptr<RenderBackend> _backend;
};

} //namespace gl
