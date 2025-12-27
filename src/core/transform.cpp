#include "core/transform.h"

namespace gl {

void Transform::translate(const Vec3f& translation) { position = position + translation; }

void Transform::rotate(const float angle, const Vec3f& axis) {
	rotation = rotation + (axis * angle);
}

Vec3f Transform::get_forward() const {
	const Mat4 rot_mat = Mat4::from_euler_angles(rotation);
	return Vec3f(rot_mat * Vec3f::forward()).normalize();
}

Vec3f Transform::get_right() const {
	const Mat4 rot_mat = Mat4::from_euler_angles(rotation);
	return Vec3f(rot_mat * Vec3f::right()).normalize();
}

Vec3f Transform::get_up() const {
	const Mat4 rot_mat = Mat4::from_euler_angles(rotation);
	return Vec3f(rot_mat * Vec3f::up()).normalize();
}

Mat4 Transform::to_mat4() const {
	const Mat4 mat_T = Mat4::translate(position);
	const Mat4 mat_R = Mat4::from_euler_angles(rotation);
	const Mat4 mat_S = Mat4::scale(scale);
	return mat_T * mat_R * mat_S;
}
} //namespace gl
