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



#include "fb_md_type.h"

#include "inst_map.h"

#include "sse_md_option.h"






// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    printf("create SSEQuota[%s]\n");
    return new SSEQuota();
}
void destroy(void *p) {
    printf("destroy SSEQuota\n");
    delete (SSEQuota*)p;
}
void get_md_api_version(char version[32]) {
    printf("[%s][%s]\n",__func__,FEBAO_MD_API_VERSION);
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif

std::atomic<bool> g_running = false;





SSEQuota::SSEQuota(): 
    m_fb_spi(nullptr),
    m_fb_md(cffex::fb::api::market_data_entity::create_entity())
{
    std::clog << std::unitbuf; //调试用

    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
}

SSEQuota::~SSEQuota(){
    std::clog << std::nounitbuf;
}

void SSEQuota::register_spi(cffex::fb::api::fb_i_md_spi *spi){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    m_fb_spi = spi;
}

int SSEQuota::init(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    cffex::fb::api::fb_md_config_helper *parser = m_fb_spi->get_config_helper();
    std::string md_jsoncfgfile;
    parser->get_attribute("file", "/md_config_path", md_jsoncfgfile);
    std::clog <<__func__<<","<< __LINE__<<",md_jsoncfgfile:"<<md_jsoncfgfile.c_str()<< std::endl;

    m_config_mgr.Load(md_jsoncfgfile);

    auto &inst_map = m_config_mgr.getInstrument();
    for (auto inst : inst_map) {
        m_quota_cache.InitOnce(inst);
        std::clog <<__func__<<","<< __LINE__<<",inst[" <<inst.c_str()<<"],cache.size:"<<m_quota_cache.size()<< std::endl;
    }

    auto &muticast = m_config_mgr.getMulticast();

    uint32_t muticast_cpu = m_config_mgr.getCpuId(0);

    std::clog <<__func__<<","<< __LINE__<<",muticast_cpu:"<<muticast.group_ip.c_str()<< std::endl;

    std::clog <<__func__<<","<< __LINE__<<",muticast_cpu:"<<muticast_cpu<< std::endl;


    m_udp_receiver.reset(new MulticastReceiver(muticast.group_ip, muticast.group_port, muticast.IfName, muticast_cpu, 1024)); //因为数据长度是135,所以buffer长度选择256
    m_udp_receiver->registerCallback((void *)this, UdpHandler);
    m_udp_receiver->start();

    m_fb_spi->on_ready();
    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;
    return 0;
}

void SSEQuota::release(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    m_udp_receiver->stop();
    g_running.store(false,std::memory_order_release);

}

void SSEQuota::connect(){
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    g_running.store(true,std::memory_order_release);
}

void SSEQuota::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id){
    std::clog <<__func__<<","<< __LINE__<< ","<<instrument_id<<","<< exchange_id<< std::endl;

    auto &inst_map = m_config_mgr.getInstrument();
    if (inst_map.count(instrument_id)==0){
        m_quota_cache.InitOnce(instrument_id);
        std::clog <<__func__<<","<< __LINE__<<",instrument_id[" <<instrument_id.c_str()<<"],m_quota_cache size:"<<m_quota_cache.size()<< std::endl;        
    }

    m_quota_cache.Sort();
}



void on_exit(int sig){
    g_running.store(false, std::memory_order_release);
}


void SSEQuota::UdpHandler(void *ctx, const char* data, size_t size,  const std::string& source_ip, uint16_t source_port, int64_t t0){
    SSEQuota *pthis = (SSEQuota *)ctx;
    if (size == sizeof(OptionSseL1)){
        pthis->onOptionSseL1((OptionSseL1*)data, t0);
    }
    else if (size == sizeof(FundSseL1)){
        pthis->onFundSseL1((FundSseL1*)data, t0);
    }
}

