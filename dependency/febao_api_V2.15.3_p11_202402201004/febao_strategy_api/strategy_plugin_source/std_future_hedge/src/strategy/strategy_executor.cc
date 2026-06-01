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

#include "strategy_executor.h"
#include "order_helper.h"
#include "stream_factory.h"

namespace cffex {
namespace strategy {

strategy_executor::strategy_executor(int strategy_instance_id, cffex::fb::i_strategy *strategy) : rule_hedge_(NULL), instrument_filter_(NULL), instance_id_(strategy_instance_id)
{
    callers_    = new caller_handler(strategy);
    data_       = new data_manager(strategy);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_created, instance[%d]", __FUNCTION__, instance_id_);
}

strategy_executor::~strategy_executor()
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_deleted, instance[%d]", __FUNCTION__, instance_id_);
    if(rule_hedge_!=NULL){
        delete rule_hedge_;
    }
    if(instrument_filter_ != NULL) {
        delete instrument_filter_;
    }
    delete data_;
    delete callers_;

}

bool strategy_executor::start()
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_started, instance[%d]", __FUNCTION__, instance_id_);
    data_->register_stream<cffex::fb::strategy_instance_param_stream>();
    data_->register_stream<cffex::fb::instrument_stream>();
    data_->register_stream<cffex::fb::custom_param_stream>();
    data_->register_stream<cffex::fb::trade_stream>();
    data_->register_stream<cffex::fb::order_stream>();
    data_->register_stream<cffex::fb::md_stream>();
    data_->register_stream<cffex::fb::position_stream>();
    data_->register_stream<cffex::fb::trading_time_template_stream>();
    data_->register_stream<cffex::fb::portfolio_stream>();
    data_->register_stream<cffex::fb::portfolio_position_stream>();

    if(!data_->init()) {
        return false;
    }

    if(instrument_filter_ != NULL) {
        delete instrument_filter_;
    }
    instrument_filter_ = cffex::fb::instrument_filter_factory::get_instance()->create_instrument_equal_filter(data_->instrument_id);
    if(instrument_filter_ == NULL) {
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, filter NULL\n", __FUNCTION__);
        return false;
    }

    data_->get_stream<cffex::fb::order_stream>()->set_filter(instrument_filter_);
    data_->get_stream<cffex::fb::order_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_order, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::order_stream>()->set_notify_cancel_callback(std::bind(&strategy_executor::on_msg_order_cancel, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::trade_stream>()->set_filter(instrument_filter_);
    data_->get_stream<cffex::fb::md_stream>()->set_filter(instrument_filter_);
    data_->get_stream<cffex::fb::trade_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_trade, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::custom_param_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_custom_param, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::md_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_md, this, std::placeholders::_1));
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, success, instance_id[%d]\n", __FUNCTION__, instance_id_);
    rule_hedge_ = new rule_hedge(callers_, data_);
    return true;
}

bool strategy_executor::stop()
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_paused, instance[%d]", __FUNCTION__, instance_id_);
    data_->unregister_stream<cffex::fb::order_stream>();
    data_->unregister_stream<cffex::fb::trade_stream>();
    data_->unregister_stream<cffex::fb::custom_param_stream>();
    data_->unregister_stream<cffex::fb::md_stream>();
    delete rule_hedge_;
    return true;
}

void strategy_executor::on_msg_order(order_msg_type *msg)
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, msg[%s]\n", __FUNCTION__, msg->to_string().c_str());
    rule_hedge_->on_order(msg);
    if(order_helper::is_final(msg->get_order_status()) && msg->get_order_status() != cffex::fb::STRATEGY_ORDER_STATUS_ERROR) {
        rule_hedge_->check_hedge();
    }
}

void strategy_executor::on_msg_order_cancel(order_cancel_msg_type *msg)
{
    rule_hedge_->on_msg_order_cancel(msg, instance_id_);
}

void strategy_executor::on_msg_trade(trade_msg_type *msg)
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, msg[%s][%s]\n", __FUNCTION__, msg->get_strategy_name(), msg->to_string().c_str());
    if( msg->get_strategy_instance_id() != instance_id_ && strcmp(msg->get_strategy_name(), STRATEGY_NAME) != 0
        && msg->get_strategy_instance_id() != 0 && strcmp(msg->get_portfolio_name(), data_->portfolio_name) == 0) {
        rule_hedge_->add_trade(msg->get_trade_id());
    }
}

void strategy_executor::on_msg_custom_param(custom_param_msg_type *msg)
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, msg[%s]\n", __FUNCTION__, msg->to_string().c_str());
    data_->init_group_param();
}

void strategy_executor::on_msg_md(md_msg_type *msg)
{
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, msg[%s]\n", __FUNCTION__, msg->to_string().c_str());
    rule_hedge_->check_hedge_cutloss_threshold(msg);
}

}
}

