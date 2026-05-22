#include <pybind11/pybind11.h>

namespace py = pybind11;

#include "pet.h"

PYBIND11_MODULE(pet, m, py::mod_gil_not_used()) {
    py::class_<Pet>(m, "Pet")
        .def(py::init<const std::string&>())
        .def("getName", &Pet::getName)
        .def("setName", &Pet::setName)

        // Add a __repr__ method for better string representation in Python
        .def("__repr__", [](const Pet &p) {
            return "<Pet named '" + p.getName() + "'>";
        })

        // Expose the 'name' member variable directly (optional)
        .def_readwrite("name", &Pet::name);

    // Method 1: template paramter:
    py::class_<Dog, Pet>(m, "Dog")
        .def(py::init<const std::string&>())
        .def("bark", &Dog::bark);

    // // Method 2: pass parent class_ object:
    // py::class_<Dog> (m, "Dog", pet)
    //     .def(py::init<const std::string&>())
    //     .def("bark", &Dog::bark);

    m.def("pet_store", []() {
        return std::unique_ptr<Pet>(new Dog("Molly"));
    });

    py::class_<Pet2>(m, "Pet2", py::dynamic_attr())
        .def(py::init<const std::string&>())
        .def("getName", &Pet2::getName)
        .def("setName", &Pet2::setName)

        // Add a __repr__ method for better string representation in Python
        .def("__repr__", [](const Pet2 &p) {
            return "<Pet2 named '" + p.getName() + "'>";
        })

        // Expose the 'name' private member variable via property (optional)
        .def_property("name", &Pet2::getName, &Pet2::setName);

    py::class_<PetO>(m, "PetO")
        .def(py::init<const std::string&, int>())
        .def("set", py::overload_cast<int>(&PetO::set), "Set the pet's age")
        .def("set", py::overload_cast<const std::string&>(&PetO::set), "Set the pet's name");

    py::class_<Widget>(m, "Widget")
        .def(py::init<>()) // Add a default constructor for Widget
        .def("foo_mutable", py::overload_cast<int, float>(&Widget::foo))
        .def("foo_const",   py::overload_cast<int, float>(&Widget::foo, py::const_));
}