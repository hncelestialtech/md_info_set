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

#ifndef FB_TRADING_PROXY_API_ENTITY_H
#define FB_TRADING_PROXY_API_ENTITY_H

#include <stddef.h>

#include <string>

namespace cffex {
namespace fb {

// todo: remove useless interface

struct exchange_entity {
    static exchange_entity *create_entity();

    virtual ~exchange_entity() {}

    /**
     * @brief 重新设置结构体
     */
    virtual void reset_entity() = 0;

    /**
     * @brief 设置交易所编号
     */
    virtual void set_exchange_id(int8_t v) = 0;

    /**
     * @brief 设置交易所名称
     */
    virtual void set_exchange_name(const char *v) = 0;

    /**
     * @brief 设置交易日
     */
    virtual void set_trading_day(const char *v) = 0;

    /**
     * @brief 设置交易时间（秒）
     */
    virtual void set_trading_sec(int32_t v) = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成string形式
     */
    virtual std::string to_string() const = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成char*形式
     */
    virtual const char *c_str() const = 0;
};

struct rsp_order_entity {
    static rsp_order_entity *create_entity();

    virtual ~rsp_order_entity() {}

    /**
     * @brief 重新设置结构体
     */
    virtual void reset_entity() = 0;

    /**
     * @brief 设置交易所编号
     */
    virtual void set_exchange_id(int8_t v) = 0;

    /**
     * @brief 设置报单号
     */
    virtual void set_order_id(int64_t v) = 0;

    /**
     * @brief 设置柜台报单编号
     */
    virtual void set_order_sys_id(const char *v) = 0;

    /**
     * @brief 设置合约号
     */
    virtual void set_instrument_id(const char *v) = 0;

    /**
     * @brief 设置买卖方向
     */
    virtual void set_direction(int8_t v) = 0;

    /**
     * @brief 设置报单状态
     */
    virtual void set_order_status(int8_t v) = 0;

    /**
     * @brief 设置错误码
     */
    virtual void set_error_id(int32_t v) = 0;

    /**
     * @brief 设置交易账户
     */
    virtual void set_trading_account_id(int16_t v) = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成string形式
     */
    virtual std::string to_string() const = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成char*形式
     */
    virtual const char *c_str() const = 0;
};

struct rtn_order_entity {
    static rtn_order_entity *create_entity();

    virtual ~rtn_order_entity() {}

    /**
     * @brief 重新设置结构体
     */
    virtual void reset_entity() = 0;

    /**
     * @brief 设置交易所
     */
    virtual void set_exchange_id(int8_t v) = 0;

    /**
     * @brief 设置订单号
     */
    virtual void set_order_id(int64_t v) = 0;

    /**
     * @brief 设置柜台报单编号
     */
    virtual void set_order_sys_id(const char *v) = 0;

    /**
     * @brief 设置合约号
     */
    virtual void set_instrument_id(const char *v) = 0;

    /**
     * @brief 设置方向
     */
    virtual void set_direction(int8_t v) = 0;

    /**
     * @brief 设置开平标志
     */
    virtual void set_offset_flag(int8_t v) = 0;

    /**
     * @brief 设置对冲标志
     */
    virtual void set_hedge_flag(int8_t v) = 0;

    /**
     * @brief 设置报单价格
     */
    virtual void set_price(double v) = 0;

    /**
     * @brief 设置报单数量
     */
    virtual void set_volume(int32_t v) = 0;

    /**
     * @brief 设置报单价格条件类型
     */
    virtual void set_price_category(int8_t v) = 0;

    /**
     * @brief 设置报单有效期类型
     */
    virtual void set_time_condition(int8_t v) = 0;

    /**
     * @brief 设置成交量类型
     */
    virtual void set_volume_condition(int8_t v) = 0;

    /**
     * @brief 设置成交数量
     */
    virtual void set_traded_volume(int32_t v) = 0;

    /**
     * @brief 设置撤销数量
     */
    virtual void set_cancel_volume(int32_t v) = 0;

    /**
     * @brief 设置报单来源
     */
    virtual void set_order_source(int8_t v) = 0;

