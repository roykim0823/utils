// =============================================================================
// Topics 1-4: Overriding virtual functions in Python (trampoline classes)
//   1. PYBIND11_OVERRIDE_PURE — Python MUST override a pure virtual
//   2. PYBIND11_OVERRIDE      — Python MAY override; falls back to C++ base
//   3. Combining virtual functions and inheritance — trampoline at each level
//   4. C++ -> Python dispatch through a base pointer
// Docs: https://pybind11.readthedocs.io/en/latest/advanced/classes.html
//       #overriding-virtual-functions-in-python
//       #combining-virtual-functions-and-inheritance
//
// A "trampoline" is a C++ subclass that forwards each virtual call to Python
// if a Python override exists, otherwise to the C++ base implementation. You
// register the trampoline as the second template arg of py::class_:
//     py::class_<Animal, PyAnimal>(m, "Animal") ...
// You MUST also register the trampoline for any C++ subclass Python may
// subclass further — otherwise Python overrides on that level are lost.
//
// Macro choice:
//   PYBIND11_OVERRIDE_PURE(ret, base, name, args...) -> throws if no override
//   PYBIND11_OVERRIDE     (ret, base, name, args...) -> falls back to base
// Zero-arg form requires a trailing comma: PYBIND11_OVERRIDE(ret, base, name,)
// =============================================================================

#include <pybind11/pybind11.h>
#include <string>

namespace py = pybind11;

namespace {

// 1. Abstract base — `go` is pure virtual, `name` has a default implementation.
struct Animal {
    virtual ~Animal() = default;
    virtual std::string go(int n_times) = 0;
    virtual std::string name() const { return "Animal"; }
};

// 2. Concrete C++ subclass — Python may subclass this too.
struct Dog : Animal {
    std::string go(int n_times) override {
        std::string s;
        for (int i = 0; i < n_times; ++i) s += "woof! ";
        return s;
    }
    std::string name() const override { return "Dog"; }
    virtual std::string fetch() const { return "*runs after stick*"; }
};

// 3a. Trampoline for Animal.
struct PyAnimal : Animal {
    using Animal::Animal;
    std::string go(int n_times) override {
        PYBIND11_OVERRIDE_PURE(std::string, Animal, go, n_times);
    }
    std::string name() const override {
        PYBIND11_OVERRIDE(std::string, Animal, name, );
    }
};

// 3b. Trampoline for Dog — needed because Python may subclass Dog.
// Without it, a Poodle(trampoline.Dog) subclass's `go` override would be
// silently ignored when called from C++.
struct PyDog : Dog {
    using Dog::Dog;
    std::string go(int n_times) override {
        PYBIND11_OVERRIDE(std::string, Dog, go, n_times);
    }
    std::string name() const override {
        PYBIND11_OVERRIDE(std::string, Dog, name, );
    }
    std::string fetch() const override {
        PYBIND11_OVERRIDE(std::string, Dog, fetch, );
    }
};

// 4. C++ side calls virtuals through Animal*; should hit Python overrides too.
std::string call_go(Animal* a, int n) {
    return a->name() + ": " + a->go(n);
}

}  // namespace

PYBIND11_MODULE(trampoline, m, py::mod_gil_not_used()) {
    py::class_<Animal, PyAnimal>(m, "Animal")
        .def(py::init<>())
        .def("go",   &Animal::go,   py::arg("n_times"))
        .def("name", &Animal::name);

    // Bind Dog with Animal as its registered base AND PyDog as its trampoline.
    py::class_<Dog, Animal, PyDog>(m, "Dog")
        .def(py::init<>())
        .def("fetch", &Dog::fetch);

    m.def("call_go", &call_go, py::arg("animal"), py::arg("n"),
          "Calls animal.name() + animal.go(n) from C++ — dispatches through "
          "the trampoline to Python overrides.");
}
