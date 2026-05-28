
#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>

#include <signal.h>


#include "common.h"



#include "gfex_nano.h"
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
    m_option_info = new OptionInfoFilter();
}

QxNanoMdMgr::~QxNanoMdMgr(){
    std::clog << std::nounitbuf;
    free(m_option_info);
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
    for (const auto& item : filter["underlying"]) {
        m_option_info->AddUnderlying(item);
    }
    m_config.filter_path = filter["filter_instrument_info"];

    std::clog <<__func__<<","<< __LINE__<<",filter_path[" <<m_config.filter_path<<"]"<< std::endl;

    bool r1 = m_option_info->LoadWithTitle(m_config.filter_path);
    std::clog <<__func__<<","<< __LINE__<<",Load ret[" <<r1<<"]"<< std::endl;

    auto & insts = m_option_info->GetAllInst();
    for(auto inst : insts){
        m_subscribe_insts.emplace_back(inst);
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

    CNanoMdReceiver mdSpi(index);

    mdSpi.RegistSender(m_fb_spi, m_fb_md);
    mdSpi.SetSubscribeInst(m_subscribe_insts);
    mdSpi.SetFilter(m_option_info);
    mdSpi.SetCpu(m_config.cpu_id[1]);

    CNanoGfexMdApi& refNanoGfexMdApi = CNanoGfexMdApi::CreateNanoGfexMdApi();

    std::string cfgname = m_config.config_path + "/config_" + std::to_string(index) + ".ini";
    std::clog <<__func__<<","<< __LINE__<<",cfgname:"<<cfgname << std::endl;
    int32_t r1 = refNanoGfexMdApi.NanoStart(mdSpi, cfgname.c_str());
    std::clog <<__func__<<","<< __LINE__<<",refNanoGfexMdApi.NanoStart,r1:"<<r1<< std::endl;
    if (r1){
        std::clog <<__func__<<","<< __LINE__<<",failed"<< std::endl;
        return;
    }

    //获取合约静态信息
    mdSpi.SaveInstStaticInfo(refNanoGfexMdApi);
    std::clog <<__func__<<","<< __LINE__<<",m_fb_initialized:"<<m_fb_initialized.load(std::memory_order_acquire)<<",g_running:"<<g_running.load(std::memory_order_relaxed)<< std::endl;
    if (m_fb_initialized.load(std::memory_order_acquire)){
        int32_t ret = 0;
        while(g_running.load(std::memory_order_relaxed) && (-1 != (ret = refNanoGfexMdApi.NanoRecv()))) {
            if (ret ==-1){
                break;
            }
        }
    }

    CNanoGfexMdApi::DestroyNanoGfexMdApi(refNanoGfexMdApi);
    std::clog <<__func__<<","<< __LINE__<<",finished,index:"<<index << std::endl;
}

CNanoMdReceiver::CNanoMdReceiver(int32_t instanceIndex):m_index(instanceIndex){
    m_ringbuffer = new LockFreeRingBuffer();
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

void CNanoMdReceiver::SaveInstStaticInfo(CNanoGfexMdApi& refNanoGfexMdApi){
    std::clog <<__func__<<","<< __LINE__<<",m_subscribe_insts size:"<< m_subscribe_insts.size()<< std::endl;
    for(auto & inst: m_subscribe_insts){
        NanoGfexInstStaticInfo refInstStaticInfo = {};
        if (0 == refNanoGfexMdApi.GetInstStaticInfo(inst.c_str(), refInstStaticInfo)){
            m_static_info.emplace(inst.c_str(), refInstStaticInfo);
            std::clog <<__func__<<","<< __LINE__<<",get static info for inst:"<<inst<< std::endl;
        }            
    }
}

void CNanoMdReceiver::RegistSender(cffex::fb::api::fb_i_md_spi *fb_spi, fb_market_data_t *fb_md){
    m_fb_spi = fb_spi;
    m_fb_md = fb_md;
    return;
}


void CNanoMdReceiver::SetSubscribeInst(std::vector<std::string> &vec){
    m_subscribe_insts.assign(vec.begin(), vec.end());
}


void CNanoMdReceiver::SetFilter( OptionInfoFilter * filter){
    m_option_info = filter;
}

void CNanoMdReceiver::SetCpu( int cpu){
    m_dispatcher_cpu = cpu;
}


// 过滤 行情，时间变小的行情和vol变小的，丢弃
// 注意，广期所无夜盘，所以暂时不考虑跨日时间变小
inline bool PRICE_COM(double a, double b){
    return std::fabs(a-b) < 1e-8;
}

inline int32_t CNanoMdReceiver::IsValidQuota(const std::string &inst, uint64_t extime, uint64_t vol, uint32_t level, double ap1, double bp1, uint32_t av1, uint32_t bv1){
    int32_t ret = COM_RESULT_OK;

    auto it = m_quota_merge.find(inst);
    if (it==m_quota_merge.end()){
        m_quota_merge.emplace(inst, QuotaMerge_t(extime, vol, level, ap1, bp1, av1, bv1));
    }
    else{
        auto & last_quota = it->second;
        if (extime < m_quota_merge.at(inst).time){
            return COM_RESULT_TIME_ERROR;
        }
        else if((vol < last_quota.volume)){
        // else if((vol < VALID_MAX_VOL) && (vol < last_quota.volume)){
            // m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_WARNING, "[%d],[%s],vol[%d][%d]\n",__LINE__,inst.c_str(),vol, last_quota.volume);
            return COM_RESULT_VOL_ERROR;
        }
        else if((last_quota.level == 5) &&(level == 1)
            && (vol == last_quota.volume) 
            && (last_quota.av1 == av1)
            && (last_quota.bv1 == bv1)
            && PRICE_COM(last_quota.ap1, ap1)
            && PRICE_COM(last_quota.bp1, bp1)){
            return COM_RESULT_USELESS_LV1;
        }
        last_quota.time   = extime;
        last_quota.volume = vol;
        last_quota.level  = level;
        last_quota.ap1    = ap1;
        last_quota.bp1    = bp1;
        last_quota.av1    = av1;
        last_quota.bv1    = bv1;
    }
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
        if (m_ringbuffer->try_pop(slot)) {
            DispatchMessage(slot);
        }
    }
}


// void CNanoMdReceiver::ProcessMsg() {
//     static int64_t cnt = 0;
//     static std::vector<Slot>  cache;
//     static int64_t max_cnt = 30000;

//     Slot slot;
//     while (true) {
//         if (m_ringbuffer->try_pop(slot)) {
//             if (cnt < max_cnt){
//                 cache.emplace_back(slot);
//             }
//             else if (cnt == max_cnt || (slot.extime/1000 > 145700)){

//                 std::string logfile = std::string("./")+ "nano_gfex_data_msg.log";
//                 std::ofstream logstream(logfile, std::ios::binary|std::ios::out|std::ios::app);

//                 //std::string line = std::string("[")+ func_name + "][" + info + "]\n";

//                 for (auto & item : cache){
//                     if (item.msg_type == MsgType::L1_MD) {
//                         const NanoGfexL1MdType & msgdata = item.l1_data;
//                         std::string line = std::string("L1_MD,") + std::string(msgdata.contract_name) + "," + std::to_string(item.extime) + ","+ std::to_string(item.timestamp_0) + "," + std::to_string(item.timestamp_1) + "," + std::to_string((int)item.msg_type)
//                                            +","+ std::to_string(msgdata.total_match_qty)+","+ std::to_string(msgdata.turn_over)+","+ std::to_string(msgdata.last_price)+","+ std::to_string(msgdata.open_interest)
//                                            +","+ std::to_string(msgdata.bid_price)+","+ std::to_string(msgdata.bid_qty)+","+ std::to_string(msgdata.ask_price)+","+ std::to_string(msgdata.ask_qty)+"\n";

//                         logstream.write(line.c_str(),line.size());
//                     } else if(item.msg_type == MsgType::L2_MD) {
//                         const NanoGfexL2MdType & msgdata = item.l2_data;
//                         std::string inst(msgdata.contract_name);
//                         std::string line = std::string("L2_MD,") + std::string(msgdata.contract_name) + "," + std::to_string(item.extime) + ","+ std::to_string(item.timestamp_0) + ","+ std::to_string(item.timestamp_1) + "," + std::to_string((int)item.msg_type)
//                                            +","+ std::to_string(msgdata.match_total_qty)+","+ std::to_string(msgdata.turn_over)+","+ std::to_string(msgdata.last_price)+","+ std::to_string(msgdata.open_interest)
//                                            +","+ std::to_string(msgdata.bid1_px)+","+ std::to_string(msgdata.bid1_vol)+","+ std::to_string(msgdata.bid2_px)+","+ std::to_string(msgdata.bid2_vol)+","+ std::to_string(msgdata.bid3_px)+","+ std::to_string(msgdata.bid3_vol)+","+ std::to_string(msgdata.bid4_px)+","+ std::to_string(msgdata.bid4_vol)+","+ std::to_string(msgdata.bid5_px)+","+ std::to_string(msgdata.bid5_vol)
//                                            +","+ std::to_string(msgdata.ask1_px)+","+ std::to_string(msgdata.ask1_vol)+","+ std::to_string(msgdata.ask2_px)+","+ std::to_string(msgdata.ask2_vol)+","+ std::to_string(msgdata.ask3_px)+","+ std::to_string(msgdata.ask3_vol)+","+ std::to_string(msgdata.ask4_px)+","+ std::to_string(msgdata.ask4_vol)+","+ std::to_string(msgdata.ask5_px)+","+ std::to_string(msgdata.ask5_vol)+"\n";

//                         logstream.write(line.c_str(),line.size());
//                     }
//                 }
//                 logstream.close();
//                 break;
//             }

//             cnt ++;

//         }
//     }
// }





void CNanoMdReceiver::DispatchMessage(const Slot& slot) {

    int64_t local_time_ns_1 = get_nanoseconds();    
    m_fb_md->reset_entity();      
    if (slot.msg_type == MsgType::L1_MD) {

        const NanoGfexL1MdType & msgdata = slot.l1_data;
        std::string inst(msgdata.contract_name);
        if (m_fb_md_interval.count(inst)==0){
            m_fb_md_interval.emplace(inst, fb_md_interval_t(msgdata.last_price,msgdata.last_price,msgdata.last_price));
        }

        auto &md_interval = m_fb_md_interval.at(inst);
        if (std::isnan(md_interval.open) && (msgdata.last_price>0)){
            md_interval.open = msgdata.last_price;
        }

        if (msgdata.last_price > md_interval.high){
            md_interval.high = msgdata.last_price;
        }

        if (msgdata.last_price < md_interval.low){
            md_interval.low = msgdata.last_price;
        }

        // if (inst == "ps2512-C-55000" || inst == "ps2512"){
        //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "l1 out [%d],[%s][%s],vol[%d],turnover[%f],oi[%d],last_price[%f],bp[%f],bv[%d],ap[%f],av[%d]\n",__LINE__,
        //                                 msgdata.contract_name, msgdata.send_time, msgdata.total_match_qty, msgdata.turn_over, msgdata.open_interest,
        //                                 msgdata.last_price, msgdata.bid_price, msgdata.bid_qty, msgdata.ask_price, msgdata.ask_qty, msgdata.avg_price);
        // }


        m_fb_md->set_local_timestamp(slot.timestamp_0); //utc ns
        m_fb_md->set_max_depth(1);                              // 设置行情深度5
        m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
        m_fb_md->set_instrument_id(msgdata.contract_name);
        m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_GFEX);
        m_fb_md->set_update_sec(slot.extime/1000);
        m_fb_md->set_update_msec(slot.extime%1000);
        m_fb_md->set_open(convert_price_if_zero(md_interval.open));
        m_fb_md->set_close(convert_price_if_zero(msgdata.last_price));

        m_fb_md->set_iopv(IOPV_TAG_LV1);

        if (m_static_info.count(inst) >0){
            auto &inst_item = m_static_info.at(inst);
            m_fb_md->set_upper_limit_price(inst_item.limit_up_px);
            m_fb_md->set_down_limit_price(inst_item.limit_down_px);
            m_fb_md->set_pre_settlement(inst_item.last_settlement_price);
            m_fb_md->set_pre_close(inst_item.last_closing_price);
            m_fb_md->set_pre_open_interest(inst_item.init_open_interst);
        }

        m_fb_md->set_high_price(convert_price_if_zero(md_interval.high));
        m_fb_md->set_low_price(convert_price_if_zero(md_interval.low));
        m_fb_md->set_last_price(convert_price_if_zero(msgdata.last_price));

        m_fb_md->set_volume(msgdata.total_match_qty);
        m_fb_md->set_turn_over(msgdata.turn_over);
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

        int cpu = sched_getcpu();
        static uint64_t  period = 1000;
        count_stats ++;
        if (count_stats%period == 0){
            m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L1, CPU[%d],per [%llu] ave, all cost[%llu], section filter[%llu],process[%llu],fill[%llu],send[%llu],sum[%llu],timeerr[%zu],volerr[%zu],useless_lv1[%zu]\n",cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period,count_stats, m_l1_time_err_sum, m_l1_vol_err_sum, m_l1_useless_lv1_sum);
            delay_sum   = 0;        
            delay_sum_0 = 0;
            delay_sum_1 = 0;
            delay_sum_2 = 0;
            delay_sum_3 = 0;
        }


    } else if(slot.msg_type == MsgType::L2_MD) {
        const NanoGfexL2MdType & msgdata = slot.l2_data;
        std::string inst(msgdata.contract_name);

        if (m_fb_md_interval.count(inst)==0){
            m_fb_md_interval.emplace(inst,fb_md_interval_t(msgdata.last_price,msgdata.last_price,msgdata.last_price));
        }

        auto &md_interval = m_fb_md_interval.at(inst);
        if (std::isnan(md_interval.open) && (msgdata.last_price>0)){
            md_interval.open = msgdata.last_price;
        }

        if (msgdata.last_price > md_interval.high){
            md_interval.high = msgdata.last_price;
        }

        if (msgdata.last_price < md_interval.low){
            md_interval.low = msgdata.last_price;
        }

        // m_fb_md->reset_entity();                                // 重置飞豹行情结构
        m_fb_md->set_local_timestamp(slot.timestamp_0); //utc ns
        m_fb_md->set_max_depth(1);                              // 设置行情深度5
        m_fb_md->set_guid(cffex::fb::api::FB_SET_GUID_TAG());   // 用于性能统计
        m_fb_md->set_instrument_id(msgdata.contract_name);
        m_fb_md->set_exchange_id(cffex::fb::api::FB_EXCHANGE_GFEX);
        m_fb_md->set_update_sec(slot.extime/1000);
        m_fb_md->set_update_msec(slot.extime%1000);

        m_fb_md->set_iopv(IOPV_TAG_LV2);

        // m_fb_md->set_pre_settlement();
        // m_fb_md->set_pre_close();
        // m_fb_md->set_pre_open_interest();

        m_fb_md->set_pre_open_interest(msgdata.open_interest);
        m_fb_md->set_open(convert_price_if_zero(md_interval.open));
        m_fb_md->set_close(convert_price_if_zero(msgdata.last_price));

        if (m_static_info.count(inst) >0){
            auto &inst_item = m_static_info.at(inst);
            m_fb_md->set_upper_limit_price(inst_item.limit_up_px);
            m_fb_md->set_down_limit_price(inst_item.limit_down_px);
            m_fb_md->set_pre_settlement(inst_item.last_settlement_price);
            m_fb_md->set_pre_close(inst_item.last_closing_price);
            m_fb_md->set_pre_open_interest(inst_item.init_open_interst);
        }

        // if (inst == "ps2512-C-55000" || inst == "ps2512"){
        //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "l2 out [%d],[%s][%s],vol[%d],turnover[%.2f],oi[%d],last_price[%.3f],bp[%.3f][%.3f][%.3f][%.3f][%.3f],bv[%d][%d][%d][%d][%d],ap[%.3f][%.3f][%.3f][%.3f][%.3f],av[%d][%d][%d][%d][%d]\n",__LINE__,
        //                                 msgdata.contract_name,msgdata.gen_time,msgdata.match_total_qty,msgdata.turn_over,msgdata.open_interest,
        //                                 msgdata.last_price,
        //                                 msgdata.bid1_px,msgdata.bid2_px,msgdata.bid3_px,msgdata.bid4_px,msgdata.bid5_px,
        //                                 msgdata.bid1_vol,msgdata.bid2_vol,msgdata.bid3_vol,msgdata.bid4_vol,msgdata.bid5_vol,
        //                                 msgdata.ask1_px,msgdata.ask2_px,msgdata.ask3_px,msgdata.ask4_px,msgdata.ask5_px,
        //                                 msgdata.ask1_vol,msgdata.ask2_vol,msgdata.ask3_vol,msgdata.ask4_vol,msgdata.ask5_vol);
        // }


        m_fb_md->set_high_price(convert_price_if_zero(md_interval.high));
        m_fb_md->set_low_price(convert_price_if_zero(md_interval.low));
        m_fb_md->set_last_price(convert_price_if_zero(msgdata.last_price));
        m_fb_md->set_volume(msgdata.match_total_qty);
        m_fb_md->set_turn_over(msgdata.turn_over);
        m_fb_md->set_open_interest(msgdata.open_interest);
        m_fb_md->set_ask1_price(convert_price_if_volume_zero(msgdata.ask1_px,msgdata.ask1_vol));
        m_fb_md->set_ask2_price(convert_price_if_volume_zero(msgdata.ask2_px,msgdata.ask2_vol));
        m_fb_md->set_ask3_price(convert_price_if_volume_zero(msgdata.ask3_px,msgdata.ask3_vol));
        m_fb_md->set_ask4_price(convert_price_if_volume_zero(msgdata.ask4_px,msgdata.ask4_vol));
        m_fb_md->set_ask5_price(convert_price_if_volume_zero(msgdata.ask5_px,msgdata.ask5_vol));
        m_fb_md->set_ask1_volume(msgdata.ask1_vol);
        m_fb_md->set_ask2_volume(msgdata.ask2_vol);
        m_fb_md->set_ask3_volume(msgdata.ask3_vol);
        m_fb_md->set_ask4_volume(msgdata.ask4_vol);
        m_fb_md->set_ask5_volume(msgdata.ask5_vol);
        m_fb_md->set_bid1_price(convert_price_if_volume_zero(msgdata.bid1_px,msgdata.bid1_vol));
        m_fb_md->set_bid2_price(convert_price_if_volume_zero(msgdata.bid2_px,msgdata.bid2_vol));
        m_fb_md->set_bid3_price(convert_price_if_volume_zero(msgdata.bid3_px,msgdata.bid3_vol));
        m_fb_md->set_bid4_price(convert_price_if_volume_zero(msgdata.bid4_px,msgdata.bid4_vol));
        m_fb_md->set_bid5_price(convert_price_if_volume_zero(msgdata.bid5_px,msgdata.bid5_vol));
        m_fb_md->set_bid1_volume(msgdata.bid1_vol);
        m_fb_md->set_bid2_volume(msgdata.bid2_vol);
        m_fb_md->set_bid3_volume(msgdata.bid3_vol);
        m_fb_md->set_bid4_volume(msgdata.bid4_vol);
        m_fb_md->set_bid5_volume(msgdata.bid5_vol);

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

        int cpu = sched_getcpu();
        static uint64_t  period = 1000;
        count_stats ++;
        if (count_stats%period == 0){
            m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO,"L2, CPU[%d],per [%llu] ave, all cost[%llu], section filter[%llu],process[%llu],fill[%llu],send[%llu],sum[%llu],timeerr[%zu],volerr[%zu]\n",
                        cpu,period, delay_sum/period, delay_sum_0/period, delay_sum_1/period, delay_sum_2/period, delay_sum_3/period, count_stats, m_l2_time_err_sum, m_l2_vol_err_sum);
            delay_sum = 0;        
            delay_sum_0 = 0;
            delay_sum_1 = 0;
            delay_sum_2 = 0;
            delay_sum_3 = 0;

        }



    }
}

