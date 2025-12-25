#pragma once

#include "glgpu/vec.h"

namespace gl {

struct Rigidbody {
	float mass = 1.0f;
	Vec3f velocity = Vec3f::ZERO;
	Vec3f force_acc = Vec3f::ZERO;
	float linear_damping = 0.01f;

	// Vec3f angular_velocity = VEC3_ZERO;
	// Vec3f torque_acc = VEC3_ZERO;
	// float angular_damping = 0.01f;

	// Mat3 local_intertia_tensor;

	bool is_static = false;
	bool use_gravity = true;

	void add_force(const Vec3f& p_force);
};

} //namespace gl
