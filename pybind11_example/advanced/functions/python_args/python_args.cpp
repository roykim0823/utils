// =============================================================================
// Topic 3 + 4: Python objects as arguments, *args and **kwargs
// Docs: https://pybind11.readthedocs.io/en/latest/advanced/functions.html
//       #python-objects-as-arguments
//       #accepting-args-and-kwargs
//
// pybind11 provides thin C++ wrappers (py::dict, py::list, py::tuple, py::str,
// py::set, py::object ...) so functions can accept native Python collections
// without forcing a conversion through std::map/std::vector. These wrappers
// are reference-counted handles to the underlying Python objects.
//
// py::args is a tuple subclass that captures any number of positional
// arguments. py::kwargs is a dict subclass that captures keyword arguments.
// kwargs (if present) must be the last argument.
// =============================================================================

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // not strictly needed, but handy
#include <sstream>
#include <string>

namespace py = pybind11;

namespace {

// 1. Accept a py::dict (native Python dict, no conversion to std::map).
std::string format_dict(const py::dict &d) {
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (auto item : d) {
        if (!first) oss << ", ";
        oss << std::string(py::str(item.first))
            << ": " << std::string(py::str(item.second));
        first = false;
    }
    oss << "}";
    return oss.str();
}

// 2. Accept a py::list.
size_t list_length(const py::list &lst) {
    return py::len(lst);
}

// 3. Accept py::args (positional) and py::kwargs (keyword) — both optional in
//    the sense that they may be empty, but if declared, kwargs must come last.
std::string describe_args(py::args args, const py::kwargs &kwargs) {
    std::ostringstream oss;
    oss << "args(" << args.size() << "): [";
    bool first = true;
    for (auto a : args) {
        if (!first) oss << ", ";
        oss << std::string(py::str(a));
        first = false;
    }
    oss << "]  kwargs(" << kwargs.size() << "): {";
    first = true;
    for (auto kv : kwargs) {
        if (!first) oss << ", ";
        oss << std::string(py::str(kv.first))
            << "=" << std::string(py::str(kv.second));
        first = false;
    }
    oss << "}";
    return oss.str();
}

// 4. Mix fixed and variadic — name must come positionally or by keyword,
//    everything else flows into args/kwargs.
std::string greet_many(const std::string &greeting,
                       py::args names,
                       const py::kwargs &opts) {
    std::ostringstream oss;
    oss << greeting;
    for (auto n : names) oss << " " << std::string(py::str(n));
    if (opts.contains("punct")) {
        oss << std::string(py::str(opts["punct"]));
    }
    return oss.str();
}

}  // namespace

PYBIND11_MODULE(python_args, m, py::mod_gil_not_used()) {
    m.def("format_dict", &format_dict, py::arg("d"),
          "Accept a native py::dict and format it as a string.");
    m.def("list_length", &list_length, py::arg("lst"),
          "Accept a native py::list and return its length.");
    m.def("describe_args", &describe_args,
          "Accept arbitrary *args and **kwargs.");
    m.def("greet_many", &greet_many,
          py::arg("greeting"),
          "greeting is fixed; the rest flow into *names / **opts. "
          "Recognized opt: 'punct' is appended to the result.");
}
