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

#include "qx_index_etf_md.h"
#include "tbj_gtjamcb.h"
#include "SockProto.h"
#include "fb_md_type.h"


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


inline bool QxIndexEtfMd::belongTo(uint32_t ip, uint16_t port){
    for(auto & it : m_config.multicast){
        if((it.ip == ip) && (it.port == port)){
            return true;
        }
    }
    return false;
}





inline void QxIndexEtfMd::handle_szse_snapshot_ms(const char *data, fb_market_data_t *targetdata){
    StaticAccMD *pappData = (StaticAccMD *)data;
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

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],code[%s],time[%d],utc_time[%d],utc_time2[%llu]\n",__LINE__,msgdata->security_id,msgdata->nTime,hhmmssToUtcSeconds(msgdata->nTime/1000),yyyymmddhhmmssToUtcSeconds(20250801161634));

    // std::clog <<__func__<<","<< __LINE__<<",["<<msgdata->iVolume<<"],["<<msgdata->iTurnover<<"]"<< std::endl;
    // std::clog <<__func__<<","<< __LINE__<<",["<< std::hex << std::setw(16) << std::setfill('0') <<msgdata->iVolume<<"],["<< std::hex << std::setw(16) << std::setfill('0') <<msgdata->iTurnover<<"]"<< std::endl;

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],iVolume[%llu],iTurnover[%llu],[%f][%f]\n",__LINE__,msgdata->iVolume,msgdata->iTurnover,convert_price_if_zero(0),convert_price_if_volume_zero(msgdata->uAskPrice[2]/PRICE_UINT,0));

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],[%llu][%d][%d][%llu],PreClose[%.4f],Open[%.4f],High[%.4f],Low[%.4f],Match[%.4f],Volume[%llu],iTurnover[%.4f]\n",__LINE__,msgdata->nTime,msgdata->nTime/1000,msgdata->nTime%1000,
    //                                     hhmmssToUtcSeconds(msgdata->nTime/1000),
    //                                     convert_price_if_zero(msgdata->uPreClose/PRICE_UINT),
    //                                     convert_price_if_zero(msgdata->uOpen/PRICE_UINT),
    //                                     convert_price_if_zero(msgdata->uHigh/PRICE_UINT),
    //                                     convert_price_if_zero(msgdata->uLow/PRICE_UINT),
    //                                     convert_price_if_zero(msgdata->uMatch/PRICE_UINT),
    //                                     msgdata->iVolume,
    //                                     msgdata->iTurnover/10000.0);

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],ap1[%.4f],ap2[%.4f],ap3[%.4f],ap4[%.4f],ap5[%.4f],ap6[%.4f],ap7[%.4f],ap8[%.4f],ap9[%.4f],ap10[%.4f]\n",__LINE__,
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

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],av1[%d],av2[%d],av3[%d],av4[%d],av5[%d],av6[%d],av7[%d],av8[%d],av9[%d],av10[%d]\n",__LINE__,
    //     msgdata->uAskVol[0],msgdata->uAskVol[1],msgdata->uAskVol[2],msgdata->uAskVol[3],msgdata->uAskVol[4],msgdata->uAskVol[5],msgdata->uAskVol[6],msgdata->uAskVol[7],msgdata->uAskVol[8],msgdata->uAskVol[9]);

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],bp1[%.4f],bp2[%.4f],bp3[%.4f],bp4[%.4f],bp5[%.4f],bp6[%.4f],bp7[%.4f],bp8[%.4f],bp9[%.4f],bp10[%.4f]\n",__LINE__,
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

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],bv1[%d],bv2[%d],bv3[%d],bv4[%d],bv5[%d],bv6[%d],bv7[%d],bv8[%d],bv9[%d],bv10[%d]\n",__LINE__,
    //     msgdata->uBidVol[0],msgdata->uBidVol[1],msgdata->uBidVol[2],msgdata->uBidVol[3],msgdata->uBidVol[4],msgdata->uBidVol[5],msgdata->uBidVol[6],msgdata->uBidVol[7],msgdata->uBidVol[8],msgdata->uBidVol[9]);

    targetdata->set_update_sec(hhmmssToUtcSeconds2(msgdata->nTime/1000));
    // targetdata->set_update_sec(msgdata->nTime/1000);
    targetdata->set_update_msec(msgdata->nTime%1000);
    targetdata->set_max_depth(10);                              // 设置行情深度10
    targetdata->set_instrument_id(msgdata->security_id);
    targetdata->set_exchange_id(cffex::fb::api::FB_EXCHANGE_SZSE);
    targetdata->set_pre_close(convert_price_if_zero(msgdata->uPreClose/PRICE_UINT));
    targetdata->set_open(convert_price_if_zero(msgdata->uOpen/PRICE_UINT));
    targetdata->set_close(convert_price_if_zero(msgdata->uMatch/PRICE_UINT));
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



