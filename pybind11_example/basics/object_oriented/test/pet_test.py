import pet

print("struct Pet:")
p = pet.Pet("Molly")

print(p.getName())
p.setName("Charly")
print(p.getName())
print(p)  # call __repr__()

print(p.name)
p.name = "Bella"
print(p.name)
print()

print("class Pet2:")
p2 = pet.Pet2("Molly2")
p2.name = "Charly2"
print(p2.name)

p2.age = 2
print(p2.age)
print(p2.__dict__)
print()

print("struct Dog: public Pet :")
d = pet.Dog("Molly")
print(d.name)
print(d.bark())
print()

print("pet_store:")
d = pet.pet_store()
type(d)
print(d.bark())
print()

print("struct PetO (overloaded set):")
po = pet.PetO("Rex", 3)
po.set(7)         # int overload    -> sets age
po.set("Buddy")   # string overload -> sets name
print("PetO created, set(int) and set(str) both dispatched without error")
print()

print("struct Widget (const-overloaded foo):")
w = pet.Widget()
print("foo_mutable(3, 2.5) =", w.foo_mutable(3, 2.5))  # non-const: 3 + int(2.5) = 5
print("foo_const(3, 2.5)   =", w.foo_const(3, 2.5))    # const:     3 * int(2.5) = 6
