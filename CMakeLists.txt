cmake_minimum_required(VERSION 3.19)
project(space_engine C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -ggdb")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_subdirectory(shaders)

find_package(X11 REQUIRED)
find_package(Vulkan REQUIRED)

file(GLOB MODULES CONFIGURE_DEPENDS "modules/*")
foreach(module ${MODULES})
	add_subdirectory(${module})
endforeach(module)

