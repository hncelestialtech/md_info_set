
#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>

#include <signal.h>


#include "common.h"


#include "dce_exanic.h"
#include "fb_md_plugin_api.h"
#include "fb_md_entity.h"
#include "fb_md_helper.h"
#include "fb_md_type.h"

// 定义插件的导出函数
#ifdef __cplusplus
extern "C" {
#endif

void *create() {
    return new QxNanoMdMgr();
}
void destroy(void *p) {
    delete (QxNanoMdMgr*)p;
}
void get_md_api_version(char version[32]) {
    strcpy(version, FEBAO_MD_API_VERSION);
}

#ifdef __cplusplus
}
#endif



std::atomic<bool> g_running = false;

QxNanoMdMgr::QxNanoMdMgr(): 
    m_fb_initialized(false),
    m_fb_spi(nullptr),
    m_fb_md(cffex::fb::api::market_data_entity::create_entity()){
    std::clog << std::unitbuf; //调试用
    std::clog <<__func__<<","<< __LINE__ << std::endl;
}

QxNanoMdMgr::~QxNanoMdMgr(){
    std::clog << std::nounitbuf;
}



int QxNanoMdMgr::init(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;
    LoadJsonCfg();
    
    for (int i = 0; i< 1; ++i){
        m_worker.emplace_back(new std::thread(&QxNanoMdMgr::Routine, this, i)); //和febao沟通，由于 on_msg的不可重入性，暂时只能有一个线程
    }

    m_fb_spi->on_ready();
    m_fb_initialized.store(true,std::memory_order_release);
    std::clog <<__func__<<","<< __LINE__<<",success"<< std::endl;
    return 0;
}

void QxNanoMdMgr::register_spi(cffex::fb::api::fb_i_md_spi *spi){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    m_fb_spi = spi;
}

void QxNanoMdMgr::release(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    g_running.store(false,std::memory_order_release);

    for (int i = 0; i< m_worker.size(); ++i){
        if(m_worker[i]->joinable()){
            m_worker[i]->join();
        }  
        delete m_worker[i];
        m_worker[i] = nullptr;
    }
    m_worker.clear();
}

void QxNanoMdMgr::connect(){
    std::clog <<__func__<<","<< __LINE__ << std::endl;
    g_running.store(true,std::memory_order_release);
}

void QxNanoMdMgr::subscribe_inst(const std::string &instrument_id, uint8_t exchange_id){
    std::clog <<__func__<<","<< __LINE__<<",instrument_id["<< instrument_id <<"],exchange_id["<<exchange_id <<"]" << std::endl;
    printf("subscribe_inst:instrument_id[%s],exchange_id[%hhu]\n",instrument_id.c_str(),exchange_id);
}

inline void QxNanoMdMgr::LoadJsonCfg(){
    std::clog <<__func__<<","<< __LINE__<< std::endl;

    cffex::fb::api::fb_md_config_helper *parser = m_fb_spi->get_config_helper();
    std::string md_jsoncfgfile;
    parser->get_attribute("file", "/md_config_path", md_jsoncfgfile);

    std::ifstream jsonfile(md_jsoncfgfile);
    nlohmann::json json_parser = nlohmann::json::parse(jsonfile);

    auto& eth = json_parser["nano"];
    m_config.config_path = eth["config_path"];

    std::clog <<__func__<<","<< __LINE__<<",config_path[" <<m_config.config_path<<"]"<< std::endl;

    auto& worker = json_parser["worker"];
    for (const auto& item : worker["cpuid"]) {
        m_config.cpu_id.emplace_back(item.get<uint32_t>());
        std::clog <<__func__<<","<< __LINE__<<",cpu[" <<m_config.cpu_id.back()<<"]"<< std::endl;        
    }

    auto& filter = json_parser["filter"];
    if(filter.contains("ob_level_mask")){
        m_config.ob_level_mask = filter["ob_level_mask"].get<uint32_t>();
    }
    else{
        m_config.ob_level_mask = OB_LEVEL_MASK_LV2;
    }

    m_config.filter_path = filter["filter_instrument_info"];

    std::clog <<__func__<<","<< __LINE__<<",filter_path[" <<m_config.filter_path<<"],ob_level_mask:"<<m_config.ob_level_mask<< std::endl;
    
    std::unordered_map<std::string, std::string> underlying_inst_map;
    LoadFebaoInstrumentInfo(m_config.filter_path, underlying_inst_map);
    std::clog <<__func__<<","<< __LINE__<<",Load ret[" <<underlying_inst_map.size()<<"]"<< std::endl;

    for(auto &itor : underlying_inst_map){
        m_subscribe_insts.emplace(itor.first);
        m_subscribe_insts.emplace(itor.second);

    }
    std::clog <<__func__<<","<< __LINE__<<",subscribe_insts[" <<m_subscribe_insts.size()<<"]"<< std::endl;
}

