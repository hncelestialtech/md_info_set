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

#include "rule_impl.h"
#include "price_helper.h"

namespace cffex {
namespace strategy {
rule_impl::rule_impl(caller_handler *callers, data_manager *data,int strategy_instance_id) : base_rule(callers, data, strategy_instance_id), quote_refresh_timer_(0), error_timer_(0), delay_quoting_(false), bid_price_(0), ask_price_(0) {
}
rule_impl::~rule_impl() {
    callers_->get_timer_caller()->unregister_timer(quote_refresh_timer_);
    callers_->get_timer_caller()->unregister_timer(error_timer_);
}

void rule_impl::on_msg_trade(trade_msg_type *msg) {
    if (strcmp(msg->get_instrument_id(), data_->instrument_id) == 0) {
        cancel();
    }
    delay_quoting_ = true;
    quote_refresh_timer_ = callers_->get_timer_caller()->register_timer(data_->quote_delay_msec, std::bind(&rule_impl::finish_delay_quote, this), false);
}

void rule_impl::finish_delay_quote() {
    delay_quoting_ = false;
    check(true);
}

void rule_impl::on_msg_md(md_msg_type *msg) {
    check(true);
}

void rule_impl::calculate() {
    bid_price_  = INVALID_PRICE;
    ask_price_  = INVALID_PRICE;
    md_msg_type *md = data_->get_msg_by_instrument<cffex::fb::md_stream>(data_->instrument_id);
    if(md == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, md is null\n", __FUNCTION__);
        return;
    }
    if(!check_md_spread(md)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, pre check failed, invalid md_spread\n", __FUNCTION__);
        cancel();
        return;
    }
    if(!check_md_volume(md)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, pre check failed, invalid md_volume\n", __FUNCTION__);
        cancel();
        return;
    }
    double base_price = 0;
    if(!calculate_baseprice(md, base_price)) {
        return;
    }
    if(!calculate_price(md, base_price, bid_price_, ask_price_)) {
        return;
    }
}

void rule_impl::check_insert() {
    if(!check_trading_phase()) {
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, pre check failed, invalid trading_phase\n", __FUNCTION__);
        cancel();
        return;
    }
    if(!check_net_position()) {
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, pre check failed, invalid net_position\n", __FUNCTION__);
        cancel();
        return;
    }
    if(delay_quoting_){
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, pre check failed, in delay_quoting\n", __FUNCTION__);
        return;
    }
    if(!data_->auto_quote_switch){
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, pre check failed, auto_quote_switch false\n", __FUNCTION__);
        return;
    }
    do_insert();
}

void rule_impl::check(bool md_update) {
    if (md_update) {
        calculate();
    }
    double cur_bid_p = 0.0;
    double cur_ask_p = 0.0;
    get_current_info(&cur_bid_p, &cur_ask_p);
    bool check_last_price_cmp = (math_helper::equal(bid_price_, cur_bid_p) && math_helper::equal(ask_price_, cur_ask_p));
    if(!check_last_price_cmp)
    {
        do_refresh();
    }
    else
    {
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, check_last_price_cmp[%d] bid_price[%lf] cur_bid_p[%lf] ask_price[%lf] cur_ask_p[%lf]\n", __FUNCTION__, check_last_price_cmp, bid_price_, cur_bid_p, ask_price_, cur_ask_p);
    }
}

bool rule_impl::check_trading_phase(trading_time_msg_type *msg) {
    if(data_->trading_section[0] == '\0') { /* if config is null, do not check */
        return true;
    }
    if(msg == NULL) {
        msg = data_->get_stream<cffex::fb::trading_time_template_stream>()->get_stream_table()->get_msg(data_->trading_section);
        if(msg == NULL) {
            callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, failed to get trading_time_template_stream msg, [%s]\n", __FUNCTION__, data_->trading_section);
            return false;
        }
    }
    return msg->is_trading_time(data_->exchange_id);
}

bool rule_impl::check_md_spread(md_msg_type *msg) {
    if(data_->md_threshold_spread <= 0) {
        return true; // do not check
    }
    if(msg == NULL) {
        msg = data_->get_msg_by_instrument<cffex::fb::md_stream>(data_->instrument_id);
        if(msg == NULL) {
            callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, failed to get md msg, [%s]\n", __FUNCTION__, data_->instrument_id);
            return false;
        }
    }
    double threshold = (data_->md_threshold_spread_type == THRESHOLD_TICK_TYPE) ? (data_->md_threshold_spread * data_->tick) : data_->md_threshold_spread;
    double bid = msg->get_bid1_price();
    double ask = msg->get_ask1_price();
    if(!math_helper::valid_price(bid) ||
       !math_helper::valid_price(ask) ||
       (ask - bid > threshold ) ) {
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, failed, bid[%lf] ask[%lf] threshold[%lf]\n", __FUNCTION__, bid, ask, threshold);
        return false;
    }
    return true;
}
bool rule_impl::check_md_volume(md_msg_type *msg) {
    if(data_->md_threshold_bid_depth <= 0 || data_->md_threshold_ask_depth <= 0 ||
       data_->md_threshold_bid_volume <= 0 || data_->md_threshold_ask_volume <= 0) {
        return true;
    }
    if(msg == NULL) {
        msg = data_->get_msg_by_instrument<cffex::fb::md_stream>(data_->instrument_id);
        if(msg == NULL) {
            callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, failed to get md msg, [%s]\n", __FUNCTION__, data_->instrument_id);
            return false;
        }
    }
    //check bid
    int bid_volume = price_helper::get_total_volume_by_level(data_->md_threshold_bid_depth, msg, cffex::fb::STRATEGY_DIRECTION_BUY);
    if(bid_volume < data_->md_threshold_bid_volume) {
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, failed, bid_volume[%d] md_threshold_bid_depth[%d] md_threshold_bid_volume[%d]\n",
            __FUNCTION__, bid_volume, data_->md_threshold_bid_depth, data_->md_threshold_bid_volume);
        return false;
    }
    //check ask
    int ask_volume = price_helper::get_total_volume_by_level(data_->md_threshold_ask_depth, msg, cffex::fb::STRATEGY_DIRECTION_SELL);
    if(ask_volume < data_->md_threshold_ask_volume) {
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, failed, ask_volume[%d] md_threshold_ask_depth[%d] md_threshold_ask_volume[%d]\n",
            __FUNCTION__, ask_volume, data_->md_threshold_ask_depth, data_->md_threshold_ask_volume);
        return false;
    }
    return true;
}
bool rule_impl::check_net_position(position_msg_type *msg) {
    if(msg == NULL) {
        msg = data_->get_msg_by_instrument<cffex::fb::position_stream>(data_->instrument_id);
        if(msg == NULL) {
            callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "rule_impl::%s, position msg is null, [%s]\n", __FUNCTION__, data_->instrument_id);
            return true;
        }
    }
    int net_position = msg->get_long_position() - msg->get_short_position();
    if(data_->position_threshold_upper != 0 && net_position > data_->position_threshold_upper) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, failed, position_threshold_upper[%d] net_position[%d]\n",
            __FUNCTION__, data_->position_threshold_upper, net_position);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, failed, position_threshold_upper[%d] net_position[%d]\n",
            __FUNCTION__, data_->position_threshold_upper, net_position);
        return false;
    }
    if(data_->position_threshold_lower != 0 && net_position < data_->position_threshold_lower) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, failed, position_threshold_lower[%d] net_position[%d]\n",
            __FUNCTION__, data_->position_threshold_lower, net_position);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, failed, position_threshold_lower[%d] net_position[%d]\n",
            __FUNCTION__, data_->position_threshold_lower, net_position);
        return false;
    }
    return true;
}

