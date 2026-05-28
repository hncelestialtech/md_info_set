#pragma once

#include <limits>



#include "macro.h"


#define DATA_TYPE_NANO_SHFE_MD      'a'
#define DATA_TYPE_NANO_CFFEX_MD     'b'
#define DATA_TYPE_NANO_GFEX_MD      'c'



#define DATA_TYPE_STOCK_MD_ORDER         'd'
#define DATA_TYPE_STOCK_MD_TRASACTION    'e'
#define DATA_TYPE_STOCK_MD_SNAPSHOT      'f'
#define DATA_TYPE_STOCK_MD_ATP_SNAPSHOT  'g'


#pragma pack(1)
struct NanoShfeMd{
	char		    inst_id[32];		// 合约代码
    uint32_t        inst_no;
    uint32_t        update_time;                //更新时间秒
    uint16_t        update_milli_sec;           //更新时间毫秒

    uint16_t        type;                       //合约数据类型(0:上期 256:能源)
    uint32_t        change_no;                  //合约行情编号

    double          open;
    double          high;
    double          low;
    double          last_price;  
    double          turn_over;                  //总成交额
    uint32_t        volume;
    uint32_t        open_interest;              //持仓量
    double          ap[5];
    double          bp[5];
    uint32_t        av[5];
    uint32_t        bv[5];

    uint32_t        bid_volume;                 //买报单总量（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
    int64_t         bid_amount;                 //买报单总额，放大100倍（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
    uint32_t        ask_volume;                 //卖报单总量（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
    int64_t         ask_amount;                 //卖报单总额，放大100倍（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
    uint32_t        max_inst_no;                //最大合约号(同对应主题配置最大合约号值一致)
    bool            last_tick_flag;             //last tick标识位
    uint64_t        local_time;

    std::string Title(){
        std::string title = "inst_id,inst_no,update_time,update_milli_sec,type,change_no,open,high,low,last_price,turn_over,volume,open_interest,ap1,ap2,ap3,ap4,ap5,bp1,bp2,bp3,bp4,bp5,av1,av2,av3,av4,av5,bv1,bv2,bv3,bv4,bv5,bid_volume,bid_amount,ask_volume,ask_amount,max_inst_no,last_tick_flag,local_time";
        return title;
    }

    std::string ToString(){
        std::string ret = this->inst_id;
        ret = ret + "," + std::to_string(update_time);
        ret = ret + "," + std::to_string(update_milli_sec);
        ret = ret + "," + std::to_string(type);
        ret = ret + "," + std::to_string(change_no);
        ret = ret + "," + std::to_string(open);
        ret = ret + "," + std::to_string(high);
        ret = ret + "," + std::to_string(low);
        ret = ret + "," + std::to_string(last_price);
        ret = ret + "," + std::to_string(turn_over);
        ret = ret + "," + std::to_string(volume);
        ret = ret + "," + std::to_string(open_interest);
        ret = ret + "," + std::to_string(ap[0]);
        ret = ret + "," + std::to_string(ap[1]);
        ret = ret + "," + std::to_string(ap[2]);
        ret = ret + "," + std::to_string(ap[3]);
        ret = ret + "," + std::to_string(ap[4]);
        ret = ret + "," + std::to_string(bp[0]);
        ret = ret + "," + std::to_string(bp[1]);
        ret = ret + "," + std::to_string(bp[2]);
        ret = ret + "," + std::to_string(bp[3]);
        ret = ret + "," + std::to_string(bp[4]);
        ret = ret + "," + std::to_string(av[0]);
        ret = ret + "," + std::to_string(av[1]);
        ret = ret + "," + std::to_string(av[2]);
        ret = ret + "," + std::to_string(av[3]);
        ret = ret + "," + std::to_string(av[4]);
        ret = ret + "," + std::to_string(bv[0]);
        ret = ret + "," + std::to_string(bv[1]);
        ret = ret + "," + std::to_string(bv[2]);
        ret = ret + "," + std::to_string(bv[3]);
        ret = ret + "," + std::to_string(bv[4]);
        ret = ret + "," + std::to_string(bid_volume);
        ret = ret + "," + std::to_string(bid_amount);
        ret = ret + "," + std::to_string(ask_volume);
        ret = ret + "," + std::to_string(ask_amount);
        ret = ret + "," + std::to_string(max_inst_no);
        ret = ret + "," + std::to_string(last_tick_flag);
        ret = ret + "," + std::to_string(local_time);
        return ret;
    }