inline void QxIndexEtfMd::handle_szse_option_snapshot(const char *data, fb_market_data_t *targetdata){
    StaticAccMD *pappData = (StaticAccMD *)data;
    OptionSnapShotL1 *msgdata = (OptionSnapShotL1 *)pappData->data;
    std::string inst(msgdata->security_id, 8);
    const double PRICE_UINT = 1000000.0;





    // if (inst == "90005721"){

    //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],[%llu][%d][%d][%llu],PreClose[%.4f],Open[%.4f],High[%.4f],Low[%.4f],Match[%.4f],upper[%.4f],lower[%.4f],position[%.4f],Volume[%llu],iTurnover[%llu]\n",__LINE__,
    //                                         msgdata->md_time,(msgdata->md_time/1000)%1000000,msgdata->md_time%1000,yyyymmddhhmmssmsToUtcSeconds2(msgdata->md_time),
    //                                         convert_price_if_zero(msgdata->pre_close/10000.0),
    //                                         convert_price_if_zero(msgdata->open/PRICE_UINT),
    //                                         convert_price_if_zero(msgdata->high/PRICE_UINT),
    //                                         convert_price_if_zero(msgdata->low/PRICE_UINT),
    //                                         convert_price_if_zero(msgdata->match/PRICE_UINT),
    //                                         msgdata->upper_limit/PRICE_UINT,
    //                                         msgdata->lower_limit/PRICE_UINT,
    //                                         msgdata->position_volume/100.0,
    //                                         msgdata->total_volume/100,
    //                                         msgdata->total_value/10000);



    //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],ap1[%.4f],ap2[%.4f],ap3[%.4f],ap4[%.4f],ap5[%.4f]\n",__LINE__,
    //                                                     convert_price_if_volume_zero(msgdata->ask_price[0]/PRICE_UINT,msgdata->ask_vol[0]),
    //                                                     convert_price_if_volume_zero(msgdata->ask_price[1]/PRICE_UINT,msgdata->ask_vol[1]),
    //                                                     convert_price_if_volume_zero(msgdata->ask_price[2]/PRICE_UINT,msgdata->ask_vol[2]),
    //                                                     convert_price_if_volume_zero(msgdata->ask_price[3]/PRICE_UINT,msgdata->ask_vol[3]),
    //                                                     convert_price_if_volume_zero(msgdata->ask_price[4]/PRICE_UINT,msgdata->ask_vol[4]));

    //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],av1[%d],av2[%d],av3[%d],av4[%d],av5[%d]\n",__LINE__,
    //         msgdata->ask_vol[0]/100,msgdata->ask_vol[1]/100,msgdata->ask_vol[2]/100,msgdata->ask_vol[3]/100,msgdata->ask_vol[4]/100);

    //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],bp1[%.4f],bp2[%.4f],bp3[%.4f],bp4[%.4f],bp5[%.4f]\n",__LINE__,
    //                                                     convert_price_if_volume_zero(msgdata->bid_price[0]/PRICE_UINT,msgdata->bid_vol[0]),
    //                                                     convert_price_if_volume_zero(msgdata->bid_price[1]/PRICE_UINT,msgdata->bid_vol[1]),
    //                                                     convert_price_if_volume_zero(msgdata->bid_price[2]/PRICE_UINT,msgdata->bid_vol[2]),
    //                                                     convert_price_if_volume_zero(msgdata->bid_price[3]/PRICE_UINT,msgdata->bid_vol[3]),
    //                                                     convert_price_if_volume_zero(msgdata->bid_price[4]/PRICE_UINT,msgdata->bid_vol[4]));

    //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],bv1[%d],bv2[%d],bv3[%d],bv4[%d],bv4[%d]\n",__LINE__,
    //         msgdata->bid_vol[0]/100,msgdata->bid_vol[1]/100,msgdata->bid_vol[2]/100,msgdata->bid_vol[3]/100,msgdata->bid_vol[4]/100);

    // }

    targetdata->set_update_sec(yyyymmddhhmmssmsToUtcSeconds2(msgdata->md_time));
    // targetdata->set_update_sec((msgdata->md_time/1000)%1000000);

    targetdata->set_update_msec(msgdata->md_time%1000);
    targetdata->set_max_depth(5);                              // 设置行情深度5
    targetdata->set_instrument_id(inst.c_str());
    targetdata->set_exchange_id(cffex::fb::api::FB_EXCHANGE_SZSE);
    targetdata->set_pre_close(convert_price_if_zero(msgdata->pre_close/10000.0));
    targetdata->set_open(convert_price_if_zero(msgdata->open/PRICE_UINT));
    targetdata->set_close(convert_price_if_zero(msgdata->match/PRICE_UINT));
    targetdata->set_upper_limit_price(msgdata->upper_limit/PRICE_UINT);
    targetdata->set_down_limit_price(msgdata->lower_limit/PRICE_UINT);
    targetdata->set_high_price(convert_price_if_zero(msgdata->high/PRICE_UINT));
    targetdata->set_low_price(convert_price_if_zero(msgdata->low/PRICE_UINT));
    targetdata->set_last_price(convert_price_if_zero(msgdata->match/PRICE_UINT));
    targetdata->set_volume(msgdata->total_volume/100);
    targetdata->set_turn_over(msgdata->total_value/10000.0);
    targetdata->set_open_interest(msgdata->position_volume/100.0);
    targetdata->set_ask1_price(convert_price_if_volume_zero(msgdata->ask_price[0]/PRICE_UINT,msgdata->ask_vol[0]/100));
    targetdata->set_ask2_price(convert_price_if_volume_zero(msgdata->ask_price[1]/PRICE_UINT,msgdata->ask_vol[1]/100));
    targetdata->set_ask3_price(convert_price_if_volume_zero(msgdata->ask_price[2]/PRICE_UINT,msgdata->ask_vol[2]/100));
    targetdata->set_ask4_price(convert_price_if_volume_zero(msgdata->ask_price[3]/PRICE_UINT,msgdata->ask_vol[3]/100));
    targetdata->set_ask5_price(convert_price_if_volume_zero(msgdata->ask_price[4]/PRICE_UINT,msgdata->ask_vol[4]/100));
    targetdata->set_ask1_volume(msgdata->ask_vol[0]/100);
    targetdata->set_ask2_volume(msgdata->ask_vol[1]/100);
    targetdata->set_ask3_volume(msgdata->ask_vol[2]/100);
    targetdata->set_ask4_volume(msgdata->ask_vol[3]/100);
    targetdata->set_ask5_volume(msgdata->ask_vol[4]/100);
    targetdata->set_bid1_price(convert_price_if_volume_zero(msgdata->bid_price[0]/PRICE_UINT,msgdata->bid_vol[0]/100));
    targetdata->set_bid2_price(convert_price_if_volume_zero(msgdata->bid_price[1]/PRICE_UINT,msgdata->bid_vol[1]/100));
    targetdata->set_bid3_price(convert_price_if_volume_zero(msgdata->bid_price[2]/PRICE_UINT,msgdata->bid_vol[2]/100));
    targetdata->set_bid4_price(convert_price_if_volume_zero(msgdata->bid_price[3]/PRICE_UINT,msgdata->bid_vol[3]/100));
    targetdata->set_bid5_price(convert_price_if_volume_zero(msgdata->bid_price[4]/PRICE_UINT,msgdata->bid_vol[4]/100));
    targetdata->set_bid1_volume(msgdata->bid_vol[0]/100);
    targetdata->set_bid2_volume(msgdata->bid_vol[1]/100);
    targetdata->set_bid3_volume(msgdata->bid_vol[2]/100);
    targetdata->set_bid4_volume(msgdata->bid_vol[3]/100);
    targetdata->set_bid5_volume(msgdata->bid_vol[4]/100);
}


