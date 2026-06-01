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
 * Date: 2019-12-10
 */

#include "data_manager.h"
#include "math_helper.h"

namespace cffex {
namespace strategy {

data_manager::data_manager(cffex::fb::i_strategy *obj, caller_handler *callers) : stream_factory(obj), callers_(callers) { }

data_manager::~data_manager() {
}

bool data_manager::init_params() {
    bool succ = true;
    succ &=  init_instance_params();
    succ &=  init_custom_params();
    succ &=  init_common_params();
    return succ;
}

bool data_manager::init_instance_params() {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "data_manager::%s\n", __FUNCTION__);

    bool succ = true;
    /* get param from stream */
    succ &= get_instance_param("trading_section", trading_section);
    succ &= get_instance_param("md_threshold_spread", &md_threshold_spread);
    succ &= get_instance_param("md_threshold_spread_type", &md_threshold_spread_type);
    succ &= get_instance_param("md_threshold_bid_depth", &md_threshold_bid_depth);
    succ &= get_instance_param("md_threshold_ask_depth", &md_threshold_ask_depth);
    succ &= get_instance_param("md_threshold_bid_volume", &md_threshold_bid_volume);
    succ &= get_instance_param("md_threshold_ask_volume", &md_threshold_ask_volume);
    succ &= get_instance_param("position_threshold_upper", &position_threshold_upper);
    succ &= get_instance_param("position_threshold_lower", &position_threshold_lower);
    succ &= get_instance_param("instrument_id", instrument_id);
    succ &= get_instance_param("quote_mode", &quote_mode);
    succ &= get_instance_param("portfolio_name", portfolio_name);
    succ &= get_instance_param("bid_volume", &bid_volume);
    succ &= get_instance_param("ask_volume", &ask_volume);
    succ &= get_instance_param("quote_refresh_msec", &quote_refresh_msec);
    succ &= get_instance_param("quote_delay_msec", &quote_delay_msec);
    succ &= get_instance_param("custom_id", custom_id);

    if(!succ) {
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "data_manager::%s, failed\n", __FUNCTION__);
        return false;
    }

    /* check param */
    if(bid_volume <= 0 || ask_volume <= 0) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "data_manager::%s, invalid bid_volume[%d] and ask_volume[%d]\n", __FUNCTION__, bid_volume, ask_volume);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "data_manager::%s, invalid bid_volume[%d] and ask_volume[%d]\n", __FUNCTION__, bid_volume, ask_volume);
        return false;
    }
    if(quote_mode != ORDER_MODE_TYPE && quote_mode != QUOTE_NORMAL_MODE_TYPE && quote_mode != QUOTE_POP_MODE_TYPE) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "data_manager::%s, invalid quote_mode[%d]\n", __FUNCTION__, quote_mode);
        return false;
    }

    return true;
}


bool data_manager::init_custom_params() {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "data_manager::%s\n", __FUNCTION__);
    bool succ = true;
    succ &= get_custom_param(custom_id, "auto_quote_switch", &auto_quote_switch);
    succ &= get_custom_param(custom_id, "baseprice_shift", &baseprice_shift);
    //succ &= get_custom_param(custom_id, "hedge_shift", &hedge_shift);
    succ &= get_custom_param(custom_id, "spread_mode", &spread_mode);
    succ &= get_custom_param(custom_id, "fix_spread", &fix_spread);
    succ &= get_custom_param(custom_id, "spread_template_name", spread_template_name);

    if(!succ) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "data_manager::%s, failed\n", __FUNCTION__);
        return false;
    }
    if(auto_quote_switch != 0 && auto_quote_switch != 1) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "data_manager::%s, invalid auto_quote_switch[%d]\n", __FUNCTION__, auto_quote_switch);
        return false;
    }
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "data_manager::%s, auto_quote_switch[%d] baseprice_shift[%d] spread_template_name[%s]\n",
        __FUNCTION__, auto_quote_switch, baseprice_shift, spread_template_name);
    return true;
}

bool data_manager::update_custom_param(custom_param_msg_type *msg) {
    if(strcmp(msg->get_custom_id(), custom_id) != 0) {
        return false;
    }
    const char *key = msg->get_param_key();
    const char *value = msg->get_param_value();
    if(strcmp(key, "auto_quote_switch") == 0) {
        auto_quote_switch = atoi(value);
        return true;
    }
    if(strcmp(key, "baseprice_shift") == 0) {
        baseprice_shift = atoi(value);
        return true;
    }
    if(strcmp(key, "spread_mode") == 0) {
        spread_mode = atoi(value);
        return true;
    }
    if(strcmp(key, "fix_spread") == 0) {
        fix_spread = atof(value);
        return true;
    }
    if(strcmp(key, "spread_template_name") == 0) {
        strncpy(spread_template_name, value, sizeof(spread_template_name));
        return true;
    }
    return false;
}

bool data_manager::init_common_params() {
    bool succ = true;
    instrument_msg_type *ins_msg = get_msg_by_instrument<cffex::fb::instrument_stream>(instrument_id);
    tick = ins_msg != NULL ? ins_msg->get_tick() : - 1.0;
    succ = succ && tick > 0.0;
    exchange_id = ins_msg != NULL ? ins_msg->get_exchange_id() : cffex::fb::STRATEGY_EXCHANGE_UNKNOWN;
    succ = succ && exchange_id != cffex::fb::STRATEGY_EXCHANGE_UNKNOWN;
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "data_manager::%s , common_param result[%d]\n", __FUNCTION__, succ);
    return succ;
}

double data_manager::get_template_spread(double price) {
    spread_template_msg_type *msg = get_stream<cffex::fb::spread_template_stream>()->get_stream_table()->get_msg(spread_template_name);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "data_manager::%s, failed to get spread template, spread_template_name[%s]\n", __FUNCTION__, spread_template_name);
        return -1;
    }
    return msg->get_spread(price);
}

}
}