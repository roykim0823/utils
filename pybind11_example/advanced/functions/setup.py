from setuptools import setup, Extension
import pybind11


def ext(name, source):
    return Extension(
        name,
        [source],
        include_dirs=[pybind11.get_include()],
        language="c++",
        extra_compile_args=["-std=c++17"],
    )


ext_modules = [
    ext("rvp",            "return_value_policies/rvp.cpp"),
    ext("call_policies",  "call_policies/call_policies.cpp"),
    ext("python_args",    "python_args/python_args.cpp"),
    ext("arg_modifiers",  "arg_modifiers/arg_modifiers.cpp"),
    ext("overloads",      "overloads/overloads.cpp"),
]

setup(
    name="advanced_functions",
    version="0.1",
    ext_modules=ext_modules,
)
