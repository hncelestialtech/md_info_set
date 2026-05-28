# mdSH行情网络版接收工具使用指南

## 当前版本 v1.2.1_20230111
---
## 功能概述
---
本工具用于接收上交所mdsh行情，适配exanic网卡(如需其他网卡类型，请联系对接人员)，并支持用户自定义函数回调对数据进行处理。

## 接口说明
---
auto MdshReceiverCreate(CMdshReceiver **rev, const char *configFile) -> int;

* @brief 初始化数据接收程序
* @param [rev] 用户创建的CMdshReceiver指针地址，内存分配无需用户分配，函数内部分配内存地址
* @param [configFile] 配置文件路径
* @return 返回0代表初始化成功，负数代表失败

auto MdshReceiverRun(CMdshReceiver *rev, MdshCallBack actionFunc) -> void;

* @brief 接收数据并进行自定义处理
* @param [rev] 已经初始化过的CMdshReceiver指针
* @param [actionFunc] 用户自定义数据处理函数，成员参数参考头文件定义及demo

auto MdshReceiverClose(CMdshReceiver *rev) -> void;

* @brief 关闭数据接收程序，释放资源
* @param [rev] 已经初始化过的CMdshReceiver指针

auto MdshGetOutputPath(CMdshReceiver *rev) -> const char *;

* @brief 获取日志文件路径
* @param [rev] 已经初始化过的CMdshReceiver指针
* @return 日志文件路径

## 行情数据结构
---

### 非指数类行情（180字节）

| 结构字段 | 数据类型 | 域名 | 说明 |
|:---:|:---:|:---:|:---:|
| verAndExchange | uint8_t | Version & Exchange | 高四位为版本号；低四位为交易所，上交所L1：1；上交所L2：2；深交所：3 |
| length | uint8_t | Length | 结构体总体长度，指数行情固定为60，其他行情固定为180 |
| tradeModeAndSecurityType | uint8_t | TradeMode & SecurityType | 高四位为交易模式，Reserved：0；系统测试：1；模拟交易：2；正常交易：3； <br> 低四位为证券类型，股票：1；衍生品：2；综合业务：3；债券：c |
| mdStreamId | uint8_t[5] | MDStreamID | 行情类别 |
| tradingPhaseCode | uint8_t[8] | TradingPhaseCode | 实时阶段及标志(字符串形式输出) |
| timestamp | uint32_t | LastUpdateTime | 行情时间戳 |
| securityId | uint8_t[8] | SecurityID | 产品代码 |
| totalValue | uint64_t | TtotalValueTraded | 成交金额；需除以100得到实际值 |
| totalVolume | uint64_t | TotalVolumeTraded | 成交数量 |
| lastPrice | uint64_t | LastPrice | 最新价；需除以10^5得到实际值 |
| openInterest | uint64_t | OpenInterest | 持仓量 |
| bidPrice | uint64_t[5] | BidPrice | 五档买价；需除以10^5得到实际值 |
| bidVolume | uint32_t[5] | BidVolume | 五档买量 |
| askPrice | uint64_t[5] | AskPrice | 五档卖价；需除以10^5得到实际值 |
| askVolume | uint32_t[5] | AskVolume | 五档卖量 |

### 指数类行情（60字节）

| 结构字段 | 数据类型 | 域名 | 说明 |
|:---:|:---:|:---:|:---:|
| verAndExchange | uint8_t | Version & Exchange | 高四位为版本号；低四位为交易所，上交所L1：1；上交所L2：2；深交所：3 |
| length | uint8_t | Length | 结构体总体长度，指数行情固定为60，其他行情固定为180 |
| tradeModeAndSecurityType | uint8_t | TradeMode & SecurityType | 高四位为交易模式，Reserved：0；系统测试：1；模拟交易：2；正常交易：3； <br> 低四位为证券类型，股票：1；衍生品：2；综合业务：3；债券：c |
| mdStreamId | uint8_t[5] | MDStreamID | 行情类别 |
| tradingPhaseCode | uint8_t[8] | TradingPhaseCode | 实时阶段及标志(字符串形式输出) |
| timestamp | uint32_t | LastUpdateTime | 行情时间戳 |
| securityId | uint8_t[8] | SecurityID | 产品代码 |
| totalValue | uint64_t | TtotalValueTraded | 成交金额；需除以100得到实际值 |
| totalVolume | uint64_t | TotalVolumeTraded | 成交数量 |
| lastPrice | uint64_t | LastPrice | 最新价；需除以10^5得到实际值 |
| openInterest | uint64_t | OpenInterest | 持仓量 |

