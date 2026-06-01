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

const char *portfolio_table_str[] = {
    "trading_account_id:1,portfolio_id:2,portfolio_name:order,status:1,portfolio_type:1",
    "trading_account_id:1,portfolio_id:3,portfolio_name:hedge,status:1,portfolio_type:1",
    "trading_account_id:1,portfolio_id:4,portfolio_name:quote,status:1,portfolio_type:1"
};

const char *instrument_table_str[] = {
    "instrument_id:IF1902,tick:0.5,status:1",
    "instrument_id:IF1901,tick:0.5,status:1"
};

const char *position_table_str[] = { // cffex::mm::MSG_POSITION
    "trading_account_id:1,instrument_id:IF1902,long_position:1,short_position:1"
};

const char *trading_time_template_table_str[] = {
    "trading_account_id:1,template_name:period1,template_value:[0-82800],status:1"
};

void init_engine_data(cffex::mm::engine *engine)
{
    init_table(engine, cffex::mm::MSG_PORTFOLIO, portfolio_table_str, sizeof(portfolio_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_INSTRUMENT, instrument_table_str, sizeof(instrument_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_TRADING_TIME_TEMPLATE, trading_time_template_table_str, sizeof(trading_time_template_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_POSITION, position_table_str, sizeof(position_table_str)/sizeof(const char*));
    engine->init(cffex::mm::MSG_CUSTOM_PARAM_DEFINE, "trading_account_id:1,param_key:hedge_buy_shift,param_type:int,status:1", false);
    engine->init(cffex::mm::MSG_CUSTOM_PARAM_DEFINE, "trading_account_id:1,param_key:hedge_sell_shift,param_type:int,status:1", false);
    engine->init(cffex::mm::MSG_CUSTOM_PARAM_DEFINE, "trading_account_id:1,param_key:rehedge_shift,param_type:int,status:1", false);
    engine->init(cffex::mm::MSG_CUSTOM_PARAM_LEVEL, "trading_account_id:1,custom_id:custom1,status:1", false);
    engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "trading_account_id:1,custom_id:custom1,param_key:hedge_buy_shift,param_value:0,status:1", false);
    engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "trading_account_id:1,custom_id:custom1,param_key:hedge_sell_shift,param_value:0,status:1", false);
    engine->add_or_update(cffex::mm::MSG_CUSTOM_PARAM, "trading_account_id:1,custom_id:custom1,param_key:rehedge_shift,param_value:2,status:1", false);
}
