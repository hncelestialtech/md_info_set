cmake_minimum_required (VERSION 2.6)

project(cubic_spline_mixed)

if (WINDOWS)
	add_definitions(-DWINDOWS)
else()
	add_definitions(-DLINUX)
endif()

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")


include_directories(../include ./src ../../../fb_i_algorithm/include)

aux_source_directory(./src DIR_LIB_SRCS)

add_library(cubic_spline_mixed SHARED ${DIR_LIB_SRCS} )

set(LIBRARY_OUTPUT_PATH ../lib)

#cmake -G "Visual Studio 16 2019" ../
