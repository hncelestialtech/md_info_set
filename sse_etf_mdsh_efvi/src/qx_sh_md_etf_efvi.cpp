#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <sched.h>

#include <string.h>
#include <stdio.h>

#include "qx_sh_md_etf_efvi.h"
#include "SockProto.h"

#include "fb_md_type.h"



// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    printf("create QxShMdEtfOptionEfvi[%s]\n");
    return new QxShMdEtfOptionEfvi();
}
void destroy(void *p) {
    printf("destroy QxShMdEtfOptionEfvi\n");
    delete (QxShMdEtfOptionEfvi*)p;
}
void get_md_api_version(char version[32]) {
    printf("[%s][%s]\n",__func__,FEBAO_MD_API_VERSION);
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif

std::atomic<bool> g_running = false;

QxShMdEtfOptionEfvi * g_ctx = nullptr; 


QxShMdEtfOptionEfvi::QxShMdEtfOptionEfvi(): 
    m_fb_spi(nullptr),
    m_fb_md(cffex::fb::api::market_data_entity::create_entity())
{
    std::clog << std::unitbuf; //调试用
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
}




QxShMdEtfOptionEfvi::~QxShMdEtfOptionEfvi(){
    std::clog << std::nounitbuf;
}

void QxShMdEtfOptionEfvi::register_spi(cffex::fb::api::fb_i_md_spi *spi){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    m_fb_spi = spi;
}



inline void QxShMdEtfOptionEfvi::LoadJsonCfg(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    cffex::fb::api::fb_md_config_helper *parser = m_fb_spi->get_config_helper();
    std::string md_jsoncfgfile;
    parser->get_attribute("file", "/md_config_path", md_jsoncfgfile);
    std::clog <<__func__<<","<< __LINE__<<",md_jsoncfgfile[" <<md_jsoncfgfile<<"]"<< std::endl;
    // cfg
    std::ifstream jsonfile(md_jsoncfgfile);
    nlohmann::json json_parser = nlohmann::json::parse(jsonfile);

    auto& eth = json_parser["eth"];

    m_config.mdshcfg = eth["mdshcfg"];
    std::clog <<__func__<<","<< __LINE__<<",mdshcfg[" <<m_config.mdshcfg<<"]"<< std::endl;

    auto& worker = json_parser["worker"];
    m_config.cpu_id = worker["cpuid"].get<uint32_t>();

    auto& filter = json_parser["filter"];
    if(filter.contains("underlying")){
        for (const auto& item : filter["underlying"]) {
            m_subscribe_insts.emplace(item.get<std::string>());
            std::clog <<__func__<<","<< __LINE__<<",subscribe_insts underlying[" <<item.get<std::string>()<<"]"<< std::endl;        
        }
    }

    std::unordered_map<std::string, std::string> underlying_inst_map;
    LoadFebaoInstrumentInfo(m_config.filter_path, underlying_inst_map);
    std::clog <<__func__<<","<< __LINE__<<",Load ret[" <<underlying_inst_map.size()<<"]"<< std::endl;

    for(auto &itor : underlying_inst_map){
        m_subscribe_insts.emplace(itor.first);
        m_subscribe_insts.emplace(itor.second);
        std::clog <<__func__<<","<< __LINE__<<",subscribe_insts underlying[" <<itor.first<<"],["<<itor.second<<"]"<< std::endl;
    }

    std::clog <<__func__<<","<< __LINE__<<",subscribe_insts[" <<m_subscribe_insts.size()<<"]"<< std::endl;


    for (auto inst : m_subscribe_insts){
        m_quota_cache.InitOnce(inst);
    }

    m_quota_cache.Sort();

    std::clog <<__func__<<","<< __LINE__<<",read_cpu[" <<m_config.cpu_id<<"]"<< std::endl;
    std::clog <<__func__<<","<< __LINE__<<",filter_path[" <<m_config.filter_path<<"]"<< std::endl;

}

void on_exit(int sig){
    g_running.store(false, std::memory_order_release);
}


// struct CMdshMd
// {
//     uint8_t verAndExchange;                 //高四位为版本号，低四位为交易所 上交所L1：1 上交所L2：2 深交所：3
//     uint8_t length;                         //总体长度
//     uint8_t tradeModeAndSecurityType;       //高四位为交易模式 Reserved:0  系统测试:1   模拟交易:2   正常交易:3
//                                             //低四位为证券类型 1: 股票 2：衍生品 3：综合业务 c：债券
//     uint8_t mdStreamId[5];                  //行情类别
//     uint8_t tradingPhaseCode[8];            //实时阶段及标志
//     uint32_t timestamp;                     //时间HHMMSSss 精度为10ms
//     uint8_t securityId[8];                  //产品代码
//     uint64_t totalValue;                    //成交金额
//     uint64_t totalVolume;                   //成交数量
//     uint64_t lastPrice;                     //最新价
//     uint64_t openInterest;                  //持仓量
//     uint64_t bidPrice[5];                   //五档买价
//     uint32_t bidVolume[5];                  //五档买量
//     uint64_t askPrice[5];                   //五档卖价
//     uint32_t askVolume[5];                  //五档卖量
// };


void QxShMdEtfOptionEfvi::OnData(uint8_t *data, int len){
    g_ctx->ReceiveData(data,len);
}


void QxShMdEtfOptionEfvi::ReceiveData(uint8_t *data, int len){
    uint64_t local_time_ns_start = get_nanoseconds();

    if (data == nullptr || len ==0){
        return;
    }

    CMdshMd *msgdata = (CMdshMd *)data;
    QuotaInfo *cache = m_quota_cache.Find((char *)msgdata->securityId);
    if (cache == nullptr){
        return;
    }

    m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],inst[%s][%s],timestamp[%d],totalValue[%.3f],totalVolume[%llu],lastPrice[%.4f],openInterest[%llu]\n",__LINE__,
                                        msgdata->securityId,msgdata->mdStreamId,
                                        msgdata->timestamp,
                                        msgdata->totalValue,
                                        msgdata->totalVolume,
                                        msgdata->lastPrice,
                                        msgdata->openInterest);

    m_fb_md->reset_entity();                                // 重置飞豹行情结构
    m_fb_md->set_local_timestamp(local_time_ns_start); //utc ns
    m_fb_md->set_max_depth(5);                              // 设置行情深度5
    m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
    m_fb_md->set_instrument_id((char *)msgdata->securityId);
    m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_SSE);
    m_fb_md->set_update_sec(hhmmssToUtcSeconds2(msgdata->timestamp/1000));
    m_fb_md->set_update_msec(msgdata->timestamp%1000);

    m_fb_md->set_last_price(convert_price_if_zero(msgdata->lastPrice));
    m_fb_md->set_volume(msgdata->totalVolume);
    m_fb_md->set_turn_over(msgdata->totalValue);
    m_fb_md->set_open_interest(msgdata->openInterest);

    m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->askPrice[0],msgdata->askVolume[0]));
    m_fb_md->set_ask2_price(convert_price_if_volume_zero(msgdata->askPrice[1],msgdata->askVolume[1]));
    m_fb_md->set_ask3_price(convert_price_if_volume_zero(msgdata->askPrice[2],msgdata->askVolume[2]));
    m_fb_md->set_ask4_price(convert_price_if_volume_zero(msgdata->askPrice[3],msgdata->askVolume[3]));
    m_fb_md->set_ask5_price(convert_price_if_volume_zero(msgdata->askPrice[4],msgdata->askVolume[4]));
    m_fb_md->set_ask1_volume(msgdata->askVolume[0]);
    m_fb_md->set_ask2_volume(msgdata->askVolume[1]);
    m_fb_md->set_ask3_volume(msgdata->askVolume[2]);
    m_fb_md->set_ask4_volume(msgdata->askVolume[3]);
    m_fb_md->set_ask5_volume(msgdata->askVolume[4]);
    m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->bidPrice[0],msgdata->bidVolume[0]));
    m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata->bidPrice[1],msgdata->bidVolume[1]));
    m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata->bidPrice[2],msgdata->bidVolume[2]));
    m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata->bidPrice[3],msgdata->bidVolume[3]));
    m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata->bidPrice[4],msgdata->bidVolume[4]));
    m_fb_md->set_bid1_volume(msgdata->bidVolume[0]);
    m_fb_md->set_bid2_volume(msgdata->bidVolume[1]);
    m_fb_md->set_bid3_volume(msgdata->bidVolume[2]);
    m_fb_md->set_bid4_volume(msgdata->bidVolume[3]);
    m_fb_md->set_bid5_volume(msgdata->bidVolume[4]);

    uint64_t local_time_ns_0 = get_nanoseconds();

    m_fb_spi->on_msg(m_fb_md);

    uint64_t local_time_ns_end = get_nanoseconds();
    static uint64_t delay_sum = 0;
    static uint64_t delay_sum_0 = 0; //  -0
    static uint64_t delay_sum_1 = 0; // 0-1

    static uint64_t period = 200;
    static uint64_t count_stats = 0;

    delay_sum   += (local_time_ns_end - local_time_ns_start);
    delay_sum_0 += (local_time_ns_0  - local_time_ns_start);
    delay_sum_1 += (local_time_ns_end   - local_time_ns_0);
    count_stats ++;
    if (count_stats % period == 0){
        int cpu = sched_getcpu();

        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"CPU[%d],per [%llu] ave, all cost[%llu], section fill[%llu],send[%llu]\n",cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period);
        delay_sum   = 0;
        delay_sum_0 = 0;
        delay_sum_1 = 0;
    }
}

