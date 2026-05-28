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
#include <bit> 
#include <cstdint>

#include "dz_gfex_md.h"

#include "SockProto.h"

#include "fb_md_type.h"

#include "inst_map.h"


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
    if((m_config.lv2config.net_src_ip == ip) && (m_config.lv2config.net_src_port == port)){
        return true;
    }
    return false;
}

inline bool GFEXQuota::level2_belongTo(uint32_t ip, uint16_t port){
    if((m_config.lv1config.net_src_ip == ip) && (m_config.lv1config.net_src_port == port)){
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

    m_lv2_receiver->stop();
    m_lv1_receiver->stop();
    // m_exanic_receiver->stop();
}

void GFEXQuota::connect(){
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    m_dispatcher = new std::thread(&GFEXQuota::ProcessMsg, this, m_config.cpu_id[0]);

    m_lv2_receiver.reset(new RawTcpReceiver(m_config.lv2config, m_config.cpu_id[1])); 
    m_lv2_receiver->registerCallback((void *)this, Lv2Handler);
    m_lv2_receiver->start();

    m_lv1_receiver.reset(new RawTcpReceiver(m_config.lv1config, m_config.cpu_id[2])); 
    m_lv1_receiver->registerCallback((void *)this, Lv1Handler);
    m_lv1_receiver->start();

    sleep(3);  

    if (!LoadMaping()){
        std::clog <<__func__<<","<< __LINE__<<",load Mapping failed."<< std::endl;
    }
    std::clog <<__func__<<","<< __LINE__<<",load Mapping finished."<< std::endl;

    g_running.store(true,std::memory_order_release);

    m_fb_spi->on_ready();
}

void GFEXQuota::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id){
    std::clog <<__func__<<","<< __LINE__<< ","<<instrument_id<<","<< exchange_id<< std::endl;

    if (m_subscribe_insts.count(instrument_id)==0){
        m_quota_cache.InitOnce(instrument_id);
        m_staticinfo_cache.InitOnce(instrument_id);
        m_lv1_quota.InitOnce(instrument_id);
        std::clog <<__func__<<","<< __LINE__<<",instrument_id[" <<instrument_id.c_str()<<"],m_quota_cache size:"<<m_quota_cache.size()<< std::endl;        
    }

    m_quota_cache.Sort();
    m_staticinfo_cache.Sort();
    m_lv1_quota.Sort();
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

    {
        auto& lv2 = json_parser["level2"];
        m_config.lv2config.interface = lv2["interface_name"];
        auto& lv2_filter = lv2["tcp_filter"];
        m_config.lv2config.src_ip     = lv2_filter["src_ip"].get<std::string>();
        m_config.lv2config.src_port   = lv2_filter["src_port"].get<uint16_t>();
        m_config.lv2config.net_src_ip   = ip_str_to_net_ip(lv2_filter["src_ip"].get<std::string>());
        m_config.lv2config.net_src_port = htons(lv2_filter["src_port"].get<uint16_t>());

        m_config.lv2config.dst_ip     = lv2_filter["dst_ip"].get<std::string>();
        m_config.lv2config.dst_port   = lv2_filter["dst_port"].get<uint16_t>();
        m_config.lv2config.net_dst_ip   = ip_str_to_net_ip(lv2_filter["dst_ip"].get<std::string>());
        m_config.lv2config.net_dst_port = htons(lv2_filter["dst_port"].get<uint16_t>());

        std::clog <<__func__<<","<< __LINE__<<",interface_name["<<m_config.lv2config.interface<<"]"<< std::endl;
        std::clog <<__func__<<","<< __LINE__<<"],level2 src_ip[" <<m_config.lv2config.src_ip<<"],port["<<m_config.lv2config.src_port<<"]"<< std::endl;
        std::clog <<__func__<<","<< __LINE__<<"],level2 net_src_ip[" <<m_config.lv2config.net_src_ip<<"],net_src_port["<<m_config.lv2config.net_src_port<<"]"<< std::endl;
        std::clog <<__func__<<","<< __LINE__<<"],level2 dst_ip[" <<m_config.lv2config.dst_ip<<"],port["<<m_config.lv2config.dst_port<<"]"<< std::endl;
        std::clog <<__func__<<","<< __LINE__<<"],level2 net_dst_ip[" <<m_config.lv2config.net_dst_ip<<"],net_dst_port["<<m_config.lv2config.net_dst_port<<"]"<< std::endl;
    }

    {
        auto& lv1 = json_parser["level1"];
        m_config.lv1config.interface = lv1["interface_name"];
        auto& lv1_filter = lv1["tcp_filter"];
        m_config.lv1config.src_ip     = lv1_filter["src_ip"].get<std::string>();
        m_config.lv1config.src_port   = lv1_filter["src_port"].get<uint16_t>();
        m_config.lv1config.net_src_ip   = ip_str_to_net_ip(lv1_filter["src_ip"].get<std::string>());
        m_config.lv1config.net_src_port = htons(lv1_filter["src_port"].get<uint16_t>());

        m_config.lv1config.dst_ip     = lv1_filter["dst_ip"].get<std::string>();
        m_config.lv1config.dst_port   = lv1_filter["dst_port"].get<uint16_t>();
        m_config.lv1config.net_dst_ip   = ip_str_to_net_ip(lv1_filter["dst_ip"].get<std::string>());
        m_config.lv1config.net_dst_port = htons(lv1_filter["dst_port"].get<uint16_t>());

        std::clog <<__func__<<","<< __LINE__<<",interface_name["<<m_config.lv1config.interface<<"]"<< std::endl;
        std::clog <<__func__<<","<< __LINE__<<"],level1 src_ip[" <<m_config.lv1config.src_ip<<"],port["<<m_config.lv1config.src_port<<"]"<< std::endl;
        std::clog <<__func__<<","<< __LINE__<<"],level1 net_src_ip[" <<m_config.lv1config.net_src_ip<<"],net_src_port["<<m_config.lv1config.net_src_port<<"]"<< std::endl;
        std::clog <<__func__<<","<< __LINE__<<"],level1 dst_ip[" <<m_config.lv1config.dst_ip<<"],port["<<m_config.lv1config.dst_port<<"]"<< std::endl;
        std::clog <<__func__<<","<< __LINE__<<"],level1 net_dst_ip[" <<m_config.lv1config.net_dst_ip<<"],net_dst_port["<<m_config.lv1config.net_dst_port<<"]"<< std::endl;
    
    }



    auto& worker = json_parser["worker"];
    m_config.mapping_local = worker["mapping_local"].get<std::string>();

    std::string today = getsysdate();
    std::string file_serverlist = m_config.mapping_local + "/" + today + ".serverlist";
    
    if (isFileExist(file_serverlist)){
        std::ifstream instream(file_serverlist);
        std::string line;
        while (std::getline(instream, line)) {
            if (line.empty()) continue;

            std::vector<std::string> vec;
            SplitString(line, vec, ",");

            if (vec.size()<2) continue;

            m_config.lv1config.src_ip = vec[0];
            m_config.lv1config.src_port = std::stoi(vec[1]);

            m_config.lv1config.net_src_ip   = ip_str_to_net_ip2(m_config.lv1config.src_ip);
            m_config.lv1config.net_src_port = htons(m_config.lv1config.src_port);

            std::clog <<__func__<<","<< __LINE__<<"],serverlist level1 src_ip[" <<m_config.lv1config.src_ip<<"],port["<<m_config.lv1config.src_port<<"]"<< std::endl;
            std::clog <<__func__<<","<< __LINE__<<"],serverlist level1 net_src_ip[" <<m_config.lv1config.net_src_ip<<"],net_src_port["<<m_config.lv1config.net_src_port<<"]"<< std::endl;
            break;
        }
    }
    else{
        std::clog <<__func__<<","<< __LINE__<<",error, can not found["<<file_serverlist<<"]"<< std::endl;
    }

    std::clog <<__func__<<","<< __LINE__<<",mapping_local["<<m_config.mapping_local<<"]"<< std::endl;
    for (const auto& item : worker["cpuid"]) {
        m_config.cpu_id.emplace_back(item.get<uint32_t>());
        std::clog <<__func__<<","<< __LINE__<<",cpu[" <<m_config.cpu_id.back()<<"]"<< std::endl;        
    }

    auto& filter = json_parser["filter"];
    m_config.filter_path = filter["filter_instrument_info"];

    for (const auto& item : filter["underlying"]) {
        m_subscribe_insts.emplace(item.get<std::string>());
        std::clog <<__func__<<","<< __LINE__<<",underlying[" <<item.get<std::string>()<<"]"<< std::endl;               
    }

    std::unordered_map<std::string, std::string> underlying_inst_map;
    LoadFebaoInstrumentInfo(m_config.filter_path, underlying_inst_map);
    for(auto &itor : underlying_inst_map){
        m_subscribe_insts.emplace(itor.first);
        m_subscribe_insts.emplace(itor.second);
    }

    for (auto inst : m_subscribe_insts){
        std::clog <<__func__<<","<< __LINE__<<",quota_cache underlying[" <<inst<<"]"<< std::endl;  
        m_quota_cache.InitOnce(inst.c_str());
        m_staticinfo_cache.InitOnce(inst.c_str());
        m_lv1_quota.InitOnce(inst.c_str());
    }

    m_quota_cache.Sort();
    m_staticinfo_cache.Sort();
    m_lv1_quota.Sort();
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



const size_t               MAX_BUFFER_SIZE = 1024;
char                       g_extern_msg_buffer[MAX_BUFFER_SIZE]={0};
std::atomic<int32_t>       g_half_size;
std::atomic<int32_t>       g_miss_size;



void GFEXQuota::Lv2Handler(void *ctx, const char* inputdata, size_t size, uint32_t net_src_ip, uint16_t net_src_port, uint32_t net_dst_ip, uint16_t net_dst_port){
    // std::clog <<__func__<<","<< __LINE__<<",size:"<<size<< std::endl;
    GFEXQuota *pthis = (GFEXQuota *)ctx;

    if (pthis->m_config.lv2config.net_src_ip != 0 && net_src_ip != pthis->m_config.lv2config.net_src_ip){
        return;
    }

    if (pthis->m_config.lv2config.net_src_port != 0 && net_src_port != pthis->m_config.lv2config.net_src_port){
        return; 
    }

    if (pthis->m_config.lv2config.net_dst_ip != 0 && net_dst_ip != pthis->m_config.lv2config.net_dst_ip){
        return;
    }

    // hexDumpToClog(inputdata, size);
    const char *dataptr   = inputdata;
    size_t data_len = size;
    size_t msg_len  = pthis->GetCurReadMsgLength(dataptr);

    while (data_len > 6){
        // std::clog<<__func__<<","<< __LINE__<<",while Tcp msg size:"<<size<<",data_len:"<<data_len<<",msg_len:"<<msg_len<<std::endl;

        // if ((data_len >= msg_len && msg_len >0)){

        //     // std::clog<<__func__<<","<< __LINE__<<",while Tcp msg size:"<<size<<",data_len:"<<data_len<<",msg_len:"<<msg_len<<std::endl;

        //     hexDumpToClog(dataptr, std::min(data_len, msg_len));
        // }

        // if ((data_len > 0 && msg_len ==0)){
        //     std::clog<<__func__<<","<< __LINE__<<",while Tcp half msg size:"<<size<<",data_len:"<<data_len<<",msg_len:"<<msg_len<<std::endl;
        //     hexDumpToClog(dataptr, data_len);
        // }

        if (g_half_size == 0){
            msg_len  = pthis->GetCurReadMsgLength(dataptr);
            // std::clog <<__func__<<","<< __LINE__<<",g_half_size:"<<g_half_size<<",data_len:"<<data_len<<",msg_len:"<<msg_len<< std::endl;

            if (msg_len >0 ){
                if (msg_len <= data_len ){
                    pthis->OnData_lv2(dataptr, msg_len);
                    dataptr += msg_len;
                    data_len -= msg_len;
                    continue;
                }
                else{
                    std::memcpy(g_extern_msg_buffer, dataptr, data_len); //最后的半包
                    g_half_size = data_len;
                    g_miss_size = msg_len - data_len;
                    // std::clog <<__func__<<","<< __LINE__<<",g_half_size:"<<g_half_size<<",data_len:"<<data_len<<",msg_len:"<<msg_len<< std::endl;
                    break;
                }
            }
            break;
        }
        else{

            if (msg_len >0 ){      //下一条是完整的报文,只能舍弃前面的半包
                // std::clog <<__func__<<","<< __LINE__<<",g_half_size:"<<g_half_size<<",g_miss_size:"<<g_miss_size<<",data_len:"<<data_len<<",msg_len:"<<msg_len<< std::endl;
                g_half_size = 0;
                g_miss_size = 0;
                memset((void *)g_extern_msg_buffer, 0, MAX_BUFFER_SIZE);

                continue;
            }

            std::memcpy(g_extern_msg_buffer + g_half_size, dataptr, g_miss_size);  //头部的半包
            dataptr += g_miss_size;
            data_len -= g_miss_size;

            // std::clog <<__func__<<","<< __LINE__<<",g_half_size:"<<g_half_size<<",data_len:"<<data_len<<",g_miss_size:"<<g_miss_size<< std::endl;

            // std::clog<<__func__<<","<< __LINE__<<",while buffer msg:"<<std::endl;
            // hexDumpToClog(g_extern_msg_buffer, g_half_size + g_miss_size);

            pthis->OnData_lv2(g_extern_msg_buffer, g_half_size + g_miss_size);
            g_half_size = 0;
            g_miss_size = 0;
            continue;
        }
    }
}

size_t GFEXQuota::GetCurReadMsgLength(const char* data) const {

    int32_t packetLen = *reinterpret_cast<const int32_t*>(data + 4);
    if (packetLen == LV2_OB_QUOTE_LEN){
        return LV2_OB_QUOTE_LEN;
    }

    char type = *data;
    std::string product({data,data+1});
    switch (type) {
        case '2':   // 类型2：固定长度 536
            return 536;
        case '4':   // 类型4：固定长度 204
            return 204;
        case '5':   // 类型5：固定长度 104
            return 104;
        case '6':   // 类型6：固定长度 120
            return 120;
        case '7':   // 类型7：固定长度 216
            return 216;
        case 'a':   // 类型a：固定长度 6
            return 6;
        case 'l':   
        case 'p':   
        case 's': 
            if(product == "lc" || product == "si" || product == "pd" || product == "ps" || product == "pt") {
                return 200;
            }
            // std::clog << "Unknown message product: " << product.c_str() << std::endl;

        default:

            // 错误处理：打印十六进制数据
            // std::clog << "Unknown message type: " << type << std::endl;
            return 0;
    }
}


void GFEXQuota::OnData_lv2(const char* databuffer, size_t size){

    uint64_t local_time_ns = get_nanoseconds();

    if (size == sizeof(Lv2_OB_Quote)){
        Lv2_OB_Quote *msgdata = (Lv2_OB_Quote *)(databuffer);

        size_t sym_len = gfex_inst_len(msgdata->Symbol);

        std::string inst(msgdata->Symbol,sym_len);
        // if(inst != "pt2606"){
        //     return;
        // }

        QuotaInfo *cache = m_quota_cache.Find(msgdata->Symbol);
        if(cache == nullptr){
            return;
        }

        double upper = msgdata->PriceCeil;
        double lower = msgdata->PriceFloor;

        uint64_t extime =  time_str_to_utc_s(msgdata->Time);
        // std::clog <<__func__<<","<< __LINE__<<",Time:"<<msgdata->Time<<",symbol:"<<msgdata->Symbol<<",extime:"<<extime<< std::endl;
        if(extime > (3600*15*1000)) return;  //level2在1500之后的ap bp是错误的，讨论之后决定,15点以后的行情不收了

        
        //俩盘口都不对的时候，认为是非法行情，一般是两边都是空
        if (((msgdata->Ask[0].Price < lower) || (msgdata->Ask[0].Price > upper))
            && ((msgdata->Bid[0].Price < lower) || (msgdata->Bid[0].Price > upper)))
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

        uint32_t bid_mask = (bv[0]>0) + (bv[1]>0) + (bv[2]>0) + (bv[3]>0) + (bv[4]>0);
        uint32_t ask_mask = (av[0]>0) + (av[1]>0) + (av[2]>0) + (av[3]>0) + (av[4]>0);
        uint32_t valid = IsValidQuota(cache, extime, msgdata->Volume, 5, msgdata->Ask[0].Price, msgdata->Bid[0].Price, av[0], bv[0]);

        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "before [%d][%d],inst[%s],extime[%llu],[%.2f],[%d][%d][%.2f],bp[%.2f][%.2f][%.2f][%.2f][%.2f],ap[%.2f][%.2f][%.2f][%.2f][%.2f],bv[%d][%d][%d][%d][%d],bv_imp[%d][%d][%d][%d][%d],av[%d][%d][%d][%d][%d],av_imp[%d][%d][%d][%d][%d],[%d][%d][%d][%d][%d],[%d][%d][%d][%d][%d],bid_mask[%d], ask_mask[%d]\n",__LINE__,valid,
        //                 msgdata->Symbol,extime,msgdata->LastPrice,msgdata->OpenInterest,msgdata->Volume,msgdata->Turnover,
        //                 msgdata->Bid[0].Price,msgdata->Bid[1].Price,msgdata->Bid[2].Price,msgdata->Bid[3].Price,msgdata->Bid[4].Price,
        //                 msgdata->Ask[0].Price,msgdata->Ask[1].Price,msgdata->Ask[2].Price,msgdata->Ask[3].Price,msgdata->Ask[4].Price,
        //                 msgdata->Bid[0].Volume,msgdata->Bid[1].Volume,msgdata->Bid[2].Volume,msgdata->Bid[3].Volume,msgdata->Bid[4].Volume,
        //                 msgdata->Bid[0]._VolumeImpl,msgdata->Bid[1]._VolumeImpl,msgdata->Bid[2]._VolumeImpl,msgdata->Bid[3]._VolumeImpl,msgdata->Bid[4]._VolumeImpl,
        //                 msgdata->Ask[0].Volume,msgdata->Ask[1].Volume,msgdata->Ask[2].Volume,msgdata->Ask[3].Volume,msgdata->Ask[4].Volume,
        //                 msgdata->Ask[0]._VolumeImpl,msgdata->Ask[1]._VolumeImpl,msgdata->Ask[2]._VolumeImpl,msgdata->Ask[3]._VolumeImpl,msgdata->Ask[4]._VolumeImpl,
        //                 bv[0],bv[1],bv[2],bv[3],bv[4],av[0],av[1],av[2],av[3],av[4],bid_mask, ask_mask);


        if (valid == COM_RESULT_TIME_ERROR){
            m_l2_time_err_sum ++;
            return;
        }
        else if(valid==COM_RESULT_VOL_ERROR){
            m_l2_vol_err_sum ++;
            return;
        }

        Quota_t quota;
        quota.extime = extime;
        // size_t sym_len = gfex_inst_len(msgdata->Symbol);
        strncpy(quota.code, msgdata->Symbol, sym_len);
        quota.code[sym_len] = '\0';
        quota.open         = msgdata->_openPrice;
        quota.high         = msgdata->HighPrice;
        quota.low          = msgdata->LowPrice;
        quota.last_price = msgdata->LastPrice;
        quota.openinterest = msgdata->OpenInterest;
        quota.settle       = msgdata->_clearPrice;
        quota.close       = msgdata->_closePrice;
        quota.volume = msgdata->Volume;
        quota.turn_over = msgdata->Turnover;

        uint32_t up_or_down_limit = 0;
        if (unlikely(((msgdata->Ask[0].Price < lower) || (msgdata->Ask[0].Price > upper))
            && ((msgdata->Ask[1].Price < lower) || (msgdata->Ask[1].Price > upper))
            && ((msgdata->Ask[2].Price < lower) || (msgdata->Ask[2].Price > upper))
            && ((msgdata->Ask[3].Price < lower) || (msgdata->Ask[3].Price > upper))
            && ((msgdata->Ask[4].Price < lower) || (msgdata->Ask[4].Price > upper))))
        {
            //广期所数据,跌停的时候 ap全是错误价格,此时ap的数据在bp上,av的数据在bv上;涨停则反之
            up_or_down_limit = PRICE_DOWN_LIMIT; 
            quota.ap1 = illegal_price(lower,upper, msgdata->Bid[0].Price);
            quota.ap2 = illegal_price(lower,upper, msgdata->Bid[1].Price);
            quota.ap3 = illegal_price(lower,upper, msgdata->Bid[2].Price);
            quota.ap4 = illegal_price(lower,upper, msgdata->Bid[3].Price);
            quota.ap5 = illegal_price(lower,upper, msgdata->Bid[4].Price);
            quota.av1 = bv[0];
            quota.av2 = bv[1];
            quota.av3 = bv[2];
            quota.av4 = bv[3];
            quota.av5 = bv[4];
        }
        else if(unlikely(((msgdata->Bid[0].Price < lower) || (msgdata->Bid[0].Price > upper))
            && ((msgdata->Bid[1].Price < lower) || (msgdata->Bid[1].Price > upper))
            && ((msgdata->Bid[2].Price < lower) || (msgdata->Bid[2].Price > upper))
            && ((msgdata->Bid[3].Price < lower) || (msgdata->Bid[3].Price > upper))
            && ((msgdata->Bid[4].Price < lower) || (msgdata->Bid[4].Price > upper))))
        {
            up_or_down_limit = PRICE_UP_LIMIT;
            quota.bp1 = illegal_price(lower,upper, msgdata->Ask[0].Price);
            quota.bp2 = illegal_price(lower,upper, msgdata->Ask[1].Price);
            quota.bp3 = illegal_price(lower,upper, msgdata->Ask[2].Price);
            quota.bp4 = illegal_price(lower,upper, msgdata->Ask[3].Price);
            quota.bp5 = illegal_price(lower,upper, msgdata->Ask[4].Price);
            quota.bv1 = av[0];
            quota.bv2 = av[1];
            quota.bv3 = av[2];
            quota.bv4 = av[3];
            quota.bv5 = av[4];

        }
        else{
            quota.bp1 = illegal_price(lower,upper, msgdata->Bid[0].Price);
            quota.bp2 = illegal_price(lower,upper, msgdata->Bid[1].Price);
            quota.bp3 = illegal_price(lower,upper, msgdata->Bid[2].Price);
            quota.bp4 = illegal_price(lower,upper, msgdata->Bid[3].Price);
            quota.bp5 = illegal_price(lower,upper, msgdata->Bid[4].Price);
            quota.ap1 = illegal_price(lower,upper, msgdata->Ask[0].Price);
            quota.ap2 = illegal_price(lower,upper, msgdata->Ask[1].Price);
            quota.ap3 = illegal_price(lower,upper, msgdata->Ask[2].Price);
            quota.ap4 = illegal_price(lower,upper, msgdata->Ask[3].Price);
            quota.ap5 = illegal_price(lower,upper, msgdata->Ask[4].Price);
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

        if ((av[0] == 0) && (bv[0] == 0)){
            m_l2_apbp_err_sum++;
            return;
        }

        uint64_t local_time_ns_0 = get_nanoseconds();
        m_ringbuffer->push_l2(&quota, local_time_ns, local_time_ns_0);

        // if (bid_mask<5 || ask_mask<5){

            // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "fill [%d],inst[%s],extime[%llu],[%.2f],[%d][%d][%.2f],bp[%.2f][%.2f][%.2f][%.2f][%.2f],ap[%.2f][%.2f][%.2f][%.2f][%.2f],bv[%d][%d][%d][%d][%d],av[%d][%d][%d][%d][%d]\n",__LINE__,
            //                 quota.inst,quota.extime,quota.last_price,quota.openinterest,quota.volume,quota.turnover,
            //                 quota.bp1,quota.bp2,quota.bp3,quota.bp4,quota.bp5,
            //                 quota.ap1,quota.ap2,quota.ap3,quota.ap4,quota.ap5,
            //                 quota.bv1,quota.bv2,quota.bv3,quota.bv4,quota.bv5,
            //                 quota.av1,quota.av2,quota.av3,quota.av4,quota.av5);


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

void GFEXQuota::Lv1Handler(void *ctx, const char* data, size_t size, uint32_t net_src_ip, uint16_t net_src_port, uint32_t net_dst_ip, uint16_t net_dst_port){
    GFEXQuota *pthis = (GFEXQuota *)ctx;
    pthis->OnData_lv1(data, size, net_src_ip, net_src_port, net_dst_ip, net_dst_port);
}


void GFEXQuota::DispatchMessage(const Slot& slot) {
    int64_t local_time_ns_1 = get_nanoseconds();

    if (slot.msg_type == L1_MD) {
        const Quota_t* msgdata = &slot.data;
        // std::clog<<__func__<<","<< __LINE__<<",msgdata->code:"<<msgdata->code<<std::endl; 
        StaticInfo *info = m_staticinfo_cache.Find(msgdata->code);
        if (info==nullptr){
            std::clog <<__func__<<","<< __LINE__<<",error:symbol:"<<msgdata->code<< std::endl;
            return;
        }
        // std::clog<<__func__<<","<< __LINE__<<",msgdata->code:"<<msgdata->code<<std::endl;
        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "fill [%d],inst[%s],extime[%llu],cache[%p]\n",__LINE__, msgdata.inst, slot.extime,cache);

        m_fb_md->reset_entity();// 重置飞豹行情结构
        m_fb_md->set_local_timestamp(slot.timestamp_0); //utc ns
        m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
        m_fb_md->set_instrument_id(msgdata->code);
        m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_GFEX);
        m_fb_md->set_update_sec(msgdata->extime/1000);
        m_fb_md->set_update_msec(msgdata->extime%1000);
        m_fb_md->set_open(convert_price_if_zero(msgdata->open));
        m_fb_md->set_close(convert_price_if_zero(msgdata->close));
        m_fb_md->set_high_price(convert_price_if_zero(msgdata->high));
        m_fb_md->set_low_price(convert_price_if_zero(msgdata->low));
        m_fb_md->set_last_price(convert_price_if_zero(msgdata->last_price));

        m_fb_md->set_upper_limit_price(convert_price_if_zero(info->upper));
        m_fb_md->set_down_limit_price(convert_price_if_zero(info->lower));
        m_fb_md->set_pre_open_interest(convert_price_if_zero(info->pre_openinterest));
        m_fb_md->set_pre_settlement(convert_price_if_zero(info->pre_settle));
        m_fb_md->set_pre_close(convert_price_if_zero(info->pre_close));

        m_fb_md->set_volume(msgdata->volume);
        m_fb_md->set_turn_over(msgdata->turn_over);
        m_fb_md->set_open_interest(msgdata->openinterest);

        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "l1send [%d],inst[%s],extime[%llu],[%.2f],[%d][%d][%.2f],bp[%.2f],ap[%.2f],bv[%d],av[%d]\n",__LINE__,
        //                 msgdata->code,msgdata->extime,msgdata->last_price,msgdata->openinterest, msgdata->volume, msgdata->turn_over,
        //                 msgdata->bp1, msgdata->ap1,msgdata->bv1,msgdata->av1);

        m_fb_md->set_max_depth(1);                              // 设置行情深度5
        m_fb_md->set_iopv(IOPV_TAG_LV1);

        m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->ap1,msgdata->av1));
        m_fb_md->set_ask1_volume(msgdata->av1);
        m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->bp1,msgdata->bv1));
        m_fb_md->set_bid1_volume(msgdata->bv1);

        int64_t local_time_ns_2 = get_nanoseconds();
        // std::clog<<__func__<<","<< __LINE__<<",msgdata->code:"<<msgdata->code<<std::endl;
        m_fb_spi->on_msg(m_fb_md);
        int64_t local_time_ns_end = get_nanoseconds();
        // std::clog<<__func__<<","<< __LINE__<<",msgdata->code:"<<msgdata->code<<std::endl;
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

        const Quota_t* msgdata = &slot.data;

        StaticInfo *info = m_staticinfo_cache.Find(msgdata->code);
        if (info==nullptr){
            std::clog <<__func__<<","<< __LINE__<<",error:symbol:"<<msgdata->code<< std::endl;
            return;
        }

        m_fb_md->reset_entity();// 重置飞豹行情结构
        m_fb_md->set_local_timestamp(slot.timestamp_0); //utc ns
        m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
        m_fb_md->set_instrument_id(msgdata->code);
        m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_GFEX);
        m_fb_md->set_update_sec(msgdata->extime/1000);
        m_fb_md->set_update_msec(msgdata->extime%1000);
        m_fb_md->set_open(convert_price_if_zero(msgdata->open));
        m_fb_md->set_close(convert_price_if_zero(msgdata->close));
        m_fb_md->set_high_price(convert_price_if_zero(msgdata->high));
        m_fb_md->set_low_price(convert_price_if_zero(msgdata->low));
        m_fb_md->set_last_price(convert_price_if_zero(msgdata->last_price));

        m_fb_md->set_upper_limit_price(convert_price_if_zero(info->upper));
        m_fb_md->set_down_limit_price(convert_price_if_zero(info->lower));
        m_fb_md->set_pre_open_interest(convert_price_if_zero(info->pre_openinterest));
        m_fb_md->set_pre_settlement(convert_price_if_zero(info->pre_settle));
        m_fb_md->set_pre_close(convert_price_if_zero(info->pre_close));

        m_fb_md->set_volume(msgdata->volume);
        m_fb_md->set_turn_over(msgdata->turn_over);
        m_fb_md->set_open_interest(msgdata->openinterest);


        m_fb_md->set_max_depth(5);                              // 设置行情深度5
        m_fb_md->set_iopv(IOPV_TAG_LV2);

        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "send [%d],inst[%s],extime[%llu],[%.2f],[%d][%d][%.2f],bp[%.2f][%.2f][%.2f][%.2f][%.2f],ap[%.2f][%.2f][%.2f][%.2f][%.2f],bv[%d][%d][%d][%d][%d],av[%d][%d][%d][%d][%d]\n",__LINE__,
        //                 msgdata->code,msgdata->extime,msgdata->last_price,msgdata->openinterest,msgdata->volume,msgdata->turn_over,
        //                 msgdata->bp1,msgdata->bp2,msgdata->bp3,msgdata->bp4,msgdata->bp5,
        //                 msgdata->ap1,msgdata->ap2,msgdata->ap3,msgdata->ap4,msgdata->ap5,
        //                 msgdata->bv1,msgdata->bv2,msgdata->bv3,msgdata->bv4,msgdata->bv5,
        //                 msgdata->av1,msgdata->av2,msgdata->av3,msgdata->av4,msgdata->av5);

        m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->ap1,msgdata->av1));
        m_fb_md->set_ask2_price(convert_price_if_volume_zero(msgdata->ap2,msgdata->av2));
        m_fb_md->set_ask3_price(convert_price_if_volume_zero(msgdata->ap3,msgdata->av3));
        m_fb_md->set_ask4_price(convert_price_if_volume_zero(msgdata->ap4,msgdata->av4));
        m_fb_md->set_ask5_price(convert_price_if_volume_zero(msgdata->ap5,msgdata->av5));
        m_fb_md->set_ask1_volume(msgdata->av1);
        m_fb_md->set_ask2_volume(msgdata->av2);
        m_fb_md->set_ask3_volume(msgdata->av3);
        m_fb_md->set_ask4_volume(msgdata->av4);
        m_fb_md->set_ask5_volume(msgdata->av5);
        m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->bp1, msgdata->bv1));
        m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata->bp2, msgdata->bv2));
        m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata->bp3, msgdata->bv3));
        m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata->bp4, msgdata->bv4));
        m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata->bp5, msgdata->bv5));
        m_fb_md->set_bid1_volume(msgdata->bv1);
        m_fb_md->set_bid2_volume(msgdata->bv2);
        m_fb_md->set_bid3_volume(msgdata->bv3);
        m_fb_md->set_bid4_volume(msgdata->bv4);
        m_fb_md->set_bid5_volume(msgdata->bv5);


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


