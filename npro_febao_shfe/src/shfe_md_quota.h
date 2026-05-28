#pragma once

#include "NProUserApi.h"
#include "fb_md_plugin_api.h"

#include "common.h"

#include <cstring>
#include <limits>
#include <atomic>
#include <thread>

#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()

struct NProConfig{
    std::string deviceName;
    std::string licenseFile;
    std::string configFile;
    int32_t cpuID;
    std::vector<std::string> subscribes;
    std::string writeFile;
    bool isVersion;
    std::string exchange;
    bool isReadOnly;
    int productClass;

    NProConfig():deviceName{},licenseFile{},configFile{},cpuID(-1),subscribes{},writeFile{},isVersion(false),
    exchange("SHFE"),isReadOnly(false),productClass(ENProUserProductClassKind::NProUserProductClassAll){};
};



struct Config_t{
    int cpu_id;
    std::string filter_path;

    bool stats;
    std::string logpath;
    uint32_t    logtime;


};



struct DelaySummary{
    uint32_t extime;
    uint32_t extime_ms;
    int64_t  local_time;
    DelaySummary(uint32_t a,uint32_t b, int64_t c):extime(a), extime_ms(b), local_time(c){}
};



class ShfeMDQuota : public cffex::fb::api::fb_md_plugin_api{
public:
    ShfeMDQuota();
    ~ShfeMDQuota();

private:
    void Routine();
    void StatsRoutine();
    inline void MsgRoutine();

    void inst_stats(std::string inst);

    inline bool LoadJsonCfg();

public:           //override  cffex::fb::api::fb_i_md_spi
    int  init();
    void release();
    void connect();
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id);
    void register_spi(cffex::fb::api::fb_i_md_spi *spi);

private:
    NProConfig     m_npro_cfg;
    CNProUserApi  *m_npro_api;

    int            m_fb_exchange_id;

    std::unordered_map<std::string, TNProInstrumentItem> m_npro_inst_info; //合约元数据

    //febao
    std::atomic<bool> m_npro_initialized;
    std::atomic<bool> m_fb_initialized;  //先ready发送，再ready接收

    Config_t    m_config;
    cffex::fb::api::fb_i_md_spi                      *m_fb_spi;
    cffex::fb::api::market_data_entity               *m_fb_md;

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

    std::unordered_map<std::string, fb_md_interval_t>     m_fb_md_interval;

    cffex::fb::api::inquiry_quote_entity             *m_fb_inquiry;
    cffex::fb::api::instrument_trading_status_entity *m_fb_status;

    std::thread*  m_worker;
    
    OptionInfoFilter  m_option_info;

    std::unordered_map<std::string, std::set<std::string>>  m_febao_inst;

    std::vector<DelaySummary>    m_delaystats;
    std::thread*                 m_stats_thread;

};
