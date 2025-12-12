#pragma once

#include "glgpu/vec.h"

namespace gl {

/**
 * SDL2 Virtual Key Representation
 *
 * Values of this type are used to represent keyboard keys using the current
 * layout of the keyboard. This enum maps directly to SDL_KeyCode in SDL2.
 *
 * Note: Scancode-derived values are explicitly written out to bypass the need
 * for the SDL_SCANCODE_TO_KEYCODE macro.
 */
enum class KeyCode : int {
	UNKNOWN = 0,

	// ASCII / Control Keycodes (Raw char values)
	RETURN = '\r',
	ESCAPE = '\x1B',
	BACKSPACE = '\b',
	TAB = '\t',
	SPACE = ' ',
	EXCLAIM = '!',
	QUOTEDBL = '"',
	HASH = '#',
	PERCENT = '%',
	DOLLAR = '$',
	AMPERSAND = '&',
	QUOTE = '\'',
	LEFTPAREN = '(',
	RIGHTPAREN = ')',
	ASTERISK = '*',
	PLUS = '+',
	COMMA = ',',
	MINUS = '-',
	PERIOD = '.',
	SLASH = '/',
	N0 = '0',
	N1 = '1',
	N2 = '2',
	N3 = '3',
	N4 = '4',
	N5 = '5',
	N6 = '6',
	N7 = '7',
	N8 = '8',
	N9 = '9',
	COLON = ':',
	SEMICOLON = ';',
	LESS = '<',
	EQUALS = '=',
	GREATER = '>',
	QUESTION = '?',
	AT = '@',

	LEFTBRACKET = '[',
	BACKSLASH = '\\',
	RIGHTBRACKET = ']',
	CARET = '^',
	UNDERSCORE = '_',
	BACKQUOTE = '`',

	// Lowercase Letters (Raw char values)
	A = 'a',
	B = 'b',
	C = 'c',
	D = 'd',
	E = 'e',
	F = 'f',
	G = 'g',
	H = 'h',
	I = 'i',
	J = 'j',
	K = 'k',
	L = 'l',
	M = 'm',
	N = 'n',
	O = 'o',
	P = 'p',
	Q = 'q',
	R = 'r',
	S = 's',
	T = 't',
	U = 'u',
	V = 'v',
	W = 'w',
	X = 'x',
	Y = 'y',
	Z = 'z',

	// DEL key is an ASCII value, grouped here
	DELETE = '\x7F',

	// Scancode-mapped Keycodes (Base value is SDL_SCANCODE_MASK | SDL_SCANCODE_X)
	// SDL_SCANCODE_MASK is (1 << 30) or 0x40000000. SDL_SCANCODE values start at 4 and increase.
	// The following values are based on the common SDL_Scancode numbering used in SDL2 headers
	// where SDL_SCANCODE_CAPSLOCK (4 * 1) is typically 57 (0x39)
	// and SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_X) = (0x40000000 | X)

	CAPSLOCK = 0x40000039,

	F1 = 0x4000003a,
	F2 = 0x4000003b,
	F3 = 0x4000003c,
	F4 = 0x4000003d,
	F5 = 0x4000003e,
	F6 = 0x4000003f,
	F7 = 0x40000040,
	F8 = 0x40000041,
	F9 = 0x40000042,
	F10 = 0x40000043,
	F11 = 0x40000044,
	F12 = 0x40000045,

	PRINTSCREEN = 0x40000046,
	SCROLLLOCK = 0x40000047,
	PAUSE = 0x40000048,
	INSERT = 0x40000049,
	HOME = 0x4000004a,
	PAGEUP = 0x4000004b,
	END = 0x4000004d,
	PAGEDOWN = 0x4000004e,
	RIGHT = 0x4000004f,
	LEFT = 0x40000050,
	DOWN = 0x40000051,
	UP = 0x40000052,