void on_exit(int sig){
    std::clog <<__func__<<","<< __LINE__<<",g_running:"<<g_running<< std::endl;
    g_running.store(false, std::memory_order_release);
}

void QxNanoMdMgr::Routine(int32_t index){

    std::clog <<__func__<<","<< __LINE__<<",index:"<<index<< std::endl;

    BindCPU(m_config.cpu_id[0]);

    // CNanoMdReceiver mdSpi(index);

    // mdSpi.RegistSender(m_fb_spi, m_fb_md);
    // mdSpi.SetSubscribeInst(m_subscribe_insts);

    // mdSpi.SetCpu(m_config.cpu_id[1]);
    // mdSpi.SetOBMask(m_config.ob_level_mask);
    // CNanoDceMdApi& refNanoDceMdApi = CNanoDceMdApi::CreateNanoDceMdApi();

    // std::string cfgname = m_config.config_path + "/config_" + std::to_string(index) + ".ini";
    // std::clog <<__func__<<","<< __LINE__<<",cfgname:"<<cfgname << std::endl;
    // int32_t r1 = refNanoDceMdApi.NanoStart(mdSpi, cfgname.c_str());
    // std::clog <<__func__<<","<< __LINE__<<",refNanoDceMdApi.NanoStart,r1:"<<r1<< std::endl;
    // if (r1){
    //     std::clog <<__func__<<","<< __LINE__<<",failed"<< std::endl;
    //     return;
    // }

    // static int cnt = 0;

    //获取合约静态信息
    // mdSpi.SaveInstStaticInfo(refNanoDceMdApi);
    // std::clog <<__func__<<","<< __LINE__<<",m_fb_initialized:"<<m_fb_initialized.load(std::memory_order_acquire)<<",g_running:"<<g_running.load(std::memory_order_relaxed)<< std::endl;
    // if (m_fb_initialized.load(std::memory_order_acquire)){
    //     int32_t ret = 0;
    //     while(g_running.load(std::memory_order_relaxed) && (-1 != (ret = refNanoDceMdApi.NanoRecv()))) {

    //         // if (cnt<1000){
    //         //    std::clog <<__func__<<","<< __LINE__<<",NanoRecv ret:"<<ret << std::endl; 
    //         //    cnt ++;
    //         // }

    //         if (ret == -1){
    //             break;
    //         }
    //     }
    // }

    // CNanoDceMdApi::DestroyNanoDceMdApi(refNanoDceMdApi);
    std::clog <<__func__<<","<< __LINE__<<",finished,index:"<<index << std::endl;
}

CNanoMdReceiver::CNanoMdReceiver(int32_t instanceIndex):m_index(instanceIndex){
    m_ringbuffer_lv1 = new LockFreeRingBuffer();
    m_ringbuffer_lv2 = new LockFreeRingBuffer();
    m_dispatcher = new std::thread(&CNanoMdReceiver::ProcessMsg, this);
}
CNanoMdReceiver::~CNanoMdReceiver(){
    // m_is_shutdown.store(true);
    // m_condition.notify_all();

    if(m_dispatcher->joinable()){
        m_dispatcher->join();
    }  
    delete m_dispatcher;
    m_dispatcher = nullptr;
}




// void CNanoMdReceiver::SaveInstStaticInfo(CNanoDceMdApi& refNanoDceMdApi){
//     std::clog <<__func__<<","<< __LINE__<<",m_subscribe_insts size:"<< m_subscribe_insts.size()<< std::endl;

//     for(auto & inst: m_subscribe_insts){
//         // NanoDceInstStaticInfo refInstStaticInfo = {};

//         std::clog <<__func__<<","<< __LINE__<<",before get static info for inst:"<<inst<< std::endl;
//         // if (0 == refNanoDceMdApi.GetInstStaticInfo(inst.c_str(), refInstStaticInfo)){
//         //     QuotaInfo *info = m_quota_cache.Find(inst.c_str());
//         //     if (info == nullptr){
//         //         std::clog <<__func__<<","<< __LINE__<<",cache miss info for inst:"<<inst<< std::endl;
//         //         continue;
//         //     }
//         //     info->last_settlement_price = refInstStaticInfo.last_settlement_price/10000.0;
//         //     info->upper = refInstStaticInfo.limit_up_px/10000.0;
//         //     info->lower = refInstStaticInfo.limit_down_px/10000.0;
//         //     info->init_open_interest = refInstStaticInfo.init_open_interst;
//         //     info->last_closing_price = refInstStaticInfo.last_closing_price/10000.0;

