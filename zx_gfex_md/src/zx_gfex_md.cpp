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

#include "zx_gfex_md.h"
#include "SockProto.h"

#include "fb_md_type.h"

#include "inst_map.h"

#include "udp_receiver.h"

#include <immintrin.h>

// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    printf("create GFEXQuota[%s]\n");
    return new GFEXQuota();
}
void destroy(void *p) {
    printf("destroy GFEXQuota\n");
    delete (GFEXQuota*)p;
}
void get_md_api_version(char version[32]) {
    printf("[%s][%s]\n",__func__,FEBAO_MD_API_VERSION);
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif


size_t gfex_inst_len(const char* data){
    size_t len = std::strlen(data);
    size_t ret = len;
    if (len<=9) return ret; //期货
    ret = 9;
    for(size_t i = 9; i< len; ++i){
        if ((data[i]>='0') &&  (data[i]<='9')){
            ret ++;
        }
    }
    return ret;
}



std::atomic<bool> g_running = false;


inline bool GFEXQuota::level1_belongTo(uint32_t ip, uint16_t port){
    if((m_config.tcpconfig.net_ip == ip) && (m_config.tcpconfig.net_port == port)){
        return true;
    }
    return false;
}

inline bool GFEXQuota::level2_belongTo(uint32_t ip, uint16_t port){
    if((m_config.multiconfig.net_group_ip == ip) && (m_config.multiconfig.net_group_port == port)){
        return true;
    }
    return false;
}


inline auto MyTransU16(uint16_t a) -> uint16_t
{
    return ((a) >> 8) | ((a) << 8);
}
inline auto MyTransU32(uint32_t a) -> uint32_t
{
    return (a = (((a) >> 16) | ((a) << 16))), (((a) >> 8) & 0x00ff00ff) | (((a) << 8) & 0xff00ff00);
}

inline auto MyTransU64(uint64_t a) -> uint64_t
{
    return (a = (((a) >> 32) | ((a) << 32))), (a = ((((a) >> 16) & 0x0000ffff0000ffff) | (((a) << 16) & 0xffff0000ffff0000))), ((((a) >> 8) & 0x00ff00ff00ff00ff) | (((a) << 8) & 0xff00ff00ff00ff00));
}

GFEXQuota::GFEXQuota(): 
    m_fb_spi(nullptr),
    m_fb_md(cffex::fb::api::market_data_entity::create_entity())
{
    std::clog << std::unitbuf; //调试用
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    m_ringbuffer = new LockFreeRingBuffer();

}


GFEXQuota::~GFEXQuota(){
    std::clog << std::nounitbuf;
}

void GFEXQuota::register_spi(cffex::fb::api::fb_i_md_spi *spi){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    m_fb_spi = spi;
}

int GFEXQuota::init(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    LoadJsonCfg();

    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;
    return 0;
}

void GFEXQuota::release(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    g_running.store(false,std::memory_order_release);

    m_udp_receiver->stop();
    // m_exanic_receiver->stop();
}

void GFEXQuota::connect(){
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;

    // m_dispatcher = new std::thread(&GFEXQuota::ProcessMsg, this, m_config.cpu_id[0]);
    // m_exanic_receiver.reset(new ExanicReceiver(m_config.multiconfig.ifname, m_config.cpu_id[1], 1024)); 
    // m_exanic_receiver->registerCallback((void *)this, ExanicHandler);
    // m_exanic_receiver->start();

    m_udp_receiver.reset(new MulticastReceiver(m_config.multiconfig.group_ip, m_config.multiconfig.group_port, m_config.multiconfig.ifname, m_config.cpu_id[0], 4096)); //因为数据长度是135,所以buffer长度选择256
    m_udp_receiver->registerCallback((void *)this, UdpHandler);
    m_udp_receiver->start();
    sleep(3);  

    g_running.store(true,std::memory_order_release);

    m_fb_spi->on_ready();
}

void GFEXQuota::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id){
    std::clog <<__func__<<","<< __LINE__<< ","<<instrument_id<<","<< exchange_id<< std::endl;

    if (m_inst_set.count(instrument_id)==0){
        m_quota_cache.InitOnce(instrument_id);
        std::clog <<__func__<<","<< __LINE__<<",instrument_id[" <<instrument_id.c_str()<<"],m_quota_cache size:"<<m_quota_cache.size()<< std::endl;        
    }
    m_quota_cache.Sort();
}


inline void GFEXQuota::LoadJsonCfg(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    cffex::fb::api::fb_md_config_helper *parser = m_fb_spi->get_config_helper();
    std::string md_jsoncfgfile;
    parser->get_attribute("file", "/md_config_path", md_jsoncfgfile);
    std::clog <<__func__<<","<< __LINE__<<",md_jsoncfgfile:"<<md_jsoncfgfile.c_str()<< std::endl;
    // cfg
    std::ifstream jsonfile(md_jsoncfgfile);
    nlohmann::json json_parser = nlohmann::json::parse(jsonfile);

    if(json_parser.contains("level2")){
        auto& eth = json_parser["level2"];
        m_config.multiconfig.ifname     = eth["interface_name"];

        m_config.multiconfig.group_ip   = eth["group_ip"].get<std::string>();
        m_config.multiconfig.group_port = eth["group_port"].get<uint16_t>();
        m_config.multiconfig.net_group_ip   = ip_str_to_net_ip(eth["group_ip"].get<std::string>());
        m_config.multiconfig.net_group_port = htons(eth["group_port"].get<uint16_t>());

        std::clog <<__func__<<","<< __LINE__<<",interface_name["<<m_config.multiconfig.ifname<<"]"<< std::endl;
        std::clog <<__func__<<","<< __LINE__<<"],level2 Group[" <<m_config.multiconfig.net_group_ip<<"],port["<<m_config.multiconfig.net_group_port<<"]"<<",["<<eth["group_ip"]<<"],["<<eth["group_port"]<<"]"<< std::endl;
    }

    if(json_parser.contains("level1")){
        auto& eth = json_parser["level1"];
        m_config.tcpconfig.ifname = eth["interface_name"];
        m_config.tcpconfig.ip     = eth["ip"].get<std::string>();
        m_config.tcpconfig.port   = eth["port"].get<uint16_t>();
        m_config.tcpconfig.net_ip   = ip_str_to_net_ip(eth["ip"].get<std::string>());
        m_config.tcpconfig.net_port = htons(eth["port"].get<uint16_t>());
        std::clog <<__func__<<","<< __LINE__<<",interface_name[" <<m_config.tcpconfig.ifname<<"]"<< std::endl;
        std::clog <<__func__<<","<< __LINE__<<"],level1 ip[" <<m_config.tcpconfig.net_ip<<"],port["<<m_config.tcpconfig.net_port<<"]"<<",["<<eth["ip"]<<"],["<<eth["port"]<<"]"<< std::endl;
    }

    auto& worker = json_parser["worker"];
    for (const auto& item : worker["cpuid"]) {
        m_config.cpu_id.emplace_back(item.get<uint32_t>());
        std::clog <<__func__<<","<< __LINE__<<",cpu[" <<m_config.cpu_id.back()<<"]"<< std::endl;        
    }

    auto& filter = json_parser["filter"];

    if(filter.contains("underlying")){
        for (const auto& item : filter["underlying"]) {
            m_inst_set.emplace(item.get<std::string>());
            std::clog <<__func__<<","<< __LINE__<<",m_inst_set underlying[" <<item.get<std::string>()<<"]"<< std::endl;        
        }
    }

    m_config.filter_path = filter["filter_instrument_info"];

    std::unordered_map<std::string, std::string> underlying_inst_map;
    LoadFebaoInstrumentInfo(m_config.filter_path, underlying_inst_map);

    for (auto &itor : underlying_inst_map) {
        std::string underlying = itor.first;
        std::string inst = itor.second;
        m_inst_set.emplace(underlying);
        m_inst_set.emplace(inst);
    }

    for (auto inst : m_inst_set) {
        m_quota_cache.InitOnce(inst);
        std::clog <<__func__<<","<< __LINE__<<",inst[" <<inst.c_str()<<"],m_quota_cache size:"<<m_quota_cache.size()<< std::endl;
    }

    std::clog <<__func__<<","<< __LINE__<<",filter_path[" <<m_config.filter_path<<"]"<< std::endl;
}

