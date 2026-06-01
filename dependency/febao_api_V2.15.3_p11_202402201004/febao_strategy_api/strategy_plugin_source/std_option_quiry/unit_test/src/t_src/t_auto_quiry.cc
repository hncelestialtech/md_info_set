/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: cc
 * Date: 2019-12-26
 */

#include <gtest/gtest.h>
#include "engine.h"
#include "msg_id.h"
#include "entity.h"
#include "strategy_impl.h"
#include "test_init_data.h"

typedef void (*t_func)();
typedef void (*test_case_func)(cffex::mm::engine *engine,cffex::strategy::strategy_impl *impl,uint32_t instance_id);

void run_test_strategy(const char* test_case,test_case_func f)
{
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    //cffex::mm::strategy_config::get_instance()->set_node_id(1);
    uint32_t instance_id = engine->create_strategy_instance(PLUGIN_NAME, 0, impl); // 65536
    printf("%s -- create_instance plugin_name[%s] instance_id[%u]\n",test_case,PLUGIN_NAME,instance_id);
    (*f)(engine,impl,instance_id);
    engine->stop();
    //sleep(2);
    delete engine;
    delete impl;
}

void init_instance_param(cffex::mm::engine *engine, int instance_id)
{
    engine->update_strategy_instance_param(instance_id,"0,1,portfolio,quote");
    engine->update_strategy_instance_param(instance_id,"0,1,volume,1");
    engine->update_strategy_instance_param(instance_id,"0,1,product,IO");
    engine->update_strategy_instance_param(instance_id,"0,1,trading_section,");
    engine->update_strategy_instance_param(instance_id,"0,1,waiting_second,1");
    engine->update_strategy_instance_param(instance_id,"0,1,duration_second,3");
    engine->update_strategy_instance_param(instance_id,"0,1,netpos_limit,-1");
}

/*
void test_1(cffex::mm::engine *engine,cffex::strategy::strategy_impl *impl,uint32_t instance_id){
    printf("%s,hello\n",__FUNCTION__);
}

TEST(quiry, test){
    run_test_strategy("test_1",test_1);
}
*/

