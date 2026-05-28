CMAKE_MINIMUM_REQUIRED(VERSION 3.17)

#创业板ETF期权和科创板ETF期权行情插件, 159915.SZ  588080.SH  588000.SH
PROJECT(qx_sz_md_etf_option_exanic)

set(CMAKE_C_COMPILER "/opt/rh/devtoolset-11/root/bin/gcc")
set(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-11/root/bin/g++")

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-std=c++17)
add_compile_options(-fPIC)
add_compile_options(-g)
add_compile_options(-O3)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

ADD_COMPILE_OPTIONS(-O3 -march=native -flto -lrt -lciul1 -lpthread -lexanic -fPIC)

ADD_LIBRARY(qx_sz_md_etf_option_exanic SHARED qx_sz_md_etf_option_exanic.cpp)


# 链接exanic
TARGET_LINK_DIRECTORIES(qx_sz_md_etf_option_exanic PRIVATE ../zqfpga_v2_exanic/)
TARGET_LINK_DIRECTORIES(qx_sz_md_etf_option_exanic PRIVATE /home/shenzx/code/dependency/nProV3/driver/libs/exanic)
TARGET_LINK_LIBRARIES(qx_sz_md_etf_option_exanic PRIVATE libexanic.a)

# 链接febao_trading_proxy_api
TARGET_LINK_DIRECTORIES(qx_sz_md_etf_option_exanic PRIVATE ../febao/febao_trading_proxy_api/lib)
TARGET_LINK_LIBRARIES(qx_sz_md_etf_option_exanic PRIVATE cffex_fb_trading_proxy_api)

# 行情插件头文件
TARGET_INCLUDE_DIRECTORIES(qx_sz_md_etf_option_exanic PRIVATE 
    /home/shenzx/code/qx_commom/
    ../febao/febao_md_proxy_api/fb_md_plugin_api/include
    ../zqfpga_v2_exanic/include/
    ./
)

# 设置动态库版本信息
SET_TARGET_PROPERTIES(qx_sz_md_etf_option_exanic PROPERTIES
    VERSION 1.1.1
    SOVERSION 1
    OUTPUT_NAME "qx_sz_md_etf_option_exanic"
)