void GFEXQuota::OnData_lv1(const char* data, size_t data_len, uint32_t net_src_ip, uint16_t net_src_port, uint32_t net_dst_ip, uint16_t net_dst_port){

    int64_t local_time_ns_0 = get_nanoseconds();

    if (m_config.lv1config.net_src_ip != 0 && net_src_ip != m_config.lv1config.net_src_ip){
        return; 
    }

    if (m_config.lv1config.net_src_port != 0 && net_src_port != m_config.lv1config.net_src_port){
        return; 
    }

    if (m_config.lv1config.net_dst_ip != 0 && net_dst_ip != m_config.lv1config.net_dst_ip){
        return;
    }


    if (unlikely(is_remote_date_notify(data, data_len))){
        return;
    }

    if (unlikely(is_remote_str(data,data_len))){
        return;
    }

    const char *start = data;
    int32_t  re_msg_len = data_len;

    
    size_t q_len= 0;
    uint32_t msg_type = get_common_head_type(start, re_msg_len, q_len);
    // std::clog<<__func__<<","<< __LINE__<<",used:"<<m_half_buffer_used <<",re_msg_len:"<<re_msg_len<<",msg_type:"<<msg_type<<std::endl;
    // hexDumpToClog2(start, re_msg_len);
    if (msg_type == TYPE_INDEX){
        // hexDumpToClog2(start, sizeof(common_head));
        start += sizeof(common_head);
        re_msg_len -= sizeof(common_head);
        if (m_half_buffer_used > 0){ //舍弃掉,上一个包的下半部分丢失了
            m_half_buffer_used = 0;
        }
    }
    else if (msg_type == TYPE_TIME){
        if (m_half_buffer_used > 0){ //舍弃掉,上一个包的下半部分丢失了
            m_half_buffer_used = 0;
        }
    }
    else if (msg_type == TYPE_QUOTA){
        if (m_half_buffer_used > 0){ //舍弃掉,上一个包的下半部分丢失了
            m_half_buffer_used = 0;
        }
    }
    else if(MARK_INFO == ((static_info_mark *)start)->mark){
        if (m_half_buffer_used > 0){ //舍弃掉,上一个包的下半部分丢失了
            m_half_buffer_used = 0;
        }
    }
    else if (msg_type == TYPE_ARBITRAGE){
        if (m_half_buffer_used > 0){ //舍弃掉,上一个包的下半部分丢失了
            m_half_buffer_used = 0;
        }
    }

    //处理半包
    if (m_half_buffer_used > 0){
        if (m_half_buffer_used < 60){  //防止只有一两个字节剩余
            std::memcpy(m_half_buffer+m_half_buffer_used, start, std::min(60,re_msg_len));
        }

        size_t need = 0;
        q_len= 0;
        uint32_t msg_type = get_common_head_type(m_half_buffer, sizeof(common_head),q_len);

        if (msg_type == TYPE_INDEX){
            need = sizeof(common_head) - m_half_buffer_used;
            std::memcpy(m_half_buffer+m_half_buffer_used, start, need);
            common_head *msg = (common_head *)m_half_buffer;
        }
        else if (MARK_INFO == ((static_info_mark *)m_half_buffer)->mark){
            need = sizeof(code_mapping_info) - m_half_buffer_used;
            if (re_msg_len < need){  //当前报文仍是半包
                std::memcpy(m_half_buffer+m_half_buffer_used, start, re_msg_len);
                m_half_buffer_used += re_msg_len;
                return;
            }

            std::memcpy(m_half_buffer+m_half_buffer_used, start, need);
            HandleLv1Code2Index(m_half_buffer,sizeof(code_mapping_info));
        }
        else if (msg_type == TYPE_TIME){
            common_head *header = (common_head *)m_half_buffer;
            uint32_t quota_sz = header->q_len + sizeof(common_head);
            need = quota_sz - m_half_buffer_used;
            // std::clog<<__func__<<","<< __LINE__<<",re_msg_len:"<<re_msg_len<<",need:"<<need<< ",quota len:"<<quota_sz<<std::endl; 
            if (re_msg_len < need){
                std::memcpy(m_half_buffer+m_half_buffer_used, start, re_msg_len);
                m_half_buffer_used += re_msg_len;
                return;
            }
            std::memcpy(m_half_buffer+m_half_buffer_used, start, need);
            if (header->q_len == 0xfe){
                HandleLv1StaticInfo(m_half_buffer + sizeof(common_head), header->q_len);
            }
        }
        else if (msg_type == TYPE_ARBITRAGE){
            common_head *header = (common_head *)m_half_buffer;
            uint32_t quota_sz = q_len + sizeof(common_head);
            need = quota_sz - m_half_buffer_used;
            std::clog<<__func__<<","<< __LINE__<<",re_msg_len:"<<re_msg_len<<",need:"<<need<< ",quota len:"<<quota_sz<<std::endl; 
            if (re_msg_len < need){
                std::memcpy(m_half_buffer+m_half_buffer_used, start, re_msg_len);
                m_half_buffer_used += re_msg_len;
                return;
            }
            std::memcpy(m_half_buffer+m_half_buffer_used, start, need);
            // hexDumpToClog2(m_half_buffer, quota_sz);
        }
        else if (msg_type == TYPE_QUOTA){
            common_head *header = (common_head *)m_half_buffer;
            uint32_t quota_sz = header->q_len + sizeof(common_head);
            need = quota_sz - m_half_buffer_used;
            // std::clog<<__func__<<","<< __LINE__<<",re_msg_len:"<<re_msg_len<<",need:"<<need<< ",quota len:"<<quota_sz<<std::endl; 
            if (re_msg_len < need){
                std::memcpy(m_half_buffer+m_half_buffer_used, start, re_msg_len);
                m_half_buffer_used += re_msg_len;
                return;
            }
            std::memcpy(m_half_buffer+m_half_buffer_used, start, need);
            HandleLv1Quota(local_time_ns_0, m_half_buffer+ sizeof(common_head), header->q_len); 
        }

        start += need;
        re_msg_len -= need;
        m_half_buffer_used = 0;
    }

    while(re_msg_len > 0){ //找到第一个完整的包
        if((re_msg_len >= sizeof(static_info_mark) && MARK_INFO == ((static_info_mark *)start)->mark) 
            ||  (re_msg_len >= sizeof(common_head) && get_common_head_type(start, re_msg_len,q_len) > 0)){
            break;
        }
        start ++;
        re_msg_len --;
    }


    // std::clog<<__func__<<","<< __LINE__<<",re_msg_len:"<<re_msg_len<<std::endl; 

    if (re_msg_len <=0) return;

    while(re_msg_len > 0){
        q_len= 0;
        uint32_t type = get_common_head_type(start, re_msg_len,q_len);
        // std::clog<<__func__<<","<< __LINE__<<",type:"<<type<<",q_len:"<<q_len<<",re_msg_len:"<<re_msg_len  <<std::endl;
        if ((re_msg_len < sizeof(common_head))
            || ((MARK_INFO == ((static_info_mark *)start)->mark) && (re_msg_len < sizeof(code_mapping_info)))
            || ((type == TYPE_TIME || type == TYPE_QUOTA) && (re_msg_len < (q_len+sizeof(common_head))))) {
            // std::clog<<__func__<<","<< __LINE__<<",handle res,re_msg_len:"<<re_msg_len  <<std::endl;
            // hexDumpToClog2(start, re_msg_len);
            std::memcpy(m_half_buffer, start, re_msg_len);
            m_half_buffer_used = re_msg_len;
            re_msg_len -= m_half_buffer_used;
            break;
        }

        if (type == TYPE_INDEX){
            // std::clog<<__func__<<","<< __LINE__<<",common_head head,q_len:"<<q_len<<std::endl; 
            // hexDumpToClog2(start, sizeof(common_head));
            start += sizeof(common_head);
            re_msg_len -= sizeof(common_head);
        }

        if (MARK_INFO == ((static_info_mark *)start)->mark && re_msg_len >= sizeof(code_mapping_info)){
            HandleLv1Code2Index(start,sizeof(code_mapping_info));
            start += sizeof(code_mapping_info);
            re_msg_len -= sizeof(code_mapping_info);

        }
        else if(type == TYPE_TIME){
            q_len = 0;
            uint32_t type = get_common_head_type(start, re_msg_len, q_len);
            uint32_t offset = q_len +  + sizeof(common_head);
            
            // std::clog<<__func__<<","<< __LINE__<<",quota re_msg_len:"<<re_msg_len<< ",offset:"<<offset<<std::endl; 
            // hexDumpToClog2(start, offset);
            common_head *header = (common_head *)start;
            if (header->q_len == 0xfe){
                HandleLv1StaticInfo(start + sizeof(common_head),header->q_len);
            }

            start += offset;
            re_msg_len -= offset;
        }
        else if (type == TYPE_ARBITRAGE){
            uint32_t type = get_common_head_type(start, re_msg_len, q_len);
            uint32_t offset = q_len +  + sizeof(common_head);
            // std::clog<<__func__<<","<< __LINE__<<",quota re_msg_len:"<<re_msg_len<< ",offset:"<<offset<<std::endl; 
            // hexDumpToClog2(start, offset);
        }
        else if(type == TYPE_QUOTA){
            q_len = 0;
            uint32_t type = get_common_head_type(start, re_msg_len, q_len);
            uint32_t offset = q_len +  + sizeof(common_head);
            HandleLv1Quota(local_time_ns_0, start + sizeof(common_head), q_len);                
            start += offset;
            re_msg_len -= offset;
        }
        else{
            // std::clog<<__func__<<","<< __LINE__<<",handle res,re_msg_len:"<<re_msg_len <<std::endl;  
            // hexDumpToClog2(start, re_msg_len);
            std::memcpy(m_half_buffer, start, re_msg_len);
            m_half_buffer_used = re_msg_len;
            re_msg_len -= m_half_buffer_used;
        }
    }
    
}



