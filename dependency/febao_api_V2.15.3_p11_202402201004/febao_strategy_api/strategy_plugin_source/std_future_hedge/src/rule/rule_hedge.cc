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
 * Date: 2020-04-09
 */

#include "rule_hedge.h"
#include "data_manager.h"
#include "order_helper.h"
#include "price_helper.h"

namespace cffex {
namespace strategy {

rule_hedge::rule_hedge(caller_handler *callers, data_manager *data) : base_rule(callers, data)
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_hedge::%s, hedge_cutloss_waiting[%d]\n", __FUNCTION__, data_->hedge_cutloss_waiting);
    error_timer_id_ = callers_->get_timer_caller()->register_timer(data_->hedge_cutloss_waiting, std::bind(&rule_hedge::check_hedge, this), true);
}

rule_hedge::~rule_hedge()
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_hedge::%s\n", __FUNCTION__);
    callers_->get_timer_caller()->unregister_timer(error_timer_id_);
}

void rule_hedge::add_trade(int64_t trade_id)
{
    hedges_.insert(std::make_pair(trade_id, 0));
    timers_.insert(std::make_pair(trade_id, get_current_milli_seconds() + 24 * 3600000));
    check_hedge();
}

void rule_hedge::check_hedge()
{
    for(TRADES::iterator itor = hedges_.begin(); itor != hedges_.end();) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_hedge::%s, trade_id[%ld] order_id[%ld]\n", __FUNCTION__, itor->first, itor->second);
        order_msg_type *order = data_->get_msg_by_id<cffex::fb::order_stream>(itor->second);
        if( itor->second == 0 ) {
            do_hedge(itor->first);
            ++itor;
        }
        else if(order == NULL) {
            ++itor;
        }
        else if(order->get_order_status() == cffex::fb::STRATEGY_ORDER_STATUS_ALL_TRADED) {
            callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_hedge::%s, erase, trade_id[%ld] order_id[%ld]\n", __FUNCTION__, itor->first, itor->second);
            timers_.erase(itor->first);
            hedges_.erase(itor++);
        }
        else if(order->get_order_status() == cffex::fb::STRATEGY_ORDER_STATUS_ERROR) {
            callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_hedge::%s, status error, trade_id[%ld] order_id[%ld], timers_size[%d]\n", __FUNCTION__, itor->first, itor->second, timers_.size());
            do_hedge(itor->first);
            ++itor;
        }
        else if(order->get_order_status() == cffex::fb::STRATEGY_ORDER_STATUS_CANCEL) {
            callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_hedge::%s, canceled, trade_id[%ld] order_id[%ld]\n", __FUNCTION__, itor->first, itor->second);
            if (cancel_hedges_.find(itor->second) != cancel_hedges_.end()) {
                rehedges_.insert(std::make_pair(itor->second, 0));
                cancel_hedges_.erase(itor->second);
            }
            timers_.erase(itor->first);
            hedges_.erase(itor++);
        } else {
            ++itor;
        }
    }
    for(TRADES::iterator itor = rehedges_.begin(); itor != rehedges_.end(); ) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_hedge::%s, hedge_order_id[%ld] rehedge_order_id[%ld]\n", __FUNCTION__, itor->first, itor->second);
        order_msg_type *order = data_->get_msg_by_id<cffex::fb::order_stream>(itor->second);
        if( itor->second == 0 ) {
            do_rehedge(itor->first);
            ++itor;
        }
        else if(order == NULL) {
            ++itor;
        }
        else if(order->get_order_status() == cffex::fb::STRATEGY_ORDER_STATUS_ERROR) {
            do_rehedge(itor->first);
            ++itor;
        }
        else if(order->get_order_status() != cffex::fb::STRATEGY_ORDER_STATUS_WAITING && order->get_order_status() != cffex::fb::STRATEGY_ORDER_STATUS_NONE) {
            callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_hedge::%s, finish, hedge_order_id[%ld] rehedge_order_id[%ld]\n", __FUNCTION__, itor->first, itor->second);
            rehedges_.erase(itor++);
        }
        else {
            ++itor;
        }
    }
}

