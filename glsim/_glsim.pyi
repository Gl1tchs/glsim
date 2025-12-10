from typing import Union

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


class Vec2u:
    x: int
    y: int

    def __init__(self, x: int, y: int) -> None:
        ...


class GpuContext:
    """
    Class representing the gpu device.
    """

    def __init__(self) -> None:
        ...


class Window:
    def __init__(self, ctx: GpuContext, size: Vec2u, title: str) -> None:
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