//         //     std::clog <<__func__<<","<< __LINE__<<",get static info for inst:"<<inst<< std::endl;
//         // }            
//     }
// }

void CNanoMdReceiver::RegistSender(cffex::fb::api::fb_i_md_spi *fb_spi, fb_market_data_t *fb_md){
    m_fb_spi = fb_spi;
    m_fb_md = fb_md;
    return;
}


void CNanoMdReceiver::SetSubscribeInst(std::set<std::string> &vec){
    uint32_t i = 0;
    for (auto inst : vec){
        m_subscribe_insts.emplace(inst);
        m_quota_cache.InitOnce(inst);
    }

    m_quota_cache.Sort();
}


void CNanoMdReceiver::SetCpu( int cpu){
    m_dispatcher_cpu = cpu;
}

void CNanoMdReceiver::SetOBMask( uint32_t mask){
    m_ob_level_mask = mask;
}


// 过滤 行情，时间变小的行情和vol变小的，丢弃
// 注意，广期所无夜盘，所以暂时不考虑跨日时间变小
inline bool PRICE_COM(double a, double b){
    return std::fabs(a-b) < 1e-8;
}

inline bool TIME_COM(uint64_t a, uint64_t b){
    return (a + (a < 60000000) * 240000000) < (b + (b < 60000000) * 240000000);
}


