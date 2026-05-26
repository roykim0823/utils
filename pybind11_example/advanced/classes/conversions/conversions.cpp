// =============================================================================
// Topics 7-8: Implicit conversions between bound types
//   py::implicitly_convertible<Source, Target>();
// Docs: https://pybind11.readthedocs.io/en/latest/advanced/classes.html
//       #implicit-conversions
//
// Tell pybind11 that any argument-binding that needs a Target can accept a
// Source — pybind11 will construct a Target from the Source automatically.
// Requirement: Target must have a (typically explicit) constructor taking
// Source, or Source must be implicitly convertible to Target in C++.
//
// Mechanism: this only kicks in on PASS 2 of overload resolution (the
// "allow conversions" pass), so explicit Target arguments are still preferred
// over Source -> Target conversion when both overloads are present.
// =============================================================================

#include <pybind11/pybind11.h>
#include <string>

namespace py = pybind11;

namespace {

struct Meters {
    double value;
    explicit Meters(double v) : value(v) {}
};

struct Feet {
    double value;
    explicit Feet(double v) : value(v) {}
    // Conversion from Meters — declared explicit, but pybind11's
    // implicitly_convertible will USE it implicitly at the binding boundary.
    explicit Feet(const Meters& m) : value(m.value * 3.28084) {}
};

double print_feet(const Feet& f) { return f.value; }

}  // namespace

PYBIND11_MODULE(conversions, m, py::mod_gil_not_used()) {
    py::class_<Meters>(m, "Meters")
        .def(py::init<double>(), py::arg("value"))
        .def_readwrite("value", &Meters::value)
        .def("__repr__", [](const Meters& m) {
            return "Meters(" + std::to_string(m.value) + ")";
        });

    py::class_<Feet>(m, "Feet")
        .def(py::init<double>(),             py::arg("value"))
        .def(py::init<const Meters&>(),      py::arg("meters"))
        .def_readwrite("value", &Feet::value)
        .def("__repr__", [](const Feet& f) {
            return "Feet(" + std::to_string(f.value) + ")";
        });

    // Tell pybind11: wherever Feet is expected, a Meters is acceptable —
    // pybind11 will call Feet(const Meters&) under the hood.
    py::implicitly_convertible<Meters, Feet>();

    m.def("print_feet", &print_feet, py::arg("f"),
          "Returns f.value. With implicitly_convertible<Meters, Feet>(), "
          "you can pass a Meters and pybind11 converts.");
}
