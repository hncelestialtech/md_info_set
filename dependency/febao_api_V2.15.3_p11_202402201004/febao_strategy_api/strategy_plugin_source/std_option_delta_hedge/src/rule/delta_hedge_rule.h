/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: wangty
 * Date: 2019-11-11
 */

#ifndef FEBAO_STRATEGY_DELTA_HEDGE_RULE_H
#define FEBAO_STRATEGY_DELTA_HEDGE_RULE_H

#include "base_rule.h"

namespace cffex {
namespace strategy {

class delta_hedge_rule : public base_rule {
 public:
    delta_hedge_rule(caller_handler *callers, data_manager *data);

    void hedge();
    void hedge(double delta);

 private:
    bool check_trigger();
    bool check_interval();
    bool check_market();
    bool calculate_hedge_data();
    void insert_order();
    double get_instrument_delta(const char* instrument);
    void on_timer();

 private:
    double      portfolio_delta_;
    double      price_;
    int         volume_;
    int8_t      direction_;

    double      md_bid_;
    double      md_ask_;

    bool        pending_timer_;

};

}
}

#endif
