/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: zhr
 * Date: 2018-08-17
 */

#ifndef FB_ENUM_TYPE_H
#define FB_ENUM_TYPE_H
namespace cffex {
namespace fb {

/**
 *   \ingroup G_DATATYPE
 *   \brief 买卖方向
 *	@{
 */
// direction
enum {
    STRATEGY_DIRECTION_BUY  = '0', /**< 买 */
    STRATEGY_DIRECTION_SELL = '1'  /**< 卖 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 对冲标记
 *	@{
 */
// hedge_flag
enum {
    STRATEGY_HEDGE_FLAG_SPECULATION  = '1', /**< 投机 */
    STRATEGY_HEDGE_FLAG_ARBITRAGE    = '2', /**< 套利 */
    STRATEGY_HEDGE_FLAG_HEDGE        = '3', /**< 套保 */
    STRATEGY_HEDGE_FLAG_MARKET_MAKER = '4'  /**< 做市商 */
};
/**<* @}*/

/**<*
 *   \ingroup G_DATATYPE
 *   \brief 开平标记
 *	@{
 */

// offset_flag
enum {
    STRATEGY_OFFSET_FLAG_OPEN            = '1', /**< 开仓 */
    STRATEGY_OFFSET_FLAG_CLOSE           = '2', /**< 平仓 */
    STRATEGY_OFFSET_FLAG_CLOSE_TODAY     = '3', /**< 平今 */
    STRATEGY_OFFSET_FLAG_CLOSE_YESTERDAY = '4', /**< 平昨 */
    STRATEGY_OFFSET_FLAG_AUTO            = '5'  /**< 自动 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 价格类型
 *	@{
 */
// price_category
enum {
    STRATEGY_PRICE_CATEGORY_ANY        = '1', /**< 任意价 */
    STRATEGY_PRICE_CATEGORY_LIMIT      = '2', /**< 限价 */
    STRATEGY_PRICE_CATEGORY_BEST       = '3', /**< 最优价 */
    STRATEGY_PRICE_CATEGORY_FIVE_LEVEL = '4', /**< 最优五档 */
    STRATEGY_PRICE_CATEGORY_ARBITRAGE  = '5', /**< 套利 */
    STRATEGY_PRICE_CATEGORY_SWAP       = '6', /**< 互换 */
    STRATEGY_PRICE_CATEGORY_BOTH       = '7', /**< 全部 */
    STRATEGY_PRICE_CATEGORY_OTHER      = '8'  /**< 其他 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 数量条件类型
 *	@{
 */
// volume_condition
enum {
    STRATEGY_VOLUME_ANY      = '1', /**< 任意数量 */
    STRATEGY_VOLUME_COMPLETE = '2'  /**< 全部 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 时间条件类型
 *	@{
 */
// time_condition
enum {
    STRATEGY_TIME_CONDITION_IOC = '1', /**< 立即完成否则撤销 */
    STRATEGY_TIME_CONDITION_GFD = '2', /**< 当日有效 */
    STRATEGY_TIME_CONDITION_GIS = '3'  /**< 小节内有效 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 策略报单状态
 *	@{
 */
// order status
enum {
    STRATEGY_ORDER_STATUS_NONE        = '0', /**< 未知状态 */
    STRATEGY_ORDER_STATUS_WAITING     = '1', /**< 已报单，待确认 */
    STRATEGY_ORDER_STATUS_IN_BOOK     = '2', /**< 交易所确认 */
    STRATEGY_ORDER_STATUS_CANCEL      = '3', /**< 订单取消 */
    STRATEGY_ORDER_STATUS_ERROR       = '4', /**< 订单错误 */
    STRATEGY_ORDER_STATUS_PART_TRADED = '5', /**< 部分成交 */
    STRATEGY_ORDER_STATUS_ALL_TRADED  = '6', /**< 全部成交 */
    STRATEGY_ORDER_STATUS_TIMEOUT     = '7'  /**< 超时状态 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 策略报价状态
 *	@{
 */
// quote status
enum {
    STRATEGY_QUOTE_STATUS_NONE        = '0', /**< 未知状态 */
    STRATEGY_QUOTE_STATUS_WAITING     = '1', /**< 已报单，待确认 */
    STRATEGY_QUOTE_STATUS_IN_BOOK     = '2', /**< 交易所确认 */
    STRATEGY_QUOTE_STATUS_PART_CANCEL = '3', /**< 部分订单取消 */
    STRATEGY_QUOTE_STATUS_ALL_CANCEL  = '4', /**< 全部订单取消 */
    STRATEGY_QUOTE_STATUS_ERROR       = '5', /**< 订单错误 */
    STRATEGY_QUOTE_STATUS_PART_TRADED = '6', /**< 部分成交 */
    STRATEGY_QUOTE_STATUS_ALL_TRADED  = '7', /**< 全部成交 */
    STRATEGY_QUOTE_STATUS_TIMEOUT     = '8'  /**< 超时状态 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 策略状态
 *	@{
 */
// status
enum { STRATEGY_INACTIVE = '0', STRATEGY_ACTIVE = '1', STRATEGY_STATUS_ALL = '2' };
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 交易所编号
 *	@{
 */
// exchange_id
enum {
    STRATEGY_EXCHANGE_CFFEX   = '0', /**< 中金所 */
    STRATEGY_EXCHANGE_SHFE    = '1', /**< 上期所 */
    STRATEGY_EXCHANGE_DCE     = '2', /**< 大商所 */
    STRATEGY_EXCHANGE_ZCE     = '3', /**< 郑商所 */
    STRATEGY_EXCHANGE_SSE     = '4', /**< 上交所 */
    STRATEGY_EXCHANGE_SZSE    = '5', /**< 深交所 */
    STRATEGY_EXCHANGE_INE     = '6', /**< 能源交易中心 */
    STRATEGY_EXCHANGE_GFEX    = '7', /**< 广期所 */
    STRATEGY_EXCHANGE_BSE     = '8', /**< 北交所 */
    STRATEGY_EXCHANGE_UNKNOWN = 'n',
    STRATEGY_EXCHANGE_ALL     = 'z'
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 策略期权分类
 *	@{
 */
// option_type
enum {
    STRATEGY_OPTION_EUROPEAN_CALL = 'c', /**< 欧式看涨期权 */
    STRATEGY_OPTION_EUROPEAN_PUT  = 'p', /**< 欧式看跌期权 */
    STRATEGY_OPTION_AMERICAN_CALL = 'C', /**< 美式看涨期权 */
    STRATEGY_OPTION_AMERICAN_PUT  = 'P'  /**< 美式看跌期权 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 合约分类
 *	@{
 */
// instrument_type
enum {
    STRATEGY_INSTRUMENT_FUTURE   = '0', /**< 期货合约 */
    STRATEGY_INSTRUMENT_OPTION   = '1', /**< 期权合约 */
    STRATEGY_INSTRUMENT_STOCK    = '2', /**< 股票合约 */
    STRATEGY_ARBITRAGE           = '3', /**< 套利合约 */
    STRATEGY_INSTRUMENT_TYPE_ALL = '9'  /**< 全部 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 报价来源
 *	@{
 */
// quote_source
enum {
    STRATEGY_QUOTE_SOURCE_MANUAL_QUOTE   = '1', /**< 手动报价 */
    STRATEGY_QUOTE_SOURCE_STRATEGY_QUOTE = '2', /**< 策略报价 */
    STRATEGY_QUOTE_SOURCE_OUTSIDE_QUOTE  = '3', /**< 外部流水 */
    STRATEGY_QUOTE_SOURCE_ALL            = 'A'  /**< 全部 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 报单来源
 *	@{
 */
// order_source
enum {
    STRATEGY_ORDER_SOURCE_MANUAL_ORDER         = '1', /**< 手动报单 */
    STRATEGY_ORDER_SOURCE_MANUAL_QUOTE_ORDER   = '2', /**< 手动报价 */
    STRATEGY_ORDER_SOURCE_STRATEGY_ORDER       = '3', /**< 策略报单 */
    STRATEGY_ORDER_SOURCE_STRATEGY_QUOTE_ORDER = '4', /**< 策略报价 */
    STRATEGY_ORDER_SOURCE_OUTSIDE_ORDER        = '5', /**< 外部报单 */
    STRATEGY_ORDER_SOURCE_OUTSIDE_QUOTE_ORDER  = '6', /**< 外部报价 */
    STRATEGY_ORDER_SOURCE_ALL                  = 'A'  /**< 全部 */
};
/** @}*/

// hedge_price_type
/**
 *   \ingroup G_DATATYPE
 *   \brief 价格类型
 *	@{
 */
// hedge_price_type
enum {
    STRATEGY_BEST_PRICE     = '1', /**< 最优价 */
    STRATEGY_OPPONENT_PRICE = '2', /**< 对手价 */
    STRATEGY_LAST_PRICE     = '3', /**< 最新价 */
    STRATEGY_LIMIT_PRICE    = '4'  /**< 限价 */
};
/** @}*/

// instrument_trading_status_type
/**
 *   \ingroup G_DATATYPE
 *   \brief 交易状态
 *	@{
 */
// instrument_trading_status_type
enum {
    STRATEGY_UNKNOWN                 = '0', /**< 未知状态 */
    STRATEGY_BEFORE_TRADING          = '1', /**< 开盘前 */
    STRATEGY_NOTRADING               = '2', /**< 非交易 */
    STRATEGY_CONTINOUS               = '3', /**< 连续交易 */
    STRATEGY_AUCTION_ORDERING        = '4', /**< 集合竞价报单 */
    STRATEGY_AUCTION_MATCH           = '5', /**< 集合竞价撮合 */
    STRATEGY_CLOSED                  = '6', /**< 收盘 */
    STRATEGY_SUSPENDED               = '7', /**< 停牌 */
    STRATEGY_CIRCUIT_BREAKER         = '8', /**< 熔断 */
    STRATEGY_VOLATILITY_DISRUPTION   = '9', /**< 波动性中断 */
    STRATEGY_INQUIRY                 = 'A', /**< 询价中 */
    STRATEGY_CLOSED_AUCTION_ORDERING = 'B', /**< 收盘集合竞价 */
    STRATEGY_AFTER_TRADING           = 'C'  /**< 盘后交易 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 成交来源
 *	@{
 */
enum {
    STRATEGY_TRADE_SOURCE_INIT                        = '1', /**< 初始化 */
    STRATEGY_TRADE_SOURCE_MANUAL                      = '2', /**< 手动单 */
    STRATEGY_TRADE_SOURCE_STRATEGY                    = '3', /**< 策略单 */
    STRATEGY_TRADE_SOURCE_OUTSIDE                     = '4', /**< 外部流水 */
    STRATEGY_TRADE_SOURCE_OUTSIDE_COMB                = '5', /**< 外部流水组合 */
    STRATEGY_TRADE_SOURCE_BOOK                        = '6', /**< 簿记 */
    STRATEGY_TRADE_SOURCE_CREATION_REDEMPTION         = '7', /**< 簿记申赎 */
    STRATEGY_TRADE_SOURCE_OUTSIDE_CREATION_REDEMPTION = '8', /**< 外部流水申赎 */
    STRATEGY_TRADE_SOURCE_SETTLEMENT                  = 'A'  /**< 结算成交 */
};
/** @}*/

// inquiry_quote_status_type
/**
 *   \ingroup G_DATATYPE
 *   \brief 报价查询状态
 *	@{
 */
// inquiry_quote_status_type
enum {
    STRATEGY_INQUIRY_QUOTE_WAITING = '0', /**< 等待回应 */
    STRATEGY_INQUIRY_QUOTE_FINISH  = '1', /**< 已回应 */
    STRATEGY_INQUIRY_QUOTE_TIMEOUT = '2'  /**< 回应超时 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief 策略实例状态
 *	@{
 */
enum {
    STRATEGY_INSTANCE_INIT_STAT    = '1', /**< 初始状态 */
    STRATEGY_INSTANCE_RUNNING_STAT = '2', /**< 运行中 */
    STRATEGY_INSTANCE_PAUSE_STAT   = '3', /**< 已暂停 */
    STRATEGY_INSTANCE_DELETE_STAT  = '4', /**< 已删除 */
    STRATEGY_INSTANCE_TIMEOUT_STAT = '5'  /**< 已失效 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief
 *	@{
 */
enum {
    STRATEGY_FIT_CONTRACT_TYPE_ALL     = '0', /**< 全部合约 */
    STRATEGY_FIT_CONTRACT_TYPE_CALL    = '1', /**< 看涨合约 */
    STRATEGY_FIT_CONTRACT_TYPE_PUT     = '2', /**< 看跌合约 */
    STRATEGY_FIT_CONTRACT_TYPE_VIRTUAL = '3'  /**< 虚值合约 */
};
/** @}*/

/**
 *   \ingroup G_DATATYPE
 *   \brief
 *	@{
 */
enum {
    STRATEGY_TERMINAL = '0', /**< 客户端 */
    STRATEGY_STRATEGY = '1'  /**< 策略 */
};
/** @}*/

// comb_action
/**
 *   \ingroup G_DATATYPE
 *   \brief 组合保证金操作类型
 *	@{
 */
// comb_action
enum {
    STRATEGY_COMB_COMBINE = '0', /**< 组合申请 */
    STRATEGY_COMB_SPLIT   = '1'  /**< 组合拆分 */
};

// comb_direction
enum {
    STRATEGY_COMB_BUY  = '0', /**<  */
    STRATEGY_COMB_SELL = '1'  /**<  */
};
/** @}*/

// comb_strategy
/**
 *   \ingroup G_DATATYPE
 *   \brief 组合保证金组合策略
 *	@{
 */

enum {
    STRATEGY_COMB_STRATEGY_CNSJC   = '0', /**< 认购牛市价差 */
    STRATEGY_COMB_STRATEGY_CXSJC   = '1', /**< 认购熊市价差 */
    STRATEGY_COMB_STRATEGY_PNSJC   = '2', /**< 认沽牛市价差 */
    STRATEGY_COMB_STRATEGY_PXSJC   = '3', /**< 认沽熊市价差 */
    STRATEGY_COMB_STRATEGY_KS      = '4', /**< 跨式空头 */
    STRATEGY_COMB_STRATEGY_KKS     = '5', /**< 宽跨式空头 */
    STRATEGY_COMB_STRATEGY_SP      = '6', /**< 跨期 */
    STRATEGY_COMB_STRATEGY_SPC     = '7', /**< 跨品种 */
    STRATEGY_COMB_STRATEGY_DS      = '8', /**< 期货对锁 */
    STRATEGY_COMB_STRATEGY_DSO     = '9', /**< 期权对锁 */
    STRATEGY_COMB_STRATEGY_STD     = 'a', /**< 跨式 */
    STRATEGY_COMB_STRATEGY_STG     = 'b', /**< 宽跨式 */
    STRATEGY_COMB_STRATEGY_BUL     = 'c', /**< 看涨垂直价差 */
    STRATEGY_COMB_STRATEGY_BER     = 'd', /**< 看跌垂直价差 */
    STRATEGY_COMB_STRATEGY_PRT     = 'e', /**< 期权期货组合 */
    STRATEGY_COMB_STRATEGY_BLT     = 'f', /**< 看涨水平价差 */
    STRATEGY_COMB_STRATEGY_BRT     = 'g', /**< 看跌水平价差 */
    STRATEGY_COMB_STRATEGY_UNKNOWN = 'n'  /**< 未知 */
};
/** @}*/

// comb_source
/**
 *   \ingroup G_DATATYPE
 *   \brief 组合保证金申报来源
 *	@{
 */

enum {
    STRATEGY_COMB_SOURCE_INIT          = '0', /**< 上场 */
    STRATEGY_COMB_SOURCE_MANUAL_COMB   = '1', /**< 手动 */
    STRATEGY_COMB_SOURCE_STRATEGY_COMB = '2', /**< 策略 */
    STRATEGY_COMB_SOURCE_OUTSIDE_COMB  = '3', /**< 外部 */
    STRATEGY_COMB_SOURCE_ALL           = 'A'  /**< 全部 */
};
/** @}*/

// comb_status
/**
 *   \ingroup G_DATATYPE
 *   \brief 组合保证金申报状态
 *	@{
 */

enum {
    STRATEGY_COMB_STATUS_NONE       = '0', /**< 未知状态 */
    STRATEGY_COMB_STATUS_WAITING    = '1', /**< 已申报，待确认 */
    STRATEGY_COMB_STATUS_IN_COMB    = '2', /**< 组合成功 */
    STRATEGY_COMB_STATUS_PART_SPLIT = '3', /**< 部分拆分 */
    STRATEGY_COMB_STATUS_ALL_SPLIT  = '4', /**< 全部拆分 */
    STRATEGY_COMB_STATUS_ERROR      = '5', /**< 组合错误 */
    STRATEGY_COMB_STATUS_TIMEOUT    = '6'  /**< 超时状态 */
};

// trigger_type
/**
 *   \ingroup G_DATATYPE
 *   \brief 触发类型
 *	@{
 */

enum {
    STRATEGY_NO_TIMER_TRIGGER    = '0', /**< 非定时器触发 */
    STRATEGY_TIMER_TRIGGER       = '1', /**< 定时器触发 */
};

}  // namespace fb
}  // namespace cffex

#endif
