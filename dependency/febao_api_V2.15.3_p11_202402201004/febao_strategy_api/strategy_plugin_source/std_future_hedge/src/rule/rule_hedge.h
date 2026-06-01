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

#ifndef RULE_HEDGE_H
#define RULE_HEDGE_H

#include <map>
#include "base_rule.h"

namespace cffex {
namespace strategy {

class rule_hedge : public base_rule
{
public:
    rule_hedge(caller_handler *callers, data_manager *data);
    virtual ~rule_hedge();

    void add_trade(int64_t trade_id);
    void check_hedge();
    void check_hedge_cutloss_threshold(md_msg_type *msg);

private:
    void do_hedge(int64_t trade_id);
    void do_rehedge(int64_t hedge_order_id);
    double get_hedge_price(int64_t trade_id);
    double get_rehedge_price(int8_t direction, double hedge_price);
    int8_t get_offset();

private:
    typedef std::map<int64_t, int64_t>  TRADES;
    typedef std::set<int64_t>           HEDGES;

    TRADES                              hedges_;
    TRADES                              rehedges_;
    TRADES                              timers_;
    HEDGES                              cancel_hedges_;
    int                                 error_timer_id_;

};

}
}

#endif