import overloads

print("=== Overload resolution + template functions ===\n")

# 10. describe(int) vs describe(double) — direct overload dispatch.
print("10a. describe — two C++ overloads under one Python name:")
print("    describe(5)   =", overloads.describe(5))     # int overload
print("    describe(5.5) =", overloads.describe(5.5))   # double overload
print()

# 10b. py::prepend — make the str-specific overload win over the py::object one.
print("10b. handle — py::object overload registered first, str-specific prepended:")
print("    handle('hi')  =", overloads.handle("hi"))           # str overload (prepended)
print("    handle(42)    =", overloads.handle(42))             # falls through to py::object
print()

# 11. Template function bindings — one specialization per type.
print("11. typed_label — three template instantiations under one Python name:")
print("    typed_label(7, 'i')      =", overloads.typed_label(7, "i"))
print("    typed_label(3.14, 'f')   =", overloads.typed_label(3.14, "f"))
print("    typed_label('hey', 's')  =", overloads.typed_label("hey", "s"))
print()

print("11b. Distinct names instead of overloading:")
print("    typed_label_int(7, 'i')      =", overloads.typed_label_int(7, "i"))
print("    typed_label_string('hey','s')=", overloads.typed_label_string("hey", "s"))
