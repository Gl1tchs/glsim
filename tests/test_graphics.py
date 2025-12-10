import os
import sys
import time

from glsim import World, Vec2u, GpuContext, Window, RenderingSystem


def main() -> None:
    gpu = GpuContext()

    size = Vec2u(800, 600)
    window = Window(gpu, size, "Hello Glsim")

    world = World()

    world.add_system(RenderingSystem(gpu, window))

    last = time.time()
    while True:
        curr = time.time()

        dt = curr - last

        world.update(dt)

        last = curr


if __name__ == "__main__":
    if "-d" in sys.argv or "--debug" in sys.argv:
        # wait for debugger
        print(os.getpid(), end="")
        input()

    main()
