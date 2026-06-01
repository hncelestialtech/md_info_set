
#include "init_data.h"

TEST(auto_quote, quote_refresh) {
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);

    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);

    /* risk params is null, do not check */
    init_instance_param(engine, instance_id);
    engine->update_strategy_instance_param(instance_id,"0,1,quote_mode,1");  //order mode
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));

    //engine->dump();
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    /********************************************************************/
    /* after start, generate first quote
        base_price = (100+98)/2 = 99
        shift: 99 + 2*0.5 = 100
        spread: 100-98 = 2
        bid = 100-1 = 98.4  ask = 100+1 = 101
     */
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:100,ask1_volume:10,bid1_price:98,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 99, 101, 4);

    /********************************************************************/
    /* quote_refresh: insert quote success, start quote_refresh timer */
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10001, ask_order_id:10002, quote_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    sleep(4);  /*wait for timeout*/
    //cancel order and insert
    // EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10001, ask_order_id:10002, quote_status:4,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    sleep(1);
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 99, 101, 4);

    /* quote_refresh: insert quote success, md changed before quote_refresh timeout */
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967304,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10003, ask_order_id:10004, quote_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    sleep(1);  /*not timeout yet*/
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:110,ask1_volume:20,bid1_price:108,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    sleep(1); /*timer should stop, only one quote triggered by md*/
    //should cancel both orders and insert
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967304,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10003, ask_order_id:10004, quote_status:4,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    sleep(5);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967308, 109, 111, 4);

    /********************************************************************/
    /* quote by md, not recv order but recv md*/
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967308,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10003, ask_order_id:10004, quote_status:0,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:120,ask1_volume:20,bid1_price:118,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:120,ask1_volume:20,bid1_price:118,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);  /*md base price not changed, do nothing*/
    sleep(1);
    EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));

    sleep(3);
    engine->stop();
    delete engine;
    delete impl;
}

TEST(auto_quote, quote_param) {
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);

    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);

    /* risk params is null, do not check */
    init_instance_param(engine, instance_id);
    engine->update_strategy_instance_param(instance_id,"0,1,quote_mode,1");  //order mode
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));

    //engine->dump();
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    /********************************************************************/
    /* after start, generate first quote */
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:100,ask1_volume:10,bid1_price:98,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 99, 101, 4);

    /********************************************************************/
    /* instrument group param changed, cancel and requote */
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10001, ask_order_id:10002, quote_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "custom_id:group1,param_key:spread_mode,param_value:1,param_type:int,status:1",false);  // spread mode change: FIX_SPREAD_MODE(fix_spread:10)
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10001, ask_order_id:10002, quote_status:4,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    sleep(1);
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 95, 105, 4);
    /********************************************************************/
    /* instrument group param changed, close auto_quote_switch, cancel all */
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967304,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10001, ask_order_id:10002, quote_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "custom_id:group1,param_key:auto_quote_switch,param_value:0,param_type:int,status:1",false);
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967304,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10001, ask_order_id:10002, quote_status:4,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    sleep(1);

    /********************************************************************/
    /* instrument group param changed, re-open auto_quote_switch, start quote */
    /********************************************************************/
    // engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "custom_id:group1,param_key:auto_quote_switch,param_value:1,param_type:int,status:1",false);
    // sleep(1);
    // EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    // check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 95, 105, 4);


    engine->stop();
    delete engine;
    delete impl;
}


TEST(auto_quote, quote_trade) {
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);

    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);

    /* risk params is null, do not check */
    init_instance_param(engine, instance_id);
    engine->update_strategy_instance_param(instance_id,"0,1,quote_mode,1");  //order mode
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));

    //engine->dump();
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    /********************************************************************/
    /* after start, generate first quote */
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:100,ask1_volume:10,bid1_price:98,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 99, 101, 4);
    
    /********************************************************************/
    /* generate quote trade */
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10001, ask_order_id:10002, quote_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false); // inbook
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10001, ask_order_id:10002, quote_status:6,portfolio_id:4,strategy_instance_id:65536,error_id:0",false); // part traded
    engine->add_or_update(cffex::mm::MSG_TRADE, "instrument_id:IF1901,volume:10,price:95,direction:0,order_id:10001,portfolio_id:4,strategy_instance_id:65536",false);  // bid order trade

    /* cancel quote wait delay_quoting and then insert order*/
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10001, ask_order_id:10002, quote_status:4,portfolio_id:4,strategy_instance_id:65536,error_id:0",false); // inbook
    ASSERT_EQ(0, engine->get_order_insert_result_count(instance_id));
    sleep(3);
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 99, 101, 4);

    engine->stop();
    delete engine;
    delete impl;
}


