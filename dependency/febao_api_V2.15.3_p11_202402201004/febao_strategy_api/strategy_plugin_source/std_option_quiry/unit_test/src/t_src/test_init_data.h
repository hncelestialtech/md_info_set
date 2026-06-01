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

#define PLUGIN_NAME "std_option_inquiry"

static void check_quote(cffex::mm::i_quote_entity *quote, int64_t quote_id, double bid, double ask, int volume, const char *instrument)
{
    printf("%s, quote_id[%ld] bid[%lf] ask[%lf]\n", __FUNCTION__, quote_id, bid, ask);
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

#endif