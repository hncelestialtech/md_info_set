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
//#include "strategy_config.h"
#include <stdint.h>

#define PLUGIN_NAME "std_option_quote"
//#define TRADING_ACCOUNT_ID 0
#define TRADING_ACCOUNT_ID 1

static void check_quote(cffex::mm::i_quote_entity *quote, int64_t quote_id, double bid, double ask, int volume, const char *instrument)
{
    printf("%s, quote_id[%ld] bid[%lf] ask[%lf]\n", __FUNCTION__, quote_id, bid, ask);
    ASSERT_TRUE(quote != NULL);
    EXPECT_EQ(quote->get_quote_id(), quote_id);
    EXPECT_EQ(quote->get_bid_volume(), volume);
    EXPECT_EQ(quote->get_ask_volume(), volume);
    EXPECT_DOUBLE_EQ(quote->get_bid_price(), bid);
    EXPECT_DOUBLE_EQ(quote->get_ask_price(), ask);
    EXPECT_STREQ(quote->get_instrument_id(), instrument);
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
    engine->update_strategy_instance_param(instance_id,"0,1,portfolio,order");
    engine->update_strategy_instance_param(instance_id,"0,1,volume,2");
    engine->update_strategy_instance_param(instance_id,"0,1,serial,IO1902;IO1903");
    engine->update_strategy_instance_param(instance_id,"0,1,trading_section,");
    engine->update_strategy_instance_param(instance_id,"0,1,base_mode,market");
    engine->update_strategy_instance_param(instance_id,"0,1,trade_pending_second,1");
    engine->update_strategy_instance_param(instance_id,"0,1,replace_quote,1");
}

#endif