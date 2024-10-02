cmake_minimum_required(VERSION 3.16)
project(aitools LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Python3 COMPONENTS Interpreter Development)
find_package(OpenMP)

# Fetch pybind11
FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v2.13.5
)
FetchContent_MakeAvailable(pybind11)

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

install(TARGETS aitoolslib RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
