find_package(X11 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(cglm REQUIRED)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "*.c")
add_library(engine STATIC ${ENGINE_SOURCES})
target_link_libraries(engine PRIVATE ${Vulkan_LIBRARIES} ${X11_LIBRARIES} cglm
                                     m)
target_include_directories(engine PUBLIC .)
add_dependencies(engine Shaders)

install(TARGETS engine)
