
#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>

#include "common.h"

#include "nano_gfex_recevier.h"
#include "nano_gfex_typedef.h"


// 09:09:34.164      09:09:34
inline uint64_t time_str_to_utc_s(const char *s){
    std::string str(s);

    if ((str.size() != 8) && (str.size() != 12)){
        std::clog <<__func__<<","<< __LINE__<<",error time,s:"<< s << std::endl;
        return 0;
    }

    uint64_t hh = std::atol(str.substr(0,2).c_str());
    uint64_t mm = std::atol(str.substr(3,2).c_str());
    uint64_t ss = std::atol(str.substr(6,2).c_str());
    uint64_t ms = 0;
    if (str.size() == 12){
        ms = std::atoi(str.substr(9,3).c_str());
    }
    return (hh*3600 + mm*60 + ss)*1000 + ms;
}

CNanoMdReceiver::CNanoMdReceiver(int32_t instanceIndex){
    m_index = instanceIndex;
}
CNanoMdReceiver::~CNanoMdReceiver(){

}

void CNanoMdReceiver::SaveInstStaticInfo(CNanoGfexMdApi& refNanoGfexMdApi){
    for(auto & itor1 : m_febao_inst){
        std::string product = itor1.first;
        auto &insts         = itor1.second;
        for(auto & inst: insts){
            NanoGfexInstStaticInfo refInstStaticInfo = {};
            if (0 == refNanoGfexMdApi.GetInstStaticInfo(inst.c_str(), refInstStaticInfo)){
                m_static_info.emplace(inst.c_str(), refInstStaticInfo);
            }
        }
    }
}

void CNanoMdReceiver::RegistMdCallback(NanoGfexMdHandler handler){
    m_md_handler  = handler;
    return;
}

// 过滤 行情，时间变小的行情和vol变小的，丢弃
// 注意，广期所无夜盘，所以暂时不考虑跨日时间变小
inline bool CNanoMdReceiver::IsValidQuota(const std::string &inst, uint64_t extime, uint64_t vol){
    if (m_quota_merge.count(inst)==0){
        m_quota_merge.emplace(inst, QuotaMerge_t(extime, vol));
    }
    else{
        auto & last_quota = m_quota_merge.at(inst);
        if ((extime < m_quota_merge.at(inst).time) || (vol < last_quota.volume)){
            last_quota.time = extime;
            last_quota.volume = vol;
            return false;
        }
    }
    return true;
}



void CNanoMdReceiver::OnNanoGfexL1Md(const NanoGfexL1MdType& msgdata){
    uint64_t local_time_ns = get_nanoseconds();

    std::string inst(msgdata.contract_name);
    uint64_t extime = time_str_to_utc_s(msgdata.send_time);
    if (IsValidQuota(inst, extime, msgdata.total_match_qty)==false){
        return;
    }

    if (m_md_interval.count(inst)==0){
        m_md_interval.emplace(inst, md_interval_t(msgdata.last_price,msgdata.last_price,msgdata.last_price));
    }

    auto &md_interval = m_md_interval.at(inst);
    if (std::isnan(md_interval.open) && (msgdata.last_price>0)){
        md_interval.open = msgdata.last_price;
    }

    if (msgdata.last_price > md_interval.high){
        md_interval.high = msgdata.last_price;
    }

    if (msgdata.last_price < md_interval.low){
        md_interval.low = msgdata.last_price;
    }

    int64_t local_time_ns_0 = get_nanoseconds();

    static NanoGfexMd md_data;
    strncpy(md_data.inst_id, msgdata.contract_name, sizeof(msgdata.contract_name));
    md_data.md_level = 1;
    md_data.extime = extime;
    md_data.open = md_interval.open;
    md_data.high = md_interval.high;
    md_data.low = md_interval.low;
    md_data.last_price = msgdata.last_price;
    md_data.last_match_qty = msgdata.last_match_qty;
    md_data.volume = msgdata.total_match_qty;
    md_data.turn_over = msgdata.turn_over;
    md_data.open_interest = msgdata.open_interest;
    md_data.open_interest_change = msgdata.open_interest_change;
    md_data.ap[0] = msgdata.ask_price;
    md_data.bp[0] = msgdata.bid_price;
    md_data.av[0] = msgdata.ask_qty;
    md_data.bv[0] = msgdata.bid_qty;

    md_data.avg_price = msgdata.avg_price;
    md_data.contract_no = msgdata.contract_no;
    md_data.L1_update_flag = msgdata.update_flag;

    md_data.ap[1] = FLOAT64_NAN;
    md_data.ap[2] = FLOAT64_NAN;
    md_data.ap[3] = FLOAT64_NAN;
    md_data.ap[4] = FLOAT64_NAN;

    md_data.bp[1] = FLOAT64_NAN;
    md_data.bp[2] = FLOAT64_NAN;
    md_data.bp[3] = FLOAT64_NAN;
    md_data.bp[4] = FLOAT64_NAN;

    md_data.av[1] = 0;
    md_data.av[2] = 0;
    md_data.av[3] = 0;
    md_data.av[4] = 0;

    md_data.bv[1] = 0;
    md_data.bv[2] = 0;
    md_data.bv[3] = 0;
    md_data.bv[4] = 0;

    md_data.buy_imply_qty[0] = 0;
    md_data.buy_imply_qty[1] = 0;
    md_data.buy_imply_qty[2] = 0;
    md_data.buy_imply_qty[3] = 0;
    md_data.buy_imply_qty[4] = 0;

    md_data.sell_imply_qty[0] = 0;
    md_data.sell_imply_qty[1] = 0;
    md_data.sell_imply_qty[2] = 0;
    md_data.sell_imply_qty[3] = 0;
    md_data.sell_imply_qty[4] = 0;
    md_data.local_time = local_time_ns;
    m_md_handler(&md_data);
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
    delay_sum_4 += (local_time_ns_end - local_time_ns_0);

    int cpu = sched_getcpu();
    static uint64_t  period = 10000;
    count_stats ++;
    if (count_stats%period == 0){
        delay_sum = 0;        
        delay_sum_0 = 0;
        delay_sum_1 = 0;
        delay_sum_2 = 0;
        delay_sum_3 = 0;
        delay_sum_4 = 0;
    }



}

