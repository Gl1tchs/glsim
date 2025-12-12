#include <pybind11/native_enum.h>
#include <pybind11/pybind11.h>
#include <pybind11/trampoline_self_life_support.h>

#include "core/event_system.h"
#include "core/gpu_context.h"
#include "core/input.h"
#include "core/log.h"
#include "core/registry.h"
#include "core/system.h"
#include "core/world.h"
#include "glgpu/vec.h"
#include "graphics/rendering_system.h"
#include "graphics/window.h"
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

enum class PyEventType {
	WINDOW_RESIZE,
	WINDOW_CLOSE,
	WINDOW_MINIMIZE,
	KEY_PRESS,
	KEY_RELEASE,
	KEY_TYPE,
	MOUSE_MOVE,
	MOUSE_SCROLL,
	MOUSE_PRESS,
	MOUSE_RELEASE,
};

void _py_subscribe_event(PyEventType p_type, py::object p_func) {
	py::gil_scoped_acquire acquire;

	if (!p_func) {
		return;
	}

	auto py_callback = std::make_shared<py::object>(std::move(p_func));

	auto cxx_wrapper = [py_callback](const auto& event_data) {
		py::gil_scoped_release release;

		if (*py_callback) {
			py::gil_scoped_acquire acquire_again;
			try {
				(*py_callback)(event_data);
			} catch (const py::error_already_set& e) {
				GL_LOG_ERROR("Python event callback failed: {}", e.what());
			}
		}
	};

	switch (p_type) {
		case PyEventType::WINDOW_RESIZE:
			event::subscribe<WindowResizeEvent>(cxx_wrapper);
			break;
		case PyEventType::WINDOW_CLOSE:
			event::subscribe<WindowCloseEvent>(cxx_wrapper);
			break;
		case PyEventType::WINDOW_MINIMIZE:
			event::subscribe<WindowMinimizeEvent>(cxx_wrapper);
		case PyEventType::KEY_PRESS:
			event::subscribe<KeyPressEvent>(cxx_wrapper);
			break;
		case PyEventType::KEY_RELEASE:
			event::subscribe<KeyReleaseEvent>(cxx_wrapper);
			break;
		case PyEventType::KEY_TYPE:
			event::subscribe<KeyTypeEvent>(cxx_wrapper);
			break;
		case PyEventType::MOUSE_MOVE:
			event::subscribe<MouseMoveEvent>(cxx_wrapper);
			break;
		case PyEventType::MOUSE_SCROLL:
			event::subscribe<MouseScrollEvent>(cxx_wrapper);
			break;
		case PyEventType::MOUSE_PRESS:
			event::subscribe<MousePressEvent>(cxx_wrapper);
			break;
		case PyEventType::MOUSE_RELEASE:
			event::subscribe<MouseReleaseEvent>(cxx_wrapper);
			break;
	}
}