## 使用指引
---
### 软件部署
1、在预设目录下解压代码包,以本地版为例，压缩包文件名：mdsh_exanic.tar
```
tar -xvf mdsh_exanic.tar
```
内含文件：
> include  
>> MdshReceiveExanic.h  
>
> lib  
>> libmdshreceive.so  
> 
> Readme.md  
> makefile  
> mdsh.ini  
> demo.cpp


2、联系国泰君安量化交易部，申请license

3、配置mdsh.ini，字段含义如下：

```
[nic]
nic_name=enp23s0                                                //接收网口名
nic_type=exanic                                                 //网卡类型 此版本固定为exanic
[license_file]
license_file_path=/home/baklava/receivetool/EXandSF/license     //license文件路径
[data_ip]
selected_ip=239.72.102.190                                      //组播ip
[option_only]
switch=on                                                       //期权模式开关on/off（若为开启，则只订阅期权行情）
[subscribe]
switch=off                                                      //订阅模式开关on/off（若为开启，则订阅选择的securityId行情）
selected_security=600118,600119                                 //订阅的securityId，逗号连接
[output]
output_path=./res.csv                                           //日志路径
```

4、将makefile中的demo替换为编写的执行文件，执行make编译出可执行文件
```
make
```
4、执行可执行文件，参数为配置文件路径
```
./demo ./mdsh.ini
```

