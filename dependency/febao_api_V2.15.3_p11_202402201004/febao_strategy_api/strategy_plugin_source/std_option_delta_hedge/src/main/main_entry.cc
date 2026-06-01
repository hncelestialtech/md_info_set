/**
 * CFFEX Confidential.
 *
 * @Copyright 2019 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: wangty
 * Date: 2019-11-15
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

void get_strategy_param_info(char param_info[64][256], int *count) {
    /** name/type/value/tick_unit;  if value is price use tick_unit */
    strcpy(param_info[0], "portfolio_name/string/default/0");
    strcpy(param_info[1], "underlying_id/string/IF1912/0");
    strcpy(param_info[2], "trading_section/string/CFFEX/0");
    strcpy(param_info[3], "trigger_threshold/double/5/0");
    strcpy(param_info[4], "target_threshold/double/3/0");
    strcpy(param_info[5], "md_max_spread/int/10/1");
    strcpy(param_info[6], "max_volume/int/5/1");
    strcpy(param_info[7], "interval/double/1/0");
    *count = 8;
}

void get_strategy_enum_info(char param_info[64][256], int *count) {
    *count = 0;
}

void get_strategy_version_info(char version[32]) {
    const char* STRATEGY_VERSION = "v1.0";
    strcpy(version,STRATEGY_VERSION);
}

void get_strategy_api_version(char version[32]) {
    strcpy(version,STRATEGY_API_VERSION);
}


#ifdef __cplusplus
}
#endif