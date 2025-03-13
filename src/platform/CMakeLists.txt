file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.c")
add_library(platform SHARED ${SOURCES})
target_link_libraries(platform PRIVATE core ${X11_LIBRARIES})
target_include_directories(platform PUBLIC public PRIVATE private public/platform)
