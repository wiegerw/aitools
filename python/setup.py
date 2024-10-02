# Copyright 2021 - 2024 Wieger Wesselink.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or http://www.boost.org/LICENSE_1_0.txt)

from pybind11.setup_helpers import Pybind11Extension
from setuptools import setup

import os
import sys

__version__ = "0.1.0"

define_macros = [('VERSION_INFO', __version__)]
include_dirs = ['../include']
extra_compile_args = ['-DFMT_HEADER_ONLY']
extra_link_args = ['-ltbb'] if not sys.platform.startswith("win") else []

# We need to use absolute paths, since the src folder is in the parent directory.
current_dir = os.path.abspath(os.path.dirname(__file__))
src_dir = os.path.abspath(os.path.join(current_dir, "../src"))

ext_modules = [
    Pybind11Extension(
        "aitools",
        [
            os.path.join(src_dir, "decision_trees.cpp"),
            os.path.join(src_dir, "logger.cpp"),
            os.path.join(src_dir, "probabilistic_circuits.cpp"),
            os.path.join(src_dir, "python-bindings.cpp"),
            os.path.join(src_dir, "utilities.cpp"),
        ],
        define_macros=define_macros,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
        include_dirs=include_dirs,
        cxx_std=17
    ),
]

setup(
    name="aitools",
    version=__version__,
    author="Wieger Wesselink",
    author_email="j.w.wesselink@tue.nl",
    description="C++ library for basic AI data structures and algorithms",
    long_description="",
    ext_modules=ext_modules,
    zip_safe=False
)
