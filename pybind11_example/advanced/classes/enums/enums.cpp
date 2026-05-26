// =============================================================================
// Topic: Enumerations
// Docs: https://pybind11.readthedocs.io/en/stable/classes.html#enumerations-and-internal-types
//       https://pybind11.readthedocs.io/en/stable/classes.html#native-enumerations-using-the-enum-module
//
// Two APIs:
//   py::enum_<E>(scope, "Name")
//       Custom pybind11 enum class. Use .export_values() to re-export values
//       into the enclosing module scope (only meaningful for UNSCOPED C enums).
//       Note: in pybind11 3.x, py::arithmetic() registers __int__ only —
//       bitwise operators do NOT work on the resulting type.
//
//   py::native_enum<E>(scope, "Name", "enum.IntFlag" | "enum.IntEnum" | ...)
//       Bridges to Python's stdlib `enum` module. Pick the Python base:
//         "enum.Enum"     — opaque values
//         "enum.IntEnum"  — int-comparable
//         "enum.IntFlag"  — int-comparable AND supports |, &, ^, ~ natively
//       Requires `.finalize()` to commit the registration.
// =============================================================================

#include <pybind11/pybind11.h>
#include <pybind11/native_enum.h>

namespace py = pybind11;

namespace {

// Unscoped C-style enum.
enum Color {
    Red   = 1,
    Green = 2,
    Blue  = 4,
};

// C++11 scoped enum class — values namespaced under Status::.
enum class Status {
    Ok,
    Warning,
    Error,
};

// Bitflag-style scoped enum, exercised with py::arithmetic().
enum class Perm : unsigned {
    None  = 0,
    Read  = 1u << 0,
    Write = 1u << 1,
    Exec  = 1u << 2,
};

int describe_color(Color c)   { return static_cast<int>(c); }
int describe_status(Status s) { return static_cast<int>(s); }
unsigned describe_perm(Perm p) { return static_cast<unsigned>(p); }

}  // namespace

PYBIND11_MODULE(enums, m, py::mod_gil_not_used()) {
    // --- Unscoped enum ---
    // .export_values() places Red/Green/Blue at module level too.
    py::enum_<Color>(m, "Color")
        .value("Red",   Red)
        .value("Green", Green)
        .value("Blue",  Blue)
        .export_values();

    // --- C++11 scoped enum class — stays scoped (no export_values) ---
    py::enum_<Status>(m, "Status")
        .value("Ok",      Status::Ok)
        .value("Warning", Status::Warning)
        .value("Error",   Status::Error);

    // --- Scoped enum exposed as Python enum.IntFlag for native | / & / ~ ---
    // native_enum bridges to stdlib enum types; IntFlag gives bitflag semantics
    // for free. Must call .finalize() to commit.
    py::native_enum<Perm>(m, "Perm", "enum.IntFlag")
        .value("None_", Perm::None)   // avoid clash with Python's None keyword
        .value("Read",  Perm::Read)
        .value("Write", Perm::Write)
        .value("Exec",  Perm::Exec)
        .finalize();

    m.def("describe_color",  &describe_color,  py::arg("c"));
    m.def("describe_status", &describe_status, py::arg("s"));
    m.def("describe_perm",   &describe_perm,   py::arg("p"));
}
