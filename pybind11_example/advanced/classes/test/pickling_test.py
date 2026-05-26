import pickle
import copy
import pickling

print("=== Pickling + deepcopy ===\n")

p = pickling.Pickleable("alpha", 7)
print("Original:", p)

# --- 11. Round-trip through pickle ---
blob = pickle.dumps(p)
print("\n11. pickle round-trip:")
print("    pickle.dumps -> bytes of length", len(blob))
restored = pickle.loads(blob)
print("    pickle.loads ->", restored)
print("    name match    ?", restored.name == p.name)
print("    counter match ?", restored.counter == p.counter)
print("    distinct C++ object (mutate restored, original unchanged):")
restored.counter = 999
print("       restored =", restored)
print("       original =", p)

# --- 12. deepcopy comes for free because py::pickle wires __reduce_ex__ ---
print("\n12. copy.deepcopy works without separate __deepcopy__:")
d = copy.deepcopy(p)
print("    deepcopy ->", d)
d.name = "mutated"
print("    after mutating deepcopy:")
print("       deepcopy =", d)
print("       original =", p)

# --- Bad state path ---
print("\nInvalid pickle state surfaces our std::runtime_error:")
try:
    pickling.Pickleable.__setstate__ if False else None  # placeholder
    # Replicate corruption by stuffing a wrong-size tuple via pickle protocol:
    bad = pickle.dumps(p)
    # Force a 1-tuple by re-constructing via __reduce_ex__ shape.
    # Easiest path: call py::pickle's setter directly through the type's __new__/__setstate__.
    cls = pickling.Pickleable
    obj = cls.__new__(cls)
    obj.__setstate__((1,))   # wrong size — triggers runtime_error
except Exception as e:
    print("    bad state ->", type(e).__name__, ":", e)
