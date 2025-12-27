#include "core/registry.h"

namespace gl {

ComponentPool::ComponentPool(size_t p_element_size) : element_size(p_element_size) {}

ComponentPool::~ComponentPool() {}

ComponentPool::ComponentPool(const ComponentPool& p_other) : element_size(p_other.element_size) {
	for (const auto& page : p_other.pages) {
		if (!page) {
			continue;
		}

		auto new_page = std::make_unique<uint8_t[]>(PAGE_SIZE * element_size);
		std::memcpy(new_page.get(), page.get(), PAGE_SIZE * element_size);
		pages.push_back(std::move(new_page));
	}
}

size_t ComponentPool::get_size() const { return element_size; }

void* ComponentPool::get(size_t p_idx) {
	const size_t page_idx = p_idx / PAGE_SIZE;
	const size_t offset = p_idx % PAGE_SIZE;

	if (page_idx >= pages.size() || !pages[page_idx]) {
		return nullptr;
	}

	return pages[page_idx].get() + (offset * element_size);
}

Registry::~Registry() { clear(); }

void Registry::clear() {
	// Clear all data
	component_pools.clear();
	entities.clear();
	free_indices = {};
	entity_counter = 0;
}

void Registry::copy_to(Registry& p_dest) {
	p_dest.clear();

	// Copy trivial data
	p_dest.entity_counter = entity_counter;
	p_dest.free_indices = free_indices;
	p_dest.entities = entities; // This copies versions and component masks

	// Prepare destination pools
	p_dest.component_pools.resize(component_pools.size(), nullptr);

	// Iterate all pools and copy component data
	for (size_t comp_id = 0; comp_id < component_pools.size(); comp_id++) {
		if (component_pools[comp_id] == nullptr) {
			continue; // This component type isn't used
		}

		// Copy the components
		const auto old_pool = component_pools[comp_id];
		p_dest.component_pools[comp_id] = std::make_unique<ComponentPool>(*old_pool);
	}
}

Entity Registry::spawn() {
	if (!free_indices.empty()) {
		uint32_t new_idx = free_indices.front();
		free_indices.pop();

		Entity new_id = create_entity_id(new_idx, get_entity_version(entities[new_idx].id));

		entities[new_idx].id = new_id;

		return new_id;
	}

	entities.push_back({ create_entity_id(entities.size(), 0), ComponentMask() });

	return entities.back().id;
}

bool Registry::is_valid(Entity p_entity) {
	if (get_entity_index(p_entity) >= entities.size()) {
		return false;
	}

	return entities[get_entity_index(p_entity)].id == p_entity;
}

void Registry::despawn(Entity p_entity) {
	const uint32_t entity_idx = get_entity_index(p_entity);

	Entity new_entity_id = create_entity_id(UINT32_MAX, get_entity_version(p_entity) + 1);

	entities[entity_idx].id = new_entity_id;
	entities[entity_idx].mask.reset();

	free_indices.push(entity_idx);
}

bool Registry::assign_id(Entity p_entity, uint32_t p_component_id) {
	if (!is_valid(p_entity)) {
		return false;
	}

	entities[get_entity_index(p_entity)].mask.set(p_component_id);

	return true;
}

bool Registry::remove(Entity p_entity, uint32_t p_component_id) {
	if (!is_valid(p_entity)) {
		return false;
	}

	const uint32_t entity_idx = get_entity_index(p_entity);

	if (entities[entity_idx].mask.test(p_component_id)) {
		// TODO: destroy the component object
		entities[entity_idx].mask.reset(p_component_id);
	}

	return true;
}

bool Registry::has(Entity p_entity, uint32_t p_component_id) {
	if (!is_valid(p_entity)) {
		return false;
	}

	return entities[get_entity_index(p_entity)].mask.test(p_component_id);
}

} //namespace gl
