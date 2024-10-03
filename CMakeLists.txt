cmake_minimum_required(VERSION 3.16)
project(aitools LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # Enable PIC for all targets

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Python3 COMPONENTS Interpreter Development)
find_package(OpenMP)

# Fetch doctest
FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

# Fetch FMT
FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 11.0.2
)
FetchContent_MakeAvailable(fmt)
include_directories(${fmt_SOURCE_DIR}/include)

# Fetch Lyra
FetchContent_Declare(
        lyra
        GIT_REPOSITORY https://github.com/bfgroup/Lyra.git
        GIT_TAG 1.6.1
)
FetchContent_MakeAvailable(lyra)
include_directories(${lyra_SOURCE_DIR}/include)

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
target_link_libraries(aitools LINK_PUBLIC aitoolslib Python3::Python pybind11::pybind11)

enable_testing()
add_subdirectory(tests)
add_subdirectory(tools)

install(TARGETS aitoolslib RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
