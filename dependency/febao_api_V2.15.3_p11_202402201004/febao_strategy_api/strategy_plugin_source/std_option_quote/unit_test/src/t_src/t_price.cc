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

#include "test_init_data.h"

TEST(price, md)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    engine->update_strategy_instance_param(instance_id,"0,1,base_mode,0");
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    // md snap insert
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 94, 104, 2, "IO1902-C-3000");

    // md mid check
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:105,bid_price:95,quote_status:2,strategy_instance_id:65536",false);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,bid1_price:99,ask1_price:-1.0",false);
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));
    engine->pop_quote_cancel_result(instance_id);
    // check_market
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:105,bid_price:95,quote_status:2,strategy_instance_id:65536",false);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,bid1_price:0,ask1_price:102",false);
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));

    // active price
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,bid1_price:null,ask1_price:null",false);
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));

    // inactive spread
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:220,ask1_volume:20,bid1_price:210,bid1_volume:10,last_price:100,upper_limit_price:240,down_limit_price:200",false);
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));

    // upper
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:120,ask1_volume:20,bid1_price:119,bid1_volume:10,last_price:100,upper_limit_price:110,down_limit_price:105",false);
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));

    // inactive threshold
    engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "custom_id:IO,trading_account_id:1,param_key:UpdateThreshold,param_value:-1,status:1",false);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:103,ask1_volume:20,bid1_price:100,bid1_volume:10,last_price:100,upper_limit_price:110,down_limit_price:105",false);
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));

    sleep(1);
    engine->stop();
    delete engine;
    delete impl;
}

TEST(price, upper)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:110,ask1_volume:20,bid1_price:100,bid1_volume:10,last_price:100,upper_limit_price:null,down_limit_price:80",false);

    engine->update_strategy_instance_param(instance_id,"0,1,base_mode,0");
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    // md snap not insert
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));

    sleep(1);
    engine->stop();
    delete engine;
    delete impl;
}

TEST(price, theo)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "custom_id:IO,trading_account_id:1,param_key:UpdateThreshold,param_value:1,status:1",false);
    engine->add_or_update(cffex::mm::MSG_INSTRUMENT_PARAM, "custom_id:IO,trading_account_id:1,trading_account_id:1,param_key:UpdateThreshold,param_value:1,last_operator_id:1,last_operate_source:2,status:1",false);
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    engine->update_strategy_instance_param(instance_id,"0,1,base_mode,1");
    engine->add_or_update(cffex::mm::MSG_OPTION_DERIVED_MD, "instrument_id:IO1902-C-3000,trading_account_id:1,theoretical_price:100.0",false);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    // md snap insert
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 95, 105, 2, "IO1902-C-3000");

    // md no change
    engine->add_or_update(cffex::mm::MSG_QUOTE, "quote_id:4294967300,trading_account_id:1,instrument_id:IO1902-C-3000,ask_price:105,bid_price:95,quote_status:2,strategy_instance_id:65536",false);
    engine->add_or_update(cffex::mm::MSG_OPTION_DERIVED_MD, "instrument_id:IO1902-C-3000,trading_account_id:1,theoretical_price:100.0",false);
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));

    // md change not reach
    engine->add_or_update(cffex::mm::MSG_OPTION_DERIVED_MD, "instrument_id:IO1902-C-3000,trading_account_id:1,theoretical_price:101.0",false);
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));
    // md change
    engine->add_or_update(cffex::mm::MSG_OPTION_DERIVED_MD, "instrument_id:IO1902-C-3000,trading_account_id:1,theoretical_price:102.0",false);
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 97, 107, 2, "IO1902-C-3000");
    // engine->stop();
    delete engine;
    delete impl;
}