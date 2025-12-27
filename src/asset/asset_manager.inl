#pragma once

#include "asset/asset_manager.h"

namespace gl {

template <typename T> AssetRegistry<T>& AssetManager::get_registry() {
	const std::type_index index(typeid(T));

	if (_registries.find(index) == _registries.end()) {
		_registries[index] = std::make_shared<AssetRegistry<T>>();
	}

	return *static_cast<AssetRegistry<T>*>(_registries[index].get());
}

template <typename T> std::shared_ptr<T> AssetManager::get(const AssetHandle& handle) {
	return get_registry<T>().get_asset(handle);
}

template <typename T> AssetHandle AssetManager::register_asset(std::shared_ptr<T> asset) {
	return get_registry<T>().register_asset(std::move(asset));
}

template <typename T> bool AssetManager::free(const AssetHandle& handle) {
	return get_registry<T>().erase(handle);
}

} //namespace gl
