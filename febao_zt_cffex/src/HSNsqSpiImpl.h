#pragma once

#include <fstream>
#include <time.h>
#include <sys/timeb.h>
#include <stdint.h>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <map>
#include <cstdlib>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <string.h>


#include <memory>

#include "HSNsqApi.h"



using MarketCodeCache = std::unordered_map < std::string, std::vector<std::string>>;

struct UserInfo
{
    // 保存登录、重建信息
    CHSNsqReqUserLoginField reqLoginField;
    CHSNsqReqSecuTransactionRebuildField reqTransRebuild;

    // user.properties指定代码保存到相应的vector中
    MarketCodeCache market_codes;
    // 保存sdk_config.ini中配置的市场
    std::unordered_set<std::string> sdk_config_market;
    UserInfo()
    {
        memset(&reqLoginField, 0, sizeof(reqLoginField));
        memset(&reqTransRebuild, 0, sizeof(reqTransRebuild));
    }
};

const std::unordered_map<std::string, std::string> MARET_TYPE_CONVERT =
    {
        {"sh", HS_EI_SSE},
        {"sz", HS_EI_SZSE},
        {"swi", HS_EI_SWI},
        {"neeq", HS_EI_TZASE},
        {"bj", HS_EI_BJSE},
        {"shfe", HS_EI_SHFE},
        {"dce", HS_EI_DCE},
        {"czce", HS_EI_CZCE},
        {"cffex", HS_EI_CFFEX},
        {"ine", HS_EI_INE},
    };


typedef void (*FutQuotaHandler)(void * ctx, CHSNsqFutuDepthMarketDataField *);

typedef void (*FutStaticInfoHandler)(void * ctx, CHSNsqFutuInstrumentStaticInfoField *info, bool bIsLast);


class CHSNsqSpiImpl : public CHSNsqSpi
{

public:
    CHSNsqSpiImpl(CHSNsqApi *lpHSNsqApi);
    ~CHSNsqSpiImpl();

    void SetUserInfo(UserInfo *info){
        m_user_info = info;
    }

    void SetFutQuotaCallBack(void *ctx, FutQuotaHandler cb, FutStaticInfoHandler cb2){
        m_ctx = ctx;
        m_future_quota_cb = cb;
        m_future_staticinfo_cb = cb2;
    }

    /// Description: 当客户端与后台开始建立通信连接，连接成功后此方法被回调。
    virtual void OnFrontConnected();

    /// Description:当客户端与后台通信连接异常时，该方法被调用。
    /// Others     :通过GetApiErrorMsg(nResult)获取详细错误信息。
    virtual void OnFrontDisconnected(int nResult);

    /// Description:客户登录
    virtual void OnRspUserLogin(CHSNsqRspUserLoginField *pRspUserLogin, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);



    /// Description: 期权订阅-行情应答
    virtual void OnRspOptDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 期权订阅取消-行情应答
    virtual void OnRspOptDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 主推-期权行情
    virtual void OnRtnOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData);

    /// Description: 获取当前交易日合约应答
    virtual void OnRspQryOptInstruments(CHSNsqOptInstrumentStaticInfoField *pOptInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 获取合约的最新快照信息应答
    virtual void OnRspQryOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);


    /// Description: 期货订阅-行情应答
    virtual void OnRspFutuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 期货订阅取消-行情应答
    virtual void OnRspFutuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 主推-期货行情
    virtual void OnRtnFutuDepthMarketData(CHSNsqFutuDepthMarketDataField *pFutuDepthMarketData);

    /// Description: 主推-静态代码表数据初始化通知
    virtual void OnRtnInstrumentsDataChangeNotice(CHSNsqInstrumentsDataChangeNoticeField *pInstrumentsDataChangeNotice);


    ///  期货码表
    /// Description: 获取当前交易日合约应答
    virtual void OnRspQryFutuInstruments(CHSNsqFutuInstrumentStaticInfoField *pFutuInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 获取合TCP连接状态
    bool GetConnectStatus() { return m_isConnected; }

    /// Description: 获取登陆状态
    bool GetLoginStatus() { return m_isLogined; }

    /// Description: 获取静态代码表状态
    bool GetInstrumentsStatus() { return m_isAllInstrReady; }

    /// Description: 获取静态代码表数量
    int GetInstrumentsCount() { return m_iAllInstrCount; }



private:
    CHSNsqApi *m_lpHSNsqApi;

    bool m_isConnected{false};
    bool m_isLogined{false};
    bool m_isAllInstrReady{false};
    int m_iAllInstrCount{0};

    UserInfo *m_user_info;

    void *m_ctx;
    FutQuotaHandler  m_future_quota_cb;
    FutStaticInfoHandler m_future_staticinfo_cb;
  
};












