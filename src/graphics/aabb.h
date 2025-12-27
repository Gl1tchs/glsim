#pragma once

#include "glgpu/matrix.h"
#include "glgpu/vector.h"

namespace gl {

struct Frustum {
	Vec4f planes[6]; // left, right, bottom, top, near, far

	static Frustum from_view_proj(const Mat4& view_proj);
};

struct AABB {
	Vec3f min;
	Vec3f max;

	bool is_inside_frustum(const Frustum& frustum) const;

	AABB transform(const Mat4& transform) const;
};

} //namespace gl
