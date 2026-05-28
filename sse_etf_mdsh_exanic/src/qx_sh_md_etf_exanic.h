#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include <utility>  




#include "fb_md_plugin_api.h"
#include "common.h"

#include <exanic/exanic.h>
#include <exanic/fifo_rx.h>
#include <exanic/config.h>

#include "quota_stub.h"

#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()





#define  MDSTREAMTYPE_D004       0x34303044   //ETF
#define  MDSTREAMTYPE_D301       0x31303344   //option
#define  MDSTREAMTYPE_D102       0x32303144


using fb_market_data_t = cffex::fb::api::market_data_entity;

#pragma pack(push, 1)
struct SseMsg
{
    uint8_t  verAndExchange; // 高四位为版本号；低四位为交易所，上交所L1：1；上交所L2：2；深交所：3 |
    uint8_t  length;          // 指数行情固定为60，其他行情固定为180 |
    uint8_t  tradeModeAndSecurityType;  // 高四位为交易模式，Reserved：0；系统测试：1；模拟交易：2；正常交易：3； <br> 低四位为证券类型，股票：1；衍生品：2；综合业务：3；债券：c |
    char  mdStreamId[5];
    char  tradingPhaseCode[8];
    uint32_t timestamp;
    char  securityId[8];
    uint64_t totalValue;   // 成交金额；需除以100得到实际值 |
    uint64_t totalVolume;  // 成交数量 |
    uint64_t lastPrice;    // 需除以10^5得到实际值 |
    uint64_t openInterest;

    uint64_t bidPrice[5];   // 需除以10^5得到实际值 |
    uint32_t bidVolume[5];
    uint64_t askPrice[5];    // 需除以10^5得到实际值 |
    uint32_t askVolume[5];

};

struct SseMsgLevel{

};


#pragma pack(pop)

struct MultiCastConf
{
    std::string section;
    uint32_t    ip;   
    uint16_t    port;
    MultiCastConf(std::string a,uint32_t b, uint16_t c):section(a),ip(b),port(c){}
};

struct Config_t{
    std::string ifname;
    std::vector<MultiCastConf> multicast;
    int cpu_id;
    std::string filter_path;
};

class QxShMdEtfOptionExanic : public cffex::fb::api::fb_md_plugin_api{
public:
    QxShMdEtfOptionExanic();
    ~QxShMdEtfOptionExanic();

private:
    inline void LoadJsonCfg();
    void Routine();

    bool InitExanic(const char *ifName);

    inline bool belongTo(uint32_t ip, uint16_t port);

    void ReleaseExanic();
    void OnData();

public:           //override  cffex::fb::api::fb_i_md_spi
    int  init() override; 
    void release() override;
    void connect() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;

private:
    Config_t                   m_config;
    std::thread*               m_worker;

    //filter
    OptionInfoFilter  m_option_info;


    //exanic 
    exanic_t          *m_exanic;
    exanic_rx_t       *m_exanic_rx;
    std::atomic<bool> m_exanic_initialized;

    //febao
    std::atomic<bool>                                m_fb_initialized;  
    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;


    struct fb_md_interval_t{
        double open;
        double high;
        double low;
        void clear(){
            open = FLOAT64_NAN;
            high = FLOAT64_NAN;
            low = FLOAT64_NAN;
        };

        fb_md_interval_t(double a, double b, double c):open(a),high(b),low(c){};
    };

    std::unordered_map<uint64_t, fb_md_interval_t>        m_fb_md_interval;  // 上交所合约名最长只有8，转成 uint64_t

    std::shared_ptr<NanoStamp> m_TS;

    uint64_t delay_sum = 0;
    uint64_t delay_sum_0 = 0; //  -0
    uint64_t delay_sum_1 = 0; // 0-1
    uint64_t delay_sum_2 = 0; // 1-2
    uint64_t delay_sum_3 = 0; // 2-3
    uint64_t delay_sum_4 = 0; // 3-end
    uint64_t delay_sum_5 = 0;
    uint64_t count_stats = 0;


};
