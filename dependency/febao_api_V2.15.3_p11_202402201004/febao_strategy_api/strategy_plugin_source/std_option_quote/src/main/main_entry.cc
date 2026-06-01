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

void get_strategy_param_info(char param_info[64][256], int *count) {
    /** name/type/value/tick_unit;  if value is price use tick_unit */
    strcpy(param_info[0], "portfolio/string/default/0");
    strcpy(param_info[1], "volume/int/3/1");
    strcpy(param_info[2], "serial/string/IO_20190118/0");
    strcpy(param_info[3], "trading_section/string/CFFEX/0");
    strcpy(param_info[4], "base_mode/base_mode_type/0/0");
    strcpy(param_info[5], "trade_pending_second/double/3.0/1");
    strcpy(param_info[6], "replace_quote/int/0/0");
    *count = 7;
}

void get_strategy_enum_info(char param_info[64][256], int *count) {
    strcpy(param_info[0],"base_mode_type/0:market;1:theo");
    *count = 1;
}

void get_strategy_version_info(char version[32]) {
    const char* STRATEGY_VERSION = "v1.1";
    strcpy(version,STRATEGY_VERSION);
}

void get_strategy_api_version(char version[32]) {
    strcpy(version,STRATEGY_API_VERSION);
}


#ifdef __cplusplus
}
#endif


