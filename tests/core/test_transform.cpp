#include <catch2/catch_test_macros.hpp>

#include "core/transform.h"

using namespace gl;

TEST_CASE("Transform initialization", "[core]") {
	Transform t;

	REQUIRE(t.position == VEC3_ZERO);
	REQUIRE(t.scale == VEC3_ONE);
	REQUIRE(t.rotation == VEC3_ZERO);
}

TEST_CASE("Translate transform", "[core]") {
	Transform t;
	Vec3f translation(1.0f, 2.0f, 3.0f);
	t.translate(translation);

	REQUIRE(t.position == translation);
}

TEST_CASE("Rotate transform", "[core]") {
	Transform t;
	t.rotate(90.0f, VEC3_UP);

	REQUIRE(t.rotation == Vec3f{ 0.0f, 90.0f, 0.0f });
}

TEST_CASE("Transform directions", "[core]") {
	Transform t;

	REQUIRE(t.get_forward() == VEC3_FORWARD);
	REQUIRE(t.get_right() == VEC3_RIGHT);
	REQUIRE(t.get_up() == VEC3_UP);
}

TEST_CASE("Transform matrix", "[core]") {
	Transform t;
	Mat4 matrix = t.to_mat4();

	REQUIRE(matrix == Mat4(1.0f));
}
