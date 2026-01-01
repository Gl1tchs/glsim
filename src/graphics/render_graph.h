#pragma once

#include "glgpu/backend.h"
#include "glgpu/types.h"
#include "graphics/render_pass.h"

namespace gl {

typedef size_t VImageHandle;

struct RenderPassImageDef {
	DataFormat format = DataFormat::UNDEFINED;
	Vec2u size = Vec2u::zero();
};

class RenderGraph {
public:
	RenderGraph(std::shared_ptr<RenderBackend> backend);
	~RenderGraph();

	template <typename T, typename... Args>
		requires std::is_base_of_v<IRenderPass, T>
	void add_pass(Args&&... args) {
		auto pass = std::make_unique<T>(std::forward<Args>(args)...);
		_passes.push_back({ std::move(pass) });
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

	void compile(const Vec2u& size);
	void execute(CommandBuffer cmd, const RenderQueue& queue);

	Image get_image(VImageHandle handle) const;

	bool transition_image(CommandBuffer cmd, VImageHandle handle, ImageLayout new_layout);

private:
	size_t _get_or_create_resource_handle(const std::string& name);

private:
	struct ResourceNode {
		std::string name;

		DataFormat format = DataFormat::UNDEFINED;
		Vec2u size = Vec2u::zero();
		Image image_handle = GL_NULL_HANDLE;
		ImageLayout current_layout = ImageLayout::UNDEFINED;
		ImageUsageFlags current_usage = 0;

		bool is_external = false;
	};

	struct RenderPassImageUsage {
		ImageUsageFlags usage;
		ImageLayout layout;
	};

	struct PassNode {
		std::unique_ptr<IRenderPass> pass;
		std::unordered_map<VImageHandle, RenderPassImageUsage> image_infos;
	};

	std::vector<ResourceNode> _resources;
	std::vector<PassNode> _passes;

	PassNode* _current_setup_pass = nullptr;
	Vec2u _last_size = Vec2u::zero();

	std::shared_ptr<RenderBackend> _backend;
};

} //namespace gl
