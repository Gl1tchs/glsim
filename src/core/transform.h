/**
 * @file transform.h
 */

#pragma once

#include "glgpu/matrix.h"
#include "glgpu/vector.h"

namespace gl {

struct Transform {
	Vec3f position = Vec3f::zero();
	Vec3f rotation = Vec3f::zero();
	Vec3f scale = Vec3f::one();

	void translate(const Vec3f& p_translation);

	void rotate(float p_angle, const Vec3f& p_axis);

	Vec3f get_forward() const;
	Vec3f get_right() const;
	Vec3f get_up() const;

	Mat4 to_mat4() const;
};

inline constexpr Transform DEFAULT_TRANSFORM{};

} //namespace gl
