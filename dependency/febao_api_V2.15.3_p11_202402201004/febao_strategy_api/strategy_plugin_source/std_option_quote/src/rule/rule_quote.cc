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

#include "rule_quote.h"
#include "math_helper.h"
#include "quote_helper.h"
#include <limits.h>

namespace cffex {
namespace strategy {

/* critical interface */
void rule_quote::insert_quote(const char *instrument_id, bool force)
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_quote::%s, instrument[%s], begin\n", __FUNCTION__, instrument_id);

    if (!check_quote_swicth(instrument_id)) {
        return;
    }

    if(!check_trading_section()) {
        return;
    }

    if (!data_->instrument_table_->check_recancel_times(instrument_id)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_quote::%s, instrument[%s] check_recancel_times failed\n", __FUNCTION__, instrument_id);
        return;
    }

    if (!create_quote(instrument_id)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_quote::%s, instrument[%s] create quote failed\n", __FUNCTION__, instrument_id);
        cancel_quote(instrument_id);
        return;
    }

    if(!check_market(instrument_id)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_quote::%s, instrument[%s] check market failed\n", __FUNCTION__, instrument_id);
        cancel_quote(instrument_id);
        return;
    }

    if(!check_risk(instrument_id)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_quote::%s, instrument[%s] check risk failed\n", __FUNCTION__, instrument_id);
        cancel_quote(instrument_id);
        return;
    }

    if(!force && !check_self(instrument_id)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_quote::%s, instrument[%s] check self failed\n", __FUNCTION__, instrument_id);
        return;
    }

    if (!data_->replace_quote) {
        // if in none_replace_quote_mode, for example CFFEX, cancel quote first
        cancel_quote(instrument_id);
    }
    send_quote(instrument_id);
}

void rule_quote::insert_quote()
{
    for(record *r = data_->instrument_table_->first(); r != NULL; r = data_->instrument_table_->next()) {
        insert_quote(r->instrument_id);
    }
}

bool rule_quote::check_trading_section() {
    return data_->is_in_trading_section();
}

bool rule_quote::check_quote_swicth(const char *instrument_id) {
    if (!data_->get_quote_switch(instrument_id)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_quote::%s instrument[%s] quote_switch is close\n", __FUNCTION__, instrument_id);
        return false;
    }
    return true;
}

bool rule_quote::create_quote(const char *instrument_id)
{
    double base_price = data_->get_base_price(instrument_id);
    if(!math_helper::active_price(base_price)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_quote::%s instrument[%s] base_price [%lf] inactive\n", __FUNCTION__, instrument_id, base_price);
        return false;
    }
    double spread = data_->get_quote_spread(instrument_id, base_price);  /* spread is absolute value rather than tick value */
    if (!math_helper::active_price(spread)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_quote::%s instrument[%s] get_trader_spread invalid, price[%.2f], check spread_template\n", __FUNCTION__, instrument_id, base_price);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "instrument[%s] get_trader_spread invalid, price[%.2f], check spread_template", instrument_id, base_price);
        return false;
    }
    double bid_offset = data_->get_bid_offset(instrument_id);
    if (!math_helper::valid_price(bid_offset)) {
        bid_offset = 0.0;
    }
    double ask_offset = data_->get_ask_offset(instrument_id);
    if (!math_helper::valid_price(ask_offset)) {
        ask_offset = 0.0;
    }
    /* generate quote, bid flooring round , ask ceiling round */
    double tick = data_->get_tick(instrument_id);
    quote_bid_ = bid_offset + math_helper::round(base_price - spread / 2.0, tick, math_helper::ROUNDING_MODEL_FLOOR);
    quote_ask_ = ask_offset + math_helper::round(base_price + spread / 2.0, tick, math_helper::ROUNDING_MODEL_CEIL);
    return true;
}

bool rule_quote::check_market(const char *instrument_id) {
    /* check band price */
    double market_upper_band = data_->instrument_table_->get_record(instrument_id)->upper_band_;
    double market_down_band = data_->instrument_table_->get_record(instrument_id)->down_band_;

    if (!math_helper::active_price(market_upper_band) || !math_helper::active_price(market_down_band)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_quote::%s instrument[%s] market_band inactive\n", __FUNCTION__, instrument_id);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "instrument[%s] market_upper_band[%.2f] market_down_band[%.2f] is null", instrument_id, market_upper_band, market_down_band);
        return false;
    }
    if (math_helper::greater(quote_ask_, market_upper_band) || math_helper::less(quote_bid_, market_down_band)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_quote::%s instrument[%s] ask[%.2f] market_upper_band[%.2f] bid[%.2f] market_down_band[%.2f]\n", __FUNCTION__,
            instrument_id, quote_ask_, market_upper_band, quote_bid_, market_down_band);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "rule_quote::%s instrument[%s] ask[%.2f] market_upper_band[%.2f] bid[%.2f] market_down_band[%.2f]", instrument_id, quote_ask_, market_upper_band, quote_bid_, market_down_band);
        return false;
    }

    /* prevent immediate deal risk */
    double market_ask = data_->get_market_ask(instrument_id);
    double market_bid = data_->get_market_bid(instrument_id);
    if (!math_helper::valid_price(market_ask) || !math_helper::valid_price(market_bid)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_auto_quote::%s instrument[%s] market ask or bid invalid\n", __FUNCTION__, instrument_id);
        return false;
    }
    if (math_helper::greater_equal(quote_bid_, market_ask)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_auto_quote::%s instrument[%s] bid[%.2f] greater than market_ask[%.2f]\n", __FUNCTION__, instrument_id, quote_bid_, market_ask);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "instrument[%s] bid[%.2f] greater than market_ask[%.2f]", instrument_id, quote_bid_, market_ask);
        return false;
    }
    if (math_helper::less_equal(quote_ask_, market_bid)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_auto_quote::%s instrument[%s] ask[%.2f] less than market_bid[%.2f]\n", __FUNCTION__, instrument_id, quote_ask_, market_bid);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "instrument[%s] ask[%.2f] less than market_bid[%.2f]", instrument_id, quote_ask_, market_bid);
        return false;
    }

    return true;
}

