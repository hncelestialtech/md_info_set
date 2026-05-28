/**
 * @file acc_md.h
 * @author zoujingwei (zoujingwei@gtjas.com)
 * @brief 硬件加速行情定义，支持深圳新债券系统
 * @version 2.2
 * @date 2022-04-14
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once

// MD_TYPE行情类型4B，上海0x00010000，深证0x00020000
#define MD_TYPE_DEFAULT 0U                      // 默认类型
#define MD_TYPE_SH_OFFSET 0x00010000U           // 上交所的行情类型偏移
#define MD_TYPE_SZ_OFFSET 0x00020000U           // 深交所的行情类型偏移
#define MD_TYPE_SZ_300111 0x00020001U           // 深交所L1快照 0x00020001
#define MD_TYPE_SZ_300111_L2 0x00020002U        // 深交所L2快照 0x00020002
#define MD_TYPE_SZ_ORDER_QUE 0x00020003U        // 深交所L2委托队列 0x00020003
#define MD_TYPE_SZ_309011 0x00020004U           // 深交所L2指数行情 0x00020004
#define MD_TYPE_SZ_300192 0x00020005U           // 深交所L2逐笔委托 0x00020005
#define MD_TYPE_SZ_300191 0x00020006U           // 深交所L2逐笔成交 0x00020006
#define MD_TYPE_SZ_300111_L1_OPTION 0x00020007U // 深交所L1期权快照 0x00020007


// 买卖方向，采用交易所的逐笔委托的定义方式，'1'买，'2'卖，'G'借入，'F'出借
#define ORDERSIDE_DEFAULT '0'
#define ORDERSIDE_BID '1'
#define ORDERSIDE_ASK '2'
#define ORDERSIDE_BORROW 'G'
#define ORDERSIDE_LEND 'F'

// 行情应用层定义
typedef struct
{
    unsigned int type;        // MD_TYPE
    unsigned int len;         // length of data
    char data[];              // data
} AccMD;

typedef struct
{
    unsigned int type; // MD_TYPE
    unsigned int len;  // length of data
    char data[1024];    // data
} StaticAccMD;

#pragma pack(push, 1)
// 债券现券细分阶段码
typedef struct {
    char phase_code[8]; // 交易方式所处的交易阶段代码
    unsigned char trading_type;      // 交易方式，无符数字	1=匹配成交，2=协商成交
                            // 3=点击成交 4=询价成交 5=竞买成交
} BondSubTradingPhaseCode;

// 深交所L2行情快照，MD_TYPE_SZ_300111_L2
typedef struct
{
    char security_id[8]; // 证券代码

    unsigned long long md_time;   // 交易所时间年月日时分秒毫秒
    unsigned long long pre_close; // 前收盘价，放大10000

    unsigned long long open; // 开盘价，放大1000000
    unsigned long long high; // 最高价，放大1000000

    unsigned long long low;   // 最低价，放大1000000
    unsigned long long match; // 最新价，放大1000000

    unsigned long long ask_price[10]; // 申卖价，放大1000000
    unsigned long long ask_vol[10];   // 申卖量，放大100
    unsigned long long bid_price[10]; // 申买价，放大1000000
    unsigned long long bid_vol[10];   // 申买量，放大100

    unsigned long long total_value;  // 成交总金额，放大10000
    int num_trades;                  // 成交笔数
    unsigned long long total_volume; // 成交总量，放大100
    unsigned long long upper_limit;  // 涨停价，放大1000000
    unsigned long long lower_limit;  // 跌停价，放大1000000
    char phase_code[8];              // 阶段码
    char mdstream_id[3]; // 行情类别

    unsigned long long weighted_avg_bid_price; // 加权平均委买价格，放大1000000
    unsigned long long total_bid_vol; // 委托买入总量，放大100    
    unsigned long long weighted_avg_ask_price; // 加权平均委卖价格，放大1000000
    unsigned long long total_ask_vol; // 委托卖出总量，放大100

    // 债券新增字段
    unsigned long long auction_match;   // 匹配成交最近价
    unsigned long long auction_volume_trade;    // 匹配成交成交量，放大100
    unsigned long long auction_value_trade;     // 匹配成交成交金额，放大10000
    int num_sub_trading_phasecodes;     // 细分交易阶段个数
    BondSubTradingPhaseCode sub_pcs[5]; // 债券细分阶段码，目前是不超过5个
    unsigned long long pre_iopv;        //基金T-1日净值，放大1000000    //20260116升级增加
    unsigned long long iopv;            //基金实时参考净值（包括ETF的IOPV），放大1000000   //20260116升级增加
} MDSnapShotL2;

// 深交所L2行情委托队列，变长，num_items是items的揭示个数，MD_TYPE_SZ_ORDER_QUE
typedef struct
{
    char security_id[8]; // 证券代码

    unsigned long long md_time;
    int side;                 // 买卖方向 '1':Bid, '2':Ask, '0'有问题
    unsigned long long price; // 一档委托价格，放大1000000
    int num_orders;           // 订单数量

    int num_items;                // 明细个数
    unsigned long long items[50]; // 深交所只揭示50笔，放大100
} OrderQueue;

// 深圳指数行情，MD_TYPE_SZ_309011
typedef struct
{
    char security_id[8];                // 指数代码
    unsigned long long md_time;         // 交易所时间年月日时分秒毫秒
    unsigned long long open_index;      // 今开盘指数，放大1000000
    unsigned long long high_index;      // 最高指数，放大1000000
    unsigned long long low_index;       // 最低指数，放大1000000
    unsigned long long last_index;      // 最新指数，放大1000000
    unsigned long long pre_close_index; // 昨收指数，放大1000000
    unsigned long long total_volume;    // 参与计算相应指数的交易数量
    unsigned long long total_value;     // 参与计算相应指数的成交金额，放大10000倍
} StockIndex;

// 深圳逐笔委托，MD_TYPE_SZ_300192
typedef struct
{
    char security_id[8];        // 证券代码
    unsigned long long seq_num; // 消息记录号
    unsigned long long price;   // 委托价格，放大10000
    unsigned long long qty;     // 委托数量，放大100
    unsigned long long md_time; // 交易所时间年月日时分秒毫秒
    char side;                  // 委托方向，'1'买，'2'卖，'G'借入，'F'出借
    char order_type;            // 委托类型，'1'市价，'2'限价，'U'本方最优
    char mdstream_id[3];        // 行情类别
    unsigned short channel_no;  // 通道号
} StepOrder;

// 深圳逐笔成交，MD_TYPE_SZ_300191
typedef struct
{
    char security_id[8];            // 证券代码
    unsigned long long seq_num;     // 消息记录号
    unsigned long long bid_seq_num; // 买方委托索引，从1开始计数，0表示无对应委托
    unsigned long long
        offer_seq_num;          // 卖方委托索引，从1开始计数，0表示无对应委托
    unsigned long long price;   // 成交价格，放大10000
    unsigned long long qty;     // 成交数量，放大100
    unsigned long long md_time; // 交易所时间年月日时分秒毫秒
    char exec_type;             // 成交类型，'4'撤销，'F'成交
    char mdstream_id[3];        // 行情类别
    unsigned short channel_no;  // 通道号
} StepTrade;

// 深圳期权L1快照行情，MD_TYPE_SZ_300111_L1_OPTION
typedef struct
{
    char security_id[8];        // 证券代码
    unsigned long long md_time; // 交易所时间年月日时分秒毫秒

    unsigned long long pre_close;    // 前收盘价，放大10000
    unsigned long long open;         // 开盘价，放大1000000
    unsigned long long high;         // 最高价，放大1000000
    unsigned long long low;          // 最低价，放大1000000
    unsigned long long match;        // 最新价，放大1000000
    unsigned long long ask_price[5]; // 申卖价，放大1000000
    unsigned long long ask_vol[5];   // 申卖量，放大100
    unsigned long long bid_price[5]; // 申买价，放大1000000
    unsigned long long bid_vol[5];   // 申买量，放大100

    unsigned long long total_value;     // 成交总金额，放大10000
    unsigned long long total_volume;    // 成交总量，放大100
    unsigned long long upper_limit;     // 涨停价，放大1000000
    unsigned long long lower_limit;     // 跌停价，放大1000000
    unsigned long long position_volume; // 持仓数量，放大100

    int num_trades;      // 成交笔数
    char phase_code[8];  // 阶段码
    char mdstream_id[3]; // 行情类别
} OptionSnapShotL1;

#pragma pack(pop)