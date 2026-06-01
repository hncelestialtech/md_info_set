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

#ifndef STD_FUTURE_AUTO_QUOTE_RULE_H
#define STD_FUTURE_AUTO_QUOTE_RULE_H

#include "rule_impl.h"
#include <set>

namespace cffex {
namespace strategy {

/* trading mode: insert quote  */
class auto_quote_rule : public rule_impl {
public:
    auto_quote_rule(caller_handler *callers, data_manager *data, int strategy_instance_id);

    virtual void cancel();
    virtual void do_insert();
    virtual void do_refresh();
    virtual void get_current_info(double *cur_bid_p, double *cur_ask_p);
    void on_msg_quote(quote_msg_type *msg);
private:
    void check_curr_quote(int64_t quote_id);

private:
    int64_t     quote_id_;
};


}
}

#endif