bool rule_quote::check_risk(const char *instrument_id) {
    int netpos_limit = data_->get_netpos_limit(instrument_id);
    if (netpos_limit == -1) {
        // -1 is magic number, means don't check netpos
        return true;
    }
    if (netpos_limit <= 0) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_auto_quote::%s instrument[%s] netpos_limit[%d] invalid\n", __FUNCTION__, instrument_id, netpos_limit);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "instrument[%s] netpos_limit[%d] invalid", instrument_id, netpos_limit);
        return false;
    }

    int cur_netpos = abs(data_->get_current_netpos(instrument_id));
    if (cur_netpos == INT_MAX) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_auto_quote::%s instrument[%s] cur_netpos[%d] invalid\n", __FUNCTION__, instrument_id, cur_netpos);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "instrument[%s] cur_netpos[%d] invalid", instrument_id, cur_netpos);
        return false;
    }
    if (cur_netpos + data_->volume > netpos_limit || data_->volume == 0) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_auto_quote::%s instrument[%s] cur_netpos[%d] plus quote_volume[%d] exceed netpos_limit[%d]\n",
            __FUNCTION__, instrument_id, cur_netpos, data_->volume, netpos_limit);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "instrument[%s] cur_netpos[%d] plus quote_volume[%d] exceed netpos_limit[%d]", instrument_id, cur_netpos, data_->volume, netpos_limit);
        return false;
    }
    return true;
}

bool rule_quote::check_self(const char *instrument_id) {
    // compare quote with existed quote
    double cur_bid = DBL_MAX;
    double cur_ask = DBL_MAX;
    data_->get_self(instrument_id, &cur_bid, &cur_ask);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_quote::%s, cur_bid[%lf], cur_ask[%lf]\n", __FUNCTION__, cur_bid, cur_ask);

    if (!math_helper::valid_price(cur_bid) || !math_helper::valid_price(cur_ask)) {
        // if not existed any quote, return true and send_quote immediately
        // if single side quote, may happen when manual cancel order
        return true;
    }

    double threshold = data_->get_update_threshold(instrument_id);
    if (!math_helper::active_price(threshold)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_quote::%s instrument[%s] update_threshold is null\n", __FUNCTION__, instrument_id);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "instrument[%s] update_threshold is null", instrument_id);
        return false;
    }
    if (math_helper::less_equal(fabs(quote_bid_ - cur_bid), threshold) && math_helper::less_equal(fabs(quote_ask_ - cur_ask), threshold)) {
        /* not exceed threshold, don't re-quote */
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_quote::%s instrument[%s] new_bid[%.2f] new_ask[%.2f], old_bid[%.2f] old_ask[%.2f], not reach update_threshold[%.2f], don't re-quote \n",
            __FUNCTION__, instrument_id, quote_bid_, quote_ask_, cur_bid, cur_ask, threshold);
        return false;
    }
    return true;

}