// TEST(auto_quote, quote_risk) {
//     cffex::mm::engine *engine = cffex::mm::engine_factory::create();
//     init_engine_data(engine);

//     cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
//     int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);

//     /* risk params is null, do not check */
//     init_instance_param(engine, instance_id);
//     EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));

//     /********************************************************************/
//     /* check md spread */
//     /********************************************************************/
//     /* update risk param */
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_spread,10"); //10*0.2 = 2
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_spread_type,0"); //tick
//     engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:121,ask1_volume:20,bid1_price:118,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false); //failed
//     engine->start_strategy_instance(instance_id);
//     sleep(1);
//     EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));
//     EXPECT_EQ(0, engine->get_quote_cancel_result_count(instance_id));

//     engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:120,ask1_volume:20,bid1_price:118,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false); //success
//     sleep(1);
//     EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
//     EXPECT_EQ(0, engine->get_quote_cancel_result_count(instance_id));
//     cffex::mm::i_quote_entity *quote = engine->pop_quote_insert_result(instance_id);
//     EXPECT_EQ(0, strcmp(quote->get_instrument_id(), "IF1901") );
//     EXPECT_EQ(quote->get_bid_volume(), 10);
//     EXPECT_EQ(quote->get_ask_volume(), 20);
//     EXPECT_DOUBLE_EQ(quote->get_bid_price(), 118.4);
//     EXPECT_DOUBLE_EQ(quote->get_ask_price(), 120.4);

//     engine->pause_strategy_instance(instance_id);
//     sleep(1);
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_spread,1"); //1
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_spread_type,1"); //abs
//     engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:120,ask1_volume:20,bid1_price:118,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);  //failed
//     engine->start_strategy_instance(instance_id);
//     sleep(1);
//     EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));
//     EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));  //caused by stop
//     engine->pop_quote_cancel_result(instance_id);

//     /********************************************************************/
//     /* check md volume */
//     /********************************************************************/
//     engine->pause_strategy_instance(instance_id);  //cancel last quote
//     sleep(1);

//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_spread,");
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_spread_type,");
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_bid_depth,2");
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_ask_depth,5");
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_bid_volume,10");
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_ask_volume,30");
//     engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:120,ask1_volume:5,ask2_price:121,ask2_volume:5,ask3_price:122,ask3_volume:5,ask4_price:123,ask4_volume:5,ask5_price:124,ask5_volume:5,bid1_price:118,bid1_volume:10,bid2_price:117,bid2_volume:10,bid3_price:116,bid3_volume:10,bid4_price:115,bid4_volume:10,bid5_price:114,bid5_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false); // ask1~5: 25 < 30  bid1~2:20 > 10
//     engine->start_strategy_instance(instance_id);
//     sleep(1);
//     EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));
//     EXPECT_EQ(0, engine->get_quote_cancel_result_count(instance_id));

//     engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:120,ask1_volume:10,ask2_price:121,ask2_volume:10,ask3_price:122,ask3_volume:10,ask4_price:123,ask4_volume:10,ask5_price:124,ask5_volume:10,bid1_price:118,bid1_volume:4,bid2_price:117,bid2_volume:4,bid3_price:116,bid3_volume:4,bid4_price:115,bid4_volume:4,bid5_price:114,bid5_volume:4,last_price:100,upper_limit_price:120,down_limit_price:80",false); // ask1~5: 50 > 30  bid1~2:8 > 10
//     sleep(1);
//     EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));
//     EXPECT_EQ(0, engine->get_quote_cancel_result_count(instance_id));

