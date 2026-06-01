/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: --
 * Date: 2019-12-17
 */

#ifndef STD_OPTION_AUTO_QUOTE_RULE_H
#define STD_OPTION_AUTO_QUOTE_RULE_H

#include <set>
#include "caller_handler.h"
#include "data_manager.h"

namespace cffex {
namespace strategy {

class rule_quote
{
public:
    rule_quote(caller_handler *callers, data_manager *data) : callers_(callers), data_(data) { }
    ~rule_quote() { callers_->get_timer_caller()->unregister_timer(timer_id_); }

    void reset() {
        quote_bid_ = DBL_MAX;
        quote_ask_ = DBL_MAX;
        timer_id_ = callers_->get_timer_caller()->register_timer(2000, std::bind(&rule_quote::on_timer, this), true);
        quoting_period_ = INVALID_QUOTING_PERIOD;
        unfinish_quotes_.clear();
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "reset\n");
    }

    /* insert quote to exchange */
    void insert_quote(const char *instrument, bool force = false);
    /* insert quote to exchange, all instance related instrument */
    void insert_quote();

    /* cancel single instrument quote*/
    void cancel_quote(const char *instrument);
    void cancel_quote(const char *instrument, int64_t quote_id);
    /* cancel instance all quote to exchange */
    void cancel_quote();

    void on_trade_pending_timer(std::string instrument_id);
    void turn_off_quote_switch(const char* instrument);
    void turn_on_quote_switch(const char* instrument);
    void on_msg_trade(trade_msg_type *msg);
    void on_msg_quote(quote_msg_type *msg);
    void on_msg_quote_cancel(quote_cancel_msg_type *msg);

private:
    bool check_trading_section();

    bool check_quote_swicth(const char *instrument_id);

    /* create new quote using base price */
    bool create_quote(const char *instrument_id);

    // compare with self quote, shouldn't deviate too much
    bool check_self(const char *instrument_id);

    // compare with market price, shouldn't cause immediate deal
    bool check_market(const char *instrument_id);

    bool check_risk(const char *instrument_id);

    void send_quote(const char *instrument_id);

    void on_recancel_timer(const char *instrument, int64_t quote_id);

    void on_timer();

private:
    double              quote_bid_;
    double              quote_ask_;
    int                 timer_id_;
    bool                quoting_period_;
    caller_handler     *callers_;
    data_manager       *data_;

    //std::set<int64_t>   unfinish_orders_;
    std::set<int64_t>   unfinish_quotes_;
    //std::map<int64_t, int64_t> order_quote_map_;
};


}
}

#endif