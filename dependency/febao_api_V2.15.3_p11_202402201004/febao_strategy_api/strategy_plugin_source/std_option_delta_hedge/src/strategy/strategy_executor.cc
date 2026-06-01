
#include "strategy_executor.h"

namespace cffex {
namespace strategy {

strategy_executor::strategy_executor(int strategy_instance_id, cffex::fb::i_strategy *strategy)
: instance_id_(strategy_instance_id), portfolio_filter_(NULL), instance_filter_(NULL)  {
    callers_         = new caller_handler(strategy);
    data_            = new data_manager(strategy, callers_);
    delta_hedge_rule_ = new delta_hedge_rule(callers_, data_);
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_created, instance[%lu]\n", __FUNCTION__, instance_id_);
}

strategy_executor::~strategy_executor() {
    delete data_;
    delete callers_;
    delete delta_hedge_rule_;
    if(portfolio_filter_)
        delete portfolio_filter_;
    if(instance_filter_)
        delete instance_filter_;
}

bool strategy_executor::start() {
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_started, instance[%lu]\n", __FUNCTION__, instance_id_);
    init_stream();

    if(!data_->init_params()) {
        return false;
    }
    if(!set_filter()) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, set_filter failed\n", __FUNCTION__);
        return false;
    }
    timer_id_ = callers_->get_timer_caller()->register_timer(data_->interval * 1000, std::bind(&strategy_executor::on_timer, this), true);
    return true;
}

bool strategy_executor::stop()  {
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_paused, instance[%lu]\n", __FUNCTION__, instance_id_);

    data_->unregister_stream<cffex::fb::md_stream>();

    callers_->get_timer_caller()->unregister_timer(timer_id_);

    return true;
}


bool strategy_executor::set_filter() {
    if (instance_filter_ != NULL) {
        delete instance_filter_;
    }
    instance_filter_ = cffex::fb::strategy_instance_filter_factory::get_instance()->create_strategy_instance_equal_filter(instance_id_);
    if (instance_filter_ == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, create instance filter failed\n", __FUNCTION__);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, create instance filter failed\n", __FUNCTION__);
        return false;
    }
    data_->get_stream<cffex::fb::order_stream>()->set_filter(instance_filter_);

    if (portfolio_filter_ != NULL) {
        delete portfolio_filter_;
    }
    portfolio_filter_ = cffex::fb::portfolio_id_filter_factory::get_instance()->create_portfolio_id_equal_filter(data_->portfolio_id);
    if (instance_filter_ == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, create portfolio filter failed\n", __FUNCTION__);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, create portfolio filter failed\n", __FUNCTION__);
        return false;
    }
    data_->get_stream<cffex::fb::portfolio_risk_stream>()->set_filter(instance_filter_);
    return true;
}

void strategy_executor::init_stream() {
    /* register stream */
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s\n", __FUNCTION__);
    data_->register_stream<cffex::fb::strategy_instance_param_stream>();
    data_->register_stream<cffex::fb::instrument_stream>();
    data_->register_stream<cffex::fb::portfolio_stream>();
    data_->register_stream<cffex::fb::order_stream>();
    data_->register_stream<cffex::fb::trading_time_template_stream>();
    data_->register_stream<cffex::fb::md_stream>();
    data_->register_stream<cffex::fb::derived_md_stream>();
    data_->register_stream<cffex::fb::portfolio_risk_stream>();

    /* set callback */
    data_->get_stream<cffex::fb::portfolio_risk_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_portfolio_risk, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::portfolio_risk_stream>()->set_snap_callback(std::bind(&strategy_executor::on_snap_portfolio_risk, this, std::placeholders::_1, std::placeholders::_2));
}

/* *******************************
 * following are stream callbacks
 * **********************************/
void strategy_executor::on_snap_portfolio_risk(portfolio_risk_msg_type *msg, bool is_last) {
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "strategy_executor::%s, msg is NULL, is_last[%d]\n", __FUNCTION__, is_last);
        return;
    }
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
    delta_hedge_rule_->hedge(msg->get_delta());
}

void strategy_executor::on_msg_portfolio_risk(portfolio_risk_msg_type *msg) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
    delta_hedge_rule_->hedge(msg->get_delta());
}

void strategy_executor::on_timer() {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, interval[%0.3f]\n", __FUNCTION__, data_->interval);
    delta_hedge_rule_->hedge();
}


}
}


