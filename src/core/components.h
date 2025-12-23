#pragma once

namespace gl {

enum class PrimitiveType {
	CUBE,
	PLANE,
	SPHERE,
};

struct MeshComponent {
	PrimitiveType type = PrimitiveType::CUBE;
};

} //namespace gl