//     engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:119,ask1_volume:10,ask2_price:121,ask2_volume:10,ask3_price:122,ask3_volume:10,ask4_price:123,ask4_volume:10,ask5_price:124,ask5_volume:10,bid1_price:118,bid1_volume:5,bid2_price:117,bid2_volume:5,bid3_price:116,bid3_volume:5,bid4_price:115,bid4_volume:5,bid5_price:114,bid5_volume:5,last_price:100,upper_limit_price:120,down_limit_price:80",false); // ask1~5: 50 > 30  bid1~2:10 = 10
//     sleep(1);
//     EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
//     EXPECT_EQ(0, engine->get_quote_cancel_result_count(instance_id));
//     engine->pop_quote_insert_result(instance_id);


//     /********************************************************************/
//     /* check net position */
//     /********************************************************************/
//     engine->pause_strategy_instance(instance_id);
//     sleep(1);
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_spread,");
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_spread_type,");
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_bid_depth,");
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_ask_depth,");
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_bid_volume,");
//     engine->update_strategy_instance_param(instance_id,"0,1,md_threshold_ask_volume,");
//     engine->update_strategy_instance_param(instance_id,"0,1,position_threshold_upper,10");
//     engine->update_strategy_instance_param(instance_id,"0,1,position_threshold_lower,-5");

//     engine->add_or_update(cffex::mm::MSG_POSITION, "trading_account_id:1,instrument_id:IF1901,long_position:20,short_position:9",false); // net 11
//     engine->start_strategy_instance(instance_id);
//     sleep(1);
//     EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));
//     EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));  //caused by stop
//     engine->pop_quote_cancel_result(instance_id);

//     engine->add_or_update(cffex::mm::MSG_POSITION, "trading_account_id:1,instrument_id:IF1901,long_position:0,short_position:6",false); // net -6
//     engine->start_strategy_instance(instance_id);
//     sleep(1);
//     EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));
//     EXPECT_EQ(0, engine->get_quote_cancel_result_count(instance_id));

//     engine->add_or_update(cffex::mm::MSG_POSITION, "trading_account_id:1,instrument_id:IF1901,long_position:0,short_position:1",false); // net -1
//     engine->start_strategy_instance(instance_id);
//     sleep(1);
//     EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
//     EXPECT_EQ(0, engine->get_quote_cancel_result_count(instance_id));
//     engine->pop_quote_insert_result(instance_id);
//     /********************************************************************/
//     /* check trading section */
//     /********************************************************************/
//     engine->pause_strategy_instance(instance_id);
//     sleep(1);
//     engine->update_strategy_instance_param(instance_id,"0,1,position_threshold_upper,");
//     engine->update_strategy_instance_param(instance_id,"0,1,position_threshold_lower,");
//     engine->update_strategy_instance_param(instance_id,"0,1,trading_section,cffex");
//     engine->add_or_update(cffex::mm::MSG_TRADING_TIME_TEMPLATE, "trading_account_id:1,template_name:cffex,template_value:[2000-4000],status:1",false);
//     engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:120,ask1_volume:20,bid1_price:118,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80, update_sec:1000",false); //out of trading time
//     engine->start_strategy_instance(instance_id);
//     sleep(1);
//     EXPECT_EQ(0, engine->get_quote_insert_result_count(instance_id));
//     EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));  //caused by stop
//     engine->pop_quote_cancel_result(instance_id);

//     engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:120,ask1_volume:20,bid1_price:118,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80, update_sec:3000",false); //in trading time
//     sleep(1);
//     EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));

// }


TEST(auto_quote, quote_error) {
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);

    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);

    /* risk params is null, do not check */
    init_instance_param(engine, instance_id);
    engine->update_strategy_instance_param(instance_id,"0,1,quote_mode,1");  //order mode
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));

    //engine->dump();
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    /********************************************************************/
    /* after start, generate first quote */
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:100,ask1_volume:10,bid1_price:98,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);
    sleep(1);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 99, 101, 4);


    /********************************************************************/
    /* first quote error, delay and do requote */
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10001, ask_order_id:10002, quote_status:5,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    sleep(5);  /*wait for timeout*/
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 99, 101, 4);

    engine->stop();
    delete engine;
    delete impl;   
}

