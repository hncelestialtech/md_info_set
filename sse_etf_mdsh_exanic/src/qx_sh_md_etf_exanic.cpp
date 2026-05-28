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

#include "qx_sh_md_etf_exanic.h"
#include "SockProto.h"

#include "fb_md_type.h"



// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    printf("create QxShMdEtfOptionExanic[%s]\n");
    return new QxShMdEtfOptionExanic();
}
void destroy(void *p) {
    printf("destroy QxShMdEtfOptionExanic\n");
    delete (QxShMdEtfOptionExanic*)p;
}
void get_md_api_version(char version[32]) {
    printf("[%s][%s]\n",__func__,FEBAO_MD_API_VERSION);
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif

std::atomic<bool> g_running = false;


inline bool QxShMdEtfOptionExanic::belongTo(uint32_t ip, uint16_t port){
    for(auto & it : m_config.multicast){
        if((it.ip == ip) && (it.port == port)){
            return true;
        }
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











QxShMdEtfOptionExanic::QxShMdEtfOptionExanic(): 
    m_fb_initialized(false),
    m_exanic_initialized(false),
    m_fb_spi(nullptr),
    m_fb_md(cffex::fb::api::market_data_entity::create_entity())
{
    m_TS.reset(new NanoStamp());
    std::clog << std::unitbuf; //调试用

    // std::string testfile = "/home/febao/shenzx/test.csv";  // TODO stub
    // m_quota_stub.Load(testfile);   // TODO stub
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
}




QxShMdEtfOptionExanic::~QxShMdEtfOptionExanic(){
    std::clog << std::nounitbuf;
}

void QxShMdEtfOptionExanic::register_spi(cffex::fb::api::fb_i_md_spi *spi){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    m_fb_spi = spi;
}



inline void QxShMdEtfOptionExanic::LoadJsonCfg(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    cffex::fb::api::fb_md_config_helper *parser = m_fb_spi->get_config_helper();
    std::string md_jsoncfgfile;
    parser->get_attribute("file", "/md_config_path", md_jsoncfgfile);
    std::clog <<__func__<<","<< __LINE__<<",md_jsoncfgfile[" <<md_jsoncfgfile<<"]"<< std::endl;
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

}

void on_exit(int sig){
    g_running.store(false, std::memory_order_release);
}

bool QxShMdEtfOptionExanic::InitExanic(const char *ifName){
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

void QxShMdEtfOptionExanic::ReleaseExanic(){
    exanic_release_rx_buffer(m_exanic_rx);
    exanic_release_handle(m_exanic);
}



void QxShMdEtfOptionExanic::OnData(){
    uint64_t local_time_ns_start = get_nanoseconds();
    char databuffer[1024];
    exanic_cycles32_t timestamp;
    auto sz = exanic_receive_frame(m_exanic_rx, databuffer, sizeof(databuffer), &timestamp);
    if (sz <= 0){
        return;
    }

    uint64_t local_time_ns_0 = get_nanoseconds();

    tcpip::ip_header *ip_ptr = (tcpip::ip_header *)(databuffer + sizeof(tcpip::ethhdr));
    // uint16_t ip_header_data_len = TransU16(ip_ptr->ip_len);

    tcpip::udp_header *udp_ptr = (tcpip::udp_header *)(databuffer + sizeof(tcpip::ethhdr) + sizeof(tcpip::ip_header));
    // uint16_t udp_header_data_len = TransU16(udp_ptr->dataLen);

    // std::string ip_str = uint32_to_ip_safe(ip_ptr->destination_ip_address);
    // uint16_t dstPort_local = ntohs(udp_ptr->dstPort);

    // std::clog <<__func__<<","<< __LINE__<<",ip_str[" <<ip_str<<"],port:" << dstPort_local << std::endl;

    if (!belongTo(ip_ptr->destination_ip_address, udp_ptr->dstPort)){
        return;
    }

    size_t offset = sizeof(tcpip::ethhdr) + sizeof(tcpip::ip_header) + sizeof(tcpip::udp_header);

    // std::clog <<__func__<<","<< __LINE__<<",ip_str[" <<ip_str<<"],port:" << dstPort_local <<",data_len:" <<udp_header_data_len<<",offset:" <<offset << std::endl;

    // char *data = (char *)(databuffer);
    // data += offset;

    SseMsg *msgdata = (SseMsg*)(databuffer+offset);
    
    uint32_t md_type;
    memcpy(&md_type, msgdata->mdStreamId + 1, sizeof(uint32_t)); //第一位永远是M
    if ((md_type != MDSTREAMTYPE_D004) &&  (md_type != MDSTREAMTYPE_D301) && (md_type != MDSTREAMTYPE_D102)){
        return;
    }

    std::string inst(msgdata->securityId,8);
    if (likely(md_type != MDSTREAMTYPE_D301)){
        inst = inst.substr(0, 6);
        if (likely(m_option_info.Filter(inst) == false)){
            return;
        }
    }

    uint64_t local_time_ns_1 = get_nanoseconds();
 
    // std::clog <<__func__<< "," << __LINE__ << ",inst[" << inst <<"],md_type[" << md_type << "]" << std::endl;
    char sh_inst_key[8] = {0};
    strncpy(sh_inst_key, inst.c_str(),inst.length());
    uint64_t inst_key = *(uint64_t*)sh_inst_key;
    if (m_fb_md_interval.count(inst_key) == 0){
        m_fb_md_interval.emplace(inst_key, fb_md_interval_t(msgdata->lastPrice,msgdata->lastPrice, msgdata->lastPrice));
    }

    auto &md_interval = m_fb_md_interval.at(inst_key);
    if (std::isnan(md_interval.open) && (msgdata->lastPrice>0)){
        md_interval.open = msgdata->lastPrice;
    }

    if (msgdata->lastPrice > md_interval.high){
        md_interval.high = msgdata->lastPrice;
    }

    if (msgdata->lastPrice < md_interval.low){
        md_interval.low = msgdata->lastPrice;
    }

    std::string mdstream(msgdata->mdStreamId,13);

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],inst[%s][%s],timestamp[%d],totalValue[%.3f],totalVolume[%llu],lastPrice[%.4f],openInterest[%llu]\n",__LINE__,
    //                                     inst.c_str(),mdstream.c_str(),
    //                                     msgdata->timestamp,
    //                                     msgdata->totalValue/100.0,
    //                                     msgdata->totalVolume,
    //                                     msgdata->lastPrice/100000.0,
    //                                     msgdata->openInterest);

    m_fb_md->reset_entity();                                // 重置飞豹行情结构
    m_fb_md->set_local_timestamp(local_time_ns_start); //utc ns
    m_fb_md->set_max_depth(5);                              // 设置行情深度5
    m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
    m_fb_md->set_instrument_id(inst.c_str());
    m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_SSE);
    m_fb_md->set_update_sec(hhmmssToUtcSeconds2(msgdata->timestamp/1000));
    m_fb_md->set_update_msec(msgdata->timestamp%1000);
    m_fb_md->set_open(convert_price_if_zero(md_interval.open));
    m_fb_md->set_close(convert_price_if_zero(msgdata->lastPrice/100000.0));
    m_fb_md->set_high_price(convert_price_if_zero(md_interval.high));
    m_fb_md->set_low_price(convert_price_if_zero(md_interval.low));
    m_fb_md->set_last_price(convert_price_if_zero(msgdata->lastPrice/100000.0));
    m_fb_md->set_volume(msgdata->totalVolume);
    m_fb_md->set_turn_over(msgdata->totalValue/100.0);
    m_fb_md->set_open_interest(msgdata->openInterest);

    if (likely(msgdata->length == sizeof(SseMsg))){
        m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->askPrice[0]/100000.0,msgdata->askVolume[0]));
        m_fb_md->set_ask2_price(convert_price_if_volume_zero(msgdata->askPrice[1]/100000.0,msgdata->askVolume[1]));
        m_fb_md->set_ask3_price(convert_price_if_volume_zero(msgdata->askPrice[2]/100000.0,msgdata->askVolume[2]));
        m_fb_md->set_ask4_price(convert_price_if_volume_zero(msgdata->askPrice[3]/100000.0,msgdata->askVolume[3]));
        m_fb_md->set_ask5_price(convert_price_if_volume_zero(msgdata->askPrice[4]/100000.0,msgdata->askVolume[4]));
        m_fb_md->set_ask1_volume(msgdata->askVolume[0]);
        m_fb_md->set_ask2_volume(msgdata->askVolume[1]);
        m_fb_md->set_ask3_volume(msgdata->askVolume[2]);
        m_fb_md->set_ask4_volume(msgdata->askVolume[3]);
        m_fb_md->set_ask5_volume(msgdata->askVolume[4]);
        m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->bidPrice[0]/100000.0,msgdata->bidVolume[0]));
        m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata->bidPrice[1]/100000.0,msgdata->bidVolume[1]));
        m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata->bidPrice[2]/100000.0,msgdata->bidVolume[2]));
        m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata->bidPrice[3]/100000.0,msgdata->bidVolume[3]));
        m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata->bidPrice[4]/100000.0,msgdata->bidVolume[4]));
        m_fb_md->set_bid1_volume(msgdata->bidVolume[0]);
        m_fb_md->set_bid2_volume(msgdata->bidVolume[1]);
        m_fb_md->set_bid3_volume(msgdata->bidVolume[2]);
        m_fb_md->set_bid4_volume(msgdata->bidVolume[3]);
        m_fb_md->set_bid5_volume(msgdata->bidVolume[4]);

        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],ap[%.4f][%.4f][%.4f][%.4f][%.4f],bp[%.4f][%.4f][%.4f][%.4f][%.4f],av[%llu][%llu][%llu][%llu][%llu],bv[%llu][%llu][%llu][%llu][%llu]\n",__LINE__,
        //             msgdata->askPrice[0]/100000.0,
        //             msgdata->askPrice[1]/100000.0,
        //             msgdata->askPrice[2]/100000.0,
        //             msgdata->askPrice[3]/100000.0,
        //             msgdata->askPrice[4]/100000.0,
        //             msgdata->bidPrice[0]/100000.0,
        //             msgdata->bidPrice[1]/100000.0,
        //             msgdata->bidPrice[2]/100000.0,
        //             msgdata->bidPrice[3]/100000.0,
        //             msgdata->bidPrice[4]/100000.0,
        //             msgdata->askVolume[0],
        //             msgdata->askVolume[1],
        //             msgdata->askVolume[2],
        //             msgdata->askVolume[3],
        //             msgdata->askVolume[4],
        //             msgdata->bidVolume[0],
        //             msgdata->bidVolume[1],
        //             msgdata->bidVolume[2],
        //             msgdata->bidVolume[3],
        //             msgdata->bidVolume[4]);

    }

    uint64_t local_time_ns_2 = get_nanoseconds();

    m_fb_spi->on_msg(m_fb_md);

    uint64_t local_time_ns_end = get_nanoseconds();
    delay_sum += (local_time_ns_end - local_time_ns_start); //all
    delay_sum_0 += (local_time_ns_0 - local_time_ns_start);  //read
    delay_sum_1 += (local_time_ns_1 - local_time_ns_0);     //filter
    delay_sum_2 += (local_time_ns_2 - local_time_ns_1);    //qry
    // delay_sum_3 += (local_time_ns_3 - local_time_ns_2);   //fill
    // delay_sum_4 += (local_time_ns_4 - local_time_ns_3);   //fill
    delay_sum_3 += (local_time_ns_end - local_time_ns_2);  //send

    int cpu = sched_getcpu();

    const int period = 20000;

    count_stats ++;
    if (count_stats%period == 0){
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"CPU[%d],per [%llu] ave, all cost[%llu], section read[%llu],filter[%llu],fill[%llu],send[%llu]\n",cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period);
        delay_sum = 0;        
        delay_sum_0 = 0;
        delay_sum_1 = 0;
        delay_sum_2 = 0;
        delay_sum_3 = 0;
    }
}

