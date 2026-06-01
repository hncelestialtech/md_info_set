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

data_manager::data_manager(cffex::fb::i_strategy *obj, caller_handler *callers)
: stream_factory(obj), callers_(callers) {
}
data_manager::~data_manager() {
}

bool data_manager::init_params() {
    bool succ = true;
    succ &=  init_instance_params();
    succ &=  init_common_params();
    return succ;
}

bool data_manager::init_instance_params() {
    callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "data_manager::%s\n", __FUNCTION__);

    bool succ = true;
    /* get param from stream */
    succ &= get_instance_param("portfolio_name", portfolio_name);
    succ &= get_instance_param("underlying_id", underlying_id);
    succ &= get_instance_param("trading_section", trading_section);
    succ &= get_instance_param("trigger_threshold", &trigger_threshold);
    succ &= get_instance_param("target_threshold", &target_threshold);
    succ &= get_instance_param("md_max_spread", &md_max_spread);
    succ &= get_instance_param("max_volume", &max_volume);
    succ &= get_instance_param("interval", &interval);

    if(!succ) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "data_manager::%s, failed[%s]\n", __FUNCTION__);
        return false;
    }

    /* check param */
    portfolio_msg_type* msg = get_stream<cffex::fb::portfolio_stream>()->get_stream_table()->get_msg(portfolio_name);
    if(msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "data_manager::%s, error portfolio_name[%s]\n", __FUNCTION__, portfolio_name);
        return false;
    }
    portfolio_id = msg->get_portfolio_id();

    return true;
}

bool data_manager::init_common_params() {
    instrument_msg_type *inst_msg = get_stream<cffex::fb::instrument_stream>()->get_stream_table()->get_msg(underlying_id);
    if(inst_msg == NULL) {
        callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "data_manager::%s, underlying_id[%s] has no intrument_msg\n", __FUNCTION__, underlying_id);
        return false;
    }
    tick = inst_msg->get_tick();
    return true;
}

}
}