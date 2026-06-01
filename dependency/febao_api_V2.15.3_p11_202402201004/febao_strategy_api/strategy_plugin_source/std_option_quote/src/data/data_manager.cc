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
 * Date: 2019-12-10
 */

#include "data_manager.h"
#include "math_helper.h"

namespace cffex {
namespace strategy {

data_manager::data_manager(cffex::fb::i_strategy *obj, caller_handler *callers) : stream_factory(obj), callers_(callers)
{
    instrument_table_ = new instrument_table(callers_);
}

data_manager::~data_manager()
{
    if(instrument_table_) {
        delete instrument_table_;
        instrument_table_ = NULL;
    }
}

bool data_manager::init_params()
{
    if(instrument_table_) {
        delete instrument_table_;
    }
    instrument_table_ = new instrument_table(callers_);
    return init_instance_params();
}

bool data_manager::init_instance_params()
{
    bool succ = true;
    succ &= get_instance_param("portfolio", portfolio);
    succ &= get_instance_param("volume", &volume);
    succ &= get_instance_param("serial", serial);
    succ &= get_instance_param("trading_section", trading_section);
    succ &= get_instance_param("base_mode", &base_mode);
    succ &= get_instance_param("trade_pending_second", &trade_pending_second);
    succ &= get_instance_param("replace_quote", &replace_quote);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "data_manager::%s, ret[%d]\n", __FUNCTION__, succ);
    return succ;
}

bool data_manager::is_in_trading_section()
{
    if ( 0 == strcmp(trading_section,"")) {
        // empty param mean don't check time
        return true;
    }

    const cffex::fb::trading_time_template_stream::i_stream_msg* trading_time_msg = get_stream<cffex::fb::trading_time_template_stream>()->get_stream_table()->get_msg(trading_section);
    if(trading_time_msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, can't get trading_section[%s]\n", __FUNCTION__, trading_section);
        return false;
    }

    if (!trading_time_msg->is_trading_time()) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, current[%d] not in trading_section\n", __FUNCTION__, trading_time_msg->get_curr_trading_sec());
        return false;
    }
    return true;
}

/******************************************************************************/

// following are instrument params
bool  data_manager::get_quote_switch(const char *instrument)
{
    bool ret = false;
    get_instrument_param(instrument, INSTRUMENT_QUOTE_SWITCH, &ret);
    return ret;
}

double  data_manager::get_bid_offset(const char *instrument)
{
    double ret = 0;
    get_instrument_param(instrument, INSTRUMENT_BID_OFFSET, &ret);
    return ret;
}

double  data_manager::get_ask_offset(const char *instrument)
{
    double ret = 0;
    get_instrument_param(instrument, INSTRUMENT_ASK_OFFSET, &ret);
    return ret;
}
// instrument params over

/******************************************************************************/

// following are instrument group params
int  data_manager::get_netpos_limit(const char *instrument)
{
    const cffex::fb::instrument_stream::i_stream_msg* instrument_msg = get_stream<cffex::fb::instrument_stream>()->get_stream_table()->get_msg(instrument);
    if (instrument_msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, no instrument[%s] found\n", __FUNCTION__, instrument);
        return 0;
    }
    int ret = 0;
    get_custom_param(instrument_msg->get_product_id(), PRODUCT_NETPOS_LIMIT , &ret);
    return ret;
}

double  data_manager::get_update_threshold(const char *instrument)
{
    const cffex::fb::instrument_stream::i_stream_msg* instrument_msg = get_stream<cffex::fb::instrument_stream>()->get_stream_table()->get_msg(instrument);
    if (instrument_msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, no instrument[%s] found\n", __FUNCTION__, instrument);
        return DBL_MAX;
    }
    double ret = 0;
    get_custom_param(instrument_msg->get_product_id(), PRODUCT_UPDATE_THRESHOLD , &ret);
    return ret;
}

