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

#include "strategy_executor.h"
#include "string_helper.h"

namespace cffex {
namespace strategy {

strategy_executor::strategy_executor(int strategy_instance_id, cffex::fb::i_strategy *strategy) :
    strategy_instance_id_(strategy_instance_id), serial_filter_(NULL), instance_filter_(NULL)
{
    callers_ = new caller_handler(strategy);
    data_ = new data_manager(strategy, callers_);
    rule_quote_ = new rule_quote(callers_, data_);

    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_created, instance[%lu]", __FUNCTION__, strategy_instance_id_);
}

strategy_executor::~strategy_executor()
{
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_deleted, instance[%lu]", __FUNCTION__, strategy_instance_id_);

    if(serial_filter_ != NULL) {
        delete serial_filter_;
        serial_filter_ = NULL;
    }
    if(instance_filter_ != NULL) {
        delete instance_filter_;
        instance_filter_ = NULL;
    }

    delete rule_quote_;
    delete data_;
    delete callers_;
}

bool strategy_executor::start()
{
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_started, instance[%lu]", __FUNCTION__, strategy_instance_id_);
    init_stream();
    /* 1. init and check instance param and instrument group param */
    if(!data_->init_params()) {
        return false;
    }

    if(serial_filter_ != NULL)
    {
        delete serial_filter_;
        serial_filter_ = NULL;
    }
    /* 3. set filters for stream */
    string_helper::split(data_->serial, ';', std::bind(&strategy_executor::init_serial, this, std::placeholders::_1));
    if(!set_filters()) {
        return false;
    }
    rule_quote_->reset();
    return true;
}

bool strategy_executor::stop()
{
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_paused, instance[%lu]", __FUNCTION__, strategy_instance_id_);
    rule_quote_->cancel_quote();
    data_->unregister_stream<cffex::fb::md_stream>();
    data_->unregister_stream<cffex::fb::derived_md_stream>();
    data_->unregister_stream<cffex::fb::order_stream>();
    data_->unregister_stream<cffex::fb::quote_stream>();
    data_->unregister_stream<cffex::fb::trade_stream>();
    return true;
}


void strategy_executor::init_stream()
{
    /* register stream */
    data_->register_stream<cffex::fb::strategy_instance_param_stream>();
    data_->register_stream<cffex::fb::instrument_stream>();
    data_->register_stream<cffex::fb::custom_param_stream>();
    data_->register_stream<cffex::fb::portfolio_stream>();
    data_->register_stream<cffex::fb::trade_stream>();
    data_->register_stream<cffex::fb::position_stream>();
    data_->register_stream<cffex::fb::quote_stream>();
    data_->register_stream<cffex::fb::trading_time_template_stream>();
    data_->register_stream<cffex::fb::spread_template_stream>();
    data_->register_stream<cffex::fb::instrument_param_stream>();
    data_->register_stream<cffex::fb::md_stream>();
    data_->register_stream<cffex::fb::derived_md_stream>();
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s", __FUNCTION__);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s\n", __FUNCTION__);
}

bool strategy_executor::set_filters()
{
    /* set serial filter */
    if (serial_filter_ == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, no serial config\n", __FUNCTION__);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, no serial config\n",  __FUNCTION__);
        return false;
    }
    data_->get_stream<cffex::fb::md_stream>()->set_filter(serial_filter_);
    data_->get_stream<cffex::fb::derived_md_stream>()->set_filter(serial_filter_);
    data_->get_stream<cffex::fb::instrument_param_stream>()->set_filter(serial_filter_);

    /* set strategy instance filter */
    instance_filter_ = cffex::fb::strategy_instance_filter_factory::get_instance()->create_strategy_instance_equal_filter(strategy_instance_id_);
    if (instance_filter_ == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, create instance filter failed\n", __FUNCTION__);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, create instance filter failed\n",  __FUNCTION__);
        return false;
    }
    data_->get_stream<cffex::fb::trade_stream>()->set_filter(instance_filter_);
    data_->get_stream<cffex::fb::quote_stream>()->set_filter(instance_filter_);

    /* set callback */
    data_->get_stream<cffex::fb::trade_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_trade, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::quote_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_quote, this, std::placeholders::_1));
    // data_->get_stream<cffex::fb::quote_stream>()->set_rsp_cancel_callback(boost::bind(&strategy_executor::on_msg_quote_cancel, this, _1));
    data_->get_stream<cffex::fb::instrument_param_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_instrument_param, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::md_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_md, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::md_stream>()->set_snap_callback(std::bind(&strategy_executor::on_snap_md, this, std::placeholders::_1, std::placeholders::_2));
    data_->get_stream<cffex::fb::derived_md_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_derived_md, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::derived_md_stream>()->set_snap_callback(std::bind(&strategy_executor::on_snap_derived_md, this, std::placeholders::_1, std::placeholders::_2));
    return true;
}

