#include <pybind11/pybind11.h>

#include "core/gpu_context.h"
#include "core/registry.h"
#include "core/system.h"
#include "core/world.h"
#include "graphics/rendering_system.h"
#include "physics/physics_system.h"

namespace gl {

namespace py = pybind11;

// Trampoline class for System
// This allows Python classes to inherit from System and override virtual methods.
class PySystem : public System, public py::trampoline_self_life_support {
public:
	using System::System;

	void on_init(Registry& p_registry) override {
		PYBIND11_OVERRIDE(void, System, on_init, p_registry);
	}

	void on_update(Registry& p_registry, float p_dt) override {
		PYBIND11_OVERRIDE(void, System, on_update, p_registry, p_dt);
	}

	void on_destroy(Registry& p_registry) override {
		PYBIND11_OVERRIDE(void, System, on_destroy, p_registry);
	}
};

PYBIND11_MODULE(_glsim, m, py::mod_gil_not_used()) {
	py::class_<GpuContext>(m, "GpuContext").def(py::init<>());

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

	py::class_<System, PySystem, py::smart_holder>(m, "System")
			.def(py::init<>())
			.def("on_init", &System::on_init)
			.def("on_update", &System::on_update)
			.def("on_destroy", &System::on_destroy);

	// Expose the world
	py::class_<World, Registry>(m, "World")
			.def(py::init<>())
			// Bind update method
			.def("update", &World::update, py::arg("p_dt") = 0.016f)
			.def("add_system", &World::add_system);

	py::class_<RenderingSystem, System, py::smart_holder>(m, "RenderingSystem")
			.def(py::init<GpuContext&>());
	py::class_<PhysicsSystem, System, py::smart_holder>(m, "PhysicsSystem")
			.def(py::init<GpuContext&>());
}

} //namespace gl
