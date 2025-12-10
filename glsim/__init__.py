from __future__ import annotations

from ._glsim import (
    GpuContext,
    Entity,
    Registry,
    System,
    PhysicsSystem,
    RenderingSystem,
    World
)

__version__ = "0.1.0"
__doc__ = "High performance simulation engine."

__all__ = [
    "__doc__",
    "__version__",
    "GpuContext",
    "Entity",
    "Registry",
    "System",
    "PhysicsSystem",
    "RenderingSystem",
    "World",
]
