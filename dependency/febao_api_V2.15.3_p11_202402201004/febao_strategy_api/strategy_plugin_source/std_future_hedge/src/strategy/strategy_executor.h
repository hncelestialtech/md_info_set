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

#ifndef STRATEGY_EXECUTOR_H
#define STRATEGY_EXECUTOR_H

#include "rule_hedge.h"
#include "caller_handler.h"

namespace cffex {
namespace strategy {

class strategy_executor
{
public:
    strategy_executor(int strategy_instance_id, cffex::fb::i_strategy *strategy);
    ~strategy_executor();

    bool start();
    bool stop();

private:
    void on_msg_order(order_msg_type *msg);
    void on_msg_order_cancel(order_cancel_msg_type *msg);
    void on_msg_trade(trade_msg_type *msg);
    void on_msg_custom_param(custom_param_msg_type *msg);
    void on_msg_md(md_msg_type *msg);

private:
    caller_handler                  *callers_;
    rule_hedge                      *rule_hedge_;
    data_manager                    *data_;
    const cffex::fb::i_filter       *instrument_filter_;
    int                              instance_id_;

};

}
}



#endif




