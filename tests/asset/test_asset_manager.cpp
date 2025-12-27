#include <catch2/catch_test_macros.hpp>

#include "asset/asset_manager.h"

using namespace gl;

struct Mesh {
	int vertices;
};

struct Shader {
	int id;
};

TEST_CASE("AssetManager Integration", "[Manager]") {
	AssetManager manager;

	SECTION("Multi-Type Registration") {
		// Register a Mesh
		AssetHandle h_mesh = manager.register_asset(std::make_shared<Mesh>(Mesh{ 100 }));

		// Register a Shader
		AssetHandle h_shader = manager.register_asset(std::make_shared<Shader>(Shader{ 5 }));

		// Verify counts in specific registries
		REQUIRE(manager.get_registry<Mesh>().get_asset_size() == 1);
		REQUIRE(manager.get_registry<Shader>().get_asset_size() == 1);
	}

	SECTION("Manager Garbage Collection") {
		AssetHandle h_mesh;

		// Scope to drop h_shader
		{
			AssetHandle h_shader = manager.register_asset(std::make_shared<Shader>(Shader{ 1 }));
			h_mesh = manager.register_asset(std::make_shared<Mesh>(Mesh{ 10 }));

			// At end of scope, h_shader dies. h_mesh lives.
		}

		// Before GC: Both exist in memory (registries hold ownership)
		// h_shader is ref 1 (registry only). h_mesh is ref 2 (registry + local).
		REQUIRE(manager.get_registry<Shader>().get_asset_size() == 1);
		REQUIRE(manager.get_registry<Mesh>().get_asset_size() == 1);

		manager.collect_garbage();

		// After GC: Shader should be gone. Mesh should stay.
		REQUIRE(manager.get_registry<Shader>().get_asset_size() == 0);
		REQUIRE(manager.get_registry<Mesh>().get_asset_size() == 1);
	}

	SECTION("Free Specific Asset") {
		AssetHandle h = manager.register_asset(std::make_shared<Mesh>());
		REQUIRE(manager.get_registry<Mesh>().get_asset_size() == 1);

		// Manually free via Manager
		bool success = manager.free<Mesh>(h);

		REQUIRE(success);
		REQUIRE(manager.get_registry<Mesh>().get_asset_size() == 0);
	}

	SECTION("Clear All") {
		manager.register_asset(std::make_shared<Mesh>());
		manager.register_asset(std::make_shared<Shader>());

		manager.clear();

		REQUIRE(manager.get_registry<Mesh>().get_asset_size() == 0);
		REQUIRE(manager.get_registry<Shader>().get_asset_size() == 0);
	}
}
