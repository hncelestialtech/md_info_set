/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: fy
 * Date: 2019-12-16
 */

#include "init_data.h"

TEST(auto_hedge_rule, snap_risk_trigger) {
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    //engine->init_csv_path("./csv_case/");
    sleep(3);
    char buf[1024] = {0};
    instance_id =  engine->create_strategy_instance("option_delta_hedge", 0,  impl);
    uint8_t state = engine->get_state(instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, state);

    init_strategy_instance_param(engine);
    strcpy(buf, "portfolio_id:3,delta:6");
    engine->add_or_update(cffex::mm::MSG_PORTFOLIO_RISK, buf, false);

    memset(buf, 0, sizeof(buf));
    strcpy(buf, "instrument_id:IF1902,trading_account_id:0,baseprice:3000");
    engine->add_or_update(cffex::mm::MSG_FUTURE_DERIVED_MD, buf, false);

    memset(buf, 0, sizeof(buf));
    strcpy(buf,"instrument_id:IF1902,ask1_price:100,ask1_volume:20,bid1_price:99,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80");
    engine->add_or_update(cffex::mm::MSG_MD, buf, false);
    sleep(3);
    /*======= case0. snap_risk trigger hedge ====== */
    engine->start_strategy_instance(instance_id);
    sleep(1);
    state = engine->get_state(instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, state);

    sleep(2);
    check_order(engine->pop_order_insert_result(instance_id), 4294967303, '1', 99, 4, 3);

    sleep(2);
    delete impl;
    delete engine;
}

//one order msg_risk_trigger
TEST(auto_hedge_rule, msg_risk_trigger) {
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    //engine->init_csv_path("./csv_case/");
    sleep(3);
    char buf[1024] = {0};
    instance_id =  engine->create_strategy_instance("option_delta_hedge", 0,  impl);
    uint8_t state = engine->get_state(instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, state);

    init_strategy_instance_param(engine);
    strcpy(buf, "portfolio_id:3,delta:2");
    engine->add_or_update(cffex::mm::MSG_PORTFOLIO_RISK, buf, false);

    memset(buf, 0, sizeof(buf));
    strcpy(buf, "instrument_id:IF1902,trading_account_id:0,baseprice:3000");
    engine->add_or_update(cffex::mm::MSG_FUTURE_DERIVED_MD, buf, false);

    memset(buf, 0, sizeof(buf));
    strcpy(buf,"instrument_id:IF1902,ask1_price:100,ask1_volume:20,bid1_price:99,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80");
    engine->add_or_update(cffex::mm::MSG_MD, buf, false);
    sleep(2);
    /*======= case0. snap_risk trigger hedge ====== */
    engine->start_strategy_instance(instance_id);
    sleep(1);
    state = engine->get_state(instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, state);

    /*======= case1. msg_risk trigger hedge ====== */
    memset(buf, 0, sizeof(buf));
    strcpy(buf, "portfolio_id:3,delta:7");
    engine->add_or_update(cffex::mm::MSG_PORTFOLIO_RISK, buf, false);
    sleep(1);
    check_order(engine->pop_order_insert_result(instance_id), 4294967303, '1', 99, 5, 3);

    memset(buf, 0, sizeof(buf));
    strcpy(buf, "portfolio_id:3,delta:-7");
    engine->add_or_update(cffex::mm::MSG_PORTFOLIO_RISK, buf, false);
    sleep(6);
    check_order(engine->pop_order_insert_result(instance_id), 4294967307, '0', 100, 5, 3);

    sleep(1);

    delete impl;
    delete engine;
}

TEST(auto_hedge_rule, md_spread) {
    cffex::strategy::strategy_impl *impl = new cffex::strategy::strategy_impl();
    cffex::mm::engine *engine = cffex::mm::engine_factory::create();
    init_engine_data(engine);
    //engine->init_csv_path("./csv_case/");
    sleep(3);
    char buf[1024] = {0};
    instance_id =  engine->create_strategy_instance("option_delta_hedge", 0,  impl);
    uint8_t state = engine->get_state(instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_INIT_STAT, state);

    init_strategy_instance_param(engine);
    strcpy(buf, "portfolio_id:3,delta:6");
    engine->add_or_update(cffex::mm::MSG_PORTFOLIO_RISK, buf, false);

    memset(buf, 0, sizeof(buf));
    strcpy(buf, "instrument_id:IF1902,trading_account_id:0,baseprice:3000");
    engine->add_or_update(cffex::mm::MSG_FUTURE_DERIVED_MD, buf, false);

    memset(buf, 0, sizeof(buf));
    strcpy(buf,"instrument_id:IF1902,ask1_price:102,ask1_volume:20,bid1_price:99,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80");
    engine->add_or_update(cffex::mm::MSG_MD, buf, false);
    sleep(3);
    /*======= case0. snap_risk trigger hedge ====== */
    engine->start_strategy_instance(instance_id);
    sleep(1);
    state = engine->get_state(instance_id);
    EXPECT_EQ(cffex::mm::STRATEGY_INSTANCE_RUNNING_STAT, state);

    sleep(1);
    EXPECT_EQ(engine->get_order_insert_result_count(instance_id), 0);

    sleep(2);
    delete impl;
    delete engine;
}
