 /**
 * CFFEX Confidential.
 *
 * @Copyright 2019 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: wangty
 * Date: 2019-11-20
 */

#include "rule_option_quiry.h"
#include "math_helper.h"
#include <limits.h>

namespace cffex {
namespace strategy {

instrument_record *rule_option_quiry::get_record(const char *instrument) {
    record_mapping::iterator itr =  records_.find(instrument);
    if (itr == records_.end()) {
        instrument_record *r = new instrument_record(instrument);
        records_[r->get_instrument_id()] = r;
        return r;
    } else {
        return itr->second;
    }
}

void rule_option_quiry::clear_recancel_times(const char* instrument) {
    instrument_record *r = get_record(instrument);
    r->clear_recancel_times();
}

bool rule_option_quiry::check_recancel_times(const char* instrument) {
    instrument_record *r = get_record(instrument);
    return r->check_recancel_times();
}

void rule_option_quiry::record_recancel_times(const char* instrument) {
    instrument_record *r = get_record(instrument);
    r->record_recancel_times();
}

bool rule_option_quiry::update_quiry_id(const char* instrument, const char* quiry_id) {
    instrument_record *r = get_record(instrument);
    return r->update_quiry_id(quiry_id);
}

void rule_option_quiry::reset() {
    quote_bid_ = DBL_MAX;
    quote_ask_ = DBL_MAX;
    quote_volume_ = data_->get_volume();
    market_bid_ = DBL_MAX;
    market_ask_ = DBL_MAX;
	portfolio_ = data_->get_portfolio();
	netpos_limit_ = data_->get_netpos_limit();
    unfinish_quotes_.clear();
}

/* public interface */
void rule_option_quiry::insert_quote(const char *instrument_id, const char *quiry_id) {
    caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_option_quiry::%s, instrument[%s] quiry[%s]\n", __FUNCTION__, instrument_id, quiry_id);

    if(!check_trading_section()) {
        caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_option_quiry::%s, not in trading_section\n", __FUNCTION__, instrument_id);
        return;
    }

    if(!check_market(instrument_id)) {
        caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_option_quiry::%s, instrument[%s] check market failed\n", __FUNCTION__, instrument_id);
        return;
    }

    if (!create_quote(instrument_id)) {
        caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_option_quiry::%s, instrument[%s] create quote failed\n", __FUNCTION__, instrument_id);
        return;
    }

    if(!check_risk(instrument_id)) {
        caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_option_quiry::%s, instrument[%s] check risk failed\n", __FUNCTION__, instrument_id);
        return;
    }

    send_quote(instrument_id, quiry_id);
}

void rule_option_quiry::cancel_quote(const char *instrument_id) {
    int cnt = 0;
    for(std::set<int64_t>::iterator itor = unfinish_quotes_.begin(); itor != unfinish_quotes_.end(); ++itor) {
        quote_msg_type *quote_msg = data_->get_msg_by_id<cffex::fb::quote_stream>(*itor);
        if(quote_msg==NULL){
            continue;
        }
        if(!quote_helper::is_final_status(quote_msg) && strcmp(quote_msg->get_instrument_id(), instrument_id) == 0)
        {
            quote_helper::cancel_quote(caller_->get_quote_caller(),*itor);
            cnt++;
        }
    }
    caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "rule_option_quiry::%s, cancel quote, instrument[%s]\n", __FUNCTION__, instrument_id);
}

void rule_option_quiry::cancel_quote(const char* instrument_id, int64_t quote_id) {
    caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "rule_option_quiry::%s, cancel quote, instrument_id[%s] quote_id[%ld]\n", __FUNCTION__, instrument_id, quote_id);

    cffex::fb::i_quote_caller::cancel_quote_entity *en = caller_->get_quote_caller()->create_cancel_entity();
    en->set_quote_id(quote_id);
    caller_->get_quote_caller()->cancel_quote(en);
}

void rule_option_quiry::cancel_quote() {
    int cnt = 0;
    for(std::set<int64_t>::iterator itor = unfinish_quotes_.begin(); itor != unfinish_quotes_.end(); ++itor) {
        quote_msg_type *quote_msg = data_->get_msg_by_id<cffex::fb::quote_stream>(*itor);
        if(quote_msg==NULL){
            continue;
        }
        if(!quote_helper::is_final_status(quote_msg))
        {
            quote_helper::cancel_quote(caller_->get_quote_caller(),*itor);
            cnt++;
        }
    }
    caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "rule_quote::%s, cancel quote number:%d\n", __FUNCTION__, cnt);
    caller_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "cancel quote number:%d", cnt);
}