void CNanoMdReceiver::OnNanoGfexL1Md(const NanoGfexL1MdType& msgdata){
    uint64_t local_time_ns = get_nanoseconds();
    std::string inst(msgdata.contract_name);
    uint64_t extime = time_str_to_utc_s(msgdata.send_time);

    // if (inst.length()>6){   //TODO debug 过滤期权
    //     return;
    // }

    if (!(inst.size()>0 && m_option_info->Filter(inst))){  //只收配置的合约
        return;
    }

    uint32_t valid = IsValidQuota(inst, extime, msgdata.total_match_qty, 1, msgdata.ask_price, msgdata.bid_price, msgdata.ask_qty, msgdata.bid_qty);
    // // if (inst == "ps2512-C-55000" || inst == "ps2512"){
    // // // if (valid==COM_RESULT_VOL_ERROR){
    // // // if (inst.length() == 6 && valid==COM_RESULT_VOL_ERROR){
    // // // if (valid == COM_RESULT_TIME_ERROR || valid==COM_RESULT_VOL_ERROR){
    // //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],[l1valid:%d],[%s][%s],vol[%d],turnover[%f],oi[%d],last_price[%f],bp[%f],bv[%d],ap[%f],av[%d]\n",__LINE__,
    // //                                 valid,msgdata.contract_name, msgdata.send_time, msgdata.total_match_qty, msgdata.turn_over, msgdata.open_interest,
    // //                                 msgdata.last_price, msgdata.bid_price, msgdata.bid_qty, msgdata.ask_price, msgdata.ask_qty, msgdata.avg_price);

    // //     // hexDumpToClog((const char *)&msgdata, sizeof(NanoGfexL1MdType));
    // // }

    // if (valid==COM_RESULT_TIME_ERROR){
    //     m_l1_time_err_sum ++;
    //     return;
    // }
    // else if(valid==COM_RESULT_VOL_ERROR){
    //     m_l1_vol_err_sum ++;
    //     return;
    // }
    // else if(valid==COM_RESULT_USELESS_LV1){
    //     m_l1_useless_lv1_sum ++;
    //     return;
    // }

    int64_t local_time_ns_0 = get_nanoseconds();

    m_ringbuffer->push_l1(msgdata,extime, local_time_ns, local_time_ns_0, valid);


}

