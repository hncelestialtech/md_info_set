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

#include "qx_sse_etf_md_exanic.h"
#include "SockProto.h"

#include "fb_md_type.h"

#include "acc_md_sh.h"


// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    printf("create QxShMdEtfExanic[%s]\n");
    return new QxShMdEtfExanic();
}
void destroy(void *p) {
    printf("destroy QxShMdEtfExanic\n");
    delete (QxShMdEtfExanic*)p;
}
void get_md_api_version(char version[32]) {
    printf("[%s][%s]\n",__func__,FEBAO_MD_API_VERSION);
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif

std::atomic<bool> g_running = false;


inline bool QxShMdEtfExanic::belongTo(uint32_t ip, uint16_t port){
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



QxShMdEtfExanic::QxShMdEtfExanic(): 
    m_fb_initialized(false),
    m_exanic_initialized(false),
    m_fb_spi(nullptr),
    m_fb_md(cffex::fb::api::market_data_entity::create_entity())
{
    std::clog << std::unitbuf; //调试用

    // std::string testfile = "/home/febao/shenzx/test.csv";  // TODO stub
    // m_quota_stub.Load(testfile);   // TODO stub
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
}




QxShMdEtfExanic::~QxShMdEtfExanic(){
    std::clog << std::nounitbuf;
}

void QxShMdEtfExanic::register_spi(cffex::fb::api::fb_i_md_spi *spi){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    m_fb_spi = spi;
}



inline void QxShMdEtfExanic::LoadJsonCfg(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    cffex::fb::api::fb_md_config_helper *parser = m_fb_spi->get_config_helper();
    std::string md_jsoncfgfile;
    parser->get_attribute("path", "/config_path", md_jsoncfgfile);

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

bool QxShMdEtfExanic::InitExanic(const char *ifName){
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

void QxShMdEtfExanic::ReleaseExanic(){
    exanic_release_rx_buffer(m_exanic_rx);
    exanic_release_handle(m_exanic);
}




// std::vector<int64_t> g_st_delay(1000);

// std::vector<int64_t> g_st_delay2(1000);


void QxShMdEtfExanic::OnData(){

    int64_t local_time_ns = get_nanoseconds();

    char databuffer[1024];
    exanic_cycles32_t timestamp;
    auto sz = exanic_receive_frame(m_exanic_rx, databuffer, sizeof(databuffer), &timestamp);
    if (sz <= 0){
        return;
    }

    // std::clog <<__func__<<","<< __LINE__<<",sz[" <<sz<<"]" << std::endl;
    int64_t local_time_ns_0 = get_nanoseconds();

    size_t offset = sizeof(tcpip::ethhdr) + sizeof(tcpip::ip_header) + sizeof(tcpip::udp_header);
    tcpip::ip_header *ip_ptr = (tcpip::ip_header *)(databuffer + sizeof(tcpip::ethhdr));
    uint16_t ip_header_data_len = TransU16(ip_ptr->ip_len);

    tcpip::udp_header *udp_ptr = (tcpip::udp_header *)(databuffer + sizeof(tcpip::ethhdr) + sizeof(tcpip::ip_header));
    uint16_t udp_header_data_len = TransU16(udp_ptr->dataLen);

    std::string ip_str = uint32_to_ip_safe(ip_ptr->destination_ip_address);
    uint16_t dstPort_local = ntohs(udp_ptr->dstPort);

    // std::clog <<__func__<<","<< __LINE__<<",ip_str[" <<ip_str<<"],port:" << dstPort_local <<",destination_ip_address:" <<ip_ptr->destination_ip_address<< dstPort_local <<",dstPort:" <<udp_ptr->dstPort<< std::endl;

    int64_t local_time_ns_1 = get_nanoseconds();

    if (!belongTo(ip_ptr->destination_ip_address, udp_ptr->dstPort)){
        return;
    }

    // std::clog <<__func__<<","<< __LINE__<<",ip_str[" <<ip_str<<"],port:" << dstPort_local <<",data_len:" <<udp_header_data_len<<",offset:" <<offset<< std::endl;

    uint8_t *data = (uint8_t *)(databuffer + offset);
    SseMsgHead * data_header = (SseMsgHead *)(databuffer + offset);

    if (data_header->MsgType != MD_TYPE_SH_LF){ //过滤掉不必要的msg
        return;
    }
    t_SH_MCBMarketDataLF *md = (t_SH_MCBMarketDataLF *)(data_header + 1);
    std::string inst(md->security_id);

    if (!(inst.size()>0 && m_option_info.Filter(inst))){
        return;
    }

    int64_t local_time_ns_2 = get_nanoseconds();

    // std::clog <<__func__<<","<< __LINE__<<",sz[" <<sz<<"],ip_str[" <<ip_str<<"],port:" << dstPort_local<<","<<sizeof(tcpip::ethhdr) <<","<<sizeof(tcpip::ip_header)<<","<<sizeof(tcpip::udp_header)<<",ip_data_len[" <<ip_header_data_len<<"],udp data_len:" << udp_header_data_len<<",msgtype[0x"<<std::hex <<data_header->MsgType<<std::oct<<"],len:"<<data_header->MsgLen<<","<<inst.c_str() << std::endl;

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],inst[%s],time[%d],[%d][%d][%d][%d][%d],vol[%llu][%llu]\n",__LINE__,inst.c_str(), md->nTime, md->uPreClose,md->uOpen,md->uHigh,md->uLow,md->uMatch,md->iVolume,md->iTurnover);

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],ap[%d][%d][%d][%d][%d],av[%d][%d][%d][%d][%d],bp[%d][%d][%d][%d][%d],bv[%d][%d][%d][%d][%d]\n",__LINE__,
    //                 md->uAskPrice[0], md->uAskPrice[1], md->uAskPrice[2],md->uAskPrice[3], md->uAskPrice[4],
    //                 md->uAskVol[0], md->uAskVol[1], md->uAskVol[2],md->uAskVol[3], md->uAskVol[4],
    //                 md->uBidPrice[0], md->uBidPrice[1], md->uBidPrice[2],md->uBidPrice[3], md->uBidPrice[4],
    //                 md->uBidVol[0], md->uBidVol[1], md->uBidVol[2],md->uBidVol[3], md->uBidVol[4]);


    // t_SH_MCBMarketDataLF *md = (t_SH_MCBMarketDataLF *)(data + +offset + 20);

    // if ( idx == 98){
    //     hexDumpToClog((char *)data, udp_header_data_len); 
    // }
    const double PRICE_UINT = 1000.0;
    
    if (md->uMatch == 0){   //!!! 注意 临时过滤MDE01 IOPV  导致的错误行情
        return;
    }

    m_fb_md->reset_entity();
    m_fb_md->set_local_timestamp(local_time_ns); //utc ns
    m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
    m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_SSE);
    m_fb_md->set_update_sec(hhmmssToUtcSeconds2(md->nTime/1000));
    m_fb_md->set_update_msec(md->nTime%1000);
    m_fb_md->set_max_depth(10);                              // 设置行情深度10
    m_fb_md->set_instrument_id(md->security_id);

    m_fb_md->set_pre_close(convert_price_if_zero(md->uPreClose/PRICE_UINT));
    m_fb_md->set_open(convert_price_if_zero(md->uOpen/PRICE_UINT));
    m_fb_md->set_close(convert_price_if_zero(md->uMatch/PRICE_UINT));
    m_fb_md->set_high_price(convert_price_if_zero(md->uHigh/PRICE_UINT));
    m_fb_md->set_low_price(convert_price_if_zero(md->uLow/PRICE_UINT));
    m_fb_md->set_last_price(convert_price_if_zero(md->uMatch/PRICE_UINT));
    m_fb_md->set_volume(md->iVolume);
    m_fb_md->set_turn_over(md->iTurnover/100000.0);
    m_fb_md->set_ask1_price(convert_price_if_volume_zero(md->uAskPrice[0]/PRICE_UINT,md->uAskVol[0]));
    m_fb_md->set_ask2_price(convert_price_if_volume_zero(md->uAskPrice[1]/PRICE_UINT,md->uAskVol[1]));
    m_fb_md->set_ask3_price(convert_price_if_volume_zero(md->uAskPrice[2]/PRICE_UINT,md->uAskVol[2]));
    m_fb_md->set_ask4_price(convert_price_if_volume_zero(md->uAskPrice[3]/PRICE_UINT,md->uAskVol[3]));
    m_fb_md->set_ask5_price(convert_price_if_volume_zero(md->uAskPrice[4]/PRICE_UINT,md->uAskVol[4]));
    m_fb_md->set_ask6_price(convert_price_if_volume_zero(md->uAskPrice[5]/PRICE_UINT,md->uAskVol[5]));
    m_fb_md->set_ask7_price(convert_price_if_volume_zero(md->uAskPrice[6]/PRICE_UINT,md->uAskVol[6]));
    m_fb_md->set_ask8_price(convert_price_if_volume_zero(md->uAskPrice[7]/PRICE_UINT,md->uAskVol[7]));
    m_fb_md->set_ask9_price(convert_price_if_volume_zero(md->uAskPrice[8]/PRICE_UINT,md->uAskVol[8]));
    m_fb_md->set_ask10_price(convert_price_if_volume_zero(md->uAskPrice[9]/PRICE_UINT,md->uAskVol[9]));
    m_fb_md->set_ask1_volume(md->uAskVol[0]);
    m_fb_md->set_ask2_volume(md->uAskVol[1]);
    m_fb_md->set_ask3_volume(md->uAskVol[2]);
    m_fb_md->set_ask4_volume(md->uAskVol[3]);
    m_fb_md->set_ask5_volume(md->uAskVol[4]);
    m_fb_md->set_ask6_volume(md->uAskVol[5]);
    m_fb_md->set_ask7_volume(md->uAskVol[6]);
    m_fb_md->set_ask8_volume(md->uAskVol[7]);
    m_fb_md->set_ask9_volume(md->uAskVol[8]);
    m_fb_md->set_ask10_volume(md->uAskVol[9]);
    m_fb_md->set_bid1_price(convert_price_if_volume_zero(md->uBidPrice[0]/PRICE_UINT,md->uBidVol[0]));
    m_fb_md->set_bid2_price(convert_price_if_volume_zero(md->uBidPrice[1]/PRICE_UINT,md->uBidVol[1]));
    m_fb_md->set_bid3_price(convert_price_if_volume_zero(md->uBidPrice[2]/PRICE_UINT,md->uBidVol[2]));
    m_fb_md->set_bid4_price(convert_price_if_volume_zero(md->uBidPrice[3]/PRICE_UINT,md->uBidVol[3]));
    m_fb_md->set_bid5_price(convert_price_if_volume_zero(md->uBidPrice[4]/PRICE_UINT,md->uBidVol[4]));
    m_fb_md->set_bid6_price(convert_price_if_volume_zero(md->uBidPrice[5]/PRICE_UINT,md->uBidVol[5]));
    m_fb_md->set_bid7_price(convert_price_if_volume_zero(md->uBidPrice[6]/PRICE_UINT,md->uBidVol[6]));
    m_fb_md->set_bid8_price(convert_price_if_volume_zero(md->uBidPrice[7]/PRICE_UINT,md->uBidVol[7]));
    m_fb_md->set_bid9_price(convert_price_if_volume_zero(md->uBidPrice[8]/PRICE_UINT,md->uBidVol[8]));
    m_fb_md->set_bid10_price(convert_price_if_volume_zero(md->uBidPrice[9]/PRICE_UINT,md->uBidVol[9]));
    m_fb_md->set_bid1_volume(md->uBidVol[0]);
    m_fb_md->set_bid2_volume(md->uBidVol[1]);
    m_fb_md->set_bid3_volume(md->uBidVol[2]);
    m_fb_md->set_bid4_volume(md->uBidVol[3]);
    m_fb_md->set_bid5_volume(md->uBidVol[4]);
    m_fb_md->set_bid6_volume(md->uBidVol[5]);
    m_fb_md->set_bid7_volume(md->uBidVol[6]);
    m_fb_md->set_bid8_volume(md->uBidVol[7]);
    m_fb_md->set_bid9_volume(md->uBidVol[8]);
    m_fb_md->set_bid10_volume(md->uBidVol[9]);




    int64_t local_time_ns_3 = get_nanoseconds();

    m_fb_spi->on_msg(m_fb_md);
    int64_t local_time_ns_end = get_nanoseconds();

    static uint64_t delay_sum = 0;
    static uint64_t delay_sum_0 = 0; //  -0
    static uint64_t delay_sum_1 = 0; // 0-1
    static uint64_t delay_sum_2 = 0; // 1-2
    static uint64_t delay_sum_3 = 0; // 2-3
    static uint64_t delay_sum_4 = 0; // 3-end

    static uint64_t count_stats = 0;
    delay_sum += (local_time_ns_end - local_time_ns);
    delay_sum_0 += (local_time_ns_0 - local_time_ns);
    delay_sum_1 += (local_time_ns_1 - local_time_ns_0);
    delay_sum_2 += (local_time_ns_2 - local_time_ns_1);
    delay_sum_3 += (local_time_ns_3 - local_time_ns_2);
    delay_sum_4 += (local_time_ns_end - local_time_ns_3);


    int cpu = sched_getcpu();
    static uint64_t  period = 1000;
    count_stats ++;

    // g_st_delay[count_stats%period] = delay_sum_3;
    // g_st_delay2[count_stats%period] = delay_sum_4;

    if (count_stats%period == 0){
        // std::sort(g_st_delay.begin(),g_st_delay.end());
        // std::sort(g_st_delay2.begin(),g_st_delay2.end());

        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"CPU[%d],per [%llu] ave, all cost[%llu], section read[%llu],decode[%llu],filter[%llu],fill[%llu],send[%llu],[%llu][%llu][%llu][%llu],[%llu][%llu][%llu][%llu]\n",cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period, delay_sum_4/period,g_st_delay[499],g_st_delay[799],g_st_delay[899],g_st_delay[949],g_st_delay2[499],g_st_delay2[799],g_st_delay2[899],g_st_delay2[949]);
 
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"CPU[%d],per [%llu] ave, all cost[%llu], section read[%llu],decode[%llu],filter[%llu],fill[%llu],send[%llu]\n",cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period, delay_sum_4/period);
        delay_sum   = 0;        
        delay_sum_0 = 0;
        delay_sum_1 = 0;
        delay_sum_2 = 0;
        delay_sum_3 = 0;
        delay_sum_4 = 0;
    }
    
}

