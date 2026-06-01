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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strategy_impl.h"
#include "main_entry.h"

#ifdef __cplusplus
extern "C"
{
#endif

void* create()
{
    fprintf(stdout, "my_strategy_so::%s\n", __FUNCTION__);
    return new cffex::strategy::strategy_impl();
}
void destroy(void* p)
{
    fprintf(stdout, "my_strategy_so::%s, p[%p]\n", __FUNCTION__, p);
    delete (cffex::strategy::strategy_impl *)p;
}

void get_strategy_enum_info(char enum_info[64][256], int *count)
{
    strcpy(enum_info[0], "threshold_type/0:tick;1:abs");
    strcpy(enum_info[1], "quote_mode_type/0:order;1:normal_quote;2:pop_quote");
    strcpy(enum_info[2], "status_type/0:inactive;1:active");
    strcpy(enum_info[3], "hedge_type/0:tick;1:abs;2:percent");
    *count = 4;
}

void get_strategy_param_info(char param_info[64][256], int *count)
{
    strcpy(param_info[0], "instrument_id/string/CFE_IF2103/0");

    strcpy(param_info[1], "trading_section/string/cffex/0");
    strcpy(param_info[2], "md_threshold_spread/double/5/0");
    strcpy(param_info[3], "md_threshold_spread_type/threshold_type/0/0.1");
    strcpy(param_info[4], "md_threshold_bid_depth/int/1/1");
    strcpy(param_info[5], "md_threshold_ask_depth/int/1/1");
    strcpy(param_info[6], "md_threshold_bid_volume/int/20/1");
    strcpy(param_info[7], "md_threshold_ask_volume/int/20/1");
    strcpy(param_info[8], "position_threshold_upper/int/50/1");
    strcpy(param_info[9], "position_threshold_lower/int/50/1");

    strcpy(param_info[10], "custom_id/string/group1/0");
    strcpy(param_info[11], "quote_mode/quote_mode_type/0/1");
    strcpy(param_info[12], "portfolio_name/string/default/0");
    strcpy(param_info[13], "bid_volume/int/1/1");
    strcpy(param_info[14], "ask_volume/int/1/1");
    strcpy(param_info[15], "quote_refresh_msec/int/2000/1");
    strcpy(param_info[16], "quote_delay_msec/int/2000/1");

    *count = 17;
}


void get_strategy_version(char version[32])
{
    const char* STRATEGY_VERSION = "v2.1";
    strcpy(version,STRATEGY_VERSION);
}

void get_strategy_api_version(char version[32]) {
    strcpy(version,STRATEGY_API_VERSION);
}

#ifdef __cplusplus
}
#endif


