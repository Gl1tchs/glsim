from __future__ import annotations

from ._glsim import (
    Entity,
    Registry,
    System,
    World,
    Vec2u,
    GpuContext,
    Window,
    RenderingSystem,
    PhysicsSystem,
)

__version__ = "0.1.0"
__doc__ = "High performance simulation engine."

__all__ = [
    "__doc__",
    "__version__",
    "Entity",
    "Registry",
    "System",
    "World",
    "Vec2u",
    "GpuContext",
    "Window",
    "RenderingSystem",
    "PhysicsSystem",
]