void QxShMdEtfOptionExanic::Routine(){
    BindCPU(m_config.cpu_id);
    usleep(100000);
    // std::clog <<__func__<<","<< __LINE__<<",g_running:"<<g_running<<","<<m_exanic_initialized.load(std::memory_order_release) << std::endl;
    std::clog <<__func__<<","<< __LINE__<<","<<g_running<<","<<m_exanic_initialized.load(std::memory_order_release)<<","<<m_fb_initialized.load(std::memory_order_release) << std::endl;
    while(g_running.load(std::memory_order_relaxed)) {
        if (m_exanic_initialized.load(std::memory_order_release) && m_fb_initialized.load(std::memory_order_release)){
            OnData();
        }
    }
}

int QxShMdEtfOptionExanic::init(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;

    LoadJsonCfg();
    InitExanic(m_config.ifname.c_str());   // TODO stub

    m_worker = new std::thread(&QxShMdEtfOptionExanic::Routine, this);   //和febao沟通，由于 on_msg的不可重入性，暂时只能有一个线程
    m_exanic_initialized.store(true,std::memory_order_release);
    m_fb_initialized.store(true,std::memory_order_release);
    m_fb_spi->on_ready();
    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;
    return 0;
}

void QxShMdEtfOptionExanic::release(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    g_running.store(false,std::memory_order_release);
    if(m_worker->joinable()){
        m_worker->join();
    }
    delete m_worker;
    m_worker = nullptr;

    ReleaseExanic();
}

void QxShMdEtfOptionExanic::connect(){
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    g_running.store(true,std::memory_order_release);
}

void QxShMdEtfOptionExanic::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id){
    std::clog <<__func__<<","<< __LINE__<< ","<<instrument_id<<","<< exchange_id<< std::endl;
}






