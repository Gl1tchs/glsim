/**
 * @file transform.h
 */

#pragma once

#include "glgpu/mat.h"
#include "glgpu/vec.h"

namespace gl {

inline constexpr Vec3f VEC3_UP(0.0f, 1.0f, 0.0f);
inline constexpr Vec3f VEC3_RIGHT(1.0f, 0.0f, 0.0f);
inline constexpr Vec3f VEC3_FORWARD(0.0f, 0.0f, -1.0f);

inline constexpr Vec3f VEC3_ZERO(0.0f, 0.0f, 0.0f);
inline constexpr Vec3f VEC3_ONE(1.0f, 1.0f, 1.0f);

inline constexpr Vec3f WORLD_UP = VEC3_UP;

struct Transform {
	Vec3f position = VEC3_ZERO;
	Vec3f rotation = VEC3_ZERO;
	Vec3f scale = VEC3_ONE;

	void translate(const Vec3f& p_translation);

	void rotate(float p_angle, Vec3f p_axis);

	Vec3f get_forward() const;
	Vec3f get_right() const;
	Vec3f get_up() const;

	Mat4 to_mat4() const;
};

inline constexpr Transform DEFAULT_TRANSFORM{};

} //namespace gl
