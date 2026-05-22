#include <memory>
#include <pybind11/pybind11.h>

namespace py = pybind11;

// ============================================================
// PROBLEM: raw pointer return with the default policy
// ============================================================
// pybind11 defaults to return_value_policy::take_ownership for raw pointers,
// so it constructs an INDEPENDENT shared_ptr from the returned Child*. Two
// unrelated shared_ptrs now both believe they own the same Child -> double-free
// at teardown (SIGABRT).
class Child { };

class Parent {
public:
   Parent() : child(std::make_shared<Child>()) { }
   Child *get_child() { return child.get(); }  // DANGER
private:
    std::shared_ptr<Child> child;
};

// ============================================================
// SOLUTION 1: change only the binding policy (no C++ change)
// ============================================================
// reference_internal tells pybind11: don't delete the returned Child, and keep
// the Parent alive while any Child wrapper from it lives. Use this when you
// can't (or don't want to) modify the C++ side.
// We need a distinct C++ class to demonstrate, because pybind11 won't let
// us register the same C++ type twice.
class ParentSafe1 {
public:
   ParentSafe1() : child(std::make_shared<Child>()) { }
   Child *get_child() { return child.get(); }
private:
    std::shared_ptr<Child> child;
};

// ============================================================
// SOLUTION 2: return std::shared_ptr<Child> directly from Parent
// ============================================================
// Child is unchanged. Parent's method returns shared_ptr<Child>, which pybind11
// folds into its shared-ownership machinery (Child is registered with
// shared_ptr as holder). Both ownerships share the same control block, so
// the Child is destroyed exactly once.
class ParentSafe2 {
public:
    ParentSafe2() : child(std::make_shared<Child>()) { }
    std::shared_ptr<Child> get_child() { return child; }  // SAFE
private:
    std::shared_ptr<Child> child;
};

// ============================================================
// SOLUTION 3: Child inherits std::enable_shared_from_this
// ============================================================
// Makes the *existing* raw-pointer signature safe. When a class is registered
// with shared_ptr as holder AND inherits enable_shared_from_this, pybind11
// uses shared_from_this() to wrap the returned raw pointer, sharing ownership
// with the original control block. Requires the object to have been created
// via shared_ptr in the first place (it is — see make_shared in the ctor).
class ChildSafe3 : public std::enable_shared_from_this<ChildSafe3> { };

class ParentSafe3 {
public:
    ParentSafe3() : child(std::make_shared<ChildSafe3>()) { }
    ChildSafe3 *get_child() { return child.get(); }  // raw pointer, but SAFE
private:
    std::shared_ptr<ChildSafe3> child;
};

PYBIND11_MODULE(pitfall, m, py::mod_gil_not_used()) {
    // ----- PROBLEM (kept to demonstrate the crash) -----
    py::class_<Child, std::shared_ptr<Child>>(m, "Child");
    py::class_<Parent, std::shared_ptr<Parent>>(m, "Parent")
       .def(py::init<>())
       .def("get_child", &Parent::get_child);  // PROBLEM

    // ----- SOLUTION 1: reference_internal -----
    py::class_<ParentSafe1, std::shared_ptr<ParentSafe1>>(m, "ParentSafe1")
       .def(py::init<>())
       .def("get_child", &ParentSafe1::get_child,
            py::return_value_policy::reference_internal);

    // ----- SOLUTION 2: return shared_ptr directly -----
    py::class_<ParentSafe2, std::shared_ptr<ParentSafe2>>(m, "ParentSafe2")
       .def(py::init<>())
       .def("get_child", &ParentSafe2::get_child);

    // ----- SOLUTION 3: enable_shared_from_this -----
    py::class_<ChildSafe3, std::shared_ptr<ChildSafe3>>(m, "ChildSafe3");
    py::class_<ParentSafe3, std::shared_ptr<ParentSafe3>>(m, "ParentSafe3")
       .def(py::init<>())
       .def("get_child", &ParentSafe3::get_child);
}
