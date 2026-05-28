#pragma once

#include <memory>

#include "data_type_def.h"

// #pragma pack(1)
// struct NanoShfeMd{
// 	char		    inst_id[32];		// 合约代码
//     uint32_t        inst_no;
//     uint32_t        update_time;                //更新时间秒
//     uint16_t        update_milli_sec;           //更新时间毫秒

//     uint16_t        type;                       //合约数据类型(0:上期 256:能源)
//     uint32_t        change_no;                  //合约行情编号

//     double          open;
//     double          high;
//     double          low;
//     double          last_price;  
//     double          turn_over;                  //总成交额
//     uint32_t        volume;      
//     uint32_t        open_interest;              //持仓量
//     double          ap[5];
//     double          bp[5];
//     uint32_t        av[5];
//     uint32_t        bv[5];

//     uint32_t        bid_volume;                 //买报单总量（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
//     int64_t         bid_amount;                 //买报单总额，放大100倍（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
//     uint32_t        ask_volume;                 //卖报单总量（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
//     int64_t         ask_amount;                 //卖报单总额，放大100倍（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
//     uint32_t        max_inst_no;                //最大合约号(同对应主题配置最大合约号值一致)
//     bool            last_tick_flag;             //last tick标识位
//     uint64_t        local_time;
//     std::string ToString();
//     void Clear();

// };
// #pragma pack()


typedef void (*NanoShfeMdHandler)(const NanoShfeMd*);

class CNanoShfeMd{
public:

    CNanoShfeMd();
    ~CNanoShfeMd();

    //依次call以下方法
    bool Init(const char * json_config_file);
    bool RegistMdCallback(NanoShfeMdHandler handler);//注意，此方法是异步回调
    bool Run();

private:
    CNanoShfeMd(const CNanoShfeMd &) =delete;
    const CNanoShfeMd &operator=(const CNanoShfeMd &) = delete;

private:
    class CNanoShfeMdImpl;
    std::unique_ptr<CNanoShfeMdImpl> pImpl;
};
