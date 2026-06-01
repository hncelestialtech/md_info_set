/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: fangyi
 * Date: 2021-08-02
 */

#ifndef STRATEGY_RULE_IMPL_H
#define STRATEGY_RULE_IMPL_H

#include "caller_handler.h"
#include "data_manager.h"
#include "math_helper.h"
#include "base_rule.h"
#include "price_helper.h"

namespace cffex {
namespace strategy {

class rule_impl : public base_rule {
public:
    rule_impl(caller_handler *callers, data_manager *data, int strategy_instance_id);
    virtual ~rule_impl();

public:
    /* public interface for auto_quote_rule and auto_order_rule */
    virtual void cancel() = 0;
    virtual void do_insert() = 0;
    virtual void do_refresh() = 0;
    virtual void on_msg_order(order_msg_type *msg) {};
    virtual void on_msg_quote(quote_msg_type *msg) {};
    virtual void get_current_info(double *cur_bid_p, double *cur_ask_p) = 0;

public:
    void on_msg_md(md_msg_type *msg);
    void finish_delay_quote();
    void check(bool md_update);
    void on_msg_trade(trade_msg_type *msg);

protected:
    bool check_trading_phase(trading_time_msg_type *msg = NULL);
    bool check_md_spread(md_msg_type *msg = NULL);
    bool check_md_volume(md_msg_type *msg = NULL);
    bool check_net_position(position_msg_type *msg = NULL);
    void check_insert();
    void calculate();
    bool calculate_baseprice(md_msg_type *msg, double &baseprice);
    bool calculate_price(md_msg_type *msg, double baseprice, double &bid_price, double &ask_price);

    int quote_refresh_timer_;
    int error_timer_;
    bool delay_quoting_;
    double bid_price_;
    double ask_price_;
};


}
}

#endif