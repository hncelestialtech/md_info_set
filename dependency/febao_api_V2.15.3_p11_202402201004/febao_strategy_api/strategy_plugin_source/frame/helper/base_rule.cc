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
 * Date: 2020-03-09
 */

#include "base_rule.h"
#include "order_helper.h"
#include "quote_helper.h"


namespace cffex {
namespace strategy {

void base_rule::on_order(order_msg_type *msg)
{
    ORDER_MAP::iterator itor = order_map_.find(msg->get_order_id());
    if(order_helper::is_final(msg, msg->get_order_id())) {
        if (itor != order_map_.end()) order_map_.erase(itor);
        return ;
    }
    if (itor == order_map_.end()) {
        std::pair<ORDER_MAP::iterator, bool> res = order_map_.insert(std::make_pair(msg->get_order_id(), strategy_order_info{}));
        itor = res.first;
	    STRATEGY_LOG(callers_->get_log_caller(), cffex::fb::i_log_caller::ILOG_DEBUG, "base_rule::%s,insert order msg[%s]\n",__FUNCTION__, msg->to_string().c_str());
    }
    itor->second.set_value(msg);
}

void base_rule::on_quote(quote_msg_type *msg)
{
    QUOTE_MAP::iterator itor = quote_map_.find(msg->get_quote_id());
    if(quote_helper::is_final(msg, msg->get_quote_id())) {
        if (itor != quote_map_.end()) quote_map_.erase(itor);
        return ;
    }
    if (itor == quote_map_.end()) {
        std::pair<QUOTE_MAP::iterator, bool> res = quote_map_.insert(std::make_pair(msg->get_quote_id(), strategy_quote_info{}));
        itor = res.first;
        STRATEGY_LOG(callers_->get_log_caller(), cffex::fb::i_log_caller::ILOG_INFO, "base_rule::%s, insert quote msg[%s]\n", __FUNCTION__, msg->to_string().c_str());
    }
    itor->second.set_value(msg);
}

bool base_rule::check_self_trade(double price, int8_t direction, const char* instrument)
{
    for(ORDER_MAP::iterator itor = order_map_.begin(); itor != order_map_.end(); ++itor) {
        strategy_order_info &order_info = itor->second;
        if(direction == order_info.direction || (instrument!=NULL && strcmp(instrument,order_info.instrument_id) != 0)) {
            continue;
        }

        if( (direction == cffex::fb::STRATEGY_DIRECTION_BUY && math_helper::greater_equal(price, order_info.price)) ||
            (direction == cffex::fb::STRATEGY_DIRECTION_SELL && math_helper::less_equal(price, order_info.price)) ) {
            callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "base_rule::%s, false, price[%lf] direction[%d] order_id[%ld]\n", __FUNCTION__, price, direction, order_info.order_id);
            in_self_trade_ = true;
            return false;
        }
    }
    in_self_trade_ = false;
    return true;
}

void base_rule::check_self_trade_and_cancel(double price, int8_t direction, int64_t order_id, std::set<int64_t>* cancel_delay, const char* instrument)
{
    for(ORDER_MAP::iterator itor = order_map_.begin(); itor != order_map_.end(); ++itor) {
        strategy_order_info &order_info = itor->second;
        if(direction == order_info.direction || (instrument!=NULL && strcmp(instrument,order_info.instrument_id) != 0) || order_id == order_info.order_id) {
            continue;
        }

        if (cancel_delay->find(order_info.order_id) != cancel_delay->end()) continue;

        if( (direction == cffex::fb::STRATEGY_DIRECTION_BUY && math_helper::greater_equal(price, order_info.price)) ||
            (direction == cffex::fb::STRATEGY_DIRECTION_SELL && math_helper::less_equal(price, order_info.price)) ) {

            if(id_helper::is_order(order_info.order_id)){
                order_helper::cancel_order(callers_->get_order_caller(),order_info.order_id);
            }else if(id_helper::is_quote(order_info.order_id)){
                quote_helper::cancel_quote(callers_->get_quote_caller(), id_helper::conver_to_quote_id(order_info.order_id));
            }else{
                STRATEGY_LOG(callers_->get_log_caller(), cffex::fb::i_log_caller::ILOG_DEBUG, "base_rule::%s, order_id[%ld] is not febao order or quote\n", __FUNCTION__, order_info.order_id);
            }

            if(order_info.order_status == cffex::fb::STRATEGY_ORDER_STATUS_WAITING){
                cancel_delay->insert(order_info.order_id);
            }
            callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "base_rule::%s, price[%lf] direction[%d] will self trade,so cancel order_id[%ld]\n", __FUNCTION__, price, direction, order_info.order_id);
        }
    }
}
void base_rule::on_msg_order_cancel(order_cancel_msg_type *msg, int instance_id)
{
    ORDER_MAP::iterator itor = order_map_.find(msg->get_order_id());

    if(itor == order_map_.end() || itor->second.strategy_instance_id != instance_id) {
        return;
    }
    if(msg->get_error_id() == cffex::fb::STRATEGY_ERROR_NONE || msg->get_error_id() == cffex::fb::STRATEGY_ERROR_CANCEL_ALL_TRADE ||
        msg->get_error_id() == cffex::fb::STRATEGY_ERROR_NO_CANCEL_ORDER_RIGHT || msg->get_error_id() == cffex::fb::STRATEGY_ERROR_CANCEL_INSUITABLE_ORDER_STATUS) {
        if(cancel_map_.find(msg->get_order_id()) != cancel_map_.end()) {
            cancel_map_.erase(msg->get_order_id());
        }
        return;
    }
    if(cancel_map_.find(msg->get_order_id()) == cancel_map_.end()) {
        cancel_map_[msg->get_order_id()] = 0;
    }
    else {
        cancel_map_[msg->get_order_id()]++;
    }
    STRATEGY_LOG(callers_->get_log_caller(), cffex::fb::i_log_caller::ILOG_WARNING, "base_rule::%s, times[%d], msg[%s]\n", __FUNCTION__, cancel_map_[msg->get_order_id()], msg->to_string().c_str());
    if(cancel_map_[msg->get_order_id()] > 2) {
        STRATEGY_LOG(callers_->get_log_caller(), cffex::fb::i_log_caller::ILOG_WARNING, "base_rule::%s, times[%d] msg[%s]", __FUNCTION__, cancel_map_[msg->get_order_id()], msg->to_string().c_str());
        return;
    }
    order_helper::cancel_order(callers_->get_order_caller(), msg->get_order_id(), msg->get_instrument_id());
}


