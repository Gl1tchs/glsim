#pragma once

#include "core/registry.h"

#include "core/assert.h"

namespace gl {

template <std::default_initializable T> T* ComponentPool::add(uint32_t idx) {
	GL_ASSERT(sizeof(T) == _element_size, "Given template argument T does not match element_size");

	const size_t page_idx = idx / PAGE_SIZE;
	const size_t offset = idx % PAGE_SIZE;

	// Ensure we have enough pages
	if (page_idx >= _pages.size()) {
		_pages.resize(page_idx + 1);
	}

	// Allocate the page if it doesn't exist
	if (!_pages[page_idx]) {
		_pages[page_idx] = std::make_unique<uint8_t[]>(PAGE_SIZE * _element_size);
	}

	void* ptr = _pages[page_idx].get() + (offset * _element_size);
	return new (ptr) T(); // In-place construction
}

template <typename T> T* Registry::assign(Entity entity) {
	if (!is_valid(entity)) {
		return nullptr;
	}

	const uint32_t component_id = get_component_id<T>();

	if (_component_pools.size() <= component_id) {
		_component_pools.resize(component_id + 1, nullptr);
	}

	if (!_component_pools[component_id]) {
		_component_pools[component_id] = std::make_shared<ComponentPool>(sizeof(T));
	}

	// Bookkeep
	T* component = _component_pools[component_id]->add<T>(get_entity_index(entity));

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

template <typename T> bool Registry::remove(Entity entity) {
	if (!is_valid(entity)) {
		return false;
	}

	const uint32_t component_id = get_component_id<T>();

	return remove(entity, component_id);
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

	const uint32_t entity_idx = get_entity_index(entity);

	T* component = static_cast<T*>(_component_pools[component_id]->get(entity_idx));
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
	} while (_index < _entities->size() && !_is_index_valid());

	return *this;
}

template <typename... TComponents> bool SceneView<TComponents...>::Iterator::_is_index_valid() {
	return
			// It's a valid entity ID
			is_entity_valid(_entities->at(_index).id) &&
			// It has the correct component mask
			(_all || _mask == (_mask & _entities->at(_index).mask));
}

} //namespace gl
