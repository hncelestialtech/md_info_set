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

TEST(common, instance)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    // create
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    // start
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 94, 104, 2, "IO1902-C-3000");
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:2,strategy_instance_id:65536",false);
    sleep(1);
    // pause
    engine->pause_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_PAUSE_STAT, engine->get_state(instance_id));
    EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));
    engine->pop_quote_cancel_result(instance_id);
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:4,strategy_instance_id:65536",false);
    // start
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 94, 104, 2, "IO1902-C-3000");
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967304,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:2,strategy_instance_id:65536",false);
    sleep(1);

    // pause
    engine->pause_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_PAUSE_STAT, engine->get_state(instance_id));
    EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));
    engine->pop_quote_cancel_result(instance_id);
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967304,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:4,strategy_instance_id:65536",false);

    // delete
    engine->delete_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_DELETE_STAT, engine->get_state(instance_id));
    // create
    instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    // start
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967308, 94, 104, 2, "IO1902-C-3000");

    engine->stop();
    delete engine;
    delete impl;
}

TEST(common, period)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    engine->update_strategy_instance_param(instance_id,"0,1,trading_section,section1,1");
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    engine->pause_strategy_instance(instance_id);
    engine->stop();
    sleep(1);
    delete engine;
    delete impl;
}

TEST(common, instrument_param)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    engine->update_strategy_instance_param(instance_id,"0,1,trading_section,section1,1");
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    engine->add_or_update(cffex::mm::MSG_INSTRUMENT_PARAM, "instrument_id:IO1902-C-3000,trading_account_id:1,param_key:Quoting,param_value:1,last_operator_id:1,last_operate_source:2,status:1",false);
    engine->add_or_update(cffex::mm::MSG_INSTRUMENT_PARAM, "instrument_id:IO1902-C-3000,trading_account_id:1,param_key:BidOffset,param_value:1,last_operator_id:1,last_operate_source:2,status:1",false);
    engine->add_or_update(cffex::mm::MSG_INSTRUMENT_PARAM, "instrument_id:IO1902-C-3000,trading_account_id:1,param_key:AskOffset,param_value:1,last_operator_id:1,last_operate_source:2,status:1",false);
    // TODO
    sleep(1);
    engine->stop();
    delete engine;
    delete impl;
}

TEST(common, filter_init)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    //cffex::mm::strategy_config::get_instance()->set_node_id(1);
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    engine->update_strategy_instance_param(instance_id,"0,1,serial,IO1902;IO1903,1");
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 94, 104, 2, "IO1902-C-3000");

    engine->stop();
    delete engine;
    delete impl;
}

TEST(common, filter_update)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    engine->update_strategy_instance_param(instance_id,"0,1,serial,IO1901;IO1902;IO1903,1");
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    // md snap insert
    sleep(1);
    EXPECT_EQ(3, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 94, 104, 2, "IO1902-C-3000");
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 94, 104, 2, "IO1901-C-3100");
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967308, 94, 104, 2, "IO1901-C-3000");
    //serial change
    engine->pause_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_PAUSE_STAT, engine->get_state(instance_id));
    init_instance_param(engine, instance_id);
    engine->update_strategy_instance_param(instance_id,"0,1,serial,IO1902;IO1903,1");
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967312, 94, 104, 2, "IO1902-C-3000");

    engine->stop();
    delete engine;
    delete impl;
}

// TEST(common, delete_timer)
// {
//     cffex::mm::engine *engine = cffex::mm::engine_factory::create();
//     init_engine_data(engine);
//     cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
//     int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
//     init_instance_param(engine, instance_id);
//     engine->update_strategy_instance_param(instance_id,"0,1,replace_quote,0,1");
//     EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
//     engine->start_strategy_instance(instance_id);
//     sleep(1);
//     EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

//     // md snap insert
//     sleep(1);
//     EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));
//     engine->pop_quote_cancel_result(instance_id);
//     EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
//     check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 94, 104, 2, "IO1902-C-3000");
//     engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:1,strategy_instance_id:65536",false);
//     engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:2,strategy_instance_id:65536",false);

//     // md change
//     engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:110,ask1_volume:20,bid1_price:108,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
//     sleep(1);
//     EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));
//     engine->pop_quote_cancel_result(instance_id);
//     check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 104, 114, 2, "IO1902-C-3000");
//     engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967304,instrument_id:IO1902-C-3000,ask_price:114,bid_price:104,quote_status:1,strategy_instance_id:65536",false);
//     // cancel failed
//     engine->add_or_update(cffex::mm::MSG_TRADING_QUOTE_CANCEL_RSP, "trading_account_id:1,quote_id:4294967300,error_id:2,instrument_id:IO1902-C-3000,strategy_instance_id:65536",false);
//     // delete
//     engine->pause_strategy_instance(instance_id);
//     engine->delete_strategy_instance(instance_id);
//     sleep(1);
//     EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_DELETE_STAT, engine->get_state(instance_id));

//     sleep(2);
//     engine->stop();
//     delete engine;
//     delete impl;
// }

TEST(common, pause)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    // create
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    // start
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 94, 104, 2, "IO1902-C-3000");
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:2,strategy_instance_id:65536",false);
    // pause
    engine->pause_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_PAUSE_STAT, engine->get_state(instance_id));
    EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));
    engine->pop_quote_cancel_result(instance_id);
    // trade
    engine->add_or_update(cffex::mm::MSG_TRADE, "trading_account_id:1,trade_id:123,instrument_id:IO1902-C-3100,price:114,strategy_instance_id:65536",false);
    sleep(1);
    engine->stop();
    delete engine;
    delete impl;
}