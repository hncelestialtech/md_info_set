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

const char *md_table_str[] = { // cffex::mm::MSG_MD
    "instrument_id:IO1901-C-3000,ask1_price:100,ask1_volume:20,bid1_price:98,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",
    "instrument_id:IO1901-C-3100,ask1_price:100,ask1_volume:20,bid1_price:98,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80",
    "instrument_id:IO1902-C-3000,ask1_price:100,ask1_volume:20,bid1_price:98,bid1_volume:10,last_price:100,upper_limit_price:120,down_limit_price:80"
};
const char *derived_md_table_str[] = { // cffex::mm::MSG_MD
    "instrument_id:IO1901-C-3000,theoretical_price:100",
    "instrument_id:IO1901-C-3100,theoretical_price:100",
    "instrument_id:IO1902-C-3000,theoretical_price:100"
};
/*
const char *portfolio_table_str[] = { // cffex::mm::MSG_PORTFOLIO
    "portfolio_id:2,portfolio_name:order,portfolio_status:1,portfolio_type:1",
    "portfolio_id:3,portfolio_name:hedge,portfolio_status:1,portfolio_type:1",
    "portfolio_id:4,portfolio_name:quote,portfolio_status:1,portfolio_type:1"
};*/
const char *portfolio_table_str[] = { // cffex::mm::MSG_PORTFOLIO
    "portfolio_id:2,trading_account_id:0,portfolio_type:1,portfolio_name:order,status:1,last_operator_id:1",
    "portfolio_id:3,trading_account_id:0,portfolio_type:1,portfolio_name:hedge,status:1,last_operator_id:1",
    "portfolio_id:4,trading_account_id:0,portfolio_type:1,portfolio_name:quote,status:1,last_operator_id:1"
};
const char *instrument_table_str[] = { // cffex::mm::MSG_INSTRUMENT
    "instrument_id:IO1901-C-3000,tick:0.2,status:1,product_id:IO,option_serial_id:IO1901",
    "instrument_id:IO1901-C-3100,tick:0.2,status:1,product_id:IO,option_serial_id:IO1901",
    "instrument_id:IO1902-C-3000,tick:0.2,status:1,product_id:IO,option_serial_id:IO1902"
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
};
const char *custom_param_table_str[] = { // cffex::mm::MSG_CUSTOM_PARAM
    "custom_id:IO,trading_account_id:0,param_key:NetPosLimit,param_value:100,last_operator_id:1,last_operate_source:2,status:1",
    "custom_id:IO,trading_account_id:0,param_key:UpdateThreshold,param_value:0.5,last_operator_id:1,last_operate_source:2,status:1",
    "custom_id:IO1901,trading_account_id:0,param_key:QuoteTemplate,param_value:template1,last_operator_id:1,last_operate_source:2,status:1",
    "custom_id:IO1902,trading_account_id:0,param_key:QuoteTemplate,param_value:template1,last_operator_id:1,last_operate_source:2,status:1"
};
/*
const char *instrument_param_table_str[] = { // cffex::mm::MSG_CUSTOM_PARAM
    "instrument_id:IO1901-C-3000,param_key:Quoting,param_value:1,param_type:bool,status:1",
    "instrument_id:IO1901-C-3000,param_key:BidOffset,param_value:0,param_type:double,status:1",
    "instrument_id:IO1901-C-3000,param_key:AskOffset,param_value:0,param_type:double,status:1",
    "instrument_id:IO1901-C-3100,param_key:Quoting,param_value:1,param_type:bool,status:1",
    "instrument_id:IO1901-C-3100,param_key:BidOffset,param_value:0,param_type:double,status:1",
    "instrument_id:IO1901-C-3100,param_key:AskOffset,param_value:0,param_type:double,status:1",
    "instrument_id:IO1902-C-3000,param_key:Quoting,param_value:1,param_type:bool,status:1",
    "instrument_id:IO1902-C-3000,param_key:BidOffset,param_value:0,param_type:double,status:1",
    "instrument_id:IO1902-C-3000,param_key:AskOffset,param_value:0,param_type:double,status:1"
};
const char *custom_param_table_str[] = { // cffex::mm::MSG_CUSTOM_PARAM
    "custom_id:IO,param_key:NetPosLimit,param_value:100,param_type:int,status:1",
    "custom_id:IO,param_key:UpdateThreshold,param_value:1,param_type:double,status:1",
    "custom_id:IO1901,param_key:QuoteTemplate,param_value:template1,param_type:string,status:1",
    "custom_id:IO1902,param_key:QuoteTemplate,param_value:template1,param_type:string,status:1"
};
*/
const char *trading_time_template_table[] = {
    //"template_name:cffex,template_value:0,status:1"
};
const char *spread_template_str[] = {
    "template_name:template1,template_value:[1.0-200.0):10.0:0,template_type:1,status:1",
    "template_name:spread1,template_value:[1.0-99999.0):1.0:0,template_type:1,status:1"
};


void init_engine_data(cffex::mm::engine *engine) {
    init_table(engine, cffex::mm::MSG_MD, md_table_str, sizeof(md_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_OPTION_DERIVED_MD, derived_md_table_str, sizeof(derived_md_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_PORTFOLIO, portfolio_table_str, sizeof(portfolio_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_INSTRUMENT, instrument_table_str, sizeof(instrument_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_POSITION, position_table_str, sizeof(position_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_INSTRUMENT_PARAM, instrument_param_table_str, sizeof(instrument_param_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_CUSTOM_PARAM, custom_param_table_str, sizeof(custom_param_table_str)/sizeof(const char*));
    init_table(engine, cffex::mm::MSG_SPREAD_TEMPLATE, spread_template_str, sizeof(spread_template_str)/sizeof(const char*));
}