void GFEXQuota::HandleLv1Code2Index(const char *data, size_t data_len){
    code_mapping_info *msg = (code_mapping_info *)data;
    // std::clog<<__func__<<","<< __LINE__<<",data_len:"<<data_len<< ",code:"<<msg->code<< ",idx:"<<msg->idx<< ",type:"<<(int)msg->type<<std::endl;    
    std::string code(msg->code);
    if (m_code_save.count(code)==0){
        StaticInfo info={};
        std::strncpy(info.code, msg->code, code.size());
        info.idx = msg->idx;
        m_staticinfo_save.emplace_back(info);
        m_code_save.emplace(code,m_code_save.size());
    }
    else{
        uint32_t id = m_code_save.at(code);
        m_staticinfo_save[id].idx = msg->idx;
    }

    if (m_subscribe_insts.count(code) == 0){
        return;
    }

    if (m_lv1_code_index_set.count(msg->idx) == 0){
        m_lv1_code_index_set.emplace(msg->idx);
        m_lv1_code_map.emplace(msg->idx,code);
        std::clog<<__func__<<","<< __LINE__<< ",code:"<<msg->code<< ",idx:"<<msg->idx<< ",type:"<<(int)msg->type<<std::endl;
    }
    // else{
    //     std::clog<<__func__<<","<< __LINE__<<",error data_len:"<<data_len<< ",code:"<<msg->code<< ",idx:"<<msg->idx<< ",type:"<<(int)msg->type<<std::endl;
    // }
}