void rule_quote::send_quote(const char *instrument_id) {
    cffex::fb::i_quote_caller::quote_entity *en = callers_->get_quote_caller()->create_quote_entity();
    en->set_portfolio_name(data_->portfolio);
    en->set_instrument_id(instrument_id);
    en->set_bid_offset_flag(cffex::fb::STRATEGY_OFFSET_FLAG_AUTO);
    en->set_ask_offset_flag(cffex::fb::STRATEGY_OFFSET_FLAG_AUTO);
    en->set_bid_hedge_flag(cffex::fb::STRATEGY_HEDGE_FLAG_MARKET_MAKER);
    en->set_ask_hedge_flag(cffex::fb::STRATEGY_HEDGE_FLAG_MARKET_MAKER);
    en->set_volume_condition(cffex::fb::STRATEGY_VOLUME_ANY);
    en->set_time_condition(cffex::fb::STRATEGY_TIME_CONDITION_GFD);
    en->set_bid_price(quote_bid_);
    en->set_ask_price(quote_ask_);
    en->set_bid_volume(data_->volume);
    en->set_ask_volume(data_->volume);
    en->set_custom_flag("op_quote");

    int64_t quote_id;
    callers_->get_quote_caller()->insert_quote(en, quote_id);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "send_quote success, instrument[%s], bid[%.2f], ask[%.2f], volume[%d], quote_id[%ld]\n",
        instrument_id, quote_bid_, quote_ask_, data_->volume, quote_id);
    data_->instrument_table_->get_record(instrument_id)->quote_id_ = quote_id;
    unfinish_quotes_.insert(quote_id);
}

void rule_quote::cancel_quote(const char *instrument_id) {
    int cnt = 0;
    for(std::set<int64_t>::iterator itor = unfinish_quotes_.begin(); itor != unfinish_quotes_.end(); ++itor) {
        quote_msg_type *quote_msg = data_->get_msg_by_id<cffex::fb::quote_stream>(*itor);
        if(quote_msg==NULL){
            continue;
        }
        if(!quote_helper::is_final_status(quote_msg) && strcmp(quote_msg->get_instrument_id(), instrument_id) == 0)
        {
            quote_helper::cancel_quote(callers_->get_quote_caller(),*itor);
            cnt++;
        }
    }
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "rule_quote::%s, cancel quote by instrument[%s], number:%d\n", __FUNCTION__, instrument_id, cnt);
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO,  "rule_quote::%s, cancel quote by instrument[%s], number:%d\n", __FUNCTION__, instrument_id, cnt);
}

void rule_quote::cancel_quote(const char *instrument_id, int64_t quote_id) {
    cffex::fb::i_quote_caller::cancel_quote_entity *en = callers_->get_quote_caller()->create_cancel_entity();
    en->set_instrument_id(instrument_id);
    en->set_quote_id(quote_id);

    callers_->get_quote_caller()->cancel_quote(en);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "rule_quote::%s, cancel instrument[%s], quote_id[%ld]\n", __FUNCTION__, instrument_id, quote_id);
}

void rule_quote::cancel_quote() {
    int cnt = 0;
    for(std::set<int64_t>::iterator itor = unfinish_quotes_.begin(); itor != unfinish_quotes_.end(); ++itor) {
        quote_msg_type *quote_msg = data_->get_msg_by_id<cffex::fb::quote_stream>(*itor);
        if(quote_msg==NULL){
            continue;
        }
        if(!quote_helper::is_final_status(quote_msg))
        {
            quote_helper::cancel_quote(callers_->get_quote_caller(),*itor);
            cnt++;
        }
    }
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "rule_quote::%s, cancel quote number:%d\n", __FUNCTION__, cnt);
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "cancel quote number:%d", cnt);
}

void rule_quote::on_trade_pending_timer(std::string instrument_id) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_quote::%s, instrument[%s] \n", __FUNCTION__, instrument_id.c_str());
    turn_on_quote_switch(instrument_id.c_str());
}



void rule_quote::turn_off_quote_switch(const char* instrument) {
    callers_->get_instrument_param_caller()->set_param(instrument, INSTRUMENT_QUOTE_SWITCH, false);
}

