import os
import copy
import unittest

import numpy as np

from glsim import Registry, PhysicsSystem


class TestPhysicsSystem(unittest.TestCase):
    def setUp(self):
        PhysicsSystem.init()

    def tearDown(self):
        PhysicsSystem.shutdown()

    def test_physics_system(self):
        reg = Registry()

        e = reg.spawn()

        reg.assign_position(e, [0, 0, 0])
        reg.assign_velocity(e, [1, 0, 0])

        p = reg.get_position(e)
        p_before = copy.deepcopy(p)

        self.assertTrue(np.array_equal(p, [0, 0, 0]))

        v = reg.get_velocity(e)
        self.assertTrue(np.array_equal(v, [1, 0, 0]))

        PhysicsSystem.update(reg)

        # it should be incremented by 1 * 1/60
        self.assertTrue(np.linalg.norm(p) > np.linalg.norm(p_before))


if __name__ == "__main__":
    print(os.getpid(), end="")
    input()

    reg = Registry()

    e = reg.spawn()

    reg.assign_position(e, [0, 0, 0])
    reg.assign_velocity(e, [1, 0, 0])

    PhysicsSystem.init()

    try:
        while True:
            p = reg.get_position(e)

            PhysicsSystem.update(reg)

            v = reg.get_velocity(e)
            print(p)
    except KeyboardInterrupt:
        PhysicsSystem.shutdown()
