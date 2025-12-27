#include "asset/asset_manager.h"

namespace gl {

AssetManager::~AssetManager() { clear(); }

void AssetManager::clear() { _registries.clear(); }

void AssetManager::collect_garbage() {
	for (auto& [type, registry] : _registries) {
		registry->collect_garbage();
	}
}

} //namespace gl
