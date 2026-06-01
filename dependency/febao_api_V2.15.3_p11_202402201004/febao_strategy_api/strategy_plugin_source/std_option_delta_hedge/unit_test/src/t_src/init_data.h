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
 * Date: 2019-12-16
 */

#ifndef INIT_DATA_H
#define INIT_DATA_H

#include <gtest/gtest.h>
#include "engine.h"
#include "msg_id.h"
#include "entity.h"
#include <boost/bind.hpp>
#include "strategy_impl.h"
#include <stdint.h>

static int instance_id = 65536;

static void check_order(cffex::mm::i_order_entity *order, int64_t order_id, int8_t direction, double price, int volume, int portfolio)
{
    printf("order_id[%ld] direction[%d] price[%lf]\n", order_id, direction, price);
    if (order == NULL) {
        printf("has no this order, [%ld]\n", order_id);
        return;
    }
    EXPECT_EQ(order->get_order_id(), order_id);
    EXPECT_EQ(order->get_direction(), direction);
    EXPECT_DOUBLE_EQ(order->get_price(), price);
    EXPECT_EQ(order->get_volume(), volume);
    EXPECT_EQ(order->get_portfolio_id(), portfolio);
}

static void init_strategy_instance_param(cffex::mm::engine *engine)
{
    char buf[1024] = {0};
    strcpy(buf,"template_name:cffex,template_value:[0-3600];[3600-7200],status:1");
    engine->add_or_update(cffex::mm::MSG_TRADING_TIME_TEMPLATE, buf, false);

    engine->update_strategy_instance_param(instance_id,"param_key:portfolio_name,param_value:hedge",false);
    //engine->update_strategy_instance_param(instance_id,"param_key:underlying_id,param_value:IF1902",false);
    engine->update_strategy_instance_param(instance_id,"param_key:underlying_id,param_value:IF1902",false);
    engine->update_strategy_instance_param(instance_id,"param_key:trading_section,param_value:cffex",false);
    engine->update_strategy_instance_param(instance_id,"0,4,trigger_threshold,5");
    engine->update_strategy_instance_param(instance_id,"0,4,target_threshold,3");
    engine->update_strategy_instance_param(instance_id,"0,4,md_max_spread,10");
    engine->update_strategy_instance_param(instance_id,"0,4,max_volume,5");
    engine->update_strategy_instance_param(instance_id,"0,4,interval,5");
}



static void init_table(cffex::mm::engine *engine, int msg, const char **table_str, int count)
{
    for(int i = 0; i < count; i++) {
        engine->add_or_update(msg, table_str[i], false);
    }
}

