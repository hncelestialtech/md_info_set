#pragma once




#include <typeinfo>
#include <cxxabi.h>





#ifndef __unused
#     define USED_API    __attribute__ ((unused))
#endif



#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()
#define FLOAT32_NAN std::numeric_limits<float>::quiet_NaN()

#define FLOAT64_INF std::numeric_limits<float64_t>::infinity()
#define FLOAT32_INF std::numeric_limits<float32_t>::infinity()

#define __CLASS__ abi::__cxa_demangle(typeid(*this).name(), 0, 0, nullptr)

#define TRAFFIC_FLOW_MODE_DISABLE     0
#define TRAFFIC_FLOW_MODE_ONLY_SPAN   1
#define TRAFFIC_FLOW_MODE_LV1_SPAN    2


using uint128_t = __uint128_t; //只有编译器支持

#define float64_t   double
#define float32_t   float

#define NAME2STR(name)   (#name)


#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)



//错误码定义
#define RET_UNKOWN_ERROR         -1
#define RET_SUCCESS              0
#define RET_DIR_ALREADY_EXIST    1001
#define RET_DIR_NOT_EXIST        1002
#define RET_DIR_EMPTY            1003
#define RET_FILE_ALREADY_EXIST   2001
#define RET_FILE NOT_EXIST       2002   // 应该存在的文件没有找到
#define RET_FILE_EMPTY           2003   //文件内容为空，大小为0
#define RET_FILE_LENGH_WRONG     2004  //文件大小不符合预期
#define RET_FILE_ERROR           2005
#define RET_NO_FILE_INCLUDE      2006  //例如 20230213更新时，没有20230213.dat
#define RET_FILE_FORMAT_ERROR    2007  //文件格式错误
#define RET_MMAP_ERROR           3001
#define RET_CACHE_EMPTY          3002

#define RET_CONFIG_NOT_FOUND       4001
#define RET_CYCLE_ERROR            4002
#define RET_DATE_ALREADY_INCLUDE   4003
#define RET_NOT_TRADINGDAY         4004