inline std::string QxIndexEtfMd::GetInstrumentInfo(int msgtype, const char *aData, uint64_t &ap1, uint64_t &av1, uint64_t &bp1, uint64_t &bv1){
    if(msgtype == MD_TYPE_SZ_300111_L1_OPTION){
        StaticAccMD *pappData = (StaticAccMD *)aData;
        OptionSnapShotL1 *msgdata = (OptionSnapShotL1 *)pappData->data;
        std::string str(msgdata->security_id, 8);
        ap1 = msgdata->ask_price[0];
        av1 = msgdata->ask_vol[0];
        bp1 = msgdata->bid_price[0];
        bv1 = msgdata->ask_vol[0];
        return str;
    }
    else if(msgtype == MD_TYPE_SZ_LF){
        StaticAccMD *pappData = (StaticAccMD *)aData;
        t_SZ_MCBMarketDataLF *msgdata = (t_SZ_MCBMarketDataLF *)pappData->data;
        ap1 = msgdata->uAskPrice[0]*100;
        av1 = msgdata->uAskVol[0]  *100;
        bp1 = msgdata->uBidPrice[0]*100;
        bv1 = msgdata->uBidVol[0]  *100;
        return msgdata->security_id;
    }

    return "";
}

QxIndexEtfMd::QxIndexEtfMd(): 
    m_fb_initialized(false),
    m_exanic_initialized(false),
    m_fb_spi(nullptr),
    m_fb_md(cffex::fb::api::market_data_entity::create_entity()),
    m_fb_inquiry(cffex::fb::api::inquiry_quote_entity::create_entity()),
    m_fb_status(cffex::fb::api::instrument_trading_status_entity::create_entity())
{
    std::clog << std::unitbuf; //调试用

    // std::string testfile = "/home/febao/shenzx/test.csv";  // TODO stub
    // m_quota_stub.Load(testfile);   // TODO stub
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
}

