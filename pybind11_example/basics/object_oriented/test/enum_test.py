import enum as _stdlib_enum
import pet_enum

print("module pet_enum (native_enum + nested Attributes class):")
# Nested enum exposed as both pet_enum.Pet.Kind and (via export_values) pet_enum.Pet.Dog / .Cat
print("  Pet.Kind          :", pet_enum.Pet.Kind)
print("  Pet.Kind.Dog      :", pet_enum.Pet.Kind.Dog, "-> .value =", pet_enum.Pet.Kind.Dog.value)
print("  Pet.Kind.Cat      :", pet_enum.Pet.Kind.Cat, "-> .value =", pet_enum.Pet.Kind.Cat.value)
print("  Pet.Dog (exported):", pet_enum.Pet.Dog)
print("  isinstance(Pet.Dog, enum.Enum):", isinstance(pet_enum.Pet.Dog, _stdlib_enum.Enum))

# Construct a Pet, exercise nested Attributes
pe = pet_enum.Pet("Molly", pet_enum.Pet.Kind.Dog)
print("  pet.name =", pe.name, " pet.type =", pe.type)
pe.attr.age = 3.5
print("  pet.attr.age =", pe.attr.age)

# Standalone Attributes instance
a = pet_enum.Pet.Attributes()
a.age = 7.0
print("  standalone Attributes(age=", a.age, ")", sep="")