    /**
     * @brief 设置报单状态
     */
    virtual void set_order_status(int8_t v) = 0;

    /**
     * @brief 设置报单插入时间
     */
    virtual void set_insert_time(int32_t v) = 0;

    /**
     * @brief 设置更新时间
     */
    virtual void set_update_time(int32_t v) = 0;

    /**
     * @brief 设置组合号
     */
    virtual void set_portfolio_id(int32_t v) = 0;

    /**
     * @brief 设置交易账户
     */
    virtual void set_trading_account_id(int16_t v) = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成string形式
     */
    virtual std::string to_string() const = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成char*形式
     */
    virtual const char *c_str() const = 0;
};

struct rsp_quote_entity {
    static rsp_quote_entity *create_entity();

    virtual ~rsp_quote_entity() {}

    /**
     * @brief 重新设置结构体
     */
    virtual void reset_entity() = 0;

    /**
     * @brief 设置交易所
     */
    virtual void set_exchange_id(int8_t v) = 0;

    /**
     * @brief 设置报价单号
     */
    virtual void set_quote_id(int64_t v) = 0;

    /**
     * @brief 设置柜台报价单号
     */
    virtual void set_quote_sys_id(const char *v) = 0;

    /**
     * @brief 设置合约号
     */
    virtual void set_instrument_id(const char *v) = 0;

    /**
     * @brief 设置报价状态
     */
    virtual void set_quote_status(int8_t v) = 0;

    /**
     * @brief 设置错误码
     */
    virtual void set_error_id(int32_t v) = 0;

    /**
     * @brief 设置交易账户
     */
    virtual void set_trading_account_id(int16_t v) = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成string形式
     */
    virtual std::string to_string() const = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成char*形式
     */
    virtual const char *c_str() const = 0;
};

struct rtn_quote_entity {
    static rtn_quote_entity *create_entity();

    virtual ~rtn_quote_entity() {}

    /**
     * @brief 重新设置结构体
     */
    virtual void reset_entity() = 0;

    /**
     * @brief 设置交易所编号
     */
    virtual void set_exchange_id(int8_t v) = 0;

    /**
     * @brief 设置报价单号
     */
    virtual void set_quote_id(int64_t v) = 0;

    /**
     * @brief 设置询价单号
     */
    virtual void set_inquiry_id(int64_t v) = 0;

    /**
     * @brief 设置柜台报价单号
     */
    virtual void set_quote_sys_id(const char *v) = 0;

    /**
     * @brief 设置合约号
     */
    virtual void set_instrument_id(const char *v) = 0;

    /**
     * @brief 设置买单边开平标志
     */
    virtual void set_bid_offset_flag(int8_t v) = 0;

    /**
     * @brief 设置卖单边开平标志
     */
    virtual void set_ask_offset_flag(int8_t v) = 0;

    /**
     * @brief 设置买单边对冲标志
     */
    virtual void set_bid_hedge_flag(int8_t v) = 0;

    /**
     * @brief 设置卖单边对冲标志
     */
    virtual void set_ask_hedge_flag(int8_t v) = 0;

    /**
     * @brief 设置买单边价格
     */
    virtual void set_bid_price(double v) = 0;

    /**
     * @brief 设置卖单边价格
     */
    virtual void set_ask_price(double v) = 0;

    /**
     * @brief 设置买单边数量
     */
    virtual void set_bid_volume(double v) = 0;

    /**
     * @brief 设置卖单边数量
     */
    virtual void set_ask_volume(double v) = 0;

    /**
     * @brief 设置买单边编号
     */
    virtual void set_bid_order_id(int64_t v) = 0;

    /**
     * @brief 设置卖单边编号
     */
    virtual void set_ask_order_id(int64_t v) = 0;

    /**
     * @brief 设置报价来源
     */
    virtual void set_quote_source(int8_t v) = 0;

    /**
     * @brief 设置报价状态
     */
    virtual void set_quote_status(int8_t v) = 0;

    /**
     * @brief 设置报价插入时间
     */
    virtual void set_insert_time(int32_t v) = 0;

    /**
     * @brief 设置报价更新时间
     */
    virtual void set_update_time(int32_t v) = 0;

