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

    // auto &muticast = m_config_mgr.getMulticast();

    // uint32_t muticast_cpu = m_config_mgr.getCpuId(0);

    // std::clog <<__func__<<","<< __LINE__<<",muticast_cpu:"<<muticast.group_ip.c_str()<< std::endl;

    // std::clog <<__func__<<","<< __LINE__<<",muticast_cpu:"<<muticast_cpu<< std::endl;


    // m_udp_receiver.reset(new MulticastReceiver(muticast.group_ip, muticast.group_port, muticast.IfName, muticast_cpu, 1024)); //因为数据长度是135,所以buffer长度选择256
    // m_udp_receiver->registerCallback((void *)this, UdpHandler);
    // m_udp_receiver->start();





    m_fb_spi->on_ready();
    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;
    return 0;
}

void SSEQuota::release(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    //m_udp_receiver->stop();
    g_running.store(false,std::memory_order_release);

    m_xeleMd->Stop();

}

void SSEQuota::connect(){
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;

    Init_Xele();

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




void SSEQuota::Init_Xele(){
    m_xeleMd = XeleMd::CreatInstance();
    // ProcessAuth();
    std::clog <<__func__<<","<< __LINE__<<",ProcessAuth bypass"<< std::endl;
    MdParam *params = m_config_mgr.getChannelParam();
    uint32_t chn_num = m_config_mgr.getChannelNum();
    m_xeleMd->Init(params, chn_num);

    XeleReceiver *xele_receiver = new XeleReceiver();
    xele_receiver->Init(m_fb_spi,m_fb_md, &m_quota_cache);
    m_xeleMd->RegisterSseSpi(xele_receiver);
    int ret = m_xeleMd->Start();
    if (!ret){
        std::clog <<__func__<<","<< __LINE__<< ",xeleMd Start ret:"<<ret<< std::endl;
    }
    
}



void SSEQuota::ProcessAuth()
{
    auto &authinfo = m_config_mgr.getAuthInfo();
    if (authinfo.tcp_ipaddr.empty() || authinfo.tcp_port == 0 || authinfo.username.empty() || authinfo.password.empty() || authinfo.interface_name.empty()){
        return;
    }

    struct LoginReq login_req = {0};
    memcpy(login_req.server_ip, authinfo.tcp_ipaddr.data(), authinfo.tcp_ipaddr.size());
    login_req.server_port = authinfo.tcp_port;
    memcpy(login_req.username, authinfo.username.data(), authinfo.username.size());
    memcpy(login_req.password, authinfo.password.data(), authinfo.password.size());
    memcpy(login_req.eth_name, authinfo.interface_name.data(), authinfo.interface_name.size());
    
    LoginRsp login_rsp{};
    struct MulticastAddr *addrBegin;
    if (!m_xeleMd->Login(&login_req, &login_rsp, &addrBegin))
    {
        std::clog <<__func__<<","<< __LINE__<<",switch to backup auth server..."<< std::endl;
        //鉴权失败，尝试登录备用鉴权服务器
        if (!authinfo.backup_tcp_ipaddr.empty() && authinfo.backup_tcp_port > 0)
        {
            memcpy(login_req.server_ip, authinfo.backup_tcp_ipaddr.data(), authinfo.backup_tcp_ipaddr.size());
            login_req.server_port = authinfo.backup_tcp_port;
            
            if (!m_xeleMd->Login(&login_req, &login_rsp, &addrBegin))
            {
                std::clog <<__func__<<","<< __LINE__<<",backup UserLogin return false,exit!"<< std::endl;
                exit(0);
            }
        }
        else{
            std::clog <<__func__<<","<< __LINE__<<",UserLogin return false,exit!"<< std::endl;
            exit(0);
        }
    }
    
    std::clog <<__func__<<","<< __LINE__<<",Login Success,login_rsp.numAddr[" <<login_rsp.numAddr<<"]" << std::endl;

    auto tempPtr = addrBegin;
    /*  登录响应会返回所有组播地址，比如对于上交行情，在上海和在深圳收组播地址不同，同时又有主备的区别，
    **  请根据实际情况加入组播组，不要重复添加，否则会收到重复的数据。  
    */
    for (unsigned int i = 0; i < login_rsp.numAddr; i++)
    {
        std::clog <<__func__<<","<< __LINE__<<",Login resp,md_system[" <<tempPtr->md_system<<"],ip:" <<tempPtr->ip<<",port:" <<tempPtr->port<<",md_type:"<<tempPtr->md_type<< std::endl;
        tempPtr++;
    }
    
    return;
}

void XeleReceiver::Init(cffex::fb::api::fb_i_md_spi *fb_spi, fb_market_data_t *fb_md, InstMap<QuotaInfo> *quota_cache){
    m_fb_spi = fb_spi;
    m_fb_md = fb_md;
    m_quota_cache = quota_cache;
}


void XeleReceiver::onStaticInfoSse(StaticInfoSse* msgdata){
    // QuotaInfo *cache = m_quota_cache.Find(msgdata->securityID);
    // if (cache==nullptr){
    //     return;
    // }


    m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "recv [%d],inst[%s],[%.4f],bp[%.4f]\n",__LINE__,
          msgdata->securityID,msgdata->upperLimitPrice,msgdata->lowerLimitPrice);


    // cache->upper = msgdata->upperLimitPrice;
    // cache->lower = msgdata->lowerLimitPrice;

};



