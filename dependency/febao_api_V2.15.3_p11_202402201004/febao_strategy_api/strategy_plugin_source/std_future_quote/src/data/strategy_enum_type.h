/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: lisc
 * Date: 2019-12-09
 */

#ifndef STD_FUTURE_ENUM_TYPE_H
#define STD_FUTURE_ENUM_TYPE_H

namespace cffex {
namespace strategy {

enum {
    THRESHOLD_TICK_TYPE = 0,
    THRESHOLD_ABS_TYPE = 1
}; // threshold_type/0:tick;1:abs


enum {
    ORDER_MODE_TYPE = 0,
    QUOTE_NORMAL_MODE_TYPE = 1,
    QUOTE_POP_MODE_TYPE = 2,  /* exchange will auto cancel last quote */
}; // quote_mode_type/0:normal_quote;1:pop_quote;2:order

enum {
    INACTIVE_STATUS = 0,
    ACTIVE_STATUS = 1
}; // status_type/0:inactive;1:active

enum {
    HEDGE_TICK_TYPE = 0,
    HEDGE_ABS_TYPE = 1,
    HEDGE_PERCENT_TYPE = 2
}; // hedge_type/0:tick;1:abs;2:percent

enum {
    MD_SPREAD_MODE = 0,
    FIX_SPREAD_MODE = 1,
    TEMPLATE_SPREAD_MODE = 2
}; // spread_mode

}
}

#endif