void GFEXQuota::HandleLv1StaticInfo(const char *data, size_t data_len){
    daily_static_info *msg = (daily_static_info *)data;

    std::clog<<__func__<<","<< __LINE__<<",data_len:"<<data_len<< ",code:"<<msg->code<< ",date:"<<msg->date<< ",time:"<<msg->time<<std::endl;  
    std::string code(msg->code); 

    if (m_code_save.count(code)==0){
        StaticInfo info={};
        std::strncpy(info.code, msg->code, code.size());
        info.pre_openinterest = reverseInt32(msg->pre_openinterest);
        info.upper = reverseDouble(msg->upper);
        info.lower = reverseDouble(msg->lower);
        info.pre_settle = reverseDouble(msg->pre_settle);
        info.pre_close = reverseDouble(msg->pre_close);
        m_staticinfo_save.emplace_back(info);
        m_code_save.emplace(code,m_code_save.size());
    }
    else{
        uint32_t id = m_code_save.at(code);
        m_staticinfo_save[id].pre_openinterest = reverseInt32(msg->pre_openinterest);
        m_staticinfo_save[id].upper = reverseDouble(msg->upper);
        m_staticinfo_save[id].lower = reverseDouble(msg->lower);
        m_staticinfo_save[id].pre_settle = reverseDouble(msg->pre_settle);
        m_staticinfo_save[id].pre_close = reverseDouble(msg->pre_close);
    }

    if (m_subscribe_insts.count(code) == 0){
        return;
    }

    StaticInfo *info = m_staticinfo_cache.Find(msg->code);
    if (info==nullptr){
        std::clog <<__func__<<","<< __LINE__<<",error:symbol:"<<msg->code<< std::endl;
        return;
    }

    info->pre_openinterest = msg->pre_openinterest;
    info->upper = msg->upper;
    info->lower = msg->lower;
    info->pre_settle = msg->pre_settle;
    info->pre_close  = msg->pre_close;
}

