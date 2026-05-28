
#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>

#include <signal.h>


#include "common.h"


#include "dcel2decode_mds.h"

#include "gz_dce_md.h"
#include "fb_md_plugin_api.h"
#include "fb_md_entity.h"
#include "fb_md_helper.h"
#include "fb_md_type.h"

// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    return new QxMdMgr();
}
void destroy(void *p) {
    delete (QxMdMgr*)p;
}
void get_md_api_version(char version[32]) {
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif



inline uint64_t dce_time_str_to_utc_s(const char *time_str){
    int nums[4] = {0, 0, 0, 0};
    int numIdx = 0;
    
    const char *time = time_str;

    while (*time) {
        if (*time >= '0' && *time <= '9') {
            nums[numIdx] = nums[numIdx] * 10 + (*time - '0');
        } else if (*time == ':' || *time == '.') {
            numIdx++;
        }
        time++;
    }

    const int64_t HOURS_TO_MS = 3600000LL;
    const int64_t MIN_TO_MS   = 60000LL;
    const int64_t SEC_TO_MS   = 1000LL;

    return (nums[0] * HOURS_TO_MS) +(nums[1] * MIN_TO_MS) + (nums[2] * SEC_TO_MS) +nums[3];
}




// inline void hexDumpToClog(const char* buffer, size_t length) {
//     // 保存原始格式状态
//     std::ios oldState(nullptr);
//     oldState.copyfmt(std::clog);
    
//     // 设置十六进制输出格式
//     std::clog << std::hex << std::setfill('0');
    
//     for (size_t i = 0; i < length; ++i) {
//         // 每128字节换行并显示偏移量
//         if (i % 16 == 0) {
//             std::clog << std::dec << std::setfill('0');
//             std::clog << "\n" << std::setw(4) << i << ": ";
//             //std::clog << "\n0x" << std::setw(4) << i << ": ";
//         }
//         std::clog << std::hex << std::setfill('0');
//         // 输出当前字节的十六进制值
//         std::clog << std::setw(2) << (static_cast<unsigned>(buffer[i]) & 0xFF) << " ";
        
//         // 每16字节增加额外空格提高可读性
//         if (i % 8 == 7 && i % 16 != 15) {
//             std::clog << " ";
//         }
//     }

//     // 恢复原始格式状态
//     std::clog.copyfmt(oldState);
//     std::clog << std::endl;
// }


inline void hexDumpToClog2(const char* buffer, size_t length) {
    // 保存原始格式状态
    std::ios oldState(nullptr);
    oldState.copyfmt(std::clog);
    std::clog << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        std::clog << std::hex << std::setfill('0')<< std::setw(2) << (static_cast<unsigned>(buffer[i]) & 0xFF) << "";
    }
    
    std::clog.copyfmt(oldState);
    std::clog << std::endl;
}








std::atomic<bool> g_running = false;

QxMdMgr::QxMdMgr(): 
    m_fb_initialized(false),
    m_fb_spi(nullptr),
    m_fb_md(cffex::fb::api::market_data_entity::create_entity()){
    std::clog << std::unitbuf; //调试用
    std::clog <<__func__<<","<< __LINE__ << std::endl;
}

QxMdMgr::~QxMdMgr(){
    std::clog << std::nounitbuf;
}



int QxMdMgr::init(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    LoadJsonCfg();

    m_fb_spi->on_ready();
    m_fb_initialized.store(true,std::memory_order_release);
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    return 0;
}

void QxMdMgr::register_spi(cffex::fb::api::fb_i_md_spi *spi){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    m_fb_spi = spi;
}

void QxMdMgr::release(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;

    g_running.store(false,std::memory_order_release);

    Mds_Deinit();
}


void QxMdMgr::connect(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;

    Mds_Init();

    m_udp_receiver.reset(new MulticastReceiver(m_config.udpconfig.group_ip, m_config.udpconfig.group_port, m_config.udpconfig.interface_name, m_config.cpu_id[0], 1024)); //因为数据长度是135,所以buffer长度选择256
    m_udp_receiver->registerCallback((void *)this, UdpHandler);
    m_udp_receiver->start();

    sleep(3);  

    g_running.store(true,std::memory_order_release);
}

void QxMdMgr::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id){
    std::clog <<__func__<<","<< __LINE__<<",instrument_id["<< instrument_id <<"],exchange_id["<<exchange_id <<"]" << std::endl;
    // printf("subscribe_inst:instrument_id[%s],exchange_id[%hhu]\n",instrument_id.c_str(),exchange_id);

    if (m_subscribe_insts.count(instrument_id) == 0){
        m_subscribe_insts.emplace(instrument_id);        
        m_quota_cache.InitOnce(instrument_id);
        m_quota_cache.Sort();
    }
}

