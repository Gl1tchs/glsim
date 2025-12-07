#pragma once

#include "scene/registry.h"

namespace gl {

template <typename T> T* Registry::assign(Entity entity) {
	if (!is_valid(entity)) {
		return nullptr;
	}

	const uint32_t component_id = get_component_id<T>();

	if (_component_pools.size() <= component_id) {
		_component_pools.resize(component_id + 1, nullptr);
		_pool_helpers.resize(component_id + 1);
	}
	if (_component_pools[component_id] == nullptr) {
		_component_pools[component_id] = new ComponentPool(sizeof(T));
		_pool_helpers[component_id] = PoolHelpers{
			.element_size = sizeof(T),
			// Copy function (uses placement new + copy constructor)
			.copy_fn = [](void* dest,
							   const void* src) { new (dest) T(*static_cast<const T*>(src)); },
			// Destroy function (calls destructor)
			.destroy_fn = [](void* data) { static_cast<T*>(data)->~T(); },
		};
	}

	// Call destructor if component already exists
	if (_entities[get_entity_index(entity)].mask.test(component_id)) {
		_pool_helpers[component_id].destroy_fn(
				_component_pools[component_id]->get(get_entity_index(entity)));
	}

	T* component = new (_component_pools[component_id]->get(get_entity_index(entity))) T();

	_entities[get_entity_index(entity)].mask.set(component_id);

	return component;
}

template <typename... TComponents>
std::tuple<TComponents*...> Registry::assign_many(Entity entity) {
	if (!is_valid(entity)) {
		return std::make_tuple(static_cast<TComponents*>(nullptr)...);
	}

	return std::make_tuple(assign<TComponents>(entity)...);
}

template <typename T> void Registry::remove(Entity entity) {
	if (!is_valid(entity)) {
		return;
	}

	const uint32_t component_id = get_component_id<T>();
	const uint32_t entity_idx = get_entity_index(entity);

	if (_entities[entity_idx].mask.test(component_id)) {
		// call component's destructor
		if (_pool_helpers.size() > component_id && _pool_helpers[component_id].destroy_fn) {
			_pool_helpers[component_id].destroy_fn(_component_pools[component_id]->get(entity_idx));
		}
		_entities[entity_idx].mask.reset(component_id);
	}
}

template <typename... TComponents> void Registry::remove_many(Entity entity) {
	if (!is_valid(entity)) {
		return;
	}

	(remove<TComponents>(entity), ...);
}

template <typename T> T* Registry::get(Entity entity) {
	if (!is_valid(entity)) {
		return nullptr;
	}

	const uint32_t component_id = get_component_id<T>();
	if (!_entities[get_entity_index(entity)].mask.test(component_id)) {
		return nullptr;
	}

	T* component = static_cast<T*>(_component_pools[component_id]->get(get_entity_index(entity)));

	return component;
}

template <typename... TComponents> std::tuple<TComponents*...> Registry::get_many(Entity entity) {
	if (!is_valid(entity)) {
		return std::make_tuple(static_cast<TComponents*>(nullptr)...);
	}

	return std::make_tuple(get<TComponents>(entity)...);
}

template <typename... TComponents> bool Registry::has(Entity entity) {
	if (!is_valid(entity)) {
		return false;
	}

	const uint32_t component_ids[] = { get_component_id<TComponents>()... };
	for (int i = 0; i < sizeof...(TComponents); i++) {
		if (!_entities[get_entity_index(entity)].mask.test(component_ids[i])) {
			return false;
		}
	}

	return true;
}

template <typename... TComponents> SceneView<TComponents...> Registry::view() {
	return SceneView<TComponents...>(&_entities);
}

template <typename... TComponents>
SceneView<TComponents...>::SceneView(EntityContainer* entities) : _entities(entities) {
	if constexpr (sizeof...(TComponents) == 0) {
		_all = true;
	} else {
		// unpack the parameter list and set the component mask accordingly
		const uint32_t component_ids[] = { get_component_id<TComponents>()... };
		for (int i = 0; i < sizeof...(TComponents); i++) {
			_component_mask.set(component_ids[i]);
		}
	}
}

template <typename... TComponents>
const typename SceneView<TComponents...>::Iterator SceneView<TComponents...>::begin() const {
	uint32_t first_index = 0;
	while (first_index < _entities->size() &&
			(_component_mask != (_component_mask & _entities->at(first_index).mask) ||
					!is_entity_valid(_entities->at(first_index).id))) {
		first_index++;
	}

	return Iterator(_entities, first_index, _component_mask, _all);
}

template <typename... TComponents>
const typename SceneView<TComponents...>::Iterator SceneView<TComponents...>::end() const {
	return Iterator(_entities, _entities->size(), _component_mask, _all);
}

template <typename... TComponents>
SceneView<TComponents...>::Iterator::Iterator(
		EntityContainer* entities, uint32_t index, ComponentMask mask, bool all) :
		_entities(entities), _index(index), _mask(mask), _all(all) {}

template <typename... TComponents> Entity SceneView<TComponents...>::Iterator::operator*() const {
	return _entities->at(_index).id;
}

template <typename... TComponents>
bool SceneView<TComponents...>::Iterator::operator==(const Iterator& other) const {
	return _index == other._index || _index == _entities->size();
}

template <typename... TComponents>
bool SceneView<TComponents...>::Iterator::operator!=(const Iterator& other) const {
	return !(*this == other);
}

template <typename... TComponents>
typename SceneView<TComponents...>::Iterator SceneView<TComponents...>::Iterator::operator++() {
	do {
		_index++;
	} while (_index < _entities->size() && !is_index_valid());

	return *this;
}

template <typename... TComponents> bool SceneView<TComponents...>::Iterator::is_index_valid() {
	return
			// It's a valid entity ID
			is_entity_valid(_entities->at(_index).id) &&
			// It has the correct component mask
			(_all || _mask == (_mask & _entities->at(_index).mask));
}

} //namespace gl
