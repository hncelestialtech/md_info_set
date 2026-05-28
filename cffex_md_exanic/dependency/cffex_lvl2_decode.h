#ifndef __CFFEX_LVL2_DECODE_H__
#define __CFFEX_LVL2_DECODE_H__

#include <stdbool.h>
#include <stdint.h>

#include "htf_cffex.h"

#ifdef __cplusplus
extern "C"  {
#endif

bool cffex_lvl2_init(const char *ini_file);

bool cffex_lvl2_decode(uint8_t *payload, int payload_len, CFFEXIncQuotaDataT *mkt);

#ifdef __cplusplus
}
#endif

#endif //__CFFEX_LVL2_DECODE_H__
