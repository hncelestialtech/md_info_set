#pragma once

#include <cstdint>

#pragma pack(1)
struct NanoShfeMdType
{
    uint32_t        inst_no;                    //合约编码
    char            inst_id[16];                //合约代码
    uint32_t        update_time;                //更新时间秒
    uint16_t        update_milli_sec;           //更新时间毫秒
    uint16_t        type;                       //合约数据类型(0:上期 256:能源)
    uint32_t        change_no;                  //合约行情编号
    int32_t         last_px;                    //最新成交价，放大100倍
    uint32_t        volume;                     //最新成交量
    int64_t         turnover;                   //成交总额，放大100倍
    int64_t         open_interest;              //未平仓总量
    int32_t         bid1_px;                    //买一价，放大100倍
    uint32_t        bid1_vol;                   //买一量
    int32_t         bid2_px;                    //买二价，放大100倍
    uint32_t        bid2_vol;                   //买二量
    int32_t         bid3_px;                    //买三价，放大100倍
    uint32_t        bid3_vol;                   //买三量
    int32_t         bid4_px;                    //买四价，放大100倍
    uint32_t        bid4_vol;                   //买四量
    int32_t         bid5_px;                    //买五价，放大100倍
    uint32_t        bid5_vol;                   //买五量
    int32_t         ask1_px;                    //卖一价，放大100倍
    uint32_t        ask1_vol;                   //卖一量
    int32_t         ask2_px;                    //卖二价，放大100倍
    uint32_t        ask2_vol;                   //卖二量
    int32_t         ask3_px;                    //卖三价，放大100倍
    uint32_t        ask3_vol;                   //卖三量
    int32_t         ask4_px;                    //卖四价，放大100倍
    uint32_t        ask4_vol;                   //卖四量
    int32_t         ask5_px;                    //卖五价，放大100倍
    uint32_t        ask5_vol;                   //卖五量
    uint32_t        bid_volume;                 //买报单总量（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
    int64_t         bid_amount;                 //买报单总额，放大100倍（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
    uint32_t        ask_volume;                 //卖报单总量（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
    int64_t         ask_amount;                 //卖报单总额，放大100倍（此值只有每秒四笔行情有，如果优选后是每秒两笔此值为0）
    int32_t         high_price;                 //行情最高价,放大100倍
    int32_t         low_price;                  //行情最低价,放大100倍
    uint32_t        max_inst_no;                //最大合约号(同对应主题配置最大合约号值一致)
    bool            last_tick_flag;             //last tick标识位
    uint8_t         reverse[3];                //预留字段(暂未使用)
};
#pragma pack()