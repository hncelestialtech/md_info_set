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
 * Date: 2019-12-12
 */

#include "test_init_data.h"

TEST(hedge, trade)
{
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    int instance_id =  engine->create_strategy_instance(STRATEGY_NAME, TRADING_ACCOUNT_ID, impl);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    init_engine_data(engine);
    init_instance_param(engine, instance_id);
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:1294967307,order_source:3,order_status:3,portfolio_id:2,strategy_instance_id:123", false);
    engine->add_or_update(cffex::mm::MSG_TRADE, "trading_account_id:1,trade_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:1294967307,portfolio_id:2,strategy_instance_id:123", false);
    sleep(1);
    check_order(engine->pop_order_insert_result(instance_id), 4294967303, '0', 102, 2);
    sleep(1);
    delete engine;
    delete impl;
}

TEST(hedge, shift)
{
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    int instance_id =  engine->create_strategy_instance(STRATEGY_NAME, TRADING_ACCOUNT_ID, impl);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    init_engine_data(engine);
    init_instance_param(engine, instance_id);
    engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "trading_account_id:1,custom_id:custom1,param_key:hedge_buy_shift,param_value:1,status:1", false);
    engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "trading_account_id:1,custom_id:custom1,param_key:hedge_sell_shift,param_value:1,status:1", false);
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:1294967307,order_source:3,order_status:3,portfolio_id:2,strategy_instance_id:123", false);
    engine->add_or_update(cffex::mm::MSG_TRADE, "trading_account_id:1,trade_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:1294967307,portfolio_id:2,strategy_instance_id:123", false);
    sleep(1);
    check_order(engine->pop_order_insert_result(instance_id), 4294967303, '0', 102.5, 2, '5', 10);
    sleep(1);
    delete engine;
    delete impl;
}

TEST(hedge, hedge_cutloss_threshold)
{
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    int instance_id =  engine->create_strategy_instance(STRATEGY_NAME, TRADING_ACCOUNT_ID, impl);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    init_engine_data(engine);
    init_instance_param(engine, instance_id);
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:1294967307,order_source:3,order_status:3,portfolio_id:2,strategy_instance_id:123", false);
    engine->add_or_update(cffex::mm::MSG_TRADE, "trading_account_id:1,trade_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:1294967307,portfolio_id:2,strategy_instance_id:123", false);
    sleep(1);
    check_order(engine->pop_order_insert_result(instance_id), 4294967303, '0', 102, 2);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,order_id:4294967303,direction:0,order_status:2,price:102,volume:2,strategy_instance_id:65536", false);
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1902,ask1_price:109,ask1_volume:10,bid1_price:95,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,order_id:4294967303,direction:0,order_status:3,price:102,volume:2,strategy_instance_id:65536", false);
    sleep(1);
    check_order(engine->pop_order_insert_result(instance_id), 4294967307, '0', 108, 2, '5', 2);
    sleep(1);
    delete engine;
    delete impl;
}

TEST(hedge, hedge_cutloss_threshold2)
{
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    int instance_id =  engine->create_strategy_instance(STRATEGY_NAME, TRADING_ACCOUNT_ID, impl);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    init_engine_data(engine);
    init_instance_param(engine, instance_id);
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:1294967307,order_source:3,order_status:3,portfolio_id:2,strategy_instance_id:123", false);
    engine->add_or_update(cffex::mm::MSG_TRADE, "trading_account_id:1,trade_id:1,instrument_id:IF1902,volume:7,price:102,direction:1,order_id:1294967307,portfolio_id:2,strategy_instance_id:123", false);
    engine->add_or_update(cffex::mm::MSG_TRADE, "trading_account_id:1,trade_id:2,instrument_id:IF1902,volume:1,price:102,direction:1,order_id:1294967307,portfolio_id:2,strategy_instance_id:123", false);
    engine->add_or_update(cffex::mm::MSG_TRADE, "trading_account_id:1,trade_id:3,instrument_id:IF1902,volume:2,price:102,direction:1,order_id:1294967307,portfolio_id:2,strategy_instance_id:123", false);

    sleep(1);
    check_order(engine->pop_order_insert_result(instance_id), 4294967303, '0', 102, 2);
    check_order(engine->pop_order_insert_result(instance_id), 4294967307, '0', 102, 2);
    check_order(engine->pop_order_insert_result(instance_id), 4294967311, '0', 102, 2);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,order_id:4294967303,direction:0,order_status:2,price:102,volume:7,strategy_instance_id:65536", false);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,order_id:4294967307,direction:0,order_status:2,price:102,volume:1,strategy_instance_id:65536", false);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,order_id:4294967311,direction:0,order_status:2,price:102,volume:2,strategy_instance_id:65536", false);
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1902,ask1_price:108,ask1_volume:10,bid1_price:100,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,order_id:4294967303,direction:0,order_status:3,price:102,volume:7,strategy_instance_id:65536", false);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,order_id:4294967307,direction:0,order_status:3,price:102,volume:1,strategy_instance_id:65536", false);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,order_id:4294967311,direction:0,order_status:3,price:102,volume:2,strategy_instance_id:65536", false);

    sleep(1);
    check_order(engine->pop_order_insert_result(instance_id), 4294967315, '0', 108, 2, '5', 7);
    check_order(engine->pop_order_insert_result(instance_id), 4294967319, '0', 108, 2, '5', 1);
    check_order(engine->pop_order_insert_result(instance_id), 4294967323, '0', 108, 2, '5', 2);
    delete engine;
    delete impl;
}

