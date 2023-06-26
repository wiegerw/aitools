cmake_minimum_required(VERSION 3.16)
project(aitools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Python3 COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)
find_package(OpenMP)

if (UNIX)
    find_package(TBB REQUIRED)
endif()

include_directories(include)

add_compile_definitions(FMT_HEADER_ONLY)

add_library(aitoolslib src/logger.cpp src/probabilistic_circuits.cpp src/decision_trees.cpp src/utilities.cpp)

pybind11_add_module(aitools src/python-bindings.cpp)

enable_testing()
add_subdirectory(tests)
add_subdirectory(tools)
add_subdirectory(tools/experimental)
