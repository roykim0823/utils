// =============================================================================
// Topic 1: Return Value Policies
// Docs: https://pybind11.readthedocs.io/en/latest/advanced/functions.html
//       #return-value-policies
//
// When a C++ function returns a pointer or reference, pybind11 needs to know
// who owns the underlying object and how long it must live. The wrong policy
// causes either memory leaks, double-frees, or use-after-free crashes.
//
// Policies demonstrated below (one binding each):
//   - automatic              : default; take_ownership for pointers, move/copy
//                              for value/reference returns.
//   - take_ownership         : Python takes ownership and will delete it.
//                              CRASH if C++ also owns the object (double-free).
//   - copy                   : pybind11 makes a Python-owned copy. Always safe.
//   - move                   : pybind11 moves into a Python-owned instance.
//   - reference              : C++ keeps ownership. UNSAFE if C++ deletes the
//                              object while Python still has a reference.
//   - reference_internal     : like reference, but ties lifetime of the
//                              returned value to the parent (this).
//   - automatic_reference    : automatic but returns reference for pointers.
// =============================================================================

#include <pybind11/pybind11.h>
#include <memory>
#include <string>

namespace py = pybind11;

namespace {

struct Data {
    int value;
    std::string label;

    Data(int v = 0, std::string l = "") : value(v), label(std::move(l)) {}
};

// A long-lived owner that holds a Data* it constructed itself.
struct Owner {
    Data data{42, "owned-by-cpp"};

    // Returning a raw pointer/reference triggers the policy debate.
    Data *get_ptr() { return &data; }
    Data &get_ref() { return data; }
    const Data &get_const_ref() const { return data; }
    Data get_copy() const { return data; }  // value return
};

// A freshly-allocated Data each call — this one Python SHOULD own.
Data *make_data(int v, const std::string &l) {
    return new Data(v, l);
}

}  // namespace

PYBIND11_MODULE(rvp, m, py::mod_gil_not_used()) {
    py::class_<Data>(m, "Data")
        .def(py::init<int, std::string>(), py::arg("value") = 0, py::arg("label") = "")
        .def_readwrite("value", &Data::value)
        .def_readwrite("label", &Data::label)
        .def("__repr__", [](const Data &d) {
            return "Data(value=" + std::to_string(d.value) + ", label='" + d.label + "')";
        });

    py::class_<Owner>(m, "Owner")
        .def(py::init<>())
        // SAFE: reference_internal ties Data lifetime to Owner.
        .def("get_ref_internal", &Owner::get_ref,
             py::return_value_policy::reference_internal)
        // SAFE: pybind11 makes a Python-owned copy.
        .def("get_copy", &Owner::get_copy,
             py::return_value_policy::copy)
        // SAFE in this layout: C++ keeps ownership, Python won't delete.
        // (Becomes unsafe if Owner is destroyed first.)
        .def("get_ref", &Owner::get_ref,
             py::return_value_policy::reference)
        // DANGEROUS: explicit take_ownership on a borrowed pointer -> double free
        // at GC. Left in only to show how to *spell* it; do not call this method
        // (or do so once at the very end of a session to observe the crash).
        .def("get_ptr_take_ownership_DANGER", &Owner::get_ptr,
             py::return_value_policy::take_ownership);

    // A factory that genuinely transfers ownership to Python is the natural
    // fit for take_ownership (which is also the *default* for raw pointer
    // returns, i.e. automatic).
    m.def("make_data", &make_data, py::arg("value"), py::arg("label"),
          py::return_value_policy::take_ownership);
}
