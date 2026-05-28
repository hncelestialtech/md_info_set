#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include <utility>  


#include <algorithm>

#include "fb_md_plugin_api.h"
#include "common.h"

#include <exanic/exanic.h>
#include <exanic/fifo_rx.h>
#include <exanic/config.h>

#include "inst_map.h"


#include "macro.h"

#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()

#define KEYMODE_REGISTER_ADDR    0x21c  //keymode
#define KEYARG_REGISTER_ADDR     0x220  //keyarg

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
    std::string zx_ini;
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

    bool writeKEYMODE();
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

    //exanic 
    exanic_t          *m_exanic;
    exanic_rx_t       *m_exanic_rx;
    std::atomic<bool>  m_exanic_initialized;

    //febao
    std::atomic<bool>                                 m_fb_initialized;  
    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;

    // QuotaStub m_quota_stub;   // TODO stub

    InstMap<QuotaInfo>           m_quota_cache;


};
