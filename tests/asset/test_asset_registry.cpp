#include <catch2/catch_test_macros.hpp>

#include "asset/asset_registry.h"

using namespace gl;

// Dummy asset for testing
struct MockAsset {
	int id;
	std::string name;
};

TEST_CASE("AssetRegistry Mechanics", "[AssetRegistry]") {
	AssetRegistry<MockAsset> registry;

	SECTION("Registering Assets") {
		auto asset = std::make_shared<MockAsset>();
		asset->id = 1;
		asset->name = "Grass";

		// Register returns a handle
		AssetHandle handle = registry.register_asset(std::move(asset));

		REQUIRE(handle.ref_count() == 2); // 1 in 'handle', 1 in 'registry map key'
		REQUIRE(registry.get_asset_size() == 1);
	}

	SECTION("Garbage Collection (Keep Alive)") {
		AssetHandle handle;
		{
			auto asset = std::make_shared<MockAsset>();
			handle = registry.register_asset(std::move(asset));
		}

		// We still hold 'handle', so ref_count is 2
		// Registry should NOT delete it
		registry.collect_garbage();
		REQUIRE(registry.get_asset_size() == 1);
	}

	SECTION("Garbage Collection (Sweep)") {
		AssetHandle handle;
		{
			auto asset = std::make_shared<MockAsset>();
			handle = registry.register_asset(std::move(asset));
		}
		// Handle is valid here, ref count 2
		REQUIRE(registry.get_asset_size() == 1);

		// Force reset/drop of our local handle
		// Now only the Registry Map Key holds a reference (ref count 1)
		handle = INVALID_ASSET_HANDLE;

		// GC should find that ref_count == 1 and delete it
		registry.collect_garbage();

		REQUIRE(registry.get_asset_size() == 0);
	}

	SECTION("Erasing Assets Manually") {
		auto asset = std::make_shared<MockAsset>();
		AssetHandle handle = registry.register_asset(std::move(asset));

		REQUIRE(registry.get_asset_size() == 1);

		bool removed = registry.erase(handle);
		REQUIRE(removed);
		REQUIRE(registry.get_asset_size() == 0);
	}

	SECTION("Clear Registry") {
		registry.register_asset(std::make_shared<MockAsset>());
		registry.register_asset(std::make_shared<MockAsset>());

		REQUIRE(registry.get_asset_size() == 2);
		registry.clear();
		REQUIRE(registry.get_asset_size() == 0);
	}
}
