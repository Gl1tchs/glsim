import unittest

import numpy as np

from glsim import Registry, PhysicsSystem


class TestPhysicsSystem(unittest.TestCase):
    def test_physics_system(self):
        reg = Registry()

        e = reg.spawn()

        reg.assign_position(e, [0, 0, 0])
        reg.assign_velocity(e, [1, 0, 0])

        p = reg.get_position(e)
        self.assertTrue(np.array_equal(p, [0, 0, 0]))

        v = reg.get_velocity(e)
        self.assertTrue(np.array_equal(v, [1, 0, 0]))

        PhysicsSystem.update(reg, 1.0)

        self.assertTrue(np.array_equal(p, [1, 0, 0]))
