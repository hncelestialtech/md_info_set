#pragma once

#include <memory>

#include "data_type_def.h"

// struct NanoGfexL1L2Md{
// 	char		    contract_name[32];		// 合约代码
//     int32_t         md_level;               //L1/L2标记      1: 只有一档，即L1   5：5档  即L2
// 	uint64_t		extime;		            // 交易时间
//     double          open;
//     double          high;
//     double          low;
//     double          last_price;
//     uint32_t        last_match_qty;             //最新成交量
//     uint32_t        match_total_qty;            //总成交量
//     double          turn_over;                  //总成交额
//     uint32_t        open_interest;              //持仓量
//     int32_t         open_interest_change;       //持仓量变化量，同一合约号先后两个持仓量的变化
//     double          ap[5];
//     double          bp[5];
//     uint32_t        av[5];
//     uint32_t        bv[5];

//     //L1 独有
//     double 	        avg_price; 				// 平均价
//     uint16_t	    contract_no;			// 合约编号
//     int16_t	        L1_update_flag;			// 更新标志

//     //L2独有
//     int32_t         buy_imply_qty[5];            //申买推导量    //L2
//     int32_t         sell_imply_qty[5];           //档1申卖推导量  //L2
//     uint64_t        local_time;
// };

typedef void (*NanoGfexMdHandler)(const NanoGfexMd*);

class CNanoGfexMd{
public:

    CNanoGfexMd();
    ~CNanoGfexMd();

    //依次call以下方法
    bool Init(const char * json_config_file);
    bool RegistMdCallback(NanoGfexMdHandler handler);//注意，此方法是异步回调
    bool Run();

private:
    CNanoGfexMd(const CNanoGfexMd &) =delete;
    const CNanoGfexMd &operator=(const CNanoGfexMd &) = delete;

private:
    class CNanoGfexMdImpl;
    std::unique_ptr<CNanoGfexMdImpl> pImpl;
};
