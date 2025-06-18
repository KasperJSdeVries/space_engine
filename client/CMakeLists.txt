file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS "*.c")

add_executable(client_bin ${CLIENT_SOURCES})
target_link_libraries(client_bin PRIVATE engine)
