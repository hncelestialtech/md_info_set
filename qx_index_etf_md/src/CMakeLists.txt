CMAKE_MINIMUM_REQUIRED(VERSION 3.17)

#创业板ETF和科创板ETF行情插件, 159915.SZ  588080.SH  588000.SH
PROJECT(qx_index_etf_md)

set(CMAKE_C_COMPILER "/opt/rh/devtoolset-11/root/bin/gcc")
set(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-11/root/bin/g++")

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-std=c++17)
add_compile_options(-fPIC)
#add_compile_options(-g)
add_compile_options(-O3)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

IF(CMAKE_BUILD_TYPE STREQUAL "Release")
    ADD_COMPILE_OPTIONS(-O3 -march=native -flto -pthread -lrt -lciul1)
ENDIF()

ADD_LIBRARY(qx_index_etf_md SHARED qx_index_etf_md.cpp)

# 链接exanic
TARGET_LINK_DIRECTORIES(qx_index_etf_md PRIVATE ..//zqfpga_v2_exanic/)
TARGET_LINK_DIRECTORIES(qx_index_etf_md PRIVATE /home/shenzx/code/pro_optimize/nProV3/driver/libs/exanic)
TARGET_LINK_LIBRARIES(qx_index_etf_md PRIVATE libexanic.a)

# 链接febao_trading_proxy_api
TARGET_LINK_DIRECTORIES(qx_index_etf_md PRIVATE ../febao/febao_trading_proxy_api/lib)
TARGET_LINK_LIBRARIES(qx_index_etf_md PRIVATE cffex_fb_trading_proxy_api)

# 行情插件头文件
TARGET_INCLUDE_DIRECTORIES(qx_index_etf_md PRIVATE 
    /home/shenzx/code/qx_commom/
    ../febao/febao_md_proxy_api/fb_md_plugin_api/include
    ../zqfpga_v2_exanic/include/
    ../febao/febao_trading_proxy_api/include
    ./
)

# 设置动态库版本信息
SET_TARGET_PROPERTIES(qx_index_etf_md PROPERTIES
    VERSION 1.0.3
    SOVERSION 1
    OUTPUT_NAME "qx_index_etf_md"
)

