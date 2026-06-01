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

#include "data_manager.h"
#include "price_helper.h"

namespace cffex {
namespace strategy {

bool data_manager::init()
{
    bool succ = true;
    succ &= get_instance_param("instrument_id", instrument_id);
    succ &= get_instance_param("portfolio_name", portfolio_name);
    succ &= get_instance_param("hedge_cutloss_threshold", &hedge_cutloss_threshold);
    succ &= get_instance_param("hedge_cutloss_waiting", &hedge_cutloss_waiting);
    succ &= get_instance_param("custom_id", custom_id);

    succ &= init_group_param();

    tick = init_tick(instrument_id);
    succ &= math_helper::active_price(tick);

    //callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "data_manager::%s, succ[%d] tick[%lf]\n", __FUNCTION__, succ, tick);
    return succ;
}

bool data_manager::init_group_param()
{
    bool succ = true;
    succ &= get_custom_param(custom_id, "hedge_buy_shift", &hedge_buy_shift);
    succ &= get_custom_param(custom_id, "hedge_sell_shift", &hedge_sell_shift);
    succ &= get_custom_param(custom_id, "rehedge_shift", &rehedge_shift);
    return succ;
}

double data_manager::init_tick(const char *instrument_id)
{
    instrument_msg_type *ins_msg = get_msg_by_instrument<cffex::fb::instrument_stream>(instrument_id);
    return ins_msg != NULL ? ins_msg->get_tick() : INVALID_PRICE;
}

}
}