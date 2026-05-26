// =============================================================================
// Topics 13-17: Inheritance edge cases
//   13. Multiple inheritance — list bases as py::class_ template args
//   14. py::module_local()    — confine a binding to one module (avoids
//                                cross-module conflicts when the same C++ type
//                                is bound twice)
//   15. py::is_final()        — Python cannot subclass this type
//   16. Protected members     — expose via a "publicist" subclass
//   17. Template classes      — bind multiple instantiations under distinct names
// Docs: https://pybind11.readthedocs.io/en/latest/advanced/classes.html
//       #multiple-inheritance
//       #module-local-class-bindings
//       #binding-final-classes
//       #binding-protected-member-functions
//       #binding-classes-with-template-parameters
// =============================================================================

#include <pybind11/pybind11.h>
#include <string>

namespace py = pybind11;

namespace {

// ---------- 13. Multiple inheritance ----------
struct Walker { std::string walk() const { return "walking"; } };
struct Swimmer { std::string swim() const { return "swimming"; } };
struct Amphibian : Walker, Swimmer {
    std::string both() const { return walk() + " + " + swim(); }
};

// ---------- 14. Module-local binding ----------
// Just demonstrates the syntax — the real benefit shows when a type is bound
// in two modules: with module_local(), each module gets its own Python type
// and they don't clash on import.
struct LocalOnly {
    int value;
    LocalOnly(int v = 0) : value(v) {}
};

// ---------- 15. Final classes ----------
struct Sealed {
    int x;
    Sealed(int x) : x(x) {}
    int doubled() const { return x * 2; }
};

// ---------- 16. Protected member via publicist pattern ----------
class WithSecret {
public:
    WithSecret(int seed) : seed_(seed) {}
    int public_api() const { return seed_; }

protected:
    int hidden_compute(int n) const { return seed_ * n; }

private:
    int seed_;
};

// "Publicist" — derives publicly and re-exposes the protected member.
class WithSecretPub : public WithSecret {
public:
    using WithSecret::hidden_compute;
};

// ---------- 17. Template classes ----------
template <typename T>
struct Box {
    T value;
    Box(T v) : value(std::move(v)) {}
    T get() const { return value; }
    void set(T v) { value = std::move(v); }
};

}  // namespace

PYBIND11_MODULE(inheritance, m, py::mod_gil_not_used()) {
    // --- 13. Multiple inheritance ---
    // List ALL bases as template args; pybind11 wires up method resolution.
    // Alternative (when bases aren't bound via py::class_ above): pass
    // py::multiple_inheritance() tag in the constructor.
    py::class_<Walker>(m, "Walker")
        .def(py::init<>()).def("walk", &Walker::walk);
    py::class_<Swimmer>(m, "Swimmer")
        .def(py::init<>()).def("swim", &Swimmer::swim);
    py::class_<Amphibian, Walker, Swimmer>(m, "Amphibian")
        .def(py::init<>())
        .def("both", &Amphibian::both);

    // --- 14. Module-local class binding ---
    // Mark with py::module_local() — only THIS module sees this binding.
    // Useful if you ship a small utility type from multiple wheels.
    py::class_<LocalOnly>(m, "LocalOnly", py::module_local())
        .def(py::init<int>(), py::arg("v") = 0)
        .def_readwrite("value", &LocalOnly::value);

    // --- 15. Final classes ---
    // Python subclassing this raises TypeError.
    py::class_<Sealed>(m, "Sealed", py::is_final())
        .def(py::init<int>(), py::arg("x"))
        .def("doubled", &Sealed::doubled);

    // --- 16. Protected member via publicist ---
    // Bind &WithSecretPub::hidden_compute — accessible because publicist made
    // it public. The Python type is still WithSecret.
    py::class_<WithSecret>(m, "WithSecret")
        .def(py::init<int>(), py::arg("seed"))
        .def("public_api", &WithSecret::public_api)
        .def("hidden_compute", &WithSecretPub::hidden_compute, py::arg("n"));

    // --- 17. Template classes: bind each instantiation under a distinct name ---
    py::class_<Box<int>>(m, "BoxInt")
        .def(py::init<int>())
        .def("get", &Box<int>::get)
        .def("set", &Box<int>::set)
        .def_readwrite("value", &Box<int>::value);

    py::class_<Box<std::string>>(m, "BoxStr")
        .def(py::init<std::string>())
        .def("get", &Box<std::string>::get)
        .def("set", &Box<std::string>::set)
        .def_readwrite("value", &Box<std::string>::value);
}
