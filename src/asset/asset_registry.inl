#pragma once

#include "asset/asset_registry.h"

namespace gl {

template <typename T> size_t AssetRegistry<T>::get_asset_size() const { return assets.size(); }

template <typename T> void AssetRegistry<T>::collect_garbage() {
	std::erase_if(assets, [](const auto& item) {
		// get_ref_count() == 1 means only the map holds it
		return item.first.ref_count() == 1;
	});
}

template <typename T> AssetHandle AssetRegistry<T>::register_asset(std::shared_ptr<T> asset) {
	AssetHandle handle;
	assets.insert_or_assign(handle, asset);
	return handle;
}

template <typename T> std::shared_ptr<T> AssetRegistry<T>::get_asset(const AssetHandle& handle) {
	const auto it = assets.find(handle);
	if (it == assets.end()) {
		return nullptr;
	}

	return it->second;
}

template <typename T> bool AssetRegistry<T>::erase(AssetHandle handle) {
	return assets.erase(handle) > 0;
}

template <typename T> void AssetRegistry<T>::clear() { assets.clear(); }

}; //namespace gl
