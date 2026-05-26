# Advanced Classes — pybind11 Tutorial

This directory walks through the *Advanced → Classes* chapter of the pybind11
manual ([docs](https://pybind11.readthedocs.io/en/stable/advanced/classes.html))
through seven small, self-contained extension modules. Each module focuses on a
distinct group of features, has a matching test script under `test/`, and is
meant to be read alongside its docs section.

```
advanced/classes/
├── trampoline/trampoline.cpp     # 1. Virtual function overrides (trampolines)
├── custom_init/custom_init.cpp   # 2. py::init(lambda), py::nodelete
├── inheritance/inheritance.cpp   # 3. Multiple inheritance, module_local,
│                                 #    is_final, protected publicist, templates
├── conversions/conversions.cpp   # 4. py::implicitly_convertible
├── operators/operators.cpp       # 5. Operator overloading
├── pickling/pickling.cpp         # 6. py::pickle + deepcopy
├── enums/enums.cpp               # 7. py::enum_ / py::native_enum
├── test/                         # one runnable demo per topic
├── setup.py                      # builds all seven .so modules
└── pyproject.toml
```

## Building

Use [`uv`](https://docs.astral.sh/uv/) to set up the venv and install the build
dependencies, then run `setup.py` to compile the extensions in-place:

```bash
# Create and activate a Python 3.12 venv (one-time).
uv venv --python 3.12
source .venv/bin/activate

# Install build deps and compile the .so files next to setup.py.
uv pip install pybind11 setuptools wheel
python setup.py build_ext --inplace
```

That drops `trampoline.cpython-*.so`, `custom_init.cpython-*.so`, … next to
`setup.py`. Run any demo from this directory so the `.so` files are on the
import path:

```bash
PYTHONPATH=. python test/trampoline_test.py
PYTHONPATH=. python test/custom_init_test.py
PYTHONPATH=. python test/inheritance_test.py
PYTHONPATH=. python test/conversions_test.py
PYTHONPATH=. python test/operators_test.py
PYTHONPATH=. python test/pickling_test.py
PYTHONPATH=. python test/enums_test.py
```

> The bindings use `py::mod_gil_not_used()`, which marks each module as
> compatible with the free-threaded (no-GIL) Python build. It is harmless on a
> standard GIL-enabled interpreter.

---

## 1. Virtual Function Overrides — `trampoline/trampoline.cpp`

To let Python subclasses override C++ virtual functions, you supply a
**trampoline class**: a C++ subclass that, for each virtual, forwards the call
to Python if there's an override and to the C++ base implementation otherwise.

Two macros do the routing:

| Macro                         | Behavior                                                             |
|-------------------------------|----------------------------------------------------------------------|
| `PYBIND11_OVERRIDE_PURE(...)` | Throws `RuntimeError` if no Python override is found (pure virtual). |
| `PYBIND11_OVERRIDE(...)`      | Falls back to the C++ base implementation.                           |

```cpp
struct Animal {
    virtual ~Animal() = default;
    virtual std::string go(int n) = 0;            // pure
    virtual std::string name() const { return "Animal"; }
};

struct PyAnimal : Animal {
    using Animal::Animal;
    std::string go(int n) override {
        PYBIND11_OVERRIDE_PURE(std::string, Animal, go, n);
    }
    std::string name() const override {
        PYBIND11_OVERRIDE(std::string, Animal, name, );   // trailing comma!
    }
};

py::class_<Animal, PyAnimal>(m, "Animal")
    .def(py::init<>())
    .def("go",   &Animal::go,   py::arg("n_times"))
    .def("name", &Animal::name);
```

The zero-argument override form requires a **trailing comma** —
`PYBIND11_OVERRIDE(ret, base, name, )`.

### Trampolines through an inheritance chain

If a C++ subclass (`Dog`) can also be subclassed in Python, it needs **its own
trampoline** (`PyDog`); otherwise Python overrides on that level are silently
dropped when C++ dispatches through the base pointer.

```cpp
struct PyDog : Dog {
    using Dog::Dog;
    std::string go(int n) override   { PYBIND11_OVERRIDE(std::string, Dog, go, n);   }
    std::string name() const override { PYBIND11_OVERRIDE(std::string, Dog, name, ); }
    std::string fetch() const override { PYBIND11_OVERRIDE(std::string, Dog, fetch, ); }
};

py::class_<Dog, Animal, PyDog>(m, "Dog")        // base AND trampoline
    .def(py::init<>())
    .def("fetch", &Dog::fetch);
```

The test verifies `call_go(animal*, n)` on the C++ side dispatches into the
Python `Poodle.go` override.

---

## 2. Custom Constructors & Non-public Destructors — `custom_init/custom_init.cpp`

### `py::init([](...) { ... })` — factory constructors

When `py::init<Args...>()` (which binds a real constructor) isn't enough, pass
a factory lambda. The lambda's return type decides ownership:

| Return type        | Behavior                                            |
|--------------------|-----------------------------------------------------|
| `T`                | Placement-new into pybind11's buffer (value return) |
| `T*`               | pybind11 takes ownership (must be heap-allocated)   |
| `std::unique_ptr`  | Matches a `unique_ptr` holder                       |
| `std::shared_ptr`  | Matches a `shared_ptr` holder                       |

```cpp
py::class_<Widget>(m, "Widget")
    .def(py::init<int, std::string>(), py::arg("x"), py::arg("label"))   // 2a
    .def(py::init([](int x) {                                            // 2b
             return Widget(x, "from-int");
         }), py::arg("x"))
    .def(py::init([](const std::string& label) {                         // 2c
             return std::make_unique<Widget>(0, label);
         }), py::arg("label"));
```

All three are bound under the same `__init__`; pybind11 picks the overload by
argument types — `Widget(7, "alpha")`, `Widget(42)`, `Widget("named-only")`.

### Non-public destructor → `py::nodelete` holder

pybind11's default holder is `std::unique_ptr<T>`, which calls `delete` at GC
time. That **won't compile** if `~T` is private (singleton-style classes). Tell
pybind11 not to delete:

```cpp
py::class_<Singleton,
           std::unique_ptr<Singleton, py::nodelete>>(m, "Singleton")
    .def_static("instance", &Singleton::instance,
                py::return_value_policy::reference)
    .def("counter", &Singleton::counter)
    .def("bump",    &Singleton::bump);
```

Don't bind any `py::init`, so Python can't construct one — it goes through
`Singleton.instance()`, which returns the same C++ object every time.

---

## 3. Inheritance Edge Cases — `inheritance/inheritance.cpp`

### 3.1 Multiple inheritance

List **every** base as a template argument to `py::class_`. If a base isn't
itself bound, use `py::multiple_inheritance()` as a tag in the constructor
arguments.

```cpp
py::class_<Walker> (m, "Walker") .def(py::init<>()).def("walk", &Walker::walk);
py::class_<Swimmer>(m, "Swimmer").def(py::init<>()).def("swim", &Swimmer::swim);

py::class_<Amphibian, Walker, Swimmer>(m, "Amphibian")
    .def(py::init<>())
    .def("both", &Amphibian::both);
```

### 3.2 Module-local bindings — `py::module_local()`

Marks the binding as belonging to **this module only**. When the same C++ type
ships in two wheels, both can bind it without an "already registered" clash.

```cpp
py::class_<LocalOnly>(m, "LocalOnly", py::module_local())
    .def(py::init<int>(), py::arg("v") = 0)
    .def_readwrite("value", &LocalOnly::value);
```

### 3.3 Final classes — `py::is_final()`

Python `class T(Sealed): pass` raises `TypeError`:

```cpp
py::class_<Sealed>(m, "Sealed", py::is_final())
    .def(py::init<int>(), py::arg("x"))
    .def("doubled", &Sealed::doubled);
```

### 3.4 Protected members via the *publicist* pattern

A protected member can't be `&Cls::method`-bound directly. Create a public
subclass that re-exposes it with `using`, then bind through that:

```cpp
class WithSecret {
public:
    WithSecret(int seed) : seed_(seed) {}
    int public_api() const { return seed_; }
protected:
    int hidden_compute(int n) const { return seed_ * n; }
private:
    int seed_;
};

class WithSecretPub : public WithSecret {
public:
    using WithSecret::hidden_compute;       // republish
};

py::class_<WithSecret>(m, "WithSecret")
    .def(py::init<int>(), py::arg("seed"))
    .def("public_api", &WithSecret::public_api)
    .def("hidden_compute", &WithSecretPub::hidden_compute, py::arg("n"));
```

The Python type is still `WithSecret`; the publicist is just plumbing.

### 3.5 Template classes — one binding per instantiation

pybind11 binds concrete types, so each `Box<T>` is a separate `py::class_` —
typically under distinct Python names (`BoxInt`, `BoxStr`):

```cpp
py::class_<Box<int>>(m, "BoxInt").def(py::init<int>()).def("get", &Box<int>::get) ...;
py::class_<Box<std::string>>(m, "BoxStr") ...;
```

---

## 4. Implicit Conversions — `conversions/conversions.cpp`

`py::implicitly_convertible<Source, Target>()` tells pybind11 that any argument
expecting `Target` may receive a `Source`, and pybind11 will construct the
`Target` automatically. Requirement: `Target` must have a constructor taking
`Source` (it may be declared `explicit` — pybind11 calls it anyway).

```cpp
struct Meters { double value; explicit Meters(double v) : value(v) {} };
struct Feet {
    double value;
    explicit Feet(double v) : value(v) {}
    explicit Feet(const Meters& m) : value(m.value * 3.28084) {}
};

py::class_<Meters>(m, "Meters").def(py::init<double>()) ...;
py::class_<Feet>  (m, "Feet")
    .def(py::init<double>())
    .def(py::init<const Meters&>()) ...;

py::implicitly_convertible<Meters, Feet>();

m.def("print_feet", &print_feet, py::arg("f"));
// print_feet(Meters(10.0)) -> 32.8084 (pybind11 calls Feet(Meters))
```

This kicks in only on **pass 2** of overload resolution (the "allow
conversions" pass), so an exact-match overload still wins over conversion.

---

## 5. Operator Overloading — `operators/operators.cpp`

Include `pybind11/operators.h` and use `py::self` inside `.def(...)`. Each
operator expression registers the corresponding Python dunder:

```cpp
#include <pybind11/operators.h>

py::class_<Vec2>(m, "Vec2")
    .def(py::init<double, double>())
    .def_readwrite("x", &Vec2::x)
    .def_readwrite("y", &Vec2::y)
    .def(py::self + py::self)       // __add__
    .def(py::self - py::self)       // __sub__
    .def(py::self == py::self)      // __eq__
    .def(py::self += py::self)      // __iadd__
    .def(-py::self)                 // __neg__
    .def(py::self * double())       // __mul__   (Vec2 on the LEFT)
    .def(double() * py::self)       // __rmul__  (Vec2 on the RIGHT)
    .def("__repr__", &Vec2::repr);
```

Mixed-type operators are **not commutative for free** — `Vec2 * double` and
`double * Vec2` are two separate bindings. The right-hand-side form needs a
free function `operator*(double, const Vec2&)` to exist.

---

## 6. Pickling & Deepcopy — `pickling/pickling.cpp`

`py::pickle(getstate, setstate)` plugs into Python's pickle protocol via two
lambdas:

- `__getstate__(const T&) -> picklable object` (a `py::tuple` is conventional)
- `__setstate__(state) -> T` (a fresh instance built from the state)

```cpp
py::class_<Pickleable>(m, "Pickleable")
    .def(py::init<std::string, int>(), py::arg("name"), py::arg("counter"))
    .def_readwrite("name",    &Pickleable::name)
    .def_readwrite("counter", &Pickleable::counter)
    .def(py::pickle(
        [](const Pickleable& p) {
            return py::make_tuple(p.name, p.counter);
        },
        [](py::tuple t) {
            if (t.size() != 2) {
                throw std::runtime_error("Pickleable: invalid state tuple");
            }
            return Pickleable(t[0].cast<std::string>(),
                              t[1].cast<int>());
        }
    ));
```

Once pickling works, `copy.deepcopy` works **for free**: Python's `deepcopy`
falls back to `__reduce_ex__`, which `py::pickle` wires up. The test exercises
both, plus a corrupted-state path that surfaces the C++
`std::runtime_error` as a Python `RuntimeError`.

---

## 7. Enumerations — `enums/enums.cpp`

Two APIs depending on what kind of Python enum you want.

### 7.1 `py::enum_<E>` — pybind11's custom enum type

```cpp
// Unscoped C enum: .export_values() also puts Red/Green/Blue at module scope.
py::enum_<Color>(m, "Color")
    .value("Red",   Red)
    .value("Green", Green)
    .value("Blue",  Blue)
    .export_values();

// Scoped enum class: stays scoped, no export_values().
py::enum_<Status>(m, "Status")
    .value("Ok",      Status::Ok)
    .value("Warning", Status::Warning)
    .value("Error",   Status::Error);
```

In pybind11 3.x, `py::arithmetic()` only registers `__int__` — bitwise
operators (`|`, `&`, `~`) **do not** work on `py::enum_`-bound types. For
bitflag semantics, use `py::native_enum` instead.

### 7.2 `py::native_enum<E>` — bridge to stdlib `enum`

`py::native_enum` produces a real subclass of `enum.Enum` / `enum.IntEnum` /
`enum.IntFlag` from Python's standard library — pick the base name as a string,
and call `.finalize()` to commit the registration.

```cpp
#include <pybind11/native_enum.h>

py::native_enum<Perm>(m, "Perm", "enum.IntFlag")
    .value("None_", Perm::None)     // avoid clash with Python's `None`
    .value("Read",  Perm::Read)
    .value("Write", Perm::Write)
    .value("Exec",  Perm::Exec)
    .finalize();

// Python: Perm.Read | Perm.Write   -> Perm.Read|Write  (int value 3)
//         (Read|Write) & Read       -> Perm.Read
```

`IntFlag` gives `|`, `&`, `^`, `~` natively; `IntEnum` gives int-comparable
without the bitwise ops; `Enum` gives opaque values.

---

## A short cheat sheet

| You want to…                                                       | Use                                                       |
|--------------------------------------------------------------------|-----------------------------------------------------------|
| Let Python override a virtual                                      | Trampoline + `PYBIND11_OVERRIDE` / `_PURE`                |
| Allow Python to subclass an already-derived C++ class              | A trampoline at **each** level                            |
| Construct via a factory function                                   | `py::init([](...){ return T / T* / unique_ptr / shared_ptr; })` |
| Bind a class with private destructor                               | Holder `std::unique_ptr<T, py::nodelete>`                 |
| Multiple inheritance                                               | List all bases as `py::class_` template args              |
| Avoid cross-module type clashes                                    | `py::module_local()`                                      |
| Disallow Python subclassing                                        | `py::is_final()`                                          |
| Expose a protected method                                          | Publicist subclass with `using Base::method;`             |
| Bind a class template                                              | One `py::class_<Tmpl<T>>` per instantiation               |
| Auto-convert one bound type to another                             | `py::implicitly_convertible<Source, Target>()`            |
| Add Python operators (+ - * == hash …)                             | `#include <pybind11/operators.h>` + `py::self ...`        |
| Make a type picklable                                              | `.def(py::pickle(getstate, setstate))`                    |
| Get `copy.deepcopy` for free                                       | Already done if `py::pickle` is bound                     |
| Bind a C enum and re-export values                                 | `py::enum_<E>(m, "E")....export_values()`                 |
| Bind a `class enum` (stays scoped)                                 | `py::enum_<E>(m, "E")` (no `.export_values()`)            |
| Get a real `IntFlag` with `|`/`&`/`~`                              | `py::native_enum<E>(m, "E", "enum.IntFlag").finalize()`   |

For the canonical list and edge cases, follow each section's link back to the
[pybind11 advanced/classes docs](https://pybind11.readthedocs.io/en/stable/advanced/classes.html).
