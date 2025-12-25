/**
 * @file transform.h
 */

#pragma once

#include "glgpu/mat.h"
#include "glgpu/vec.h"

namespace gl {

struct Transform {
	Vec3f position = Vec3f::ZERO;
	Vec3f rotation = Vec3f::ZERO;
	Vec3f scale = Vec3f::ONE;

	void translate(const Vec3f& p_translation);

	void rotate(float p_angle, const Vec3f& p_axis);

	Vec3f get_forward() const;
	Vec3f get_right() const;
	Vec3f get_up() const;

	Mat4 to_mat4() const;
};

inline constexpr Transform DEFAULT_TRANSFORM{};

} //namespace gl