void on_exit(int sig){
    g_running.store(false, std::memory_order_release);
}



void GFEXQuota::ProcessMsg(int32_t cpu_id) {
    sleep(2);
    BindCPU(cpu_id);
    Slot slot;
    while (true) {
        if (m_ringbuffer->try_pop(slot)) {
            // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "out [%d],[%d],\n",__LINE__,(int)slot.msg_type);
            DispatchMessage(slot);
        }
    }
}


inline double illegal_price(double lower, double upper,double &price){
    if ((price >= lower) && (price <= upper)){
        return price;
    }
    return FLOAT64_NAN;
}





void GFEXQuota::OnData_lv2(const char* databuffer, size_t size){

    uint64_t local_time_ns = get_nanoseconds();

    // char databuffer[1024];
    // exanic_cycles32_t timestamp;
    // auto sz = exanic_receive_frame(m_exanic_rx, databuffer, sizeof(databuffer), &timestamp);
    // if (sz <= 0){
    //     return;
    // }

    // std::clog <<__func__<<","<< __LINE__<<",sz[" <<sz<<"]" << std::endl;

    size_t offset = sizeof(tcpip::ethhdr) + sizeof(tcpip::ip_header) + sizeof(tcpip::udp_header);
    tcpip::ip_header *ip_ptr = (tcpip::ip_header *)(databuffer + sizeof(tcpip::ethhdr));
    // uint16_t ip_header_data_len = TransU16(ip_ptr->ip_len);

    tcpip::udp_header *udp_ptr = (tcpip::udp_header *)(databuffer + sizeof(tcpip::ethhdr) + sizeof(tcpip::ip_header));
    uint16_t udp_header_data_len = TransU16(udp_ptr->dataLen);

    std::string ip_str = uint32_to_ip_safe(ip_ptr->destination_ip_address);
    // uint16_t dstPort_local = ntohs(udp_ptr->dstPort);

    //std::clog <<__func__<<","<< __LINE__<<",ip_str[" <<ip_str<<"],port:" << dstPort_local <<",data_len:" <<udp_header_data_len<<",offset:" <<offset<< std::endl;

    if (!level2_belongTo(ip_ptr->destination_ip_address, udp_ptr->dstPort)){
        //std::clog <<__func__<<","<< __LINE__<<",ip_str[" <<ip_str<<"],port:" << dstPort_local<<",data_len:" <<udp_header_data_len<<",offset:" <<offset <<",destination_ip_address:" <<ip_ptr->destination_ip_address<< dstPort_local <<",dstPort:" <<udp_ptr->dstPort<< std::endl;
        return;
    }

    if (udp_header_data_len == 684){
        Lv2_OB_Quote *msgdata = (Lv2_OB_Quote *)(databuffer + offset);

        QuotaInfo *cache = m_quota_cache.Find(msgdata->Symbol);
        if(cache == nullptr){
            return;
        }
        else{
            cache->high               = msgdata->HighPrice;
            cache->open               = msgdata->_openPrice;
            cache->low                = msgdata->LowPrice;
            cache->upper              = msgdata->PriceCeil;
            cache->lower              = msgdata->PriceFloor;
            cache->last_openinterest  = msgdata->_lastOpenInterest;
            cache->last_settle_price  = msgdata->_clearPrice;
            cache->settle_price       = msgdata->_lastClear;
        }
        uint64_t extime =  time_str_to_utc_s(msgdata->Time);
        // std::clog <<__func__<<","<< __LINE__<<",Time:"<<msgdata->Time<<",symbol:"<<msgdata->Symbol<<",extime:"<<extime<< std::endl;
        if(extime > (3600*15*1000)) return;  //level2在1500之后的ap bp是错误的，讨论之后决定,15点以后的行情不收了

        
        //俩盘口都不对的时候，认为是非法行情，一般是两边都是空
        if (((msgdata->Ask[0].Price < cache->lower) || (msgdata->Ask[0].Price > cache->upper))
            && ((msgdata->Bid[0].Price < cache->lower) || (msgdata->Bid[0].Price > cache->upper)))
        {
            m_l2_apbp_err_sum++;
            return;
        }


        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "recv [%d],inst[%s],extime[%s],[%.2f],[%d][%d][%.2f],\n",__LINE__,
        //                 msgdata->Symbol,msgdata->Time,msgdata->LastPrice,msgdata->OpenInterest,msgdata->Volume,msgdata->Turnover);

        uint32_t av[5] = {0};
        uint32_t bv[5] = {0};



        uint32_t bid_vol_mask = (msgdata->Bid[0].Volume>0) + (msgdata->Bid[1].Volume>0) + (msgdata->Bid[2].Volume>0) + (msgdata->Bid[3].Volume>0) + (msgdata->Bid[4].Volume>0);
        uint32_t ask_vol_mask = (msgdata->Ask[0].Volume>0) + (msgdata->Ask[1].Volume>0) + (msgdata->Ask[2].Volume>0) + (msgdata->Ask[3].Volume>0) + (msgdata->Ask[4].Volume>0);

        uint32_t bid_imp_vol_mask = (msgdata->Bid[0]._VolumeImpl>0) + (msgdata->Bid[1]._VolumeImpl>0) + (msgdata->Bid[2]._VolumeImpl>0) + (msgdata->Bid[3]._VolumeImpl>0) + (msgdata->Bid[4]._VolumeImpl>0);
        uint32_t ask_imp_vol_mask = (msgdata->Ask[0]._VolumeImpl>0) + (msgdata->Ask[1]._VolumeImpl>0) + (msgdata->Ask[2]._VolumeImpl>0) + (msgdata->Ask[3]._VolumeImpl>0) + (msgdata->Ask[4]._VolumeImpl>0);

        if ((bid_vol_mask<5) && (ask_vol_mask<5) ){

            if (ask_imp_vol_mask ==5){
                av[0] = msgdata->Ask[0]._VolumeImpl;
                av[1] = msgdata->Ask[1]._VolumeImpl;
                av[2] = msgdata->Ask[2]._VolumeImpl;
                av[3] = msgdata->Ask[3]._VolumeImpl;
                av[4] = msgdata->Ask[4]._VolumeImpl;
            }

            if (bid_imp_vol_mask==5){
                bv[0] = msgdata->Bid[0]._VolumeImpl;
                bv[1] = msgdata->Bid[1]._VolumeImpl;
                bv[2] = msgdata->Bid[2]._VolumeImpl;
                bv[3] = msgdata->Bid[3]._VolumeImpl;
                bv[4] = msgdata->Bid[4]._VolumeImpl;
            }
        }
        else if ((msgdata->Bid[0].Volume%3 + msgdata->Bid[1].Volume%3 + msgdata->Bid[2].Volume%3 + msgdata->Bid[3].Volume%3 + msgdata->Bid[4].Volume%3)==0 
            &&(msgdata->Ask[0].Volume%3 + msgdata->Ask[1].Volume%3 + msgdata->Ask[2].Volume%3 + msgdata->Ask[3].Volume%3 + msgdata->Ask[4].Volume%3)==0 
            && bid_imp_vol_mask<5 && ask_imp_vol_mask<5)
        {
            bv[0] = msgdata->Bid[0].Volume/3;
            bv[1] = msgdata->Bid[1].Volume/3;
            bv[2] = msgdata->Bid[2].Volume/3;
            bv[3] = msgdata->Bid[3].Volume/3;
            bv[4] = msgdata->Bid[4].Volume/3;

            av[0] = msgdata->Ask[0].Volume/3;
            av[1] = msgdata->Ask[1].Volume/3;
            av[2] = msgdata->Ask[2].Volume/3;
            av[3] = msgdata->Ask[3].Volume/3;
            av[4] = msgdata->Ask[4].Volume/3;
        }
        else if ((msgdata->Bid[0].Volume > 376 && msgdata->Bid[1].Volume> 376 && msgdata->Bid[2].Volume> 376 && msgdata->Bid[3].Volume> 376 && msgdata->Bid[4].Volume> 376) 
            &&(msgdata->Ask[0].Volume > 376 &&msgdata->Ask[1].Volume > 376 && msgdata->Ask[2].Volume > 376 && msgdata->Ask[3].Volume > 376 && msgdata->Ask[4].Volume > 376)
            && bid_imp_vol_mask<5 && ask_imp_vol_mask<5)
        {   
            bv[0] = msgdata->Bid[0].Volume - 377;
            bv[1] = msgdata->Bid[1].Volume - 377;
            bv[2] = msgdata->Bid[2].Volume - 377;
            bv[3] = msgdata->Bid[3].Volume - 377;
            bv[4] = msgdata->Bid[4].Volume - 377;

            av[0] = msgdata->Ask[0].Volume - 377;
            av[1] = msgdata->Ask[1].Volume - 377;
            av[2] = msgdata->Ask[2].Volume - 377;
            av[3] = msgdata->Ask[3].Volume - 377;
            av[4] = msgdata->Ask[4].Volume - 377;
        }
        else if ((msgdata->Bid[0].Volume > 196 && msgdata->Bid[1].Volume> 196 && msgdata->Bid[2].Volume> 196 && msgdata->Bid[3].Volume> 196 && msgdata->Bid[4].Volume> 196) 
            &&(msgdata->Ask[0].Volume> 196 && msgdata->Ask[1].Volume> 196 && msgdata->Ask[2].Volume> 196 && msgdata->Ask[3].Volume> 196 && msgdata->Ask[4].Volume> 196)
            && bid_imp_vol_mask<5 && ask_imp_vol_mask<5)
        {   
            bv[0] = msgdata->Bid[0].Volume - 197;
            bv[1] = msgdata->Bid[1].Volume - 197;
            bv[2] = msgdata->Bid[2].Volume - 197;
            bv[3] = msgdata->Bid[3].Volume - 197;
            bv[4] = msgdata->Bid[4].Volume - 197;

            av[0] = msgdata->Ask[0].Volume - 197;
            av[1] = msgdata->Ask[1].Volume - 197;
            av[2] = msgdata->Ask[2].Volume - 197;
            av[3] = msgdata->Ask[3].Volume - 197;
            av[4] = msgdata->Ask[4].Volume - 197;
        }
        else if ((msgdata->Bid[0].Volume%2 + msgdata->Bid[1].Volume%2 + msgdata->Bid[2].Volume%2 + msgdata->Bid[3].Volume%2 + msgdata->Bid[4].Volume%2)==0
            &&(msgdata->Ask[0].Volume%2 + msgdata->Ask[1].Volume%2 + msgdata->Ask[2].Volume%2 + msgdata->Ask[3].Volume%2 + msgdata->Ask[4].Volume%2)==0 
            && bid_imp_vol_mask<5 && ask_imp_vol_mask<5)
        {   
            bv[0] = msgdata->Bid[0].Volume/2;
            bv[1] = msgdata->Bid[1].Volume/2;
            bv[2] = msgdata->Bid[2].Volume/2;
            bv[3] = msgdata->Bid[3].Volume/2;
            bv[4] = msgdata->Bid[4].Volume/2;

            av[0] = msgdata->Ask[0].Volume/2;
            av[1] = msgdata->Ask[1].Volume/2;
            av[2] = msgdata->Ask[2].Volume/2;
            av[3] = msgdata->Ask[3].Volume/2;
            av[4] = msgdata->Ask[4].Volume/2;
        }


        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "before [%d],inst[%s],extime[%llu],[%.2f],[%d][%d][%.2f],bp[%.2f][%.2f][%.2f][%.2f][%.2f],ap[%.2f][%.2f][%.2f][%.2f][%.2f],bv[%d][%d][%d][%d][%d],bv_imp[%d][%d][%d][%d][%d],av[%d][%d][%d][%d][%d],av_imp[%d][%d][%d][%d][%d],[%d][%d][%d][%d][%d],[%d][%d][%d][%d][%d]\n",__LINE__,
        //                 msgdata->Symbol,extime,msgdata->LastPrice,msgdata->OpenInterest,msgdata->Volume,msgdata->Turnover,
        //                 msgdata->Bid[0].Price,msgdata->Bid[1].Price,msgdata->Bid[2].Price,msgdata->Bid[3].Price,msgdata->Bid[4].Price,
        //                 msgdata->Ask[0].Price,msgdata->Ask[1].Price,msgdata->Ask[2].Price,msgdata->Ask[3].Price,msgdata->Ask[4].Price,
        //                 msgdata->Bid[0].Volume,msgdata->Bid[1].Volume,msgdata->Bid[2].Volume,msgdata->Bid[3].Volume,msgdata->Bid[4].Volume,
        //                 msgdata->Bid[0]._VolumeImpl,msgdata->Bid[1]._VolumeImpl,msgdata->Bid[2]._VolumeImpl,msgdata->Bid[3]._VolumeImpl,msgdata->Bid[4]._VolumeImpl,
        //                 msgdata->Ask[0].Volume,msgdata->Ask[1].Volume,msgdata->Ask[2].Volume,msgdata->Ask[3].Volume,msgdata->Ask[4].Volume,
        //                 msgdata->Ask[0]._VolumeImpl,msgdata->Ask[1]._VolumeImpl,msgdata->Ask[2]._VolumeImpl,msgdata->Ask[3]._VolumeImpl,msgdata->Ask[4]._VolumeImpl,
        //                 bv[0],bv[1],bv[2],bv[3],bv[4],av[0],av[1],av[2],av[3],av[4]);


        uint32_t bid_mask = (bv[0]>0) + (bv[1]>0) + (bv[2]>0) + (bv[3]>0) + (bv[4]>0);
        uint32_t ask_mask = (av[0]>0) + (av[1]>0) + (av[2]>0) + (av[3]>0) + (av[4]>0);
        uint32_t valid = IsValidQuota(cache, extime, msgdata->Volume, 5, msgdata->Ask[0].Price, msgdata->Bid[0].Price, av[0], bv[0]);
        if (valid == COM_RESULT_TIME_ERROR){
            m_l2_time_err_sum ++;
            return;
        }
        else if(valid==COM_RESULT_VOL_ERROR){
            m_l2_vol_err_sum ++;
            return;
        }

        DataQuote quota;
        quota.extime = extime;
        size_t sym_len = gfex_inst_len(msgdata->Symbol);
        strncpy(quota.inst, msgdata->Symbol, sym_len);
        quota.inst[sym_len] = '\0';
        quota.last_price = msgdata->LastPrice;
        quota.openinterest = msgdata->OpenInterest;
        quota.volume = msgdata->Volume;
        quota.turnover = msgdata->Turnover;


        uint32_t up_or_down_limit = 0;
        if (unlikely(((msgdata->Ask[0].Price < cache->lower) || (msgdata->Ask[0].Price > cache->upper))
            && ((msgdata->Ask[1].Price < cache->lower) || (msgdata->Ask[1].Price > cache->upper))
            && ((msgdata->Ask[2].Price < cache->lower) || (msgdata->Ask[2].Price > cache->upper))
            && ((msgdata->Ask[3].Price < cache->lower) || (msgdata->Ask[3].Price > cache->upper))
            && ((msgdata->Ask[4].Price < cache->lower) || (msgdata->Ask[4].Price > cache->upper))))
        {
            //广期所数据,跌停的时候 ap全是错误价格,此时ap的数据在bp上,av的数据在bv上;涨停则反之
            up_or_down_limit = PRICE_DOWN_LIMIT; 
            quota.ap1 = illegal_price(cache->lower,cache->upper, msgdata->Bid[0].Price);
            quota.ap2 = illegal_price(cache->lower,cache->upper, msgdata->Bid[1].Price);
            quota.ap3 = illegal_price(cache->lower,cache->upper, msgdata->Bid[2].Price);
            quota.ap4 = illegal_price(cache->lower,cache->upper, msgdata->Bid[3].Price);
            quota.ap5 = illegal_price(cache->lower,cache->upper, msgdata->Bid[4].Price);
            quota.av1 = bv[0];
            quota.av2 = bv[1];
            quota.av3 = bv[2];
            quota.av4 = bv[3];
            quota.av5 = bv[4];
        }
        else if(unlikely(((msgdata->Bid[0].Price < cache->lower) || (msgdata->Bid[0].Price > cache->upper))
            && ((msgdata->Bid[1].Price < cache->lower) || (msgdata->Bid[1].Price > cache->upper))
            && ((msgdata->Bid[2].Price < cache->lower) || (msgdata->Bid[2].Price > cache->upper))
            && ((msgdata->Bid[3].Price < cache->lower) || (msgdata->Bid[3].Price > cache->upper))
            && ((msgdata->Bid[4].Price < cache->lower) || (msgdata->Bid[4].Price > cache->upper))))
        {
            up_or_down_limit = PRICE_UP_LIMIT;

            quota.bp1 = illegal_price(cache->lower,cache->upper, msgdata->Ask[0].Price);
            quota.bp2 = illegal_price(cache->lower,cache->upper, msgdata->Ask[1].Price);
            quota.bp3 = illegal_price(cache->lower,cache->upper, msgdata->Ask[2].Price);
            quota.bp4 = illegal_price(cache->lower,cache->upper, msgdata->Ask[3].Price);
            quota.bp5 = illegal_price(cache->lower,cache->upper, msgdata->Ask[4].Price);
            quota.bv1 = av[0];
            quota.bv2 = av[1];
            quota.bv3 = av[2];
            quota.bv4 = av[3];
            quota.bv5 = av[4];

        }
        else{

            quota.bp1 = illegal_price(cache->lower,cache->upper, msgdata->Bid[0].Price);
            quota.bp2 = illegal_price(cache->lower,cache->upper, msgdata->Bid[1].Price);
            quota.bp3 = illegal_price(cache->lower,cache->upper, msgdata->Bid[2].Price);
            quota.bp4 = illegal_price(cache->lower,cache->upper, msgdata->Bid[3].Price);
            quota.bp5 = illegal_price(cache->lower,cache->upper, msgdata->Bid[4].Price);
            quota.ap1 = illegal_price(cache->lower,cache->upper, msgdata->Ask[0].Price);
            quota.ap2 = illegal_price(cache->lower,cache->upper, msgdata->Ask[1].Price);
            quota.ap3 = illegal_price(cache->lower,cache->upper, msgdata->Ask[2].Price);
            quota.ap4 = illegal_price(cache->lower,cache->upper, msgdata->Ask[3].Price);
            quota.ap5 = illegal_price(cache->lower,cache->upper, msgdata->Ask[4].Price);
            quota.bv1 = bv[0];
            quota.bv2 = bv[1];
            quota.bv3 = bv[2];
            quota.bv4 = bv[3];
            quota.bv5 = bv[4];
            quota.av1 = av[0];
            quota.av2 = av[1];
            quota.av3 = av[2];
            quota.av4 = av[3];
            quota.av5 = av[4];
        }

        if ((av[0] == 0) && (bv[0] == 0))
        {
            m_l2_apbp_err_sum++;
            return;
        }

        uint64_t local_time_ns_0 = get_nanoseconds();


        m_ringbuffer->push_l2(quota, extime, local_time_ns, local_time_ns_0, valid);


        // if (bid_mask<5 || ask_mask<5){

        //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "fill [%d],inst[%s],extime[%llu],[%.2f],[%d][%d][%.2f],bp[%.2f][%.2f][%.2f][%.2f][%.2f],ap[%.2f][%.2f][%.2f][%.2f][%.2f],bv[%d][%d][%d][%d][%d],av[%d][%d][%d][%d][%d]\n",__LINE__,
        //                     quote.inst,quote.extime,quote.last_price,quote.openinterest,quote.volume,quote.turnover,
        //                     quote.bp1,quote.bp2,quote.bp3,quote.bp4,quote.bp5,
        //                     quote.ap1,quote.ap2,quote.ap3,quote.ap4,quote.ap5,
        //                     quote.bv1,quote.bv2,quote.bv3,quote.bv4,quote.bv5,
        //                     quote.av1,quote.av2,quote.av3,quote.av4,quote.av5);


        // }



    }
}


