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

#ifndef STD_FUTURE_AUTO_ORDER_RULE_H
#define STD_FUTURE_AUTO_ORDER_RULE_H

#include "rule_impl.h"
#include <set>

namespace cffex {
namespace strategy {

/* trading mode: insert bi-order  */
class auto_order_rule : public rule_impl {
public:
    auto_order_rule(caller_handler *callers, data_manager *data, int strategy_instance_id);

    virtual void cancel();
    virtual void do_insert();
    virtual void do_refresh();
    virtual void get_current_info(double *cur_bid_p, double *cur_ask_p);
    void on_msg_order(order_msg_type *msg);
    void check_curr_order(int64_t ask_id, int64_t bid_id);

private:
    int64_t                          bid_id_;
    int64_t                          ask_id_;
    int8_t                           bid_status_;
    int8_t                           ask_status_;
};


}
}

#endif