void test_quiry_on_md(cffex::mm::engine *engine,cffex::strategy::strategy_impl *impl,uint32_t instance_id){
    init_engine_data(engine);
    init_instance_param(engine,instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    engine->update_strategy_instance_param(instance_id,"0,1,trading_section,CFFEX");
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    sleep(1);

    // not in trading_section
    engine->add_or_update(cffex::mm::MSG_TRADING_DAY, "exchange_id:0,exchange_name:CFFEX,trading_day:20190115,trading_sec:32399");
    engine->add_or_update(cffex::mm::MSG_INQUIRY_QUOTE, "inquiry_id:1,product_id:IO,instrument_id:IO1902-C-3000,inquiry_quote_status:0",false);
    sleep(2);
    EXPECT_EQ(0,engine->get_order_insert_result_count(instance_id));

    // invalid market price
    engine->update_strategy_instance_param(instance_id,"0,1,trading_section");
    engine->start_strategy_instance(instance_id);
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:,ask1_volume:20,bid1_price:98,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    engine->add_or_update(cffex::mm::MSG_INQUIRY_QUOTE, "inquiry_id:2,product_id:IO,instrument_id:IO1902-C-3000,inquiry_quote_status:0",false);
    sleep(2);
    EXPECT_EQ(0,engine->get_order_insert_result_count(instance_id));

    // invalid spread
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:100,ask1_volume:20,bid1_price:98,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "instrument_id:IO1902,trading_account_id:0,param_key:quiry_template,param_value:spread1,last_operator_id:1,last_operate_source:2,status:1",false);
    //engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "custom_id:IO1902,param_key:quiry_template,param_type:string,param_value:spread1,status:1",false);
    engine->add_or_update(cffex::mm::MSG_INQUIRY_QUOTE, "inquiry_id:3,product_id:IO,instrument_id:IO1902-C-3000,inquiry_quote_status:0",false);
    sleep(2);
    EXPECT_EQ(0,engine->get_order_insert_result_count(instance_id));

    // invalid netpos_limit
    engine->update_strategy_instance_param(instance_id,"0,1,netpos_limit,-2");
    engine->start_strategy_instance(instance_id);
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "instrument_id:IO1902,trading_account_id:0,param_key:quiry_template,param_value:template1,last_operator_id:1,last_operate_source:2,status:1",false);
    //engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "custom_id:IO1902,param_key:quiry_template,param_type:string,param_value:template1,status:1",false);
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IO1902-C-3000,ask1_price:100,ask1_volume:20,bid1_price:98,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",false);
    engine->add_or_update(cffex::mm::MSG_INQUIRY_QUOTE, "inquiry_id:4,product_id:IO,instrument_id:IO1902-C-3000,inquiry_quote_status:0",false);
    sleep(2);
    EXPECT_EQ(0,engine->get_order_insert_result_count(instance_id));

    // max position
    engine->update_strategy_instance_param(instance_id,"0,1,netpos_limit,1");
    engine->start_strategy_instance(instance_id);
    engine->add_or_update(cffex::mm::MSG_POSITION, "instrument_id:IO1902-C-3000,long_position:2147483647,short_position:0,yd_long_position:2147483647,yd_short_position:0",false);
    engine->add_or_update(cffex::mm::MSG_INQUIRY_QUOTE, "inquiry_id:5,product_id:IO,instrument_id:IO1902-C-3000,inquiry_quote_status:0",false);
    sleep(2);
    EXPECT_EQ(0,engine->get_order_insert_result_count(instance_id));

    // invalid volume
    engine->update_strategy_instance_param(instance_id,"0,1,volume,-1");
    engine->start_strategy_instance(instance_id);
    engine->add_or_update(cffex::mm::MSG_POSITION, "instrument_id:IO1902-C-3000,long_position:0,short_position:0,yd_long_position:0,yd_short_position:0",false);
    engine->add_or_update(cffex::mm::MSG_INQUIRY_QUOTE, "inquiry_id:6,product_id:IO,instrument_id:IO1902-C-3000,inquiry_quote_status:0",false);
    sleep(2);
    EXPECT_EQ(0,engine->get_order_insert_result_count(instance_id));

    // netpos_limit
    engine->update_strategy_instance_param(instance_id,"0,1,volume,1");
    engine->update_strategy_instance_param(instance_id,"0,1,netpos_limit,5");
    engine->start_strategy_instance(instance_id);
    engine->add_or_update(cffex::mm::MSG_POSITION, "instrument_id:IO1902-C-3000,long_position:5,short_position:0,yd_long_position:5,yd_short_position:0",false);
    engine->add_or_update(cffex::mm::MSG_INQUIRY_QUOTE, "inquiry_id:7,product_id:IO,instrument_id:IO1902-C-3000,inquiry_quote_status:0",false);
    sleep(2);
    EXPECT_EQ(0,engine->get_order_insert_result_count(instance_id));

    // inquiry_quote
    engine->update_strategy_instance_param(instance_id,"0,1,volume,1");
    engine->update_strategy_instance_param(instance_id,"0,1,netpos_limit,-1");
    engine->update_strategy_instance_param(instance_id,"0,1,trading_section,");
    engine->start_strategy_instance(instance_id);
    sleep(1);
    engine->add_or_update(cffex::mm::MSG_INQUIRY_QUOTE, "inquiry_id:8,product_id:IO,instrument_id:IO1902-C-3000,inquiry_quote_status:0",false);
    sleep(2);
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 98, 100, 1, "IO1902-C-3000");
    engine->pause_strategy_instance(instance_id);
}

