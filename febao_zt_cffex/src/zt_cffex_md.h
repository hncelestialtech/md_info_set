#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include <utility>  


#include <algorithm>

#include "fb_md_plugin_api.h"
#include "common.h"



#include "inst_map.h"


#include "macro.h"

#include "HSDataType.h"
#include "HSNsqApi.h"
#include "HSNsqStruct.h"

#include "HSNsqSpiImpl.h"


#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()



using fb_market_data_t = cffex::fb::api::market_data_entity;




struct Config_t{
    std::string sdk_config;
    std::string user_config;
    int         cpu_id;
    std::string filter_path;
};

//lc2601-p-234000



struct QuotaInfo{

    double   open;
    double   high;
    double   low;
};





class CFFEXExanicQuota : public cffex::fb::api::fb_md_plugin_api{
public:
    CFFEXExanicQuota();
    ~CFFEXExanicQuota();

private:
    inline void LoadJsonCfg();
    void Routine();

    void OnData();

    void readUserFile(UserInfo &user_info);
    void readSDKConfig(UserInfo &user_info);

    void ReqAllFutInstruments(const char *market, int len, int &nRequestID);
    void ReqFutInstruments(const char *market, int len, int &nRequestID);

    void FutSubscribe(const char *market, int len, int &nRequestID);

    static void Quota_CallBack(void * ctx, CHSNsqFutuDepthMarketDataField *quota);
    static void StaticInfo_CallBack(void * ctx, CHSNsqFutuInstrumentStaticInfoField *info, bool bIsLast);


    void Quota_Recv(CHSNsqFutuDepthMarketDataField *quota);
    void StaticInfo_Recv(CHSNsqFutuInstrumentStaticInfoField *info, bool bIsLast);

public:           //override  cffex::fb::api::fb_i_md_spi
    int  init() override; 
    void release() override;
    void connect() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;

private:
    Config_t                   m_config;
    std::thread*               m_worker;

    std::set<std::string>      m_subscribe_insts;

    //febao
    std::atomic<bool>                                 m_fb_initialized;  
    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;

    // QuotaStub m_quota_stub;   // TODO stub

    InstMap<QuotaInfo>           m_quota_cache;


    UserInfo m_user_info;

    CHSNsqApi *m_NsqApi;

    CHSNsqSpiImpl *m_NsqSpi;


};
