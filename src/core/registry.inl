#pragma once

#include "core/registry.h"

#include "core/assert.h"

namespace gl {

template <std::default_initializable T> T* ComponentPool::add(uint32_t p_idx) {
	GL_ASSERT(sizeof(T) == element_size, "Given template argument T does not match element_size");

	const size_t page_idx = p_idx / PAGE_SIZE;
	const size_t offset = p_idx % PAGE_SIZE;

	// Ensure we have enough pages
	if (page_idx >= pages.size()) {
		pages.resize(page_idx + 1);
	}

	// Allocate the page if it doesn't exist
	if (!pages[page_idx]) {
		pages[page_idx] = std::make_unique<uint8_t[]>(PAGE_SIZE * element_size);
	}

	void* ptr = pages[page_idx].get() + (offset * element_size);
	return new (ptr) T(); // In-place construction
}

template <typename T> T* Registry::assign(Entity p_entity) {
	if (!is_valid(p_entity)) {
		return nullptr;
	}

	const uint32_t component_id = get_component_id<T>();

	if (component_pools.size() <= component_id) {
		component_pools.resize(component_id + 1, nullptr);
	}

	if (!component_pools[component_id]) {
		component_pools[component_id] = std::make_shared<ComponentPool>(sizeof(T));
	}

	// Bookkeep
	T* component = component_pools[component_id]->add<T>(get_entity_index(p_entity));

	entities[get_entity_index(p_entity)].mask.set(component_id);

	return component;
}

template <typename... TComponents>
std::tuple<TComponents*...> Registry::assign_many(Entity p_entity) {
	if (!is_valid(p_entity)) {
		return std::make_tuple(static_cast<TComponents*>(nullptr)...);
	}

	return std::make_tuple(assign<TComponents>(p_entity)...);
}

template <typename T> bool Registry::remove(Entity p_entity) {
	if (!is_valid(p_entity)) {
		return false;
	}

	const uint32_t component_id = get_component_id<T>();

	return remove(p_entity, component_id);
}

template <typename... TComponents> void Registry::remove_many(Entity p_entity) {
	if (!is_valid(p_entity)) {
		return;
	}

	(remove<TComponents>(p_entity), ...);
}

template <typename T> T* Registry::get(Entity p_entity) {
	if (!is_valid(p_entity)) {
		return nullptr;
	}

	const uint32_t component_id = get_component_id<T>();
	if (!entities[get_entity_index(p_entity)].mask.test(component_id)) {
		return nullptr;
	}

	const uint32_t entity_idx = get_entity_index(p_entity);

	T* component = static_cast<T*>(component_pools[component_id]->get(entity_idx));
	return component;
}

template <typename... TComponents> std::tuple<TComponents*...> Registry::get_many(Entity p_entity) {
	if (!is_valid(p_entity)) {
		return std::make_tuple(static_cast<TComponents*>(nullptr)...);
	}

	return std::make_tuple(get<TComponents>(p_entity)...);
}

template <typename... TComponents> bool Registry::has(Entity p_entity) {
	if (!is_valid(p_entity)) {
		return false;
	}

	const uint32_t component_ids[] = { get_component_id<TComponents>()... };
	for (int i = 0; i < sizeof...(TComponents); i++) {
		if (!entities[get_entity_index(p_entity)].mask.test(component_ids[i])) {
			return false;
		}
	}

	return true;
}

template <typename... TComponents> SceneView<TComponents...> Registry::view() {
	return SceneView<TComponents...>(&entities);
}

template <typename... TComponents>
SceneView<TComponents...>::SceneView(EntityContainer* p_entities) : entities(p_entities) {
	if constexpr (sizeof...(TComponents) == 0) {
		all = true;
	} else {
		// unpack the parameter list and set the component mask accordingly
		const uint32_t component_ids[] = { get_component_id<TComponents>()... };
		for (int i = 0; i < sizeof...(TComponents); i++) {
			component_mask.set(component_ids[i]);
		}
	}
}

template <typename... TComponents>
const typename SceneView<TComponents...>::Iterator SceneView<TComponents...>::begin() const {
	uint32_t first_index = 0;
	while (first_index < entities->size() &&
			(component_mask != (component_mask & entities->at(first_index).mask) ||
					!is_entity_valid(entities->at(first_index).id))) {
		first_index++;
	}

	return Iterator(entities, first_index, component_mask, all);
}

template <typename... TComponents>
const typename SceneView<TComponents...>::Iterator SceneView<TComponents...>::end() const {
	return Iterator(entities, entities->size(), component_mask, all);
}

template <typename... TComponents>
SceneView<TComponents...>::Iterator::Iterator(
		EntityContainer* p_entities, uint32_t p_index, ComponentMask p_mask, bool p_all) :
		entities(p_entities), index(p_index), mask(p_mask), all(p_all) {}

template <typename... TComponents> Entity SceneView<TComponents...>::Iterator::operator*() const {
	return entities->at(index).id;
}

template <typename... TComponents>
bool SceneView<TComponents...>::Iterator::operator==(const Iterator& p_other) const {
	return index == p_other.index || index == entities->size();
}

template <typename... TComponents>
bool SceneView<TComponents...>::Iterator::operator!=(const Iterator& p_other) const {
	return !(*this == p_other);
}

template <typename... TComponents>
typename SceneView<TComponents...>::Iterator SceneView<TComponents...>::Iterator::operator++() {
	do {
		index++;
	} while (index < entities->size() && !_is_index_valid());

	return *this;
}

template <typename... TComponents> bool SceneView<TComponents...>::Iterator::_is_index_valid() {
	return
			// It's a valid entity ID
			is_entity_valid(entities->at(index).id) &&
			// It has the correct component mask
			(all || mask == (mask & entities->at(index).mask));
}

} //namespace gl