bool rule_impl::calculate_baseprice(md_msg_type *msg, double &baseprice) {
    double bid_price = msg->get_bid1_price();
    double ask_price = msg->get_ask1_price();
    if(!math_helper::valid_price(bid_price) || !math_helper::valid_price(ask_price)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_impl::%s, invalid md price, bid_price[%lf] ask_price[%lf]\n", __FUNCTION__, bid_price, ask_price);
        return false;
    }
    /*calculate baseprice*/
    baseprice = (bid_price + ask_price) / 2;
    /*baseprice shift*/
    baseprice += data_->baseprice_shift * data_->tick;
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_impl::%s, baseprice[%lf]\n", __FUNCTION__, baseprice);
    return true;
}

bool rule_impl::calculate_price(md_msg_type *msg, double baseprice, double &bid_price, double &ask_price) {
    /*calculate spread*/
    double market_spread = msg->get_ask1_price() - msg->get_bid1_price();
    double spread;
    switch(data_->spread_mode) {
      case MD_SPREAD_MODE:
        spread = market_spread;
        break;
      case FIX_SPREAD_MODE:
        spread = data_->fix_spread;
        break;
      case TEMPLATE_SPREAD_MODE:
        spread = data_->get_template_spread(baseprice);
        break;
      default:
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "rule_impl::%s, invalid spread_mode[%d]\n", __FUNCTION__, data_->spread_mode);
        return false;
    }
    if(!math_helper::valid_price(spread) || spread < 0) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "rule_impl::%s, invalid spread[%lf], spread_mode[%d]\n", __FUNCTION__, spread, data_->spread_mode);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "rule_impl::%s, invalid spread[%lf], spread_mode[%d]\n", __FUNCTION__, spread, data_->spread_mode);
        return false;
    }
    /*calculate price*/
    bid_price = math_helper::round(baseprice - spread / 2, data_->tick, math_helper::ROUNDING_MODEL_CEIL);
    ask_price = math_helper::round(baseprice + spread / 2, data_->tick, math_helper::ROUNDING_MODEL_CEIL);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_impl::%s, bid_price[%lf], ask_price[%lf]\n", __FUNCTION__, bid_price, ask_price);

    return true;
}

}
}


