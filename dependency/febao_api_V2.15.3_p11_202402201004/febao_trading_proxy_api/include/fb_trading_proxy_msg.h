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

#ifndef FB_TRADING_PROXY_API_MSG_H
#define FB_TRADING_PROXY_API_MSG_H

#include <stddef.h>

#include <string>


namespace cffex {
namespace fb {

// todo: remove useless interface
struct req_order_msg {
    virtual int64_t     get_order_id() const                = 0;
    virtual const char *get_order_sys_id() const            = 0;
    virtual const char *get_instrument_id() const           = 0;
    virtual int8_t      get_direction() const               = 0;
    virtual int8_t      get_exchange_id() const             = 0;
    virtual int8_t      get_offset_flag() const             = 0;
    virtual int8_t      get_hedge_flag() const              = 0;
    virtual double      get_price() const                   = 0;
    virtual int32_t     get_volume() const                  = 0;
    virtual int8_t      get_price_category() const          = 0;
    virtual int8_t      get_time_condition() const          = 0;
    virtual int8_t      get_volume_condition() const        = 0;
    virtual int8_t      get_order_status() const            = 0;
    virtual int8_t      get_outside_self_trade_type() const = 0;
    virtual int16_t     get_portfolio_id() const            = 0;
    virtual int16_t     get_trading_account_id() const      = 0;
    virtual int32_t     get_error_id() const                = 0;
    virtual std::string to_string() const                   = 0;
    virtual const char *c_str() const                       = 0;
};
struct req_quote_msg {
    virtual int64_t     get_quote_id() const                = 0;
    virtual const char *get_inquiry_id() const              = 0;
    virtual const char *get_quote_sys_id() const            = 0;
    virtual const char *get_instrument_id() const           = 0;
    virtual int8_t      get_exchange_id() const             = 0;
    virtual int8_t      get_bid_offset_flag() const         = 0;
    virtual int8_t      get_ask_offset_flag() const         = 0;
    virtual int8_t      get_bid_hedge_flag() const          = 0;
    virtual int8_t      get_ask_hedge_flag() const          = 0;
    virtual double      get_bid_price() const               = 0;
    virtual double      get_ask_price() const               = 0;
    virtual int32_t     get_bid_volume() const              = 0;
    virtual int32_t     get_ask_volume() const              = 0;
    virtual int8_t      get_time_condition() const          = 0;
    virtual int8_t      get_quote_status() const            = 0;
    virtual int8_t      get_outside_self_trade_type() const = 0;
    virtual int16_t     get_portfolio_id() const            = 0;
    virtual int16_t     get_trading_account_id() const      = 0;
    virtual int8_t      get_cancel_type() const             = 0;
    virtual std::string to_string() const                   = 0;
    virtual const char *c_str() const                       = 0;
};

struct instrument_msg {
    virtual const int8_t  get_instrument_type() const          = 0;
    virtual const char   *get_instrument_id() const            = 0;
    virtual const char   *get_instrument_name() const          = 0;
    virtual const int8_t  get_exchange_id() const              = 0;
    virtual const char   *get_product_id() const               = 0;
    virtual const double  get_multiple() const                 = 0;
    virtual const double  get_tick() const                     = 0;
    virtual const char   *get_option_serial_id() const         = 0;
    virtual const double  get_strike_price() const             = 0;
    virtual const char   *get_expire_date() const              = 0;
    virtual const int8_t  get_option_type() const              = 0;
    virtual const int8_t  get_expire_date_type() const         = 0;
    virtual const char   *get_underlying_instrument_id() const = 0;
    virtual const int8_t  get_exercise_type() const            = 0;
    virtual const int8_t  get_exercise_style() const           = 0;
    virtual const int16_t get_unit() const                     = 0;
    virtual std::string   to_string() const                    = 0;
    virtual const char   *c_str() const                        = 0;
};

class req_comb_msg {
public:
    virtual int64_t     get_comb_id() const              = 0;
    virtual int8_t      get_exchange_id() const          = 0;
    virtual const char *get_comb_sys_id() const          = 0;
    virtual int32_t     get_comb_strategy() const        = 0;
    virtual const char *get_instrument_id1() const       = 0;
    virtual const char *get_instrument_id2() const       = 0;
    virtual int8_t      get_direction1() const           = 0;
    virtual int8_t      get_direction2() const           = 0;
    virtual int8_t      get_comb_direction() const       = 0;
    virtual int32_t     get_position() const             = 0;
    virtual int32_t     get_split_position() const       = 0;
    virtual int8_t      get_comb_source() const          = 0;
    virtual int8_t      get_comb_status() const          = 0;
    virtual int8_t      get_comb_action() const          = 0;
    virtual int16_t     get_priority() const             = 0;
    virtual int32_t     get_strategy_instance_id() const = 0;
    virtual int16_t     get_user_id() const              = 0;
    virtual int16_t     get_action_user_id() const       = 0;
    virtual int16_t     get_trading_account_id() const   = 0;
    virtual int16_t     get_last_operator_id() const     = 0;
    virtual int32_t     get_error_id() const             = 0;
    virtual std::string to_string() const                = 0;
    virtual const char *c_str() const                    = 0;
};

}  // namespace fb
}  // namespace cffex

#endif
