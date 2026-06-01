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

#ifndef MAIN_ENTRY_H
#define MAIN_ENTRY_H

#ifdef __cplusplus
extern "C"
{
#endif

void* create();
void destroy(void* p);
void get_strategy_param_info(char param_info[64][256], int *count);
void get_strategy_enum_info(char param_info[64][256], int *count);
void get_strategy_version(char version[32]);
void get_strategy_api_version(char version[32]);

#ifdef __cplusplus
}
#endif

#endif
