CMAKE_MINIMUM_REQUIRED(VERSION 3.17)

#上期所行情插件
PROJECT(shfe_npro_md)

set(CMAKE_C_COMPILER "/opt/rh/devtoolset-11/root/bin/gcc")
set(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-11/root/bin/g++")

add_compile_options(-std=c++17)
add_compile_options(-fPIC)
add_compile_options(-O3)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

IF(CMAKE_BUILD_TYPE STREQUAL "Release")
    ADD_COMPILE_OPTIONS(-O3 -march=native -flto -pthread)
ENDIF()

ADD_LIBRARY(shfe_npro_md SHARED shfe_md_quota.cpp)

# 链接mProV3
TARGET_LINK_DIRECTORIES(shfe_npro_md PRIVATE ../nProV3/api)
TARGET_LINK_LIBRARIES(shfe_npro_md PRIVATE libnpro.so)

# 链接febao_trading_proxy_api
TARGET_LINK_DIRECTORIES(shfe_npro_md PRIVATE ../febao/febao_trading_proxy_api/lib)
TARGET_LINK_LIBRARIES(shfe_npro_md PRIVATE cffex_fb_trading_proxy_api)

# 行情插件头文件
TARGET_INCLUDE_DIRECTORIES(shfe_npro_md PRIVATE 
    /home/shenzx/code/qx_commom/
    ../febao/febao_md_proxy_api/fb_md_plugin_api/include
    ../nProV3/api
    ../febao/febao_trading_proxy_api/include
    ./
)

# 设置动态库版本信息
SET_TARGET_PROPERTIES(shfe_npro_md PROPERTIES
    VERSION 1.0.6
    SOVERSION 1
    OUTPUT_NAME "shfe_npro_md"
)

