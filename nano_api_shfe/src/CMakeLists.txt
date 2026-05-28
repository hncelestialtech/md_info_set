CMAKE_MINIMUM_REQUIRED(VERSION 3.17)

#上期所行情插件
PROJECT(nano_shfe_md)

set(CMAKE_C_COMPILER "/opt/rh/devtoolset-11/root/bin/gcc")
set(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-11/root/bin/g++")

add_compile_options(-std=c++17)
add_compile_options(-fPIC)
add_compile_options(-O3)
# add_compile_options(-lrt)


SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

ADD_LIBRARY(nano_shfe_md SHARED nano_shfe_md.cpp nano_shfe_impl.cpp nano_shfe_recevier.cpp)

# 链接nano
LINK_DIRECTORIES(../dependency)
LINK_LIBRARIES(libshfe_u50_threads.so)

# 行情插件头文件
TARGET_INCLUDE_DIRECTORIES(nano_shfe_md PRIVATE 
    /home/shenzx/code/qx_commom/
    ../dependency/
    ../include
    ./
)



# 设置动态库版本信息
SET_TARGET_PROPERTIES(nano_shfe_md PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    OUTPUT_NAME "nano_shfe_md"
)