inline bool PRICE_COM(double a, double b){
    return std::fabs(a-b) < 1e-8;
}

inline int32_t GFEXQuota::IsValidQuota(QuotaInfo *cache, uint64_t extime, uint64_t vol, uint32_t level, double ap1, double bp1, uint32_t av1, uint32_t bv1){
    int32_t ret = COM_RESULT_OK;

    if (cache->level != 0){
        if (extime < cache->time){
            return COM_RESULT_TIME_ERROR;
        }

        if(vol < cache->volume){
            return COM_RESULT_VOL_ERROR;
        }

        if(((cache->level == 5) && (level == 1)) && (vol == cache->volume)
            && (cache->av1 == av1) && (cache->bv1 == bv1)
            && PRICE_COM(cache->ap1, ap1) && PRICE_COM(cache->bp1, bp1)){
                return COM_RESULT_REPEAT_L1;
        }
    }

    cache->time   = extime;
    cache->volume = vol;
    cache->level  = level;
    cache->ap1    = ap1;
    cache->bp1    = bp1;
    cache->av1    = av1;
    cache->bv1    = bv1;
    return ret;
}

void GFEXQuota::UdpHandler(void *ctx, const char* data, size_t size){

    // hexDumpToClog(data, size);

    GFEXQuota *pthis = (GFEXQuota *)ctx;


    pthis->DecodeMsg(data, size);
}