QxIndexEtfMd::~QxIndexEtfMd(){
    std::clog << std::nounitbuf;
}

void QxIndexEtfMd::register_spi(cffex::fb::api::fb_i_md_spi *spi){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    m_fb_spi = spi;
}

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

    auto& worker = json_parser["worker"];
    m_config.cpu_id = worker["cpuid"].get<uint32_t>();

    auto& filter = json_parser["filter"];
    for (const auto& item : filter["underlying"]) {
        m_option_info.AddUnderlying(item);
    }
    m_config.filter_path = filter["filter_instrument_info"];
    m_option_info.Load(m_config.filter_path);

    std::clog <<__func__<<","<< __LINE__<<",read_cpu[" <<m_config.cpu_id<<"]"<< std::endl;
    std::clog <<__func__<<","<< __LINE__<<",filter_path[" <<m_config.filter_path<<"]"<< std::endl;

    auto &traffic = json_parser["traffic"];
    m_config.mode = traffic["mode"].get<uint32_t>();
    m_config.time_span = traffic["time_span"].get<long long>() *1000000;

    std::clog <<__func__<<","<< __LINE__<<",mode[" << m_config.mode<<"]"<< std::endl;
    std::clog <<__func__<<","<< __LINE__<<",time_span[" << m_config.time_span<<"]"<< std::endl;

    auto & insts = m_option_info.GetAllInst();
    for(auto inst: insts){
        m_monitor_last_quota.emplace(inst, Monitor_Lable_t());
    }

}

void on_exit(int sig){
    g_running.store(false, std::memory_order_release);
}

bool QxIndexEtfMd::InitExanic(const char *ifName){
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

    std::clog <<__func__<<","<< __LINE__<<",m_exanic["<< reinterpret_cast<int64_t>(m_exanic) <<"]" << std::endl;

    int port = 0;
    m_exanic_rx = exanic_acquire_rx_buffer(m_exanic, portNum, 0);
    if (!m_exanic_rx)
    {
        std::cerr << "exanic acquire rx buffer error: " << exanic_get_last_error() << std::endl;
        return false;
    }
    std::clog <<__func__<<","<< __LINE__<<",m_exanic_rx["<< reinterpret_cast<int64_t>(m_exanic_rx)<<"]" << std::endl;
    signal(SIGINT, &on_exit);
    signal(SIGQUIT, &on_exit);

    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;

    return true;
}

