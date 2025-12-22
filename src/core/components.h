#pragma once

namespace gl {

// TODO: remove  this
enum class MeshType {
	CUBE,
};

struct MeshComponent {
	MeshType type = MeshType::CUBE;
};

} //namespace gl
