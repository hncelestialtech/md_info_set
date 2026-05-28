
#include <algorithm>
#include <ctime>
#include <chrono>
#include <float.h>
#include <math.h>
#include <fstream>

#include "common.h"

#include "nano_shfe_recevier.h"
#include "nano_shfe_typedef.h"


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


void CNanoMdReceiver::RegistMdCallback(NanoShfeMdHandler handler){
    m_md_handler  = handler;
    return;
}



void CNanoMdReceiver::OnNanoShfeMd(const NanoShfeMdType& msgdata){
    uint64_t local_time_ns = get_nanoseconds();


    std::string inst(msgdata.inst_id);

    if (m_md_interval.count(inst)==0){
        m_md_interval.emplace(inst,md_interval_t(msgdata.last_px,msgdata.last_px,msgdata.last_px));
    }

    auto &md_interval = m_md_interval.at(inst);
    if (std::isnan(md_interval.open) && (msgdata.last_px>0)){
        md_interval.open = msgdata.last_px;
    }

    if (msgdata.last_px > md_interval.high){
        md_interval.high = msgdata.last_px;
    }

    if (msgdata.last_px < md_interval.low){
        md_interval.low = msgdata.last_px;
    }

    // int64_t local_time_ns_0 = get_nanoseconds();


	// char		    inst_id[32];		// 合约代码
    // uint32_t        inst_no;
    // uint32_t        update_time;                //更新时间秒
    // uint16_t        update_milli_sec;           //更新时间毫秒

    // uint16_t        type;                       //合约数据类型(0:上期 256:能源)
    // uint32_t        change_no;                  //合约行情编号

    // double          open;
    // double          high;
    // double          low;
    // double          last_price;  
    // double          turn_over;                  //总成交额
    // uint32_t        volume;      
    // uint32_t        open_interest;              //持仓量
    // double          ap[5];
    // double          bp[5];
    // uint32_t        av[5];
    // uint32_t        bv[5];

    // uint32_t        bid_volume;                 //买报单总量（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
    // int64_t         bid_amount;                 //买报单总额，放大100倍（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
    // uint32_t        ask_volume;                 //卖报单总量（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
    // int64_t         ask_amount;                 //卖报单总额，放大100倍（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
    // uint32_t        max_inst_no;                //最大合约号(同对应主题配置最大合约号值一致)
    // bool            last_tick_flag;             //last tick标识位



    static NanoShfeMd md_data;
    strncpy(md_data.inst_id, msgdata.inst_id, sizeof(msgdata.inst_id));
    md_data.inst_no = msgdata.inst_no;

    md_data.type = msgdata.type;
    md_data.change_no = msgdata.change_no;

    md_data.update_time = msgdata.update_time;
    md_data.update_milli_sec = msgdata.update_milli_sec;
    md_data.open = md_interval.open/100.0;
    md_data.high = md_interval.high/100.0;
    md_data.low = md_interval.low/100.0;
    md_data.last_price = msgdata.last_px/100.0;
    md_data.volume = msgdata.volume;
    md_data.turn_over = msgdata.turnover/100.0;
    md_data.open_interest = msgdata.open_interest;
    md_data.ap[0] = msgdata.ask1_px/100.0;
    md_data.ap[1] = msgdata.ask2_px/100.0;
    md_data.ap[2] = msgdata.ask3_px/100.0;
    md_data.ap[3] = msgdata.ask4_px/100.0;
    md_data.ap[4] = msgdata.ask5_px/100.0;

    md_data.bp[0] = msgdata.bid1_px/100.0;
    md_data.bp[1] = msgdata.bid2_px/100.0;
    md_data.bp[2] = msgdata.bid3_px/100.0;
    md_data.bp[3] = msgdata.bid4_px/100.0;
    md_data.bp[4] = msgdata.bid5_px/100.0;


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
 
    md_data.bid_volume = msgdata.bid_volume;
    md_data.bid_amount = msgdata.bid_amount/100.0;
    md_data.ask_volume = msgdata.ask_volume;
    md_data.ask_amount = msgdata.ask_amount/100.0;
    md_data.max_inst_no = msgdata.max_inst_no;
    md_data.last_tick_flag = msgdata.last_tick_flag;
    md_data.local_time = local_time_ns;
    m_md_handler(&md_data);

    // LOG_TRACE(std::string(md_data.inst_id));

    int64_t local_time_ns_end = get_nanoseconds();
    
    // std::clog <<"OnNanoShfeMd:"<< local_time_ns<<","<<local_time_ns_end<< std::endl;


    // static uint64_t delay_sum = 0;
    // static uint64_t delay_sum_0 = 0; //  -0
    // static uint64_t delay_sum_1 = 0; // 0-1
    // static uint64_t delay_sum_2 = 0; // 1-2
    // static uint64_t delay_sum_3 = 0; // 2-3
    // static uint64_t delay_sum_4 = 0; // 3-end

    // static uint64_t count_stats = 0;
    // delay_sum += (local_time_ns_end - local_time_ns);
    // delay_sum_0 += (local_time_ns_0 - local_time_ns);
    // delay_sum_4 += (local_time_ns_end - local_time_ns_0);


    // int cpu = sched_getcpu();
    // static uint64_t  period = 10000;
    // count_stats ++;
    // if (count_stats%period == 0){
    //     delay_sum = 0;        
    //     delay_sum_0 = 0;
    //     delay_sum_1 = 0;
    //     delay_sum_2 = 0;
    //     delay_sum_3 = 0;
    //     delay_sum_4 = 0;
    // }



}

