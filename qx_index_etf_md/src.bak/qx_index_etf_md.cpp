#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <signal.h>

#include "qx_index_etf_md.h"

#include "tbj_gtjamcb.h"


#include "SockProto.h"

#include <unordered_map>
#include <functional>
#include <cmath>

#include "fb_md_type.h"

#include "tszch_gtjamch.h"


// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    printf("create QxIndexEtfMd[%s]\n");
    return new QxIndexEtfMd();
}
void destroy(void *p) {
    printf("destroy QxIndexEtfMd\n");
    delete (QxIndexEtfMd*)p;
}
void get_md_api_version(char version[32]) {
    printf("[%s][%s]\n",__func__,FEBAO_MD_API_VERSION);
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif




std::atomic<bool> g_running = false;


alignas(64) Element             g_quota_queue[QUEUE_MAX_LEN];  //只存储过滤的的行情，新增行情的时候要扩容，单生产者，单消费者
alignas(64) std::atomic<size_t> g_write_pos = 0;
alignas(64) std::atomic<size_t> g_read_pos = 0;


inline bool QxIndexEtfMd::belongTo(std::string &section){
    for(auto & it : m_config.multicast){
        if(it.section == section){
            return true;
        }
    }
    return false;
}

inline bool QxIndexEtfMd::belongTo(uint32_t ip, uint16_t port){
    for(auto & it : m_config.multicast){
        if((it.ip == ip) && (it.port == port)){
            return true;
        }
    }
    return false;
}



void handle_shse_snapshot(QxIndexEtfMd *pthis, Element &elem, fb_market_data_t *targetdata){
    Lev2WholeData *msgdata = (Lev2WholeData *)(elem.data);
    const double SHSE_UINT = 1000.0;

    targetdata->set_update_sec(hhmmssToUtcSeconds(msgdata->time_stamp));
    targetdata->set_update_msec(0); //这个是交易所的时间，只到秒
    targetdata->set_max_depth(10);                             // 设置行情深度5
    targetdata->set_instrument_id(msgdata->security_id);
    targetdata->set_exchange_id(cffex::fb::api::FB_EXCHANGE_SHFE);
    targetdata->set_pre_close(msgdata->pre_close_price/SHSE_UINT);
    targetdata->set_open(convert_price_if_zero(msgdata->open_price/SHSE_UINT));
    targetdata->set_close(convert_price_if_zero(msgdata->match_price/SHSE_UINT));
    targetdata->set_upper_limit_price(msgdata->high_limit_price/10000.0);
    targetdata->set_down_limit_price(msgdata->low_limit_price/10000.0);
    targetdata->set_high_price(convert_price_if_zero(msgdata->high_price/SHSE_UINT));
    targetdata->set_low_price(convert_price_if_zero(msgdata->low_price/SHSE_UINT));
    targetdata->set_last_price(convert_price_if_zero(msgdata->match_price/SHSE_UINT));
    targetdata->set_volume(msgdata->total_volume_trade/SHSE_UINT);
    targetdata->set_turn_over(msgdata->total_value_trade/100000.0);
    targetdata->set_ask1_price(convert_price_if_volume_zero(msgdata->offer_price[0]/SHSE_UINT,msgdata->offer_qty[0]/SHSE_UINT));
    targetdata->set_ask2_price(convert_price_if_volume_zero(msgdata->offer_price[1]/SHSE_UINT,msgdata->offer_qty[1]/SHSE_UINT));
    targetdata->set_ask3_price(convert_price_if_volume_zero(msgdata->offer_price[2]/SHSE_UINT,msgdata->offer_qty[2]/SHSE_UINT));
    targetdata->set_ask4_price(convert_price_if_volume_zero(msgdata->offer_price[3]/SHSE_UINT,msgdata->offer_qty[3]/SHSE_UINT));
    targetdata->set_ask5_price(convert_price_if_volume_zero(msgdata->offer_price[4]/SHSE_UINT,msgdata->offer_qty[4]/SHSE_UINT));
    targetdata->set_ask6_price(convert_price_if_volume_zero(msgdata->offer_price[5]/SHSE_UINT,msgdata->offer_qty[5]/SHSE_UINT));
    targetdata->set_ask7_price(convert_price_if_volume_zero(msgdata->offer_price[6]/SHSE_UINT,msgdata->offer_qty[6]/SHSE_UINT));
    targetdata->set_ask8_price(convert_price_if_volume_zero(msgdata->offer_price[7]/SHSE_UINT,msgdata->offer_qty[7]/SHSE_UINT));
    targetdata->set_ask9_price(convert_price_if_volume_zero(msgdata->offer_price[8]/SHSE_UINT,msgdata->offer_qty[8]/SHSE_UINT));
    targetdata->set_ask10_price(convert_price_if_volume_zero(msgdata->offer_price[9]/SHSE_UINT,msgdata->offer_qty[9]/SHSE_UINT));
    targetdata->set_ask1_volume(msgdata->offer_qty[0]/SHSE_UINT);
    targetdata->set_ask2_volume(msgdata->offer_qty[1]/SHSE_UINT);
    targetdata->set_ask3_volume(msgdata->offer_qty[2]/SHSE_UINT);
    targetdata->set_ask4_volume(msgdata->offer_qty[3]/SHSE_UINT);
    targetdata->set_ask5_volume(msgdata->offer_qty[4]/SHSE_UINT);
    targetdata->set_ask6_volume(msgdata->offer_qty[5]/SHSE_UINT);
    targetdata->set_ask7_volume(msgdata->offer_qty[6]/SHSE_UINT);
    targetdata->set_ask8_volume(msgdata->offer_qty[7]/SHSE_UINT);
    targetdata->set_ask9_volume(msgdata->offer_qty[8]/SHSE_UINT);
    targetdata->set_ask10_volume(msgdata->offer_qty[9]/SHSE_UINT);
    targetdata->set_bid1_price(convert_price_if_volume_zero(msgdata->bid_price[0]/SHSE_UINT,msgdata->bid_qty[0]/SHSE_UINT));
    targetdata->set_bid2_price(convert_price_if_volume_zero(msgdata->bid_price[1]/SHSE_UINT,msgdata->bid_qty[1]/SHSE_UINT));
    targetdata->set_bid3_price(convert_price_if_volume_zero(msgdata->bid_price[2]/SHSE_UINT,msgdata->bid_qty[2]/SHSE_UINT));
    targetdata->set_bid4_price(convert_price_if_volume_zero(msgdata->bid_price[3]/SHSE_UINT,msgdata->bid_qty[3]/SHSE_UINT));
    targetdata->set_bid5_price(convert_price_if_volume_zero(msgdata->bid_price[4]/SHSE_UINT,msgdata->bid_qty[4]/SHSE_UINT));
    targetdata->set_bid6_price(convert_price_if_volume_zero(msgdata->bid_price[5]/SHSE_UINT,msgdata->bid_qty[5]/SHSE_UINT));
    targetdata->set_bid7_price(convert_price_if_volume_zero(msgdata->bid_price[6]/SHSE_UINT,msgdata->bid_qty[6]/SHSE_UINT));
    targetdata->set_bid8_price(convert_price_if_volume_zero(msgdata->bid_price[7]/SHSE_UINT,msgdata->bid_qty[7]/SHSE_UINT));
    targetdata->set_bid9_price(convert_price_if_volume_zero(msgdata->bid_price[8]/SHSE_UINT,msgdata->bid_qty[8]/SHSE_UINT));
    targetdata->set_bid10_price(convert_price_if_volume_zero(msgdata->bid_price[9]/SHSE_UINT,msgdata->bid_qty[9]/SHSE_UINT));
    targetdata->set_bid1_volume(msgdata->bid_qty[0]/SHSE_UINT);
    targetdata->set_bid2_volume(msgdata->bid_qty[1]/SHSE_UINT);
    targetdata->set_bid3_volume(msgdata->bid_qty[2]/SHSE_UINT);
    targetdata->set_bid4_volume(msgdata->bid_qty[3]/SHSE_UINT);
    targetdata->set_bid5_volume(msgdata->bid_qty[4]/SHSE_UINT);
    targetdata->set_bid6_volume(msgdata->bid_qty[5]/SHSE_UINT);
    targetdata->set_bid7_volume(msgdata->bid_qty[6]/SHSE_UINT);
    targetdata->set_bid8_volume(msgdata->bid_qty[7]/SHSE_UINT);
    targetdata->set_bid9_volume(msgdata->bid_qty[8]/SHSE_UINT);
    targetdata->set_bid10_volume(msgdata->bid_qty[9]/SHSE_UINT);

}



void handle_shse_option_snapshot(QxIndexEtfMd *pthis, Element &elem, fb_market_data_t *targetdata){
    Lev1Option *msgdata = (Lev1Option *)(elem.data);
    const double SHSE_UINT = 100000.0;

    targetdata->set_update_sec(hhmmssToUtcSeconds(msgdata->data_timestamp));
    targetdata->set_update_msec(msgdata->data_timestamp/1000); 
    targetdata->set_max_depth(5);                              // 设置行情深度5
    targetdata->set_instrument_id(msgdata->security_id);
    targetdata->set_exchange_id(cffex::fb::api::FB_EXCHANGE_SHFE);
    targetdata->set_pre_settlement(msgdata->pre_settle_price/SHSE_UINT);
    targetdata->set_open(convert_price_if_zero(msgdata->open_price/SHSE_UINT));
    targetdata->set_close(convert_price_if_zero(msgdata->last_price/SHSE_UINT));
    targetdata->set_high_price(convert_price_if_zero(msgdata->high_price/SHSE_UINT));
    targetdata->set_low_price(convert_price_if_zero(msgdata->low_price/SHSE_UINT));
    targetdata->set_last_price(convert_price_if_zero(msgdata->last_price/SHSE_UINT));
    targetdata->set_volume(msgdata->total_volume_trade);
    targetdata->set_turn_over(msgdata->total_value_trade/100.0);
    targetdata->set_open_interest(msgdata->total_long_position);
    targetdata->set_ask1_price(convert_price_if_volume_zero(msgdata->offer_price[0]/SHSE_UINT,msgdata->offer_qty[0]));
    targetdata->set_ask2_price(convert_price_if_volume_zero(msgdata->offer_price[1]/SHSE_UINT,msgdata->offer_qty[1]));
    targetdata->set_ask3_price(convert_price_if_volume_zero(msgdata->offer_price[2]/SHSE_UINT,msgdata->offer_qty[2]));
    targetdata->set_ask4_price(convert_price_if_volume_zero(msgdata->offer_price[3]/SHSE_UINT,msgdata->offer_qty[3]));
    targetdata->set_ask5_price(convert_price_if_volume_zero(msgdata->offer_price[4]/SHSE_UINT,msgdata->offer_qty[4]));
    targetdata->set_ask1_volume(msgdata->offer_qty[0]);
    targetdata->set_ask2_volume(msgdata->offer_qty[1]);
    targetdata->set_ask3_volume(msgdata->offer_qty[2]);
    targetdata->set_ask4_volume(msgdata->offer_qty[3]);
    targetdata->set_ask5_volume(msgdata->offer_qty[4]);
    targetdata->set_bid1_price(convert_price_if_volume_zero(msgdata->bid_price[0]/SHSE_UINT,msgdata->bid_qty[0]));
    targetdata->set_bid2_price(convert_price_if_volume_zero(msgdata->bid_price[1]/SHSE_UINT,msgdata->bid_qty[1]));
    targetdata->set_bid3_price(convert_price_if_volume_zero(msgdata->bid_price[2]/SHSE_UINT,msgdata->bid_qty[2]));
    targetdata->set_bid4_price(convert_price_if_volume_zero(msgdata->bid_price[3]/SHSE_UINT,msgdata->bid_qty[3]));
    targetdata->set_bid5_price(convert_price_if_volume_zero(msgdata->bid_price[4]/SHSE_UINT,msgdata->bid_qty[4]));
    targetdata->set_bid1_volume(msgdata->bid_qty[0]);
    targetdata->set_bid2_volume(msgdata->bid_qty[1]);
    targetdata->set_bid3_volume(msgdata->bid_qty[2]);
    targetdata->set_bid4_volume(msgdata->bid_qty[3]);
    targetdata->set_bid5_volume(msgdata->bid_qty[4]);
}

void handle_szse_snapshot(QxIndexEtfMd *pthis, Element &elem, fb_market_data_t *targetdata){
    StaticAccMD *pappData = (StaticAccMD *)elem.data;
    MDSnapShotL2 *msgdata = (MDSnapShotL2 *)pappData->data;
    const double PRICE_UINT = 1000000.0;

    targetdata->set_update_sec(yyyymmddhhmmssToUtcSeconds(msgdata->md_time/1000));
    targetdata->set_update_msec(msgdata->md_time%1000);

    targetdata->set_max_depth(10);                              // 设置行情深度10
    targetdata->set_instrument_id(msgdata->security_id);
    targetdata->set_exchange_id(cffex::fb::api::FB_EXCHANGE_SZSE);

    targetdata->set_pre_close(msgdata->pre_close/10000.0);
    targetdata->set_open(convert_price_if_zero(msgdata->open/PRICE_UINT));
    targetdata->set_close(convert_price_if_zero(msgdata->match/PRICE_UINT));
    targetdata->set_upper_limit_price(msgdata->upper_limit/PRICE_UINT);
    targetdata->set_down_limit_price(msgdata->lower_limit/PRICE_UINT);
    targetdata->set_high_price(convert_price_if_zero(msgdata->high/PRICE_UINT));
    targetdata->set_low_price(convert_price_if_zero(msgdata->low/PRICE_UINT));
    targetdata->set_last_price(convert_price_if_zero(msgdata->match/PRICE_UINT));
    targetdata->set_volume(msgdata->total_volume/100);
    targetdata->set_turn_over(msgdata->total_value/10000.0);
    targetdata->set_ask1_price(convert_price_if_volume_zero(msgdata->ask_price[0]/PRICE_UINT,msgdata->ask_vol[0]/100));
    targetdata->set_ask2_price(convert_price_if_volume_zero(msgdata->ask_price[1]/PRICE_UINT,msgdata->ask_vol[1]/100));
    targetdata->set_ask3_price(convert_price_if_volume_zero(msgdata->ask_price[2]/PRICE_UINT,msgdata->ask_vol[2]/100));
    targetdata->set_ask4_price(convert_price_if_volume_zero(msgdata->ask_price[3]/PRICE_UINT,msgdata->ask_vol[3]/100));
    targetdata->set_ask5_price(convert_price_if_volume_zero(msgdata->ask_price[4]/PRICE_UINT,msgdata->ask_vol[4]/100));
    targetdata->set_ask6_price(convert_price_if_volume_zero(msgdata->ask_price[5]/PRICE_UINT,msgdata->ask_vol[5]/100));
    targetdata->set_ask7_price(convert_price_if_volume_zero(msgdata->ask_price[6]/PRICE_UINT,msgdata->ask_vol[6]/100));
    targetdata->set_ask8_price(convert_price_if_volume_zero(msgdata->ask_price[7]/PRICE_UINT,msgdata->ask_vol[7]/100));
    targetdata->set_ask9_price(convert_price_if_volume_zero(msgdata->ask_price[8]/PRICE_UINT,msgdata->ask_vol[8]/100));
    targetdata->set_ask10_price(convert_price_if_volume_zero(msgdata->ask_price[9]/PRICE_UINT,msgdata->ask_vol[9]/100));
    targetdata->set_ask1_volume(msgdata->ask_vol[0]/100);
    targetdata->set_ask2_volume(msgdata->ask_vol[1]/100);
    targetdata->set_ask3_volume(msgdata->ask_vol[2]/100);
    targetdata->set_ask4_volume(msgdata->ask_vol[3]/100);
    targetdata->set_ask5_volume(msgdata->ask_vol[4]/100);
    targetdata->set_ask6_volume(msgdata->ask_vol[5]/100);
    targetdata->set_ask7_volume(msgdata->ask_vol[6]/100);
    targetdata->set_ask8_volume(msgdata->ask_vol[7]/100);
    targetdata->set_ask9_volume(msgdata->ask_vol[8]/100);
    targetdata->set_ask10_volume(msgdata->ask_vol[9]/100);
    targetdata->set_bid1_price(convert_price_if_volume_zero(msgdata->bid_price[0]/PRICE_UINT,msgdata->bid_vol[0]/100));
    targetdata->set_bid2_price(convert_price_if_volume_zero(msgdata->bid_price[1]/PRICE_UINT,msgdata->bid_vol[1]/100));
    targetdata->set_bid3_price(convert_price_if_volume_zero(msgdata->bid_price[2]/PRICE_UINT,msgdata->bid_vol[2]/100));
    targetdata->set_bid4_price(convert_price_if_volume_zero(msgdata->bid_price[3]/PRICE_UINT,msgdata->bid_vol[3]/100));
    targetdata->set_bid5_price(convert_price_if_volume_zero(msgdata->bid_price[4]/PRICE_UINT,msgdata->bid_vol[4]/100));
    targetdata->set_bid6_price(convert_price_if_volume_zero(msgdata->bid_price[6]/PRICE_UINT,msgdata->bid_vol[5]/100));
    targetdata->set_bid7_price(convert_price_if_volume_zero(msgdata->bid_price[7]/PRICE_UINT,msgdata->bid_vol[6]/100));
    targetdata->set_bid8_price(convert_price_if_volume_zero(msgdata->bid_price[8]/PRICE_UINT,msgdata->bid_vol[7]/100));
    targetdata->set_bid9_price(convert_price_if_volume_zero(msgdata->bid_price[9]/PRICE_UINT,msgdata->bid_vol[8]/100));
    targetdata->set_bid10_price(convert_price_if_volume_zero(msgdata->bid_price[10]/PRICE_UINT,msgdata->bid_vol[9]/100));
    targetdata->set_bid1_volume(msgdata->bid_vol[0]/100);
    targetdata->set_bid2_volume(msgdata->bid_vol[1]/100);
    targetdata->set_bid3_volume(msgdata->bid_vol[2]/100);
    targetdata->set_bid4_volume(msgdata->bid_vol[3]/100);
    targetdata->set_bid5_volume(msgdata->bid_vol[4]/100);
    targetdata->set_bid6_volume(msgdata->bid_vol[5]/100);
    targetdata->set_bid7_volume(msgdata->bid_vol[6]/100);
    targetdata->set_bid8_volume(msgdata->bid_vol[7]/100);
    targetdata->set_bid9_volume(msgdata->bid_vol[8]/100);
    targetdata->set_bid10_volume(msgdata->bid_vol[9]/100);
}


void handle_szse_option_snapshot(QxIndexEtfMd *pthis, Element &elem, fb_market_data_t *targetdata){
    StaticAccMD *pappData = (StaticAccMD *)elem.data;
    OptionSnapShotL1 *msgdata = (OptionSnapShotL1 *)pappData->data;

    const double PRICE_UINT = 1000000.0;

    targetdata->set_update_sec(yyyymmddhhmmssToUtcSeconds(msgdata->md_time/1000));
    targetdata->set_update_msec(msgdata->md_time%1000);
    targetdata->set_max_depth(5);                              // 设置行情深度5
    targetdata->set_instrument_id(msgdata->security_id);
    targetdata->set_exchange_id(cffex::fb::api::FB_EXCHANGE_SZSE);
    targetdata->set_pre_close(convert_price_if_zero(msgdata->pre_close/10000.0));
    targetdata->set_open(convert_price_if_zero(msgdata->open/PRICE_UINT));
    targetdata->set_close(convert_price_if_zero(msgdata->match/PRICE_UINT));
    targetdata->set_upper_limit_price(msgdata->upper_limit/PRICE_UINT);
    targetdata->set_down_limit_price(msgdata->lower_limit/PRICE_UINT);
    targetdata->set_high_price(convert_price_if_zero(msgdata->high/PRICE_UINT));
    targetdata->set_low_price(convert_price_if_zero(msgdata->low/PRICE_UINT));
    targetdata->set_last_price(convert_price_if_zero(msgdata->match/PRICE_UINT));
    targetdata->set_volume(msgdata->total_volume/100.0);
    targetdata->set_turn_over(msgdata->total_value/10000.0);
    targetdata->set_open_interest(msgdata->position_volume/100.0);
    targetdata->set_ask1_price(convert_price_if_volume_zero(msgdata->ask_price[0]/PRICE_UINT,msgdata->ask_vol[0]/100.0));
    targetdata->set_ask2_price(convert_price_if_volume_zero(msgdata->ask_price[1]/PRICE_UINT,msgdata->ask_vol[1]/100.0));
    targetdata->set_ask3_price(convert_price_if_volume_zero(msgdata->ask_price[2]/PRICE_UINT,msgdata->ask_vol[2]/100.0));
    targetdata->set_ask4_price(convert_price_if_volume_zero(msgdata->ask_price[3]/PRICE_UINT,msgdata->ask_vol[3]/100.0));
    targetdata->set_ask5_price(convert_price_if_volume_zero(msgdata->ask_price[4]/PRICE_UINT,msgdata->ask_vol[4]/100.0));
    targetdata->set_ask1_volume(msgdata->ask_vol[0]/100.0);
    targetdata->set_ask2_volume(msgdata->ask_vol[1]/100.0);
    targetdata->set_ask3_volume(msgdata->ask_vol[2]/100.0);
    targetdata->set_ask4_volume(msgdata->ask_vol[3]/100.0);
    targetdata->set_ask5_volume(msgdata->ask_vol[4]/100.0);
    targetdata->set_bid1_price(convert_price_if_volume_zero(msgdata->bid_price[0]/PRICE_UINT,msgdata->bid_vol[0]/100.0));
    targetdata->set_bid2_price(convert_price_if_volume_zero(msgdata->bid_price[1]/PRICE_UINT,msgdata->bid_vol[1]/100.0));
    targetdata->set_bid3_price(convert_price_if_volume_zero(msgdata->bid_price[2]/PRICE_UINT,msgdata->bid_vol[2]/100.0));
    targetdata->set_bid4_price(convert_price_if_volume_zero(msgdata->bid_price[3]/PRICE_UINT,msgdata->bid_vol[3]/100.0));
    targetdata->set_bid5_price(convert_price_if_volume_zero(msgdata->bid_price[4]/PRICE_UINT,msgdata->bid_vol[4]/100.0));
    targetdata->set_bid1_volume(msgdata->bid_vol[0]/100.0);
    targetdata->set_bid2_volume(msgdata->bid_vol[1]/100.0);
    targetdata->set_bid3_volume(msgdata->bid_vol[2]/100.0);
    targetdata->set_bid4_volume(msgdata->bid_vol[3]/100.0);
    targetdata->set_bid5_volume(msgdata->bid_vol[4]/100.0);
}


void handle_szse_snapshot_ms(QxIndexEtfMd *pthis,Element &elem, fb_market_data_t *targetdata){
    StaticAccMD *pappData = (StaticAccMD *)elem.data;
    t_SZ_MCBMarketDataLF *msgdata = (t_SZ_MCBMarketDataLF *)pappData->data;

    const double PRICE_UINT = 10000.0;

    //std::clog <<__func__<<","<< __LINE__<<",used["<< elem.used<<"]" <<","<<msgdata->security_id<< std::endl;
    //pthis->m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],msg_type[0x%X],used[%d],timestamp[%llu],[%llu][%llu]\n",__LINE__,elem.msg_type,elem.used,elem.timestamp,g_read_pos.load(std::memory_order_acquire),g_write_pos.load(std::memory_order_relaxed));


    // std::clog <<__func__<<","<< __LINE__<<",nTime["<< msgdata->nTime<<"]"<< std::endl;
    // std::clog <<__func__<<","<< __LINE__<<",update_sec["<< hhmmssToUtcSeconds(msgdata->nTime/1000)<<"]"<< std::endl;
    // std::clog <<__func__<<","<< __LINE__<<",update_msec["<< (msgdata->nTime%1000)<<"]"<< std::endl;
    // std::clog <<__func__<<","<< __LINE__<<",instrument_id["<< msgdata->security_id<<"]"<< std::endl;
    // std::clog <<__func__<<","<< __LINE__<<",pre_close["<< convert_price_if_zero(msgdata->uPreClose/PRICE_UINT)<<"]"<< std::endl;
    // std::clog <<__func__<<","<< __LINE__<<",open["<< convert_price_if_zero(msgdata->uOpen/PRICE_UINT)<<"]"<< std::endl;
    // std::clog <<__func__<<","<< __LINE__<<",close["<< convert_price_if_zero(msgdata->uMatch/PRICE_UINT)<<"]"<< std::endl;
    // std::clog <<__func__<<","<< __LINE__<<",high["<< convert_price_if_zero(msgdata->uHigh/PRICE_UINT)<<"]"<< std::endl;
    // std::clog <<__func__<<","<< __LINE__<<",low["<< convert_price_if_zero(msgdata->uLow/PRICE_UINT)<<"]"<< std::endl;
    // std::clog <<__func__<<","<< __LINE__<<",last_price["<< convert_price_if_zero(msgdata->uMatch/PRICE_UINT)<<"]"<< std::endl;
    // std::clog <<__func__<<","<< __LINE__<<",volume["<< msgdata->iVolume<<"]"<< std::endl;
    // std::clog <<__func__<<","<< __LINE__<<",turn_over["<< msgdata->iTurnover/PRICE_UINT<<"]"<< std::endl;






    double midprice = msgdata->uMatch;
    if(msgdata->uAskPrice[0] && msgdata->uBidPrice[0]){
        midprice = (msgdata->uAskPrice[0]+msgdata->uBidPrice[0])/(2.0*PRICE_UINT);
    }

    //TODO 涨跌停的处理



    // pthis->m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],code[%s],time[%d],utc_time[%d]\n",__LINE__,msgdata->security_id,msgdata->nTime,hhmmssToUtcSeconds(msgdata->nTime/1000));

    // pthis->m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],PreClose[%.4f],Open[%.4f],midprice[%.4f],High[%.4f],Low[%.4f],Match[%.4f],Volume[%llu],iTurnover[%llu]\n",__LINE__,
    //                                     convert_price_if_zero(msgdata->uPreClose/PRICE_UINT),
    //                                     convert_price_if_zero(msgdata->uOpen/PRICE_UINT),
    //                                     midprice,
    //                                     convert_price_if_zero(msgdata->uHigh/PRICE_UINT),
    //                                     convert_price_if_zero(msgdata->uLow/PRICE_UINT),
    //                                     convert_price_if_zero(msgdata->uMatch/PRICE_UINT),
    //                                     msgdata->iVolume,
    //                                     msgdata->iTurnover/PRICE_UINT);

    // pthis->m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],ap1[%.4f],ap2[%.4f],ap3[%.4f],ap4[%.4f],ap5[%.4f],ap6[%.4f],ap7[%.4f],ap8[%.4f],ap9[%.4f],ap10[%.4f]\n",__LINE__,
    //                                                             convert_price_if_volume_zero(msgdata->uAskPrice[0]/PRICE_UINT,msgdata->uAskVol[0]),
    //                                                             convert_price_if_volume_zero(msgdata->uAskPrice[1]/PRICE_UINT,msgdata->uAskVol[1]),
    //                                                             convert_price_if_volume_zero(msgdata->uAskPrice[2]/PRICE_UINT,msgdata->uAskVol[2]),
    //                                                             convert_price_if_volume_zero(msgdata->uAskPrice[3]/PRICE_UINT,msgdata->uAskVol[3]),
    //                                                             convert_price_if_volume_zero(msgdata->uAskPrice[4]/PRICE_UINT,msgdata->uAskVol[4]),
    //                                                             convert_price_if_volume_zero(msgdata->uAskPrice[5]/PRICE_UINT,msgdata->uAskVol[5]),
    //                                                             convert_price_if_volume_zero(msgdata->uAskPrice[6]/PRICE_UINT,msgdata->uAskVol[6]),
    //                                                             convert_price_if_volume_zero(msgdata->uAskPrice[7]/PRICE_UINT,msgdata->uAskVol[7]),
    //                                                             convert_price_if_volume_zero(msgdata->uAskPrice[8]/PRICE_UINT,msgdata->uAskVol[8]),
    //                                                             convert_price_if_volume_zero(msgdata->uAskPrice[9]/PRICE_UINT,msgdata->uAskVol[9]));

    // pthis->m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],av1[%d],av2[%d],av3[%d],av4[%d],av5[%d],av6[%d],av7[%d],av8[%d],av9[%d],av10[%d]\n",__LINE__,
    // msgdata->uAskVol[0],msgdata->uAskVol[1],msgdata->uAskVol[2],msgdata->uAskVol[3],msgdata->uAskVol[4],msgdata->uAskVol[5],msgdata->uAskVol[6],msgdata->uAskVol[7],msgdata->uAskVol[8],msgdata->uAskVol[9]);

    // pthis->m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],ap1[%.4f],ap2[%.4f],ap3[%.4f],ap4[%.4f],ap5[%.4f],ap6[%.4f],ap7[%.4f],ap8[%.4f],ap9[%.4f],ap10[%.4f]\n",__LINE__,
    //                                                             convert_price_if_volume_zero(msgdata->uBidPrice[0]/PRICE_UINT,msgdata->uBidVol[0]),
    //                                                             convert_price_if_volume_zero(msgdata->uBidPrice[1]/PRICE_UINT,msgdata->uBidVol[1]),
    //                                                             convert_price_if_volume_zero(msgdata->uBidPrice[2]/PRICE_UINT,msgdata->uBidVol[2]),
    //                                                             convert_price_if_volume_zero(msgdata->uBidPrice[3]/PRICE_UINT,msgdata->uBidVol[3]),
    //                                                             convert_price_if_volume_zero(msgdata->uBidPrice[4]/PRICE_UINT,msgdata->uBidVol[4]),
    //                                                             convert_price_if_volume_zero(msgdata->uBidPrice[5]/PRICE_UINT,msgdata->uBidVol[5]),
    //                                                             convert_price_if_volume_zero(msgdata->uBidPrice[6]/PRICE_UINT,msgdata->uBidVol[6]),
    //                                                             convert_price_if_volume_zero(msgdata->uBidPrice[7]/PRICE_UINT,msgdata->uBidVol[7]),
    //                                                             convert_price_if_volume_zero(msgdata->uBidPrice[8]/PRICE_UINT,msgdata->uBidVol[8]),
    //                                                             convert_price_if_volume_zero(msgdata->uBidPrice[9]/PRICE_UINT,msgdata->uBidVol[9]));

    // pthis->m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],bv1[%d],bv2[%d],bv3[%d],bv4[%d],bv5[%d],bv6[%d],bv7[%d],bv8[%d],bv9[%d],bv10[%d]\n",__LINE__,
    // msgdata->uBidVol[0],msgdata->uBidVol[1],msgdata->uBidVol[2],msgdata->uBidVol[3],msgdata->uBidVol[4],msgdata->uBidVol[5],msgdata->uBidVol[6],msgdata->uBidVol[7],msgdata->uBidVol[8],msgdata->uBidVol[9]);

    targetdata->set_update_sec(hhmmssToUtcSeconds(msgdata->nTime/1000));
    targetdata->set_update_msec(msgdata->nTime%1000);
    targetdata->set_max_depth(10);                              // 设置行情深度5
    targetdata->set_instrument_id(msgdata->security_id);
    targetdata->set_exchange_id(cffex::fb::api::FB_EXCHANGE_SZSE);
    targetdata->set_pre_close(convert_price_if_zero(msgdata->uPreClose/PRICE_UINT));
    targetdata->set_open(convert_price_if_zero(msgdata->uOpen/PRICE_UINT));
    targetdata->set_close(midprice);
    targetdata->set_high_price(convert_price_if_zero(msgdata->uHigh/PRICE_UINT));
    targetdata->set_low_price(convert_price_if_zero(msgdata->uLow/PRICE_UINT));
    targetdata->set_last_price(convert_price_if_zero(msgdata->uMatch/PRICE_UINT));
    targetdata->set_volume(msgdata->iVolume);
    targetdata->set_turn_over(msgdata->iTurnover/PRICE_UINT);
    targetdata->set_ask1_price(convert_price_if_volume_zero(msgdata->uAskPrice[0]/PRICE_UINT,msgdata->uAskVol[0]));
    targetdata->set_ask2_price(convert_price_if_volume_zero(msgdata->uAskPrice[1]/PRICE_UINT,msgdata->uAskVol[1]));
    targetdata->set_ask3_price(convert_price_if_volume_zero(msgdata->uAskPrice[2]/PRICE_UINT,msgdata->uAskVol[2]));
    targetdata->set_ask4_price(convert_price_if_volume_zero(msgdata->uAskPrice[3]/PRICE_UINT,msgdata->uAskVol[3]));
    targetdata->set_ask5_price(convert_price_if_volume_zero(msgdata->uAskPrice[4]/PRICE_UINT,msgdata->uAskVol[4]));
    targetdata->set_ask6_price(convert_price_if_volume_zero(msgdata->uAskPrice[5]/PRICE_UINT,msgdata->uAskVol[5]));
    targetdata->set_ask7_price(convert_price_if_volume_zero(msgdata->uAskPrice[6]/PRICE_UINT,msgdata->uAskVol[6]));
    targetdata->set_ask8_price(convert_price_if_volume_zero(msgdata->uAskPrice[7]/PRICE_UINT,msgdata->uAskVol[7]));
    targetdata->set_ask9_price(convert_price_if_volume_zero(msgdata->uAskPrice[8]/PRICE_UINT,msgdata->uAskVol[8]));
    targetdata->set_ask10_price(convert_price_if_volume_zero(msgdata->uAskPrice[9]/PRICE_UINT,msgdata->uAskVol[9]));
    targetdata->set_ask1_volume(msgdata->uAskVol[0]);
    targetdata->set_ask2_volume(msgdata->uAskVol[1]);
    targetdata->set_ask3_volume(msgdata->uAskVol[2]);
    targetdata->set_ask4_volume(msgdata->uAskVol[3]);
    targetdata->set_ask5_volume(msgdata->uAskVol[4]);
    targetdata->set_ask6_volume(msgdata->uAskVol[5]);
    targetdata->set_ask7_volume(msgdata->uAskVol[6]);
    targetdata->set_ask8_volume(msgdata->uAskVol[7]);
    targetdata->set_ask9_volume(msgdata->uAskVol[8]);
    targetdata->set_ask10_volume(msgdata->uAskVol[9]);
    targetdata->set_bid1_price(convert_price_if_volume_zero(msgdata->uBidPrice[0]/PRICE_UINT,msgdata->uBidVol[0]));
    targetdata->set_bid2_price(convert_price_if_volume_zero(msgdata->uBidPrice[1]/PRICE_UINT,msgdata->uBidVol[1]));
    targetdata->set_bid3_price(convert_price_if_volume_zero(msgdata->uBidPrice[2]/PRICE_UINT,msgdata->uBidVol[2]));
    targetdata->set_bid4_price(convert_price_if_volume_zero(msgdata->uBidPrice[3]/PRICE_UINT,msgdata->uBidVol[3]));
    targetdata->set_bid5_price(convert_price_if_volume_zero(msgdata->uBidPrice[4]/PRICE_UINT,msgdata->uBidVol[4]));
    targetdata->set_bid6_price(convert_price_if_volume_zero(msgdata->uBidPrice[5]/PRICE_UINT,msgdata->uBidVol[5]));
    targetdata->set_bid7_price(convert_price_if_volume_zero(msgdata->uBidPrice[6]/PRICE_UINT,msgdata->uBidVol[6]));
    targetdata->set_bid8_price(convert_price_if_volume_zero(msgdata->uBidPrice[7]/PRICE_UINT,msgdata->uBidVol[7]));
    targetdata->set_bid9_price(convert_price_if_volume_zero(msgdata->uBidPrice[8]/PRICE_UINT,msgdata->uBidVol[8]));
    targetdata->set_bid10_price(convert_price_if_volume_zero(msgdata->uBidPrice[9]/PRICE_UINT,msgdata->uBidVol[9]));
    targetdata->set_bid1_volume(msgdata->uBidVol[0]);
    targetdata->set_bid2_volume(msgdata->uBidVol[1]);
    targetdata->set_bid3_volume(msgdata->uBidVol[2]);
    targetdata->set_bid4_volume(msgdata->uBidVol[3]);
    targetdata->set_bid5_volume(msgdata->uBidVol[4]);
    targetdata->set_bid6_volume(msgdata->uBidVol[5]);
    targetdata->set_bid7_volume(msgdata->uBidVol[6]);
    targetdata->set_bid8_volume(msgdata->uBidVol[7]);
    targetdata->set_bid9_volume(msgdata->uBidVol[8]);
    targetdata->set_bid10_volume(msgdata->uBidVol[9]);


}




std::unordered_map<uint32_t, MsgHander> g_hander_mapping = {
    {MD_TYPE_SH_L2,               handle_shse_snapshot},
    {MD_TYPE_SH_L1_OPTION,        handle_shse_option_snapshot},
    {MD_TYPE_SZ_300111_L2,        handle_szse_snapshot},
    {MD_TYPE_SZ_300111_L1_OPTION, handle_szse_option_snapshot},
    {MD_TYPE_SZ_LF,               handle_szse_snapshot_ms},
};


std::string QxIndexEtfMd::GetInstrument(int msgtype, const char *aData){
    if (msgtype == MD_TYPE_SH_L2){
        Lev2WholeData *msgdata = (Lev2WholeData *)aData;
        return msgdata->security_id;
    }
    else if(msgtype == MD_TYPE_SH_L1_OPTION){
        Lev1Option *msgdata = (Lev1Option *)aData;
        return msgdata->security_id;
    }
    else if(msgtype == MD_TYPE_SZ_300111_L2){
        StaticAccMD *pappData = (StaticAccMD *)aData;
        MDSnapShotL2 *msgdata = (MDSnapShotL2 *)pappData->data;
        return msgdata->security_id;
    }
    else if(msgtype == MD_TYPE_SZ_300111_L1_OPTION){
        StaticAccMD *pappData = (StaticAccMD *)aData;
        OptionSnapShotL1 *msgdata = (OptionSnapShotL1 *)pappData->data;
        return msgdata->security_id;
    }
    else if(msgtype == MD_TYPE_SZ_LF){
        StaticAccMD *pappData = (StaticAccMD *)aData;
        t_SZ_MCBMarketDataLF *msgdata = (t_SZ_MCBMarketDataLF *)pappData->data;
        return msgdata->security_id;
    }


    return "";
}





QxIndexEtfMd::QxIndexEtfMd(): 
    m_fb_initialized(false),
    m_fb_spi(nullptr),
    m_fb_md(cffex::fb::api::market_data_entity::create_entity()),
    m_fb_inquiry(cffex::fb::api::inquiry_quote_entity::create_entity()),
    m_fb_status(cffex::fb::api::instrument_trading_status_entity::create_entity()),
    m_delay_stats(0),
    m_count_stats(0),
    m_reader_running(false)
{

    m_fb_md_interval.clear();
    std::clog << std::unitbuf; //调试用
}

QxIndexEtfMd::~QxIndexEtfMd(){
    std::clog << std::nounitbuf;
}

void QxIndexEtfMd::register_spi(cffex::fb::api::fb_i_md_spi *spi)
{
    printf("QxIndexEtfMd::register_spi[%p]\n",spi);
    m_fb_spi = spi;
}


    /*
    <demo_counterapi_md>
        <flow_path path="./flow/"/>
        <md_xlog loglevel="all" />
        <addr addr="tcp://172.31.194.19:8005"/>
        <thread_bind cpu_no="1" />
        <account brokerid="0001"  userid="renjh"  passwd="111111" encrypt="false" investorid="2032004">
            <item topicid="111"/>
        </account>
    </demo_counterapi_md>
    */
    // 从server_config.xml中解析出插件自定义的配置, 这里以femas柜台api插件的配置为例


/*
        <md_plugins>
            <libqx_index_etf_md>
                <md_xlog loglevel="all"/>
                <config exa_if="enp101s0" event_core_cpu_id="12"/>
                <config_path path="/home/febao/mdsz_config2.ini" cpu_no="11" md_info_cpu_no="12"/>
            </libqx_index_etf_md>
        </md_plugins>
*/



inline void QxIndexEtfMd::LoadJsonCfg(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    cffex::fb::api::fb_md_config_helper *parser = m_fb_spi->get_config_helper();
    std::string md_jsoncfgfile;
    parser->get_attribute("file", "/md_config_path", md_jsoncfgfile);

    // cfg
    std::ifstream jsonfile(md_jsoncfgfile);
    nlohmann::json json_parser = nlohmann::json::parse(jsonfile);

    auto& eth = json_parser["eth"];
    m_config.ifname = eth["IfName"];

    std::clog <<__func__<<","<< __LINE__<<",ifname[" <<m_config.ifname<<"]"<< std::endl;


    for (auto& [key, addr] : eth["addresses"].items()) {
        uint32_t ip = ip_str_to_net_ip(addr["GroupIp"].get<std::string>());
        uint16_t port = htons(addr["GroupPort"].get<uint16_t>());
        m_config.multicast.emplace_back(key, ip, port);

        std::clog <<__func__<<","<< __LINE__<<",section[" <<key<<"],Group[" <<ip<<"],port["<<port<<"]"<<",["<<addr["GroupIp"].get<std::string>()<<"],["<<addr["GroupPort"].get<uint16_t>()<<"]"<< std::endl;

    }

    auto& read = json_parser["read"];
    m_config.read_cpu = read["cpuid"].get<uint32_t>();
    m_config.filter_path = read["filter_instrument_info"];
    m_config.write_cpu = (json_parser["write"]["cpuid"]).get<uint32_t>();

    std::clog <<__func__<<","<< __LINE__<<",read_cpu[" <<m_config.read_cpu<<"]"<< std::endl;
    std::clog <<__func__<<","<< __LINE__<<",filter_path[" <<m_config.filter_path<<"]"<< std::endl;
    std::clog <<__func__<<","<< __LINE__<<",write_cpu[" <<m_config.write_cpu<<"]"<< std::endl;

}


void on_exit(int sig){
    g_running.store(false, std::memory_order_release);
}

bool QxIndexEtfMd::InitExanic(const char *ifName)
{
    std::clog <<__func__<<","<< __LINE__<<",ifName["<< ifName<<"]" << std::endl;
    char exaName[46];
    int portNum;
    
    if (0 != exanic_find_port_by_interface_name(ifName, exaName, sizeof(exaName) - 1, &portNum))
    {
        std::cerr << "exanic find port error:" <<ifName <<",error:"<< strerror(errno)<< std::endl;
        return false;
    }
    std::clog <<__func__<<","<< __LINE__<<",exaName["<< exaName<<"]"<<",portNum["<< portNum<<"]" << std::endl;
    
    m_exanic = exanic_acquire_handle(exaName);

    if (!m_exanic)
    {
        std::cerr << "exanic nullptr" << std::endl;
        return false;
    }

    std::clog <<__func__<<","<< __LINE__<<",m_exanic["<< reinterpret_cast<int64_t>(m_exanic) << std::endl;

    int port = 0;
    m_exanic_rx = exanic_acquire_rx_buffer(m_exanic, portNum, 0);
    if (!m_exanic_rx)
    {
        std::cerr << "exanic acquire rx buffer error: " << exanic_get_last_error() << std::endl;
        return false;
    }
    std::clog <<__func__<<","<< __LINE__<<",m_exanic_rx["<< reinterpret_cast<int64_t>(m_exanic_rx) << std::endl;
    signal(SIGINT, &on_exit);
    signal(SIGQUIT, &on_exit);

    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;

    return true;
}

void QxIndexEtfMd::ReleaseExanic()
{
    exanic_release_rx_buffer(m_exanic_rx);
    exanic_release_handle(m_exanic);
}





//ethhdr + udp_header + data
void QxIndexEtfMd::OnRcvData(const char *data, unsigned int dataLen, uint64_t timestamp)
{
    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "data[%p],dataLen[%d],timestamp[%llu]\n",data,dataLen,timestamp);
    //std::clog <<__func__<<","<< __LINE__<<","<<dataLen<<","<<timestamp<< std::endl;
    uint64_t local_time_ns = get_nanoseconds();
    tcpip::ip_header *pIpHeader = (tcpip::ip_header *)(data + sizeof(tcpip::ethhdr));
    uint8_t &&ip_h_len = (pIpHeader->ip_version & 0x0f) << 2;
    uint16_t dst_port =  ((tcpip::udp_header *)(data + ip_h_len + sizeof(tcpip::ethhdr)))->dstPort;
    uint16_t dstPort_local = ntohs(dst_port);
    std::string ip_str = uint32_to_ip_safe(pIpHeader->destination_ip_address);

    //m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "dataLen[%d],timestamp[%llu],Port[%u][%u],destination_ip_address[%d][%s]\n",dataLen,timestamp,dst_port,dstPort_local,pIpHeader->destination_ip_address,ip_str.c_str());

    if (!belongTo(pIpHeader->destination_ip_address, dst_port)){
        return;
    }

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "dataLen[%d],timestamp[%llu],Port[%u][%u],destination_ip_address[%d][%s] passed\n",dataLen,timestamp,dst_port,dstPort_local,pIpHeader->destination_ip_address,ip_str.c_str());

    const char *aData;
    aData = (data + ip_h_len + sizeof(tcpip::ethhdr) + sizeof(tcpip::udp_header));                          // 数据包
    unsigned int &&aLen = ntohs(((tcpip::udp_header *)(data + ip_h_len + sizeof(tcpip::ethhdr)))->dataLen); // 数据包的长度
    int msgtype = *((int*)aData);


    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "dataLen[%d],timestamp[%llu],[%s][%u],msgtype[0x%X]\n",dataLen,timestamp,ip_str.c_str(),dstPort_local,msgtype);


    if (g_hander_mapping.find(msgtype) == g_hander_mapping.end()){ //过滤掉不必要的msg
        return;
    }

    //过滤合约
    std::string inst = GetInstrument(msgtype, aData);

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "dataLen[%d],[%s][%u],msgtype[0x%X],inst[%s]\n",dataLen,ip_str.c_str(),dstPort_local,msgtype,inst.c_str());


    if (!(inst.size()>0 && m_option_info.Filter(inst))){
        return;
    }

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "dataLen[%d],timestamp[%llu],Port[%u][%u],destination_ip_address[%d][%s] passed\n",dataLen,timestamp,dst_port,dstPort_local,pIpHeader->destination_ip_address,ip_str.c_str());


    //m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],dataLen[%d],[%s][%u],msgtype[0x%X],inst[%s],[%llu][%llu]\n",__LINE__,dataLen,ip_str.c_str(),dstPort_local,msgtype,inst.c_str(),g_read_pos.load(std::memory_order_acquire),g_write_pos.load(std::memory_order_relaxed));


    size_t current_write = g_write_pos.load(std::memory_order_relaxed);
    size_t next_write = (current_write + 1) % QUEUE_MAX_LEN;
    
    if(next_write == g_read_pos.load(std::memory_order_acquire)){
        printf("FAULT:queue is full,please check!!!!!!!!!!!!!!!!!!!!!!!!!");// 队列已满，会丢弃msg
        return;
    }

    g_quota_queue[current_write].msg_type = msgtype;
    g_quota_queue[current_write].used = aLen;
    g_quota_queue[current_write].timestamp = local_time_ns;
    memcpy(g_quota_queue[current_write].data, aData, aLen - sizeof(tcpip::udp_header));
    g_write_pos.store(next_write, std::memory_order_release);

    uint64_t local_time_ns_end = get_nanoseconds();

    static uint64_t delay_sum = 0;
    static uint64_t count_stats = 0;
    delay_sum += (local_time_ns_end - local_time_ns);
    count_stats ++;
    if (count_stats%500 == 0){
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"read,ave cost[%llu]ns\n", delay_sum/500);
        delay_sum = 0;
    }

    //m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],dataLen[%d],[%s][%u],msgtype[0x%X],inst[%s],[%llu][%llu]\n",__LINE__,dataLen,ip_str.c_str(),dstPort_local,msgtype,inst.c_str(),g_read_pos.load(std::memory_order_acquire),g_write_pos.load(std::memory_order_relaxed));



}


