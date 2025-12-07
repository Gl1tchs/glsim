from typing import Union

import numpy as np

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

    def get_position(self, entity: EntityID) -> np.ndarray: ...
    def assign_position(self, entity: EntityID,
                        position: np.ndarray) -> None: ...

    def get_velocity(self, entity: EntityID) -> np.ndarray: ...
    def assign_velocity(self, entity: EntityID,
                        velocity: np.ndarray) -> None: ...


class PhysicsSystem:
    @staticmethod
    def update(registry: Registry, dt: float) -> None:
        """
        Updates the physics loop

        Args:
            registry: Registry to loop on
            dt: Delta time step
        """
        ...