inline void QxMdMgr::LoadJsonCfg(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;

    cffex::fb::api::fb_md_config_helper *parser = m_fb_spi->get_config_helper();
    std::string md_jsoncfgfile;
    parser->get_attribute("file", "/md_config_path", md_jsoncfgfile);

    std::ifstream jsonfile(md_jsoncfgfile);
    nlohmann::json json_parser = nlohmann::json::parse(jsonfile);

    if(json_parser.contains("decode")){
        auto& decode = json_parser["decode"];
        m_config.decrypt = decode["decrypt"].get<uint32_t>();
        m_config.remote_key_path = decode["remote_key_path"].get<std::string>();
        m_config.remote_key_path_bak = decode["remote_key_path_bak"].get<std::string>();
        m_config.channel = decode["channel"].get<uint32_t>();
    }

    std::clog <<__func__<<","<< __LINE__<<",decrypt:"<<m_config.decrypt<<",remote_key_path:"<<m_config.remote_key_path<<",remote_key_path_bak:"<<m_config.remote_key_path_bak<<",channel:"<<m_config.channel<< std::endl;

    if(json_parser.contains("level2")){
        auto& eth = json_parser["level2"];
        m_config.udpconfig.interface_name = eth["interface_name"];
        m_config.udpconfig.group_ip   = eth["group_ip"].get<std::string>();
        m_config.udpconfig.group_port = eth["group_port"].get<uint16_t>();
        m_config.udpconfig.net_group_ip   = ip_str_to_net_ip(eth["group_ip"].get<std::string>());
        m_config.udpconfig.net_group_port = htons(eth["group_port"].get<uint16_t>());



        std::clog <<__func__<<","<< __LINE__<<",interface_name["<<m_config.udpconfig.interface_name<<"]"<< std::endl;
        std::clog <<__func__<<","<< __LINE__<<"],level2 group_ip[" <<m_config.udpconfig.group_ip<<"],group_port["<<m_config.udpconfig.group_port<<"],["<<m_config.udpconfig.net_group_ip<<"],["<<m_config.udpconfig.net_group_port<<"]"<< std::endl;
    }

    auto& worker = json_parser["worker"];
    for (const auto& item : worker["cpuid"]) {
        m_config.cpu_id.emplace_back(item.get<uint32_t>());
        std::clog <<__func__<<","<< __LINE__<<",cpu[" <<m_config.cpu_id.back()<<"]"<< std::endl;     
        m_config.udpconfig.cpu_no   = m_config.cpu_id[0];           
    }

    auto& filter = json_parser["filter"];
    m_config.filter_path = filter["filter_instrument_info"];

    std::clog <<__func__<<","<< __LINE__<<",filter_path[" <<m_config.filter_path<<"]"<< std::endl;
    
    for (const auto& item : filter["underlying"]) {
        m_subscribe_insts.emplace(item.get<std::string>());
        std::clog <<__func__<<","<< __LINE__<<",underlying[" <<item.get<std::string>()<<"]"<< std::endl;               
    }

    std::unordered_map<std::string, std::string> underlying_inst_map;
    LoadFebaoInstrumentInfo(m_config.filter_path, underlying_inst_map);
    std::clog <<__func__<<","<< __LINE__<<",Load ret[" <<underlying_inst_map.size()<<"]"<< std::endl;

    for(auto &itor : underlying_inst_map){
        m_subscribe_insts.emplace(itor.first);
        m_subscribe_insts.emplace(itor.second);
    }

    for (auto inst : m_subscribe_insts){
        std::clog <<__func__<<","<< __LINE__<<",quota_cache underlying[" <<inst<<"]"<< std::endl;  
        m_quota_cache.InitOnce(inst);
    }

    m_quota_cache.Sort();

    std::clog <<__func__<<","<< __LINE__<<",subscribe_insts[" <<m_subscribe_insts.size()<<"]"<< std::endl;
}

void on_exit(int sig){
    std::clog <<__func__<<","<< __LINE__<<",g_running:"<<g_running<< std::endl;

    g_running.store(false, std::memory_order_release);
}