bool base_rule::on_msg_order_cancel(order_cancel_msg_type *msg, int instance_id, int cancel_time)
{
    ORDER_MAP::iterator itor = order_map_.find(msg->get_order_id());

    if(itor == order_map_.end() || itor->second.strategy_instance_id != instance_id) {
        return true;
    }
    if(msg->get_error_id() == cffex::fb::STRATEGY_ERROR_NONE || msg->get_error_id() == cffex::fb::STRATEGY_ERROR_CANCEL_ALL_TRADE ||
        msg->get_error_id() == cffex::fb::STRATEGY_ERROR_NO_CANCEL_ORDER_RIGHT || msg->get_error_id() == cffex::fb::STRATEGY_ERROR_CANCEL_INSUITABLE_ORDER_STATUS) {
        if(cancel_map_.find(msg->get_order_id()) != cancel_map_.end()) {
            cancel_map_.erase(msg->get_order_id());
        }
        return true;
    }
    if(cancel_map_.find(msg->get_order_id()) == cancel_map_.end()) {
        cancel_map_[msg->get_order_id()] = 0;
    }
    else {
        cancel_map_[msg->get_order_id()]++;
    }
    STRATEGY_LOG(callers_->get_log_caller(), cffex::fb::i_log_caller::ILOG_WARNING, "base_rule::%s, times[%d], msg[%s]\n", __FUNCTION__, cancel_map_[msg->get_order_id()], msg->to_string().c_str());
    if(cancel_map_[msg->get_order_id()] > cancel_time) {
        STRATEGY_LOG(callers_->get_log_caller(), cffex::fb::i_log_caller::ILOG_WARNING, "base_rule::%s, times[%d] msg[%s]", __FUNCTION__, cancel_map_[msg->get_order_id()], msg->to_string().c_str());
        return false;
    }
    order_helper::cancel_order(callers_->get_order_caller(), msg->get_order_id(), msg->get_instrument_id());
    return true;
}


void base_rule::on_msg_quote_cancel(quote_cancel_msg_type *msg, int instance_id) {
    QUOTE_MAP::iterator itor = quote_map_.find(msg->get_quote_id());

    if(itor == quote_map_.end() || itor->second.strategy_instance_id != instance_id) {
        return;
    }
    if (msg->get_error_id() == cffex::fb::STRATEGY_ERROR_NONE || msg->get_error_id() == cffex::fb::STRATEGY_ERROR_NO_CANCEL_ORDER_RIGHT ||
     msg->get_error_id() == cffex::fb::STRATEGY_ERROR_CANCEL_INSUITABLE_ORDER_STATUS) {
        if(cancel_map_.find(msg->get_quote_id()) != cancel_map_.end()) {
            cancel_map_.erase(msg->get_quote_id());
        }
        return;
    }
    if(cancel_map_.find(msg->get_quote_id()) == cancel_map_.end()) {
        cancel_map_[msg->get_quote_id()] = 0;
    }
    else {
        cancel_map_[msg->get_quote_id()]++;
    }
    STRATEGY_LOG(callers_->get_log_caller(), cffex::fb::i_log_caller::ILOG_WARNING, "base_rule::%s, times[%d], msg[%s]\n", __FUNCTION__, cancel_map_[msg->get_quote_id()], msg->to_string().c_str());
    if(cancel_map_[msg->get_quote_id()] > 2) {
        STRATEGY_LOG(callers_->get_log_caller(), cffex::fb::i_log_caller::ILOG_WARNING, "base_rule::%s, times[%d] msg[%s]", __FUNCTION__, cancel_map_[msg->get_quote_id()], msg->to_string().c_str());
        return;
    }
    quote_helper::cancel_quote(callers_->get_quote_caller(), msg->get_quote_id(), msg->get_instrument_id());
}

