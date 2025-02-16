import os

import skbuild
from setuptools import find_packages
from skbuild import setup

skbuild_path = os.path.dirname(skbuild.__file__)

setup(
    name="pycallm",
    version="0.1.2",
    description="The CaLLM python library",
    author="Thomas MARQUIS",
    license="MIT",
    packages=find_packages("python"),
    package_dir={"": "python"},
    package_data={"pycallm": ["*.so"]},
    python_requires=">=3.12",
    cmake_args=["-DRELEASE_TYPE=PYTHON", f"-DBASE_SKBUILD_LIB_PATH={skbuild_path}"],
    cmake_languages=("C",),
    cmake_install_dir="python/pycallm",
)
