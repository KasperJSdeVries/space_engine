find_package(glfw3 3.4 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(cglm REQUIRED)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "*.c")
add_library(engine STATIC ${ENGINE_SOURCES})
target_link_libraries(engine PRIVATE ${Vulkan_LIBRARIES} glfw cglm m)
target_include_directories(engine PUBLIC .)
target_include_directories(engine PRIVATE ../extern/)
add_dependencies(engine Assets)

install(TARGETS engine)
