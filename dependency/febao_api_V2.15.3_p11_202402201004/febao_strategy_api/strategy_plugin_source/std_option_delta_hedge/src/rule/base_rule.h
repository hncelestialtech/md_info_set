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
 * Date: 2019-01-06
 */


#ifndef CFFEX_STRATEGY_BASE_RULE_H
#define CFFEX_STRATEGY_BASE_RULE_H

#include "caller_handler.h"
#include "data_manager.h"
#include "math_helper.h"

namespace cffex {
namespace strategy {

class base_rule {
public:
    base_rule(caller_handler *callers, data_manager *data) : callers_(callers), data_(data) { }
    virtual ~base_rule() { }
    virtual bool insert_order(double price, int volume, int8_t direction, const char *portfolio_name, int64_t &order_id, int8_t offset_flag = cffex::fb::STRATEGY_OFFSET_FLAG_AUTO);
    virtual bool check_trading_phase(trading_time_msg_type *msg = NULL);
    virtual double get_market_bid(const char* instrument);
    virtual double get_market_ask(const char* instrument);
    virtual int get_market_bid_volume(const char* instrument);
    virtual int get_market_ask_volume(const char* instrument);

protected:
    caller_handler          *callers_;
    data_manager            *data_;
};

}
}

#endif
