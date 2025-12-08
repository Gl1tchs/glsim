# import os
from glsim import World, System, RenderingSystem, PhysicsSystem


class MySystem(System):
    def on_init(self, registry):
        print("my system initialized")

    def on_update(self, registry, dt):
        print("my system updated")

    def on_destroy(self, registry):
        print("my system destroyed")


# wait for debugger
# print(os.getpid(), end="")
# input()

world = World()

world.add_system(MySystem())
world.add_system(RenderingSystem())
world.add_system(PhysicsSystem())

world.update()

# if we do not delete the world MySystem::on_destroy would not get called
del world