inline int32_t CNanoMdReceiver::IsValidQuota(QuotaInfo *cache, uint64_t extime, uint64_t vol, uint32_t level, double ap1, double bp1, uint32_t av1, uint32_t bv1){
    int32_t ret = COM_RESULT_OK;

    if (cache->level != 0){
        if (extime < cache->time){
            return COM_RESULT_TIME_ERROR;
        }

        if(vol < cache->volume){
            return COM_RESULT_VOL_ERROR;
        }

        if((vol == cache->volume)
            && (cache->av1 == av1)
            && (cache->bv1 == bv1)
            && PRICE_COM(cache->ap1, ap1)
            && PRICE_COM(cache->bp1, bp1)){
                if (cache->level = 1 && (level == 5)){
                    return COM_RESULT_REPEAT_L2;
                }
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

// 09:09:34.164      09:09:34
inline uint64_t time_str_to_utc_s(const char *time_str){
    // 直接访问字符数组
    const char* h  = time_str;
    const char* m  = time_str + 3;
    const char* s  = time_str + 6;
    const char* ms = time_str + 9;

    int hours        = (h[0] - '0') * 10 + (h[1] - '0');
    int minutes      = (m[0] - '0') * 10 + (m[1] - '0');
    int seconds      = (s[0] - '0') * 10 + (s[1] - '0');
    int milliseconds = (ms[0] - '0') * 100 +(ms[1] - '0') * 10 +(ms[2] - '0');

    const int64_t HOURS_TO_MS = 3600000LL;
    const int64_t MIN_TO_MS   = 60000LL;
    const int64_t SEC_TO_MS   = 1000LL;
    return (hours * HOURS_TO_MS) +(minutes * MIN_TO_MS) + (seconds * SEC_TO_MS) +milliseconds;
}


void CNanoMdReceiver::ProcessMsg() {
    BindCPU(m_dispatcher_cpu);
    Slot slot;
    while (true) {
        if (m_ringbuffer_lv1->try_pop(slot)) {
            // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "out [%d],[%d],\n",__LINE__,(int)slot.msg_type);
            DispatchMessage(slot);
        }

        // if (m_ringbuffer_lv2->try_pop(slot)) {
        //     // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "out [%d],[%d],\n",__LINE__,(int)slot.msg_type);
        //     DispatchMessage(slot);
        // }
    }
}


void CNanoMdReceiver::DispatchMessage(const Slot& slot) {

    int64_t local_time_ns_1 = get_nanoseconds();
    m_fb_md->reset_entity();// 重置飞豹行情结构
    if (slot.msg_type == MsgType::L1_MD) {

        const NanoDceL1MdType & msgdata = slot.l1_data;
        QuotaInfo *cache = m_quota_cache.Find((char *)msgdata.contract_name);
        // std::string inst((char *)msgdata.contract_name);
        // if (inst == "i2601"){
            // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "l1 out [%d],[%s][%llu],vol[%d],turnover[%.3f],oi[%d],last_price[%f],bp[%.3f],bv[%d],ap[%.3f],av[%d]\n",__LINE__,
            //                             (char *)msgdata.contract_name, msgdata.send_time, msgdata.total_qty, msgdata.turn_over/10000.0, msgdata.open_interest,
            //                             msgdata.last_price/10000.0, msgdata.bid_price/10000.0, msgdata.bid_qty, msgdata.ask_price/10000.0, msgdata.ask_qty, msgdata.avg_price/10000.0);
        // }

        m_fb_md->set_local_timestamp(slot.timestamp_0); //utc ns
        m_fb_md->set_max_depth(1);                              // 设置行情深度5
        m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
        m_fb_md->set_instrument_id((char *)msgdata.contract_name);
        m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_DCE);
        m_fb_md->set_update_sec(slot.extime/1000);
        m_fb_md->set_update_msec(slot.extime%1000);
        m_fb_md->set_open(convert_price_if_volume_zero(cache->open, msgdata.total_qty));
        m_fb_md->set_close(convert_price_if_volume_zero(msgdata.last_price/10000.0, msgdata.total_qty));
        m_fb_md->set_iopv(IOPV_TAG_LV1);
        m_fb_md->set_upper_limit_price(cache->upper);
        m_fb_md->set_down_limit_price(cache->lower);
        m_fb_md->set_pre_settlement(cache->last_settlement_price);
        m_fb_md->set_pre_close(cache->last_closing_price);
        m_fb_md->set_pre_open_interest(cache->init_open_interest);
        m_fb_md->set_high_price(convert_price_if_volume_zero(cache->high, msgdata.total_qty));
        m_fb_md->set_low_price(convert_price_if_volume_zero(cache->low, msgdata.total_qty));
        m_fb_md->set_last_price(convert_price_if_volume_zero(msgdata.last_price/10000.0, msgdata.total_qty));
        m_fb_md->set_volume(msgdata.total_qty);
        m_fb_md->set_turn_over(msgdata.turn_over/10000.0);
        m_fb_md->set_open_interest(msgdata.open_interest);
        m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata.ask_price/10000.0,msgdata.ask_qty));
        m_fb_md->set_ask1_volume(msgdata.ask_qty);
        m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata.bid_price/10000.0,msgdata.bid_qty));
        m_fb_md->set_bid1_volume(msgdata.bid_qty);

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
        delay_sum_0 += (slot.timestamp_1  - slot.timestamp_0);
        delay_sum_1 += (local_time_ns_1   - slot.timestamp_1);
        delay_sum_2 += (local_time_ns_2   - local_time_ns_1);
        delay_sum_3 += (local_time_ns_end - local_time_ns_2);


        static uint64_t  period = 1000;
        count_stats ++;
        if (count_stats%period == 0){
            int cpu = sched_getcpu();
            m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L1   CPU[%d],per [%llu] ave, all cost[%llu], section filter[%llu],process[%llu],fill[%llu],send[%llu],sum[%llu],l1_stats[%zu][%zu][%zu]\n",cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period,count_stats, m_l1_time_err_sum, m_l1_vol_err_sum, m_l1_repeat);
            delay_sum   = 0;
            delay_sum_0 = 0;
            delay_sum_1 = 0;
            delay_sum_2 = 0;
            delay_sum_3 = 0;
        }

        if ((slot.timestamp_1 - slot.timestamp_0) > 50000 || (local_time_ns_1 - slot.timestamp_1) > 50000 || (local_time_ns_2 - local_time_ns_1) > 50000){
            m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L1, filter[%llu],process[%llu],fill[%llu]\n",slot.timestamp_1 - slot.timestamp_0, local_time_ns_1 - slot.timestamp_1, local_time_ns_2 - local_time_ns_1);
        }




    } else if(slot.msg_type == MsgType::L2_BPMD) {
        const NanoDceL2ContractBestPriceMdType &msgdata = slot.l2_BestPricedata;
        QuotaInfo *cache = m_quota_cache.Find((char *)msgdata.contract_name);

        // if (inst == "i2601"){
            // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "l2bp out [%d],[%s][%s],vol[%d],turnover[%.2f],oi[%d],[%.3f][%.3f],last_price[%.3f],bp[%.3f],bv[%d],ap[%.3f],av[%d]\n",
            //         __LINE__, msgdata.contract_name,msgdata.gen_time,msgdata.match_tot_qty,msgdata.turnover,msgdata.open_interest,
            //         msgdata.high_price,msgdata.low_price,msgdata.last_price,msgdata.ask_price,msgdata.ask_qty,msgdata.bid_price,msgdata.bid_qty);
        // }

        m_fb_md->set_local_timestamp(slot.timestamp_0); //utc ns
        m_fb_md->set_max_depth(5);                              // 设置行情深度5
        m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
        m_fb_md->set_instrument_id((char *)msgdata.contract_name);
        m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_DCE);
        m_fb_md->set_update_sec(slot.extime/1000);
        m_fb_md->set_update_msec(slot.extime%1000);
        m_fb_md->set_iopv(IOPV_TAG_LV1);
        m_fb_md->set_open(convert_price_if_volume_zero(cache->open, msgdata.match_tot_qty));
        m_fb_md->set_close(convert_price_if_volume_zero(msgdata.last_price, msgdata.match_tot_qty));
        m_fb_md->set_upper_limit_price(cache->upper);
        m_fb_md->set_down_limit_price(cache->lower);
        m_fb_md->set_pre_settlement(cache->last_settlement_price);
        m_fb_md->set_pre_close(cache->last_closing_price);
        m_fb_md->set_pre_open_interest(cache->init_open_interest);
        m_fb_md->set_high_price(convert_price_if_volume_zero(cache->high, msgdata.match_tot_qty));
        m_fb_md->set_low_price(convert_price_if_volume_zero(cache->low, msgdata.match_tot_qty));
        m_fb_md->set_last_price(convert_price_if_volume_zero(msgdata.last_price, msgdata.match_tot_qty));
        m_fb_md->set_volume(msgdata.match_tot_qty);
        m_fb_md->set_turn_over(msgdata.turnover);
        m_fb_md->set_open_interest(msgdata.open_interest);
        m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata.ask_price,msgdata.ask_qty));
        m_fb_md->set_ask1_volume(msgdata.ask_qty);
        m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata.bid_price,msgdata.bid_qty));
        m_fb_md->set_bid1_volume(msgdata.bid_qty);

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
        delay_sum_0 += (slot.timestamp_1   - slot.timestamp_0);
        delay_sum_1 += (local_time_ns_1   - slot.timestamp_1);
        delay_sum_2 += (local_time_ns_2   - local_time_ns_1);
        delay_sum_3 += (local_time_ns_end - local_time_ns_2);


        static uint64_t  period = 2000;
        count_stats ++;
        if (count_stats%period == 0){
            int cpu = sched_getcpu();            
            m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L2BP CPU[%d],per [%llu] ave, all cost[%llu], section filter[%llu],process[%llu],fill[%llu],send[%llu],sum[%llu],l2_stats[%zu][%zu][%zu]\n",cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period,count_stats,m_l2_time_err_sum, m_l2_vol_err_sum, m_l2_repeat);
            delay_sum   = 0;
            delay_sum_0 = 0;
            delay_sum_1 = 0;
            delay_sum_2 = 0;
            delay_sum_3 = 0;
        }

    } else if(slot.msg_type == MsgType::L2_MD) {
        const NanoL2MdType &msgdata = slot.l2_data;
        // std::string inst(msgdata.contract_name);

        QuotaInfo *cache = m_quota_cache.Find((char *)msgdata.contract_name);

        // if (inst == "i2601"){
            // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "l2lv out [%d],[%s][%s],vol[%d],turnover[%.2f],oi[%d],[%.3f][%.3f],last_price[%.3f],bp[%.3f][%.3f][%.3f][%.3f][%.3f],bv[%d][%d][%d][%d][%d],ap[%.3f][%.3f][%.3f][%.3f][%.3f],av[%d][%d][%d][%d][%d]\n",
            //         __LINE__, msgdata.contract_name,msgdata.gen_time,msgdata.volume,msgdata.turn_over,msgdata.open_interest,
            //         msgdata.high,msgdata.low,msgdata.last_price,
            //         msgdata.bp1,msgdata.bp2,msgdata.bp3,msgdata.bp4,msgdata.bp5,
            //         msgdata.bv1,msgdata.bv2,msgdata.bv3,msgdata.bv4,msgdata.bv5,
            //         msgdata.ap1,msgdata.ap2,msgdata.ap3,msgdata.ap4,msgdata.ap5,
            //         msgdata.av1,msgdata.av2,msgdata.av3,msgdata.av4,msgdata.av5);
        // }
        
        m_fb_md->set_local_timestamp(slot.timestamp_0); //utc ns
        m_fb_md->set_max_depth(5);                              // 设置行情深度5
        m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
        m_fb_md->set_instrument_id(msgdata.contract_name);
        m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_DCE);
        m_fb_md->set_update_sec(slot.extime/1000);
        m_fb_md->set_update_msec(slot.extime%1000);
        m_fb_md->set_iopv(IOPV_TAG_LV2);
        m_fb_md->set_open(convert_price_if_volume_zero(msgdata.open,msgdata.volume));
        m_fb_md->set_close(convert_price_if_volume_zero(msgdata.last_price,msgdata.volume));
        m_fb_md->set_upper_limit_price(cache->upper);
        m_fb_md->set_down_limit_price(cache->lower);
        m_fb_md->set_pre_settlement(cache->last_settlement_price);
        m_fb_md->set_pre_close(cache->last_closing_price);
        m_fb_md->set_pre_open_interest(cache->init_open_interest);
        m_fb_md->set_high_price(convert_price_if_volume_zero(cache->high,msgdata.volume));
        m_fb_md->set_low_price(convert_price_if_volume_zero(cache->low,msgdata.volume));
        m_fb_md->set_last_price(convert_price_if_volume_zero(msgdata.last_price,msgdata.volume));
        m_fb_md->set_volume(msgdata.volume);
        m_fb_md->set_turn_over(msgdata.turn_over);
        m_fb_md->set_open_interest(msgdata.open_interest);
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
        m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata.bp1,msgdata.bv1));
        m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata.bp2,msgdata.bv2));
        m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata.bp3,msgdata.bv3));
        m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata.bp4,msgdata.bv4));
        m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata.bp5,msgdata.bv5));
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
        delay_sum   += (local_time_ns_end - slot.timestamp_0);
        delay_sum_0 += (slot.timestamp_1  - slot.timestamp_0);
        delay_sum_1 += (local_time_ns_1   - slot.timestamp_1);
        delay_sum_2 += (local_time_ns_2   - local_time_ns_1);
        delay_sum_3 += (local_time_ns_end - local_time_ns_2);

        int cpu = sched_getcpu();
        static uint64_t  period = 2000;
        count_stats ++;
        if (count_stats%period == 0){
            m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L2LV CPU[%d],per [%llu] ave, all cost[%llu], section filter[%llu],process[%llu],fill[%llu],send[%llu],sum[%llu],l2_stats[%zu][%zu][%zu]\n",cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period,count_stats,m_l2_time_err_sum, m_l2_vol_err_sum, m_l2_repeat);
            delay_sum   = 0;
            delay_sum_0 = 0;
            delay_sum_1 = 0;
            delay_sum_2 = 0;
            delay_sum_3 = 0;
        }

    }
}


