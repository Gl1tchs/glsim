#include "graphics/render_graph.h"

#include "core/log.h"
#include "glgpu/types.h"
#include "graphics/render_pass.h"

namespace gl {

RenderGraph::RenderGraph(std::shared_ptr<RenderBackend> backend) : _backend(backend) {}

RenderGraph::~RenderGraph() {
	for (auto& [handle, node] : _resources) {
		if (node.is_external || node.image_handle == GL_NULL_HANDLE) {
			continue;
		}

		_backend->image_free(node.image_handle);
		node.image_handle = GL_NULL_HANDLE;
	}
}

VImageHandle RenderGraph::declare_image(const std::string& name, RenderPassImageDef def) {
	uint32_t id = _get_or_create_resource_id(name);

	ResourceNode& node = _resources[id];

	// Resource was already declared by another pass
	if (node.format != DataFormat::UNDEFINED || node.is_external) {
		return { id };
	}

	node.format = def.format;
	node.size = def.size;
#ifdef GL_DEBUG_BUILD
	node.debug_name = name;
#endif

	return { id };
}

VImageHandle RenderGraph::import_image(const std::string& name, Image physical_handle) {
	uint32_t id = _get_or_create_resource_id(name);

	ResourceNode& node = _resources[id];
	node.format = _backend->image_get_format(physical_handle).value();
	node.size = _backend->image_get_size(physical_handle).value();
	node.current_usage = _backend->image_get_image_usage(physical_handle).value();
	node.image_handle = physical_handle;
	node.is_external = true;
#ifdef GL_DEBUG_BUILD
	node.debug_name = name;
#endif

	return { id };
}

void RenderGraph::set_render_target(VImageHandle handle) {
	if (!_current_setup_pass) {
		return;
	}

	const auto it = _resources.find(handle.id);
	if (it == _resources.end()) {
		return;
	}

	const ResourceNode& node = it->second;

	auto& info = _current_setup_pass->image_infos[handle];
	info.usage = IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (is_depth_format(node.format)) {
		info.usage |= IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		info.layout = ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	} else {
		info.usage |= IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	}
}

void RenderGraph::set_sampled(VImageHandle handle) {
	if (!_current_setup_pass) {
		return;
	}

	const auto it = _resources.find(handle.id);
	if (it == _resources.end()) {
		return;
	}

	const ResourceNode& node = it->second;

	auto& info = _current_setup_pass->image_infos[handle];
	info.usage = IMAGE_USAGE_SAMPLED_BIT;
	if (is_depth_format(node.format)) {
		info.layout = ImageLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	} else {
		info.layout = ImageLayout::SHADER_READ_ONLY_OPTIMAL;
	}
}

void RenderGraph::set_storage_write(VImageHandle handle) {
	if (!_current_setup_pass) {
		return;
	}

	const auto it = _resources.find(handle.id);
	if (it == _resources.end()) {
		return;
	}

	auto& info = _current_setup_pass->image_infos[handle];
	info.layout = ImageLayout::GENERAL;
	info.usage = IMAGE_USAGE_STORAGE_BIT | IMAGE_USAGE_SAMPLED_BIT;
}

void RenderGraph::compile(const RenderContext& ctx) {
	const bool target_resized = (ctx.backbuffer_size.x != _last_ctx.backbuffer_size.x ||
			ctx.backbuffer_size.y != _last_ctx.backbuffer_size.y);
	_last_ctx = ctx;

	// Setup phase
	for (auto& pass_node : _passes) {
		pass_node.image_infos.clear();
		_current_setup_pass = &pass_node;
		pass_node.pass->setup(*this);
		_current_setup_pass = nullptr;
	}

	// Accumulate usage flags
	std::unordered_map<uint32_t, ImageUsageFlags> global_usage;
	for (const auto& pass_node : _passes) {
		for (const auto& [handle, info] : pass_node.image_infos) {
			global_usage[handle.id] |= info.usage;
		}
	}

	// Lazy resource phase
	for (auto& [id, node] : _resources) {
		if (node.is_external)
			continue;

		// info.size == 0: screen relative, else fixed
		Vec2u intended_size = (node.size == Vec2u::zero()) ? ctx.backbuffer_size : node.size;
		ImageUsageFlags needed_usage = global_usage[id];

		// Check for resize or usage change
		bool needs_recreate = false;

		if (node.image_handle != GL_NULL_HANDLE) {
			// size changed
			if (target_resized && node.size == Vec2u::zero())
				needs_recreate = true;
			// usage changed
			if (node.current_usage != needed_usage)
				needs_recreate = true;
		}

		if (needs_recreate && node.image_handle != GL_NULL_HANDLE) {
			_backend->image_free(node.image_handle);
			node.image_handle = GL_NULL_HANDLE;
			node.current_layout = ImageLayout::UNDEFINED;
		}

		// Check if we need to create
		if (node.image_handle != GL_NULL_HANDLE) {
			continue;
		}

		// Check if this resource is actually used this frame
		if (needed_usage == 0) {
			continue; // not used this frame
		}

		ImageCreateInfo create_info = {};
		create_info.format = node.format;
		create_info.size = intended_size;
		create_info.usage = needed_usage;
		create_info.mipmapped = false;
		create_info.samples = 1;

		const auto res = _backend->image_create(create_info);
		if (res.is_ok()) {
			node.image_handle = res.value();
			node.current_usage = needed_usage;
			node.current_layout = ImageLayout::UNDEFINED;
		} else {
			GL_LOG_ERROR("Failed to create RG image");
		}
	}

	if (target_resized) {
		// Invoke resize events on render passes
		for (const auto& pass_node : _passes) {
			pass_node.pass->on_resize(*this, ctx.backbuffer_size);
		}
	}
}

void RenderGraph::execute(CommandBuffer cmd, const RenderQueue& queue) {
	for (const auto& pass_node : _passes) {
		// Pipeline barriers
		for (const auto& [handle, info] : pass_node.image_infos) {
			transition_image(cmd, handle, info.layout);
		}

		// Execute render pass
		pass_node.pass->execute(cmd, *this, queue);
	}
}

Image RenderGraph::get_image(VImageHandle handle) const {
	const auto it = _resources.find(handle.id);
	if (it == _resources.end()) {
		return GL_NULL_HANDLE;
	}

	return it->second.image_handle;
}

bool RenderGraph::transition_image(CommandBuffer cmd, VImageHandle handle, ImageLayout new_layout) {
	auto it = _resources.find(handle.id);
	if (it == _resources.end()) {
		return false;
	}

	ResourceNode& node = it->second;
	if (node.image_handle == GL_NULL_HANDLE) {
		return false;
	}

	// Transition image layout and save it
	const Res<> result = _backend->command_transition_image(
			cmd, node.image_handle, node.current_layout, new_layout);
	if (result.is_ok()) {
		node.current_layout = new_layout;
	}

	return (bool)result;
}

uint32_t RenderGraph::_get_or_create_resource_id(const std::string& name) {
	auto it = _resource_names.find(name);
	if (it != _resource_names.end()) {
		return it->second;
	}

	static uint32_t s_id_counter = 1;

	const uint32_t id = s_id_counter++;
	_resource_names[name] = id;

	return id;
}

} //namespace gl