bool GFEXQuota::LoadMaping(){
    std::string date_str = getsysdate();

    std::string filename =  m_config.mapping_local + "/" + date_str + ".idx.bin";
    if (IsFileExist(filename)){
        int fd = open(filename.c_str(), O_RDONLY);
        if (fd == -1) {
            std::cerr << "Failed to open file: " << strerror(errno) << "\n";
            return false;
        }

        struct stat sb;
        if (fstat(fd, &sb) == -1) {
            std::cerr << "Failed to get file size: " << strerror(errno) << "\n";
            close(fd);
            return false;
        }

        char* data_ptr = (char *)mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data_ptr == MAP_FAILED) {
            std::cerr << "Failed to mmap file: " << strerror(errno) << "\n";
            close(fd);
            return false;
        }

        size_t number_ele = sb.st_size/sizeof(StaticInfo);
        StaticInfo *info = (StaticInfo *)data_ptr;
        for (size_t i = 0;i< number_ele; i++){
            std::string code(info->code);
            if (m_code_save.count(code)==0){
                m_code_save.emplace(code, m_code_save.size());
                m_staticinfo_save.emplace_back(*info);
                // std::clog<<__func__<<","<< __LINE__<<",code:"<<code<< ",idx:"<<info->idx<< ",pre_openinterest:"<<info->pre_openinterest<< ",upper:"<<info->upper<< ",lower:"<<info->lower<< ",pre_settle:"<<info->pre_settle<< ",pre_close:"<<info->pre_close<<std::endl;
            }

            if (m_subscribe_insts.count(code)>0){
                if (m_lv1_code_index_set.count(info->idx) == 0){
                    m_lv1_code_index_set.emplace(info->idx);
                    m_lv1_code_map.emplace(info->idx, code);

                    std::clog<<__func__<<","<< __LINE__<<",code:"<<code<< ",idx:"<<info->idx<< ",pre_openinterest:"<<info->pre_openinterest<< ",upper:"<<info->upper<< ",lower:"<<info->lower<< ",pre_settle:"<<info->pre_settle<< ",pre_close:"<<info->pre_close<<std::endl;

                    // std::clog<<__func__<<","<< __LINE__<< ",subscribe_inst code:"<<code<<",idx:"<<info->idx<<std::endl;
                }

                StaticInfo *cache_info = m_staticinfo_cache.Find(code.c_str());
                if (cache_info == nullptr){
                    std::clog <<__func__<<","<< __LINE__<<",error:symbol:"<<code<< std::endl;
                }

                strncpy(cache_info->code, code.c_str(), code.size());
                cache_info->idx = info->idx;
                cache_info->pre_openinterest = info->pre_openinterest;
                cache_info->upper = info->upper;
                cache_info->lower = info->lower;
                cache_info->pre_settle = info->pre_settle;
                cache_info->pre_close  = info->pre_close;
            }
            info ++;
        }
        close(fd);
        munmap(data_ptr,sb.st_size);
        return true;
    }
    else{

        filename =  m_config.mapping_local + "/" + date_str + ".idx.csv";
        std::ifstream instream(filename);

        std::string line;
        while (std::getline(instream, line)) {
            if (line.empty()) continue;

            std::vector<std::string> vec;
            SplitString(line, vec, ",");
            StaticInfo info = {0};
            std::string code(vec[0]);
            std::strncpy(info.code, code.c_str(), code.size());
            info.idx = std::stoi(vec[1]);
            info.pre_openinterest = std::stoi(vec[2]);
            info.upper = std::stod(vec[3]);
            info.lower = std::stod(vec[4]);
            info.pre_settle = std::stod(vec[5]);
            info.pre_close = std::stod(vec[6]);
            if (m_code_save.count(code)==0){
                m_code_save.emplace(code, m_code_save.size());
                m_staticinfo_save.emplace_back(info);
                std::clog<<__func__<<","<< __LINE__<<",code:"<<code<< ",idx:"<<info.idx<< ",pre_openinterest:"<<info.pre_openinterest<< ",upper:"<<info.upper<< ",lower:"<<info.lower<< ",pre_settle:"<<info.pre_settle<< ",pre_close:"<<info.pre_close<<std::endl;
            }

            if (m_subscribe_insts.count(code)>0){
                if (m_lv1_code_index_set.count(info.idx) == 0){
                    m_lv1_code_index_set.emplace(info.idx);
                    m_lv1_code_map.emplace(info.idx, code);
                    std::clog<<__func__<<","<< __LINE__<< ",subscribe_inst code:"<<code<<",idx:"<<info.idx<<std::endl;
                }

                StaticInfo *cache_info = m_staticinfo_cache.Find(code.c_str());
                if (cache_info == nullptr){
                    std::clog <<__func__<<","<< __LINE__<<",error:symbol:"<<code<< std::endl;
                }
                strncpy(cache_info->code, code.c_str(), code.size());
                cache_info->idx = info.idx;
                cache_info->pre_openinterest = info.pre_openinterest;
                cache_info->upper = info.upper;
                cache_info->lower = info.lower;
                cache_info->pre_settle = info.pre_settle;
                cache_info->pre_close  = info.pre_close;
            }
        }


    }





    return true;
}




