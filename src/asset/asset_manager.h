#pragma once

#include "asset/asset_registry.h"

namespace gl {

class AssetManager {
public:
	AssetManager() = default;
	~AssetManager();

	// No copy allowed, but moving is okay.
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;
	AssetManager(AssetManager&&) = default;
	AssetManager& operator=(AssetManager&&) = default;

	void clear();

	void collect_garbage();

	template <typename T> AssetRegistry<T>& get_registry();

	template <typename T> std::shared_ptr<T> get(const AssetHandle& handle);

	template <typename T> AssetHandle register_asset(std::shared_ptr<T> asset);

	template <typename T> bool free(const AssetHandle& handle);

private:
	std::unordered_map<std::type_index, std::shared_ptr<IAssetRegistry>> _registries;
};

} //namespace gl

#include "asset/asset_manager.inl"
