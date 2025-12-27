#include <catch2/catch_test_macros.hpp>

#include "core/ref_counted.h"

using AssetHandle = gl::RefCounted<int>;

TEST_CASE("RefCounted tests", "[RefCounted]") {
	using namespace gl;

	SECTION("Initial Ref Count is 1") {
		int test_id = 100;
		AssetHandle handle(test_id);

		// Created once, ref count should be 1
		REQUIRE(handle.ref_count() == 1);
		REQUIRE(handle.value() == test_id);
	}

	SECTION("Copy Increment") {
		AssetHandle h1(int(1));

		{
			AssetHandle h2 = h1; // Copy construction
			REQUIRE(h1.ref_count() == 2);
			REQUIRE(h2.ref_count() == 2);
			REQUIRE(h1.value() == h2.value());
		}

		// h2 goes out of scope
		REQUIRE(h1.ref_count() == 1);
	}

	SECTION("Assignment Increment/Decrement") {
		AssetHandle h1(10);
		AssetHandle h2(20);

		REQUIRE(h1.ref_count() == 1);
		REQUIRE(h2.ref_count() == 1);

		h2 = h1; // h2 drops 20, takes 10

		// h1 is now shared by h2
		REQUIRE(h1.ref_count() == 2);
		// h2 is the same as h1
		REQUIRE(h2.ref_count() == 2);
	}

	SECTION("Move Semantics") {
		AssetHandle h1(50);
		REQUIRE(h1.ref_count() == 1);

		AssetHandle h2 = std::move(h1);

		// Ownership transferred
		REQUIRE(h2.ref_count() == 1);
		// h1 is now in a moved-from state (valid but unspecified, usually null/0 refs)
		REQUIRE(h2.value() == int(50));
	}
}
