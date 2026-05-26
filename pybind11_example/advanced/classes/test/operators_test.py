import operators

print("=== Operator overloading ===\n")

a = operators.Vec2(1.0, 2.0)
b = operators.Vec2(3.0, 4.0)

print("Vec2 instances:")
print("    a =", a)
print("    b =", b)
print()

print("Binary:")
print("    a + b      =", a + b)
print("    a - b      =", a - b)
print("    a == a     =", a == a)
print("    a == b     =", a == b)
print()

print("Mixed-type (commutative — both directions registered):")
print("    a * 2.5    =", a * 2.5)
print("    2.5 * a    =", 2.5 * a)
print()

print("Unary:")
print("    -a         =", -a)
print()

print("In-place += mutates the Python wrapper's underlying C++ object:")
c = operators.Vec2(0.0, 0.0)
print("    c          =", c)
c += b
print("    c += b -> c=", c)
