#pragma once

#include "glgpu/vector.h"

namespace gl {

struct Rigidbody {
	float mass = 1.0f;
	Vec3f velocity = Vec3f::zero();
	Vec3f force_acc = Vec3f::zero();
	float linear_damping = 0.01f;

	// Vec3f angular_velocity = Vec3f::zero();
	// Vec3f torque_acc = Vec3f::zero();
	// float angular_damping = 0.01f;

	// Mat3 local_intertia_tensor;

	bool is_static = false;
	bool use_gravity = true;

	void add_force(const Vec3f& force);
};

} //namespace gl