bool strategy_executor::init_serial(const char *serial)
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, serial[%s]\n", __FUNCTION__, serial);
    // init instrument table
    for(instrument_msg_type *msg = data_->get_stream<cffex::fb::instrument_stream>()->get_stream_table()->first_by_serial(serial); msg != NULL;
        msg = data_->get_stream<cffex::fb::instrument_stream>()->get_stream_table()->next_by_serial(serial)) {
        data_->instrument_table_->get_record(msg->get_instrument_id());
    }
    // init serial filter TODO delete tmp
    const cffex::fb::i_filter *tmp = cffex::fb::option_serial_filter_factory::get_instance()->create_option_serial_equal_filter(serial);
    serial_filter_ = (serial_filter_ == NULL) ? tmp : &((*serial_filter_) | (*tmp));
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void strategy_executor::on_snap_md(md_msg_type *msg, bool is_last)
{
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, msg is NULL, is_last[%d]\n", __FUNCTION__, is_last);
        return;
    }
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]-is_last[%d]\n", __FUNCTION__, msg->to_string().c_str(), is_last);
    data_->instrument_table_->update_band_price(msg->get_instrument_id(), msg->get_down_limit_price(), msg->get_upper_limit_price());
}

void strategy_executor::on_snap_derived_md(derived_md_msg_type *msg, bool is_last)
{
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, msg is NULL, is_last[%d]\n", __FUNCTION__, is_last);
    } else {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]-is_last[%d]\n", __FUNCTION__, msg->to_string().c_str(), is_last);
    }
    if (is_last) {
        /* all snapshots have bee received, start quoting for each instrument */
        rule_quote_->insert_quote();
    }
}

void strategy_executor::on_msg_md(md_msg_type *msg)
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
    if (data_->base_mode == MARKET_BASE_MODE) {
        rule_quote_->insert_quote(msg->get_instrument_id());
    }
}

void strategy_executor::on_msg_derived_md(derived_md_msg_type *msg)
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
    if (data_->base_mode == THEO_BASE_MODE) {
        rule_quote_->insert_quote(msg->get_instrument_id());
    }
}

void strategy_executor::on_msg_instrument_param(instrument_param_msg_type *msg)
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
    if(data_->instrument_table_->get_record(msg->get_instrument_id(), false) == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "strategy_executor::%s, invalid instrument_id[%s]\n", __FUNCTION__, msg->to_string().c_str());
        return;
    }
    if(0 == strcmp(INSTRUMENT_QUOTE_SWITCH, msg->get_param_name())) {
        bool quote_switch = (atoi(msg->get_param_value()) == 1) ? true : false;
        if (quote_switch) {
            /* switch change to true, start quoting immediately */
            data_->instrument_table_->get_record(msg->get_instrument_id())->recancel_times_ = 0;
            rule_quote_->insert_quote(msg->get_instrument_id());
        }
        else {
            /* switch change to false, cancel quoting immediately */
            rule_quote_->cancel_quote(msg->get_instrument_id());
        }
    }
    else if(0 == strcmp(INSTRUMENT_BID_OFFSET, msg->get_param_name())) {
        rule_quote_->insert_quote(msg->get_instrument_id(), true);
    }
    else if(0 == strcmp(INSTRUMENT_ASK_OFFSET, msg->get_param_name())) {
        rule_quote_->insert_quote(msg->get_instrument_id(), true);
    }
}

void strategy_executor::on_msg_trade(trade_msg_type *msg)
{
    rule_quote_->on_msg_trade(msg);
}

void strategy_executor::on_msg_quote(quote_msg_type *msg)
{
    rule_quote_->on_msg_quote(msg);
}
void strategy_executor::on_msg_quote_cancel(quote_cancel_msg_type *msg)
{
    rule_quote_->on_msg_quote_cancel(msg);
}



}
}