double data_manager::get_quote_spread(const char* instrument, double price)
{
    const cffex::fb::instrument_stream::i_stream_msg* instrument_msg = get_stream<cffex::fb::instrument_stream>()->get_stream_table()->get_msg(instrument);
    if (instrument_msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, no instrument[%s] found\n", __FUNCTION__, instrument);
        return DBL_MAX;
    }
    char template_name[256] = {0};
    if (! get_stream<cffex::fb::custom_param_stream>()->get_stream_table()->get_param(instrument_msg->get_option_serial_id(), SERIAL_QUOTE_TEMPLATE, template_name)) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, can't get spread_template name for group[%s]\n", __FUNCTION__, instrument_msg->get_option_serial_id());
        return DBL_MAX;
    }
    const cffex::fb::spread_template_stream::i_stream_msg* spread_msg = get_stream<cffex::fb::spread_template_stream>()->get_stream_table()->get_msg(template_name);
    if(spread_msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, can't get spread_template[%s]\n", __FUNCTION__, template_name);
        return DBL_MAX;
    }
    return spread_msg->get_spread(price);
}
// instrument group params over

/******************************************************************************/

double data_manager::get_base_price(const char *instrument_id)
{
    if (base_mode == MARKET_BASE_MODE) {
        return get_market_mid(instrument_id);
    }
    else {
        return get_theo_price(instrument_id);
    }
    return DBL_MAX;
}

double data_manager::get_theo_price(const char* instrument)
{
    const cffex::fb::derived_md_stream::i_stream_msg* msg = get_stream<cffex::fb::derived_md_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, invalid instrument[%s]\n", __FUNCTION__, instrument);
        return 0.0;
    }
    return msg->get_theoretical_price();
}

double data_manager::get_tick(const char *instrument)
{
    const cffex::fb::instrument_stream::i_stream_msg* msg = get_stream<cffex::fb::instrument_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, can't get instrument msg instrument[%s]\n", __FUNCTION__, instrument);
        return -1;
    }
    return msg->get_tick();
}

double data_manager::get_market_bid(const char* instrument)
{
    const cffex::fb::md_stream::i_stream_msg* msg = get_stream<cffex::fb::md_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, invalid instrument[%s]\n", __FUNCTION__, instrument);
        return DBL_MAX;
    }
    return msg->get_bid1_price();
}

double data_manager::get_market_ask(const char* instrument)
{
    const cffex::fb::md_stream::i_stream_msg* msg = get_stream<cffex::fb::md_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, invalid instrument[%s]\n", __FUNCTION__, instrument);
        return DBL_MAX;
    }
    return msg->get_ask1_price();
}

double data_manager::get_market_mid(const char* instrument)
{
    const cffex::fb::md_stream::i_stream_msg* msg = get_stream<cffex::fb::md_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, invalid instrument[%s]\n", __FUNCTION__, instrument);
        return DBL_MAX;
    }
    if (math_helper::valid_price(msg->get_ask1_price()) && math_helper::valid_price(msg->get_bid1_price())) {
        return (msg->get_ask1_price() + msg->get_bid1_price()) / 2.0;
    }
    return DBL_MAX;
}

int data_manager::get_current_netpos(const char* instrument)
{
    const cffex::fb::position_stream::i_stream_msg* msg = get_stream<cffex::fb::position_stream>()->get_stream_table()->get_msg(instrument);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, can't get position msg instrument[%s]\n", __FUNCTION__, instrument);
        return 0;
    }

    return msg->get_long_position() - msg->get_short_position();
}

void data_manager::get_self(const char *instrument, double *bid, double *ask)
{
    record *r = instrument_table_->get_record(instrument);
    if(r == NULL) {
        *bid = DBL_MAX;
        *ask = DBL_MAX;
        return;
    }
    quote_msg_type *quote = get_msg_by_id<cffex::fb::quote_stream>(r->quote_id_);
    if(quote == NULL || is_final(quote->get_quote_status()) ) {
        *bid = DBL_MAX;
        *ask = DBL_MAX;
        return;
    }
    *bid = quote->get_bid_price();
    *ask = quote->get_ask_price();
}

bool data_manager::is_final(int8_t quote_status)
{
    if(quote_status == cffex::fb::STRATEGY_QUOTE_STATUS_ALL_CANCEL || quote_status == cffex::fb::STRATEGY_QUOTE_STATUS_ERROR
        || quote_status == cffex::fb::STRATEGY_QUOTE_STATUS_ALL_TRADED || quote_status == cffex::fb::STRATEGY_QUOTE_STATUS_TIMEOUT) {
        return true;
    }
    return false;
}

}
}