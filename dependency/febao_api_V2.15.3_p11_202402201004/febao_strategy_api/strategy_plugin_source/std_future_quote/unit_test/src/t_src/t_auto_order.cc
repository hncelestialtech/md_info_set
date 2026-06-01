#include "init_data.h"

TEST(auto_order, order_refresh) {
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);

    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);

    /* risk params is null, do not check */
    init_instance_param(engine, instance_id);
    ASSERT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));

    //engine->dump();
    engine->start_strategy_instance(instance_id);
    sleep(1);
    ASSERT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    /********************************************************************/
    /* after start, generate first quote
        base_price = (100+98)/2 = 99
        shift: 99 + 2*0.2 = 99.4
        spread: 100-98 = 2
        bid = 99.4-1 = 98.4  ask = 99.4+1 = 100.4
     */
    /********************************************************************/

    ASSERT_EQ(0, engine->get_order_insert_result_count(instance_id));
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:100,ask1_volume:10,bid1_price:98,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);
    sleep(1);
    ASSERT_EQ(2, engine->get_order_insert_result_count(instance_id));
    check_order(engine->pop_order_insert_result(instance_id), 4294967303, 48, 99, 4, 53, 10);
    check_order(engine->pop_order_insert_result(instance_id), 4294967307, 49, 101, 4, 53, 10);

    /********************************************************************/
    /* quote_refresh: insert quote success, start quote_refresh timer */
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967303,instrument_id:IF1901,direction:0,price:98.40000,volume:10,order_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967307,instrument_id:IF1901,direction:1,price:100.40000,volume:20,order_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    sleep(4);  /*wait for timeout*/

    //ASSERT_EQ(2, engine->get_order_cancel_result_count(instance_id));
    //cancel order and insert
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967303,instrument_id:IF1901,direction:0,price:98.40000,volume:10,order_status:3,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967307,instrument_id:IF1901,direction:1,price:100.40000,volume:20,order_status:3,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    sleep(1);
    check_order(engine->pop_order_insert_result(instance_id), 4294967311, 48, 99, 4,53, 10);
    check_order(engine->pop_order_insert_result(instance_id), 4294967315, 49, 101, 4,53, 10);
    /* quote_refresh: insert quote success, md changed before quote_refresh timeout */
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967311,instrument_id:IF1901,direction:0,price:98.40000,volume:10,order_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967315,instrument_id:IF1901,direction:1,price:100.40000,volume:20,order_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    sleep(1);  /*not timeout yet*/
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:110,ask1_volume:20,bid1_price:108,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    sleep(1); /*timer should stop, only one quote(two orders) triggered by md*/
    //should cancel both orders and insert
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967311,instrument_id:IF1901,direction:0,price:98.40000,volume:10,order_status:3,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967315,instrument_id:IF1901,direction:1,price:100.40000,volume:20,order_status:3,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    sleep(5);
    // ASSERT_EQ(2, engine->get_order_cancel_result_count(instance_id));
    ASSERT_EQ(2, engine->get_order_insert_result_count(instance_id));
    check_order(engine->pop_order_insert_result(instance_id), 4294967319, 48, 109, 4,53, 10);
    check_order(engine->pop_order_insert_result(instance_id), 4294967323, 49, 111, 4,53, 10);
    
    // /* quote_refresh: insert quote success, order both canceled, do not trigger quote_refresh timer */
    // engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967319,instrument_id:IF1901,direction:0,price:98.40000,volume:10,order_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    // engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967323,instrument_id:IF1901,direction:1,price:100.40000,volume:20,order_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    // sleep(2);  /*not timeout yet*/
    // engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967319,instrument_id:IF1901,direction:0,price:98.40000,volume:10,order_status:3,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    // engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967323,instrument_id:IF1901,direction:1,price:100.40000,volume:20,order_status:3,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    // sleep(5);
    // ASSERT_EQ(0, engine->get_order_cancel_result_count(instance_id));
    // ASSERT_EQ(0, engine->get_order_insert_result_count(instance_id));

    /********************************************************************/
    /* quote by md, not recv order but recv md*/
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:120,ask1_volume:20,bid1_price:118,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    sleep(1);
    ASSERT_EQ(0, engine->get_order_insert_result_count(instance_id));
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:120,ask1_volume:20,bid1_price:118,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);  /*md base price not changed, do nothing*/
    sleep(1);
    ASSERT_EQ(0, engine->get_order_insert_result_count(instance_id));
    sleep(3);
    engine->stop();
    delete engine;
    delete impl;
}

