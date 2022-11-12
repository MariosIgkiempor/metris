cmake_minimum_required(VERSION 3.20)

include(FindPkgConfig)

project(metris)

set(CMAKE_CXX_STANDARD 20)

set(CXX_COMPILER_FLAGS "-Wall -Wextra -Werror -Wswitch-enum -Wconversion -Wunused")

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf)
find_package(fmt REQUIRED)

add_executable(metris src/main.cc)

target_include_directories(metris PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(metris ${SDL2_LIBRARIES})
target_link_libraries(metris ${SDL2TTF_LIBRARIES})
target_link_libraries(metris ${SDL2_LIBRARIES} fmt::fmt-header-only)
