import time
import threading
import call_policies

print("=== Additional Call Policies ===\n")

# ----- keep_alive -----
print("1. keep_alive<1, 2>: arg stays alive at least as long as self")
pl = call_policies.PtrList()
some_obj = ["hello"]
pl.append(some_obj)
del some_obj          # without keep_alive<1, 2>, the appended ptr would dangle
print("   PtrList size after del'ing the source:", pl.size())
print()

# ----- call_guard / GIL release -----
def time_concurrent(target, label, num_threads=4):
    t0 = time.perf_counter()
    threads = [threading.Thread(target=target) for _ in range(num_threads)]
    for t in threads: t.start()
    for t in threads: t.join()
    elapsed = time.perf_counter() - t0
    print(f"   {label:30s} {num_threads} threads x 100ms sleep took {elapsed*1000:7.1f}ms")

print("2. call_guard<gil_scoped_release> lets Python threads run in parallel:")
time_concurrent(lambda: call_policies.sleep_with_gil(100),    "WITH GIL held  (serial):", 16)
time_concurrent(lambda: call_policies.sleep_release_gil(100), "WITHOUT GIL    (parallel):", 16)
print("   (the second should be ~100ms; the first ~1600ms.)")
print()

# ----- multi-guard construction order -----
# Reference: pybind11/tests/test_call_policies.cpp
print("3. call_guard<G1, G2>: G1 ctor runs before G2 ctor (so G2 can read G1's state)")
print("   correct order <CustomGuard, DependentGuard>:")
print("    ", call_policies.guarded_call_correct_order())
# Expected: dependent_saw_custom = True

print("   wrong order   <DependentGuard, CustomGuard>:")
print("    ", call_policies.guarded_call_wrong_order())
# Expected: dependent_saw_custom = False (DependentGuard ran first, didn't see custom)

print("   single guard  <CustomGuard>:")
print("    ", call_policies.guarded_call_custom_only())
# custom_active reflects inside-the-call state (True); dependent_saw_custom is
# whatever the previous call left it (thread_local persists).
