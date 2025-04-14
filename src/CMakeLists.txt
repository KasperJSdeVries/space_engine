find_package(X11 REQUIRED)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.c")
add_executable(game ${SOURCES})
target_link_libraries(game PRIVATE ${Vulkan_LIBRARIES} ${X11_LIBRARIES})
target_include_directories(game PUBLIC .)
add_dependencies(game Shaders)