const char *md_table_str[] = { // cffex::mm::MSG_MD
    "instrument_id:IO1901-C-3000,ask1_price:100,ask1_volume:20,bid1_price:98,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",
    "instrument_id:IO1901-C-3100,ask1_price:100,ask1_volume:20,bid1_price:98,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",
    "instrument_id:IO1902-C-3000,ask1_price:100,ask1_volume:20,bid1_price:98,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80"
};
const char *derived_md_table_str[] = { // cffex::mm::MSG_OPTION_DERIVED_MD
    "instrument_id:IO1901-C-3000,trading_account_id:0,theoretical_price:100",
    "instrument_id:IO1901-C-3100,trading_account_id:0,theoretical_price:100",
    "instrument_id:IO1902-C-3000,trading_account_id:0,theoretical_price:100"
};
const char *portfolio_table_str[] = { // cffex::mm::MSG_PORTFOLIO
    "portfolio_id:1,trading_account_id:0,portfolio_type:0,portfolio_name:default,status:1,last_operator_id:27",
    "portfolio_id:2,trading_account_id:0,portfolio_type:0,portfolio_name:order,status:1,last_operator_id:27",
    "portfolio_id:3,trading_account_id:0,portfolio_type:0,portfolio_name:hedge,status:1,last_operator_id:27"
    /*"portfolio_id:2,trading_account_id:0,portfolio_type:1,portfolio_name:order,status:1,last_operator_id:1",
    "portfolio_id:3,trading_account_id:0,portfolio_type:1,portfolio_name:hedge,status:1,last_operator_id:1",
    "portfolio_id:4,trading_account_id:0,portfolio_type:1,portfolio_name:quote,status:1,last_operator_id:1"*/
};
const char *instrument_table_str[] = { // cffex::mm::MSG_INSTRUMENT
/*
    "instrument_id:IO1901-C-3000,tick:0.2,status:1,product_id:IO,option_serial_id:IO1901",
    "instrument_id:IO1901-C-3100,tick:0.2,status:1,product_id:IO,option_serial_id:IO1901",
    "instrument_id:IO1902-C-3000,tick:0.2,status:1,product_id:IO,option_serial_id:IO1902"
*/
    "exchange_id:0,product_id:IF,instrument_id:IF1902,instrument_name:IF1902,multiple:300,tick:0.2,expire_date:20190118,expire_date_type:1,status:1,last_operator_id:0"

};
const char *position_table_str[] = { // cffex::mm::MSG_POSITION
    "instrument_id:IO1901-C-3000,long_position:1,short_position:1"
};
const char *instrument_param_table_str[] = { // cffex::mm::INSTRUMENT_PARAM_VALUE
    "instrument_id:IO1901-C-3000,trading_account_id:0,param_key:Quoting,param_value:1,last_operator_id:1,last_operate_source:2,status:1",
    "instrument_id:IO1901-C-3000,trading_account_id:0,param_key:BidOffset,param_value:0,last_operator_id:1,last_operate_source:2,status:1",
    "instrument_id:IO1901-C-3000,trading_account_id:0,param_key:AskOffset,param_value:0,last_operator_id:1,last_operate_source:2,status:1",
    "instrument_id:IO1901-C-3100,trading_account_id:0,param_key:Quoting,param_value:1,last_operator_id:1,last_operate_source:2,status:1",
    "instrument_id:IO1901-C-3100,trading_account_id:0,param_key:BidOffset,param_value:0,last_operator_id:1,last_operate_source:2,status:1",
    "instrument_id:IO1901-C-3100,trading_account_id:0,param_key:AskOffset,param_value:0,last_operator_id:1,last_operate_source:2,status:1",
    "instrument_id:IO1902-C-3000,trading_account_id:0,param_key:Quoting,param_value:1,last_operator_id:1,last_operate_source:2,status:1",
    "instrument_id:IO1902-C-3000,trading_account_id:0,param_key:BidOffset,param_value:0,last_operator_id:1,last_operate_source:2,status:1",
    "instrument_id:IO1902-C-3000,trading_account_id:0,param_key:AskOffset,param_value:0,last_operator_id:1,last_operate_source:2,status:1"

    "instrument_id:IO1901-C-3000,trading_account_id:0,param_key:NetPosLimit,param_value:100,last_operator_id:1,last_operate_source:2,status:1",
    "instrument_id:IO1901-C-3000,trading_account_id:0,param_key:UpdateThreshold,param_value:0.5,last_operator_id:1,last_operate_source:2,status:1",
    "instrument_id:IO1901-C-3100,trading_account_id:0,param_key:NetPosLimit,param_value:100,last_operator_id:1,last_operate_source:2,status:1",
    "instrument_id:IO1901-C-3100,trading_account_id:0,param_key:UpdateThreshold,param_value:0.5,last_operator_id:1,last_operate_source:2,status:1",
    "instrument_id:IO1902-C-3000,trading_account_id:0,param_key:NetPosLimit,param_value:100,last_operator_id:1,last_operate_source:2,status:1",
    "instrument_id:IO1902-C-3000,trading_account_id:0,param_key:UpdateThreshold,param_value:0.5,last_operator_id:1,last_operate_source:2,status:1",
};
const char *custom_param_table_str[] = { // cffex::mm::MSG_CUSTOM_PARAM
    "custom_id:IO,trading_account_id:0,param_key:NetPosLimit,param_value:100,last_operator_id:1,last_operate_source:2,status:1",
    "custom_id:IO,trading_account_id:0,param_key:UpdateThreshold,param_value:0.5,last_operator_id:1,last_operate_source:2,status:1",
    "custom_id:IO1901,trading_account_id:0,param_key:QuoteTemplate,param_value:template1,last_operator_id:1,last_operate_source:2,status:1",
    "custom_id:IO1902,trading_account_id:0,param_key:QuoteTemplate,param_value:template1,last_operator_id:1,last_operate_source:2,status:1"
};
const char *trading_time_template_table[] = {
    //"template_name:cffex,template_value:0,status:1"
    "trading_time_template_id:1,trading_account_id:0,exchange_id:0,template_name:1,template_value:[0-3600];[3600-7200],last_operator_id:27,status:1"
};

const char *spread_template_str[] = {
    "spread_template_id:1,trading_account_id:0,template_name:template1,template_value:[1.0-200.0):10.0:0,template_type:1,status:1"
};

static void init_engine_data(cffex::mm::engine *engine) {
    init_table(engine, cffex::mm::MSG_MD, md_table_str, sizeof(md_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_OPTION_DERIVED_MD, derived_md_table_str, sizeof(derived_md_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_PORTFOLIO, portfolio_table_str, sizeof(portfolio_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_INSTRUMENT, instrument_table_str, sizeof(instrument_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_POSITION, position_table_str, sizeof(position_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_INSTRUMENT_PARAM, instrument_param_table_str, sizeof(instrument_param_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_CUSTOM_PARAM, custom_param_table_str, sizeof(custom_param_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_SPREAD_TEMPLATE, spread_template_str, sizeof(spread_template_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_TRADING_TIME_TEMPLATE, trading_time_template_table, sizeof(trading_time_template_table)/sizeof(const char*));
}


#endif