void rule_quote::turn_on_quote_switch(const char* instrument) {
    callers_->get_instrument_param_caller()->set_param(instrument, INSTRUMENT_QUOTE_SWITCH, true);
}



void rule_quote::on_recancel_timer(const char *instrument, int64_t quote_id) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_quote::%s, instrument[%s] quote_id[%ld] \n", __FUNCTION__, instrument, quote_id);
    cancel_quote(instrument, quote_id);
}

void rule_quote::on_timer() {
    if (quoting_period_ == INVALID_QUOTING_PERIOD) {
        // prevent on_start trigger
        quoting_period_ = data_->is_in_trading_section() ? IN_QUOTING_PERIOD : OUT_QUOTING_PERIOD;
        return;
    }

    if (quoting_period_ == OUT_QUOTING_PERIOD && data_->is_in_trading_section()) {
        // go from no-quoting period to quoting period, trigger active quoting
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "rule_quote::%s, go from none-trading-status to trading, start quoting\n", __FUNCTION__);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "go from none-trading-status to trading, start quoting");
        insert_quote();
        quoting_period_ = IN_QUOTING_PERIOD;
    }
    else if (quoting_period_ == IN_QUOTING_PERIOD && !data_->is_in_trading_section()) {
        // cancel quote when time pass over trading section
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "rule_quote::%s, go from trading-status to none-trading, stop quoting\n", __FUNCTION__);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "go from trading-status to none-trading, stop quoting");
        cancel_quote();
        quoting_period_ = OUT_QUOTING_PERIOD;
    }
}

void rule_quote::on_msg_trade(trade_msg_type *msg)
{
    //trade means not satisfy obligations, cancel immediately
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_quote::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
    cancel_quote(msg->get_instrument_id());
    turn_off_quote_switch(msg->get_instrument_id());

    // sleep several time until next active quote
    std::string instrument_id = msg->get_instrument_id();
    callers_->get_timer_caller()->register_timer(data_->trade_pending_second * 1000, std::bind(&rule_quote::on_trade_pending_timer, this, instrument_id), false);
}

void rule_quote::on_msg_quote(quote_msg_type *msg)
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_quote::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
    if(msg->get_quote_status() == cffex::fb::STRATEGY_QUOTE_STATUS_ERROR) {
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "quote[%s] error[%d]", msg->get_instrument_id(), msg->get_error_id());
        if (msg->get_error_id() != cffex::fb::STRATEGY_ERROR_RISK_FLOW_LIMIT && msg->get_error_id() != cffex::fb::STRATEGY_ERROR_CANCEL_COUNTER_TRANSIT_FLOW_LIMIT) {
            turn_off_quote_switch(msg->get_instrument_id());
        }
    }
    if(quote_helper::is_final_status(msg)){
        int quote_id = msg->get_quote_id();
        unfinish_quotes_.erase(quote_id);
    }
}

void rule_quote::on_msg_quote_cancel(quote_cancel_msg_type *msg)
{
    if (msg->get_error_id() == cffex::fb::STRATEGY_ERROR_NONE || msg->get_error_id() == cffex::fb::STRATEGY_ERROR_CANCEL_ALL_TRADE ||
        msg->get_error_id() == cffex::fb::STRATEGY_ERROR_NO_CANCEL_ORDER_RIGHT || msg->get_error_id() == cffex::fb::STRATEGY_ERROR_CANCEL_INSUITABLE_ORDER_STATUS) {
        data_->instrument_table_->clear_recancel_times(msg->get_instrument_id());
        return;
    }
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_quote::%s, instrument[%s] cancel[%ld] failed, reason[%d]\n", __FUNCTION__, msg->get_instrument_id(), msg->get_quote_id(), msg->get_error_id());
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "instrument[%s] cancel[%ld] failed, reason[%d], recancel later", msg->get_instrument_id(), msg->get_quote_id(), msg->get_error_id());

    data_->instrument_table_->record_recancel_times(msg->get_instrument_id());
    const char *instrument = data_->instrument_table_->get_record(msg->get_instrument_id())->instrument_id;
    if (!data_->instrument_table_->check_recancel_times(instrument)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_quote::%s, instrument[%s] check_recancel_times failed\n", __FUNCTION__, instrument);
        return;
    }
    // re-cancel after certain interval
    callers_->get_timer_caller()->register_timer(RECANCEL_INTERVAL, std::bind(&rule_quote::on_recancel_timer, this, instrument, msg->get_quote_id()), false);
}

}
}