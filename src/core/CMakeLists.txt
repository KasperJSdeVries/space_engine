file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.c")
add_library(core SHARED ${SOURCES})
target_include_directories(core PUBLIC public PRIVATE private public/core)

