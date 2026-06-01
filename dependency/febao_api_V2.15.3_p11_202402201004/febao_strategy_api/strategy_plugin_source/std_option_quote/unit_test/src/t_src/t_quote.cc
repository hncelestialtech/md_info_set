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

TEST(quote, replace)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    // md snap insert
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 94, 104, 2, "IO1902-C-3000");
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:1,strategy_instance_id:65536",false);
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:2,strategy_instance_id:65536",false);

    // md change
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:110,ask1_volume:20,bid1_price:108,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_cancel_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 104, 114, 2, "IO1902-C-3000");
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967304,instrument_id:IO1902-C-3000,ask_price:114,bid_price:104,quote_status:1,strategy_instance_id:65536",false);

    //md no change
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967304,instrument_id:IO1902-C-3000,ask_price:114,bid_price:104,quote_status:2,strategy_instance_id:65536",false);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:110,ask1_volume:20,bid1_price:108,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));

    engine->stop();
    delete engine;
    delete impl;
}

TEST(quote, trade)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    // md snap insert
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 94, 104, 2, "IO1902-C-3000");
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:1,strategy_instance_id:65536",false);
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:2,strategy_instance_id:65536",false);

    // status trade
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:7,strategy_instance_id:65536",false);

    // trade
    engine->add_or_update(cffex::mm::MSG_TRADE, "trading_account_id:1,order_id:4294967300,instrument_id:IO1902-C-3000,trade_id:1,price:94,direction:0,strategy_instance_id:65536",false);
    // sleep(1);
    // EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));    //quote_status:7 ALL_TRADE, why test cancel???
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_INSTRUMENT_PARAM, "instrument_id:IO1902-C-3000,trading_account_id:1,param_key:Quoting,param_value:1,last_operator_id:1,last_operate_source:2,status:1",false);
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 94, 104, 2, "IO1902-C-3000");
    sleep(1);
    engine->stop();
    delete engine;
    delete impl;
}

// TEST(quote, cancel)
// {
//     cffex::mm::engine *engine = cffex::mm::engine_factory::create();
//     init_engine_data(engine);
//     cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
//     int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
//     init_instance_param(engine, instance_id);
//     EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
//     engine->start_strategy_instance(instance_id);
//     sleep(1);
//     EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

//     EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
//     check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 94, 104, 2, "IO1902-C-3000");

//     engine->update_strategy_instance_param(instance_id,"0,1,replace_quote,0");

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
//     engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967304,instrument_id:IO1902-C-3000,ask_price:114,bid_price:104,quote_status:2,strategy_instance_id:65536",false);

//     // cancel failed 1
//     engine->add_or_update(cffex::mm::MSG_TRADING_QUOTE_CANCEL_RSP, "quote_id:4294967300,error_id:2,instrument_id:IO1902-C-3000",false);
//     sleep(2);
//     EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));
//     engine->pop_quote_cancel_result(instance_id);
//     // cancel failed 2
//     engine->add_or_update(cffex::mm::MSG_TRADING_QUOTE_CANCEL_RSP, "trading_account_id:1,quote_id:4294967300,error_id:2,instrument_id:IO1902-C-3000",false);
//     sleep(2);
//     EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));
//     engine->pop_quote_cancel_result(instance_id);
//     // cancel failed 3
//     engine->add_or_update(cffex::mm::MSG_TRADING_QUOTE_CANCEL_RSP, "trading_account_id:1,quote_id:4294967300,error_id:2,instrument_id:IO1902-C-3000",false);
//     sleep(2);
//     EXPECT_EQ(0, engine->get_quote_cancel_result_count(instance_id));
//     // md change, dont quote
//     engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:115,ask1_volume:20,bid1_price:114,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
//     sleep(1);
//     EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));
//     // cancel success
//     engine->add_or_update(cffex::mm::MSG_TRADING_QUOTE_CANCEL_RSP, "trading_account_id:1,quote_id:4294967300,error_id:0,instrument_id:IO1902-C-3000",false);
//     EXPECT_EQ(0, engine->get_quote_cancel_result_count(instance_id));
//     sleep(1);
//     engine->stop();
//     delete engine;
//     delete impl;
// }

TEST(quote, error)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    // md snap insert
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 94, 104, 2, "IO1902-C-3000");
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:1,strategy_instance_id:65536",false);
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:2,strategy_instance_id:65536",false);


    // status error flow
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:5,error_id:9,strategy_instance_id:65536",false);
    //md insert
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:110,ask1_volume:20,bid1_price:108,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 104, 114, 2, "IO1902-C-3000");
    //EXPECT_EQ(0, engine->get_instrument_param_result_count(instance_id));

    // status error not flow
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:5,error_id:12,strategy_instance_id:65536",false);
    //md insert
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:92,ask1_volume:20,bid1_price:90,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967308, 86, 96, 2, "IO1902-C-3000");
    //EXPECT_EQ(1, engine->get_instrument_param_result_count(instance_id));

    // param back
    engine->add_or_update(cffex::mm::MSG_INSTRUMENT_PARAM, "instrument_id:IO1902-C-3000,trading_account_id:1,param_key:Quoting,param_value:0,status:1",false);
    //engine->add_or_update(cffex::mm::MSG_INSTRUMENT_PARAM, "instrument_id:IO1902-C-3000,trading_account_id:1,param_key:Quoting,param_value:0,last_operator_id:1,last_operate_source:2,status:1",false);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:112,ask1_volume:20,bid1_price:111,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));
    //TO_DO
    engine->stop();
    delete engine;
    delete impl;
}

TEST(quote, position)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);
    init_instance_param(engine, instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    // md snap insert
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 94, 104, 2, "IO1902-C-3000");
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:1,strategy_instance_id:65536",false);
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:104,bid_price:94,quote_status:2,strategy_instance_id:65536",false);

    // position failed
    engine->add_or_update(cffex::mm::MSG_POSITION, "trading_account_id:1,instrument_id:IO1902-C-3000,long_position:1000,short_position:1",false);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:105,ask1_volume:20,bid1_price:103,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));
    //not pos limit
    engine->add_or_update(cffex::mm::MSG_POSITION, "trading_account_id:1,instrument_id:IO1902-C-3000,long_position:1,short_position:1",false);
    //engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "custom_id:IO,param_key:NetPosLimit,param_value:0,param_type:int,status:1",false);
    engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "custom_id:IO,trading_account_id:1,param_key:NetPosLimit,param_value:0,last_operator_id:1,last_operate_source:2,status:1",false);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:105,ask1_volume:20,bid1_price:103,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));

    engine->stop();
    delete engine;
    delete impl;
}