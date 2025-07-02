cmake_minimum_required(VERSION 3.19)
project(space_engine C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS
    "-Wall -Wextra -Wpedantic -Werror -Wno-language-extension-token -ggdb")

add_compile_definitions($<$<CONFIG:Debug>:_DEBUG=1>)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_subdirectory(assets)
add_subdirectory(src)
add_subdirectory(client)

enable_testing()
add_subdirectory(test)
