CMAKE_MINIMUM_REQUIRED(VERSION 3.17)


PROJECT(gfex_md_exanic)

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
ADD_LIBRARY(gfex_md_exanic SHARED exanic_md.cpp)

# 链接exanic
TARGET_LINK_DIRECTORIES(gfex_md_exanic PRIVATE ../zqfpga_v2_exanic/)
TARGET_LINK_DIRECTORIES(gfex_md_exanic PRIVATE /home/shenzx/code/dependency/nProV3/driver/libs/exanic)
TARGET_LINK_LIBRARIES(gfex_md_exanic PRIVATE libexanic.a )

# 链接febao_trading_proxy_api
TARGET_LINK_DIRECTORIES(gfex_md_exanic PRIVATE ../febao/febao_trading_proxy_api/lib)
TARGET_LINK_LIBRARIES(gfex_md_exanic PRIVATE cffex_fb_trading_proxy_api)

# TARGET_LINK_DIRECTORIES(gfex_md_exanic PRIVATE ../dependency)
# TARGET_LINK_LIBRARIES(gfex_md_exanic PRIVATE libcffex_lvl2_lvl2.so)


# 行情插件头文件
TARGET_INCLUDE_DIRECTORIES(gfex_md_exanic PRIVATE
    /home/shenzx/code/qx_commom/
    ../febao/febao_md_proxy_api/fb_md_plugin_api/include
    ../dependency/
    ./
)

# 设置动态库版本信息
SET_TARGET_PROPERTIES(gfex_md_exanic PROPERTIES
    VERSION 1.0.g
    SOVERSION 1
    OUTPUT_NAME "gfex_md_exanic"
)