    /**
     * @brief 设置组合号
     */
    virtual void set_portfolio_id(int32_t v) = 0;

    /**
     * @brief 设置交易账户
     */
    virtual void set_trading_account_id(int16_t v) = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成string形式
     */
    virtual std::string to_string() const = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成char*形式
     */
    virtual const char *c_str() const = 0;
};

struct rtn_trade_entity {
    static rtn_trade_entity *create_entity();

    virtual ~rtn_trade_entity() {}

    /**
     * @brief 重新设置结构体
     */
    virtual void reset_entity() = 0;

    /**
     * @brief 设置交易所编号
     */
    virtual void set_exchange_id(int8_t v) = 0;

    /**
     * @brief 设置成交编号
     */
    virtual void set_trade_id(int32_t v) = 0;

    /**
     * @brief 设置订单编号
     */
    virtual void set_order_id(int64_t v) = 0;

    /**
     * @brief 设置柜台报单编号
     */
    virtual void set_order_sys_id(const char *v) = 0;

    /**
     * @brief 设置柜台成交编号
     */
    virtual void set_trade_sys_id(const char *v) = 0;

    /**
     * @brief 设置合约号
     */
    virtual void set_instrument_id(const char *v) = 0;

    /**
     * @brief 设置方向
     */
    virtual void set_direction(int8_t v) = 0;

    /**
     * @brief 设置开平标志
     */
    virtual void set_offset_flag(int8_t v) = 0;

    /**
     * @brief 设置对冲标志
     */
    virtual void set_hedge_flag(int8_t v) = 0;

    /**
     * @brief 设置成交价
     */
    virtual void set_price(double v) = 0;

    /**
     * @brief 设置成交数量
     */
    virtual void set_volume(int32_t v) = 0;

    /**
     * @brief 设置成交时间
     */
    virtual void set_trade_time(int32_t v) = 0;

    /**
     * @brief 设置成交来源
     */
    virtual void set_trade_source(int8_t v) = 0;

    /**
     * @brief 设置交易账户
     */
    virtual void set_trading_account_id(int16_t v) = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成string形式
     */
    virtual std::string to_string() const = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成char*形式
     */
    virtual const char *c_str() const = 0;
};

struct inquiry_quote_entity {
    static inquiry_quote_entity *create_entity();

    virtual ~inquiry_quote_entity() {}

    /**
     * @brief 重新设置结构体
     */
    virtual void reset_entity() = 0;

    /**
     * @brief 设置询价编号
     */
    virtual void set_inquiry_id(const char *v) = 0;

    /**
     * @brief 设置交易所编号
     */
    virtual void set_exchange_id(int8_t v) = 0;

    /**
     * @brief 设置产品编号
     */
    virtual void set_product_id(const char *v) = 0;

    /**
     * @brief 设置合约号
     */
    virtual void set_instrument_id(const char *v) = 0;

    /**
     * @brief 设置询价状态
     */
    virtual void set_inquiry_quote_status(int8_t v) = 0;

    /**
     * @brief 设置询价时间
     */
    virtual void set_inquiry_time(int32_t v) = 0;

    /**
     * @brief 设置应答时间
     */
    virtual void set_response_time(int32_t v) = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成string形式
     */
    virtual std::string to_string() const = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成char*形式
     */
    virtual const char *c_str() const = 0;
};

struct instrument_trading_status_entity {
    static instrument_trading_status_entity *create_entity();

    virtual ~instrument_trading_status_entity() {}

    /**
     * @brief 重新设置结构体
     */
    virtual void reset_entity() = 0;

    /**
     * @brief 设置合约编号
     */
    virtual void        set_instrument_id(const char *v)        = 0;

    /**
     * @brief 设置合约交易状态
     */
    virtual void        set_instrument_trading_status(int8_t v) = 0;

    /**
     * @brief 设置交易所编号
     */
    virtual void        set_exchange_id(int8_t v)               = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成string形式
     */
    virtual std::string to_string() const                       = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成char*形式
     */
    virtual const char *c_str() const                           = 0;
};

struct rtn_position_entity {
    static rtn_position_entity *create_entity();

