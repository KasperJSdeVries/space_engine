cmake_minimum_required(VERSION 3.19)
project(space_engine C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(shaders)

find_package(X11 REQUIRED)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "src/*.c")
add_executable(game ${GAME_SOURCES})
target_link_libraries(game ${X11_LIBRARIES} ${Vulkan_LIBRARIES})
target_include_directories(game PUBLIC ${X11_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})
add_dependencies(game Shaders)