void CNanoMdReceiver::OnNanoGfexL2Md(const NanoGfexL2MdType& msgdata){
    uint64_t local_time_ns = get_nanoseconds();

    std::string inst(msgdata.contract_name);

    // if (inst.length()>6){   //TODO debug 过滤期权
    //     return;
    // }

    if (!(inst.size()>0 && m_option_info->Filter(inst))){  //只收配置的合约
        return;
    }

    uint64_t extime = time_str_to_utc_s(msgdata.gen_time);

    uint32_t valid = IsValidQuota(inst, extime, msgdata.match_total_qty, 5, msgdata.ask1_px, msgdata.bid1_px, msgdata.ask1_vol, msgdata.bid1_vol);

    // // if (valid == COM_RESULT_TIME_ERROR || valid==COM_RESULT_VOL_ERROR){
    // // // if (inst == "lc2510-C-73000"){
    // // // if (inst.length() == 6){
    // //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],[l2valid:%d],[%s][%s],extime[%llu],vol[%d],turnover[%.2f],oi[%d],last_price[%.3f]\n",__LINE__,
    // //         valid,msgdata.contract_name,msgdata.gen_time,extime,msgdata.match_total_qty,msgdata.turn_over,msgdata.open_interest,msgdata.last_price);
    // // }

    // // if (inst == "ps2512-C-55000" || inst == "ps2512"){
    // //     m_fb_spi->get_xlog_helper()->xlog(FB_XLOG_INFO, "[%d],[%d],[%s][%s],vol[%d],turnover[%.2f],oi[%d],last_price[%.3f],bp[%.3f][%.3f][%.3f][%.3f][%.3f],bv[%d][%d][%d][%d][%d],ap[%.3f][%.3f][%.3f][%.3f][%.3f],av[%d][%d][%d][%d][%d]\n",__LINE__,
    // //                                 valid,msgdata.contract_name,msgdata.gen_time,msgdata.match_total_qty,msgdata.turn_over,msgdata.open_interest,
    // //                                 msgdata.last_price,
    // //                                 msgdata.bid1_px,msgdata.bid2_px,msgdata.bid3_px,msgdata.bid4_px,msgdata.bid5_px,
    // //                                 msgdata.bid1_vol,msgdata.bid2_vol,msgdata.bid3_vol,msgdata.bid4_vol,msgdata.bid5_vol,
    // //                                 msgdata.ask1_px,msgdata.ask2_px,msgdata.ask3_px,msgdata.ask4_px,msgdata.ask5_px,
    // //                                 msgdata.ask1_vol,msgdata.ask2_vol,msgdata.ask3_vol,msgdata.ask4_vol,msgdata.ask5_vol);
    // // }

    // static uint64_t time_err_sum = 0;
    // static uint64_t vol_err_sum = 0;
    // if (valid==COM_RESULT_TIME_ERROR){
    //     time_err_sum ++;
    //     return;
    // }
    // else if(valid==COM_RESULT_VOL_ERROR){
    //     vol_err_sum ++;
    //     return;
    // }

    int64_t local_time_ns_0 = get_nanoseconds();

    m_ringbuffer->push_l2(msgdata, extime, local_time_ns, local_time_ns_0, valid);




}




