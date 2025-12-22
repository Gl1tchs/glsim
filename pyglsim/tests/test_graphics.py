import os
import sys
import time

from pyglsim import (
    World,
    Vec2u,
    GpuContext,
    Window,
    RenderingSystem,
    KeyCode,
    Input,
    WindowMinimizeEvent,
    EventType,
    subscribe_event,
)


def on_window_minimize(e: WindowMinimizeEvent):
    print("Window Minimized")


def main() -> None:
    gpu = GpuContext()

    world = World()

    window = None
    if "--headless" not in sys.argv:
        window = Window(gpu, Vec2u(800, 600), "Hello pyglsim")
        world.add_system(RenderingSystem(gpu, window))

        subscribe_event(EventType.WINDOW_MINIMIZE, on_window_minimize)

    e = world.spawn()
    world.add_mesh(e)

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

        world.update(dt)

        last = curr


if __name__ == "__main__":
    if "-d" in sys.argv or "--debug" in sys.argv:
        # wait for debugger
        print(os.getpid(), end="")
        input()

    main()
