/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: wangty
 * Date: 2019-11-11
 */

#include "delta_hedge_rule.h"
#include "math_helper.h"
#include "float.h"

namespace cffex {
namespace strategy {

delta_hedge_rule::delta_hedge_rule(caller_handler *callers, data_manager *data)
: base_rule(callers, data), portfolio_delta_(0), price_(0.0), volume_(0), direction_(0), md_bid_(0.0), md_ask_(0), pending_timer_(false) {

}

void delta_hedge_rule::hedge() {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "delta_hedge_rule::%s\n", __FUNCTION__);
    hedge(portfolio_delta_);
}

void delta_hedge_rule::hedge(double delta) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "delta_hedge_rule::%s, %0.3f\n", __FUNCTION__, delta);
    portfolio_delta_ = delta;
    if(!check_trading_phase()) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "delta_hedge_rule::%s current time not in trading_section\n", __FUNCTION__);
        return ;
    }

    if (!check_trigger()) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "delta_hedge_rule::%s trigger_threshold check failed\n", __FUNCTION__);
        return ;
    }

    if(!check_interval()) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "delta_hedge_rule::%s pending timer, don't hedge too frequently\n", __FUNCTION__);
        return ;
    }

    if(!check_market()) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "delta_hedge_rule::%s check market failed\n", __FUNCTION__);
        return ;
    }

    if (!calculate_hedge_data()) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "delta_hedge_rule::%s calculate_hedge_data failed\n", __FUNCTION__);
    }

    insert_order();
}

bool delta_hedge_rule::check_trigger() {
    if ( math_helper::less(fabs(portfolio_delta_), data_->trigger_threshold) ) {
        return false;
    }
    return true;
}

bool delta_hedge_rule::check_interval() {
    return !pending_timer_;
}

bool delta_hedge_rule::check_market() {
    md_bid_ = get_market_bid(data_->underlying_id);
    md_ask_ = get_market_ask(data_->underlying_id);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "delta_hedge_rule::%s, md_bid[%.3f], md_ask[%.3f]\n", __FUNCTION__, md_bid_, md_ask_);

    if ( !math_helper::active_price(md_bid_) || !math_helper::active_price(md_ask_) ) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "delta_hedge_rule::%s market bid or ask is null\n", __FUNCTION__);
        return false;
    }

    if ( math_helper::greater(md_ask_ - md_bid_, data_->md_max_spread * data_->tick) ) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "delta_hedge_rule::%s, failed, underlying too big md_spread[%.2f], bid[%.2f], ask[%.2f], md_max_spread[%.2f]\n",
            __FUNCTION__, md_ask_ - md_bid_,  md_bid_, md_ask_, data_->md_max_spread);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "delta_hedge_rule::%s, failed, underlying too big md_spread[%.2f],  bid[%.2f], ask[%.2f], md_max_spread[%.2f]",
            __FUNCTION__, md_ask_ - md_bid_, md_bid_, md_ask_, data_->md_max_spread);
        return false;
    }
    return true;
}

bool delta_hedge_rule::calculate_hedge_data() {
    double underlying_delta = get_instrument_delta(data_->underlying_id);
    if (math_helper::equal(underlying_delta, 0.0)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "delta_hedge_rule::%s underlying[%s] delta is zero\n", __FUNCTION__, data_->underlying_id);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "underlying[%s] delta is zero", data_->underlying_id);
        return false;
    }

    double change_delta;
    if (portfolio_delta_ > 0) {  // portfolio_delta_ > threshold
        change_delta = data_->target_threshold - portfolio_delta_;  // < 0
        volume_    = int(fabs(change_delta/underlying_delta)) + 1;
        direction_ = math_helper::less(underlying_delta, 0 ) ? cffex::fb::STRATEGY_DIRECTION_BUY : cffex::fb::STRATEGY_DIRECTION_SELL;
        price_     = math_helper::less(underlying_delta, 0 ) ? md_ask_ : md_bid_;
        volume_    = math_helper::less(underlying_delta, 0 ) ? math_helper::min(volume_, get_market_ask_volume(data_->underlying_id)) : math_helper::min(volume_, get_market_bid_volume(data_->underlying_id));
    }
    else {   // portfolio_delta_ < -threshold
        change_delta = 0 - data_->target_threshold - portfolio_delta_;  // > 0
        volume_    = int(fabs(change_delta / underlying_delta)) + 1;
        direction_ = math_helper::greater(underlying_delta, 0 ) ? cffex::fb::STRATEGY_DIRECTION_BUY : cffex::fb::STRATEGY_DIRECTION_SELL;
        price_     = math_helper::greater(underlying_delta, 0 ) ? md_ask_ : md_bid_;
        volume_    = math_helper::greater(underlying_delta, 0 ) ? math_helper::min(volume_, get_market_ask_volume(data_->underlying_id)) : math_helper::min(volume_, get_market_bid_volume(data_->underlying_id));
    }

    volume_ = math_helper::min(volume_, data_->max_volume);
    if(volume_ < 1) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "delta_hedge_rule::%s invalid hedge volume\n",  __FUNCTION__);
        return false;
    }
    return true;
}

void delta_hedge_rule::insert_order() {
    int64_t order_id;
    base_rule::insert_order(price_, volume_, direction_, data_->portfolio_name, order_id);

    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "send_order success, portfolio_delta[%.2f], underlying[%s], price[%.2f], volume[%d], direction[%d], order_id[%ld]\n",
        portfolio_delta_, data_->underlying_id, price_, volume_, direction_, order_id);
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO,
        "send_order success, portfolio_delta[%.2f], instrument_id[%s], price[%.2f], volume[%d], direction[%d], order_id[%ld]",
        portfolio_delta_, data_->underlying_id, price_, volume_, direction_, order_id);

    // waiting several minutes before next hedge order send
    pending_timer_ = true;
    callers_->get_timer_caller()->register_timer(data_->interval * 1000, std::bind(&delta_hedge_rule::on_timer, this), false);
}

void delta_hedge_rule::on_timer() {
    pending_timer_ = false;
    this->hedge();
}

double delta_hedge_rule::get_instrument_delta(const char* instrument) {
    derived_md_msg_type* msg = data_->get_msg_by_instrument<cffex::fb::derived_md_stream>(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "delta_hedge_rule::%s, invalid instrument[%s]\n", __FUNCTION__, instrument);
        return 0.0;
    }
    instrument_msg_type* ins_msg = data_->get_msg_by_instrument<cffex::fb::instrument_stream>(instrument);
    if(ins_msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "delta_hedge_rule::%s, invalid instrument[%s]\n", __FUNCTION__, instrument);
        return 0.0;
    }
    if (cffex::fb::STRATEGY_INSTRUMENT_OPTION == ins_msg->get_instrument_type()) {
        return msg->get_delta();
    }
    else {
        return 1.0;
    }

}

}
}
