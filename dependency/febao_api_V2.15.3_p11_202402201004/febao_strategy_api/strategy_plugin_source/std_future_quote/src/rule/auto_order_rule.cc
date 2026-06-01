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

#include "auto_order_rule.h"
#include "math_helper.h"

namespace cffex {
namespace strategy {

auto_order_rule::auto_order_rule(caller_handler *callers, data_manager *data, int strategy_instance_id) : rule_impl(callers, data, strategy_instance_id), bid_id_(0), ask_id_(0) {
}

void auto_order_rule::get_current_info(double *cur_bid_p, double *cur_ask_p)
{
    order_msg_type *bid_msg = data_->get_msg_by_id<cffex::fb::order_stream>(bid_id_);
    *cur_bid_p = order_helper::is_active(bid_msg) ? bid_msg->get_price() : INVALID_PRICE;

    order_msg_type *ask_msg = data_->get_msg_by_id<cffex::fb::order_stream>(ask_id_);
    *cur_ask_p = order_helper::is_active(ask_msg) ? ask_msg->get_price() : INVALID_PRICE;
}

void auto_order_rule::on_msg_order(order_msg_type *msg) {
    int8_t status = msg->get_order_status();
    if(bid_id_ == 0 || ask_id_ == 0) return;
    if(data_->get_msg_by_id<cffex::fb::order_stream>(bid_id_) == NULL || data_->get_msg_by_id<cffex::fb::order_stream>(ask_id_) == NULL)
        return;
    int8_t bid_status_ = data_->get_msg_by_id<cffex::fb::order_stream>(bid_id_)->get_order_status();
    int8_t ask_status_ = data_->get_msg_by_id<cffex::fb::order_stream>(ask_id_)->get_order_status();

    if(status == cffex::fb::STRATEGY_ORDER_STATUS_ERROR) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "auto_order_rule::%s, error order, do requote, [%s]\n", __FUNCTION__, msg->to_string().c_str());
        cancel();
        delay_quoting_ = true;
        error_timer_ = callers_->get_timer_caller()->register_timer(data_->quote_delay_msec, std::bind(&rule_impl::finish_delay_quote, this), false);
    }
    else if(bid_status_ == cffex::fb::STRATEGY_ORDER_STATUS_IN_BOOK && ask_status_ == cffex::fb::STRATEGY_ORDER_STATUS_IN_BOOK) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "auto_order_rule::%s, bi-order success, start timer, [%s] ask_id[%ld] bid_id[%ld]\n", __FUNCTION__, msg->to_string().c_str(),ask_id_,bid_id_);
        quote_refresh_timer_ = callers_->get_timer_caller()->register_timer(data_->quote_refresh_msec,
            std::bind(&auto_order_rule::check_curr_order, this, ask_id_, bid_id_), false);
    }
    else if(order_helper::is_final(bid_status_) && order_helper::is_final(ask_status_)) {
        check_insert();
    }
}


void auto_order_rule::do_insert()
{
    order_helper::insert_order(callers_->get_order_caller(), data_->portfolio_name, data_->instrument_id, cffex::fb::STRATEGY_DIRECTION_BUY,
        cffex::fb::STRATEGY_OFFSET_FLAG_AUTO, bid_price_, data_->bid_volume, bid_id_, STRATEGY_NAME);
    order_helper::insert_order(callers_->get_order_caller(), data_->portfolio_name, data_->instrument_id, cffex::fb::STRATEGY_DIRECTION_SELL,
        cffex::fb::STRATEGY_OFFSET_FLAG_AUTO, ask_price_, data_->ask_volume, ask_id_, STRATEGY_NAME);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "auto_order_rule::%s, bid_id_[%ld] ask_id_[%ld]\n", __FUNCTION__, bid_id_, ask_id_);
}

void auto_order_rule::cancel()
{
    bool pre_check = true;
    pre_check &= (data_->get_msg_by_id<cffex::fb::order_stream>(bid_id_) != NULL || bid_id_ == 0);
    pre_check &= (data_->get_msg_by_id<cffex::fb::order_stream>(ask_id_) != NULL || ask_id_ == 0);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "auto_order_rule::%s, pre_check[%d]\n", __FUNCTION__, pre_check);
    if(pre_check ) {
        cancel_instance_order(instance_id_);
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "auto_order_rule::%s, bid_id_[%ld] ask_id_[%ld]\n", __FUNCTION__, bid_id_, ask_id_);
    }
}

void auto_order_rule::do_refresh() {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "auto_order_rule::%s, bid_id_[%ld] ask_id_[%ld]\n", __FUNCTION__, bid_id_, ask_id_);
    if( order_helper::is_final(data_->get_msg_by_id<cffex::fb::order_stream>(bid_id_), bid_id_) &&
        order_helper::is_final(data_->get_msg_by_id<cffex::fb::order_stream>(ask_id_), ask_id_)) {
        check_insert();
    }
    else {
        cancel();
    }
}

void auto_order_rule::check_curr_order(int64_t ask_id, int64_t bid_id) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "auto_order_rule::%s, bid_id_[%ld] ask_id_[%ld]\n", __FUNCTION__, bid_id_, ask_id_);
    if(ask_id_ != ask_id || bid_id_ != bid_id)
        return;
    do_refresh();
}

}
}