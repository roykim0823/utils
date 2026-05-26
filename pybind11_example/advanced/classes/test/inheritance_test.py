import inheritance

print("=== Inheritance edge cases ===\n")

# --- 13. Multiple inheritance ---
print("13. Multiple inheritance — Amphibian sees Walker and Swimmer methods:")
a = inheritance.Amphibian()
print("    a.walk() =", a.walk())
print("    a.swim() =", a.swim())
print("    a.both() =", a.both())
print("    isinstance(a, Walker)  ?", isinstance(a, inheritance.Walker))
print("    isinstance(a, Swimmer) ?", isinstance(a, inheritance.Swimmer))
print()

# --- 14. Module-local class binding ---
print("14. LocalOnly is bound with py::module_local() — syntax demo:")
lo = inheritance.LocalOnly(7)
print("    LocalOnly(7).value =", lo.value)
print()

# --- 15. Final classes ---
print("15. Sealed is py::is_final() — Python subclassing raises TypeError:")
s = inheritance.Sealed(5)
print("    Sealed(5).doubled() =", s.doubled())
try:
    class TrySubclass(inheritance.Sealed):
        pass
except TypeError as e:
    print("    class T(Sealed): pass -> TypeError:", e)
print()

# --- 16. Protected member via publicist pattern ---
print("16. WithSecret — protected method exposed via publicist:")
w = inheritance.WithSecret(3)
print("    w.public_api()       =", w.public_api())
print("    w.hidden_compute(4)  =", w.hidden_compute(4), "  (was protected in C++)")
print()

# --- 17. Template classes — distinct Python names ---
print("17. Box<T> bound as BoxInt / BoxStr:")
bi = inheritance.BoxInt(10); bi.set(99)
bs = inheritance.BoxStr("hi"); bs.set("world")
print("    BoxInt -> get() =", bi.get())
print("    BoxStr -> get() =", bs.get())