// void XeleReceiver::UdpHandler(void *ctx, const char* data, size_t size,  const std::string& source_ip, uint16_t source_port){
//     SSEQuota *pthis = (SSEQuota *)ctx;
//     if (size == sizeof(OptionSseL1)){
//         pthis->onOptionSseL1((OptionSseL1*)data);
//     }

//     if (size == sizeof(StaticInfoSse)){
//         pthis->onStaticInfoSse((StaticInfoSse*)data);
//     }

//     if (size == sizeof(FundSseL1)){
//         pthis->onFundSseL1((FundSseL1*)data);
//     }
// }




void XeleReceiver::onFundSseL1(FundSseL1* msgdata) {

    if (msgdata == nullptr){
        std::clog <<__func__<<","<< __LINE__<<",xele recv len 0 msg!"<< std::endl;
        return;
    }



    int64_t T_start = get_nanoseconds();
    double multi = 100000.0;
    // std::string inst(msgdata->securityID);

    // if ((inst != "510500") && (inst != "510050") && (inst != "510300")){
    //     return;
    // }

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "recv [%d],inst[%s],lastUpdateTime[%llu],[%.4f],[%llu][%llu][%.4f],bp[%.4f],ap[%.4f],bv[%llu],av[%llu]\n",__LINE__,
    //                 msgdata->securityID,msgdata->lastUpdateTime,msgdata->lastPrice/multi,msgdata->securityType,msgdata->totalVolumeTraded,msgdata->totalValueTraded/100.0,
    //                 msgdata->askPriceQty[0].price/multi, msgdata->bidPriceQty[0].price/multi,msgdata->askPriceQty[0].qty,msgdata->bidPriceQty[0].qty);



    QuotaInfo *cache = m_quota_cache->Find(msgdata->securityID);
    if (cache==nullptr){
        return;
    }

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "filter recv [%d],inst[%s],lastUpdateTime[%llu],[%.4f],[%llu][%llu][%.4f],bp[%.4f],ap[%.4f],bv[%llu],av[%llu]\n",__LINE__,
    //                 msgdata->securityID,msgdata->lastUpdateTime,msgdata->lastPrice/multi,msgdata->securityType,msgdata->totalVolumeTraded,msgdata->totalValueTraded/100.0,
    //                 msgdata->askPriceQty[0].price/multi, msgdata->bidPriceQty[0].price/multi,msgdata->askPriceQty[0].qty,msgdata->bidPriceQty[0].qty);


    uint64_t sss    = msgdata->lastUpdateTime%1000L;
    uint64_t hhmmss = msgdata->lastUpdateTime/1000L;
    uint64_t extime = (hhmmss/10000L)*3600+((hhmmss%10000L)/100)*60+(hhmmss%100L);

    int64_t T1 = get_nanoseconds();
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

    // m_fb_md->set_upper_limit_price(convert_price_if_zero(cache->upper));
    // m_fb_md->set_down_limit_price(convert_price_if_zero(cache->lower));


    m_fb_md->set_max_depth(5);                              // 设置行情深度5
    m_fb_md->set_iopv(msgdata->IOPV);
    m_fb_md->set_dynamic_reference_price(IOPV_TAG_LV1);

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

    int64_t T2 = get_nanoseconds();

    m_fb_spi->on_msg(m_fb_md);
    int64_t T_end = get_nanoseconds();
    
    static uint64_t delay_sum = 0;
    static uint64_t delay_sum_0 = 0; //  -0
    static uint64_t delay_sum_1 = 0; // 0-1
    static uint64_t delay_sum_2 = 0; // 0-1

    static uint64_t count_stats = 0;
    delay_sum += (T_end - T_start);
    delay_sum_0 += (T1 - T_start);
    delay_sum_1 += (T2 - T1);
    delay_sum_2 += (T_end - T2);

    static uint64_t  period = 100;
    count_stats ++;
    if (count_stats%period == 0){
        int cpu = sched_getcpu();
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L1 Fund, CPU[%d],per [%llu] ave, all cost[%llu], section read[%llu],fill[%llu],send[%llu]\n",
                    cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period);
        delay_sum = 0;        
        delay_sum_0 = 0;
        delay_sum_1 = 0;
        delay_sum_2 = 0;
    }



}



