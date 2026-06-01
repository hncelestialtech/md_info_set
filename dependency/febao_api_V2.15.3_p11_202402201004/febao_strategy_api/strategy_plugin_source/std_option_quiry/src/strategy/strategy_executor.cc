
#include "strategy_executor.h"
#include "data_manager.h"

namespace cffex {
namespace strategy {

#define RECANCEL_INTERVAL 1000

strategy_executor::strategy_executor(int strategy_instance_id, cffex::fb::i_strategy *strategy)
:product_filter_(NULL), instance_filter_(NULL), strategy_instance_id_(strategy_instance_id) {
    callers_ = new caller_handler(strategy);
    data_ = new data_manager(strategy, callers_);
    rule_ = new rule_option_quiry(callers_, data_);

    //rule_->set(data_);
    //rule_->set(callers_);

    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s on_created, instance[%lu]", __FUNCTION__, strategy_instance_id_);
}
strategy_executor::~strategy_executor() {
    if (data_) {
        delete data_;
    }
    if (callers_) {
        delete callers_;
    }
}

bool strategy_executor::start() {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s\n", __FUNCTION__);
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s",  __FUNCTION__);

    init_stream();
    /* 1. init and check instance param*/
	if (!data_->init_params()) {
		return false;
	}
	if (!set_filters()) {
		return false;
	}
	rule_->reset();
    show_instance_params();
    return true;
}

bool strategy_executor::stop()  {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s\n", __FUNCTION__);
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s",  __FUNCTION__);

    rule_->cancel_quote();

    data_->unregister_stream<cffex::fb::inquiry_quote_stream>();
    data_->unregister_stream<cffex::fb::order_stream>();
    data_->unregister_stream<cffex::fb::quote_stream>();
    data_->unregister_stream<cffex::fb::trade_stream>();
    return true;
}

void strategy_executor::init_stream() {
    // register streams
    data_->register_stream<cffex::fb::strategy_instance_param_stream>();
    data_->register_stream<cffex::fb::instrument_stream>();
    data_->register_stream<cffex::fb::custom_param_stream>();
    data_->register_stream<cffex::fb::portfolio_stream>();
    data_->register_stream<cffex::fb::md_stream>();
    data_->register_stream<cffex::fb::trade_stream>();
    data_->register_stream<cffex::fb::position_stream>();
    data_->register_stream<cffex::fb::quote_stream>();
    data_->register_stream<cffex::fb::trading_time_template_stream>();
    data_->register_stream<cffex::fb::spread_template_stream>();
    data_->register_stream<cffex::fb::inquiry_quote_stream>();

    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s", __FUNCTION__);
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_INFO, "strategy_executor::%s\n", __FUNCTION__);
}

bool strategy_executor::show_instance_params() {
    // get & record all instance param
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "instance param: portfolio[%s]", data_->get_portfolio_name());
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "instance param: trading_section[%s]", data_->get_trading_section());
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "instance param: volume[%d]", data_->get_volume());
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "instance param: product[%s]", data_->get_product());
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "instance param: waiting_second[%.1f]", data_->get_waiting_second());
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "instance param: duration_second[%.1f]", data_->get_duration_second());
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_INFO, "instance param: get_netpos_limit[%d]", data_->get_netpos_limit());

	return true;
}

bool strategy_executor::set_filters() {
    if(product_filter_ != NULL) {
        delete product_filter_;
        product_filter_ = NULL;
    }
    /* set serial filter */
    product_filter_ = cffex::fb::product_filter_factory::get_instance()->create_product_equal_filter(data_->get_product());
    if (product_filter_ == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, no product config\n", __FUNCTION__);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, no product config\n",  __FUNCTION__);
        return false;
    }
    data_->get_stream<cffex::fb::inquiry_quote_stream>()->set_filter(product_filter_);

    if(instance_filter_ != NULL) {
        delete instance_filter_;
        instance_filter_ = NULL;
    }
    /* set strategy instance filter */
    instance_filter_ = cffex::fb::strategy_instance_filter_factory::get_instance()->create_strategy_instance_equal_filter(strategy_instance_id_);
    if (instance_filter_ == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, create instance filter failed\n", __FUNCTION__);
        callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_impl::%s, create instance filter failed\n",  __FUNCTION__);
        return false;
    }
    data_->get_stream<cffex::fb::trade_stream>()->set_filter(instance_filter_);
    data_->get_stream<cffex::fb::quote_stream>()->set_filter(instance_filter_);

    // set callback function
    data_->get_stream<cffex::fb::trade_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_trade, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::inquiry_quote_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_inquiry, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::quote_stream>()->set_msg_callback(std::bind(&strategy_executor::on_msg_quote, this, std::placeholders::_1));
    data_->get_stream<cffex::fb::quote_stream>()->set_notify_cancel_callback(std::bind(&strategy_executor::on_msg_quote_cancel, this, std::placeholders::_1));

	return true;
}

