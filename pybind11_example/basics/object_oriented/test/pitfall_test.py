import pitfall

print("submodule pitfall (Parent/Child raw-pointer demo):")
print()

print("PROBLEM: raw-pointer return + default take_ownership -> double-free at exit")
parent = pitfall.Parent()
child = parent.get_child()
print("  Parent:", parent)
print("  Child :", child)
print("  (interpreter may exit with SIGABRT / 134 because of this)")
print()

print("SOLUTION 1: keep C++ unchanged, bind with reference_internal")
ps1 = pitfall.ParentSafe1()
c1 = ps1.get_child()
print("  ParentSafe1 -> Child:", c1)
print()

print("SOLUTION 2: Parent.get_child() returns shared_ptr<Child> directly")
ps2 = pitfall.ParentSafe2()
c2 = ps2.get_child()
print("  ParentSafe2 -> Child:", c2)
print()

print("SOLUTION 3: Child inherits std::enable_shared_from_this")
ps3 = pitfall.ParentSafe3()
c3 = ps3.get_child()
print("  ParentSafe3 -> ChildSafe3:", c3)