void GFEXQuota::DecodeMsg(const char* data, int32_t data_len)
{
	int32_t iProcSize = 0;
	GFEX::Zx_PacketHead* pPacketHead = (GFEX::Zx_PacketHead*)data;

    // std::clog <<__func__<<","<< __LINE__<<", data_len:"<<data_len<<",pPacketHead->iPktSize:"<<pPacketHead->iPktSize<<",m_iLen_packetHead:"<<m_iLen_packetHead<< std::endl;

	iProcSize += m_iLen_packetHead;
	if (data_len != pPacketHead->iPktSize){
        std::clog <<__func__<<","<< __LINE__<<",error, data_len:"<<data_len<<",pPacketHead->iPktSize:"<<pPacketHead->iPktSize<< std::endl;
    }

	while (true)
	{
		if (data_len - (iProcSize + m_iLen_msgHead) <= 0)
		{
			break;
		}
		GFEX::Zx_MsgHead* pMsgHead = (GFEX::Zx_MsgHead*)(data + iProcSize);
		iProcSize += m_iLen_msgHead;

        // std::clog <<__func__<<","<< __LINE__<<", m_iLen_msgHead:"<<m_iLen_msgHead<<",iProcSize:"<<iProcSize<<",m_iLen_packetHead:"<<m_iLen_packetHead<< std::endl;

        hexDumpToClog((char *)pMsgHead, pMsgHead->iMsgSize);
		switch (pMsgHead->iMsgType)
		{
            case GFEX::MsgZx_BestDeep:
            {
                GFEX::Zx_BestDeep* msgdata = (GFEX::Zx_BestDeep*)(data + iProcSize);
                uint64_t extime =  time_str_to_utc_s(msgdata->arrGenTime);
                std::clog <<__func__<<","<< __LINE__<<",Time:"<<msgdata->arrGenTime<<",symbol:"<<msgdata->arrInstrument<<",extime:"<<extime<< std::endl;
                if(extime > (3600*15*1000)) return;  //level2在1500之后的ap bp是错误的，讨论之后决定,15点以后的行情不收了
                int64_t local_time_ns = get_nanoseconds();
  
                m_fb_md->reset_entity();// 重置飞豹行情结构
                m_fb_md->set_local_timestamp(local_time_ns); //utc ns
                m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
                m_fb_md->set_instrument_id(msgdata->arrInstrument);
                m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_GFEX);
                m_fb_md->set_update_sec(extime/1000);
                m_fb_md->set_update_msec(extime%1000);
                m_fb_md->set_open(convert_price_if_zero(msgdata->dOpenPrice));
                m_fb_md->set_close(convert_price_if_zero(msgdata->dClosePrice));

                m_fb_md->set_high_price(convert_price_if_zero(msgdata->dHighPrice));
                m_fb_md->set_low_price(convert_price_if_zero(msgdata->dLowPrice));
                m_fb_md->set_last_price(convert_price_if_zero(msgdata->dLastPrice));

                m_fb_md->set_upper_limit_price(convert_price_if_zero(msgdata->dRiseLimit));
                m_fb_md->set_down_limit_price(convert_price_if_zero(msgdata->dFallLimit));
                m_fb_md->set_pre_open_interest(convert_price_if_zero(msgdata->iInitOpenInterest));
                m_fb_md->set_pre_settlement(convert_price_if_zero(msgdata->dLastClearPrice));

                m_fb_md->set_volume(msgdata->iMatchTotQty);
                m_fb_md->set_turn_over(msgdata->dTurnover);
                m_fb_md->set_open_interest(msgdata->iOpenInterest);

                m_fb_md->set_max_depth(5);                              // 设置行情深度5
                m_fb_md->set_iopv(IOPV_TAG_LV2);

 
                // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "send [%d],inst[%s],extime[%s][%llu],[%.2f],[%d][%d][%.2f],bp[%.2f][%.2f][%.2f][%.2f][%.2f],ap[%.2f][%.2f][%.2f][%.2f][%.2f],bv[%d][%d][%d][%d][%d],av[%d][%d][%d][%d][%d],bv_imp[%d][%d][%d][%d][%d],av_imp[%d][%d][%d][%d][%d]\n",__LINE__,
                //                 msgdata->arrInstrument,msgdata->arrGenTime,extime,msgdata->dLastPrice,msgdata->iOpenInterest,msgdata->iMatchTotQty,msgdata->dTurnover,
                //                 msgdata->dBidPrice1,msgdata->dBidPrice2,msgdata->dBidPrice3,msgdata->dBidPrice4,msgdata->dBidPrice5,
                //                 msgdata->dAskPrice1,msgdata->dAskPrice2,msgdata->dAskPrice3,msgdata->dAskPrice4,msgdata->dAskPrice5,
                //                 msgdata->iBidVolume1,msgdata->iBidVolume2,msgdata->iBidVolume3,msgdata->iBidVolume4,msgdata->iBidVolume5,
                //                 msgdata->iAskVolume1,msgdata->iAskVolume2,msgdata->iAskVolume3,msgdata->iAskVolume4,msgdata->iAskVolume5,
                //                 msgdata->iBidImplyVolume1,msgdata->iBidImplyVolume2,msgdata->iBidImplyVolume3,msgdata->iBidImplyVolume4,msgdata->iBidImplyVolume5,
                //                 msgdata->iAskImplyVolume1,msgdata->iAskImplyVolume2,msgdata->iAskImplyVolume3,msgdata->iAskImplyVolume4,msgdata->iAskImplyVolume5);



                m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->dAskPrice1,msgdata->iAskVolume1));
                m_fb_md->set_ask2_price(convert_price_if_volume_zero(msgdata->dAskPrice2,msgdata->iAskVolume2));
                m_fb_md->set_ask3_price(convert_price_if_volume_zero(msgdata->dAskPrice3,msgdata->iAskVolume3));
                m_fb_md->set_ask4_price(convert_price_if_volume_zero(msgdata->dAskPrice4,msgdata->iAskVolume4));
                m_fb_md->set_ask5_price(convert_price_if_volume_zero(msgdata->dAskPrice5,msgdata->iAskVolume5));
                m_fb_md->set_ask1_volume(msgdata->iAskVolume1);
                m_fb_md->set_ask2_volume(msgdata->iAskVolume2);
                m_fb_md->set_ask3_volume(msgdata->iAskVolume3);
                m_fb_md->set_ask4_volume(msgdata->iAskVolume4);
                m_fb_md->set_ask5_volume(msgdata->iAskVolume5);
                m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->dBidPrice1, msgdata->iBidVolume1));
                m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata->dBidPrice2, msgdata->iBidVolume2));
                m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata->dBidPrice3, msgdata->iBidVolume3));
                m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata->dBidPrice4, msgdata->iBidVolume4));
                m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata->dBidPrice5, msgdata->iBidVolume5));
                m_fb_md->set_bid1_volume(msgdata->iBidVolume1);
                m_fb_md->set_bid2_volume(msgdata->iBidVolume2);
                m_fb_md->set_bid3_volume(msgdata->iBidVolume3);
                m_fb_md->set_bid4_volume(msgdata->iBidVolume4);
                m_fb_md->set_bid5_volume(msgdata->iBidVolume5);

                m_fb_spi->on_msg(m_fb_md);

                break;
		    }
		    case GFEX::MsgZx_ArbBestDeep:
		    {

                GFEX::Zx_ArbBestDeep* msgdata = (GFEX::Zx_ArbBestDeep*)(data + iProcSize);
                // uint64_t extime =  time_str_to_utc_s(msgdata->arrGenTime);
                // // std::clog <<__func__<<","<< __LINE__<<",Time:"<<msgdata->arrGenTime<<",symbol:"<<msgdata->arrInstrument<<",extime:"<<extime<< std::endl;
                // if(extime > (3600*15*1000)) return;  //level2在1500之后的ap bp是错误的，讨论之后决定,15点以后的行情不收了

			    break;
		    }
            case GFEX::MsgZx_TenEntrust:
            case GFEX::MsgZx_Real:
            case GFEX::MsgZx_OrderStatistic:
            case GFEX::MsgZx_MatchPriceQty:
            {
                // std::clog <<__func__<<","<< __LINE__<<",MsgType:"<<pMsgHead->iMsgType<<",data_len:"<<data_len<< std::endl;
                break;
            }
            default:{
                std::cout << "abnormal msgType:" << pMsgHead->iMsgType << std::endl;
                assert(0);
                break;
            }
	    }
		iProcSize += pMsgHead->iMsgSize;
    }
}









