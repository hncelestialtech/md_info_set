#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include <utility>  

#include "fb_md_plugin_api.h"
#include "common.h"

#include "MdshReceiveEfvi.h"


#include "inst_map.h"

#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()





#define  MDSTREAMTYPE_D004       0x34303044   //ETF
#define  MDSTREAMTYPE_D301       0x31303344   //option
#define  MDSTREAMTYPE_D102       0x32303144


using fb_market_data_t = cffex::fb::api::market_data_entity;

#pragma pack(1)
struct QuotaInfo{
    double open;
    double high;
    double low;
};
#pragma pack()

struct MultiCastConf
{
    std::string section;
    uint32_t    ip;   
    uint16_t    port;
    MultiCastConf(std::string a,uint32_t b, uint16_t c):section(a),ip(b),port(c){}
};

struct Config_t{
    std::string mdshcfg;
    int cpu_id;
    std::string filter_path;
};

class QxShMdEtfOptionEfvi : public cffex::fb::api::fb_md_plugin_api{
public:
    QxShMdEtfOptionEfvi();
    ~QxShMdEtfOptionEfvi();

private:
    inline void LoadJsonCfg();
    void Routine();

    static void OnData(uint8_t *data, int len);
    void ReceiveData(uint8_t *data, int len);


public:           //override  cffex::fb::api::fb_i_md_spi
    int  init() override; 
    void release() override;
    void connect() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;

private:
    Config_t                   m_config;
    std::thread*               m_worker;

    CMdshReceiver *m_efvi_recevier;


    //febao
    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    fb_market_data_t                                 *m_fb_md;


    std::set<std::string>     m_subscribe_insts;
    InstMap<QuotaInfo>            m_quota_cache; 



};