TEST(auto_order, error_order)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);

    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);

    /* risk params is null, do not check */
    init_instance_param(engine, instance_id);
    ASSERT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));

    //engine->dump();
    engine->start_strategy_instance(instance_id);
    sleep(1);
    ASSERT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    /********************************************************************/
    /* after start, generate first quote */
    /********************************************************************/
    ASSERT_EQ(0, engine->get_order_insert_result_count(instance_id));
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:100,ask1_volume:10,bid1_price:98,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);
    sleep(1);
    ASSERT_EQ(2, engine->get_order_insert_result_count(instance_id));
    check_order(engine->pop_order_insert_result(instance_id), 4294967303, 48, 99, 4, 53, 10);
    check_order(engine->pop_order_insert_result(instance_id), 4294967307, 49, 101, 4, 53, 10);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967303,instrument_id:IF1901,direction:0,price:98.40000,volume:10,order_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false); //in book
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967307,instrument_id:IF1901,direction:0,price:98.40000,volume:10,order_status:4,portfolio_id:4,strategy_instance_id:65536,error_id:0",false); //in book
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967303,instrument_id:IF1901,direction:0,price:98.40000,volume:10,order_status:3,portfolio_id:4,strategy_instance_id:65536,error_id:0",false); //in book
    sleep(5);
    check_order(engine->pop_order_insert_result(instance_id), 4294967311, 48, 99, 4, 53, 10);
    check_order(engine->pop_order_insert_result(instance_id), 4294967315, 49, 101, 4, 53, 10);    
    engine->stop();
    delete engine;
    delete impl;
}

TEST(auto_order, order_trade) {
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);

    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);

    /* risk params is null, do not check */
    init_instance_param(engine, instance_id);
    ASSERT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));

    //engine->dump();
    engine->start_strategy_instance(instance_id);
    sleep(1);
    ASSERT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    /********************************************************************/
    /* after start, generate first quote */
    /********************************************************************/
    ASSERT_EQ(0, engine->get_order_insert_result_count(instance_id));
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:100,ask1_volume:10,bid1_price:98,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);
    sleep(1);
    ASSERT_EQ(2, engine->get_order_insert_result_count(instance_id));
    check_order(engine->pop_order_insert_result(instance_id), 4294967303, 48, 99, 4, 53, 10);
    check_order(engine->pop_order_insert_result(instance_id), 4294967307, 49, 101, 4, 53, 10);

    /********************************************************************/
    /* generate trade */
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967303,instrument_id:IF1901,direction:0,price:98.40000,volume:10,order_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false); //in book
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967307,instrument_id:IF1901,direction:0,price:98.40000,volume:10,order_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false); //in book
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967303,instrument_id:IF1901,direction:0,price:98.40000,volume:10,order_status:5,portfolio_id:4,strategy_instance_id:65536,error_id:0",false); //part trade
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967303,instrument_id:IF1901,direction:0,price:98.40000,volume:10,order_status:6,portfolio_id:4,strategy_instance_id:65536,error_id:0",false); //part trade
    engine->add_or_update(cffex::mm::MSG_TRADE, "instrument_id:IF1901,volume:10,price:95,direction:0,trading_account_id:1,order_id:4294967303,portfolio_id:4,strategy_instance_id:65536",false);  // bid order trade

    /* cancel quote wait delay_quoting and then insert order*/
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_ORDER, "trading_account_id:1,order_id:4294967307,instrument_id:IF1901,direction:0,price:101.00000,volume:10,order_status:3,portfolio_id:4,strategy_instance_id:65536,error_id:0",false); //in book
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:100,ask1_volume:10,bid1_price:98,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);    
    sleep(1);  /*wait for timeout*/
    ASSERT_EQ(0, engine->get_order_insert_result_count(instance_id));
    sleep(3);
    check_order(engine->pop_order_insert_result(instance_id), 4294967311, 48, 99, 4, 53, 10);
    check_order(engine->pop_order_insert_result(instance_id), 4294967315, 49, 101, 4, 53, 10);

    engine->stop();
    delete engine;
    delete impl;
}