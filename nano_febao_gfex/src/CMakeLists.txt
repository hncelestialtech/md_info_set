CMAKE_MINIMUM_REQUIRED(VERSION 3.17)

#上期所行情插件
PROJECT(gfex_nano_md)

set(CMAKE_C_COMPILER "/opt/rh/devtoolset-11/root/bin/gcc")
set(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-11/root/bin/g++")

add_compile_options(-std=c++20)
add_compile_options(-fPIC)
add_compile_options(-O3)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

IF(CMAKE_BUILD_TYPE STREQUAL "Release")
    ADD_COMPILE_OPTIONS(-O3 -march=native -flto -pthread)
ENDIF()

ADD_LIBRARY(gfex_nano_md SHARED gfex_nano.cpp)

# 链接nano
TARGET_LINK_DIRECTORIES(gfex_nano_md PRIVATE ../dependency)
TARGET_LINK_LIBRARIES(gfex_nano_md PRIVATE libgfex_nano_u50.so)


# 链接febao_trading_proxy_api
LINK_DIRECTORIES(../febao/febao_trading_proxy_api/lib)
LINK_LIBRARIES(cffex_fb_trading_proxy_api)

# 行情插件头文件
TARGET_INCLUDE_DIRECTORIES(gfex_nano_md PRIVATE 
    /home/shenzx/code/qx_commom/
    ../febao/febao_md_proxy_api/fb_md_plugin_api/include
    ../dependency
    ./
)

# 设置动态库版本信息
SET_TARGET_PROPERTIES(gfex_nano_md PROPERTIES
    VERSION 1.0.9
    SOVERSION 1
    OUTPUT_NAME "gfex_nano_md"
)

