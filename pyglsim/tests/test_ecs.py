import os
import sys
import unittest

from pyglsim import World, System


class MySystem(System):
    def on_init(self, registry):
        print("my system initialized")

    def on_update(self, registry, dt):
        print("my system updated")

    def on_destroy(self, registry):
        print("my system destroyed")


class TestECS(unittest.TestCase):
    def test_ecs(self):
        world = World()

        world.add_system(MySystem())

        world.update()

        # if we not delete the world MySystem::on_destroy would not get called
        del world


if __name__ == "__main__":
    if "-d" in sys.argv or "--debug" in sys.argv:
        # wait for debugger
        print(os.getpid(), end="")
        input()

    TestECS().test_ecs()
