import os
import sys
import unittest

from pyglsim import GpuContext, Registry, PhysicsSystem


class TestPhysicsSystem(unittest.TestCase):
    def setUp(self):
        self.gpu = GpuContext()
        self.reg = Registry()
        self.physics = PhysicsSystem(self.gpu)

        self.physics.on_init(self.reg)

    def tearDown(self):
        self.physics.on_destroy(self.reg)

    def test_physics_system(self):
        self.physics.on_update(self.reg, 0.016)


if __name__ == "__main__":
    if "-d" in sys.argv or "--debug" in sys.argv:
        print(os.getpid(), end="")
        input()

    unittest.main()