	// Keypad (KP)
	NUMLOCKCLEAR = 0x40000053,
	KP_DIVIDE = 0x40000054,
	KP_MULTIPLY = 0x40000055,
	KP_MINUS = 0x40000056,
	KP_PLUS = 0x40000057,
	KP_ENTER = 0x40000058,
	KP_1 = 0x40000059,
	KP_2 = 0x4000005a,
	KP_3 = 0x4000005b,
	KP_4 = 0x4000005c,
	KP_5 = 0x4000005d,
	KP_6 = 0x4000005e,
	KP_7 = 0x4000005f,
	KP_8 = 0x40000060,
	KP_9 = 0x40000061,
	KP_0 = 0x40000062,
	KP_PERIOD = 0x40000063,

	APPLICATION = 0x40000065,
	POWER = 0x40000066,
	KP_EQUALS = 0x40000067,

	F13 = 0x40000068,
	F14 = 0x40000069,
	F15 = 0x4000006a,
	F16 = 0x4000006b,
	F17 = 0x4000006c,
	F18 = 0x4000006d,
	F19 = 0x4000006e,
	F20 = 0x4000006f,
	F21 = 0x40000070,
	F22 = 0x40000071,
	F23 = 0x40000072,
	F24 = 0x40000073,

	EXECUTE = 0x40000074,
	HELP = 0x40000075,
	MENU = 0x40000076,
	SELECT = 0x40000077,
	STOP = 0x40000078,
	AGAIN = 0x40000079,
	UNDO = 0x4000007a,
	CUT = 0x4000007b,
	COPY = 0x4000007c,
	PASTE = 0x4000007d,
	FIND = 0x4000007e,
	MUTE = 0x4000007f, // Replaced by AUDIOMUTE in the list below, but keeping for direct mapping
	VOLUMEUP = 0x40000080, // Replaced by MUTE above
	VOLUMEDOWN = 0x40000081, // Replaced by VOLUMEUP above

	KP_COMMA = 0x40000085,
	KP_EQUALSAS400 = 0x40000086,

	ALTERASE = 0x40000099,
	SYSREQ = 0x4000009a,
	CANCEL = 0x4000009b,
	CLEAR = 0x4000009c,
	PRIOR = 0x4000009d,
	RETURN2 = 0x4000009e,
	SEPARATOR = 0x4000009f,
	OUT = 0x400000a0,
	OPER = 0x400000a1,
	CLEARAGAIN = 0x400000a2,
	CRSEL = 0x400000a3,
	EXSEL = 0x400000a4,

	KP_00 = 0x400000b0,
	KP_000 = 0x400000b1,
	THOUSANDSSEPARATOR = 0x400000b2,
	DECIMALSEPARATOR = 0x400000b3,
	CURRENCYUNIT = 0x400000b4,
	CURRENCYSUBUNIT = 0x400000b5,
	KP_LEFTPAREN = 0x400000b6,
	KP_RIGHTPAREN = 0x400000b7,
	KP_LEFTBRACE = 0x400000b8,
	KP_RIGHTBRACE = 0x400000b9,
	KP_TAB = 0x400000ba,
	KP_BACKSPACE = 0x400000bb,
	KP_A = 0x400000bc,
	KP_B = 0x400000bd,
	KP_C = 0x400000be,
	KP_D = 0x400000bf,
	KP_E = 0x400000c0,
	KP_F = 0x400000c1,
	KP_XOR = 0x400000c2,
	KP_POWER = 0x400000c3,
	KP_PERCENT = 0x400000c4,
	KP_LESS = 0x400000c5,
	KP_GREATER = 0x400000c6,
	KP_AMPERSAND = 0x400000c7,
	KP_DBLAMPERSAND = 0x400000c8,
	KP_VERTICALBAR = 0x400000c9,
	KP_DBLVERTICALBAR = 0x400000ca,
	KP_COLON = 0x400000cb,
	KP_HASH = 0x400000cc,
	KP_SPACE = 0x400000cd,
	KP_AT = 0x400000ce,
	KP_EXCLAM = 0x400000cf,
	KP_MEMSTORE = 0x400000d0,
	KP_MEMRECALL = 0x400000d1,
	KP_MEMCLEAR = 0x400000d2,
	KP_MEMADD = 0x400000d3,
	KP_MEMSUBTRACT = 0x400000d4,
	KP_MEMMULTIPLY = 0x400000d5,
	KP_MEMDIVIDE = 0x400000d6,
	KP_PLUSMINUS = 0x400000d7,
	KP_CLEAR = 0x400000d8,
	KP_CLEARENTRY = 0x400000d9,
	KP_BINARY = 0x400000da,
	KP_OCTAL = 0x400000db,
	KP_DECIMAL = 0x400000dc,
	KP_HEXADECIMAL = 0x400000dd,

