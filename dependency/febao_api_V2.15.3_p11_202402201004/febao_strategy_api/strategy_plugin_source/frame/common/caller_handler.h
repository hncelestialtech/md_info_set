/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: gp
 * Date: 2019-07-13
 */

#ifndef STRATEGY_FRAME_CALLER_HANDLER_H
#define STRATEGY_FRAME_CALLER_HANDLER_H

#include "strategy_api.h"

namespace cffex {
namespace strategy {

class caller_handler
{
 public:
    caller_handler(cffex::fb::i_strategy *obj) : obj_(obj) { }
    ~caller_handler() { }

    cffex::fb::i_timer_caller   *get_timer_caller()
    {
        return get_caller<cffex::fb::i_timer_caller>();
    }
    cffex::fb::i_quote_caller   *get_quote_caller()
    {
        return get_caller<cffex::fb::i_quote_caller>();
    }
    cffex::fb::i_order_caller   *get_order_caller()
    {
        return get_caller<cffex::fb::i_order_caller>();
    }
    cffex::fb::i_instrument_group_param_caller   *get_instrument_group_param_caller()
    {
        return get_caller<cffex::fb::i_instrument_group_param_caller>();
    }
    cffex::fb::i_instrument_param_caller   *get_instrument_param_caller()
    {
        return get_caller<cffex::fb::i_instrument_param_caller>();
    }
    cffex::fb::i_log_caller   *get_log_caller()
    {
        return get_caller<cffex::fb::i_log_caller>();
    }
    cffex::fb::i_custom_param_caller   *get_custom_param_caller()
    {
        return get_caller<cffex::fb::i_custom_param_caller>();
    }
    cffex::fb::i_serial_pricing_param_caller   *get_serial_pricing_param_caller()
    {
        return get_caller<cffex::fb::i_serial_pricing_param_caller>();
    }
    cffex::fb::i_instrument_pricing_param_caller   *get_instrument_pricing_param_caller()
    {
        return get_caller<cffex::fb::i_instrument_pricing_param_caller>();
    }
    cffex::fb::i_volatility_param_caller   *get_volatility_param_caller()
    {
        return get_caller<cffex::fb::i_volatility_param_caller>();
    }
    cffex::fb::i_volatility_offset_caller   *get_volatility_offset_caller()
    {
        return get_caller<cffex::fb::i_volatility_offset_caller>();
    }
    cffex::fb::i_trade_caller *get_trade_caller() {
        return get_caller<cffex::fb::i_trade_caller>();
    }

 private:
    template <typename caller_type>
    caller_type *get_caller()
    {
        return (caller_type *)obj_->get_caller(caller_type::ID);
    }

 private:
    cffex::fb::i_strategy *obj_;
};

}
}

#endif