bool base_rule::on_msg_quote_cancel(quote_cancel_msg_type *msg, int instance_id, int cancel_time) {
    QUOTE_MAP::iterator itor = quote_map_.find(msg->get_quote_id());

    if(itor == quote_map_.end() || itor->second.strategy_instance_id != instance_id) {
        return true;
    }
    if (msg->get_error_id() == cffex::fb::STRATEGY_ERROR_NONE || msg->get_error_id() == cffex::fb::STRATEGY_ERROR_NO_CANCEL_ORDER_RIGHT ||
        msg->get_error_id() == cffex::fb::STRATEGY_ERROR_CANCEL_INSUITABLE_ORDER_STATUS) {
        if(cancel_map_.find(msg->get_quote_id()) != cancel_map_.end()) {
            cancel_map_.erase(msg->get_quote_id());
        }
        return true;
    }
    if(cancel_map_.find(msg->get_quote_id()) == cancel_map_.end()) {
        cancel_map_[msg->get_quote_id()] = 0;
    }
    else {
        cancel_map_[msg->get_quote_id()]++;
    }
    STRATEGY_LOG(callers_->get_log_caller(), cffex::fb::i_log_caller::ILOG_WARNING, "base_rule::%s, times[%d], msg[%s]\n", __FUNCTION__, cancel_map_[msg->get_quote_id()], msg->to_string().c_str());
    if(cancel_map_[msg->get_quote_id()] > cancel_time) {
        STRATEGY_LOG(callers_->get_log_caller(), cffex::fb::i_log_caller::ILOG_WARNING, "base_rule::%s, times[%d] msg[%s]", __FUNCTION__, cancel_map_[msg->get_quote_id()], msg->to_string().c_str());
        return false;
    }
    quote_helper::cancel_quote(callers_->get_quote_caller(), msg->get_quote_id(), msg->get_instrument_id());
    return true;
}


void base_rule::cancel_instance_order(int instance_id)
{
    instance_id = instance_id == 0 ? instance_id_ : instance_id;
    for(ORDER_MAP::iterator itor = order_map_.begin(); itor != order_map_.end(); ++itor) {
        strategy_order_info &order_info = itor->second;
        if(id_helper::is_order(order_info.order_id) && order_info.strategy_instance_id == instance_id)
        {
            order_helper::cancel_order(callers_->get_order_caller(), order_info.order_id, order_info.instrument_id);
        }
    }
}

void base_rule::cancel_instance_quote(int instance_id)
{
    instance_id = instance_id == 0 ? instance_id_ : instance_id;
    STRATEGY_LOG(callers_->get_log_caller(), cffex::fb::i_log_caller::ILOG_DEBUG, "base_rule::%s,instance_id_[%d], instance_id[%d]\n",__FUNCTION__, instance_id_, instance_id);
    for(QUOTE_MAP::iterator itor = quote_map_.begin(); itor != quote_map_.end(); ++itor) {
        strategy_quote_info &quote_info = itor->second;
        if(quote_info.strategy_instance_id == instance_id)
        {
            quote_helper::cancel_quote(callers_->get_quote_caller(), quote_info.quote_id, quote_info.instrument_id);
        }
    }
}

void base_rule::cancel_instrument_order(const char* instrument)
{
    for(ORDER_MAP::iterator itor = order_map_.begin(); itor != order_map_.end(); ++itor) {
        strategy_order_info &order_info = itor->second;
        if(id_helper::is_order(order_info.order_id) && strcmp(order_info.instrument_id, instrument)==0)
        {
            order_helper::cancel_order(callers_->get_order_caller(), order_info.order_id, order_info.instrument_id);
        }
    }
}

void base_rule::cancel_instrument_quote(const char* instrument)
{
    for(QUOTE_MAP::iterator itor = quote_map_.begin(); itor != quote_map_.end(); ++itor) {
        strategy_quote_info &quote_info = itor->second;
        if(strcmp(quote_info.instrument_id, instrument)==0)
        {
            quote_helper::cancel_quote(callers_->get_quote_caller(), quote_info.quote_id, quote_info.instrument_id);
        }
    }
}

bool base_rule::exsit_order_in_book(){
    return !order_map_.empty();
}

}
}
