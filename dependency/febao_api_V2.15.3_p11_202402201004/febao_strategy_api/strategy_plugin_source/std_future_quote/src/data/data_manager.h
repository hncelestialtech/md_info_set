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

#ifndef STD_FUTURE_QUOTE_DATA_MANAGER_H
#define STD_FUTURE_QUOTE_DATA_MANAGER_H

#include "stream_factory.h"
#include "strategy_enum_type.h"
#include "caller_handler.h"

namespace cffex {
namespace strategy {

#define STRATEGY_NAME "std_future_quote"

class data_manager : public stream_factory {
public:
    data_manager(cffex::fb::i_strategy *obj, caller_handler *callers);
    virtual ~data_manager();

    /* called by executor, prepare and check params */
    bool init_params();

    /* called by rules */
    bool   update_custom_param(custom_param_msg_type *msg);
    double get_template_spread(double price);

private:
    bool init_instance_params();
    bool init_custom_params();
    bool init_common_params();

public:
    /* instance param */
    char    trading_section[256];
    double  md_threshold_spread;
    int     md_threshold_spread_type;
    int     md_threshold_bid_depth;
    int     md_threshold_ask_depth;
    int     md_threshold_bid_volume;
    int     md_threshold_ask_volume;
    int     position_threshold_upper;
    int     position_threshold_lower;

    char    instrument_id[256];
    char    custom_id[256];
    int     quote_mode;
    char    portfolio_name[256];
    int     bid_volume;
    int     ask_volume;
    int     quote_refresh_msec;
    int     quote_delay_msec;

    /* instrument group param */
    int     auto_quote_switch;
    int     baseprice_shift;
    int     spread_mode;
    double  fix_spread;
    char    spread_template_name[256];

    /* rule common param */
    double  tick;
    bool    valid_net_position;
    int8_t  exchange_id;

    caller_handler          *callers_;
};


}
}

#endif