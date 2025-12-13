cmake_minimum_required(VERSION 3.15...4.0)

project(glsim VERSION 0.1.0)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(GL_HEADLESS "Build headless mode" OFF)

# ------------------------------------------------------------------------------
# Configuration
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Append into module path for our own configurations
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(GL_DEBUG_BUILD)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(GL_RELEASE_BUILD)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_definitions(GL_DEBUG_BUILD)
    add_compile_definitions(GL_RELEASE_BUILD)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    add_compile_definitions(GL_RELEASE_BUILD)
    add_compile_definitions(GL_DIST_BUILD)
endif()

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
set(GL_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(GL_ENABLE_POSITION_INDEPENDENT_CODE "" FORCE)
add_subdirectory(external/glgpu)

add_subdirectory(external/pybind11)

# Build shaders
add_subdirectory(shaders)

# ------------------------------------------------------------------------------
# glsim module
# ------------------------------------------------------------------------------
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.h")

python_add_library(_glsim MODULE ${SOURCE_FILES} WITH_SOABI)

target_include_directories(_glsim PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/external/pybind11/include/
    ${CMAKE_CURRENT_LIST_DIR}/external/glgpu/include/
)
target_link_libraries(_glsim PRIVATE glgpu pybind11::headers)
target_precompile_headers(_glsim PRIVATE src/pch.h)

# If not headless mode link into SDL2
if(NOT GL_HEADLESS)
    find_package(SDL2 REQUIRED)
    target_link_libraries(_glsim PRIVATE SDL2)
else()
    target_compile_definitions(_glsim PRIVATE GL_HEADLESS)
endif()

# ------------------------------------------------------------------------------
# Install target
# ------------------------------------------------------------------------------

# We tell CMake to output the binaries directly into the Python package folder
set_target_properties(_glsim PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/glsim"
)

# The install directory is the output (wheel) directory
install(TARGETS _glsim DESTINATION "glsim")
