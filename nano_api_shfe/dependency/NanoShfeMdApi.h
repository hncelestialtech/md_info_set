#pragma once

#include "NanoShfeMdStruct.h"

#ifdef LIB_NANO_API_EXPORT
#define LIB_API_EXPORT __attribute__ ((visibility("default")))
#else
#define LIB_API_EXPORT 
#endif

struct CNanoShfeMdSpi
{
    virtual void OnNanoShfeMd(const NanoShfeMdType& refSNanoShfeMd) = 0;
};

struct LIB_API_EXPORT CNanoShfeMdApi
{
    static CNanoShfeMdApi& CreateNanoShfeMdApi();
    static void DestroyNanoShfeMdApi(CNanoShfeMdApi& refNanoShfeMdApi);

    /*********************************************阻塞接口*********************************************/
    /*
    @description 阻塞启动(复用调用线程，不额外启动线程。使用仅需调用NanoRecvStart即可，内部会循环获取DMA数据并回调给用户层)
    @param      refNanoShfeMdSpi    回调实例对象引用
    @param      lpConfig            配置文件(默认当前路径，非当前路口请指定路径)
    @return     0:成功  其他:失败
    */
    virtual int32_t NanoRecvStart(CNanoShfeMdSpi& refNanoShfeMdSpi, const char* lpConfig) = 0;


    /*********************************************非阻塞接口*********************************************/
    //非阻塞启动(复用调用线程，不额外启动线程。使用先调用NanoStart，调用NanoStart成功后需循环调用NanoRecv从DMA获取数据，建议循环中不要做太多事情，否则DMA数据获取不及时，DMA缓存慢数据会丢失)
    /*
    @description 启动Nano服务，完成初始化
    @param      refNanoShfeMdSpi    回调实例对象引用
    @param      lpConfig            配置文件(默认当前路径，非当前路口请指定路径)
    @return     0:成功  其他:失败
    */
    virtual int32_t NanoStart(CNanoShfeMdSpi& refNanoShfeMdSpi, const char* lpConfig) = 0;

    /*
    @description 非阻塞，NanoRecv从DMA获取数据并OnNanoShfeMd回调给应用层，建议循环中不要做太多事情，否则DMA数据获取不及时，DMA缓存慢数据会丢失
    @return     -1:程序退出，外层可以把-1作为SIGINT信号使用
                 0:没有数据，不触发回调
                 1:有数据，触发回调
    */
    virtual int32_t NanoRecv() = 0;


    /*********************************************非回调接口*********************************************/
    /*
    @description 启动Nano服务，完成初始化
    @param      lpConfig            配置文件(默认当前路径，非当前路口请指定路径)
    @return     0:成功  其他:失败
    */
    virtual int32_t NanoStart(const char* lpConfig) = 0;

    /*
    @description 非阻塞，NanoRecv从DMA获取数据并将数据存入输入输出参数refSNanoShfeMd，建议循环中不要做太多事情，否则DMA数据获取不及时，DMA缓存慢数据会丢失
    @param      refSNanoShfeMd    行情数据
    @return     -1:程序退出，外层可以把-1作为SIGINT信号使用
                 0:没有数据
                 1:有数据，行情数据存入输入输出参数refSNanoShfeMd
    */
    virtual int32_t NanoRecv(NanoShfeMdType& refSNanoShfeMd) = 0;

    /*********************************************非阻塞接口*********************************************/
    /*
    @description 停止Nano服务，主动停止接收行情
    @param      lpConfig            配置文件(默认当前路径，非当前路口请指定路径)
    @return     0:成功  其他:失败
    */
    virtual int32_t NanoStop() = 0;

};
