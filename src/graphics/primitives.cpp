#include "graphics/primitives.h"

#include "graphics/mesh.h"

namespace gl {

const std::vector<MeshVertex> CUBE_VERTICES = {
	// Front face (Z = 1)
	{ { -0.5f, -0.5f, 0.5f }, 0.0f, { 0.0f, 0.0f, 1.0f }, 0.0f }, // 0
	{ { 0.5f, -0.5f, 0.5f }, 1.0f, { 0.0f, 0.0f, 1.0f }, 0.0f }, // 1
	{ { 0.5f, 0.5f, 0.5f }, 1.0f, { 0.0f, 0.0f, 1.0f }, 1.0f }, // 2
	{ { -0.5f, 0.5f, 0.5f }, 0.0f, { 0.0f, 0.0f, 1.0f }, 1.0f }, // 3

	// Back face (Z = -1)
	{ { -0.5f, -0.5f, -0.5f }, 1.0f, { 0.0f, 0.0f, -1.0f }, 0.0f }, // 4
	{ { -0.5f, 0.5f, -0.5f }, 1.0f, { 0.0f, 0.0f, -1.0f }, 1.0f }, // 5
	{ { 0.5f, 0.5f, -0.5f }, 0.0f, { 0.0f, 0.0f, -1.0f }, 1.0f }, // 6
	{ { 0.5f, -0.5f, -0.5f }, 0.0f, { 0.0f, 0.0f, -1.0f }, 0.0f }, // 7

	// Top face (Y = 1)
	{ { -0.5f, 0.5f, 0.5f }, 0.0f, { 0.0f, 1.0f, 0.0f }, 0.0f }, // 8
	{ { 0.5f, 0.5f, 0.5f }, 1.0f, { 0.0f, 1.0f, 0.0f }, 0.0f }, // 9
	{ { 0.5f, 0.5f, -0.5f }, 1.0f, { 0.0f, 1.0f, 0.0f }, 1.0f }, // 10
	{ { -0.5f, 0.5f, -0.5f }, 0.0f, { 0.0f, 1.0f, 0.0f }, 1.0f }, // 11

	// Bottom face (Y = -1)
	{ { -0.5f, -0.5f, 0.5f }, 0.0f, { 0.0f, -1.0f, 0.0f }, 1.0f }, // 12
	{ { -0.5f, -0.5f, -0.5f }, 0.0f, { 0.0f, -1.0f, 0.0f }, 0.0f }, // 13
	{ { 0.5f, -0.5f, -0.5f }, 1.0f, { 0.0f, -1.0f, 0.0f }, 0.0f }, // 14
	{ { 0.5f, -0.5f, 0.5f }, 1.0f, { 0.0f, -1.0f, 0.0f }, 1.0f }, // 15

	// Right face (X = 1)
	{ { 0.5f, -0.5f, 0.5f }, 0.0f, { 1.0f, 0.0f, 0.0f }, 0.0f }, // 16
	{ { 0.5f, -0.5f, -0.5f }, 1.0f, { 1.0f, 0.0f, 0.0f }, 0.0f }, // 17
	{ { 0.5f, 0.5f, -0.5f }, 1.0f, { 1.0f, 0.0f, 0.0f }, 1.0f }, // 18
	{ { 0.5f, 0.5f, 0.5f }, 0.0f, { 1.0f, 0.0f, 0.0f }, 1.0f }, // 19

	// Left face (X = -1)
	{ { -0.5f, -0.5f, 0.5f }, 1.0f, { -1.0f, 0.0f, 0.0f }, 0.0f }, // 20
	{ { -0.5f, 0.5f, 0.5f }, 1.0f, { -1.0f, 0.0f, 0.0f }, 1.0f }, // 21
	{ { -0.5f, 0.5f, -0.5f }, 0.0f, { -1.0f, 0.0f, 0.0f }, 1.0f }, // 22
	{ { -0.5f, -0.5f, -0.5f }, 0.0f, { -1.0f, 0.0f, 0.0f }, 0.0f } // 23
};

const std::vector<uint32_t> CUBE_INDICES = {
	0, 1, 2, 0, 2, 3, // Front
	4, 5, 6, 4, 6, 7, // Back
	8, 9, 10, 8, 10, 11, // Top
	12, 13, 14, 12, 14, 15, // Bottom
	16, 17, 18, 16, 18, 19, // Right
	20, 21, 22, 20, 22, 23 // Left
};

std::shared_ptr<StaticMesh> create_cube_mesh(std::shared_ptr<RenderBackend> p_backend) {
	return StaticMesh::create(p_backend, CUBE_VERTICES, CUBE_INDICES);
}

const std::vector<MeshVertex> PLANE_VERTICES = { { { -0.5f, 0.0f, 0.5f }, 0.0f,
														 { 0.0f, 1.0f, 0.0f }, 0.0f },
	{ { 0.5f, 0.0f, 0.5f }, 1.0f, { 0.0f, 1.0f, 0.0f }, 0.0f },
	{ { 0.5f, 0.0f, -0.5f }, 1.0f, { 0.0f, 1.0f, 0.0f }, 1.0f },
	{ { -0.5f, 0.0f, -0.5f }, 0.0f, { 0.0f, 1.0f, 0.0f }, 1.0f } };

const std::vector<uint32_t> PLANE_INDICES = { 0, 1, 2, 0, 2, 3 };

std::shared_ptr<StaticMesh> create_plane_mesh(std::shared_ptr<RenderBackend> p_backend) {
	return StaticMesh::create(p_backend, PLANE_VERTICES, PLANE_INDICES);
}

std::shared_ptr<StaticMesh> create_sphere_mesh(
		std::shared_ptr<RenderBackend> p_backend, uint32_t p_sectors, uint32_t p_stacks) {
	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;

	Vec3f v_pos;
	Vec3f v_normal;
	float s, t;

	float sector_step = 2 * M_PI / p_sectors;
	float stack_step = M_PI / p_stacks;
	float sector_angle, stack_angle;

	for (unsigned int i = 0; i <= p_stacks; ++i) {
		stack_angle = M_PI / 2 - i * stack_step; // starting from pi/2 to -pi/2
		float xy = cosf(stack_angle); // r * cos(u)
		v_pos.z = sinf(stack_angle); // r * sin(u)

		for (unsigned int j = 0; j <= p_sectors; ++j) {
			sector_angle = j * sector_step; // starting from 0 to 2pi

			// Position
			v_pos.x = xy * cosf(sector_angle); // r * cos(u) * cos(v)
			v_pos.y = xy * sinf(sector_angle); // r * cos(u) * sin(v)

			// Normal vector (for a unit sphere, this is just the position)
			v_normal = v_pos;

			// TexCoords
			s = (float)j / p_sectors;
			t = (float)i / p_stacks;

			vertices.push_back({ v_pos, s, v_normal, t });
		}
	}

	// Generate Indices
	uint32_t k1, k2;
	for (unsigned int i = 0; i < p_stacks; ++i) {
		k1 = i * (p_sectors + 1); // beginning of current stack
		k2 = k1 + p_sectors + 1; // beginning of next stack

		for (unsigned int j = 0; j < p_sectors; ++j, ++k1, ++k2) {
			if (i != 0) {
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			if (i != (p_stacks - 1)) {
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}

	return StaticMesh::create(p_backend, vertices, indices);
}

} //namespace gl
