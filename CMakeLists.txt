cmake_minimum_required(VERSION 3.15...4.0)
project(glsim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(external/pybind11)

# set(GENERATE_DEBUG_SYMBOLS OFF CACHE BOOL "" FORCE)
set(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE OFF CACHE BOOL "" FORCE)
add_subdirectory(external/JoltPhysics/Build)

# Add your source files
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.h")

python_add_library(_glsim MODULE ${SOURCE_FILES} WITH_SOABI)

target_include_directories(_glsim PRIVATE
    src/
    external/pybind11/include/
    external/JoltPhysics/
)
target_link_libraries(_glsim PRIVATE pybind11::headers Jolt)

target_precompile_headers(_glsim PRIVATE src/pch.h)

# We tell CMake to output the binary directly into the Python package folder
set_target_properties(_glsim PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/glsim"
)

# The install directory is the output (wheel) directory
install(TARGETS _glsim DESTINATION "$glsim")
