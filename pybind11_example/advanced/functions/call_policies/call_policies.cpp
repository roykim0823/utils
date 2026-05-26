// =============================================================================
// Topic 2: Additional Call Policies (keep_alive, call_guard)
// Docs: https://pybind11.readthedocs.io/en/latest/advanced/functions.html
//       #additional-call-policies
// Reference test: pybind11/tests/test_call_policies.cpp
//
// keep_alive<Nurse, Patient>():
//   Keeps `Patient` alive at least as long as `Nurse` lives.
//   Index 0 = return value, 1 = this/self, 2..N = positional arguments.
//   Use when a container/parent stores raw references to its arguments.
//
// call_guard<Guard1, Guard2, ...>():
//   Wraps each C++ call in RAII guards. Construction order is left-to-right;
//   destruction is right-to-left (standard C++ aggregate semantics). So if
//   Guard2 depends on Guard1's state, Guard1 MUST come first.
//   Most common single guard: py::gil_scoped_release for long-running C++
//   that doesn't touch the Python C API.
// =============================================================================

#include <pybind11/pybind11.h>
#include <chrono>
#include <thread>
#include <vector>
#include <string>

namespace py = pybind11;

namespace {

// ---------- keep_alive helpers ----------
struct PtrList {
    std::vector<py::object *> items;
    void append(py::object *o) { items.push_back(o); }
    size_t size() const { return items.size(); }
};

// ---------- gil_scoped_release helpers ----------
void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ---------- multi-guard construction-order demo ----------
//
// CustomGuard sets a thread-local flag in its ctor and clears it in its dtor.
// DependentGuard SNAPSHOTS that flag at construction time. With the correct
// guard order `<CustomGuard, DependentGuard>`, DependentGuard sees the flag
// set; reversed, it sees it cleared. The Python-visible function returns both
// the live flag and the snapshot so the test can compare.
thread_local bool tls_custom_active        = false;
thread_local bool tls_dependent_saw_custom = false;

struct CustomGuard {
    CustomGuard()  { tls_custom_active = true;  }
    ~CustomGuard() { tls_custom_active = false; }
};

struct DependentGuard {
    DependentGuard() { tls_dependent_saw_custom = tls_custom_active; }
    // No dtor work needed for this demo.
};

py::dict report_guards() {
    py::dict d;
    d["custom_active"]        = tls_custom_active;
    d["dependent_saw_custom"] = tls_dependent_saw_custom;
    return d;
}

// A no-op function we can wrap to observe destructor order. The CustomGuard
// dtor clears tls_custom_active, so after the call returns the flag is false
// regardless of order — destruction is observable only via side-effects that
// outlive the call. Kept simple here; the official test uses richer state.

}  // namespace

PYBIND11_MODULE(call_policies, m, py::mod_gil_not_used()) {
    // ----- keep_alive -----
    py::class_<PtrList>(m, "PtrList")
        .def(py::init<>())
        .def("size", &PtrList::size)
        // keep_alive<1, 2>(): keep argument 2 alive at least as long as self (1).
        // Without this, you could append(obj) and then `del obj` -> dangling
        // ptr inside `items`. Illustrative of the policy.
        .def("append", &PtrList::append, py::keep_alive<1, 2>());

    // ----- single guard: gil_scoped_release -----
    m.def("sleep_with_gil",    &sleep_ms, py::arg("ms"));
    m.def("sleep_release_gil", &sleep_ms, py::arg("ms"),
          py::call_guard<py::gil_scoped_release>());

    // ----- multiple guards: construction order matters -----
    // Correct: CustomGuard constructed first, then DependentGuard sees the flag.
    m.def("guarded_call_correct_order", &report_guards,
          py::call_guard<CustomGuard, DependentGuard>());
    // Wrong:   DependentGuard constructed first, sees flag still false.
    m.def("guarded_call_wrong_order",   &report_guards,
          py::call_guard<DependentGuard, CustomGuard>());

    // ----- single CustomGuard for completeness -----
    // Without any other guard, dependent_saw_custom stays at whatever the
    // previous call left it (thread_local), but custom_active reflects "we
    // are inside the guarded function" during the call.
    m.def("guarded_call_custom_only", &report_guards,
          py::call_guard<CustomGuard>());
}
