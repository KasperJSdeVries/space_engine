file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.c")
add_library(render_system SHARED ${SOURCES})
target_link_libraries(render_system PRIVATE core platform ${Vulkan_LIBRARIES})
target_include_directories(render_system PUBLIC public PRIVATE private public/render_system )
