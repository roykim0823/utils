// =============================================================================
// Topics 10-11: Overload resolution + template functions
//   10. Overload resolution order, py::prepend()
//   11. Binding template functions (must instantiate each specialization)
// Docs: https://pybind11.readthedocs.io/en/latest/advanced/functions.html
//       #overload-resolution-order
//       #binding-functions-with-template-parameters
//
// Overload resolution is two-pass:
//   Pass 1: try each registered overload with NO implicit conversions.
//   Pass 2: if none matched, try each overload allowing conversions, in
//           registration order.
// Use py::prepend() to insert a new overload at the FRONT of the list (useful
// for adding a more-specific overload after a general one is already bound).
//
// Templates: pybind11 binds concrete function pointers — you can't bind an
// uninstantiated template. Instantiate each specialization separately, either
// under one Python name (overloading) or under distinct names.
// =============================================================================

#include <pybind11/pybind11.h>
#include <string>

namespace py = pybind11;

namespace {

// Two overloads of the same Python name: pybind11 picks by argument type.
std::string describe(int    x) { return "int="    + std::to_string(x); }
std::string describe(double x) { return "double=" + std::to_string(x); }

// 11. A template function with multiple instantiations bound under one name.
template <typename T>
std::string typed_label(T value, const std::string &tag) {
    return tag + ":" + std::to_string(value);
}

// 11b. Same template, instantiated for std::string under a distinct name.
template <>
std::string typed_label<std::string>(std::string value, const std::string &tag) {
    return tag + ":" + value;
}

// 10b. A "general" overload registered first, and a more-specific one we
// insert at the FRONT later via py::prepend so it's tried first.
std::string handle_general(py::object o) { return "general: " + std::string(py::str(o)); }
std::string handle_str(const std::string &s) { return "str-specific: " + s; }

}  // namespace

PYBIND11_MODULE(overloads, m, py::mod_gil_not_used()) {
    // --- 10. Overload resolution ---
    // Pass 1 (no-conversion) tries int(1) -> describe(int); 1.5 -> describe(double).
    // Pass 2 would convert if needed, but here both overloads match exactly.
    m.def("describe", py::overload_cast<int>   (&describe));
    m.def("describe", py::overload_cast<double>(&describe));

    // Register the GENERAL overload first ...
    m.def("handle", &handle_general, py::arg("o"));
    // ... then PREPEND the more-specific str overload so it's tried first.
    // Without py::prepend(), the general py::object overload would shadow it
    // (since py::object matches everything in pass 1).
    m.def("handle", &handle_str, py::arg("s"), py::prepend());

    // --- 11. Template function bindings ---
    // One instantiation per type. Bound under the same Python name -> overload.
    m.def("typed_label", &typed_label<int>,         py::arg("value"), py::arg("tag"));
    m.def("typed_label", &typed_label<double>,      py::arg("value"), py::arg("tag"));
    m.def("typed_label", &typed_label<std::string>, py::arg("value"), py::arg("tag"));

    // Or under distinct names if you'd prefer no overload at all.
    m.def("typed_label_int",    &typed_label<int>,         py::arg("value"), py::arg("tag"));
    m.def("typed_label_string", &typed_label<std::string>, py::arg("value"), py::arg("tag"));
}