// [7b000000][800]    last_price
// [7c000000][800]    high
// [7d000000][800]    low
// [7e000000][400]    last_match_vol
// [7f000000][400]    volume
// [80000000][800]    turn_over
// [81000000][400]    open_interest
// [82000000][400]    open_interest_chg
// [83000000][800]    settle
// [84000000][800]
// [85000000][800]
// [88000000][400]    bv_imp
// [86000000][800]    bp1
// [87000000][400]    bv1
// [89000000][800]    ap1
// [9a000000][400]    av1
// [9b000000][400]    av_imp
// [9c000000][800]    avg_price
// [9d000000][800]    open
// [9e000000][800]    close


void GFEXQuota::HandleLv1Quota(uint64_t t0, const char *data, size_t data_len){
    QuotaBase * quotabase = (QuotaBase *)data;
    // std::clog <<__func__<<","<< __LINE__<<",extime:"<<quotabase->extime<<",idx:"<<quotabase->idx<< std::endl;
    // hexDumpToClog2(data, data_len);
    std::string code = "";
    if (m_lv1_code_map.count(quotabase->idx) >0 ){
        code = m_lv1_code_map.at(quotabase->idx);
    }
    else{
        // std::clog <<__func__<<","<< __LINE__<<",unknow code id quota:"<< std::endl;
        // hexDumpToClog2(data, data_len);
        return;
    }
    // std::clog <<__func__<<","<< __LINE__<<",extime:"<<quotabase->extime<<",idx:"<<quotabase->idx<<",code:"<<code<< std::endl;
    const char * ptr = data+sizeof(QuotaBase);
    int32_t remain = data_len-sizeof(QuotaBase);

    std::string quata_str = code + "," + quotabase->extime;

    Quota_t *quota = m_lv1_quota.Find(code.c_str());
    if (quota==nullptr){
        std::clog <<__func__<<","<< __LINE__<<",error:symbol:"<<code<< std::endl;
        return;
    }
    strncpy(quota->code, code.c_str(), code.size());
    quota->extime = time_str_to_utc_s(quotabase->extime);
    // std::clog <<__func__<<","<< __LINE__<<",extime:"<<quotabase->extime<<",idx:"<<quotabase->idx<<",code:"<<code<<",extime:"<<quota->extime<< std::endl;

    while (remain>0){
        const quota_head *qh = (quota_head *)ptr;
        if (qh->type == 0x7b000000){
            quota->last_price = reverseDouble(*(double *)(qh +1));
        }
        else if(qh->type == 0x7f000000){
            quota->volume = reverseU32(*(uint32_t *)(qh +1));
        }
        else if(qh->type == 0x80000000){
            quota->turn_over = reverseDouble(*(double *)(qh +1));
        }
        else if(qh->type == 0x81000000){
            quota->openinterest = reverseU32(*(uint32_t *)(qh +1));
        }
        else if(qh->type == 0x86000000){
            quota->bp1 = reverseDouble(*(double *)(qh +1));
        }
        else if(qh->type == 0x87000000){
            quota->bv1 = reverseU32(*(uint32_t *)(qh +1));
        }
        else if(qh->type == 0x89000000){
            quota->ap1 = reverseDouble(*(double *)(qh +1));
        }
        else if(qh->type == 0x9a000000){
            quota->av1 = reverseU32(*(uint32_t *)(qh +1));
        }
        else if(qh->type == 0x7c000000){
            quota->high = reverseDouble(*(double *)(qh +1));
        }
        else if(qh->type == 0x7d000000){
            quota->low = reverseDouble(*(double *)(qh +1));
        }
        else if(qh->type == 0x9d000000){
            quota->open = reverseDouble(*(double *)(qh +1));
        }
        else if(qh->type == 0x9e000000){
            quota->close = reverseDouble(*(double *)(qh +1));
        }
        else if(qh->type == 0x83000000){
            quota->settle = reverseDouble(*(double *)(qh +1));
        }
        // else if(qh->type == 0x7e000000){
        // }
        // else if(qh->type == 0x82000000){
        // }
        // else if(qh->type == 0x84000000){
        // }
        // else if(qh->type == 0x85000000){
        // }
        // else if(qh->type == 0x88000000){
        // }
        // else if(qh->type == 0x9b000000){
        // }
        // else if(qh->type == 0x9c000000){
        // }

        if (qh->d_type == 0x0800){
            ptr += sizeof(quota_head) + 8;
            remain -= (sizeof(quota_head) + 8);
        }
        else if(qh->d_type == 0x0400){
            ptr += sizeof(quota_head) + 4;
            remain -= (sizeof(quota_head) + 4);
        }
    }

    // std::clog <<__func__<<","<< __LINE__<<",quata_str:"<<quota->ToString()<< std::endl;



    QuotaInfo *cache = m_quota_cache.Find(code.c_str());
    if (cache == nullptr){
        return;
    }

    // std::clog <<__func__<<","<< __LINE__<<",extime:"<<quotabase->extime<<",idx:"<<quotabase->idx<<",code:"<<code<<",extime:"<<quota->extime<< std::endl;



    uint32_t valid = IsValidQuota(cache, quota->extime, quota->volume, 1,quota->ap1, quota->bp1, quota->av1,quota->bv1);

    // std::clog <<__func__<<","<< __LINE__<<",extime:"<<quotabase->extime<<",idx:"<<quotabase->idx<<",code:"<<code<<",valid:"<<valid<< std::endl;



    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "fill [%d],valid[%d],inst[%s],extime[%llu],[%.2f],[%d][%d][%.2f],bp[%.2f],ap[%.2f],bv[%d],av[%d]\n",__LINE__,
    //     valid,code.c_str(),quota->extime,quota->last_price,quota->openinterest,quota->volume,quota->turn_over,
    //     quota->bp1,quota->ap1,quota->bv1,quota->av1);



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

    // std::clog <<__func__<<","<< __LINE__<<",extime:"<<quotabase->extime<<",idx:"<<quotabase->idx<<",code:"<<code<<",valid:"<<valid<< std::endl;



    uint64_t local_time_ns_0 = get_nanoseconds();
    m_ringbuffer->push_l1(quota, t0, local_time_ns_0);

    // std::clog <<__func__<<","<< __LINE__<<",extime:"<<quotabase->extime<<",idx:"<<quotabase->idx<<",code:"<<code<<",valid:"<<valid<< std::endl;



    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "lv1 [%s]\n",__LINE__, quota->ToString().c_str());

    // std::clog<<__func__<<","<<__LINE__<<",quota:"<<quata_str<<std::endl;
}











