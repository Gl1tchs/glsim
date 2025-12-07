from glsim import Registry, Entity

reg = Registry()

e1 = reg.spawn()
print(Entity.get_index(e1))

e2 = reg.spawn()
print(Entity.get_index(e2))

print(reg.is_valid(e1))

reg.despawn(e1)
print(reg.is_valid(e1))