void CNanoMdReceiver::OnNanoGfexL2Md(const NanoGfexL2MdType& msgdata){
    uint64_t local_time_ns = get_nanoseconds();

    std::string inst(msgdata.contract_name);

    uint64_t extime = time_str_to_utc_s(msgdata.gen_time);
    if (IsValidQuota(inst, extime, msgdata.match_total_qty)==false){
        return;
    }

    if (m_md_interval.count(inst)==0){
        m_md_interval.emplace(inst,md_interval_t(msgdata.last_price,msgdata.last_price,msgdata.last_price));
    }

    auto &md_interval = m_md_interval.at(inst);
    if (std::isnan(md_interval.open) && (msgdata.last_price>0)){
        md_interval.open = msgdata.last_price;
    }

    if (msgdata.last_price > md_interval.high){
        md_interval.high = msgdata.last_price;
    }

    if (msgdata.last_price < md_interval.low){
        md_interval.low = msgdata.last_price;
    }

    int64_t local_time_ns_0 = get_nanoseconds();



    static NanoGfexMd md_data;
    strncpy(md_data.inst_id, msgdata.contract_name, sizeof(msgdata.contract_name));
    md_data.md_level = 1;
    md_data.extime = extime;
    md_data.open = md_interval.open;
    md_data.high = md_interval.high;
    md_data.low = md_interval.low;
    md_data.last_price = msgdata.last_price;
    md_data.last_match_qty = msgdata.last_match_qty;
    md_data.volume = msgdata.match_total_qty;
    md_data.turn_over = msgdata.turn_over;
    md_data.open_interest = msgdata.open_interest;
    md_data.open_interest_change = msgdata.open_interest_change;
    md_data.ap[0] = msgdata.ask1_px;
    md_data.ap[1] = msgdata.ask2_px;
    md_data.ap[2] = msgdata.ask3_px;
    md_data.ap[3] = msgdata.ask4_px;
    md_data.ap[4] = msgdata.ask5_px;

    md_data.bp[0] = msgdata.bid1_px;
    md_data.bp[1] = msgdata.bid2_px;
    md_data.bp[2] = msgdata.bid3_px;
    md_data.bp[3] = msgdata.bid4_px;
    md_data.bp[4] = msgdata.bid5_px;


    md_data.av[0] = msgdata.ask1_vol;
    md_data.av[1] = msgdata.ask2_vol;
    md_data.av[2] = msgdata.ask3_vol;
    md_data.av[3] = msgdata.ask4_vol;
    md_data.av[4] = msgdata.ask5_vol;

    md_data.bv[0] = msgdata.bid1_vol;
    md_data.bv[1] = msgdata.bid2_vol;
    md_data.bv[2] = msgdata.bid3_vol;
    md_data.bv[3] = msgdata.bid4_vol;
    md_data.bv[4] = msgdata.bid5_vol;

    md_data.avg_price = FLOAT64_NAN;
    md_data.contract_no = 0;
    md_data.L1_update_flag = 0;



    md_data.buy_imply_qty[0] = msgdata.buy_imply_qty_1;
    md_data.buy_imply_qty[1] = msgdata.buy_imply_qty_2;
    md_data.buy_imply_qty[2] = msgdata.buy_imply_qty_3;
    md_data.buy_imply_qty[3] = msgdata.buy_imply_qty_4;
    md_data.buy_imply_qty[4] = msgdata.buy_imply_qty_5;

    md_data.sell_imply_qty[0] = msgdata.sell_imply_qty_1;
    md_data.sell_imply_qty[1] = msgdata.sell_imply_qty_2;
    md_data.sell_imply_qty[2] = msgdata.sell_imply_qty_3;
    md_data.sell_imply_qty[3] = msgdata.sell_imply_qty_4;
    md_data.sell_imply_qty[4] = msgdata.sell_imply_qty_5;

    md_data.local_time = local_time_ns;
    m_md_handler(&md_data);


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
    delay_sum_4 += (local_time_ns_end - local_time_ns_0);


    int cpu = sched_getcpu();
    static uint64_t  period = 10000;
    count_stats ++;
    if (count_stats%period == 0){
        delay_sum = 0;        
        delay_sum_0 = 0;
        delay_sum_1 = 0;
        delay_sum_2 = 0;
        delay_sum_3 = 0;
        delay_sum_4 = 0;
    }



}

