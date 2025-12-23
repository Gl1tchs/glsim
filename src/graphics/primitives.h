#pragma once

#include "graphics/mesh.h"

namespace gl {

std::shared_ptr<StaticMesh> create_cube_mesh(std::shared_ptr<RenderBackend> p_backend);

std::shared_ptr<StaticMesh> create_plane_mesh(std::shared_ptr<RenderBackend> p_backend);

std::shared_ptr<StaticMesh> create_sphere_mesh(
		std::shared_ptr<RenderBackend> p_backend, uint32_t p_sectors = 32, uint32_t p_stacks = 16);

} //namespace gl