void QxMdMgr::mds_on_best(const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_best* msgdata, void* dataAttachedInfo){

    std::string inst(contractInfo->ContractId);
    if (m_subscribe_insts.count(inst)==0){
        return;
    }
    // std::clog <<__func__<<","<< __LINE__<<", ContractId:"<<contractInfo->ContractId<<", GenTime:"<<contractInfo->GenTime<< std::endl;

    int64_t local_time_ns_0 = get_nanoseconds();
    QuotaInfo *cache = m_quota_cache.Find(contractInfo->ContractId);
    if (cache == nullptr){
        std::clog <<__func__<<","<< __LINE__<<", can not find ContractId:"<<contractInfo->ContractId<< std::endl;
        return;
    }

    int64_t local_time_ns_1 = get_nanoseconds();
    uint64_t extime = dce_time_str_to_utc_s(contractInfo->GenTime); 



	// std::clog <<__func__<<","<< __LINE__<<", code:"<<contractInfo->ContractId<<", GenTime:"<<contractInfo->GenTime<<", extime:"<<extime
    //     <<",last_price:"<<msgdata->LastPrice<<",open:"<<msgdata->OpenPrice<<",high:"<<msgdata->HighPrice<<",low:"<<msgdata->LowPrice
    //     <<",volume:"<<msgdata->MatchTotQty
    //     <<",turn_over:"<<msgdata->Turnover
    //     <<",pre_open_interest:"<<msgdata->LastOpenInterest
    //     <<",open_interest:"<<msgdata->OpenInterest
    //     <<",upper:"<<msgdata->RiseLimit
    //     <<",lower:"<<msgdata->FallLimit
    //     <<",pre_settle:"<<msgdata->LastClearPrice
    //     <<",pre_close:"<<msgdata->LastClose
    //     <<",bp1:"<<msgdata->BuyPriceOne
    //     <<",bv1:"<<msgdata->BuyQtyOne
    //     <<",ap1:"<<msgdata->SellPriceOne
    //     <<",av1:"<<msgdata->SellQtyOne
    //     << std::endl;


    m_fb_md->reset_entity();// 重置飞豹行情结构
    m_fb_md->set_local_timestamp(local_time_ns_0); //utc ns
    m_fb_md->set_max_depth(1);                              // 设置行情深度5
    m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
    m_fb_md->set_instrument_id(contractInfo->ContractId);
    m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_DCE);
    m_fb_md->set_update_sec(extime/1000);
    m_fb_md->set_update_msec(extime%1000);
    m_fb_md->set_open(convert_price_if_volume_zero(msgdata->OpenPrice, msgdata->MatchTotQty));
    m_fb_md->set_close(convert_price_if_volume_zero(msgdata->LastPrice, msgdata->MatchTotQty));
    m_fb_md->set_iopv(IOPV_TAG_LV1);
    m_fb_md->set_upper_limit_price(msgdata->RiseLimit);
    m_fb_md->set_down_limit_price(msgdata->FallLimit);
    m_fb_md->set_pre_settlement(msgdata->LastClearPrice);
    m_fb_md->set_pre_close(msgdata->LastClose);
    m_fb_md->set_pre_open_interest(msgdata->LastOpenInterest);
    m_fb_md->set_high_price(convert_price_if_volume_zero(msgdata->HighPrice, msgdata->MatchTotQty));
    m_fb_md->set_low_price(convert_price_if_volume_zero(msgdata->LowPrice, msgdata->MatchTotQty));
    m_fb_md->set_last_price(convert_price_if_volume_zero(msgdata->LastPrice, msgdata->MatchTotQty));
    m_fb_md->set_volume(msgdata->MatchTotQty);
    m_fb_md->set_turn_over(msgdata->Turnover);
    m_fb_md->set_open_interest(msgdata->OpenInterest);
    m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->SellPriceOne,msgdata->SellQtyOne));
    m_fb_md->set_ask1_volume(msgdata->SellQtyOne);
    m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->BuyPriceOne,msgdata->BuyQtyOne));
    m_fb_md->set_bid1_volume(msgdata->BuyQtyOne);

    int64_t local_time_ns_2 = get_nanoseconds();

    m_fb_spi->on_msg(m_fb_md);
    int64_t local_time_ns_end = get_nanoseconds();
    

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"code[%s],extime[%s],upper[%.4f],lower[%.4f],ohl[%.4f][%.4f][%.4f],pre_settle[%.4f],pre_close[%.4f],pre_oi[%d],oi[%d], vol[%d],turnover[%.2f],last_price[%.3f],ap[%.3f],bp[%.3f],av[%d],bv[%d]\n",
    //     contractInfo->ContractId, contractInfo->GenTime, msgdata->RiseLimit, msgdata->FallLimit, msgdata->OpenPrice, msgdata->HighPrice, msgdata->LowPrice,
    //     msgdata->LastClearPrice,msgdata->LastClose,msgdata->LastOpenInterest,msgdata->OpenInterest,msgdata->MatchTotQty,msgdata->Turnover,msgdata->LastPrice,
    //     msgdata->SellPriceOne,msgdata->BuyPriceOne,msgdata->SellQtyOne, msgdata->BuyQtyOne);


    cache->time = local_time_ns_0;
    cache->volume = msgdata->MatchTotQty;
    cache->ap1 = msgdata->SellPriceOne;
    cache->bp1 = msgdata->BuyPriceOne;
    cache->turn_over = msgdata->Turnover;               // 成交金额
    cache->open = msgdata->OpenPrice;
    cache->high = msgdata->HighPrice;
    cache->low = msgdata->LowPrice;
    cache->last_price = msgdata->LastPrice;
    cache->upper = msgdata->RiseLimit;            
    cache->lower = msgdata->FallLimit;
    cache->pre_settle = msgdata->LastClearPrice;
    cache->pre_close = msgdata->LastClose;
    cache->av1 = msgdata->SellQtyOne;
    cache->bv1 = msgdata->BuyQtyOne;
    cache->open_interest = msgdata->OpenInterest;
    cache->pre_open_interest = msgdata->LastOpenInterest;      // 初始持仓量



    static uint64_t delay_sum = 0;
    static uint64_t delay_sum_0 = 0; //  -0
    static uint64_t delay_sum_1 = 0; // 0-1
    static uint64_t delay_sum_2 = 0; // 0-1

    static uint64_t count_stats = 0;
    delay_sum   += (local_time_ns_end - local_time_ns_0);
    delay_sum_0 += (local_time_ns_1 - local_time_ns_0);
    delay_sum_1 += (local_time_ns_2 - local_time_ns_1);
    delay_sum_2 += (local_time_ns_end - local_time_ns_2);

    int cpu = sched_getcpu();
    static uint64_t  period = 1000;
    count_stats ++;
    if (count_stats%period == 0){
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L1 CPU[%d],per [%llu] ave, all cost[%llu], section filter and decode[%llu],fill[%llu],send[%llu]\n",cpu, period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period);
        delay_sum   = 0;
        delay_sum_0 = 0;
        delay_sum_1 = 0;
        delay_sum_2 = 0;
    }


}