void CNanoMdReceiver::OnNanoDceL1Md(const NanoDceL1MdType &msgdata){
    // uint64_t local_time_ns = get_nanoseconds();
    // std::clog <<__func__<<","<< __LINE__<<",send_time:"<<msgdata.send_time<< std::endl;

    QuotaInfo *cache = m_quota_cache.Find((char *)msgdata.contract_name);
    if (cache == nullptr){
        return;
    }

    if (unlikely((!(cache->open > 0)) && msgdata.last_price != DBL_MAX && msgdata.last_price >0)){
        cache->open = msgdata.last_price/10000.0;
    }

    uint64_t sec = msgdata.send_time/1000000000L;
    uint64_t sec2 = (sec - (sec/86400)*86400 ) + 8*3600;
    uint64_t extime = sec2*1000L + (msgdata.send_time%1000000000L)/1000000L;

    uint32_t valid = IsValidQuota(cache, extime, msgdata.total_qty, 1, msgdata.ask_price/10000.0, msgdata.bid_price/10000.0, msgdata.ask_qty, msgdata.bid_qty);
    // if (inst == "i2601"){
        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "OnNanoDceL1Md [%d][%llu],valid[%d],[%s][%llu],last_vol[%d],vol[%d],turnover[%.2f],oi[%d],last_price[%.3f],bp[%.3f],bv[%d],ap[%.3f],av[%d],avp[%.3f]\n",__LINE__,local_time_ns,
        //     valid,(char *)msgdata.contract_name, msgdata.send_time, msgdata.last_qty, msgdata.total_qty, msgdata.turn_over/10000.0, msgdata.open_interest,
        //     msgdata.last_price/10000.0, msgdata.bid_price/10000.0, msgdata.bid_qty, msgdata.ask_price/10000.0, msgdata.ask_qty, msgdata.avg_price/10000.0);

        // hexDumpToClog((const char *)&msgdata, 44);
    // }

    if(valid != COM_RESULT_OK){
        if(valid==COM_RESULT_REPEAT_L1){
            m_l1_repeat ++;
            return;
        }
        else if (valid==COM_RESULT_TIME_ERROR){
            m_l1_time_err_sum ++;
            return;
        }
        else if(valid==COM_RESULT_VOL_ERROR){
            m_l1_vol_err_sum ++;
            return;
        }
    }

    // if (inst == "i2601"){
        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "OnNanoDceL1Md before [%d][%llu],valid[%d],[%s][%llu],last_vol[%d],vol[%d],turnover[%.2f],oi[%d],last_price[%.3f],bp[%.3f],bv[%d],ap[%.3f],av[%d],avp[%.3f]\n",__LINE__,local_time_ns,
        //     valid,(char *)msgdata.contract_name, msgdata.send_time, msgdata.last_qty, msgdata.total_qty, msgdata.turn_over/10000.0, msgdata.open_interest,
        //     msgdata.last_price/10000.0, msgdata.bid_price/10000.0, msgdata.bid_qty, msgdata.ask_price/10000.0, msgdata.ask_qty, msgdata.avg_price/10000.0);
    // }
    uint64_t local_time_ns = get_nanoseconds();
    
    m_ringbuffer_lv1->push_l1(msgdata, extime, local_time_ns, 0);
}