/* private functions */
bool rule_option_quiry::check_trading_section() {
    if (!data_->is_in_trading_section()) {
        caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_option_quiry::%s current time not in trading_sections\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool rule_option_quiry::check_market(const char *instrument_id) {
    market_bid_ =  data_->get_market_bid(instrument_id);
    market_ask_ =  data_->get_market_ask(instrument_id);
    if (!math_helper::valid_price(market_bid_) || !math_helper::valid_price(market_ask_)) {
        caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_option_quiry::%s instrument[%s] market bid or ask is null \n", __FUNCTION__, instrument_id);
        caller_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "instrument[%s] market bid or ask is null", instrument_id);
        return false;
    }

    //检查市场价差与规定报价价差
	double exchange_spread = data_->get_quiry_spread(instrument_id, market_bid_);
	double market_spread = market_ask_ - market_bid_;
	if (math_helper::greater(market_spread, exchange_spread)) {
		// don't response to quiry when market spread is larger
		caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_option_quiry::%s instrument[%s] market spread[%.2f] larger than exchange[%.2f] required\n", __FUNCTION__, instrument_id, market_spread, exchange_spread);
        	caller_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "instrument[%s] market spread[%.2f] larger than exchange[%.2f] required", instrument_id, market_spread, exchange_spread);
		return false;
	}

	return true;
}

bool rule_option_quiry::create_quote(const char *instrument_id) {
	//follow the market
	quote_bid_ = market_bid_;
	quote_ask_ = market_ask_;
    return true;
}

bool rule_option_quiry::check_risk(const char *instrument_id) {
    int netpos_limit = data_->get_netpos_limit();
    if (netpos_limit == -1) {
		// -1 is magic number, means don't check netpos
        caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "rule_option_quiry::%s, don't check netpos, netpos_limit[%d]\n", __FUNCTION__, netpos_limit);
		return true;
	}
    if (netpos_limit <= 0) {
        caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_option_quiry::%s instrument[%s] netpos_limit[%d] invalid\n", __FUNCTION__, instrument_id, netpos_limit);
        caller_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "instrument[%s] netpos_limit[%d] invalid", instrument_id, netpos_limit);
        return false;
    }

    int cur_netpos = abs(data_->get_current_netpos(instrument_id));
    if (cur_netpos == INT_MAX) {
        caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_option_quiry::%s instrument[%s] cur_netpos[%d] invalid\n", __FUNCTION__, instrument_id, cur_netpos);
        caller_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "instrument[%s] cur_netpos[%d] invalid", instrument_id, cur_netpos);
        return false;
    }
    if (quote_volume_ <= 0) {
        caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_option_quiry::%s instrument[%s] quote_volume[%d] invalid\n", __FUNCTION__, instrument_id, quote_volume_);
        caller_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "instrument[%s] quote_volume[%d] invalid", instrument_id, quote_volume_);
        return false;
    }
    if (cur_netpos + quote_volume_ > netpos_limit) {
        caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "rule_option_quiry::%s instrument[%s] cur_netpos[%d] plus quote_volume[%d] exceed netpos_limit[%d]\n",
            __FUNCTION__, instrument_id, cur_netpos, quote_volume_, netpos_limit);
        caller_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING,
            "instrument[%s] cur_netpos[%d] plus quote_volume[%d] exceed netpos_limit[%d]", instrument_id, cur_netpos, quote_volume_, netpos_limit);
        return false;
    }
    return true;
}


void rule_option_quiry::send_quote(const char *instrument_id, const char *quiry_id ) {
    cffex::fb::i_quote_caller::quote_entity *en = caller_->get_quote_caller()->create_quote_entity();

	// if porfolio is not set, use portfolio defaultrule
    if (portfolio_ > 0) { en->set_portfolio_id(portfolio_); }
    en->set_instrument_id(instrument_id);
    en->set_inquiry_id(quiry_id);
    en->set_bid_offset_flag(cffex::fb::STRATEGY_OFFSET_FLAG_AUTO);
    en->set_ask_offset_flag(cffex::fb::STRATEGY_OFFSET_FLAG_AUTO);
    en->set_bid_hedge_flag(cffex::fb::STRATEGY_HEDGE_FLAG_MARKET_MAKER);
    en->set_ask_hedge_flag(cffex::fb::STRATEGY_HEDGE_FLAG_MARKET_MAKER);
    en->set_volume_condition(cffex::fb::STRATEGY_VOLUME_ANY);
    en->set_time_condition(cffex::fb::STRATEGY_TIME_CONDITION_GFD);
    en->set_bid_price(quote_bid_);
    en->set_ask_price(quote_ask_);
    en->set_bid_volume(quote_volume_);
    en->set_ask_volume(quote_volume_);
    en->set_custom_flag("op_quiry");

    int64_t quote_id;
    caller_->get_quote_caller()->insert_quote(en, quote_id);

    caller_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "send_quote success, instrument_id[%s],  bid[%.2f], ask[%.2f], volume[%d], quote_id[%ld], quiry_id[%s]\n",
        instrument_id, quote_bid_, quote_ask_, quote_volume_, quote_id, quiry_id);
    unfinish_quotes_.insert(quote_id);
}


}
}