void QxIndexEtfMd::ReleaseExanic(){
    exanic_release_rx_buffer(m_exanic_rx);
    exanic_release_handle(m_exanic);
}

//ethhdr + udp_header + data
void QxIndexEtfMd::OnData(){
    uint64_t local_time_ns_start = get_nanoseconds();

#if 1  // TODO stub






    char data[2048];
    exanic_cycles32_t timestamp;
    auto sz = exanic_receive_frame(m_exanic_rx, data, sizeof(data), &timestamp);
    if (sz <= 0){
        return;
    }

    //uint64_t local_time_ns_0 = get_nanoseconds();

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "data[%p],dataLen[%d],timestamp[%llu]\n",data,dataLen,timestamp);
    //std::clog <<__func__<<","<< __LINE__<<","<<dataLen<<","<<timestamp<< std::endl;

    tcpip::ip_header *pIpHeader = (tcpip::ip_header *)(data + sizeof(tcpip::ethhdr));
    uint8_t &&ip_h_len = (pIpHeader->ip_version & 0x0f) << 2;
    uint16_t dst_port =  ((tcpip::udp_header *)(data + ip_h_len + sizeof(tcpip::ethhdr)))->dstPort;
    // uint16_t dstPort_local = ntohs(dst_port);
    // std::string ip_str = uint32_to_ip_safe(pIpHeader->destination_ip_address);

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],timestamp[%llu],Port[%u][%u],destination_ip_address[%d][%s]\n",__LINE__,timestamp,dst_port,dstPort_local,pIpHeader->destination_ip_address,ip_str.c_str());

    if (!belongTo(pIpHeader->destination_ip_address, dst_port)){
        return;
    }

    // if (ip_str == "239.0.200.205" || ip_str == "239.1.1.9" || ip_str == "239.1.1.6"|| ip_str == "239.1.1.4" || ip_str == "239.0.200.206"  || ip_str == "239.1.1.5"){
    //     return;
    // }

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "dataLen[%d],timestamp[%llu],Port[%u][%u],destination_ip_address[%d][%s] passed\n",dataLen,timestamp,dst_port,dstPort_local,pIpHeader->destination_ip_address,ip_str.c_str());

    const char *aData;
    aData = (data + ip_h_len + sizeof(tcpip::ethhdr) + sizeof(tcpip::udp_header));                          // 数据包
    unsigned int &&aLen = ntohs(((tcpip::udp_header *)(data + ip_h_len + sizeof(tcpip::ethhdr)))->dataLen); // 数据包的长度
    int msgtype = *((int*)aData);

    //m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],timestamp[%llu],[%s][%u],msgtype[0x%X]\n",__LINE__,timestamp,ip_str.c_str(),dstPort_local,msgtype);

    //uint64_t local_time_ns_1 = get_nanoseconds();

    if ((msgtype!=MD_TYPE_SZ_LF) && (msgtype!= MD_TYPE_SZ_300111_L1_OPTION)){ //过滤掉不必要的msg
        return;
    }


