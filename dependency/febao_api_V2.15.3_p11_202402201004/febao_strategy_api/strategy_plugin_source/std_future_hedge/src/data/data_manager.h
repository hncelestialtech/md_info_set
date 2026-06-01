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

#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "stream_factory.h"

namespace cffex {
namespace strategy {

#define STRATEGY_NAME "future_hedge"

class data_manager : public stream_factory
{
public:
    data_manager(cffex::fb::i_strategy *obj) : stream_factory(obj) { }
    virtual ~data_manager() { }

    bool init();
    bool init_group_param();
    double init_tick(const char *instrument_id);

public:
    // instance string param
    char                            instrument_id[128];
    char                            portfolio_name[128];
    int                             hedge_cutloss_threshold;
    int                             hedge_cutloss_waiting;
    char                            custom_id[128];
    // group param
    int                             hedge_buy_shift;
    int                             hedge_sell_shift;
    int                             rehedge_shift;
    // data
    double                          tick;
};


}
}

#endif