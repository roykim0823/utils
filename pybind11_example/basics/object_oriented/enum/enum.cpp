#include <string>
#include <pybind11/pybind11.h>
#include <pybind11/native_enum.h>  // Not already included with pybind11.h

namespace py = pybind11;

// Anonymous namespace -> this Pet is a distinct C++ type from pet/pet.h's Pet,
// so pybind11's global type registry won't see a collision when both .so's load.
namespace {
struct Pet {
    enum Kind {
        Dog = 0,
        Cat
    };

    struct Attributes {
        float age = 0;
    };

    Pet(const std::string &name, Kind type) : name(name), type(type) { }

    std::string name;
    Kind type;
    Attributes attr;
};
}  // namespace

PYBIND11_MODULE(pet_enum, m, py::mod_gil_not_used()) {
    py::class_<Pet> pet(m, "Pet");

    pet.def(py::init<const std::string &, Pet::Kind>())
        .def_readwrite("name", &Pet::name)
        .def_readwrite("type", &Pet::type)
        .def_readwrite("attr", &Pet::attr);

    py::native_enum<Pet::Kind>(pet, "Kind", "enum.Enum")
        .value("Dog", Pet::Kind::Dog)
        .value("Cat", Pet::Kind::Cat)
        .export_values()
        .finalize();

    py::class_<Pet::Attributes>(pet, "Attributes")
        .def(py::init<>())
        .def_readwrite("age", &Pet::Attributes::age);
}
