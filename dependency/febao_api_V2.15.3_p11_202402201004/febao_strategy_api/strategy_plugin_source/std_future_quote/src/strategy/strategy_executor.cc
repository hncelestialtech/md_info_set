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
 * Date: 2019-12-06
 */

#include "strategy_executor.h"

namespace cffex {
namespace strategy {

strategy_executor::strategy_executor(int strategy_instance_id, cffex::fb::i_strategy *strategy)
:strategy_instance_id_(strategy_instance_id), instrument_filter_(NULL), custom_filter_(NULL), rule_(NULL) {
    callers_ = new caller_handler(strategy);
    data_ = new data_manager(strategy, callers_);
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_created, instance[%lu]\n", __FUNCTION__, strategy_instance_id_);
}
strategy_executor::~strategy_executor() {
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_deleted, instance[%lu]\n", __FUNCTION__, strategy_instance_id_);

    if(rule_ != NULL) {
        delete rule_;
    }
    delete data_;
    delete callers_;

    if(instrument_filter_ != NULL)
        delete instrument_filter_;
    if(custom_filter_ != NULL)
        delete custom_filter_;
}

bool strategy_executor::start() {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s\n", __FUNCTION__);
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_started, instance[%lu]\n", __FUNCTION__, strategy_instance_id_);

    /* 1. register streams, invoke snapshot after start */
    init_stream();

    /* 2. init and check instance param and instrument group param */
    if(!data_->init_params()) {
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, init params failed\n", __FUNCTION__);
        return false;
    }
    /* create auto_order_rule or auto_quote_rule according to instance param */

    if(data_->quote_mode == ORDER_MODE_TYPE) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s, quote_mode[%d], create auto_order_rule\n", __FUNCTION__, data_->quote_mode);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s, quote_mode[%d], create auto_order_rule\n", __FUNCTION__, data_->quote_mode);
        rule_ = new auto_order_rule(callers_, data_, strategy_instance_id_);
    } else {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s, quote_mode[%d], create auto_quote_rule\n", __FUNCTION__, data_->quote_mode);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s, quote_mode[%d], create auto_quote_rule\n", __FUNCTION__, data_->quote_mode);
        rule_ = new auto_quote_rule(callers_, data_, strategy_instance_id_);
    }

    /* 3. set filters for stream */
    if(!set_filter()) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, set_filter failed\n", __FUNCTION__);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, set_filter failed\n", __FUNCTION__);
        return false;
    }

    return true;
}
bool strategy_executor::stop()  {
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_paused, instance[%lu]\n", __FUNCTION__, strategy_instance_id_);
    rule_->cancel();

    data_->unregister_stream<cffex::fb::md_stream>();
    data_->unregister_stream<cffex::fb::order_stream>();
    data_->unregister_stream<cffex::fb::trade_stream>();
    data_->unregister_stream<cffex::fb::quote_stream>();
    data_->unregister_stream<cffex::fb::custom_param_stream>();
    data_->unregister_stream<cffex::fb::position_stream>();
    return true;
}


void strategy_executor::init_stream() {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, register stream\n", __FUNCTION__);

    data_->register_stream<cffex::fb::strategy_instance_param_stream>();
    data_->register_stream<cffex::fb::instrument_stream>();
    data_->register_stream<cffex::fb::custom_param_stream>();
    data_->register_stream<cffex::fb::portfolio_stream>();
    data_->register_stream<cffex::fb::order_stream>();
    data_->register_stream<cffex::fb::quote_stream>();
    data_->register_stream<cffex::fb::trade_stream>();
    data_->register_stream<cffex::fb::position_stream>();
    data_->register_stream<cffex::fb::trading_time_template_stream>();
    data_->register_stream<cffex::fb::spread_template_stream>();
    data_->register_stream<cffex::fb::md_stream>();
}