void _py_unsubscribe_event(PyEventType p_type) {
	switch (p_type) {
		case PyEventType::WINDOW_RESIZE:
			event::unsubscribe<WindowResizeEvent>();
			break;
		case PyEventType::WINDOW_CLOSE:
			event::unsubscribe<WindowCloseEvent>();
			break;
		case PyEventType::WINDOW_MINIMIZE:
			event::unsubscribe<WindowMinimizeEvent>();
		case PyEventType::KEY_PRESS:
			event::unsubscribe<KeyPressEvent>();
			break;
		case PyEventType::KEY_RELEASE:
			event::unsubscribe<KeyReleaseEvent>();
			break;
		case PyEventType::KEY_TYPE:
			event::unsubscribe<KeyTypeEvent>();
			break;
		case PyEventType::MOUSE_MOVE:
			event::unsubscribe<MouseMoveEvent>();
			break;
		case PyEventType::MOUSE_SCROLL:
			event::unsubscribe<MouseScrollEvent>();
			break;
		case PyEventType::MOUSE_PRESS:
			event::unsubscribe<MousePressEvent>();
			break;
		case PyEventType::MOUSE_RELEASE:
			event::unsubscribe<MouseReleaseEvent>();
			break;
	}
}

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

	py::class_<Vec2u>(m, "Vec2u")
			.def(py::init<uint32_t, uint32_t>())
			.def_readwrite("x", &Vec2u::x)
			.def_readwrite("y", &Vec2u::y);

	py::class_<Vec2f>(m, "Vec2f")
			.def(py::init<float, float>())
			.def_readwrite("x", &Vec2f::x)
			.def_readwrite("y", &Vec2f::y);

	py::class_<GpuContext>(m, "GpuContext").def(py::init<>());

	py::class_<Window, py::smart_holder>(m, "Window")
			.def(py::init<GpuContext&, const Vec2u&, const char*>())
			.def("should_close", &Window::should_close)
			.def("get_size", &Window::get_size)
			.def("poll_events", &Window::poll_events);

	m.def("subscribe_event", &_py_subscribe_event);
	m.def("unsubscribe_event", &_py_unsubscribe_event);

	py::class_<RenderingSystem, System, py::smart_holder>(m, "RenderingSystem")
			.def(py::init<GpuContext&, std::shared_ptr<Window>>());
	// TODO: headless .def(py::init<GpuContext&, Image>);
	py::class_<PhysicsSystem, System, py::smart_holder>(m, "PhysicsSystem")
			.def(py::init<GpuContext&>());

	py::native_enum<KeyCode>(m, "KeyCode", "enum.IntEnum")
			.value("UNKNOWN", KeyCode::UNKNOWN)
			.value("RETURN", KeyCode::RETURN)
			.value("ESCAPE", KeyCode::ESCAPE)
			.value("BACKSPACE", KeyCode::BACKSPACE)
			.value("TAB", KeyCode::TAB)
			.value("SPACE", KeyCode::SPACE)
			.value("EXCLAIM", KeyCode::EXCLAIM)
			.value("QUOTEDBL", KeyCode::QUOTEDBL)
			.value("HASH", KeyCode::HASH)
			.value("PERCENT", KeyCode::PERCENT)
			.value("DOLLAR", KeyCode::DOLLAR)
			.value("AMPERSAND", KeyCode::AMPERSAND)
			.value("QUOTE", KeyCode::QUOTE)
			.value("LEFTPAREN", KeyCode::LEFTPAREN)
			.value("RIGHTPAREN", KeyCode::RIGHTPAREN)
			.value("ASTERISK", KeyCode::ASTERISK)
			.value("PLUS", KeyCode::PLUS)
			.value("COMMA", KeyCode::COMMA)
			.value("MINUS", KeyCode::MINUS)
			.value("PERIOD", KeyCode::PERIOD)
			.value("SLASH", KeyCode::SLASH)
			.value("N0", KeyCode::N0)
			.value("N1", KeyCode::N1)
			.value("N2", KeyCode::N2)
			.value("N3", KeyCode::N3)
			.value("N4", KeyCode::N4)
			.value("N5", KeyCode::N5)
			.value("N6", KeyCode::N6)
			.value("N7", KeyCode::N7)
			.value("N8", KeyCode::N8)
			.value("N9", KeyCode::N9)
			.value("COLON", KeyCode::COLON)
			.value("SEMICOLON", KeyCode::SEMICOLON)
			.value("LESS", KeyCode::LESS)
			.value("EQUALS", KeyCode::EQUALS)
			.value("GREATER", KeyCode::GREATER)
			.value("QUESTION", KeyCode::QUESTION)
			.value("AT", KeyCode::AT)
			.value("LEFTBRACKET", KeyCode::LEFTBRACKET)
			.value("BACKSLASH", KeyCode::BACKSLASH)
			.value("RIGHTBRACKET", KeyCode::RIGHTBRACKET)
			.value("CARET", KeyCode::CARET)
			.value("UNDERSCORE", KeyCode::UNDERSCORE)
			.value("BACKQUOTE", KeyCode::BACKQUOTE)
			.value("A", KeyCode::A)
			.value("B", KeyCode::B)
			.value("C", KeyCode::C)
			.value("D", KeyCode::D)
			.value("E", KeyCode::E)
			.value("F", KeyCode::F)
			.value("G", KeyCode::G)
			.value("H", KeyCode::H)
			.value("I", KeyCode::I)
			.value("J", KeyCode::J)
			.value("K", KeyCode::K)
			.value("L", KeyCode::L)
			.value("M", KeyCode::M)
			.value("N", KeyCode::N)
			.value("O", KeyCode::O)
			.value("P", KeyCode::P)
			.value("Q", KeyCode::Q)
			.value("R", KeyCode::R)
			.value("S", KeyCode::S)
			.value("T", KeyCode::T)
			.value("U", KeyCode::U)
			.value("V", KeyCode::V)
			.value("W", KeyCode::W)
			.value("X", KeyCode::X)
			.value("Y", KeyCode::Y)
			.value("Z", KeyCode::Z)
			.value("DELETE", KeyCode::DELETE)
			.value("CAPSLOCK", KeyCode::CAPSLOCK)
			.value("F1", KeyCode::F1)
			.value("F2", KeyCode::F2)
			.value("F3", KeyCode::F3)
			.value("F4", KeyCode::F4)
			.value("F5", KeyCode::F5)
			.value("F6", KeyCode::F6)
			.value("F7", KeyCode::F7)
			.value("F8", KeyCode::F8)
			.value("F9", KeyCode::F9)
			.value("F10", KeyCode::F10)
			.value("F11", KeyCode::F11)
			.value("F12", KeyCode::F12)
			.value("PRINTSCREEN", KeyCode::PRINTSCREEN)
			.value("SCROLLLOCK", KeyCode::SCROLLLOCK)
			.value("PAUSE", KeyCode::PAUSE)
			.value("INSERT", KeyCode::INSERT)
			.value("HOME", KeyCode::HOME)
			.value("PAGEUP", KeyCode::PAGEUP)
			.value("END", KeyCode::END)
			.value("PAGEDOWN", KeyCode::PAGEDOWN)
			.value("ARROW_RIGHT", KeyCode::RIGHT)
			.value("ARROW_LEFT", KeyCode::LEFT)
			.value("ARROW_DOWN", KeyCode::DOWN)
			.value("ARROW_UP", KeyCode::UP)
			.value("NUMLOCKCLEAR", KeyCode::NUMLOCKCLEAR)
			.value("KP_DIVIDE", KeyCode::KP_DIVIDE)
			.value("KP_MULTIPLY", KeyCode::KP_MULTIPLY)
			.value("KP_MINUS", KeyCode::KP_MINUS)
			.value("KP_PLUS", KeyCode::KP_PLUS)
			.value("KP_ENTER", KeyCode::KP_ENTER)
			.value("KP_1", KeyCode::KP_1)
			.value("KP_2", KeyCode::KP_2)
			.value("KP_3", KeyCode::KP_3)
			.value("KP_4", KeyCode::KP_4)
			.value("KP_5", KeyCode::KP_5)
			.value("KP_6", KeyCode::KP_6)
			.value("KP_7", KeyCode::KP_7)
			.value("KP_8", KeyCode::KP_8)
			.value("KP_9", KeyCode::KP_9)
			.value("KP_0", KeyCode::KP_0)
			.value("KP_PERIOD", KeyCode::KP_PERIOD)
			.value("APPLICATION", KeyCode::APPLICATION)
			.value("POWER", KeyCode::POWER)
			.value("KP_EQUALS", KeyCode::KP_EQUALS)
			.value("F13", KeyCode::F13)
			.value("F14", KeyCode::F14)
			.value("F15", KeyCode::F15)
			.value("F16", KeyCode::F16)
			.value("F17", KeyCode::F17)
			.value("F18", KeyCode::F18)
			.value("F19", KeyCode::F19)
			.value("F20", KeyCode::F20)
			.value("F21", KeyCode::F21)
			.value("F22", KeyCode::F22)
			.value("F23", KeyCode::F23)
			.value("F24", KeyCode::F24)
			.value("EXECUTE", KeyCode::EXECUTE)
			.value("HELP", KeyCode::HELP)
			.value("MENU", KeyCode::MENU)
			.value("SELECT", KeyCode::SELECT)
			.value("STOP", KeyCode::STOP)
			.value("AGAIN", KeyCode::AGAIN)
			.value("UNDO", KeyCode::UNDO)
			.value("CUT", KeyCode::CUT)
			.value("COPY", KeyCode::COPY)
			.value("PASTE", KeyCode::PASTE)
			.value("FIND", KeyCode::FIND)
			.value("MUTE", KeyCode::MUTE)
			.value("VOLUMEUP", KeyCode::VOLUMEUP)
			.value("VOLUMEDOWN", KeyCode::VOLUMEDOWN)
			.value("KP_COMMA", KeyCode::KP_COMMA)
			.value("KP_EQUALSAS400", KeyCode::KP_EQUALSAS400)
			.value("ALTERASE", KeyCode::ALTERASE)
			.value("SYSREQ", KeyCode::SYSREQ)
			.value("CANCEL", KeyCode::CANCEL)
			.value("CLEAR", KeyCode::CLEAR)
			.value("PRIOR", KeyCode::PRIOR)
			.value("RETURN2", KeyCode::RETURN2)
			.value("SEPARATOR", KeyCode::SEPARATOR)
			.value("OUT", KeyCode::OUT)
			.value("OPER", KeyCode::OPER)
			.value("CLEARAGAIN", KeyCode::CLEARAGAIN)
			.value("CRSEL", KeyCode::CRSEL)
			.value("EXSEL", KeyCode::EXSEL)
			.value("KP_00", KeyCode::KP_00)
			.value("KP_000", KeyCode::KP_000)
			.value("THOUSANDSSEPARATOR", KeyCode::THOUSANDSSEPARATOR)
			.value("DECIMALSEPARATOR", KeyCode::DECIMALSEPARATOR)
			.value("CURRENCYUNIT", KeyCode::CURRENCYUNIT)
			.value("CURRENCYSUBUNIT", KeyCode::CURRENCYSUBUNIT)
			.value("KP_LEFTPAREN", KeyCode::KP_LEFTPAREN)
			.value("KP_RIGHTPAREN", KeyCode::KP_RIGHTPAREN)
			.value("KP_LEFTBRACE", KeyCode::KP_LEFTBRACE)
			.value("KP_RIGHTBRACE", KeyCode::KP_RIGHTBRACE)
			.value("KP_TAB", KeyCode::KP_TAB)
			.value("KP_BACKSPACE", KeyCode::KP_BACKSPACE)
			.value("KP_A", KeyCode::KP_A)
			.value("KP_B", KeyCode::KP_B)
			.value("KP_C", KeyCode::KP_C)
			.value("KP_D", KeyCode::KP_D)
			.value("KP_E", KeyCode::KP_E)
			.value("KP_F", KeyCode::KP_F)
			.value("KP_XOR", KeyCode::KP_XOR)
			.value("KP_POWER", KeyCode::KP_POWER)
			.value("KP_PERCENT", KeyCode::KP_PERCENT)
			.value("KP_LESS", KeyCode::KP_LESS)
			.value("KP_GREATER", KeyCode::KP_GREATER)
			.value("KP_AMPERSAND", KeyCode::KP_AMPERSAND)
			.value("KP_DBLAMPERSAND", KeyCode::KP_DBLAMPERSAND)
			.value("KP_VERTICALBAR", KeyCode::KP_VERTICALBAR)
			.value("KP_DBLVERTICALBAR", KeyCode::KP_DBLVERTICALBAR)
			.value("KP_COLON", KeyCode::KP_COLON)
			.value("KP_HASH", KeyCode::KP_HASH)
			.value("KP_SPACE", KeyCode::KP_SPACE)
			.value("KP_AT", KeyCode::KP_AT)
			.value("KP_EXCLAM", KeyCode::KP_EXCLAM)
			.value("KP_MEMSTORE", KeyCode::KP_MEMSTORE)
			.value("KP_MEMRECALL", KeyCode::KP_MEMRECALL)
			.value("KP_MEMCLEAR", KeyCode::KP_MEMCLEAR)
			.value("KP_MEMADD", KeyCode::KP_MEMADD)
			.value("KP_MEMSUBTRACT", KeyCode::KP_MEMSUBTRACT)
			.value("KP_MEMMULTIPLY", KeyCode::KP_MEMMULTIPLY)
			.value("KP_MEMDIVIDE", KeyCode::KP_MEMDIVIDE)
			.value("KP_PLUSMINUS", KeyCode::KP_PLUSMINUS)
			.value("KP_CLEAR", KeyCode::KP_CLEAR)
			.value("KP_CLEARENTRY", KeyCode::KP_CLEARENTRY)
			.value("KP_BINARY", KeyCode::KP_BINARY)
			.value("KP_OCTAL", KeyCode::KP_OCTAL)
			.value("KP_DECIMAL", KeyCode::KP_DECIMAL)
			.value("KP_HEXADECIMAL", KeyCode::KP_HEXADECIMAL)
			.value("LEFT_CTRL", KeyCode::LEFT_CTRL)
			.value("LEFT_SHIFT", KeyCode::LEFT_SHIFT)
			.value("LEFT_ALT", KeyCode::LEFT_ALT)
			.value("LEFT_GUI", KeyCode::LEFT_GUI)
			.value("RIGHT_CTRL", KeyCode::RIGHT_CTRL)
			.value("RIGHT_SHIFT", KeyCode::RIGHT_SHIFT)
			.value("RIGHT_ALT", KeyCode::RIGHT_ALT)
			.value("RIGHT_GUI", KeyCode::RIGHT_GUI)
			.value("MODE", KeyCode::MODE)
			.value("AUDIONEXT", KeyCode::AUDIONEXT)
			.value("AUDIOPREV", KeyCode::AUDIOPREV)
			.value("AUDIOSTOP", KeyCode::AUDIOSTOP)
			.value("AUDIOPLAY", KeyCode::AUDIOPLAY)
			.value("AUDIOMUTE", KeyCode::AUDIOMUTE)
			.value("MEDIASELECT", KeyCode::MEDIASELECT)
			.value("WWW", KeyCode::WWW)
			.value("MAIL", KeyCode::MAIL)
			.value("CALCULATOR", KeyCode::CALCULATOR)
			.value("COMPUTER", KeyCode::COMPUTER)
			.value("AC_SEARCH", KeyCode::AC_SEARCH)
			.value("AC_HOME", KeyCode::AC_HOME)
			.value("AC_BACK", KeyCode::AC_BACK)
			.value("AC_FORWARD", KeyCode::AC_FORWARD)
			.value("AC_STOP", KeyCode::AC_STOP)
			.value("AC_REFRESH", KeyCode::AC_REFRESH)
			.value("AC_BOOKMARKS", KeyCode::AC_BOOKMARKS)
			.value("BRIGHTNESSDOWN", KeyCode::BRIGHTNESSDOWN)
			.value("BRIGHTNESSUP", KeyCode::BRIGHTNESSUP)
			.value("DISPLAYSWITCH", KeyCode::DISPLAYSWITCH)
			.value("KBDILLUMTOGGLE", KeyCode::KBDILLUMTOGGLE)
			.value("KBDILLUMDOWN", KeyCode::KBDILLUMDOWN)
			.value("KBDILLUMUP", KeyCode::KBDILLUMUP)
			.value("EJECT", KeyCode::EJECT)
			.value("SLEEP", KeyCode::SLEEP)
			.value("APP1", KeyCode::APP1)
			.value("APP2", KeyCode::APP2)
			.value("AUDIOREWIND", KeyCode::AUDIOREWIND)
			.value("AUDIOFASTFORWARD", KeyCode::AUDIOFASTFORWARD)
			.value("SOFTLEFT", KeyCode::SOFTLEFT)
			.value("SOFTRIGHT", KeyCode::SOFTRIGHT)
			.value("CALL", KeyCode::CALL)
			.value("ENDCALL", KeyCode::ENDCALL)
			.export_values()
			.finalize();

	py::native_enum<MouseButton>(m, "MouseButton", "enum.IntEnum")
			.value("LEFT", MouseButton::LEFT)
			.value("MIDDLE", MouseButton::MIDDLE)
			.value("RIGHT", MouseButton::RIGHT)
			.value("X1", MouseButton::X1)
			.value("X2", MouseButton::X2)
			.export_values()
			.finalize();

	py::native_enum<PyEventType>(m, "EventType", "enum.Enum")
			.value("WINDOW_RESIZE", PyEventType::WINDOW_RESIZE)
			.value("WINDOW_MINIMIZE", PyEventType::WINDOW_MINIMIZE)
			.value("WINDOW_CLOSE", PyEventType::WINDOW_CLOSE)
			.value("KEY_PRESS", PyEventType::KEY_PRESS)
			.value("KEY_RELEASE", PyEventType::KEY_RELEASE)
			.value("KEY_TYPE", PyEventType::KEY_TYPE)
			.value("MOUSE_MOVE", PyEventType::MOUSE_MOVE)
			.value("MOUSE_SCROLL", PyEventType::MOUSE_SCROLL)
			.value("MOUSE_PRESS", PyEventType::MOUSE_PRESS)
			.value("MOUSE_RELEASE", PyEventType::MOUSE_RELEASE)
			.export_values()
			.finalize();

	py::class_<WindowResizeEvent>(m, "WindowResizeEvent")
			.def_readwrite("size", &WindowResizeEvent::size);

	py::class_<WindowMinimizeEvent>(m, "WindowMinimizeEvent").def(py::init<>());

	py::class_<WindowCloseEvent>(m, "WindowCloseEvent").def(py::init<>());

	py::class_<KeyPressEvent>(m, "KeyPressEvent")
			.def_readwrite("key_code", &KeyPressEvent::key_code);

	py::class_<KeyReleaseEvent>(m, "KeyReleaseEvent")
			.def_readwrite("key_code", &KeyReleaseEvent::key_code);

	py::class_<KeyTypeEvent>(m, "KeyTypeEvent").def_readwrite("text", &KeyTypeEvent::text);

	py::class_<MouseMoveEvent>(m, "MouseMoveEvent")
			.def_readwrite("position", &MouseMoveEvent::position);

	py::class_<MouseScrollEvent>(m, "MouseScrollEvent")
			.def_readwrite("offset", &MouseScrollEvent::offset);

	py::class_<MousePressEvent>(m, "MousePressEvent")
			.def_readwrite("button_code", &MousePressEvent::button_code);

	py::class_<MouseReleaseEvent>(m, "MouseReleaseEvent")
			.def_readwrite("button_code", &MouseReleaseEvent::button_code);

	py::class_<Input>(m, "Input")
			.def_static("init", &Input::init)
			.def_static("is_key_pressed_once", &Input::is_key_pressed_once)
			.def_static("is_key_pressed", &Input::is_key_pressed)
			.def_static("is_key_released", &Input::is_key_released)
			.def_static("is_mouse_pressed", &Input::is_mouse_pressed)
			.def_static("is_mouse_released", &Input::is_mouse_released)
			.def_static("get_mouse_position", &Input::get_mouse_position)
			.def_static("get_scroll_offset", &Input::get_scroll_offset);
}

} //namespace gl
