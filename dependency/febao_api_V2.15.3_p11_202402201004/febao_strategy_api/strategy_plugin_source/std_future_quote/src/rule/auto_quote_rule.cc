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
 * Date: 2019-12-09
 */

#include "auto_quote_rule.h"
#include "math_helper.h"

namespace cffex {
namespace strategy {

auto_quote_rule::auto_quote_rule(caller_handler *callers, data_manager *data, int strategy_instance_id): rule_impl(callers, data, strategy_instance_id), quote_id_(0) {
}

void auto_quote_rule::get_current_info(double *cur_bid_p, double *cur_ask_p){
    quote_msg_type *quote = data_->get_msg_by_id<cffex::fb::quote_stream>(quote_id_);
    *cur_bid_p = quote_helper::is_active(quote) ? quote->get_bid_price() : INVALID_PRICE;

    *cur_ask_p = quote_helper::is_active(quote) ? quote->get_ask_price() : INVALID_PRICE;
}

void auto_quote_rule::on_msg_quote(quote_msg_type *msg) {
    int8_t quote_status = msg->get_quote_status();
    if(quote_status == cffex::fb::STRATEGY_QUOTE_STATUS_ERROR) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "auto_quote_rule::%s, error quote, do requote, [%s]\n", __FUNCTION__, msg->to_string().c_str());
        delay_quoting_ = true;
        error_timer_ = callers_->get_timer_caller()->register_timer(data_->quote_delay_msec, std::bind(&auto_quote_rule::finish_delay_quote, this), false);
    } else if(quote_status == cffex::fb::STRATEGY_QUOTE_STATUS_IN_BOOK) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "auto_quote_rule::%s, quote success, start timer, [%s]\n", __FUNCTION__, msg->to_string().c_str());
        quote_refresh_timer_ = callers_->get_timer_caller()->register_timer(data_->quote_refresh_msec,
            std::bind(&auto_quote_rule::check_curr_quote, this, quote_id_), false);
    }else if(quote_status == cffex::fb::STRATEGY_QUOTE_STATUS_ALL_CANCEL && data_->quote_mode == QUOTE_NORMAL_MODE_TYPE){
         check_insert();
     }
}

void auto_quote_rule::cancel()
{
    quote_helper::cancel_quote(callers_->get_quote_caller(), data_->portfolio_name, data_->instrument_id, quote_id_);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "auto_quote_rule::%s, quote_id_[%ld] \n", __FUNCTION__, quote_id_);
}

void auto_quote_rule::do_insert()
{
    quote_helper::insert_quote(callers_->get_quote_caller(), data_->portfolio_name, data_->instrument_id,
        cffex::fb::STRATEGY_OFFSET_FLAG_AUTO, cffex::fb::STRATEGY_OFFSET_FLAG_AUTO,
        bid_price_, ask_price_, data_->bid_volume, data_->ask_volume, quote_id_);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "auto_quote_rule::%s, quote_id_[%ld] \n", __FUNCTION__, quote_id_);
}

void auto_quote_rule::check_curr_quote(int64_t quote_id) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "auto_order_rule::%s, quote_id[%ld] \n", __FUNCTION__, quote_id_);
    if(quote_id_ != quote_id)
        return;
    do_refresh();
}

void auto_quote_rule::do_refresh()
{
    if (data_->quote_mode == QUOTE_POP_MODE_TYPE) {
        check_insert();
    } else {
        quote_msg_type *quote = data_->get_msg_by_id<cffex::fb::quote_stream>(quote_id_);
        if (!quote_helper::is_active(quote))  {
            check_insert();
        } else {
            cancel();
        }
    }
}

}
}