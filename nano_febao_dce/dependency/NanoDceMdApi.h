#pragma once

#include "NanoDceMdStruct.h"

#ifdef LIB_NANO_API_EXPORT
#define LIB_API_EXPORT __attribute__ ((visibility("default")))
#else
#define LIB_API_EXPORT 
#endif

struct CNanoDceMdSpi
{
    //一档行情回调接口
    virtual void OnNanoDceL1Md(const NanoDceL1MdType& refNanoDceL1Md) = 0;


    //五档行情回调接口
    virtual void OnNanoDceL2ContractBestPriceMd(const NanoDceL2ContractBestPriceMdType& refNanoDceL2ContractBestPriceMd) = 0;
    virtual void OnNanoDceL2ArbBestPriceMd(const NanoDceL2ArbBestPriceMdType& refNanoDceL2ArbBestPriceMd) = 0;
    virtual void OnNanoDceL2SegQuotaMd(const NanoDceL2SegQuotaMdType& refNanoDceL2SegQuotaMd) = 0;
    virtual void OnNanoDceL2OrderStatisticsMd(const NanoDceL2OrderStatisticsMdType& refNanoDceL2OrderStatisticsMd) = 0;
    virtual void OnNanoDceL2DeepOrderVolumeMd(const NanoDceL2DeepOrderVolumeMdType& refDeepOrderVolumeMd) = 0;
    virtual void OnNanoDceL2DeepQuoteMd(const NanoDceL2DeepQuoteMdType& refNanoDceL2DeepMd) = 0;

    //事件通知
    virtual void OnEvent(const NanoEventType& refEventType) = 0;
};

struct LIB_API_EXPORT CNanoDceMdApi
{
    static CNanoDceMdApi& CreateNanoDceMdApi();
    static void DestroyNanoDceMdApi(CNanoDceMdApi& refNanoDceMdApi);

    /*********************************************阻塞接口*********************************************/
    /*
    @description 阻塞启动(复用调用线程，不额外启动线程。使用仅需调用NanoRecvStart即可，内部会循环获取DMA数据并回调给用户层)
    @param      refNanoDceMdSpi    回调实例对象引用
    @param      lpConfig            配置文件(默认当前路径，非当前路口请指定路径)
    @return     0:成功  其他:失败
    */
    virtual int32_t NanoRecvStart(CNanoDceMdSpi& refNanoDceMdSpi, const char* lpConfig) = 0;

    /*********************************************非阻塞接口*********************************************/
    //非阻塞启动(复用调用线程，不额外启动线程。使用先调用NanoStart，调用NanoStart成功后需循环调用NanoRecv从DMA获取数据，建议循环中不要做太多事情，否则DMA数据获取不及时，DMA缓存慢数据会丢失)
    //启动后NanoStart会阻塞等待快照数据完成初始化和订阅，盘中出现丢包NanoRecv会阻塞重新等待快照初始化和订阅，如果长时间收不到有效快照数据，会超时报错返回。超时时间300秒。
    /*
    @description 启动Nano服务，完成初始化
    @param      refNanoDceMdSpi    回调实例对象引用
    @param      lpConfig            配置文件(默认当前路径，非当前路口请指定路径)
    @return     0:成功  其他:失败
    */
    virtual int32_t NanoStart(CNanoDceMdSpi& refNanoDceMdSpi, const char* lpConfig) = 0;

    /*
    @description 非阻塞，NanoRecv从DMA获取数据并OnNanoDceMd回调给应用层，建议循环中不要做太多事情，否则DMA数据获取不及时，DMA缓存慢数据会丢失
    @return     -1:程序退出，外层可以把-1作为SIGINT信号使用
                 0:没有数据，不触发回调
                 1:有数据，触发回调
    */
    virtual int32_t NanoRecv() = 0;

    /*
    @description 获取合约静态信息（合约静态信息来源于一档快照信息，请在一档初始化订阅完成之后再进行获取）
    @param      lpContractName          合约名称
    @param      refInstStaticInfo       合约静态信息
    @return     
            0:成功
            -1:入参非法
            -2:合约不存在
    */
    virtual int32_t GetInstStaticInfo(const char* lpContractName, NanoDceInstStaticInfo& refInstStaticInfo) = 0;
};