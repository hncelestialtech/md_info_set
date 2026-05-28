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

#include "exanic_md.h"
#include "SockProto.h"

#include "fb_md_type.h"

#include "cffex_lvl2_decode.h"

#include "inst_map.h"

#include <exanic/exanic.h>
#include <exanic/fifo_rx.h>
#include <exanic/util.h>



// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    printf("create CFFEXExanicQuota[%s]\n");
    return new CFFEXExanicQuota();
}
void destroy(void *p) {
    printf("destroy CFFEXExanicQuota\n");
    delete (CFFEXExanicQuota*)p;
}
void get_md_api_version(char version[32]) {
    printf("[%s][%s]\n",__func__,FEBAO_MD_API_VERSION);
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif

std::atomic<bool> g_running = false;


inline bool CFFEXExanicQuota::belongTo(uint32_t ip, uint16_t port){
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

CFFEXExanicQuota::CFFEXExanicQuota(): 
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




CFFEXExanicQuota::~CFFEXExanicQuota(){
    std::clog << std::nounitbuf;
}

void CFFEXExanicQuota::register_spi(cffex::fb::api::fb_i_md_spi *spi){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    m_fb_spi = spi;
}


inline void CFFEXExanicQuota::LoadJsonCfg(){
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
    m_config.filter_path = filter["filter_instrument_info"];
    std::clog <<__func__<<","<< __LINE__<<",filter_path[" <<m_config.filter_path<<"]"<< std::endl;
    std::unordered_map<std::string, std::string> underlying_inst_map;
    LoadFebaoInstrumentInfo(m_config.filter_path, underlying_inst_map);

    std::set<std::string> inst_map;
    for (auto &itor : underlying_inst_map) {
        std::string underlying = itor.first;
        std::string inst = itor.second;
        inst_map.emplace(underlying);
        inst_map.emplace(inst);
    }

    for (auto inst : inst_map) {
        m_quota_cache.InitOnce(inst);
        std::clog <<__func__<<","<< __LINE__<<",inst[" <<inst.c_str()<<"]"<< std::endl;
    }
    m_quota_cache.Sort();
    std::clog <<__func__<<","<< __LINE__<<",read_cpu[" <<m_config.cpu_id<<"]"<< std::endl;

    m_config.zx_ini = json_parser["zx_ini_cfg"];
    std::clog <<__func__<<","<< __LINE__<<",zx_ini[" <<m_config.zx_ini<<"]"<< std::endl;
}

void on_exit(int sig){
    g_running.store(false, std::memory_order_release);
}

bool CFFEXExanicQuota::writeKEYMODE()
{
    volatile uint32_t *registers;
    if ((registers = exanic_get_devkit_registers(m_exanic)) == NULL) {
        std::clog <<__func__<<","<< __LINE__<<",exanic err:"<< exanic_get_last_error()<< std::endl;
        return false;
    }
    unsigned char codeData[118];
    int keyMode = -1;
    FILE* fp = fopen(m_config.zx_ini.c_str(), "rb");
    if (fp)
    {
        int retSize = fread(codeData, 1, sizeof(codeData), fp);
        if (retSize != 118)
        {
            std::clog <<__func__<<","<< __LINE__<<",Ini File is wrong"<< std::endl;
            fclose(fp);
            return false;
        }
        switch (codeData[117])
        {
        case 0x83:
            /* code */
            keyMode = 0;
            break;
        case 0x31:
            /* code */
            keyMode = 1;
            break;
        case 0xeb:
            /* code */
            keyMode = 2;
            break;
        
        default:
            break;
        }

        registers[KEYMODE_REGISTER_ADDR] = keyMode;
        registers[KEYARG_REGISTER_ADDR] = codeData[8];
        std::clog <<__func__<<","<< __LINE__<<",mode:"<<keyMode<<",arg:"<<codeData[8]<< std::endl;
        return true;
    }
    else
    {
        std::clog <<__func__<<","<< __LINE__<<",can't open cffex_lvl2_decode.ini"<< std::endl;
        return false;
    }

}

bool CFFEXExanicQuota::InitExanic(const char *ifName){

    std::clog <<__func__<<","<< __LINE__<<",ifName["<< ifName<<"]" << std::endl;
    // cffex_lvl2_init(m_config.zx_ini.c_str());
    char exaName[46] = "exanic0";
    int portNum = 0;
    
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

    if (writeKEYMODE() == false){
        std::cerr << "set key mode failed" << std::endl;
        return false;        
    }
    else{
        std::clog <<__func__<<","<< __LINE__<<",m_exanic_rx["<< reinterpret_cast<int64_t>(m_exanic_rx)<<"]" << std::endl;
    }

    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;
    return true;
}

void CFFEXExanicQuota::ReleaseExanic(){
    exanic_release_rx_buffer(m_exanic_rx);
    exanic_release_handle(m_exanic);
}

inline uint64_t time_str_to_utc_s(const char *time_str){
    // 直接访问字符数组
    const char* h  = time_str;
    const char* m  = time_str + 3;
    const char* s  = time_str + 6;

    int hours        = (h[0] - '0') * 10 + (h[1] - '0');
    int minutes      = (m[0] - '0') * 10 + (m[1] - '0');
    int seconds      = (s[0] - '0') * 10 + (s[1] - '0');

    const int64_t HOURS_TO_MS = 3600LL;
    const int64_t MIN_TO_MS   = 60LL;

    return (hours * HOURS_TO_MS) +(minutes * MIN_TO_MS) + seconds;
}


#define LEN 409600000


void test_io(){
    char *t_src = new char[LEN];
    char *t_tar = new char[LEN];
    for(int i = 0;i<LEN;++i){
        *t_src++ = *t_tar++;
    }
}

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




void CFFEXExanicQuota::OnData(){
    int64_t local_time_ns = get_nanoseconds();

    char databuffer[1024];
    exanic_cycles32_t timestamp;
    auto sz = exanic_receive_frame(m_exanic_rx, databuffer, sizeof(databuffer), &timestamp);


    // std::clog <<__func__<<","<< __LINE__<<",sz["<< sz<<"]" << std::endl;


    if (sz !=288 || (databuffer[2] == 'T')){
        return;
    }

    CFFEXIncQuotaDataT *msgdata = (CFFEXIncQuotaDataT *)(databuffer);

    std::string  inst(msgdata->InstrumentID);
    if(inst.length()!=6 ){                        //注意，暂时只收ETF期货，国债期货和所有的期权都过滤掉
        return;
    }

    // hexDumpToClog(databuffer,sz);

    // std::clog <<__func__<<","<< __LINE__<<","<<sz<<","<<sizeof(CFFEXIncQuotaDataT)<<",inst[" <<msgdata->InstrumentID<<"],UpdateTime:" << msgdata->UpdateTime <<",UpdateMillisec:" <<msgdata->UpdateMillisec
    //         <<",LastPrice:" <<msgdata->LastPrice<<",Volume:" <<msgdata->Volume<<",Turnover:" <<msgdata->Turnover<<",OpenInterest:" <<msgdata->OpenInterest
    //         <<",bp1:" <<msgdata->BidPrice1<<",bp2:" <<msgdata->BidPrice2<<",bp3:" <<msgdata->BidPrice3<<",bp4:" <<msgdata->BidPrice4<<",bp5:" <<msgdata->BidPrice5
    //         <<",ap1:" <<msgdata->AskPrice1<<",ap2:" <<msgdata->AskPrice2<<",ap3:" <<msgdata->AskPrice3<<",ap4:" <<msgdata->AskPrice4<<",ap5:" <<msgdata->AskPrice5
    //         <<",bv1:" <<msgdata->BidVolume1<<",bv2:" <<msgdata->BidVolume2<<",bv3:" <<msgdata->BidVolume3<<",bv4:" <<msgdata->BidVolume4<<",bv5:" <<msgdata->BidVolume5
    //         <<",av1:" <<msgdata->AskVolume1<<",av2:" <<msgdata->AskVolume2<<",av3:" <<msgdata->AskVolume3<<",av4:" <<msgdata->AskVolume4<<",av5:" <<msgdata->AskVolume5<< std::endl;


    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "recv [%d],inst[%s],extime[%s],[%.2f],vol[%d][%.2f][%.2f],bp[%.2f][%.2f][%.2f][%.2f][%.2f],bv[%llu][%llu][%llu][%llu][%llu],ap[%.2f][%.2f][%.2f][%.2f][%.2f],av[%llu][%llu][%llu][%llu][%llu]\n",__LINE__,
    //         msgdata->InstrumentID,msgdata->UpdateTime,msgdata->LastPrice,msgdata->Volume,msgdata->Turnover,msgdata->OpenInterest,
    //         msgdata->BidPrice1,msgdata->BidPrice2,msgdata->BidPrice3,msgdata->BidPrice4,msgdata->BidPrice5,
    //         msgdata->BidVolume1,msgdata->BidVolume2,msgdata->BidVolume3,msgdata->BidVolume4,msgdata->BidVolume5,
    //         msgdata->AskPrice1,msgdata->AskPrice2,msgdata->AskPrice3,msgdata->AskPrice4,msgdata->AskPrice5,
    //         msgdata->AskVolume1,msgdata->AskVolume2,msgdata->AskVolume3,msgdata->AskVolume4,msgdata->AskVolume5);



    uint64_t extime = time_str_to_utc_s(msgdata->UpdateTime);

    int64_t local_time_ns_0 = get_nanoseconds();
    m_fb_md->reset_entity();
    m_fb_md->set_local_timestamp(local_time_ns); //utc ns
    m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
    m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_CFFEX);
    m_fb_md->set_update_sec(extime);
    m_fb_md->set_update_msec(msgdata->UpdateMillisec);
    m_fb_md->set_max_depth(5);                              // 设置行情深度5
    m_fb_md->set_instrument_id(msgdata->InstrumentID);
    m_fb_md->set_upper_limit_price(convert_price_if_zero(msgdata->UpperlimitPrice));
    m_fb_md->set_down_limit_price(convert_price_if_zero(msgdata->LowerlimitPrice));
    m_fb_md->set_open_interest(msgdata->OpenInterest);
    m_fb_md->set_open(convert_price_if_zero(msgdata->OpenPrice));
    m_fb_md->set_close(convert_price_if_zero(msgdata->ClosePrice));
    m_fb_md->set_high_price(convert_price_if_zero(msgdata->HighestPrice));
    m_fb_md->set_low_price(convert_price_if_zero(msgdata->LowestPrice));
    m_fb_md->set_last_price(convert_price_if_zero(msgdata->LastPrice));
    m_fb_md->set_volume(msgdata->Volume);
    m_fb_md->set_turn_over(msgdata->Turnover);
    m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->AskPrice1,msgdata->AskVolume1));
    m_fb_md->set_ask2_price(convert_price_if_volume_zero(msgdata->AskPrice2,msgdata->AskVolume2));
    m_fb_md->set_ask3_price(convert_price_if_volume_zero(msgdata->AskPrice3,msgdata->AskVolume3));
    m_fb_md->set_ask4_price(convert_price_if_volume_zero(msgdata->AskPrice4,msgdata->AskVolume4));
    m_fb_md->set_ask5_price(convert_price_if_volume_zero(msgdata->AskPrice5,msgdata->AskVolume5));
    m_fb_md->set_ask1_volume(msgdata->AskVolume1);
    m_fb_md->set_ask2_volume(msgdata->AskVolume2);
    m_fb_md->set_ask3_volume(msgdata->AskVolume3);
    m_fb_md->set_ask4_volume(msgdata->AskVolume4);
    m_fb_md->set_ask5_volume(msgdata->AskVolume5);
    m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->BidPrice1,msgdata->BidVolume1));
    m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata->BidPrice2,msgdata->BidVolume2));
    m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata->BidPrice3,msgdata->BidVolume3));
    m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata->BidPrice4,msgdata->BidVolume4));
    m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata->BidPrice5,msgdata->BidVolume5));
    m_fb_md->set_bid1_volume(msgdata->BidVolume1);
    m_fb_md->set_bid2_volume(msgdata->BidVolume2);
    m_fb_md->set_bid3_volume(msgdata->BidVolume3);
    m_fb_md->set_bid4_volume(msgdata->BidVolume4);
    m_fb_md->set_bid5_volume(msgdata->BidVolume5);

    int64_t local_time_ns_1 = get_nanoseconds();

    m_fb_spi->on_msg(m_fb_md);
    int64_t local_time_ns_end = get_nanoseconds();

    // test_cpu();

    // int64_t T_test = get_nanoseconds();




    static uint64_t delay_sum = 0;
    static uint64_t delay_sum_0 = 0; //  -0
    static uint64_t delay_sum_1 = 0; // 0-1
    static uint64_t delay_sum_2 = 0; // 1-end

    // static uint64_t delay_sum_a = 0;


    static uint64_t count_stats = 0;
    delay_sum += (local_time_ns_end - local_time_ns);
    delay_sum_0 += (local_time_ns_0 - local_time_ns);
    delay_sum_1 += (local_time_ns_1 - local_time_ns_0);
    delay_sum_2 += (local_time_ns_end - local_time_ns_1);

    // delay_sum_a += (T_test  - local_time_ns_end);

    
    static uint64_t  period = 1000;
    count_stats ++;
    if (count_stats%period == 0){
        int cpu = sched_getcpu();
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"CPU[%d],per[%llu], ave allcost[%llu], section read[%llu],fill[%llu],send[%llu]\n",
        cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period);
        delay_sum   = 0;        
        delay_sum_0 = 0;
        delay_sum_1 = 0;
        delay_sum_2 = 0;

        // delay_sum_a = 0;


    }






}