void QxMdMgr::mds_on_deep(const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_deep* msgdata, void* dataAttachedInfo){
    // std::clog <<__func__<<","<< __LINE__<<", ContractId:"<<contractInfo->ContractId<<", GenTime:"<<contractInfo->GenTime<< std::endl;

    std::string inst(contractInfo->ContractId);
    if (m_subscribe_insts.count(inst)==0){
        return;
    }

    int64_t local_time_ns_0 = get_nanoseconds();
    QuotaInfo *cache = m_quota_cache.Find(contractInfo->ContractId);
    if (cache == nullptr){
        std::clog <<__func__<<","<< __LINE__<<", can not find ContractId:"<<contractInfo->ContractId<< std::endl;
        return;
    }
    int64_t local_time_ns_1 = get_nanoseconds();

    m_fb_md->reset_entity();// 重置飞豹行情结构
    uint64_t extime = dce_time_str_to_utc_s(contractInfo->GenTime);
    m_fb_md->set_local_timestamp(local_time_ns_0); //utc ns
    m_fb_md->set_max_depth(5);                              // 设置行情深度5
    m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
    m_fb_md->set_instrument_id(contractInfo->ContractId);
    m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_DCE);
    m_fb_md->set_update_sec(extime/1000);
    m_fb_md->set_update_msec(extime%1000);
    m_fb_md->set_open(convert_price_if_volume_zero(cache->open, cache->volume));
    m_fb_md->set_close(convert_price_if_volume_zero(cache->last_price, cache->volume));
    m_fb_md->set_iopv(IOPV_TAG_LV2);
    m_fb_md->set_upper_limit_price(cache->upper);
    m_fb_md->set_down_limit_price(cache->lower);
    m_fb_md->set_pre_settlement(cache->pre_settle);
    m_fb_md->set_pre_close(cache->pre_close);
    m_fb_md->set_pre_open_interest(cache->pre_open_interest);
    m_fb_md->set_high_price(convert_price_if_volume_zero(cache->high, cache->volume));
    m_fb_md->set_low_price(convert_price_if_volume_zero(cache->low, cache->volume));
    m_fb_md->set_last_price(convert_price_if_volume_zero(cache->last_price, cache->volume));
    m_fb_md->set_volume(cache->volume);
    m_fb_md->set_turn_over(cache->turn_over);
    m_fb_md->set_open_interest(cache->open_interest);


	// std::clog <<__func__<<","<< __LINE__<<", code:"<<contractInfo->ContractId<<", GenTime:"<<contractInfo->GenTime<<", extime:"<<extime
    //     <<",last_price:"<<cache->last_price<<",open:"<<cache->open<<",high:"<<cache->high<<",low:"<<cache->low
    //     <<",volume:"<<cache->volume
    //     <<",turn_over:"<<cache->turn_over
    //     <<",pre_open_interest:"<<cache->pre_open_interest
    //     <<",open_interest:"<<cache->open_interest
    //     <<",upper:"<<cache->upper
    //     <<",lower:"<<cache->lower
    //     <<",pre_settle:"<<cache->pre_settle
    //     <<",pre_close:"<<cache->pre_close
    //     <<",bp1:"<<msgdata->BuyPriceOne
    //     <<",bv1:"<<msgdata->BuyQtyOne
    //     <<",ap1:"<<msgdata->SellPriceOne
    //     <<",av1:"<<msgdata->SellQtyOne
    //     << std::endl;





    m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->SellPriceOne,msgdata->SellQtyOne));
    m_fb_md->set_ask2_price(convert_price_if_volume_zero(msgdata->SellPriceTwo,msgdata->SellQtyTwo));
    m_fb_md->set_ask3_price(convert_price_if_volume_zero(msgdata->SellPriceThree,msgdata->SellQtyThree));
    m_fb_md->set_ask4_price(convert_price_if_volume_zero(msgdata->SellPriceFour,msgdata->SellQtyFour));
    m_fb_md->set_ask5_price(convert_price_if_volume_zero(msgdata->SellPriceFive,msgdata->SellQtyFive));
    m_fb_md->set_ask1_volume(msgdata->SellQtyOne);
    m_fb_md->set_ask2_volume(msgdata->SellQtyTwo);
    m_fb_md->set_ask3_volume(msgdata->SellQtyThree);
    m_fb_md->set_ask4_volume(msgdata->SellQtyFour);
    m_fb_md->set_ask5_volume(msgdata->SellQtyFive);
    m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->BuyPriceOne,msgdata->BuyQtyOne));
    m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata->BuyPriceTwo,msgdata->BuyQtyTwo));
    m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata->BuyPriceThree,msgdata->BuyQtyThree));
    m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata->BuyPriceFour,msgdata->BuyQtyFour));
    m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata->BuyPriceFive,msgdata->BuyQtyFive));
    m_fb_md->set_bid1_volume(msgdata->BuyQtyOne);
    m_fb_md->set_bid2_volume(msgdata->BuyQtyTwo);
    m_fb_md->set_bid3_volume(msgdata->BuyQtyThree);
    m_fb_md->set_bid4_volume(msgdata->BuyQtyFour);
    m_fb_md->set_bid5_volume(msgdata->BuyQtyFive);


    int64_t local_time_ns_2 = get_nanoseconds();

    m_fb_spi->on_msg(m_fb_md);


    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"code[%s],extime[%s],upper[%.4f],lower[%.4f],ohl[%.4f][%.4f][%.4f],pre_settle[%.4f],pre_close[%.4f],pre_oi[%d],oi[%d], vol[%d],turnover[%.2f],last_price[%.3f],ap[%.3f][%.3f][%.3f][%.3f][%.3f],bp[%.3f][%.3f][%.3f][%.3f][%.3f],av[%d][%d][%d][%d][%d],bv[%d][%d][%d][%d][%d]\n",
    //     contractInfo->ContractId, contractInfo->GenTime, cache->upper, cache->lower, cache->open, cache->high, cache->low,cache->pre_settle,cache->pre_close,cache->pre_open_interest,cache->open_interest,cache->volume,cache->turn_over,cache->last_price,
    //     msgdata->SellPriceOne,msgdata->SellPriceTwo,msgdata->SellPriceThree,msgdata->SellPriceFour,msgdata->SellPriceFive,
    //     msgdata->BuyPriceOne,msgdata->BuyPriceTwo,msgdata->BuyPriceThree,msgdata->BuyPriceFour,msgdata->BuyPriceFive,       
    //     msgdata->SellQtyOne,msgdata->SellQtyTwo,msgdata->SellQtyThree,msgdata->SellQtyFour,msgdata->SellQtyFive,
    //     msgdata->BuyQtyOne,msgdata->BuyQtyTwo,msgdata->BuyQtyThree,msgdata->BuyQtyFour,msgdata->BuyQtyFive);
 





    int64_t local_time_ns_end = get_nanoseconds();
    
    static uint64_t delay_sum = 0;
    static uint64_t delay_sum_0 = 0; //  -0
    static uint64_t delay_sum_1 = 0; // 0-1
    static uint64_t delay_sum_2 = 0; // 0-1

    static uint64_t count_stats = 0;
    delay_sum   += (local_time_ns_end - local_time_ns_0);
    delay_sum_0 += (local_time_ns_1 - local_time_ns_0);
    delay_sum_1 += (local_time_ns_2 - local_time_ns_1);
    delay_sum_2 += (local_time_ns_end - local_time_ns_2);

    int cpu = sched_getcpu();
    static uint64_t  period = 1000;
    count_stats ++;
    if (count_stats%period == 0){
        m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L2 CPU[%d],per [%llu] ave, all cost[%llu], section filter and decode[%llu],fill[%llu],send[%llu]\n",cpu, period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period);
        delay_sum   = 0;
        delay_sum_0 = 0;
        delay_sum_1 = 0;
        delay_sum_2 = 0;
    }


}

