#pragma once

#include <string> 
#include <set>
#include <atomic>
#include <thread>

#include <utility>  


#include "MdszRecv.h"

#include "fb_md_plugin_api.h"
#include "common.h"

#include <exanic/exanic.h>
#include <exanic/fifo_rx.h>
#include <exanic/config.h>

// #include "quota_stub.h"

#define FLOAT64_NAN std::numeric_limits<double>::quiet_NaN()
#define MSG_MAX_LEN     1516  //所有类型的行情数据msg的最大长度，不包含协议头
#define QUEUE_MAX_LEN   (1024*1024)  //环形队列的长度






#define  MDENTRYTYPE_0       0x2030
#define  MDENTRYTYPE_1       0x2031
#define  MDENTRYTYPE_2       0x2032 
#define  MDENTRYTYPE_4       0x2034 
#define  MDENTRYTYPE_7       0x2037 
#define  MDENTRYTYPE_8       0x2038 
#define  MDENTRYTYPE_xe      0x6578 
#define  MDENTRYTYPE_xf      0x6678 
#define  MDENTRYTYPE_xg      0x6778 
#define  MDENTRYTYPE_x1      0x3178 
#define  MDENTRYTYPE_x2      0x3278
#define  MDENTRYTYPE_xi      0x6978 

using fb_market_data_t = cffex::fb::api::market_data_entity;




struct Quotation{
    std::string instrument_id;
    //int8_t   exchange_id;
    int32_t  update_sec;
    int32_t  update_msec;
    double   pre_settlement;
    double   pre_close;
    double   pre_open_interest;
    double   open;
    double   close;
    double   upper;
    double   lower;
    double   high;
    double   low;
    double   last_price;
    int32_t  volume;
    double   turn_over;
    double   open_interest;
    int32_t  level;
    double   bp[10];
    double   ap[10];
    int32_t  bv[10];
    int32_t  av[10];
    double   iopv;
    double   dynamic_reference_price;
    // uint64_t local_timestamp;


    std::string to_string(){
        std::string ret = instrument_id +",update_sec:" + std::to_string(update_sec) + ",update_msec:" + std::to_string(update_msec)
                                        +",pre_settlement:" + std::to_string(pre_settlement) + ",pre_close:" + std::to_string(pre_close)+",pre_open_interest:" + std::to_string(pre_open_interest) 
                                        +",open:" + std::to_string(open)+",close:" + std::to_string(close) 
                                        +",upper:" + std::to_string(upper)+ ",lower:" + std::to_string(lower)
                                        +",high:" + std::to_string(high) + ",low:" + std::to_string(low)
                                        +",last_price:" + std::to_string(last_price) + ",volume:" + std::to_string(volume)+",turn_over:" + std::to_string(turn_over) 
                                        +",open_interest:" + std::to_string(open_interest) +",level:" + std::to_string(level);

        for (int i = 0; i< level; ++i){
            ret += (",bp" + std::to_string(i) + ":" + std::to_string(bp[i]));
            ret += (",bv" + std::to_string(i) + ":" + std::to_string(bv[i]));
            ret += (",ap" + std::to_string(i) + ":" + std::to_string(ap[i]));
            ret += (",av" + std::to_string(i) + ":" + std::to_string(av[i]));
        }
        return ret;
    }


};










struct Config_t{
    std::string mdsz_cfg_file;
    int cpu_id;
    std::string filter_path;

    // 流控
    // uint32_t mode;   // 0: 不做流控。1：time_span内直接丢弃  2：time_span内LV1 没变化的丢弃
    // // 相同的合约，在time_span 内，如果1档位的值无变化，则过滤此条行情
    // long long time_span;   //配置单位ms，存储时单位 ns
};



class QxSzMdEtfOption : public cffex::fb::api::fb_md_plugin_api{
public:
    QxSzMdEtfOption();
    ~QxSzMdEtfOption();

private:
    inline void LoadJsonCfg();
    void Routine();

    bool InitSzMd(std::string &cfgfile);

    static void OnData(uint8_t *data, int len);



public:           //override  cffex::fb::api::fb_i_md_spi
    int  init() override; 
    void release() override;
    void connect() override;
    void subscribe_inst(const std::string &instrument_id, uint8_t exchange_id) override;
    void register_spi(cffex::fb::api::fb_i_md_spi *spi) override;

private:
    Config_t                   m_config;
    std::thread*               m_worker;



    //szmd rev 
    MdszL1Receiver   *m_szmd_rev;
    std::atomic<bool> m_szmd_initialized;

    //febao



    



};
