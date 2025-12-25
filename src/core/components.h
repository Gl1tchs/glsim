#pragma once

#include "graphics/camera.h"

namespace gl {

// Rendering Components

enum class CameraProjection {
	ORTHOGRAPHIC,
	PERSPECTIVE,
};

struct CameraComponent {
	CameraProjection projection = CameraProjection::PERSPECTIVE;
	bool enabled = true;
	PerspectiveCamera persp = {};
	OrthographicCamera ortho = {};
};

enum class PrimitiveType : uint32_t {
	CUBE,
	PLANE,
	SPHERE,
};

struct MeshComponent {
	PrimitiveType type = PrimitiveType::CUBE;
};

} //namespace gl
