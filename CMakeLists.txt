cmake_minimum_required(VERSION 3.20)

project(metris)

set(CMAKE_CXX_STANDARD 20)

set(CXX_COMPILER_FLAGS "-Wall -Wextra -Werror -Wswitch-enum -Wconversion")

find_package(SDL2 REQUIRED)
find_package(fmt REQUIRED)

add_executable(metris src/main.cc)

target_include_directories(metris PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(metris ${SDL2_LIBRARIES})
target_link_libraries(metris ${SDL2_LIBRARIES} fmt::fmt-header-only)
