#include <HSNsqApi.h>
#include "Functions.h"
#include "HSNsqSpiImpl.h"
#include "InnerData.h"

#include <cstdio>
#include <thread>
#include <chrono>
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
#define SIGQUIT SIGBREAK
#define atoll(val) _atoi64(val)
#else
#include <unistd.h>
#endif // _WIN32

// 用户信息
UserInfo g_user_info;
// 是否继续运行
volatile int keepRunning = 1;
volatile int g_quit = 0;

/*******************************************************************
    本示例仅作为接口调用简单示例，实际运行中各异常需自行判断处理
*******************************************************************/
int main(int argc, char **argv)
{
    printf("HSNsqApiDemo start, built time : [%s %s]\n", __DATE__, __TIME__);
    // 注册中断信号
    (void)signal(SIGABRT, sigHandler); // 异常终止
    (void)signal(SIGINT, sigHandler);  // ctrl + c
    (void)signal(SIGTERM, sigHandler); // 程序结束信号
    (void)signal(SIGQUIT, sigHandler); // ctrl + "\"

    std::string ApiConfigName = "sdk_config.ini";
    // 检查是否提供了所需的参数
    if (argc > 1)
    {
        ApiConfigName = argv[1];
    }

    int nRequestID = 0;
    int iRetries = 0;

    // 随包demo读取自己的配置文件，获取用户名、密码、订阅代码。请根据实际场景自行实现用户名密码等信息的获取
    readUserFile(g_user_info);

    // 初始化行情API
    // NewNsqApi接口创建的NSQ API实例默认读取当前程序目录下的sdk_config.ini配置文件，NewNsqApiExt接口创建的NSQ API实例可读取指定的配置文件
    CHSNsqApi *lpNsqApi = NewNsqApiExt("./log/", ApiConfigName.c_str());
    // 可能是NULL
    if (NULL == lpNsqApi)
    {
        printf("ERROR: NewNsqApiExt failed!\n");
        return -1;
    }
    CHSNsqSpiImpl *lpNsqSpi = new CHSNsqSpiImpl(lpNsqApi, g_user_info.output_data);
    lpNsqApi->RegisterSpi(lpNsqSpi);

    // 随包demo读取NSQ SDK的配置文件，读取配置文件中的support_markets配置项
    // 如果需要读取其他路径或者其他名字的配置文件,使用NewNsqApiExt接口，同时需要改sdk_cfg_path变量，重新编译
    // 请根据实际场景自行决定是否需要读取sdk配置文件，NSQ SDK会自动读取指定的配置文件，此处为随包demo由于实现需要读取SDK配置文件
    readSDKConfig(g_user_info, ApiConfigName);

    // 说明：RegisterFront接口NSQ中无用，NSQ通过配置文件配置行情站点和灾备地址。为了和恒生经纪柜台保持兼容而保留该接口，调用无效
    (void)lpNsqApi->RegisterFront("");

    // 说明：Init接口需调用，NSQ中接口参数均可不填，初始化参数从配置文件中读取。为了和恒生经纪柜台保持兼容
    int iRet = lpNsqApi->Init("");
    if (iRet != 0)
    {
        printf("ERROR: init failed: iRet %d, error: %s\n", iRet, lpNsqApi->GetApiErrorMsg(iRet));
        lpNsqApi->ReleaseApi();
        return -1;
    }

    // 若Tcp连接超时，自动切换到备用地址
    std::this_thread::sleep_for(std::chrono::seconds(3));
    while (keepRunning && !lpNsqSpi->GetConnectStatus())
    {
        printf("ERROR: Connect to sailfish_service or HQReceiverGW failed, please check the ip/port or waiting to switch address!\n");
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    // 当连接出现错误时，直接退出。
    if (!lpNsqSpi->GetConnectStatus())
    {
        printf("ERROR: Connect failed, error exit!\n");
        lpNsqApi->ReleaseApi();
        return -1;
    }

    iRetries = 3000; /* 等待登录成功，等待30秒超时 */
    while (keepRunning && !lpNsqSpi->GetLoginStatus() && (iRetries--) > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    // 当登录出现错误时，程序会打印出错误原因，之后直接退出。
    if (!lpNsqSpi->GetLoginStatus())
    {
        printf("ERROR: login failed, error exit! error: %s\n", lpNsqApi->GetApiErrorMsg(0));
        lpNsqApi->ReleaseApi();
        return -1;
    }

    // 现货静态信息请求
    ReqStockInstruments(lpNsqApi, HS_EI_SZSE, sizeof(HS_EI_SZSE), nRequestID);   // 深交所
    ReqStockInstruments(lpNsqApi, HS_EI_SSE, sizeof(HS_EI_SSE), nRequestID);     // 上交所
    ReqStockInstruments(lpNsqApi, HS_EI_BJSE, sizeof(HS_EI_BJSE), nRequestID);   // 北交所
    ReqStockInstruments(lpNsqApi, HS_EI_TZASE, sizeof(HS_EI_TZASE), nRequestID); // 股转
    // 期货、期货期权静态信息请求
    // ReqFutInstruments(lpNsqApi, HS_EI_SHFE, sizeof(HS_EI_SHFE), nRequestID);   // 上期所
    // ReqFutInstruments(lpNsqApi, HS_EI_DCE, sizeof(HS_EI_DCE), nRequestID);     // 大商所
    // ReqFutInstruments(lpNsqApi, HS_EI_CZCE, sizeof(HS_EI_CZCE), nRequestID);   // 郑商所
    // ReqFutInstruments(lpNsqApi, HS_EI_INE, sizeof(HS_EI_INE), nRequestID);     // 能源
    // ReqFutInstruments(lpNsqApi, HS_EI_CFFEX, sizeof(HS_EI_CFFEX), nRequestID); // 中金所

    iRetries = 500; /* 等待所有代码就绪，等待5秒超时 */
    /* m_isAllInstrReady 在 CHSNsqSpiImpl::OnRspQrySecuAllInstruments 中收到最后一个代码置成true */
    while (!lpNsqSpi->GetInstrumentsStatus() && (iRetries--) > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!lpNsqSpi->GetInstrumentsStatus())
    {
        printf("WARNING: All Instruments are not ready after ReqQrySecuInstruments!\n");
        // lpNsqApi->ReleaseApi();
        // return -1; /// NOTE: 请根据自身业务逻辑判断是否要退出
    }
    else
    {
        printf("All Instruments are ready, count %d!\n", lpNsqSpi->GetInstrumentsCount());
    }

    // 写文件线程
    std::thread thr_write;
    if (OutPutData::SCREEN != g_user_info.output_data)
    {
        // 创建用于数据落地的写文件线程
        thr_write = std::thread(thr_write_fn, lpNsqSpi);
    }

    // 现货市场订阅
    StockSubscribe(lpNsqApi, HS_EI_SZSE, sizeof(HS_EI_SZSE), nRequestID);   // 深交所
    StockSubscribe(lpNsqApi, HS_EI_SSE, sizeof(HS_EI_SSE), nRequestID);     // 上交所
    StockSubscribe(lpNsqApi, HS_EI_BJSE, sizeof(HS_EI_BJSE), nRequestID);   // 北交所
    StockSubscribe(lpNsqApi, HS_EI_TZASE, sizeof(HS_EI_TZASE), nRequestID); // 股转
    // 期货、期货期权订阅
    FutSubscribe(lpNsqApi, HS_EI_SHFE, sizeof(HS_EI_SHFE), nRequestID);   // 上期所
    FutSubscribe(lpNsqApi, HS_EI_DCE, sizeof(HS_EI_DCE), nRequestID);     // 大商所
    FutSubscribe(lpNsqApi, HS_EI_CZCE, sizeof(HS_EI_CZCE), nRequestID);   // 郑商所
    FutSubscribe(lpNsqApi, HS_EI_INE, sizeof(HS_EI_INE), nRequestID);     // 能源
    FutSubscribe(lpNsqApi, HS_EI_CFFEX, sizeof(HS_EI_CFFEX), nRequestID); // 中金所

    // sdk配置文件配置沪深市场才会发起重建请求
    Rebuild(lpNsqApi, nRequestID);

    while (keepRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    g_quit = 1;

    if (thr_write.joinable())
    {
        thr_write.join();
    }

    lpNsqApi->ReleaseApi();

    delete lpNsqSpi;

    return 0;
}