void CFFEXExanicQuota::Routine(){
    BindCPU(m_config.cpu_id);
    exanic_cycles32_t timestamp;
    std::clog <<__func__<<","<< __LINE__<<","<<g_running<<","<<m_exanic_initialized.load(std::memory_order_acquire) << std::endl;
    if (m_exanic_initialized.load(std::memory_order_acquire) && m_fb_initialized.load(std::memory_order_acquire)){
        while(g_running.load(std::memory_order_relaxed)) {
            OnData();
        }
    }
}

int CFFEXExanicQuota::init(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;

    LoadJsonCfg();
    InitExanic(m_config.ifname.c_str());   // TODO stub

    m_worker = new std::thread(&CFFEXExanicQuota::Routine, this);   //和febao沟通，由于 on_msg的不可重入性，暂时只能有一个线程
    m_exanic_initialized.store(true,std::memory_order_release);
    m_fb_initialized.store(true,std::memory_order_release);
    m_fb_spi->on_ready();
    std::clog <<__func__<<","<< __LINE__<<",success" << std::endl;
    return 0;
}

void CFFEXExanicQuota::release(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    g_running.store(false,std::memory_order_release);
    if(m_worker->joinable()){
        m_worker->join();
    }
    delete m_worker;
    m_worker = nullptr;

    ReleaseExanic();
}

void CFFEXExanicQuota::connect(){
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    g_running.store(true,std::memory_order_release);
}

void CFFEXExanicQuota::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id){
    std::clog <<__func__<<","<< __LINE__<< ","<<instrument_id<<","<< exchange_id<< std::endl;
}






