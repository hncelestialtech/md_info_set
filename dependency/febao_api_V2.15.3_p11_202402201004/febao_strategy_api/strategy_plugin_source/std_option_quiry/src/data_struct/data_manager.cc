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
 * Date: 2019-11-18
 */

#include "data_manager.h"
#include "math_helper.h"

namespace cffex {
namespace strategy {

data_manager::data_manager(cffex::fb::i_strategy *obj, caller_handler *callers)
: stream_factory(obj), callers_(callers) {
}
data_manager::~data_manager() {

}
bool data_manager::init_params()
{
    bool succ = true;
    succ &= get_instance_param("portfolio", portfolio_name_);
    succ &= get_instance_param("volume", &volume_);
    succ &= get_instance_param("product", product_);
    succ &= get_instance_param("trading_section", trading_section_);
    succ &= get_instance_param("waiting_second", &waiting_second_);
    succ &= get_instance_param("duration_second", &duration_second_);
    succ &= get_instance_param("netpos_limit", &netpos_limit_);
    portfolio_id_= get_portfolio_id(portfolio_name_);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "data_manager::%s, ret[%d]\n", __FUNCTION__, succ);
    return succ;
}

// following are instance params
int  data_manager::get_portfolio() {
    return portfolio_id_;
}

const char* data_manager::get_portfolio_name() {
    return portfolio_name_;
}

int  data_manager::get_volume() {
    return volume_;
}

const char* data_manager::get_trading_section() {
    return trading_section_;
}

const char* data_manager::get_product() {
	return product_;
}

bool data_manager::is_in_trading_section() {
	if ( 0 == strcmp(trading_section_,"")) {
		// empty param mean don't check time
		return true;
	}

    const cffex::fb::trading_time_template_stream::i_stream_msg* trading_time_msg = get_stream<cffex::fb::trading_time_template_stream>()->get_stream_table()->get_msg(trading_section_);
    if(trading_time_msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, can't get trading_section[%s]\n", __FUNCTION__, trading_section_);
        return false;
    }

    if (!trading_time_msg->is_trading_time('0')) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, current[%d] not in trading_section\n", __FUNCTION__, trading_time_msg->get_curr_trading_sec());
        return false;
	}
	return true;
}

double  data_manager::get_waiting_second() {
    return waiting_second_;
}

double  data_manager::get_duration_second() {
    return duration_second_;
}

int  data_manager::get_netpos_limit() {
    return netpos_limit_;
}
// instance params over

/******************************************************************************/

// following are instrument group params
double data_manager::get_quiry_spread(const char* instrument, double bid_price) {
    const cffex::fb::instrument_stream::i_stream_msg* instrument_msg = get_stream<cffex::fb::instrument_stream>()->get_stream_table()->get_msg(instrument);
    if (instrument_msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, no instrument[%s] found\n", __FUNCTION__, instrument);
        return DBL_MAX;
    }
    char template_name[256] = {0};
    if (! get_stream<cffex::fb::custom_param_stream>()->get_stream_table()->get_param(instrument_msg->get_option_serial_id(), PRODUCT_QUIRY_TEMPLATE, template_name)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, can't get quiry_template name for group[%s]\n", __FUNCTION__, instrument_msg->get_option_serial_id());
        return DBL_MAX;
    }
    const cffex::fb::spread_template_stream::i_stream_msg* spread_msg = get_stream<cffex::fb::spread_template_stream>()->get_stream_table()->get_msg(template_name);
    if(spread_msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, can't get spread_template[%s]\n", __FUNCTION__, template_name);
        return DBL_MAX;
    }
    return spread_msg->get_spread(bid_price);
}
// instrument group params over

/******************************************************************************/

// following are helper functions

double data_manager::get_tick(const char *instrument) {
    const cffex::fb::instrument_stream::i_stream_msg* msg = get_stream<cffex::fb::instrument_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, can't get instrument msg instrument[%s]\n", __FUNCTION__, instrument);
        return -1;
    }
    return msg->get_tick();
}

double data_manager::get_market_bid(const char* instrument) {
    const cffex::fb::md_stream::i_stream_msg* msg = get_stream<cffex::fb::md_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, invalid instrument[%s]\n", __FUNCTION__, instrument);
        return DBL_MAX;
    }
    return msg->get_bid1_price();
}

double data_manager::get_market_ask(const char* instrument) {
    const cffex::fb::md_stream::i_stream_msg* msg = get_stream<cffex::fb::md_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, invalid instrument[%s]\n", __FUNCTION__, instrument);
        return DBL_MAX;
    }
    return msg->get_ask1_price();
}

double data_manager::get_market_mid(const char* instrument) {
    const cffex::fb::md_stream::i_stream_msg* msg = get_stream<cffex::fb::md_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, invalid instrument[%s]\n", __FUNCTION__, instrument);
        return DBL_MAX;
    }
	if (math_helper::valid_price(msg->get_ask1_price()) && math_helper::valid_price(msg->get_bid1_price())) {
    	return (msg->get_ask1_price() + msg->get_bid1_price()) / 2.0;
	}
    return (msg->get_ask1_price() + msg->get_bid1_price()) / 2.0;
}

int data_manager::get_current_netpos(const char* instrument) {
    const cffex::fb::position_stream::i_stream_msg* msg = get_stream<cffex::fb::position_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "config_retrieval::%s, can't get position msg instrument[%s]\n", __FUNCTION__, instrument);
        return 0;
    }

    return msg->get_long_position() - msg->get_short_position();
}


}
}
