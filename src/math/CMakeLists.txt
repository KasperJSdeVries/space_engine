file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.h")
add_library(math INTERFACE)
target_include_directories(math INTERFACE .)
