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
 * Date: 2019-07-13
 */

#ifndef ORDER_HELPER_H
#define ORDER_HELPER_H

#include "math_helper.h"
#include "strategy_api.h"

namespace cffex {
namespace strategy {

class order_helper
{
public:
    static bool insert_order(cffex::fb::i_order_caller *caller, const char *portfolio_name, const char *instrument_id,
        int8_t direction, int8_t offset_flag, double price, int volume, int64_t &order_id,const char* custom_flag="", const char* seat_no="")
    {
        bool pre_check = math_helper::active_price(price);
        pre_check &= (volume > 0);
        pre_check &= (caller != NULL);
        if(pre_check) {
            cffex::fb::i_order_caller::order_entity *en = caller->create_order_entity();
            en->set_portfolio_name(portfolio_name);
            en->set_instrument_id(instrument_id);
            en->set_direction(direction);
            en->set_volume(volume);
            en->set_price(price);
            en->set_offset_flag(offset_flag);
            en->set_volume_condition(cffex::fb::STRATEGY_VOLUME_ANY);
            en->set_price_category(cffex::fb::STRATEGY_PRICE_CATEGORY_LIMIT);
            en->set_time_condition(cffex::fb::STRATEGY_TIME_CONDITION_GFD);
            en->set_hedge_flag(cffex::fb::STRATEGY_HEDGE_FLAG_MARKET_MAKER);
            en->set_custom_flag(custom_flag);
            en->set_seat_no(seat_no);
            caller->insert_order(en, order_id);
        }
        return pre_check;
    }

    static bool cancel_order(cffex::fb::i_order_caller *caller, int64_t order_id, const char *instrument_id = "")
    {
        bool pre_check = true;
        pre_check &= (caller != NULL);
        pre_check &= (order_id > 0);
        if(pre_check) {
            cffex::fb::i_order_caller::cancel_order_entity *en = caller->create_cancel_entity();
            en->set_order_id(order_id);
            en->set_instrument_id(instrument_id);
            caller->cancel_order(en);
        }
        // callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "order_helper::%s, pre_check[%d] order_id[%ld]\n", // __FUNCTION__, pre_check, order_id);
        return true;
    }

    static bool is_final(const cffex::fb::order_stream::i_stream_msg *msg, int64_t order_id)
    {
        if(order_id < 1) {
            return true;
        }
        return is_final_status(msg);
    }

    static bool is_active(const cffex::fb::order_stream::i_stream_msg *msg)
    {
        if(msg == NULL) {
            // callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "order_helper::%s, false, msg NULL\n", __FUNCTION__);
            return false;
        }
        return !is_final_status(msg);
    }

    static bool is_final_status(const cffex::fb::order_stream::i_stream_msg *msg)
    {
        if(msg == NULL) {
            // callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "order_helper::%s, false, msg NULL\n", __FUNCTION__);
            return false;
        }
        return is_final(msg->get_order_status());
    }

    static bool is_final(int8_t status)
    {
        return (status == cffex::fb::STRATEGY_ORDER_STATUS_CANCEL || status == cffex::fb::STRATEGY_ORDER_STATUS_ERROR || status == cffex::fb::STRATEGY_ORDER_STATUS_ALL_TRADED) ? true : false;
    }

};

}
}

#endif