// void GFEXQuota::ExanicHandler(void *ctx, const char* data, size_t size){
//     // std::clog <<__func__<<","<< __LINE__<<",ctx:"<<ctx<<",data:"<<data<<",size:"<<size<< std::endl;
//     GFEXQuota *pthis = (GFEXQuota *)ctx;
//     pthis->OnData_lv2(data, size);
// }




void GFEXQuota::OnData_lv1(const char* data, size_t size){
    // std::clog <<__func__<<","<< __LINE__<< ",sz:"<<sizeof(MarketDataField)<< ",size:"<<size<<",source_ip:"<< source_ip.c_str()<<",source_port:"<<source_port<< std::endl;
    int64_t local_time_ns = get_nanoseconds();
    MarketDataField *msgdata = (MarketDataField *)data;

    std::string inst(msgdata->instrumentID);

    QuotaInfo *cache = m_quota_cache.Find(msgdata->instrumentID);
    if (cache == nullptr){
        return;
    }

    if (unlikely((!(cache->open > 0)) && msgdata->latestPrice != DBL_MAX && msgdata->latestPrice >0)){
        cache->open = msgdata->latestPrice;
    }

    // 94038398
    uint64_t hh = msgdata->updateTime/10000000L;
    uint64_t mm = (msgdata->updateTime/100000L)%100;
    uint64_t ss = (msgdata->updateTime/1000L)%100;

    uint64_t extime = (hh*3600L + mm*60 + ss)*1000  + msgdata->updateTime%1000;

    // std::clog <<__func__<<","<< __LINE__<<",Time:"<<msgdata->updateTime<<",symbol:"<<msgdata->instrumentID<<",extime:"<<extime<< std::endl;

    uint32_t valid = IsValidQuota(cache, extime, msgdata->matchAmount, 1,msgdata->sellPrice1, msgdata->buyPrice1, msgdata->sellAmount1,msgdata->buyAmount1);

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "fill [%d],valid[%d],inst[%s],extime[%llu],[%.2f],[%d][%d][%.2f],bp[%.2f],ap[%.2f],bv[%d],av[%d]\n",__LINE__,
    //     valid,msgdata->instrumentID,extime,msgdata->latestPrice,msgdata->positionAmount,msgdata->matchAmount,msgdata->macthMoney,
    //     msgdata->buyPrice1,msgdata->sellPrice1,msgdata->buyAmount1,msgdata->sellAmount1);

    if (valid==COM_RESULT_TIME_ERROR){
        m_l1_time_err_sum ++;
        return;
    }
    else if(valid==COM_RESULT_VOL_ERROR){
        m_l1_vol_err_sum ++;
        return;
    }
    else if(valid==COM_RESULT_REPEAT_L1){
        m_l1_useless_lv1_sum ++;
        return;
    }

    uint64_t local_time_ns_0 = get_nanoseconds();

    // m_ringbuffer->push_l1(quote, extime, local_time_ns, local_time_ns_0,0);
    m_ringbuffer->push_l1(DataQuote(extime,msgdata->instrumentID,gfex_inst_len(msgdata->instrumentID),msgdata->latestPrice,msgdata->positionAmount,msgdata->matchAmount,msgdata->macthMoney,
                                    msgdata->buyPrice1,msgdata->sellPrice1,msgdata->buyAmount1,msgdata->sellAmount1), extime, local_time_ns, local_time_ns_0,0);


}


