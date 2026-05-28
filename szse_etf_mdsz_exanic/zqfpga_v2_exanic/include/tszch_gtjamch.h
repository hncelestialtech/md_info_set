/**
 * @file tszch_gtjamcb.h
 * @author zhouzhengxin (zhouzhengxin@gtjas.com)
 * @brief 深交所撮合行情统一组播行情协议
 * @version 1.0
 * @date 2021-06-09
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once

// MD_TYPE行情类型4B，上海0x00010000，深证0x00020000
#define MD_TYPE_SZ_LF 0x00020008U // 深交所撮合快照 0x00020008

// 买卖方向，采用交易所的逐笔委托的定义方式，'1'买，'2'卖，'G'借入，'F'出借
#define ORDERSIDE_DEFAULT '0'
#define ORDERSIDE_BID '1'
#define ORDERSIDE_ASK '2'
#define ORDERSIDE_BORROW 'G'
#define ORDERSIDE_LEND 'F'

#pragma pack(push, 1)
//撮合行情
typedef struct t_SZ_MCBMarketDataLF
{
    char security_id[8];        // 证券代码
    int nTime;                  //时间(HHMMSSmmmm)
    unsigned int uPreClose;     //前收盘价,扩大10000倍
    unsigned int uOpen;         //开盘价,扩大10000倍
    unsigned int uHigh;         //最高价,扩大10000倍
    unsigned int uLow;          //最低价,扩大10000倍
    unsigned int uMatch;        //最新价,扩大10000倍
    unsigned int uAskPrice[10]; //申卖价,扩大10000倍
    unsigned int uAskVol[10];   //申卖量
    unsigned int uBidPrice[10]; //申买价,扩大10000倍
    unsigned int uBidVol[10];   //申买量
    unsigned int uNumTrades;    //成交笔数
    long long iVolume;          //成交总量
    long long iTurnover;        //成交总金额,扩大10000倍
} T_SZ_MCBMarketDataLF, *PSZ_MCBMarketDataLF;
#pragma pack(pop)