#endif

    // static size_t q_id = 0;
    // Quotatation quota = {};
    // q_id = m_quota_stub.GetNextQuota(q_id, quota, 6);

    // if (q_id <0) return;

    // const char *aData = (const char *)(&quota);
    // int msgtype = quota.type;

    //过滤合约
    uint64_t ap1 = 0;
    uint64_t av1 = 0;
    uint64_t bp1 = 0;
    uint64_t bv1 = 0;
    std::string inst = GetInstrumentInfo(msgtype, aData, ap1, av1, bp1, bv1);
    if (!(inst.size()>0 && m_option_info.Filter(inst))){
        return;
    }

    //std::clog <<__func__<<","<< __LINE__<<",q_id:"<<q_id<<",msgtype:"<<msgtype<<",inst:"<<inst<<","<<ap1<<","<<av1<<","<<bp1<<","<<bv1<<","<<local_time_ns_start<< std::endl;

    if (m_config.mode){
        if (m_monitor_last_quota.count(inst) > 0){
            auto & last_quota = m_monitor_last_quota.at(inst);
            if (m_config.mode == TRAFFIC_FLOW_MODE_ONLY_SPAN){
                if ((last_quota.time > 0) && (local_time_ns_start < (last_quota.time + m_config.time_span))){  //时间间隔不足，直接丢弃
                    last_quota.time = local_time_ns_start;
                    return;
                }
                last_quota.time = local_time_ns_start;
            }
            else if(m_config.mode == TRAFFIC_FLOW_MODE_LV1_SPAN){ //时间间隔内，Lv1无变化直接丢弃
                // std::clog <<__func__<<","<< __LINE__<<",q_id:"<<q_id<<",msgtype:"<<msgtype<<",inst:"<<inst<<","<<last_quota.ap1<<","<<last_quota.av1<<","<<last_quota.bp1<<","<<last_quota.bv1<<","<<last_quota.time<< std::endl;

                // std::clog <<__func__<<","<< __LINE__<<",q_id:"<<q_id<<",msgtype:"<<msgtype<<",inst:"<<inst<<","<<ap1<<","<<av1<<","<<bp1<<","<<bv1<<","<<local_time_ns_start<< std::endl;

                if ((last_quota.time > 0) && (local_time_ns_start < (last_quota.time + m_config.time_span))
                    && (last_quota.ap1 == ap1) && (last_quota.av1 == av1)
                    && (last_quota.bp1 == bp1) && (last_quota.bv1 == bv1)){
                    last_quota.time = local_time_ns_start;
                    last_quota.ap1 = ap1;
                    last_quota.av1 = av1;
                    last_quota.bp1 = bp1;
                    last_quota.bv1 = bv1;

                    // std::clog <<__func__<<","<< __LINE__<<",q_id:"<<q_id<<",msgtype:"<<msgtype<<",inst:"<<inst<<","<<last_quota.ap1<<","<<last_quota.av1<<","<<last_quota.bp1<<","<<last_quota.bv1<<","<<last_quota.time<< std::endl;

                    return;
                }
                last_quota.time = local_time_ns_start;
                last_quota.ap1 = ap1;
                last_quota.av1 = av1;
                last_quota.bp1 = bp1;
                last_quota.bv1 = bv1;
            }
        }
        else{
            //std::clog <<__func__<<","<< __LINE__<<",q_id:"<<q_id<<",msgtype:"<<msgtype<<",inst:"<<inst<<","<<ap1<<","<<av1<<","<<bp1<<","<<bv1<<","<<local_time_ns_start<< std::endl;

            m_monitor_last_quota.emplace(inst,Monitor_Lable_t());
            m_monitor_last_quota[inst].ap1 = ap1;
            m_monitor_last_quota[inst].av1 = av1;
            m_monitor_last_quota[inst].bp1 = bp1;
            m_monitor_last_quota[inst].bv1 = bv1;
            m_monitor_last_quota[inst].time = local_time_ns_start;
        };
    }

    // std::clog <<__func__<<","<< __LINE__<<",q_id:"<<q_id<< std::endl;

    //m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d][%s][%u],msgtype[0x%X],inst[%s]\n",__LINE__,ip_str.c_str(),dstPort_local,msgtype,inst.c_str());


    // static std::set<std::string> insts;
    // if (insts.count(inst)==0){
    //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d][%s][%u],msgtype[0x%X],inst[%s]\n",__LINE__,ip_str.c_str(),dstPort_local,msgtype,inst.c_str());
    //     insts.emplace(inst);
    // }

    //m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d][%s][%u],msgtype[0x%X],inst[%s]\n",__LINE__,ip_str.c_str(),dstPort_local,msgtype,inst.c_str());

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "dataLen[%d],timestamp[%llu],Port[%u][%u],destination_ip_address[%d][%s] passed\n",dataLen,timestamp,dst_port,dstPort_local,pIpHeader->destination_ip_address,ip_str.c_str());

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],local[%llu],[%s][%u],msgtype[0x%X],inst[%s],[%llu][%llu]\n",__LINE__,local_time_ns_0,ip_str.c_str(),dstPort_local,msgtype,inst.c_str(),g_read_pos.load(std::memory_order_acquire),g_write_pos.load(std::memory_order_relaxed));
 

    //uint64_t local_time_ns_2 = get_nanoseconds();

    m_fb_md->reset_entity();
    m_fb_md->set_local_timestamp(local_time_ns_start); //utc ns
    m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计

    if (msgtype==MD_TYPE_SZ_LF){
        handle_szse_snapshot_ms(aData, m_fb_md);
    }
    else if (msgtype== MD_TYPE_SZ_300111_L1_OPTION){
        handle_szse_option_snapshot(aData, m_fb_md);
    }
    else {
        return;
    }

    //uint64_t local_time_ns_3 = get_nanoseconds();

    m_fb_spi->on_msg(m_fb_md);

    uint64_t local_time_ns_end = get_nanoseconds();

    static uint64_t delay_sum = 0;
    static uint64_t delay_sum_0 = 0; //  -0
    static uint64_t delay_sum_1 = 0; // 0-1
    static uint64_t delay_sum_2 = 0; // 1-2
    static uint64_t delay_sum_3 = 0; // 2-3
    static uint64_t delay_sum_4 = 0; // 3-end

    static uint64_t count_stats = 0;
    delay_sum += (local_time_ns_end - local_time_ns_start);
    // delay_sum_0 += (local_time_ns_0 - local_time_ns_start);
    // delay_sum_1 += (local_time_ns_1 - local_time_ns_0);
    // delay_sum_2 += (local_time_ns_2 - local_time_ns_1);
    // delay_sum_3 += (local_time_ns_3 - local_time_ns_2);
    // delay_sum_4 += (local_time_ns_end - local_time_ns_3);


    int cpu = sched_getcpu();

    static uint64_t  period = 10000;

    count_stats ++;
    if (count_stats%period == 0){
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"CPU[%d],count sum [%llu], per msg routine cost[%llu]\n",cpu,period, delay_sum/period);

        //m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"CPU[%d],per [%llu] ave, all cost[%llu], section read[%llu],decode[%llu],filter[%llu],fill[%llu],send[%llu][%.4f]\n",cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period, delay_sum_4/period,delay_sum_4/(delay_sum*1.0));
        delay_sum = 0;        
        // delay_sum_0 = 0;
        // delay_sum_1 = 0;
        // delay_sum_2 = 0;
        // delay_sum_3 = 0;
        // delay_sum_4 = 0;
    }

    //m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],dataLen[%d],[%s][%u],msgtype[0x%X],inst[%s],[%llu][%llu]\n",__LINE__,dataLen,ip_str.c_str(),dstPort_local,msgtype,inst.c_str(),g_read_pos.load(std::memory_order_acquire),g_write_pos.load(std::memory_order_relaxed));

}