void CNanoMdReceiver::OnNanoDceL2ContractBestPriceMd(const NanoDceL2ContractBestPriceMdType& msgdata){
    uint64_t local_time_ns = get_nanoseconds();
    // std::string inst((char *)msgdata.contract_name);
    QuotaInfo *cache = m_quota_cache.Find((char *)msgdata.contract_name);
    if (cache==nullptr){
        return;
    }

    uint64_t extime = time_str_to_utc_s((char *)msgdata.gen_time);
    uint32_t valid = IsValidQuota(cache, extime, msgdata.match_tot_qty, 5, msgdata.ask_price, msgdata.bid_price, msgdata.ask_qty, msgdata.bid_qty); //只有一档，当作L1来过滤
    // if (inst == "i2601"){
        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "OnNanoDceL2ContractBestPriceMd[%d][%llu],valid[%d],[%s][%s],vol[%d],turnover[%.2f],oi[%d],last_price[%.3f],[%.3f][%.3f][%d],bp[%.3f],bv[%d],ap[%.3f],av[%d]\n",__LINE__,local_time_ns,
        //                             valid, (char*)msgdata.contract_name,(char *)msgdata.gen_time,msgdata.match_tot_qty,msgdata.turnover,msgdata.open_interest,
        //                             msgdata.last_price,msgdata.high_price,msgdata.low_price,msgdata.last_match_qty,
        //                             msgdata.bid_price,msgdata.bid_qty,msgdata.ask_price,msgdata.ask_qty);


    // hexDumpToClog((const char *)&msgdata, 32);

    // }


    if (unlikely((!(cache->open > 0)) && msgdata.last_price != DBL_MAX && msgdata.last_price >0)){
        cache->open = msgdata.last_price;
    }

    if (unlikely(valid == COM_RESULT_TIME_ERROR)){
        m_l1_time_err_sum ++;
        return;
    }
    else if (unlikely(valid == COM_RESULT_VOL_ERROR)){
        m_l1_vol_err_sum ++;
        return;
    }

    cache->high = msgdata.high_price;
    cache->low = msgdata.low_price;
    cache->volume = msgdata.match_tot_qty;
    cache->turn_over = msgdata.turnover;
    cache->last_price = msgdata.last_price;
    cache->open_interest = msgdata.open_interest;

    if(valid==COM_RESULT_REPEAT_L2){
        m_l2_repeat ++;
        return;
    }

    // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "OnNanoDceL2ContractBestPriceMd before send[%d][%llu],valid[%d],[%s][%s],vol[%d],turnover[%.2f],oi[%d],last_price[%.3f],[%.3f][%.3f][%d],bp[%.3f],bv[%d],ap[%.3f],av[%d]\n",__LINE__,local_time_ns,
    //                             valid, (char*)msgdata.contract_name,(char *)msgdata.gen_time,msgdata.match_tot_qty,msgdata.turnover,msgdata.open_interest,
    //                             msgdata.last_price,msgdata.high_price,msgdata.low_price,msgdata.last_match_qty,
    //                             msgdata.bid_price,msgdata.bid_qty,msgdata.ask_price,msgdata.ask_qty);


    int64_t local_time_ns_0 = get_nanoseconds();
    // m_ringbuffer_lv2->push_l2(msgdata, extime, local_time_ns, local_time_ns_0);
}

