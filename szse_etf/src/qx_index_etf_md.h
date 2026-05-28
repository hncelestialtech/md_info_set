#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include "acc_md_sh.h"
#include "acc_md.h"
#include "fb_md_plugin_api.h"
#include "common.h"
#include "tszch_gtjamch.h"

#include <exanic/exanic.h>
#include <exanic/fifo_rx.h>
#include <exanic/config.h>

#include "quota_stub.h"

#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()
#define MSG_MAX_LEN     1516  //所有类型的行情数据msg的最大长度，不包含协议头
#define QUEUE_MAX_LEN   (1024*1024)  //环形队列的长度

using fb_market_data_t = cffex::fb::api::market_data_entity;

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

    // 流控
    uint32_t mode;   // 0: 不做流控。1：time_span内直接丢弃  2：time_span内LV1 没变化的丢弃
    // 相同的合约，在time_span 内，如果1档位的值无变化，则过滤此条行情
    long long time_span;   //配置单位ms，存储时单位 ns
};

struct Monitor_Lable_t{
    uint64_t time;
    uint64_t ap1;  //放大1000000
    uint64_t av1;  //放大100
    uint64_t bp1;  //放大1000000
    uint64_t bv1;  //放大100
    Monitor_Lable_t(): time(0), ap1(0),av1(0),bp1(0),bv1(0){};
};

class QxIndexEtfMd : public cffex::fb::api::fb_md_plugin_api{
public:
    QxIndexEtfMd();
    ~QxIndexEtfMd();

private:
    inline void LoadJsonCfg();
    void Routine();

    bool InitExanic(const char *ifName);

    inline bool belongTo(uint32_t ip, uint16_t port);
    inline void handle_szse_snapshot_ms(const char *data, fb_market_data_t *targetdata);
    inline void handle_szse_option_snapshot(const char *data, fb_market_data_t *targetdata);

    void ReleaseExanic();
    void OnData();

    inline std::string GetInstrumentInfo(int msgtype, const char *aData, uint64_t &ap1, uint64_t &av1, uint64_t &bp1, uint64_t &bv1);

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

    //traffic
    std::unordered_map<std::string, Monitor_Lable_t>   m_monitor_last_quota;

    //exanic 
    exanic_t          *m_exanic;
    exanic_rx_t       *m_exanic_rx;
    std::atomic<bool> m_exanic_initialized;

    //febao
    std::atomic<bool>                                m_fb_initialized;  
    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;
    cffex::fb::api::inquiry_quote_entity             *m_fb_inquiry;
    cffex::fb::api::instrument_trading_status_entity *m_fb_status;

    // QuotaStub m_quota_stub;   // TODO stub



};