    void Clear(){
        this->inst_id[0] = '\n';
        this->update_time = 0;
        this->update_milli_sec = 0;
        this->type = 0;
        this->change_no = 0;
        this->open = FLOAT64_NAN;
        this->high = FLOAT64_NAN;
        this->low = FLOAT64_NAN;
        this->last_price = FLOAT64_NAN;
        this->turn_over = FLOAT64_NAN;
        this->volume = 0;
        this->open_interest = 0;
        this->ap[0] = FLOAT64_NAN;
        this->ap[1] = FLOAT64_NAN;
        this->ap[2] = FLOAT64_NAN;
        this->ap[3] = FLOAT64_NAN;
        this->ap[4] = FLOAT64_NAN;
        this->bp[0] = FLOAT64_NAN;
        this->bp[1] = FLOAT64_NAN;
        this->bp[2] = FLOAT64_NAN;
        this->bp[3] = FLOAT64_NAN;
        this->bp[4] = FLOAT64_NAN;
        this->av[0] = 0;
        this->av[1] = 0;
        this->av[2] = 0;
        this->av[3] = 0;
        this->av[4] = 0;
        this->bv[0] = 0;
        this->bv[1] = 0;
        this->bv[2] = 0;
        this->bv[3] = 0;
        this->bv[4] = 0;
        this->bid_volume = 0;
        this->bid_amount = 0;
        this->ask_volume = 0;
        this->ask_amount = 0;
        this->max_inst_no = 0;
        this->last_tick_flag = false;
        this->local_time = 0;
    }
};




struct NanoCffexMd 
{
    char            inst_id[32];
    uint32_t        update_time;
    int32_t         update_milli_sec;
    double          open;
    double          high;
    double          low;
    double          close;
    double          last_price;
    double          turn_over;
    int32_t         volume;
    double          open_interest;
    double          ap[5];
    double          bp[5];
    int32_t         av[5];
    int32_t         bv[5];
    double          upper;
    double          lower;
    double          settlement_price;
    double          curr_delta;
    uint64_t        local_time;

    std::string Title(){
        std::string title = "inst_id,update_time,update_milli_sec,open,high,low,close,last_price,turn_over,volume,open_interest,ap1,ap2,ap3,ap4,ap5,bp1,bp2,bp3,bp4,bp5,av1,av2,av3,av4,av5,bv1,bv2,bv3,bv4,bv5,upper,lower,settlement_price,curr_delta,local_time";
        return title;
    }

    std::string ToString(){
        std::string ret = this->inst_id;
        ret = ret + "," + std::to_string(update_time);
        ret = ret + "," + std::to_string(update_milli_sec);
        ret = ret + "," + std::to_string(open);
        ret = ret + "," + std::to_string(high);
        ret = ret + "," + std::to_string(low);
        ret = ret + "," + std::to_string(close);
        ret = ret + "," + std::to_string(last_price);
        ret = ret + "," + std::to_string(turn_over);
        ret = ret + "," + std::to_string(volume);
        ret = ret + "," + std::to_string(open_interest);
        ret = ret + "," + std::to_string(ap[0]);
        ret = ret + "," + std::to_string(ap[1]);
        ret = ret + "," + std::to_string(ap[2]);
        ret = ret + "," + std::to_string(ap[3]);
        ret = ret + "," + std::to_string(ap[4]);
        ret = ret + "," + std::to_string(bp[0]);
        ret = ret + "," + std::to_string(bp[1]);
        ret = ret + "," + std::to_string(bp[2]);
        ret = ret + "," + std::to_string(bp[3]);
        ret = ret + "," + std::to_string(bp[4]);
        ret = ret + "," + std::to_string(av[0]);
        ret = ret + "," + std::to_string(av[1]);
        ret = ret + "," + std::to_string(av[2]);
        ret = ret + "," + std::to_string(av[3]);
        ret = ret + "," + std::to_string(av[4]);
        ret = ret + "," + std::to_string(bv[0]);
        ret = ret + "," + std::to_string(bv[1]);
        ret = ret + "," + std::to_string(bv[2]);
        ret = ret + "," + std::to_string(bv[3]);
        ret = ret + "," + std::to_string(bv[4]);
        ret = ret + "," + std::to_string(upper);
        ret = ret + "," + std::to_string(lower);
        ret = ret + "," + std::to_string(settlement_price);
        ret = ret + "," + std::to_string(curr_delta);
        ret = ret + "," + std::to_string(local_time);
        return ret;
    }



