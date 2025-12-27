#pragma once

#include "core/event_system.h"

namespace gl {

class Input {
public:
	static void init();

	static bool is_key_pressed_once(KeyCode key);

	static bool is_key_pressed(KeyCode key);

	static bool is_key_released(KeyCode key);

	static bool is_mouse_pressed(MouseButton button);

	static bool is_mouse_released(MouseButton button);

	static Vec2f get_mouse_position();

	static Vec2f get_scroll_offset();
};

} //namespace gl
