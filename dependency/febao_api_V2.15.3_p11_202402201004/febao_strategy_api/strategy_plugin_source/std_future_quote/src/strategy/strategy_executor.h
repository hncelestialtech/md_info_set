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
 * Date: 2019-12-06
 */

#ifndef STRATEGY_EXECUTOR_H
#define STRATEGY_EXECUTOR_H

#include "auto_quote_rule.h"
#include "auto_order_rule.h"

namespace cffex {
namespace strategy {

class strategy_executor {
public:
    strategy_executor(int strategy_instance_id, cffex::fb::i_strategy *strategy);
    ~strategy_executor();
    bool start();
    bool stop();

private:
    void init_stream();
    bool set_filter();

    /* stream callbacks */
    void on_snap_md(md_msg_type *msg, bool is_last);
    void on_msg_md(md_msg_type *msg);
    void on_msg_custom_param(custom_param_msg_type *msg);
    void on_msg_position(position_msg_type *msg);
    void on_msg_trade(trade_msg_type *msg);
    void on_msg_quote(quote_msg_type *msg);
    void on_msg_quote_cancel(quote_cancel_msg_type *msg);
    void on_msg_order(order_msg_type *msg);
    void on_msg_order_cancel(order_cancel_msg_type *msg);


private:
    int                     strategy_instance_id_;
    caller_handler         *callers_;
    data_manager           *data_;

    const cffex::fb::i_filter    *instrument_filter_;
    const cffex::fb::i_filter    *custom_filter_;
    rule_impl                 *rule_;
};


}
}



#endif