    void Clear(){
        this->inst_id[0] = '\n';
        this->update_time = 0;
        this->update_milli_sec = 0;
        this->open = FLOAT64_NAN;
        this->high = FLOAT64_NAN;
        this->low = FLOAT64_NAN;
        this->close = FLOAT64_NAN;
        this->last_price = FLOAT64_NAN;
        this->turn_over = FLOAT64_NAN;
        this->volume = 0;
        this->open_interest = FLOAT64_NAN;
        this->ap[0] = FLOAT64_NAN;
        this->ap[1] = FLOAT64_NAN;
        this->ap[2] = FLOAT64_NAN;
        this->ap[3] = FLOAT64_NAN;
        this->ap[4] = FLOAT64_NAN;
        this->bp[0] = FLOAT64_NAN;
        this->bp[1] = FLOAT64_NAN;
        this->bp[2] = FLOAT64_NAN;
        this->bp[3] = FLOAT64_NAN;
        this->bp[4] = FLOAT64_NAN;
        this->av[0] = 0;
        this->av[1] = 0;
        this->av[2] = 0;
        this->av[3] = 0;
        this->av[4] = 0;
        this->bv[0] = 0;
        this->bv[1] = 0;
        this->bv[2] = 0;
        this->bv[3] = 0;
        this->bv[4] = 0;
        this->upper = FLOAT64_NAN;
        this->lower = FLOAT64_NAN;
        this->upper = settlement_price;
        this->lower = curr_delta;
        this->local_time = 0;
    }
};




struct NanoGfexMd 
{

	char		    inst_id[32];		// 合约代码
    int32_t         md_level;               //L1/L2标记      1: 只有一档，即L1   5：5档  即L2
	uint64_t		extime;		            // 交易时间
    double          open;
    double          high;
    double          low;
    double          last_price;
    double          turn_over;                  //总成交额
    uint32_t        volume;                     //总成交量
    uint32_t        last_match_qty;             //最新成交量
    uint32_t        open_interest;              //持仓量
    int32_t         open_interest_change;       //持仓量变化量，同一合约号先后两个持仓量的变化
    double          ap[5];
    double          bp[5];
    uint32_t        av[5];
    uint32_t        bv[5];

    //L1 独有
    double 	        avg_price; 				// 平均价
    uint16_t	    contract_no;			// 合约编号
    int16_t	        L1_update_flag;			// 更新标志

    //L2独有
    int32_t         buy_imply_qty[5];            //申买推导量    //L2
    int32_t         sell_imply_qty[5];           //档1申卖推导量  //L2
    uint64_t        local_time;

    std::string Title(){
        std::string title = "inst_id,md_level,extime,open,high,low,last_price,turn_over,volume,last_match_qty,open_interest,open_interest_change,ap1,ap2,ap3,ap4,ap5,bp1,bp2,bp3,bp4,bp5,av1,av2,av3,av4,av5,bv1,bv2,bv3,bv4,bv5,avg_price,contract_no,L1_update_flag,buy_imply_qty1,buy_imply_qty2,buy_imply_qty3,buy_imply_qty4,buy_imply_qty5,sell_imply_qty1,sell_imply_qty2,sell_imply_qty3,sell_imply_qty4,sell_imply_qty5,local_time";
        return title;
    }


