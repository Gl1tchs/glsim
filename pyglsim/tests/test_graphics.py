import os
import sys
import time

from pyglsim import (
    Vec2u,
    Vec3f,
    PrimitiveType,
    World,
    GpuContext,
    Window,
    RenderingSystem,
    KeyCode,
    WindowMinimizeEvent,
    EventType,
    subscribe_event,
    Input,
)


def on_window_minimize(_: WindowMinimizeEvent) -> None:
    print("Window Minimized")


def main() -> None:
    gpu = GpuContext()

    world = World()

    window = None
    if "--headless" not in sys.argv:
        window = Window(gpu, Vec2u(800, 600), "Hello pyglsim")
        world.add_system(RenderingSystem(gpu, window))

        subscribe_event(EventType.WINDOW_MINIMIZE, on_window_minimize)

    # Setup camera
    c = world.spawn()

    world.get_transform(c).position.z = 5
    world.get_camera(c)

    # Create a cube
    e = world.spawn()

    mc = world.get_mesh(e)
    mc.primitive_type = PrimitiveType.SPHERE

    t = world.get_transform(e)

    last = time.time()
    while True:
        if window and window.should_close():
            break

        curr = time.time()
        dt = curr - last

        if window:
            window.poll_events()

            if Input.is_key_pressed(KeyCode.SPACE):
                print("Hello World")

        t.rotate(20 * dt, Vec3f(0, 1, 0))
        t.rotate(30 * dt, Vec3f(1, 0, 0))

        world.update(dt)

        last = curr


if __name__ == "__main__":
    if "-d" in sys.argv or "--debug" in sys.argv:
        # wait for debugger
        print(os.getpid(), end="")
        input()

    main()
