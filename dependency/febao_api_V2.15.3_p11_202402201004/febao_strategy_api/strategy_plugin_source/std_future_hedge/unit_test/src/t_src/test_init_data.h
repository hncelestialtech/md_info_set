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

const int TRADING_ACCOUNT_ID = 1;

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

void init_engine_data(cffex::mm::engine *engine);

static void init_table(cffex::mm::engine *engine, int msg, const char **table_str, int count)
{
    for(int i = 0; i < count; i++) {
        engine->add_or_update(msg, table_str[i], false);
    }
}

static void init_instance_param(cffex::mm::engine *engine, int instance_id)
{
    engine->update_strategy_instance_param(instance_id,"0,1,instrument_id,IF1902");
    engine->update_strategy_instance_param(instance_id,"0,1,portfolio_name,order");
    engine->update_strategy_instance_param(instance_id,"0,1,hedge_cutloss_threshold,10");
    engine->update_strategy_instance_param(instance_id,"0,1,hedge_cutloss_waiting,500");
    engine->update_strategy_instance_param(instance_id,"param_key:custom_id,param_value:custom1",false);
}

#endif