    std::string ToString(){
        std::string ret = this->inst_id;
        ret = ret + "," + std::to_string(md_level);
        ret = ret + "," + std::to_string(extime);
        ret = ret + "," + std::to_string(open);
        ret = ret + "," + std::to_string(high);
        ret = ret + "," + std::to_string(low);
        ret = ret + "," + std::to_string(last_price);
        ret = ret + "," + std::to_string(turn_over);
        ret = ret + "," + std::to_string(volume);
        ret = ret + "," + std::to_string(last_match_qty);
        ret = ret + "," + std::to_string(open_interest);
        ret = ret + "," + std::to_string(open_interest_change);
        ret = ret + "," + std::to_string(ap[0]);
        ret = ret + "," + std::to_string(ap[1]);
        ret = ret + "," + std::to_string(ap[2]);
        ret = ret + "," + std::to_string(ap[3]);
        ret = ret + "," + std::to_string(ap[4]);
        ret = ret + "," + std::to_string(bp[0]);
        ret = ret + "," + std::to_string(bp[1]);
        ret = ret + "," + std::to_string(bp[2]);
        ret = ret + "," + std::to_string(bp[3]);
        ret = ret + "," + std::to_string(bp[4]);
        ret = ret + "," + std::to_string(av[0]);
        ret = ret + "," + std::to_string(av[1]);
        ret = ret + "," + std::to_string(av[2]);
        ret = ret + "," + std::to_string(av[3]);
        ret = ret + "," + std::to_string(av[4]);
        ret = ret + "," + std::to_string(bv[0]);
        ret = ret + "," + std::to_string(bv[1]);
        ret = ret + "," + std::to_string(bv[2]);
        ret = ret + "," + std::to_string(bv[3]);
        ret = ret + "," + std::to_string(bv[4]);
        ret = ret + "," + std::to_string(avg_price);
        ret = ret + "," + std::to_string(contract_no);
        ret = ret + "," + std::to_string(L1_update_flag);
        ret = ret + "," + std::to_string(buy_imply_qty[0]);
        ret = ret + "," + std::to_string(buy_imply_qty[1]);
        ret = ret + "," + std::to_string(buy_imply_qty[2]);
        ret = ret + "," + std::to_string(buy_imply_qty[3]);
        ret = ret + "," + std::to_string(buy_imply_qty[4]);
        ret = ret + "," + std::to_string(sell_imply_qty[0]);
        ret = ret + "," + std::to_string(sell_imply_qty[1]);
        ret = ret + "," + std::to_string(sell_imply_qty[2]);
        ret = ret + "," + std::to_string(sell_imply_qty[3]);
        ret = ret + "," + std::to_string(sell_imply_qty[4]);
        ret = ret + "," + std::to_string(local_time);
        return ret;
    }


    void Clear(){
        this->inst_id[0] = '\n';
        this->md_level = 0;
        this->extime = 0;
        this->open = FLOAT64_NAN;
        this->high = FLOAT64_NAN;
        this->low = FLOAT64_NAN;
        this->last_price = FLOAT64_NAN;
        this->turn_over = FLOAT64_NAN;
        this->volume = 0;
        this->last_match_qty = 0;
        this->open_interest = 0;
        this->open_interest_change = 0;
        this->ap[0] = FLOAT64_NAN;
        this->ap[1] = FLOAT64_NAN;
        this->ap[2] = FLOAT64_NAN;
        this->ap[3] = FLOAT64_NAN;
        this->ap[4] = FLOAT64_NAN;
        this->bp[0] = FLOAT64_NAN;
        this->bp[1] = FLOAT64_NAN;
        this->bp[2] = FLOAT64_NAN;
        this->bp[3] = FLOAT64_NAN;
        this->bp[4] = FLOAT64_NAN;
        this->av[0] = 0;
        this->av[1] = 0;
        this->av[2] = 0;
        this->av[3] = 0;
        this->av[4] = 0;
        this->bv[0] = 0;
        this->bv[1] = 0;
        this->bv[2] = 0;
        this->bv[3] = 0;
        this->bv[4] = 0;
        this->avg_price = FLOAT64_NAN;
        this->contract_no = 0;
        this->L1_update_flag = 0;
        this->buy_imply_qty[0] = 0;
        this->buy_imply_qty[0] = 0;
        this->buy_imply_qty[0] = 0;
        this->buy_imply_qty[0] = 0;
        this->buy_imply_qty[0] = 0;
        this->sell_imply_qty[0] = 0;
        this->sell_imply_qty[0] = 0;
        this->sell_imply_qty[0] = 0;
        this->sell_imply_qty[0] = 0;
        this->sell_imply_qty[0] = 0;
        this->local_time = 0;
    }
};


