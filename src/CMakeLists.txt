find_package(X11 REQUIRED)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.c")
add_executable(client ${SOURCES})
target_link_libraries(client PRIVATE ${Vulkan_LIBRARIES} ${X11_LIBRARIES})
target_include_directories(client PUBLIC .)
add_dependencies(client Shaders)

