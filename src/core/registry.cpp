#include "core/registry.h"

namespace gl {

ComponentPool::ComponentPool(size_t element_size) : _element_size(element_size) {}

ComponentPool::~ComponentPool() {}

ComponentPool::ComponentPool(const ComponentPool& other) : _element_size(other._element_size) {
	for (const auto& page : other._pages) {
		if (!page) {
			continue;
		}

		auto new_page = std::make_unique<uint8_t[]>(PAGE_SIZE * _element_size);
		std::memcpy(new_page.get(), page.get(), PAGE_SIZE * _element_size);
		_pages.push_back(std::move(new_page));
	}
}

size_t ComponentPool::get_size() const { return _element_size; }

void* ComponentPool::get(size_t idx) {
	const size_t page_idx = idx / PAGE_SIZE;
	const size_t offset = idx % PAGE_SIZE;

	if (page_idx >= _pages.size() || !_pages[page_idx]) {
		return nullptr;
	}

	return _pages[page_idx].get() + (offset * _element_size);
}

Registry::~Registry() { clear(); }

void Registry::clear() {
	// Clear all data
	_component_pools.clear();
	_entities.clear();
	_free_indices = {};
	_entity_counter = 0;
}

void Registry::copy_to(Registry& dest) {
	dest.clear();

	// Copy trivial data
	dest._entity_counter = _entity_counter;
	dest._free_indices = _free_indices;
	dest._entities = _entities; // This copies versions and component masks

	// Prepare destination pools
	dest._component_pools.resize(_component_pools.size(), nullptr);

	// Iterate all pools and copy component data
	for (size_t comid = 0; comid < _component_pools.size(); comid++) {
		if (_component_pools[comid] == nullptr) {
			continue; // This component type isn't used
		}

		// Copy the components
		const auto old_pool = _component_pools[comid];
		dest._component_pools[comid] = std::make_unique<ComponentPool>(*old_pool);
	}
}

Entity Registry::spawn() {
	if (!_free_indices.empty()) {
		uint32_t new_idx = _free_indices.front();
		_free_indices.pop();

		Entity new_id = create_entity_id(new_idx, get_entity_version(_entities[new_idx].id));

		_entities[new_idx].id = new_id;

		return new_id;
	}

	_entities.push_back({ create_entity_id(_entities.size(), 0), ComponentMask() });

	return _entities.back().id;
}

bool Registry::is_valid(Entity entity) {
	if (get_entity_index(entity) >= _entities.size()) {
		return false;
	}

	return _entities[get_entity_index(entity)].id == entity;
}

void Registry::despawn(Entity entity) {
	const uint32_t entity_idx = get_entity_index(entity);

	Entity new_entity_id = create_entity_id(UINT32_MAX, get_entity_version(entity) + 1);

	_entities[entity_idx].id = new_entity_id;
	_entities[entity_idx].mask.reset();

	_free_indices.push(entity_idx);
}

bool Registry::assign_id(Entity entity, uint32_t component_id) {
	if (!is_valid(entity)) {
		return false;
	}

	_entities[get_entity_index(entity)].mask.set(component_id);

	return true;
}

bool Registry::remove(Entity entity, uint32_t component_id) {
	if (!is_valid(entity)) {
		return false;
	}

	const uint32_t entity_idx = get_entity_index(entity);

	if (_entities[entity_idx].mask.test(component_id)) {
		// TODO: destroy the component object
		_entities[entity_idx].mask.reset(component_id);
	}

	return true;
}

bool Registry::has(Entity entity, uint32_t component_id) {
	if (!is_valid(entity)) {
		return false;
	}

	return _entities[get_entity_index(entity)].mask.test(component_id);
}

} //namespace gl