void strategy_executor::on_msg_inquiry(inquiry_quote_msg_type *msg) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());

    if (msg->get_inquiry_id() == NULL) {
		// don't deal with empty request
		return;
    }

    if (!rule_->update_quiry_id(msg->get_instrument_id(), msg->get_inquiry_id())) {
		// may receive multiple request for the same id
		return;
    }

	// sleep several time until next active quote
    std::string instrument_id = msg->get_instrument_id();
	std::string quiry_id = msg->get_inquiry_id();

	callers_->get_timer_caller()->register_timer(data_->get_waiting_second()*1000, std::bind(&strategy_executor::on_waiting_timer, this, instrument_id, quiry_id), false);
}

void strategy_executor::on_waiting_timer(std::string instrument, std::string quiry_id) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, instrument[%s] quiry[%ld] \n", __FUNCTION__, instrument.c_str(), quiry_id.c_str());
    rule_->insert_quote(instrument.c_str(), quiry_id.c_str());
}

void strategy_executor::on_msg_quote(IN const cffex::fb::quote_stream::i_stream_msg *msg) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
    int8_t status = msg->get_quote_status();

    switch (status) {
        case cffex::fb::STRATEGY_QUOTE_STATUS_IN_BOOK: {
			// sleep several time until next active quote
			std::string instrument_id = msg->get_instrument_id();
			callers_->get_timer_caller()->register_timer(data_->get_duration_second()*1000, std::bind(&strategy_executor::on_duration_timer, this, instrument_id), false);
			break;
        }
        case cffex::fb::STRATEGY_QUOTE_STATUS_ALL_CANCEL:
        case cffex::fb::STRATEGY_QUOTE_STATUS_ALL_TRADED: {
            break;
        }
        case cffex::fb::STRATEGY_QUOTE_STATUS_ERROR: {
            callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "strategy_executor::%s, quote[%s] error\n", __FUNCTION__, msg->get_instrument_id());
            callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "quote[%s] error", msg->get_instrument_id());
            break;
        }
        case cffex::fb::STRATEGY_QUOTE_STATUS_WAITING:
        case cffex::fb::STRATEGY_QUOTE_STATUS_PART_TRADED: {
            break;
        }
		default: {
			break;
		}
    }
    if(quote_helper::is_final_status(msg)){
        int64_t quote_id = msg->get_quote_id();
        rule_->delete_finish_quotes(quote_id);
    }

}

void strategy_executor::on_duration_timer(std::string instrument) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, instrument[%s]\n", __FUNCTION__, instrument.c_str());
    rule_->cancel_quote(instrument.c_str());
}

void strategy_executor::on_msg_trade(trade_msg_type *msg) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
    rule_->cancel_quote(msg->get_instrument_id());
}

void strategy_executor::on_msg_quote_cancel(quote_cancel_msg_type *msg) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, [%s]\n", __FUNCTION__, msg->to_string().c_str());
	if (msg->get_error_id() == 0) {
		// clear re-cancel times when cancel success
		rule_->clear_recancel_times(msg->get_instrument_id());
		return ;
	}
	// if re-cancel times exceed fixed number(3), stop quoting and report error
	if (!rule_->check_recancel_times(msg->get_instrument_id())) {
    	callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_ERROR, "strategy_executor::%s, instrument[%s] cancel[%ld] failed, reason[%d], stop quoting\n", __FUNCTION__, msg->get_instrument_id(), msg->get_quote_id(), msg->get_error_id());
    	callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_ERROR, "instrument[%s] cancel[%ld] failed, reason[%d], stop quoting", msg->get_instrument_id(), msg->get_quote_id(), msg->get_error_id());
		return;
	}
	// re-cancel if cancel failed, mostly due to flow_control problem
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "strategy_executor::%s, instrument[%s] cancel[%ld] failed, reason[%d], recancel later.. \n", __FUNCTION__, msg->get_instrument_id(), msg->get_quote_id(), msg->get_error_id());
    callers_->get_log_caller()->log_remote(cffex::fb::i_log_caller::ILOG_WARNING, "instrument[%s] cancel[%ld] failed, reason[%d], recancel later.. ", msg->get_instrument_id(), msg->get_quote_id(), msg->get_error_id());

	rule_->record_recancel_times(msg->get_instrument_id());
    std::string instrument = msg->get_instrument_id();
    int64_t quote_id = msg->get_quote_id();
	// re-cancel after certain interval
    callers_->get_timer_caller()->register_timer(RECANCEL_INTERVAL, std::bind(&strategy_executor::on_recancel_timer, this, instrument, quote_id), false);
}

void strategy_executor::on_recancel_timer(std::string instrument, int64_t quote_id) {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "strategy_executor::%s, instrument[%s] quote_id[%ld] \n", __FUNCTION__, instrument.c_str(), quote_id);
    rule_->cancel_quote(instrument.c_str(), quote_id);
}


}
}


