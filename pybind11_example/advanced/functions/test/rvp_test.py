import rvp

print("=== Return Value Policies ===\n")

# 1. take_ownership for a genuine factory — pybind11 deletes when Python GCs.
print("1. make_data() returns a freshly-allocated Data; take_ownership is safe:")
d = rvp.make_data(7, "freshly-made")
print("  ", d)
print()

# 2. reference_internal: Data tied to Owner lifetime, no double-free.
print("2. Owner.get_ref_internal() with reference_internal — safe by design:")
o = rvp.Owner()
ri = o.get_ref_internal()
print("  ", ri)
ri.value = 100
print("   after mutating via Python:", o.get_ref_internal())
print()

# 3. copy: independent Python-owned copy.
print("3. Owner.get_copy() with copy — modifications don't affect the original:")
c = o.get_copy()
c.value = 999
print("   modified copy :", c)
print("   original Owner:", o.get_ref_internal())  # still 100, not 999
print()

# 4. reference: C++ keeps ownership; safe as long as Owner outlives the wrapper.
print("4. Owner.get_ref() with reference — no copy, ties to Owner lifetime:")
r = o.get_ref()
print("  ", r)
r.value = 42
print("   mutated through Python, Owner sees it:", o.get_ref_internal())
print()

# 5. take_ownership on a BORROWED pointer is the classic mistake.
# Don't actually call this in normal runs — it will SIGABRT at exit.
print("5. get_ptr_take_ownership_DANGER is intentionally unsafe.")
print("   To witness the double-free, uncomment the line below and rerun.")
# bad = o.get_ptr_take_ownership_DANGER()  # SIGABRT at interpreter exit