void rule_hedge::check_hedge_cutloss_threshold(md_msg_type *msg) {
    for(TRADES::iterator itor = hedges_.begin(); itor != hedges_.end();) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_hedge::%s, trade_id[%ld] order_id[%ld]\n", __FUNCTION__, itor->first, itor->second);
        order_msg_type *order = data_->get_msg_by_id<cffex::fb::order_stream>(itor->second);
        if( itor->second == 0 ) {
            do_hedge(itor->first);
            ++itor;
        }
        else if(order == NULL) {
            ++itor;
        } else if ((order->get_direction() == cffex::fb::STRATEGY_DIRECTION_BUY && fabs(order->get_price() - msg->get_ask1_price()) > data_->hedge_cutloss_threshold * data_->tick && math_helper::valid_price(msg->get_ask1_price()))
         || (order->get_direction() == cffex::fb::STRATEGY_DIRECTION_SELL && fabs(order->get_price() - msg->get_bid1_price()) > data_->hedge_cutloss_threshold * data_->tick && math_helper::valid_price(msg->get_bid1_price())) ) {
            order_helper::cancel_order(callers_->get_order_caller(), itor->second);
            cancel_hedges_.insert(itor->second);
            ++itor;
        } else {
            ++itor;
        }
    }
}

void rule_hedge::do_hedge(int64_t trade_id)
{
    trade_msg_type *trade = data_->get_msg_by_id<cffex::fb::trade_stream>(trade_id);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_hedge::%s, trade_id[%ld] order_id[%ld]\n", __FUNCTION__, trade_id, hedges_.at(trade_id));
    int8_t direction = trade->get_direction() == cffex::fb::STRATEGY_DIRECTION_BUY ? cffex::fb::STRATEGY_DIRECTION_SELL : cffex::fb::STRATEGY_DIRECTION_BUY;
    double price = get_hedge_price(trade_id);
    if(!check_self_trade(price, direction, data_->instrument_id)) {
        return;
    }
    timers_.at(trade_id) = get_current_milli_seconds();
    int8_t offset = get_offset();
    order_helper::insert_order(callers_->get_order_caller(), data_->portfolio_name, data_->instrument_id, direction, offset, price, trade->get_volume(), hedges_.at(trade_id), STRATEGY_NAME);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_hedge::%s, trade_id[%ld] order_id[%ld]\n", __FUNCTION__, trade_id, hedges_.at(trade_id));
}

void rule_hedge::do_rehedge(int64_t hedge_order_id)
{
    order_msg_type *order = data_->get_msg_by_id<cffex::fb::order_stream>(hedge_order_id);
    int8_t offset = get_offset();
    double rehedge_price = get_rehedge_price(order->get_direction(), order->get_price());
    int volume = order->get_volume() - order->get_traded_volume();
    int direction = order->get_direction();
    if(!check_self_trade(rehedge_price, direction, data_->instrument_id)) {
        return;
    }
    order_helper::insert_order(callers_->get_order_caller(), data_->portfolio_name, data_->instrument_id, direction, offset,
        rehedge_price, volume, rehedges_.at(hedge_order_id), STRATEGY_NAME);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_hedge::%s, hedge_order_id[%ld] rehedge_order_id[%ld], rehedge_price[%lf] volume[%d], direction[%d]\n", __FUNCTION__,
        hedge_order_id, rehedges_.at(hedge_order_id), rehedge_price, volume, direction);
}

double rule_hedge::get_hedge_price(int64_t trade_id)
{
    trade_msg_type *trade = data_->get_msg_by_id<cffex::fb::trade_stream>(trade_id);
    double price = trade->get_price();
    price = trade->get_direction() == cffex::fb::STRATEGY_DIRECTION_BUY ? (price + data_->hedge_sell_shift * data_->tick) : (price + data_->hedge_buy_shift * data_->tick);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_hedge::%s, price[%lf]\n", __FUNCTION__, price);
    return price;
}

double rule_hedge::get_rehedge_price(int8_t direction, double hedge_price)
{
    double hedge_shift = (data_->hedge_cutloss_threshold + data_->rehedge_shift) * data_->tick;
    return hedge_price + hedge_shift * (direction == cffex::fb::STRATEGY_DIRECTION_BUY ? 1 : -1);
}

int8_t rule_hedge::get_offset()
{
    return cffex::fb::STRATEGY_OFFSET_FLAG_AUTO;
}

}
}