// =============================================================================
// Topics 5-6: Custom constructors + non-public destructors
//   5. py::init([](...) { ... })  — factory functions returning T, T*,
//                                    std::unique_ptr<T>, or std::shared_ptr<T>
//   6. Non-public destructor      — bind with holder std::unique_ptr<T, py::nodelete>
// Docs: https://pybind11.readthedocs.io/en/latest/advanced/classes.html
//       #custom-constructors
//       #non-public-destructors
//
// py::init<Args...>()   binds a real C++ constructor (the simple case).
// py::init([](...){})   lets the lambda decide HOW to build the object:
//   - return T               -> placement-new into pybind11's buffer
//   - return T*              -> pybind11 takes ownership (must be heap-allocated)
//   - return std::unique_ptr -> matches a unique_ptr holder
//   - return std::shared_ptr -> matches a shared_ptr holder
//
// Non-public destructor: pybind11's default holder is std::unique_ptr<T>, which
// will try to `delete` at GC time and fail to compile if ~T is private. Use
// std::unique_ptr<T, py::nodelete> so pybind11 never deletes; the C++ side
// retains lifetime control (e.g., a singleton).
// =============================================================================

#include <pybind11/pybind11.h>
#include <memory>
#include <string>

namespace py = pybind11;

namespace {

// 5. A type with several "construction recipes" expressed as factory lambdas.
struct Widget {
    int x;
    std::string label;
    Widget(int x, std::string label) : x(x), label(std::move(label)) {}
    std::string repr() const { return "Widget(x=" + std::to_string(x) + ", label='" + label + "')"; }
};

// 6. A class with a private destructor: instances live forever (singleton-style).
//    pybind11 needs to be told NOT to call delete on it.
class Singleton {
public:
    static Singleton& instance() {
        static Singleton s;
        return s;
    }
    int counter() const { return count_; }
    void bump() { ++count_; }

private:
    Singleton() = default;
    ~Singleton() = default;  // private — pybind11 must not delete
    int count_ = 0;
};

}  // namespace

PYBIND11_MODULE(custom_init, m, py::mod_gil_not_used()) {
    // --- 5. Factory constructors via py::init(lambda) ---
    py::class_<Widget>(m, "Widget")
        // 5a. Real C++ ctor — the simple form.
        .def(py::init<int, std::string>(), py::arg("x"), py::arg("label"))

        // 5b. Factory lambda returning T by value (placement-new).
        //     Overloads on int — useful when you want a "from_x" constructor
        //     under the SAME Python name as __init__.
        .def(py::init([](int x) {
                 return Widget(x, "from-int");
             }),
             py::arg("x"))

        // 5c. Factory lambda returning unique_ptr — pybind11 adopts ownership.
        //     Demonstrates a different overload (single string arg).
        .def(py::init([](const std::string& label) {
                 return std::make_unique<Widget>(0, label);
             }),
             py::arg("label"))

        .def_readwrite("x", &Widget::x)
        .def_readwrite("label", &Widget::label)
        .def("__repr__", &Widget::repr);

    // --- 6. Non-public destructor → py::nodelete holder ---
    // The holder type goes as the second template argument to py::class_.
    // pybind11 will NOT free the object when the last Python reference dies.
    py::class_<Singleton, std::unique_ptr<Singleton, py::nodelete>>(m, "Singleton")
        // No py::init() bound — Python cannot construct it directly.
        // Expose the static accessor; reference_internal isn't right here
        // because there's no parent object — use reference policy.
        .def_static("instance", &Singleton::instance,
                    py::return_value_policy::reference,
                    "Returns the process-wide Singleton.")
        .def("counter", &Singleton::counter)
        .def("bump",    &Singleton::bump);
}
