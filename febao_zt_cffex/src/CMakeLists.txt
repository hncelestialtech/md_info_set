CMAKE_MINIMUM_REQUIRED(VERSION 3.17)

#创业板ETF期权和科创板ETF期权行情插件, 159915.SZ  588080.SH  588000.SH
PROJECT(febao_zt_cffex)

set(CMAKE_C_COMPILER "/opt/rh/devtoolset-11/root/bin/gcc")
set(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-11/root/bin/g++")

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-std=c++17)
add_compile_options(-fPIC)
#add_compile_options(-g)
add_compile_options(-O3)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

ADD_COMPILE_OPTIONS(-O3 -march=native -flto -lrt -lciul1 -lpthread -lexanic -fPIC)
ADD_LIBRARY(febao_zt_cffex SHARED zt_cffex_md.cpp HSNsqSpiImpl.cpp)

# 链接exanic
# TARGET_LINK_DIRECTORIES(febao_zt_cffex PRIVATE ../zqfpga_v2_exanic/)
# TARGET_LINK_DIRECTORIES(febao_zt_cffex PRIVATE ../dependency/libs/exanic)
# TARGET_LINK_LIBRARIES(febao_zt_cffex PRIVATE libexanic.a )

# 链接febao_trading_proxy_api
TARGET_LINK_DIRECTORIES(febao_zt_cffex PRIVATE ../febao/febao_trading_proxy_api/lib)
TARGET_LINK_LIBRARIES(febao_zt_cffex PRIVATE cffex_fb_trading_proxy_api)

TARGET_LINK_DIRECTORIES(febao_zt_cffex PRIVATE ../dependency/lib)
TARGET_LINK_LIBRARIES(febao_zt_cffex PRIVATE libHSNsqApi.so)


# 行情插件头文件
TARGET_INCLUDE_DIRECTORIES(febao_zt_cffex PRIVATE
    /home/shenzx/code/qx_commom/
    ../febao/febao_md_proxy_api/fb_md_plugin_api/include
    ../dependency/include/
    ./
)

# 设置动态库版本信息
SET_TARGET_PROPERTIES(febao_zt_cffex PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    OUTPUT_NAME "febao_zt_cffex"
)

