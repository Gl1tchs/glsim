#pragma once

#include "core/transform.h"
#include "glgpu/matrix.h"

namespace gl {

struct Camera {
	float aspect_ratio = 1.0f;
	float near_clip = -1.0f;
	float far_clip = 1.0f;

	virtual Mat4 get_view_matrix(const Transform& transform) const = 0;
	virtual Mat4 get_projection_matrix() const = 0;
};

struct OrthographicCamera : Camera {
	float zoom_level = 1.0f;

	OrthographicCamera();
	~OrthographicCamera() = default;

	Mat4 get_view_matrix(const Transform& transform) const override;
	Mat4 get_projection_matrix() const override;
};

struct PerspectiveCamera : Camera {
	float fov = 45.0f;

	PerspectiveCamera();
	~PerspectiveCamera() = default;

	Mat4 get_view_matrix(const Transform& transform) const override;
	Mat4 get_projection_matrix() const override;
};

} //namespace gl