    virtual ~rtn_position_entity() {}

    /**
     * @brief 重新设置结构体
     */
    virtual void reset_entity() = 0;

    /**
     * @brief 设置合约号
     */
    virtual void        set_instrument_id(const char *v)  = 0;

    /**
     * @brief 设置交易账户
     */
    virtual void        set_trading_account_id(int16_t v) = 0;

    /**
     * @brief 设置方向
     */
    virtual void        set_direction(int8_t v)           = 0;

    /**
     * @brief 设置持仓
     */
    virtual void        set_position(int32_t v)           = 0;

    /**
     * @brief 设置今持仓
     */
    virtual void        set_td_position(int32_t v)        = 0;

    /**
     * @brief 设置昨持仓
     */
    virtual void        set_yd_position(int32_t v)        = 0;

    /**
     * @brief 设置交易所编号
     */
    virtual void        set_exchange_id(int8_t v)         = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成string形式
     */
    virtual std::string to_string() const                 = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成char*形式
     */
    virtual const char *c_str() const                     = 0;
};

struct rtn_security_position_entity {
    static rtn_security_position_entity *create_entity();

    virtual ~rtn_security_position_entity() {}

    /**
     * @brief 重新设置结构体
     */
    virtual void reset_entity() = 0;

    /**
     * @brief 设置合约号
     */
    virtual void        set_instrument_id(const char *v)  = 0;

    /**
     * @brief 设置交易账户
     */
    virtual void        set_trading_account_id(int16_t v) = 0;

    /**
     * @brief 设置今持仓
     */
    virtual void        set_td_position(int32_t v)        = 0;

    /**
     * @brief 设置昨持仓
     */
    virtual void        set_yd_position(int32_t v)        = 0;

    /**
     * @brief 设置申购仓
     */
    virtual void        set_creation_position(int32_t v)  = 0;

    /**
     * @brief 设置交易所编号
     */
    virtual void        set_exchange_id(int8_t v)         = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成string形式
     */
    virtual std::string to_string() const                 = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成char*形式
     */
    virtual const char *c_str() const                     = 0;
};

struct investor_account_fund_entity {
    static investor_account_fund_entity *create_entity();

    virtual ~investor_account_fund_entity() {}

    /**
     * @brief 重新设置结构体
     */
    virtual void reset_entity() = 0;

    /**
     * @brief 设置交易账户
     */
    virtual void        set_investor_account_id(const char *v) = 0;

    /**
     * @brief 设置经纪公司编号
     */
    virtual void        set_broker_id(const char *v)       = 0;

    /**
     * @brief 设置动态权利金
     */
    virtual void        set_dynamic_rights(double v)       = 0;

    /**
     * @brief 设置柜台编号
     */
    virtual void        set_counter_id(int8_t v)           = 0;

    /**
     * @brief 设置可用资金
     */
    virtual void        set_available_fund(double v)       = 0;

    /**
     * @brief 设置当前保证金
     */
    virtual void        set_curr_margin(double v)          = 0;

    /**
     * @brief 设置持仓盈亏
     */
    virtual void        set_position_profit(double v)      = 0;

    /**
     * @brief 设置冻结保证金
     */
    virtual void        set_frozen_margin(double v)        = 0;

    /**
     * @brief 设置冻结税费
     */
    virtual void        set_frozen_fee(double v)           = 0;

    /**
     * @brief 设置冻结权利金
	 */
    virtual void        set_frozen_premium(double v)       = 0;

    /**
     * @brief 设置收盘盈亏
     */
    virtual void        set_close_profit(double v)         = 0;

    /**
     * @brief 设置权利金
     */
    virtual void        set_premium(double v)              = 0;

    /**
     * @brief 设置税费
     */
    virtual void        set_fee(double v)                  = 0;

    /**
     * @brief 设置入金金额
     */
    virtual void        set_deposit(double v)              = 0;

    /**
     * @brief 设置出金金额
     */
    virtual void        set_withdraw(double v)             = 0;

    /**
     * @brief 设置货币质入金额
     */
    virtual void        set_mortgage_in_fund(double v)     = 0;

