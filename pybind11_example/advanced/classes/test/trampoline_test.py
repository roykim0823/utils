import trampoline

print("=== Virtual function overrides (trampoline classes) ===\n")

# --- 1. PYBIND11_OVERRIDE_PURE / PYBIND11_OVERRIDE ---
class PyCat(trampoline.Animal):
    def go(self, n):           # overrides pure virtual
        return "meow! " * n
    def name(self):            # overrides non-pure virtual
        return "Cat"

cat = PyCat()
print("1a. Python subclass of Animal:")
print("    cat.go(3)  =", cat.go(3))
print("    cat.name() =", cat.name())
print()

# 1b. Override pure only → name() falls back to C++ base default.
class HalfCat(trampoline.Animal):
    def go(self, n):
        return "..." * n
print("1b. Non-pure left to C++ default:")
print("    HalfCat().name() =", HalfCat().name(), "  (C++ Animal::name)")
print()

# 1c. Forget to override pure → RuntimeError at call time.
class Broken(trampoline.Animal):
    pass
try:
    Broken().go(1)
except RuntimeError as e:
    print("1c. Pure virtual not overridden:")
    print("    Broken().go(1) -> RuntimeError:", e)
print()

# --- 2/3. Combining virtual functions and inheritance ---
dog = trampoline.Dog()
print("2a. Pure C++ Dog (no Python override):")
print("    dog.go(2)    =", dog.go(2))
print("    dog.name()   =", dog.name())
print("    dog.fetch()  =", dog.fetch())
print()

class Poodle(trampoline.Dog):
    def go(self, n):
        return "yip " * n
    def fetch(self):
        return "*prances back*"

p = Poodle()
print("2b. Python subclass of Dog overrides go and fetch:")
print("    p.go(2)      =", p.go(2))
print("    p.fetch()    =", p.fetch())
print("    p.name()     =", p.name(), "  (inherits C++ Dog::name)")
print()

# --- 4. C++ -> Python dispatch through Animal* ---
print("4. call_go(Animal*) dispatches to Python override:")
print("   call_go(cat, 2) =", trampoline.call_go(cat, 2))
print("   call_go(dog, 2) =", trampoline.call_go(dog, 2))
print("   call_go(p,   2) =", trampoline.call_go(p, 2))
