#pragma once

#include "graphics/mesh.h"

namespace gl {

std::shared_ptr<StaticMesh> create_cube_mesh(std::shared_ptr<RenderBackend> backend);

std::shared_ptr<StaticMesh> create_plane_mesh(std::shared_ptr<RenderBackend> backend);

std::shared_ptr<StaticMesh> create_sphere_mesh(
		std::shared_ptr<RenderBackend> backend, uint32_t sectors = 32, uint32_t stacks = 16);

} //namespace gl
