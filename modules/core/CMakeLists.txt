file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.c")
add_library(core SHARED ${SOURCES})
target_include_directories(core PUBLIC "include" PRIVATE "src" "include/core")

