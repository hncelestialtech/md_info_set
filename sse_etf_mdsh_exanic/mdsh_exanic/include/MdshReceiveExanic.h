#ifndef H_MDSH_RECEIVER_EXANIC_H
#define H_MDSH_RECEIVER_EXANIC_H

#include "exanic/exanic.h"
#include "exanic/time.h"
#include "exanic/config.h"
#include "exanic/fifo_rx.h"
#include "exanic/register.h"
#include "exanic/util.h"

#pragma pack(push, 1)
struct CMdshMd
{
    uint8_t verAndExchange;                 //高四位为版本号，低四位为交易所 上交所L1：1 上交所L2：2 深交所：3
    uint8_t length;                         //总体长度
    uint8_t tradeModeAndSecurityType;       //高四位为交易模式 Reserved:0  系统测试:1   模拟交易:2   正常交易:3
                                            //低四位为证券类型 1: 股票 2：衍生品 3：综合业务 c：债券
    uint8_t mdStreamId[5];                  //行情类别
    uint8_t tradingPhaseCode[8];            //实时阶段及标志
    uint32_t timestamp;                     //时间HHMMSSss 精度为10ms
    uint8_t securityId[8];                  //产品代码
    uint64_t totalValue;                    //成交金额
    uint64_t totalVolume;                   //成交数量
    uint64_t lastPrice;                     //最新价
    uint64_t openInterest;                  //持仓量
    uint64_t bidPrice[5];                   //五档买价
    uint32_t bidVolume[5];                  //五档买量
    uint64_t askPrice[5];                   //五档卖价
    uint32_t askVolume[5];                  //五档卖量
};
#pragma pack(pop)

typedef void (*MdshCallBack)(uint8_t *data, int len);

struct CMdshReceiver;

/**
 * @brief 初始化数据接收程序
 * @param [rev] 用户创建的CMdshReceiver指针地址，内存分配无需用户分配，函数内部分配内存地址
 * @param [configFile] 配置文件路径
 * @return 返回0代表初始化成功，负数代表失败
 */
auto MdshReceiverCreate(CMdshReceiver **rev, const char *configFile) -> int;

/**
 * @brief 接收单条数据，存放至buf
 * @param [rev] 已经初始化过的CMdshReceiver指针
 * @param [actionFunc] 用户自定义行情处理函数
 */
auto MdshReceiverRun(CMdshReceiver *rev, MdshCallBack actionFunc) -> void;

/**
 * @brief 关闭数据接收程序，释放资源
 * @param [rev] 已经初始化过的CMdshReceiver指针
 */
auto MdshReceiverClose(CMdshReceiver *rev) -> void;

/**
 * @brief 获取日志文件路径
 * @param [rev] 已经初始化过的CMdshReceiver指针
 * @return 日志文件路径
 */
auto MdshGetOutputPath(CMdshReceiver *rev) -> const char *;

#endif