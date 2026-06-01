#ifndef STRUCT_HELPER_H
#define STRUCT_HELPER_H

#include "strategy_api.h"
#include "string.h"

namespace cffex {
namespace strategy {

struct strategy_order_info {
    int64_t order_id;
    char    instrument_id[32];
    int8_t  direction;
    int8_t  offset_flag;
    int8_t  hedge_flag;
    double  price;
    int32_t volume;
    int8_t  order_status;
    int16_t portfolio_id;
    int8_t  exchange_id;
    int32_t traded_volume;
    int32_t cancel_volume;
    int8_t  order_source;
    int32_t strategy_instance_id;
    int32_t error_id;

    void set_value(order_msg_type *msg) {
        order_id = msg->get_order_id();
        strncpy(instrument_id, msg->get_instrument_id(), sizeof(instrument_id) - 1);
        direction = msg->get_direction();
        offset_flag = msg->get_offset_flag();
        hedge_flag = msg->get_hedge_flag();
        price = msg->get_price();
        volume = msg->get_volume();
        order_status = msg->get_order_status();
        portfolio_id = msg->get_portfolio_id();
        exchange_id = msg->get_exchange_id();
        traded_volume = msg->get_traded_volume();
        cancel_volume = msg->get_cancel_volume();
        order_source = msg->get_order_source();
        strategy_instance_id = msg->get_strategy_instance_id();
        error_id = msg->get_error_id();
    }

};

struct strategy_quote_info {
    int64_t quote_id;
    char    instrument_id[32];
    int8_t  bid_offset_flag;
    int8_t  ask_offset_flag;
    int8_t  bid_hedge_flag;
    int8_t  ask_hedge_flag;
    double  bid_price;
    double  ask_price;
    int32_t bid_volume;
    int32_t ask_volume;
    int32_t bid_traded_volume;
    int32_t ask_traded_volume;
    int64_t bid_order_id;
    int64_t ask_order_id;
    int8_t  quote_status;
    int8_t  exchange_id;
    int16_t portfolio_id;
    int32_t strategy_instance_id;
    int32_t error_id;

    void set_value(quote_msg_type *msg) {

        quote_id = msg->get_quote_id();
        strncpy(instrument_id, msg->get_instrument_id(), sizeof(instrument_id) - 1);
        bid_offset_flag = msg->get_bid_offset_flag();
        ask_offset_flag = msg->get_ask_offset_flag();
        bid_hedge_flag = msg->get_bid_hedge_flag();
        ask_hedge_flag = msg->get_ask_hedge_flag();
        bid_price = msg->get_bid_price();
        ask_price = msg->get_ask_price();
        bid_volume = msg->get_bid_volume();
        ask_volume = msg->get_ask_volume();
        bid_traded_volume = msg->get_bid_traded_volume();
        ask_traded_volume = msg->get_ask_traded_volume();
        bid_order_id = msg->get_bid_order_id();
        ask_order_id = msg->get_ask_order_id();
        quote_status = msg->get_quote_status();
        exchange_id = msg->get_exchange_id();
        portfolio_id = msg->get_portfolio_id();
        strategy_instance_id = msg->get_strategy_instance_id();
        error_id = msg->get_error_id();
    }
};


}
}


#endif