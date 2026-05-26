// =============================================================================
// Topics 11-12: Pickling and deepcopy
//   py::pickle(__getstate__, __setstate__)
// Docs: https://pybind11.readthedocs.io/en/latest/advanced/classes.html
//       #pickling-support
//       #deepcopy-support
//
// pickle requires two lambdas:
//   __getstate__(const T&) -> some picklable Python object (commonly py::tuple)
//   __setstate__(state)    -> T  (a fresh instance reconstructed from state)
//
// Once pickle is implemented, copy.deepcopy works for free: Python's deepcopy
// uses __reduce_ex__ when present, and py::pickle wires that up.
// =============================================================================

#include <pybind11/pybind11.h>
#include <string>

namespace py = pybind11;

namespace {

struct Pickleable {
    std::string name;
    int counter;

    Pickleable(std::string n, int c) : name(std::move(n)), counter(c) {}

    std::string repr() const {
        return "Pickleable(name='" + name + "', counter=" + std::to_string(counter) + ")";
    }
};

}  // namespace

PYBIND11_MODULE(pickling, m, py::mod_gil_not_used()) {
    py::class_<Pickleable>(m, "Pickleable")
        .def(py::init<std::string, int>(), py::arg("name"), py::arg("counter"))
        .def_readwrite("name",    &Pickleable::name)
        .def_readwrite("counter", &Pickleable::counter)
        .def("__repr__", &Pickleable::repr)

        // The two halves of the pickle protocol.
        .def(py::pickle(
            // __getstate__: produce a picklable representation of the C++ state.
            // py::tuple is the conventional carrier — anything picklable works.
            [](const Pickleable& p) {
                return py::make_tuple(p.name, p.counter);
            },
            // __setstate__: reconstruct an instance from the state tuple.
            // pybind11 wraps the returned object in a fresh Python wrapper.
            [](py::tuple t) {
                if (t.size() != 2) {
                    throw std::runtime_error("Pickleable: invalid state tuple");
                }
                return Pickleable(t[0].cast<std::string>(),
                                  t[1].cast<int>());
            }
        ));
}