///逐笔委托数据信息
struct HSOrder_t {
    char             exch[8];      ///交易所代码
    char             code[32];     /// 合约代码
    int32_t          trans_flag;    ///逐笔行情数据标识	///HS_TRF_Alone(逐笔独立编号)：表示逐笔成交与逐笔委托SeqNo字段独立编号	///HS_TRF_Unified(逐笔统一编号):表示逐笔成交与逐笔委托SeqNo字段统一编号
    int64_t          seq_no;    ///消息序号    ///SH:非合并逐笔 委托单独序号，在同一个ChannelNo内唯一,从1开始连续    ///SH:债券逐笔、合并后逐笔,  成交与委托、状态订单统一序号，在同一个ChannelNo内唯一，从1开始连续    ///SZ:逐笔成交与委托统一序号，在同一个ChannelNo内唯一，从1开始连续
    int32_t          channel_no;    ///频道代码
    int32_t          date;    ///委托日期
    int32_t          extime;    ///委托时间
    double           price;    ///委托价格
    int64_t          volume;    ///委托数量
    char             side;    ///买卖方向    /// SH: ('1':买单; '2':卖单); 如产品状态订单逐笔(OrdType=='S'), 该字段无值    /// SZ: ('1':买; '2':卖; 'G':借入; 'F':出借)
    char             order_type;    ///订单类别    /// SH: ('A':增加订单; 'D':删除订单；'S':产品状态订单)    /// SZ: ('1':市价; '2':限价; 'U':本方最优)
    char             tick_status;    // 产品状态订单状态(仅适用上交所产品状态订单, OrdType=='S')    /// SH: ('1':ADD产品未上市, '2':START启动, '3':OCALL开市集合竞价, '4':TRADE连续自动撮合)    ///     ('5':SUSP停牌, '6':CCALL收盘集合竞价, '7':CLOSE闭市, '8':ENDTR交易结束)
    int64_t          order_no;    /// SH: 原始订单号(仅适用上交所)
    int64_t          biz_index;    /// SH: 逐笔成交与逐笔委托统一编号(仅适用上交所, 使用合并逐笔后该字段无意义) [弃用] 
    int64_t          trade_volume;    /// SH: 已成交委托数量(仅适用上交所逐笔合并数据) 
};


struct HSTransaction_t {
    char             exch[8];      ///交易所代码
    char             code[32];     /// 合约代码
    int32_t          trans_flag;   ///逐笔行情数据标识 ///HS_TRF_Alone(逐笔独立编号)：表示逐笔成交与逐笔委托SeqNo字段独立编号	///HS_TRF_Unified(逐笔统一编号):表示逐笔成交与逐笔委托SeqNo字段统一编号
    int64_t          seq_no;       ///消息序号    ///SH:非合并逐笔 成交单独序号，在同一个ChannelNo内唯一,从1开始连续    ///SH:债券逐笔、合并逐笔  成交与委托统一序号，在同一个ChannelNo内唯一，从1开始连续    ///SZ:逐笔成交与委托统一序号，在同一个ChannelNo内唯一，从1开始连续
    int32_t          channel_no;   ///频道代码
    int32_t          date;         ///成交日期
    int32_t          extime;       ///成交时间
    double           price;        ///成交价格
    int64_t          volume;       ///成交量
    double           turn_over;    ///成交金额(仅适用上交所)
    int64_t          buy_no;       ///买方订单号
    int64_t          sell_no;      ///卖方订单号
    char             BS_flag;      /// SH: 内外盘标识('B':主动买; 'S':主动卖; 'N':未知)    /// SZ: 成交标识('4':撤; 'F':成交)
    int64_t          biz_index;    /// SH: 逐笔成交与逐笔委托统一序号(仅适用上交所) [弃用] 
};


