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
 * Date: 2019-10-25
 */

#ifndef INIT_DATA_H
#define INIT_DATA_H

#include <gtest/gtest.h>
#include "engine.h"
#include "msg_id.h"
#include "entity.h"
#include <boost/bind.hpp>
#include "strategy_impl.h"
#include <stdint.h>

#define INSTANCE_ID 65536
#define TRADING_ACCOUNT_ID  1
#define PLUGIN_NAME "std_future_quote"


static void check_order(cffex::mm::i_order_entity *order, int64_t order_id, int8_t direction, double price, int portfolio, int8_t offset = 0, int volume = 0)
{
    printf("order_id[%ld] direction[%d] price[%lf] offset[%d] volume[%d]\n", order_id, direction, price, offset, volume);
    ASSERT_TRUE(order != NULL);
    EXPECT_EQ(order->get_order_id(), order_id);
    EXPECT_EQ(order->get_direction(), direction);
    EXPECT_DOUBLE_EQ(order->get_price(), price);
    EXPECT_EQ(order->get_portfolio_id(), portfolio);
    if(offset > 0) {
        EXPECT_EQ(order->get_offset_flag(), offset);
    }
    if(volume > 0) {
        EXPECT_EQ(order->get_volume(), volume);
    }
}

static void check_quote(cffex::mm::i_quote_entity *quote, int64_t quote_id, double bid, double ask, int portfolio)
{
    printf("quote_id[%ld]\n", quote_id);
    ASSERT_TRUE(quote != NULL);
    EXPECT_EQ(quote->get_quote_id(), quote_id);
    EXPECT_DOUBLE_EQ(quote->get_bid_price(), bid);
    EXPECT_DOUBLE_EQ(quote->get_ask_price(), ask);
    EXPECT_EQ(quote->get_portfolio_id(), portfolio);
}

void init_engine_data(cffex::mm::engine *engine);
static void init_table(cffex::mm::engine *engine, int msg, const char **table_str, int count)
{
    for(int i = 0; i < count; i++) {
        engine->add_or_update(msg, table_str[i], false);
    }
}

static void init_instance_param(cffex::mm::engine *engine, int instance_id) {
    engine->update_strategy_instance_param(instance_id,"0,1,trading_section,");
    engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_spread,");
    engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_spread_type,");
    engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_bid_depth,");
    engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_ask_depth,");
    engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_bid_volume,");
    engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_ask_volume,");
    engine->update_strategy_instance_param(instance_id,"0,1,position_threshold_upper,");
    engine->update_strategy_instance_param(instance_id,"0,1,position_threshold_lower,");

    engine->update_strategy_instance_param(instance_id,"0,1,instrument_id,IF1901");
    engine->update_strategy_instance_param(instance_id,"0,1,custom_id,group1");
    engine->update_strategy_instance_param(instance_id,"0,1,quote_mode,0");  //order mode
    engine->update_strategy_instance_param(instance_id,"param_key:portfolio_name,param_value:quote",false);
    engine->update_strategy_instance_param(instance_id,"0,1,bid_volume,10");
    engine->update_strategy_instance_param(instance_id,"0,1,ask_volume,10");
    engine->update_strategy_instance_param(instance_id,"0,1,quote_refresh_msec,2000");
    engine->update_strategy_instance_param(instance_id,"0,1,quote_delay_msec,3000");
}

#endif