void QxMdMgr::on_best(void* userData, const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_best* best, void* dataAttachedInfo){
    QxMdMgr *pthis = (QxMdMgr *)userData;
    pthis->mds_on_best(msgHeader, contractInfo, best, dataAttachedInfo);
}

void QxMdMgr::on_deep(void* userData, const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_deep* deep, void* dataAttachedInfo){
    QxMdMgr *pthis = (QxMdMgr *)userData;
    pthis->mds_on_deep(msgHeader, contractInfo, deep, dataAttachedInfo);
}

void QxMdMgr::on_arbi_best(void* userData, const msg_header* msgHeader, const mds_contract_info* contractInfo, const mds_arbi_best* arbiBest, void* dataAttachedInfo){
    QxMdMgr *pthis = (QxMdMgr *)userData;
}

void QxMdMgr::Mds_Init(){

    mds_rtn_data rtn = dcel2_mds_init_from_svr(m_config.remote_key_path.c_str(), m_config.channel);
    if (rtn.ErrNo != 0) {
        std::clog <<__func__<<","<< __LINE__<<",get key from:"<<m_config.remote_key_path<<",channel:"<<m_config.channel <<" failed:"<<rtn.ErrNo<< std::endl;

        rtn = dcel2_mds_init_from_svr(m_config.remote_key_path_bak.c_str(), m_config.channel);
        if (rtn.ErrNo != 0) {
            std::clog <<__func__<<","<< __LINE__<<",get key from bak:"<<m_config.remote_key_path_bak<<",channel:"<<m_config.channel <<" failed:"<<rtn.ErrNo<< std::endl;
        }
    }

    if (rtn.ErrNo == 0){
        m_mds_param = rtn.Param1;
        int mode = 0;
        uint8_t key[64];
        if (dcel2_mds_get_key(m_mds_param, &mode, key)) {
            std::clog <<__func__<<","<< __LINE__<<", key is:"<< std::endl;
            hexDumpToClog((char *)key, 64);
        }
        else{
            m_config.decrypt = DECRYPT_LOCAL_KEY;
        }
    }

    if (NULL == m_mds_param) {
        m_mds_param = dcel2_mds_init_empty();
    }

    std::clog <<__func__<<","<< __LINE__<<", m_mds_param:"<<m_mds_param<< std::endl;

    if (m_mds_param) {
        dcel2_mds_set_on_best(m_mds_param, &on_best, this);
        dcel2_mds_set_on_arbi_best(m_mds_param, &on_arbi_best, this);
        dcel2_mds_set_on_deep(m_mds_param, &on_deep, this);

        std::clog <<__func__<<","<< __LINE__<<", set callback success"<< std::endl;

    }
}



void QxMdMgr::Mds_Deinit(){

    if (m_mds_param) {
        dcel2_mds_destroy_param(m_mds_param);
    }

}

struct DataRecvTime {
	uint32_t time_sec;
	uint32_t time_ns;
};


void GetCurTime(DataRecvTime& t) {
	struct timespec tv1;
	clock_gettime(CLOCK_REALTIME, &tv1);
	t.time_sec = tv1.tv_sec;
	t.time_ns = tv1.tv_nsec;

}

void QxMdMgr::RemoteKeyDecodeMsg(const char* data, size_t size){
    DataRecvTime t;
    GetCurTime(t);
    parse_result_t re = dcel2_mds_parse(m_mds_param, data, size, &t);
    // std::clog <<__func__<<","<< __LINE__<<", recv size:"<<size<<", re:"<<re<< std::endl;
    if (e_parse_succ_and_key_updated == re) {
        int mode = 0;
        uint8_t key[64];
        if (dcel2_mds_get_key(m_mds_param, &mode, key)) {
            std::clog <<__func__<<","<< __LINE__<<",undate key:" << std::endl;
            hexDumpToClog((char *)key, 64);
        }
    }
}


