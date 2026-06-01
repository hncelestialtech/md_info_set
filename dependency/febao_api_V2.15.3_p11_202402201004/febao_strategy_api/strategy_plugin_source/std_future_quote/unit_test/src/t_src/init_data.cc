#include "init_data.h"

const char *portfolio_table_str[] = {
    "trading_account_id:1,portfolio_id:2,portfolio_name:order,status:1,portfolio_type:1",
    "trading_account_id:1,portfolio_id:3,portfolio_name:hedge,status:1,portfolio_type:1",
    "trading_account_id:1,portfolio_id:4,portfolio_name:quote,status:1,portfolio_type:1"
};

const char *instrument_table_str[] = {
    "exchange_id:0,product_id:IF,instrument_id:IF1902,tick:0.5,status:1",
    "exchange_id:0,product_id:IF,instrument_id:IF1901,tick:0.5,status:1"
};

const char *portfolio_position_profit_table_str[] = {
    "instrument_id:IF1902,portfolio_id:2,td_market_profit:10",
    "instrument_id:IF1902,portfolio_id:3,td_market_profit:5"
};

const char *trading_time_template_table_str[] = {
    "trading_account_id:1,template_name:period1,template_value:[0-82800],status:1"
};

const char *custom_param_table_str[] = { // cffex::mm::MSG_CUSTOM_PARAM
    "custom_id:group1,trading_account_id:1,param_key:auto_quote_switch,param_value:1,status:1",
    "custom_id:group1,trading_account_id:1,param_key:baseprice_shift,param_value:2,status:1",
    "custom_id:group1,trading_account_id:1,param_key:spread_mode,param_value:0,status:1",
    "custom_id:group1,trading_account_id:1,param_key:fix_spread,param_value:10,status:1",
    "custom_id:group1,trading_account_id:1,param_key:spread_template_name,param_value:temp1,status:1"
};

const char *spread_template_table[] = {
};


void init_engine_data(cffex::mm::engine *engine) {
    init_table(engine, cffex::mm::MSG_PORTFOLIO_POSITION_PROFIT, portfolio_position_profit_table_str, sizeof(portfolio_position_profit_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_PORTFOLIO, portfolio_table_str, sizeof(portfolio_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_INSTRUMENT, instrument_table_str, sizeof(instrument_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_TRADING_TIME_TEMPLATE, trading_time_template_table_str, sizeof(trading_time_template_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_SPREAD_TEMPLATE, spread_template_table, sizeof(spread_template_table)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_CUSTOM_PARAM, custom_param_table_str, sizeof(custom_param_table_str)/sizeof(const char*));

    engine->init(cffex::mm::MSG_CUSTOM_PARAM_DEFINE, "trading_account_id:1,param_key:auto_quote_switch,param_type:int,status:1", false);
    engine->init(cffex::mm::MSG_CUSTOM_PARAM_DEFINE, "trading_account_id:1,param_key:baseprice_shift,param_type:double,status:1", false);
    engine->init(cffex::mm::MSG_CUSTOM_PARAM_DEFINE, "trading_account_id:1,param_key:spread_mode,param_type:int,status:1", false);
    engine->init(cffex::mm::MSG_CUSTOM_PARAM_DEFINE, "trading_account_id:1,param_key:fix_spread,param_type:double,status:1", false);
    engine->init(cffex::mm::MSG_CUSTOM_PARAM_DEFINE, "trading_account_id:1,param_key:spread_template_name,param_type:string,status:1", false);

    engine->init(cffex::mm::MSG_CUSTOM_PARAM_LEVEL, "trading_account_id:1,custom_id:group1,status:1", false);

    init_table(engine, cffex::mm::MSG_CUSTOM_PARAM, custom_param_table_str, sizeof(custom_param_table_str)/sizeof(const char*));
}