void CNanoMdReceiver::OnNanoDceL2DeepQuoteMd(const NanoDceL2DeepQuoteMdType& msgdata){
    if (m_ob_level_mask==OB_LEVEL_MASK_LV1){
        return;
    }

    uint64_t local_time_ns = get_nanoseconds();

    QuotaInfo *cache = m_quota_cache.Find((char *)msgdata.contract_name);
    if (cache==nullptr){
        return;
    }

    uint64_t extime = time_str_to_utc_s((char *)msgdata.gen_time);

    // if (inst == "i2601"){
        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "OnNanoDceL2DeepQuoteMd in[%d][%llu],[%s][%s],bp[%.3f][%.3f][%.3f][%.3f][%.3f],bv[%d][%d][%d][%d][%d],ap[%.3f][%.3f][%.3f][%.3f][%.3f],av[%d][%d][%d][%d][%d]\n",__LINE__,local_time_ns,
        //                             (char*)msgdata.contract_name,(char*)msgdata.gen_time,
        //                             msgdata.bid_1,msgdata.bid_2,msgdata.bid_3,msgdata.bid_4,msgdata.bid_5,
        //                             msgdata.bid_1_qty,msgdata.bid_2_qty,msgdata.bid_3_qty,msgdata.bid_4_qty,msgdata.bid_5_qty,
        //                             msgdata.ask_1,msgdata.ask_2,msgdata.ask_3,msgdata.ask_4,msgdata.ask_5,
        //                             msgdata.ask_1_qty,msgdata.ask_2_qty,msgdata.ask_3_qty,msgdata.ask_4_qty,msgdata.ask_5_qty);

        // hexDumpToClog((const char *)&msgdata, 32);

    // }
    NanoL2MdType L2Md={};
    memcpy(L2Md.contract_name, msgdata.contract_name, sizeof(L2Md.contract_name));
    memcpy(L2Md.gen_time, msgdata.gen_time, sizeof(L2Md.gen_time));

    L2Md.open          = cache->open;
    L2Md.high          = cache->high;
    L2Md.low           = cache->low;
    L2Md.last_price    = cache->last_price;
    L2Md.volume        = cache->volume;
    L2Md.turn_over     = cache->turn_over;
    L2Md.open_interest = cache->open_interest;

    L2Md.bp1 = msgdata.bid_1;
    L2Md.bp2 = msgdata.bid_2;
    L2Md.bp3 = msgdata.bid_3;
    L2Md.bp4 = msgdata.bid_4;
    L2Md.bp5 = msgdata.bid_5;
    L2Md.bv1 = msgdata.bid_1_qty;
    L2Md.bv2 = msgdata.bid_2_qty;
    L2Md.bv3 = msgdata.bid_3_qty;
    L2Md.bv4 = msgdata.bid_4_qty;
    L2Md.bv5 = msgdata.bid_5_qty;

    L2Md.ap1 = msgdata.ask_1;    
    L2Md.ap2 = msgdata.ask_2;
    L2Md.ap3 = msgdata.ask_3;
    L2Md.ap4 = msgdata.ask_4;
    L2Md.ap5 = msgdata.ask_5;
    L2Md.av1 = msgdata.ask_1_qty;    
    L2Md.av2 = msgdata.ask_2_qty;
    L2Md.av3 = msgdata.ask_3_qty;
    L2Md.av4 = msgdata.ask_4_qty;
    L2Md.av5 = msgdata.ask_5_qty;

    // if (inst == "i2601"){
        // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "OnNanoDceL2DeepQuoteMd before push[%d][%llu],[%s][%s],open[%.3f][%.3f][%.3f],vol[%d][%.3f][%d],oi[%d],bp[%.3f][%.3f][%.3f][%.3f][%.3f],bv[%d][%d][%d][%d][%d],ap[%.3f][%.3f][%.3f][%.3f][%.3f],av[%d][%d][%d][%d][%d]\n",__LINE__,local_time_ns,
        //                             L2Md.contract_name,L2Md.gen_time,L2Md.high,L2Md.low,L2Md.last_price,L2Md.volume,L2Md.turn_over,L2Md.open_interest,
        //                             L2Md.bp1,L2Md.bp2,L2Md.bp3,L2Md.bp4,L2Md.bp5,
        //                             L2Md.bv1,L2Md.bv2,L2Md.bv3,L2Md.bv4,L2Md.bv5,
        //                             L2Md.ap1,L2Md.ap2,L2Md.ap3,L2Md.ap4,L2Md.ap5,
        //                             L2Md.av1,L2Md.av2,L2Md.av3,L2Md.av4,L2Md.av5);
    // }

    int64_t local_time_ns_0 = get_nanoseconds();

    // m_ringbuffer_lv2->push_l2(L2Md, extime, local_time_ns, local_time_ns_0);
}

void CNanoMdReceiver::OnNanoDceL2ArbBestPriceMd(const NanoDceL2ArbBestPriceMdType& msgdata){
}


void CNanoMdReceiver::OnNanoDceL2OrderStatisticsMd(const NanoDceL2OrderStatisticsMdType& msgdata){
}

void CNanoMdReceiver::OnNanoDceL2DeepOrderVolumeMd(const NanoDceL2DeepOrderVolumeMdType& msgdata){
}

void CNanoMdReceiver::OnNanoDceL2SegQuotaMd(const NanoDceL2SegQuotaMdType& msgdata){
}



void CNanoMdReceiver::OnEvent(const NanoEventType& refEventType){
    uint64_t local_time_ns = get_nanoseconds();
    m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d][%llu],refEventType[%d]\n",__LINE__,local_time_ns,refEventType);

}

