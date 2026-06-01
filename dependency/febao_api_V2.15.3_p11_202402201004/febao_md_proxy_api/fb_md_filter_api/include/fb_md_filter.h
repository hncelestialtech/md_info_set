/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: renjh
 * Date: 2022-05-16
 */


#ifndef CFFEX_FB_MD_FILTER_H
#define CFFEX_FB_MD_FILTER_H

#include "fb_md_msg.h"

#ifdef __cplusplus
extern "C"
{
#endif

void *create();

void destroy(void *p);

#ifdef __cplusplus
}
#endif

namespace cffex {
namespace fb {

class fb_md_filter {
public:
    virtual ~fb_md_filter() {}
    virtual bool on_msg(market_data_msg *old_msg, market_data_msg *new_msg) = 0; // true : pub this md, false : drop this md
};

}
}


#endif