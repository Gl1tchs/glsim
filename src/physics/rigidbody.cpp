#include "physics/rigidbody.h"

namespace gl {

void Rigidbody::add_force(const Vec3f& force) { force_acc += force; }

} //namespace gl
