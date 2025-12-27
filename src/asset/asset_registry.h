#pragma once

#include "core/ref_counted.h"
#include "core/uid.h"

namespace gl {

// Counted atomic reference for GC system.
class AssetHandle : public RefCounted<UID> {
public:
	AssetHandle() : RefCounted<UID>(UID()) {}
	AssetHandle(UID id) : RefCounted<UID>(std::move(id)) {}

	AssetHandle(const AssetHandle& rhs) : RefCounted<UID>(rhs) {}
	AssetHandle(AssetHandle&& rhs) noexcept : RefCounted<UID>(std::move(rhs)) {}

	virtual ~AssetHandle() override = default;

	AssetHandle& operator=(const AssetHandle& rhs) {
		RefCounted<UID>::operator=(rhs);
		return *this;
	}

	AssetHandle& operator=(AssetHandle&& rhs) noexcept {
		RefCounted<UID>::operator=(std::move(rhs));
		return *this;
	}
};

const AssetHandle INVALID_ASSET_HANDLE = AssetHandle();

struct IAssetRegistry {
	virtual ~IAssetRegistry() = default;
	virtual size_t get_asset_size() const = 0;
	virtual void collect_garbage() = 0;
	virtual void clear() = 0;
};

template <typename T> struct AssetRegistry : public IAssetRegistry {
	std::unordered_map<AssetHandle, std::shared_ptr<T>> assets;

	virtual ~AssetRegistry() = default;

	size_t get_asset_size() const override;

	void collect_garbage() override;

	AssetHandle register_asset(std::shared_ptr<T> asset);

	std::shared_ptr<T> get_asset(const AssetHandle& handle);

	bool erase(AssetHandle handle);

	void clear() override;
};

} // namespace gl

namespace std {
template <> struct hash<gl::AssetHandle> {
	size_t operator()(const gl::AssetHandle& handle) const { // Check for null pointer first
		if (handle.ref_count() == 0) {
			return 0;
		}
		return std::hash<gl::UID>{}(handle.value());
	}
};
} //namespace std

#include "asset/asset_registry.inl"
