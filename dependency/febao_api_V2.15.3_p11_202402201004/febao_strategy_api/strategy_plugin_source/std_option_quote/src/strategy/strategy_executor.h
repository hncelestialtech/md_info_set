/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: --
 * Date: 2019-12-17
 */

#ifndef STRATEGY_EXECUTOR_H
#define STRATEGY_EXECUTOR_H

#include "rule_quote.h"

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
    bool set_filters();

    void on_snap_md(md_msg_type *msg, bool is_last);
    void on_msg_md(md_msg_type *msg);
    void on_msg_derived_md(derived_md_msg_type *msg);
    void on_snap_derived_md(derived_md_msg_type *msg, bool is_last);
    void on_msg_instrument_param(instrument_param_msg_type *msg);
    void on_msg_custom_param(custom_param_msg_type *msg);
    void on_msg_trade(trade_msg_type *msg);
    void on_msg_quote(quote_msg_type *msg);
    void on_msg_quote_cancel(quote_cancel_msg_type *msg);

private:
    bool init_serial(const char *serial);

    int                     strategy_instance_id_;
    caller_handler         *callers_;
    data_manager           *data_;

    const cffex::fb::i_filter    *serial_filter_;
    const cffex::fb::i_filter    *instance_filter_;

    rule_quote                   *rule_quote_;


};


}
}



#endif




