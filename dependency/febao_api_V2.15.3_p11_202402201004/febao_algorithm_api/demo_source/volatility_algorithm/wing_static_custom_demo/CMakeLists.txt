cmake_minimum_required (VERSION 2.6)

project(wing_static)

if (WINDOWS)
	add_definitions(-DWINDOWS)
else()
	add_definitions(-DLINUX)
endif()

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")


include_directories(../include ./src ../../../fb_i_algorithm/include)

aux_source_directory(./src DIR_LIB_SRCS)

add_library(wing_static SHARED ${DIR_LIB_SRCS})

set(LIBRARY_OUTPUT_PATH ../lib)

#cmake -G "Visual Studio 11 2012 Win64" ../
