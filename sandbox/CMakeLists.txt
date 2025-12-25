file(GLOB_RECURSE SANDBOX_SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")

add_executable(glsim_sandbox ${SANDBOX_SOURCES})

target_include_directories(glsim_sandbox PRIVATE
    ${glsim_INCLUDE_DIRECTORIES}
)

target_link_libraries(glsim_sandbox PRIVATE
    glsim
)
