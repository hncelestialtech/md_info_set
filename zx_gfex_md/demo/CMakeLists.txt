# CMakeList.txt: LinuxTest 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.16)
project ("gfexRcv")

if(CMAKE_BUILD_TYPE STREQUAL Debug) 
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/)
else()
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/)
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug) 
  add_definitions(-D_DEBUG)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_VERBOSE_MAKEFILE ON)

# 将源代码添加到此项目的可执行文件。
FILE(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}}/src/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc")
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# TODO: 如有需要，请添加测试并安装目标。

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if(CMAKE_BUILD_TYPE STREQUAL Debug) 
else()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE dl)
target_link_libraries(${PROJECT_NAME} PRIVATE pthread)