void QxMdMgr::UdpHandler(void *ctx, const char* data, size_t size,  const std::string& source_ip, uint16_t source_port, int64_t t0){
    // hexDumpToClog(data, size);




    QxMdMgr *pthis = (QxMdMgr *)ctx;
    if (pthis->m_config.decrypt == DECRYPT_REMOTE_KEY){
        return pthis->RemoteKeyDecodeMsg(data,size);
    }

    pthis->DecodeMsg(data, size);
}


inline void QxMdMgr::decrypt(const char *input, int32_t msg_len, char *output) {
    for (int i = 0; i< msg_len; i++){
        output[i] = input[i] ^ m_calc_key[i%KEY_LENGTH];
    }
}

inline void QxMdMgr::GenKey_290(const char *msg, int16_t len){

    char *len_ptr = (char *)&len;

    static unsigned char DBL_MAX_MEN[8] = {0xff,0xff,0xff,0xff,0xff,0xff,0xef,0x7f};

    //len
    m_calc_key[0] = msg[0] ^ len_ptr[0];
    m_calc_key[1] = msg[1] ^ len_ptr[1];

    uint32_t date = getsysdate2();
    // uint32_t date = 20260408;
    char *date_ptr = (char *)&date;

    // hexDumpToClog((char *)g_key+49, 4);
    // hexDumpToClog(date_ptr, 4);
    // hexDumpToClog(msg + 177, 4);

    //date
    m_calc_key[177%64] = msg[177]^date_ptr[0]; //49
    m_calc_key[178%64] = msg[178]^date_ptr[1]; //50
    m_calc_key[179%64] = msg[179]^date_ptr[2]; //51
    m_calc_key[180%64] = msg[180]^date_ptr[3]; //52

    m_calc_key[193%64] = msg[193]; //extime 的最后一位是0

    // hexDumpToClog((char *)msg + 44, 44);

    m_calc_key[206%64] = msg[206] ^ DBL_MAX_MEN[0]; //14
    m_calc_key[207%64] = msg[207] ^ DBL_MAX_MEN[1]; //15
    m_calc_key[208%64] = msg[208] ^ DBL_MAX_MEN[2]; //16
    m_calc_key[209%64] = msg[209] ^ DBL_MAX_MEN[3]; //17
    m_calc_key[210%64] = msg[210] ^ DBL_MAX_MEN[4]; //18
    m_calc_key[211%64] = msg[211] ^ DBL_MAX_MEN[5]; //19
    m_calc_key[212%64] = msg[212] ^ DBL_MAX_MEN[6]; //20
    m_calc_key[213%64] = msg[213] ^ DBL_MAX_MEN[7]; //21

    m_calc_key[214%64] = msg[214]; //22
    m_calc_key[215%64] = msg[215]; //23
    m_calc_key[216%64] = msg[216]; //24
    m_calc_key[217%64] = msg[217]; //25


    m_calc_key[218%64] = msg[218] ^ DBL_MAX_MEN[0]; //26
    m_calc_key[219%64] = msg[219] ^ DBL_MAX_MEN[1]; //27
    m_calc_key[220%64] = msg[220] ^ DBL_MAX_MEN[2]; //28
    m_calc_key[221%64] = msg[221] ^ DBL_MAX_MEN[3]; //29
    m_calc_key[222%64] = msg[222] ^ DBL_MAX_MEN[4]; //30
    m_calc_key[223%64] = msg[223] ^ DBL_MAX_MEN[5]; //31
    m_calc_key[224%64] = msg[224] ^ DBL_MAX_MEN[6]; //32
    m_calc_key[225%64] = msg[225] ^ DBL_MAX_MEN[7]; //33


    m_calc_key[226%64] = msg[226] ^ DBL_MAX_MEN[0]; //34
    m_calc_key[227%64] = msg[227] ^ DBL_MAX_MEN[1]; //35
    m_calc_key[228%64] = msg[228] ^ DBL_MAX_MEN[2]; //36
    m_calc_key[229%64] = msg[229] ^ DBL_MAX_MEN[3]; //37
    m_calc_key[230%64] = msg[230] ^ DBL_MAX_MEN[4]; //38
    m_calc_key[231%64] = msg[231] ^ DBL_MAX_MEN[5]; //39
    m_calc_key[232%64] = msg[232] ^ DBL_MAX_MEN[6]; //40
    m_calc_key[233%64] = msg[233] ^ DBL_MAX_MEN[7]; //41

    m_calc_key[234%64] = msg[234] ^ DBL_MAX_MEN[0]; //42
    m_calc_key[235%64] = msg[235] ^ DBL_MAX_MEN[1]; //43
    m_calc_key[236%64] = msg[236] ^ DBL_MAX_MEN[2]; //44
    m_calc_key[237%64] = msg[237] ^ DBL_MAX_MEN[3]; //45
    m_calc_key[238%64] = msg[238] ^ DBL_MAX_MEN[4]; //46
    m_calc_key[239%64] = msg[239] ^ DBL_MAX_MEN[5]; //47
    m_calc_key[240%64] = msg[240] ^ DBL_MAX_MEN[6]; //48
    // m_calc_key[241%64] = msg[241] ^ DBL_MAX_MEN[7]; //49

    // m_calc_key[242%64] = msg[242] ^ DBL_MAX_MEN[0]; //50
    // m_calc_key[243%64] = msg[243] ^ DBL_MAX_MEN[1]; //51
    // m_calc_key[244%64] = msg[244] ^ DBL_MAX_MEN[2]; //52
    m_calc_key[245%64] = msg[245] ^ DBL_MAX_MEN[3]; //53
    m_calc_key[246%64] = msg[246] ^ DBL_MAX_MEN[4]; //54
    m_calc_key[247%64] = msg[247] ^ DBL_MAX_MEN[5]; //55
    m_calc_key[248%64] = msg[248] ^ DBL_MAX_MEN[6]; //56
    m_calc_key[249%64] = msg[249] ^ DBL_MAX_MEN[7]; //57

    m_calc_key[58] = msg[58]; 
    m_calc_key[59] = msg[59];
    m_calc_key[60] = msg[60];
    m_calc_key[61] = msg[61];
    m_calc_key[62] = msg[62];
    m_calc_key[63] = msg[63];

    m_calc_key[66%64] = msg[66]; //2
    m_calc_key[67%64] = msg[67]; //3
    m_calc_key[68%64] = msg[68]; //4
    m_calc_key[69%64] = msg[69]; //5
    m_calc_key[70%64] = msg[70]; //6
    m_calc_key[71%64] = msg[71]; //7

    m_calc_key[72%64] = msg[72]; //8
    m_calc_key[73%64] = msg[73]; //9
    m_calc_key[74%64] = msg[74]; //10
    m_calc_key[75%64] = msg[75]; //11
    m_calc_key[76%64] = msg[76]; //12
    m_calc_key[77%64] = msg[77]; //13

    std::clog<<"date:"<<date<<std::endl;
    hexDumpToClog((char *)m_calc_key, 64);

    m_key_gen_finished.store(true, std::memory_order_release);

}