// TEST(auto_quote, quote_cancel) {
//     cffex::mm::engine *engine = cffex::mm::engine_factory::create();
//     init_engine_data(engine);

//     cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
//     int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);

//     /* risk params is null, do not check */
//     init_instance_param(engine, instance_id);
//     EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));

//     //engine->dump();
//     engine->start_strategy_instance(instance_id);
//     sleep(1);
//     EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

//     /********************************************************************/
//     /* after start, generate first quote */
//     /********************************************************************/
//     EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
//     cffex::mm::i_quote_entity *quote = engine->pop_quote_insert_result(instance_id);
//     printf("quote[%s]\n", quote->to_string().c_str());
//     EXPECT_EQ(0, strcmp(quote->get_instrument_id(), "IF1901") );
//     EXPECT_EQ(quote->get_bid_volume(), 10);
//     EXPECT_EQ(quote->get_ask_volume(), 20);
//     EXPECT_DOUBLE_EQ(quote->get_bid_price(), 98.4);
//     EXPECT_DOUBLE_EQ(quote->get_ask_price(), 100.4);

//     //md update, requote
//     engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10001, ask_order_id:10002, quote_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
//     engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:150,ask1_volume:20,bid1_price:118,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
//     sleep(1);
//     EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));
//     EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
//     quote = engine->pop_quote_cancel_result(instance_id);
//     EXPECT_EQ(quote->get_quote_id(), 4294967300);

//     //cancel failed 1
//     engine->add_or_update(cffex::mm::MSG_TRADING_QUOTE_CANCEL_RSP, "trading_account_id:1,quote_id:4294967300,error_id:1",false);

//     sleep(3);
//     EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));
//     quote = engine->pop_quote_cancel_result(instance_id);
//     EXPECT_EQ(quote->get_quote_id(), 4294967300);

//     //cancel failed 2
//     engine->add_or_update(cffex::mm::MSG_TRADING_QUOTE_CANCEL_RSP, "trading_account_id:1,quote_id:4294967300,error_id:1",false);
//     sleep(3);
//     EXPECT_EQ(1, engine->get_quote_cancel_result_count(instance_id));
//     quote = engine->pop_quote_cancel_result(instance_id);
//     EXPECT_EQ(quote->get_quote_id(), 4294967300);

//     //cancel failed 3
//     engine->add_or_update(cffex::mm::MSG_TRADING_QUOTE_CANCEL_RSP, "trading_account_id:1,quote_id:4294967300,error_id:1",false);
//     sleep(3);
//     EXPECT_EQ(0, engine->get_quote_cancel_result_count(instance_id));  //stop quoting

// }

TEST(auto_quote, quote_mode) {
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);

    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    int instance_id = engine->create_strategy_instance(PLUGIN_NAME, TRADING_ACCOUNT_ID, impl);

    /* risk params is null, do not check */
    init_instance_param(engine, instance_id);
    engine->update_strategy_instance_param(instance_id,"0,1,quote_mode,2");  //QUOTE_POP_MODE_TYPE
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));

    //engine->dump();
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));

    /********************************************************************/
    /* after start, generate first quote */
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:100,ask1_volume:10,bid1_price:98,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);
    sleep(5);
    EXPECT_EQ(1, engine->get_quote_insert_result_count(instance_id));
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 99, 101, 4);
    /********************************************************************/
    /* md changed, insert new quote, do not cancel last quote */
    /********************************************************************/
    engine->add_or_update(cffex::mm::MSG_QUOTE, "trading_account_id:1,quote_id:4294967300,quote_sys_id:1,instrument_id:IF1901,bid_price:99,ask_price:101,bid_volume:10,ask_volume:20, bid_order_id:10001, ask_order_id:10002, quote_status:2,portfolio_id:4,strategy_instance_id:65536,error_id:0",false);
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:110,ask1_volume:20,bid1_price:108,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    sleep(1);
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 109, 111, 4);

    engine->stop();
    delete engine;
    delete impl;   
}