void QxShMdEtfExanic::Routine(){
    BindCPU(m_config.cpu_id);
    exanic_cycles32_t timestamp;
    std::clog <<__func__<<","<< __LINE__<<","<<g_running<<","<<m_exanic_initialized.load(std::memory_order_acquire) << std::endl;
    if (m_exanic_initialized.load(std::memory_order_acquire) && m_fb_initialized.load(std::memory_order_acquire)){
        while(g_running.load(std::memory_order_relaxed)) {
            OnData();
        }
    }
}

int QxShMdEtfExanic::init(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;

    LoadJsonCfg();
    InitExanic(m_config.ifname.c_str());   // TODO stub

    m_worker = new std::thread(&QxShMdEtfExanic::Routine, this);   //和febao沟通，由于 on_msg的不可重入性，暂时只能有一个线程
    m_exanic_initialized.store(true,std::memory_order_release);
    m_fb_initialized.store(true,std::memory_order_release);
    m_fb_spi->on_ready();
    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;
    return 0;
}

void QxShMdEtfExanic::release(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    g_running.store(false,std::memory_order_release);
    if(m_worker->joinable()){
        m_worker->join();
    }
    delete m_worker;
    m_worker = nullptr;

    ReleaseExanic();
}

void QxShMdEtfExanic::connect(){
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    g_running.store(true,std::memory_order_release);
}

void QxShMdEtfExanic::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id){
    std::clog <<__func__<<","<< __LINE__<< ","<<instrument_id<<","<< exchange_id<< std::endl;
}