void QxIndexEtfMd::ReaderRoutine(){
    BindCPU(m_config.write_cpu);
    char buff[2048];
    exanic_cycles32_t timestamp;
    std::clog <<__func__<<","<< __LINE__<<","<<m_reader_running<<","<<m_exanic_initialized.load(std::memory_order_relaxed) << std::endl;
    if (m_exanic_initialized.load(std::memory_order_relaxed)){
        while(g_running.load(std::memory_order_relaxed) && m_reader_running) {
            auto sz = exanic_receive_frame(m_exanic_rx, buff, sizeof(buff), &timestamp);
            auto expand_timestamp = exanic_expand_timestamp(m_exanic, timestamp);
            //std::clog <<__func__<<","<< __LINE__<<","<<sz<<","<<expand_timestamp << std::endl;
            if (sz > 0)
            {
                OnRcvData(buff, sz, expand_timestamp);
            }
        }
    }
}


int QxIndexEtfMd::init()
{
    std::clog <<__func__<<","<< __LINE__<< std::endl;

    LoadJsonCfg();
    InitExanic(m_config.ifname.c_str());

    m_reader.emplace_back(new std::thread(&QxIndexEtfMd::ReaderRoutine, this));  
    m_exanic_initialized = true;

    m_writer.emplace_back(new std::thread(&QxIndexEtfMd::WriterRoutine, this)); //和febao沟通，由于 on_msg的不可重入性，暂时只能有一个线程
    m_fb_initialized = true;
    
    m_reader_running = true;
    m_fb_spi->on_ready();
    //g_running.store(true,std::memory_order_release);
    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;
    return 0;
}



