from typing import Union, Callable
from dataclasses import dataclass
from enum import Enum, IntEnum

# Define the C++ primitive type used for entities
# Union[int, Entity] is used to allow Python to pass regular ints
EntityID = Union[int, "Entity"]


class Entity:
    """
    Represents a unique entity identifier (ID) in the ECS registry.

    The ID is a 64-bit integer composed of a 32-bit index and a 32-bit version
    to prevent stale references.
    """

    def __init__(self) -> None:
        """Initializes a default, likely invalid, Entity ID (usually 0)."""
        ...

    @staticmethod
    def create_id(index: int, version: int) -> int:
        """
        Creates a valid 64-bit Entity ID by combining the 32-bit index
        and the 32-bit version.

        Args:
            index: The slot index in the entity array (0 to 2^32 - 1).
            version: The generation counter for the slot.
        """
        ...

    @staticmethod
    def get_index(entity: EntityID) -> int:
        """
        Extracts the 32-bit index portion from the 64-bit Entity ID.

        Args:
            entity: The 64-bit Entity ID.
        """
        ...

    @staticmethod
    def get_version(entity: EntityID) -> int:
        """
        Extracts the 32-bit version portion from the 64-bit Entity ID.

        Args:
            entity: The 64-bit Entity ID.
        """
        ...


class Registry:
    """
    The core component container and manager for the Entity Component System
    (ECS).

    It manages entity creation/destruction and provides access to component
    data pools.
    """

    def __init__(self) -> None:
        """Initializes the ECS Registry with empty component pools."""
        ...

    def clear(self) -> None:
        """
        Destroys all entities and components in the registry, resetting the
        state.
        """
        ...

    def spawn(self) -> EntityID:
        """
        Creates and registers a new, bare Entity, returning its unique ID.
        """
        ...

    def is_valid(self, entity: EntityID) -> bool:
        """
        Checks if the Entity ID is currently valid (i.e., its version matches
        the stored version in the registry).

        Args:
            entity: The 64-bit Entity ID.
        """
        ...

    def despawn(self, entity: EntityID) -> None:
        """
        Removes the Entity from the scene and invalidates its ID by
        incrementing its version counter.

        Args:
            entity: The 64-bit Entity ID to be destroyed.
        """
        ...


class System:
    """
    Base class for all logic and behavior in the ECS.

    Systems query the Registry for entities that possess a specific set of
    components and perform operations on those components.
    This class is intended to be inherited by custom systems.
    """

    def on_init(self, registry: Registry) -> None:
        """
        Called once when the system is added to the World. Use for setup and
        component registration.

        Args:
            registry: The Registry/World instance.
        """
        ...

    def on_update(self, registry: Registry, dt: float) -> None:
        """
        Called every frame to perform the system's logic.

        Args:
            registry: The Registry/World instance.
            dt: The time elapsed since the last frame (delta time).
        """
        ...

    def on_destroy(self, registry: Registry) -> None:
        """
        Called once when the system is removed from the World. Use for cleanup.

        Args:
            registry: The Registry/World instance.
        """
        ...


class World(Registry):
    """
    The main ECS orchestration object, inheriting from Registry and managing
    the execution of all registered Systems.
    """

    def __init__(self) -> None:
        """Initializes the World and the list of systems."""
        super().__init__()
        ...

    def update(self, dt: float = 0.016) -> None:
        """
        Executes the on_update method for all registered systems in order.

        Args:
            dt: The time elapsed since the last frame (defaulting to ~60 FPS).
        """
        ...

    def add_system(self, system: System) -> None:
        """
        Adds a new System to the World's execution loop and calls its on_init
        method.

        Args:
            system: The System instance to add (e.g., PhysicsSystem, PySystem).
        """
        ...


@dataclass
class Vec2u:
    x: int
    y: int


@dataclass
class Vec2f:
    x: float
    y: float


class GpuContext:
    """
    Class representing the gpu device.
    """

    def __init__(self) -> None:
        ...


class Window:
    def __init__(self, ctx: GpuContext, size: Vec2u, title: str) -> None:
        ...

    def should_close(self) -> bool:
        ...

    def get_size(self) -> Vec2u:
        ...

    def poll_events(self) -> None:
        ...


