find_package(Vulkan REQUIRED COMPONENTS glslc)

file(GLOB_RECURSE GLSL_SOURCES CONFIGURE_DEPENDS "*.vert" "*.frag")

foreach(GLSL ${GLSL_SOURCES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(OUTPUT ${SPIRV}
      COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
      COMMAND Vulkan::glslc ${GLSL} -o ${SPIRV}
      DEPENDS ${GLSL}
 )
  list(APPEND SPIRV_BINARIES ${SPIRV})
endforeach(GLSL)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARIES})

install(DIRECTORY ${PROJECT_BINARY_DIR}/shaders DESTINATION shaders)
