# Advanced Functions — pybind11 Tutorial

This directory walks through the *Advanced → Functions* chapter of the pybind11
manual ([docs](https://pybind11.readthedocs.io/en/stable/advanced/functions.html))
through five small, self-contained extension modules. Each module focuses on a
single topic, has a matching test script under `test/`, and is intended to be
read top-to-bottom alongside its docs.

```
advanced/functions/
├── return_value_policies/rvp.cpp     # 1. Return value policies
├── call_policies/call_policies.cpp   # 2. keep_alive, call_guard
├── python_args/python_args.cpp       # 3. py::dict/list, py::args, py::kwargs
├── arg_modifiers/arg_modifiers.cpp   # 4. defaults, kw_only, pos_only,
│                                     #    .noconvert(), .none()
├── overloads/overloads.cpp           # 5. overload resolution + templates
├── test/                             # one runnable demo per topic
├── setup.py                          # builds all five .so modules
└── pyproject.toml
```

## Building

The five modules are built as a single `setup.py` invocation. Use
[`uv`](https://docs.astral.sh/uv/) to set up the venv and install the build
dependencies, then run `setup.py` to compile the extensions in-place:

```bash
# Create and activate a Python 3.12 venv (one-time).
uv venv --python 3.12
source .venv/bin/activate

# Install build deps and compile the .so files next to setup.py.
uv pip install pybind11 setuptools wheel
python setup.py build_ext --inplace
```

That drops `rvp.cpython-*.so`, `call_policies.cpython-*.so`, … next to
`setup.py`. Run any demo from this directory so the `.so` files are on the
import path:

```bash
PYTHONPATH=. python test/rvp_test.py
PYTHONPATH=. python test/call_policies_test.py
PYTHONPATH=. python test/python_args_test.py
PYTHONPATH=. python test/arg_modifiers_test.py
PYTHONPATH=. python test/overloads_test.py
```

> The bindings use `py::mod_gil_not_used()`, which marks each module as
> compatible with the free-threaded (no-GIL) Python build. It is harmless on a
> standard GIL-enabled interpreter.

---

## 1. Return Value Policies — `return_value_policies/rvp.cpp`

When a C++ function returns a **pointer** or **reference**, pybind11 has to
decide who owns the underlying object and how long it must live. Pick the wrong
policy and you get a memory leak, a double-free, or a use-after-free. The seven
policies (`automatic`, `take_ownership`, `copy`, `move`, `reference`,
`reference_internal`, `automatic_reference`) are listed in the
[docs](https://pybind11.readthedocs.io/en/stable/advanced/functions.html#return-value-policies);
this module covers the four you'll actually use day-to-day plus the classic
foot-gun.

```cpp
py::class_<Owner>(m, "Owner")
    .def(py::init<>())
    // SAFE: lifetime of the returned Data is tied to the Owner.
    .def("get_ref_internal", &Owner::get_ref,
         py::return_value_policy::reference_internal)
    // SAFE: pybind11 makes a Python-owned copy.
    .def("get_copy", &Owner::get_copy,
         py::return_value_policy::copy)
    // SAFE here: C++ keeps ownership, Python won't delete.
    .def("get_ref", &Owner::get_ref,
         py::return_value_policy::reference)
    // DANGEROUS: take_ownership on a *borrowed* pointer -> double free.
    .def("get_ptr_take_ownership_DANGER", &Owner::get_ptr,
         py::return_value_policy::take_ownership);

// A genuine factory: take_ownership is the right call (and the default
// for raw pointer returns, i.e. `automatic`).
m.def("make_data", &make_data, py::arg("value"), py::arg("label"),
      py::return_value_policy::take_ownership);
```

Rules of thumb:

| You return…                       | Pick                                       |
|-----------------------------------|--------------------------------------------|
| a newly-allocated `T*` (factory)  | `take_ownership` (also the default)        |
| `T&` to a member of `this`        | `reference_internal`                       |
| `T` (by value)                    | `copy` (or default — pybind11 moves/copies)|
| `T*`/`T&` to a singleton/global   | `reference`                                |
| `T*`/`T&` to something C++ might delete | rethink the API — at minimum `copy`  |

The DANGER variant is registered intentionally but **not called** by the test;
the comment in the test file shows how to opt-in if you want to watch the
SIGABRT.

---

## 2. Additional Call Policies — `call_policies/call_policies.cpp`

Call policies are extra annotations passed to `.def(...)` that change *how* the
call is sequenced. Two are essential:

### `py::keep_alive<Nurse, Patient>()`

Keeps argument `Patient` alive **at least as long as** `Nurse` lives. Indexing:
`0` = return value, `1` = `self`, `2..N` = positional args. Use it whenever a
container stores a raw reference to one of its arguments — without it, the user
could `del` the source and leave a dangling pointer inside your container.

```cpp
py::class_<PtrList>(m, "PtrList")
    .def(py::init<>())
    // Without keep_alive<1, 2>: append(obj) then `del obj` -> dangling pointer.
    .def("append", &PtrList::append, py::keep_alive<1, 2>());
```

### `py::call_guard<Guard1, Guard2, ...>()`

Wraps every C++ call in RAII guards. Guards are **constructed left-to-right**
and **destroyed right-to-left** (normal C++ aggregate semantics). If `Guard2`
needs to observe state set up by `Guard1`, `Guard1` must come first.

The most common single guard releases the GIL so other Python threads can run:

```cpp
m.def("sleep_with_gil",    &sleep_ms, py::arg("ms"));
m.def("sleep_release_gil", &sleep_ms, py::arg("ms"),
      py::call_guard<py::gil_scoped_release>());
```

Observed by the test (4 threads × 100 ms `sleep_ms`):

```
WITH GIL held  (serial):    ~400 ms
WITHOUT GIL    (parallel):  ~100 ms
```

The construction-order demo uses two toy guards — `CustomGuard` sets a
thread-local flag in its ctor, `DependentGuard` snapshots it in its ctor:

```cpp
m.def("guarded_call_correct_order", &report_guards,
      py::call_guard<CustomGuard, DependentGuard>());   // sees flag = true
m.def("guarded_call_wrong_order",   &report_guards,
      py::call_guard<DependentGuard, CustomGuard>());   // sees flag = false
```

This mirrors a real-world pattern: e.g. `gil_scoped_release` must come **after**
any guard that touches Python state.

---

## 3. Python Objects as Arguments — `python_args/python_args.cpp`

pybind11 ships C++ wrapper types (`py::dict`, `py::list`, `py::tuple`,
`py::str`, `py::set`, `py::object`, …) that are reference-counted handles to
real Python objects. Use them whenever you want to skip a conversion through
`std::map` / `std::vector` and just operate on the Python object directly.

```cpp
std::string format_dict(const py::dict &d) {
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (auto item : d) {
        if (!first) oss << ", ";
        oss << std::string(py::str(item.first)) << ": "
            << std::string(py::str(item.second));
        first = false;
    }
    oss << "}";
    return oss.str();
}
```

`py::args` and `py::kwargs` are subclasses of `py::tuple` / `py::dict` that
capture a function's variadic positional / keyword arguments — the Python
`*args` / `**kwargs`. `kwargs`, if declared, **must be last**.

```cpp
std::string describe_args(py::args args, const py::kwargs &kwargs);

std::string greet_many(const std::string &greeting,
                       py::args names,
                       const py::kwargs &opts);

m.def("describe_args", &describe_args);
m.def("greet_many",    &greet_many, py::arg("greeting"));
```

You can mix fixed arguments before `py::args`; everything not bound by name
flows into the variadic capture.

---

## 4. Argument Modifiers — `arg_modifiers/arg_modifiers.cpp`

Five small features that change how Python calls map onto your C++ signature.

### 4.1 Default arguments (`py::arg("x") = value`, `py::arg_v`)

```cpp
m.def("add", &add, py::arg("x") = 10, py::arg("y") = 20);

// py::arg_v lets you control the *signature string* shown in help().
m.def("with_config",
      [](const Config &c) { return c.a + c.b; },
      py::arg_v("cfg", Config(3, 4), "Config(3, 4)"));
```

Without `arg_v`, complex default values render as ugly auto-generated
representations in `help()`; with it, the signature reads `cfg: Config =
Config(3, 4)`.

### 4.2 Keyword-only — `py::kw_only()`

Everything after `py::kw_only()` must be passed by name:

```cpp
m.def("kw_only_add", &kw_only_add,
      py::arg("a"), py::kw_only(), py::arg("b"));
// kw_only_add(1, b=2)  -> 3
// kw_only_add(1, 2)    -> TypeError
```

### 4.3 Positional-only — `py::pos_only()`

Everything before `py::pos_only()` is positional-only:

```cpp
m.def("pos_only_add", &pos_only_add,
      py::arg("a"), py::pos_only(), py::arg("b"));
// pos_only_add(1, 2)     -> 3
// pos_only_add(1, b=2)   -> 3
// pos_only_add(a=1, b=2) -> TypeError
```

### 4.4 Disable implicit conversion — `.noconvert()`

By default pybind11 happily converts `int` → `float`. `.noconvert()` makes a
parameter strict:

```cpp
m.def("supports_float", &half, py::arg("f"));               // accepts int
m.def("only_float",     &half, py::arg("f").noconvert());   // rejects int
```

### 4.5 Allow / forbid `None` — `.none(true)` / `.none(false)`

For pointer-typed arguments to bound classes, `None` is converted to `nullptr`
by default. You can opt in or out explicitly:

```cpp
m.def("bark", &bark, py::arg("dog").none(true));   // bark(None) -> nullptr
m.def("meow", &meow, py::arg("cat").none(false));  // meow(None) -> TypeError
```

---

## 5. Overloads & Templates — `overloads/overloads.cpp`

### 5.1 Overload resolution order

When several C++ functions are bound under the **same Python name**, pybind11
dispatches in two passes:

1. **Pass 1** — try each overload with **no implicit conversions**.
2. **Pass 2** — if none matched, try each overload **allowing conversions**, in
   the order they were registered.

```cpp
m.def("describe", py::overload_cast<int>   (&describe));
m.def("describe", py::overload_cast<double>(&describe));
// describe(5)   -> "int=5"
// describe(5.5) -> "double=5.500000"
```

`py::overload_cast<...>` resolves the function pointer when overloads exist; use
`py::overload_cast<Args...>(&fn, py::const_)` for the `const` member-function
variant.

### 5.2 `py::prepend()` — insert at the FRONT of the overload list

Registration order matters: a general overload registered first will shadow a
more specific one registered later. `py::prepend()` flips that and inserts the
new overload at the front:

```cpp
// General overload first ...
m.def("handle", &handle_general, py::arg("o"));
// ... then prepend a more-specific one so it's tried first.
m.def("handle", &handle_str, py::arg("s"), py::prepend());
// handle("hi") -> "str-specific: hi"
// handle(42)   -> "general: 42"
```

This is the idiomatic way to add a fast-path specialization to a binding you
don't control without rewriting the original registration.

### 5.3 Binding template functions

pybind11 binds **concrete function pointers**, so you can't bind an
un-instantiated template — instantiate each specialization yourself:

```cpp
template <typename T>
std::string typed_label(T value, const std::string &tag);

// Three instantiations under one Python name (becomes a 3-way overload).
m.def("typed_label", &typed_label<int>);
m.def("typed_label", &typed_label<double>);
m.def("typed_label", &typed_label<std::string>);

// Or expose them under distinct names if you'd rather not overload.
m.def("typed_label_int",    &typed_label<int>);
m.def("typed_label_string", &typed_label<std::string>);
```

---

## A short cheat sheet

| You want to…                                          | Use                                          |
|-------------------------------------------------------|----------------------------------------------|
| Return a freshly-allocated object Python should own   | `py::return_value_policy::take_ownership` (default for `T*`) |
| Return a reference to a member of `this`              | `py::return_value_policy::reference_internal`|
| Tie an arg's lifetime to a container/self             | `py::keep_alive<Nurse, Patient>()`           |
| Let other Python threads run during a long C++ call   | `py::call_guard<py::gil_scoped_release>()`   |
| Accept a Python dict/list/tuple without converting    | `py::dict`, `py::list`, `py::tuple`          |
| Accept `*args` / `**kwargs`                           | `py::args`, `py::kwargs` (kwargs last)       |
| Force keyword-only / positional-only parameters       | `py::kw_only()`, `py::pos_only()`            |
| Disable implicit conversions                          | `py::arg("x").noconvert()`                   |
| Allow / forbid `None` for pointer args                | `py::arg("x").none(true|false)`              |
| Add an overload at the front of an existing list      | `py::prepend()`                              |
| Bind a template function                              | One `m.def` per instantiation                |

For the canonical list and edge cases, follow each section's link back to the
[pybind11 advanced/functions docs](https://pybind11.readthedocs.io/en/stable/advanced/functions.html).
