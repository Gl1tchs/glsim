include(ShaderUtils)

# Build shader bundler target
add_executable(glsim_bundler bundler.cpp)

set(PIPELINES_DIR ${CMAKE_CURRENT_LIST_DIR}/pipelines)
set(COMP_DIR ${CMAKE_CURRENT_LIST_DIR}/compute)
set(BUILD_DIR ${CMAKE_BINARY_DIR}/shaders)

file(GLOB_RECURSE VS_FILES ${PIPELINES_DIR}/*.vert)
file(GLOB_RECURSE FS_FILES ${PIPELINES_DIR}/*.frag)
file(GLOB_RECURSE COMP_FILES ${COMP_DIR}/*.comp)

compile_shader_group(SPV_VS_LIST "${VS_FILES}" "${PIPELINES_DIR}")
compile_shader_group(SPV_FS_LIST "${FS_FILES}" "${PIPELINES_DIR}")
compile_shader_group(SPV_COMP_LIST "${COMP_FILES}" "${COMPUTE_DIR}")

add_custom_target(glsim_shaders ALL
    DEPENDS ${SPV_VS_LIST} ${SPV_FS_LIST} ${SPV_COMP_LIST}
)

# Bundle the shaders if any provided
if (SPV_VS_LIST OR SPV_FS_LIST OR SPV_COMP_LIST)
    set(BUNDLE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/include")

    add_custom_command(TARGET glsim_shaders PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BUNDLE_OUTPUT_DIR}
        COMMAND $<TARGET_FILE:glsim_bundler>
            ${BUNDLE_OUTPUT_DIR}/shader_bundle.gen.h
            ${BUILD_DIR}
            ${SPV_VS_LIST}
            ${SPV_FS_LIST}
            ${SPV_COMP_LIST}
        COMMENT "Bundling engine shader sources..."
    )
endif()

add_dependencies(glsim_shaders glsim_bundler)
