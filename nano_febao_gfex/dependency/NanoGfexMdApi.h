#pragma once

#include "NanoGfexMdStruct.h"

#ifdef LIB_NANO_API_EXPORT
#define LIB_API_EXPORT __attribute__ ((visibility("default")))
#else
#define LIB_API_EXPORT 
#endif

struct CNanoGfexMdSpi
{
    //一档行情回调接口
    virtual void OnNanoGfexL1Md(const NanoGfexL1MdType& refNanoGfexL1Md) = 0;


    //五档行情回调接口
    virtual void OnNanoGfexL2Md(const NanoGfexL2MdType& refNanoGfexL2Md) = 0;
};

struct LIB_API_EXPORT CNanoGfexMdApi
{
    static CNanoGfexMdApi& CreateNanoGfexMdApi();
    static void DestroyNanoGfexMdApi(CNanoGfexMdApi& refNanoGfexMdApi);

    /*********************************************阻塞接口*********************************************/
    /*
    @description 阻塞启动(复用调用线程，不额外启动线程。使用仅需调用NanoRecvStart即可，内部会循环获取DMA数据并回调给用户层)
    @param      refNanoGfexMdSpi    回调实例对象引用
    @param      lpConfig            配置文件(默认当前路径，非当前路口请指定路径)
    @return     0:成功  其他:失败
    */
    virtual int32_t NanoRecvStart(CNanoGfexMdSpi& refNanoGfexMdSpi, const char* lpConfig) = 0;


    /*********************************************非阻塞接口*********************************************/
    //非阻塞启动(复用调用线程，不额外启动线程。使用先调用NanoStart，调用NanoStart成功后需循环调用NanoRecv从DMA获取数据，建议循环中不要做太多事情，否则DMA数据获取不及时，DMA缓存慢数据会丢失)
    /*
    @description 启动Nano服务，完成初始化
    @param      refNanoGfexMdSpi    回调实例对象引用
    @param      lpConfig            配置文件(默认当前路径，非当前路口请指定路径)
    @return     0:成功  其他:失败
    */
    virtual int32_t NanoStart(CNanoGfexMdSpi& refNanoGfexMdSpi, const char* lpConfig) = 0;

    /*
    @description 非阻塞，NanoRecv从DMA获取数据并OnNanoGfexMd回调给应用层，建议循环中不要做太多事情，否则DMA数据获取不及时，DMA缓存慢数据会丢失
    @return     -1:程序退出，外层可以把-1作为SIGINT信号使用
                 0:没有数据，不触发回调
                 1:有数据，触发回调
    */
    virtual int32_t NanoRecv() = 0;

    /*
    @description 获取合约静态信息（合约静态信息来源于一档快照信息，请在一档初始化订阅完成之后再进行获取）
    @param      lpContractName          合约名称
    @param      refInstStaticInfo       合约静态信息
    @return     0:成功  其他:失败
    */
    virtual int32_t GetInstStaticInfo(const char* lpContractName, NanoGfexInstStaticInfo& refInstStaticInfo) = 0;
};