void QxMdMgr::DecodeMsg(const char* msgptr, size_t msg_len) {
    int64_t local_time_ns_0 = get_nanoseconds();
    if(msg_len == 290){
        if (m_key_gen_finished.load(std::memory_order_relaxed)==false){
            GenKey_290(msgptr,(int16_t)msg_len);
            std::clog<<"Gen key success"<<std::endl;
            return;
        }
	}

	if (m_key_gen_finished.load(std::memory_order_relaxed) == false) {
        return;
    }

	char buffer[1024]={};

    if(msg_len == 370){

        decrypt(msgptr, msg_len, buffer);
        DceL2_370 *msgdata = (DceL2_370 *)buffer;
        QuotaInfo *cache = m_quota_cache.Find((char *)msgdata->code);
        if (cache == nullptr){
            return;
        }

        int64_t local_time_ns_1 = get_nanoseconds();
        uint64_t extime = dce_time_str_to_utc_s((char *)msgdata->extime); 

        m_fb_md->reset_entity();// 重置飞豹行情结构
        m_fb_md->set_local_timestamp(local_time_ns_0); //utc ns
        m_fb_md->set_max_depth(1);                              // 设置行情深度5
        m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
        m_fb_md->set_instrument_id((char *)msgdata->code);
        m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_DCE);
        m_fb_md->set_update_sec(extime/1000);
        m_fb_md->set_update_msec(extime%1000);
        m_fb_md->set_open(convert_price_if_volume_zero(msgdata->open, msgdata->volume));
        m_fb_md->set_close(convert_price_if_volume_zero(msgdata->last_price, msgdata->volume));
        m_fb_md->set_iopv(IOPV_TAG_LV1);
        m_fb_md->set_upper_limit_price(msgdata->upper);
        m_fb_md->set_down_limit_price(msgdata->lower);
        m_fb_md->set_pre_settlement(msgdata->pre_settle);
        m_fb_md->set_pre_close(msgdata->pre_close);
        m_fb_md->set_pre_open_interest(msgdata->pre_open_interest);
        m_fb_md->set_high_price(convert_price_if_volume_zero(msgdata->high, msgdata->volume));
        m_fb_md->set_low_price(convert_price_if_volume_zero(msgdata->low, msgdata->volume));
        m_fb_md->set_last_price(convert_price_if_volume_zero(msgdata->last_price, msgdata->volume));
        m_fb_md->set_volume(msgdata->volume);
        m_fb_md->set_turn_over(msgdata->turn_over);
        m_fb_md->set_open_interest(msgdata->open_interest);
        m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->ap1,msgdata->av1));
        m_fb_md->set_ask1_volume(msgdata->av1);
        m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->bp1,msgdata->bv1));
        m_fb_md->set_bid1_volume(msgdata->bv1);

        int64_t local_time_ns_2 = get_nanoseconds();

        m_fb_spi->on_msg(m_fb_md);
        int64_t local_time_ns_end = get_nanoseconds();
        
        cache->time = local_time_ns_0;
        cache->volume = msgdata->volume;
        cache->ap1 = msgdata->ap1;
        cache->bp1 = msgdata->bp1;
        cache->turn_over = msgdata->turn_over;               // 成交金额
        cache->open = msgdata->open;
        cache->high = msgdata->high;
        cache->low = msgdata->low;
        cache->last_price = msgdata->last_price;
        cache->upper = msgdata->upper;            
        cache->lower = msgdata->lower;
        cache->pre_settle = msgdata->pre_settle;
        cache->pre_close = msgdata->pre_close;
        cache->av1 = msgdata->av1;
        cache->bv1 = msgdata->bv1;
        cache->open_interest = msgdata->open_interest;
        cache->pre_open_interest = msgdata->pre_open_interest;      // 初始持仓量

        static uint64_t delay_sum = 0;
        static uint64_t delay_sum_0 = 0; //  -0
        static uint64_t delay_sum_1 = 0; // 0-1
        static uint64_t delay_sum_2 = 0; // 0-1

        static uint64_t count_stats = 0;
        delay_sum   += (local_time_ns_end - local_time_ns_0);
        delay_sum_0 += (local_time_ns_1 - local_time_ns_0);
        delay_sum_1 += (local_time_ns_2 - local_time_ns_1);
        delay_sum_2 += (local_time_ns_end - local_time_ns_2);

        int cpu = sched_getcpu();
        static uint64_t  period = 2000;
        count_stats ++;
        if (count_stats%period == 0){
            m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L1 CPU[%d],per [%llu] ave, all cost[%llu], section filter and decode[%llu],fill[%llu],send[%llu]\n",cpu, period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period);
            delay_sum   = 0;
            delay_sum_0 = 0;
            delay_sum_1 = 0;
            delay_sum_2 = 0;
        }



    }
    else if(msg_len == 366){


        decrypt(msgptr, msg_len, buffer);
        DceL2_366 *msgdata = (DceL2_366 *)buffer;
        QuotaInfo *cache = m_quota_cache.Find((char *)msgdata->code);
        if (cache == nullptr){
            return;
        }
        int64_t local_time_ns_1 = get_nanoseconds();

        m_fb_md->reset_entity();// 重置飞豹行情结构
        uint64_t extime = dce_time_str_to_utc_s((char *)msgdata->extime);
        m_fb_md->set_local_timestamp(local_time_ns_0); //utc ns
        m_fb_md->set_max_depth(5);                              // 设置行情深度5
        m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
        m_fb_md->set_instrument_id((char *)msgdata->code);
        m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_DCE);
        m_fb_md->set_update_sec(extime/1000);
        m_fb_md->set_update_msec(extime%1000);
        m_fb_md->set_open(convert_price_if_volume_zero(cache->open, cache->volume));
        m_fb_md->set_close(convert_price_if_volume_zero(cache->last_price, cache->volume));
        m_fb_md->set_iopv(IOPV_TAG_LV2);
        m_fb_md->set_upper_limit_price(cache->upper);
        m_fb_md->set_down_limit_price(cache->lower);
        m_fb_md->set_pre_settlement(cache->pre_settle);
        m_fb_md->set_pre_close(cache->pre_close);
        m_fb_md->set_pre_open_interest(cache->pre_open_interest);
        m_fb_md->set_high_price(convert_price_if_volume_zero(cache->high, cache->volume));
        m_fb_md->set_low_price(convert_price_if_volume_zero(cache->low, cache->volume));
        m_fb_md->set_last_price(convert_price_if_volume_zero(cache->last_price, cache->volume));
        m_fb_md->set_volume(cache->volume);
        m_fb_md->set_turn_over(cache->turn_over);
        m_fb_md->set_open_interest(cache->open_interest);

        m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata->ask[0].price,msgdata->ask[0].qty));
        m_fb_md->set_ask2_price(convert_price_if_volume_zero(msgdata->ask[1].price,msgdata->ask[1].qty));
        m_fb_md->set_ask3_price(convert_price_if_volume_zero(msgdata->ask[2].price,msgdata->ask[2].qty));
        m_fb_md->set_ask4_price(convert_price_if_volume_zero(msgdata->ask[3].price,msgdata->ask[3].qty));
        m_fb_md->set_ask5_price(convert_price_if_volume_zero(msgdata->ask[4].price,msgdata->ask[4].qty));
        m_fb_md->set_ask1_volume(msgdata->ask[0].qty);
        m_fb_md->set_ask2_volume(msgdata->ask[1].qty);
        m_fb_md->set_ask3_volume(msgdata->ask[2].qty);
        m_fb_md->set_ask4_volume(msgdata->ask[3].qty);
        m_fb_md->set_ask5_volume(msgdata->ask[4].qty);
        m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata->bid[0].price,msgdata->bid[0].qty));
        m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata->bid[1].price,msgdata->bid[1].qty));
        m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata->bid[2].price,msgdata->bid[2].qty));
        m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata->bid[3].price,msgdata->bid[3].qty));
        m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata->bid[4].price,msgdata->bid[4].qty));
        m_fb_md->set_bid1_volume(msgdata->bid[0].qty);
        m_fb_md->set_bid2_volume(msgdata->bid[1].qty);
        m_fb_md->set_bid3_volume(msgdata->bid[2].qty);
        m_fb_md->set_bid4_volume(msgdata->bid[3].qty);
        m_fb_md->set_bid5_volume(msgdata->bid[4].qty);


        int64_t local_time_ns_2 = get_nanoseconds();

        m_fb_spi->on_msg(m_fb_md);
        int64_t local_time_ns_end = get_nanoseconds();
        
        static uint64_t delay_sum = 0;
        static uint64_t delay_sum_0 = 0; //  -0
        static uint64_t delay_sum_1 = 0; // 0-1
        static uint64_t delay_sum_2 = 0; // 0-1

        static uint64_t count_stats = 0;
        delay_sum   += (local_time_ns_end - local_time_ns_0);
        delay_sum_0 += (local_time_ns_1 - local_time_ns_0);
        delay_sum_1 += (local_time_ns_2 - local_time_ns_1);
        delay_sum_2 += (local_time_ns_end - local_time_ns_2);

        int cpu = sched_getcpu();
        static uint64_t  period = 2000;
        count_stats ++;
        if (count_stats%period == 0){
            m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L2 CPU[%d],per [%llu] ave, all cost[%llu], section filter and decode[%llu],fill[%llu],send[%llu]\n",cpu, period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period);
            delay_sum   = 0;
            delay_sum_0 = 0;
            delay_sum_1 = 0;
            delay_sum_2 = 0;
        }

    }


}