class RenderingSystem(System):
    """
    A concrete System implementation responsible for drawing entities to the
    screen, typically querying position and mesh/material components.
    """

    def __init__(self, gpu: GpuContext, window: Window) -> None:
        ...

    def on_init(self, registry: Registry) -> None:
        ...

    def on_update(self, registry: Registry, dt: float) -> None:
        """Handles culling, batching, and drawing of visible entities."""
        ...

    def on_destroy(self, registry: Registry) -> None:
        ...


class PhysicsSystem(System):
    """
    A concrete System implementation responsible for updating physics component
    like position, velocity, and applying collision detection/response.
    """

    def __init__(self, gpu: GpuContext) -> None:
        ...

    def on_init(self, registry: Registry) -> None:
        ...

    def on_update(self, registry: Registry, dt: float) -> None:
        """Handles integration of forces and collision checks."""
        ...

    def on_destroy(self, registry: Registry) -> None:
        ...


class KeyCode(IntEnum):
    UNKNOWN = 0
    RETURN = 13
    ESCAPE = 27
    BACKSPACE = 8
    TAB = 9
    SPACE = 32
    EXCLAIM = 33
    QUOTEDBL = 34
    HASH = 35
    PERCENT = 37
    DOLLAR = 36
    AMPERSAND = 38
    QUOTE = 39
    LEFTPAREN = 40
    RIGHTPAREN = 41
    ASTERISK = 42
    PLUS = 43
    COMMA = 44
    MINUS = 45
    PERIOD = 46
    SLASH = 47
    N0 = 48
    N1 = 49
    N2 = 50
    N3 = 51
    N4 = 52
    N5 = 53
    N6 = 54
    N7 = 55
    N8 = 56
    N9 = 57
    COLON = 58
    SEMICOLON = 59
    LESS = 60
    EQUALS = 61
    GREATER = 62
    QUESTION = 63
    AT = 64
    LEFTBRACKET = 91
    BACKSLASH = 92
    RIGHTBRACKET = 93
    CARET = 94
    UNDERSCORE = 95
    BACKQUOTE = 96
    A = 97
    B = 98
    C = 99
    D = 100
    E = 101
    F = 102
    G = 103
    H = 104
    I = 105
    J = 106
    K = 107
    L = 108
    M = 109
    N = 110
    O = 111
    P = 112
    Q = 113
    R = 114
    S = 115
    T = 116
    U = 117
    V = 118
    W = 119
    X = 120
    Y = 121
    Z = 122
    DELETE = 127
    CAPSLOCK = 0x40000039
    F1 = 0x4000003a
    F2 = 0x4000003b
    F3 = 0x4000003c
    F4 = 0x4000003d
    F5 = 0x4000003e
    F6 = 0x4000003f
    F7 = 0x40000040
    F8 = 0x40000041
    F9 = 0x40000042
    F10 = 0x40000043
    F11 = 0x40000044
    F12 = 0x40000045
    PRINTSCREEN = 0x40000046
    SCROLLLOCK = 0x40000047
    PAUSE = 0x40000048
    INSERT = 0x40000049
    HOME = 0x4000004a
    PAGEUP = 0x4000004b
    END = 0x4000004d
    PAGEDOWN = 0x4000004e
    ARROW_RIGHT = 0x4000004f
    ARROW_LEFT = 0x40000050
    ARROW_DOWN = 0x40000051
    ARROW_UP = 0x40000052
    NUMLOCKCLEAR = 0x40000053
    KP_DIVIDE = 0x40000054
    KP_MULTIPLY = 0x40000055
    KP_MINUS = 0x40000056
    KP_PLUS = 0x40000057
    KP_ENTER = 0x40000058
    KP_1 = 0x40000059
    KP_2 = 0x4000005a
    KP_3 = 0x4000005b
    KP_4 = 0x4000005c
    KP_5 = 0x4000005d
    KP_6 = 0x4000005e
    KP_7 = 0x4000005f
    KP_8 = 0x40000060
    KP_9 = 0x40000061
    KP_0 = 0x40000062
    KP_PERIOD = 0x40000063
    APPLICATION = 0x40000065
    POWER = 0x40000066
    KP_EQUALS = 0x40000067
    F13 = 0x40000068
    F14 = 0x40000069
    F15 = 0x4000006a
    F16 = 0x4000006b
    F17 = 0x4000006c
    F18 = 0x4000006d
    F19 = 0x4000006e
    F20 = 0x4000006f
    F21 = 0x40000070
    F22 = 0x40000071
    F23 = 0x40000072
    F24 = 0x40000073
    EXECUTE = 0x40000074
    HELP = 0x40000075
    MENU = 0x40000076
    SELECT = 0x40000077
    STOP = 0x40000078
    AGAIN = 0x40000079
    UNDO = 0x4000007a
    CUT = 0x4000007b
    COPY = 0x4000007c
    PASTE = 0x4000007d
    FIND = 0x4000007e
    MUTE = 0x4000007f
    VOLUMEUP = 0x40000080
    VOLUMEDOWN = 0x40000081
    KP_COMMA = 0x40000085
    KP_EQUALSAS400 = 0x40000086
    ALTERASE = 0x40000099
    SYSREQ = 0x4000009a
    CANCEL = 0x4000009b
    CLEAR = 0x4000009c
    PRIOR = 0x4000009d
    RETURN2 = 0x4000009e
    SEPARATOR = 0x4000009f
    OUT = 0x400000a0
    OPER = 0x400000a1
    CLEARAGAIN = 0x400000a2
    CRSEL = 0x400000a3
    EXSEL = 0x400000a4
    KP_00 = 0x400000b0
    KP_000 = 0x400000b1
    THOUSANDSSEPARATOR = 0x400000b2
    DECIMALSEPARATOR = 0x400000b3
    CURRENCYUNIT = 0x400000b4
    CURRENCYSUBUNIT = 0x400000b5
    KP_LEFTPAREN = 0x400000b6
    KP_RIGHTPAREN = 0x400000b7
    KP_LEFTBRACE = 0x400000b8
    KP_RIGHTBRACE = 0x400000b9
    KP_TAB = 0x400000ba
    KP_BACKSPACE = 0x400000bb
    KP_A = 0x400000bc
    KP_B = 0x400000bd
    KP_C = 0x400000be
    KP_D = 0x400000bf
    KP_E = 0x400000c0
    KP_F = 0x400000c1
    KP_XOR = 0x400000c2
    KP_POWER = 0x400000c3
    KP_PERCENT = 0x400000c4
    KP_LESS = 0x400000c5
    KP_GREATER = 0x400000c6
    KP_AMPERSAND = 0x400000c7
    KP_DBLAMPERSAND = 0x400000c8
    KP_VERTICALBAR = 0x400000c9
    KP_DBLVERTICALBAR = 0x400000ca
    KP_COLON = 0x400000cb
    KP_HASH = 0x400000cc
    KP_SPACE = 0x400000cd
    KP_AT = 0x400000ce
    KP_EXCLAM = 0x400000cf
    KP_MEMSTORE = 0x400000d0
    KP_MEMRECALL = 0x400000d1
    KP_MEMCLEAR = 0x400000d2
    KP_MEMADD = 0x400000d3
    KP_MEMSUBTRACT = 0x400000d4
    KP_MEMMULTIPLY = 0x400000d5
    KP_MEMDIVIDE = 0x400000d6
    KP_PLUSMINUS = 0x400000d7
    KP_CLEAR = 0x400000d8
    KP_CLEARENTRY = 0x400000d9
    KP_BINARY = 0x400000da
    KP_OCTAL = 0x400000db
    KP_DECIMAL = 0x400000dc
    KP_HEXADECIMAL = 0x400000dd
    LEFT_CTRL = 0x400000e0
    LEFT_SHIFT = 0x400000e1
    LEFT_ALT = 0x400000e2
    LEFT_GUI = 0x400000e3
    RIGHT_CTRL = 0x400000e4
    RIGHT_SHIFT = 0x400000e5
    RIGHT_ALT = 0x400000e6
    RIGHT_GUI = 0x400000e7
    MODE = 0x40000101
    AUDIONEXT = 0x40000102
    AUDIOPREV = 0x40000103
    AUDIOSTOP = 0x40000104
    AUDIOPLAY = 0x40000105
    AUDIOMUTE = 0x40000106
    MEDIASELECT = 0x40000107
    WWW = 0x40000108
    MAIL = 0x40000109
    CALCULATOR = 0x4000010a
    COMPUTER = 0x4000010b
    AC_SEARCH = 0x4000010c
    AC_HOME = 0x4000010d
    AC_BACK = 0x4000010e
    AC_FORWARD = 0x4000010f
    AC_STOP = 0x40000110
    AC_REFRESH = 0x40000111
    AC_BOOKMARKS = 0x40000112
    BRIGHTNESSDOWN = 0x40000113
    BRIGHTNESSUP = 0x40000114
    DISPLAYSWITCH = 0x40000115
    KBDILLUMTOGGLE = 0x40000116
    KBDILLUMDOWN = 0x40000117
    KBDILLUMUP = 0x40000118
    EJECT = 0x40000119
    SLEEP = 0x4000011a
    APP1 = 0x4000011b
    APP2 = 0x4000011c
    AUDIOREWIND = 0x4000011d
    AUDIOFASTFORWARD = 0x4000011e
    SOFTLEFT = 0x4000011f
    SOFTRIGHT = 0x40000120
    CALL = 0x40000121
    ENDCALL = 0x40000122