TEST(hedge, manual_cancel_hedge)
{
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    int instance_id =  engine->create_strategy_instance(STRATEGY_NAME, TRADING_ACCOUNT_ID, impl);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    init_engine_data(engine);
    init_instance_param(engine, instance_id);
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:1294967307,order_source:3,order_status:3,portfolio_id:2,strategy_instance_id:123", false);
    engine->add_or_update(cffex::mm::MSG_TRADE, "trading_account_id:1,trade_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:1294967307,portfolio_id:2,strategy_instance_id:123", false);
    sleep(1);
    check_order(engine->pop_order_insert_result(instance_id), 4294967303, '0', 102, 2);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,order_id:4294967303,direction:0,order_status:2,price:102,volume:2,strategy_instance_id:65536", false);
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,order_id:4294967303,direction:0,order_status:3,price:102,volume:2,strategy_instance_id:65536", false);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1902,ask1_price:108,ask1_volume:10,bid1_price:100,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);
    sleep(1);
    ASSERT_EQ(0, engine->get_order_insert_result_count(instance_id));
    sleep(1);
    delete engine;
    delete impl;
}

TEST(hedge, error)
{
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    int instance_id =  engine->create_strategy_instance(STRATEGY_NAME, TRADING_ACCOUNT_ID, impl);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    init_engine_data(engine);
    init_instance_param(engine, instance_id);
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:1294967307,order_source:3,order_status:3,portfolio_id:2,strategy_instance_id:123", false);
    engine->add_or_update(cffex::mm::MSG_TRADE, "trading_account_id:1,trade_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:1294967307,portfolio_id:2,strategy_instance_id:123", false);
    sleep(1);
    ASSERT_EQ(1, engine->get_order_insert_result_count(instance_id));
    check_order(engine->pop_order_insert_result(instance_id), 4294967303, '0', 102, 2, '5', 10);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:4294967303,order_source:3,order_status:4,portfolio_id:2,strategy_instance_id:65536", false);
    sleep(1);
    ASSERT_EQ(1, engine->get_order_insert_result_count(instance_id));
    check_order(engine->pop_order_insert_result(instance_id), 4294967307, '0', 102, 2, '5', 10);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:4294967307,order_source:3,order_status:4,error_id:23,portfolio_id:2,strategy_instance_id:65536", false);
    sleep(1);
    ASSERT_EQ(1, engine->get_order_insert_result_count(instance_id));
    check_order(engine->pop_order_insert_result(instance_id), 4294967311, '0', 102, 2, '5', 10);
    sleep(1);
    delete engine;
    delete impl;
}

TEST(rehedge, error)
{
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    int instance_id =  engine->create_strategy_instance(STRATEGY_NAME, TRADING_ACCOUNT_ID, impl);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    init_engine_data(engine);
    init_instance_param(engine, instance_id);
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:1294967307,order_source:3,order_status:3,portfolio_id:2,strategy_instance_id:123", false);
    engine->add_or_update(cffex::mm::MSG_TRADE, "trading_account_id:1,trade_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:1294967307,portfolio_id:2,strategy_instance_id:123", false);
    sleep(1);
    check_order(engine->pop_order_insert_result(instance_id), 4294967303, '0', 102, 2);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,order_id:4294967303,direction:0,order_status:2,price:102,volume:2,strategy_instance_id:65536", false);
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1902,ask1_price:108,ask1_volume:10,bid1_price:100,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,order_id:4294967303,direction:0,order_status:3,price:102,volume:2,strategy_instance_id:65536", false);
    sleep(1);
    check_order(engine->pop_order_insert_result(instance_id), 4294967307, '0', 108, 2, '5', 2);
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:4294967307,order_source:3,order_status:4,error_id:23,portfolio_id:2,strategy_instance_id:65536", false);
    sleep(1);
    ASSERT_EQ(1, engine->get_order_insert_result_count(instance_id));
    check_order(engine->pop_order_insert_result(instance_id), 4294967311, '0', 108, 2, '5', 2);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,instrument_id:IF1902,volume:10,price:102,direction:1,order_id:4294967311,order_source:3,order_status:2,error_id:23,portfolio_id:2,strategy_instance_id:65536", false);
    delete engine;
    delete impl;
}