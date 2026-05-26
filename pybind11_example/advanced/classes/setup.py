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
    ext("trampoline",   "trampoline/trampoline.cpp"),
    ext("custom_init",  "custom_init/custom_init.cpp"),
    ext("inheritance",  "inheritance/inheritance.cpp"),
    ext("conversions",  "conversions/conversions.cpp"),
    ext("operators",    "operators/operators.cpp"),
    ext("pickling",     "pickling/pickling.cpp"),
    ext("enums",        "enums/enums.cpp"),
]

setup(
    name="advanced_classes",
    version="0.1",
    ext_modules=ext_modules,
)
