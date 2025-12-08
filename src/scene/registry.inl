#pragma once

#include "scene/registry.h"

namespace gl {

template <typename T> T* Registry::assign(Entity p_entity) {
	if (!is_valid(p_entity)) {
		return nullptr;
	}

	const uint32_t component_id = get_component_id<T>();

	if (component_pools.size() <= component_id) {
		component_pools.resize(component_id + 1, nullptr);
		pool_helpers.resize(component_id + 1);
	}
	if (component_pools[component_id] == nullptr) {
		component_pools[component_id] = new ComponentPool(sizeof(T));
		pool_helpers[component_id] = PoolHelpers{
			.element_size = sizeof(T),
			// Copy function (uses placement new + copy constructor)
			.copy_fn = [](void* dest,
							   const void* src) { new (dest) T(*static_cast<const T*>(src)); },
			// Destroy function (calls destructor)
			.destroy_fn = [](void* data) { static_cast<T*>(data)->~T(); },
		};
	}

	// Call destructor if component already exists
	if (entities[get_entity_index(p_entity)].mask.test(component_id)) {
		pool_helpers[component_id].destroy_fn(
				component_pools[component_id]->get(get_entity_index(p_entity)));
	}

	T* component = new (component_pools[component_id]->get(get_entity_index(p_entity))) T();

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

template <typename T> void Registry::remove(Entity p_entity) {
	if (!is_valid(p_entity)) {
		return;
	}

	const uint32_t component_id = get_component_id<T>();
	const uint32_t entity_idx = get_entity_index(p_entity);

	if (entities[entity_idx].mask.test(component_id)) {
		// call component's destructor
		if (pool_helpers.size() > component_id && pool_helpers[component_id].destroy_fn) {
			pool_helpers[component_id].destroy_fn(component_pools[component_id]->get(entity_idx));
		}
		entities[entity_idx].mask.reset(component_id);
	}
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

	T* component = static_cast<T*>(component_pools[component_id]->get(get_entity_index(p_entity)));

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