void SSEQuota::onFundSseL1(FundSseL1* msgdata, int64_t T_start) {
    int64_t T1 = get_nanoseconds();
    double multi = 100000.0;
    // std::string inst(msgdata->securityID);

    // if ((inst != "510500") && (inst != "510050") && (inst != "510300")){
    //     return;
    // }

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "recv [%d],inst[%s],lastUpdateTime[%llu],[%.4f],[%llu][%llu][%.4f],bp[%.4f],ap[%.4f],bv[%llu],av[%llu]\n",__LINE__,
    //                 msgdata->securityID,msgdata->lastUpdateTime,msgdata->lastPrice/multi,msgdata->securityType,msgdata->totalVolumeTraded,msgdata->totalValueTraded/100.0,
    //                 msgdata->askPriceQty[0].price/multi, msgdata->bidPriceQty[0].price/multi,msgdata->askPriceQty[0].qty,msgdata->bidPriceQty[0].qty);



    QuotaInfo *cache = m_quota_cache.Find(msgdata->securityID);
    if (cache==nullptr){
        return;
    }

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "filter recv [%d],inst[%s],lastUpdateTime[%llu],[%.4f],[%llu][%llu][%.4f],bp[%.4f],ap[%.4f],bv[%llu],av[%llu]\n",__LINE__,
    //                 msgdata->securityID,msgdata->lastUpdateTime,msgdata->lastPrice/multi,msgdata->securityType,msgdata->totalVolumeTraded,msgdata->totalValueTraded/100.0,
    //                 msgdata->askPriceQty[0].price/multi, msgdata->bidPriceQty[0].price/multi,msgdata->askPriceQty[0].qty,msgdata->bidPriceQty[0].qty);


    uint64_t sss    = msgdata->lastUpdateTime%1000L;
    uint64_t extime = (msgdata->lastUpdateTime/10000L)*3600+((msgdata->lastUpdateTime%10000L)/100)*60+(msgdata->lastUpdateTime%100L);

    int64_t T2 = get_nanoseconds();

    m_fb_md->reset_entity();// 重置飞豹行情结构
    m_fb_md->set_local_timestamp(T_start); //utc ns
    m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
    m_fb_md->set_instrument_id(msgdata->securityID);
    m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_SSE);
    m_fb_md->set_update_sec(extime);
    m_fb_md->set_update_msec(sss);
    m_fb_md->set_open(convert_price_if_zero(msgdata->openPrice/multi));
    m_fb_md->set_close(convert_price_if_zero(msgdata->closePrice/multi));

    m_fb_md->set_high_price(convert_price_if_zero(msgdata->highPrice/multi));
    m_fb_md->set_low_price(convert_price_if_zero(msgdata->lowPrice/multi));
    m_fb_md->set_last_price(convert_price_if_zero(msgdata->lastPrice/multi));
    m_fb_md->set_pre_close(convert_price_if_zero(msgdata->preClosePrice/multi));
    m_fb_md->set_pre_settlement(convert_price_if_zero(msgdata->preSettlePrice/multi));

    m_fb_md->set_volume(msgdata->totalVolumeTraded);
    m_fb_md->set_turn_over(msgdata->totalValueTraded/100.0);

    m_fb_md->set_max_depth(5);                              // 设置行情深度5
    m_fb_md->set_iopv(IOPV_TAG_LV1);

    m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->askPriceQty[0].price/multi,msgdata->askPriceQty[0].qty));
    m_fb_md->set_ask2_price(convert_price_if_volume_zero(msgdata->askPriceQty[1].price/multi,msgdata->askPriceQty[1].qty));
    m_fb_md->set_ask3_price(convert_price_if_volume_zero(msgdata->askPriceQty[2].price/multi,msgdata->askPriceQty[2].qty));
    m_fb_md->set_ask4_price(convert_price_if_volume_zero(msgdata->askPriceQty[3].price/multi,msgdata->askPriceQty[3].qty));
    m_fb_md->set_ask5_price(convert_price_if_volume_zero(msgdata->askPriceQty[4].price/multi,msgdata->askPriceQty[4].qty));
    m_fb_md->set_ask1_volume(msgdata->askPriceQty[0].qty);
    m_fb_md->set_ask2_volume(msgdata->askPriceQty[1].qty);
    m_fb_md->set_ask3_volume(msgdata->askPriceQty[2].qty);
    m_fb_md->set_ask4_volume(msgdata->askPriceQty[3].qty);
    m_fb_md->set_ask5_volume(msgdata->askPriceQty[4].qty);
    m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->bidPriceQty[0].price/multi,msgdata->bidPriceQty[0].qty));
    m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata->bidPriceQty[1].price/multi,msgdata->bidPriceQty[1].qty));
    m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata->bidPriceQty[2].price/multi,msgdata->bidPriceQty[2].qty));
    m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata->bidPriceQty[3].price/multi,msgdata->bidPriceQty[3].qty));
    m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata->bidPriceQty[4].price/multi,msgdata->bidPriceQty[4].qty));
    m_fb_md->set_bid1_volume(msgdata->bidPriceQty[0].qty);
    m_fb_md->set_bid2_volume(msgdata->bidPriceQty[1].qty);
    m_fb_md->set_bid3_volume(msgdata->bidPriceQty[2].qty);
    m_fb_md->set_bid4_volume(msgdata->bidPriceQty[3].qty);
    m_fb_md->set_bid5_volume(msgdata->bidPriceQty[4].qty);

    int64_t T3 = get_nanoseconds();

    m_fb_spi->on_msg(m_fb_md);
    int64_t T_end = get_nanoseconds();
    
    static uint64_t delay_sum = 0;
    static uint64_t delay_sum_0 = 0;
    static uint64_t delay_sum_1 = 0; 
    static uint64_t delay_sum_2 = 0; 
    static uint64_t delay_sum_3 = 0; 

    static uint64_t count_stats = 0;
    delay_sum += (T_end - T_start);
    delay_sum_0 += (T1 - T_start);
    delay_sum_1 += (T2 - T1);
    delay_sum_2 += (T3 - T2);
    delay_sum_3 += (T_end - T3);

    static uint64_t  period = 100;
    count_stats ++;
    if (count_stats%period == 0){
        int cpu = sched_getcpu();
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L1   Fund, CPU[%d],per [%llu] ave, all cost[%llu], section read[%llu],filter[%llu],fill[%llu],send[%llu]\n",
                    cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period);
        delay_sum = 0;        
        delay_sum_0 = 0;
        delay_sum_1 = 0;
        delay_sum_2 = 0;
        delay_sum_3 = 0;
    }

}