void XeleReceiver::onOptionSseL1(OptionSseL1* msgdata) {

    if (msgdata == nullptr){
        std::clog <<__func__<<","<< __LINE__<<",xele recv len 0 msg!"<< std::endl;
        return;
    }

    int64_t T_start = get_nanoseconds();
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



    QuotaInfo *cache = m_quota_cache->Find(msgdata->securityID);
    if (cache==nullptr){
        return;
    }
    

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "filter recv [%d],inst[%s],extime[%llu],[%.4f],[%llu][%llu][%.4f],bp[%.4f],ap[%.4f],bv[%llu],av[%llu]\n",__LINE__,
    //                 msgdata->securityID,msgdata->lastUpdateTime,msgdata->lastPrice/multi,msgdata->totalLongPosition,msgdata->totalVolumeTraded,msgdata->totalValueTraded/100.0,
    //                 msgdata->askPriceQty[0].price/multi, msgdata->bidPriceQty[0].price/multi,msgdata->askPriceQty[0].qty,msgdata->bidPriceQty[0].qty);




    uint64_t sss    = msgdata->lastUpdateTime%1000L;
    uint64_t hhmmss = msgdata->lastUpdateTime/1000L;
    uint64_t extime = (hhmmss/10000L)*3600+((hhmmss%10000L)/100)*60+(hhmmss%100L);

    int64_t T1 = get_nanoseconds();

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
    m_fb_md->set_dynamic_reference_price(IOPV_TAG_LV1);

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

    int64_t T2 = get_nanoseconds();

    m_fb_spi->on_msg(m_fb_md);
    int64_t T_end = get_nanoseconds();
    
    static uint64_t delay_sum = 0;
    static uint64_t delay_sum_0 = 0; //  -0
    static uint64_t delay_sum_1 = 0; // 0-1
    static uint64_t delay_sum_2 = 0; // 0-1

    static uint64_t count_stats = 0;
    delay_sum += (T_end - T_start);
    delay_sum_0 += (T1 - T_start);
    delay_sum_1 += (T2 - T1);
    delay_sum_2 += (T_end - T2);

    static uint64_t  period = 500;
    count_stats ++;
    if (count_stats%period == 0){
        int cpu = sched_getcpu();
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L1 Option, CPU[%d],per [%llu] ave, all cost[%llu], section read[%llu],fill[%llu],send[%llu]\n",
                    cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period);
        delay_sum = 0;        
        delay_sum_0 = 0;
        delay_sum_1 = 0;
        delay_sum_2 = 0;
    }

};

