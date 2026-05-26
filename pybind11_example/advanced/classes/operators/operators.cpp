// =============================================================================
// Topic: Operator overloading via pybind11/operators.h
// Docs: https://pybind11.readthedocs.io/en/latest/advanced/classes.html
//       #operator-overloading
//
// pybind11 exposes `py::self` as a placeholder for "the bound type". Combining
// it with operators inside `.def(...)` registers the corresponding Python
// dunder. Mixed-type operators (Vec2 * double, double * Vec2) need both
// orderings registered separately.
//   py::self + py::self       -> __add__
//   py::self += py::self      -> __iadd__
//   py::self * float()        -> __mul__   (Vec2 on the LEFT)
//   float()  * py::self       -> __rmul__  (Vec2 on the RIGHT)
//   -py::self                 -> __neg__
//   py::self == py::self      -> __eq__
//   hash(py::self)            -> __hash__  (needs std::hash<T> specialization)
// =============================================================================

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <sstream>
#include <string>

namespace py = pybind11;

namespace {

struct Vec2 {
    double x, y;

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator-()              const { return {-x, -y};            }
    Vec2 operator*(double s)      const { return {x * s, y * s};      }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }

    std::string repr() const {
        std::ostringstream os;
        os << "Vec2(" << x << ", " << y << ")";
        return os.str();
    }
};

// Free function so we can register the reversed mixed-type op `double * Vec2`.
Vec2 operator*(double s, const Vec2& v) { return v * s; }

}  // namespace

PYBIND11_MODULE(operators, m, py::mod_gil_not_used()) {
    py::class_<Vec2>(m, "Vec2")
        .def(py::init<double, double>(), py::arg("x"), py::arg("y"))
        .def_readwrite("x", &Vec2::x)
        .def_readwrite("y", &Vec2::y)

        // Binary
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self == py::self)

        // In-place
        .def(py::self += py::self)

        // Unary
        .def(-py::self)

        // Mixed type: register BOTH directions for commutativity.
        .def(py::self * double())
        .def(double() * py::self)

        .def("__repr__", &Vec2::repr);
}
