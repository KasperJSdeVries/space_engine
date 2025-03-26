file(GLOB SOURCES CONFIGURE_DEPENDS "*.c")
add_executable(game ${SOURCES})
target_link_libraries(game PRIVATE core platform render_system math ${Vulkan_LIBRARIES})
add_dependencies(game Shaders)
