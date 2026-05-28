cmake_minimum_required(VERSION 3.17)
project(gfex_nano_md)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/lib)

add_subdirectory(src)

add_subdirectory(nano_gfex_dump)

add_subdirectory(demo)