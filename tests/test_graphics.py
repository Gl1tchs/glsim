import os
import sys
import time

from glsim import GpuContext, World, RenderingSystem


def main() -> None:
    gpu = GpuContext()

    world = World()

    world.add_system(RenderingSystem(gpu))

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