bool strategy_executor::set_filter() {
    if(instrument_filter_ != NULL) {
        delete instrument_filter_;
    }
    instrument_filter_ = cffex::fb::instrument_filter_factory::get_instance()->create_instrument_equal_filter(data_->instrument_id);
    if(instrument_filter_ == NULL ) {
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, create instrument failed, instrument_id: [%s] \n",
          __FUNCTION__, data_->instrument_id);
        return false;
    }

    if(custom_filter_!=NULL){
        delete custom_filter_;
    }
    custom_filter_ = cffex::fb::custom_filter_factory::get_instance()->create_custom_equal_filter(data_->custom_id);
    if(custom_filter_ == NULL){
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, create custom_filter failed, custom_id: [%s] \n",
          __FUNCTION__, data_->custom_id);
        return false;
    }
    data_->get_stream<cffex::fb::instrument_stream>()->set_filter(instrument_filter_);
    data_->get_stream<cffex::fb::md_stream>()->set_filter(instrument_filter_);
    data_->get_stream<cffex::fb::position_stream>()->set_filter(instrument_filter_);
    data_->get_stream<cffex::fb::trade_stream>()->set_filter(instrument_filter_);
    data_->get_stream<cffex::fb::quote_stream>()->set_filter(instrument_filter_);
    data_->get_stream<cffex::fb::order_stream>()->set_filter(instrument_filter_);
    data_->get_stream<cffex::fb::custom_param_stream>()->set_filter(custom_filter_);

    /* set callback */
    data_->get_stream<cffex::fb::md_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_md, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::md_stream>()->set_snap_callback(std::bind(&strategy_executor::on_snap_md, this, std::placeholders::_1, std::placeholders::_2));
    data_->get_stream<cffex::fb::custom_param_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_custom_param, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::trade_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_trade, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::quote_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_quote, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::quote_stream>()->set_notify_cancel_callback(std::bind(&strategy_executor::on_msg_quote_cancel, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::order_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_order, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::order_stream>()->set_notify_cancel_callback(std::bind(&strategy_executor::on_msg_order_cancel, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::position_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_position, this, std::placeholders::_1));

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void strategy_executor::on_snap_md(md_msg_type *msg, bool is_last) {
    if(msg != NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]-is_last[%d]\n", __FUNCTION__, msg->to_string().c_str(), is_last);
    }
    if(is_last) {
        rule_->check(true);
    }
}
void strategy_executor::on_msg_md(md_msg_type *msg) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
    rule_->on_msg_md(msg);
}

void strategy_executor::on_msg_position(position_msg_type *msg) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
}

void strategy_executor::on_msg_custom_param(custom_param_msg_type *msg) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
    bool has_update = data_->update_custom_param(msg);
    if(!has_update) {
        return;
    }
    rule_->cancel();
    if(data_->auto_quote_switch) {
        rule_->check(true);
    }
}
void strategy_executor::on_msg_trade(trade_msg_type *msg) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
    rule_->on_msg_trade(msg);
}

void strategy_executor::on_msg_quote(quote_msg_type *msg) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, msg[%s][%s]\n", __FUNCTION__, msg->get_strategy_name(), msg->to_string().c_str());
    if (msg->get_strategy_instance_id() == strategy_instance_id_ ) {
        rule_->on_msg_quote(msg);
    }
}
void strategy_executor::on_msg_quote_cancel(quote_cancel_msg_type *msg) {
    if(data_->quote_mode == QUOTE_NORMAL_MODE_TYPE) {
        rule_->on_msg_quote_cancel(msg, strategy_instance_id_);
    }
}

void strategy_executor::on_msg_order(order_msg_type *msg) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, msg[%s][%s]\n", __FUNCTION__, msg->get_strategy_name(), msg->to_string().c_str());
    if( data_->quote_mode == ORDER_MODE_TYPE && msg->get_strategy_instance_id() == strategy_instance_id_ ) {
        rule_->on_order(msg);
        rule_->on_msg_order(msg);
    }
}
void strategy_executor::on_msg_order_cancel(order_cancel_msg_type *msg) {
    if(data_->quote_mode == ORDER_MODE_TYPE) {
        rule_->on_msg_order_cancel(msg, strategy_instance_id_);
    }
}



}
}

