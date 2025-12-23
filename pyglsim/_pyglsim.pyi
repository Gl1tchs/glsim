from typing import TypeVar, Union, Callable
from dataclasses import dataclass
from enum import Enum, IntEnum

_T = TypeVar("_T")

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


@dataclass
class Vec2u:
    x: int
    y: int


@dataclass
class Vec2f:
    x: float
    y: float

    def dot(self) -> float:
        ...

    def length(self) -> float:
        ...

@dataclass
class Vec3u:
    x: int
    y: int
    z: int


@dataclass
class Vec3f:
    x: float
    y: float
    z: float

    def dot(self) -> float:
        ...

    def cross(self) -> Vec3f:
        ...

    def length(self) -> float:
        ...

class PrimitiveType(IntEnum):
    CUBE = 0
    PLANE = 1
    SPHERE = 2

class Transform:
    position: Vec3f
    rotation: Vec3f
    scale: Vec3f

    def __init__(self) -> None: ...

    def translate(self, translation: Vec3f) -> None:
        """
        Translate the transform by given translation
        """
        ...

    def rotate(self, angle: float, axis: Vec3f) -> None:
        """
        Rotate transform by given eular angles
        """
        ...

    def get_forward(self) -> Vec3f:
        """
        Get forward vector
        """
        ...

    def get_right(self) -> Vec3f:
        """
        Get right vector
        """
        ...
    
    def get_up(self) -> Vec3f:
        """
        Get up vector
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

    def get_transform(self, entity: EntityID) -> Transform:
        """
        Get transform component of an entity
        
        Args:
            entity: Owner of the transform component
        """
        ...

    def add_mesh(self, entity: EntityID, prim: PrimitiveType = PrimitiveType.CUBE) -> None:
        """
        Adds mesh component to the given entity.

        Args:
            entity: Entity to add component to
            prim: Type of the mesh being added
        """
        ...

class GpuContext:
    """
    Class representing the gpu device.
    """

    def __init__(self) -> None: ...

class Window:
    def __init__(self, ctx: GpuContext, size: Vec2u, title: str) -> None: ...
    def should_close(self) -> bool: ...
    def get_size(self) -> Vec2u: ...
    def poll_events(self) -> None: ...

class RenderingSystem(System):
    """
    A concrete System implementation responsible for drawing entities to the
    screen, typically querying position and mesh/material components.
    """

    def __init__(self, gpu: GpuContext, window: Window) -> None: ...
    def on_init(self, registry: Registry) -> None: ...
    def on_update(self, registry: Registry, dt: float) -> None:
        """Handles culling, batching, and drawing of visible entities."""
        ...

    def on_destroy(self, registry: Registry) -> None: ...

class PhysicsSystem(System):
    """
    A concrete System implementation responsible for updating physics component
    like position, velocity, and applying collision detection/response.
    """

    def __init__(self, gpu: GpuContext) -> None: ...
    def on_init(self, registry: Registry) -> None: ...
    def on_update(self, registry: Registry, dt: float) -> None:
        """Handles integration of forces and collision checks."""
        ...

    def on_destroy(self, registry: Registry) -> None: ...

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
    F1 = 0x4000003A
    F2 = 0x4000003B
    F3 = 0x4000003C
    F4 = 0x4000003D
    F5 = 0x4000003E
    F6 = 0x4000003F
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
    HOME = 0x4000004A
    PAGEUP = 0x4000004B
    END = 0x4000004D
    PAGEDOWN = 0x4000004E
    ARROW_RIGHT = 0x4000004F
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
    KP_2 = 0x4000005A
    KP_3 = 0x4000005B
    KP_4 = 0x4000005C
    KP_5 = 0x4000005D
    KP_6 = 0x4000005E
    KP_7 = 0x4000005F
    KP_8 = 0x40000060
    KP_9 = 0x40000061
    KP_0 = 0x40000062
    KP_PERIOD = 0x40000063
    APPLICATION = 0x40000065
    POWER = 0x40000066
    KP_EQUALS = 0x40000067
    F13 = 0x40000068
    F14 = 0x40000069
    F15 = 0x4000006A
    F16 = 0x4000006B
    F17 = 0x4000006C
    F18 = 0x4000006D
    F19 = 0x4000006E
    F20 = 0x4000006F
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
    UNDO = 0x4000007A
    CUT = 0x4000007B
    COPY = 0x4000007C
    PASTE = 0x4000007D
    FIND = 0x4000007E
    MUTE = 0x4000007F
    VOLUMEUP = 0x40000080
    VOLUMEDOWN = 0x40000081
    KP_COMMA = 0x40000085
    KP_EQUALSAS400 = 0x40000086
    ALTERASE = 0x40000099
    SYSREQ = 0x4000009A
    CANCEL = 0x4000009B
    CLEAR = 0x4000009C
    PRIOR = 0x4000009D
    RETURN2 = 0x4000009E
    SEPARATOR = 0x4000009F
    OUT = 0x400000A0
    OPER = 0x400000A1
    CLEARAGAIN = 0x400000A2
    CRSEL = 0x400000A3
    EXSEL = 0x400000A4
    KP_00 = 0x400000B0
    KP_000 = 0x400000B1
    THOUSANDSSEPARATOR = 0x400000B2
    DECIMALSEPARATOR = 0x400000B3
    CURRENCYUNIT = 0x400000B4
    CURRENCYSUBUNIT = 0x400000B5
    KP_LEFTPAREN = 0x400000B6
    KP_RIGHTPAREN = 0x400000B7
    KP_LEFTBRACE = 0x400000B8
    KP_RIGHTBRACE = 0x400000B9
    KP_TAB = 0x400000BA
    KP_BACKSPACE = 0x400000BB
    KP_A = 0x400000BC
    KP_B = 0x400000BD
    KP_C = 0x400000BE
    KP_D = 0x400000BF
    KP_E = 0x400000C0
    KP_F = 0x400000C1
    KP_XOR = 0x400000C2
    KP_POWER = 0x400000C3
    KP_PERCENT = 0x400000C4
    KP_LESS = 0x400000C5
    KP_GREATER = 0x400000C6
    KP_AMPERSAND = 0x400000C7
    KP_DBLAMPERSAND = 0x400000C8
    KP_VERTICALBAR = 0x400000C9
    KP_DBLVERTICALBAR = 0x400000CA
    KP_COLON = 0x400000CB
    KP_HASH = 0x400000CC
    KP_SPACE = 0x400000CD
    KP_AT = 0x400000CE
    KP_EXCLAM = 0x400000CF
    KP_MEMSTORE = 0x400000D0
    KP_MEMRECALL = 0x400000D1
    KP_MEMCLEAR = 0x400000D2
    KP_MEMADD = 0x400000D3
    KP_MEMSUBTRACT = 0x400000D4
    KP_MEMMULTIPLY = 0x400000D5
    KP_MEMDIVIDE = 0x400000D6
    KP_PLUSMINUS = 0x400000D7
    KP_CLEAR = 0x400000D8
    KP_CLEARENTRY = 0x400000D9
    KP_BINARY = 0x400000DA
    KP_OCTAL = 0x400000DB
    KP_DECIMAL = 0x400000DC
    KP_HEXADECIMAL = 0x400000DD
    LEFT_CTRL = 0x400000E0
    LEFT_SHIFT = 0x400000E1
    LEFT_ALT = 0x400000E2
    LEFT_GUI = 0x400000E3
    RIGHT_CTRL = 0x400000E4
    RIGHT_SHIFT = 0x400000E5
    RIGHT_ALT = 0x400000E6
    RIGHT_GUI = 0x400000E7
    MODE = 0x40000101
    AUDIONEXT = 0x40000102
    AUDIOPREV = 0x40000103
    AUDIOSTOP = 0x40000104
    AUDIOPLAY = 0x40000105
    AUDIOMUTE = 0x40000106
    MEDIASELECT = 0x40000107
    WWW = 0x40000108
    MAIL = 0x40000109
    CALCULATOR = 0x4000010A
    COMPUTER = 0x4000010B
    AC_SEARCH = 0x4000010C
    AC_HOME = 0x4000010D
    AC_BACK = 0x4000010E
    AC_FORWARD = 0x4000010F
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
    SLEEP = 0x4000011A
    APP1 = 0x4000011B
    APP2 = 0x4000011C
    AUDIOREWIND = 0x4000011D
    AUDIOFASTFORWARD = 0x4000011E
    SOFTLEFT = 0x4000011F
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

def subscribe_event(event_type: EventType, callback: Callable[[_T], None]) -> None:
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
