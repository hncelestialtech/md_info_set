/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: lisc
 * Date: 2020-03-09
 */

#ifndef FB_TRADING_PROXY_API_TYPE_H
#define FB_TRADING_PROXY_API_TYPE_H

#include <stddef.h>

namespace cffex {
namespace fb {

enum {
    FB_ROUNDTRIP_TAG_INSERT_ORDER_REQ = 1, /* 报单请求入口 */
    FB_ROUNDTRIP_TAG_INSERT_ORDER_RSP = 2, /* 报单应答入口 */
    FB_ROUNDTRIP_TAG_CANCEL_ORDER_REQ = 3, /* 撤单请求入口 */
    FB_ROUNDTRIP_TAG_CANCEL_ORDER_RSP = 4, /* 撤单应答入口 */
    FB_ROUNDTRIP_TAG_INSERT_QUOTE_REQ = 5, /* 报价请求入口 */
    FB_ROUNDTRIP_TAG_INSERT_QUOTE_RSP = 6, /* 报价应答入口 */
    FB_ROUNDTRIP_TAG_CANCEL_QUOTE_REQ = 7, /* 撤报价请求入口 */
    FB_ROUNDTRIP_TAG_CANCEL_QUOTE_RSP = 8  /* 撤报价应答入口 */
};

// direction
enum { FB_DIRECTION_BUY = '0', FB_DIRECTION_SELL = '1' };

// hedge_flag
enum {
    FB_HEDGE_FLAG_SPECULATION  = '1', /* 投机 */
    FB_HEDGE_FLAG_ARBITRAGE    = '2', /* 套利 */
    FB_HEDGE_FLAG_HEDGE        = '3', /* 套保 */
    FB_HEDGE_FLAG_MARKET_MAKER = '4'  /* 做市商 */
};
// offset_flag
enum {
    FB_OFFSET_FLAG_OPEN            = '1',
    FB_OFFSET_FLAG_CLOSE           = '2',
    FB_OFFSET_FLAG_CLOSE_TODAY     = '3',
    FB_OFFSET_FLAG_CLOSE_YESTERDAY = '4',
    FB_OFFSET_FLAG_AUTO            = '5'
};
// price_category
enum {
    FB_PRICE_CATEGORY_ANY        = '1', /**< 市价 */
    FB_PRICE_CATEGORY_LIMIT      = '2', /**< 限价 */
    FB_PRICE_CATEGORY_BEST       = '3', /**< 最优价 */
    FB_PRICE_CATEGORY_FIVE_LEVEL = '4', /**< 五档价 todo */
    FB_PRICE_CATEGORY_ARBITRAGE  = '5', /**< 套利 */
    FB_PRICE_CATEGORY_SWAP       = '6', /**< 互换 */
    FB_PRICE_CATEGORY_BOTH       = '7', /**< 报价衍生 */
    FB_PRICE_CATEGORY_OTHER      = '8', /**< 其他 */
    FB_PRICE_CATEGORY_OWN_BEST   = '9'  /**< 本方最优价 */
};
// volume_condition
enum { FB_VOLUME_ANY = '1', FB_VOLUME_COMPLETE = '2' };
// time_condition
enum { FB_TIME_CONDITION_IOC = '1', FB_TIME_CONDITION_GFD = '2', FB_TIME_CONDITION_GIS = '3' };
// order status
enum {
    FB_ORDER_STATUS_NONE        = '0', /* 未知状态 */
    FB_ORDER_STATUS_WAITING     = '1', /* 已报单，待确认 */
    FB_ORDER_STATUS_IN_BOOK     = '2', /* 交易所确认 */
    FB_ORDER_STATUS_CANCEL      = '3', /* 订单取消 */
    FB_ORDER_STATUS_ERROR       = '4', /* 订单错误 */
    FB_ORDER_STATUS_PART_TRADED = '5', /* 部分成交 */
    FB_ORDER_STATUS_ALL_TRADED  = '6', /* 全部成交 */
    FB_ORDER_STATUS_TIMEOUT     = '7'  /* 超时状态 */
};
// quote status
enum {
    FB_QUOTE_STATUS_NONE        = '0', /* 未知状态 */
    FB_QUOTE_STATUS_WAITING     = '1', /* 已报单，待确认 */
    FB_QUOTE_STATUS_IN_BOOK     = '2', /* 交易所确认 */
    FB_QUOTE_STATUS_PART_CANCEL = '3', /* 部分订单取消 */
    FB_QUOTE_STATUS_ALL_CANCEL  = '4', /* 全部订单取消 */
    FB_QUOTE_STATUS_ERROR       = '5', /* 订单错误 */
    FB_QUOTE_STATUS_PART_TRADED = '6', /* 部分成交 */
    FB_QUOTE_STATUS_ALL_TRADED  = '7', /* 全部成交 */
    FB_QUOTE_STATUS_TIMEOUT     = '8'  /* 超时状态 */
};
// status
enum { FB_INACTIVE = '0', FB_ACTIVE = '1', FB_STATUS_ALL = '2' };
// exchange_id
enum {
    FB_EXCHANGE_CFFEX   = '0', /* 中金所 */
    FB_EXCHANGE_SHFE    = '1', /* 上期所 */
    FB_EXCHANGE_DCE     = '2', /* 大商所 */
    FB_EXCHANGE_ZCE     = '3', /* 郑商所 */
    FB_EXCHANGE_SSE     = '4', /* 上交所 */
    FB_EXCHANGE_SZSE    = '5', /* 深交所 */
    FB_EXCHANGE_INE     = '6', /* 能源交易中心 */
    FB_EXCHANGE_GFEX    = '7', /* 广期所 */
    FB_EXCHANGE_UNKNOWN = 'n',
    FB_EXCHANGE_ALL     = 'z'
};
enum {
    FB_EXERCISE_STYLE_AMERICAN = '0', /* 美式 */
    FB_EXERCISE_STYLE_EUROPEAN = '1'  /* 欧式 */
};
// option_type
enum { FB_OPTION_CALL = 'c', FB_OPTION_PUT = 'p' };
// position_type
enum {
    FB_LONG_POSITION  = '0', /* 多头持仓 */
    FB_SHORT_POSITION = '1'  /* 空头持仓 */
};
// instrument_type
enum {
    FB_INSTRUMENT_FUTURE   = '0',
    FB_INSTRUMENT_OPTION   = '1',
    FB_INSTRUMENT_SECURITY = '2',
    FB_INSTRUMENT_TYPE_ALL = '9'
};
// quote_source
enum {
    FB_QUOTE_SOURCE_MANUAL_QUOTE  = '1',
    FB_QUOTE_SOURCE_FB_QUOTE      = '2',
    FB_QUOTE_SOURCE_OUTSIDE_QUOTE = '3',
    FB_QUOTE_SOURCE_HISTORY_QUOTE = '9',
    FB_QUOTE_SOURCE_ALL           = 'A'
};
// order_source
enum {
    FB_ORDER_SOURCE_MANUAL_ORDER        = '1',
    FB_ORDER_SOURCE_MANUAL_QUOTE_ORDER  = '2',
    FB_ORDER_SOURCE_FB_ORDER            = '3',
    FB_ORDER_SOURCE_FB_QUOTE_ORDER      = '4',
    FB_ORDER_SOURCE_OUTSIDE_ORDER       = '5',
    FB_ORDER_SOURCE_OUTSIDE_QUOTE_ORDER = '6',
    FB_ORDER_SOURCE_HISTORY_ORDER       = '9',
    FB_ORDER_SOURCE_ALL                 = 'A'
};

// hedge_price_type
enum { FB_BEST_PRICE = '1', FB_OPPONENT_PRICE = '2', FB_LAST_PRICE = '3', FB_LIMIT_PRICE = '4' };

// instrument_trading_status_type
enum {
    FB_UNKNOWN                 = '0', /* 未知状态 */
    FB_BEFORE_TRADING          = '1', /* 开盘前 */
    FB_NOTRADING               = '2', /* 非交易 */
    FB_CONTINOUS               = '3', /* 连续交易 */
    FB_AUCTION_ORDERING        = '4', /* 集合竞价报单 */
    FB_AUCTION_MATCH           = '5', /* 集合竞价撮合 */
    FB_CLOSED                  = '6', /* 收盘 */
    FB_SUSPENDED               = '7', /* 停牌 */
    FB_CIRCUIT_BREAKER         = '8', /* 熔断 */
    FB_VOLATILITY_DISRUPTION   = '9', /* 波动性中断 */
    FB_INQUIRY                 = 'A', /* 询价中 */
    FB_CLOSED_AUCTION_ORDERING = 'B', /* 收盘集合竞价 */
    FB_AFTER_TRADING           = 'C'  /* 盘后交易 */
};

// trade_source_type
enum {
    FB_TRADE_SOURCE_INIT                        = '1', /* 初始化 */
    FB_TRADE_SOURCE_MANUAL                      = '2', /* 手动单 */
    FB_TRADE_SOURCE_STRATEGY                    = '3', /* 策略单 */
    FB_TRADE_SOURCE_OUTSIDE                     = '4', /* 外部流水 */
    FB_TRADE_SOURCE_OUTSIDE_COMB                = '5', /* 外部流水组合 */
    FB_TRADE_SOURCE_BOOK                        = '6', /* 簿记 */
    FB_TRADE_SOURCE_CREATION_REDEMPTION         = '7', /* 簿记申赎 */
    FB_TRADE_SOURCE_OUTSIDE_CREATION_REDEMPTION = '8', /* 外部流水申赎 */
    FB_TRADE_SOURCE_SETTLEMENT                  = 'A'  /* 结算成交 */
};

// inquiry_quote_status_type
enum {
    FB_INQUIRY_QUOTE_WAITING = '0', /* 等待回应 */
    FB_INQUIRY_QUOTE_FINISH  = '1', /* 已回应 */
    FB_INQUIRY_QUOTE_TIMEOUT = '2'  /* 回应超时 */
};

// security_class_type
enum {
    FB_SECURITY_FUND    = '0', /* 基金 */
    FB_SECURITY_STOCK   = '1', /* 股票 */
    FB_SECURITY_BOND    = '2', /* 债券 */
    FB_SECURITY_UNKNOWN = 'n'  /* 未配置 */
};

// fund_class_type
enum {
    FB_FUND_ETF     = '0', /* FUND_ETF */
    FB_FUND_UNKNOWN = 'n'  /* 未配置 */
};

// comb_action_type
enum {
    FB_COMB_COMBINE = '0', /* 组合申报 */
    FB_COMB_SPLIT   = '1'  /* 组合拆分 */
};

// comb_source_type
enum {
    FB_COMB_SOURCE_INIT          = '0', /*  */
    FB_COMB_SOURCE_MANUAL_COMB   = '1', /*  */
    FB_COMB_SOURCE_STRATEGY_COMB = '2', /*  */
    FB_COMB_SOURCE_OUTSIDE_COMB  = '3', /*  */
    FB_COMB_SOURCE_ALL           = 'A'  /*  */
};

// comb_strategy_type
enum {
    FB_COMB_STRATEGY_CNSJC    = '0', /* 认购牛市价差 */
    FB_COMB_STRATEGY_CXSJC    = '1', /* 认购熊市价差 */
    FB_COMB_STRATEGY_PNSJC    = '2', /* 认沽牛市价差 */
    FB_COMB_STRATEGY_PXSJC    = '3', /* 认沽熊市价差 */
    FB_COMB_STRATEGY_KS       = '4', /* 跨式空头 */
    FB_COMB_STRATEGY_KKS      = '5', /* 宽跨式空头 */
    FB_COMB_STRATEGY_SP       = '6', /* 期货跨期 */
    FB_COMB_STRATEGY_SPC      = '7', /* 期货跨品种 */
    FB_COMB_STRATEGY_DS       = '8', /* 期货对锁 */
    FB_COMB_STRATEGY_DSO      = '9', /* 期权对锁 */
    FB_COMB_STRATEGY_STD      = 'a', /* 期权跨式 */
    FB_COMB_STRATEGY_STG      = 'b', /* 期权宽跨式 */
    FB_COMB_STRATEGY_BVS      = 'c', /* 买入垂直价差 */
    FB_COMB_STRATEGY_SVS      = 'd', /* 卖出垂直价差 */
    FB_COMB_STRATEGY_PRT_BUY  = 'e', /* 买入期权期货组合 */
    FB_COMB_STRATEGY_PRT_SELL = 'f', /* 卖出期权期货组合 */
    FB_COMB_STRATEGY_UNKNOWN  = 'n'  /* 未知 */
};

// comb_status_type
enum {
    FB_COMB_STATUS_NONE       = '0', /* 未知状态 */
    FB_COMB_STATUS_WAITING    = '1', /* 已申报，待确认 */
    FB_COMB_STATUS_IN_COMB    = '2', /* 组合成功 */
    FB_COMB_STATUS_PART_SPLIT = '3', /* 部分拆分 */
    FB_COMB_STATUS_ALL_SPLIT  = '4', /* 全部拆分 */
    FB_COMB_STATUS_ERROR      = '5', /* 组合错误 */
    FB_COMB_STATUS_TIMEOUT    = '6'  /* 超时状态 */
};

// counter_id_type
/**
 *   \ingroup TRADING_PROXY_DATATYPE
 *   \brief 柜台类型
 *	@{
 */
enum {
    MM_COUNTER_FEMAS     = '0', /*                      */
    MM_COUNTER_CTP       = '1', /*                      */
    MM_COUNTER_XONE      = '2', /*                      */
    MM_COUNTER_TAP       = '3', /*                      */
    MM_COUNTER_O32       = '4', /*                      */
    MM_COUNTER_FEMASOP   = '5', /*                      */
    MM_COUNTER_YD        = '6', /*                      */
    MM_COUNTER_APEX      = '7', /*                      */
    MM_COUNTER_YLINK     = '8', /*                      */
    MM_COUNTER_ROOTNET   = '9', /*                      */
    MM_COUNTER_QDP       = 'a', /*                      */
    MM_COUNTER_ROHON     = 'b', /*                      */
    MM_COUNTER_SHENGLI   = 'c', /*                      */
    MM_COUNTER_SIMULATOR = 'd', /*                      */
    MM_COUNTER_ATP       = 'e', /*                      */
    MM_COUNTER_MOCK      = 'f', /*                      */
    MM_COUNTER_EXSIM     = 'g', /*                      */
    MM_COUNTER_UNKNOWN   = 'u', /*                      */
    MM_COUNTER_ALL       = 'z'  /*                      */
};

}  // namespace fb
}  // namespace cffex

#endif
