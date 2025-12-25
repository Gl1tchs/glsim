#include "physics/rigidbody.h"

namespace gl {

void Rigidbody::add_force(const Vec3f& p_force) { force_acc += p_force; }

} //namespace gl
