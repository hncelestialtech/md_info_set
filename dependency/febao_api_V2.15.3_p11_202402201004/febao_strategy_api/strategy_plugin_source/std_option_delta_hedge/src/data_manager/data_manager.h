/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: fy
 * Date: 2019-12-24
 */

#ifndef STD_DATA_MANAGER_H
#define STD_DATA_MANAGER_H

#include "stream_factory.h"
#include "caller_handler.h"

namespace cffex {
namespace strategy {

class data_manager : public stream_factory {
public:
    data_manager(cffex::fb::i_strategy *obj, caller_handler *callers);
    virtual ~data_manager();

    /* called by executor, prepare and check params */
    bool init_params();

    /* called by rules */
    bool  update_custom_param(custom_param_msg_type *msg);

private:
    bool init_instance_params();
    bool init_custom_params();
    bool init_common_params();

    caller_handler       *callers_;

public:
    /* instance param */
    char    portfolio_name[256];
    char    underlying_id[256];
    char    trading_section[256];
    double  trigger_threshold;
    double  target_threshold;
    int     md_max_spread;
    int     max_volume;
    double  interval;

    /* rule common param */
    double  tick;
    int     portfolio_id;

};


}
}

#endif
