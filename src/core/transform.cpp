#include "core/transform.h"

namespace gl {

void Transform::translate(const Vec3f& p_translation) { position = position + p_translation; }

void Transform::rotate(const float p_angle, const Vec3f& p_axis) {
	rotation = rotation + (p_axis * p_angle);
}

Vec3f Transform::get_forward() const {
	const Mat4 rot_mat = Mat4::from_euler_angles(rotation);
	return Vec3f(rot_mat * Vec3f::FORWARD).normalize();
}

Vec3f Transform::get_right() const {
	const Mat4 rot_mat = Mat4::from_euler_angles(rotation);
	return Vec3f(rot_mat * Vec3f::RIGHT).normalize();
}

Vec3f Transform::get_up() const {
	const Mat4 rot_mat = Mat4::from_euler_angles(rotation);
	return Vec3f(rot_mat * Vec3f::UP).normalize();
}

Mat4 Transform::to_mat4() const {
	const Mat4 mat_T = Mat4::translate(position);
	const Mat4 mat_R = Mat4::from_euler_angles(rotation);
	const Mat4 mat_S = Mat4::scale(scale);
	return mat_T * mat_R * mat_S;
}
} //namespace gl
