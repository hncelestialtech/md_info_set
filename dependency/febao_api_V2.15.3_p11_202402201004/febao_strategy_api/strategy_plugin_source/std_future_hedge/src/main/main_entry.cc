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
 * Date: 2020-04-09
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

void get_strategy_param_info(char param_info[64][256], int *count)
{
    strcpy(param_info[0], "instrument_id/string/IF1901/0");
    strcpy(param_info[1], "portfolio_name/string/portfolio1/0");
    strcpy(param_info[2], "hedge_cutloss_threshold/int/10/0");
    strcpy(param_info[3], "hedge_cutloss_waiting/int/50/0");
    strcpy(param_info[4], "custom_id/string/custom_1/0");
    *count = 5;
}

void get_strategy_enum_info(char param_info[64][256], int *count)
{
    *count = 0;
}

void get_strategy_version(char version[32])
{
    const char* STRATEGY_VERSION = "v1.0";
    strcpy(version, STRATEGY_VERSION);
}

void get_strategy_api_version(char version[32]) {
    strcpy(version,STRATEGY_API_VERSION);
}

#ifdef __cplusplus
}
#endif


