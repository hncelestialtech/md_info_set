
cmake_minimum_required(VERSION 3.17)
project(nano_gfex_dump)

set(CMAKE_C_COMPILER "/opt/rh/devtoolset-11/root/bin/gcc")
set(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-11/root/bin/g++")

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 设置C++标准
ADD_COMPILE_OPTIONS(-std=c++17)
ADD_COMPILE_OPTIONS(-fPIC)
ADD_COMPILE_OPTIONS(-O3)




# 获取父目录路径
GET_FILENAME_COMPONENT(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

# 包含头文件目录
INCLUDE_DIRECTORIES(
    ${PARENT_DIR}/include
    ${PARENT_DIR}/dependency
    /home/shenzx/code/qx_commom/
)

# 链接库文件目录
LINK_DIRECTORIES(
    ${PARENT_DIR}/lib
    ${PARENT_DIR}/dependency
)

# 添加可执行目标
ADD_EXECUTABLE(nano_gfex_dump nano_gfex_dump.cpp)

# 链接动态库
TARGET_LINK_LIBRARIES(nano_gfex_dump
    PRIVATE
    libnano_gfex_md.so    
    libgfex_nano_u50.so 
)

# 设置运行时库搜索路径
set(CMAKE_BUILD_RPATH ${PARENT_DIR}/lib;${PARENT_DIR}/dependency)
set(CMAKE_INSTALL_RPATH ${CMAKE_BUILD_RPATH})