// #define LEN 409600000


// bool isPrime(uint64_t n) {
//     if (n <= 1) return false;
//     if (n == 2) return true;
//     if ((n & 1) == 0) return false;

//     for (uint64_t i = 3; i * i <= n; i += 2) {
//         if (n % i == 0) return false;
//     }
//     return true;
// }

// void test_cpu(){
//     for(uint64_t i = 0;i<LEN;++i){
//        isPrime(i);
//     }
// }


void SSEQuota::onOptionSseL1(OptionSseL1* msgdata, int64_t T_start) {



    int64_t T1 = get_nanoseconds();
    double multi = 100000.0;
    // std::string inst(msgdata->securityID);
    // if ((inst != "10009261") && (inst != "10009761")){
    //     return;
    // }

    // if ((inst != "10009261")){
    //     return;
    // }

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "recv [%d],inst[%s],extime[%llu],[%.4f],[%llu][%llu][%.4f],bp[%.4f],ap[%.4f],bv[%llu],av[%llu]\n",__LINE__,
    //                 msgdata->securityID,msgdata->lastUpdateTime,msgdata->lastPrice/multi,msgdata->totalLongPosition,msgdata->totalVolumeTraded,msgdata->totalValueTraded/100.0,
    //                 msgdata->askPriceQty[0].price/multi, msgdata->bidPriceQty[0].price/multi,msgdata->askPriceQty[0].qty,msgdata->bidPriceQty[0].qty);

    QuotaInfo *cache = m_quota_cache.Find(msgdata->securityID);
    if (cache==nullptr){
        return;
    }
    
    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "filter recv [%d],inst[%s],extime[%llu],[%.4f],[%llu][%llu][%.4f],bp[%.4f],ap[%.4f],bv[%llu],av[%llu]\n",__LINE__,
    //                 msgdata->securityID,msgdata->lastUpdateTime,msgdata->lastPrice/multi,msgdata->totalLongPosition,msgdata->totalVolumeTraded,msgdata->totalValueTraded/100.0,
    //                 msgdata->askPriceQty[0].price/multi, msgdata->bidPriceQty[0].price/multi,msgdata->askPriceQty[0].qty,msgdata->bidPriceQty[0].qty);

    uint64_t sss    = msgdata->lastUpdateTime%1000L;
    uint64_t extime = (msgdata->lastUpdateTime/10000L)*3600+((msgdata->lastUpdateTime%10000L)/100)*60+(msgdata->lastUpdateTime%100L);

    int64_t T2 = get_nanoseconds();

    m_fb_md->reset_entity();// 重置飞豹行情结构
    m_fb_md->set_local_timestamp(T_start); //utc ns
    m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
    m_fb_md->set_instrument_id(msgdata->securityID);
    m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_SSE);
    m_fb_md->set_update_sec(extime);
    m_fb_md->set_update_msec(sss);
    m_fb_md->set_open(convert_price_if_zero(msgdata->openPrice/multi));
    m_fb_md->set_close(convert_price_if_zero(msgdata->closePrice/multi));

    m_fb_md->set_high_price(convert_price_if_zero(msgdata->highPrice/multi));
    m_fb_md->set_low_price(convert_price_if_zero(msgdata->lowPrice/multi));
    m_fb_md->set_last_price(convert_price_if_zero(msgdata->lastPrice/multi));
    m_fb_md->set_pre_close(convert_price_if_zero(msgdata->preClosePrice/multi));
    m_fb_md->set_pre_settlement(convert_price_if_zero(msgdata->preSettlePrice/multi));

    m_fb_md->set_volume(msgdata->totalVolumeTraded);
    m_fb_md->set_turn_over(msgdata->totalValueTraded/100.0);
    m_fb_md->set_open_interest(msgdata->totalLongPosition);

    // m_fb_md->set_upper_limit_price(convert_price_if_zero(cache->upper));
    // m_fb_md->set_down_limit_price(convert_price_if_zero(cache->lower));


    m_fb_md->set_max_depth(5);                              // 设置行情深度5
    m_fb_md->set_iopv(IOPV_TAG_LV1);

    m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->askPriceQty[0].price/multi,msgdata->askPriceQty[0].qty));
    m_fb_md->set_ask2_price(convert_price_if_volume_zero(msgdata->askPriceQty[1].price/multi,msgdata->askPriceQty[1].qty));
    m_fb_md->set_ask3_price(convert_price_if_volume_zero(msgdata->askPriceQty[2].price/multi,msgdata->askPriceQty[2].qty));
    m_fb_md->set_ask4_price(convert_price_if_volume_zero(msgdata->askPriceQty[3].price/multi,msgdata->askPriceQty[3].qty));
    m_fb_md->set_ask5_price(convert_price_if_volume_zero(msgdata->askPriceQty[4].price/multi,msgdata->askPriceQty[4].qty));
    m_fb_md->set_ask1_volume(msgdata->askPriceQty[0].qty);
    m_fb_md->set_ask2_volume(msgdata->askPriceQty[1].qty);
    m_fb_md->set_ask3_volume(msgdata->askPriceQty[2].qty);
    m_fb_md->set_ask4_volume(msgdata->askPriceQty[3].qty);
    m_fb_md->set_ask5_volume(msgdata->askPriceQty[4].qty);
    m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->bidPriceQty[0].price/multi,msgdata->bidPriceQty[0].qty));
    m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata->bidPriceQty[1].price/multi,msgdata->bidPriceQty[1].qty));
    m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata->bidPriceQty[2].price/multi,msgdata->bidPriceQty[2].qty));
    m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata->bidPriceQty[3].price/multi,msgdata->bidPriceQty[3].qty));
    m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata->bidPriceQty[4].price/multi,msgdata->bidPriceQty[4].qty));
    m_fb_md->set_bid1_volume(msgdata->bidPriceQty[0].qty);
    m_fb_md->set_bid2_volume(msgdata->bidPriceQty[1].qty);
    m_fb_md->set_bid3_volume(msgdata->bidPriceQty[2].qty);
    m_fb_md->set_bid4_volume(msgdata->bidPriceQty[3].qty);
    m_fb_md->set_bid5_volume(msgdata->bidPriceQty[4].qty);

    int64_t T3 = get_nanoseconds();

    m_fb_spi->on_msg(m_fb_md);
    int64_t T_end = get_nanoseconds();
    // test_cpu();
    // int64_t T_test = get_nanoseconds();

    static uint64_t delay_sum = 0;
    static uint64_t delay_sum_0 = 0;
    static uint64_t delay_sum_1 = 0;
    static uint64_t delay_sum_2 = 0;
    static uint64_t delay_sum_3 = 0;

    // static uint64_t delay_sum_a = 0;



    static uint64_t count_stats = 0;
    delay_sum += (T_end - T_start);
    delay_sum_0 += (T1 - T_start);
    delay_sum_1 += (T2 - T1);
    delay_sum_2 += (T3 - T2);
    delay_sum_3 += (T_end - T3);


    // delay_sum_a += (T_test  - T_end);


    static uint64_t  period = 500;
    count_stats ++;
    if (count_stats%period == 0){
        int cpu = sched_getcpu();
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L1 Option, CPU[%d],per [%llu] ave, all cost[%llu], section read[%llu],filter[%llu],fill[%llu],send[%llu]\n",
                    cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period);
        delay_sum = 0;        
        delay_sum_0 = 0;
        delay_sum_1 = 0;
        delay_sum_2 = 0;
        delay_sum_3 = 0;

        // delay_sum_a = 0;

    }

};

