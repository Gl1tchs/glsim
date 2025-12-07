#include <pybind11/pybind11.h>

#include "core/engine.h"

namespace py = pybind11;

PYBIND11_MODULE(_glsim, m, py::mod_gil_not_used()) {
	py::class_<Engine>(m, "Engine")
			.def(py::init()) // Constructor
			.def("say_hello", &Engine::say_hello);
}
