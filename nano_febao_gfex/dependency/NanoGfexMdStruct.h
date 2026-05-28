#pragma once

#include <cstdint>

#pragma pack(1)
/******************************************************一档行情******************************************************/
//一档行情
struct NanoGfexL1MdType
{
	uint16_t	contract_no;			// 合约编号
	char		contract_name[20];		// 合约代码
	char		send_time[13];		    // 交易时间
	double	    last_price;				// 最新成交价格
	uint32_t	last_match_qty;		    // 最新成交数量
	uint32_t	total_match_qty;	    // 总成交数量   
	double 	    turn_over;			    // 成交量金额
	uint32_t	open_interest;			// 持仓量
	int32_t		open_interest_change;	// 持仓量变化量，同一合约号先后两个持仓量的变化
	double 	    bid_price; 				// 买价
	int32_t		bid_qty;				// 买量
	double 	    ask_price; 				// 卖价
	int32_t		ask_qty;				// 卖量
    double 	    avg_price; 				// 平均价
	int16_t	    update_flag;			// 更新标志
};

/******************************************************五档行情******************************************************/
struct NanoGfexL2MdType
{
    char            contract_name[20];          //合约代码
    double          last_price;                 //最新成交价
    uint32_t        last_match_qty;             //最新成交量
    uint32_t        match_total_qty;            //总成交量
    double          turn_over;                  //总成交额
    uint32_t        open_interest;              //持仓量
    int32_t         open_interest_change;       //持仓量变化量，同一合约号先后两个持仓量的变化
    char            gen_time[16];               //交易时间
    double          bid1_px;                    //买一价
    uint32_t        bid1_vol;                   //买一量
    double          bid2_px;                    //买二价
    uint32_t        bid2_vol;                   //买二量
    double          bid3_px;                    //买三价
    uint32_t        bid3_vol;                   //买三量
    double          bid4_px;                    //买四价
    uint32_t        bid4_vol;                   //买四量
    double          bid5_px;                    //买五价
    uint32_t        bid5_vol;                   //买五量
    double          ask1_px;                    //卖一价
    uint32_t        ask1_vol;                   //卖一量        
    double          ask2_px;                    //卖二价
    uint32_t        ask2_vol;                   //卖二量
    double          ask3_px;                    //卖三价
    uint32_t        ask3_vol;                   //卖三量
    double          ask4_px;                    //卖四价
    uint32_t        ask4_vol;                   //卖四量
    double          ask5_px;                    //卖五价
    uint32_t        ask5_vol;                   //卖五量
    int32_t         buy_imply_qty_1;            //档1申买推导量
    int32_t         buy_imply_qty_2;            //档2申买推导量
    int32_t         buy_imply_qty_3;            //档3申买推导量
    int32_t         buy_imply_qty_4;            //档4申买推导量
    int32_t         buy_imply_qty_5;            //档5申买推导量
    int32_t         sell_imply_qty_1;           //档1申卖推导量
    int32_t         sell_imply_qty_2;           //档2申卖推导量
    int32_t         sell_imply_qty_3;           //档3申卖推导量
    int32_t         sell_imply_qty_4;           //档4申卖推导量
    int32_t         sell_imply_qty_5;           //档5申卖推导量
};

//合约静态信息
struct NanoGfexInstStaticInfo
{
    uint32_t    contract_no;            // 合约编号
    char        contract_name[20];      // 合约名称, 例如: si2403
    uint32_t    trade_date;             // 交易日期
    uint32_t    init_open_interst;      // 初始持仓量
    double      limit_up_px;            // 涨停板
    double      limit_down_px;          // 跌停板
    double      last_settlement_price;  // 上日结算价
    double      last_closing_price;     // 上日收盘价
};
#pragma pack()