class MouseButton(IntEnum):
    LEFT = 1
    MIDDLE = 2
    RIGHT = 3
    X1 = 4
    X2 = 5


class EventType(Enum):
    WINDOW_RESIZE = 0
    WINDOW_MINIMIZE = 1
    WINDOW_CLOSE = 2
    KEY_PRESS = 3
    KEY_RELEASE = 4
    KEY_TYPE = 5
    MOUSE_MOVE = 6
    MOUSE_SCROLL = 7
    MOUSE_PRESS = 8
    MOUSE_RELEASE = 9


class WindowResizeEvent:
    size: Vec2u


class WindowMinimizeEvent:
    def __init__(self) -> None: ...


class WindowCloseEvent:
    def __init__(self) -> None: ...


class KeyPressEvent:
    key_code: KeyCode


class KeyReleaseEvent:
    key_code: KeyCode


class KeyTypeEvent:
    text: str


class MouseMoveEvent:
    position: Vec2f


class MouseScrollEvent:
    offset: Vec2f


class MousePressEvent:
    button_code: MouseButton


class MouseReleaseEvent:
    button_code: MouseButton


def subscribe_event(event_type: EventType,
                    callback: Callable[[object], None]) -> None:
    """Subscribes a Python callable to a C++ event type."""
    ...


def unsubscribe_event(event_type: EventType) -> None:
    """Unsubscribes all Python callables from a C++ event type."""
    ...


class Input:
    """Static utility class for direct input querying."""
    @staticmethod
    def init() -> None:
        """Initializes the Input tracking state."""
        ...

    @staticmethod
    def is_key_pressed_once(key: KeyCode) -> bool:
        """Checks if the key was pressed down in the current frame."""
        ...

    @staticmethod
    def is_key_pressed(key: KeyCode) -> bool:
        """Checks if the key is currently held down."""
        ...

    @staticmethod
    def is_key_released(key: KeyCode) -> bool:
        """Checks if the key was released in the current frame."""
        ...

    @staticmethod
    def is_mouse_pressed(button: MouseButton) -> bool:
        """Checks if the mouse button is currently held down."""
        ...

    @staticmethod
    def is_mouse_released(button: MouseButton) -> bool:
        """Checks if the mouse button was released in the current frame."""
        ...

    @staticmethod
    def get_mouse_position() -> Vec2f:
        """Returns the current mouse cursor position."""
        ...

    @staticmethod
    def get_scroll_offset() -> Vec2f:
        """Returns the mouse scroll wheel delta for the current frame."""
        ...
