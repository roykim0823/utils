// =============================================================================
// Topics 5-9: Argument modifiers
//   5. Default arguments (py::arg("x") = value, py::arg_v for repr)
//   6. Keyword-only arguments (py::kw_only)
//   7. Positional-only arguments (py::pos_only)
//   8. Non-converting arguments (.noconvert())
//   9. Allow/Prohibit None (.none(true)/.none(false))
// Docs: https://pybind11.readthedocs.io/en/latest/advanced/functions.html
// =============================================================================

#include <pybind11/pybind11.h>
#include <string>

namespace py = pybind11;

namespace {

struct Config {
    int a;
    int b;
    Config(int a = 1, int b = 2) : a(a), b(b) {}
};

struct Dog {};  // marker type for the None-allowed/disallowed demo
struct Cat {};

// 5. default args
int add(int x, int y) { return x + y; }

// 6. kw_only: a is positional-or-keyword; b must be keyword.
int kw_only_add(int a, int b) { return a + b; }

// 7. pos_only: a is positional-only; b is positional-or-keyword.
int pos_only_add(int a, int b) { return a + b; }

// 8. noconvert: only_float will reject ints (no implicit int->float),
//   supports_float will accept them.
double half(double f) { return 0.5 * f; }

// 9. none: bark accepts None (becomes nullptr); meow rejects None.
std::string bark(Dog *d) { return d ? "woof!" : "(no dog)"; }
std::string meow(Cat *c) { return c ? "meow"  : "(no cat)"; }

}  // namespace

PYBIND11_MODULE(arg_modifiers, m, py::mod_gil_not_used()) {
    // --- 5. default arguments ---
    // The default value is converted at binding time. py::arg_v lets you supply
    // a string used in the function's signature (otherwise pybind11 generates
    // one that may be ugly for complex types).
    py::class_<Config>(m, "Config")
        .def(py::init<int, int>(), py::arg("a") = 1, py::arg("b") = 2)
        .def_readwrite("a", &Config::a)
        .def_readwrite("b", &Config::b);

    m.def("add", &add, py::arg("x") = 10, py::arg("y") = 20,
          "Both arguments have defaults; call with 0, 1, or 2 args.");

    m.def("with_config",
          [](const Config &c) { return c.a + c.b; },
          // py::arg_v provides a nicer repr in help(); Config(3, 4) is the
          // default value used at runtime.
          py::arg_v("cfg", Config(3, 4), "Config(3, 4)"),
          "Default arg shown in signature as 'Config(3, 4)'.");

    // --- 6. keyword-only arguments ---
    // evertyhing after py::kw_only() must be passed as a keyword
    m.def("kw_only_add", &kw_only_add,
          py::arg("a"), py::kw_only(), py::arg("b"),
          "a is positional-or-keyword; b must be keyword.");

    // --- 7. positional-only arguments ---
    // everything before py::pos_only() must be passed positionally
    m.def("pos_only_add", &pos_only_add,
          py::arg("a"), py::pos_only(), py::arg("b"),
          "a is positional-only; b is positional-or-keyword.");

    // --- 8. .noconvert() ---
    m.def("supports_float", &half, py::arg("f"),
          "Accepts int via implicit int->float conversion.");
    m.def("only_float",     &half, py::arg("f").noconvert(),
          "Rejects int — only true floats.");

    // --- 9. .none() ---
    py::class_<Dog>(m, "Dog").def(py::init<>());
    py::class_<Cat>(m, "Cat").def(py::init<>());

    m.def("bark", &bark, py::arg("dog").none(true),
          "None becomes nullptr; bark(None) is legal.");
    m.def("meow", &meow, py::arg("cat").none(false),
          "Rejects None at the boundary with TypeError.");
}
