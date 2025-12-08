cmake_minimum_required(VERSION 3.15...4.0)

project(glsim VERSION 0.1.0)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

add_subdirectory(external/pybind11)

# ------------------------------------------------------------------------------
# glsim module
# ------------------------------------------------------------------------------
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.h")

python_add_library(_glsim MODULE ${SOURCE_FILES} WITH_SOABI)

target_include_directories(_glsim PRIVATE
    src/
    external/pybind11/include/
)
target_link_libraries(_glsim PRIVATE pybind11::headers)
target_precompile_headers(_glsim PRIVATE src/pch.h)

# ------------------------------------------------------------------------------
# Install target
# ------------------------------------------------------------------------------

# We tell CMake to output the binaries directly into the Python package folder
set_target_properties(_glsim PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/glsim"
)

# The install directory is the output (wheel) directory
install(TARGETS _glsim DESTINATION "glsim")
