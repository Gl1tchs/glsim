#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "physics/physics_system.h"
#include "physics/rigidbody.h"
#include "scene/registry.h"

namespace py = pybind11;

using namespace gl;

PYBIND11_MODULE(_glsim, m, py::mod_gil_not_used()) {
	py::class_<Entity>(m, "Entity")
			.def(py::init<>())
			// Allow Python to treat Entity like a number (cast to int/long)
			.def("__int__", [](const Entity& e) { return (uint64_t)e; })
			// Expose utility functions
			.def_static("create_id", &create_entity_id)
			.def_static("get_index", &get_entity_index)
			.def_static("get_version", &get_entity_version);

	py::class_<Registry>(m, "Registry")
			.def(py::init<>())
			.def("clear", &Registry::clear)
			.def("spawn", &Registry::spawn)
			.def("is_valid", &Registry::is_valid)
			.def("despawn", &Registry::despawn)
			// Temporary methods
			.def(
					"get_position",
					[](Registry& self, Entity entity) {
						Position* pos = self.get<Position>(entity);
						if (!pos) {
							throw std::runtime_error("Position component does not exists.");
						}

						float* ptr = &(pos->x);
						py::capsule free_when_done(
								ptr, [](void* f) { /* Do nothing - Registry owns the memory */ });
						return py::array_t<float>({ 3 }, // Shape: 3 elements
								{ sizeof(
										float) }, // Strides: distance between elements (contiguous)
								ptr, // Pointer to the data (starting at pos->x)
								free_when_done // Capsule (ensures ownership/lifetime is safe)
						);
					},
					py::return_value_policy::reference_internal)
			.def("assign_position",
					[](Registry& self, Entity entity, py::array_t<float> position) {
						py::buffer_info buf = position.request();

						if (buf.size != 3) {
							throw std::runtime_error("Number of dimensions must be 3");
						}

						Position* p = self.assign<Position>(entity);
						p->x = ((float*)buf.ptr)[0];
						p->y = ((float*)buf.ptr)[1];
						p->z = ((float*)buf.ptr)[2];
					})
			.def(
					"get_velocity",
					[](Registry& self, Entity entity) {
						Velocity* vel = self.get<Velocity>(entity);
						if (!vel) {
							throw std::runtime_error("Position component does not exists.");
						}

						float* ptr = &(vel->x);
						py::capsule free_when_done(
								ptr, [](void* f) { /* Do nothing - Registry owns the memory */ });
						return py::array_t<float>({ 3 }, // Shape: 3 elements
								{ sizeof(
										float) }, // Strides: distance between elements (contiguous)
								ptr, // Pointer to the data (starting at pos->x)
								free_when_done // Capsule (ensures ownership/lifetime is safe)
						);
					},
					py::return_value_policy::reference_internal)
			.def("assign_velocity", [](Registry& self, Entity entity, py::array_t<float> velocity) {
				py::buffer_info buf = velocity.request();

				if (buf.size != 3) {
					throw std::runtime_error("Number of dimensions must be 3");
				}

				Velocity* v = self.assign<Velocity>(entity);
				v->x = ((float*)buf.ptr)[0];
				v->y = ((float*)buf.ptr)[1];
				v->z = ((float*)buf.ptr)[2];
			});

	py::class_<PhysicsSystem>(m, "PhysicsSystem")
			.def_static("init", &PhysicsSystem::init)
			.def_static("shutdown", &PhysicsSystem::init)
			.def_static("update", &PhysicsSystem::update);
}