void test_quiry_on_trade(cffex::mm::engine *engine,cffex::strategy::strategy_impl *impl,uint32_t instance_id){
    init_engine_data(engine);
    init_instance_param(engine,instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    sleep(1);

    // cancel on duration_second timeout
    engine->add_or_update(cffex::mm::MSG_INQUIRY_QUOTE, "inquiry_id:1,product_id:IO,instrument_id:IO1902-C-3000,inquiry_quote_status:0",false);
    sleep(2);
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 98, 100, 1, "IO1902-C-3000");
    engine->add_or_update(cffex::mm::MSG_QUOTE, "quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:100,bid_price:98,quote_status:2,strategy_instance_id:65536",false);
    sleep(4);
    EXPECT_EQ(1,engine->get_quote_cancel_result_count(instance_id));
    engine->pop_quote_cancel_result(instance_id);

    // cancel on trade
    engine->add_or_update(cffex::mm::MSG_INQUIRY_QUOTE, "inquiry_id:2,product_id:IO,instrument_id:IO1902-C-3000,inquiry_quote_status:0",false);
    sleep(2);
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 98, 100, 1, "IO1902-C-3000");
    engine->add_or_update(cffex::mm::MSG_TRADE, "exchange_id:0,trade_id:1,order_id:4294967305,order_sys_id:1,trade_sys_id:1,instrument_id:IO1902-C-3000,strategy_instance_id:65536",false);
    sleep(1);
    EXPECT_EQ(1,engine->get_quote_cancel_result_count(instance_id));
    engine->pop_quote_cancel_result(instance_id);
}


void test_quiry_on_cancel(cffex::mm::engine *engine,cffex::strategy::strategy_impl *impl,uint32_t instance_id){
    init_engine_data(engine);
    init_instance_param(engine,instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    sleep(1);

    // cancel on errorid=1
    engine->add_or_update(cffex::mm::MSG_INQUIRY_QUOTE, "inquiry_id:1,product_id:IO,instrument_id:IO1902-C-3000,inquiry_quote_status:0",false);
    sleep(2);
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967300, 98, 100, 1, "IO1902-C-3000");
        // quote error
    engine->add_or_update(cffex::mm::MSG_QUOTE, "quote_id:4294967300,instrument_id:IO1902-C-3000,ask_price:100,bid_price:98,quote_status:5,strategy_instance_id:65536",false);
    /**
        // cancel & wait(1) for re-cancel
    engine->add_or_update(cffex::mm::MSG_TRADING_QUOTE_CANCEL_RSP, "quote_id:4294967300,quote_sys_id:1,instrument_id:IO1902-C-3000,quote_status:5,error_id:1",false);
    sleep(1);
        // cancel & wait(2) for re-cancel
    engine->add_or_update(cffex::mm::MSG_TRADING_QUOTE_CANCEL_RSP, "quote_id:4294967300,quote_sys_id:1,instrument_id:IO1902-C-3000,quote_status:5,error_id:1",false);
    sleep(1);
        // cancel & wait(3) for re-cancel
    engine->add_or_update(cffex::mm::MSG_TRADING_QUOTE_CANCEL_RSP, "quote_id:4294967300,quote_sys_id:1,instrument_id:IO1902-C-3000,quote_status:5,error_id:1",false);
    sleep(2);
    EXPECT_EQ(3,engine->get_quote_cancel_result_count(instance_id));
    engine->pop_quote_cancel_result(instance_id);
    engine->pop_quote_cancel_result(instance_id);
    engine->pop_quote_cancel_result(instance_id);

    // cancel on errorid=0
    engine->add_or_update(cffex::mm::MSG_INQUIRY_QUOTE, "inquiry_id:2,product_id:IO,instrument_id:IO1902-C-3000,inquiry_quote_status:0",false);
    sleep(2);
    check_quote(engine->pop_quote_insert_result(instance_id), 4294967304, 98, 100, 1, "IO1902-C-3000");
        // quote error
    engine->add_or_update(cffex::mm::MSG_QUOTE, "quote_id:4294967304,instrument_id:IO1902-C-3000,ask_price:100,bid_price:98,quote_status:5,strategy_instance_id:65536",false);
        // cancel quote
    engine->add_or_update(cffex::mm::MSG_TRADING_QUOTE_CANCEL_RSP, "quote_id:4294967304,quote_sys_id:1,instrument_id:IO1902-C-3000,quote_status:5,error_id:0",false);
    sleep(1);
    EXPECT_EQ(0,engine->get_quote_cancel_result_count(instance_id));
    */
}

TEST(quiry, on_md){
    run_test_strategy("quiry_on_md",test_quiry_on_md);
}

TEST(quiry, on_trade){
    run_test_strategy("quiry_on_trade",test_quiry_on_trade);
}

TEST(quiry, on_cancel){
    run_test_strategy("quiry_on_cancel",test_quiry_on_cancel);
}