    /**
     * @brief 设置货币质出金额
     */
    virtual void        set_mortgage_out_fund(double v)    = 0;

    /**
     * @brief 设置持仓风险等级
     */
    virtual void        set_position_risk_degree(double v) = 0;

    /**
     * @brief 设置风险等级
     */
    virtual void        set_risk_degree(double v)          = 0;

    /**
     * @brief 设置交易账户
     */
    virtual void        set_trading_account_id(int16_t v)  = 0;

    /**
     * @brief 设置本地更新时间
     */
    virtual void        set_local_update_time(int32_t v)   = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成string形式
     */
    virtual std::string to_string() const                  = 0;

    /**
     * @brief 将结构体的所有字段名及值转换成char*形式
     */
    virtual const char *c_str() const                      = 0;
};

struct rsp_comb_entity {
    static rsp_comb_entity *create_entity();

    virtual ~rsp_comb_entity() {}
    virtual void reset_entity()                    = 0;
    virtual void set_comb_id(int64_t v)            = 0;
    virtual void set_exchange_id(int8_t v)         = 0;
    virtual void set_comb_sys_id(const char *v)    = 0;
    virtual void set_comb_strategy(int8_t v)       = 0;
    virtual void set_instrument_id1(const char *v) = 0;
    virtual void set_instrument_id2(const char *v) = 0;
    virtual void set_comb_direction(int8_t v)      = 0;
    virtual void set_comb_status(int8_t v)         = 0;
    virtual void set_trading_account_id(int16_t v) = 0;
    virtual void set_error_id(int32_t v)           = 0;

    virtual std::string to_string() const = 0;
    virtual const char *c_str() const     = 0;
};

struct rtn_comb_entity {
    static rtn_comb_entity *create_entity();

    virtual ~rtn_comb_entity() {}
    virtual void reset_entity() = 0;

    virtual void set_comb_id(int64_t v)            = 0;
    virtual void set_exchange_id(int8_t v)         = 0;
    virtual void set_comb_sys_id(const char *v)    = 0;
    virtual void set_comb_strategy(int8_t v)       = 0;
    virtual void set_instrument_id1(const char *v) = 0;
    virtual void set_instrument_id2(const char *v) = 0;
    virtual void set_direction1(int8_t v)          = 0;
    virtual void set_direction2(int8_t v)          = 0;
    virtual void set_comb_direction(int8_t v)      = 0;
    virtual void set_position(int32_t v)           = 0;
    virtual void set_comb_source(int8_t v)         = 0;
    virtual void set_comb_status(int8_t v)         = 0;
    virtual void set_comb_action(int8_t v)         = 0;
    virtual void set_trading_account_id(int16_t v) = 0;
    virtual void set_error_id(int32_t v)           = 0;

    virtual std::string to_string() const = 0;
    virtual const char *c_str() const     = 0;
};

struct comb_single_position_entity {
    static comb_single_position_entity *create_entity();

    virtual ~comb_single_position_entity() {}
    virtual void reset_entity() = 0;

    virtual void set_exchange_id(int8_t v)         = 0;
    virtual void set_instrument_id(const char *v)  = 0;
    virtual void set_direction(int8_t v)           = 0;
    virtual void set_position(int32_t v)           = 0;
    virtual void set_trading_account_id(int16_t v) = 0;

    virtual std::string to_string() const = 0;
    virtual const char *c_str() const     = 0;
};

struct comb_strategy_position_entity {
    static comb_strategy_position_entity *create_entity();

    virtual ~comb_strategy_position_entity() {}
    virtual void reset_entity() = 0;

    virtual void set_comb_strategy(int8_t v)       = 0;
    virtual void set_instrument_id1(const char *v) = 0;
    virtual void set_instrument_id2(const char *v) = 0;
    virtual void set_direction(int8_t v)           = 0;
    virtual void set_position(int32_t v)           = 0;
    virtual void set_exchange_id(int8_t v)         = 0;
    virtual void set_trading_account_id(int16_t v) = 0;

    virtual std::string to_string() const = 0;
    virtual const char *c_str() const     = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
