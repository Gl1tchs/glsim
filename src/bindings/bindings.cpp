#include <pybind11/pybind11.h>

#include "core/engine.h"
#include "scene/registry.h"

namespace py = pybind11;

using namespace gl;

PYBIND11_MODULE(_glsim, m, py::mod_gil_not_used()) {
	py::class_<Engine>(m, "Engine")
			.def(py::init()) // Constructor
			.def("say_hello", &Engine::say_hello);

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
			.def("despawn", &Registry::despawn);
}
