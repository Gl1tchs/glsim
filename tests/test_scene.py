import unittest

from glsim import Registry, Entity


class TestScene(unittest.TestCase):
    def test_registry(self) -> None:
        reg = Registry()

        e1 = reg.spawn()
        self.assertEqual(Entity.get_index(e1), 0)

        e2 = reg.spawn()
        self.assertEqual(Entity.get_index(e2), 1)

        self.assertTrue(reg.is_valid(e1))

        reg.despawn(e1)
        self.assertFalse(reg.is_valid(e1))