void GFEXQuota::DispatchMessage(const Slot& slot) {

    int64_t local_time_ns_1 = get_nanoseconds();


    const DataQuote &msgdata = slot.data;

    QuotaInfo *cache = m_quota_cache.Find(msgdata.inst);
    if (cache==nullptr){
        std::clog <<__func__<<","<< __LINE__<<",error:symbol:"<<msgdata.inst<< std::endl;
        return;
    }


    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "fill [%d],inst[%s],extime[%llu],cache[%p]\n",__LINE__, msgdata.inst, slot.extime,cache);

    m_fb_md->reset_entity();// 重置飞豹行情结构
    m_fb_md->set_local_timestamp(slot.timestamp_0); //utc ns
    m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
    m_fb_md->set_instrument_id(msgdata.inst);
    m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_GFEX);
    m_fb_md->set_update_sec(slot.extime/1000);
    m_fb_md->set_update_msec(slot.extime%1000);
    m_fb_md->set_open(convert_price_if_zero(cache->open));
    m_fb_md->set_close(convert_price_if_zero(msgdata.last_price));

    m_fb_md->set_high_price(convert_price_if_zero(cache->high));
    m_fb_md->set_low_price(convert_price_if_zero(cache->low));
    m_fb_md->set_last_price(convert_price_if_zero(msgdata.last_price));

    m_fb_md->set_upper_limit_price(convert_price_if_zero(cache->upper));
    m_fb_md->set_down_limit_price(convert_price_if_zero(cache->lower));
    m_fb_md->set_pre_open_interest(convert_price_if_zero(cache->last_openinterest));
    m_fb_md->set_pre_settlement(convert_price_if_zero(cache->last_settle_price));

    m_fb_md->set_volume(msgdata.volume);
    m_fb_md->set_turn_over(msgdata.turnover);
    m_fb_md->set_open_interest(msgdata.openinterest);

    if (slot.msg_type == L1_MD) {

        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "send [%d],inst[%s],extime[%llu],[%.2f],[%d][%d][%.2f],bp[%.2f],ap[%.2f],bv[%d],av[%d]\n",__LINE__,
        //                 msgdata.inst,msgdata.extime,msgdata.last_price,msgdata.openinterest,msgdata.volume,msgdata.turnover,
        //                 msgdata.bp1, msgdata.ap1,msgdata.bv1,msgdata.av1);

        m_fb_md->set_max_depth(1);                              // 设置行情深度5
        m_fb_md->set_iopv(IOPV_TAG_LV1);

        m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata.ap1,msgdata.av1));
        m_fb_md->set_ask1_volume(msgdata.av1);
        m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata.bp1,msgdata.bv1));
        m_fb_md->set_bid1_volume(msgdata.bv1);

        int64_t local_time_ns_2 = get_nanoseconds();

        m_fb_spi->on_msg(m_fb_md);
        int64_t local_time_ns_end = get_nanoseconds();
        
        static uint64_t delay_sum = 0;
        static uint64_t delay_sum_0 = 0; //  -0
        static uint64_t delay_sum_1 = 0; // 0-1
        static uint64_t delay_sum_2 = 0; // 1-2
        static uint64_t delay_sum_3 = 0; // 2-end

        static uint64_t count_stats = 0;
        delay_sum   += (local_time_ns_end - slot.timestamp_0);
        delay_sum_0 += (slot.timestamp_1 - slot.timestamp_0);
        delay_sum_1 += (local_time_ns_1 - slot.timestamp_1);
        delay_sum_2 += (local_time_ns_2 - local_time_ns_1);
        delay_sum_3 += (local_time_ns_end - local_time_ns_2);

        static uint64_t  period = 1000;
        count_stats ++;
        if (count_stats%period == 0){
            int cpu = sched_getcpu();
            m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L1, CPU[%d],per [%llu] ave, all cost[%llu], section filter[%llu],process[%llu],fill[%llu],send[%llu],sum[%llu],timeerr[%zu],volerr[%zu],useless_lv1[%zu]\n",cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period,count_stats, m_l1_time_err_sum, m_l1_vol_err_sum, m_l1_useless_lv1_sum);
            delay_sum   = 0;        
            delay_sum_0 = 0;
            delay_sum_1 = 0;
            delay_sum_2 = 0;
            delay_sum_3 = 0;
        }

        // if ((slot.timestamp_1 - slot.timestamp_0) > 100000 || (local_time_ns_1 - slot.timestamp_1) > 100000 || (local_time_ns_2 - local_time_ns_1) > 100000){
        //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L1, filter[%llu],process[%llu],fill[%llu]\n",slot.timestamp_1 - slot.timestamp_0, local_time_ns_1 - slot.timestamp_1, local_time_ns_2 - local_time_ns_1);
        // }
    } else if(slot.msg_type == L2_MD) {
        m_fb_md->set_max_depth(5);                              // 设置行情深度5
        m_fb_md->set_iopv(IOPV_TAG_LV2);

        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "send [%d],inst[%s],extime[%llu],[%.2f],[%d][%d][%.2f],bp[%.2f][%.2f][%.2f][%.2f][%.2f],ap[%.2f][%.2f][%.2f][%.2f][%.2f],bv[%d][%d][%d][%d][%d],av[%d][%d][%d][%d][%d]\n",__LINE__,
        //                 msgdata.inst,msgdata.extime,msgdata.last_price,msgdata.openinterest,msgdata.volume,msgdata.turnover,
        //                 msgdata.bp1,msgdata.bp2,msgdata.bp3,msgdata.bp4,msgdata.bp5,
        //                 msgdata.ap1,msgdata.ap2,msgdata.ap3,msgdata.ap4,msgdata.ap5,
        //                 msgdata.bv1,msgdata.bv2,msgdata.bv3,msgdata.bv4,msgdata.bv5,
        //                 msgdata.av1,msgdata.av2,msgdata.av3,msgdata.av4,msgdata.av5);

        m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata.ap1,msgdata.av1));
        m_fb_md->set_ask2_price(convert_price_if_volume_zero(msgdata.ap2,msgdata.av2));
        m_fb_md->set_ask3_price(convert_price_if_volume_zero(msgdata.ap3,msgdata.av3));
        m_fb_md->set_ask4_price(convert_price_if_volume_zero(msgdata.ap4,msgdata.av4));
        m_fb_md->set_ask5_price(convert_price_if_volume_zero(msgdata.ap5,msgdata.av5));
        m_fb_md->set_ask1_volume(msgdata.av1);
        m_fb_md->set_ask2_volume(msgdata.av2);
        m_fb_md->set_ask3_volume(msgdata.av3);
        m_fb_md->set_ask4_volume(msgdata.av4);
        m_fb_md->set_ask5_volume(msgdata.av5);
        m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata.bp1, msgdata.bv1));
        m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata.bp2, msgdata.bv2));
        m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata.bp3, msgdata.bv3));
        m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata.bp4, msgdata.bv4));
        m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata.bp5, msgdata.bv5));
        m_fb_md->set_bid1_volume(msgdata.bv1);
        m_fb_md->set_bid2_volume(msgdata.bv2);
        m_fb_md->set_bid3_volume(msgdata.bv3);
        m_fb_md->set_bid4_volume(msgdata.bv4);
        m_fb_md->set_bid5_volume(msgdata.bv5);

        int64_t local_time_ns_2 = get_nanoseconds();

        m_fb_spi->on_msg(m_fb_md);
        int64_t local_time_ns_end = get_nanoseconds();
        
        static uint64_t delay_sum = 0;
        static uint64_t delay_sum_0 = 0; //  -0
        static uint64_t delay_sum_1 = 0; // 0-1
        static uint64_t delay_sum_2 = 0; // 1-2
        static uint64_t delay_sum_3 = 0; // 2-end

        static uint64_t count_stats = 0;
        delay_sum += (local_time_ns_end - slot.timestamp_0);
        delay_sum_0 += (slot.timestamp_1 - slot.timestamp_0);
        delay_sum_1 += (local_time_ns_1 - slot.timestamp_1);
        delay_sum_2 += (local_time_ns_2 - local_time_ns_1);
        delay_sum_3 += (local_time_ns_end - local_time_ns_2);

       
        static uint64_t  period = 2000;
        count_stats ++;
        if (count_stats%period == 0){
            int cpu = sched_getcpu();
            m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L2, CPU[%d],per [%llu] ave, all cost[%llu], section filter[%llu],process[%llu],fill[%llu],send[%llu],sum[%llu],timeerr[%zu],volerr[%zu],l2_apbp_err[%llu]\n",
                        cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period, count_stats, m_l2_time_err_sum, m_l2_vol_err_sum,m_l2_apbp_err_sum);
            delay_sum = 0;        
            delay_sum_0 = 0;
            delay_sum_1 = 0;
            delay_sum_2 = 0;
            delay_sum_3 = 0;

        }



        // if ((slot.timestamp_1 - slot.timestamp_0) > 100000 || (local_time_ns_1 - slot.timestamp_1) > 100000 || (local_time_ns_2 - local_time_ns_1) > 100000){
        //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L2, filter[%llu],process[%llu],fill[%llu]\n",slot.timestamp_1 - slot.timestamp_0, local_time_ns_1 - slot.timestamp_1, local_time_ns_2 - local_time_ns_1);
        // }


    }
}



