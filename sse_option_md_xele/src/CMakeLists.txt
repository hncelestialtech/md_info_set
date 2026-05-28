CMAKE_MINIMUM_REQUIRED(VERSION 3.17)

# 上期所 LV1 期权
PROJECT(sse_md_option_xele)

set(CMAKE_C_COMPILER "/opt/rh/devtoolset-11/root/bin/gcc")
set(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-11/root/bin/g++")

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-std=c++17)
add_compile_options(-fPIC)
# add_compile_options(-g)
add_compile_options(-O3)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

ADD_COMPILE_OPTIONS(-march=native -funroll-loops -ffast-math -flto -lrt -lciul1 -lpthread -lexanic -fPIC)
ADD_LIBRARY(sse_md_option_xele SHARED sse_md_option.cpp)

# 链接febao_trading_proxy_api
TARGET_LINK_DIRECTORIES(sse_md_option_xele PRIVATE ../febao/febao_trading_proxy_api/lib)
TARGET_LINK_LIBRARIES(sse_md_option_xele PRIVATE cffex_fb_trading_proxy_api)

TARGET_LINK_DIRECTORIES(sse_md_option_xele PRIVATE ../dependency/xele_md_api/)
TARGET_LINK_LIBRARIES(sse_md_option_xele PRIVATE libxelemd.a)


# 行情插件头文件
TARGET_INCLUDE_DIRECTORIES(sse_md_option_xele PRIVATE
    ../../qx_commom/
    ../febao/febao_md_proxy_api/fb_md_plugin_api/include
    ../dependency/xele_md_api/
    ./
)

# 设置动态库版本信息
SET_TARGET_PROPERTIES(sse_md_option_xele PROPERTIES
    VERSION 1.0.5
    SOVERSION 1
    OUTPUT_NAME "sse_md_option_xele"
)