	// Modifiers (Expanded names)
	LEFT_CTRL = 0x400000e0,
	LEFT_SHIFT = 0x400000e1,
	LEFT_ALT = 0x400000e2,
	LEFT_GUI = 0x400000e3,
	RIGHT_CTRL = 0x400000e4,
	RIGHT_SHIFT = 0x400000e5,
	RIGHT_ALT = 0x400000e6,
	RIGHT_GUI = 0x400000e7,

	MODE = 0x40000101,

	// Multimedia and App Control
	AUDIONEXT = 0x40000102,
	AUDIOPREV = 0x40000103,
	AUDIOSTOP = 0x40000104,
	AUDIOPLAY = 0x40000105,
	AUDIOMUTE = 0x40000106,
	MEDIASELECT = 0x40000107,
	WWW = 0x40000108,
	MAIL = 0x40000109,
	CALCULATOR = 0x4000010a,
	COMPUTER = 0x4000010b,
	AC_SEARCH = 0x4000010c,
	AC_HOME = 0x4000010d,
	AC_BACK = 0x4000010e,
	AC_FORWARD = 0x4000010f,
	AC_STOP = 0x40000110,
	AC_REFRESH = 0x40000111,
	AC_BOOKMARKS = 0x40000112,

	BRIGHTNESSDOWN = 0x40000113,
	BRIGHTNESSUP = 0x40000114,
	DISPLAYSWITCH = 0x40000115,
	KBDILLUMTOGGLE = 0x40000116,
	KBDILLUMDOWN = 0x40000117,
	KBDILLUMUP = 0x40000118,
	EJECT = 0x40000119,
	SLEEP = 0x4000011a,
	APP1 = 0x4000011b,
	APP2 = 0x4000011c,

	AUDIOREWIND = 0x4000011d,
	AUDIOFASTFORWARD = 0x4000011e,

	SOFTLEFT = 0x4000011f,
	SOFTRIGHT = 0x40000120,
	CALL = 0x40000121,
	ENDCALL = 0x40000122
};

/**
 * SDL2 Mouse Button Codes
 *
 * These values correspond to the button field in SDL_MouseButtonEvent
 * (event.button.button).
 */
enum class MouseButton : unsigned char {
	LEFT = 1,
	MIDDLE = 2,
	RIGHT = 3,
	X1 = 4, // Often mapped to "Back" on gaming mice
	X2 = 5 // Often mapped to "Forward" on gaming mice
};

struct KeyPressEvent {
	KeyCode key_code;
};

struct KeyReleaseEvent {
	KeyCode key_code;
};

struct KeyTypeEvent {
	const char* text;
};

struct MouseMoveEvent {
	Vec2f position;
};

struct MouseScrollEvent {
	Vec2f offset;
};

struct MousePressEvent {
	MouseButton button_code;
};

struct MouseReleaseEvent {
	MouseButton button_code;
};

struct WindowResizeEvent {
	Vec2u size;
};

struct WindowMinimizeEvent {};

struct WindowCloseEvent {};

namespace event {

template <typename T> using EventCallbackFunc = std::function<void(const T&)>;

template <typename T> inline auto g_callbacks = std::vector<EventCallbackFunc<T>>();

template <typename T> inline void subscribe(const EventCallbackFunc<T>& p_callback) {
	g_callbacks<T>.push_back(p_callback);
}

template <typename T> inline void unsubscribe() { g_callbacks<T>.clear(); }

template <typename T> inline void pop() { g_callbacks<T>.pop_back(); }

template <typename T> inline void notify(T p_event) {
	for (const auto& callback : g_callbacks<T>) {
		callback(p_event);
	}
}

} //namespace event

} //namespace gl
