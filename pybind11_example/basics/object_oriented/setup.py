from setuptools import setup, Extension
import pybind11

ext_modules = [
    Extension(
        "pet",
        ["pet/pet_pybind.cpp"],
        include_dirs=[pybind11.get_include()],
        language="c++",
    ),
    Extension(
        "pitfall",
        ["pitfall/pitfall_raw_poiners.cpp"],
        include_dirs=[pybind11.get_include()],
        language="c++",
    ),
    Extension(
        "pet_enum",
        ["enum/enum.cpp"],
        include_dirs=[pybind11.get_include()],
        language="c++",
    ),
]

setup(
    name="object_oriented",
    version="0.1",
    ext_modules=ext_modules,
)
