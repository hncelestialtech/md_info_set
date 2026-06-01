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

#ifndef QUOTE_HELPER_H
#define QUOTE_HELPER_H

namespace cffex {
namespace strategy {

#include "math_helper.h"

class quote_helper
{
public:
    static bool insert_quote(cffex::fb::i_quote_caller *caller, const char *portfolio_name, const char *instrument_id, int8_t bid_offset_flag, int8_t ask_offset_flag,
        double bid_price, double ask_price, int bid_volume, int ask_volume, int64_t &quote_id, const char *req_id = "", const char* custom_flag="", const char* seat_no="")
    {
        bool pre_check = true;
        pre_check &= caller != NULL;
        pre_check &= math_helper::active_price(bid_price);
        pre_check &= math_helper::active_price(ask_price);
        pre_check &= bid_volume > 0;
        pre_check &= ask_volume > 0;
        if(pre_check) {
            cffex::fb::i_quote_caller::quote_entity *en = caller->create_quote_entity();
            en->set_portfolio_name(portfolio_name);
            en->set_instrument_id(instrument_id);
            en->set_bid_offset_flag(bid_offset_flag);
            en->set_ask_offset_flag(ask_offset_flag);
            en->set_bid_hedge_flag(cffex::fb::STRATEGY_HEDGE_FLAG_MARKET_MAKER);
            en->set_ask_hedge_flag(cffex::fb::STRATEGY_HEDGE_FLAG_MARKET_MAKER);
            en->set_volume_condition(cffex::fb::STRATEGY_VOLUME_ANY);
            en->set_time_condition(cffex::fb::STRATEGY_TIME_CONDITION_GFD);
            en->set_bid_price(bid_price);
            en->set_ask_price(ask_price);
            en->set_bid_volume(bid_volume);
            en->set_ask_volume(ask_volume);
            en->set_inquiry_id(req_id);
            en->set_custom_flag(custom_flag);
            en->set_seat_no(seat_no);
            caller->insert_quote(en, quote_id);
        }
        // callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "quote_helper::%s, pre_check[%d] quote_id[%ld], portfolio_name[%s], instrument_id[%s] bid[%.2f], ask[%.2f], bid_volume[%d] ask_volume[%d] ask_offset_flag[%d] bid_offset_flag[%d] req_id[%s]\n",
            // __FUNCTION__, pre_check, quote_id, portfolio_name, instrument_id, bid_price, ask_price, bid_volume, ask_volume, ask_offset_flag, bid_offset_flag, req_id);
        return true;
    }

    static void cancel_quote(cffex::fb::i_quote_caller *caller, const char *portfolio_name, const char *instrument_id, int64_t quote_id)
    {
        bool pre_check = true;
        pre_check &= caller != NULL;
        pre_check &= quote_id > 0;
        if(pre_check) {
            cffex::fb::i_quote_caller::cancel_quote_entity *en = caller->create_cancel_entity();
            en->set_instrument_id(instrument_id);
            en->set_quote_id(quote_id);
            caller->cancel_quote(en);
        }
        // callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "quote_helper::%s, pre_check[%d] quote_id[%ld], portfolio_name[%s], instrument_id[%s]\n", // __FUNCTION__, pre_check, quote_id, portfolio_name, instrument_id);
    }

    static void cancel_quote(cffex::fb::i_quote_caller *caller, int64_t quote_id, const char *instrument_id = "")
    {
        bool pre_check = true;
        pre_check &= caller != NULL;
        pre_check &= quote_id > 0;
        if(pre_check) {
            cffex::fb::i_quote_caller::cancel_quote_entity *en = caller->create_cancel_entity();
            en->set_quote_id(quote_id);
            en->set_instrument_id(instrument_id);
            caller->cancel_quote(en);
        }
        // callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_DEBUG, "quote_helper::%s, pre_check[%d] quote_id[%ld], portfolio_name[%s], instrument_id[%s]\n", // __FUNCTION__, pre_check, quote_id, portfolio_name, instrument_id);
    }
    static bool is_final(const cffex::fb::quote_stream::i_stream_msg *msg, int64_t quote_id)
    {
        if(quote_id < 1) {
            return true;
        }
        return is_final_status(msg);
    }

    static bool is_final_status(const cffex::fb::quote_stream::i_stream_msg *msg)
    {
        if(msg == NULL) {
            // callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "order_helper::%s, false, msg NULL\n", __FUNCTION__);
            return false;
        }
        return is_final(msg->get_quote_status());
    }

    static bool is_active(const cffex::fb::quote_stream::i_stream_msg *msg)
    {
        if(msg == NULL) {
            // callers_->get_log_caller()->log(cffex::fb::i_log_caller::ILOG_WARNING, "quote_helper::%s, false, msg NULL\n", __FUNCTION__);
            return false;
        }
        return !is_final(msg->get_quote_status());
    }

    static bool is_final(int64_t status)
    {
        if(status == cffex::fb::STRATEGY_QUOTE_STATUS_ALL_CANCEL
            || status == cffex::fb::STRATEGY_QUOTE_STATUS_ERROR
            || status == cffex::fb::STRATEGY_QUOTE_STATUS_ALL_TRADED ) {
            return true;
        }
        return false;
    }
};

}
}

#endif