void QxIndexEtfMd::WriterRoutine(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    BindCPU(m_config.write_cpu);
    if (m_fb_initialized.load(std::memory_order_relaxed)){
        while(g_running) {
            SendMsg();
        }
    }
}

inline void QxIndexEtfMd::SendMsg() {
    uint64_t local_time_ns_start = get_nanoseconds();
    size_t cur = g_read_pos.load(std::memory_order_relaxed);
    if (cur == g_write_pos.load(std::memory_order_acquire)){ //empty
        return;
    }

    uint64_t local_time_ns_0 = get_nanoseconds();

    //std::clog <<__func__<<","<< __LINE__ <<","<< cur <<"," << g_write_pos.load(std::memory_order_acquire)<< std::endl;

    auto &element = g_quota_queue[cur];
    uint64_t local_time_ns_1 = get_nanoseconds();

    auto handler = g_hander_mapping.at(element.msg_type);

    uint64_t local_time_ns_2 = get_nanoseconds();

    m_fb_md->reset_entity();

    uint64_t local_time_ns_3 = get_nanoseconds();

    m_fb_md->set_local_timestamp(element.timestamp); //utc ns
    m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
    // m_fb_md->set_update_sec(msgdata->TimestampSecond);
    // m_fb_md->set_update_msec(msgdata->TimestampMillisecond);


    uint64_t local_time_ns_4 = get_nanoseconds();

    handler(this, element, m_fb_md);
    
    uint64_t local_time_ns_5 = get_nanoseconds();


    m_fb_spi->on_msg(m_fb_md);

    uint64_t local_time_ns_end = get_nanoseconds();

    g_read_pos.store((cur + 1) % QUEUE_MAX_LEN, std::memory_order_release);

    static uint64_t delay_sum = 0;
    static uint64_t delay_sum_0 = 0; //  -0
    static uint64_t delay_sum_1 = 0; // 0-1
    static uint64_t delay_sum_2 = 0; // 1-2
    static uint64_t delay_sum_3 = 0; // 2-3
    static uint64_t delay_sum_4 = 0; // 3-4
    static uint64_t delay_sum_5 = 0; // 4-5
    static uint64_t delay_sum_6 = 0; // 5-end


    static uint64_t count_stats = 0;
    delay_sum += (local_time_ns_end - local_time_ns_start);
    delay_sum_0 += (local_time_ns_0 - local_time_ns_start);
    delay_sum_1 += (local_time_ns_1 - local_time_ns_0);
    delay_sum_2 += (local_time_ns_2 - local_time_ns_1);
    delay_sum_3 += (local_time_ns_3 - local_time_ns_2);
    delay_sum_4 += (local_time_ns_4 - local_time_ns_3);
    delay_sum_5 += (local_time_ns_5 - local_time_ns_4);
    delay_sum_6 += (local_time_ns_end - local_time_ns_5);

    

    count_stats ++;
    if (count_stats%500 == 0){
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"send, [%llu], per[%llu],[%llu],[%llu],[%llu],[%llu],[%llu]\n", delay_sum/500, delay_sum_0/500, delay_sum_1/500, delay_sum_2/500, delay_sum_3/500, delay_sum_4/500, delay_sum_5/500, delay_sum_6/500);
        delay_sum = 0;        
        delay_sum_0 = 0;
        delay_sum_1 = 0;
        delay_sum_2 = 0;
        delay_sum_4 = 0;
        delay_sum_5 = 0;
        delay_sum_6 = 0;
    }

    m_delay_stats += (local_time_ns_end - element.timestamp);
    ++m_count_stats;
    if (m_count_stats%500 == 0){
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"from read to send,ave cost[%llu]ns\n", m_delay_stats/500);
        m_delay_stats = 0;
    }
    //std::clog <<__func__<<","<< __LINE__<<","<<g_read_pos.load(std::memory_order_relaxed)<< std::endl;
}

void QxIndexEtfMd::release()
{
    std::clog <<__func__<<","<< __LINE__ << std::endl;

    g_running.store(false,std::memory_order_release);
    for(size_t i = 0; i< m_writer.size();i++){
        if(m_writer[i]->joinable()){
            m_writer[i]->join();
        }
        delete m_writer[i];
        m_writer[i] = nullptr;
    }

    for(size_t i = 0; i< m_reader.size();i++){
        if(m_reader[i]->joinable()){
            m_reader[i]->join();
        }
        delete m_reader[i];
        m_reader[i] = nullptr;
    }

    m_writer.clear();
    m_reader.clear();

    ReleaseExanic();
}

void QxIndexEtfMd::connect()
{
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    g_running.store(true,std::memory_order_relaxed);
}

void QxIndexEtfMd::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id)
{
    std::clog <<__func__<<","<< __LINE__<< ","<<instrument_id<<","<< exchange_id<< std::endl;
}