struct HSSnapShot_t {
    char             exch[8];     ///交易所代码
    char             code[32];    ///合约代码
    double           last_price;  ///最新价
    double           pre_close;   ///昨收盘
    double	         open;        ///今开盘
    double	         high;        ///最高价
    double           low;         ///最低价
    double	         close;       ///今收盘
    double	         upper;       ///涨停价
    double	         lower;       ///跌停价
    int32_t	         date;        ///交易日期，格式为YYYYMMDD
    int32_t	         update_time; ///更新时间,格式为HHMMSSsss
    int64_t          volume;      ///量额数据    ///数量，为总成交量(单位与交易所一致)    ///除了上海指数，债券与回购单位为手外，其他类型的单位都为股    ///20220425日起，上海债券指数、上海债券快照单位为千元面额
    double           turn_over;   ///成交金额，为总成交金额(单位元，与交易所一致)
    double           average_price;    ///当日均价=(TradeBalance/TradeVolume)
    double           bidprice[10];     // 买卖盘    ///十档申买价
    double           askprice[10];     ///十档申卖价
    int64_t          bidvolume[10];    ///十档申买量
    int64_t          askvolume[10];    ///十档申卖量
    int64_t          trades_num;                // 额外数据    ///成交笔数
    char             instrument_trade_status;   ///当前交易状态说明
    int64_t          total_bidvolume;           ///委托买入总量(SH,SZ)
    int64_t          total_askvolume;           ///委托卖出总量(SH,SZ)
    double           ma_bidprice;               ///加权平均委买价格(SH,SZ)
    double           ma_askprice;               ///加权平均委卖价格(SH,SZ)
    double           ma_bond_bidprice;          ///债券加权平均委买价格(SH)
    double           ma_bond_askprice;          ///债券加权平均委卖价格(SH)
    double           yield_to_maturity;         ///债券到期收益率(SH)
    double           IOPV;                      ///基金实时参考净值(SH,SZ)
    int32_t          etf_buy_count;             ///ETF申购笔数(SH,SZ)
    int32_t          etf_sell_count;            ///ETF赎回笔数(SH,SZ)
    int64_t          etf_buy_volume;            ///ETF申购数量(SH,SZ)
    double           etf_buy_balance;           ///ETF申购金额(SH)
    int64_t          etf_sell_volume;           ///ETF赎回数量(SH,SZ)
    double           etf_sell_balance;          ///ETF赎回金额(SH)
    int64_t          total_warrant_exec_volume; ///权证执行的总数量(SH)
    double           warrant_lower_price;       ///权证跌停价格(元)(SH)
    double           warrant_upper_price;       ///权证涨停价格(元)(SH)
    int32_t          cancel_buy_num;            ///买入撤单笔数(SH)
    int32_t          cancel_sell_num;           ///卖出撤单笔数(SH)
    int64_t          cancel_buy_volume;         ///买入撤单数量(SH)
    int64_t          cancel_sell_volume;        ///卖出撤单数量(SH)
    double           cancel_buy_value;          ///买入撤单金额(SH)
    double           cancel_sell_value;         ///卖出撤单金额(SH)
    int32_t          total_buy_num;             ///买入总笔数(SH)
    int32_t          total_sell_num;            ///卖出总笔数(SH)
    int32_t          duration_after_buy;        ///买入委托成交最大等待时间(SH)
    int32_t          duration_after_sell;       ///卖出委托成交最大等待时间(SH)
    int32_t          bid_orders_num;            ///买方委托价位数(SH)
    int32_t          ask_orders_num;            ///卖方委托价位数(SH)
    double           pre_IOPV;                  ///基金T-1日净值(SZ)
    int32_t          channel_no;                ///频道代码(SZ)
    double           bond_last_auction_price;   ///匹配成交最近成交价(SZ 债券现券交易)
    int64_t          bond_auction_volume;       ///匹配成交成交量(SZ 债券现券交易)
    double           bond_auction_balance;      ///匹配成交成交金额(SZ 债券现券交易)
    int8_t           bond_last_trade_type;      ///最近价成交方式(SZ 债券现券交易)
    char             bond_trade_status[5];      ///债券交易方式对应的交易状态(SZ 债券现券交易  0匹配成交 1协商成交 2点击成交 3询价成交 4竞买成交)
    int32_t          bid_num_orders[10];        ///申买十档委托笔数
    int32_t          ask_num_orders[10];        ///申卖十档委托笔数
    char             res[16];                   ///预留
};

struct HSATPSnapShot_t {
    char             exch[8];     ///交易所代码
    char             code[32];    ///合约代码
    double	         pre_close;    ///昨收盘
    double	         close;    ///今收盘价
    int32_t	         date;    /// 交易日期，格式为YYYYMMDD
    int32_t	         update_time;    ///更新时间,格式为HHMMSSsss
    char             instrument_trade_status;    ///当前交易状态说明
    int64_t	         volume;// 量额数据    ///盘后数量，单位都为股 
    double           turn_over;    ///盘后成交金额，为总成交金额(单位元，与交易所一致)
    int64_t	         trades_num;    ///盘后成交笔数
    int64_t          total_bidvolume;    ///盘后委托买入总量(SH)
    int64_t          total_askvolume;    ///盘后委托卖出总量(SH)
    int32_t          cancel_buy_num;    ///盘后买入撤单笔数(SH)
    int32_t          cancel_sell_num;    ///盘后卖出撤单笔数(SH)
    int64_t          cancel_buy_volume;    ///盘后买入撤单数量(SH)
    int64_t          cancel_sell_volume;    ///盘后卖出撤单数量(SH)
    double	         bp1;    // 盘后买卖盘    ///一档申买价
    double	         ap1;    ///一档申卖价
    int64_t	         bv1;    ///一档申买量
    int64_t	         av1;    ///一档申卖量
    int32_t          channel_no;    ///频道代码(SZ)
    char             res[16];    ///预留
};





#pragma pack()


struct SharedMemoryHeader_t {
    size_t total_size;    //空间大小
    size_t used_size;     //实际使用的大小
    size_t item_count;    //数据记录条数
    uint32_t  shm_stat;   //标记数据是否写完。  1 表示写完 0表示非完整数据
    uint32_t  r_lock;     //暂时用不到，多路写竞态锁
    char   res[64];       //保留
};














