import conversions

print("=== Implicit conversions between bound types ===\n")

# Direct Feet — straightforward.
f = conversions.Feet(10.0)
print("7a. print_feet(Feet(10.0)) =", conversions.print_feet(f))

# Meters constructs Feet explicitly in C++; here Python can do it directly too.
m = conversions.Meters(10.0)
print("7b. Meters(10.0) =>", m, "; Feet(m) =", conversions.Feet(m))

# Implicit conversion at the binding boundary:
# print_feet expects Feet, but we pass Meters — pybind11 calls Feet(Meters).
print("7c. print_feet(Meters(10.0)) — implicit conversion kicks in:")
print("    result =", conversions.print_feet(m))
print()

# Negative case: a totally unrelated type still raises.
try:
    conversions.print_feet("ten meters")
except TypeError as e:
    print("8. print_feet('ten meters') -> TypeError:", e)