## 使用样例
---
### mdsh_demo
```
#include <signal.h>
#include <atomic>
#include <fstream>
#include <iostream>
#include "include/MdshReceiveExanic.h"
#include "sstream"
#include "string.h"

#define ARRLEN 24

std::string resPath;
std::ofstream resFile;

struct PackData
{
    uint64_t LocalTimeStamp;
    CMdshMd Md;
};

alignas(64) std::atomic<uint32_t> g_bStop {0};
PackData g_mdArray[ARRLEN];
alignas(64) uint32_t g_arrHeader {0};
alignas(64) uint32_t g_arrTailer {0};


static void on_exit(int sig)
{
    g_bStop = 1;
}

static inline uint64_t GetLocalTime()
{
    struct timespec tp = {0};
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((uint64_t)tp.tv_sec) * 1000000000 + tp.tv_nsec;
}

static void *RecordThread(void *)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(6, &mask);
    int result = sched_setaffinity(0, sizeof(mask), &mask);
    
    while (!g_bStop)
    {
        if (g_arrTailer < g_arrHeader)
        {
            
            PackData &item = g_mdArray[g_arrTailer % ARRLEN];

            std::stringstream ss;
            ss.setf(std::ios::fixed);
            ss.unsetf(std::ios::showpoint);

            ss << item.LocalTimeStamp << ","
                << std::hex << int(item.Md.verAndExchange) << ","
                << std::dec << int(item.Md.length) << ","
                << std::hex << int(item.Md.tradeModeAndSecurityType) << ","
                << item.Md.mdStreamId[0] << item.Md.mdStreamId[1] << item.Md.mdStreamId[2]
                << item.Md.mdStreamId[3] << item.Md.mdStreamId[4] << ","
                << std::dec << item.Md.tradingPhaseCode[0] << item.Md.tradingPhaseCode[1] << item.Md.tradingPhaseCode[2]
                << item.Md.tradingPhaseCode[3] << item.Md.tradingPhaseCode[4] << item.Md.tradingPhaseCode[5]
                << item.Md.tradingPhaseCode[6] << item.Md.tradingPhaseCode[7] << ","
                << std::dec << int(item.Md.timestamp) << ","
                << item.Md.securityId[0] << item.Md.securityId[1] << item.Md.securityId[2] << item.Md.securityId[3]
                << item.Md.securityId[4] << item.Md.securityId[5] << item.Md.securityId[6] << item.Md.securityId[7] << ","
                << std::dec << long(item.Md.totalValue) << ","
                << std::dec << long(item.Md.totalVolume) << ","
                << item.Md.lastPrice << ","
                << item.Md.openInterest;
            
            if (item.Md.length == 60)
            {
                resFile << ss.str() << std::endl;
                resFile.flush();
                ++g_arrTailer;
                continue;
            }

            ss << "," << item.Md.bidPrice[0] << ","
                << item.Md.bidPrice[1] << ","
                << item.Md.bidPrice[2] << ","
                << item.Md.bidPrice[3] << ","
                << item.Md.bidPrice[4] << ","
                << item.Md.bidVolume[0] << ","
                << item.Md.bidVolume[1] << ","
                << item.Md.bidVolume[2] << ","
                << item.Md.bidVolume[3] << ","
                << item.Md.bidVolume[4] << ","
                << item.Md.askPrice[0] << ","
                << item.Md.askPrice[1] << ","
                << item.Md.askPrice[2] << ","
                << item.Md.askPrice[3] << ","
                << item.Md.askPrice[4] << ","
                << item.Md.askVolume[0] << ","
                << item.Md.askVolume[1] << ","
                << item.Md.askVolume[2] << ","
                << item.Md.askVolume[3] << ","
                << item.Md.askVolume[4];
            
            resFile << ss.str() << "\n";
            resFile.flush();
            ++g_arrTailer;
        }
    }
    return NULL;
}

void ReceiveData(uint8_t *data, int len)
{
    PackData &item = g_mdArray[g_arrHeader % ARRLEN];
    item.LocalTimeStamp = GetLocalTime();
    memcpy(&item.Md, data, len);
    ++g_arrHeader;
}

void beginRecord(void)
{
    resFile << "LocalTimeStamp"
            << ","
            << "verAndExchange"
            << ","
            << "length"
            << ","
            << "tradeModeAndSecurityType"
            << ","
            << "mdStreamId"
            << ","
            << "tradingPhaseCode"
            << ","
            << "timestamp"
            << ","
            << "securityId"
            << ","
            << "totalValue"
            << ","
            << "totalVolume"
            << ","
            << "lastPrice"
            << ","
            << "openInterest"
            << ","
            << "bidPrice[0]"
            << ","
            << "bidPrice[1]"
            << ","
            << "bidPrice[2]"
            << ","
            << "bidPrice[3]"
            << ","
            << "bidPrice[4]"
            << ","
            << "bidVolume[0]"
            << ","
            << "bidVolume[1]"
            << ","
            << "bidVolume[2]"
            << ","
            << "bidVolume[3]"
            << ","
            << "bidVolume[4]"
            << ","
            << "askPrice[0]"
            << ","
            << "askPrice[1]"
            << ","
            << "askPrice[2]"
            << ","
            << "askPrice[3]"
            << ","
            << "askPrice[4]"
            << ","
            << "askVolume[0]"
            << ","
            << "askVolume[1]"
            << ","
            << "askVolume[2]"
            << ","
            << "askVolume[3]"
            << ","
            << "askVolume[4]"
            << "\n";
    resFile.flush();
}

int main(int argc, char **argv)
{
    signal(SIGINT, &on_exit);
    signal(SIGQUIT, &on_exit);
    
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(5, &mask);
    int result = sched_setaffinity(0, sizeof(mask), &mask);

    if (argc < 2)
    {
        std::cout << "no ini file input" << std::endl;
        return 0;
    }

    const char *configFile = argv[1];
    CMdshReceiver *rev;
    if (MdshReceiverCreate(&rev, configFile) < 0)
    {
        std::cout << "init error!" << std::endl;
        return 0;
    }

    const char *resPath = MdshGetOutputPath(rev);
    resFile.open(resPath, std::ios_base::out);
    if (!resFile)
    {
        std::cout << "ResFile open failed!" << std::endl;
        return 0;
    }

    beginRecord();

    pthread_t thread;
    pthread_create(&thread, NULL, &RecordThread, NULL);

    while (!g_bStop)
    {
        MdshReceiverRun(rev, ReceiveData);
    }

    std::cout << "finish" << std::endl;
    MdshReceiverClose(rev);
    return 0;
}
```
## 常见问题
---
## 版本更新记录
---
### 1. v0.1.0_20220922
初始发布版本

### 2. v1.1.0_20221025
更新license验证方式，并纳入投入产出系统管理，后续客户延期使用可直接在后台修改

### 3. v1.2.0_20221210
接口命名规范性调整，增加期权模式、订阅模式，增加配置文件日志路径功能

### 4. v1.2.1_20230111
对demo进行线程优化，独立线程记录日志及到达本地的软件时间