void QxShMdEtfOptionEfvi::Routine(){
    BindCPU(m_config.cpu_id);
    usleep(100000);
    std::clog <<__func__<<","<< __LINE__<<","<<g_running<< std::endl;
    while(g_running.load(std::memory_order_relaxed)) {
        MdshReceiverRun(m_efvi_recevier, OnData);
    }
}

int QxShMdEtfOptionEfvi::init(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    g_ctx = this;
    LoadJsonCfg();
    if (MdshReceiverCreate(&m_efvi_recevier, m_config.mdshcfg.c_str()) < 0){
        std::clog <<__func__<<","<< __LINE__<<",init error!" << std::endl;
        return 0;
    }

    m_worker = new std::thread(&QxShMdEtfOptionEfvi::Routine, this);   //和febao沟通，由于 on_msg的不可重入性，暂时只能有一个线程
    m_fb_spi->on_ready();

    signal(SIGINT, &on_exit);
    signal(SIGQUIT, &on_exit);

    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;
    return 0;
}

void QxShMdEtfOptionEfvi::release(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    g_running.store(false,std::memory_order_release);
    if(m_worker->joinable()){
        m_worker->join();
    }
    delete m_worker;
    m_worker = nullptr;
    MdshReceiverClose(m_efvi_recevier);
    m_efvi_recevier == nullptr;

}

void QxShMdEtfOptionEfvi::connect(){
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    g_running.store(true,std::memory_order_release);
}

void QxShMdEtfOptionEfvi::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id){
    std::clog <<__func__<<","<< __LINE__<< ","<<instrument_id<<","<< exchange_id<< std::endl;
}






