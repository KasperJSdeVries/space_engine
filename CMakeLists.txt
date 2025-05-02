cmake_minimum_required(VERSION 3.19)
project(space_engine C)

set(CMAKE_C_STANDARD 11)
message(STATUS "Compiling for zen 5 architecture")
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Werror -ggdb -march=znver5")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_subdirectory(test)
add_subdirectory(dist)
add_subdirectory(src)