void QxIndexEtfMd::Routine(){
    BindCPU(m_config.cpu_id);
    exanic_cycles32_t timestamp;
    std::clog <<__func__<<","<< __LINE__<<","<<g_running<<","<<m_exanic_initialized.load(std::memory_order_acquire) << std::endl;
    if (m_exanic_initialized.load(std::memory_order_acquire) && m_fb_initialized.load(std::memory_order_acquire)){
        while(g_running.load(std::memory_order_relaxed)) {
            OnData();
        }
    }
}


int QxIndexEtfMd::init(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;

    LoadJsonCfg();
    InitExanic(m_config.ifname.c_str());   // TODO stub

    m_worker = new std::thread(&QxIndexEtfMd::Routine, this);   //和febao沟通，由于 on_msg的不可重入性，暂时只能有一个线程
    m_exanic_initialized.store(true,std::memory_order_release);
    m_fb_initialized.store(true,std::memory_order_release);
    m_fb_spi->on_ready();
    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;
    return 0;
}

void QxIndexEtfMd::release(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    g_running.store(false,std::memory_order_release);
    if(m_worker->joinable()){
        m_worker->join();
    }
    delete m_worker;
    m_worker = nullptr;

    ReleaseExanic();
}

void QxIndexEtfMd::connect(){
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    g_running.store(true,std::memory_order_relaxed);
}

void QxIndexEtfMd::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id){
    m_monitor_last_quota.emplace(instrument_id,Monitor_Lable_t());
    std::clog <<__func__<<","<< __LINE__<< ","<<instrument_id<<","<< exchange_id<< std::endl;
}




