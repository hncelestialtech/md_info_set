#include "init_data.h"

TEST(price, market_data)
{
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    int instance_id =  engine->create_strategy_instance(STRATEGY_NAME, TRADING_ACCOUNT_ID, impl);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, engine->get_state(instance_id));
    init_engine_data(engine);
    init_instance_param(engine, instance_id);
    engine->update_strategy_instance_param(instance_id,"0,1,mode,1");
    engine->start_strategy_instance(instance_id);
    sleep(1);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, engine->get_state(instance_id));
    engine->add_or_update(cffex::mm::MSG_MD, "instrument_id:IF1901,ask1_price:106,ask1_volume:10,bid1_price:100,bid1_volume:20,ask2_volume:9,bid2_volume:1,last_price:100,upper_limit_price:120,down_limit_price:80", false);
    sleep(1);
    ASSERT_EQ(2, engine->get_order_insert_result_count(instance_id));
    sleep(1);
    check_order(engine->pop_order_insert_result(instance_id), 4294967303, 48, 101, 4, 53, 10);
    check_order(engine->pop_order_insert_result(instance_id), 4294967307, 49, 107, 4, 53, 10);
    delete impl;
    delete engine;
}

